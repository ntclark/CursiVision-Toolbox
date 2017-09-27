
#include "EnhancedReplicator.h"

   static POINTL lastMouse;
   static long oldActiveRegion = -1L;
   static templateDocument::tdUI *pTemplateDocumentUI = NULL;
   static long cornerGrabIndex = -1L;
   static RECT rSizeBase;

   LRESULT CALLBACK theReplicator::propertiesHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   theReplicator *p = (theReplicator *)GetWindowLong(hwnd,GWL_USERDATA);

   switch ( msg ) {

   case WM_INITDIALOG: {

      PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);

      p = (theReplicator *)pPage -> lParam;

      SetWindowLong(hwnd,GWL_USERDATA,(long)p);

      p -> hwndProperties = hwnd;

      p -> pIGProperties -> Push();
      p -> pIGProperties -> Push();

      char szInfo[1024];
      LoadString(hModule,IDS_EDIT_INSTRUCTIONS,szInfo,1024);
      SetDlgItemText(hwnd,IDDI_REPLICATOR_INSTRUCTIONS,szInfo);

      p -> load();

      IPrintingSupportProfile *px = NULL;

      p -> pICursiVisionServices -> get_PrintingSupportProfile(&px);

      if ( ! p -> pICursiVisionServices -> IsAdministrator() && px ) {
         RECT rc = {0};
         GetClientRect(hwnd,&rc);
         SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,rc.bottom - 32,0,0,SWP_NOSIZE);
         EnableWindow(hwnd,FALSE);
      } else
         ShowWindow(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),SW_HIDE);

      pTemplateDocumentUI = p -> pTemplateDocument -> createView(hwnd,16,80,theReplicator::drawSignatures);

      SetTimer(hwnd,1,300,NULL);

      }
      return LRESULT(FALSE);

   case WM_TIMER: {
      KillTimer(hwnd,1);
      p -> drawSignatures(NULL,pTemplateDocumentUI);
      }
      break;

   case WM_DESTROY: {
      if ( pTemplateDocumentUI )
         pTemplateDocumentUI -> releaseView();
      if ( hActionMenu )
         DestroyMenu(hActionMenu);
      }
      break;

   case WM_MOUSEMOVE: {
      
      if ( ! pTemplateDocumentUI )
         break;

      POINTL ptlMouse = {LOWORD(lParam),HIWORD(lParam)};

      if ( ptlMouse.x == lastMouse.x && ptlMouse.y == lastMouse.y )
         break;

      if ( ptlMouse.x < pTemplateDocumentUI -> rcPageParentCoordinates.left || ptlMouse.x > pTemplateDocumentUI -> rcPageParentCoordinates.right || 
                  ptlMouse.y < pTemplateDocumentUI -> rcPageParentCoordinates.top || ptlMouse.y > pTemplateDocumentUI -> rcPageParentCoordinates.bottom ) 
         break;

      lastMouse.x = ptlMouse.x;
      lastMouse.y = ptlMouse.y;

      if ( 0 == (wParam & MK_LBUTTON) ) {

         if ( ! ( -1L == p -> replicationIndex ) ) {

            long cx = ptlMouse.x - p -> rightClickMousePoint.x;
            long cy = ptlMouse.y - p -> rightClickMousePoint.y;

            p -> rightClickMousePoint.x = ptlMouse.x;
            p -> rightClickMousePoint.y = ptlMouse.y;

            p -> replicationOrigin.x += cx;
            p -> replicationOrigin.y += cy;

            p -> drawSignatures(NULL,pTemplateDocumentUI);

            p -> drawSignature(NULL,p -> replicationIndex,p -> replicationOrigin.x,p -> replicationOrigin.y,NULL,pTemplateDocumentUI);

            break;

         }

         long oldActiveIndex = p -> activeIndex;

         p -> activeIndex = -1L;

         ptlMouse.x -= pTemplateDocumentUI -> rcPageParentCoordinates.left;
         ptlMouse.y -= pTemplateDocumentUI -> rcPageParentCoordinates.top;

         pTemplateDocumentUI -> convertToPoints(&ptlMouse);

         for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {

            if ( ! p -> pWritingLocations[k] )
               break;

            writingLocation *pSG = p -> pWritingLocations[k];

            if ( pSG -> zzpdfPageNumber != pTemplateDocumentUI -> currentPageNumber )
               continue;

            RECT r;

            memcpy(&r,&pSG -> documentRect,sizeof(RECT));

            if ( ptlMouse.x < r.left || ptlMouse.x > r.right || ptlMouse.y < r.bottom || ptlMouse.y > r.top )
               continue;

            p -> activeIndex = k;

            if ( oldActiveIndex != p -> activeIndex ) {
               if ( ! ( -1L == oldActiveIndex ) )
                  p -> drawSignature(NULL,oldActiveIndex,0L,0L,NULL,pTemplateDocumentUI);
               oldActiveIndex = p -> activeIndex;
               p -> drawSignature(NULL,p -> activeIndex,0L,0L,NULL,pTemplateDocumentUI);
            }

            if ( p -> isReplicant[k] ) {
               if ( abs(ptlMouse.x - r.left) < CORNER_PROXIMITY && abs(ptlMouse.y - r.top) < CORNER_PROXIMITY ) {
                  SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
                  cornerGrabIndex = 0;
               } else if ( abs(ptlMouse.x - r.left) < CORNER_PROXIMITY && abs(ptlMouse.y - r.bottom) < CORNER_PROXIMITY ) {
                  SetCursor(LoadCursor(NULL,IDC_SIZENESW));
                  cornerGrabIndex = 3;
               } else if ( abs(ptlMouse.x - r.right) < CORNER_PROXIMITY && abs(ptlMouse.y - r.top) < CORNER_PROXIMITY ) {
                  SetCursor(LoadCursor(NULL,IDC_SIZENESW));
                  cornerGrabIndex = 1;
               } else if ( abs(ptlMouse.x - r.right) < CORNER_PROXIMITY && abs(ptlMouse.y - r.bottom) < CORNER_PROXIMITY ) {
                  SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
                  cornerGrabIndex = 2;
               } else {
                  SetCursor(LoadCursor(NULL,IDC_HAND));
                  cornerGrabIndex = -1L;
               }
            } else {
               SetCursor(LoadCursor(NULL,IDC_ARROW));
               cornerGrabIndex = -1L;
            }

            if ( p -> isReplicant[k] )
               break;

         }

         if ( ! ( oldActiveIndex == p -> activeIndex ) ) {
            if ( ! ( -1L == oldActiveIndex ) )
               p -> drawSignature(NULL,oldActiveIndex,0L,0L,NULL,pTemplateDocumentUI);
         }

         if ( -1L == p -> activeIndex ) 
            SetCursor(LoadCursor(NULL,IDC_ARROW));

         break;

      }

      if ( -1L == p -> activeIndex )
         break;

      if ( ! p -> isReplicant[p -> activeIndex] ) {
         MessageBeep(MB_ICONASTERISK);
         break;
      }

      writingLocation *pSG = p -> pWritingLocations[p -> activeIndex];

      long deltaX = ptlMouse.x - p -> leftClickMousePoint.x;
      long deltaY = ptlMouse.y - p -> leftClickMousePoint.y;

      if ( ! ( -1L == cornerGrabIndex ) ) {

         if ( 0 == deltaX || 0 == deltaY )
            break;

         long nativeIndex = p -> replicantIndex[p -> activeIndex];

         deltaX = (long)((double)deltaX / pTemplateDocumentUI -> scaleToPixelsX);
         deltaY = (long)((double)deltaY / pTemplateDocumentUI -> scaleToPixelsY);

         double aspectRatio = (double)(p -> pWritingLocations[nativeIndex] -> documentRect.right - p -> pWritingLocations[nativeIndex] -> documentRect.left) / 
                                       (double)(p -> pWritingLocations[nativeIndex] -> documentRect.top - p -> pWritingLocations[nativeIndex] -> documentRect.bottom);

         deltaY = (long)((double)deltaX / aspectRatio);

         long moveX = pSG -> documentRect.left;
         long moveY = pSG -> documentRect.bottom;

         long padWidthPDFUnits = 0L;
         long padHeightPDFUnits = 0L;

         switch ( cornerGrabIndex ) {

         case 0:
            moveX = rSizeBase.left + deltaX;
            padWidthPDFUnits = rSizeBase.right - rSizeBase.left - deltaX;
            padHeightPDFUnits = rSizeBase.top - rSizeBase.bottom - deltaY;
            break;

         case 1:
            padWidthPDFUnits = rSizeBase.right - rSizeBase.left + deltaX;
            padHeightPDFUnits = rSizeBase.top - rSizeBase.bottom + deltaY;
            break;

         case 2:
            padWidthPDFUnits = rSizeBase.right - rSizeBase.left + deltaX;
            moveY = rSizeBase.bottom - deltaY;
            padHeightPDFUnits = rSizeBase.top - rSizeBase.bottom + deltaY;
            break;

         case 3:
            moveX = rSizeBase.left + deltaX;
            padWidthPDFUnits = rSizeBase.right - rSizeBase.left - deltaX;
            moveY = rSizeBase.bottom + deltaY;
            padHeightPDFUnits = rSizeBase.top - rSizeBase.bottom - deltaY;
            break;

         }

         pSG -> documentRect.right = pSG -> documentRect.left + padWidthPDFUnits;
         pSG -> documentRect.top = pSG -> documentRect.bottom + padHeightPDFUnits;

         p -> moveReplicant(p -> activeIndex,moveX,moveY,pTemplateDocumentUI -> currentPageNumber);

         p -> replicantDragOrigin.x = 0;
         p -> replicantDragOrigin.y = 0;

      } else {

         p -> replicantDragOrigin.x += deltaX;
         p -> replicantDragOrigin.y += deltaY;

         p -> leftClickMousePoint.x = ptlMouse.x;
         p -> leftClickMousePoint.y = ptlMouse.y;

      }

      HDC hdc = GetDC(pTemplateDocumentUI -> hwndVellum);

      long cxHTML = pTemplateDocumentUI -> rcPageParentCoordinates.right - pTemplateDocumentUI -> rcPageParentCoordinates.left;
      long cyHTML = pTemplateDocumentUI -> rcPageParentCoordinates.bottom - pTemplateDocumentUI -> rcPageParentCoordinates.top;

      BitBlt(hdc,0,0,cxHTML,cyHTML,pTemplateDocumentUI -> pdfDC(),0,0,SRCCOPY);

      for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {

         if ( ! p -> pWritingLocations[k] )
            break;

         if ( k == p -> activeIndex )
            continue;

         if ( p -> pWritingLocations[k] -> zzpdfPageNumber != pTemplateDocumentUI -> currentPageNumber ) 
            continue;

         p -> drawSignature(hdc,k,0L,0L,NULL,pTemplateDocumentUI);
         
      }

      p -> drawSignature(hdc,p -> activeIndex,p -> replicantDragOrigin.x,p -> replicantDragOrigin.y,NULL,pTemplateDocumentUI);

      ReleaseDC(pTemplateDocumentUI -> hwndVellum,hdc);

      }
      break;

   case WM_LBUTTONUP: {

      if ( -1L == p -> activeIndex )
         break;

      if ( ! p -> isReplicant[p -> activeIndex] )
         break;

      RECT rcNew;

      p -> drawSignature(NULL,p -> activeIndex,p -> replicantDragOrigin.x,p -> replicantDragOrigin.y,&rcNew,pTemplateDocumentUI);

      p -> moveReplicant(p -> activeIndex,rcNew.left,rcNew.bottom,pTemplateDocumentUI -> currentPageNumber);

      SetWindowText(GetDlgItem(hwnd,IDDI_REPLICATOR_ACTION_INSTRUCTIONS),"");

      p -> replicantDragOrigin.x = 0;

      p -> replicantDragOrigin.y = 0;

      }
      break;


   case WM_LBUTTONDOWN: {
   
      p -> leftClickMousePoint.x = LOWORD(lParam);
      p -> leftClickMousePoint.y = HIWORD(lParam);

      if ( ! ( -1L == p -> activeIndex ) ) {

         if ( ! p -> isReplicant[p -> activeIndex] )
            break;

         memcpy(&rSizeBase,&p -> pWritingLocations[p -> activeIndex] -> documentRect,sizeof(RECT));

         p -> replicantDragOrigin.x = 0;
         p -> replicantDragOrigin.y = 0;

         break;

      }

      if ( -1L == p -> replicationIndex )
         break;

      RECT rcNew;

      p -> drawSignature(NULL,p -> replicationIndex,p -> replicationOrigin.x,p -> replicationOrigin.y,&rcNew,pTemplateDocumentUI);

      p -> activeIndex = p -> addReplicant(pTemplateDocumentUI,p -> replicationIndex,rcNew.left,rcNew.bottom);

      p -> replicationIndex = -1L;

      SetWindowText(GetDlgItem(hwnd,IDDI_REPLICATOR_ACTION_INSTRUCTIONS),"");

      p -> replicationOrigin.x = 0;

      p -> replicationOrigin.y = 0;

      p -> drawSignatures(NULL,pTemplateDocumentUI);

      }
      break;

   case WM_RBUTTONDOWN: {
      p -> rightClickMousePoint.x = LOWORD(lParam);
      p -> rightClickMousePoint.y = HIWORD(lParam);
      }
      break;


   case WM_RBUTTONUP: {

      if ( -1L == p -> activeIndex )
         break;

      long x = LOWORD(lParam) - pTemplateDocumentUI -> rcPageParentCoordinates.left;
      long y = HIWORD(lParam) - pTemplateDocumentUI -> rcPageParentCoordinates.top;

      if ( hActionMenu )
         DestroyMenu(hActionMenu);

      hActionMenu = CreatePopupMenu();

      MENUITEMINFO menuItem = {0};

      menuItem.cbSize = sizeof(MENUITEMINFO);
      menuItem.fMask = MIIM_ID | MIIM_TYPE;
      menuItem.fType = MFT_STRING;
      menuItem.fState = MFS_ENABLED;

      menuItem.wID = IDDI_SIGNATURE_REGION_EVERY_PAGE;
      menuItem.dwTypeData = "Every Page";
      InsertMenuItem(hActionMenu,0,MF_BYPOSITION,&menuItem);

      if ( p -> isReplicant[p -> activeIndex] ) {
         menuItem.wID = IDDI_SIGNATURE_REGION_DELETE;
         menuItem.dwTypeData = "Delete";
      } else {
         menuItem.wID = IDDI_SIGNATURE_REGION_REPLICATE;
         menuItem.dwTypeData = "Replicate";
      }

      InsertMenuItem(hActionMenu,0,MF_BYPOSITION,&menuItem);

      RECT rcView;
      GetWindowRect(pTemplateDocumentUI -> hwndVellum,&rcView);

      TrackPopupMenu(hActionMenu,TPM_LEFTALIGN,rcView.left + x,rcView.top + y,0,hwnd,NULL);

      }
      break;

   case WM_COMMAND: {

      switch ( LOWORD(wParam) ) {

      case IDDI_REPLICATOR_RESET: {
         p -> reset();
         p -> drawSignatures(NULL,pTemplateDocumentUI);
         }
         break;

      case IDDI_SIGNATURE_REGION_DELETE: {
         if ( -1L == p -> activeIndex )
            break;
         p -> deleteReplicant(p -> activeIndex);
         p -> activeIndex = -1L;
         p -> drawSignatures(NULL,pTemplateDocumentUI);
         }
         break;

      case IDDI_SIGNATURE_REGION_REPLICATE: {
         if ( -1L == p -> activeIndex )
            break;
         p -> replicationIndex = p -> activeIndex;
         p -> replicationOrigin.x = 32;
         p -> replicationOrigin.y = 32;
         p -> activeIndex = -1L;
         SetCursor(LoadCursor(NULL,IDC_HAND));
         }
         break;

      case IDDI_SIGNATURE_REGION_EVERY_PAGE: {

         if ( -1L == p -> activeIndex )
            break;

         for ( long k = 1; k <= p -> pTemplateDocument -> PDFPageCount(); k++ ) {
            if ( k == pTemplateDocumentUI -> currentPageNumber )
               continue;
            if ( ! p -> duplicateReplicant(p -> activeIndex,
                                             p -> pWritingLocations[p -> activeIndex] -> documentRect.left,
                                               p -> pWritingLocations[p -> activeIndex] -> documentRect.top,k) )
               break;
         }

         }
         break;

      default:
         break;

      }

      }
      break;   

   case WM_SIZE: {

      if ( ! pTemplateDocumentUI )
         break;

      pTemplateDocumentUI -> size();

      RECT rcView,rcParent,rcReset;

      GetWindowRect(hwnd,&rcParent);
      GetWindowRect(pTemplateDocumentUI -> hwndVellum,&rcView);
      GetWindowRect(GetDlgItem(hwnd,IDDI_REPLICATOR_RESET),&rcReset);

      SetWindowPos(GetDlgItem(hwnd,IDDI_REPLICATOR_INSTRUCTIONS),HWND_TOP,0,0,
                     rcParent.right - rcParent.left - (rcReset.right - rcParent.left) - 2 * (rcView.left - rcParent.left),(rcView.top - rcParent.top) - 16,SWP_NOMOVE);

      }
      break;

   case WM_NOTIFY: {

      NMHDR *pNotifyHeader = (NMHDR *)lParam;

      switch ( pNotifyHeader -> code ) {

      case PSN_KILLACTIVE:
         SetWindowLong(pNotifyHeader -> hwndFrom,DWL_MSGRESULT,FALSE);
         break;

      case PSN_APPLY: {

         PSHNOTIFY *pNotify = (PSHNOTIFY *)lParam;

         if ( pNotify -> lParam ) {
            p -> unload();
            IPrintingSupportProfile *px = NULL;
            p -> pICursiVisionServices -> get_PrintingSupportProfile(&px);
            if ( p -> pICursiVisionServices -> IsAdministrator() || ! px )
               p -> pIGProperties -> Save();
            p -> pIGProperties -> Discard();
            p -> pIGProperties -> Discard();
         } else {
            p -> pIGProperties -> Discard();
            p -> pIGProperties -> Push();
         }

         SetWindowLong(pNotifyHeader -> hwndFrom,DWL_MSGRESULT,PSNRET_NOERROR);

         }
         break;

      case PSN_RESET: {
         p -> pIGProperties -> Pop();
         p -> pIGProperties -> Pop();
         }
         break;

      }

      }
      break;

   default:
      break;
   }

   return LRESULT(FALSE);
   }


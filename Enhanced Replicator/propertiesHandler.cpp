// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EnhancedReplicator.h"

   static POINTL lastMouse;
   static long oldActiveRegion = -1L;
   static templateDocument::tdUI *pTemplateDocumentUI = NULL;
   static long cornerGrabIndex = -1L;
   static RECT rSizeBase;

   LRESULT CALLBACK theReplicator::propertiesHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   theReplicator *p = (theReplicator *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

   switch ( msg ) {

   case WM_INITDIALOG: {

      PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);

      p = (theReplicator *)pPage -> lParam;

      SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)p);

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

      pTemplateDocumentUI = p -> pTemplateDocument -> createView(hwnd,16,80,theReplicator::clearBitmapsAndDrawSignatures);

      }
      return LRESULT(FALSE);

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

      if ( ptlMouse.x < pTemplateDocumentUI -> rcPageParentCoordinates.left || ptlMouse.x > pTemplateDocumentUI -> rcPageParentCoordinates.right || 
                  ptlMouse.y < pTemplateDocumentUI -> rcPageParentCoordinates.top || ptlMouse.y > pTemplateDocumentUI -> rcPageParentCoordinates.bottom ) 
         break;

      ptlMouse.x -= pTemplateDocumentUI -> rcPageParentCoordinates.left;
      ptlMouse.y -= pTemplateDocumentUI -> rcPageParentCoordinates.top;

      lastMouse.x = ptlMouse.x;
      lastMouse.y = ptlMouse.y;

      pTemplateDocumentUI -> resolveCurrentPageNumber(&ptlMouse);

      if ( 0 == (wParam & MK_LBUTTON) ) {

         long oldActiveIndex = p -> activeIndex;

         p -> activeIndex = -1L;

         if ( ptlMouse.x < pTemplateDocumentUI -> rcPDFPagePixelsInView.left || ptlMouse.x > pTemplateDocumentUI -> rcPDFPagePixelsInView.right || 
                     ptlMouse.y < pTemplateDocumentUI -> rcPDFPagePixelsInView.top || ptlMouse.y > pTemplateDocumentUI -> rcPDFPagePixelsInView.bottom ) 
            break;

         pTemplateDocumentUI -> convertToPoints(&ptlMouse);

         for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {

            if ( ! p -> pWritingLocations[k] )
               break;

            writingLocation *pSG = p -> pWritingLocations[k];

            if ( ! ( pSG -> zzpdfPageNumber == pTemplateDocumentUI -> currentPageNumber ) )
               continue;

            if ( ptlMouse.x < pSG -> documentRect.left - CORNER_PROXIMITY || ptlMouse.x > pSG -> documentRect.right + CORNER_PROXIMITY || 
                              ptlMouse.y < pSG -> documentRect.bottom - CORNER_PROXIMITY || ptlMouse.y > pSG -> documentRect.top + CORNER_PROXIMITY )
               continue;

            p -> activeIndex = k;

            if ( oldActiveIndex != p -> activeIndex ) {
               if ( ! ( -1L == oldActiveIndex ) )
                  p -> drawSignature(NULL,oldActiveIndex,0L,0L,NULL,pTemplateDocumentUI);
               oldActiveIndex = p -> activeIndex;
               p -> drawSignature(NULL,p -> activeIndex,0L,0L,NULL,pTemplateDocumentUI);
            }

            if ( p -> isReplicant[k] ) {

               if ( abs(ptlMouse.x - pSG -> documentRect.left) < CORNER_PROXIMITY && abs(ptlMouse.y - pSG -> documentRect.top) < CORNER_PROXIMITY ) {
                  SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
                  cornerGrabIndex = 0;
               } else if ( abs(ptlMouse.x - pSG -> documentRect.left) < CORNER_PROXIMITY && abs(ptlMouse.y - pSG -> documentRect.bottom) < CORNER_PROXIMITY ) {
                  SetCursor(LoadCursor(NULL,IDC_SIZENESW));
                  cornerGrabIndex = 3;
               } else if ( abs(ptlMouse.x - pSG -> documentRect.right) < CORNER_PROXIMITY && abs(ptlMouse.y - pSG -> documentRect.top) < CORNER_PROXIMITY ) {
                  SetCursor(LoadCursor(NULL,IDC_SIZENESW));
                  cornerGrabIndex = 1;
               } else if ( abs(ptlMouse.x - pSG -> documentRect.right) < CORNER_PROXIMITY && abs(ptlMouse.y - pSG -> documentRect.bottom) < CORNER_PROXIMITY ) {
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

      deltaX = (long)((double)deltaX / pTemplateDocumentUI -> scaleToPixels);
      deltaY = (long)((double)deltaY / pTemplateDocumentUI -> scaleToPixels);

      if ( 0 == deltaX || 0 == deltaY )
         break;

      if ( ! ( -1L == cornerGrabIndex ) ) {

         long nativeIndex = p -> replicantIndex[p -> activeIndex];

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
            SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
            break;

         case 1:
            padWidthPDFUnits = rSizeBase.right - rSizeBase.left + deltaX;
            padHeightPDFUnits = rSizeBase.top - rSizeBase.bottom + deltaY;
            SetCursor(LoadCursor(NULL,IDC_SIZENESW));
            break;

         case 2:
            padWidthPDFUnits = rSizeBase.right - rSizeBase.left + deltaX;
            moveY = rSizeBase.bottom - deltaY;
            padHeightPDFUnits = rSizeBase.top - rSizeBase.bottom + deltaY;
            SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
            break;

         case 3:
            moveX = rSizeBase.left + deltaX;
            padWidthPDFUnits = rSizeBase.right - rSizeBase.left - deltaX;
            moveY = rSizeBase.bottom + deltaY;
            padHeightPDFUnits = rSizeBase.top - rSizeBase.bottom - deltaY;
            SetCursor(LoadCursor(NULL,IDC_SIZENESW));
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

         SetCursor(LoadCursor(NULL,IDC_HAND));

      }

      HDC hdc = GetDC(pTemplateDocumentUI -> hwndPane);

      for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {

         if ( ! p -> pWritingLocations[k] )
            break;

         if ( k == p -> activeIndex ) {
            p -> clearSignature(pTemplateDocumentUI,k);
            continue;
         }

         p -> drawSignature(hdc,k,0L,0L,NULL,pTemplateDocumentUI);
         
      }

      p -> drawSignature(hdc,p -> activeIndex,p -> replicantDragOrigin.x,p -> replicantDragOrigin.y,NULL,pTemplateDocumentUI);

      ReleaseDC(pTemplateDocumentUI -> hwndPane,hdc);

      }
      break;

   case WM_LBUTTONUP: {

      if ( -1L == p -> activeIndex )
         break;

      POINTL ptlMouse = {LOWORD(lParam),HIWORD(lParam)};

      if ( ptlMouse.x < pTemplateDocumentUI -> rcPageParentCoordinates.left || ptlMouse.x > pTemplateDocumentUI -> rcPageParentCoordinates.right || 
                  ptlMouse.y < pTemplateDocumentUI -> rcPageParentCoordinates.top || ptlMouse.y > pTemplateDocumentUI -> rcPageParentCoordinates.bottom ) 
         break;

      ptlMouse.x -= pTemplateDocumentUI -> rcPageParentCoordinates.left;
      ptlMouse.y -= pTemplateDocumentUI -> rcPageParentCoordinates.top;

      pTemplateDocumentUI -> resolveCurrentPageNumber(&ptlMouse);

      if ( ! p -> isReplicant[p -> activeIndex] )
         break;

      RECT rcNew;

      p -> clearSignature(pTemplateDocumentUI,p -> activeIndex);

      writingLocation *pSG = p -> pWritingLocations[p -> activeIndex];

      memcpy(&rcNew,&pSG -> documentRect,sizeof(RECT));

      if ( ! ( pSG -> zzpdfPageNumber == pTemplateDocumentUI -> currentPageNumber ) ) {
         pTemplateDocumentUI -> convertToPanePixels(pSG -> zzpdfPageNumber,&rcNew);
         pTemplateDocumentUI -> convertToPoints(&rcNew);
         memcpy(&pSG -> documentRect,&rcNew,sizeof(RECT));
         pSG -> zzpdfPageNumber = pTemplateDocumentUI -> currentPageNumber;
      }

      rcNew.left = pSG -> documentRect.left + p -> replicantDragOrigin.x;
      rcNew.right = rcNew.left + pSG -> documentRect.right - pSG -> documentRect.left;
      rcNew.bottom = pSG -> documentRect.bottom - p -> replicantDragOrigin.y;
      rcNew.top = rcNew.bottom + pSG -> documentRect.top - pSG -> documentRect.bottom;

      p -> moveReplicant(p -> activeIndex,rcNew.left,rcNew.bottom,pTemplateDocumentUI -> currentPageNumber);

      p -> drawSignature(NULL,p -> activeIndex,0,0,NULL,pTemplateDocumentUI);

      SetWindowText(GetDlgItem(hwnd,IDDI_REPLICATOR_ACTION_INSTRUCTIONS),"");

      p -> replicantDragOrigin.x = 0;

      p -> replicantDragOrigin.y = 0;

      }
      break;


   case WM_LBUTTONDOWN: {

      POINTL ptlMouse = {LOWORD(lParam),HIWORD(lParam)};

      if ( ptlMouse.x < pTemplateDocumentUI -> rcPageParentCoordinates.left || ptlMouse.x > pTemplateDocumentUI -> rcPageParentCoordinates.right || 
                  ptlMouse.y < pTemplateDocumentUI -> rcPageParentCoordinates.top || ptlMouse.y > pTemplateDocumentUI -> rcPageParentCoordinates.bottom ) 
         break;

      ptlMouse.x -= pTemplateDocumentUI -> rcPageParentCoordinates.left;
      ptlMouse.y -= pTemplateDocumentUI -> rcPageParentCoordinates.top;

      pTemplateDocumentUI -> resolveCurrentPageNumber(&ptlMouse);
   
      p -> leftClickMousePoint.x = ptlMouse.x;
      p -> leftClickMousePoint.y = ptlMouse.y;

      if ( ! ( -1L == p -> activeIndex ) ) {

         if ( ! p -> isReplicant[p -> activeIndex] )
            break;

         memcpy(&rSizeBase,&p -> pWritingLocations[p -> activeIndex] -> documentRect,sizeof(RECT));

         p -> replicantDragOrigin.x = 0;
         p -> replicantDragOrigin.y = 0;

         break;

      }

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

      menuItem.wID = IDDI_SIGNATURE_REGION_EVERY_PAGE;
      menuItem.dwTypeData = "Every Subsequent Page";
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
      GetWindowRect(pTemplateDocumentUI -> hwndPane,&rcView);

      TrackPopupMenu(hActionMenu,TPM_LEFTALIGN,rcView.left + x,rcView.top + y,0,hwnd,NULL);

      }
      break;

   case WM_COMMAND: {

      switch ( LOWORD(wParam) ) {

      case IDDI_REPLICATOR_RESET: {
         p -> reset();
         p -> clearPage(pTemplateDocumentUI);
         p -> drawSignatures(NULL,pTemplateDocumentUI);
         }
         break;

      case IDDI_SIGNATURE_REGION_DELETE: {
         if ( -1L == p -> activeIndex )
            break;
         p -> deleteReplicant(p -> activeIndex);
         p -> activeIndex = -1L;
         p -> clearPage(pTemplateDocumentUI);
         p -> drawSignatures(NULL,pTemplateDocumentUI);
         }
         break;

      case IDDI_SIGNATURE_REGION_REPLICATE: {
         if ( -1L == p -> activeIndex )
            break;
         RECT rcNew;
         long keepIndex = p -> activeIndex;
         p -> activeIndex = -1L;
         p -> drawSignature(NULL,keepIndex,0,0,NULL,pTemplateDocumentUI);
         p -> activeIndex = keepIndex;
         p -> drawSignature(NULL,p -> activeIndex,32,32,&rcNew,pTemplateDocumentUI);
         p -> addReplicant(pTemplateDocumentUI,p -> activeIndex,rcNew.left,rcNew.bottom);
         p -> activeIndex = -1L;
         SetWindowText(GetDlgItem(hwnd,IDDI_REPLICATOR_ACTION_INSTRUCTIONS),"");
         }
         break;

      case IDDI_SIGNATURE_REGION_EVERY_PAGE: {

         if ( -1L == p -> activeIndex )
            break;

         for ( long k = 1; k <= p -> pTemplateDocument -> PDFPageCount(); k++ ) {
            if ( k == pTemplateDocumentUI -> currentPageNumber )
               continue;
            if ( ! p -> duplicateReplicant(p -> activeIndex,k) )
               break;
         }

         }
         break;

      case IDDI_SIGNATURE_REGION_EVERY_SUBSEQUENT_PAGE: {

         if ( -1L == p -> activeIndex )
            break;

         for ( long k = pTemplateDocumentUI -> currentPageNumber + 1; k <= p -> pTemplateDocument -> PDFPageCount(); k++ ) {
            if ( ! p -> duplicateReplicant(p -> activeIndex,k) )
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
      GetWindowRect(pTemplateDocumentUI -> hwndPane,&rcView);
      GetWindowRect(GetDlgItem(hwnd,IDDI_REPLICATOR_RESET),&rcReset);

      SetWindowPos(GetDlgItem(hwnd,IDDI_REPLICATOR_INSTRUCTIONS),HWND_TOP,0,0,
                     rcParent.right - rcParent.left - (rcReset.right - rcParent.left) - 2 * (rcView.left - rcParent.left),(rcView.top - rcParent.top) - 16,SWP_NOMOVE);

      }
      break;

   case WM_NOTIFY: {

      NMHDR *pNotifyHeader = (NMHDR *)lParam;

      switch ( pNotifyHeader -> code ) {

      case PSN_KILLACTIVE:
         SetWindowLongPtr(pNotifyHeader -> hwndFrom,DWLP_MSGRESULT,FALSE);
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

         SetWindowLongPtr(pNotifyHeader -> hwndFrom,DWLP_MSGRESULT,PSNRET_NOERROR);

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


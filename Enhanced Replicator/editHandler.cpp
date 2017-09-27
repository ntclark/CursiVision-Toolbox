
#include "EnhancedReplicator.h"

#if 0
   LRESULT CALLBACK documentView::editHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   documentView *p = (documentView *)GetWindowLong(hwnd,GWL_USERDATA);

   switch ( msg ) {

   case WM_INITDIALOG: {

      p = (documentView *)lParam;

      SetWindowLong(hwnd,GWL_USERDATA,(long)p);

      char szInfo[1024];
      LoadString(hModule,IDS_EDIT_INSTRUCTIONS,szInfo,1024);
      SetDlgItemText(hwnd,IDDI_REPLICATOR_INSTRUCTIONS,szInfo);

      char *pDocument = NULL;

      if ( p -> pParent -> pIPrintingSupportProfile )
         pDocument = p -> pParent -> pIPrintingSupportProfile -> DocumentName();
   
      else if ( p -> pParent -> pICursiVisionServices )
         pDocument = p -> pParent -> pICursiVisionServices -> DocumentName();

      if ( pDocument ) {

         HRESULT rc = CoCreateInstance(CLSID_PdfEnabler,NULL,CLSCTX_INPROC_SERVER,IID_IPdfEnabler,reinterpret_cast<void **>(&p -> pIPdfEnabler));

         p -> pIPdfEnabler -> Document(&p -> pIPdfDocument);
         BSTR bstrDocument = SysAllocStringLen(NULL,strlen(pDocument));
         MultiByteToWideChar(CP_ACP,0,pDocument,-1,bstrDocument,strlen(pDocument));
         p -> pIPdfDocument -> Open(bstrDocument,NULL,NULL);
         SysFreeString(bstrDocument);

         p -> pIPdfDocument -> get_PageCount(&p -> pageCount);

         p -> currentPageNumber = 1L;

         p -> pIPdfDocument -> Page(p -> currentPageNumber,NULL,&p -> pIPdfPage);

         RECT rcView,rcParent;

         GetWindowRect(hwnd,&rcParent);
         GetWindowRect(GetDlgItem(hwnd,IDDI_REPLICATOR_VIEW),&rcView);

         p -> hwndView = CreateWindowEx(WS_EX_CLIENTEDGE,"documentView","",WS_CHILD | WS_VISIBLE | WS_TABSTOP,rcView.left - rcParent.left,rcView.top - rcParent.top,rcView.right - rcView.left,rcView.bottom - rcView.top,hwnd,(HMENU)IDDI_REPLICATOR_VIEW,hModule,p);

         DestroyWindow(GetDlgItem(hwnd,IDDI_REPLICATOR_VIEW));

         p -> hwndScroll = GetDlgItem(hwnd,IDDI_REPLICATOR_SCROLL);

         p -> hwndActionInstructions = GetDlgItem(hwnd,IDDI_REPLICATOR_ACTION_INSTRUCTIONS);

         si.cbSize = sizeof(SCROLLINFO);
         si.nPage = 1;
         si.nPos = 1;
         si.nMin = 1;
         si.nMax = p -> pageCount;
         si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;

         SetScrollInfo(p -> hwndScroll,SB_CTL,&si,TRUE);

      }

      p -> load();

      p -> activeIndex = -1L;

      p -> replicationIndex = -1L;

      if ( p -> pParent -> rcDialog.left != p -> pParent -> rcDialog.right )
         SetWindowPos(hwnd,HWND_TOP,p -> pParent -> rcDialog.left,p -> pParent -> rcDialog.top,p -> pParent -> rcDialog.right - p -> pParent -> rcDialog.left,p -> pParent -> rcDialog.bottom - p -> pParent -> rcDialog.top,0L);
      else {
         RECT rcThis;
         GetClientRect(hwnd,&rcThis);
         SetWindowPos(hwnd,HWND_TOP,0,0,rcThis.right - rcThis.left,rcThis.bottom - rcThis.top,SWP_NOMOVE);
      }

      }
      return (LRESULT)IDDI_REPLICATOR_VIEW;

   case WM_CHAR:
   case WM_KEYUP:
   case WM_KEYDOWN:

      if ( VK_NEXT == wParam || VK_PRIOR == wParam )
         return SendMessage(p -> pParent -> hwndProperties,msg,wParam,lParam);

      if ( VK_ESCAPE == wParam && -1L != p -> replicationIndex )
         return SendMessage(p -> pParent -> hwndProperties,msg,wParam,lParam);

      break;

#if 0
   case WM_COMMAND: {

      switch ( LOWORD(wParam) ) {

      case IDDI_REPLICATOR_RESET: {
//         if ( IDNO == MessageBox(hwnd,"This action is irreversable, do you want to clear the replicated signatures ?","Note!",MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2) )
//            break;
         p -> reset();
         p -> drawDocument();
         }
         break;

      case IDDI_REPLICATOR_OK: {
         GetWindowRect(hwnd,&p -> pParent -> rcDialog);
         EndDialog(hwnd,1L);
         }
         break;

      case IDDI_REPLICATOR_CANCEL: {
         GetWindowRect(hwnd,&p -> pParent -> rcDialog);
         EndDialog(hwnd,0L);
         }
         break;

      default:
         SendMessage(GetParent(hwnd),msg,wParam,lParam);
         break;

      }

      }
      break;   

   case WM_SIZE: {

      RECT rcScroll,rcDocument,rcOk,rcCancel,rcParent,rcAdjust;

      memset(&rcAdjust,0,sizeof(RECT));
      AdjustWindowRect(&rcAdjust,GetWindowLong(hwnd,GWL_STYLE),FALSE);

      GetWindowRect(hwnd,&rcParent);
      GetWindowRect(p -> hwndScroll,&rcScroll);
      GetWindowRect(p -> hwndView,&rcDocument);
      GetWindowRect(GetDlgItem(hwnd,IDDI_REPLICATOR_OK),&rcOk);
      GetWindowRect(GetDlgItem(hwnd,IDDI_REPLICATOR_CANCEL),&rcCancel);

      rcDocument.right = rcDocument.left + LOWORD(lParam) - 2 * (rcDocument.left - rcParent.left) + rcAdjust.left;
      rcDocument.bottom = rcDocument.top + HIWORD(lParam) - (rcDocument.top - rcParent.top) + rcAdjust.top;

      SetWindowPos(p -> hwndScroll,HWND_TOP,rcDocument.right - rcParent.left + rcAdjust.left,rcDocument.top - rcParent.top + rcAdjust.top,GetSystemMetrics(SM_CXVSCROLL),rcDocument.bottom - rcDocument.top,0L);

      SetWindowPos(GetDlgItem(hwnd,IDDI_REPLICATOR_OK),HWND_TOP,rcDocument.left - rcParent.left + rcAdjust.left,rcDocument.bottom - rcParent.top + rcAdjust.top + 8,0,0,SWP_NOSIZE);

      SetWindowPos(GetDlgItem(hwnd,IDDI_REPLICATOR_CANCEL),HWND_TOP,rcDocument.left - rcParent.left + rcAdjust.left + (rcOk.right - rcOk.left) + 8,rcDocument.bottom - rcParent.top + rcAdjust.top + 8,0,0,SWP_NOSIZE);

      SetWindowPos(GetDlgItem(hwnd,IDDI_REPLICATOR_ACTION_INSTRUCTIONS),HWND_TOP,
                           rcDocument.left - rcParent.left + rcAdjust.left + (rcOk.right - rcOk.left + 16) + (rcCancel.right - rcCancel.left + 16),rcDocument.bottom - rcParent.top + rcAdjust.top + 8,rcDocument.right - rcDocument.left - (rcOk.right - rcOk.left + 16) - (rcCancel.right - rcCancel.left + 16),rcOk.bottom - rcOk.top,0L);

      RECT rcInstructions;
      GetClientRect(GetDlgItem(hwnd,IDDI_REPLICATOR_INSTRUCTIONS),&rcInstructions);
      SetWindowPos(GetDlgItem(hwnd,IDDI_REPLICATOR_INSTRUCTIONS),HWND_TOP,0,0,rcDocument.right - rcDocument.left,rcInstructions.bottom - rcInstructions.top,SWP_NOMOVE);

      SetWindowPos(p -> hwndView,HWND_TOP,0,0,rcDocument.right - rcDocument.left,rcDocument.bottom - rcDocument.top,SWP_NOMOVE);

      }
      break;

   case WM_VSCROLL: {

      SCROLLINFO si = {0};

      si.cbSize = sizeof(SCROLLINFO);

      si.fMask = SIF_ALL;

      GetScrollInfo(p -> hwndScroll,SB_CTL,&si);

      switch ( LOWORD(wParam) ) {
      case SB_LINEDOWN:
      case SB_PAGEDOWN:
         si.nPos += 1;
         break;

      case SB_LINEUP:
      case SB_PAGEUP:
         si.nPos -= 1;
         break;

      case SB_THUMBPOSITION:
         si.nPos = HIWORD(wParam);
         break;

      default:
         break;
      }

      SetScrollInfo(p -> hwndScroll,SB_CTL,&si,TRUE);
      GetScrollInfo(p -> hwndScroll,SB_CTL,&si);

      p -> currentPageNumber = si.nPos;

      InvalidateRect(p -> hwndView,NULL,TRUE);

      }
      break;

   case WM_DESTROY:
      if ( p -> hActionMenu )
         DestroyMenu(p -> hActionMenu);
      DestroyWindow(p -> hwndView);
      break;

#endif

   default:
      break;
   }
   
   return LRESULT(FALSE);
   }
#endif

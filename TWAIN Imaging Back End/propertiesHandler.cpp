// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ImagingBackEnd.h"
#include <commctrl.h>
#include <shTypes.h>
#include <shlobj.h>

extern "C" int GetDocumentsLocation(HWND hwnd,char *);

#include "dispositionSettingsDefines.h"

#define OBJECT_WITH_PROPERTIES ImagingBackEnd

#define LOAD_ADDITIONAL                                          \
{                                                                \
   PUT_BOOL(p -> doProperties,IDDI_SHOW_PROPERTIES);        \
   PUT_BOOL(pObject -> skipImaging,IDDI_SKIP);              \
   PUT_BOOL(pObject -> scanOne,IDDI_SCAN_ONE);                   \
   PUT_BOOL(pObject -> scanMultiple,IDDI_SCAN_MULTIPLE);         \
   PUT_BOOL(pObject -> specifyPage,IDDI_PAGE_PAGENO_OPT);   \
   PUT_BOOL(pObject -> lastPage,IDDI_PAGE_ONLAST);          \
   PUT_BOOL(pObject -> newLastPage,IDDI_PAGE_NEWLAST);      \
   PUT_BOOL(pObject -> keepImage,IDDI_POSITION_KEEP_IMAGE); \
   PUT_DOUBLE(pObject -> inchesLeft,IDDI_POSITION_FROMLEFT);\
   PUT_DOUBLE(pObject -> inchesTop,IDDI_POSITION_FROMTOP);  \
   PUT_BOOL(pObject -> fitToPage,IDDI_SIZE_FITTOPAGE);      \
   PUT_DOUBLE(pObject -> inchesWidth,IDDI_SIZE_WIDTH);      \
   PUT_DOUBLE(pObject -> inchesHeight,IDDI_SIZE_HEIGHT);    \
   PUT_BOOL(pObject -> keepAspectRatio,IDDI_SIZE_MAINTAIN_ASPECT_RATIO); \
   PUT_LONG(pObject -> pageNumber,IDDI_PAGE_PAGENO);        \
}

#define UNLOAD_ADDITIONAL \
{  \
   GET_STRING(pObject -> szChosenDevice,IDDI_IMAGER);             \
   GET_BOOL(p -> doProperties,IDDI_SHOW_PROPERTIES);        \
   GET_BOOL(pObject -> skipImaging,IDDI_SKIP);              \
   GET_BOOL(pObject -> scanOne,IDDI_SCAN_ONE);                    \
   GET_BOOL(pObject -> scanMultiple,IDDI_SCAN_MULTIPLE);          \
   GET_BOOL(pObject -> specifyPage,IDDI_PAGE_PAGENO_OPT);   \
   GET_BOOL(pObject -> lastPage,IDDI_PAGE_ONLAST);          \
   GET_BOOL(pObject -> newLastPage,IDDI_PAGE_NEWLAST);      \
   GET_BOOL(pObject -> keepImage,IDDI_POSITION_KEEP_IMAGE); \
   GET_DOUBLE(pObject -> inchesLeft,IDDI_POSITION_FROMLEFT);\
   GET_DOUBLE(pObject -> inchesTop,IDDI_POSITION_FROMTOP);  \
   GET_BOOL(pObject -> fitToPage,IDDI_SIZE_FITTOPAGE);      \
   GET_DOUBLE(pObject -> inchesWidth,IDDI_SIZE_WIDTH);      \
   GET_DOUBLE(pObject -> inchesHeight,IDDI_SIZE_HEIGHT);    \
   GET_BOOL(pObject -> keepAspectRatio,IDDI_SIZE_MAINTAIN_ASPECT_RATIO); \
   GET_LONG(pObject -> pageNumber,IDDI_PAGE_PAGENO);        \
}

   BOOL CALLBACK adjustTop(HWND hwndTest,LPARAM lParam);
   BOOL CALLBACK page1(HWND hwndTest,LPARAM lParam);
   BOOL CALLBACK page2(HWND hwndTest,LPARAM lParam);
   BOOL CALLBACK page3(HWND hwndTest,LPARAM lParam);

   LRESULT CALLBACK ImagingBackEnd::propertiesHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   resultDisposition *p = (resultDisposition *)GetWindowLongPtr(hwnd,GWLP_USERDATA);
   ImagingBackEnd *pObject = NULL;
   if ( p )
      pObject = (ImagingBackEnd *)(p -> pParent);

   switch ( msg ) {

   case WM_INITDIALOG: {

      PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);
      p = (resultDisposition *)pPage -> lParam;
      SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)p);

      pObject = (ImagingBackEnd *)(p -> pParent);

      pObject -> PushProperties();

      pObject -> PushProperties();

      SendDlgItemMessage(hwnd,IDDI_PAGE_PAGENO_OPT,BM_SETCHECK,0L,0L);
      SendDlgItemMessage(hwnd,IDDI_PAGE_ONLAST,BM_SETCHECK,0L,0L);
      SendDlgItemMessage(hwnd,IDDI_PAGE_NEWLAST,BM_SETCHECK,0L,0L);

#if 0
      if ( pObject -> skipImaging )
         SendDlgItemMessage(hwnd,IDDI_SKIP,BM_SETCHECK,(WPARAM)BST_CHECKED,0L);
      else if ( pObject -> scanOne )
         SendDlgItemMessage(hwnd,IDDI_SCAN_ONE,BM_SETCHECK,(WPARAM)BST_CHECKED,0L);
      else if ( pObject -> scanMultiple )
         SendDlgItemMessage(hwnd,IDDI_SCAN_MULTIPLE,BM_SETCHECK,(WPARAM)BST_CHECKED,0L);   
#endif

      if ( pObject -> specifyPage )
         SendDlgItemMessage(hwnd,IDDI_PAGE_PAGENO_OPT,BM_SETCHECK,(WPARAM)BST_CHECKED,0L);
      else if ( pObject -> lastPage )
         SendDlgItemMessage(hwnd,IDDI_PAGE_ONLAST,BM_SETCHECK,(WPARAM)BST_CHECKED,0L);
      else 
         SendDlgItemMessage(hwnd,IDDI_PAGE_NEWLAST,BM_SETCHECK,(WPARAM)BST_CHECKED,0L);

      EnableWindow(GetDlgItem(hwnd,IDDI_PAGE_PAGENO),pObject -> specifyPage);

      if ( pObject -> fitToPage ) {
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_HEIGHT),FALSE);
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_WIDTH),FALSE);
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_MAINTAIN_ASPECT_RATIO),FALSE);
         SendDlgItemMessage(hwnd,IDDI_SIZE_FITTOPAGE,BM_SETCHECK,(WPARAM)BST_CHECKED,0L);
      } else {
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_WIDTH),TRUE);
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_HEIGHT),TRUE);
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_MAINTAIN_ASPECT_RATIO),TRUE);
         SendDlgItemMessage(hwnd,IDDI_SIZE_FITTOPAGE,BM_SETCHECK,(WPARAM)BST_UNCHECKED,0L);
      }

      EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_HEIGHT),! pObject -> keepAspectRatio);

      TCITEM tcItem = {0};

      tcItem.pszText = "Scanner";
      tcItem.mask = TCIF_TEXT;

      SendDlgItemMessage(hwnd,IDDI_TABS,TCM_INSERTITEM,0,(LPARAM)&tcItem);

      tcItem.pszText = "Location";
      SendDlgItemMessage(hwnd,IDDI_TABS,TCM_INSERTITEM,1,(LPARAM)&tcItem);

      EnumChildWindows(hwnd,page1,NULL);

      tcItem.pszText = DISPOSITION_TITLE;
      SendDlgItemMessage(hwnd,IDDI_TABS,TCM_INSERTITEM,2,(LPARAM)&tcItem);

      EnumChildWindows(hwnd,page1,NULL);

      LOAD_CONTROLS

      LOAD_ADDITIONAL

      TW_UINT16 ret = dsmEntryProcedure(&pObject -> twainIdentity,0,DG_CONTROL,DAT_PARENT,MSG_OPENDSM,(TW_MEMREF)&hwnd);

      TW_IDENTITY *pSource = new TW_IDENTITY;

      memset(pSource,0,sizeof(TW_IDENTITY));

      if ( 0 == pObject -> twainSources.size() ) {

         TW_IDENTITY *pSource = new TW_IDENTITY;

         memset(pSource,0,sizeof(TW_IDENTITY));

         ret = dsmEntryProcedure(&pObject -> twainIdentity,0,DG_CONTROL,DAT_IDENTITY,MSG_GETFIRST,(TW_MEMREF)pSource);

         while ( TWRC_SUCCESS == ret ) {
            pObject -> twainSources.insert(pObject -> twainSources.end(),pSource);
            pSource = new TW_IDENTITY;
            memset(pSource,0,sizeof(TW_IDENTITY));
            ret = dsmEntryProcedure(&pObject -> twainIdentity,0,DG_CONTROL,DAT_IDENTITY,MSG_GETNEXT,(TW_MEMREF)pSource);
         }

         delete pSource;

      }

      TW_UINT16 rc = dsmEntryProcedure(&pObject -> twainIdentity,0,DG_CONTROL,DAT_IDENTITY,MSG_GETFIRST,(TW_MEMREF)pSource);

      while ( TWRC_SUCCESS == rc ) {
         if ( 1 == 1 || strncmp("WIA-",pSource -> ProductName,4) ) {
            long index = (long)SendDlgItemMessage(hwnd,IDDI_IMAGER,CB_INSERTSTRING,0L,(WPARAM)pSource -> ProductName);
            if ( 0 == strcmp(pObject -> szChosenDevice,pSource -> ProductName) )
               SendDlgItemMessage(hwnd,IDDI_IMAGER,CB_SETCURSEL,index,0L);
            pObject -> twainSources.insert(pObject -> twainSources.end(),pSource);
         } else
            delete pSource;
         pSource = new TW_IDENTITY;
         memset(pSource,0,sizeof(TW_IDENTITY));
         rc = dsmEntryProcedure(&pObject -> twainIdentity,0,DG_CONTROL,DAT_IDENTITY,MSG_GETNEXT,(TW_MEMREF)pSource);
      }

      ret = dsmEntryProcedure(&pObject -> twainIdentity,0,DG_CONTROL,DAT_PARENT,MSG_CLOSEDSM,(TW_MEMREF)&pObject -> hwndParent);

      RECT rcTabs;

      memset(&rcTabs,0,sizeof(RECT));

      SendDlgItemMessage(hwnd,IDDI_TABS,TCM_ADJUSTRECT,(WPARAM)TRUE,(LPARAM)&rcTabs);

      long deltaY = rcTabs.bottom - rcTabs.top;

      EnumChildWindows(hwnd,adjustTop,deltaY);

      IPrintingSupportProfile *px = NULL;

      pObject -> pICursiVisionServices -> get_PrintingSupportProfile(&px);
      if ( ! pObject -> pICursiVisionServices -> IsAdministrator() && px ) {
         RECT rc = {0};
         GetClientRect(hwnd,&rc);
         SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,rc.bottom - 32,0,0,SWP_NOSIZE);
         EnableWindow(hwnd,FALSE);
      } else
         ShowWindow(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),SW_HIDE);

      }
      return LRESULT(FALSE);

   case WM_COMMAND: {

      switch ( LOWORD(wParam) ) {

#include "dispositionSettingsSaveOptionsWMCommand.cpp"

#include "dispositionSettingsSaveMoreOptionWMCommand.cpp"

      case IDDI_SIZE_MAINTAIN_ASPECT_RATIO:
         pObject -> keepAspectRatio = (BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_SIZE_MAINTAIN_ASPECT_RATIO,BM_GETCHECK,0L,0L));
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_HEIGHT),! pObject -> keepAspectRatio && ! pObject -> fitToPage);
         break;
		 
      case IDDI_PAGE_PAGENO_OPT:
      case IDDI_PAGE_ONLAST:
      case IDDI_PAGE_NEWLAST:
         EnableWindow(GetDlgItem(hwnd,IDDI_PAGE_PAGENO),BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_PAGE_PAGENO_OPT,BM_GETCHECK,0L,0L) ? TRUE : FALSE);
         break;

      case IDDI_SIZE_FITTOPAGE:
         pObject -> fitToPage = (BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_SIZE_FITTOPAGE,BM_GETCHECK,0L,0L));
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_HEIGHT),! pObject -> fitToPage && ! pObject -> keepAspectRatio);
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_WIDTH),! pObject -> fitToPage);
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_MAINTAIN_ASPECT_RATIO),! pObject -> fitToPage);
         break;

      default:
         break;
      }

      }
      break;

   case WM_NOTIFY: {

      NMHDR *pNotifyHeader = (NMHDR *)lParam;

      switch ( pNotifyHeader -> code ) {

      case TCN_SELCHANGE: {
         switch ( SendDlgItemMessage(hwnd,IDDI_TABS,TCM_GETCURSEL,0L,0L) ) {
         case 0:
            EnumChildWindows(hwnd,page1,NULL);
            break;
         case 1:
            EnumChildWindows(hwnd,page2,NULL);
            break;
         case 2:
            EnumChildWindows(hwnd,page3,NULL);
            break;
         }
         }
         break;

      case PSN_KILLACTIVE: {
         UNLOAD_CONTROLS
         UNLOAD_ADDITIONAL
         SetWindowLongPtr(hwnd,DWLP_MSGRESULT,FALSE);
         }
         break;

      case PSN_APPLY: {

         PSHNOTIFY *pNotify = (PSHNOTIFY *)lParam;

         UNLOAD_CONTROLS

         UNLOAD_ADDITIONAL

         if ( pNotify -> lParam ) {
            IPrintingSupportProfile *px = NULL;
            pObject -> pICursiVisionServices -> get_PrintingSupportProfile(&px);
            if ( pObject -> pICursiVisionServices -> IsAdministrator() || ! px )
               pObject -> SaveProperties();
            pObject -> DiscardProperties();
            pObject -> DiscardProperties();
         } else {
            pObject -> DiscardProperties();
            pObject -> PushProperties();
         }

         SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_NOERROR);

         return (LRESULT)TRUE;
         }
         break;

      case PSN_RESET: {
         pObject -> doExecute = false;
         pObject -> PopProperties();
         pObject -> PopProperties();
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


   BOOL CALLBACK adjustTop(HWND hwndTest,LPARAM lParam) {
   long id = (long)GetWindowLongPtr(hwndTest,GWL_ID);
   if ( ( 1000 < id && id < 1100 ) || ( 2000 < id && id < 2100 ) || ( 3000 < id && id < 3100 ) ) {
      RECT rcNow,rcParent;
      GetWindowRect(GetParent(hwndTest),&rcParent);
      GetWindowRect(hwndTest,&rcNow);
      rcNow.top += (long)lParam;
      rcNow.bottom += (long)lParam;
      SetWindowPos(hwndTest,HWND_TOP,rcNow.left - rcParent.left,rcNow.top - rcParent.top,0,0,SWP_NOSIZE | SWP_NOZORDER);
   }
   return TRUE;
   }


   BOOL CALLBACK page1(HWND hwndTest,LPARAM lParam) {
   long id = (long)GetWindowLongPtr(hwndTest,GWL_ID);
   if ( 1000 < id && id < 1100 )
      ShowWindow(hwndTest,SW_SHOW);
   else
      if ( ! ( 4000 < id && id < 4100 ) )
         ShowWindow(hwndTest,SW_HIDE);
   return TRUE;
   }

   BOOL CALLBACK page2(HWND hwndTest,LPARAM lParam) {
   long id = (long)GetWindowLongPtr(hwndTest,GWL_ID);
   if ( 2000 < id && id < 2100 )
      ShowWindow(hwndTest,SW_SHOW);
   else 
      if ( ! ( 4000 < id && id < 4100 ) )
         ShowWindow(hwndTest,SW_HIDE);
   return TRUE;
   }

   BOOL CALLBACK page3(HWND hwndTest,LPARAM lParam) {
   long id = (long)GetWindowLongPtr(hwndTest,GWL_ID);
   if ( ( 1000 < id && id < 1100 ) || ( 2000 < id && id < 2100 ) )
      ShowWindow(hwndTest,SW_HIDE);
   else  
      ShowWindow(hwndTest,SW_SHOW);
   return TRUE;
   }

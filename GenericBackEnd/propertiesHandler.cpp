// Copyright 2017, 2018, 2019 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GenericBackEnd.h"

   LRESULT CALLBACK GenericBackEnd::propertiesHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   GenericBackEnd *p = (GenericBackEnd *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

   switch ( msg ) {

   case WM_INITDIALOG: {

      PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);

      p = (GenericBackEnd *)pPage -> lParam;

      SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)p);

      p -> PushProperties();
      p -> PushProperties();

      SetDlgItemText(hwnd,IDDI_BATCH_FILE,p -> szBatchFileName);
      SendMessage(GetDlgItem(hwnd,IDDI_WAIT_FOR_COMPLETION),BM_SETCHECK,p -> waitForCompletion ? BST_CHECKED : BST_UNCHECKED, 0L);

      IPrintingSupportProfile *px = NULL;

      p -> pICursiVisionServices -> get_PrintingSupportProfile(&px);
      if ( ! p -> pICursiVisionServices -> IsAdministrator() && px ) {
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

      case IDDI_BATCH_FILE_GET: {

         OPENFILENAME openFileName;
         char szFilter[MAX_PATH],szFile[MAX_PATH];

         memset(szFilter,0,sizeof(szFilter));
         memset(szFile,0,sizeof(szFile));
         memset(&openFileName,0,sizeof(OPENFILENAME));

         strcpy(szFile,p -> szBatchFileName);

         sprintf(szFilter,"Batch or script files");
         long k = (long)strlen(szFilter) + sprintf(szFilter + (long)strlen(szFilter) + 1,"*.cmd");
         k = k + sprintf(szFilter + k + 2,"All Files");
         sprintf(szFilter + k + 3,"*.*");

         openFileName.lStructSize = sizeof(OPENFILENAME);
         openFileName.hwndOwner = hwnd;
         openFileName.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
         openFileName.lpstrFilter = szFilter;
         openFileName.lpstrFile = szFile;
         openFileName.lpstrDefExt = "cmd";
         openFileName.nFilterIndex = 1;
         openFileName.nMaxFile = MAX_PATH;
         openFileName.lpstrTitle = "Select the existing windows batch or script file";
//         openFileName.lpstrInitialDir = szUserDirectory;

         if ( ! GetOpenFileName(&openFileName) ) 
            break;

         strcpy(p -> szBatchFileName,openFileName.lpstrFile);

         SetDlgItemText(hwnd,IDDI_BATCH_FILE,p -> szBatchFileName);

         }
         break;

      case IDDI_BATCH_FILE:
         break;

      default:
         break;
      }

      }
      break;

   case WM_NOTIFY: {

      NMHDR *pNotifyHeader = (NMHDR *)lParam;

      switch ( pNotifyHeader -> code ) {

      case PSN_SETACTIVE: {

         }
         break;

      case PSN_KILLACTIVE: {

         GetDlgItemText(hwnd,IDDI_BATCH_FILE,p -> szBatchFileName,MAX_PATH);

         SetWindowLongPtr(hwnd,DWLP_MSGRESULT,FALSE);
         }
         break;

      case PSN_APPLY: {

         PSHNOTIFY *pNotify = (PSHNOTIFY *)lParam;

         IPrintingSupportProfile *px = NULL;
         p -> pICursiVisionServices -> get_PrintingSupportProfile(&px);
         if ( ! p -> pICursiVisionServices -> IsAdministrator() && px )
            return (LRESULT)TRUE;

         GetDlgItemText(hwnd,IDDI_BATCH_FILE,p -> szBatchFileName,MAX_PATH);

         p -> waitForCompletion = BST_CHECKED == SendMessage(GetDlgItem(hwnd,IDDI_WAIT_FOR_COMPLETION),BM_GETCHECK,0L,0L);

         SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_NOERROR);

         return (LRESULT)TRUE;
         }
         break;

      case PSN_RESET: {
         p -> PopProperties();
         p -> PopProperties();
         p -> doExecute = false;
         }
         break;

      }

      }
      break;

   default:
      break;
   }


   return (LRESULT)FALSE;
   }
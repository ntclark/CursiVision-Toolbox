// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "emailBackEnd.h"

#include "ToolBoxResources.h"

#define PUT_BOOL(v,id)  SendDlgItemMessage(hwnd,id,BM_SETCHECK,v ? BST_CHECKED : BST_UNCHECKED,0L);
#define PUT_LONG(v,id)  { char szX[32]; sprintf(szX,"%ld",v); SetDlgItemText(hwnd,id,szX); }
#define PUT_STRING(v,id) SetDlgItemText(hwnd,id,v);

#define GET_BOOL(v,id)  v = (BST_CHECKED == (long)SendDlgItemMessage(hwnd,id,BM_GETCHECK,0L,0L));
#define GET_LONG(v,id) {char szX[32]; GetDlgItemText(hwnd,id,szX,32); v = atol(szX); }
#define GET_STRING(v,id) GetDlgItemText(hwnd,id,v,MAX_PATH);

#define LOAD_CONTROLS                                           \
{                                                               \
   PUT_STRING(p -> szEmailFrom,IDDI_BACKENDS_EMAIL_FROM)        \
   PUT_STRING(p -> szEmailServer,IDDI_BACKENDS_EMAIL_SERVER)    \
   PUT_STRING(p -> szEmailUserName,IDDI_BACKENDS_EMAIL_USERNAME)\
   PUT_STRING(p -> szEmailPassword,IDDI_BACKENDS_EMAIL_PASSWORD)\
   PUT_STRING(p -> szEmailTo,IDDI_BACKENDS_EMAIL_TO)            \
   PUT_STRING(p -> szEmailCC,IDDI_BACKENDS_EMAIL_CC)            \
   PUT_STRING(p -> szEmailBCC,IDDI_BACKENDS_EMAIL_BCC)          \
   PUT_STRING(p -> szEmailSubject,IDDI_BACKENDS_EMAIL_SUBJECT)  \
   PUT_STRING(p -> szEmailBody,IDDI_BACKENDS_EMAIL_BODY)        \
   PUT_LONG(p -> smtpPort,IDDI_BACKENDS_EMAIL_PORT)             \
   PUT_BOOL(p -> showProperties,IDDI_BACKENDS_EMAIL_SHOWDIALOG) \
}

#define UNLOAD_CONTROLS                                         \
{                                                               \
   GET_STRING(p -> szEmailFrom,IDDI_BACKENDS_EMAIL_FROM)        \
   GET_STRING(p -> szEmailServer,IDDI_BACKENDS_EMAIL_SERVER)    \
   GET_STRING(p -> szEmailUserName,IDDI_BACKENDS_EMAIL_USERNAME)\
   GET_STRING(p -> szEmailPassword,IDDI_BACKENDS_EMAIL_PASSWORD)\
   GET_STRING(p -> szEmailTo,IDDI_BACKENDS_EMAIL_TO)            \
   GET_STRING(p -> szEmailCC,IDDI_BACKENDS_EMAIL_CC)            \
   GET_STRING(p -> szEmailBCC,IDDI_BACKENDS_EMAIL_BCC)          \
   GET_STRING(p -> szEmailSubject,IDDI_BACKENDS_EMAIL_SUBJECT)  \
   GET_STRING(p -> szEmailBody,IDDI_BACKENDS_EMAIL_BODY)        \
   GET_LONG(p -> smtpPort,IDDI_BACKENDS_EMAIL_PORT)             \
   GET_BOOL(p -> showProperties,IDDI_BACKENDS_EMAIL_SHOWDIALOG) \
}

   LRESULT CALLBACK EmailBackEnd::propertiesHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   EmailBackEnd *p = (EmailBackEnd *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

   static long controlsLoaded = false;

   switch ( msg ) {

   case WM_INITDIALOG: {
      PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);
      p = (EmailBackEnd *)pPage -> lParam;
      SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)p);
      p -> pIGProperties -> Push();
      p -> pIGProperties -> Push();
      controlsLoaded = false;

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
   
      if ( controlsLoaded )
         SendMessage(GetParent(hwnd),PSM_CHANGED,(WPARAM)hwnd,0L);

      switch ( LOWORD(wParam) ) {

      case IDDI_BACKENDS_EMAIL_BODY_EDIT: {
         GetDlgItemText(hwnd,IDDI_BACKENDS_EMAIL_BODY,p -> szEmailBody,1024);
         DLGTEMPLATE *dt = (DLGTEMPLATE *)LoadResource(hModule,FindResource(hModule,MAKEINTRESOURCE(IDD_DISPOSITION_EMAIL_BODY),RT_DIALOG));
         if ( 1L == DialogBoxIndirectParam(hModule,dt,hwnd,(DLGPROC)EmailBackEnd::bodyHandler,(LPARAM)p) ) 
            SetWindowText(GetDlgItem(hwnd,IDDI_BACKENDS_EMAIL_BODY),p -> szEmailBody);
         
         }
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
         LOAD_CONTROLS
         }
         break;

      case PSN_KILLACTIVE:
         SetWindowLongPtr(pNotifyHeader -> hwndFrom,DWLP_MSGRESULT,FALSE);
         break;

      case PSN_APPLY: {

         PSHNOTIFY *pNotify = (PSHNOTIFY *)lParam;

         UNLOAD_CONTROLS;

         if ( pNotify -> lParam ) {
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
         p -> doExecute = false;
         }
         break;

      }

      }
      break;

   default:
      break;
   }

   return (LRESULT)0L;
   }

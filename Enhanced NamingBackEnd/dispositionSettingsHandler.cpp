// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EnhancedNamingBackEnd.h"

#include <shellapi.h>
#include <shTypes.h>
#include <shlobj.h>

#define OBJECT_WITH_PROPERTIES NamingBackEnd
#define CURSIVISION_SERVICES_INTERFACE pObject -> pICursiVisionServices

#include "dispositionSettingsDefines.h"

#define ROLLUP_ASSOCIATE_PROCESS

#define ADDITIONAL_INITIALIZATION \
   needsAdmin = false;                                                                      \
   IPrintingSupportProfile *px = NULL;                                                      \
   if ( CURSIVISION_SERVICES_INTERFACE )                                                    \
      CURSIVISION_SERVICES_INTERFACE -> get_PrintingSupportProfile(&px);                    \
   if ( px && ! px -> AllowPrintProfileChanges() && ! pObject -> editAllowed ) {            \
      SetDlgItemText(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES,"Changes are disabled because Admin privileges are required to change print profiles");        \
      needsAdmin = true;                                                                    \
   } else {                                                                                 \
      if ( ! CURSIVISION_SERVICES_INTERFACE -> AllowToolboxPropertyChanges() && ! pObject -> editAllowed ) { \
         SetDlgItemText(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES,"Changes are disabled because Admin privileges are required to change tool properties");    \
         needsAdmin = true;                                                                 \
      } else                                                                                \
          ShowWindow(GetDlgItem(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES),SW_HIDE);      \
   }                                                                                        \
   loadComboBox(hwnd,pObject -> szNamePrefix[0],0,pObject -> pICursiVisionServices);        \
   loadComboBox(hwnd,pObject -> szNamePrefix[1],1,pObject -> pICursiVisionServices);        \
   SendDlgItemMessage(hwnd,IDDI_NAME1_PREFIX + 1,CB_INSERTSTRING,(WPARAM)-1L,(LPARAM)"<none>"); \
   if ( needsAdmin ) {                                                                          \
        enableDisableSiblings(GetDlgItem(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES),FALSE);   \
        moveUpAllAmount(hwnd,-16,NULL);                                                         \
        SetWindowPos(GetDlgItem(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE | SWP_SHOWWINDOW);     \
        if ( NULL == defaultTextHandler )                                                                                       \
            defaultTextHandler = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES),GWLP_WNDPROC,(UINT_PTR)redTextHandler); \
        else                                                                                                                    \
            SetWindowLongPtr(GetDlgItem(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES),GWLP_WNDPROC,(UINT_PTR)redTextHandler);    \
    }


#define UNLOAD_ADDITIONAL \
   SendMessage(GetDlgItem(hwnd,IDDI_NAME1_PREFIX),CB_GETLBTEXT,SendMessage(GetDlgItem(hwnd,IDDI_NAME1_PREFIX),CB_GETCURSEL,0L,0L),(WPARAM)pObject -> szNamePrefix[0]); \
   SendMessage(GetDlgItem(hwnd,IDDI_NAME2_PREFIX),CB_GETLBTEXT,SendMessage(GetDlgItem(hwnd,IDDI_NAME2_PREFIX),CB_GETCURSEL,0L,0L),(WPARAM)pObject -> szNamePrefix[1]);

#define REGISTER_TOOLTIPS_ADDITIONAL(hInst) \
   REGISTER_TOOLTIP(hInst,IDDI_NAME1_PREFIX) \
   REGISTER_TOOLTIP(hInst,IDDI_NAME2_PREFIX)

    void loadComboBox(HWND hwnd,char *pszNamePrefix,long index,ICursiVisionServices *pServices);

    static boolean needsAdmin = false;
    LRESULT CALLBACK redTextHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
    WNDPROC defaultTextHandler{NULL};

   LRESULT CALLBACK NamingBackEnd::dispositionSettingsHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   resultDisposition *p = (resultDisposition *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

#include "dispositionSettingsBody.cpp"

   return (LRESULT)FALSE;
   }


   void loadComboBox(HWND hwnd,char *pszNamePrefix,long index,ICursiVisionServices *pServices) {

   char *pFieldNames = pServices -> FieldLabels();

   char *pStart = pFieldNames;

   long fieldCount = pServices -> FieldCount();

   SendDlgItemMessage(hwnd,IDDI_NAME1_PREFIX + index,CB_RESETCONTENT,(WPARAM)0L,(LPARAM)0L);

   for ( long j = 0; j < fieldCount; j++ ) {

      if ( pStart[0] ) {
         SendDlgItemMessage(hwnd,IDDI_NAME1_PREFIX + index,CB_INSERTSTRING,(WPARAM)-1L,(LPARAM)pStart);
         if ( 0 < strlen(pszNamePrefix) && 0 == strcmp(pszNamePrefix,pStart) )
            SendDlgItemMessage(hwnd,IDDI_NAME1_PREFIX + index,CB_SETCURSEL,(WPARAM)SendDlgItemMessage(hwnd,IDDI_NAME1_PREFIX + index,CB_GETCOUNT,0L,0L) - 1,0L);
      }

      pStart += 32;

   }

   return;
   }


    LRESULT CALLBACK redTextHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

    switch ( msg ) {

    case WM_PAINT: {
        PAINTSTRUCT ps = {0};
        BeginPaint(hwnd,&ps);
        char szText[1024];
        GetWindowText(hwnd,szText,1024);
        HFONT hGUIFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SelectObject(ps.hdc,hGUIFont);
        SetTextColor(ps.hdc,RGB(255,0,0));
        SetBkColor(ps.hdc,GetSysColor(COLOR_MENU));
        DrawText(ps.hdc,szText,(int)strlen(szText),&ps.rcPaint,DT_TOP);
        EndPaint(hwnd,&ps);
        }
        break;

    default:
        break;

    }

    return CallWindowProc(defaultTextHandler,hwnd,msg,wParam,lParam);
    }
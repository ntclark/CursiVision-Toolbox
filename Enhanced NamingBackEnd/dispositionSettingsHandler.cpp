// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EnhancedNamingBackEnd.h"

#include <shellapi.h>
#include <shTypes.h>
#include <shlobj.h>

#define OBJECT_WITH_PROPERTIES NamingBackEnd

#include "dispositionSettingsDefines.h"

#define ROLLUP_ASSOCIATE_PROCESS

#define ADDITIONAL_INITIALIZATION \
   IPrintingSupportProfile *px = NULL;                                                                               \
   if ( pObject -> pICursiVisionServices )                                                                           \
      pObject -> pICursiVisionServices -> get_PrintingSupportProfile(&px);                                           \
   if ( px && ! px -> AllowSaveProperties() ) {                                                                      \
      RECT rc = {0};                                                                                                 \
      GetClientRect(hwnd,&rc);                                                                                       \
      SetWindowPos(GetDlgItem(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,rc.bottom - 32,0,0,SWP_NOSIZE);\
      EnableWindow(hwnd,FALSE);                                                                                      \
   } else {                                                                                                          \
      ShowWindow(GetDlgItem(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES),SW_HIDE);                                   \
      loadComboBox(hwnd,pObject -> szNamePrefix[0],0,pObject -> pICursiVisionServices);                              \
      loadComboBox(hwnd,pObject -> szNamePrefix[1],1,pObject -> pICursiVisionServices);                              \
      SendDlgItemMessage(hwnd,IDDI_NAME1_PREFIX + 1,CB_INSERTSTRING,(WPARAM)-1L,(LPARAM)"<none>");                   \
   }


#define UNLOAD_ADDITIONAL \
   SendMessage(GetDlgItem(hwnd,IDDI_NAME1_PREFIX),CB_GETLBTEXT,SendMessage(GetDlgItem(hwnd,IDDI_NAME1_PREFIX),CB_GETCURSEL,0L,0L),(WPARAM)pObject -> szNamePrefix[0]); \
   SendMessage(GetDlgItem(hwnd,IDDI_NAME2_PREFIX),CB_GETLBTEXT,SendMessage(GetDlgItem(hwnd,IDDI_NAME2_PREFIX),CB_GETCURSEL,0L,0L),(WPARAM)pObject -> szNamePrefix[1]);

#define REGISTER_TOOLTIPS_ADDITIONAL(hInst) \
   REGISTER_TOOLTIP(hInst,IDDI_NAME1_PREFIX) \
   REGISTER_TOOLTIP(hInst,IDDI_NAME2_PREFIX)

   void loadComboBox(HWND hwnd,char *pszNamePrefix,long index,ICursiVisionServices *pServices);

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
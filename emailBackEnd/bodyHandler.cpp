// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "emailBackEnd.h"

   LRESULT CALLBACK EmailBackEnd::bodyHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   EmailBackEnd *p = (EmailBackEnd *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

   switch ( msg ) {

   case WM_INITDIALOG: {

      char szText[1024];

      p = (EmailBackEnd *)lParam;
      SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)p);

      ShowWindow(GetDlgItem(hwnd,IDDI_OK),SW_HIDE);
      ShowWindow(GetDlgItem(hwnd,IDDI_CANCEL),SW_HIDE);

      LoadString(hModule,IDS_EMAIL_BODY_INSTRUCTIONS,szText,1024);
      SetWindowText(GetDlgItem(hwnd,IDS_EMAIL_BODY_INSTRUCTIONS),szText);

      p -> hwndBody = CreateWindowEx(WS_EX_CLIENTEDGE,"RICHEDIT50W","",WS_CHILD | ES_MULTILINE | ES_WANTRETURN | WS_VISIBLE,0,0,0,0,hwnd,NULL,NULL,NULL);

      SendMessage(hwnd,WM_SIZE,0L,0L);

      SetWindowText(p -> hwndBody,p -> szEmailBody);

      }
      return LRESULT(FALSE);

   case WM_SIZE: {

      RECT rcInstructions;
      RECT rcFrame,rcClient,rcOk;

      GetWindowRect(hwnd,&rcFrame);
      GetWindowRect(GetDlgItem(hwnd,IDDI_OK),&rcOk);

      memcpy(&rcClient,&rcFrame,sizeof(RECT));

      AdjustWindowRect(&rcClient,(DWORD)GetWindowLongPtr(hwnd,GWL_STYLE),FALSE);

      long dx = (rcClient.right - rcClient.left) - (rcFrame.right - rcFrame.left);
      long dy = (rcClient.bottom - rcClient.top) - (rcFrame.bottom - rcFrame.top);
      long cxClient = (rcFrame.right - rcFrame.left) - dx;
      long cyClient = (rcFrame.bottom - rcFrame.top) - dy;

      SetWindowPos(GetDlgItem(hwnd,IDDI_OK),HWND_TOP,8,cyClient - (rcOk.bottom - rcOk.top) - 8,0,0,SWP_NOSIZE | SWP_SHOWWINDOW);

      SetWindowPos(GetDlgItem(hwnd,IDDI_CANCEL),HWND_TOP,8 + (rcOk.right - rcOk.left) + 8,cyClient - (rcOk.bottom - rcOk.top) - 8,0,0,SWP_NOSIZE | SWP_SHOWWINDOW);

      SetWindowPos(GetDlgItem(hwnd,IDS_EMAIL_BODY_INSTRUCTIONS),HWND_TOP,0,0,cxClient - 16,32,SWP_NOMOVE);

      GetClientRect(GetDlgItem(hwnd,IDS_EMAIL_BODY_INSTRUCTIONS),&rcInstructions);

      SetWindowPos(p -> hwndBody,HWND_TOP,16,rcInstructions.bottom + dy + 2,cxClient - 32,cyClient - (rcOk.bottom - rcOk.top) - 16 - (rcInstructions.bottom + dy + 2),SWP_SHOWWINDOW);

      HFONT hGUIFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

      SendMessage(p -> hwndBody,WM_SETFONT,(WPARAM)hGUIFont,(LPARAM)TRUE);
      }
      break;

   case WM_COMMAND: {

      switch ( LOWORD(wParam) ) {
      case IDDI_OK:
         GetWindowText(p -> hwndBody,p -> szEmailBody,1024);
         EndDialog(hwnd,1L);
         break;

      case IDDI_CANCEL:
         EndDialog(hwnd,0L);
         break;
      }

      }
      break;

   case WM_DESTROY:
      DestroyWindow(p -> hwndBody);
      p -> hwndBody = NULL;
      break;
      
   default:
      break;

   }

   return (LRESULT)0L;
   }
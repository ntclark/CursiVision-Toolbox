// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ftpBackEnd.h"
#include "utilities.h"

#define PUT_BOOL(v,id)  SendDlgItemMessage(hwnd,id,BM_SETCHECK,v ? BST_CHECKED : BST_UNCHECKED,0L);
#define PUT_LONG(v,id)  { char szX[32]; sprintf(szX,"%ld",v); SetDlgItemText(hwnd,id,szX); }
#define PUT_STRING(v,id) SetDlgItemText(hwnd,id,v);

#define GET_BOOL(v,id)  v = (BST_CHECKED == SendDlgItemMessage(hwnd,id,BM_GETCHECK,0L,0L));
#define GET_LONG(v,id) {char szX[32]; GetDlgItemText(hwnd,id,szX,32); v = atol(szX); }
#define GET_STRING(v,id) GetDlgItemText(hwnd,id,v,MAX_PATH);

#define LOAD_CONTROLS                               \
{                                                   \
   PUT_STRING(p -> szFTPServer,IDDI_FTP_SERVER)     \
   PUT_STRING(p -> szFTPUserName,IDDI_FTP_USERNAME) \
   PUT_STRING(p -> szFTPPassword,IDDI_FTP_PASSWORD) \
   PUT_LONG(p -> ftpPort,IDDI_FTP_PORT)             \
   PUT_BOOL(p -> showProperties,IDDI_FTP_SHOWDIALOG)\
}

#define UNLOAD_CONTROLS                             \
{                                                   \
   GET_STRING(p -> szFTPServer,IDDI_FTP_SERVER)     \
   GET_STRING(p -> szFTPUserName,IDDI_FTP_USERNAME) \
   GET_STRING(p -> szFTPPassword,IDDI_FTP_PASSWORD) \
   GET_LONG(p -> ftpPort,IDDI_FTP_PORT)             \
   GET_BOOL(p -> showProperties,IDDI_FTP_SHOWDIALOG)\
}

    static boolean needsAdmin = false;
    LRESULT CALLBACK redTextHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
    WNDPROC defaultTextHandler{NULL};

    LRESULT CALLBACK FTPBackEnd::propertiesHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

    FTPBackEnd *p = (FTPBackEnd *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

    switch ( msg ) {

    case WM_INITDIALOG: {

        PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);
        p = (FTPBackEnd *)pPage -> lParam;
        SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)p);

        if ( ! p -> isProcessing ) {
            ShowWindow(GetDlgItem(hwnd,IDDI_FTP_SEND),SW_HIDE);
        } else {
            RECT rcGroup,rcParent;
            GetWindowRect(GetDlgItem(hwnd,IDDI_FTP_GROUP),&rcGroup);
            GetWindowRect(hwnd,&rcParent);
            p -> hwndLog = CreateWindowEx(WS_EX_CLIENTEDGE,"RICHEDIT50W","",WS_CHILD | WS_VSCROLL | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL | WS_VISIBLE,
                        rcGroup.left - rcParent.left,rcGroup.bottom - rcParent.top + 8,rcGroup.right - rcGroup.left,(long)(1.65 * (double)(rcGroup.bottom - rcGroup.top)),hwnd,NULL,NULL,NULL);
            HFONT hGUIFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            SendMessage(p -> hwndLog,WM_SETFONT,(WPARAM)hGUIFont,(LPARAM)TRUE);
        }

        p -> pIGProperties -> Push();
        p -> pIGProperties -> Push();

        needsAdmin = false;

        IPrintingSupportProfile *px = NULL;
        p -> pICursiVisionServices -> get_PrintingSupportProfile(&px);

        if ( px && ! px -> AllowPrintProfileChanges() && ! p -> editAllowed ) {
            SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE);
            SetDlgItemText(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES,"Changes are disabled because Admin privileges are required to change print profiles");
            EnableWindow(hwnd,FALSE);
            needsAdmin = true;
        } else {
            if ( ! p -> pICursiVisionServices -> AllowToolboxPropertyChanges() && ! p -> editAllowed ) {
                SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE);
                SetDlgItemText(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES,"Changes are disabled because Admin privileges are required to change tool properties");
                EnableWindow(hwnd,FALSE);
                needsAdmin = true;
            } else
                ShowWindow(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),SW_HIDE);
        }

        if ( needsAdmin ) {
            moveUpAllAmount(hwnd,-16,NULL);
            enableDisableSiblings(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),FALSE);
            SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE);
            if ( NULL == defaultTextHandler )
                defaultTextHandler = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),GWLP_WNDPROC,(UINT_PTR)redTextHandler);
            else
                SetWindowLongPtr(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),GWLP_WNDPROC,(UINT_PTR)redTextHandler);
        }

        }
        return LRESULT(FALSE);

    case WM_COMMAND: {

        switch ( LOWORD(wParam) ) {
        case IDDI_FTP_SEND: {
            UNLOAD_CONTROLS
            sendDocument(p -> szFTPServer,p -> ftpPort,p -> szFTPUserName,p -> szFTPPassword,p -> szResultFile,p -> hwndLog);
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

            if ( pNotify -> lParam && ! needsAdmin ) {
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

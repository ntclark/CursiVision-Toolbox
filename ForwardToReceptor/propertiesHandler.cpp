// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "forwardToReceptor.h"

#include "utilities.h"
#include "resource.h"

#include "ToolBoxResources.h"

#define PUT_BOOL(v,id)  SendDlgItemMessage(hwnd,id,BM_SETCHECK,v ? BST_CHECKED : BST_UNCHECKED,0L);
#define PUT_LONG(v,id)  { char szX[32]; sprintf(szX,"%ld",v); SetDlgItemText(hwnd,id,szX); }
#define PUT_STRING(v,id) SetDlgItemText(hwnd,id,v);

#define GET_BOOL(v,id)  v = (BST_CHECKED == SendDlgItemMessage(hwnd,id,BM_GETCHECK,0L,0L));
#define GET_LONG(v,id) {char szX[32]; GetDlgItemText(hwnd,id,szX,32); v = atol(szX); }
#define GET_STRING(v,id) GetDlgItemText(hwnd,id,v,MAX_PATH);

#define LOAD_CONTROLS                                           \
{                                                               \
   PUT_BOOL(p -> showProperties,IDDI_RECEPTOR_SHOWDIALOG)       \
   PUT_BOOL(p -> saveOnly,IDDI_SAVE_ON_SERVER)                  \
   PUT_STRING(p -> szServerName,IDDI_BACKENDS_RECEPTOR_SERVER)  \
   PUT_STRING(p -> szNextServerName,IDDI_BACKENDS_RECEPTOR_NEXT_SERVER)  \
   PUT_STRING(p -> szServerStoreLocation,IDDI_SERVER_LOCATION)  \
}

#define UNLOAD_CONTROLS                                         \
{                                                               \
   GET_BOOL(p -> showProperties,IDDI_RECEPTOR_SHOWDIALOG)       \
   GET_BOOL(p -> saveOnly,IDDI_SAVE_ON_SERVER)                  \
   GET_STRING(p -> szServerName,IDDI_BACKENDS_RECEPTOR_SERVER)  \
   GET_STRING(p -> szNextServerName,IDDI_BACKENDS_RECEPTOR_NEXT_SERVER)  \
   GET_STRING(p -> szServerStoreLocation,IDDI_SERVER_LOCATION)  \
}

    static boolean needsAdmin = false;

    LRESULT CALLBACK forwardToReceptor::propertiesHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

    forwardToReceptor *p = (forwardToReceptor *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

    static long controlsLoaded = false;

    switch ( msg ) {

    case WM_INITDIALOG: {

        PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);
        p = (forwardToReceptor *)pPage -> lParam;
        SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)p);
        p -> pIGProperties -> Push();
        p -> pIGProperties -> Push();
        controlsLoaded = false;

        needsAdmin = false;

        IPrintingSupportProfile *px = NULL;
        p -> pICursiVisionServices -> get_PrintingSupportProfile(&px);

        if ( px && ! px -> AllowPrintProfileChanges() ) {
            SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE);
            SetDlgItemText(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES,"Changes are disabled because Admin privileges are required to change print profiles");
            EnableWindow(hwnd,FALSE);
            needsAdmin = true;
        } else {
            if ( ! p -> pICursiVisionServices -> AllowToolboxPropertyChanges() ) {
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
        }

        }
        return LRESULT(FALSE);

    case WM_COMMAND: {
   
        if ( controlsLoaded )
            SendMessage(GetParent(hwnd),PSM_CHANGED,(WPARAM)hwnd,0L);

        switch ( LOWORD(wParam) ) {

        default:
            break;

        }

        }
        break;   

    case WM_USER + 1: {
        controlsLoaded = 1L;
        }
        break;

    case WM_NOTIFY: {

        NMHDR *pNotifyHeader = (NMHDR *)lParam;

        switch ( pNotifyHeader -> code ) {

        case PSN_SETACTIVE: {
            LOAD_CONTROLS
            PostMessage(hwnd,WM_USER + 1,0L,0L);
            }
            break;

        case PSN_KILLACTIVE: {
            SetWindowLongPtr(pNotifyHeader -> hwndFrom,DWLP_MSGRESULT,FALSE);
            }
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
            p -> doExecute = false;
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
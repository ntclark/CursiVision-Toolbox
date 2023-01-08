// Copyright 2017, 2018, 2019 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PrintingBackEnd.h"
#include <commctrl.h>

extern "C" int GetDocumentsLocation(HWND hwnd,char *);

#define PUT_BOOL(v,id)  SendDlgItemMessage(hwnd,id,BM_SETCHECK,v ? BST_CHECKED : BST_UNCHECKED,0L);
#define PUT_LONG(v,id)  { char szX[32]; sprintf(szX,"%ld",v); SetDlgItemText(hwnd,id,szX); }
#define PUT_STRING(v,id) SetDlgItemText(hwnd,id,v);
#define DEFAULT_LONG(v,def) { if ( 0 == v ) v = def; };

#define GET_BOOL(v,id)  v = (BST_CHECKED == SendDlgItemMessage(hwnd,id,BM_GETCHECK,0L,0L));
#define GET_LONG(v,id) {char szX[32]; GetDlgItemText(hwnd,id,szX,32); v = atol(szX); }
#define GET_STRING(v,id) GetDlgItemText(hwnd,id,v,MAX_PATH);

#define LOAD_CONTROLS \
{                                                                                                                                \
   DWORD countBytes = 0L;                                                                                                        \
   DWORD countPrinters = 0L;                                                                                                     \
   pObject -> copies = max(pObject -> copies,1);                                                                                 \
   PUT_BOOL(pObject -> useDefaultPrinter,IDDI_USE_DEFAULT_PRINTER);                                                              \
   EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,NULL,4,NULL,0,&countBytes,&countPrinters);                         \
   PRINTER_INFO_4 *pPrinterInfo = (PRINTER_INFO_4 *)new BYTE[countBytes];                                                        \
   EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,NULL,4,(BYTE *)pPrinterInfo,countBytes,&countBytes,&countPrinters);\
   SendMessage(GetDlgItem(hwnd,IDDI_PRINTER),CB_RESETCONTENT,0L,0L);                                                             \
   for ( DWORD k = 0; k < countPrinters; k++ ) {                                                                                 \
      SendMessage(GetDlgItem(hwnd,IDDI_PRINTER),CB_ADDSTRING,0L,(LPARAM)pPrinterInfo[k].pPrinterName);                           \
      if ( 0 == strcmp(pPrinterInfo[k].pPrinterName,pObject -> szChosenPrinter) )                                                \
         SendMessage(GetDlgItem(hwnd,IDDI_PRINTER),CB_SETCURSEL,k,0L);                                                           \
   }                                                                                                                             \
   delete [] pPrinterInfo;                                                                                                       \
   EnableWindow(GetDlgItem(hwnd,IDDI_PRINTER),pObject -> useDefaultPrinter ? FALSE : TRUE );                                     \
   EnableWindow(GetDlgItem(hwnd,IDDI_CHOOSE_PRINTER_LABEL),pObject -> useDefaultPrinter ? FALSE : TRUE );                        \
   PUT_BOOL(p -> doProperties,IDDI_DISPOSITION_SHOW_PROPERTIES);                                                                 \
   PUT_BOOL(pObject -> skipPrinting,IDDI_SKIP);                                                                                  \
   PUT_LONG(pObject -> copies,IDDI_PRINT_COPIES);                                                                                \
}

#define UNLOAD_CONTROLS \
{  \
   SendMessage(GetDlgItem(hwnd,IDDI_PRINTER),CB_GETLBTEXT,SendMessage(GetDlgItem(hwnd,IDDI_PRINTER),CB_GETCURSEL,0L,0L),(LPARAM)pObject -> szChosenPrinter); \
   GET_BOOL(pObject -> useDefaultPrinter,IDDI_USE_DEFAULT_PRINTER); \
   if ( pObject -> useDefaultPrinter ) pObject -> szChosenPrinter[0] = '\0'; \
   GET_BOOL(p -> doProperties,IDDI_DISPOSITION_SHOW_PROPERTIES); \
   GET_BOOL(pObject -> skipPrinting,IDDI_SKIP);   \
   GET_LONG(pObject -> copies,IDDI_PRINT_COPIES); \
}

    static boolean needsAdmin = false;
    LRESULT CALLBACK redTextHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
    WNDPROC defaultTextHandler{NULL};

    LRESULT CALLBACK PrintingBackEnd::propertiesHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

    resultDisposition *p = (resultDisposition *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

    static long controlsLoaded = 0L;

    switch ( msg ) {

    case WM_INITDIALOG: {

        PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);
        p = (resultDisposition *)pPage -> lParam;
        SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)p);

        PrintingBackEnd *pObject = (PrintingBackEnd *)(p -> pParent);

        pObject -> PushProperties();
        pObject -> PushProperties();

        controlsLoaded = 0L;

        HWND hwndSpinner = CreateWindowEx(0L,UPDOWN_CLASS,"",WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT,0,0,20,24,hwnd,(HMENU)IDDI_PRINT_COPIES_SPINNER,NULL,NULL);

        RECT rcOwner,rcParent;
        GetWindowRect(hwnd,&rcParent);
        GetWindowRect(GetDlgItem(hwnd,IDDI_PRINT_COPIES),&rcOwner);
        rcOwner.left -= rcParent.left;
        rcOwner.right -= rcParent.left;
        rcOwner.top -= rcParent.top;  
        rcOwner.bottom -= rcParent.top;
        SetWindowPos(hwndSpinner,HWND_TOP,rcOwner.left + rcOwner.right - rcOwner.left,rcOwner.top - 2,0,0,SWP_NOSIZE);

        SendMessage(hwndSpinner,UDM_SETBUDDY,(WPARAM)GetDlgItem(hwnd,IDDI_PRINT_COPIES),0L);
        SendMessage(hwndSpinner,UDM_SETRANGE,0L,MAKELONG((short)UD_MAXVAL,(short)1));

        LOAD_CONTROLS

        needsAdmin = false;

        IPrintingSupportProfile *px = NULL;
        pObject -> pICursiVisionServices -> get_PrintingSupportProfile(&px);

        if ( px && ! px -> AllowPrintProfileChanges() && ! pObject -> editAllowed ) {
            SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE);
            SetDlgItemText(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES,"Changes are disabled because Admin privileges are required to change print profiles");
            needsAdmin = true;
        } else {
            if ( ! pObject -> pICursiVisionServices -> AllowToolboxPropertyChanges() && ! pObject -> editAllowed ) {
                SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE);
                SetDlgItemText(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES,"Changes are disabled because Admin privileges are required to change tool properties");
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

        PrintingBackEnd *pObject = (PrintingBackEnd *)(p -> pParent);

        switch ( LOWORD(wParam) ) {

        case IDDI_USE_DEFAULT_PRINTER:
            GET_BOOL(pObject -> useDefaultPrinter,IDDI_USE_DEFAULT_PRINTER)
            EnableWindow(GetDlgItem(hwnd,IDDI_PRINTER),pObject -> useDefaultPrinter ? FALSE : TRUE );
            EnableWindow(GetDlgItem(hwnd,IDDI_CHOOSE_PRINTER_LABEL),pObject -> useDefaultPrinter ? FALSE : TRUE );
            break;

        case IDDI_DISPOSITION_PRINT:
            p -> doPrint = BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_DISPOSITION_PRINT,BM_GETCHECK,0L,0L);
            EnableWindow(GetDlgItem(hwnd,IDDI_DISPOSITION_PRINTER),p -> doPrint ? TRUE : FALSE);
            break;

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

        PrintingBackEnd *pObject = (PrintingBackEnd *)(p -> pParent);

        NMHDR *pNotifyHeader = (NMHDR *)lParam;

        switch ( pNotifyHeader -> code ) {

        case PSN_KILLACTIVE: {

            UNLOAD_CONTROLS

            SetWindowLongPtr(hwnd,DWLP_MSGRESULT,FALSE);

            }
            break;

        case PSN_APPLY: {

            PSHNOTIFY *pNotify = (PSHNOTIFY *)lParam;

            UNLOAD_CONTROLS

            if ( pNotify -> lParam && ! needsAdmin ) {
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
            pObject -> PopProperties();
            pObject -> PopProperties();
            pObject -> doExecute = false;
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

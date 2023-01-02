// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "VideoBackEnd.h"
#include <commctrl.h>
#include <shTypes.h>
#include <shlobj.h>

#include "dispositionSettingsDefines.h"

#include "resource.h"

extern IVMRWindowlessControl9 *pIVMRWindowlessControl;

static bool waitingForImage = false;
static bool skipToDraw = false;
static HWND hwndCameras = NULL;

#define OBJECT_WITH_PROPERTIES VideoBackEnd

#define WM_TIMER_ALLOW_DRAW         1

#define LOAD_ADDITIONAL                                                                \
{                                                                                      \
   SendMessage(hwndCameras,CB_RESETCONTENT,0L,0L);                                     \
   for ( long k = 0; k < (long)pObject -> cameraCount; k++ ) {                         \
      SendMessageW(hwndCameras,CB_ADDSTRING,0L,(LPARAM)pObject -> pCameraNames[k]);    \
      if ( 0 == wcscmp(pObject -> pCameraNames[k],pObject -> szwChosenDevice) )        \
         SendMessage(hwndCameras,CB_SETCURSEL,k,0L);                                   \
   }                                                                                   \
   PUT_BOOL(p -> doProperties,IDDI_SHOW_PROPERTIES);        \
   PUT_BOOL(pObject -> skipImaging,IDDI_SKIP);              \
   PUT_BOOL(pObject -> autoSnap,IDDI_AUTO_SNAP);            \
   PUT_BOOL(pObject -> timeStamp,IDDI_TIME_STAMP);          \
   PUT_BOOL(pObject -> includeComputerName,IDDI_COMPUTER_NAME);   \
   PUT_BOOL(pObject -> useAnyCamera,IDDI_USE_ANY_CAMERA);         \
   PUT_BOOL(pObject -> ignoreNoCamera,IDDI_IGNORE_NO_CAMERA);     \
   PUT_BOOL(pObject -> saveDocumentAnyway,IDDI_SAVE_FILE_ANYWAY); \
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
   PUT_LONG(pObject -> autoFocusDelay,IDDI_FOCUS_TIME);     \
}

#define UNLOAD_ADDITIONAL \
{  \
   SendMessageW(hwndCameras,CB_GETLBTEXT,SendMessageW(hwndCameras,CB_GETCURSEL,0L,0L),(LPARAM)pObject -> szwChosenDevice); \
   GET_BOOL(p -> doProperties,IDDI_SHOW_PROPERTIES);        \
   GET_BOOL(pObject -> skipImaging,IDDI_SKIP);              \
   GET_BOOL(pObject -> autoSnap,IDDI_AUTO_SNAP);            \
   GET_BOOL(pObject -> useAnyCamera,IDDI_USE_ANY_CAMERA);   \
   GET_BOOL(pObject -> ignoreNoCamera,IDDI_IGNORE_NO_CAMERA);     \
   GET_BOOL(pObject -> saveDocumentAnyway,IDDI_SAVE_FILE_ANYWAY); \
   GET_BOOL(pObject -> timeStamp,IDDI_TIME_STAMP);                \
   GET_BOOL(pObject -> includeComputerName,IDDI_COMPUTER_NAME);   \
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
   GET_LONG(pObject -> autoFocusDelay,IDDI_FOCUS_TIME);     \
}

    BOOL CALLBACK adjustTop(HWND hwndTest,LPARAM lParam);
    BOOL CALLBACK page1(HWND hwndTest,LPARAM lParam);
    BOOL CALLBACK page2(HWND hwndTest,LPARAM lParam);
    BOOL CALLBACK page3(HWND hwndTest,LPARAM lParam);

    IVideoWindow *pIVideoWindow = NULL;

    static boolean needsAdmin = false;
    LRESULT CALLBACK redTextHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
    WNDPROC defaultTextHandler{NULL};

    LRESULT CALLBACK VideoBackEnd::propertiesHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

    resultDisposition *p = (resultDisposition *)GetWindowLongPtr(hwnd,GWLP_USERDATA);
    VideoBackEnd *pObject = NULL;
    if ( p )
        pObject = (VideoBackEnd *)(p -> pParent);

    switch ( msg ) {

    case WM_INITDIALOG: {

        PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);
        p = (resultDisposition *)pPage -> lParam;
        SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)p);

        pObject = (VideoBackEnd *)(p -> pParent);

        pObject -> PushProperties();
        pObject -> PushProperties();

        RECT rcCameras,rcParent;

        GetWindowRect(GetDlgItem(hwnd,IDDI_IMAGER),&rcCameras);
        GetWindowRect(hwnd,&rcParent);

        DestroyWindow(GetDlgItem(hwnd,IDDI_IMAGER));

        hwndCameras = CreateWindowExW(0L,WC_COMBOBOXW,L"",CBS_DROPDOWNLIST | WS_VSCROLL | WS_CHILD | WS_VISIBLE,rcCameras.left - rcParent.left,rcCameras.top - rcParent.top,
                            rcCameras.right - rcCameras.left,128 + rcCameras.bottom - rcCameras.top,hwnd,(HMENU)IDDI_IMAGER,hModule,NULL);

        SendMessage(hwndCameras,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),0L);

        LOAD_CONTROLS

        LOAD_ADDITIONAL

        EnableWindow(GetDlgItem(hwnd,IDDI_IMAGER),pObject -> useAnyCamera ? FALSE : TRUE);
        EnableWindow(GetDlgItem(hwnd,IDDI_SAVE_FILE_ANYWAY),pObject -> ignoreNoCamera ? TRUE : FALSE);

        SendDlgItemMessage(hwnd,IDDI_PAGE_PAGENO_OPT,BM_SETCHECK,0L,0L);
        SendDlgItemMessage(hwnd,IDDI_PAGE_ONLAST,BM_SETCHECK,0L,0L);
        SendDlgItemMessage(hwnd,IDDI_PAGE_NEWLAST,BM_SETCHECK,0L,0L);

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

        if ( pObject -> isProcessing || pObject -> szwChosenDevice[0] )
            PostMessage(hwnd,WM_START_VIDEO,0L,0L);

        RECT rcTabs,rcVideo;

        GetWindowRect(GetDlgItem(hwnd,IDDI_TABS),&rcTabs);
        GetWindowRect(GetDlgItem(hwnd,IDDI_VIDEO),&rcVideo);

        rcVideo.right += (rcVideo.right - rcVideo.left) % 8;
        rcVideo.bottom += (rcVideo.bottom - rcVideo.top) % 8;

        AdjustWindowRectEx(&rcVideo,(DWORD)GetWindowLongPtr(GetDlgItem(hwnd,IDDI_VIDEO),GWL_STYLE),FALSE,(DWORD)GetWindowLongPtr(GetDlgItem(hwnd,IDDI_VIDEO),GWL_EXSTYLE));

        SetWindowPos(GetDlgItem(hwnd,IDDI_VIDEO),HWND_TOP,0,0,rcVideo.right - rcVideo.left,rcVideo.bottom - rcVideo.top,SWP_NOMOVE);

        SetWindowPos(GetDlgItem(hwnd,IDDI_TABS),HWND_TOP,0,0,rcVideo.right - rcVideo.left + 64,rcVideo.bottom - rcTabs.top + 32,SWP_NOMOVE);

        TCITEM tcItem = {0};

        tcItem.pszText = "Image";
        tcItem.mask = TCIF_TEXT;

        SendDlgItemMessage(hwnd,IDDI_TABS,TCM_INSERTITEM,0,(LPARAM)&tcItem);

        tcItem.pszText = "Location";
        SendDlgItemMessage(hwnd,IDDI_TABS,TCM_INSERTITEM,1,(LPARAM)&tcItem);

        tcItem.pszText = DISPOSITION_TITLE;
        SendDlgItemMessage(hwnd,IDDI_TABS,TCM_INSERTITEM,2,(LPARAM)&tcItem);

        EnumChildWindows(hwnd,page1,NULL);

        if ( pObject -> isProcessing )
            EnableWindow(GetDlgItem(GetParent(hwnd),IDOK),FALSE);

        memset(&rcTabs,0,sizeof(RECT));

        SendDlgItemMessage(hwnd,IDDI_TABS,TCM_ADJUSTRECT,(WPARAM)TRUE,(LPARAM)&rcTabs);

        long deltaY = rcTabs.bottom - rcTabs.top;

        EnumChildWindows(hwnd,adjustTop,deltaY);

        GetWindowRect(GetDlgItem(hwnd,IDDI_TABS),&rcTabs);

        SetWindowPos(GetDlgItem(hwnd,IDDI_TABS),HWND_TOP,0,0,rcTabs.right - rcTabs.left,rcTabs.bottom - rcTabs.top + deltaY,SWP_NOMOVE | SWP_NOZORDER);

        SetWindowLongPtr(GetDlgItem(hwnd,IDDI_VIDEO),GWLP_USERDATA,(LONG_PTR)pObject);

        VideoBackEnd::defaultImageHandler = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwnd,IDDI_VIDEO),GWLP_WNDPROC,(LONG_PTR)VideoBackEnd::imageHandler);

        UDACCEL accelerators[1];
        accelerators[0].nSec = 0;
        accelerators[0].nInc = 100;

        SendDlgItemMessage(hwnd,IDDI_FOCUS_TIME_SPINNER,UDM_SETBUDDY,(WPARAM)GetDlgItem(hwnd,IDDI_FOCUS_TIME),0L);
        SendDlgItemMessage(hwnd,IDDI_FOCUS_TIME_SPINNER,UDM_SETRANGE,0L,MAKELPARAM(10000,0));
        SendDlgItemMessage(hwnd,IDDI_FOCUS_TIME_SPINNER,UDM_SETACCEL,1L,(LPARAM)accelerators);

        EnableWindow(GetDlgItem(hwnd,IDDI_FOCUS_TIME),pObject -> autoSnap);
        EnableWindow(GetDlgItem(hwnd,IDDI_FOCUS_TIME_SPINNER),pObject -> autoSnap);

        if ( pObject -> isProcessing && pObject -> autoSnap ) {
            waitingForImage = true;
            skipToDraw = true;
            PostMessage(hwnd,WM_SNAP_PHOTO,0L,0L);
        }

        needsAdmin = false;

        IPrintingSupportProfile *px = NULL;
        pObject -> pICursiVisionServices -> get_PrintingSupportProfile(&px);

        if ( px && ! px -> AllowPrintProfileChanges() && ! pObject -> editAllowed ) {
            SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE);
            SetDlgItemText(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES,"Changes are disabled because Admin privileges are required to change print profiles");
            EnableWindow(hwnd,FALSE);
            needsAdmin = true;
        } else {
            if ( ! pObject -> pICursiVisionServices -> AllowToolboxPropertyChanges() && ! pObject -> editAllowed ) {
                SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE);
                SetDlgItemText(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES,"Changes are disabled because Admin privileges are required to change tool properties");
                EnableWindow(hwnd,FALSE);
                needsAdmin = true;
            } else
                ShowWindow(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),SW_HIDE);
        }

        if ( needsAdmin ) {
            moveUpAllAmount(hwnd,-24,NULL);
            enableDisableSiblings(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),FALSE);
            SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE);
            if ( NULL == defaultTextHandler )
                defaultTextHandler = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),GWLP_WNDPROC,(UINT_PTR)redTextHandler);
            else
                SetWindowLongPtr(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),GWLP_WNDPROC,(UINT_PTR)redTextHandler);
        }

        }
        return LRESULT(FALSE);

    case WM_START_VIDEO: {

        SendMessage(hwnd,WM_STOP_VIDEO,0L,0L);

        if ( ! pObject -> szwChosenDevice[0] )
            break;

        if ( ! pObject -> hostVideo(GetDlgItem(hwnd,IDDI_VIDEO)) )
            break;

        waitingForImage = false;

        if ( pObject -> isProcessing )
            EnableWindow(GetDlgItem(GetParent(hwnd),IDOK),TRUE);

        }
        break;

    case WM_SNAP_PHOTO: {

        if ( waitingForImage ) {
            PostMessage(hwnd,msg,wParam,lParam);
            break;
        } else if ( skipToDraw ) {
            SetTimer(hwnd,WM_TIMER_ALLOW_DRAW,pObject -> autoFocusDelay,NULL);
            break;
        }

        BYTE *pImage = NULL;

        HRESULT hr = pIVMRWindowlessControl -> GetCurrentImage(&pImage);

        if ( ! pImage || ( S_OK != hr ) ) {
            SetTimer(hwnd,WM_TIMER_ALLOW_DRAW,pObject -> autoFocusDelay,NULL);
            break;
        }

        if ( pObject -> timeStamp || pObject -> includeComputerName ) 
            TimeStampBitmap(pImage,pObject -> szTargetFile,pObject -> timeStamp,pObject -> includeComputerName);
        else
            SaveJPEG(pImage,pObject -> szTargetFile);

        CoTaskMemFree(pImage);

        if ( pObject -> autoSnap )
            SendMessage(GetParent(hwnd),PSM_REMOVEPAGE,0L,SendMessage(GetParent(hwnd),PSM_INDEXTOPAGE,0L,0L));

        }
        break;

    case WM_DESTROY:
    case WM_STOP_VIDEO:
        pObject -> unHostVideo();
        break;

    case WM_TIMER: {
        if ( WM_TIMER_ALLOW_DRAW == wParam ) {
            skipToDraw = false;
            KillTimer(hwnd,WM_TIMER_ALLOW_DRAW);
            PostMessage(hwnd,WM_SNAP_PHOTO,0L,0L);
        }
        }
        break;

    case WM_COMMAND: {

        switch ( LOWORD(wParam) ) {

#include "dispositionSettingsSaveOptionsWMCommand.cpp"

#include "dispositionSettingsSaveMoreOptionWMCommand.cpp"

        case IDDI_SIZE_MAINTAIN_ASPECT_RATIO:
            pObject -> keepAspectRatio = (BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_SIZE_MAINTAIN_ASPECT_RATIO,BM_GETCHECK,0L,0L));
            EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_HEIGHT),! pObject -> keepAspectRatio && ! pObject -> fitToPage);
            break;

        case IDDI_USE_ANY_CAMERA:
            pObject -> useAnyCamera = (BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_USE_ANY_CAMERA,BM_GETCHECK,0L,0L));
            EnableWindow(GetDlgItem(hwnd,IDDI_IMAGER),pObject -> useAnyCamera ? FALSE : TRUE);
            break;

        case IDDI_IGNORE_NO_CAMERA:
            pObject -> ignoreNoCamera = (BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_IGNORE_NO_CAMERA,BM_GETCHECK,0L,0L));
            EnableWindow(GetDlgItem(hwnd,IDDI_SAVE_FILE_ANYWAY),pObject -> ignoreNoCamera ? TRUE : FALSE);
            break;

        case IDDI_IMAGER: {
            if ( ! (HIWORD(wParam) == CBN_SELCHANGE) )
            break;
            long index = (long)SendMessage(hwndCameras,CB_GETCURSEL,0L,0L);
            if ( CB_ERR == index )
            break;
            SendMessageW(hwndCameras,CB_GETLBTEXT,(WPARAM)index,(LPARAM)pObject -> szwChosenDevice);
            PostMessage(hwnd,WM_START_VIDEO,0L,0L);
            }
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

        case IDDI_AUTO_SNAP:
            pObject -> autoSnap = (BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_AUTO_SNAP,BM_GETCHECK,0L,0L));
            EnableWindow(GetDlgItem(hwnd,IDDI_FOCUS_TIME),pObject -> autoSnap);
            EnableWindow(GetDlgItem(hwnd,IDDI_FOCUS_TIME_SPINNER),pObject -> autoSnap);
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

            if ( pNotify -> lParam && ! needsAdmin ) {
                pObject -> SaveProperties();
                pObject -> DiscardProperties();
                pObject -> DiscardProperties();
            } else {
                pObject -> DiscardProperties();
                pObject -> PushProperties();
            }

            if ( pObject -> isProcessing ) 
            SendMessage(hwnd,WM_SNAP_PHOTO,0L,0L);

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


    LRESULT CALLBACK VideoBackEnd::imageHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

    VideoBackEnd *p = (VideoBackEnd *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

    switch ( msg ) {

    case WM_PAINT: {
        if ( ! pIVMRWindowlessControl )
            return CallWindowProc(VideoBackEnd::defaultImageHandler,hwnd,msg,wParam,lParam);
        PAINTSTRUCT ps = {0};
        BeginPaint(hwnd,&ps);
        pIVMRWindowlessControl -> RepaintVideo(hwnd,ps.hdc);  
        EndPaint(hwnd,&ps);
        }
        break;

    default:
        break;

    }

    return CallWindowProc(VideoBackEnd::defaultImageHandler,hwnd,msg,wParam,lParam);
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

// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EnhancedReplicator.h"
#include "drawBoxDefines.h"


    struct RECTD {
        double left;
        double top;
        double right;
        double bottom;
    };

    static RECTD dragRect;

    static POINTL lastMouse;
    static POINTL dragOrigin;
    static long oldActiveRegion = -1L;
    static long cornerGrabIndex = -1L;

    static boolean needsAdmin = false;
    LRESULT CALLBACK redTextHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
    WNDPROC defaultTextHandler{NULL};

    LRESULT CALLBACK theReplicator::propertiesHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

    theReplicator *p = (theReplicator *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

    switch ( msg ) {

    case WM_INITDIALOG: {

        PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);

        p = (theReplicator *)pPage -> lParam;

        SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)p);

        p -> hwndProperties = hwnd;

        p -> pIGProperties -> Push();
        p -> pIGProperties -> Push();

        char szInfo[1024];
        LoadString(hModule,IDS_EDIT_INSTRUCTIONS,szInfo,1024);
        SetDlgItemText(hwnd,IDDI_REPLICATOR_INSTRUCTIONS,szInfo);

        p -> load();

        needsAdmin = false;

        IPrintingSupportProfile *px = NULL;
        p -> pICursiVisionServices -> get_PrintingSupportProfile(&px);

        if ( px && ! px -> AllowPrintProfileChanges() && ! p -> editAllowed ) {
            SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE);
            SetDlgItemText(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES,"Changes are disabled because Admin privileges are required to change print profiles");
            needsAdmin = true;
        } else {
            if ( ! p -> pICursiVisionServices -> AllowToolboxPropertyChanges() && ! p -> editAllowed ) {
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

        p -> pTemplateDocumentUI = p -> pTemplateDocument -> createView(hwnd,8,64,false,theReplicator::drawSignatures);

        }
        return LRESULT(FALSE);

    case WM_LBUTTONDOWN: {

        if ( -1L == p -> activeIndex )
            break;

        POINTL ptlMouse = {LOWORD(lParam),HIWORD(lParam)};

        if ( ptlMouse.x < p -> pTemplateDocumentUI -> rcPageParentCoordinates.left || ptlMouse.x > p -> pTemplateDocumentUI -> rcPageParentCoordinates.right || 
                    ptlMouse.y < p -> pTemplateDocumentUI -> rcPageParentCoordinates.top || ptlMouse.y > p -> pTemplateDocumentUI -> rcPageParentCoordinates.bottom ) 
            break;

        lastMouse.x = ptlMouse.x - p -> pTemplateDocumentUI -> rcPageParentCoordinates.left;
        lastMouse.y = ptlMouse.y - p -> pTemplateDocumentUI -> rcPageParentCoordinates.top;

        RECT rc = p -> pWritingLocations[p -> activeIndex] -> documentRect;

        p -> pTemplateDocumentUI -> convertToPixels(p -> pWritingLocations[p -> activeIndex] -> pdfPageNumber,&rc);

        dragRect.left = (double)rc.left;
        dragRect.top = (double)rc.top;
        dragRect.right = (double)rc.right;
        dragRect.bottom = (double)rc.bottom;

        }
        break;

    case WM_MOUSEMOVE: {
      
        if ( ! p -> pTemplateDocumentUI )
            break;

        POINTL ptlMouse = {LOWORD(lParam),HIWORD(lParam)};

        if ( ptlMouse.x < p -> pTemplateDocumentUI -> rcPageParentCoordinates.left || ptlMouse.x > p -> pTemplateDocumentUI -> rcPageParentCoordinates.right || 
                    ptlMouse.y < p -> pTemplateDocumentUI -> rcPageParentCoordinates.top || ptlMouse.y > p -> pTemplateDocumentUI -> rcPageParentCoordinates.bottom ) 
            break;

        ptlMouse.x -= p -> pTemplateDocumentUI -> rcPageParentCoordinates.left;
        ptlMouse.y -= p -> pTemplateDocumentUI -> rcPageParentCoordinates.top;

        if ( 0 == (wParam & MK_LBUTTON) ) {

            long oldActiveIndex = p -> activeIndex;

            p -> activeIndex = -1L;

            if ( ptlMouse.x < p -> pTemplateDocumentUI -> rcPDFPagePixelsInView.left || ptlMouse.x > p -> pTemplateDocumentUI -> rcPDFPagePixelsInView.right || 
                        ptlMouse.y < p -> pTemplateDocumentUI -> rcPDFPagePixelsInView.top || ptlMouse.y > p -> pTemplateDocumentUI -> rcPDFPagePixelsInView.bottom ) 
                break;

            long pageNumber{-1};
            p -> pTemplateDocumentUI -> PDFiumControl() -> get_PDFPageUnderMouse(&pageNumber);

            RECT rcMousePoints{ptlMouse.x,ptlMouse.y,ptlMouse.x,ptlMouse.y};

            p -> pTemplateDocumentUI -> convertToPoints(pageNumber,&rcMousePoints);

            for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {

                if ( ! p -> pWritingLocations[k] )
                    break;

                writingLocation *pSG = p -> pWritingLocations[k];

                if ( ! ( pSG -> pdfPageNumber == p -> pTemplateDocumentUI -> currentPageNumber() ) )
                    continue;

                if ( rcMousePoints.left < pSG -> documentRect.left - CORNER_PROXIMITY || rcMousePoints.left > pSG -> documentRect.right + CORNER_PROXIMITY || 
                                 rcMousePoints.top < pSG -> documentRect.bottom - CORNER_PROXIMITY || rcMousePoints.top > pSG -> documentRect.top + CORNER_PROXIMITY )
                    continue;

                p -> activeIndex = k;

                if ( ! ( oldActiveIndex == p -> activeIndex ) ) {
                    if ( ! ( -1L == oldActiveIndex ) )
                        p -> drawSignature(NULL,oldActiveIndex,NULL,NULL);
                    oldActiveIndex = p -> activeIndex;
                    p -> drawSignature(NULL,p -> activeIndex,NULL,NULL);
                }

                if ( p -> isReplicant[k] ) {

                    if ( abs(rcMousePoints.left - pSG -> documentRect.left) < CORNER_PROXIMITY && abs(rcMousePoints.top - pSG -> documentRect.top) < CORNER_PROXIMITY ) {
                        SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
                        cornerGrabIndex = 0;
                    } else if ( abs(rcMousePoints.left - pSG -> documentRect.left) < CORNER_PROXIMITY && abs(rcMousePoints.top - pSG -> documentRect.bottom) < CORNER_PROXIMITY ) {
                        SetCursor(LoadCursor(NULL,IDC_SIZENESW));
                        cornerGrabIndex = 3;
                    } else if ( abs(rcMousePoints.left - pSG -> documentRect.right) < CORNER_PROXIMITY && abs(rcMousePoints.top - pSG -> documentRect.top) < CORNER_PROXIMITY ) {
                        SetCursor(LoadCursor(NULL,IDC_SIZENESW));
                        cornerGrabIndex = 1;
                    } else if ( abs(rcMousePoints.left - pSG -> documentRect.right) < CORNER_PROXIMITY && abs(rcMousePoints.top - pSG -> documentRect.bottom) < CORNER_PROXIMITY ) {
                        SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
                        cornerGrabIndex = 2;
                    } else {
                        SetCursor(LoadCursor(NULL,IDC_HAND));
                        cornerGrabIndex = -1L;
                    }

                    break;

                }

                SetCursor(LoadCursor(NULL,IDC_ARROW));
                cornerGrabIndex = -1L;

            }

            if ( ! ( oldActiveIndex == p -> activeIndex ) ) 
                if ( ! ( -1L == oldActiveIndex ) ) 
                    p -> drawSignature(NULL,oldActiveIndex,NULL,NULL);

            if ( -1L == p -> activeIndex ) 
                SetCursor(LoadCursor(NULL,IDC_ARROW));

            break;

        }

        if ( -1L == p -> activeIndex )
            break;

        if ( ! p -> isReplicant[p -> activeIndex] ) {
            MessageBeep(MB_ICONASTERISK);
            break;
        }

        writingLocation *pSG = p -> pWritingLocations[p -> activeIndex];

        double deltaX = (double)(ptlMouse.x - lastMouse.x);
        double deltaY = (double)(ptlMouse.y - lastMouse.y);

        lastMouse.x = ptlMouse.x;
        lastMouse.y = ptlMouse.y;

        if ( ! ( -1L == cornerGrabIndex ) ) {

            long nativeIndex = p -> replicantIndex[p -> activeIndex];

            double aspectRatio = (double)(p -> pWritingLocations[nativeIndex] -> documentRect.right - p -> pWritingLocations[nativeIndex] -> documentRect.left) / 
                                        (double)(p -> pWritingLocations[nativeIndex] -> documentRect.top - p -> pWritingLocations[nativeIndex] -> documentRect.bottom);

            deltaY = deltaX / aspectRatio;

            switch ( cornerGrabIndex ) {

            case 0:
                dragRect.left += deltaX;
                dragRect.top += deltaY;
                SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
                break;

            case 1:
                dragRect.right += deltaX;
                dragRect.top -= deltaY;
                SetCursor(LoadCursor(NULL,IDC_SIZENESW));
                break;

            case 2:
                dragRect.right += deltaX;
                dragRect.bottom += deltaY;
                SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
                break;

            case 3:
                dragRect.left += deltaX;
                dragRect.bottom -= deltaY;
                SetCursor(LoadCursor(NULL,IDC_SIZENESW));
                break;

            }

        } else {

            dragRect.left += deltaX;
            dragRect.right += deltaX;
            dragRect.top += deltaY;
            dragRect.bottom += deltaY;
            SetCursor(LoadCursor(NULL,IDC_HAND));

        }

        RECT rcPixels{(long)dragRect.left,(long)dragRect.top,(long)dragRect.right,(long)dragRect.bottom};

        p -> drawSignature(NULL,p -> activeIndex,&rcPixels,&pSG -> documentRect);

        }
        break;

    case WM_LBUTTONUP: {

        if ( -1L == p -> activeIndex )
            break;

        if ( ! p -> isReplicant[p -> activeIndex] )
            break;

        p -> drawSignature(NULL,p -> activeIndex,NULL,NULL);

        SetWindowText(GetDlgItem(hwnd,IDDI_REPLICATOR_ACTION_INSTRUCTIONS),"");

        }
        break;

    case WM_RBUTTONDOWN: {
        }
        break;

    case WM_RBUTTONUP: {

        if ( -1L == p -> activeIndex )
            break;

        long x = LOWORD(lParam) - p -> pTemplateDocumentUI -> rcPageParentCoordinates.left;
        long y = HIWORD(lParam) - p -> pTemplateDocumentUI -> rcPageParentCoordinates.top;

        if ( hActionMenu )
            DestroyMenu(hActionMenu);

        hActionMenu = CreatePopupMenu();

        MENUITEMINFO menuItem = {0};

        menuItem.cbSize = sizeof(MENUITEMINFO);
        menuItem.fMask = MIIM_ID | MIIM_TYPE;
        menuItem.fType = MFT_STRING;
        menuItem.fState = MFS_ENABLED;

        menuItem.wID = IDDI_SIGNATURE_REGION_EVERY_PAGE;
        menuItem.dwTypeData = "Every Page";
        InsertMenuItem(hActionMenu,0,MF_BYPOSITION,&menuItem);

        menuItem.wID = IDDI_SIGNATURE_REGION_EVERY_PAGE;
        menuItem.dwTypeData = "Every Subsequent Page";
        InsertMenuItem(hActionMenu,0,MF_BYPOSITION,&menuItem);

        if ( p -> isReplicant[p -> activeIndex] ) {
            menuItem.wID = IDDI_SIGNATURE_REGION_DELETE;
            menuItem.dwTypeData = "Delete";
        } else {
            menuItem.wID = IDDI_SIGNATURE_REGION_REPLICATE;
            menuItem.dwTypeData = "Replicate";
        }

        InsertMenuItem(hActionMenu,0,MF_BYPOSITION,&menuItem);

        RECT rcView;
        GetWindowRect(p -> pTemplateDocumentUI -> hwndPane,&rcView);

        TrackPopupMenu(hActionMenu,TPM_LEFTALIGN,rcView.left + x,rcView.top + y,0,hwnd,NULL);

        }
        break;

    case WM_COMMAND: {

        switch ( LOWORD(wParam) ) {

        case IDDI_REPLICATOR_RESET: {
            p -> reset();
            p -> clearPage();
            p -> drawSignatures(NULL,p -> pTemplateDocumentUI);
            }
            break;

        case IDDI_SIGNATURE_REGION_DELETE: {
            if ( -1L == p -> activeIndex )
                break;
            p -> deleteReplicant(p -> activeIndex);
            p -> activeIndex = -1L;
            p -> clearPage();
            p -> drawSignatures(NULL,p -> pTemplateDocumentUI);
            }
            break;

        case IDDI_SIGNATURE_REGION_REPLICATE: {
            if ( -1L == p -> activeIndex )
                break;
            RECT rcNew;
            long keepIndex = p -> activeIndex;
            p -> activeIndex = -1L;
            p -> drawSignature(NULL,keepIndex,NULL,NULL);
            p -> activeIndex = keepIndex;
            writingLocation *pSG = p -> pWritingLocations[p -> activeIndex];
            RECT rcPixels = pSG -> documentRect;
            p -> pTemplateDocumentUI -> convertToPixels(pSG -> pdfPageNumber,&rcPixels);
            rcPixels.left += 32;
            rcPixels.top += 32;
            rcPixels.right += 32;
            rcPixels.bottom += 32;
            p -> drawSignature(NULL,p -> activeIndex,&rcPixels,&rcNew);
            p -> addReplicant(p -> activeIndex,rcNew.left,rcNew.bottom);
            p -> activeIndex = -1L;
            SetWindowText(GetDlgItem(hwnd,IDDI_REPLICATOR_ACTION_INSTRUCTIONS),"");
            }
            break;

        case IDDI_SIGNATURE_REGION_EVERY_PAGE: {

            if ( -1L == p -> activeIndex )
                break;

            for ( long k = 1; k <= p -> pTemplateDocument -> PDFPageCount(); k++ ) {
                if ( k == p -> pTemplateDocumentUI -> currentPageNumber() )
                    continue;
                if ( ! p -> duplicateReplicant(p -> activeIndex,k) )
                    break;
            }

            }
            break;

        case IDDI_SIGNATURE_REGION_EVERY_SUBSEQUENT_PAGE: {

            if ( -1L == p -> activeIndex )
                break;

            for ( long k = p -> pTemplateDocumentUI -> currentPageNumber() + 1; k <= p -> pTemplateDocument -> PDFPageCount(); k++ ) {
                if ( ! p -> duplicateReplicant(p -> activeIndex,k) )
                    break;
            }

            }
            break;
        default:
            break;

        }

        }
        break;   

    case WM_SIZE: {

        if ( ! p -> pTemplateDocumentUI )
            break;

        p -> pTemplateDocumentUI -> size();

        RECT rcView,rcParent,rcReset;

        GetWindowRect(hwnd,&rcParent);
        GetWindowRect(p -> pTemplateDocumentUI -> hwndPane,&rcView);
        GetWindowRect(GetDlgItem(hwnd,IDDI_REPLICATOR_RESET),&rcReset);

        SetWindowPos(GetDlgItem(hwnd,IDDI_REPLICATOR_INSTRUCTIONS),HWND_TOP,0,0,
                        rcParent.right - rcParent.left - (rcReset.right - rcParent.left) - 2 * (rcView.left - rcParent.left),(rcView.top - rcParent.top) - 16,SWP_NOMOVE);

        }
        break;

    case WM_NOTIFY: {

        NMHDR *pNotifyHeader = (NMHDR *)lParam;

        switch ( pNotifyHeader -> code ) {

        case PSN_KILLACTIVE:
            SetWindowLongPtr(pNotifyHeader -> hwndFrom,DWLP_MSGRESULT,FALSE);
            break;

        case PSN_APPLY: {

            PSHNOTIFY *pNotify = (PSHNOTIFY *)lParam;

            if ( pNotify -> lParam && ! needsAdmin) {
                p -> unload();
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
            }
            break;

        }

        }
        break;

    case WM_DESTROY: {
        if ( p -> pTemplateDocumentUI )
            p -> pTemplateDocumentUI -> releaseView();
        p -> pTemplateDocumentUI = NULL;
        if ( hActionMenu )
            DestroyMenu(hActionMenu);
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


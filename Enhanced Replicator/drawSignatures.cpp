// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EnhancedReplicator.h"

#include "drawBoxDefines.h"

#ifdef BORDER_RADIUS
#undef BORDER_RADIUS
#endif

#define BORDER_RADIUS 20

    void theReplicator::drawSignatures(HDC hdc,templateDocument::tdUI *pDocument) {

    if ( ! pDocument -> isDocumentRendered() )
        return;

    for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {
        if ( ! pThis -> pWritingLocations[k] )
            break;
        if ( pThis -> pWritingLocations[k] -> pdfPageNumber != pDocument -> currentPageNumber() ) 
            continue;
        pThis -> drawSignature(NULL,k,NULL,NULL);
    }

    return;
    }


    void theReplicator::drawSignature(HDC hdc,long index,RECT *prcNewPixels,RECT *pNewLocation) {

    bool wasProvided = true;

    if ( ! hdc ) {
        hdc = GetDC(pTemplateDocumentUI -> hwndPane);
        wasProvided = false;
    }

    writingLocation *pSG = pWritingLocations[index];

    LOGFONT systemFont;
    HFONT hGUIFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    GetObject(hGUIFont,sizeof(LOGFONT),&systemFont);

    systemFont.lfHeight = (long)((double)systemFont.lfHeight * 1.25);
    systemFont.lfWeight *= 2;

    HFONT hfontMain = CreateFontIndirect(&systemFont);

    HGDIOBJ oldFont = SelectObject(hdc,hfontMain);

    HGDIOBJ oldPen;

    HPEN hPen = NULL;
    if ( index == activeIndex )
        hPen = CreatePen(PS_SOLID,BORDER_WEIGHT,DB_RED);
    else
        hPen = CreatePen(PS_SOLID,BORDER_WEIGHT,DB_GRAY);

    oldPen = SelectObject(hdc,hPen);

    HBRUSH hBrush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    HGDIOBJ oldBrush = SelectObject(hdc,hBrush);

    if ( ! ( NULL == hdcDrawRestore[index] ) ) {
        long scrollAmount = pageScrollTop[index] - pTemplateDocumentUI -> prcPDFSpecificPagePixels[pSG -> pdfPageNumber - 1].top;
        BitBlt(hdc,restoreRect[index].left,restoreRect[index].top - scrollAmount,
                        restoreRect[index].right - restoreRect[index].left,restoreRect[index].bottom - restoreRect[index].top,
                            hdcDrawRestore[index],0,0,SRCCOPY);
        pTemplateDocumentUI -> pdfDCRelease(hdcDrawRestore[index]);
        hdcDrawRestore[index] = NULL;
    }

    restoreRect[index] = pSG -> documentRect;
    pTemplateDocumentUI -> convertToPixels(pSG -> pdfPageNumber,&restoreRect[index]);

    restoreRect[index].left -= BORDER_WEIGHT;
    restoreRect[index].right += BORDER_WEIGHT;
    restoreRect[index].top -= BORDER_WEIGHT;
    restoreRect[index].bottom += BORDER_WEIGHT;
    hdcDrawRestore[index] = pTemplateDocumentUI -> pdfDCArea(pSG -> pdfPageNumber,&restoreRect[index]);
    pageScrollTop[index] = pTemplateDocumentUI -> prcPDFSpecificPagePixels[pSG -> pdfPageNumber - 1].top;

    RECT rcPixels{0,0,0,0};

    if ( ! ( NULL == prcNewPixels ) )
        rcPixels = *prcNewPixels;
    else {
        rcPixels = pSG -> documentRect;
        pTemplateDocumentUI -> convertToPixels(pSG -> pdfPageNumber,&rcPixels);
    }

    RoundRect(hdc,rcPixels.left,rcPixels.top,rcPixels.right,rcPixels.bottom,BORDER_WEIGHT,BORDER_RADIUS);

    RECT rcText{rcPixels.left + 8,rcPixels.top + 8,rcPixels.right - 16,rcPixels.bottom - 16};

    if ( isReplicant[index] )
        DrawText(hdc,"Replicant",-1,&rcText,DT_CENTER | DT_VCENTER);
    else  
        DrawText(hdc,"Native",-1,&rcText,DT_CENTER | DT_VCENTER);

    DeleteObject(SelectObject(hdc,oldPen));

    if ( pNewLocation )  {
        if ( ! ( NULL == prcNewPixels ) ) {
            pTemplateDocumentUI -> convertToPoints(pSG -> pdfPageNumber,&rcPixels);
            *pNewLocation = rcPixels;
        } else
            *pNewLocation = pSG -> documentRect;
    }

    DeleteObject(SelectObject(hdc,oldFont));
    SelectObject(hdc,oldBrush);

    if ( ! wasProvided )
        ReleaseDC(pTemplateDocumentUI -> hwndPane,hdc);

    return;
    }


    void theReplicator::clearPage() {
    HDC hdc = GetDC(pTemplateDocumentUI -> hwndPane);
    long cxHTML = pTemplateDocumentUI -> rcPDFPagePixels.right - pTemplateDocumentUI -> rcPDFPagePixels.left;
    long cyHTML = pTemplateDocumentUI -> rcPDFPagePixels.bottom - pTemplateDocumentUI -> rcPDFPagePixels.top;
    HDC hdcPage = pTemplateDocumentUI -> pdfDC();
    BitBlt(hdc,pTemplateDocumentUI -> rcPDFPagePixels.left,pTemplateDocumentUI -> rcPDFPagePixels.top,cxHTML,cyHTML,hdcPage,0,0,SRCCOPY);
    pTemplateDocumentUI -> pdfDCRelease(hdcPage);
    ReleaseDC(pTemplateDocumentUI -> hwndPane,hdc);
    return;
    }
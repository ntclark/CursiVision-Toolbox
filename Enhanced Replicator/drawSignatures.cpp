// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EnhancedReplicator.h"

   void theReplicator::drawSignatures(HDC hdc,templateDocument::tdUI *pDocument) {

   if ( ! pDocument -> isDocumentRendered() )
      return;

   for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {
      if ( ! pThis -> pWritingLocations[k] )
         break;
      if ( pThis -> pWritingLocations[k] -> zzpdfPageNumber != pDocument -> currentPageNumber ) 
         continue;
      pThis -> drawSignature(NULL,k,0L,0L,NULL,pDocument);
   }

   return;
   }


   void theReplicator::drawSignature(HDC hdc,long index,long shiftX,long shiftY,RECT *pNewLocation,templateDocument::tdUI *pDocument) {

   bool wasProvided = true;

   if ( ! hdc ) {
      hdc = GetDC(pDocument -> hwndPane);
      wasProvided = false;
   }

   writingLocation *pSG = pWritingLocations[index];

   HGDIOBJ oldFont = SelectObject(hdc,GetStockObject(DEFAULT_GUI_FONT));

   HGDIOBJ oldPen;

   HPEN hPen = NULL;
   if ( index == activeIndex )
      hPen = CreatePen(PS_SOLID,1,RGB(255,0,0));
   else
      hPen = CreatePen(PS_SOLID,1,RGB(128,128,128));

   oldPen = SelectObject(hdc,hPen);

   RECT r;

   memcpy(&r,&pSG -> documentRect,sizeof(RECT));

   r.left += shiftX;
   r.right += shiftX;
   r.top -= shiftY;
   r.bottom -= shiftY;

   pDocument -> convertToPanePixels(pSG -> zzpdfPageNumber,&r);

   if ( NULL == hbmDrawRestore[index] ) { // && isReplicant[index] ) {

      long cx = r.right - r.left;
      long cy = r.bottom - r.top;

      HDC hdcSave = CreateCompatibleDC(NULL);

      if ( hbmDrawRestore[index] )
         DeleteObject(hbmDrawRestore[index]);

      memcpy(&restoreRect[index],&r,sizeof(RECT));

      hbmDrawRestore[index] = CreateBitmap(cx + 4,cy + 4,1,GetDeviceCaps(hdcSave,BITSPIXEL),NULL);

      HGDIOBJ oldBitmap = SelectObject(hdcSave,hbmDrawRestore[index]);

      BitBlt(hdcSave,0,0,cx + 4,cy + 4,hdc,r.left - 2,r.top - 2,SRCCOPY);

      SelectObject(hdcSave,oldBitmap);

      DeleteDC(hdcSave);

   }

   MoveToEx(hdc,r.left,r.top,NULL);
   LineTo(hdc,r.right,r.top);
   LineTo(hdc,r.right,r.bottom);
   LineTo(hdc,r.left,r.bottom);
   LineTo(hdc,r.left,r.top);

   r.left += 8;
   r.top += 8;
   r.right -= 16;
   r.bottom -= 16;

   if ( isReplicant[index] || shiftX || shiftY )
      DrawText(hdc,"Replicant",9,(RECT *)&r,0L);
   else  
      DrawText(hdc,"Native",6,(RECT *)&r,0);

   DeleteObject(SelectObject(hdc,oldPen));         

   if ( pNewLocation ) {
      pNewLocation -> left = pSG -> documentRect.left + shiftX;
      pNewLocation -> right = pNewLocation -> left + pSG -> documentRect.right - pSG -> documentRect.left;
      pNewLocation -> bottom = pSG -> documentRect.bottom - shiftY;
      pNewLocation -> top = pNewLocation -> bottom + pSG -> documentRect.top - pSG -> documentRect.bottom;
   }

   SelectObject(hdc,oldFont);

   if ( ! wasProvided )
      ReleaseDC(pDocument -> hwndPane,hdc);

   return;
   }

   void theReplicator::reDrawSignature(HDC hdc,long index,long shiftX,long shiftY,RECT *pNewLocation,templateDocument::tdUI *pDocument) {

   clearSignature(pDocument,index);

   drawSignature(hdc,index,shiftX,shiftY,pNewLocation,pDocument);

   return;
   }

   void theReplicator::clearBitmapsAndDrawSignatures(HDC hdc,templateDocument::tdUI *pDocument) {

   for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {
      if ( ! pThis -> pWritingLocations[k] )
         break;
      if ( NULL == pThis -> hbmDrawRestore[k] )
         continue;
      DeleteObject(pThis -> hbmDrawRestore[k]);
      pThis -> hbmDrawRestore[k] = NULL;
   }

   drawSignatures(hdc,pDocument);

   return;
   }


   void theReplicator::clearSignature(templateDocument::tdUI *pDocument,long index) {

   if ( NULL == hbmDrawRestore[index] )
      return;

   writingLocation *pSG = pWritingLocations[index];

   RECT r;

   memcpy(&r,&restoreRect[index],sizeof(RECT));

   HDC hdc = GetDC(pDocument -> hwndPane);

   HDC hdcRestore = CreateCompatibleDC(NULL);

   HGDIOBJ oldBitmap = SelectObject(hdcRestore,hbmDrawRestore[index]);

   BOOL rc = BitBlt(hdc,r.left - 2,r.top - 2,r.right - r.left + 4,r.bottom - r.top + 4,hdcRestore,0,0,SRCCOPY);

   SelectObject(hdcRestore,oldBitmap);

   DeleteDC(hdcRestore);

   ReleaseDC(pDocument -> hwndPane,hdc);

   DeleteObject(hbmDrawRestore[index]);

   hbmDrawRestore[index] = NULL;

   return;
   }


   void theReplicator::clearPage(templateDocument::tdUI *pDocument) {

   HDC hdc = GetDC(pDocument -> hwndPane);

   long cxHTML = pDocument -> rcPageParentCoordinates.right - pDocument -> rcPageParentCoordinates.left;
   long cyHTML = pDocument -> rcPageParentCoordinates.bottom - pDocument -> rcPageParentCoordinates.top;

   BitBlt(hdc,0,0,cxHTML,cyHTML,pDocument -> pdfDC(),0,0,SRCCOPY);

   ReleaseDC(pDocument -> hwndPane,hdc);
   
   return;
   }
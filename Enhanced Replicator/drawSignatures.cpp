
#include "EnhancedReplicator.h"

   void theReplicator::drawSignatures(HDC hdc,templateDocument::tdUI *pDocument) {

   bool wasProvided = true;

   if ( ! hdc ) {
      hdc = GetDC(pDocument -> hwndVellum);
      wasProvided = false;
   }

   long cxHTML = pDocument -> rcPageParentCoordinates.right - pDocument -> rcPageParentCoordinates.left;
   long cyHTML = pDocument -> rcPageParentCoordinates.bottom - pDocument -> rcPageParentCoordinates.top;

   BitBlt(hdc,0,0,cxHTML,cyHTML,pDocument -> pdfDC(),0,0,SRCCOPY);

   for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {
      if ( ! pThis -> pWritingLocations[k] )
         break;
      if ( pThis -> pWritingLocations[k] -> zzpdfPageNumber != pDocument -> currentPageNumber ) 
         continue;
      pThis -> drawSignature(hdc,k,0L,0L,NULL,pDocument);
   }

   if ( ! wasProvided )
      ReleaseDC(pDocument -> hwndVellum,hdc);

   return;
   }


   void theReplicator::drawSignature(HDC hdc,long index,long shiftX,long shiftY,RECT *pNewLocation,templateDocument::tdUI *pDocument) {

   bool wasProvided = true;

   if ( ! hdc ) {
      hdc = GetDC(pDocument -> hwndVellum);
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

   pDocument -> convertToPixels(&r);

   r.left += shiftX;
   r.right += shiftX;
   r.top += shiftY;
   r.bottom += shiftY;

//   r.left = min(pDocument -> rcPageParentCoordinates.right - 2,max(pDocument -> rcPageParentCoordinates.left + 2,r.left));
//   r.right = min(pDocument -> rcPageParentCoordinates.right - 2,max(pDocument -> rcPageParentCoordinates.left + 2,r.right));
//   r.top = min(pDocument -> rcPageParentCoordinates.bottom - 2,max(pDocument -> rcPageParentCoordinates.top + 2,r.top));
//   r.bottom = min(pDocument -> rcPageParentCoordinates.bottom - 2,max(pDocument -> rcPageParentCoordinates.top + 2,r.bottom));

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
      pNewLocation -> left = pSG -> documentRect.left + (long)((double)shiftX / pDocument -> scaleToPixelsX);
      pNewLocation -> right = pNewLocation -> left + pSG -> documentRect.right - pSG -> documentRect.left;
      pNewLocation -> bottom = pSG -> documentRect.bottom - (long)((double)shiftY / pDocument -> scaleToPixelsY);
      pNewLocation -> top = pNewLocation -> bottom + pSG -> documentRect.top - pSG -> documentRect.bottom;
   }

   SelectObject(hdc,oldFont);

   if ( ! wasProvided )
      ReleaseDC(pDocument -> hwndVellum,hdc);

   return;
   }


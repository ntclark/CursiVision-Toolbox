
#include "EnhancedReplicator.h"

#include "writingLocation.h"

#include "pdfEnabler_i.c"

   theReplicator * theReplicator::pThis = NULL;

   theReplicator::theReplicator(IUnknown *pIUnknownOuter) :

   pIGProperties(NULL),
   pIGPropertiesClient(NULL),
   pIGPropertyPageClient(NULL),

   pIPrintingSupportProfile(NULL),
   pICursiVisionServices(NULL),

   hwndProperties(NULL),
   hwndParent(NULL),

   pTemplateDocument(NULL),

   activeIndex(-1L),
   replicationIndex(-1L),

   startParameters(0),
   endParameters(0),

   refCount(0)

   {

   memset(writingLocations,0,sizeof(writingLocations));
   memset(pWritingLocations,0,sizeof(pWritingLocations));

   memset(isReplicant,0,sizeof(isReplicant));
   memset(replicantIndex,0xFF,sizeof(replicantIndex));

   memset(&rightClickMousePoint,0,sizeof(POINTL));
   memset(&leftClickMousePoint,0,sizeof(POINTL));
   memset(&replicationOrigin,0,sizeof(POINTL));
   memset(&replicantDragOrigin,0,sizeof(POINTL));

   memset(replicantSignatureOrigins,0,sizeof(replicantSignatureOrigins));
   memset(nativeSignatureOrigins,0,sizeof(nativeSignatureOrigins));
   memset(replicantSignatureIndex,0,sizeof(replicantSignatureIndex));
   memset(replicantSignaturePage,0,sizeof(replicantSignaturePage));

   memset(hbmDrawRestore,0,sizeof(hbmDrawRestore));
   memset(restoreRect,0,sizeof(restoreRect));

   for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {
      internalScaleX[k] = 1.0;
      internalScaleY[k] = 1.0;
   }

   pIGPropertyPageClient = new _IGPropertyPageClient(this);

   long sizeParameters = offsetof(theReplicator,endParameters) - offsetof(theReplicator,startParameters);

   memset(&startParameters,0,sizeParameters);

   showProperties = false;

   HRESULT rc = CoCreateInstance(CLSID_InnoVisioNateProperties,NULL,CLSCTX_ALL,IID_IGProperties,reinterpret_cast<void **>(&pIGProperties));

   pIGPropertiesClient = new _IGPropertiesClient(this);

   pIGProperties -> Advise(static_cast<IGPropertiesClient *>(pIGPropertiesClient));

   refCount = 0L;

   pIGProperties -> Add(L"replicator parameters",NULL);
   pIGProperties -> DirectAccess(L"replicator parameters",TYPE_BINARY,&startParameters,sizeParameters);

   char szTemp[MAX_PATH];
   char szRootName[MAX_PATH];      

   strcpy(szRootName,szModuleName);

   char *p = strrchr(szModuleName,'\\');
   if ( ! p )
      p = strrchr(szModuleName,'/');
   if ( p ) {
      strcpy(szRootName,p + 1);
   }

   p = strrchr(szRootName,'.');
   if ( p )
      *p = '\0';

   sprintf(szTemp,"%s\\Settings\\%s.settings",szApplicationDataDirectory,szRootName);

   BSTR bstrFileName = SysAllocStringLen(NULL,MAX_PATH);
   MultiByteToWideChar(CP_ACP,0,szTemp,-1,bstrFileName,MAX_PATH);

   pIGProperties -> put_FileName(bstrFileName);

   SysFreeString(bstrFileName);

   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);

   if ( ! bSuccess )

      pIGPropertiesClient -> InitNew();

   else {

      for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {
         if ( 0.0 == internalScaleX[k] )
            internalScaleX[k] = 1.0;
         if ( 0.0 == internalScaleY[k] )
            internalScaleY[k] = 1.0;
      }

   }

   return;
   }


   theReplicator::~theReplicator() {
   IPrintingSupportProfile *px = NULL;
   pICursiVisionServices -> get_PrintingSupportProfile(&px);
   if ( pICursiVisionServices -> IsAdministrator() || ! px )
      pIGProperties -> Save();
   if ( pIPrintingSupportProfile )
      pIPrintingSupportProfile -> Release();
   if ( pTemplateDocument )
      delete pTemplateDocument;
   return;
   }

   
   void theReplicator::load() {

   if ( ! pICursiVisionServices )
      return;

   memset(pWritingLocations,0,sizeof(pWritingLocations));

   memset(isReplicant,0,sizeof(isReplicant));

   writingLocation *pLocation = (writingLocation *)pICursiVisionServices -> SignatureLocations();

   for ( long k = 0; k < pICursiVisionServices -> SignatureLocationCount(); k++, pLocation++ ) {
      memcpy(&writingLocations[k],pLocation,sizeof(writingLocation));
      pWritingLocations[k] = &writingLocations[k];
      isReplicant[k] = false;
      replicantIndex[k] = -1L;
   }

   long nativeSignatureCount = pICursiVisionServices -> SignatureLocationCount();
   long replicantStoredIndex = 0;

   for ( long k = nativeSignatureCount; k < WRITING_LOCATION_COUNT; k++, replicantStoredIndex++ ) {

      if ( 0 == replicantSignatureOrigins[replicantStoredIndex].x && 0 == replicantSignatureOrigins[replicantStoredIndex].y )
         break;

      pWritingLocations[k] = &writingLocations[k];

      replicantIndex[k] = replicantSignatureIndex[replicantStoredIndex];

      memcpy(pWritingLocations[k],&writingLocations[replicantIndex[k]],sizeof(writingLocation));

      isReplicant[k] = true;

      scaleReplicant(k,internalScaleX[replicantStoredIndex],internalScaleY[replicantStoredIndex]);

      moveReplicant(k,replicantSignatureOrigins[replicantStoredIndex].x,replicantSignatureOrigins[replicantStoredIndex].y,replicantSignaturePage[replicantStoredIndex]);
      
   }

   return;
   }


   void theReplicator::reset() {

   memset(replicantSignatureOrigins,0,sizeof(replicantSignatureOrigins));
   memset(nativeSignatureOrigins,0,sizeof(nativeSignatureOrigins));
   memset(replicantSignatureIndex,0,sizeof(replicantSignatureIndex));
   memset(replicantSignaturePage,0,sizeof(replicantSignaturePage));

   long k = 0;

   while ( k < WRITING_LOCATION_COUNT ) {

      if ( ! pWritingLocations[k] )
         break;

      if ( ! isReplicant[k] ) {
         k++;
         continue;
      }

      deleteReplicant(k);

   }

   return;
   }


   void theReplicator::unload() {

   memset(replicantSignatureOrigins,0,sizeof(replicantSignatureOrigins));
   memset(nativeSignatureOrigins,0,sizeof(nativeSignatureOrigins));
   memset(replicantSignatureIndex,0,sizeof(replicantSignatureIndex));
   memset(replicantSignaturePage,0,sizeof(replicantSignaturePage));
   memset(internalScaleX,0,sizeof(internalScaleX));
   memset(internalScaleY,0,sizeof(internalScaleY));

   long countReplicants = 0;

   for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {

      if ( ! pWritingLocations[k] )
         break;

      if ( ! isReplicant[k] )
         continue;

      replicantSignaturePage[countReplicants] = pWritingLocations[k] -> zzpdfPageNumber;
      replicantSignatureIndex[countReplicants] = replicantIndex[k];

      replicantSignatureOrigins[countReplicants].x = pWritingLocations[k] -> documentRect.left;
      replicantSignatureOrigins[countReplicants].y = pWritingLocations[k] -> documentRect.bottom;

      nativeSignatureOrigins[countReplicants].x = pWritingLocations[replicantIndex[k]] -> documentRect.left;
      nativeSignatureOrigins[countReplicants].y = pWritingLocations[replicantIndex[k]] -> documentRect.bottom;

      internalScaleX[countReplicants] = (double)(pWritingLocations[k] -> documentRect.right - pWritingLocations[k] -> documentRect.left) / 
                                                   (double)(pWritingLocations[replicantIndex[k]] -> documentRect.right - pWritingLocations[replicantIndex[k]] -> documentRect.left);

      internalScaleY[countReplicants] = (double)(pWritingLocations[k] -> documentRect.top - pWritingLocations[k] -> documentRect.bottom) / 
                                                   (double)(pWritingLocations[replicantIndex[k]] -> documentRect.top - pWritingLocations[replicantIndex[k]] -> documentRect.bottom);

      countReplicants++;

   }
   
   return;
   }


   void theReplicator::deleteReplicant(long theIndex) {

   long maxIndex = WRITING_LOCATION_COUNT - 1;

   for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {
      if ( ! pWritingLocations[k] ) {
         maxIndex = k;
         break;
      }
   }
   
//   if ( pICursiVisionServices )
//      pICursiVisionServices -> FreeSignatureGraphic(pSignatureGraphics[theIndex]);

   memcpy(&pWritingLocations[theIndex],&pWritingLocations[theIndex + 1],(maxIndex - theIndex - 1) * sizeof(writingLocation *));
   memcpy(&isReplicant[theIndex],&isReplicant[theIndex + 1],(maxIndex - theIndex - 1) * sizeof(bool));
   memcpy(&replicantIndex[theIndex],&replicantIndex[theIndex + 1],(maxIndex - theIndex - 1) * sizeof(long));
   memcpy(&internalScaleX[theIndex],&internalScaleX[theIndex + 1],(maxIndex - theIndex - 1) * sizeof(double));
   memcpy(&internalScaleY[theIndex],&internalScaleY[theIndex + 1],(maxIndex - theIndex - 1) * sizeof(double));

   pWritingLocations[maxIndex - 1] = NULL;
   isReplicant[maxIndex - 1] = false;

   activeIndex = -1L;
   replicationIndex = -1L;

   return;
   }


   void theReplicator::moveReplicant(long theIndex,long moveX,long moveY,long toPageNumber) {

   writingLocation *pSG = pWritingLocations[theIndex];

   pSG -> zzpdfPageNumber = toPageNumber;

   long cx = pSG -> documentRect.right - pSG -> documentRect.left;
   long cy = pSG -> documentRect.top - pSG -> documentRect.bottom;

   pSG -> documentRect.left = moveX;
   pSG -> documentRect.bottom = moveY;
   pSG -> documentRect.right = moveX + cx;
   pSG -> documentRect.top = moveY + cy;

   return;
   }


   void theReplicator::scaleReplicant(long theIndex,double scaleX,double scaleY) {

   if ( 1.0 == scaleX && 1.0 == scaleY )
      return;

   writingLocation *pSG = pWritingLocations[theIndex];

   pSG -> documentRect.right = pSG -> documentRect.left + (long)((double)(pSG -> documentRect.right - pSG -> documentRect.left) * scaleX);
   pSG -> documentRect.top = pSG -> documentRect.bottom + (long)((double)(pSG -> documentRect.top - pSG -> documentRect.bottom) * scaleY);

   return;
   }


   long theReplicator::addReplicant(templateDocument::tdUI *pDocument,long sourceIndex,long moveX,long moveY) {

   long targetIndex = -1L;

   for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {
      if ( ! pWritingLocations[k] ) {
         targetIndex = k;
         break;
      }
   }

   if ( -1L == targetIndex ) {
      MessageBox(NULL,"The maximum number of replicated signatures has been reached.\n\nWe apologize for any inconvenience","Note!",MB_ICONEXCLAMATION);
      return -1L;
   }

   writingLocation *pSG = &writingLocations[targetIndex];

   memcpy(pSG,&writingLocations[sourceIndex],sizeof(writingLocation));

   pWritingLocations[targetIndex] = pSG;

   replicantIndex[targetIndex] = sourceIndex;

   pSG -> zzpdfPageNumber = pDocument -> currentPageNumber;

   long cx = pSG -> documentRect.right - pSG -> documentRect.left;
   long cy = pSG -> documentRect.top - pSG -> documentRect.bottom;

   pSG -> documentRect.left = moveX;
   pSG -> documentRect.bottom = moveY;

   pSG -> documentRect.right = moveX + cx;
   pSG -> documentRect.top = moveY + cy;

   isReplicant[targetIndex] = true;

   return targetIndex;
   }

   bool theReplicator::duplicateReplicant(long sourceIndex,long pageNumber) {

   long targetIndex = -1L;

   for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {
      if ( ! pWritingLocations[k] ) {
         targetIndex = k;
         break;
      }
   }

   if ( -1L == targetIndex ) {
      MessageBox(NULL,"The maximum number of replicated signatures has been reached.\n\nWe apologize for any inconvenience","Note!",MB_ICONEXCLAMATION);
      return false;
   }

   long fromIndex = replicantIndex[sourceIndex];
   if ( ! isReplicant[sourceIndex] )
      fromIndex = sourceIndex;

//   if ( pICursiVisionServices )
//      pSignatureGraphics[targetIndex] = (signatureGraphic *)pICursiVisionServices -> GetSignatureGraphic(fromIndex);

   replicantIndex[targetIndex] = fromIndex;

   writingLocation *pSG = &writingLocations[targetIndex];

   pWritingLocations[targetIndex] = pSG;

   pSG -> zzpdfPageNumber = pageNumber;

   memcpy(&pSG -> documentRect,&pWritingLocations[fromIndex] -> documentRect,sizeof(RECT));

   isReplicant[targetIndex] = true;

   return true;
   }

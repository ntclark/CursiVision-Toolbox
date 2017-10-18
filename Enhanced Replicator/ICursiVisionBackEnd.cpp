// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EnhancedReplicator.h"

   HRESULT __stdcall theReplicator::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

   if ( showProperties ) {

      IUnknown *pIUnknown = NULL;
      QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
      pIGProperties -> ShowProperties(hwndParent,pIUnknown);
      pIUnknown -> Release();

   }

   IPdfEnabler *pIPdfEnabler = NULL;
   IPdfDocument *pIPdfDocument = NULL;

   long rc = CoCreateInstance(CLSID_PdfEnabler,NULL,CLSCTX_ALL,IID_IPdfEnabler,reinterpret_cast<void **>(&pIPdfEnabler));

   pIPdfEnabler -> Document(&pIPdfDocument);

   pIPdfDocument -> Open(bstrResultFileName,NULL,NULL);

   for ( long k = 0; k < WRITING_LOCATION_COUNT; k++ ) {

      if ( 0 == replicantSignatureOrigins[k].x && 0 == replicantSignatureOrigins[k].y )
         break;

//      POINT ptNew = {
//               (long)((double)(replicantSignatureOrigins[k].x - nativeSignatureOrigins[k].x) * internalScaleX[k]),
//               (long)((double)(replicantSignatureOrigins[k].y - nativeSignatureOrigins[k].y) * internalScaleY[k])};

      POINT ptOriginal = {
               (long)((double)(nativeSignatureOrigins[k].x)),
               (long)((double)(nativeSignatureOrigins[k].y))};

      POINT ptOffset = {
               (long)((double)(replicantSignatureOrigins[k].x - nativeSignatureOrigins[k].x)),
               (long)((double)(replicantSignatureOrigins[k].y - nativeSignatureOrigins[k].y))};

      pIPdfDocument -> ReplicateStream(replicantSignatureIndex[k] + 1,replicantSignaturePage[k],&ptOriginal,&ptOffset,internalScaleX[k],internalScaleY[k]);

   }

   pIPdfDocument -> Write(bstrResultFileName);
   
   pIPdfDocument -> Release();
   pIPdfEnabler -> Release();

   return S_OK;
   }


   HRESULT __stdcall theReplicator::put_PropertiesFileName(BSTR propertiesFileName) {
   pIGProperties -> put_FileName(propertiesFileName);
   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   //pIGProperties -> Save();
   return S_OK;
   }


   HRESULT __stdcall theReplicator::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }

   HRESULT __stdcall theReplicator::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvbeEnhRepl");
   return S_OK;
   }

   HRESULT __stdcall theReplicator::get_SavedDocumentName(BSTR *pDocumentName) {
   return E_NOTIMPL;
   }

   HRESULT __stdcall theReplicator::put_ParentWindow(HWND hp) {
   hwndParent = hp;
   return S_OK;
   }

   HRESULT __stdcall theReplicator::put_CommandLine(BSTR theCommandLine) {
   return E_NOTIMPL;
   }

   HRESULT __stdcall theReplicator::put_PrintingSupportProfile(IPrintingSupportProfile *pp) {
   if ( pIPrintingSupportProfile )
      pIPrintingSupportProfile -> Release();
   pIPrintingSupportProfile = pp;
   if ( pIPrintingSupportProfile )
      pIPrintingSupportProfile -> AddRef();
   return S_OK;
   }

   HRESULT __stdcall theReplicator::ServicesAdvise(ICursiVisionServices *pServices) {
   pICursiVisionServices = pServices;
   return S_OK; 
   }
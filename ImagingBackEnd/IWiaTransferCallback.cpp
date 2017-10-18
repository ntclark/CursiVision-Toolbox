// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ImagingBackEnd.h"
#include <shlwapi.h>

#if (_WIN32_WINNT >= 0x0600) // Windows Vista and later

   // IUnknown

   long __stdcall ImagingBackEnd::_IWiaTransferCallback::QueryInterface(REFIID riid,void **ppv) {
   *ppv = NULL;
   if ( riid == IID_IUnknown )
      *ppv = static_cast<IUnknown *>(this);
   else
   if ( riid == IID_IWiaTransferCallback )
      *ppv = static_cast<IWiaTransferCallback *>(this);
   else
      return pParent -> QueryInterface(riid,ppv);
   AddRef();
   return S_OK;
   }
   unsigned long __stdcall ImagingBackEnd::_IWiaTransferCallback::AddRef() {
   return pParent -> AddRef();
   }
   unsigned long __stdcall ImagingBackEnd::_IWiaTransferCallback::Release() { 
   return pParent -> Release();
   }


   HRESULT __stdcall ImagingBackEnd::_IWiaTransferCallback::TransferCallback(LONG flags,WiaTransferParams *pWiaTransferParams) {
   return S_OK;
   }


   HRESULT __stdcall ImagingBackEnd::_IWiaTransferCallback::GetNextStream(LONG lFlags,BSTR bstrItemName,BSTR bstrFullItemName,IStream **ppDestination) {
   DeleteFileW(destinationFile);
   return SHCreateStreamOnFileW(destinationFile,STGM_READWRITE | STGM_CREATE,ppDestination);
   }
#endif

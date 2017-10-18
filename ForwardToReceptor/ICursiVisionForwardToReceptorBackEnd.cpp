// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "forwardToReceptor.h"


   long __stdcall forwardToReceptor::_ICursiVisionForwardToReceptorBackEnd::QueryInterface(REFIID riid,void **ppv) {

   *ppv = NULL;

   if ( riid == IID_ICursiVisionForwardToReceptorBackEnd )
      *ppv = static_cast<ICursiVisionForwardToReceptorBackEnd *>(this);
   else
      return pParent -> QueryInterface(riid,ppv);

   AddRef();
   return S_OK;
   }

   unsigned long __stdcall forwardToReceptor::_ICursiVisionForwardToReceptorBackEnd::AddRef() {
   return 1;
   }

   unsigned long __stdcall forwardToReceptor::_ICursiVisionForwardToReceptorBackEnd::Release() { 
   return 1;
   }
   

   HRESULT __stdcall forwardToReceptor::_ICursiVisionForwardToReceptorBackEnd::SetServer(char *pszServerName) {
   strcpy(pParent -> szServerName,pszServerName);
   return S_OK;
   }

   HRESULT __stdcall forwardToReceptor::_ICursiVisionForwardToReceptorBackEnd::SetPort(long portNumber) {
   pParent -> portNumber = portNumber;
   return S_OK;
   }
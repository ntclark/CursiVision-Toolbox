// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EnhancedNamingBackEnd.h"


   long __stdcall NamingBackEnd::_IGPropertiesClient::QueryInterface(REFIID riid,void **ppv) {
   *ppv = NULL; 
 
   if ( riid == IID_IUnknown )
      *ppv = static_cast<IUnknown*>(this); 
   else

   if ( riid == IID_IDispatch )
      *ppv = this;
   else

      return pParent -> QueryInterface(riid,ppv);
 
   static_cast<IUnknown*>(*ppv) -> AddRef();
  
   return S_OK; 
   }
 
   unsigned long __stdcall NamingBackEnd::_IGPropertiesClient::AddRef() {
   return pParent -> AddRef();
   }
 
   unsigned long __stdcall NamingBackEnd::_IGPropertiesClient::Release() {
   return pParent -> Release();
   }


   HRESULT NamingBackEnd::_IGPropertiesClient::SavePrep() {
   return S_OK;
   }


   HRESULT NamingBackEnd::_IGPropertiesClient::InitNew() {
   return Loaded();
   }


   HRESULT NamingBackEnd::_IGPropertiesClient::Loaded() {
   return S_OK;
   }


   HRESULT NamingBackEnd::_IGPropertiesClient::Saved() {
   return S_OK;
   }


   HRESULT NamingBackEnd::_IGPropertiesClient::IsDirty() {
   return S_FALSE;
   }

   HRESULT NamingBackEnd::_IGPropertiesClient::GetClassID(BYTE *pCLSID) {
   return E_NOTIMPL;
   }

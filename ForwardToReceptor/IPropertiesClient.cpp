/*

                       Copyright (c) 2009 Nathan T. Clark

*/

#include "forwardToReceptor.h"


   long __stdcall forwardToReceptor::_IGPropertiesClient::QueryInterface(REFIID riid,void **ppv) {
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
 
   unsigned long __stdcall forwardToReceptor::_IGPropertiesClient::AddRef() {
   return pParent -> AddRef();
   }
 
   unsigned long __stdcall forwardToReceptor::_IGPropertiesClient::Release() {
   return pParent -> Release();
   }


   HRESULT forwardToReceptor::_IGPropertiesClient::SavePrep() {
   return S_OK;
   }


   HRESULT forwardToReceptor::_IGPropertiesClient::InitNew() {
   return Loaded();
   }


   HRESULT forwardToReceptor::_IGPropertiesClient::Loaded() {
   return S_OK;
   }


   HRESULT forwardToReceptor::_IGPropertiesClient::Saved() {
   return S_OK;
   }


   HRESULT forwardToReceptor::_IGPropertiesClient::IsDirty() {
   return S_FALSE;
   }

   HRESULT forwardToReceptor::_IGPropertiesClient::GetClassID(BYTE *pCLSID) {
   return E_NOTIMPL;
   }

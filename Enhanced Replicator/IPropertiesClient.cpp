/*

                       Copyright (c) 2009 Nathan T. Clark

*/

#include "EnhancedReplicator.h"


   long __stdcall theReplicator::_IGPropertiesClient::QueryInterface(REFIID riid,void **ppv) {
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
 
   unsigned long __stdcall theReplicator::_IGPropertiesClient::AddRef() {
   return pParent -> AddRef();
   }
 
   unsigned long __stdcall theReplicator::_IGPropertiesClient::Release() {
   return pParent -> Release();
   }


   HRESULT theReplicator::_IGPropertiesClient::SavePrep() {
   return S_OK;
   }


   HRESULT theReplicator::_IGPropertiesClient::InitNew() {
   return Loaded();
   }


   HRESULT theReplicator::_IGPropertiesClient::Loaded() {
   return S_OK;
   }


   HRESULT theReplicator::_IGPropertiesClient::Saved() {
   return S_OK;
   }


   HRESULT theReplicator::_IGPropertiesClient::IsDirty() {
   return S_FALSE;
   }

   HRESULT theReplicator::_IGPropertiesClient::GetClassID(BYTE *pCLSID) {
   return E_NOTIMPL;
   }

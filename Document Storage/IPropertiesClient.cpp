/*

                       Copyright (c) 2009,2010 Nathan T. Clark

*/

#include "DocumentStorage.h"


   long __stdcall DocumentStorage::_IGPropertiesClient::QueryInterface(REFIID riid,void **ppv) {
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
 
   unsigned long __stdcall DocumentStorage::_IGPropertiesClient::AddRef() {
   return pParent -> AddRef();
   }
 
   unsigned long __stdcall DocumentStorage::_IGPropertiesClient::Release() {
   return pParent -> Release();
   }


   HRESULT DocumentStorage::_IGPropertiesClient::SavePrep() {
   return S_OK;
   }


   HRESULT DocumentStorage::_IGPropertiesClient::InitNew() {
   return Loaded();
   }


   HRESULT DocumentStorage::_IGPropertiesClient::Loaded() {
   return S_OK;
   }


   HRESULT DocumentStorage::_IGPropertiesClient::Saved() {
   return S_OK;
   }


   HRESULT DocumentStorage::_IGPropertiesClient::IsDirty() {
   return S_FALSE;
   }

   HRESULT DocumentStorage::_IGPropertiesClient::GetClassID(BYTE *pCLSID) {
   return E_NOTIMPL;
   }

/*

                       Copyright (c) 2009,2010 Nathan T. Clark

*/

#include "GenericBackEnd.h"


   long __stdcall GenericBackEnd::_IGPropertiesClient::QueryInterface(REFIID riid,void **ppv) {
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
 
   unsigned long __stdcall GenericBackEnd::_IGPropertiesClient::AddRef() {
   return pParent -> AddRef();
   }
 
   unsigned long __stdcall GenericBackEnd::_IGPropertiesClient::Release() {
   return pParent -> Release();
   }


   HRESULT GenericBackEnd::_IGPropertiesClient::SavePrep() {
   return S_OK;
   }


   HRESULT GenericBackEnd::_IGPropertiesClient::InitNew() {
   return Loaded();
   }


   HRESULT GenericBackEnd::_IGPropertiesClient::Loaded() {
   return S_OK;
   }


   HRESULT GenericBackEnd::_IGPropertiesClient::Saved() {
   return S_OK;
   }


   HRESULT GenericBackEnd::_IGPropertiesClient::IsDirty() {
   return S_FALSE;
   }

   HRESULT GenericBackEnd::_IGPropertiesClient::GetClassID(BYTE *pCLSID) {
   return E_NOTIMPL;
   }

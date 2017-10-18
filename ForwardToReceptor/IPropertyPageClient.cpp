// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "forwardToReceptor.h"


   long __stdcall forwardToReceptor::_IGPropertyPageClient::QueryInterface(REFIID riid,void **ppv) {
   *ppv = NULL; 
 
   if ( riid == IID_IUnknown )
      *ppv = static_cast<IUnknown*>(this); 
   else

   if ( riid == IID_IDispatch )
      *ppv = this;
   else

   if ( riid == IID_IGPropertyPageClient )
      *ppv = static_cast<IGPropertyPageClient*>(this);
   else
 
      return pParent -> QueryInterface(riid,ppv);
 
   static_cast<IUnknown*>(*ppv) -> AddRef();
  
   return S_OK; 
   }
 
   unsigned long __stdcall forwardToReceptor::_IGPropertyPageClient::AddRef() {
   return pParent -> AddRef();
   }
 
   unsigned long __stdcall forwardToReceptor::_IGPropertyPageClient::Release() {
   return pParent -> Release();
   }


   HRESULT forwardToReceptor::_IGPropertyPageClient::BeforeAllPropertyPages() {
   return S_OK;
   }


   HRESULT forwardToReceptor::_IGPropertyPageClient::GetPropertyPagesInfo(long* pCntPages,SAFEARRAY** thePageNames,SAFEARRAY** theHelpDirs,SAFEARRAY** pSize) {
   return S_OK;
   }


   HRESULT forwardToReceptor::_IGPropertyPageClient::CreatePropertyPage(long pageNumber,HWND hwndParent,RECT* pRect,BOOL fModal,HWND *pHwnd) {
   return S_OK;
   }


   HRESULT forwardToReceptor::_IGPropertyPageClient::Apply() {
   return S_OK;
   }


   HRESULT forwardToReceptor::_IGPropertyPageClient::IsPageDirty(long pageNumber,BOOL* isDirty) {
   return S_OK;
   }


   HRESULT forwardToReceptor::_IGPropertyPageClient::Help(BSTR bstrHelpDir) {
   return  S_OK;
   }


   HRESULT forwardToReceptor::_IGPropertyPageClient::TranslateAccelerator(long,long* pResult) {
   *pResult = S_FALSE;
   return S_OK;
   }


   HRESULT forwardToReceptor::_IGPropertyPageClient::AfterAllPropertyPages(BOOL userCanceled) {
   return S_OK;
   }


   HRESULT forwardToReceptor::_IGPropertyPageClient::DestroyPropertyPage(long index) {
   return S_OK;
   }


   HRESULT forwardToReceptor::_IGPropertyPageClient::GetPropertySheetHeader(void *pv) {

   if ( ! pv )
      return E_POINTER;

   PROPSHEETHEADER *pHeader = reinterpret_cast<PROPSHEETHEADER *>(pv);

   pHeader -> dwFlags = PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP;
   if ( pParent -> isProcessing )
      pHeader -> dwFlags |= PSH_NOAPPLYNOW;

   pHeader -> hInstance = hModule;
   pHeader -> pszIcon = NULL;
   pHeader -> pszCaption = "Forward to Receptor Properties";
   pHeader -> pfnCallback = NULL;

   return S_OK;
   }


   HRESULT forwardToReceptor::_IGPropertyPageClient::get_PropertyPageCount(long *pCount) {
   if ( ! pCount )
      return E_POINTER;
   *pCount = 1;
   return S_OK;
   }


   HRESULT forwardToReceptor::_IGPropertyPageClient::GetPropertySheets(void *pPages) {

   PROPSHEETPAGE *pPropSheetPages = reinterpret_cast<PROPSHEETPAGE *>(pPages);

   pPropSheetPages[0].dwSize = sizeof(PROPSHEETPAGE);
   pPropSheetPages[0].dwFlags = PSP_USETITLE;
   pPropSheetPages[0].hInstance = hModule;
   pPropSheetPages[0].pszTemplate = MAKEINTRESOURCE(IDD_RECEPTOR);
   pPropSheetPages[0].pfnDlgProc = (DLGPROC)forwardToReceptor::propertiesHandler;
   pPropSheetPages[0].pszTitle = "Forward to Receptor";
   pPropSheetPages[0].lParam = (LONG_PTR)pParent;
   pPropSheetPages[0].pfnCallback = NULL;

   return S_OK;
   }

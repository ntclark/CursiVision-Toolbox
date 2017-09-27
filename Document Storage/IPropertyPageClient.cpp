/*

                       Copyright (c) 2009,2010 Nathan T. Clark

*/

#include "DocumentStorage.h"


   long __stdcall DocumentStorage::_IGPropertyPageClient::QueryInterface(REFIID riid,void **ppv) {
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
 
   unsigned long __stdcall DocumentStorage::_IGPropertyPageClient::AddRef() {
   return pParent -> AddRef();
   }
 
   unsigned long __stdcall DocumentStorage::_IGPropertyPageClient::Release() {
   return pParent -> Release();
   }


   HRESULT DocumentStorage::_IGPropertyPageClient::BeforeAllPropertyPages() {
   return S_OK;
   }


   HRESULT DocumentStorage::_IGPropertyPageClient::GetPropertyPagesInfo(long* pCntPages,SAFEARRAY** thePageNames,SAFEARRAY** theHelpDirs,SAFEARRAY** pSize) {
   return S_OK;
   }


   HRESULT DocumentStorage::_IGPropertyPageClient::CreatePropertyPage(long pageNumber,HWND hwndParent,RECT* pRect,BOOL fModal,HWND *pHwnd) {
   return S_OK;
   }


   HRESULT DocumentStorage::_IGPropertyPageClient::Apply() {
   return S_OK;
   }


   HRESULT DocumentStorage::_IGPropertyPageClient::IsPageDirty(long pageNumber,BOOL* isDirty) {
   return S_OK;
   }


   HRESULT DocumentStorage::_IGPropertyPageClient::Help(BSTR bstrHelpDir) {
   return  S_OK;
   }


   HRESULT DocumentStorage::_IGPropertyPageClient::TranslateAccelerator(long,long* pResult) {
   *pResult = S_FALSE;
   return S_OK;
   }


   HRESULT DocumentStorage::_IGPropertyPageClient::AfterAllPropertyPages(BOOL userCanceled) {
   return S_OK;
   }


   HRESULT DocumentStorage::_IGPropertyPageClient::DestroyPropertyPage(long index) {
   return S_OK;
   }


   HRESULT DocumentStorage::_IGPropertyPageClient::GetPropertySheetHeader(void *pv) {

   if ( ! pv )
      return E_POINTER;

   PROPSHEETHEADER *pHeader = reinterpret_cast<PROPSHEETHEADER *>(pv);

   pHeader -> dwFlags = PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP;
   pHeader -> hInstance = hModule;
   pHeader -> pszIcon = NULL;
   pHeader -> pszCaption = "CursiVision Document Storage Tool properties";
   pHeader -> pfnCallback = NULL;

   return S_OK;
   }


   HRESULT DocumentStorage::_IGPropertyPageClient::get_PropertyPageCount(long *pCount) {
   if ( ! pCount )
      return E_POINTER;
   *pCount = 1;
   return S_OK;
   }



   HRESULT DocumentStorage::_IGPropertyPageClient::GetPropertySheets(void *pPages) {
   PROPSHEETPAGE *pPropSheetPages = reinterpret_cast<PROPSHEETPAGE *>(pPages);

   pPropSheetPages[0].dwSize = sizeof(PROPSHEETPAGE);
   pPropSheetPages[0].dwFlags = PSP_USETITLE;
   pPropSheetPages[0].hInstance = hModule;
   pPropSheetPages[0].pszTemplate = MAKEINTRESOURCE(IDDIALOG_PROPERTIES);
   pPropSheetPages[0].pfnDlgProc = (DLGPROC)DocumentStorage::dispositionSettingsHandler;
   pPropSheetPages[0].pszTitle = "CursiVision Document Storage Tool properties";
   pPropSheetPages[0].lParam = (long)pParent;
   pPropSheetPages[0].pfnCallback = NULL;

   return S_OK;
   }

/*

                       Copyright (c) 1999,2000,2001,2002,2009 Nathan T. Clark

*/

#include "forwardToReceptor.h"
#include "..\resource.h"

   long __stdcall forwardToReceptor::_IPropertyPage::QueryInterface(REFIID riid,void **ppv) {

   *ppv = NULL;
  
   if ( riid == IID_IUnknown ) 
      *ppv = static_cast<void *>(this);
   else

      if ( riid == IID_IPropertyPage )
         *ppv = static_cast<IPropertyPage *>(this);
      else
 
         return pParent -> QueryInterface(riid,ppv);
 
   static_cast<IUnknown*>(*ppv) -> AddRef(); 
   return S_OK; 
   }
   unsigned long __stdcall forwardToReceptor::_IPropertyPage::AddRef() {
   return ++refCount;
   }
   unsigned long __stdcall forwardToReceptor::_IPropertyPage::Release() {
   if ( --refCount  ) return refCount;
   delete this;
   forwardToReceptor::pIPropertyPage = NULL;
   return 0;
   }
 
 
   HRESULT forwardToReceptor::_IPropertyPage::SetPageSite(IPropertyPageSite* pPageSite) {
   return S_OK;
   }

   
   HRESULT forwardToReceptor::_IPropertyPage::Activate(HWND hwndParent,const RECT* prc,BOOL fModal) {

   if ( ! pParent -> hwndProperties ) {
      DLGTEMPLATE *dt = (DLGTEMPLATE *)LoadResource(hModule,FindResource(hModule,MAKEINTRESOURCE(IDD_DISPOSITION_EMAIL),RT_DIALOG));
      pParent -> hwndProperties = CreateDialogIndirectParam(hModule,dt,(HWND)hwndParent,(DLGPROC)forwardToReceptor::propertiesHandler,(long)pParent);
   } else {
      SetParent(pParent -> hwndProperties,(HWND)hwndParent);
   }

   SetWindowPos(pParent -> hwndProperties,HWND_TOP,prc -> left,prc -> top,prc -> right - prc -> left,prc -> bottom - prc -> top,0L);

   pParent -> pIGProperties -> Push();
   pParent -> pIGProperties -> Push();

   return S_OK;
   }


   HRESULT forwardToReceptor::_IPropertyPage::Deactivate() {
   if ( pParent -> hwndProperties ) {
      DestroyWindow(pParent -> hwndProperties);
      pParent -> hwndProperties = NULL;
   }
   return S_OK;
   }



   HRESULT forwardToReceptor::_IPropertyPage::GetPageInfo(PROPPAGEINFO *pPageInfo) {

   memset(pPageInfo,0,sizeof(PROPPAGEINFO));

   pPageInfo -> cb = sizeof(PROPPAGEINFO);

   pPageInfo -> pszTitle = (BSTR)CoTaskMemAlloc(256);
   memset(pPageInfo -> pszTitle,0,256);

   BSTR pn = SysAllocString(L"E-mail settings");
   memcpy(pPageInfo -> pszTitle,pn,wcslen(pn) * sizeof(OLECHAR));

   pPageInfo -> size.cx = 350;
   pPageInfo -> size.cy = 370;

   SysFreeString(pn);

   return S_OK;
   }


   HRESULT forwardToReceptor::_IPropertyPage::SetObjects(ULONG cObjects,IUnknown** pUnk) {
   return S_OK;
   }


   HRESULT forwardToReceptor::_IPropertyPage::Show(UINT nCmdShow) {
   ShowWindow(pParent -> hwndProperties,nCmdShow);
   return S_OK;
   }


   HRESULT forwardToReceptor::_IPropertyPage::Move(const RECT* prc) {
   SetWindowPos(pParent -> hwndProperties,HWND_TOP,prc -> left,prc -> top,prc -> right - prc -> left,prc -> bottom - prc -> top,0L);
   return S_OK;
   }


   HRESULT forwardToReceptor::_IPropertyPage::IsPageDirty() {
   return S_OK;
   }


   HRESULT forwardToReceptor::_IPropertyPage::Apply() {
   SendMessage(pParent -> hwndProperties,WM_COMMAND,MAKELPARAM(IDDI_BACKENDS_EMAIL_OK,0),0L);
   pParent -> pIGProperties -> Discard();
   pParent -> pIGProperties -> Push();
   return S_OK;
   }


   HRESULT forwardToReceptor::_IPropertyPage::Help(LPCOLESTR pszHelpDir) {
   return E_FAIL;
   }


   HRESULT forwardToReceptor::_IPropertyPage::TranslateAccelerator(MSG* pMsg) {
   return S_FALSE;
   }

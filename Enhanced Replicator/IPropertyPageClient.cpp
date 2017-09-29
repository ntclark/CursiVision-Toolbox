
#include "EnhancedReplicator.h"


   long __stdcall theReplicator::_IGPropertyPageClient::QueryInterface(REFIID riid,void **ppv) {
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
 
   unsigned long __stdcall theReplicator::_IGPropertyPageClient::AddRef() {
   return pParent -> AddRef();
   }
 
   unsigned long __stdcall theReplicator::_IGPropertyPageClient::Release() {
   return pParent -> Release();
   }


   HRESULT theReplicator::_IGPropertyPageClient::BeforeAllPropertyPages() {
   return S_OK;
   }


   HRESULT theReplicator::_IGPropertyPageClient::GetPropertyPagesInfo(long* pCntPages,SAFEARRAY** thePageNames,SAFEARRAY** theHelpDirs,SAFEARRAY** pSize) {
   return S_OK;
   }


   HRESULT theReplicator::_IGPropertyPageClient::CreatePropertyPage(long pageNumber,HWND hwndParent,RECT* pRect,BOOL fModal,HWND *pHwnd) {
   return S_OK;
   }


   HRESULT theReplicator::_IGPropertyPageClient::Apply() {
   return S_OK;
   }


   HRESULT theReplicator::_IGPropertyPageClient::IsPageDirty(long pageNumber,BOOL* isDirty) {
   return S_OK;
   }


   HRESULT theReplicator::_IGPropertyPageClient::Help(BSTR bstrHelpDir) {
   return  S_OK;
   }


   HRESULT theReplicator::_IGPropertyPageClient::TranslateAccelerator(long,long* pResult) {
   *pResult = S_FALSE;
   return S_OK;
   }


   HRESULT theReplicator::_IGPropertyPageClient::AfterAllPropertyPages(BOOL userCanceled) {
   return S_OK;
   }


   HRESULT theReplicator::_IGPropertyPageClient::DestroyPropertyPage(long index) {
   return S_OK;
   }


   HRESULT theReplicator::_IGPropertyPageClient::GetPropertySheetHeader(void *pv) {

   if ( ! pv )
      return E_POINTER;

   PROPSHEETHEADER *pHeader = reinterpret_cast<PROPSHEETHEADER *>(pv);

   pHeader -> dwFlags = PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP;
   pHeader -> hInstance = hModule;
   pHeader -> pszIcon = NULL;
   pHeader -> pszCaption = "Replicator Properties";
   pHeader -> pfnCallback = NULL;

   return S_OK;
   }


   HRESULT theReplicator::_IGPropertyPageClient::get_PropertyPageCount(long *pCount) {
   if ( ! pCount )
      return E_POINTER;
   *pCount = 1;
   return S_OK;
   }


   HRESULT theReplicator::_IGPropertyPageClient::GetPropertySheets(void *pPages) {

   if ( pParent -> pTemplateDocument ) {
      delete pParent -> pTemplateDocument;
      pParent -> pTemplateDocument = NULL;
   }

   char *pDocument = NULL;

   if ( pParent -> pIPrintingSupportProfile )
      pDocument = pParent -> pIPrintingSupportProfile -> DocumentName();
   
   else if ( pParent -> pICursiVisionServices )
      pDocument = pParent -> pICursiVisionServices -> DocumentName();

   pParent -> pTemplateDocument = new templateDocument(pParent -> pICursiVisionServices,NULL,pDocument,NULL,NULL);

   pParent -> pTemplateDocument -> openDocument();

   SIZEL sizelDisplay{0,0};

   pParent -> pTemplateDocument -> GetSinglePagePDFDisplaySize(&sizelDisplay);

   PROPSHEETPAGE *pPropSheetPages = reinterpret_cast<PROPSHEETPAGE *>(pPages);

   theReplicator::pThis = pParent;

   pPropSheetPages[0].dwSize = sizeof(PROPSHEETPAGE);
   pPropSheetPages[0].dwFlags = PSP_USETITLE | PSP_DLGINDIRECT;
   pPropSheetPages[0].hInstance = hModule;
   pPropSheetPages[0].pfnDlgProc = (DLGPROC)theReplicator::propertiesHandler;
   pPropSheetPages[0].pszTitle = "Replicator Settings";
   pPropSheetPages[0].lParam = (LONG_PTR)pParent;
   pPropSheetPages[0].pfnCallback = NULL;

#if 0
   pPropSheetPages[0].pResource = (PROPSHEETPAGE_RESOURCE)LoadResource(hModule,FindResource(hModule,MAKEINTRESOURCE(IDD_REPLICATOR),RT_DIALOG));
#else
   //
   //NTC: 09-27-2017: I am aware that the copy of the dialog resource allocated here leaks.
   //
   // For some reason, perhaps by compiling with Visual Studio 2015 (compiler switches ?), changing memory loaded via
   // LoadResource throws an access violation, so the dialog template needs to be copied to allocated memory.
   //
   // However, I am not sure where or when the opportunity would come to delete the copy, perhaps somewhere
   // in the properties component's flow, however that would not know whether the caller just called LoadResource
   // which I don't think should be deleted.
   //
   HRSRC hDialog = FindResource(hModule,MAKEINTRESOURCE(IDD_REPLICATOR),RT_DIALOG);
   HGLOBAL hResource = LoadResource(hModule,hDialog);
   void *pResource = LockResource(hResource);
   BYTE *pDialog = new BYTE[SizeofResource(hModule,hDialog)];
   memcpy(pDialog,pResource,SizeofResource(hModule,hDialog));
   pPropSheetPages[0].pResource = (PROPSHEETPAGE_RESOURCE)pDialog;
#endif

  adjustPropertiesDialogSize(&sizelDisplay,(DLGTEMPLATEEX *)pPropSheetPages[0].pResource,128);

   return S_OK;
   }

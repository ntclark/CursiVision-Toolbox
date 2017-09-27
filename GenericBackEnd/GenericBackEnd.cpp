
#include "GenericBackEnd.h"

   GenericBackEnd::_IPropertyPage *GenericBackEnd::pIPropertyPage = NULL;

   GenericBackEnd::GenericBackEnd(IUnknown *pIUnknownOuter) :

   pIGProperties(NULL),
   pIGPropertiesClient(NULL),
   pIGPropertyPageClient(NULL),

   hwndProperties(NULL),
   hwndParent(NULL),

   doExecute(true),
   waitForCompletion(false),

   pICursiVisionServices(NULL),

   refCount(0)

   {

   memset(szBatchFileName,0,sizeof(szBatchFileName));

   pIGPropertyPageClient = new _IGPropertyPageClient(this);

   HRESULT rc = CoCreateInstance(CLSID_InnoVisioNateProperties,NULL,CLSCTX_ALL,IID_IGProperties,reinterpret_cast<void **>(&pIGProperties));

#ifdef DEBUG
   pIGProperties -> put_DebuggingEnabled(true);
#endif

   pIGPropertiesClient = new _IGPropertiesClient(this);

   pIGProperties -> Advise(static_cast<IGPropertiesClient *>(pIGPropertiesClient));

//
// 9-1-2011: IGProperties is adding a reference (as it should) which can be removed
// It may be better to not load properties in the constructor.
//
   refCount = 0L;

   pIGProperties -> Add(L"batch file",NULL);
   pIGProperties -> DirectAccess(L"batch file",TYPE_BINARY,szBatchFileName,sizeof(szBatchFileName));

   pIGProperties -> Add(L"wait for completion",NULL);
   pIGProperties -> DirectAccess(L"wait for completion",TYPE_BINARY,&waitForCompletion,sizeof(waitForCompletion));

   char szTemp[MAX_PATH];
   char szRootName[MAX_PATH];      

   strcpy(szRootName,szModuleName);

   char *p = strrchr(szModuleName,'\\');
   if ( ! p )
      p = strrchr(szModuleName,'/');
   if ( p ) {
      strcpy(szRootName,p + 1);
   }

   p = strrchr(szRootName,'.');
   if ( p )
      *p = '\0';

   sprintf(szTemp,"%s\\Settings\\%s.settings",szApplicationDataDirectory,szRootName);

   BSTR bstrFileName = SysAllocStringLen(NULL,MAX_PATH);
   MultiByteToWideChar(CP_ACP,0,szTemp,-1,bstrFileName,MAX_PATH);

   pIGProperties -> put_FileName(bstrFileName);

   SysFreeString(bstrFileName);

   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   if ( ! bSuccess )
      pIGPropertiesClient -> InitNew();

   return;
   }


   GenericBackEnd::~GenericBackEnd() {
   if ( pIPropertyPage )
      pIPropertyPage -> Release();
   pIPropertyPage = NULL;
   IPrintingSupportProfile *px = NULL;
   pICursiVisionServices -> get_PrintingSupportProfile(&px);
   if ( pICursiVisionServices -> IsAdministrator() || ! px )
      pIGProperties -> Save();
   return;
   }

   HRESULT GenericBackEnd::PushProperties() {
   return pIGProperties -> Push();
   }

   HRESULT GenericBackEnd::PopProperties() {
   return pIGProperties -> Pop();
   }

   HRESULT GenericBackEnd::DiscardProperties() {
   return pIGProperties -> Discard();
   }

   HRESULT GenericBackEnd::SaveProperties() {
   IPrintingSupportProfile *px = NULL;
   pICursiVisionServices -> get_PrintingSupportProfile(&px);
   if ( ! pICursiVisionServices -> IsAdministrator() && px )
      return S_OK;
   BSTR bstrFileName = NULL;
   pIGProperties -> get_FileName(&bstrFileName);
   if ( ! bstrFileName || 0 == bstrFileName[0] ) {
      return E_UNEXPECTED;
   }
   return pIGProperties -> Save();
   }

   
   

#include "NamingBackEnd.h"

   NamingBackEnd *NamingBackEnd::pCurrentEmailBackEnd = NULL;
   NamingBackEnd::_IPropertyPage *NamingBackEnd::pIPropertyPage = NULL;

   NamingBackEnd::NamingBackEnd(IUnknown *pIUnknownOuter) :

   pIGProperties(NULL),
   pIGPropertiesClient(NULL),
   pIGPropertyPageClient(NULL),

   hwndProperties(NULL),
   hwndParent(NULL),

   startParameters(0),
   endParameters(0),

   bstrResultsFile(NULL),

   pIPdfEnabler(NULL),
   pIPdfDocument(NULL),

   refCount(0)

   {

   memset(szNamePrefix,0,sizeof(szNamePrefix));

   pIGPropertyPageClient = new _IGPropertyPageClient(this);

   long sizeParameters = offsetof(NamingBackEnd,endParameters) - offsetof(NamingBackEnd,startParameters);

   memset(&startParameters,0,sizeParameters);

   memset(szNamePrefix,0,sizeof(szNamePrefix));

   HRESULT rc = CoCreateInstance(CLSID_GSystemProperties,NULL,CLSCTX_ALL,IID_IGProperties,reinterpret_cast<void **>(&pIGProperties));

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

   pIGProperties -> Add(L"naming parameters",NULL);
   pIGProperties -> DirectAccess(L"naming parameters",TYPE_BINARY,&startParameters,sizeParameters);

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

   pCurrentEmailBackEnd = this;

   return;
   }


   NamingBackEnd::~NamingBackEnd() {
   if ( pIPropertyPage )
      pIPropertyPage -> Release();
   pIPropertyPage = NULL;
   pIGProperties -> Save();
   pCurrentEmailBackEnd = NULL;
   if ( bstrResultsFile )
      SysFreeString(bstrResultsFile);
   return;
   }

   HRESULT NamingBackEnd::PushProperties() {
   return pIGProperties -> Push();
   }

   HRESULT NamingBackEnd::PopProperties() {
   return pIGProperties -> Pop();
   }

   HRESULT NamingBackEnd::DiscardProperties() {
   return pIGProperties -> Discard();
   }

   HRESULT NamingBackEnd::SaveProperties() {
   BSTR bstrFileName = NULL;
   pIGProperties -> get_FileName(&bstrFileName);
   if ( ! bstrFileName || 0 == bstrFileName[0] ) {
      return E_UNEXPECTED;
   }
   return pIGProperties -> Save();
   }

   
   
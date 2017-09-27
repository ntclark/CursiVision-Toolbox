
#include "DocumentStorage.h"

   DocumentStorage::_IPropertyPage *DocumentStorage::pIPropertyPage = NULL;

   DocumentStorage::DocumentStorage(IUnknown *pIUnknownOuter) :

   pIGProperties(NULL),
   pIGPropertiesClient(NULL),
   pIGPropertyPageClient(NULL),

   hwndProperties(NULL),
   hwndParent(NULL),

   pDatabase(NULL),

   refCount(0)

   {

   memset(szDatabaseDirectory,0,sizeof(szDatabaseDirectory));
   memset(szDatabaseName,0,sizeof(szDatabaseName));

   sprintf(szDatabaseDirectory,"%s\\DocumentStorage",szApplicationDataDirectory);

   CreateDirectory(szDatabaseDirectory,NULL);

   sprintf(szDatabaseName,"cvDocuments.accdb");

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

   pIGProperties -> Add(L"database location",NULL);
   pIGProperties -> DirectAccess(L"database location",TYPE_BINARY,szDatabaseDirectory,sizeof(szDatabaseDirectory));

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

   pDatabase = new database();

   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   if ( ! bSuccess )
      pIGPropertiesClient -> InitNew();


   return;
   }


   DocumentStorage::~DocumentStorage() {
   if ( pIPropertyPage )
      pIPropertyPage -> Release();
   pIPropertyPage = NULL;
   pIGProperties -> Save();
   return;
   }

   HRESULT DocumentStorage::PushProperties() {
   return pIGProperties -> Push();
   }

   HRESULT DocumentStorage::PopProperties() {
   return pIGProperties -> Pop();
   }

   HRESULT DocumentStorage::DiscardProperties() {
   return pIGProperties -> Discard();
   }

   HRESULT DocumentStorage::SaveProperties() {
   BSTR bstrFileName = NULL;
   pIGProperties -> get_FileName(&bstrFileName);
   if ( ! bstrFileName || 0 == bstrFileName[0] ) {
      return E_UNEXPECTED;
   }
   return pIGProperties -> Save();
   }

   
   
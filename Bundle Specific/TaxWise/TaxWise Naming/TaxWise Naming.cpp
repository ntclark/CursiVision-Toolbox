#include "TaxWise Naming.h"

   NamingBackEnd::NamingBackEnd(IUnknown *pIUnknownOuter) :

   pIGProperties(NULL),
   pIGPropertiesClient(NULL),
   pIGPropertyPageClient(NULL),

   pICursiVisionServices(NULL),

   hwndProperties(NULL),
   hwndParent(NULL),

   startParameters(0),
   endParameters(0),

   bstrResultsFile(NULL),

   refCount(0)

   {

   pIGPropertyPageClient = new _IGPropertyPageClient(this);

   long sizeParameters = offsetof(NamingBackEnd,endParameters) - offsetof(NamingBackEnd,startParameters);

   memset(&startParameters,0,sizeParameters);

   HRESULT rc = CoCreateInstance(CLSID_InnoVisioNateProperties,NULL,CLSCTX_ALL,IID_IGProperties,reinterpret_cast<void **>(&pIGProperties));

#ifdef DEBUG
   pIGProperties -> put_DebuggingEnabled(true);
#endif

   pIGPropertiesClient = new _IGPropertiesClient(this);

   pIGProperties -> Advise(static_cast<IGPropertiesClient *>(pIGPropertiesClient));

   refCount = 0L;

   pIGProperties -> Add(L"naming parameters",NULL);
   pIGProperties -> DirectAccess(L"naming parameters",TYPE_BINARY,&startParameters,sizeParameters);

   char szTemp[MAX_PATH];

   sprintf(szTemp,"%s\\TaxWiseNaming.settings",szTaxWiseBundleSettings);

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


   NamingBackEnd::~NamingBackEnd() {

   IPrintingSupportProfile *px = NULL;
   pICursiVisionServices -> get_PrintingSupportProfile(&px);
   if ( pICursiVisionServices -> IsAdministrator() || ! px )
      pIGProperties -> Save();

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

   
   
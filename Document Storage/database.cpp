
#include "DocumentStorage.h"


   database::database() :
   pConnection(NULL),
   pEngine(NULL),
   pDatabasePtr(NULL),
   bstrConnectionString(NULL)
/*   pConnectionEvents(NULL),
   pRecordsetEvents(NULL) */
   {
   }

   database::~database() {

   if ( pConnection )
      disconnect();

   if ( bstrConnectionString) 
      SysFreeString(bstrConnectionString);

   }


   long database::connect(char *pszDirectory,char *pszName) {

   HRESULT hr = CoCreateInstance(__uuidof(DAO::DBEngine),NULL,CLSCTX_ALL,IID_IDispatch,(LPVOID*)&pEngine);

   bstrConnectionString = SysAllocStringLen(NULL,1024);
   wchar_t bstrDirectory[MAX_PATH],bstrName[MAX_PATH];

   MultiByteToWideChar(CP_ACP,0,pszDirectory,-1,bstrDirectory,MAX_PATH);
   MultiByteToWideChar(CP_ACP,0,pszName,-1,bstrName,MAX_PATH);

   swprintf(bstrConnectionString,L"%s\\%s",bstrDirectory,bstrName);

   try {

   pDatabasePtr = pEngine -> OpenDatabase(bstrConnectionString);

   } catch ( _com_error e ) {

      DAO::ErrorsPtr pErrors = NULL;
      pErrors = pEngine -> GetErrors();
      for ( long k = 0; k < pErrors -> Count; k++ ) {
         MessageBoxW(NULL,pErrors -> GetItem(k) -> Description,L"",MB_OK);
      }

   }

   return 1L;
   }


   long database::disconnect() {
   if ( ! pConnection )
      return E_UNEXPECTED;
   pConnection -> Close();
   return S_OK;
   }


   long database::insert(BSTR pdfDocumentName,BSTR settingsFileName) {

   try {

   pDatabasePtr -> Execute(L"INSERT INTO Documents(Created) VALUES(time())", _variant_t(DAO::dbFailOnError));

   DAO::RecordsetPtr theNewRecord = pDatabasePtr -> OpenRecordset(L"SELECT TheDocument,TheSettings FROM Documents WHERE ID = (SELECT MAX(ID) FROM Documents)");

   theNewRecord -> MoveFirst();

   theNewRecord -> Edit();

   DAO::RecordsetPtr theInnerRecordset = theNewRecord -> GetFields() -> GetItem(0) -> Value;

   theInnerRecordset -> AddNew();

   DAO::Field2Ptr theInnerField = theInnerRecordset -> GetFields() -> Item["FileData"];

   theInnerField -> LoadFromFile(pdfDocumentName);

   theInnerRecordset -> Update(1,0);

   theInnerRecordset -> Close();

   theInnerRecordset = theNewRecord -> GetFields() -> GetItem(1) -> Value;

   theInnerRecordset -> AddNew();

   theInnerField = theInnerRecordset -> GetFields() -> Item["FileData"];

   theInnerField -> LoadFromFile(settingsFileName);

   theInnerRecordset -> Update(1,0);

   theInnerRecordset -> Close();

   theNewRecord -> Update(1,0);

   theNewRecord -> Close();

   } catch ( _com_error e ) {

      OLECHAR theMessage[1024];
      IErrorInfo *pIErrorInfo = e.ErrorInfo();

      if ( pIErrorInfo ) {
         BSTR errorString;
         pIErrorInfo -> GetDescription(&errorString);
         swprintf(theMessage,L"The system could not insert the document into the database:\n\n\t%s\n\n"
                             L"The database connection returned the error: #%s",bstrConnectionString,errorString);
         MessageBoxW(NULL,theMessage,L"Error!",MB_ICONEXCLAMATION);
         return E_FAIL;
      }

      swprintf(theMessage,L"The system could not insert the document into the database:\n\n\t%s\n\n"
                          L"This may mean the database is incorrect or needs compressed.\n\n"
                          L"The database connection returned the error code #%ld",bstrConnectionString,e.Error());

      MessageBoxW(NULL,theMessage,L"Error!",MB_ICONEXCLAMATION);

      return E_FAIL;
      
   }

   return S_OK;
   }
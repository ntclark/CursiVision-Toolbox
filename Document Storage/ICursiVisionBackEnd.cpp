
#include "DocumentStorage.h"

#include <process.h>
#include <time.h>

   extern "C" int GetDocumentsLocation(HWND hwnd,char *);

   HRESULT __stdcall DocumentStorage::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

   if ( ! szDatabaseDirectory[0] ) {

SetProperties:

      IUnknown *pIUnknown = NULL;
      QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
      pIGProperties -> ShowProperties(hwndParent,pIUnknown);
      pIUnknown -> Release();

   }

   if ( ! pDatabase -> connect(szDatabaseDirectory,szDatabaseName) ) {

      char szMessage[1024];
      char szRefinedMessage[1024];

      LoadString(hModule,IDS_DATABASE_CONNECT_ERROR,szMessage,1024);

      sprintf(szRefinedMessage,szMessage,szDatabaseDirectory,szDatabaseName);

      if ( IDYES == MessageBox(NULL,szRefinedMessage,"Note",MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON1) ) 
         goto SetProperties;

      return E_FAIL;

   }

   if ( S_OK != pDatabase -> insert(bstrResultFileName,dispositionSettingsFileName) ) {

      OLECHAR bstrMessage[1024];
      OLECHAR bstrRefinedMessage[1024];

      LoadStringW(hModule,IDS_DATABASE_INSERT_ERROR,bstrMessage,1024);

      swprintf(bstrRefinedMessage,bstrMessage,bstrResultFileName);

      if ( IDYES == MessageBoxW(NULL,bstrRefinedMessage,L"Note",MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON1) )
         goto SetProperties;

      return E_FAIL;

   }

   pDatabase -> disconnect();

   return S_OK;
   }


   HRESULT __stdcall DocumentStorage::put_PropertiesFileName(BSTR propertiesFileName) {
   pIGProperties -> put_FileName(propertiesFileName);
   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   pIGProperties -> Save();
   return S_OK;
   }


   HRESULT __stdcall DocumentStorage::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }

   HRESULT __stdcall DocumentStorage::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvbeDocuments");
   return S_OK;
   }

   HRESULT __stdcall DocumentStorage::get_SavedDocumentName(BSTR *pDocumentName) {
   return E_NOTIMPL;
   }

   HRESULT __stdcall DocumentStorage::put_ParentWindow(HWND hp) {
   hwndParent = hp;
   return S_OK;
   }

   HRESULT __stdcall DocumentStorage::put_CommandLine(BSTR theCommandLine) {
   return E_NOTIMPL;
   }
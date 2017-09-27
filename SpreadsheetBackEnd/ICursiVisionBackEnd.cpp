
#include "SpreadsheetBackEnd.h"

#include <process.h>
#include <time.h>

   extern "C" int GetDocumentsLocation(HWND hwnd,char *);

   HRESULT __stdcall SpreadsheetBackEnd::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

   if ( processingDisposition.doProperties ) {

SetProperties:

      IUnknown *pIUnknown = NULL;
      QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
      pIGProperties -> ShowProperties((long)hwndParent,pIUnknown);
      pIUnknown -> Release();

   }

   resultDisposition *pDisposition = &processingDisposition;

   WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szSignedDocument,MAX_PATH,0,0);

   BSTR bstrLocalResultFileName = SysAllocString(bstrResultFileName);

   for ( long k = 0; k < FIELD_COUNT; k++ ) {

      if ( ! szNamePrefix[k][0] )
         continue;

      if ( ! parseName(szSignedDocument,k,szFieldValue[k]) ) {
         char szMessage[512];
         sprintf(szMessage,"The system did not find the name field in the document using the prefix string \"%s\""
                           "\n\nRemember that capitalization and the number of spaces between words is important."
                           "\n\nSelect Retry to respecify the properties, or Cancel to continue",szNamePrefix[k]);
         if ( IDRETRY == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL) )
            goto SetProperties;
      }

   }

   loadExcelSpreadsheet();

   return S_OK;
   }

   HRESULT __stdcall SpreadsheetBackEnd::put_PropertiesFileName(BSTR propertiesFileName) {
   pIGProperties -> put_FileName(propertiesFileName);
   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   pIGProperties -> Save();
   return S_OK;
   }


   HRESULT __stdcall SpreadsheetBackEnd::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }

   HRESULT __stdcall SpreadsheetBackEnd::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvbeSpreadsheet");
   return S_OK;
   }

   HRESULT __stdcall SpreadsheetBackEnd::get_SavedDocumentName(BSTR *pDocumentName) {
   return E_NOTIMPL;
   }

   HRESULT __stdcall SpreadsheetBackEnd::put_ParentWindow(HWND hp) {
   hwndParent = hp;
   return S_OK;
   }

   HRESULT __stdcall SpreadsheetBackEnd::put_CommandLine(BSTR theCommandLine) {
   return E_NOTIMPL;
   }
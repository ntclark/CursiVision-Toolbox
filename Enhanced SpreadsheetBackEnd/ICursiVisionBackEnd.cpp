

#include "SpreadsheetBackEnd.h"

   HRESULT __stdcall SpreadsheetBackEnd::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

   if ( processingDisposition.doProperties ) {

SetProperties:

      IUnknown *pIUnknown = NULL;
      QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
      pIGProperties -> ShowProperties(hwndParent,pIUnknown);
      pIUnknown -> Release();

   }

   if ( ! pICursiVisionServices )
      return E_FAIL;

   resultDisposition *pDisposition = &processingDisposition;

   WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szSignedDocument,MAX_PATH,0,0);

   BSTR bstrLocalResultFileName = SysAllocString(bstrResultFileName);

   for ( long k = 0; k < FIELD_COUNT; k++ ) {

      if ( ! szNamePrefix[k][0] )
         continue;

      char *pValue = pICursiVisionServices -> FieldValueFromLabel(szNamePrefix[k]);

      if ( ! pValue ) {
         char szMessage[512];
         sprintf(szMessage,"The system did not find the field labeled %s in the document's printing profile."
                           "\n\nSelect Retry to respecify the properties, or Cancel to continue",szNamePrefix[k]);
         if ( IDRETRY == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_TOPMOST) )
            goto SetProperties;
         continue;
      }

      strcpy(szFieldValue[k],pValue);

   }

   FILE *fX = fopen(szWorkbookName,"rb");

   if ( ! fX ) {
      char szMessage[1024];
      sprintf(szMessage,"The file:\n\n\t%s\n\nDoes not exist.\n\nSelect Retry to specify the workbook name, or Cancel to exit.",szWorkbookName);
      if ( IDRETRY == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_TOPMOST) )
         goto SetProperties;
      return E_FAIL;
   }

   fclose(fX);

   loadExcelSpreadsheet();

   return S_OK;
   }

   HRESULT __stdcall SpreadsheetBackEnd::put_PropertiesFileName(BSTR propertiesFileName) {
   pIGProperties -> put_FileName(propertiesFileName);
   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   //pIGProperties -> Save();
   return S_OK;
   }


   HRESULT __stdcall SpreadsheetBackEnd::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }

   HRESULT __stdcall SpreadsheetBackEnd::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvbeEnhancedSpreadsheet");
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

   HRESULT __stdcall SpreadsheetBackEnd::put_PrintingSupportProfile(IPrintingSupportProfile *pp) {
   return E_NOTIMPL;
   }

   HRESULT __stdcall SpreadsheetBackEnd::ServicesAdvise(ICursiVisionServices *pServices) {
   pICursiVisionServices = pServices;
   return S_OK; 
   }
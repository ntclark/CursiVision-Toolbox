
#include "ftpBackEnd.h"

   HRESULT __stdcall FTPBackEnd::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

   isProcessing = true;

   char *requiredFields[] = {szFTPServer,szFTPUserName,szFTPPassword,NULL};

   char *requiredFieldLabels[] = {"FTP Server","FTP Account name","FTP Account Password",NULL};

   for ( long k = 0; 1; k++ ) {

      if ( ! requiredFields[k] )
         break;

      if ( requiredFields[k][0] )
         continue;

      char szMessage[512];
      sprintf(szMessage,"The required field\n\n\t%s\n\nHas not been provided. Would you like to set the properties ?\n\nChoosing No will cancel the send.",requiredFieldLabels[k]);
      if ( IDYES == MessageBox(NULL,szMessage,"Note",MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON1) ) {
         goto SetProperties;
      }

      return E_FAIL;

   }

   WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szResultFile,MAX_PATH,0,0);
   
   if ( showProperties ) {

SetProperties:

      IUnknown *pIUnknown = NULL;
      QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
      pIGProperties -> ShowProperties(hwndParent,pIUnknown);
      pIUnknown -> Release();

      if ( ! doExecute )
         return S_OK;

   } else {

      if ( 0 == sendDocument(szFTPServer,ftpPort,szFTPUserName,szFTPPassword,szResultFile,NULL) )
         goto SetProperties;

   }

   return S_OK;
   }


   HRESULT __stdcall FTPBackEnd::put_PropertiesFileName(BSTR propertiesFileName) {
   pIGProperties -> put_FileName(propertiesFileName);
   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   pIGProperties -> Save();
   return S_OK;
   }


   HRESULT __stdcall FTPBackEnd::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }


   HRESULT __stdcall FTPBackEnd::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvbeFTP");
   return S_OK;
   }

   HRESULT __stdcall FTPBackEnd::get_SavedDocumentName(BSTR *pDocumentName) {
   return E_NOTIMPL;
   }

   HRESULT __stdcall FTPBackEnd::put_ParentWindow(HWND hp) {
   hwndParent = hp;
   return S_OK;
   }

   HRESULT __stdcall FTPBackEnd::put_CommandLine(BSTR theCommandLine) {
   return E_NOTIMPL;
   }
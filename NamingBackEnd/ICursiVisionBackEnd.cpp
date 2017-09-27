
#include "NamingBackEnd.h"

#include <process.h>
#include <time.h>

   extern "C" int GetDocumentsLocation(HWND hwnd,char *);

   HRESULT __stdcall NamingBackEnd::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

   if ( processingDisposition.doProperties ) {

SetProperties:

      IUnknown *pIUnknown = NULL;
      QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
      pIGProperties -> ShowProperties((long)hwndMainFrame,pIUnknown);
      pIUnknown -> Release();

   }

   resultDisposition *pDisposition = &processingDisposition;

   char szResultFile[MAX_PATH];

   char szActiveDocument[MAX_PATH];
   char szSignedDocument[MAX_PATH];

   memset(szResultFile,0,sizeof(szResultFile));

   WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szSignedDocument,MAX_PATH,0,0);

   BSTR bstrLocalResultFileName = SysAllocString(bstrResultFileName);

   if ( ! parseName(szSignedDocument,szActiveDocument) ) {
      char szMessage[512];
      sprintf(szMessage,"The system did not find the name field in the document using the prefix string \"%s\""
                        "\n\nRemember that capitalization and the number of spaces between words is important."
                        "\n\nSelect Retry to respecify the properties, or Cancel to exit without saving",szNamePrefix);
      if ( IDRETRY == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL) )
         goto SetProperties;
      return E_FAIL;
   }

   char *p = strrchr(szSignedDocument,'\\');
   if ( ! p )
      p = strrchr(szSignedDocument,'/');
   if ( p ) {
      char szTemp[MAX_PATH];
      *p = '\0';
      sprintf(szTemp,"%s\\%s.pdf",szSignedDocument,szActiveDocument);
      *p = '\\';
      DeleteFile(szTemp);
      CopyFile(szSignedDocument,szTemp,FALSE);
      FILE *fX = fopen(szTemp,"rb");
      if ( ! fX ) {
         char szMessage[1024];
         sprintf(szMessage,"The system was not able to save the file with the base name: \n\n\"%s.pdf\""
                           "\n\nThere may be invalid characters in the name."
                           "\n\nSelect Retry to respecify the properties, or Cancel to exit without saving",szActiveDocument);
         if ( IDRETRY == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL) )
            goto SetProperties;
         return E_FAIL;
      }
      fclose(fX);
      SysFreeString(bstrLocalResultFileName);
      bstrLocalResultFileName = SysAllocStringLen(NULL,strlen(szTemp));
      MultiByteToWideChar(CP_ACP,0,szTemp,-1,bstrLocalResultFileName,MAX_PATH);
      strcpy(szActiveDocument,szTemp);
   }

   strcpy(szResultFile,szActiveDocument);
   p = strrchr(szResultFile,'.');
   if ( p )
      *p = '\0';
                  
   bool isFileSaved = false;
   HWND hwndMainFrame = hwndParent;

#define SAVE_FILE   \
   CopyFileW(bstrLocalResultFileName,bstrResultsFile,FALSE); 

#include "..\savePDFFile.cpp"

   if ( strcmp(szActiveDocument,szResultFile) )
      DeleteFile(szActiveDocument);

   SysFreeString(bstrLocalResultFileName);

   return S_OK;
   }


   HRESULT __stdcall NamingBackEnd::put_PropertiesFileName(BSTR propertiesFileName) {
   pIGProperties -> put_FileName(propertiesFileName);
   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   pIGProperties -> Save();
   return S_OK;
   }


   HRESULT __stdcall NamingBackEnd::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }

   HRESULT __stdcall NamingBackEnd::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvbeNaming");
   return S_OK;
   }

   HRESULT __stdcall NamingBackEnd::get_SavedDocumentName(BSTR *pDocumentName) {
   *pDocumentName = SysAllocString(bstrResultsFile);
   if ( ! bstrResultsFile )
      return E_FAIL;
   return S_OK;
   }


   HRESULT __stdcall NamingBackEnd::put_ParentWindow(HWND hp) {
   hwndParent = hp;
   hwndMainFrame = hp;
   return S_OK;
   }

   HRESULT __stdcall NamingBackEnd::put_CommandLine(BSTR theCommandLine) {
   return E_NOTIMPL;
   }
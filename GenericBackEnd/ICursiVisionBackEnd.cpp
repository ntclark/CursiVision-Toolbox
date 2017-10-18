// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GenericBackEnd.h"

#include <process.h>
#include <time.h>

   extern "C" int GetDocumentsLocation(HWND hwnd,char *);

   HRESULT __stdcall GenericBackEnd::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

   if ( ! szBatchFileName[0] ) {

SetProperties:

      IUnknown *pIUnknown = NULL;
      QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
      pIGProperties -> ShowProperties(hwndParent,pIUnknown);
      pIUnknown -> Release();

   }

   if ( ! doExecute )
      return S_OK;

   FILE *fX = fopen(szBatchFileName,"rb");

   if ( ! fX ) {

      char szMessage[512];
      sprintf(szMessage,"The specified batch or script file (%s) does not exist. Would you like to set the properties?",szBatchFileName);
      if ( IDYES == MessageBox(NULL,szMessage,"Note",MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON1) ) {
         goto SetProperties;
      }

      return E_FAIL;

   }

   fclose(fX);

   PROCESS_INFORMATION processInfo = {0};
   STARTUPINFO startupInfo = {0};

   SECURITY_ATTRIBUTES securityAttributes = {0}; 
 
   securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES); 
   securityAttributes.lpSecurityDescriptor = NULL; 
 
   startupInfo.cb = sizeof(STARTUPINFO); 
   startupInfo.dwFlags = STARTF_USESHOWWINDOW;
   startupInfo.lpDesktop = "Winsta0\\Default";
    
   char szCommand[1024];
   char szResultFileName[MAX_PATH];

   WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szResultFileName,MAX_PATH,0,0);

   sprintf(szCommand,"\"%s\" \"%s\"",szBatchFileName,szResultFileName);

   BOOL rc = CreateProcess(NULL,szCommand,NULL,NULL,FALSE,0,NULL,NULL,&startupInfo,&processInfo);

   if ( ! rc ) {

      char szMessage[512];
      sprintf(szMessage,"The specified batch or script file (%s) could not be started. Would you like to set the properties?",szBatchFileName);
      if ( IDYES == MessageBox(NULL,szMessage,"Note",MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON1) ) {
         goto SetProperties;
      }

      return E_FAIL;

   }

   if ( waitForCompletion )
      WaitForSingleObject(processInfo.hProcess,INFINITE);

   CloseHandle(processInfo.hProcess);
   CloseHandle(processInfo.hThread);

   return S_OK;
   }


   HRESULT __stdcall GenericBackEnd::put_PropertiesFileName(BSTR propertiesFileName) {
   pIGProperties -> put_FileName(propertiesFileName);
   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   pIGProperties -> Save();
   return S_OK;
   }


   HRESULT __stdcall GenericBackEnd::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }

   HRESULT __stdcall GenericBackEnd::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvbeGeneric");
   return S_OK;
   }

   HRESULT __stdcall GenericBackEnd::get_SavedDocumentName(BSTR *pDocumentName) {
   return E_NOTIMPL;
   }

   HRESULT __stdcall GenericBackEnd::put_ParentWindow(HWND hp) {
   hwndParent = hp;
   return S_OK;
   }

   HRESULT __stdcall GenericBackEnd::put_CommandLine(BSTR theCommandLine) {
   return E_NOTIMPL;
   }
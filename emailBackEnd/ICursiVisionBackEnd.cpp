// Copyright 2017, 2018, 2019 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "emailBackEnd.h"

   HRESULT __stdcall EmailBackEnd::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

   isProcessing = true;

   if ( showProperties ) {

SetProperties:

      IUnknown *pIUnknown = NULL;
      QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
      pIGProperties -> ShowProperties(hwndParent,pIUnknown);
      pIUnknown -> Release();

   }

   if ( ! doExecute )
      return S_OK;

   char *requiredFields[] = {szEmailServer,szEmailUserName,szEmailPassword,szEmailFrom,szEmailTo,NULL};

   char *requiredFieldLabels[] = {"SMTP Server","SMTP Account name","SMTP Account Password","From:","To:",NULL};

   for ( long k = 0; 1; k++ ) {

      if ( ! requiredFields[k] )
         break;

      if ( requiredFields[k][0] )
         continue;

      char szMessage[512];
      sprintf(szMessage,"The required field\n\n\t%s\n\nHas not been provided. Would you like to set the e-mail properties ?\n\nChoosing No will cancel the e-mail.",requiredFieldLabels[k]);
      if ( IDYES == MessageBox(NULL,szMessage,"Note",MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON1) ) {
         goto SetProperties;
      }

      return E_FAIL;

   }

   char szResultFile[MAX_PATH];
   char szOriginalFile[MAX_PATH];

   WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szResultFile,MAX_PATH,0,0);
   WideCharToMultiByte(CP_ACP,0,bstrOriginalFile,-1,szOriginalFile,MAX_PATH,0,0);

   char szSubject[MAX_PATH];
   strcpy(szSubject,szEmailSubject);

   while ( strstr(szSubject,"%f") || strstr(szSubject,"%F") ) {

      char szSubstitutedSubject[MAX_PATH];

      if ( strstr(szSubject,"%f") ) {
         *(strstr(szSubject,"%f") + 1) = 's';
         bool replaceChar = false;  
         if ( strstr(szSubject,"%F") ) {
            *(strstr(szSubject,"%F")) = '_';
            replaceChar = true;
         }
         char *pBaseName = strrchr(szOriginalFile,'/');
         if ( ! pBaseName )
            pBaseName = strrchr(szOriginalFile,'\\');
         if ( pBaseName )
            sprintf(szSubstitutedSubject,szSubject,pBaseName + 1);
         else
            sprintf(szSubstitutedSubject,szSubject,szOriginalFile);
         if ( replaceChar ) 
            *(strstr(szSubstitutedSubject,"_F")) = '%';
      } else {
         *(strstr(szSubject,"%F") + 1) = 's';
         bool replaceChar = false;  
         if ( strstr(szSubject,"%f") ) {
            *(strstr(szSubject,"%f")) = '_';
            replaceChar = true;
         }
         sprintf(szSubstitutedSubject,szSubject,szOriginalFile);
         if ( replaceChar ) 
            *(strstr(szSubstitutedSubject,"_f")) = '%';
      }

      strcpy(szSubject,szSubstitutedSubject);

   }    

   if ( 0 == sendMail(szEmailServer,smtpPort,szEmailUserName,szEmailPassword,
                       szResultFile,szOriginalFile,
                        szEmailFrom,szEmailTo,szEmailCC,szEmailBCC,szSubject,szEmailBody) )
      goto SetProperties;

   return S_OK;
   }


   HRESULT __stdcall EmailBackEnd::put_PropertiesFileName(BSTR propertiesFileName) {
   pIGProperties -> put_FileName(propertiesFileName);
   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   pIGProperties -> Save();
   return S_OK;
   }


   HRESULT __stdcall EmailBackEnd::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }


   HRESULT __stdcall EmailBackEnd::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvbeEmail");
   return S_OK;
   }

   HRESULT __stdcall EmailBackEnd::get_SavedDocumentName(BSTR *pDocumentName) {
   return E_NOTIMPL;
   }

   HRESULT __stdcall EmailBackEnd::put_ParentWindow(HWND hp) {
   hwndParent = hp;
   return S_OK;
   }

   HRESULT __stdcall EmailBackEnd::put_CommandLine(BSTR theCommandLine) {
   return E_NOTIMPL;
   }
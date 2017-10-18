// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PrintingBackEnd.h"

#include <time.h>

   extern "C" int GetDocumentsLocation(HWND hwnd,char *);

   HRESULT __stdcall PrintingBackEnd::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

   if ( processingDisposition.doProperties || ( ! useDefaultPrinter && ! szChosenPrinter[0] ) ) {

SetProperties:

      IUnknown *pIUnknown = NULL;
      QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
      pIGProperties -> ShowProperties(hwndParent,pIUnknown);
      pIUnknown -> Release();

   }

   if ( skipPrinting )
      return S_OK;

   if ( ! doExecute )
      return S_OK;

   if ( ! useDefaultPrinter && ! szChosenPrinter[0] ) {
      char szMessage[512];
      sprintf(szMessage,"The printer has not been selected."
                        "\n\nSelect Retry to respecify the properties, or Cancel to exit without printing");
      if ( IDRETRY == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL) )
         goto SetProperties;

      return E_FAIL;
   }

   if ( ! useDefaultPrinter ) {

      HANDLE hPrinter = NULL;

      if ( ! OpenPrinter(szChosenPrinter,&hPrinter,NULL) ) {
         char szMessage[512];
         sprintf(szMessage,"The system did not find the printer \n\n\t\"%s\""
                           "\n\nSelect Retry to respecify the properties, or Cancel to exit without printing",szChosenPrinter);
         if ( IDRETRY == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_TOPMOST) )
            goto SetProperties;
         return E_FAIL;
      }

      ClosePrinter(hPrinter);

   }

#if 0
   return printDocument(bstrResultFileName,szChosenPrinter,printerDevMode,sizeof(printerDevMode),copies);
#else
   return printDocument(bstrResultFileName,szChosenPrinter,NULL,0,copies);
#endif
   }


   HRESULT __stdcall PrintingBackEnd::put_PropertiesFileName(BSTR propertiesFileName) {
   pIGProperties -> put_FileName(propertiesFileName);
   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   //pIGProperties -> Save();
   return S_OK;
   }


   HRESULT __stdcall PrintingBackEnd::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }

   HRESULT __stdcall PrintingBackEnd::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvpePrinting");
   return S_OK;
   }

   HRESULT __stdcall PrintingBackEnd::get_SavedDocumentName(BSTR *pDocumentName) {
   return E_NOTIMPL;
   }

   HRESULT __stdcall PrintingBackEnd::put_ParentWindow(HWND hp) {
   hwndParent = hp;
   return S_OK;
   }


   HRESULT __stdcall PrintingBackEnd::put_CommandLine(BSTR theCommandLine) {
   return E_NOTIMPL;
   }
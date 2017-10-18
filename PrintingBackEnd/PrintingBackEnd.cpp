// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PrintingBackEnd.h"
#include <process.h>

   PrintingBackEnd::_IPropertyPage *PrintingBackEnd::pIPropertyPage = NULL;

   PrintingBackEnd::PrintingBackEnd(IUnknown *pIUnknownOuter) :

   pIGProperties(NULL),
   pIGPropertiesClient(NULL),
   pIGPropertyPageClient(NULL),

   pICursiVisionServices(NULL),

   hwndProperties(NULL),
   hwndParent(NULL),

   startParameters(0),
   endParameters(0),

   skipPrinting(false),
   useDefaultPrinter(true),

   copies(1L),

   doExecute(true),

   pIPdfEnabler(NULL),
   pIPdfDocument(NULL),

   refCount(0)

   {

   memset(szChosenPrinter,0,sizeof(szChosenPrinter));

#if 0
   memset(printerDevMode,0,2 * sizeof(DEVMODE));
#endif

   pIGPropertyPageClient = new _IGPropertyPageClient(this);

   long sizeParameters = offsetof(PrintingBackEnd,endParameters) - offsetof(PrintingBackEnd,startParameters);

   memset(&startParameters,0,sizeParameters);

   HRESULT rc = CoCreateInstance(CLSID_InnoVisioNateProperties,NULL,CLSCTX_ALL,IID_IGProperties,reinterpret_cast<void **>(&pIGProperties));

#ifdef DEBUG
   pIGProperties -> put_DebuggingEnabled(true);
#endif

   pIGPropertiesClient = new _IGPropertiesClient(this);

   pIGProperties -> Advise(static_cast<IGPropertiesClient *>(pIGPropertiesClient));

//
// 9-1-2011: IGProperties is adding a reference (as it should) which can be removed
// It may be better to not load properties in the constructor.
//
   refCount = 0L;

   pIGProperties -> Add(L"printing parameters",NULL);

   pIGProperties -> DirectAccess(L"printing parameters",TYPE_BINARY,&startParameters,sizeParameters);

   char szTemp[MAX_PATH];
   char szRootName[MAX_PATH];      

   strcpy(szRootName,szModuleName);

   char *p = strrchr(szModuleName,'\\');
   if ( ! p )
      p = strrchr(szModuleName,'/');
   if ( p ) {
      strcpy(szRootName,p + 1);
   }

   p = strrchr(szRootName,'.');
   if ( p )
      *p = '\0';

   sprintf(szTemp,"%s\\Settings\\%s.settings",szApplicationDataDirectory,szRootName);

   BSTR bstrFileName = SysAllocStringLen(NULL,MAX_PATH);
   MultiByteToWideChar(CP_ACP,0,szTemp,-1,bstrFileName,MAX_PATH);

   pIGProperties -> put_FileName(bstrFileName);

   SysFreeString(bstrFileName);

   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   if ( ! bSuccess )
      pIGPropertiesClient -> InitNew();

   pIPrintingBackEndAdditional = new _IPrintingBackEndAdditional(this);

   return;
   }


   PrintingBackEnd::~PrintingBackEnd() {
      
   if ( pIPropertyPage )
      pIPropertyPage -> Release();

   pIPropertyPage = NULL;

   if ( pICursiVisionServices ) {
      IPrintingSupportProfile *px = NULL;
      pICursiVisionServices -> get_PrintingSupportProfile(&px);
      if ( pICursiVisionServices -> IsAdministrator() || ! px )
         pIGProperties -> Save();
   }

   pIPrintingBackEndAdditional -> Release();

   return;
   }

   HRESULT PrintingBackEnd::PushProperties() {
   return pIGProperties -> Push();
   }

   HRESULT PrintingBackEnd::PopProperties() {
   return pIGProperties -> Pop();
   }

   HRESULT PrintingBackEnd::DiscardProperties() {
   return pIGProperties -> Discard();
   }

   HRESULT PrintingBackEnd::SaveProperties() {
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

   
   HRESULT PrintingBackEnd::printDocument(BSTR bstrFileName,char *pszChosenPrinter,BYTE *pPrinterDevMode,long sizeOfDevMode,long copies) {
   char szTemp[MAX_PATH];
   WideCharToMultiByte(CP_ACP,0,bstrFileName,-1,szTemp,MAX_PATH,0,0);
   return printDocument(szTemp,pszChosenPrinter,pPrinterDevMode,sizeOfDevMode);
   }


   HRESULT PrintingBackEnd::printDocument(char *pszFileName,char *pszChosenPrinter,BYTE *pPrinterDevMode,long sizeOfDevMode,long copies) {

#if 0
   char szPrinterSettingsFile[MAX_PATH];

   memset(szPrinterSettingsFile,0,MAX_PATH * sizeof(char));

   sprintf(szPrinterSettingsFile,"\"%s\\Settings\\printerSettings",szApplicationDataDirectory);
   FILE *fPrinterSettings = fopen(szPrinterSettingsFile + 1,"wb");
   fwrite(pPrinterDevMode,sizeOfDevMode,1,fPrinterSettings);
   fclose(fPrinterSettings);

   szPrinterSettingsFile[strlen(szPrinterSettingsFile)] = '\"';
#endif

   char szExecutable[MAX_PATH];
   char szSignedDocument[MAX_PATH];
   char szPrinter[128];

   memset(szSignedDocument,0,sizeof(szSignedDocument));

   char *pszPrintFile = _tempnam(NULL,"cvtmp");

   CopyFile(pszFileName,pszPrintFile,FALSE);

   szSignedDocument[0] = '\"';

   strcpy(szSignedDocument + 1,pszPrintFile);

   szSignedDocument[strlen(szSignedDocument)] = '\"';

   sprintf(szPrinter,"\"%s\"",pszChosenPrinter);

#if 1

   GetModuleFileName(hModule,szExecutable,MAX_PATH);

   char *p = strrchr(szExecutable,'\\');
   if ( ! p )
      p = strrchr(szExecutable,'/');

   if ( p )
      *p = '\0';

#else
   HKEY hKey = NULL;

   if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\InnoVisioNate\\CursiVision",0,KEY_QUERY_VALUE,&hKey) ) {
      DWORD cb = MAX_PATH;
      RegQueryValueEx(hKey,"Installation Directory",NULL,NULL,(BYTE *)szExecutable,&cb);
      RegCloseKey(hKey);
   } else
      return E_FAIL;

#endif

   char szCopies[8];
   sprintf(szCopies,"%ld",copies);

   sprintf(szExecutable + strlen(szExecutable),"\\Print Document.exe");

#if 0
   _spawnl(_P_NOWAIT,szExecutable,"/File",szSignedDocument,"/PrintTo",szPrinter,"/Settings",szPrinterSettingsFile,"/Copies",szCopies,NULL);
#else
   if ( useDefaultPrinter )
      _spawnl(_P_NOWAIT,szExecutable,"/File",szSignedDocument,"/DefaultPrinter","/Copies",szCopies,NULL);
   else
      _spawnl(_P_NOWAIT,szExecutable,"/File",szSignedDocument,"/PrintTo",szPrinter,"/Copies",szCopies,NULL);
#endif

   return S_OK;
   }


   
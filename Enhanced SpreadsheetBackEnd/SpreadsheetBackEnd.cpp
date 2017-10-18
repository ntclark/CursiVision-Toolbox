// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SpreadsheetBackEnd.h"

   SpreadsheetBackEnd::_IPropertyPage *SpreadsheetBackEnd::pIPropertyPage = NULL;

   SpreadsheetBackEnd::SpreadsheetBackEnd(IUnknown *pIUnknownOuter) :

   pIGProperties(NULL),
   pIGPropertiesClient(NULL),
   pIGPropertyPageClient(NULL),

   pICursiVisionServices(NULL),

   hwndProperties(NULL),
   hwndParent(NULL),

   startParameters(0),
   endParameters(0),

   includeDate(true),
   includeTime(true),
   includeDocumentLink(true),

   refCount(0)

   {

   memset(szSignedDocument,0,sizeof(szSignedDocument));
   memset(szNamePrefix,0,sizeof(szNamePrefix));
   memset(szColumnName,0,sizeof(szColumnName));
   memset(szColumnId,0,sizeof(szColumnId));
   memset(szWorkbookName,0,sizeof(szWorkbookName));
   memset(szSpreadsheetName,0,sizeof(szSpreadsheetName));
   memset(szAllSheetNames,0,sizeof(szAllSheetNames));
   memset(szFieldValue,0,sizeof(szFieldValue));
   memset(szDateColumn,0,sizeof(szDateColumn));
   memset(szTimeColumn,0,sizeof(szTimeColumn));
   memset(szDocumentLinkColumn,0,sizeof(szDocumentLinkColumn));

   CLSIDFromString(L"Excel.Application",&CLSID_excel);

   pIGPropertyPageClient = new _IGPropertyPageClient(this);

   long sizeParameters = offsetof(SpreadsheetBackEnd,endParameters) - offsetof(SpreadsheetBackEnd,startParameters);

   memset(&startParameters,0,sizeParameters);

   memset(szNamePrefix,0,sizeof(szNamePrefix));

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

   pIGProperties -> Add(L"spreadsheet parameters",NULL);
   pIGProperties -> DirectAccess(L"spreadsheet parameters",TYPE_BINARY,&startParameters,sizeParameters);

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

   return;
   }


   SpreadsheetBackEnd::~SpreadsheetBackEnd() {

   IPrintingSupportProfile *px = NULL;
   pICursiVisionServices -> get_PrintingSupportProfile(&px);
   if ( pICursiVisionServices -> IsAdministrator() || ! px )
      pIGProperties -> Save();

   if ( pIPropertyPage )
      pIPropertyPage -> Release();

   pIPropertyPage = NULL;

   return;
   }

   HRESULT SpreadsheetBackEnd::PushProperties() {
   return pIGProperties -> Push();
   }

   HRESULT SpreadsheetBackEnd::PopProperties() {
   return pIGProperties -> Pop();
   }

   HRESULT SpreadsheetBackEnd::DiscardProperties() {
   return pIGProperties -> Discard();
   }

   HRESULT SpreadsheetBackEnd::SaveProperties() {
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


   long SpreadsheetBackEnd::getExcelWorksheets(char *pszFile) {

   Excel::_Application *pIApplication = NULL;

   long count = 0L;

   try {

   HWND hwndExcel = FindWindow("XLMain",NULL);
   if ( hwndExcel )
      hwndExcel = (HWND)FindWindowWithClass(hwndExcel,"Excel7");

   HRESULT hr = S_OK;

   if ( hwndExcel ) {
      Excel::Window *pIWindow;
      hr = AccessibleObjectFromWindow(hwndExcel, OBJID_NATIVEOM,__uuidof(Excel::Window),(void**)&pIWindow);
      if ( ! pIWindow ) 
         hr = CoCreateInstance(CLSID_excel,NULL,CLSCTX_LOCAL_SERVER,__uuidof(Excel::_Application),reinterpret_cast<void**>(&pIApplication));
      else {
         Excel::_ApplicationPtr theApplication = pIWindow -> GetApplication();
         hr = theApplication.QueryInterface(__uuidof(Excel::_Application),&pIApplication);
         pIWindow -> Release();
      }
   } else
      hr = CoCreateInstance(CLSID_excel,NULL,CLSCTX_LOCAL_SERVER,__uuidof(Excel::_Application),reinterpret_cast<void**>(&pIApplication));

   if ( S_OK != hr ) {
      char szMessage[1024];
      sprintf(szMessage,"There was an error opening Excel with the workbook %s.\n\nIs Microsoft Excel Installed ?",pszFile);
      MessageBox(NULL,szMessage,"Error",MB_ICONEXCLAMATION | MB_TOPMOST);
      return 0L;
   }

   Excel::Workbooks *pIWorkbooks = NULL;

   hr = pIApplication -> get_Workbooks(&pIWorkbooks);

   BSTR bstrFile = SysAllocStringLen(NULL,MAX_PATH);

   MultiByteToWideChar(CP_ACP,0,pszFile,-1,bstrFile,MAX_PATH);

   count = 0L;
   pIWorkbooks -> get_Count(&count);
   bool wasOpen = false;

   Excel::_Workbook *pIWorkbook = NULL;

   for ( long k = 0; k < count; k++ ) {

      VARIANT vIndex;
      vIndex.vt = VT_I4;
      vIndex.lVal = k + 1;

      hr = pIWorkbooks -> get_Item(vIndex,&pIWorkbook);

      bstr_t theName = pIWorkbook -> GetFullName();

      if ( 0 == wcscmp(theName,bstrFile) ) {
         wasOpen = true;
         break;
      }

      pIWorkbook -> Release();

      pIWorkbook = NULL;

   } 

   if ( ! wasOpen ) {
      VARIANT vIndex;
      vIndex.vt = VT_I4;
      vIndex.lVal = 1L;
      pIWorkbooks -> Open(bstrFile);
      pIWorkbooks -> get_Item(vIndex,&pIWorkbook);
   }

   SysFreeString(bstrFile);

   Excel::Sheets *pISheets = NULL;
   hr = pIWorkbook -> get_Worksheets(&pISheets);

   count = 0L;
   pISheets -> get_Count(&count);

   memset(szAllSheetNames,0,sizeof(szAllSheetNames));
   char *pNames = szAllSheetNames;
   char *pEnd = pNames + sizeof(szAllSheetNames);

//   bool wasFound = false;

   for ( long k = 0; k < count; k++ ) {

      VARIANT vIndex;
      vIndex.vt = VT_I4;
      vIndex.lVal = k + 1;

      IDispatch *pIDispatch = NULL;
      Excel::_Worksheet *pIWorksheet = NULL;

      hr = pISheets -> get_Item(vIndex,&pIDispatch);

      pIDispatch -> QueryInterface(__uuidof(Excel::_Worksheet),reinterpret_cast<void **>(&pIWorksheet));

      bstr_t theName = pIWorksheet -> GetName();

      pIDispatch -> Release();

      pIWorksheet -> Release();

      WideCharToMultiByte(CP_ACP,0,theName.GetBSTR(),-1,pNames,(int)(pEnd - pNames),0,0);

      pNames += strlen(pNames) + 1;

   }

   VARIANT vtFalse;
   vtFalse.vt = VT_BOOL;
   vtFalse.boolVal = false;

   if ( ! wasOpen )
      pIWorkbook -> Close(vtFalse);

   pISheets -> Release();
   pIWorkbook -> Release();
   pIWorkbooks -> Release();
   pIApplication -> Release();

   } catch ( _com_error e ) {

      char szMessage[1024];

      sprintf(szMessage,"There was an error working with the workbook %s in worksheet %s.\n\nIs Microsoft Excel Installed ?"
                           "Please check your Workbook or try using a different one.",
                  szWorkbookName,szSpreadsheetName);

      MessageBox(NULL,szMessage,"Error",MB_ICONEXCLAMATION | MB_TOPMOST);

   }

   return count;
   }


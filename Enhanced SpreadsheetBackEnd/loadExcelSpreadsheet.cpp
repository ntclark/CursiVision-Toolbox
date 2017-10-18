// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SpreadsheetBackEnd.h"
#include <time.h>

   long SpreadsheetBackEnd::loadExcelSpreadsheet() {

   long count = 0L;
   long cutoff = -1L;
   bool wasOpen = false;
   long isHyperLink[FIELD_COUNT] = {0};

   VARIANT vtFalse;
   vtFalse.vt = VT_BOOL;
   vtFalse.boolVal = false;
   
   VARIANT vtIndex;
   vtIndex.vt = VT_I4;
   vtIndex.lVal = 0L;

   Excel::_Application *pIApplication = NULL;
   Excel::Workbooks *pIWorkbooks = NULL;
   Excel::_Workbook *pIWorkbook = NULL;
   Excel::Sheets *pIWorksheets = NULL;
   Excel::_Worksheet *pIWorksheet = NULL;

   BSTR bstrFile = SysAllocStringLen(NULL,MAX_PATH);

   MultiByteToWideChar(CP_ACP,0,szWorkbookName,-1,bstrFile,MAX_PATH);

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

   hr = pIApplication -> get_Workbooks(&pIWorkbooks);

   count = 0L;

   pIWorkbooks -> get_Count(&count);

   for ( long k = 0; k < count; k++ ) {

      vtIndex.lVal = k + 1;

      hr = pIWorkbooks -> get_Item(vtIndex,&pIWorkbook);

      bstr_t theName = pIWorkbook -> GetFullName();

      if ( 0 == wcscmp(theName,bstrFile) ) {
         wasOpen = true;
         break;
      }

      pIWorkbook -> Release();

      pIWorkbook = NULL;

   } 

   if ( ! wasOpen ) {
      vtIndex.lVal = count + 1;
      pIWorkbooks -> Open(bstrFile);
      pIWorkbooks -> get_Item(vtIndex,&pIWorkbook);
   }

   SysFreeString(bstrFile);

   hr = pIWorkbook -> get_Worksheets(&pIWorksheets);

   count = 0L;
   pIWorksheets -> get_Count(&count);
   char szThisSheet[64];

   std::list<char *> existingSheets;

   for ( long k = 0; k < count; k++ ) {

      vtIndex.lVal = k + 1;

      IDispatch *pIDispatch = NULL;

      hr = pIWorksheets -> get_Item(vtIndex,&pIDispatch);

      pIDispatch -> QueryInterface(__uuidof(Excel::_Worksheet),reinterpret_cast<void **>(&pIWorksheet));

      bstr_t theName = pIWorksheet -> GetName();

      pIDispatch -> Release();

      WideCharToMultiByte(CP_ACP,0,theName.GetBSTR(),-1,szThisSheet,64,0,0);

      char *p = new char[strlen(szThisSheet) + 1];
      strcpy(p,szThisSheet);
      existingSheets.insert(existingSheets.end(),p);

      if ( 0 == strcmp(szThisSheet,szSpreadsheetName) )
         break;

      pIWorksheet -> Release();

      pIWorksheet = NULL;

   }

   if ( ! pIWorksheet ) {

      char szMessage[1024];
      sprintf(szMessage,"The workbook %s does not contain the worksheet %s.\n\nWould you like CursiVision to create this worksheet in the workbook ?",szWorkbookName,szSpreadsheetName);
      if ( IDNO == MessageBox(NULL,szMessage,"Error!",MB_YESNO | MB_DEFBUTTON1 | MB_TOPMOST) ) {

         if ( ! wasOpen )
            pIWorkbook -> Close(vtFalse);

         pIWorksheets -> Release();
         pIWorkbook -> Release();
         pIWorkbooks -> Release();
         pIApplication -> Release();

         for ( std::list<char *>::iterator it = existingSheets.begin(); it != existingSheets.end(); it++ )
            delete [] (*it);

         existingSheets.clear();

         return 0L;
      }
      
      pIWorksheets -> Add();

      pIWorksheets -> get_Count(&count);

      for ( long k = 0; k < count; k++ ) {
   
         vtIndex.lVal = k + 1;

         IDispatch *pIDispatch = NULL;

         hr = pIWorksheets -> get_Item(vtIndex,&pIDispatch);

         pIDispatch -> QueryInterface(__uuidof(Excel::_Worksheet),reinterpret_cast<void **>(&pIWorksheet));

         bstr_t theName = pIWorksheet -> GetName();

         pIDispatch -> Release();

         WideCharToMultiByte(CP_ACP,0,theName.GetBSTR(),-1,szThisSheet,64,0,0);
         
         bool wasFound = false;
         for ( std::list<char *>::iterator it = existingSheets.begin(); it != existingSheets.end(); it++ ) {
            if ( 0 == strcmp((*it),szThisSheet) ) {
               wasFound = true;
               break;
            }
         }
   
         if ( ! wasFound )
            break;

         pIWorksheet -> Release();

         pIWorksheet = NULL;

      }

      BSTR theSheetName = SysAllocStringLen(NULL,64);

      MultiByteToWideChar(CP_ACP,0,szSpreadsheetName,-1,theSheetName,64);

      pIWorksheet -> PutName(theSheetName);

      SysFreeString(theSheetName);

   }

   for ( std::list<char *>::iterator it = existingSheets.begin(); it != existingSheets.end(); it++ )
      delete [] (*it);

   existingSheets.clear();

   VARIANT row,column,value;

   row.vt = VT_I4;
   row.lVal = 1L;
   column.vt = VT_BSTR;
   column.bstrVal = SysAllocStringLen(NULL,16);

   char szColumn[64];

   value.vt = VT_BSTR;
   value.bstrVal = SysAllocStringLen(NULL,128);

   long entryRow = 0L;

   for ( long k = 0; k < FIELD_COUNT; k++ ) {

      isHyperLink[k] = 0L;

      if ( ! szColumnId[k][0] ) {
         
         cutoff = k;
         long nextK = k;

         if ( includeDate ) {
            sprintf(szColumnId[k],szDateColumn);
            if ( ! szColumnId[k][0] ) {
               MessageBox(NULL,"You cannot include the date signed field without also specifying the column letter in the spreadsheet.","Error",MB_OK | MB_TOPMOST);
            } else {
               _strdate(szFieldValue[k]);
               sprintf(szColumnName[k],"Date Signed");
               nextK = k + 1;
            }
         }

         k = nextK;

         if ( includeTime ) {
            sprintf(szColumnId[k],szTimeColumn);
            if ( ! szColumnId[k][0] ) {
               MessageBox(NULL,"You cannot include the time signed field without also specifying the column letter in the spreadsheet.","Error",MB_OK | MB_TOPMOST);
            } else {
               _strtime(szFieldValue[k]);
               sprintf(szColumnName[k],"Time Signed");
               nextK = k + 1;
            }
         }

         k = nextK;

         if ( includeDocumentLink ) {
            if ( ! szDocumentLinkColumn[0] ) {
               MessageBox(NULL,"You cannot include the document link without also specifying the column letter in the spreadsheet.","Error",MB_OK | MB_TOPMOST);
            } else {
               isHyperLink[k] = 1L;
               sprintf(szColumnId[k],szDocumentLinkColumn);
               sprintf(szFieldValue[k],szSignedDocument);
               sprintf(szColumnName[k],"Signed Document");
            }
         }

         break;

      }

   }

   entryRow = 2;

   for ( long k = 0; k < FIELD_COUNT; k++ ) {

      if ( ! szColumnId[k][0] )
         break;

      sprintf(szColumn,"$%s$1:$%s$1",szColumnId[k],szColumnId[k]);

      MultiByteToWideChar(CP_ACP,0,szColumn,-1,column.bstrVal,16);

      MultiByteToWideChar(CP_ACP,0,szColumnName[k],-1,value.bstrVal,128);

      pIWorksheet -> GetCells() -> GetRange(&column) -> PutValue2(&value);

      for ( long j = 2; 1; j++ ) {
         sprintf(szColumn,"$%s$%ld",szColumnId[k],j);
         MultiByteToWideChar(CP_ACP,0,szColumn,-1,column.bstrVal,16);
         VARIANT v = pIWorksheet -> GetCells() -> GetRange(column) -> GetValue2();
         if ( v.vt == VT_EMPTY ) {
            entryRow = max(entryRow,j);
            break;
         }

      }

   }

   for ( long k = 0; k < FIELD_COUNT; k++ ) {

      if ( ! szColumnId[k][0] )
         break;

      if ( ! szFieldValue[k][0] )
         continue;

      MultiByteToWideChar(CP_ACP,0,szFieldValue[k],-1,value.bstrVal,128);

      sprintf(szColumn,"$%s$%ld",szColumnId[k],entryRow);
      MultiByteToWideChar(CP_ACP,0,szColumn,-1,column.bstrVal,16);

      if ( isHyperLink[k] ) {
         Excel::Hyperlinks *pIHyperlinks = NULL;
         pIWorksheet -> get_Hyperlinks(&pIHyperlinks);
         bstr_t theLink(value.bstrVal);
         char *p = strrchr(szFieldValue[k],'\\');
         if ( ! p )
            p = strrchr(szFieldValue[k],'/');
         if ( ! p )
            p = szFieldValue[k] - 1;
         bstr_t theText(p + 1);
         pIHyperlinks -> Add(pIWorksheet -> GetCells() -> GetRange(column).GetInterfacePtr(),theLink,&vtMissing,&vtMissing,theText);
         pIHyperlinks -> Release();
      } else
         pIWorksheet -> GetCells() -> GetRange(&column) -> PutValue2(&value);

   }

   SysFreeString(value.bstrVal);
   SysFreeString(column.bstrVal);

   } catch ( _com_error e ) {

      char szMessage[1024];

      sprintf(szMessage,"There was an error working with the workbook %s in worksheet %s.\n\nThe Excel subsystem returned the error %s (%ld).\n\n"
                           "Please check your Workbook or try using a different one.",
                  szWorkbookName,szSpreadsheetName,e.ErrorMessage(),e.Error());

      MessageBox(NULL,szMessage,"Error",MB_ICONEXCLAMATION | MB_TOPMOST);

   }

   if ( -1L != cutoff ) {
      for ( long k = cutoff; k < FIELD_COUNT; k++ ) {
         memset(szColumnId[k],0,sizeof(szColumnId[k]));
         memset(szColumnName[k],0,sizeof(szColumnName[k]));
         memset(szNamePrefix[k],0,sizeof(szNamePrefix[k]));
      }
   }

   if ( pIWorkbook && ! wasOpen )
      pIWorkbook -> Close(VARIANT_TRUE);

   if ( pIWorksheet )
      pIWorksheet -> Release();
   if ( pIWorksheets )
      pIWorksheets -> Release();
   if ( pIWorkbook )
      pIWorkbook -> Release();
   if ( pIWorkbooks )
      pIWorkbooks -> Release();
   if ( pIApplication )
      pIApplication -> Release();

   return count;
   }

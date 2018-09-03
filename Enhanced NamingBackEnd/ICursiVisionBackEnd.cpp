// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EnhancedNamingBackEnd.h"

#include <process.h>
#include <time.h>

   extern "C" int GetDocumentsLocation(HWND hwnd,char *);

   HRESULT __stdcall NamingBackEnd::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

   if ( ! pICursiVisionServices || processingDisposition.doProperties ) {

SetProperties:

      IUnknown *pIUnknown = NULL;
      QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
      pIGProperties -> ShowProperties(hwndMainFrame,pIUnknown);
      pIUnknown -> Release();

   }

   if ( ! pICursiVisionServices )
      return E_FAIL;

   resultDisposition *pDisposition = &processingDisposition;

   char szResultFile[MAX_PATH];

   memset(szResultFile,0,sizeof(szResultFile));

   char szCombinedName[MAX_PATH];

   memset(szCombinedName,0,sizeof(szCombinedName));

   for ( long index = 0; index < 2; index++ ) {

      if ( ! szNamePrefix[index][0] )
         continue;

      if ( 0 == strcmp(szNamePrefix[index],"<none>") )
         continue;

      char *pValue = pICursiVisionServices -> FieldValueFromLabel(szNamePrefix[index]);

      if ( ! pValue ) {
         char szMessage[512];
         sprintf(szMessage,"The system did not find the field labeled %s in the document's printing profile."
                           "\n\nSelect Retry to respecify the properties, or Cancel to continue",szNamePrefix[index]);
         if ( IDRETRY == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_TOPMOST) )
            goto SetProperties;
         return E_FAIL;
      }

      char *p = pValue + strlen(pValue) - 1;
      while ( p > pValue && *p == ' ' ) {
         *p = '\0';
         p--;
      }

      p = pValue;
      while ( *p ) {
         if ( 32 > *p )
            *p = ' ';
         if ( strchr("<>:/\\|?*",*p) )
            *p = ' ';
         p++;
      }

      sprintf(szCombinedName + strlen(szCombinedName),"%s ",pValue);

   }

   if ( ' ' == szCombinedName[strlen(szCombinedName)] )
      szCombinedName[strlen(szCombinedName)] = '\0';

   char szSignedDocument[MAX_PATH];
   char szNewDocument[MAX_PATH];

   WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szSignedDocument,MAX_PATH,0,0);
//
// szSignedDocument is the document name on input, replace it's base name with the new name
//
   char *p = strrchr(szSignedDocument,'\\');
   if ( ! p )
      p = strrchr(szSignedDocument,'/');

   if ( p ) {

      char szTemp[MAX_PATH];

      *p = '\0';
      sprintf(szTemp,"%s\\%s.pdf",szSignedDocument,szCombinedName);
      *p = '\\';

      strcpy(szNewDocument,szTemp);

   } else

      strcpy(szNewDocument,szCombinedName);

   BSTR bstrActiveDocument = SysAllocStringLen(NULL,(DWORD)strlen(szSignedDocument));
   MultiByteToWideChar(CP_ACP,0,szSignedDocument,-1,bstrActiveDocument,(DWORD)strlen(szSignedDocument));

//
//NTC: 03-12-2012: Attempting to copy the file to check if the file name is valid. This approach doesn't work
// because if the other "Save Location" settings do not embellish the file name in any way, and, if this
// copy is successful, the file will exist and the user will be prompted for replace.
//
// I also do not know why the following (expanded) definition for SAVE_FILE does not seem to work either.
//
// For now, I am leaving the CopyFile test in place - and deleting the copied file - this potentially could overwrite
// an existing undecorated file.
//
#if 1
   if ( 0 == CopyFile(szSignedDocument,szNewDocument,FALSE) ) {

      char szMessage[1024];
      sprintf(szMessage,"The system was not able to save the file with the base name: \n\n\t\"%s.pdf\""
                        "\n\nThere may be invalid characters in the name."
                        "\n\nSelect File Save-As from the main menu to save this file.",szCombinedName);

      if ( IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_OK | MB_TOPMOST ) )
         return E_FAIL;

      OPENFILENAMEA openFileName;

      char szFilter[MAX_PATH];
      char szTemp[2 * MAX_PATH];

      memset(szFilter,0,sizeof(szFilter));
      memset(&openFileName,0,sizeof(OPENFILENAME));

      sprintf(szTemp,"Save %s.",szCombinedName);

      sprintf(szFilter,"Signed Documents");
      long k = (DWORD)strlen(szFilter) + sprintf(szFilter + (DWORD)strlen(szFilter) + 1,"*.pdf");
      k = k + sprintf(szFilter + k + 2,"All Files");
      sprintf(szFilter + k + 3,"*.*");

      openFileName.lStructSize = sizeof(OPENFILENAMEA);
      openFileName.hwndOwner = hwndMainFrame;
      openFileName.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
      openFileName.lpstrFilter = szFilter;
      openFileName.lpstrFile = szResultFile;
      openFileName.lpstrDefExt = "pdf";
      openFileName.nFilterIndex = 1;
      openFileName.nMaxFile = MAX_PATH;
      openFileName.lpstrTitle = szTemp;

      sprintf(openFileName.lpstrFile,"%s.pdf",szCombinedName);

      if ( ! GetSaveFileNameA(&openFileName) ) 
         return E_FAIL;

      strcpy(szNewDocument,openFileName.lpstrFile);

   } else 

      DeleteFile(szNewDocument);

#endif

   strcpy(szResultFile,szNewDocument);
   p = strrchr(szResultFile,'.');
   if ( p )
      *p = '\0';

   bool isFileSaved = false;

   HWND hwndMainFrame = hwndParent;

#if 1
#define SAVE_FILE                                                                                              \
   CopyFileW(bstrActiveDocument,bstrResultsFile,FALSE);
#else
#define SAVE_FILE                                                                                              \
   {                                                                                                           \
   if ( ! CopyFileW(bstrActiveDocument,bstrResultsFile,FALSE) ) {                                              \
      char szMessage[1024];                                                                                    \
      sprintf(szMessage,"CursiVision was not able to save the file with the base name: \n\n\t\"%s.pdf\""       \
                        "\n\nThere may be invalid characters in the name."                                     \
                        "\n\nSelect Ok to manually save the file, or Cancel to exit without saving",pValue);   \
                                                                                                               \
      if ( IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_OKCANCEL | MB_TOPMOST ) )    \
         return E_FAIL;                                                                                        \
                                                                                                               \
      OPENFILENAMEA ofn;                                                                                       \
                                                                                                               \
      char szFilter[MAX_PATH];                                                                                 \
      char szTemp[2 * MAX_PATH];                                                                               \
                                                                                                               \
      memset(szFilter,0,sizeof(szFilter));                                                                     \
      memset(&ofn,0,sizeof(ofn));                                                                              \
                                                                                                               \
      sprintf(szTemp,"Save %s",pValue);                                                                        \
                                                                                                               \
      sprintf(szFilter,"Signed Documents");                                                                    \
      long k = strlen(szFilter) + sprintf(szFilter + strlen(szFilter) + 1,"*.pdf");                            \
      k = k + sprintf(szFilter + k + 2,"All Files");                                                           \
      sprintf(szFilter + k + 3,"*.*");                                                                         \
                                                                                                               \
      ofn.lStructSize = sizeof(OPENFILENAMEA);                                                                 \
      ofn.hwndOwner = hwndMainFrame;                                                                           \
      ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;                                                     \
      ofn.lpstrFilter = szFilter;                                                                              \
      ofn.lpstrFile = szResultFile;                                                                            \
      ofn.lpstrDefExt = "pdf";                                                                                 \
      ofn.nFilterIndex = 1;                                                                                    \
      ofn.nMaxFile = MAX_PATH;                                                                                 \
      ofn.lpstrTitle = szTemp;                                                                                 \
                                                                                                               \
      sprintf(ofn.lpstrFile,"%s.pdf",pValue);                                                                  \
                                                                                                               \
      if ( ! GetSaveFileNameA(&ofn) )                                                                          \
         return E_FAIL;                                                                                        \
                                                                                                               \
      strcpy(szActiveDocument,ofn.lpstrFile);                                                                  \
      strcpy(szResultFile,szActiveDocument);                                                                   \
      char *psx = strrchr(szResultFile,'.');                                                                   \
      if ( psx )                                                                                               \
         *psx = '\0';                                                                                          \
                                                                                                               \
      goto reAttemptFileDecoration;                                                                            \
                                                                                                               \
   }                                                                                                           \
   }
#endif

#include "savePDFFile.cpp"

   if ( stricmp(szNewDocument,szResultFile) )
      DeleteFile(szNewDocument);

   SysFreeString(bstrActiveDocument);

   return S_OK;
   }


   HRESULT __stdcall NamingBackEnd::put_PropertiesFileName(BSTR propertiesFileName) {
   pIGProperties -> put_FileName(propertiesFileName);
   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   //pIGProperties -> Save();
   return S_OK;
   }


   HRESULT __stdcall NamingBackEnd::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }

   HRESULT __stdcall NamingBackEnd::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvbeEnhancedNaming");
   return S_OK;
   }

   HRESULT __stdcall NamingBackEnd::get_SavedDocumentName(BSTR *pDocumentName) {
   if ( ! bstrResultsFile )
      return E_FAIL;
   *pDocumentName = SysAllocString(bstrResultsFile);
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


   HRESULT __stdcall NamingBackEnd::put_PrintingSupportProfile(IPrintingSupportProfile *pp) {
   return E_NOTIMPL;
   }

   HRESULT __stdcall NamingBackEnd::ServicesAdvise(ICursiVisionServices *pServices) {
   pICursiVisionServices = pServices;
   return S_OK; 
   }
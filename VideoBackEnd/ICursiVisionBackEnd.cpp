
#include "VideoBackEnd.h"

#include <process.h>
#include <time.h>

   extern "C" int GetDocumentsLocation(HWND hwnd,char *);

   HRESULT __stdcall VideoBackEnd::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

   strcpy(szTargetFile,_tempnam(NULL,NULL));

   isProcessing = true;

   if ( processingDisposition.doProperties || ( ! useAnyCamera && ! szChosenDevice[0] ) ) {

SetProperties:

      IUnknown *pIUnknown = NULL;
      QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
      pIGProperties -> ShowProperties(hwndParent,pIUnknown);
      pIUnknown -> Release();

   }

   if ( skipImaging )
      return S_OK;

   if ( ! doExecute )
      return S_OK;

   if ( ! szChosenDevice[0] && ! useAnyCamera ) {
      char szMessage[512];
      sprintf(szMessage,"The camera has not been selected."
                        "\n\nSelect Retry to respecify the properties, or Cancel to exit without taking a photograph");
      if ( IDRETRY == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_TOPMOST) )
         goto SetProperties;

      return E_FAIL;
   }

   if ( 0 == cameraCount && ! ignoreNoCamera ) {
      char szMessage[1024];
      sprintf(szMessage,"There is no appropriate camera installed on this computer.\n\nThe use of the Video tool should be unspecified, or, the option\nto ignore the error on computers with no camera should be checked.");
      MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_TOPMOST);
      return E_FAIL;
   }

   long deviceIndex = -1L;

   if ( useAnyCamera )
      wcscpy(szChosenDevice,pCameraNames[0]);
      
   for ( long k = 0; k < cameraCount; k++ ) {
      if ( wcscmp(szChosenDevice,pCameraNames[k]) ) 
         continue;
      deviceIndex = k;
      break;
   }

   if ( -1L == deviceIndex && ! ignoreNoCamera ) {

      char szMessage[1024];
      sprintf(szMessage,"The device associated with the name: %ls\n\nWas not found on this system, is it disconnected ?\n\nPress Retry to specify the settings, or Cancel to exit",szChosenDevice);
      if ( IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1 ) )
         return E_FAIL;
      goto SetProperties;

   }

   if ( -1L == deviceIndex && ! processingDisposition.doSave )
      return S_OK;

   if ( -1L == deviceIndex && ! saveDocumentAnyway )
      return S_OK;


   if ( ! ( -1L == deviceIndex ) ) {

      FILE *fX = fopen(szTargetFile,"rb");

      if ( ! fX ) {

         IUnknown *pIUnknown = NULL;
         QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
         pIGProperties -> ShowProperties(hwndParent,pIUnknown);
         pIUnknown -> Release();

         fX = fopen(szTargetFile,"rb");
         if ( ! fX )
            return S_OK;

      }

      fclose(fX);
   
      HRESULT rc = CoCreateInstance(CLSID_PdfEnabler,NULL,CLSCTX_ALL,IID_IPdfEnabler,reinterpret_cast<void **>(&pIPdfEnabler));

      pIPdfEnabler -> Document(&pIPdfDocument);

      if ( ! ( S_OK == pIPdfDocument -> Open(bstrResultFileName,NULL,NULL) ) ) {

         pIPdfDocument -> Release();
         pIPdfEnabler -> Release();

         if ( ! keepImage )
            DeleteFile(szTargetFile);

         char szMessage[1024];
         char szDocument[MAX_PATH];
         WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szDocument,MAX_PATH,0,0);
         sprintf(szMessage,"The InnoVisioNate Video processing tool could not open the Document %s.\n\nThe Windows error code is: %ld\n\nPress Retry to specify the settings, or Cancel to exit",szDocument,GetLastError());
         if ( IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1 ) ) 
            return E_FAIL;

         goto SetProperties;

      }

      IPdfPage *pIPdfPage = NULL;

      if ( specifyPage ) {

         pIPdfDocument -> Page(pageNumber,NULL,&pIPdfPage);

         if ( NULL == pIPdfPage ) {

            pIPdfDocument -> Release();
            pIPdfEnabler -> Release();

            if ( ! keepImage )
               DeleteFile(szTargetFile);

            char szMessage[1024];
            sprintf(szMessage,"The page requested (%ld) does not exist in the document.\n\nPress Retry to specify the settings, or Cancel to exit",pageNumber);
            if ( IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1 ) ) 
               return E_FAIL;
            goto SetProperties;
   
         }

      } else if ( lastPage ) {

         pIPdfDocument -> LastPage(&pIPdfPage);

      } else {

         pIPdfDocument -> AddPage(NULL,&pIPdfPage);

      }

      BSTR bstrImageFile = SysAllocStringLen(NULL,MAX_PATH);
      MultiByteToWideChar(CP_ACP,0,szTargetFile,-1,bstrImageFile,MAX_PATH);

      if ( fitToPage )
         pIPdfPage -> AddCenteredImageFromFile(inchesLeft,inchesTop,bstrImageFile);
      else
         if ( keepAspectRatio )
            pIPdfPage -> AddSizedImageFromFile(inchesLeft,inchesTop,inchesWidth,-1.0,bstrImageFile);
         else
            pIPdfPage -> AddSizedImageFromFile(inchesLeft,inchesTop,inchesWidth,inchesHeight,bstrImageFile);

      SysFreeString(bstrImageFile);

      pIPdfPage -> Release();

      if ( ! processingDisposition.doSave ) {

         if ( ! keepImage )

            DeleteFile(szTargetFile);

         else {

            char szMoveImage[MAX_PATH];

            WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szMoveImage,MAX_PATH,0,0);

            char *p = strrchr(szMoveImage,'.');
            if ( ! p ) 
               p = szMoveImage + strlen(szMoveImage);

            sprintf(p,".jpg");

            DeleteFile(szMoveImage);
            CopyFile(szTargetFile,szMoveImage,FALSE);
            DeleteFile(szTargetFile);

         }

         pIPdfDocument -> Write(bstrResultFileName);

         bstrResultsFile = SysAllocString(bstrResultFileName);

         pIPdfDocument -> Release();
         pIPdfEnabler -> Release();

         return S_OK;

      }

   }

   bool isFileSaved = false;

   char szResultFile[MAX_PATH];
   memset(szResultFile,0,sizeof(szResultFile));

   if ( isTempFile )
      WideCharToMultiByte(CP_ACP,0,bstrOriginalFile,-1,szResultFile,MAX_PATH,0,0);
   else
      WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szResultFile,MAX_PATH,0,0);

   char *p = strrchr(szResultFile,'.');
   if ( p )
      *p = '\0';

   bstrResultsFile = SysAllocStringLen(NULL,MAX_PATH);

   resultDisposition *pDisposition = &processingDisposition;

   HWND hwndMainFrame = hwndParent;

#define SAVE_FILE   \
   pIPdfDocument -> Write(bstrResultsFile);

#include "savePDFFile.cpp"

   if ( ! ( -1L == deviceIndex ) ) {

      if ( ! keepImage || ! isFileSaved )

         DeleteFile(szTargetFile); 

      else {

         char szMoveImage[MAX_PATH];

         WideCharToMultiByte(CP_ACP,0,bstrResultsFile,-1,szMoveImage,MAX_PATH,0,0);

         char *p = strrchr(szMoveImage,'.');
         if ( ! p ) 
            p = szMoveImage + strlen(szMoveImage);

         sprintf(p,".jpg");

         DeleteFile(szMoveImage);
         CopyFile(szTargetFile,szMoveImage,FALSE);
         DeleteFile(szTargetFile);

      }

   }

   if ( ! isFileSaved ) {
      SysFreeString(bstrResultsFile);
      bstrResultsFile = NULL;
   }

   pIPdfDocument -> Release();
   pIPdfEnabler -> Release();

   return S_OK;
   }


   HRESULT __stdcall VideoBackEnd::put_PropertiesFileName(BSTR propertiesFileName) {
   pIGProperties -> put_FileName(propertiesFileName);
   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   //pIGProperties -> Save();
   return S_OK;
   }


   HRESULT __stdcall VideoBackEnd::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }

   HRESULT __stdcall VideoBackEnd::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvbeVideo");
   return S_OK;
   }


   HRESULT __stdcall VideoBackEnd::get_SavedDocumentName(BSTR *pDocumentName) {
   if ( ! bstrResultsFile )
      return E_FAIL;
   *pDocumentName = SysAllocString(bstrResultsFile);
   return S_OK;
   }

   HRESULT __stdcall VideoBackEnd::put_ParentWindow(HWND hp) {
   hwndParent = hp;
   return S_OK;
   }

   HRESULT __stdcall VideoBackEnd::put_CommandLine(BSTR theCommandLine) {
   return E_NOTIMPL;
   }

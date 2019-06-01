// Copyright 2017, 2018, 2019 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ImagingBackEnd.h"

#include <process.h>
#include <time.h>

   extern "C" int GetDocumentsLocation(HWND hwnd,char *);

   HBITMAP getBitmapHandle(HDC hdc,BSTR bstrFileName);

   HRESULT __stdcall ImagingBackEnd::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

   isProcessing = true;

   if ( ! skipImaging && 0 == twainSources.size() ) {

      TW_UINT16 ret = dsmEntryProcedure(&twainIdentity,0,DG_CONTROL,DAT_PARENT,MSG_OPENDSM,(TW_MEMREF)&hwndParent);

      TW_IDENTITY *pSource = new TW_IDENTITY;

      memset(pSource,0,sizeof(TW_IDENTITY));

      ret = dsmEntryProcedure(&twainIdentity,0,DG_CONTROL,DAT_IDENTITY,MSG_GETFIRST,(TW_MEMREF)pSource);

      while ( TWRC_SUCCESS == ret ) {
         twainSources.insert(twainSources.end(),pSource);
         pSource = new TW_IDENTITY;
         memset(pSource,0,sizeof(TW_IDENTITY));
         ret = dsmEntryProcedure(&twainIdentity,0,DG_CONTROL,DAT_IDENTITY,MSG_GETNEXT,(TW_MEMREF)pSource);
      }

      delete pSource;

      ret = dsmEntryProcedure(&twainIdentity,0,DG_CONTROL,DAT_PARENT,MSG_CLOSEDSM,(TW_MEMREF)&hwndParent);

   }

   if ( processingDisposition.doProperties || ! szChosenDevice[0] ) {

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

   if ( ! szChosenDevice[0] ) {
      char szMessage[512];
      sprintf(szMessage,"The imaging device has not been selected."
                        "\n\nSelect Retry to respecify the properties, or Cancel to exit without scanning an image");
      if ( IDRETRY == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL) )
         goto SetProperties;

      return E_FAIL;
   }

   HRESULT rc = CoCreateInstance(CLSID_PdfEnabler,NULL,CLSCTX_ALL,IID_IPdfEnabler,reinterpret_cast<void **>(&pIPdfEnabler));

   pIPdfEnabler -> Document(&pIPdfDocument);

   if ( ! ( S_OK == pIPdfDocument -> Open(bstrResultFileName,NULL,NULL) ) ) {

      pIPdfEnabler -> Release();
      pIPdfDocument -> Release();

      char szMessage[1024];
      char szDocument[MAX_PATH];
      WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szDocument,MAX_PATH,0,0);
      sprintf(szMessage,"The InnoVisioNate Image Scanning processing tool could not open the Document %s.\n\nThe Windows error code is: %ld\n\nPress Retry to specify the settings, or Cancel to exit",szDocument,GetLastError());
      if ( IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1 ) ) 
         return E_FAIL;

      goto SetProperties;

   }

   doScanFailureProperties = false;

   scanningCanceled = false;

   do {

#if 0
      getScans(reinterpret_cast<void *>(this));
#else
      unsigned int threadAddr;
      HANDLE hImageThread = (HANDLE)_beginthreadex(NULL,32768,getScans,(void *)this,CREATE_SUSPENDED,&threadAddr);

      ResumeThread(hImageThread);

//if ( hwndParent )
//ShowWindow(hwndParent,SW_HIDE);

      Sleep(1000);
      while ( ! ( NULL == hwndScanner ) )
         Sleep(250);

//if ( hwndParent )
//ShowWindow(hwndParent,SW_SHOW);
#endif

      if ( scanningCanceled ) {
         pIPdfDocument -> Release();
         pIPdfEnabler -> Release();
         return E_FAIL;
      }

      if ( doScanFailureProperties )
         goto SetProperties;

      if ( ! scanMultiple )
         break;

   } while ( IDOK == DialogBox(hModule,MAKEINTRESOURCE(IDD_SCAN_AGAIN),(HWND)hwndParent,(DLGPROC)reScanOptionHandler) ) ;

   std::list<BSTR> originalImageFiles;

   for ( std::list<BSTR>::iterator it = imageFiles.begin(); it != imageFiles.end(); it++ ) {

      IPdfPage *pIPdfPage = NULL;

      if ( ! selectPDFPage(&pIPdfPage) ) {

         pIPdfDocument -> Release();
         pIPdfEnabler -> Release();

         char szMessage[1024];
         sprintf(szMessage,"The page requested (%ld) does not exist in the document.\n\nPress Retry to specify the settings, or Cancel to exit",pageNumber);
         if ( IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1 ) ) 
            return E_FAIL;

         goto SetProperties;

      }

      if ( fitToPage )
         pIPdfPage -> AddCenteredImageFromFile(inchesLeft,inchesTop,(*it));
      else
         if ( keepAspectRatio )
            pIPdfPage -> AddSizedImageFromFile(inchesLeft,inchesTop,inchesWidth,-1.0,(*it));
         else
            pIPdfPage -> AddSizedImageFromFile(inchesLeft,inchesTop,inchesWidth,inchesHeight,(*it));

      pIPdfPage -> Release();

      if ( ! keepImage )
         DeleteFileW((*it));
      else 
         originalImageFiles.insert(originalImageFiles.end(),SysAllocString((*it)));

      SysFreeString((*it));

   }

   imageFiles.clear();

   bool isFileSaved = false;

   if ( ! processingDisposition.doSave ) {

      pIPdfDocument -> Write(bstrResultFileName);

      bstrResultsFile = SysAllocString(bstrResultFileName);

      isFileSaved = true;

   } else {

      char szResultFile[MAX_PATH];
      memset(szResultFile,0,sizeof(szResultFile));

      if ( isTempFile )
         WideCharToMultiByte(CP_ACP,0,bstrOriginalFile,-1,szResultFile,MAX_PATH,0,0);
      else
         WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szResultFile,MAX_PATH,0,0);

      char *px = strrchr(szResultFile,'.');
      if ( px )
         *px = '\0';

      bstrResultsFile = SysAllocStringLen(NULL,MAX_PATH);

      resultDisposition *pDisposition = &processingDisposition;

      HWND hwndMainFrame = hwndParent;

#define SAVE_FILE \
      pIPdfDocument -> Write(bstrResultsFile); 

#include "savePDFFile.cpp"

   }

   if ( isFileSaved ) {

      BSTR resultDirectory = SysAllocString(bstrResultsFile);

      BSTR resultBaseName = NULL;

      OLECHAR *p = wcsrchr(resultDirectory,'\\');
      if ( ! p )
         p = wcschr(resultDirectory,'/');
      if ( p ) {
         *p = '\0';
         resultBaseName = SysAllocString(p + 1);
      } else
         resultBaseName = SysAllocString(L"Image");

      p = wcsrchr(resultBaseName,'.');
      if ( p )
         *p = '\0';

      swprintf(resultBaseName,L"%s-Image",resultBaseName);

      long imageCount = 0L;

      for ( std::list<BSTR>::iterator it = originalImageFiles.begin(); it != originalImageFiles.end(); it++ ) {

         if ( keepImage ) {
            BSTR bstrImage = SysAllocStringLen(NULL,MAX_PATH);
            OLECHAR *originalExtension = wcsrchr((*it),'.');
            if ( ! originalExtension )
               originalExtension = L"";

            OLECHAR *p = wcsrchr((*it),'\\');
            if ( ! p )
               p = wcsrchr((*it),'/');
            if ( ! p )
               p = (*it) - 1;

            OLECHAR szExtension[] = {L"jpg"};
#if 0

            if ( fileTransfer ) {
               switch ( imageFormat ) {
               case TWFF_JFIF:
                  break;
               case TWFF_PNG:
                  swprintf(szExtension,L"png");
                  break;
               case TWFF_BMP:
                  swprintf(szExtension,L"bmp");
                  break;
               }
            }
#endif

            if ( 1 < originalImageFiles.size() )
               swprintf(bstrImage,L"%s\\%s%ld%s.%s",resultDirectory,resultBaseName,++imageCount,originalExtension,szExtension);
            else
               swprintf(bstrImage,L"%s\\%s%s.%s",resultDirectory,resultBaseName,originalExtension,szExtension);

            CopyFileW((*it),bstrImage,FALSE);

            SysFreeString(bstrImage);
         } 

      }

      SysFreeString(resultDirectory);
      SysFreeString(resultBaseName);

   }

   for ( std::list<BSTR>::iterator it = originalImageFiles.begin(); it != originalImageFiles.end(); it++ ) {
      DeleteFileW((*it));
      SysFreeString((*it));
   }

   originalImageFiles.clear();

   pIPdfDocument -> Release();
   pIPdfEnabler -> Release();

   return S_OK;
   }


   HRESULT __stdcall ImagingBackEnd::put_PropertiesFileName(BSTR propertiesFileName) {
   pIGProperties -> put_FileName(propertiesFileName);
   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   //pIGProperties -> Save();
   return S_OK;
   }


   HRESULT __stdcall ImagingBackEnd::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }

   HRESULT __stdcall ImagingBackEnd::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvbeTWAINImaging");
   return S_OK;
   }

   HRESULT __stdcall ImagingBackEnd::get_SavedDocumentName(BSTR *pDocumentName) {
   if ( ! bstrResultsFile )
      return E_FAIL;
   *pDocumentName = SysAllocString(bstrResultsFile);
   return S_OK;
   }

   HRESULT __stdcall ImagingBackEnd::put_ParentWindow(HWND hp) {
   hwndParent = hp;
   return S_OK;
   }

   HRESULT __stdcall ImagingBackEnd::put_CommandLine(BSTR theCommandLine) {
   return E_NOTIMPL;
   }


   HBITMAP getBitmapHandle(HDC hdc,BSTR bstrFileName) {

   FILE *fBitmap = _wfopen(bstrFileName,L"rb");

   BITMAPFILEHEADER bitmapFileHeader;

   if ( ! fread(&bitmapFileHeader,sizeof(BITMAPFILEHEADER),1,fBitmap) ) {
      fclose(fBitmap);
      return NULL;
   }

   long colorTableSize = bitmapFileHeader.bfOffBits - sizeof(BITMAPFILEHEADER);

   BYTE *buffer = new BYTE[colorTableSize];

   if ( ! fread(buffer,colorTableSize,1,fBitmap) ) {
      delete [] buffer;
      fclose(fBitmap);
      return NULL;
   }

   BITMAPINFO *pBitmapInfo = (BITMAPINFO *)buffer;

   BYTE *pBits = new BYTE[pBitmapInfo -> bmiHeader.biSizeImage];

   long rc = (long)fread(pBits,1,pBitmapInfo -> bmiHeader.biSizeImage,fBitmap);

   fclose(fBitmap);

   HBITMAP hBitmap = CreateDIBitmap(hdc,&pBitmapInfo->bmiHeader,CBM_INIT,pBits,pBitmapInfo,DIB_RGB_COLORS);

   delete [] buffer;
   delete [] pBits;

   return hBitmap;
   }



   bool ImagingBackEnd::selectPDFPage(IPdfPage **ppIPdfPage) {

   *ppIPdfPage = NULL;

   if ( specifyPage )

      pIPdfDocument -> Page(pageNumber,NULL,ppIPdfPage);

   else if ( lastPage )

      pIPdfDocument -> LastPage(ppIPdfPage);

   else

      pIPdfDocument -> AddPage(NULL,ppIPdfPage);

   if ( NULL == *ppIPdfPage ) 
      return false;

   return true;
   }


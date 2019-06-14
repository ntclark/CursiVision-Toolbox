// Copyright 2017, 2018, 2019 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ImagingBackEnd.h"

#include <process.h>
#include <time.h>

   extern "C" int GetDocumentsLocation(HWND hwnd,char *);

   HBITMAP getBitmapHandle(HDC hdc,BSTR bstrFileName);

   HRESULT __stdcall ImagingBackEnd::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

   isProcessing = true;

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

   ULONG countDevices = 0;

   BSTR deviceId = SysAllocStringLen(NULL,MAX_PATH);
   MultiByteToWideChar(CP_ACP,0,szDeviceId,-1,deviceId,MAX_PATH);

   if ( NULL == deviceId ) {
      char szMessage[1024];
      sprintf(szMessage,"The device associated with the name: %s\n\nWas not found on this system, is it disconnected ?\n\nPress Retry to specify the settings, or Cancel to exit",szChosenDevice);
      if ( IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1 ) )
         return E_FAIL;

      goto SetProperties;

   }

   HRESULT rc = CoCreateInstance(CLSID_PdfEnabler,NULL,CLSCTX_ALL,IID_IPdfEnabler,reinterpret_cast<void **>(&pIPdfEnabler));

   pIPdfEnabler -> Document(&pIPdfDocument);

   if ( ! ( S_OK == pIPdfDocument -> Open(bstrResultFileName,NULL,NULL) ) ) {

      pIPdfEnabler -> Release();
      pIPdfDocument -> Release();

      char szMessage[1024];
      char szDocument[MAX_PATH];
      WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szDocument,MAX_PATH,0,0);
      sprintf(szMessage,"The InnoVisioNate Video processing tool could not open the Document %s.\n\nThe Windows error code is: %ld\n\nPress Retry to specify the settings, or Cancel to exit",szDocument,GetLastError());
      if ( IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1 ) ) 
         return E_FAIL;

      goto SetProperties;

   }

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

   BSTR destinationFile = NULL;

   if ( keepImage ) {

      destinationFile = SysAllocString(bstrResultFileName);

      OLECHAR *p = wcsrchr(destinationFile,'.');
      if ( ! p ) 
         p = destinationFile + wcslen(destinationFile);

      wcscpy(p,L".bmp");

   } else {

      char szTemp[MAX_PATH];
      strcpy(szTemp,_tempnam(NULL,NULL));
      sprintf(szTemp + strlen(szTemp),".bmp");

      destinationFile = SysAllocStringLen(NULL,MAX_PATH);
      MultiByteToWideChar(CP_ACP,0,szTemp,-1,destinationFile,MAX_PATH);

   }

   DeleteFileW(destinationFile);

   std::list<BSTR> imageFiles;
   std::list<BSTR> originalImageFiles;

#if (_WIN32_WINNT >= 0x0600) // Windows Vista and later

   IWiaItem2 *pIWiaItem = NULL;

   if ( ! ( S_OK == pIWiaDevMgr -> CreateDevice(0,deviceId,&pIWiaItem) ) ) {

      pIPdfDocument -> Release();
      pIPdfEnabler -> Release();

      char szMessage[1024];
      sprintf(szMessage,"The device associated with the name: %s\n\nWas not found on this system, is it disconnected ?\n\nPress Retry to specify the settings, or Cancel to exit",szChosenDevice);
      if ( IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1 ) )
         return E_FAIL;

      goto SetProperties;

   }

   BSTR destinationFolder = SysAllocString(_wtempnam(NULL,NULL));
   BSTR baseName = NULL;
   
   OLECHAR *p = wcsrchr(destinationFolder,'\\');
   if ( ! p )
      p = wcschr(destinationFolder,'/');
   if ( p ) {
      *p = '\0';
      baseName = SysAllocString(p + 1);
   } else 
      baseName = SysAllocString(destinationFolder);

   long countFiles = 0;

   BSTR *resultFiles = new BSTR[0];

   IWiaItem2 *ppIWiaItemResult = NULL;

// FUCK MICROSOFT
   pIWiaItem -> DeviceDlg(WIA_DEVICE_DIALOG_USE_COMMON_UI | WIA_DEVICE_DIALOG_SINGLE_IMAGE,
                              hwndParent,destinationFolder,baseName,&countFiles,(BSTR **)&resultFiles,&ppIWiaItemResult);

   SysFreeString(destinationFolder);
   SysFreeString(baseName);

   if ( 0 == countFiles )
      return S_OK;

   IWiaTransfer *pIWiaTransfer = NULL;

   for ( long k = 0; k < countFiles; k++ ) {

      long itemType = 0L;

// FUCK MICROSOFT
      ppIWiaItemResult[0].GetItemType(&itemType);

      if ( 0 == itemType )
         continue;

      if ( ! ( itemType & WiaItemTypeFile ) )
         continue;

      originalImageFiles.insert(originalImageFiles.end(),SysAllocString(resultFiles[k]));

      PROPSPEC propSpecs[2] = {0};

      propSpecs[0].ulKind = PRSPEC_PROPID;
      propSpecs[0].propid = WIA_IPA_FORMAT;
      propSpecs[1].ulKind = PRSPEC_PROPID;
      propSpecs[1].propid = WIA_IPA_TYMED;

      PROPVARIANT theProperties[2] = {0};

      theProperties[0].vt = VT_CLSID;
      theProperties[0].puuid = (CLSID *)&WiaImgFmt_JPEG;
      theProperties[1].vt = VT_I4;
      theProperties[1].lVal = TYMED_FILE;

      IWiaPropertyStorage* pIWiaPropertyStorage = NULL;
// FUCK MICROSOFT
      rc = ppIWiaItemResult[0].QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropertyStorage);
      rc = pIWiaPropertyStorage -> WriteMultiple(2,propSpecs,theProperties,WIA_IPA_FIRST);
      pIWiaPropertyStorage -> Release();

      BSTR newImageFile = SysAllocString(_wtempnam(NULL,NULL));

      pIWiaTransferCallback = new _IWiaTransferCallback(this,newImageFile);

// FUCK MICROSOFT
      rc = ppIWiaItemResult[0].QueryInterface(IID_IWiaTransfer,reinterpret_cast<void **>(&pIWiaTransfer));

// FUCK MICROSOFT
      rc = pIWiaTransfer -> Download(0L,static_cast<IWiaTransferCallback *>(pIWiaTransferCallback));

      imageFiles.insert(imageFiles.end(),SysAllocString(newImageFile));

      SysFreeString(newImageFile);

      delete pIWiaTransferCallback;

      pIWiaTransferCallback = NULL;

      pIWiaTransfer -> Release();

   }

//
// FUCK MICROSOFT
//
   ppIWiaItemResult[0].Release();
   for ( long k = 0; k < countFiles; k++ ) {
// FUCK MICROSOFT
      //ppIWiaItemResult[k].Release();
      SysFreeString(resultFiles[k]);
   }

//   pIWiaItem -> Release();

//   CoTaskMemFree(ppIWiaItemResult);

#else

   GUID guidFormat = IID_NULL;
   IWiaItem *pIWiaItem = NULL;

   if ( ! ( S_OK == pIWiaDevMgr -> CreateDevice(deviceId,&pIWiaItem) ) ) {

      pIPdfDocument -> Release();
      pIPdfEnabler -> Release();

      char szMessage[1024];
      sprintf(szMessage,"The device associated with the name: %s\n\nWas not found on this system, is it disconnected ?\n\nPress Retry to specify the settings, or Cancel to exit",szChosenDevice);
      if ( IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1 ) )
         return E_FAIL;

      goto SetProperties;
   }

   pIWiaDevMgr -> GetImageDlg(hwndParent,StiDeviceTypeDefault,WIA_DEVICE_DIALOG_USE_COMMON_UI,WIA_INTENT_IMAGE_TYPE_COLOR,pIWiaItem,destinationFile,&guidFormat);
   pIWiaItem -> Release();

   FILE *fX = _wfopen(destinationFile,L"rb");
   if ( ! fX ) {
      pIWiaItem -> Release();
      pIPdfDocument -> Release();
      pIPdfEnabler -> Release();
      return S_OK;
   }
   fclose(fX);

   imageFiles.insert(imageFiles.end(),destinationFile);
   originalImageFiles.insert(originalImageFiles.end(),destinationFile);

#endif

   for ( std::list<BSTR>::iterator it = imageFiles.begin(); it != imageFiles.end(); it++ ) {
      if ( fitToPage )
         pIPdfPage -> AddCenteredImageFromFile(inchesLeft,inchesTop,(*it));
      else
         if ( keepAspectRatio )
            pIPdfPage -> AddSizedImageFromFile(inchesLeft,inchesTop,inchesWidth,-1.0,(*it));
         else
            pIPdfPage -> AddSizedImageFromFile(inchesLeft,inchesTop,inchesWidth,inchesHeight,(*it));
      DeleteFileW((*it));
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

            if ( 1 < originalImageFiles.size() )
               swprintf(bstrImage,L"%s\\%s%ld%s",resultDirectory,resultBaseName,++imageCount,originalExtension);
            else
               swprintf(bstrImage,L"%s\\%s%s",resultDirectory,resultBaseName,originalExtension);

            CopyFileW((*it),bstrImage,FALSE);

            SysFreeString(bstrImage);
         } 

         DeleteFileW((*it));
         SysFreeString((*it));
      }

      SysFreeString(resultDirectory);
      SysFreeString(resultBaseName);

   } else {

      for ( std::list<BSTR>::iterator it = originalImageFiles.begin(); it != originalImageFiles.end(); it++ ) {
         DeleteFileW((*it));
         SysFreeString((*it));
      }

      originalImageFiles.clear();

   }

   pIPdfPage -> Release();
   pIPdfDocument -> Release();
   pIPdfEnabler -> Release();

   return S_OK;
   }


   HRESULT __stdcall ImagingBackEnd::put_PropertiesFileName(BSTR propertiesFileName) {
   pIGProperties -> put_FileName(propertiesFileName);
   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   pIGProperties -> Save();
   return S_OK;
   }


   HRESULT __stdcall ImagingBackEnd::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }

   HRESULT __stdcall ImagingBackEnd::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvbeImaging");
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


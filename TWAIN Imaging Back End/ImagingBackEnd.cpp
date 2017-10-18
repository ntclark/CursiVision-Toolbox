// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ImagingBackEnd.h"

#include <gdiplus.h>

   ImagingBackEnd::ImagingBackEnd(IUnknown *pIUnknownOuter) :

   pIGProperties(NULL),
   pIGPropertiesClient(NULL),
   pIGPropertyPageClient(NULL),

   pICursiVisionServices(NULL),

   hwndProperties(NULL),
   hwndParent(NULL),

   hModule_TWAIN(NULL),

   startParameters(0),
   endParameters(0),

   doExecute(true),
   isProcessing(false),
   fileTransfer(false),

   imageFormat(-1L),

   skipImaging(false),
   scanOne(true),
   scanMultiple(false),

   pIPdfEnabler(NULL),
   pIPdfDocument(NULL),

   inchesLeft(0.0),
   inchesTop(0.0),
   inchesWidth(0.0),
   inchesHeight(0.0),
   fitToPage(true),
   keepAspectRatio(true),

   pageNumber(0),
   specifyPage(false),
   lastPage(false),
   newLastPage(true),

   bstrResultsFile(NULL),

   hwndScanner(NULL),
   pCurrentSource(NULL),

   doScanFailureProperties(false),
   scannerStarted(false),
   scanningCanceled(false),

   refCount(0)

   {

   memset(szChosenDevice,0,sizeof(szChosenDevice));

   pIGPropertyPageClient = new _IGPropertyPageClient(this);

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

   pIGProperties -> Add(L"scanning parameters",NULL);

   pIGProperties -> DirectAccess(L"scanning parameters",TYPE_BINARY,&startParameters,offsetof(ImagingBackEnd,endParameters) - offsetof(ImagingBackEnd,startParameters));

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

   memset(&twainIdentity,0,sizeof(TW_IDENTITY));

   twainIdentity.Id = 0L;
   twainIdentity.Version.MajorNum = 1;
   twainIdentity.Version.MinorNum = 1;
   twainIdentity.Version.Country = TWCY_USA;
   twainIdentity.Version.Language = TWLG_ENGLISH_USA;
   twainIdentity.ProtocolMajor = TWON_PROTOCOLMAJOR;
   twainIdentity.ProtocolMinor = TWON_PROTOCOLMINOR;
   twainIdentity.SupportedGroups = DF_APP2 | DG_IMAGE | DG_CONTROL | DF_DSM2;

   strcpy(twainIdentity.Version.Info,"1.0.0");
   strcpy(twainIdentity.Manufacturer,"InnoVisioNate Inc.");
   strcpy(twainIdentity.ProductFamily,"CursiVision");
   strcpy(twainIdentity.ProductName,"CursiVision");

   hModule_TWAIN = LoadLibrary("twain_32.dll");

   dsmEntryProcedure = (DSMENTRYPROC)GetProcAddress(hModule_TWAIN,"DSM_Entry");

   return;
   }


   ImagingBackEnd::~ImagingBackEnd() {

   IPrintingSupportProfile *px = NULL;
   pICursiVisionServices -> get_PrintingSupportProfile(&px);
   if ( pICursiVisionServices -> IsAdministrator() || ! px )
      pIGProperties -> Save();

   for ( std::list<TW_IDENTITY *>::iterator it = twainSources.begin(); it != twainSources.end(); it++ )
      delete (*it);

   twainSources.clear();

   if ( dsmEntryProcedure )
      dsmEntryProcedure(&twainIdentity,0,DG_CONTROL,DAT_PARENT,MSG_CLOSEDSM,(TW_MEMREF)NULL);

   if ( hModule_TWAIN )
      FreeLibrary(hModule_TWAIN);

   dsmEntryProcedure = NULL;
   hModule_TWAIN = NULL;

   for ( std::list<BSTR>::iterator it = imageFiles.begin(); it != imageFiles.end(); it++ )
      SysFreeString((*it));

   imageFiles.clear();

   return;
   }

   HRESULT ImagingBackEnd::PushProperties() {
   return pIGProperties -> Push();
   }

   HRESULT ImagingBackEnd::PopProperties() {
   return pIGProperties -> Pop();
   }

   HRESULT ImagingBackEnd::DiscardProperties() {
   return pIGProperties -> Discard();
   }

   HRESULT ImagingBackEnd::SaveProperties() {
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


   void SaveJPEG(char *pszBitmapFileName) {

   char szHold[MAX_PATH];

   strcpy(szHold,_tempnam(NULL,NULL));
   MoveFile(pszBitmapFileName,szHold);

   ULONG_PTR gdiplusToken;
   Gdiplus::GdiplusStartupInput gdiplusStartupInput = 0L;

   Gdiplus::GdiplusStartup(&gdiplusToken,&gdiplusStartupInput,NULL);
   
   BSTR bstrFileName = SysAllocStringLen(NULL,MAX_PATH);
   MultiByteToWideChar(CP_ACP,0,szHold,-1,bstrFileName,MAX_PATH);

   Gdiplus::Image *pImage = new Gdiplus::Bitmap(bstrFileName);

   UINT countEncoders;
   UINT arraySize;
   Gdiplus::ImageCodecInfo *pEncoders;
   Gdiplus::ImageCodecInfo *pTheEncoder = NULL;

   Gdiplus::GetImageEncodersSize(&countEncoders,&arraySize);

   pEncoders = (Gdiplus::ImageCodecInfo *)new BYTE[arraySize];

   Gdiplus::GetImageEncoders(countEncoders,arraySize,pEncoders);

   for ( UINT k = 0; k < countEncoders; k++ ) {
      if ( _wcsicmp(pEncoders[k].MimeType,L"image/jpeg") )
         continue;
      pTheEncoder = &pEncoders[k];
      break;
   }


   SysFreeString(bstrFileName);

   bstrFileName = SysAllocStringLen(NULL,MAX_PATH);
   MultiByteToWideChar(CP_ACP,0,pszBitmapFileName,-1,bstrFileName,MAX_PATH);

   Gdiplus::Status fuckMS = pImage -> Save(bstrFileName,&pTheEncoder -> Clsid,NULL);

   SysFreeString(bstrFileName);

   delete [] pEncoders;

   delete pImage;

   SysFreeString(bstrFileName);

   Gdiplus::GdiplusShutdown(gdiplusToken);

   return;
   }


   void SaveJPEG(TW_IMAGEINFO *pImageInfo,HBITMAP hDIBBitmap,char *pszBitmapFileName) {

   SaveBitmapFile(pImageInfo,hDIBBitmap,pszBitmapFileName);

   char szHold[MAX_PATH];

   strcpy(szHold,_tempnam(NULL,NULL));

   MoveFile(pszBitmapFileName,szHold);

   ULONG_PTR gdiplusToken;
   Gdiplus::GdiplusStartupInput gdiplusStartupInput = 0L;

   Gdiplus::GdiplusStartup(&gdiplusToken,&gdiplusStartupInput,NULL);
   
   BSTR bstrFileName = SysAllocStringLen(NULL,MAX_PATH);
   MultiByteToWideChar(CP_ACP,0,szHold,-1,bstrFileName,MAX_PATH);

   Gdiplus::Image *pImage = new Gdiplus::Bitmap(bstrFileName);

   UINT countEncoders;
   UINT arraySize;
   Gdiplus::ImageCodecInfo *pEncoders;
   Gdiplus::ImageCodecInfo *pTheEncoder = NULL;

   Gdiplus::GetImageEncodersSize(&countEncoders,&arraySize);

   pEncoders = (Gdiplus::ImageCodecInfo *)new BYTE[arraySize];

   Gdiplus::GetImageEncoders(countEncoders,arraySize,pEncoders);

   for ( UINT k = 0; k < countEncoders; k++ ) {
      if ( _wcsicmp(pEncoders[k].MimeType,L"image/jpeg") )
         continue;
      pTheEncoder = &pEncoders[k];
      break;
   }

   SysFreeString(bstrFileName);

   bstrFileName = SysAllocStringLen(NULL,MAX_PATH);
   MultiByteToWideChar(CP_ACP,0,pszBitmapFileName,-1,bstrFileName,MAX_PATH);

   Gdiplus::Status fuckMS = pImage -> Save(bstrFileName,&pTheEncoder -> Clsid,NULL);

   SysFreeString(bstrFileName);

   delete [] pEncoders;

   delete pImage;

   SysFreeString(bstrFileName);

   Gdiplus::GdiplusShutdown(gdiplusToken);

   return;
   }


   void SaveBitmapFile(TW_IMAGEINFO *pImageInfo,HBITMAP hDIBBitmap,char *pszBitmapFileName) {

   BYTE *pBits = (BYTE *)GlobalLock(hDIBBitmap);

   BITMAPINFOHEADER *pBitmapInfo = (BITMAPINFOHEADER *)pBits;

   long colorCount = 0L;

   if ( 24 != pBitmapInfo -> biBitCount )
      colorCount = 1 << pBitmapInfo -> biBitCount;

   long colorTableSize = colorCount * sizeof(RGBQUAD);

   BITMAPFILEHEADER bitMapFileHeader = {0};

   bitMapFileHeader.bfType = 0x4d42;
   bitMapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTableSize;
   bitMapFileHeader.bfSize = bitMapFileHeader.bfOffBits + pBitmapInfo -> biSizeImage;

   pBits += sizeof(BITMAPINFOHEADER);

   FILE *fBitmap = fopen(pszBitmapFileName,"wb");

   fwrite(&bitMapFileHeader,sizeof(BITMAPFILEHEADER),1,fBitmap);
   fwrite(pBitmapInfo,sizeof(BITMAPINFOHEADER),1,fBitmap);
   fwrite(pBits,colorTableSize + pBitmapInfo -> biSizeImage,1,fBitmap);

   fclose(fBitmap);

   GlobalUnlock(hDIBBitmap);

   return;
   }   

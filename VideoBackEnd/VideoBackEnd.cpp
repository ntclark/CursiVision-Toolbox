// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "VideoBackEnd.h"

#include <gdiplus.h>
#include <time.h>

   IBaseFilter *pIBaseFilter = NULL;
   ICaptureGraphBuilder2 *pICaptureGraphBuilder = NULL;
   IGraphBuilder *pIGraphBuilder = NULL;
   IBaseFilter *pIVideoMixingRenderer = NULL;
   IVMRWindowlessControl9 *pIVMRWindowlessControl;
   IMediaControl *pIMediaControl = NULL;

   WNDPROC VideoBackEnd::defaultImageHandler = NULL;

   VideoBackEnd::VideoBackEnd(IUnknown *pIUnknownOuter) :

   pIGProperties(NULL),
   pIGPropertiesClient(NULL),
   pIGPropertyPageClient(NULL),

   pICursiVisionServices(NULL),

   pVisioLoggerVideoBackEnd(NULL),

   hwndProperties(NULL),
   hwndParent(NULL),

   doExecute(true),

   startParameters(0),
   endParameters(0),

   autoSnap(false),
   timeStamp(false),
   skipImaging(false),
   keepImage(false),
   useAnyCamera(true),
   ignoreNoCamera(false),
   saveDocumentAnyway(true),
   includeComputerName(false),

   cameraCount(0L),
   pCameraNames(NULL),

   isProcessing(false),

   processingSemaphore(NULL),

   pIPdfEnabler(NULL),
   pIPdfDocument(NULL),

   inchesLeft(1.0),
   inchesTop(1.0),
   inchesWidth(0.0),
   inchesHeight(0.0),
   fitToPage(true),
   keepAspectRatio(true),

   pageNumber(1L),
   specifyPage(false),
   lastPage(false),
   newLastPage(true),

   autoFocusDelay(1000L),

   bstrResultsFile(NULL),

   refCount(0)

   {

   memset(szwChosenDevice,0,sizeof(szwChosenDevice));
   memset(szTargetFile,0,sizeof(szTargetFile));

   pIGPropertyPageClient = new _IGPropertyPageClient(this);

   long sizeParameters = offsetof(VideoBackEnd,endParameters) - offsetof(VideoBackEnd,startParameters);

   pageNumber = 1L;
   newLastPage = true;
   inchesLeft = 1.0;
   inchesTop = 1.0;
   autoFocusDelay = 1000L;

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

   pIGProperties -> Add(L"video parameters",NULL);

   pIGProperties -> DirectAccess(L"video parameters",TYPE_BINARY,&startParameters,sizeParameters);

   pVisioLoggerVideoBackEnd = new VisioLoggerVideoBackEnd(this);

   pIGProperties -> Add(L"visiologger video parameters",NULL);

   pIGProperties -> DirectAccess(L"visiologger video parameters",TYPE_BINARY,pVisioLoggerVideoBackEnd -> settings(),pVisioLoggerVideoBackEnd -> settingsSize());

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

   ICreateDevEnum *pICreateDevEnum = NULL;
   IEnumMoniker *pIEnumMoniker = NULL;

   HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum,NULL,CLSCTX_INPROC_SERVER,IID_ICreateDevEnum,reinterpret_cast<void**>(&pICreateDevEnum));

   hr = pICreateDevEnum -> CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pIEnumMoniker,0);

   if ( ! pIEnumMoniker ) {
      pICreateDevEnum -> Release();
      return;
   }

   cameraCount = 0L;
   IMoniker *pIMoniker = NULL;

   while ( S_OK == pIEnumMoniker -> Next(1, &pIMoniker, NULL) ) {
      cameraCount++;
      pIMoniker -> Release();
   }

   pIEnumMoniker -> Reset();

   pCameraNames = new BSTR[cameraCount];

   for ( long k = 0; k < (long)cameraCount; k++ ) {

      pIEnumMoniker -> Next(1, &pIMoniker, NULL);

      IPropertyBag *pIPropertyBag;

      hr = pIMoniker -> BindToStorage(0,0,IID_IPropertyBag,reinterpret_cast<void **>(&pIPropertyBag));

      VARIANT varName;

      VariantInit(&varName);
      hr = pIPropertyBag -> Read(L"FriendlyName", &varName, 0);
      pCameraNames[k] = SysAllocString(varName.bstrVal);

      pIPropertyBag -> Release();

      pIMoniker -> Release();

   }

   pIEnumMoniker -> Release();
   pICreateDevEnum -> Release();

   return;
   }


   VideoBackEnd::~VideoBackEnd() {

   pIGProperties -> Save();

   unHostVideo();

   IPrintingSupportProfile *px = NULL;

   if ( pICursiVisionServices ) {
      pICursiVisionServices -> get_PrintingSupportProfile(&px);
      if ( pICursiVisionServices -> IsAdministrator() || ! px )
         pIGProperties -> Save();
   }

   if ( pCameraNames ) {
      for ( long k = 0; k < (long)cameraCount; k++ )
         SysFreeString(pCameraNames[k]);
      delete [] pCameraNames;
   }


   if ( bstrResultsFile )
      SysFreeString(bstrResultsFile);

   delete pVisioLoggerVideoBackEnd;

   return;
   }


   bool VideoBackEnd::findSolitaryCamera(char *pszDeviceName) {

   ICreateDevEnum *pICreateDevEnum = NULL;
   IEnumMoniker *pIEnumMoniker = NULL;

   HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum,NULL,CLSCTX_INPROC_SERVER,IID_ICreateDevEnum,reinterpret_cast<void**>(&pICreateDevEnum));

   hr = pICreateDevEnum -> CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pIEnumMoniker,0);

   if ( ! pIEnumMoniker ) {
      pICreateDevEnum -> Release();
      return false;
   }

   cameraCount = 0L;

   IMoniker *pIMoniker = NULL;
   while ( S_OK == pIEnumMoniker -> Next(1, &pIMoniker, NULL) ) {
      cameraCount++;
      pIMoniker -> Release();
   }

   if ( ! ( 1 == cameraCount ) ) 
      return false;

   pIEnumMoniker -> Reset();

   pIEnumMoniker -> Next(1, &pIMoniker, NULL);

   IPropertyBag *pIPropertyBag;

   hr = pIMoniker -> BindToStorage(0,0,IID_IPropertyBag,reinterpret_cast<void **>(&pIPropertyBag));

   VARIANT varName;
   VariantInit(&varName);

   hr = pIPropertyBag -> Read(L"FriendlyName", &varName, 0);

   WideCharToMultiByte(CP_ACP,0,varName.bstrVal,-1,pszDeviceName,MAX_PATH,0,0);

   pIPropertyBag -> Release();

   pIMoniker -> Release();

   pIEnumMoniker -> Release();

   pICreateDevEnum -> Release();

   return true;
   }




   HRESULT VideoBackEnd::PushProperties() {
   return pIGProperties -> Push();
   }

   HRESULT VideoBackEnd::PopProperties() {
   return pIGProperties -> Pop();
   }

   HRESULT VideoBackEnd::DiscardProperties() {
   return pIGProperties -> Discard();
   }

   HRESULT VideoBackEnd::SaveProperties() {
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

   
   void SaveBitmapFile(BYTE *pImageData,char *pszBitmapFileName) {
   BITMAPFILEHEADER bitmapFileHeader = {0};
   BITMAPINFOHEADER *pBitmapInfoHeader = (BITMAPINFOHEADER *)pImageData;
   bitmapFileHeader.bfType = 0x4d42;
   bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
   bitmapFileHeader.bfSize = bitmapFileHeader.bfOffBits + pBitmapInfoHeader -> biSizeImage;
   FILE *fImage = fopen(pszBitmapFileName,"wb");
   fwrite(&bitmapFileHeader,sizeof(BITMAPFILEHEADER),1,fImage);
   fwrite(pImageData,sizeof(BITMAPINFOHEADER) + pBitmapInfoHeader -> biSizeImage,1,fImage);
   fclose(fImage);
   return;
   }


   void TimeStampBitmap(BYTE *pImageData,char *pszTargetFile,bool doTimeStamp,bool doComputerName) {

   time_t currentTime;
   struct tm *ptmCurrentTime = NULL;
   OLECHAR szTimeString[256];

   time(&currentTime);
   ptmCurrentTime = localtime(&currentTime);

   wcscpy(szTimeString,_wasctime(ptmCurrentTime));

   OLECHAR szComputerName[MAX_PATH];
   DWORD cb = MAX_PATH;
   GetComputerNameW(szComputerName,&cb);

   if ( doTimeStamp && doComputerName )
      swprintf(szTimeString + wcslen(szTimeString) - 1,L" on %s     ",szComputerName);
   else if ( doComputerName ) 
      wcscpy(szTimeString,szComputerName);

//MessageBoxW(NULL,szTimeString,L"",MB_OK);
   char szTempBitmap[MAX_PATH];
   strcpy(szTempBitmap,_tempnam(NULL,NULL));
   SaveBitmapFile(pImageData,szTempBitmap);

   ULONG_PTR gdiplusToken;
   Gdiplus::GdiplusStartupInput gdiplusStartupInput = 0L;
   Gdiplus::GdiplusStartup(&gdiplusToken,&gdiplusStartupInput,NULL);

   BSTR bstrFileName = SysAllocStringLen(NULL,MAX_PATH);
   MultiByteToWideChar(CP_ACP,0,szTempBitmap,-1,bstrFileName,MAX_PATH);

   Gdiplus::Image *pImageSource = new Gdiplus::Image(bstrFileName);
   Gdiplus::Graphics *pImageGraphics = new Gdiplus::Graphics(pImageSource);

   SysFreeString(bstrFileName);

   Gdiplus::REAL fontSize = 1.0f;
   Gdiplus::RectF rcText;
   Gdiplus::RectF rcTextBoundingBox;

   rcText.X = (Gdiplus::REAL)pImageSource -> GetWidth() / 2.0f;
   rcText.Y = (Gdiplus::REAL)pImageSource -> GetHeight() - 32.0f;
   rcText.Width = (Gdiplus::REAL)pImageSource -> GetWidth() / 2.0f;
   rcText.Height = 32.0f;

   Gdiplus::StringFormat *pStringFormat = new Gdiplus::StringFormat();

   pStringFormat -> SetAlignment(Gdiplus::StringAlignmentCenter);

   Gdiplus::Font *pFont = new Gdiplus::Font(L"Courier",fontSize);

   pImageGraphics -> MeasureString(szTimeString,-1,pFont,rcText,pStringFormat,&rcTextBoundingBox);

   if ( rcTextBoundingBox.Width < 0.90 * rcText.Width ) do {
      pImageGraphics -> MeasureString(szTimeString,-1,pFont,rcText,pStringFormat,&rcTextBoundingBox);
      fontSize += 1.0f;
      delete pFont;
      pFont = new Gdiplus::Font(L"Courier",fontSize);
   } while ( rcTextBoundingBox.Width < 0.90 * rcText.Width );

   rcText.Y = pImageSource -> GetHeight() - rcTextBoundingBox.Height;
   rcText.Height = rcTextBoundingBox.Height;

   Gdiplus::SolidBrush *pBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255,255,255));

   pImageGraphics -> DrawString(szTimeString,-1,pFont,rcText,pStringFormat,pBrush);

   delete pFont;
   delete pStringFormat;
   delete pBrush;
   delete pImageGraphics;

   UINT countEncoders;
   UINT arraySize;
   Gdiplus::ImageCodecInfo *pEncoders;
   Gdiplus::ImageCodecInfo *pTheEncoder = NULL;

   Gdiplus::GetImageEncodersSize(&countEncoders,&arraySize);

   pEncoders = (Gdiplus::ImageCodecInfo *)new BYTE[arraySize];

   Gdiplus::GetImageEncoders(countEncoders,arraySize,pEncoders);

   for ( UINT k = 0; k < countEncoders; k++ ) {
      if ( _wcsicmp(pEncoders[k].MimeType,L"image/bmp") )
         continue;
      pTheEncoder = &pEncoders[k];
      break;
   }

   bstrFileName = SysAllocStringLen(NULL,MAX_PATH);

   MultiByteToWideChar(CP_ACP,0,pszTargetFile,-1,bstrFileName,MAX_PATH);

   pImageSource -> Save(bstrFileName,&pTheEncoder -> Clsid,NULL);

   SysFreeString(bstrFileName);

   delete [] pEncoders;

   delete pImageSource;

   DeleteFile(szTempBitmap);

   Gdiplus::GdiplusShutdown(gdiplusToken);

   return;
   }


   void SaveJPEG(BYTE *pImageData,char *pszBitmapFileName) {

   SaveBitmapFile(pImageData,pszBitmapFileName);

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

   Gdiplus::Status fms = pImage -> Save(bstrFileName,&pTheEncoder -> Clsid,NULL);

   SysFreeString(bstrFileName);

   delete [] pEncoders;

   delete pImage;

   Gdiplus::GdiplusShutdown(gdiplusToken);

   return;
   }


   bool VideoBackEnd::hostVideo(HWND hwndHost) {

   ICreateDevEnum *pICreateDevEnum = NULL;
   IEnumMoniker *pIEnumMoniker = NULL;
   IMoniker *pIMoniker = NULL;

   HRESULT hr;

   CoCreateInstance(CLSID_SystemDeviceEnum,NULL,CLSCTX_INPROC_SERVER,IID_ICreateDevEnum,reinterpret_cast<void**>(&pICreateDevEnum));

   pICreateDevEnum -> CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pIEnumMoniker,0);

   if ( ! pIEnumMoniker ) {
      OLECHAR szwTemp[1024];
      swprintf(szwTemp,L"The video device: %ls is not found or is disconnected",szwChosenDevice);
      MessageBoxW(NULL,szwTemp,L"Error",MB_ICONEXCLAMATION | MB_TOPMOST);
      return false;
   }


   while ( S_OK == pIEnumMoniker -> Next(1, &pIMoniker, NULL) ) {

      IPropertyBag *pIPropertyBag;

      hr = pIMoniker -> BindToStorage(0,0,IID_IPropertyBag,reinterpret_cast<void **>(&pIPropertyBag));

      VARIANT varName;
      VariantInit(&varName);
      hr = pIPropertyBag -> Read(L"FriendlyName", &varName, 0);

      pIPropertyBag -> Release();

      if ( 0 == wcscmp(varName.bstrVal,szwChosenDevice) ) {
         hr = pIMoniker -> BindToObject(0,0,IID_IBaseFilter,reinterpret_cast<void **>(&pIBaseFilter));
         pIMoniker -> Release();
         break;
      }

      pIMoniker -> Release();

   }

   pIEnumMoniker -> Release();
   pICreateDevEnum -> Release();

   if ( ! pIBaseFilter ) {
      OLECHAR szwTemp[1024];
      swprintf(szwTemp,L"The video device: %ls is not found or is disconnected",szwChosenDevice);
      MessageBoxW(NULL,szwTemp,L"Error",MB_ICONEXCLAMATION | MB_TOPMOST);
      return false;
   }

   hr = CoCreateInstance(CLSID_CaptureGraphBuilder2,NULL,CLSCTX_INPROC_SERVER,IID_ICaptureGraphBuilder2,reinterpret_cast<void **>(&pICaptureGraphBuilder));

   hr = CoCreateInstance(CLSID_FilterGraph,0,CLSCTX_INPROC_SERVER,IID_IGraphBuilder,reinterpret_cast<void **>(&pIGraphBuilder));

   hr = pICaptureGraphBuilder -> SetFiltergraph(pIGraphBuilder);

   hr = pIGraphBuilder -> AddFilter(pIBaseFilter, L"Preview Renderer");

   hr = CoCreateInstance(CLSID_VideoMixingRenderer9,NULL,CLSCTX_INPROC,IID_IBaseFilter,(void**)&pIVideoMixingRenderer);

   hr = pIGraphBuilder -> AddFilter(pIVideoMixingRenderer, L"Video Mixing Renderer");

   {
   IVMRFilterConfig *pConfig = NULL;
   hr = pIVideoMixingRenderer -> QueryInterface(IID_IVMRFilterConfig9,(void **)&pConfig);
   pConfig -> SetRenderingMode(VMRMode_Windowless);
   pConfig -> Release();
   }

   pIVideoMixingRenderer -> QueryInterface(IID_IVMRWindowlessControl9,(void **)&pIVMRWindowlessControl);

   pIVMRWindowlessControl -> SetVideoClippingWindow(hwndHost);

   RECT rcClient;

   GetClientRect(hwndHost,&rcClient);

   pIVMRWindowlessControl -> SetVideoPosition(NULL,&rcClient);

   pICaptureGraphBuilder -> RenderStream(&PIN_CATEGORY_PREVIEW,&MEDIATYPE_Video,pIBaseFilter,NULL,pIVideoMixingRenderer);

   pIGraphBuilder -> QueryInterface(IID_IMediaControl,reinterpret_cast<void **>(&pIMediaControl));

   pIMediaControl -> Run();

   return true;
   }


   void VideoBackEnd::unHostVideo() {

   if ( ! pIMediaControl )
      return;

   HRESULT hr = pIMediaControl -> Stop();

   pIMediaControl -> Release();
   pIMediaControl = NULL;

   pIVMRWindowlessControl -> Release();
   pIVMRWindowlessControl = NULL;

   pIVideoMixingRenderer -> Release();
   pIVideoMixingRenderer = NULL;

   pIGraphBuilder -> Release();
   pIGraphBuilder = NULL;

   pICaptureGraphBuilder -> Release();
   pICaptureGraphBuilder = NULL;
   
   pIBaseFilter -> Release();
   pIBaseFilter = NULL;

   return;
   }
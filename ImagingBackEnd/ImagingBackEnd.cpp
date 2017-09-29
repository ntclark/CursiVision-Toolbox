
#include "ImagingBackEnd.h"

   ImagingBackEnd::ImagingBackEnd(IUnknown *pIUnknownOuter) :

   pIGProperties(NULL),
   pIGPropertiesClient(NULL),
   pIGPropertyPageClient(NULL),

   pICursiVisionServices(NULL),

   hwndProperties(NULL),
   hwndParent(NULL),

   doExecute(true),
   isProcessing(false),

   startParameters(0),
   endParameters(0),

   skipImaging(false),
   keepImage(true),

   pIWiaDevMgr(NULL),

#if (_WIN32_WINNT >= 0x0600) // Windows Vista and later
   pIWiaTransferCallback(NULL),
#endif

   pIPdfEnabler(NULL),
   pIPdfDocument(NULL),

   inchesLeft(0.0),
   inchesTop(0.0),
   inchesWidth(0.0),
   inchesHeight(0.0),
   fitToPage(true),
   keepAspectRatio(true),
   pageNumber(1),
   specifyPage(false),
   lastPage(false),
   newLastPage(true),

   bstrResultsFile(NULL),

   refCount(0)

   {

   memset(szChosenDevice,0,sizeof(szChosenDevice));
   memset(szDeviceId,0,sizeof(szDeviceId));

   pIGPropertyPageClient = new _IGPropertyPageClient(this);

   HRESULT rc = CoCreateInstance(CLSID_InnoVisioNateProperties,NULL,CLSCTX_ALL,IID_IGProperties,reinterpret_cast<void **>(&pIGProperties));

#ifdef DEBUG
   pIGProperties -> put_DebuggingEnabled(true);
#endif

   long sizeParameters = offsetof(ImagingBackEnd,endParameters) - offsetof(ImagingBackEnd,startParameters);

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

#if (_WIN32_WINNT >= 0x0600) // Windows Vista and later
   HRESULT hr = CoCreateInstance(CLSID_WiaDevMgr2, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr2, (void**)&pIWiaDevMgr );
#else
   HRESULT hr = CoCreateInstance(CLSID_WiaDevMgr, NULL, /*CLSCTX_INPROC_SERVER*/ CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr, (void**)&pIWiaDevMgr );
#endif

   return;
   }


   ImagingBackEnd::~ImagingBackEnd() {

   IPrintingSupportProfile *px = NULL;
   pICursiVisionServices -> get_PrintingSupportProfile(&px);
   if ( pICursiVisionServices -> IsAdministrator() || ! px )
      pIGProperties -> Save();

#if (_WIN32_WINNT >= 0x0600) // Windows Vista and later
   if ( pIWiaTransferCallback )
      pIWiaTransferCallback -> Release();
#endif

   if ( pIWiaDevMgr )
      pIWiaDevMgr -> Release();

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


   void SaveBitmapFile(HDC hdcSource,HBITMAP hBitmap,char *pszBitmapFileName) {

   BITMAP bitMap;

   GetObject(hBitmap,sizeof(BITMAP),&bitMap);

   _int32 colorTableSize = (_int32)sizeof(RGBQUAD) * ( 1 << (bitMap.bmPlanes * bitMap.bmBitsPixel) );

   long entireSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTableSize;

   BYTE *pBuffer = new BYTE[entireSize];

   memset(pBuffer,0,entireSize * sizeof(BYTE));

   BITMAPFILEHEADER *pBitmapFileHeader = (BITMAPFILEHEADER *)pBuffer;

   BITMAPINFO *pBitmapInfo = (BITMAPINFO *)(pBuffer + sizeof(BITMAPFILEHEADER));

   BITMAPINFOHEADER *pBitmapInfoHeader = &(pBitmapInfo -> bmiHeader);
   
   pBitmapInfoHeader -> biSize = sizeof(BITMAPINFOHEADER); 
   pBitmapInfoHeader -> biWidth = bitMap.bmWidth;
   pBitmapInfoHeader -> biHeight = bitMap.bmHeight;
   pBitmapInfoHeader -> biPlanes = bitMap.bmPlanes; 
   pBitmapInfoHeader -> biBitCount = bitMap.bmBitsPixel;
   pBitmapInfoHeader -> biCompression = BI_RGB;
   if ( 1 == bitMap.bmBitsPixel ) {
      pBitmapInfoHeader -> biClrUsed = 2;
      pBitmapInfoHeader -> biClrImportant = 2;
   } else {
      pBitmapInfoHeader -> biClrUsed = 0;
      pBitmapInfoHeader -> biClrImportant = 0;
   }

   pBitmapInfoHeader -> biSizeImage = bitMap.bmHeight * ((bitMap.bmWidth * bitMap.bmPlanes * bitMap.bmBitsPixel + 31) & ~31) / 8 ;

   BYTE *pBits = new BYTE[pBitmapInfoHeader -> biSizeImage];

   memset(pBits,0,pBitmapInfoHeader -> biSizeImage);

   long rc = GetDIBits(hdcSource,hBitmap,0,bitMap.bmHeight,pBits,pBitmapInfo,DIB_RGB_COLORS);

   pBitmapFileHeader -> bfType = 0x4d42;

   pBitmapFileHeader -> bfSize = entireSize + pBitmapInfoHeader -> biSizeImage;

   pBitmapFileHeader -> bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTableSize;

   FILE *fBitmap = fopen(pszBitmapFileName,"wb");

   fwrite(pBitmapFileHeader,sizeof(BITMAPFILEHEADER),1,fBitmap);

   fwrite(pBitmapInfoHeader,sizeof(BITMAPINFOHEADER),1,fBitmap);

   fwrite(pBitmapInfo -> bmiColors,colorTableSize,1,fBitmap);

   fwrite(pBits,pBitmapInfoHeader -> biSizeImage,1,fBitmap);

   fclose(fBitmap);

   delete [] pBuffer;
   delete [] pBits;

   return;
   }   
   
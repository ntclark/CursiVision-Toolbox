
#include "VideoBackEnd.h"

   VideoBackEnd::VisioLoggerVideoBackEnd::VisioLoggerVideoBackEnd(VideoBackEnd *pp) :

   pParent(pp),
#if 0
   pIGProperties(NULL),
   pIGPropertiesClient(NULL),
#endif
   pIGPropertyPageClient(NULL),

   hwndProperties(NULL),
   hwndParent(NULL),

   startParameters(0),
   endParameters(0),

   doProperties(false),
   autoSnap(false),
   skipImaging(false),
   snapSecretly(false),
   isMainMenuAction(false),
   showNoPictureWarning(true),

#if 0
   cameraCount(0L),
   pCameraNames(NULL),
#endif

   cxImage(0L),
   cyImage(0L),
   bmPlanes(0L),
   bmBitsPixel(0L),

   autoFocusTime(750),

   isProcessing(false),

   processingSemaphore(NULL),

   pPropertiesTemplate(NULL),

   pIVisioLoggerAction(NULL),
   pIVisioLoggerPreSignature(NULL),

   refCount(0)

   {

   memset(szImageField,0,sizeof(szImageField));
   memset(szActionField,0,sizeof(szActionField));
   memset(szTargetFile,0,sizeof(szTargetFile));
   memset(szKnownImageFields,0,sizeof(szKnownImageFields));
   memset(szKnownTextFields,0,sizeof(szKnownTextFields));
   memset(szStatusInfo,0,sizeof(szStatusInfo));

   pIGPropertyPageClient  = new _IGPropertyPageClient(this);

   long sizeParameters = offsetof(VisioLoggerVideoBackEnd,endParameters) - offsetof(VisioLoggerVideoBackEnd,startParameters);

   memset(&startParameters,0,sizeParameters);

   showNoPictureWarning = true;

   pIVisioLoggerAction = new VisioLoggerVideoBackEnd::_IVisioLoggerAction(this);

   pIVisioLoggerPreSignature = new VisioLoggerVideoBackEnd::_IVisioLoggerPreSignature(this);

   return;
   }


   void VideoBackEnd::VisioLoggerVideoBackEnd::initialize() {

   short bSuccess;
   pParent -> pIGProperties -> LoadFile(&bSuccess);
   if ( ! bSuccess )
      pParent -> pIGPropertiesClient -> InitNew();

   char *p = strtok(szKnownImageFields,":");
   while ( p ) {
      char *p2 = new char[strlen(p) + 1];
      strcpy(p2,p);
      knownImageFields.insert(knownImageFields.end(),p2);
      p = strtok(NULL,":");
   }

   p = strtok(szKnownTextFields,":");
   while ( p ) {
      char *p2 = new char[strlen(p) + 1];
      strcpy(p2,p);
      knownTextFields.insert(knownTextFields.end(),p2);
      p = strtok(NULL,":");
   }

   return;
   }


   VideoBackEnd::VisioLoggerVideoBackEnd::~VisioLoggerVideoBackEnd() {

   if ( pIVisioLoggerAction )
      delete pIVisioLoggerAction;

   if ( pIVisioLoggerPreSignature )
      delete pIVisioLoggerPreSignature;

   unInitialize();

   if ( pPropertiesTemplate )
      delete [] (BYTE *)pPropertiesTemplate;

   pPropertiesTemplate = NULL;

   return;
   }


   void VideoBackEnd::VisioLoggerVideoBackEnd::unInitialize() {

   memset(szKnownImageFields,0,sizeof(szKnownImageFields));

   for ( std::list<char *>::iterator it = knownImageFields.begin(); it != knownImageFields.end(); it++ ) {
      char *p = *it;
      sprintf(szKnownImageFields + strlen(szKnownImageFields),"%s:",p);
      delete [] p;
   }

   knownImageFields.clear();

   memset(szKnownTextFields,0,sizeof(szKnownTextFields));

   for ( std::list<char *>::iterator it = knownTextFields.begin(); it != knownTextFields.end(); it++ ) {
      char *p = *it;
      sprintf(szKnownTextFields + strlen(szKnownTextFields),"%s:",p);
      delete [] p;
   }

   knownTextFields.clear();

   return;
   }


   long VideoBackEnd::VisioLoggerVideoBackEnd::findImageIndex(SAFEARRAY *imageFieldNames) {

   long elementUpperBound = -1L;
   long index[] = {0};

   SafeArrayGetUBound(imageFieldNames,1,&elementUpperBound);

   for ( long k = 0; k <= elementUpperBound; k++ ) {
      BSTR bstrField = NULL;
      SafeArrayGetElement(imageFieldNames,index,&bstrField);
      char szField[64];
      WideCharToMultiByte(CP_ACP,0,bstrField,-1,szField,64,0,0);
      if ( 0 == strcmp(szField,szImageField) )
         break;
      index[0]++;
   }

   if ( elementUpperBound < index[0] )
      return -1L;

   return index[0];
   }


   void VideoBackEnd::VisioLoggerVideoBackEnd::findImageSize(SAFEARRAY *imageFieldNames,SAFEARRAY *imageHandles) {

   long index[] = { findImageIndex(imageFieldNames) };

   if ( -1L == index[0] )
      return;

   ULONG currentImage;
   SafeArrayGetElement(imageHandles,index,&currentImage);

   BITMAP bitmapCurrent;
   GetObject((HANDLE)currentImage,sizeof(BITMAP),&bitmapCurrent);

   cxImage = bitmapCurrent.bmWidth;
   cyImage = bitmapCurrent.bmHeight;
   bmPlanes = bitmapCurrent.bmPlanes;
   bmBitsPixel = bitmapCurrent.bmBitsPixel;

   return;
   }


   void SaveImage(BYTE *pBitmap,char *pszBitmapFileName) {

   BITMAPINFO *pBitmapInfo = (BITMAPINFO *)(pBitmap);

   BITMAPINFOHEADER *pBitmapInfoHeader = &(pBitmapInfo -> bmiHeader);

   long colorTableSize = sizeof(RGBQUAD) * ( 1 << (pBitmapInfoHeader -> biPlanes * pBitmapInfoHeader -> biClrUsed) );

   long entireSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTableSize;

   BITMAPFILEHEADER bitmapFileHeader = {0};

   bitmapFileHeader.bfType = 0x4d42;

   bitmapFileHeader.bfSize = entireSize + pBitmapInfoHeader -> biSizeImage;

   bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTableSize;

   FILE *fBitmap = fopen(pszBitmapFileName,"wb");

   fwrite(&bitmapFileHeader,sizeof(BITMAPFILEHEADER),1,fBitmap);

   fwrite(pBitmapInfoHeader,sizeof(BITMAPINFOHEADER),1,fBitmap);

   fwrite(pBitmapInfo -> bmiColors,colorTableSize,1,fBitmap);

   BYTE *pBits = pBitmap + sizeof(BITMAPINFO) + sizeof(BITMAPINFOHEADER);

   fwrite(pBits,pBitmapInfoHeader -> biSizeImage,1,fBitmap);

   fclose(fBitmap);

   return;
   }


   void SaveBitmapFile(HDC hdcSource,HBITMAP hBitmap,char *pszBitmapFileName) {

   BITMAP bitMap;

   GetObject(hBitmap,sizeof(BITMAP),&bitMap);

   long colorTableSize = sizeof(RGBQUAD) * ( 1 << (bitMap.bmPlanes * bitMap.bmBitsPixel) );

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
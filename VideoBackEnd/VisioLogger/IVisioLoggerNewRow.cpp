
#include "VideoBackEnd.h"

#include "visioLoggerResource.h"

   HRESULT VideoBackEnd::VisioLoggerVideoBackEnd::Handle(OLE_HANDLE hParent,SAFEARRAY *passedFieldNames,SAFEARRAY *imageFieldNames,SAFEARRAY *documentFieldNames,SAFEARRAY *passedFieldValues,SAFEARRAY *imageHandles,SAFEARRAY *documentValues,SAFEARRAY *valuesChanged,SAFEARRAY *imagesChanged,SAFEARRAY *documentsChanged,BSTR *pBstrSwitchToProcessName,VARIANT_BOOL *pRetainValues) {

   if ( skipImaging )
      return S_OK;

   hwndParent = (HWND)hParent;

   if ( ( ! pParent -> szwChosenDevice[0] && 0 < pParent -> cameraCount ) || isMainMenuAction ) {

SetProperties:

      if ( isMainMenuAction )
         isProcessing = true;

      IUnknown *pIUnknown = NULL;
      QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
      pParent -> pIGProperties -> ShowProperties(hwndParent,pIUnknown);
      pIUnknown -> Release();

   }

   if ( skipImaging )
      return S_OK;

   HBITMAP hbmNotification = NULL;

   if ( NULL == imageFieldNames && ! isMainMenuAction ) {
      hbmNotification = (HBITMAP)LoadImage(hModule,MAKEINTRESOURCE(ID_BITMAP_NO_IMAGE_FIELD_AVAILABLE),IMAGE_BITMAP,0,0,0L);
      sprintf(szStatusInfo,"There are no image fields defined in this database");
   }

   long index[] = { findImageIndex(imageFieldNames) };

   if ( ! szImageField[0] && NULL == hbmNotification && ! isMainMenuAction ) {
      hbmNotification = (HBITMAP)LoadImage(hModule,MAKEINTRESOURCE(ID_BITMAP_NO_IMAGE_FIELD_SPECIFIED),IMAGE_BITMAP,0,0,0L);
      sprintf(szStatusInfo,"The image field used to store the picture has not been defined");
      index[0] = 0;
   }

   if ( -1L == index[0] && NULL == hbmNotification && ! isMainMenuAction ) {
      hbmNotification = (HBITMAP)LoadImage(hModule,MAKEINTRESOURCE(ID_BITMAP_IMAGE_FIELD_NOT_AVAILABLE),IMAGE_BITMAP,0,0,0L);
      sprintf(szStatusInfo,"The configured image field is not present in the image fields defined for this database");
      index[0] = 0;
   }

   if ( 0 == pParent -> cameraCount && NULL == hbmNotification ) {
      hbmNotification = (HBITMAP)LoadImage(hModule,MAKEINTRESOURCE(ID_BITMAP_NO_CAMERA),IMAGE_BITMAP,0,0,0L);
      sprintf(szStatusInfo,"There is no camera on this computer");
   }

   if ( ! pParent -> szwChosenDevice[0] ) {
      char szMessage[512];
      sprintf(szMessage,"The camera has not been selected."
                        "\n\nSelect Retry to respecify the properties, or Cancel to exit without taking a photograph");

      if ( IDRETRY == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL) )
         goto SetProperties;

      return S_OK;
   }

   long deviceIndex = -1L;

   for ( long k = 0; k < pParent -> cameraCount; k++ ) {
      if ( wcscmp(pParent -> szwChosenDevice,pParent -> pCameraNames[k]) ) 
         continue;
      deviceIndex = k;
      break;
   }

   if ( -1L == deviceIndex ) {
      OLECHAR szwMessage[1024];
      swprintf(szwMessage,L"The device associated with the name: %ls\n\nWas not found on this system, is it disconnected ?\n\nPress Retry to specify the settings, or Cancel to exit",pParent -> szwChosenDevice);
      if ( IDCANCEL == MessageBoxW(NULL,szwMessage,L"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1 ) )
         return S_OK;
      goto SetProperties;
   }

   findImageSize(imageFieldNames,imageHandles);

   if ( 0 == cxImage ) {
      cxImage = 320;
      cyImage = 240;
   }

   isProcessing = true;

   if ( snapSecretly ) {
      PROPSHEETPAGE propertySheetPage = {0};
      propertySheetPage.lParam = (long)(void *)this;
      DialogBoxParam(hModule,MAKEINTRESOURCE(IDD_DISPOSITION_PROPERTIES),hwndParent,(DLGPROC)propertiesHandler,(long)&propertySheetPage);
   } else if ( ! isMainMenuAction ) {
      IUnknown *pIUnknown = NULL;
      QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
      pParent -> pIGProperties -> ShowProperties(hwndParent,pIUnknown);
      pIUnknown -> Release();
   } 

   isProcessing = false;

   FILE *fBitmap = fopen(szTargetFile,"rb");

   if ( ! fBitmap ) {

      if ( isMainMenuAction ) {
         if ( ! showNoPictureWarning )
            return S_OK;
         DLGTEMPLATE *dt = (DLGTEMPLATE *)LoadResource(hModule,FindResource(hModule,MAKEINTRESOURCE(IDD_NO_PICTURE),RT_DIALOG));
         PROPSHEETPAGE propertySheetPage = {0};
         propertySheetPage.lParam = (long)(void *)this;
         LRESULT hwndWarning = DialogBoxIndirectParam(hModule,dt,(HWND)hwndParent,(DLGPROC)VideoBackEnd::propertiesHandler,(long)&propertySheetPage);
         return S_OK;
      } else {
         char szMessage[1024];
         sprintf(szMessage,"The system was not able to open the file: %s",szTargetFile);
         MessageBox(NULL,szMessage,"Error!",MB_OK);
         return S_OK;
      }

   } else if ( isMainMenuAction ) {

      fclose(fBitmap);

      OPENFILENAME openFileName = {0};
      char szFilter[MAX_PATH],szFile[MAX_PATH];

      memset(szFilter,0,sizeof(szFilter));
      memset(szFile,0,sizeof(szFile));

      sprintf(szFilter,"Bitmaps (*.bmp)");

      openFileName.lStructSize = sizeof(OPENFILENAME);
      openFileName.hwndOwner = NULL;
      openFileName.Flags = OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
      openFileName.lpstrFilter = szFilter;
      openFileName.lpstrFile = szFile;
      openFileName.lpstrDefExt = "bmp";
      openFileName.nFilterIndex = 1;
      openFileName.nMaxFile = MAX_PATH;
      openFileName.lpstrTitle = "Save your picture";

      if ( GetSaveFileName(&openFileName) ) 
         CopyFile(szTargetFile,openFileName.lpstrFile,FALSE);

      DeleteFile(szTargetFile);

      return S_OK;

   }
   
   BITMAPFILEHEADER bitmapFileHeader;
   
   if ( ! fread(&bitmapFileHeader,sizeof(BITMAPFILEHEADER),1,fBitmap) ) {
      fclose(fBitmap);
      return E_UNEXPECTED;
   }
   
   long colorTableSize = bitmapFileHeader.bfOffBits - sizeof(BITMAPFILEHEADER);

   BYTE *buffer = new BYTE[colorTableSize];
   
   if ( ! fread(buffer,colorTableSize,1,fBitmap) ) {
      delete [] buffer;
      fclose(fBitmap);
      return E_UNEXPECTED;
   }
   
   BITMAPINFO *pBitmapInfo = (BITMAPINFO *)buffer;

   BYTE *pBits = new BYTE[pBitmapInfo -> bmiHeader.biSizeImage];

   long rc = fread(pBits,pBitmapInfo -> bmiHeader.biSizeImage,1,fBitmap);

   fclose(fBitmap);

   HDC hdcTarget = CreateCompatibleDC(NULL);
   HBITMAP bitmapTarget = CreateBitmap(pBitmapInfo -> bmiHeader.biWidth,pBitmapInfo -> bmiHeader.biHeight,bmPlanes,bmBitsPixel,NULL);
   HGDIOBJ oldBitmapTarget = SelectObject(hdcTarget,bitmapTarget);

   rc = StretchDIBits(hdcTarget,0,0,pBitmapInfo -> bmiHeader.biWidth,pBitmapInfo -> bmiHeader.biHeight,0,0,
                                 pBitmapInfo -> bmiHeader.biWidth,pBitmapInfo -> bmiHeader.biHeight,pBits,pBitmapInfo,DIB_RGB_COLORS,SRCCOPY);

   delete [] pBits;
   delete [] buffer;

   ULONG currentImage;

   SafeArrayGetElement(imageHandles,index,&currentImage);

   if ( 0 != currentImage )
      DeleteObject((HANDLE)currentImage);

   ULONG v = (ULONG)bitmapTarget;

   SafeArrayPutElement(imageHandles,index,&v);

   long v2 = 1L;
   SafeArrayPutElement(imagesChanged,index,&v2);

   DeleteDC(hdcTarget);

   DeleteFile(szTargetFile);

   return S_OK;
   }


   HRESULT VideoBackEnd::VisioLoggerVideoBackEnd::TakeFields(SAFEARRAY *fieldNames,SAFEARRAY *imageFieldNames,SAFEARRAY *documentFieldNames) {

   for ( std::list<char *>::iterator it = knownImageFields.begin(); it != knownImageFields.end(); it++ ) {
      char *p = *it;
      delete [] p;
   }

   knownImageFields.clear();

   for ( std::list<char *>::iterator it = knownTextFields.begin(); it != knownTextFields.end(); it++ ) {
      char *p = *it;
      delete [] p;
   }

   knownTextFields.clear();

   long countElements = 0L;
   long index[] = {0};

   if ( imageFieldNames ) {

      SafeArrayGetUBound(imageFieldNames,1,&countElements);

      for ( long k = 0; k <= countElements; k++ ) {

         BSTR passedName = NULL;

         SafeArrayGetElement(imageFieldNames,index,(void **)&passedName);

         char *p = new char[wcslen(passedName) + 1];
         WideCharToMultiByte(CP_ACP,0,passedName,-1,p,wcslen(passedName) + 1,0,0);
         knownImageFields.insert(knownImageFields.end(),p);

         index[0]++;

      }

   }

   if ( fieldNames ) {

      index[0] = 0;

      SafeArrayGetUBound(fieldNames,1,&countElements);

      for ( long k = 0; k <= countElements; k++ ) {

         BSTR passedName = NULL;

         SafeArrayGetElement(fieldNames,index,(void **)&passedName);

         char *p = new char[wcslen(passedName) + 1];
         WideCharToMultiByte(CP_ACP,0,passedName,-1,p,wcslen(passedName) + 1,0,0);
         knownTextFields.insert(knownTextFields.end(),p);

         index[0]++;

      }

   }

   return S_OK;
   }

   HRESULT VideoBackEnd::VisioLoggerVideoBackEnd::TakeDatabaseInfo(long pIDbConnectionUnknown,BSTR,BSTR,VARIANT_BOOL,BSTR,BSTR,BSTR,BSTR,BSTR,long) {
   return S_OK;
   }

   HRESULT VideoBackEnd::VisioLoggerVideoBackEnd::get_StatusNotification(BSTR *pResult) {
   *pResult = SysAllocStringLen(NULL,strlen(szStatusInfo));
   MultiByteToWideChar(CP_ACP,0,szStatusInfo,-1,*pResult,strlen(szStatusInfo));
   return S_OK;
   }


   HRESULT VideoBackEnd::VisioLoggerVideoBackEnd::Properties(OLE_HANDLE hParent) {

   hwndParent = (HWND)hParent;

   IUnknown *pIUnknown = NULL;
   QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
   pParent -> pIGProperties -> ShowProperties(hwndParent,pIUnknown);
   pIUnknown -> Release();

   return S_OK;
   }

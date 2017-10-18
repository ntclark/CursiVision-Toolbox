// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ImagingBackEnd.h"

   TW_UINT16 twainCallback(pTW_IDENTITY pOrigin,pTW_IDENTITY pDest,TW_UINT32 DG,TW_UINT16 DAT,TW_UINT16 MSG,TW_MEMREF pData);

   unsigned int __stdcall ImagingBackEnd::getScans(void *p) {

   CoInitialize(NULL);

   ImagingBackEnd *pThis = (ImagingBackEnd *)p;

   WNDCLASS gClass = {0};
   
   gClass.style = CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
   gClass.cbClsExtra = 32;
   gClass.cbWndExtra = 32;
   gClass.hInstance = hModule;
   gClass.hIcon = NULL;
   gClass.hCursor = NULL;
   gClass.hbrBackground = 0;
   gClass.lpszMenuName = NULL;

   gClass.lpfnWndProc = scannerHandler;
   gClass.lpszClassName = "scannerHost";

   RegisterClass(&gClass);

   pThis -> hwndScanner = CreateWindowEx(0L,"scannerHost","",0L,256,256,256,128,NULL,NULL,hModule,(void *)pThis);

   SetWindowPos(pThis -> hwndScanner,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);

   pThis -> scannerStarted = false;
   pThis -> scanningCanceled = false;

   TW_UINT16 ret = dsmEntryProcedure(&pThis -> twainIdentity,0,DG_CONTROL,DAT_PARENT,MSG_OPENDSM,(TW_MEMREF)&pThis -> hwndScanner);

   pThis -> pCurrentSource = NULL;

   for ( std::list<TW_IDENTITY *>::iterator it = pThis -> twainSources.begin(); it != pThis -> twainSources.end(); it++ ) {
      if ( strcmp((*it) -> ProductName,pThis -> szChosenDevice) )
         continue;
      pThis -> pCurrentSource = (*it);
      break;
   }

   if ( ! pThis -> pCurrentSource ) {

      dsmEntryProcedure(&pThis -> twainIdentity,0,DG_CONTROL,DAT_PARENT,MSG_CLOSEDSM,(TW_MEMREF)&pThis -> hwndScanner);

      char szMessage[1024];
      sprintf(szMessage,"The device associated with the name: %s\n\nWas not found on this system, is it disconnected ?\n\nPress Retry to specify the settings, or Cancel to exit",pThis -> szChosenDevice);
      if ( IDRETRY == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1 ) )
         pThis -> doScanFailureProperties = true;
      else
         pThis -> scanningCanceled = true;

      DestroyWindow(pThis -> hwndScanner);
      pThis -> hwndScanner = NULL;
      return 0L;

   }

   TW_CALLBACK twCallback = {0};
   twCallback.CallBackProc = twainCallback;
   ret = dsmEntryProcedure(&pThis -> twainIdentity,NULL,DG_CONTROL,DAT_CALLBACK,MSG_REGISTER_CALLBACK,(TW_MEMREF)&twCallback);

   PostMessage(pThis -> hwndScanner,WM_START_SCANNER,0L,0L);

   MSG msg;
   TW_EVENT twainEvent;
   TW_INT16 rc;

   while ( GetMessage(&msg,NULL,0L,0L) ) {

      if ( PeekMessage(&msg,NULL,WM_QUIT,WM_QUIT,PM_REMOVE) ) break;

      rc = TWRC_NOTDSEVENT;

      if ( pThis -> scannerStarted ) {
         twainEvent.pEvent = (TW_MEMREF)&msg;
         twainEvent.TWMessage = MSG_NULL;
         if ( TWRC_DSEVENT == ( rc = dsmEntryProcedure(&pThis -> twainIdentity,pThis -> pCurrentSource,DG_CONTROL,DAT_EVENT,MSG_PROCESSEVENT,(TW_MEMREF)&twainEvent) ) ) {
            switch ( twainEvent.TWMessage ) {
            case MSG_XFERREADY:
               PostMessage(pThis -> hwndScanner,WM_START_TRANSFER,0L,0L);
               break;
            case MSG_CLOSEDSREQ:
               PostMessage(pThis -> hwndScanner,WM_STOP_SCANNER,0L,0L);
               break;
            }
         }
      }

      if ( TWRC_NOTDSEVENT == rc )
         DispatchMessage(&msg);

   }

   if ( pThis -> pCurrentSource )
      dsmEntryProcedure(&pThis -> twainIdentity,pThis -> pCurrentSource,DG_CONTROL,DAT_PARENT,MSG_CLOSEDS,(TW_MEMREF)&pThis -> hwndScanner);

   dsmEntryProcedure(&pThis -> twainIdentity,0,DG_CONTROL,DAT_PARENT,MSG_CLOSEDSM,(TW_MEMREF)&pThis -> hwndScanner);

   pThis -> pCurrentSource = NULL;

   DestroyWindow(pThis -> hwndScanner);

   pThis -> hwndScanner = NULL;

   CoUninitialize();

   return 0L;
   }


   LRESULT CALLBACK ImagingBackEnd::scannerHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   ImagingBackEnd *p = (ImagingBackEnd *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

   switch ( msg ) {

   case WM_NCCREATE:
   case WM_CREATE: {
      CREATESTRUCT *pc = reinterpret_cast<CREATESTRUCT *>(lParam);
      p = reinterpret_cast<ImagingBackEnd *>(pc -> lpCreateParams);
      SetWindowLongPtr(hwnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(p));   
      }
      return (LRESULT)1L;

   case WM_START_SCANNER: {

      TW_UINT16 ret;

      if ( ! ( TWRC_SUCCESS == ( ret = dsmEntryProcedure(&p -> twainIdentity,0,DG_CONTROL,DAT_IDENTITY,MSG_OPENDS,(TW_MEMREF)p -> pCurrentSource) ) ) ) {

         TW_STATUS twStatus;

         ret = dsmEntryProcedure(&p -> twainIdentity,0,DG_CONTROL,DAT_STATUS,MSG_GET,(TW_MEMREF)&twStatus);

         char szMessage[1024];
         sprintf(szMessage,"The device associated with the name: %s\n\nMay not be functional, is it disconnected ?\n\nPress Retry to specify the settings, or Cancel to exit",p -> szChosenDevice);
         if ( IDRETRY == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1 ) )
            p -> doScanFailureProperties = true;
         else
            p -> scanningCanceled = true;

         PostQuitMessage(0L);

         return (LRESULT)0L;

      }

      TW_CAPABILITY capability = {0};
      TW_ONEVALUE *pValue;

      capability.Cap = CAP_XFERCOUNT;
      capability.ConType = TWON_ONEVALUE;
      capability.hContainer = GlobalAlloc(GHND,sizeof(TW_ONEVALUE));

      pValue = (TW_ONEVALUE *)GlobalLock(capability.hContainer);
      pValue -> ItemType = TWTY_INT16;
      pValue -> Item = -1L;

      GlobalUnlock(capability.hContainer);

      ret = dsmEntryProcedure(&p -> twainIdentity,p -> pCurrentSource,DG_CONTROL,DAT_CAPABILITY,MSG_SET,(TW_MEMREF)&capability);

      GlobalFree(capability.hContainer);

#if 0
      capability.Cap = ICAP_PIXELTYPE;
      capability.hContainer = GlobalAlloc(GHND,sizeof(TW_ONEVALUE));
      pValue = (TW_ONEVALUE *)GlobalLock(capability.hContainer);
      pValue -> ItemType = TWON_ONEVALUE;
      pValue -> Item = TWPT_RGB;

      GlobalUnlock(capability.hContainer);

      ret = dsmEntryProcedure(&p -> twainIdentity,p -> pCurrentSource,DG_CONTROL,DAT_CAPABILITY,MSG_SET,(TW_MEMREF)&capability);

      GlobalFree(capability.hContainer);
#endif

#if 0
      capability.Cap = ICAP_XFERMECH;
      capability.ConType = TWON_ENUMERATION;
      capability.hContainer = NULL;

      ret = dsmEntryProcedure(&p -> twainIdentity,p -> pCurrentSource,DG_CONTROL,DAT_CAPABILITY,MSG_GET,(TW_MEMREF)&capability);

      if ( capability.hContainer ) {

         TW_ENUMERATION *pEnumeratedValue = (TW_ENUMERATION *)GlobalLock(capability.hContainer);

         for ( unsigned long k = 0; k < pEnumeratedValue -> NumItems; k++ ) {
            if ( ! ( pEnumeratedValue -> ItemList[k] == TWSX_FILE ) )
               continue;
            p -> fileTransfer = true;
            break;
         }

         GlobalFree(capability.hContainer);

         if ( p -> fileTransfer ) {

            capability.ConType = TWON_ONEVALUE;
            capability.hContainer = GlobalAlloc(GHND,sizeof(TW_ONEVALUE));

            pValue = (TW_ONEVALUE *)GlobalLock(capability.hContainer);

            pValue -> ItemType = TWTY_UINT16;
            pValue -> Item = TWSX_FILE;

            ret = dsmEntryProcedure(&p -> twainIdentity,p -> pCurrentSource,DG_CONTROL,DAT_CAPABILITY,MSG_SET,(TW_MEMREF)&capability);

            GlobalFree(capability.hContainer);

            capability.ConType = TWON_ENUMERATION;
            capability.Cap = ICAP_IMAGEFILEFORMAT;
            capability.hContainer = NULL;

            ret = dsmEntryProcedure(&p -> twainIdentity,p -> pCurrentSource,DG_CONTROL,DAT_CAPABILITY,MSG_GET,(TW_MEMREF)&capability);

            TW_ENUMERATION *pEnumeratedValue = (TW_ENUMERATION *)GlobalLock(capability.hContainer);

            TW_INT8 bestFormats[] = {TWFF_JFIF,TWFF_PNG,TWFF_BMP,-1};

            p -> imageFormat = -1L;

            for ( long fmtIndex = 0; 1; fmtIndex++ ) {

               if ( -1 == bestFormats[fmtIndex] )
                  break;

               for ( unsigned long k = 0; k < pEnumeratedValue -> NumItems; k++ ) {
                  if ( ! ( (TW_UINT16)pEnumeratedValue -> ItemList[k] == bestFormats[fmtIndex] ) )
                     continue;
                  p -> imageFormat = bestFormats[fmtIndex];
                  break;
               }

               if ( ! ( -1L == p -> imageFormat ) )
                  break;

            }

            GlobalFree(capability.hContainer);

         }

      }
#endif

      TW_USERINTERFACE twainUI = {0};

      twainUI.ShowUI = true;
      twainUI.ModalUI = true;
      twainUI.hParent = p -> hwndScanner;

      ret = dsmEntryProcedure(&p -> twainIdentity,p -> pCurrentSource,DG_CONTROL,DAT_USERINTERFACE,MSG_ENABLEDS,(TW_MEMREF)&twainUI);

      p -> scannerStarted = true;

      }
      break;

   case WM_START_TRANSFER: {

      TW_IMAGEINFO imageInfo = {0};
      HBITMAP hBitmap;
      TW_UINT16 rc;
      bool pendingTransfers = true;

      while ( pendingTransfers ) {

         rc = dsmEntryProcedure(&p -> twainIdentity,p -> pCurrentSource,DG_IMAGE,DAT_IMAGEINFO,MSG_GET,(TW_MEMREF)&imageInfo);

         TW_SETUPFILEXFER twSetupFileXFer = {0};

         if ( p -> fileTransfer ) {

            memset(&twSetupFileXFer,0,sizeof(TW_SETUPFILEXFER));

            strcpy(twSetupFileXFer.FileName,_tempnam(NULL,NULL));
            twSetupFileXFer.Format = p -> imageFormat;
            twSetupFileXFer.VRefNum = TWON_DONTCARE16;

            rc = dsmEntryProcedure(&p -> twainIdentity,p -> pCurrentSource,DG_CONTROL,DAT_SETUPFILEXFER,MSG_SET,(TW_MEMREF)&twSetupFileXFer);

            if ( rc ) {
               TW_STATUS twStatus = {0};
               rc = dsmEntryProcedure(&p -> twainIdentity,0,DG_CONTROL,DAT_STATUS,MSG_GET,(TW_MEMREF)&twStatus);
            }

            rc = dsmEntryProcedure(&p -> twainIdentity,p -> pCurrentSource,DG_IMAGE,DAT_IMAGEFILEXFER,MSG_GET,NULL);

         } else {

            rc = dsmEntryProcedure(&p -> twainIdentity,p -> pCurrentSource,DG_IMAGE,DAT_IMAGENATIVEXFER,MSG_GET,(TW_MEMREF)&hBitmap);

         }

         switch ( rc ) {

         case TWRC_XFERDONE: {

            char szBitmap[MAX_PATH];

            if ( p -> fileTransfer ) {
               strcpy(szBitmap,twSetupFileXFer.FileName);
               SaveJPEG(szBitmap);
            } else {
               strcpy(szBitmap,_tempnam(NULL,NULL));
               SaveJPEG(&imageInfo,hBitmap,szBitmap);
               GlobalFree(hBitmap);
            }
            BSTR bstrBitmap = SysAllocStringLen(NULL,MAX_PATH);
            MultiByteToWideChar(CP_ACP,0,szBitmap,-1,bstrBitmap,MAX_PATH);
            p -> imageFiles.insert(p -> imageFiles.end(),bstrBitmap);
            }
            break;

         case TWRC_CANCEL: {
            }
            break;

         case TWRC_FAILURE: {
            pendingTransfers = false;
            }
            break;

         default:
            break;
         }

         TW_PENDINGXFERS twainPendingXfers = {0};

         rc = dsmEntryProcedure(&p -> twainIdentity,p -> pCurrentSource,DG_CONTROL,DAT_PENDINGXFERS,MSG_ENDXFER,(TW_MEMREF)&twainPendingXfers);

         if ( 0 == twainPendingXfers.Count )
            break;

         if ( ! p -> scanMultiple ) {
            rc = dsmEntryProcedure(&p -> twainIdentity,p -> pCurrentSource,DG_CONTROL,DAT_PENDINGXFERS,MSG_RESET,(TW_MEMREF)&twainPendingXfers);
            break;
         }

      }

      if ( ! p -> scanMultiple )
         PostMessage(hwnd,WM_STOP_SCANNER,0L,0L);

      }
      break;

   case WM_STOP_SCANNER: {
      if ( p -> pCurrentSource )
         dsmEntryProcedure(&p -> twainIdentity,p -> pCurrentSource,DG_CONTROL,DAT_PARENT,MSG_CLOSEDS,(TW_MEMREF)&p -> hwndScanner);
      p -> pCurrentSource = NULL;
      PostQuitMessage(0L);
      }
      break;

   default:
      break;
   
   }
   
   return (LRESULT)0L;
   }


   LRESULT CALLBACK ImagingBackEnd::reScanOptionHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   ImagingBackEnd *p = (ImagingBackEnd *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

   switch ( msg ) {
   case WM_INITDIALOG:
   case WM_CREATE:
      SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);
      break;
   case WM_COMMAND:
      switch ( LOWORD(wParam) ) {
      case IDOK:
         EndDialog(hwnd,IDOK);
         break;
      case IDNO:
         EndDialog(hwnd,IDNO);
         break;
      }
      break;

   default:
      break;
   }

   return (LRESULT)0L;
   }


   TW_UINT16 twainCallback(pTW_IDENTITY pOrigin,pTW_IDENTITY pDest,TW_UINT32 DG,TW_UINT16 DAT,TW_UINT16 MSG,TW_MEMREF pData) {
Beep(2000,1000);
   return 0;
   }
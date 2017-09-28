/*
                       Copyright (c) 2009,2010,2011 Nathan T. Clark
*/

#include "VideoBackEnd.h"
#include <commctrl.h>
#include <shTypes.h>
#include <shlobj.h>

#include "dispositionSettingsDefines.h"

static bool waitingForImage = false;
static bool skipToDraw = false;
static HWND hwndCameras = NULL;

#define OBJECT_WITH_PROPERTIES VideoBackEnd

#define WM_TIMER_ALLOW_DRAW         1

#define LOAD_ADDITIONAL                                                                \
{                                                                                      \
   SendMessage(hwndCameras,CB_RESETCONTENT,0L,0L);                                     \
   for ( long k = 0; k < (long)pObject -> cameraCount; k++ ) {                         \
      SendMessageW(hwndCameras,CB_ADDSTRING,0L,(LPARAM)pObject -> pCameraNames[k]);    \
      if ( 0 == wcscmp(pObject -> pCameraNames[k],pObject -> szChosenDevice) )         \
         SendMessage(hwndCameras,CB_SETCURSEL,k,0L);                                   \
   }                                                                                   \
   PUT_BOOL(p -> doProperties,IDDI_SHOW_PROPERTIES);        \
   PUT_BOOL(pObject -> skipImaging,IDDI_SKIP);              \
   PUT_BOOL(pObject -> autoSnap,IDDI_AUTO_SNAP);            \
   PUT_BOOL(pObject -> timeStamp,IDDI_TIME_STAMP);          \
   PUT_BOOL(pObject -> includeComputerName,IDDI_COMPUTER_NAME);   \
   PUT_BOOL(pObject -> useAnyCamera,IDDI_USE_ANY_CAMERA);         \
   PUT_BOOL(pObject -> ignoreNoCamera,IDDI_IGNORE_NO_CAMERA);     \
   PUT_BOOL(pObject -> saveDocumentAnyway,IDDI_SAVE_FILE_ANYWAY); \
   PUT_BOOL(pObject -> specifyPage,IDDI_PAGE_PAGENO_OPT);   \
   PUT_BOOL(pObject -> lastPage,IDDI_PAGE_ONLAST);          \
   PUT_BOOL(pObject -> newLastPage,IDDI_PAGE_NEWLAST);      \
   PUT_BOOL(pObject -> keepImage,IDDI_POSITION_KEEP_IMAGE); \
   PUT_DOUBLE(pObject -> inchesLeft,IDDI_POSITION_FROMLEFT);\
   PUT_DOUBLE(pObject -> inchesTop,IDDI_POSITION_FROMTOP);  \
   PUT_BOOL(pObject -> fitToPage,IDDI_SIZE_FITTOPAGE);      \
   PUT_DOUBLE(pObject -> inchesWidth,IDDI_SIZE_WIDTH);      \
   PUT_DOUBLE(pObject -> inchesHeight,IDDI_SIZE_HEIGHT);    \
   PUT_BOOL(pObject -> keepAspectRatio,IDDI_SIZE_MAINTAIN_ASPECT_RATIO); \
   PUT_LONG(pObject -> pageNumber,IDDI_PAGE_PAGENO);        \
   PUT_LONG(pObject -> autoFocusDelay,IDDI_FOCUS_TIME);     \
}

#define UNLOAD_ADDITIONAL \
{  \
   SendMessageW(hwndCameras,CB_GETLBTEXT,SendMessageW(hwndCameras,CB_GETCURSEL,0L,0L),(LPARAM)pObject -> szChosenDevice); \
   GET_BOOL(p -> doProperties,IDDI_SHOW_PROPERTIES);        \
   GET_BOOL(pObject -> skipImaging,IDDI_SKIP);              \
   GET_BOOL(pObject -> autoSnap,IDDI_AUTO_SNAP);            \
   GET_BOOL(pObject -> useAnyCamera,IDDI_USE_ANY_CAMERA);   \
   GET_BOOL(pObject -> ignoreNoCamera,IDDI_IGNORE_NO_CAMERA);     \
   GET_BOOL(pObject -> saveDocumentAnyway,IDDI_SAVE_FILE_ANYWAY); \
   GET_BOOL(pObject -> timeStamp,IDDI_TIME_STAMP);                \
   GET_BOOL(pObject -> includeComputerName,IDDI_COMPUTER_NAME);   \
   GET_BOOL(pObject -> specifyPage,IDDI_PAGE_PAGENO_OPT);   \
   GET_BOOL(pObject -> lastPage,IDDI_PAGE_ONLAST);          \
   GET_BOOL(pObject -> newLastPage,IDDI_PAGE_NEWLAST);      \
   GET_BOOL(pObject -> keepImage,IDDI_POSITION_KEEP_IMAGE); \
   GET_DOUBLE(pObject -> inchesLeft,IDDI_POSITION_FROMLEFT);\
   GET_DOUBLE(pObject -> inchesTop,IDDI_POSITION_FROMTOP);  \
   GET_BOOL(pObject -> fitToPage,IDDI_SIZE_FITTOPAGE);      \
   GET_DOUBLE(pObject -> inchesWidth,IDDI_SIZE_WIDTH);      \
   GET_DOUBLE(pObject -> inchesHeight,IDDI_SIZE_HEIGHT);    \
   GET_BOOL(pObject -> keepAspectRatio,IDDI_SIZE_MAINTAIN_ASPECT_RATIO); \
   GET_LONG(pObject -> pageNumber,IDDI_PAGE_PAGENO);        \
   GET_LONG(pObject -> autoFocusDelay,IDDI_FOCUS_TIME);     \
}

   BOOL CALLBACK adjustTop(HWND hwndTest,LPARAM lParam);
   BOOL CALLBACK page1(HWND hwndTest,LPARAM lParam);
   BOOL CALLBACK page2(HWND hwndTest,LPARAM lParam);
   BOOL CALLBACK page3(HWND hwndTest,LPARAM lParam);

   IVideoWindow *pIVideoWindow = NULL;

   LRESULT CALLBACK VideoBackEnd::propertiesHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   resultDisposition *p = (resultDisposition *)GetWindowLongPtr(hwnd,GWLP_USERDATA);
   VideoBackEnd *pObject = NULL;
   if ( p )
      pObject = (VideoBackEnd *)(p -> pParent);

   switch ( msg ) {

   case WM_INITDIALOG: {

      PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);
      p = (resultDisposition *)pPage -> lParam;
      SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)p);

      pObject = (VideoBackEnd *)(p -> pParent);

      pObject -> PushProperties();
      pObject -> PushProperties();

      RECT rcCameras,rcParent;

      GetWindowRect(GetDlgItem(hwnd,IDDI_IMAGER),&rcCameras);
      GetWindowRect(hwnd,&rcParent);

      DestroyWindow(GetDlgItem(hwnd,IDDI_IMAGER));

      hwndCameras = CreateWindowExW(0L,WC_COMBOBOXW,L"",CBS_DROPDOWNLIST | WS_VSCROLL | WS_CHILD | WS_VISIBLE,rcCameras.left - rcParent.left,rcCameras.top - rcParent.top,
                           rcCameras.right - rcCameras.left,128 + rcCameras.bottom - rcCameras.top,hwnd,(HMENU)IDDI_IMAGER,hModule,NULL);

      SendMessage(hwndCameras,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),0L);

      LOAD_CONTROLS

      LOAD_ADDITIONAL

      EnableWindow(GetDlgItem(hwnd,IDDI_IMAGER),pObject -> useAnyCamera ? FALSE : TRUE);
      EnableWindow(GetDlgItem(hwnd,IDDI_SAVE_FILE_ANYWAY),pObject -> ignoreNoCamera ? TRUE : FALSE);

      SendDlgItemMessage(hwnd,IDDI_PAGE_PAGENO_OPT,BM_SETCHECK,0L,0L);
      SendDlgItemMessage(hwnd,IDDI_PAGE_ONLAST,BM_SETCHECK,0L,0L);
      SendDlgItemMessage(hwnd,IDDI_PAGE_NEWLAST,BM_SETCHECK,0L,0L);

      if ( pObject -> specifyPage )
         SendDlgItemMessage(hwnd,IDDI_PAGE_PAGENO_OPT,BM_SETCHECK,(WPARAM)BST_CHECKED,0L);
      else if ( pObject -> lastPage )
         SendDlgItemMessage(hwnd,IDDI_PAGE_ONLAST,BM_SETCHECK,(WPARAM)BST_CHECKED,0L);
      else 
         SendDlgItemMessage(hwnd,IDDI_PAGE_NEWLAST,BM_SETCHECK,(WPARAM)BST_CHECKED,0L);

      EnableWindow(GetDlgItem(hwnd,IDDI_PAGE_PAGENO),pObject -> specifyPage);

      if ( pObject -> fitToPage ) {
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_HEIGHT),FALSE);
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_WIDTH),FALSE);
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_MAINTAIN_ASPECT_RATIO),FALSE);
         SendDlgItemMessage(hwnd,IDDI_SIZE_FITTOPAGE,BM_SETCHECK,(WPARAM)BST_CHECKED,0L);
      } else {
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_WIDTH),TRUE);
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_HEIGHT),TRUE);
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_MAINTAIN_ASPECT_RATIO),TRUE);
         SendDlgItemMessage(hwnd,IDDI_SIZE_FITTOPAGE,BM_SETCHECK,(WPARAM)BST_UNCHECKED,0L);
      }

      EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_HEIGHT),! pObject -> keepAspectRatio);

      if ( pObject -> isProcessing || pObject -> szChosenDevice[0] )
         PostMessage(hwnd,WM_START_VIDEO,0L,0L);

      RECT rcTabs,rcVideo;

      GetWindowRect(GetDlgItem(hwnd,IDDI_TABS),&rcTabs);
      GetWindowRect(GetDlgItem(hwnd,IDDI_VIDEO),&rcVideo);

      rcVideo.right += (rcVideo.right - rcVideo.left) % 8;
      rcVideo.bottom += (rcVideo.bottom - rcVideo.top) % 8;

      AdjustWindowRectEx(&rcVideo,GetWindowLongPtr(GetDlgItem(hwnd,IDDI_VIDEO),GWL_STYLE),FALSE,GetWindowLongPtr(GetDlgItem(hwnd,IDDI_VIDEO),GWL_EXSTYLE));

      SetWindowPos(GetDlgItem(hwnd,IDDI_VIDEO),HWND_TOP,0,0,rcVideo.right - rcVideo.left,rcVideo.bottom - rcVideo.top,SWP_NOMOVE);

      SetWindowPos(GetDlgItem(hwnd,IDDI_TABS),HWND_TOP,0,0,rcVideo.right - rcVideo.left + 64,rcVideo.bottom - rcTabs.top + 32,SWP_NOMOVE);

      TCITEM tcItem = {0};

      tcItem.pszText = "Image";
      tcItem.mask = TCIF_TEXT;

      SendDlgItemMessage(hwnd,IDDI_TABS,TCM_INSERTITEM,0,(LPARAM)&tcItem);

      tcItem.pszText = "Location";
      SendDlgItemMessage(hwnd,IDDI_TABS,TCM_INSERTITEM,1,(LPARAM)&tcItem);

      tcItem.pszText = DISPOSITION_TITLE;
      SendDlgItemMessage(hwnd,IDDI_TABS,TCM_INSERTITEM,2,(LPARAM)&tcItem);

      EnumChildWindows(hwnd,page1,NULL);

      if ( pObject -> isProcessing )
         EnableWindow(GetDlgItem(GetParent(hwnd),IDOK),FALSE);

      memset(&rcTabs,0,sizeof(RECT));

      SendDlgItemMessage(hwnd,IDDI_TABS,TCM_ADJUSTRECT,(WPARAM)TRUE,(LPARAM)&rcTabs);

      long deltaY = rcTabs.bottom - rcTabs.top;

      EnumChildWindows(hwnd,adjustTop,deltaY);

      GetWindowRect(GetDlgItem(hwnd,IDDI_TABS),&rcTabs);

      SetWindowPos(GetDlgItem(hwnd,IDDI_TABS),HWND_TOP,0,0,rcTabs.right - rcTabs.left,rcTabs.bottom - rcTabs.top + deltaY,SWP_NOMOVE | SWP_NOZORDER);

      SetWindowLongPtr(GetDlgItem(hwnd,IDDI_VIDEO),GWLP_USERDATA,(LONG_PTR)pObject);

      VideoBackEnd::defaultImageHandler = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwnd,IDDI_VIDEO),GWLP_WNDPROC,(LONG_PTR)VideoBackEnd::imageHandler);

      UDACCEL accelerators[1];
      accelerators[0].nSec = 0;
      accelerators[0].nInc = 100;

      SendDlgItemMessage(hwnd,IDDI_FOCUS_TIME_SPINNER,UDM_SETBUDDY,(WPARAM)GetDlgItem(hwnd,IDDI_FOCUS_TIME),0L);
      SendDlgItemMessage(hwnd,IDDI_FOCUS_TIME_SPINNER,UDM_SETRANGE,0L,MAKELPARAM(10000,0));
      SendDlgItemMessage(hwnd,IDDI_FOCUS_TIME_SPINNER,UDM_SETACCEL,1L,(LPARAM)accelerators);

      EnableWindow(GetDlgItem(hwnd,IDDI_FOCUS_TIME),pObject -> autoSnap);
      EnableWindow(GetDlgItem(hwnd,IDDI_FOCUS_TIME_SPINNER),pObject -> autoSnap);

      if ( pObject -> isProcessing && pObject -> autoSnap ) {
         waitingForImage = true;
         skipToDraw = true;
         PostMessage(hwnd,WM_SNAP_PHOTO,0L,0L);
      }

      IPrintingSupportProfile *px = NULL;

      pObject -> pICursiVisionServices -> get_PrintingSupportProfile(&px);

      if ( ! pObject -> pICursiVisionServices -> IsAdministrator() && px ) {
         RECT rc = {0};
         GetClientRect(hwnd,&rc);
         SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,rc.bottom - 32,0,0,SWP_NOSIZE);
         EnableWindow(hwnd,FALSE);
      } else
         DestroyWindow(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES));

      }
      return LRESULT(FALSE);

   case WM_START_VIDEO: {

      SendMessage(hwnd,WM_STOP_VIDEO,0L,0L);

      if ( ! pObject -> szChosenDevice[0] )
         break;

      HRESULT hr;
      ICreateDevEnum *pICreateDevEnum = NULL;
      IEnumMoniker *pIEnumMoniker = NULL;

      CoCreateInstance(CLSID_SystemDeviceEnum,NULL,CLSCTX_INPROC_SERVER,IID_ICreateDevEnum,reinterpret_cast<void**>(&pICreateDevEnum));

      pICreateDevEnum -> CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pIEnumMoniker,0);

      if ( ! pIEnumMoniker ) {
         char szTemp[1024];
         char szDevice[128];
         WideCharToMultiByte(CP_ACP,0,pObject -> szChosenDevice, -1, szDevice,128,0,0);
         sprintf(szTemp,"The video device: %s is not found or is disconnected",szDevice);
         MessageBox(pObject -> hwndParent,szTemp,"Error",MB_ICONEXCLAMATION | MB_TOPMOST);
         break;
      }

      IMoniker *pIMoniker = NULL;

      pObject -> pIBaseFilter = NULL;

      while ( S_OK == pIEnumMoniker -> Next(1, &pIMoniker, NULL) ) {

         IPropertyBag *pIPropertyBag;

         hr = pIMoniker -> BindToStorage(0,0,IID_IPropertyBag,reinterpret_cast<void **>(&pIPropertyBag));

         VARIANT varName;
         VariantInit(&varName);
         hr = pIPropertyBag -> Read(L"FriendlyName", &varName, 0);

         pIPropertyBag -> Release();

         if ( 0 == wcscmp(varName.bstrVal,pObject -> szChosenDevice) ) {
            hr = pIMoniker -> BindToObject(0,0,IID_IBaseFilter,reinterpret_cast<void **>(&pObject -> pIBaseFilter));
            pIMoniker -> Release();
            break;
         }

         pIMoniker -> Release();

      }

      pIEnumMoniker -> Release();
      pICreateDevEnum -> Release();

      if ( ! pObject -> pIBaseFilter ) {
         char szTemp[1024];
         char szDevice[128];
         WideCharToMultiByte(CP_ACP,0,pObject -> szChosenDevice, -1, szDevice,128,0,0);
         sprintf(szTemp,"The video device: %s is not found or is disconnected",szDevice);
         MessageBox(pObject -> hwndParent,szTemp,"Error",MB_ICONEXCLAMATION | MB_TOPMOST);
         break;
      }

      IBaseFilter *pIVideoMixingRenderer = NULL;

      ICaptureGraphBuilder2 *pICaptureGraphBuilder = NULL;

      CoCreateInstance(CLSID_FilterGraph,0,CLSCTX_INPROC_SERVER,IID_IGraphBuilder,reinterpret_cast<void **>(&pObject -> pIGraphBuilder));
      CoCreateInstance(CLSID_CaptureGraphBuilder2,NULL,CLSCTX_INPROC_SERVER,IID_ICaptureGraphBuilder2,reinterpret_cast<void **>(&pICaptureGraphBuilder));

      pICaptureGraphBuilder -> SetFiltergraph(pObject -> pIGraphBuilder);

      pObject -> pIGraphBuilder -> AddFilter(pObject -> pIBaseFilter, L"Preview Renderer");

      CoCreateInstance(CLSID_VideoMixingRenderer,NULL,CLSCTX_INPROC,IID_IBaseFilter,(void**)&pIVideoMixingRenderer);

      pObject -> pIGraphBuilder -> AddFilter(pIVideoMixingRenderer, L"Video Mixing Renderer");

      {
      IVMRFilterConfig *pConfig = NULL;
      pIVideoMixingRenderer -> QueryInterface(IID_IVMRFilterConfig,(void **)&pConfig);
      pConfig -> SetRenderingMode(VMRMode_Windowless);
      pConfig -> Release();
      }

      pIVideoMixingRenderer -> QueryInterface(IID_IVMRWindowlessControl,(void **)&pObject -> pIVMRWindowlessControl);

      pObject -> pIVMRWindowlessControl -> SetVideoClippingWindow(GetDlgItem(hwnd,IDDI_VIDEO));

      RECT rcClient;

      GetClientRect(GetDlgItem(hwnd,IDDI_VIDEO),&rcClient);

      pObject -> pIVMRWindowlessControl ->SetVideoPosition(NULL,&rcClient);

      pICaptureGraphBuilder -> RenderStream(&PIN_CATEGORY_PREVIEW,&MEDIATYPE_Video,pObject -> pIBaseFilter,NULL,pIVideoMixingRenderer);

      IMediaControl *pIMediaControl = NULL;

      pObject -> pIGraphBuilder -> QueryInterface(IID_IMediaControl,reinterpret_cast<void **>(&pIMediaControl));

      pIMediaControl -> Run();

      pIMediaControl -> Release();

      pICaptureGraphBuilder -> Release();

      pIVideoMixingRenderer -> Release();

      waitingForImage = false;

      if ( pObject -> isProcessing )
         EnableWindow(GetDlgItem(GetParent(hwnd),IDOK),TRUE);

      }
      break;

   case WM_SNAP_PHOTO: {

      if ( waitingForImage ) {
         PostMessage(hwnd,msg,wParam,lParam);
         break;
      } else if ( skipToDraw ) {
         SetTimer(hwnd,WM_TIMER_ALLOW_DRAW,pObject -> autoFocusDelay,NULL);
         break;
      }

      BYTE *pImage = NULL;

      HRESULT hr = pObject -> pIVMRWindowlessControl -> GetCurrentImage(&pImage);

      if ( ! pImage || ( S_OK != hr ) ) {
         SetTimer(hwnd,WM_TIMER_ALLOW_DRAW,pObject -> autoFocusDelay,NULL);
         break;
      }

      if ( pObject -> timeStamp || pObject -> includeComputerName ) 
         TimeStampBitmap(pImage,pObject -> szTargetFile,pObject -> timeStamp,pObject -> includeComputerName);
      else
         SaveJPEG(pImage,pObject -> szTargetFile);

      CoTaskMemFree(pImage);

      if ( pObject -> autoSnap )
         SendMessage(GetParent(hwnd),PSM_REMOVEPAGE,0L,SendMessage(GetParent(hwnd),PSM_INDEXTOPAGE,0L,0L));

      }
      break;

   case WM_DESTROY:
   case WM_STOP_VIDEO: {

      if ( ! pObject -> pIGraphBuilder )
         break;

      IMediaControl *pIMediaControl = NULL;
      HRESULT hr = pObject -> pIGraphBuilder -> QueryInterface(IID_IMediaControl,reinterpret_cast<void **>(&pIMediaControl));
      hr = pIMediaControl -> Stop();
      pIMediaControl -> Release();

      if ( pObject -> pIVMRWindowlessControl )
         pObject -> pIVMRWindowlessControl -> Release();
      pObject -> pIVMRWindowlessControl = NULL;

      pObject -> pIGraphBuilder -> Release();
      pObject -> pIGraphBuilder = NULL;

      pObject -> pIBaseFilter -> Release();
      pObject -> pIBaseFilter = NULL;

      }
      break;

   case WM_TIMER: {
      if ( WM_TIMER_ALLOW_DRAW == wParam ) {
         skipToDraw = false;
         KillTimer(hwnd,WM_TIMER_ALLOW_DRAW);
         PostMessage(hwnd,WM_SNAP_PHOTO,0L,0L);
      }
      }
      break;

   case WM_COMMAND: {

      switch ( LOWORD(wParam) ) {

#include "dispositionSettingsSaveOptionsWMCommand.cpp"

#include "dispositionSettingsSaveMoreOptionWMCommand.cpp"

      case IDDI_SIZE_MAINTAIN_ASPECT_RATIO:
         pObject -> keepAspectRatio = (BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_SIZE_MAINTAIN_ASPECT_RATIO,BM_GETCHECK,0L,0L));
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_HEIGHT),! pObject -> keepAspectRatio && ! pObject -> fitToPage);
         break;

      case IDDI_USE_ANY_CAMERA:
         pObject -> useAnyCamera = (BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_USE_ANY_CAMERA,BM_GETCHECK,0L,0L));
         EnableWindow(GetDlgItem(hwnd,IDDI_IMAGER),pObject -> useAnyCamera ? FALSE : TRUE);
         break;

      case IDDI_IGNORE_NO_CAMERA:
         pObject -> ignoreNoCamera = (BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_IGNORE_NO_CAMERA,BM_GETCHECK,0L,0L));
         EnableWindow(GetDlgItem(hwnd,IDDI_SAVE_FILE_ANYWAY),pObject -> ignoreNoCamera ? TRUE : FALSE);
         break;

      case IDDI_IMAGER: {
         if ( ! (HIWORD(wParam) == CBN_SELCHANGE) )
            break;
         long index = SendMessage(hwndCameras,CB_GETCURSEL,0L,0L);
         if ( CB_ERR == index )
            break;
         SendMessageW(hwndCameras,CB_GETLBTEXT,(WPARAM)index,(LPARAM)pObject -> szChosenDevice);
         PostMessage(hwnd,WM_START_VIDEO,0L,0L);
         }
         break;

      case IDDI_PAGE_PAGENO_OPT:
      case IDDI_PAGE_ONLAST:
      case IDDI_PAGE_NEWLAST:
         EnableWindow(GetDlgItem(hwnd,IDDI_PAGE_PAGENO),BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_PAGE_PAGENO_OPT,BM_GETCHECK,0L,0L) ? TRUE : FALSE);
         break;

      case IDDI_SIZE_FITTOPAGE:
         pObject -> fitToPage = (BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_SIZE_FITTOPAGE,BM_GETCHECK,0L,0L));
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_HEIGHT),! pObject -> fitToPage && ! pObject -> keepAspectRatio);
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_WIDTH),! pObject -> fitToPage);
         EnableWindow(GetDlgItem(hwnd,IDDI_SIZE_MAINTAIN_ASPECT_RATIO),! pObject -> fitToPage);
         break;

      case IDDI_AUTO_SNAP:
         pObject -> autoSnap = (BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_AUTO_SNAP,BM_GETCHECK,0L,0L));
         EnableWindow(GetDlgItem(hwnd,IDDI_FOCUS_TIME),pObject -> autoSnap);
         EnableWindow(GetDlgItem(hwnd,IDDI_FOCUS_TIME_SPINNER),pObject -> autoSnap);
         break;

      default:
         break;
      }

      }
      break;

   case WM_NOTIFY: {

      NMHDR *pNotifyHeader = (NMHDR *)lParam;

      switch ( pNotifyHeader -> code ) {

      case TCN_SELCHANGE: {
         switch ( SendDlgItemMessage(hwnd,IDDI_TABS,TCM_GETCURSEL,0L,0L) ) {
         case 0:
            EnumChildWindows(hwnd,page1,NULL);
            break;
         case 1:
            EnumChildWindows(hwnd,page2,NULL);
            break;
         case 2:
            EnumChildWindows(hwnd,page3,NULL);
            break;
         }
         }
         break;

      case PSN_KILLACTIVE: {
         UNLOAD_CONTROLS
         UNLOAD_ADDITIONAL
         SetWindowLongPtr(hwnd,DWLP_MSGRESULT,FALSE);
         }
         break;

      case PSN_APPLY: {

         PSHNOTIFY *pNotify = (PSHNOTIFY *)lParam;

         UNLOAD_CONTROLS

         UNLOAD_ADDITIONAL

         if ( pNotify -> lParam ) {
            IPrintingSupportProfile *px = NULL;
            pObject -> pICursiVisionServices -> get_PrintingSupportProfile(&px);
            if ( pObject -> pICursiVisionServices -> IsAdministrator() || ! px )
               pObject -> SaveProperties();
            pObject -> DiscardProperties();
            pObject -> DiscardProperties();
         } else {
            pObject -> DiscardProperties();
            pObject -> PushProperties();
         }

         if ( pObject -> isProcessing ) 
            SendMessage(hwnd,WM_SNAP_PHOTO,0L,0L);

         SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_NOERROR);

         return (LRESULT)TRUE;
         }
         break;

      case PSN_RESET: {
         pObject -> doExecute = false;
         pObject -> PopProperties();
         pObject -> PopProperties();
         }
         break;

      }

      }
      break;

   default:
      break;
   }

   return LRESULT(FALSE);
   }


   LRESULT CALLBACK VideoBackEnd::imageHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   VideoBackEnd *p = (VideoBackEnd *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

   switch ( msg ) {

   case WM_PAINT: {
      if ( ! p -> pIVMRWindowlessControl )
         return CallWindowProc(VideoBackEnd::defaultImageHandler,hwnd,msg,wParam,lParam);
      PAINTSTRUCT ps = {0};
      BeginPaint(hwnd,&ps);
      p -> pIVMRWindowlessControl -> RepaintVideo(hwnd,ps.hdc);  
      EndPaint(hwnd,&ps);
      }
      break;

   default:
      break;

   }

   return CallWindowProc(VideoBackEnd::defaultImageHandler,hwnd,msg,wParam,lParam);
   }

   BOOL CALLBACK adjustTop(HWND hwndTest,LPARAM lParam) {
   long id = GetWindowLongPtr(hwndTest,GWL_ID);
   if ( ( 1000 < id && id < 1100 ) || ( 2000 < id && id < 2100 ) || ( 3000 < id && id < 3100 ) ) {
      RECT rcNow,rcParent;
      GetWindowRect(GetParent(hwndTest),&rcParent);
      GetWindowRect(hwndTest,&rcNow);
      rcNow.top += lParam;
      rcNow.bottom += lParam;
      SetWindowPos(hwndTest,HWND_TOP,rcNow.left - rcParent.left,rcNow.top - rcParent.top,0,0,SWP_NOSIZE | SWP_NOZORDER);
   }
   return TRUE;
   }


   BOOL CALLBACK page1(HWND hwndTest,LPARAM lParam) {
   long id = GetWindowLongPtr(hwndTest,GWL_ID);
   if ( 1000 < id && id < 1100 )
      ShowWindow(hwndTest,SW_SHOW);
   else
      if ( ! ( 4000 < id && id < 4100 ) )
         ShowWindow(hwndTest,SW_HIDE);
   return TRUE;
   }

   BOOL CALLBACK page2(HWND hwndTest,LPARAM lParam) {
   long id = GetWindowLongPtr(hwndTest,GWL_ID);
   if ( 2000 < id && id < 2100 )
      ShowWindow(hwndTest,SW_SHOW);
   else 
      if ( ! ( 4000 < id && id < 4100 ) )
         ShowWindow(hwndTest,SW_HIDE);
   return TRUE;
   }

   BOOL CALLBACK page3(HWND hwndTest,LPARAM lParam) {
   long id = GetWindowLongPtr(hwndTest,GWL_ID);
   if ( ( 1000 < id && id < 1100 ) || ( 2000 < id && id < 2100 ) )
      ShowWindow(hwndTest,SW_HIDE);
   else  
      ShowWindow(hwndTest,SW_SHOW);
   return TRUE;
   }

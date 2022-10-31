/*
                       Copyright (c) 2009,2010,2011,2012,2013,2014 Nathan T. Clark
*/

#include "VideoBackEnd.h"

#include "visioLoggerResource.h"

#include <commctrl.h>

extern IVMRWindowlessControl9 *pIVMRWindowlessControl;

extern "C" int GetDocumentsLocation(HWND hwnd,char *);

static bool waitingForImage = false;
static bool skipToDraw = false;

#define WM_TIMER_ALLOW_DRAW 1

#define PUT_BOOL(v,id)  SendDlgItemMessage(hwnd,id,BM_SETCHECK,v ? BST_CHECKED : BST_UNCHECKED,0L);
#define PUT_LONG(v,id)  { char szX[32]; sprintf(szX,"%ld",v); SetDlgItemText(hwnd,id,szX); }
#define PUT_STRING(v,id) SetDlgItemText(hwnd,id,v);
#define DEFAULT_LONG(v,def) { if ( 0 == v ) v = def; };

#define GET_BOOL(v,id)  v = (BST_CHECKED == SendDlgItemMessage(hwnd,id,BM_GETCHECK,0L,0L));
#define GET_LONG(v,id) {char szX[32]; GetDlgItemText(hwnd,id,szX,32); v = atol(szX); }
#define GET_STRING(v,id) GetDlgItemText(hwnd,id,v,MAX_PATH);

#define LOAD_CONTROLS                                                                        \
{                                                                                            \
   SendMessage(GetDlgItem(hwnd,IDDI_IMAGER),CB_RESETCONTENT,0L,0L);                          \
   for ( long k = 0; k < p -> pParent -> cameraCount; k++ ) {                                \
      char szDeviceName[128];                                                                \
      WideCharToMultiByte(CP_ACP,0,p -> pParent -> pCameraNames[k],-1,szDeviceName,128,0,0); \
      SendMessage(GetDlgItem(hwnd,IDDI_IMAGER),CB_ADDSTRING,0L,(LPARAM)szDeviceName);        \
      if ( 0 == wcscmp(p -> pParent -> pCameraNames[k],p -> pParent -> szwChosenDevice) )    \
         SendMessage(GetDlgItem(hwnd,IDDI_IMAGER),CB_SETCURSEL,k,0L);                        \
   }                                                                                         \
   for ( std::list<char *>::iterator it = p -> knownImageFields.begin(); it != p -> knownImageFields.end(); it++ ) { \
      SendDlgItemMessage(hwnd,IDDI_IMAGE_FIELD_LIST,CB_ADDSTRING,0L,(LPARAM)(*it));                            \
      if ( p -> szImageField[0] && 0 == strcmp(p -> szImageField,(*it)) )                                      \
         SendDlgItemMessage(hwnd,IDDI_IMAGE_FIELD_LIST,CB_SETCURSEL,                                           \
               (WPARAM)SendDlgItemMessage(hwnd,IDDI_IMAGE_FIELD_LIST,CB_GETCOUNT,0L,0L) - 1,0L);               \
   }                                                                                                           \
   SendDlgItemMessage(hwnd,IDDI_IMAGE_FIELD_LIST,CB_ADDSTRING,0L,(LPARAM)"<none>");                            \
   for ( std::list<char *>::iterator it = p -> knownTextFields.begin(); it != p -> knownTextFields.end(); it++ ) { \
      SendDlgItemMessage(hwnd,IDDI_TEXT_FIELD_LIST,CB_ADDSTRING,0L,(LPARAM)(*it));                             \
      if ( p -> szActionField[0] && 0 == strcmp(p -> szActionField,(*it)) )                                    \
         SendDlgItemMessage(hwnd,IDDI_TEXT_FIELD_LIST,CB_SETCURSEL,                                            \
               (WPARAM)SendDlgItemMessage(hwnd,IDDI_TEXT_FIELD_LIST,CB_GETCOUNT,0L,0L) - 1,0L);                \
   }                                                                                                           \
   SendDlgItemMessage(hwnd,IDDI_TEXT_FIELD_LIST,CB_ADDSTRING,0L,(LPARAM)"<none>");                             \
   PUT_BOOL(p -> doProperties,IDDI_DISPOSITION_SHOW_PROPERTIES);                                               \
   PUT_BOOL(p -> autoSnap,IDDI_AUTO_SNAP);                                             \
   PUT_BOOL(p -> skipImaging,IDDI_SKIP);                                               \
   PUT_LONG(p -> autoFocusTime,IDDI_FOCUS_TIME);                                       \
   PUT_BOOL(p -> snapSecretly,IDDI_SECRET_SNAP);                                       \
   EnableWindow(GetDlgItem(hwnd,IDDI_SECRET_SNAP),p -> autoSnap);                      \
}

#define UNLOAD_CONTROLS \
{  \
   SendMessageW(GetDlgItem(hwnd,IDDI_IMAGER),CB_GETLBTEXT,SendMessageW(GetDlgItem(hwnd,IDDI_IMAGER),CB_GETCURSEL,0L,0L),(LPARAM)p -> pParent -> szwChosenDevice); \
   GetDlgItemText(hwnd,IDDI_IMAGE_FIELD_LIST,p -> szImageField,64); \
   if ( 0 == strcmp(p -> szImageField,"<none>") ) p -> szImageField[0] = '\0'; \
   GetDlgItemText(hwnd,IDDI_TEXT_FIELD_LIST,p -> szActionField,64); \
   if ( 0 == strcmp(p -> szActionField,"<none>") ) p -> szActionField[0] = '\0'; \
   GET_BOOL(p -> doProperties,IDDI_DISPOSITION_SHOW_PROPERTIES);  \
   GET_BOOL(p -> autoSnap,IDDI_AUTO_SNAP);                  \
   GET_BOOL(p -> skipImaging,IDDI_SKIP);                    \
   GET_LONG(p -> autoFocusTime,IDDI_FOCUS_TIME);            \
   GET_BOOL(p -> snapSecretly,IDDI_SECRET_SNAP);            \
   if ( ! p -> autoSnap ) p -> snapSecretly = false;        \
}

   LRESULT CALLBACK VideoBackEnd::VisioLoggerVideoBackEnd::propertiesHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   VideoBackEnd::VisioLoggerVideoBackEnd *p = (VideoBackEnd::VisioLoggerVideoBackEnd *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

   static long controlsLoaded = 0L;

   switch ( msg ) {

   case WM_INITDIALOG: {

      PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);

      p = (VideoBackEnd::VisioLoggerVideoBackEnd *)pPage -> lParam;

      SetWindowLongPtr(hwnd,GWLP_USERDATA,(ULONG_PTR)p);

      p -> pParent -> pIGProperties -> Push();
      p -> pParent -> pIGProperties -> Push();

      if ( 0 < p -> cxImage ) {
         SetWindowPos(GetDlgItem(hwnd,IDDI_VIDEO),NULL,0,0,p -> cxImage,p -> cyImage,SWP_NOMOVE);
         RECT rcImage,rcParent;
         GetWindowRect(hwnd,&rcParent);
         GetWindowRect(GetDlgItem(hwnd,IDDI_VIDEO),&rcImage);
         long x = rcImage.left - rcParent.left;
         long y = rcImage.bottom - rcParent.top + 16;
         SetWindowPos(GetDlgItem(hwnd,IDDI_VIDEO_SNAP),NULL,x,y,0,0,SWP_NOSIZE);
      }

      UDACCEL accelerators[1];
      accelerators[0].nSec = 0;
      accelerators[0].nInc = 100;

      SendDlgItemMessage(hwnd,IDDI_FOCUS_TIME_SPINNER,UDM_SETBUDDY,(WPARAM)GetDlgItem(hwnd,IDDI_FOCUS_TIME),0L);
      SendDlgItemMessage(hwnd,IDDI_FOCUS_TIME_SPINNER,UDM_SETRANGE,0L,MAKELPARAM(10000,0));
      SendDlgItemMessage(hwnd,IDDI_FOCUS_TIME_SPINNER,UDM_SETACCEL,1L,(LPARAM)accelerators);

      EnableWindow(GetDlgItem(hwnd,IDDI_FOCUS_TIME),p -> autoSnap);
      EnableWindow(GetDlgItem(hwnd,IDDI_FOCUS_TIME_SPINNER),p -> autoSnap);

      LOAD_CONTROLS

      ShowWindow(GetDlgItem(hwnd,IDDI_SET_WARNINGS_DISPLAY),p -> showNoPictureWarning ? SW_HIDE : SW_SHOW);

      if ( ! p -> isProcessing ) {
         ShowWindow(GetDlgItem(hwnd,IDDI_VIDEO_SNAP),SW_HIDE);
         PostMessage(hwnd,WM_START_VIDEO,0L,0L);
      } else 
         SetFocus(GetDlgItem(hwnd,IDDI_VIDEO_SNAP));

      if ( p -> isProcessing && p -> autoSnap ) {
         waitingForImage = true;
         skipToDraw = true;
         PostMessage(hwnd,WM_COMMAND,MAKEWPARAM(IDDI_VIDEO_SNAP,0L),0L);
      }

      if ( p -> isProcessing )
         PostMessage(hwnd,WM_START_VIDEO,0L,0L);

      if ( p -> snapSecretly ) 
         SetWindowPos(hwnd,HWND_BOTTOM,16000,0,0,0,SWP_NOSIZE);

      }
      return LRESULT(FALSE);


   case WM_START_VIDEO: {

      SendMessage(hwnd,WM_STOP_VIDEO,0L,0L);

      if ( ! p -> pParent -> szwChosenDevice[0] )
         break;

      if ( ! p -> pParent -> hostVideo(GetDlgItem(hwnd,IDDI_VIDEO)) )
         break;

      waitingForImage = false;

      }
      break;

   case WM_DESTROY:
   case WM_STOP_VIDEO:
      p -> pParent -> unHostVideo();
      break;

   case WM_TIMER: {
      if ( WM_TIMER_ALLOW_DRAW == wParam ) {
         skipToDraw = false;
         KillTimer(hwnd,WM_TIMER_ALLOW_DRAW);
         PostMessage(hwnd,WM_COMMAND,MAKEWPARAM(IDDI_VIDEO_SNAP,0L),0L);
      }
      }
      break;

   case WM_COMMAND: {

      switch ( LOWORD(wParam) ) {

      case IDDI_NO_PICTURE_OK:
         if ( BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_PROHIBIT_MESSAGE,BM_GETCHECK,0L,0L) ) {
            p -> showNoPictureWarning = false;
            p -> pParent -> pIGProperties -> Save();
         }
         EndDialog(hwnd,0);
         break;

      case IDDI_AUTO_SNAP:
         p -> autoSnap = BST_CHECKED == SendDlgItemMessage(hwnd,IDDI_AUTO_SNAP,BM_GETCHECK,0L,0L);
         EnableWindow(GetDlgItem(hwnd,IDDI_FOCUS_TIME),p -> autoSnap);
         EnableWindow(GetDlgItem(hwnd,IDDI_FOCUS_TIME_SPINNER),p -> autoSnap);
         EnableWindow(GetDlgItem(hwnd,IDDI_SECRET_SNAP),p -> autoSnap);
         break;

      case IDDI_IMAGER: {
         if ( ! (HIWORD(wParam) == CBN_SELCHANGE) )
            break;
         long index = SendMessage(GetDlgItem(hwnd,IDDI_IMAGER),CB_GETCURSEL,0L,0L);
         if ( CB_ERR == index )
            break;
         SendMessageW(GetDlgItem(hwnd,IDDI_IMAGER),CB_GETLBTEXT,(WPARAM)index,(LPARAM)p -> pParent -> szwChosenDevice);
         PostMessage(hwnd,WM_START_VIDEO,0L,0L);
         }
         break;

      case IDDI_VIDEO_SNAP: {

         if ( waitingForImage ) {
            PostMessage(hwnd,msg,wParam,lParam);
            break;
         } else if ( skipToDraw ) {
            SetTimer(hwnd,WM_TIMER_ALLOW_DRAW,p -> autoFocusTime,NULL);
            break;
         }

         BYTE *pbImage;
         HRESULT hr = pIVMRWindowlessControl -> GetCurrentImage(&pbImage);
         sprintf(p -> szTargetFile,"%s.bmp",_tempnam(NULL,NULL));
         SaveImage(pbImage,p -> szTargetFile);
         CoTaskMemFree((void *)pbImage);
         SendMessage(GetParent(hwnd),PSM_REMOVEPAGE,0L,SendMessage(GetParent(hwnd),PSM_INDEXTOPAGE,0L,0L));

         if ( p -> snapSecretly ) {
            EndDialog(hwnd,0);
            break;
         }

         }
         break;

      default:
         break;
      }

      }
      break;

   case WM_NOTIFY: {

      NMHDR *pNotifyHeader = (NMHDR *)lParam;

      switch ( pNotifyHeader -> code ) {

      case PSN_KILLACTIVE: {

         UNLOAD_CONTROLS

         SetWindowLongPtr(hwnd,DWLP_MSGRESULT,FALSE);

         }
         break;

      case PSN_APPLY: {

         PSHNOTIFY *pNotify = (PSHNOTIFY *)lParam;

         UNLOAD_CONTROLS

         if ( pNotify -> lParam ) {
            if ( ! ( NULL == GetDlgItem(hwnd,IDDI_SET_WARNINGS_DISPLAY) ) ) {
               if ( BST_CHECKED == SendMessage(GetDlgItem(hwnd,IDDI_SET_WARNINGS_DISPLAY),BM_GETCHECK,0L,0L) )
                  p -> showNoPictureWarning = true;
            }
            p -> pParent -> pIGProperties -> Save();
            p -> pParent -> pIGProperties -> Discard();
            p -> pParent -> pIGProperties -> Discard();
         } else {
            p -> pParent -> pIGProperties -> Discard();
            p -> pParent -> pIGProperties -> Push();
         }

         SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_NOERROR);

         return (LRESULT)TRUE;
         }
         break;

      case PSN_RESET: {
         p -> pParent -> pIGProperties -> Pop();
         p -> pParent -> pIGProperties -> Pop();
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

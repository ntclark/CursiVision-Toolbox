/*
                       Copyright (c) 2009,2010 Nathan T. Clark
*/

#include "NamingBackEnd.h"

#include <shellapi.h>
#include <shTypes.h>
#include <shlobj.h>

#define OBJECT_WITH_PROPERTIES NamingBackEnd
#define USE_PROPSHEETS

#include "..\dispositionSettingsDefines.h"

#define ROLLUP_ASSOCIATE_PROCESS

#define LOAD_ADDITIONAL \
{  \
   PUT_STRING(pObject -> szNamePrefix,IDDI_NAME_PREFIX)  \
}

#define UNLOAD_ADDITIONAL \
{  \
   GET_STRING(pObject -> szNamePrefix,IDDI_NAME_PREFIX) \
}

#define REGISTER_TOOLTIPS_ADDITIONAL(hInst) \
   REGISTER_TOOLTIP(hInst,IDDI_NAME_PREFIX)

   LRESULT CALLBACK NamingBackEnd::dispositionSettingsHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   resultDisposition *p = (resultDisposition *)GetWindowLong(hwnd,GWL_USERDATA);

#include "..\dispositionSettingsBody.cpp"

   return (LRESULT)FALSE;
   }



   BOOL CALLBACK doDisable(HWND hwndTest,LPARAM lParam);
   BOOL CALLBACK doEnable(HWND hwndTest,LPARAM lParam);
   BOOL CALLBACK doMoveUp(HWND hwndTest,LPARAM lParam);
   BOOL CALLBACK doHide(HWND hwndTest,LPARAM lParam);
   BOOL CALLBACK doShow(HWND hwndTest,LPARAM lParam);

   extern "C" void disableAll(HWND hwnd,long *pExceptions) {
   EnumChildWindows(hwnd,doDisable,(LPARAM)pExceptions);
   return;
   }

   extern "C" void enableAll(HWND hwnd,long *pExceptions) {
   EnumChildWindows(hwnd,doEnable,(LPARAM)pExceptions);
   return;
   }

   extern "C" void moveUpAll(HWND hwnd,long *pExceptions) {
   EnumChildWindows(hwnd,doMoveUp,(LPARAM)pExceptions);
   return;
   }

   extern "C" void hideAll(HWND hwnd,long *pExceptions) {
   EnumChildWindows(hwnd,doHide,(LPARAM)pExceptions);
   return;
   }

   extern "C" void showAll(HWND hwnd,long *pExceptions) {
   EnumChildWindows(hwnd,doShow,(LPARAM)pExceptions);
   return;
   }

   BOOL CALLBACK doDisable(HWND hwndTest,LPARAM lParam) {
   long *pExceptions = (long *)lParam;
   long id = GetWindowLong(hwndTest,GWL_ID);
   for ( long k = 0; 1; k++ ) {
      if ( ! pExceptions[k] )
         break;
      if ( id == pExceptions[k] ) {
         EnableWindow(hwndTest,TRUE);
         return TRUE;
      }
   }
   EnableWindow(hwndTest,FALSE);
   return TRUE;
   }


   BOOL CALLBACK doEnable(HWND hwndTest,LPARAM lParam) {
   long *pExceptions = (long *)lParam;
   long id = GetWindowLong(hwndTest,GWL_ID);
   for ( long k = 0; 1; k++ ) {
      if ( ! pExceptions[k] )
         break;
      if ( id == pExceptions[k] ) {
         EnableWindow(hwndTest,FALSE);
         return TRUE;
      }
   }
   EnableWindow(hwndTest,TRUE);
   return TRUE;
   }


   BOOL CALLBACK doMoveUp(HWND hwndTest,LPARAM lParam) {
   long *pExceptions = (long *)lParam;
   long id = GetWindowLong(hwndTest,GWL_ID);
   for ( long k = 0; 1; k++ ) {
      if ( ! pExceptions[k] )
         break;
      if ( id == pExceptions[k] ) {
         return TRUE;
      }
   }
   RECT rcParent,rcCurrent;
   GetWindowRect(GetParent(hwndTest),&rcParent);
   GetWindowRect(hwndTest,&rcCurrent);
   SetWindowPos(hwndTest,NULL,rcCurrent.left - rcParent.left,rcCurrent.top - 32 - rcParent.top,0,0,SWP_NOZORDER | SWP_NOSIZE);
   return TRUE;
   }

   BOOL CALLBACK doHide(HWND hwndTest,LPARAM lParam) {
   long *pExceptions = (long *)lParam;
   long id = GetWindowLong(hwndTest,GWL_ID);
   for ( long k = 0; 1; k++ ) {
      if ( ! pExceptions[k] )
         break;
      if ( id == pExceptions[k] ) {
         return TRUE;
      }
   }
   ShowWindow(hwndTest,SW_HIDE);
   return TRUE;
   }

   BOOL CALLBACK doShow(HWND hwndTest,LPARAM lParam) {
   long *pExceptions = (long *)lParam;
   long id = GetWindowLong(hwndTest,GWL_ID);
   for ( long k = 0; 1; k++ ) {
      if ( ! pExceptions[k] )
         break;
      if ( id == pExceptions[k] ) {
         return TRUE;
      }
   }
   ShowWindow(hwndTest,SW_SHOW);
   return TRUE;
   }
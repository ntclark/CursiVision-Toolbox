/*
                       Copyright (c) 2009,2010 Nathan T. Clark
*/

#include "DocumentStorage.h"

#include <shellapi.h>
#include <shTypes.h>
#include <shlobj.h>

static HWND propertySheetWindow = NULL;

   LRESULT CALLBACK DocumentStorage::dispositionSettingsHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   DocumentStorage *p = (DocumentStorage *)GetWindowLong(hwnd,GWL_USERDATA);

   switch ( msg ) {

   case WM_INITDIALOG: {

      PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);

      p = (DocumentStorage *)pPage -> lParam;

      SetWindowLong(hwnd,GWL_USERDATA,(long)p);

      p -> PushProperties();
      p -> PushProperties();

      char szTemp[1024];
      LoadString(hModule,IDDI_GENERAL_DESCRIPTION,szTemp,1024);

      SetDlgItemText(hwnd,IDDI_GENERAL_DESCRIPTION,szTemp);

      RECT rcCurrent,rcDialog;
      GetWindowRect(hwnd,&rcDialog);
      GetWindowRect(GetDlgItem(hwnd,IDDI_GENERAL_DESCRIPTION),&rcCurrent);
      SetWindowPos(GetDlgItem(hwnd,IDDI_GENERAL_DESCRIPTION),HWND_TOP,16,16,rcDialog.right - rcDialog.left - 32,rcCurrent.bottom - rcCurrent.top,0L);

      SetDlgItemText(hwnd,IDDI_DATABASE_LOCATION,p -> szDatabaseDirectory);

      propertySheetWindow = hwnd;

      }
      return LRESULT(FALSE);


   case WM_COMMAND: {

      switch ( LOWORD(wParam) ) {
      case IDDI_DATABASE_LOCATION_GET: {

         BROWSEINFO bi;
         char file[1024];
         memset(file,0,sizeof(file));
         memset(&bi,0,sizeof(BROWSEINFO));
         bi.hwndOwner = hwnd;
         bi.pszDisplayName = file;
         bi.lpszTitle = "Browse to the desired database directory";
         bi.ulFlags = BIF_NEWDIALOGSTYLE;
         bi.lpfn = DocumentStorage::initializeBrowse;
         bi.lParam = (LPARAM)p;
         ITEMIDLIST *pPIDL = SHBrowseForFolder(&bi);

         if ( NULL == bi.pszDisplayName[0] )
            break;

         char szDirectory[MAX_PATH];

         SHGetPathFromIDList(pPIDL,szDirectory);

         SetDlgItemText(hwnd,IDDI_DATABASE_LOCATION,szDirectory);

         }
         break;

      default: 
         break;
      }
      
      }
      break;


   case WM_NOTIFY: {

//      OBJECT_WITH_PROPERTIES *pObject = (OBJECT_WITH_PROPERTIES *)(p -> pParent);

      NMHDR *pNotifyHeader = (NMHDR *)lParam;

      switch ( pNotifyHeader -> code ) {

      case PSN_SETACTIVE: {

//         PostMessage(hwnd,WM_COMMAND,MAKEWPARAM(IDDI_DISPOSITION_ASSOCIATE_PRC,0L),0L);

//         PostMessage(hwnd,WM_USER + 1,0L,0L);

         }
         break;

      case PSN_KILLACTIVE: {
         SetWindowLong(hwnd,DWL_MSGRESULT,FALSE);
         }
         break;

      case PSN_APPLY: {

         PSHNOTIFY *pNotify = (PSHNOTIFY *)lParam;

         SetWindowLong(hwnd,DWL_MSGRESULT,PSNRET_NOERROR);

         GetDlgItemText(hwnd,IDDI_DATABASE_LOCATION,p -> szDatabaseDirectory,sizeof(p -> szDatabaseDirectory));

         return (LRESULT)TRUE;
         }
         break;

      case PSN_RESET: {
         p -> PopProperties();
         p -> PopProperties();
         }
         break;

      }

      }
      break;

   default:
      break;
   }


   return (LRESULT)FALSE;
   }


   int CALLBACK DocumentStorage::initializeBrowse(HWND hwnd,UINT msg,LPARAM lParam,LPARAM lpData) {

   DocumentStorage *p = (DocumentStorage *)lpData;

   switch ( msg) {
   case BFFM_INITIALIZED: {

      char szTemp[MAX_PATH];

      GetDlgItemText(propertySheetWindow,IDDI_DATABASE_LOCATION,szTemp,sizeof(szTemp));

      WIN32_FIND_DATA findData;
      memset(&findData,0,sizeof(WIN32_FIND_DATA));

      HANDLE filesHandle = FindFirstFile(szTemp,&findData);

      if ( INVALID_HANDLE_VALUE != filesHandle )
         SendMessage(hwnd,BFFM_SETSELECTION,(WPARAM)TRUE,(LPARAM)szTemp);

      FindClose(filesHandle);

      }
      break;

   default:
      break;
   }

   return 0L;
   }
   
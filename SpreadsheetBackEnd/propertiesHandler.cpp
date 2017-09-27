/*
                       Copyright (c) 2009,2010 Nathan T. Clark
*/

#include "SpreadsheetBackEnd.h"
#include <commctrl.h>


extern "C" int GetDocumentsLocation(HWND hwnd,char *);

#define PUT_BOOL(v,id)  SendDlgItemMessage(hwnd,id,BM_SETCHECK,v ? BST_CHECKED : BST_UNCHECKED,0L);
#define PUT_LONG(v,id)  { char szX[32]; sprintf(szX,"%ld",v); SetDlgItemText(hwnd,id,szX); }
#define PUT_STRING(v,id) SetDlgItemText(hwnd,id,v);
#define DEFAULT_LONG(v,def) { if ( 0 == v ) v = def; };

#define GET_BOOL(v,id)  v = (BST_CHECKED == SendDlgItemMessage(hwnd,id,BM_GETCHECK,0L,0L));
#define GET_LONG(v,id) {char szX[32]; GetDlgItemText(hwnd,id,szX,32); v = atol(szX); }
#define GET_STRING(v,id) GetDlgItemText(hwnd,id,v,MAX_PATH);

#define LOAD_CONTROLS \
{  \
   PUT_BOOL(p -> doProperties,IDDI_SHOW_PROPERTIES)                                             \
   PUT_BOOL(pObject -> includeDate,IDDI_INCLUDE_DATE)                                           \
   PUT_BOOL(pObject -> includeTime,IDDI_INCLUDE_TIME)                                           \
   PUT_BOOL(pObject -> includeDocumentLink,IDDI_INCLUDE_DOCUMENT_LINK)                          \
   PUT_STRING(pObject -> szWorkbookName,IDDI_WORKBOOK_NAME)                                     \
   PUT_STRING(pObject -> szDateColumn,IDDI_DATE_COLUMN_ID)                                      \
   PUT_STRING(pObject -> szTimeColumn,IDDI_TIME_COLUMN_ID)                                      \
   PUT_STRING(pObject -> szDocumentLinkColumn,IDDI_DOCUMENT_LINK_COLUMN)                        \
   for ( long k = 0; k < FIELD_DISPLAY_COUNT; k++ ) PUT_STRING(pObject -> szNamePrefix[k],IDDI_NAME_PREFIX + k)  \
   for ( long k = 0; k < FIELD_DISPLAY_COUNT; k++ ) PUT_STRING(pObject -> szColumnName[k],IDDI_COLUMN_NAME + k)  \
   for ( long k = 0; k < FIELD_DISPLAY_COUNT; k++ ) PUT_STRING(pObject -> szColumnId[k],IDDI_COLUMN_ID + k)  \
   char *pNames = pObject -> szAllSheetNames;                                                   \
   while ( *pNames ) {                                                                          \
      long index = SendMessage(GetDlgItem(hwnd,IDDI_CHOOSE_SPREADSHEET),CB_ADDSTRING,0L,(LPARAM)pNames); \
      if ( 0 == strcmp(pNames,pObject -> szSpreadsheetName) )                                   \
         SendMessage(GetDlgItem(hwnd,IDDI_CHOOSE_SPREADSHEET),CB_SETCURSEL,(WPARAM)index,0L);   \
      pNames += strlen(pNames) + 1;                                                             \
   }                                                                                            \
}

#define UNLOAD_CONTROLS \
{  \
   GET_BOOL(p -> doProperties,IDDI_SHOW_PROPERTIES)                                                      \
   GET_BOOL(pObject -> includeDate,IDDI_INCLUDE_DATE)                                                    \
   GET_BOOL(pObject -> includeTime,IDDI_INCLUDE_TIME)                                                    \
   GET_BOOL(pObject -> includeDocumentLink,IDDI_INCLUDE_DOCUMENT_LINK)                                   \
   GET_STRING(pObject -> szWorkbookName,IDDI_WORKBOOK_NAME)                                              \
   GET_STRING(pObject -> szDateColumn,IDDI_DATE_COLUMN_ID)                                               \
   GET_STRING(pObject -> szTimeColumn,IDDI_TIME_COLUMN_ID)                                               \
   GET_STRING(pObject -> szDocumentLinkColumn,IDDI_DOCUMENT_LINK_COLUMN)                                 \
   for ( long k = 0; k < FIELD_DISPLAY_COUNT; k++ ) GET_STRING(pObject -> szNamePrefix[k],IDDI_NAME_PREFIX + k)  \
   for ( long k = 0; k < FIELD_DISPLAY_COUNT; k++ ) GET_STRING(pObject -> szColumnName[k],IDDI_COLUMN_NAME + k)  \
   for ( long k = 0; k < FIELD_DISPLAY_COUNT; k++ ) GET_STRING(pObject -> szColumnId[k],IDDI_COLUMN_ID + k)      \
   SendMessage(GetDlgItem(hwnd,IDDI_CHOOSE_SPREADSHEET),CB_GETLBTEXT,(WPARAM)SendMessage(GetDlgItem(hwnd,IDDI_CHOOSE_SPREADSHEET),CB_GETCURSEL,0L,0L),(LPARAM)pObject -> szSpreadsheetName); \
}

   LRESULT CALLBACK SpreadsheetBackEnd::propertiesHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   resultDisposition *p = (resultDisposition *)GetWindowLong(hwnd,GWL_USERDATA);

   static long controlsLoaded = 0L;

   switch ( msg ) {

   case WM_INITDIALOG: {

      PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);
      p = (resultDisposition *)pPage -> lParam;
      SetWindowLong(hwnd,GWL_USERDATA,(long)p);

      SpreadsheetBackEnd *pObject = (SpreadsheetBackEnd *)(p -> pParent);

      pObject -> PushProperties();

      pObject -> PushProperties();

      RECT rcParent,rcPrefix,rcColumnName,rcColumnId;

      HFONT hGUIFont = (HFONT)SendMessage(GetDlgItem(hwnd,IDDI_NAME_PREFIX),WM_GETFONT,0L,0L);

      GetWindowRect(hwnd,&rcParent);
      GetWindowRect(GetDlgItem(hwnd,IDDI_NAME_PREFIX),&rcPrefix);
      GetWindowRect(GetDlgItem(hwnd,IDDI_COLUMN_NAME),&rcColumnName);
      GetWindowRect(GetDlgItem(hwnd,IDDI_COLUMN_ID),&rcColumnId);

      rcPrefix.left -= rcParent.left;
      rcPrefix.right -= rcParent.left;
      rcPrefix.top -= rcParent.top;
      rcPrefix.bottom -= rcParent.top;

      rcColumnName.left -= rcParent.left;
      rcColumnName.right -= rcParent.left;
      rcColumnName.top -= rcParent.top;
      rcColumnName.bottom -= rcParent.top;

      rcColumnId.left -= rcParent.left;
      rcColumnId.right -= rcParent.left;
      rcColumnId.top -= rcParent.top;
      rcColumnId.bottom -= rcParent.top;

      long cxPrefix = rcPrefix.right - rcPrefix.left;
      long cyPrefix = rcPrefix.bottom - rcPrefix.top;
      long cxColumnName = rcColumnName.right - rcColumnName.left;
      long cyColumnName = rcColumnName.bottom - rcColumnName.top;
      long cxColumnId = rcColumnId.right - rcColumnId.left;
      long cyColumnId = rcColumnId.bottom - rcColumnId.top;
      long y = rcPrefix.top + cyPrefix + 4;

      for ( long k = 0; k < FIELD_DISPLAY_COUNT - 1; k++ ) {
         HWND hwndx = CreateWindowEx(WS_EX_CLIENTEDGE,"Edit","",WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,rcPrefix.left,y,cxPrefix,cyPrefix,hwnd,(HMENU)(IDDI_NAME_PREFIX + k + 1),NULL,NULL);
         SendMessage(hwndx,WM_SETFONT,(WPARAM)hGUIFont,(LPARAM)TRUE);
         hwndx = CreateWindowEx(WS_EX_CLIENTEDGE,"Edit","",WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,rcColumnName.left,y,cxColumnName,cyColumnName,hwnd,(HMENU)(IDDI_COLUMN_NAME + k + 1),NULL,NULL);
         SendMessage(hwndx,WM_SETFONT,(WPARAM)hGUIFont,(LPARAM)TRUE);
         hwndx = CreateWindowEx(WS_EX_CLIENTEDGE,"Edit","",WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER | WS_TABSTOP,rcColumnId.left,y,cxColumnId,cyColumnId,hwnd,(HMENU)(IDDI_COLUMN_ID + k + 1),NULL,NULL);
         SendMessage(hwndx,WM_SETFONT,(WPARAM)hGUIFont,(LPARAM)TRUE);
         y += cyPrefix + 4;
      }

      SetWindowPos(GetDlgItem(hwnd,IDDI_INCLUDE_DATE),HWND_TOP,rcPrefix.left,y + 4,0,0,SWP_NOSIZE);
      SetWindowPos(GetDlgItem(hwnd,IDDI_INCLUDE_TIME),HWND_TOP,rcPrefix.left,y + 4 + cyPrefix + 4,0,0,SWP_NOSIZE);

      RECT rcIncludeDate,rcIncludeDateLabel;

      GetWindowRect(GetDlgItem(hwnd,IDDI_INCLUDE_DATE),&rcIncludeDate);
      GetWindowRect(GetDlgItem(hwnd,IDDI_INCLUDE_DATE_LABEL),&rcIncludeDateLabel);

      long cx = rcIncludeDate.right - rcIncludeDate.left;

      SetWindowPos(GetDlgItem(hwnd,IDDI_INCLUDE_DATE_LABEL),HWND_TOP,rcPrefix.left + cx + 4,y + 8,0,0,SWP_NOSIZE);
      SetWindowPos(GetDlgItem(hwnd,IDDI_DATE_COLUMN_ID),HWND_TOP,rcPrefix.left + cx + rcIncludeDateLabel.right - rcIncludeDateLabel.left + 8,y + 6,0,0,SWP_NOSIZE);

      SetWindowPos(GetDlgItem(hwnd,IDDI_INCLUDE_TIME_LABEL),HWND_TOP,rcPrefix.left + cx + 4,y + 8 + cyPrefix + 4,0,0,SWP_NOSIZE);
      SetWindowPos(GetDlgItem(hwnd,IDDI_TIME_COLUMN_ID),HWND_TOP,rcPrefix.left + cx + rcIncludeDateLabel.right - rcIncludeDateLabel.left + 8,y + 6 + cyPrefix + 4,0,0,SWP_NOSIZE);

      GetWindowRect(GetDlgItem(hwnd,IDDI_INCLUDE_DOCUMENT_LINK),&rcIncludeDate);
      GetWindowRect(GetDlgItem(hwnd,IDDI_DOCUMENT_LINK_LABEL),&rcIncludeDateLabel);

      cx = rcIncludeDate.right - rcIncludeDate.left;

      SetWindowPos(GetDlgItem(hwnd,IDDI_INCLUDE_DOCUMENT_LINK),HWND_TOP,rcPrefix.left,y + 6 + 2 * cyPrefix + 8,0,0,SWP_NOSIZE);
      SetWindowPos(GetDlgItem(hwnd,IDDI_DOCUMENT_LINK_LABEL),HWND_TOP,rcPrefix.left + cx + 4,y + 6 + 2 * cyPrefix + 12,0,0,SWP_NOSIZE);
      SetWindowPos(GetDlgItem(hwnd,IDDI_DOCUMENT_LINK_COLUMN),HWND_TOP,rcPrefix.left + cx + rcIncludeDateLabel.right - rcIncludeDateLabel.left + 8,y + 6 + 2 * cyPrefix + 10,0,0,SWP_NOSIZE);

      controlsLoaded = 0L;
   
      LOAD_CONTROLS

      }
      return (LRESULT)TRUE;

   case WM_COMMAND: {

      SpreadsheetBackEnd *pObject = (SpreadsheetBackEnd *)(p -> pParent);

      switch ( LOWORD(wParam) ) {

      case IDDI_WORKBOOK_GET: {

         OPENFILENAME openFileName = {0};
         char szFilter[MAX_PATH],szFile[MAX_PATH];

         memset(szFilter,0,sizeof(szFilter));
         memset(szFile,0,sizeof(szFile));

         sprintf(szFilter,"Excel Workbooks (*.xls;*.xlsx)");
         long k = strlen(szFilter) + sprintf(szFilter + strlen(szFilter) + 1,"*.xl*");
         k = k + sprintf(szFilter + k + 2,"All Files");
         sprintf(szFilter + k + 3,"*.*");
 
         openFileName.lStructSize = sizeof(OPENFILENAME);
         openFileName.hwndOwner = hwnd;
         openFileName.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
         openFileName.lpstrFilter = szFilter;
         openFileName.lpstrFile = szFile;
         openFileName.lpstrDefExt = "";//"xls";
         openFileName.nFilterIndex = 1;
         openFileName.nMaxFile = MAX_PATH;
         openFileName.lpstrTitle = "Select the Microsoft Excel Workbook";
         openFileName.lpstrInitialDir = szUserDirectory;

         if ( ! GetOpenFileName(&openFileName) ) 
            break;

         SetDlgItemText(hwnd,IDDI_WORKBOOK_NAME,szFile);

         long countSheets = pObject -> getExcelWorksheets(szFile);

         if ( ! countSheets )
            break;

         char *pNames = pObject -> szAllSheetNames;

         SendDlgItemMessage(hwnd,IDDI_CHOOSE_SPREADSHEET,CB_RESETCONTENT,0L,0L);

         bool wasFound = false;
         for ( long k = 0; k < countSheets; k++ ) {
            long index = SendMessage(GetDlgItem(hwnd,IDDI_CHOOSE_SPREADSHEET),CB_ADDSTRING,0L,(LPARAM)pNames);
            if ( 0 == strcmp(pNames,pObject -> szSpreadsheetName) ) {
               SendMessage(GetDlgItem(hwnd,IDDI_CHOOSE_SPREADSHEET),CB_SETCURSEL,(WPARAM)index,0L);
               wasFound = true;
            }
            pNames = pNames + strlen(pNames) + 1;
         }

         if ( ! wasFound )
            SendMessage(GetDlgItem(hwnd,IDDI_CHOOSE_SPREADSHEET),CB_SETCURSEL,(WPARAM)0L,0L);

         }
         break;

      default:
         break;
      }

      }
      break;

   case WM_NOTIFY: {

      SpreadsheetBackEnd *pObject = (SpreadsheetBackEnd *)(p -> pParent);

      NMHDR *pNotifyHeader = (NMHDR *)lParam;

      switch ( pNotifyHeader -> code ) {

      case PSN_QUERYINITIALFOCUS:
         SetWindowLong(hwnd,DWL_MSGRESULT,(long)GetDlgItem(hwnd,IDDI_SHOW_PROPERTIES));
         return (LRESULT)TRUE;

      case PSN_SETACTIVE: {
         }
         break;

      case PSN_KILLACTIVE: {

         UNLOAD_CONTROLS

         SetWindowLong(hwnd,DWL_MSGRESULT,FALSE);

         }
         break;

      case PSN_APPLY: {

         PSHNOTIFY *pNotify = (PSHNOTIFY *)lParam;

         UNLOAD_CONTROLS

         if ( pNotify -> lParam ) {
            pObject -> SaveProperties();
            pObject -> DiscardProperties();
            pObject -> DiscardProperties();
         } else {
            pObject -> DiscardProperties();
            pObject -> PushProperties();
         }

         SetWindowLong(hwnd,DWL_MSGRESULT,PSNRET_NOERROR);

         return (LRESULT)TRUE;
         }
         break;

      case PSN_RESET: {
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

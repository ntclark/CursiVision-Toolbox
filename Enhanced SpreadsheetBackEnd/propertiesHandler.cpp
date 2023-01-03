
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
      long index = (long)SendMessage(GetDlgItem(hwnd,IDDI_CHOOSE_SPREADSHEET),CB_ADDSTRING,0L,(LPARAM)pNames); \
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
   for ( long k = 0; k < FIELD_DISPLAY_COUNT; k++ ) if ( 0 == strcmp(pObject -> szNamePrefix[k],"<none>") ) pObject -> szNamePrefix[k][0] = '\0';    \
}

    static boolean needsAdmin = false;
    LRESULT CALLBACK redTextHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
    WNDPROC defaultTextHandler{NULL};

    LRESULT CALLBACK SpreadsheetBackEnd::propertiesHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

    resultDisposition *p = (resultDisposition *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

    static long controlsLoaded = 0L;

    switch ( msg ) {

    case WM_INITDIALOG: {

        PROPSHEETPAGE *pPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);
        p = (resultDisposition *)pPage -> lParam;
        SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)p);

        SpreadsheetBackEnd *pObject = (SpreadsheetBackEnd *)(p -> pParent);

        pObject -> PushProperties();
        pObject -> PushProperties();

        needsAdmin = false;

        IPrintingSupportProfile *px = NULL;
        pObject -> pICursiVisionServices -> get_PrintingSupportProfile(&px);

        if ( px && ! px -> AllowPrintProfileChanges() && ! pObject -> editAllowed ) {
            SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE);
            SetDlgItemText(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES,"Changes are disabled because Admin privileges are required to change print profiles");
            EnableWindow(hwnd,FALSE);
            needsAdmin = true;
        } else {
            if ( ! pObject -> pICursiVisionServices -> AllowToolboxPropertyChanges() && ! pObject -> editAllowed ) {
                SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE);
                SetDlgItemText(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES,"Changes are disabled because Admin privileges are required to change tool properties");
                EnableWindow(hwnd,FALSE);
                needsAdmin = true;
            } else
                ShowWindow(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),SW_HIDE);
        }

        if ( needsAdmin ) {
            moveUpAllAmount(hwnd,-16,NULL);
            enableDisableSiblings(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),FALSE);
            SetWindowPos(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE);
            if ( NULL == defaultTextHandler )
                defaultTextHandler = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),GWLP_WNDPROC,(UINT_PTR)redTextHandler);
            else
                SetWindowLongPtr(GetDlgItem(hwnd,IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES),GWLP_WNDPROC,(UINT_PTR)redTextHandler);
        }

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

        long y = rcPrefix.top + cyPrefix + 4 - cyPrefix;

        DestroyWindow(GetDlgItem(hwnd,IDDI_NAME_PREFIX));
        DestroyWindow(GetDlgItem(hwnd,IDDI_COLUMN_NAME));
        DestroyWindow(GetDlgItem(hwnd,IDDI_COLUMN_ID));

        char *pFieldNames = NULL;
        long fieldCount = 0L;

        if ( pObject -> pICursiVisionServices ) {
            pFieldNames = pObject -> pICursiVisionServices -> FieldLabels();
            fieldCount = pObject -> pICursiVisionServices -> FieldCount();
        }

        for ( long k = 0; k < FIELD_DISPLAY_COUNT - 1; k++ ) {

            HWND hwndx = CreateWindowEx(0L,"ComboBox","",WS_CHILD | WS_VSCROLL | WS_VISIBLE | ES_AUTOVSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST,
                                                rcPrefix.left,y,cxPrefix,6 * cyPrefix,hwnd,(HMENU)(UINT_PTR)(IDDI_NAME_PREFIX + k),NULL,NULL);
            SendMessage(hwndx,WM_SETFONT,(WPARAM)hGUIFont,(LPARAM)TRUE);

            EnableWindow(hwndx,needsAdmin ? FALSE : TRUE);

            char *pStart = pFieldNames;

            for ( long j = 0; j < fieldCount; j++, pStart += 32 ) {

                if ( pStart[0] ) {
                    SendMessage(hwndx,CB_INSERTSTRING,(WPARAM)-1L,(LPARAM)pStart);
                    if ( 0 < strlen(pObject -> szNamePrefix[k]) && 0 == strcmp(pObject -> szNamePrefix[k],pStart) )
                        SendMessage(hwndx,CB_SETCURSEL,(WPARAM)SendMessage(hwndx,CB_GETCOUNT,0L,0L) - 1,0L);
                }

            }

            SendMessage(hwndx,CB_INSERTSTRING,(WPARAM)-1L,(LPARAM)"<none>");

            hwndx = CreateWindowEx(WS_EX_CLIENTEDGE,"Edit","",WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,rcColumnName.left,y,cxColumnName,cyColumnName,hwnd,(HMENU)(UINT_PTR)(IDDI_COLUMN_NAME + k),NULL,NULL);
            SendMessage(hwndx,WM_SETFONT,(WPARAM)hGUIFont,(LPARAM)TRUE);
            EnableWindow(hwndx,needsAdmin ? FALSE : TRUE);

            hwndx = CreateWindowEx(WS_EX_CLIENTEDGE,"Edit","",WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER | WS_TABSTOP,rcColumnId.left,y,cxColumnId,cyColumnId,hwnd,(HMENU)(UINT_PTR)(IDDI_COLUMN_ID + k),NULL,NULL);
            SendMessage(hwndx,WM_SETFONT,(WPARAM)hGUIFont,(LPARAM)TRUE);
            EnableWindow(hwndx,needsAdmin ? FALSE : TRUE);

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
            long k = (long)strlen(szFilter) + sprintf(szFilter + (long)strlen(szFilter) + 1,"*.xl*");
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
                long index = (long)SendMessage(GetDlgItem(hwnd,IDDI_CHOOSE_SPREADSHEET),CB_ADDSTRING,0L,(LPARAM)pNames);
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
            SetWindowLongPtr(hwnd,DWLP_MSGRESULT,(LONG_PTR)GetDlgItem(hwnd,IDDI_SHOW_PROPERTIES));
            return (LRESULT)TRUE;

        case PSN_SETACTIVE: {
            }
            break;

        case PSN_KILLACTIVE: {

            UNLOAD_CONTROLS

            SetWindowLongPtr(hwnd,DWLP_MSGRESULT,FALSE);

            }
            break;

        case PSN_APPLY: {

            PSHNOTIFY *pNotify = (PSHNOTIFY *)lParam;

            UNLOAD_CONTROLS

            if ( pNotify -> lParam && ! needsAdmin ) {
                pObject -> SaveProperties();
                pObject -> DiscardProperties();
                pObject -> DiscardProperties();
            } else {
                pObject -> DiscardProperties();
                pObject -> PushProperties();
            }

            SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_NOERROR);

            }
            return (LRESULT)TRUE;

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


    LRESULT CALLBACK SpreadsheetBackEnd::noProfileHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

    switch ( msg ) {
    case WM_INITDIALOG: {
        RECT rcClient;
        char szMessage[2048];
        LoadString(hModule,IDS_NO_PROFILE,szMessage,2048);
        GetWindowRect(hwnd,&rcClient);
        SetDlgItemText(hwnd,IDDI_NO_PROFILE_NOTE,szMessage);
        SetWindowPos(GetDlgItem(hwnd,IDDI_NO_PROFILE_NOTE),HWND_TOP,0,0,rcClient.right - rcClient.left - 32,rcClient.bottom - rcClient.top - 32,SWP_NOMOVE);
        }
        return (LRESULT)0L;

    default:
        break;

    }

    return (LRESULT)0L;
    }


    LRESULT CALLBACK redTextHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

    switch ( msg ) {

    case WM_PAINT: {
        PAINTSTRUCT ps = {0};
        BeginPaint(hwnd,&ps);
        char szText[1024];
        GetWindowText(hwnd,szText,1024);
        HFONT hGUIFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SelectObject(ps.hdc,hGUIFont);
        SetTextColor(ps.hdc,RGB(255,0,0));
        SetBkColor(ps.hdc,GetSysColor(COLOR_MENU));
        DrawText(ps.hdc,szText,(int)strlen(szText),&ps.rcPaint,DT_TOP);
        EndPaint(hwnd,&ps);
        }
        break;

    default:
        break;

    }

    return CallWindowProc(defaultTextHandler,hwnd,msg,wParam,lParam);
    }



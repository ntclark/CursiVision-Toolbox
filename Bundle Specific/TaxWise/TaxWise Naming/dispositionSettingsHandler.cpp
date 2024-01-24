
#include "TaxWise Naming.h"

#include <shellapi.h>
#include <shTypes.h>
#include <shlobj.h>

#define OBJECT_WITH_PROPERTIES NamingBackEnd
#define CURSIVISION_SERVICES_INTERFACE pObject -> pICursiVisionServices
#define DEFAULT_RESULT_DISPOSITION_PTR &pObject -> processingDisposition

#include "dispositionSettingsDefines.h"

#define ROLLUP_ASSOCIATE_PROCESS

#define ADDITIONAL_INITIALIZATION \
    needsAdmin = false;                                                                      \
    if ( ! CURSIVISION_SERVICES_INTERFACE -> AllowToolboxPropertyChanges() && ! pObject -> editAllowed ) { \
        SetDlgItemText(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES,"Changes are disabled because Admin privileges are required to change tool properties");    \
        needsAdmin = true;                                                                 \
    } else                                                                                \
        ShowWindow(GetDlgItem(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES),SW_HIDE);      \
    if ( needsAdmin ) {                                                                          \
        enableDisableSiblings(GetDlgItem(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES),FALSE);   \
        moveUpAllAmount(hwnd,-16,NULL);                                                         \
        SetWindowPos(GetDlgItem(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES),HWND_TOP,8,8,0,0,SWP_NOSIZE | SWP_SHOWWINDOW);     \
        if ( NULL == defaultTextHandler )                                                                                       \
            defaultTextHandler = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES),GWLP_WNDPROC,(UINT_PTR)redTextHandler); \
        else                                                                                                                    \
            SetWindowLongPtr(GetDlgItem(hwnd,IDDI_DISPOSITION_NEED_ADMIN_PRIVILEGES),GWLP_WNDPROC,(UINT_PTR)redTextHandler);    \
    }   \
    PUT_BOOL(pObject -> saveInTaxYear,IDDI_SAVE_IN_TAX_YEAR_SUBFOLDER)          \
    PUT_BOOL(pObject -> saveInStateFederal,IDDI_SAVE_IN_STATE_FED_SUBFOLDER)    \
    PUT_BOOL(pObject -> saveInPackageName,IDDI_SAVE_IN_PACKAGE_NAME_SUBFOLDER)  \
    PUT_BOOL(pObject -> saveInProfileName,IDDI_SAVE_IN_PROFILE_NAME_SUBFOLDER)

#define UNLOAD_ADDITIONAL   \
    GET_BOOL(pObject -> saveInTaxYear,IDDI_SAVE_IN_TAX_YEAR_SUBFOLDER)          \
    GET_BOOL(pObject -> saveInStateFederal,IDDI_SAVE_IN_STATE_FED_SUBFOLDER)    \
    GET_BOOL(pObject -> saveInPackageName,IDDI_SAVE_IN_PACKAGE_NAME_SUBFOLDER)  \
    GET_BOOL(pObject -> saveInProfileName,IDDI_SAVE_IN_PROFILE_NAME_SUBFOLDER)

    static boolean needsAdmin = false;
    LRESULT CALLBACK redTextHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
    WNDPROC defaultTextHandler{NULL};

   LRESULT CALLBACK NamingBackEnd::dispositionSettingsHandler(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {

   resultDisposition *p = (resultDisposition *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

#include "dispositionSettingsBody.cpp"

   return (LRESULT)FALSE;
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
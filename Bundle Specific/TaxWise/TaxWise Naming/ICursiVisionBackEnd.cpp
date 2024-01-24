#include "TaxWise Naming.h"

#include <process.h>
#include <Psapi.h>
#include <time.h>

    extern "C" int GetDocumentsLocation(HWND hwnd,char *);

    HRESULT __stdcall NamingBackEnd::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

    if ( ! pICursiVisionServices || processingDisposition.doProperties ) {

SetProperties:

        IUnknown *pIUnknown = NULL;
        QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
        pIGProperties -> ShowProperties(hwndMainFrame,pIUnknown);
        pIUnknown -> Release();

    }

    if ( ! pICursiVisionServices )
        return E_FAIL;

    resultDisposition *pDisposition = &processingDisposition;

    char szOriginalFile[MAX_PATH];

    WideCharToMultiByte(CP_ACP,0,bstrOriginalFile,-1,szOriginalFile,MAX_PATH,0,0);

    char *p = strrchr(szOriginalFile,'/');
    if ( ! p )
        p = strrchr(szOriginalFile,'\\');

    p++;
    char *pStart = p;
    while ( isdigit(*p) ||  *p == '-' )
        p++;

    if ( p == pStart ) {
        char szMessage[512];
        sprintf(szMessage,"The system was not able to determine the tax ID from the file:\n\n\t"
                        "%s\n\nThis processing tool is built to handle files with a name that begins with a series of digits and '-' characters",pStart);
        MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_OK | MB_TOPMOST);
        return E_FAIL;
    }

    char cKeep = *p;

    *p = '\0';

    char szResultFile[MAX_PATH];
    memset(szResultFile,0,sizeof(szResultFile));

    strcpy(szResultFile,pStart);

    *p = cKeep;

    if ( saveInTaxYear ) {

        DWORD allProcesses[4096], cbNeeded;

        EnumProcesses(allProcesses, sizeof(allProcesses), &cbNeeded);

        char szProcessName[MAX_PATH];

        for ( long k = 0; k < (long)(cbNeeded / sizeof(DWORD)); k++ ) {
            HMODULE hModule;
            DWORD cb;
            HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE | PROCESS_VM_READ,FALSE, allProcesses[k]);
            if ( EnumProcessModules( hProcess, &hModule, sizeof(HMODULE), &cb) ) {
                GetModuleBaseName( hProcess, hModule, szProcessName, MAX_PATH);
                if ( strstr(szProcessName,"TWW") )
                    break;
                szProcessName[0] = '\0';
            }
        }

        if ( ! ( '\0' == szProcessName[0] ) ) {

            char *p = strrchr(szProcessName,'.');

            if ( ! ( NULL == p ) ) {

                *p = '\0';

                p--;

                if ( isdigit(*p) ) {

                    while ( isdigit(*p) ) p--;

                    char szNewFile[MAX_PATH];

                    sprintf_s<MAX_PATH>(szNewFile,"20%s`%s",p + 1,szResultFile);

                    strcpy(szResultFile,szNewFile);

                }
            }
        }
    }

#define ADD_FOLDER(theFolder)  \
    {                                       \
    char szNewResult[MAX_PATH];             \
    strcpy(szNewResult,szResultFile);       \
    char *p = strrchr(szNewResult,'`');     \
    if ( p ) {                              \
        *(p + 1) = '\0';                    \
        strcat(szNewResult,theFolder);      \
        strcat(szNewResult,strrchr(szResultFile,'`'));  \
        strcpy(szResultFile,szNewResult);   \
    } else {                                \
        sprintf_s<MAX_PATH>(szNewResult,"%s`%s",theFolder,szResultFile);  \
        strcpy(szResultFile,szNewResult);   \
    }  \
    }

    if ( saveInStateFederal ) {

        if ( ! ( NULL == strstr(szOriginalFile," State ") ) )
            ADD_FOLDER("State")

        if ( ! ( NULL == strstr(szOriginalFile," Federal ") ) )
            ADD_FOLDER("Federal")
    }

    if ( saveInPackageName ) {

        IPrintingSupportProfile *pIPrintingSupportProfile = NULL;

        pICursiVisionServices -> get_PrintingSupportProfile(&pIPrintingSupportProfile);

        if ( ! ( NULL == pIPrintingSupportProfile ) ) 
            ADD_FOLDER(pIPrintingSupportProfile -> PackageName())

    }

    if ( saveInProfileName ) {

        IPrintingSupportProfile *pIPrintingSupportProfile = NULL;

        pICursiVisionServices -> get_PrintingSupportProfile(&pIPrintingSupportProfile);

        if ( ! ( NULL == pIPrintingSupportProfile ) )
            ADD_FOLDER(pIPrintingSupportProfile -> Name())

    }

    bool isFileSaved = false;

#define SAVE_FILE \
    CopyFileW(bstrResultFileName,bstrResultsFile,FALSE);

#include "savePDFFile.cpp"

    return S_OK;
    }


   HRESULT __stdcall NamingBackEnd::put_PropertiesFileName(BSTR propertiesFileName) {
   return S_OK;
   }


   HRESULT __stdcall NamingBackEnd::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }

   HRESULT __stdcall NamingBackEnd::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvbeTaxWiseNaming");
   return S_OK;
   }

   HRESULT __stdcall NamingBackEnd::get_SavedDocumentName(BSTR *pDocumentName) {
   if ( ! bstrResultsFile )
      return E_FAIL;
   *pDocumentName = SysAllocString(bstrResultsFile);
   return S_OK;
   }


   HRESULT __stdcall NamingBackEnd::put_ParentWindow(HWND hp) {
   hwndParent = hp;
   hwndMainFrame = hp;
   return S_OK;
   }

   HRESULT __stdcall NamingBackEnd::put_CommandLine(BSTR theCommandLine) {
   return E_NOTIMPL;
   }


   HRESULT __stdcall NamingBackEnd::put_PrintingSupportProfile(IPrintingSupportProfile *pp) {
   return E_NOTIMPL;
   }

   HRESULT __stdcall NamingBackEnd::ServicesAdvise(ICursiVisionServices *pServices) {
   pICursiVisionServices = pServices;
   return S_OK; 
   }
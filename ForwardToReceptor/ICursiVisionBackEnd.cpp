// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Ws2tcpip.h>

#include "forwardToReceptor.h"
#include "resultDisposition.h"

    HRESULT __stdcall forwardToReceptor::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

    isProcessing = true;

    if ( showProperties || ! szServerName[0] ) {

SetProperties:

        IUnknown *pIUnknown = NULL;
        QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
        pIGProperties -> ShowProperties(hwndParent,pIUnknown);
        pIUnknown -> Release();

    }

    if ( ! doExecute )
        return S_OK;

    char szCommand[4096];
    char szFileName[MAX_PATH];

    HANDLE hPipeToReceptor = CreateFileW(L"\\\\.\\pipe\\CVReceptor",GENERIC_WRITE | GENERIC_READ,0L,NULL,OPEN_EXISTING,0L,NULL);

    BSTR translatedDispositionSettingsFile = NULL;

    if ( dispositionSettingsFileName ) {
        struct resultDisposition disposition;
        IGProperties *pProperties;
        HRESULT rc = CoCreateInstance(CLSID_InnoVisioNateProperties,NULL,CLSCTX_ALL,IID_IGProperties,reinterpret_cast<void **>(&pProperties));
        pProperties -> Add(L"result disposition",NULL);
        pProperties -> DirectAccess(L"result disposition",TYPE_BINARY,&disposition,sizeof(resultDisposition));
        pProperties -> put_FileName(dispositionSettingsFileName);
        VARIANT_BOOL bSuccess;
        pProperties -> LoadFile(&bSuccess);
        pProperties -> Release();
        translatedDispositionSettingsFile = SysAllocString(_wtempnam(NULL,NULL));
        FILE *fSettings = _wfopen(translatedDispositionSettingsFile,L"wb");
        fwrite(&disposition,1,sizeof(resultDisposition),fSettings);
        fclose(fSettings);
    }

    BSTR filesToSend[] = {bstrResultFileName,translatedDispositionSettingsFile,L""};
    char fileCommand[][64] = {"filesize","profilesize",0};
    char fileType[][64] = {"signed document","document profile",0};

    DWORD cb;

    for ( long k = 0; 1; k++ ) {

        if ( ! filesToSend[k] ) 
            continue;

        if ( 0 == wcslen(filesToSend[k]) )
            break;

        WideCharToMultiByte(CP_ACP,0,filesToSend[k],-1,szFileName,MAX_PATH,0,0);

        FILE *fPDF = fopen(szFileName,"rb");
        fseek(fPDF,0,SEEK_END);
        long fileSize = ftell(fPDF);
        fseek(fPDF,0,SEEK_SET);

        BYTE *pBinary = new BYTE[fileSize];
        fread(pBinary,fileSize,1,fPDF);
        fclose(fPDF);

        cb = sprintf(szCommand,"%s:%ld",fileCommand[k],fileSize);
        WriteFile(hPipeToReceptor,(void *)szCommand,cb,&cb,NULL);

        memset(szCommand,0,sizeof(szCommand));
        cb = 1024;
        ReadFile(hPipeToReceptor,(void *)szCommand,cb,&cb,NULL);

        if ( strncmp(szCommand,"send",4) ) {
            sprintf(szCommand,"The CursiVision Receptor on server %s did not accept the %s for processing.",szServerName,fileType[k]);
            MessageBox(NULL,szCommand,"CursiVision Forward To Receptor Error!",MB_ICONEXCLAMATION);
            delete [] pBinary;
            CloseHandle(hPipeToReceptor);
            return E_FAIL;
        }

        long totalBytes = 0;
        for ( totalBytes = 0; totalBytes + 1024 < fileSize; totalBytes += 1024 ) {
            cb = 1024;
            WriteFile(hPipeToReceptor,(void *)(pBinary + totalBytes),cb,&cb,NULL);
        }

        if ( totalBytes < fileSize ) {
            cb = fileSize - totalBytes;
            WriteFile(hPipeToReceptor,(void *)(pBinary + totalBytes),cb,&cb,NULL);
        }

        memset(szCommand,0,sizeof(szCommand));
        cb = 1024;
        ReadFile(hPipeToReceptor,(void *)szCommand,cb,&cb,NULL);

        if ( strncmp(szCommand,"received",8) ) {
            sprintf(szCommand,"The CursiVision Receptor on server %s did not accept the %s for processing.",szServerName,fileType[k]);
            MessageBox(NULL,szCommand,"CursiVision Forward To Receptor Error!",MB_ICONEXCLAMATION);
            delete [] pBinary;
            CloseHandle(hPipeToReceptor);
            return E_FAIL;
        }

        delete [] pBinary;

    }

    WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szFileName,MAX_PATH,0,0);

    char *pSlash = strrchr(szFileName,'\\');
    if ( ! pSlash )
        pSlash = strrchr(szFileName,'/');
    if ( pSlash )
        sprintf(szCommand,"name %s",pSlash + 1);
    else
        sprintf(szCommand,"name %s",szFileName);

    cb = (DWORD)strlen(szCommand);
    WriteFile(hPipeToReceptor,(void *)szCommand,cb,&cb,NULL);
   
    memset(szCommand,0,sizeof(szCommand));
    cb = 1024;
    ReadFile(hPipeToReceptor,(void *)szCommand,cb,&cb,NULL);

    WriteFile(hPipeToReceptor,(void *)"overwrite",9,&cb,NULL);
    cb = 1024;
    ReadFile(hPipeToReceptor,(void *)szCommand,cb,&cb,NULL);

    if ( szNextServerName[0] ) {
        memset(szCommand,0,sizeof(szCommand));
        sprintf(szCommand,"forward %s",szNextServerName);
        cb = (DWORD)strlen(szCommand);
        WriteFile(hPipeToReceptor,(void *)szCommand,cb,&cb,NULL);
        memset(szCommand,0,sizeof(szCommand));
        cb = 1024;
        ReadFile(hPipeToReceptor,(void *)szCommand,cb,&cb,NULL);
    }

    if ( saveOnly && szServerStoreLocation[0] ) {
        memset(szCommand,0,sizeof(szCommand));
        sprintf(szCommand,"store %s",szServerStoreLocation);
        cb = (DWORD)strlen(szCommand);
        WriteFile(hPipeToReceptor,(void *)szCommand,cb,&cb,NULL);
        memset(szCommand,0,sizeof(szCommand));
        cb = 1024;
        ReadFile(hPipeToReceptor,(void *)szCommand,cb,&cb,NULL);
    }

    WriteFile(hPipeToReceptor,(void *)"go",2,&cb,NULL);

    memset(szCommand,0,sizeof(szCommand));
    cb = 1024;
    ReadFile(hPipeToReceptor,(void *)szCommand,cb,&cb,NULL);

    if ( _stricmp(szCommand,"ok") ) {
        char szTemp[1024];
        sprintf(szTemp,"The CursiVisionReceptor on server %s did not accept the signed document for processing.\nThe server reported:\n\n\t%s",szServerName,szCommand);
        MessageBox(NULL,szTemp,"CursiVision Forward To Receptor Error!",MB_ICONEXCLAMATION);
        CloseHandle(hPipeToReceptor);
        return E_FAIL;
    }

   
    CloseHandle(hPipeToReceptor);

    if ( translatedDispositionSettingsFile ) {
        DeleteFileW(translatedDispositionSettingsFile);
        SysFreeString(translatedDispositionSettingsFile);
    }

    return S_OK;
    }


    HRESULT __stdcall forwardToReceptor::put_PropertiesFileName(BSTR propertiesFileName) {
    pIGProperties -> put_FileName(propertiesFileName);
    short bSuccess;
    pIGProperties -> LoadFile(&bSuccess);
    pIGProperties -> Save();
    return S_OK;
    }


    HRESULT __stdcall forwardToReceptor::get_PropertiesFileName(BSTR *pPropertiesFileName) {
    return pIGProperties -> get_FileName(pPropertiesFileName);
    }


    HRESULT __stdcall forwardToReceptor::get_CodeName(BSTR *pPropertiesRootName) {
    if ( ! pPropertiesRootName )
        return E_POINTER;
    *pPropertiesRootName = SysAllocString(L"cvbeForward");
    return S_OK;
    }


    HRESULT __stdcall forwardToReceptor::get_SavedDocumentName(BSTR *pDocumentName) {
    return E_NOTIMPL;
    }

    HRESULT __stdcall forwardToReceptor::put_ParentWindow(HWND hp) {
    hwndParent = hp;
    return S_OK;
    }

    HRESULT __stdcall forwardToReceptor::put_CommandLine(BSTR theCommandLine) {
    return E_NOTIMPL;
    }
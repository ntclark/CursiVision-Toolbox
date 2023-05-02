// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "forwardToReceptor.h"

#include "pdfEnabler_i.c"
#include "CursiVisionReceptor.h"

   forwardToReceptor *forwardToReceptor::pCurrentReplicatorBackEnd = NULL;
   forwardToReceptor::_IPropertyPage *forwardToReceptor::pIPropertyPage = NULL;

    forwardToReceptor::forwardToReceptor(IUnknown *pIUnknownOuter) :

        pIGProperties(NULL),
        pIGPropertiesClient(NULL),
        pIGPropertyPageClient(NULL),

        hwndProperties(NULL),
        hwndParent(NULL),

        isProcessing(false),
        doExecute(true),

        portNumber(-1L),

        startParameters(0),
        endParameters(0),

        pICursiVisionServices(NULL),

        refCount(0)

    {

    pIGPropertyPageClient = new _IGPropertyPageClient(this);

    pICursiVisionForwardToReceptorBackEnd = new _ICursiVisionForwardToReceptorBackEnd(this);

    long sizeParameters = offsetof(forwardToReceptor,endParameters) - offsetof(forwardToReceptor,startParameters);

    memset(&startParameters,0,sizeParameters);

    portNumber = atol(SERVICE_PORT_A);

    HRESULT rc = CoCreateInstance(CLSID_InnoVisioNateProperties,NULL,CLSCTX_ALL,IID_IGProperties,reinterpret_cast<void **>(&pIGProperties));

#ifdef DEBUG
    pIGProperties -> put_DebuggingEnabled(true);
#endif

    pIGPropertiesClient = new _IGPropertiesClient(this);

    pIGProperties -> Advise(static_cast<IGPropertiesClient *>(pIGPropertiesClient));

//
// 9-1-2011: IGProperties is adding a reference (as it should) which can be removed
// It may be better to not load properties in the constructor.
//
    refCount = 0L;

    pIGProperties -> Add(L"forward to receptor parameters",NULL);
    pIGProperties -> DirectAccess(L"forward to receptor parameters",TYPE_BINARY,&startParameters,sizeParameters);

    char szTemp[MAX_PATH];
    char szRootName[MAX_PATH];

    strcpy(szRootName,szModuleName);

    char *p = strrchr(szModuleName,'\\');
    if ( ! p )
        p = strrchr(szModuleName,'/');
    if ( p ) {
        strcpy(szRootName,p + 1);
    }

    p = strrchr(szRootName,'.');
    if ( p )
        *p = '\0';

    sprintf(szTemp,"%s\\Settings\\%s.settings",szApplicationDataDirectory,szRootName);

    BSTR bstrFileName = SysAllocStringLen(NULL,MAX_PATH);
    MultiByteToWideChar(CP_ACP,0,szTemp,-1,bstrFileName,MAX_PATH);

    pIGProperties -> put_FileName(bstrFileName);

    SysFreeString(bstrFileName);

    short bSuccess;
    pIGProperties -> LoadFile(&bSuccess);
    if ( ! bSuccess )
        pIGPropertiesClient -> InitNew();

    pCurrentReplicatorBackEnd = this;

    return;
    }


    forwardToReceptor::~forwardToReceptor() {
    if ( pIPropertyPage )
        pIPropertyPage -> Release();
    pIPropertyPage = NULL;
    IPrintingSupportProfile *px = NULL;
    pICursiVisionServices -> get_PrintingSupportProfile(&px);
    if ( pICursiVisionServices -> IsAdministrator() || ! px )
        pIGProperties -> Save();
    pCurrentReplicatorBackEnd = NULL;
    return;
    }

#include "resultDisposition.cpp"

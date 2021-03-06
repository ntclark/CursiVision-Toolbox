// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "emailBackEnd.h"

   EmailBackEnd::EmailBackEnd(IUnknown *pIUnknownOuter) :

   pIGProperties(NULL),
   pIGPropertiesClient(NULL),
   pIGPropertyPageClient(NULL),

   hwndProperties(NULL),
   hwndParent(NULL),
   hwndBody(NULL),

   startParameters(0),
   endParameters(0),

   isProcessing(false),
   doExecute(true),

   pICursiVisionServices(NULL),

   refCount(0)

   {

   pIGPropertyPageClient = new _IGPropertyPageClient(this);

   long sizeParameters = offsetof(EmailBackEnd,endParameters) - offsetof(EmailBackEnd,startParameters);

   memset(&startParameters,0,sizeParameters);

   smtpPort = 25;

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

   pIGProperties -> Add(L"email parameters",NULL);
   pIGProperties -> DirectAccess(L"email parameters",TYPE_BINARY,&startParameters,sizeParameters);

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

   return;
   }


   EmailBackEnd::~EmailBackEnd() {
   IPrintingSupportProfile *px = NULL;
   pICursiVisionServices -> get_PrintingSupportProfile(&px);
   if ( pICursiVisionServices -> IsAdministrator() || ! px ) 
      pIGProperties -> Save();
   return;
   }

   
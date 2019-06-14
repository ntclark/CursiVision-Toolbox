// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <ShlObj.h>
#include <olectl.h>
#include <PrSht.h>

#include "resource.h"

#include "pdfEnabler_i.h"
#include "PrintingSupport_i.h"
#include "ftpBackEnd_i.h"
#include "Properties_i.h"
#include "SignaturePad_i.h"
#include "CursiVision_i.h"

   class FTPBackEnd : public ICursiVisionBackEnd {
   public:

      FTPBackEnd(IUnknown *pOuter);
      ~FTPBackEnd();

      //   IUnknown

      STDMETHOD (QueryInterface)(REFIID riid,void **ppv);
      STDMETHOD_ (ULONG, AddRef)();
      STDMETHOD_ (ULONG, Release)();

   private:

      HRESULT __stdcall put_PropertiesFileName(BSTR propertiesFileName);
      HRESULT __stdcall get_PropertiesFileName(BSTR *pPropertiesFileName);

      HRESULT __stdcall get_CodeName(BSTR *);

      HRESULT __stdcall get_SavedDocumentName(BSTR *pTheSavedDocumentNameReturnNOT_IMPLIfNotSaved);

      HRESULT __stdcall put_ParentWindow(HWND);

      HRESULT __stdcall put_CommandLine(BSTR theCommandLine);

      HRESULT __stdcall put_PrintingSupportProfile(IPrintingSupportProfile *) { return E_NOTIMPL; };

      HRESULT __stdcall get_Description(BSTR *p) { if ( ! p ) return E_POINTER; *p = SysAllocString(L"CursiVision FTP Tool"); return S_OK; };

      HRESULT __stdcall Dispose(BSTR originalFile,BSTR resultsFile,BSTR graphicFileName,BSTR dispositionSettingsFileName,BOOL isTempFile);

      HRESULT __stdcall CanRunFromTools() { return S_OK; };

      HRESULT __stdcall CanRunFromCursiVisionControl() { return S_OK; };

      HRESULT __stdcall ServicesAdvise(ICursiVisionServices *p) { pICursiVisionServices = p; return S_OK; };

      //   IPropertiesClient

      class _IGPropertiesClient : public IGPropertiesClient {

      public:
   
         _IGPropertiesClient(FTPBackEnd *pp) : pParent(pp), refCount(0) {};
         ~_IGPropertiesClient() {};

         //   IUnknown
      
         STDMETHOD (QueryInterface)(REFIID riid,void **ppv);
         STDMETHOD_ (ULONG, AddRef)();
         STDMETHOD_ (ULONG, Release)();
   
         STDMETHOD(SavePrep)();
         STDMETHOD(InitNew)();
         STDMETHOD(Loaded)();
         STDMETHOD(Saved)();
         STDMETHOD(IsDirty)();
         STDMETHOD(GetClassID)(BYTE *pCLSID);

      private:

         FTPBackEnd *pParent;
         long refCount;

      } *pIGPropertiesClient;

      class _IGPropertyPageClient : public IGPropertyPageClient {

      public:
         
         _IGPropertyPageClient(FTPBackEnd *pp) : pParent(pp) {};
         ~_IGPropertyPageClient();

//      IPropertyPageClient
 
         STDMETHOD (QueryInterface)(REFIID riid,void **ppv);
         STDMETHOD_ (ULONG, AddRef)();
         STDMETHOD_ (ULONG, Release)();

         STDMETHOD(BeforeAllPropertyPages)();
         STDMETHOD(GetPropertyPagesInfo)(long* countPages,SAFEARRAY** stringDescriptions,SAFEARRAY** pHelpDirs,SAFEARRAY** pSizes);
         STDMETHOD(CreatePropertyPage)(long indexNumber,HWND,RECT*,BOOL,HWND *pHwndPropertyPage);
         STDMETHOD(IsPageDirty)(long,BOOL*);
         STDMETHOD(Help)(BSTR);
         STDMETHOD(TranslateAccelerator)(long,long*);
         STDMETHOD(Apply)();
         STDMETHOD(AfterAllPropertyPages)(BOOL);
         STDMETHOD(DestroyPropertyPage)(long indexNumber);

         STDMETHOD(GetPropertySheetHeader)(void *pHeader);
         STDMETHOD(get_PropertyPageCount)(long *pCount);
         STDMETHOD(GetPropertySheets)(void *pSheets);

      private:
   
         FTPBackEnd* pParent;

      } * pIGPropertyPageClient;

      long refCount;   

      IGProperties *pIGProperties;

      HWND hwndParent,hwndLog;

      bool isProcessing,doExecute;

      long startParameters;

      long ftpPort;

      char szFTPServer[64];
      char szFTPUserName[64];
      char szFTPPassword[64];

      bool showProperties;

      long endParameters;

      ICursiVisionServices *pICursiVisionServices;

      char szResultFile[MAX_PATH];

      static LRESULT CALLBACK propertiesHandler( HWND,UINT,WPARAM,LPARAM);

   };

extern "C" long sendDocument(char *serverName,long ftpPort,char *userName,char *password,char *pszAttachment,HWND hwndLog);

#ifdef DEFINE_DATA

   HMODULE hModule = NULL;
   char szModuleName[MAX_PATH];
   char szApplicationDataDirectory[MAX_PATH];
   char szGlobalDataStore[MAX_PATH];

#else

   extern HMODULE hModule;
   extern char szModuleName[];
   extern char szApplicationDataDirectory[];
   extern char szGlobalDataStore[];

#endif

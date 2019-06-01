// Copyright 2017, 2018, 2019 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <olectl.h>
#include <PrSht.h>

#include "resource.h"

#include "pdfEnabler_i.h"
#include "PrintingSupport_i.h"
#include "PrintingBackEnd_i.h"
#include "Properties_i.h"
#include "pdfEnabler_i.h"
#include "SignaturePad_i.h"
#include "CursiVision_i.h"

#include "resultDisposition.h"

   class PrintingBackEnd : public ICursiVisionBackEnd {
   public:

      PrintingBackEnd(IUnknown *pOuter);
      ~PrintingBackEnd();

      //   IUnknown

      STDMETHOD (QueryInterface)(REFIID riid,void **ppv);
      STDMETHOD_ (ULONG, AddRef)();
      STDMETHOD_ (ULONG, Release)();

      STDMETHOD(PushProperties)();
      STDMETHOD(PopProperties)();
      STDMETHOD(DiscardProperties)();
      STDMETHOD(SaveProperties)();

   private:

      HRESULT __stdcall put_PropertiesFileName(BSTR propertiesFileName);
      HRESULT __stdcall get_PropertiesFileName(BSTR *pPropertiesFileName);

      HRESULT __stdcall get_CodeName(BSTR *);

      HRESULT __stdcall put_ParentWindow(HWND);

      HRESULT __stdcall get_SavedDocumentName(BSTR *pTheSavedDocumentNameReturnNOT_IMPLIfNotSaved);

      HRESULT __stdcall put_CommandLine(BSTR theCommandLine);

      HRESULT __stdcall put_PrintingSupportProfile(IPrintingSupportProfile *) { return E_NOTIMPL; };

      HRESULT __stdcall get_Description(BSTR *p) { if ( ! p ) return E_POINTER; *p = SysAllocString(L"CursiVision Printing Tool"); return S_OK; };

      HRESULT __stdcall Dispose(BSTR inputFile,BSTR resultsFile,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile);

      HRESULT __stdcall CanRunFromTools() { return S_OK; };

      HRESULT __stdcall CanRunFromCursiVisionControl() { return S_OK; };

      HRESULT __stdcall ServicesAdvise(ICursiVisionServices *p) { pICursiVisionServices = p; return S_OK; };

      class _IPrintingBackEndAdditional : IUnknown {

         _IPrintingBackEndAdditional(PrintingBackEnd *pp) : pParent(pp) { };
         ~_IPrintingBackEndAdditional() {};

         //   IUnknown

         STDMETHOD (QueryInterface)(REFIID riid,void **ppv);
         STDMETHOD_ (ULONG, AddRef)();
         STDMETHOD_ (ULONG, Release)();

         // IPrintingBackEndAdditional

         STDMETHOD (TakeMainWindow)(HWND hwndMainWindow);

         STDMETHOD (PrintDocument)(char *);

         PrintingBackEnd *pParent;

         friend class PrintingBackEnd;

      } *pIPrintingBackEndAdditional;
  
      //   IPropertiesClient

      class _IGPropertiesClient : public IGPropertiesClient {

      public:
   
            _IGPropertiesClient(PrintingBackEnd *pp) : pParent(pp), refCount(0) {};
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

            PrintingBackEnd *pParent;
            long refCount;

      } *pIGPropertiesClient;

      class _IPropertyPage : public IPropertyPage {
      public:

         _IPropertyPage(PrintingBackEnd *pp) : pParent(pp), refCount(0) {};
         ~_IPropertyPage() {};
   
         //   IUnknown

         STDMETHOD (QueryInterface)(REFIID riid,void **ppv);
         STDMETHOD_ (ULONG, AddRef)();
         STDMETHOD_ (ULONG, Release)();

         //   IPropertyPage

         STDMETHOD(SetPageSite)(IPropertyPageSite *pPageSite);
         STDMETHOD(Activate)(HWND hWndParent, LPCRECT prc, BOOL fModal);
         STDMETHOD(Deactivate)();
         STDMETHOD(GetPageInfo)(PROPPAGEINFO *pPageInfo);
         STDMETHOD(SetObjects)(ULONG cObjects, IUnknown **ppUnk);
         STDMETHOD(Show)(UINT nCmdShow);
         STDMETHOD(Move)(LPCRECT prc);
         STDMETHOD(IsPageDirty)();
         STDMETHOD(Apply)();
         STDMETHOD(Help)(LPCOLESTR pszHelpDir);
         STDMETHOD(TranslateAccelerator)(LPMSG pMsg);

         //   IPropertyPage2

//         STDMETHOD(EditProperty)(DISPID);

         PrintingBackEnd *pParent;

         long refCount;

         friend class _IPropertyPage;

      };

      class _IGPropertyPageClient : public IGPropertyPageClient {

      public:
         
         _IGPropertyPageClient(PrintingBackEnd *pp) : pParent(pp) {};
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
   
         PrintingBackEnd* pParent;

      } * pIGPropertyPageClient;

      HRESULT printDocument(BSTR bstrFileName,char *pszChosenPrinter,BYTE *pPrinterDevMode,long sizeOfDevMode,long copies = 1);
      HRESULT printDocument(char *pszFileName,char *pszChosenPrinter,BYTE *pPrinterDevMode,long sizeOfDevMode,long copies = 1);

      long refCount;   

      IGProperties *pIGProperties;

      HWND hwndProperties,hwndParent;

      bool doExecute;

      long startParameters;

      resultDisposition processingDisposition;

      char szChosenPrinter[64];

      bool skipPrinting;

      long copies;

      bool useDefaultPrinter;

//      BYTE printerDevMode[4096];

      long endParameters;

      IPdfEnabler *pIPdfEnabler;
      IPdfDocument *pIPdfDocument;

      ICursiVisionServices *pICursiVisionServices;

      static _IPropertyPage *pIPropertyPage;

      static LRESULT CALLBACK propertiesHandler(HWND,UINT,WPARAM,LPARAM);

      friend class _IPropertyPage;

   };

#ifdef DEFINE_DATA

   HMODULE hModule = NULL;
   char szModuleName[MAX_PATH];
   char szApplicationDataDirectory[MAX_PATH];
   char szGlobalDataStore[MAX_PATH];
   char szUserDirectory[MAX_PATH];

#else

   extern HMODULE hModule;
   extern char szModuleName[];
   extern char szApplicationDataDirectory[];
   extern char szGlobalDataStore[];
   extern char szUserDirectory[];

#endif

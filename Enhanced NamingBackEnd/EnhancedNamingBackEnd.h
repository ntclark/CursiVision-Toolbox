// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma warning(disable:4996)

#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <olectl.h>
#include <PrSht.h>

#include <list>

#include "thisToolResource.h"
#include "resource.h"

#include "pdfEnabler_i.h"
#include "PrintingSupport_i.h"
#include "EnhancedNamingBackEnd_i.h"
#include "Properties_i.h"
#include "pdfEnabler_i.h"
#include "SignaturePad_i.h"
#include "CursiVision_i.h"

#include "resultDisposition.h"

   class NamingBackEnd : public ICursiVisionBackEnd {
   public:

      NamingBackEnd(IUnknown *pOuter);
      ~NamingBackEnd();

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

      HRESULT __stdcall get_SavedDocumentName(BSTR *pTheSavedDocumentNameReturnNOT_IMPLIfNotSaved);

      HRESULT __stdcall put_ParentWindow(HWND);

      HRESULT __stdcall put_CommandLine(BSTR theCommandLine);

      HRESULT __stdcall put_PrintingSupportProfile(IPrintingSupportProfile *pProfile);

      HRESULT __stdcall get_Description(BSTR *p) { if ( ! p ) return E_POINTER; *p = SysAllocString(L"CursiVision File Naming Tool"); return S_OK; }

      HRESULT __stdcall put_UserMayEdit(BOOL mayEdit) { editAllowed = mayEdit; return S_OK; }

      HRESULT __stdcall Dispose(BSTR inputFile,BSTR resultsFile,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile);

      HRESULT __stdcall CanRunFromTools() { return S_OK; }

      HRESULT __stdcall CanRunFromCursiVisionControl() { return S_OK; }

      HRESULT __stdcall ServicesAdvise(ICursiVisionServices *);

      //   IPropertiesClient

      class _IGPropertiesClient : public IGPropertiesClient {

      public:
   
         _IGPropertiesClient(NamingBackEnd *pp) : pParent(pp), refCount(0) {};
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

          NamingBackEnd *pParent;
          long refCount;

      } *pIGPropertiesClient;

      class _IPropertyPage : public IPropertyPage {
      public:

         _IPropertyPage(NamingBackEnd *pp) : pParent(pp), refCount(0) {};
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

         NamingBackEnd *pParent;

         long refCount;

         friend class _IPropertyPage;

      };

      class _IGPropertyPageClient : public IGPropertyPageClient {

      public:
         
         _IGPropertyPageClient(NamingBackEnd *pp) : pParent(pp) {};
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
   
         NamingBackEnd* pParent;

      } * pIGPropertyPageClient;

      long refCount;   

      IGProperties *pIGProperties;
      ICursiVisionServices *pICursiVisionServices;

      HWND hwndProperties,hwndParent;

      long startParameters;

      resultDisposition processingDisposition;

      char szNamePrefix[16][64];

      long endParameters;

      bool editAllowed{false};

      BSTR bstrResultsFile;

      static LRESULT CALLBACK dispositionSettingsHandler(HWND,UINT,WPARAM,LPARAM);
      static LRESULT CALLBACK multiSignOptionsHandler(HWND,UINT,WPARAM,LPARAM);
      static LRESULT CALLBACK additionalSaveOptionsHandler(HWND,UINT,WPARAM,LPARAM);

      friend class _IPropertyPage;

   public:

      //static NamingBackEnd::_IPropertyPage *CurrentPropertyPage() { return pIPropertyPage; };

   };

#ifdef DEFINE_DATA

   HMODULE hModule = NULL;
   HWND hwndMainFrame;

#else

   extern HMODULE hModule;
   extern HWND hwndMainFrame;

#endif

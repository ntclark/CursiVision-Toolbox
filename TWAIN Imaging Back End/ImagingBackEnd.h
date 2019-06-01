// Copyright 2017, 2018, 2019 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <olectl.h>
#include <PrSht.h>
#include <wia.h>
#include <sti.h>
#include <list>

#include "twain.h"

#include "resource.h"

#include "pdfEnabler_i.h"
#include "PrintingSupport_i.h"
#include "TWAINImagingBackEnd_i.h"
#include "Properties_i.h"
#include "pdfEnabler_i.h"
#include "SignaturePad_i.h"
#include "CursiVision_i.h"

#include "resultDisposition.h"

   class ImagingBackEnd : public ICursiVisionBackEnd {
   public:

      ImagingBackEnd(IUnknown *pOuter);
      ~ImagingBackEnd();

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

      HRESULT __stdcall get_Description(BSTR *p) { if ( ! p ) return E_POINTER; *p = SysAllocString(L"CursiVision TWAIN Imaging Tool"); return S_OK; };

      HRESULT __stdcall Dispose(BSTR inputFile,BSTR resultsFile,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile);

      HRESULT __stdcall CanRunFromTools() { return S_OK; };

      HRESULT __stdcall CanRunFromCursiVisionControl() { return S_OK; };

      HRESULT __stdcall ServicesAdvise(ICursiVisionServices *p) { pICursiVisionServices = p; return S_OK; };

      //   IPropertiesClient

      class _IGPropertiesClient : public IGPropertiesClient {

      public:
   
         _IGPropertiesClient(ImagingBackEnd *pp) : pParent(pp) {};
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

         ImagingBackEnd *pParent;

      } *pIGPropertiesClient;

      class _IPropertyPage : public IPropertyPage {
      public:

         _IPropertyPage(ImagingBackEnd *pp) : pParent(pp), refCount(0) {};
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

         ImagingBackEnd *pParent;

         long refCount;

         friend class _IPropertyPage;

      };

      class _IGPropertyPageClient : public IGPropertyPageClient {

      public:
         
         _IGPropertyPageClient(ImagingBackEnd *pp) : pParent(pp) {};
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
   
         ImagingBackEnd* pParent;

      } * pIGPropertyPageClient;

      bool selectPDFPage(IPdfPage **ppIPdfPage);

      TW_IDENTITY twainIdentity;

      long refCount;   

      IGProperties *pIGProperties;

      HWND hwndProperties,hwndParent;
      bool doExecute,isProcessing,fileTransfer;
      TW_INT8 imageFormat;

      HMODULE hModule_TWAIN;

      long startParameters;

      resultDisposition processingDisposition;

      char szChosenDevice[64];

      bool skipImaging,scanOne,scanMultiple,keepImage;
	  
      double inchesLeft,inchesTop;
      long pageNumber;
      bool timeStamp,specifyPage,lastPage,newLastPage,fitToPage,keepAspectRatio;
      double inchesWidth,inchesHeight;

      long endParameters;
	  
      BSTR bstrResultsFile;

      IPdfEnabler *pIPdfEnabler;
      IPdfDocument *pIPdfDocument;

      ICursiVisionServices *pICursiVisionServices;

      std::list<TW_IDENTITY *> twainSources;
      TW_IDENTITY *pCurrentSource;

      HWND hwndScanner;

      std::list<BSTR> imageFiles;

      bool doScanFailureProperties,scannerStarted,scanningCanceled;

      static unsigned int __stdcall getScans(void *);

      static LRESULT CALLBACK propertiesHandler(HWND,UINT,WPARAM,LPARAM);
      static LRESULT CALLBACK additionalSaveOptionsHandler(HWND,UINT,WPARAM,LPARAM);

      static LRESULT CALLBACK scannerHandler(HWND,UINT,WPARAM,LPARAM);
      static LRESULT CALLBACK reScanOptionHandler(HWND,UINT,WPARAM,LPARAM);

   };

void SaveJPEG(TW_IMAGEINFO *pImageInfo,HBITMAP hDIBBitmap,char *pszBitmapFileName);
void SaveJPEG(char *pszBitmapFileName);
void SaveBitmapFile(TW_IMAGEINFO *,HBITMAP hBitmap,char *pszBitmapFileName);

TW_UINT16 FAR PASCAL DSMCallback(pTW_IDENTITY _pOrigin,pTW_IDENTITY _pDest,TW_UINT32 _DG,TW_UINT16 _DAT,TW_UINT16 _MSG,TW_MEMREF _pData);

#ifdef DEFINE_DATA

   HMODULE hModule = NULL;
   char szModuleName[MAX_PATH];
   char szApplicationDataDirectory[MAX_PATH];
   char szGlobalDataStore[MAX_PATH];
   char szUserDirectory[MAX_PATH];

   DSMENTRYPROC dsmEntryProcedure = NULL;

#else

   extern HMODULE hModule;
   extern char szModuleName[];
   extern char szApplicationDataDirectory[];
   extern char szGlobalDataStore[];
   extern char szUserDirectory[];

   extern DSMENTRYPROC dsmEntryProcedure;

#endif

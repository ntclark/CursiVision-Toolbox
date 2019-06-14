// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//#pragma warning( disable : 4995 )

#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <olectl.h>
#include <PrSht.h>

#include <dshow.h>

#include <list>

#undef GetWindowID

#include "resource.h"

#include "pdfEnabler_i.h"
#include "PrintingSupport_i.h"
#include "VideoBackEnd_i.h"
#include "Properties_i.h"
#include "pdfEnabler_i.h"
#include "SignaturePad_i.h"
#include "CursiVision_i.h"

#include "directories.h"
#include "resultDisposition.h"

   class VideoBackEnd : public ICursiVisionBackEnd {

   public:

      VideoBackEnd(IUnknown *pOuter);
      ~VideoBackEnd();

      //   IUnknown

      STDMETHOD (QueryInterface)(REFIID riid,void **ppv);
      STDMETHOD_ (ULONG, AddRef)();
      STDMETHOD_ (ULONG, Release)();

      STDMETHOD(PushProperties)();
      STDMETHOD(PopProperties)();
      STDMETHOD(DiscardProperties)();
      STDMETHOD(SaveProperties)();

      bool IsProcessing() { return isProcessing; };

   private:

      HRESULT __stdcall put_PropertiesFileName(BSTR propertiesFileName);
      HRESULT __stdcall get_PropertiesFileName(BSTR *pPropertiesFileName);

      HRESULT __stdcall get_CodeName(BSTR *);

      HRESULT __stdcall put_ParentWindow(HWND);

      HRESULT __stdcall get_SavedDocumentName(BSTR *pTheSavedDocumentNameReturnNOT_IMPLIfNotSaved);

      HRESULT __stdcall put_CommandLine(BSTR theCommandLine);

      HRESULT __stdcall put_PrintingSupportProfile(IPrintingSupportProfile *) { return E_NOTIMPL; };

      HRESULT __stdcall get_Description(BSTR *p) { if ( ! p ) return E_POINTER; *p = SysAllocString(L"CursiVision Video Tool"); return S_OK; };

      HRESULT __stdcall Dispose(BSTR inputFile,BSTR resultsFile,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile);

      HRESULT __stdcall CanRunFromTools() { return S_OK; };

      HRESULT __stdcall CanRunFromCursiVisionControl() { return S_OK; };

      HRESULT __stdcall ServicesAdvise(ICursiVisionServices *p) { pICursiVisionServices = p; return S_OK; };

      //   IPropertiesClient

      class _IGPropertiesClient : public IGPropertiesClient {

      public:
   
         _IGPropertiesClient(VideoBackEnd *pp) : pParent(pp), refCount(0) {};
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

         VideoBackEnd *pParent;
         long refCount;

      } *pIGPropertiesClient;

      class _IPropertyPage : public IPropertyPage {

      public:

         _IPropertyPage(VideoBackEnd *pp) : pParent(pp), refCount(0) {};
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

         VideoBackEnd *pParent;

         long refCount;

         friend class _IPropertyPage;

      };

      class _IGPropertyPageClient : public IGPropertyPageClient {

      public:
         
         _IGPropertyPageClient(VideoBackEnd *pp) : pParent(pp) {};
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
   
         VideoBackEnd* pParent;

      } * pIGPropertyPageClient;

      bool findSolitaryCamera(char *pszDeviceName);

      long refCount;   

      IGProperties *pIGProperties;

      HWND hwndProperties,hwndParent;

      bool doExecute;

      long startParameters;

      resultDisposition processingDisposition;

      OLECHAR szChosenDevice[64];

      bool skipImaging,autoSnap,keepImage,useAnyCamera,ignoreNoCamera,saveDocumentAnyway;

      double inchesLeft,inchesTop;
      long pageNumber;
      bool timeStamp,specifyPage,lastPage,newLastPage,fitToPage,keepAspectRatio;
      double inchesWidth,inchesHeight;
      long autoFocusDelay;

      bool includeComputerName;

      long endParameters;

      bool isProcessing;
      DWORD cameraCount;
      BSTR *pCameraNames;

      BSTR bstrResultsFile;

      char szTargetFile[MAX_PATH];
      HSEMAPHORE processingSemaphore;

      IBaseFilter *pIBaseFilter;
      IGraphBuilder *pIGraphBuilder;
      IVMRWindowlessControl *pIVMRWindowlessControl;

      IPdfEnabler *pIPdfEnabler;
      IPdfDocument *pIPdfDocument;

      ICursiVisionServices *pICursiVisionServices;

      static LRESULT CALLBACK propertiesHandler(HWND,UINT,WPARAM,LPARAM);
      static LRESULT CALLBACK additionalSaveOptionsHandler(HWND,UINT,WPARAM,LPARAM);
      static LRESULT CALLBACK imageHandler(HWND,UINT,WPARAM,LPARAM);

      static WNDPROC defaultImageHandler;

   public:

   };

void TimeStampBitmap(BYTE *pImageData,char *pszBitmapFileName,bool doTimeStamp,bool doComputerName);
void SaveBitmapFile(BYTE *pImageData,char *pszBitmapFileName);
void SaveJPEG(BYTE *pImageData,char *pszBitmapFileName);

#ifdef DEFINE_DATA

   HMODULE hModule = NULL;
   char szModuleName[MAX_PATH];

#else

   extern HMODULE hModule;
   extern char szModuleName[];

#endif

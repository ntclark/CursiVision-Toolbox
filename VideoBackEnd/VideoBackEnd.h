// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//#pragma warning( disable : 4995 )

#pragma once

#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <olectl.h>
#include <PrSht.h>

#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>

#include <list>

#undef GetWindowID

#include "pdfEnabler_i.h"
#include "PrintingSupport_i.h"
#include "VideoBackEnd_i.h"
#include "Properties_i.h"
#include "pdfEnabler_i.h"
#include "SignaturePad_i.h"
#include "CursiVision_i.h"
#include "VisioLoggerVideo_i.h"
#include "VisioLoggerIntegration_i.h"

#include "directories.h"
#include "resultDisposition.h"
#include "utilities.h"

   class VideoBackEnd : public ICursiVisionBackEnd {

   public:

      VideoBackEnd(IUnknown *pOuter);
      ~VideoBackEnd();

      STDMETHOD(QueryInterfaceSpecial)(bool isCursiVision,REFIID riid,void **ppv);

      //   IUnknown

      STDMETHOD (QueryInterface)(REFIID riid,void **ppv);
      STDMETHOD_ (ULONG, AddRef)();
      STDMETHOD_ (ULONG, Release)();

      STDMETHOD(PushProperties)();
      STDMETHOD(PopProperties)();
      STDMETHOD(DiscardProperties)();
      STDMETHOD(SaveProperties)();

      bool IsProcessing() { return isProcessing; };

      bool hostVideo(HWND hwnd);
      void unHostVideo();

   private:

      HRESULT __stdcall put_PropertiesFileName(BSTR propertiesFileName);
      HRESULT __stdcall get_PropertiesFileName(BSTR *pPropertiesFileName);

      HRESULT __stdcall get_CodeName(BSTR *);

      HRESULT __stdcall put_ParentWindow(HWND);

      HRESULT __stdcall get_SavedDocumentName(BSTR *pTheSavedDocumentNameReturnNOT_IMPLIfNotSaved);

      HRESULT __stdcall put_CommandLine(BSTR theCommandLine);

      HRESULT __stdcall put_PrintingSupportProfile(IPrintingSupportProfile *) { return E_NOTIMPL; };

      HRESULT __stdcall get_Description(BSTR *p) { if ( ! p ) return E_POINTER; *p = SysAllocString(L"CursiVision Camera Tool"); return S_OK; };

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

#if 1

      class VisioLoggerVideoBackEnd : public IVisioLoggerNewRow {
      public:

         VisioLoggerVideoBackEnd(VideoBackEnd *ppParent);
         ~VisioLoggerVideoBackEnd();

         //   IUnknown

         STDMETHOD (QueryInterface)(REFIID riid,void **ppv);
         STDMETHOD_ (ULONG, AddRef)();
         STDMETHOD_ (ULONG, Release)();

         HRESULT __stdcall TakeFields(SAFEARRAY *fieldNames,SAFEARRAY *imageFieldNames,SAFEARRAY *documentFieldNames);
         HRESULT __stdcall TakeDatabaseInfo(long pIDbConnectionUnknown,BSTR serverName,BSTR databaseName,VARIANT_BOOL useWindowsLogon,BSTR userName,BSTR password,BSTR dbProvider,BSTR odbcConnectionOptions,BSTR processName,long processId);
         HRESULT __stdcall Handle(OLE_HANDLE hwndParent,SAFEARRAY *fieldNames,SAFEARRAY *imageFieldNames,SAFEARRAY *documentFIeldNames,SAFEARRAY *fieldValues,SAFEARRAY *imageHandles,SAFEARRAY *documentValues,SAFEARRAY *valuesChanged,SAFEARRAY *imagesChanged,SAFEARRAY *documentsChanged,BSTR *pBstrSwitchToProcessName,VARIANT_BOOL *pRetainValues);
         HRESULT __stdcall get_StatusNotification(BSTR *);
         HRESULT __stdcall Properties(OLE_HANDLE hwndParent);

         void initialize();
         void unInitialize();

         void *settings() { return (void *)&startParameters; }
         long settingsSize() { return offsetof(VisioLoggerVideoBackEnd,endParameters) - offsetof(VisioLoggerVideoBackEnd,startParameters);}

      private:

         // IVisioLoggerAction

         class _IVisioLoggerAction : public IVisioLoggerAction {
         public:

            _IVisioLoggerAction(VisioLoggerVideoBackEnd *pp) : pParent(pp) {}

            //   IUnknown

            STDMETHOD (QueryInterface)(REFIID riid,void **ppv) { return pParent -> QueryInterface(riid,ppv); };
            STDMETHOD_ (ULONG, AddRef)() { return pParent -> AddRef(); };
            STDMETHOD_ (ULONG, Release)() { return pParent -> Release(); };

         private:

            HRESULT __stdcall TakeFields(SAFEARRAY *fieldNames,SAFEARRAY *imageFieldNames,SAFEARRAY *documentFieldNames) { return pParent -> TakeFields(fieldNames,imageFieldNames,documentFieldNames); };
            HRESULT __stdcall TakeDatabaseInfo(long pIDbConnectionUnknown,BSTR serverName,BSTR databaseName,VARIANT_BOOL useWindowsLogon,BSTR userName,BSTR password,BSTR dbProvider,BSTR odbcConnectionOptions,BSTR processName,long processId) { return S_OK; };
            HRESULT __stdcall ActionPerformed(OLE_HANDLE hwndParent,BSTR actionFieldName,SAFEARRAY *fieldNames,SAFEARRAY *imageFieldNames,SAFEARRAY *documentFieldNames,SAFEARRAY *fieldValues,SAFEARRAY *imageHandles,SAFEARRAY *documentValues,SAFEARRAY *valuesChanged,SAFEARRAY *imagesChanged,SAFEARRAY *documentsChanged);
            HRESULT __stdcall get_StatusNotification(BSTR *bstr) { return pParent -> get_StatusNotification(bstr); };
            HRESULT __stdcall get_ActionFieldName(BSTR *pResult);
            HRESULT __stdcall Properties(OLE_HANDLE hwndParent) { return pParent -> Properties(hwndParent); };

            VisioLoggerVideoBackEnd *pParent;

         } *pIVisioLoggerAction;

         // IVisioLoggerPreSignature

         class _IVisioLoggerPreSignature : public IVisioLoggerPreSignature {
         public:

            _IVisioLoggerPreSignature(VisioLoggerVideoBackEnd *pp) : pParent(pp) {}

            //   IUnknown

            STDMETHOD (QueryInterface)(REFIID riid,void **ppv) { return pParent -> QueryInterface(riid,ppv); };
            STDMETHOD_ (ULONG, AddRef)() { return pParent -> AddRef(); };
            STDMETHOD_ (ULONG, Release)() { return pParent -> Release(); };

         private:

            HRESULT __stdcall TakeFields(SAFEARRAY *fieldNames,SAFEARRAY *imageFieldNames,SAFEARRAY *documentFieldNames) { return pParent -> TakeFields(fieldNames,imageFieldNames,documentFieldNames); };
            HRESULT __stdcall TakeDatabaseInfo(long pIDbConnectionUnknown,BSTR serverName,BSTR databaseName,VARIANT_BOOL useWindowsLogon,BSTR userName,BSTR password,BSTR dbProvider,BSTR odbcConnectionOptions,BSTR processName,long processId) { return S_OK; };
            HRESULT __stdcall Handle(OLE_HANDLE hwndParent,SAFEARRAY *fieldNames,SAFEARRAY *imageFieldNames,SAFEARRAY *documentFieldNames,SAFEARRAY *fieldValues,SAFEARRAY *imageHandles,SAFEARRAY *documentValues,SAFEARRAY *valuesChanged,SAFEARRAY *imagesChanged,SAFEARRAY *documentsChanged) { return pParent -> Handle(hwndParent,fieldNames,imageFieldNames,documentFieldNames,fieldValues,imageHandles,documentValues,valuesChanged,imagesChanged,documentsChanged,NULL,NULL); };
            HRESULT __stdcall get_StatusNotification(BSTR *bstr) { return pParent -> get_StatusNotification(bstr); };
            HRESULT __stdcall Properties(OLE_HANDLE hwndParent) { return pParent -> Properties(hwndParent); };

            VisioLoggerVideoBackEnd *pParent;

         } *pIVisioLoggerPreSignature;

         class _IGPropertyPageClient : public IGPropertyPageClient {

         public:
         
            _IGPropertyPageClient(VisioLoggerVideoBackEnd *pp) : pParent(pp) {};
            ~_IGPropertyPageClient();

   //      IPropertyPageClient
 
            STDMETHOD (QueryInterface)(REFIID riid,void **ppv);
            STDMETHOD_ (ULONG, AddRef)() { return 1; }
            STDMETHOD_ (ULONG, Release)() { return 1; }

            STDMETHOD(BeforeAllPropertyPages)() { return S_OK; }
            STDMETHOD(GetPropertyPagesInfo)(long* countPages,SAFEARRAY** stringDescriptions,SAFEARRAY** pHelpDirs,SAFEARRAY** pSizes) { return S_OK; };
            STDMETHOD(CreatePropertyPage)(long indexNumber,HWND,RECT*,BOOL,HWND *pHwndPropertyPage) { return S_OK; }
            STDMETHOD(IsPageDirty)(long,BOOL*) { return S_OK; }
            STDMETHOD(Help)(BSTR) { return S_OK; }
            STDMETHOD(TranslateAccelerator)(long,long *pResult) { *pResult = S_FALSE;return S_OK; }
            STDMETHOD(Apply)() { return S_OK; }
            STDMETHOD(AfterAllPropertyPages)(BOOL) { return S_OK; }
            STDMETHOD(DestroyPropertyPage)(long indexNumber) { return S_OK; }

            STDMETHOD(GetPropertySheetHeader)(void *pHeader);
            STDMETHOD(get_PropertyPageCount)(long *pCount);
            STDMETHOD(GetPropertySheets)(void *pSheets);

         private:
   
            VisioLoggerVideoBackEnd* pParent;

         } * pIGPropertyPageClient;

      private:

         VideoBackEnd *pParent;

         long findImageIndex(SAFEARRAY *imageFieldNames);
         void findImageSize(SAFEARRAY *imageFieldNames,SAFEARRAY *imageHandles);

         long refCount;   

         HWND hwndProperties,hwndParent;

         long startParameters;

         char szImageField[64];
         char szActionField[64];
         bool doProperties,skipImaging,autoSnap;
         char szKnownImageFields[1024];
         char szKnownTextFields[1024];

         char szTargetFile[MAX_PATH];

         long autoFocusTime;
         bool snapSecretly;

         bool showNoPictureWarning;

         long endParameters;

         long cxImage,cyImage,bmPlanes,bmBitsPixel;

         char szStatusInfo[MAX_PATH];

         bool isProcessing;
         bool isMainMenuAction;

         std::list<char *> knownImageFields;
         std::list<char *> knownTextFields;

         HSEMAPHORE processingSemaphore;

         DLGTEMPLATEEX2 *pPropertiesTemplate;

         static LRESULT CALLBACK propertiesHandler(HWND,UINT,WPARAM,LPARAM);
         
         friend class VideoBackEnd;

      } *pVisioLoggerVideoBackEnd;

#endif

      bool findSolitaryCamera(char *pszDeviceName);

      long refCount;   

      IGProperties *pIGProperties;

      HWND hwndProperties,hwndParent;

      bool doExecute;

      long startParameters;

      resultDisposition processingDisposition;

      OLECHAR szwChosenDevice[64];

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
void SaveImage(BYTE *pBitmap,char *pszBitmapFileName);

#ifdef DEFINE_DATA

   HMODULE hModule = NULL;
   char szModuleName[MAX_PATH];

#else

   extern HMODULE hModule;
   extern char szModuleName[];

#endif

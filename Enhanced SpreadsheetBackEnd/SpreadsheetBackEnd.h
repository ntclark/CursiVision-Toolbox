// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <olectl.h>
#include <PrSht.h>
#include <oleacc.h>
#include <list>

#include "resource.h"

#include "pdfEnabler_i.h"
#include "PrintingSupport_i.h"
#include "EnhancedSpreadsheetBackEnd_i.h"
#include "Properties_i.h"
#include "SignaturePad_i.h"
#include "CursiVision_i.h"

#include "utilities.h"

#include "resultDisposition.h"

#import "C:\Program Files (x86)\Common Files\Microsoft Shared\Office12\MSO.dll" \
      rename("RGB","MSORGB") \
      rename("IAccessible","msoIAccessible") \
      rename("DocumentProperties","msoDocumentProperties")

using namespace Office;

#import "C:\Program Files (x86)\Common Files\Microsoft Shared\VBA\VBA6\VBE6EXT.OLB" 

using namespace VBIDE;

#import "C:\Program Files (x86)\Microsoft Office\Office12\excel.exe" \
      rename( "RGB", "excelRGB") \
      rename( "ReplaceText","excelReplaceText") \
      rename( "CopyFile", "excelCopyFile" ) \
      rename( "DialogBox","excelDialogBox") 

#define FIELD_COUNT 16
#define FIELD_DISPLAY_COUNT   8

   class SpreadsheetBackEnd : public ICursiVisionBackEnd {
   public:

      SpreadsheetBackEnd(IUnknown *pOuter);
      ~SpreadsheetBackEnd();

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

      HRESULT __stdcall get_Description(BSTR *p) { if ( ! p ) return E_POINTER; *p = SysAllocString(L"CursiVision Spreadsheet Tool"); return S_OK; };

      HRESULT __stdcall Dispose(BSTR inputFile,BSTR resultsFile,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile);

      HRESULT __stdcall CanRunFromTools() { return S_OK; };

      HRESULT __stdcall CanRunFromCursiVisionControl() { return S_OK; };

      HRESULT __stdcall ServicesAdvise(ICursiVisionServices *);

      //   IPropertiesClient

      class _IGPropertiesClient : public IGPropertiesClient {

      public:
   
         _IGPropertiesClient(SpreadsheetBackEnd *pp) : pParent(pp), refCount(0) {};
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

          SpreadsheetBackEnd *pParent;
          long refCount;

      } *pIGPropertiesClient;

      class _IPropertyPage : public IPropertyPage {
      public:

         _IPropertyPage(SpreadsheetBackEnd *pp) : pParent(pp), refCount(0) {};
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

         SpreadsheetBackEnd *pParent;

         long refCount;

         friend class _IPropertyPage;

      };

      class _IGPropertyPageClient : public IGPropertyPageClient {

      public:
         
         _IGPropertyPageClient(SpreadsheetBackEnd *pp) : pParent(pp) {};
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
   
         SpreadsheetBackEnd* pParent;

      } * pIGPropertyPageClient;

      long getExcelWorksheets(char *pszFile);
      long loadExcelSpreadsheet();

      long refCount;   

      IGProperties *pIGProperties;
      ICursiVisionServices *pICursiVisionServices;

      HWND hwndProperties,hwndParent;

      long startParameters;

      resultDisposition processingDisposition;

      char szNamePrefix[FIELD_COUNT][64];
      char szColumnName[FIELD_COUNT][64];
      char szColumnId[FIELD_COUNT][8];

      char szWorkbookName[MAX_PATH];
      char szSpreadsheetName[64];
      char szAllSheetNames[1024];

      bool includeDate,includeTime,includeDocumentLink;

      char szDateColumn[16],szTimeColumn[16],szDocumentLinkColumn[16];

      long endParameters;

      char szSignedDocument[MAX_PATH];
      char szFieldValue[16][1024];

      CLSID CLSID_excel;

      static _IPropertyPage *pIPropertyPage;

      static LRESULT CALLBACK propertiesHandler(HWND,UINT,WPARAM,LPARAM);
      static LRESULT CALLBACK noProfileHandler(HWND,UINT,WPARAM,LPARAM);

      friend class _IPropertyPage;

   public:

      static SpreadsheetBackEnd::_IPropertyPage *CurrentPropertyPage() { return pIPropertyPage; };

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

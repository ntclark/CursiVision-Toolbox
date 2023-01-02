

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

//
//NTC: 08-04-2019: Edit the following '.tlh files, that are in the include folder of the
// Common repository, and edit the lines of the following form:
//
//   excel.tlh:
//       #pragma start_map_region("d:\development\common\include\excel.tli")
//
//   vbe6ext.tlh:
//       #include "c:\development\Common\include\vbe6ext.tli"
//
// to point to the correct location of your git clone.
//
#if 1
#include "mso.tlh"
#else
#import "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Packages\vwd\MSO.dll" \
      rename("RGB","MSORGB") \
      rename("IAccessible","msoIAccessible") \
      rename("DocumentProperties","msoDocumentProperties")
#endif

using namespace Office;

#if 1
#include "vbe6ext.tlh"
#else
#import "C:\Program Files (x86)\Common Files\Microsoft Shared\VBA\VBA6\VBE6EXT.OLB" 
#endif

using namespace VBIDE;

#if 1
#include "excel.tlh"
#else
#import "C:\Program Files (x86)\Microsoft Office\Office12\excel.exe" \
      rename( "RGB", "excelRGB") \
      rename( "ReplaceText","excelReplaceText") \
      rename( "CopyFile", "excelCopyFile" ) \
      rename( "DialogBox","excelDialogBox") 
#endif

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

      HRESULT __stdcall get_Description(BSTR *p) { if ( ! p ) return E_POINTER; *p = SysAllocString(L"CursiVision Spreadsheet Tool"); return S_OK; }

      HRESULT __stdcall put_UserMayEdit(BOOL mayEdit) { editAllowed = mayEdit; return S_OK; }

      HRESULT __stdcall Dispose(BSTR inputFile,BSTR resultsFile,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile);

      HRESULT __stdcall CanRunFromTools() { return S_OK; }

      HRESULT __stdcall CanRunFromCursiVisionControl() { return S_OK; }

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

      bool editAllowed{false};

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

#else

   extern HMODULE hModule;

#endif

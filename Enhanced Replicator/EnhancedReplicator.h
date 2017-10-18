// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>
#include <stddef.h>
#include <stdio.h>

#include "resource.h"

#include "writingLocation.h"

#include "pdfEnabler_i.h"
#include "PrintingSupport_i.h"
#include "Replicator_i.h"

#include "PDFEnabler_i.h"

#include "Properties_i.h"
#include "pdfEnabler_i.h"
#include "SignaturePad_i.h"
#include "CursiVision_i.h"
#include "PDFiumControl_i.h"

#include "templateDocument.h"
#include "utilities.h"

#define WRITING_LOCATION_COUNT 32
#define CORNER_PROXIMITY 10

   class theReplicator : public ICursiVisionBackEnd {
   public:

      theReplicator(IUnknown *pOuter);
      ~theReplicator();

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

      HRESULT __stdcall put_PrintingSupportProfile(IPrintingSupportProfile *);

      HRESULT __stdcall get_Description(BSTR *p) { if ( ! p ) return E_POINTER; *p = SysAllocString(L"CursiVision Signature Replication tool"); return S_OK; };

      HRESULT __stdcall Dispose(BSTR inputFile,BSTR resultsFile,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile);

      HRESULT __stdcall CanRunFromTools() { return S_OK; };

      HRESULT __stdcall CanRunFromCursiVisionControl() { return S_OK; };

      HRESULT __stdcall ServicesAdvise(ICursiVisionServices *pICursiVisionServices);

      //   IPropertiesClient

      class _IGPropertiesClient : public IGPropertiesClient {

      public:
   
            _IGPropertiesClient(theReplicator *pp) : pParent(pp), refCount(0) {};
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

            theReplicator *pParent;
            long refCount;

      } *pIGPropertiesClient;

      class _IGPropertyPageClient : public IGPropertyPageClient {

      public:
         
         _IGPropertyPageClient(theReplicator *pp) : pParent(pp) {};
         ~_IGPropertyPageClient();

//      IPropertyPageClient
 
         STDMETHOD (QueryInterface)(REFIID riid,void **ppv);
         STDMETHOD_ (ULONG, AddRef)();
         STDMETHOD_ (ULONG, Release)();

         STDMETHOD(BeforeAllPropertyPages)();
         STDMETHOD(GetPropertyPagesInfo)(long* countPages,SAFEARRAY** stringDescriptions,SAFEARRAY** pHelpDirs,SAFEARRAY** pSizes);
         STDMETHOD(CreatePropertyPage)(long indexNumber,HWND,RECT*,BOOL,HWND * hwndPropertyPage);
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
   
         theReplicator* pParent;

      } * pIGPropertyPageClient;

      void load();
      void unload();
      void reset();

      long addReplicant(templateDocument::tdUI *pDocument,long replicationIndex,long moveX,long moveY);
      bool duplicateReplicant(long sourceIndex,long pageNumber);
      void deleteReplicant(long activeIndex);
      void moveReplicant(long activeIndex,long moveToX,long moveToY,long toPageNumber);
      void scaleReplicant(long activeIndex,double scaleX,double scaleY);

      void clearPage(templateDocument::tdUI *pDocument);

      long refCount;   

      IGProperties *pIGProperties;
      IPrintingSupportProfile *pIPrintingSupportProfile;
      ICursiVisionServices *pICursiVisionServices;

      HWND hwndProperties,hwndParent;

      templateDocument *pTemplateDocument;

      long startParameters;

      bool showProperties;

      POINT replicantSignatureOrigins[WRITING_LOCATION_COUNT];
      POINT nativeSignatureOrigins[WRITING_LOCATION_COUNT];
      long replicantSignatureIndex[WRITING_LOCATION_COUNT];
      long replicantSignaturePage[WRITING_LOCATION_COUNT];
      double internalScaleX[WRITING_LOCATION_COUNT];
      double internalScaleY[WRITING_LOCATION_COUNT];

      long endParameters;

      long activeIndex,replicationIndex;
      POINTL rightClickMousePoint,leftClickMousePoint;
      POINTL replicationOrigin,replicantDragOrigin;

      writingLocation writingLocations[WRITING_LOCATION_COUNT];
      writingLocation *pWritingLocations[WRITING_LOCATION_COUNT];

      HBITMAP hbmDrawRestore[WRITING_LOCATION_COUNT];
      RECT restoreRect[WRITING_LOCATION_COUNT];

      bool isReplicant[WRITING_LOCATION_COUNT];
      long replicantIndex[WRITING_LOCATION_COUNT];

      void drawSignature(HDC hdc,long index,long moveX,long moveY,RECT *pNewLocation,templateDocument::tdUI *pDocument);
      void clearSignature(templateDocument::tdUI *pDocument,long index);
      void reDrawSignature(HDC hdc,long index,long moveX,long moveY,RECT *pNewLocation,templateDocument::tdUI *pDocument);

      static theReplicator *pThis;

      static LRESULT CALLBACK propertiesHandler(HWND,UINT,WPARAM,LPARAM);

      static void clearBitmapsAndDrawSignatures(HDC hdc,templateDocument::tdUI *pDocument);
      static void drawSignatures(HDC hdc,templateDocument::tdUI *pDocument);

   };


#ifdef DEFINE_DATA

   HMODULE hModule = NULL;
   char szModuleName[MAX_PATH];
   char szApplicationDataDirectory[MAX_PATH];
   char szGlobalDataStore[MAX_PATH];
   char szUserDirectory[MAX_PATH];
   HMENU hActionMenu = NULL;

#else

   extern HMODULE hModule;
   extern char szModuleName[];
   extern char szApplicationDataDirectory[];
   extern char szGlobalDataStore[];
   extern char szUserDirectory[];
   extern HMENU hActionMenu;

#endif

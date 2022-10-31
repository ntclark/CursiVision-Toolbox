/*

                       Copyright (c) 2009 Nathan T. Clark

*/

#include "VideoBackEnd.h"

#include "visioLoggerResource.h"

   long __stdcall VideoBackEnd::VisioLoggerVideoBackEnd::_IGPropertyPageClient::QueryInterface(REFIID riid,void **ppv) {

   *ppv = NULL; 
 
   if ( riid == IID_IUnknown )
      *ppv = static_cast<IUnknown*>(this); 
   else

   if ( riid == IID_IDispatch )
      *ppv = this;
   else

   if ( riid == IID_IGPropertyPageClient )
      *ppv = static_cast<IGPropertyPageClient*>(this);
   else
 
      return pParent -> QueryInterface(riid,ppv);
 
   AddRef();
  
   return S_OK; 
   }

   HRESULT VideoBackEnd::VisioLoggerVideoBackEnd::_IGPropertyPageClient::GetPropertySheetHeader(void *pv) {

   if ( ! pv )
      return E_POINTER;

   PROPSHEETHEADER *pHeader = reinterpret_cast<PROPSHEETHEADER *>(pv);

   pHeader -> dwFlags = PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP;

   pHeader -> hInstance = hModule;

   pHeader -> pszIcon = NULL;

   pHeader -> hwndParent = pParent -> hwndParent;

   if ( pParent -> isProcessing )
      pHeader -> pszCaption = "Click Snap";
   else
      pHeader -> pszCaption = "Properties";

   pHeader -> pfnCallback = NULL;

   return S_OK;
   }


   HRESULT VideoBackEnd::VisioLoggerVideoBackEnd::_IGPropertyPageClient::get_PropertyPageCount(long *pCount) {
   if ( ! pCount )
      return E_POINTER;
   *pCount = 1;
   return S_OK;
   }


   HRESULT VideoBackEnd::VisioLoggerVideoBackEnd::_IGPropertyPageClient::GetPropertySheets(void *pPages) {

   HANDLE hFMS = FindResource(hModule,MAKEINTRESOURCE(IDD_DISPOSITION_PROPERTIES),RT_DIALOG);

   DLGTEMPLATEEX2 *pd = (DLGTEMPLATEEX2 *)LoadResource(hModule,(HRSRC)hFMS);

   long n = SizeofResource(hModule,(HRSRC)hFMS);

   pParent -> pPropertiesTemplate = (DLGTEMPLATEEX2 *)new BYTE[n];

   memcpy(pParent -> pPropertiesTemplate,(BYTE *)pd,n);

   if ( 0 < pParent -> cxImage ) {

      HDC hdc = CreateCompatibleDC(NULL);

      HFONT hFont = CreateFontW(pParent -> pPropertiesTemplate -> pointsize,0,0,0,pParent -> pPropertiesTemplate -> weight,pParent -> pPropertiesTemplate -> bItalic,0,0,0,0,0,0,0,pParent -> pPropertiesTemplate -> font);

      SelectObject(hdc,hFont);

      TEXTMETRIC textMetrics = {0};

      GetTextMetrics(hdc,&textMetrics);

      long cxUnits = textMetrics.tmAveCharWidth;
      long cyUnits = textMetrics.tmHeight;

      long cxDesiredImage = MulDiv(pParent -> cxImage,4,cxUnits);
      long cyDesiredImage = MulDiv(pParent -> cyImage,8,cyUnits);

      long nativeWidth = pParent -> pPropertiesTemplate -> cx;
      long nativeHeight = pParent -> pPropertiesTemplate -> cy;

      pParent -> pPropertiesTemplate -> cx = 16 + cxDesiredImage;

      if ( pParent -> pPropertiesTemplate -> cx < nativeWidth )
         pParent -> pPropertiesTemplate -> cx = nativeWidth;

      pParent -> pPropertiesTemplate -> cy = 8 + 14 + 14 + 14 + 14 + 28 + cyDesiredImage + 64;

      if ( pParent -> pPropertiesTemplate -> cy < nativeHeight )
         pParent -> pPropertiesTemplate -> cy = nativeHeight;

      DeleteObject(hFont);
      DeleteDC(hdc);

   }

   PROPSHEETPAGE *pPropSheetPages = reinterpret_cast<PROPSHEETPAGE *>(pPages);

   pPropSheetPages[0].dwSize = sizeof(PROPSHEETPAGE);
   pPropSheetPages[0].hInstance = hModule;

   pPropSheetPages[0].dwFlags = PSP_USETITLE | PSP_DLGINDIRECT;
   pPropSheetPages[0].pResource = (PROPSHEETPAGE_RESOURCE)pParent -> pPropertiesTemplate;

   pPropSheetPages[0].pfnDlgProc = (DLGPROC)VideoBackEnd::VisioLoggerVideoBackEnd::propertiesHandler;

   if ( pParent -> isProcessing )
      pPropSheetPages[0].pszTitle = "Click Snap";
   else
      pPropSheetPages[0].pszTitle = "Video Tools properties";

   pPropSheetPages[0].lParam = (long)pParent;

   pPropSheetPages[0].pfnCallback = NULL;

   return S_OK;
   }

/*

                       Copyright (c) 2009,2010 Nathan T. Clark

*/

#include "PrintingBackEnd.h"


   long __stdcall PrintingBackEnd::_IPrintingBackEndAdditional::QueryInterface(REFIID riid,void **ppv) {
   *ppv = NULL; 
 
   if ( riid == IID_IUnknown )
      *ppv = static_cast<IUnknown*>(this); 
   else

   if ( riid == IID_IPrintingBackEndAdditional )
      *ppv = this;
   else

      return pParent -> QueryInterface(riid,ppv);
 
   static_cast<IUnknown*>(*ppv) -> AddRef();
  
   return S_OK; 
   }
 
   unsigned long __stdcall PrintingBackEnd::_IPrintingBackEndAdditional::AddRef() {
   return pParent -> AddRef();
   }
 
   unsigned long __stdcall PrintingBackEnd::_IPrintingBackEndAdditional::Release() {
   return pParent -> Release();
   }


   HRESULT __stdcall PrintingBackEnd::_IPrintingBackEndAdditional::TakeMainWindow(HWND hwnd) {
   pParent -> hwndParent = hwnd;
   return S_OK;
   }


   HRESULT __stdcall PrintingBackEnd::_IPrintingBackEndAdditional::PrintDocument(char *pszDocument) {

#if 0
   HANDLE hPrinter;

   OpenPrinter(NULL,&hPrinter,NULL);

   long sizeDevMode = DocumentProperties(hwndMainFrame,hPrinter,NULL/*szPrinter*/,NULL,NULL,0);
#endif

   PRINTDLG printDialog = {0};

   printDialog.lStructSize = sizeof(PRINTDLG);
   printDialog.hwndOwner = pParent -> hwndParent;
   printDialog.Flags = PD_PRINTSETUP | PD_USELARGETEMPLATE;

   if ( ! PrintDlg(&printDialog) )
      return E_FAIL;

   if ( ! printDialog.hDevMode )
      return E_FAIL;

   long devModeSize = GlobalSize(printDialog.hDevMode);
   BYTE *pDevMode = (BYTE *)GlobalLock(printDialog.hDevMode);

   long rc = pParent -> printDocument(pszDocument,(char *)pDevMode,pDevMode,devModeSize,1);
#if 0
   char szPrinterSettingsFile[MAX_PATH];

   memset(szPrinterSettingsFile,0,MAX_PATH * sizeof(char));

   sprintf(szPrinterSettingsFile,"\"%s\\Settings\\printerSettings",szApplicationDataDirectory);

   FILE *fPrinterSettings = fopen(szPrinterSettingsFile + 1,"wb");

   if ( ! fPrinterSettings ) {
      GlobalFree(printDialog.hDevMode);
      if ( printDialog.hDevNames )
         GlobalFree(printDialog.hDevNames);
      return E_FAIL;
   }

   fwrite(pDevMode,devModeSize,1,fPrinterSettings);
   fclose(fPrinterSettings);

   GlobalFree(printDialog.hDevMode);

   szPrinterSettingsFile[strlen(szPrinterSettingsFile)] = '\"';
#endif

   GlobalFree(printDialog.hDevMode);
   if ( printDialog.hDevNames )
      GlobalFree(printDialog.hDevNames);

   return rc;
   }

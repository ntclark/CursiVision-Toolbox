
#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>

#include <olectl.h>
#include <comcat.h>
#include <stdio.h>

#define DEFINE_DATA

#include "PrintingBackEnd.h"

#include "Properties_i.c"
#include "pdfEnabler_i.c"
#include "PrintingBackEnd_i.c"
#include "CursiVision_i.c"

   extern "C" int GetCommonAppDataLocation(HWND hwnd,char *);
   extern "C" int GetDocumentsLocation(HWND hwnd,char *);
   int GetLocation(HWND hwnd,long key,char *szFolderLocation);

   OLECHAR wstrModuleName[256];

   extern "C" BOOL WINAPI DllMain(HINSTANCE hI, DWORD dwReason, LPVOID) {

   switch ( dwReason ) {

   case DLL_PROCESS_ATTACH: {

      CoInitialize(NULL);

      hModule = hI;

      GetModuleFileName(hModule,szModuleName,1024);
      memset(wstrModuleName,0,sizeof(wstrModuleName));

      MultiByteToWideChar(CP_ACP, 0, szModuleName, -1, wstrModuleName, 1024);  

      HKEY hKeySettings = NULL;

      RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\InnoVisioNate\\CursiVision",0L,KEY_QUERY_VALUE,&hKeySettings);
      
      if ( ! ( NULL == hKeySettings ) ) {

         DWORD cb = MAX_PATH;
         DWORD dwType = REG_SZ;
         RegQueryValueEx(hKeySettings,"Global Settings Store",NULL,&dwType,(BYTE *)&szGlobalDataStore,&cb);

         RegCloseKey(hKeySettings);

      } 

      char szTemp[MAX_PATH];

      GetCommonAppDataLocation(NULL,szTemp);

      sprintf(szApplicationDataDirectory,"%s\\CursiVision",szTemp);

      CreateDirectory(szApplicationDataDirectory,NULL);

      sprintf(szApplicationDataDirectory,"%s\\CursiVision\\Settings",szTemp);

      CreateDirectory(szApplicationDataDirectory,NULL);

      sprintf(szApplicationDataDirectory,"%s\\CursiVision",szTemp);

      GetDocumentsLocation(NULL,szTemp);

      sprintf(szUserDirectory,"%s\\CursiVision Files",szTemp);

      }

      break;
  
   case DLL_PROCESS_DETACH:
      break;
  
   }
  
   return TRUE;
   }
  
  
   extern "C" int  __cdecl GetDocumentsLocation(HWND hwnd,char *szFolderLocation) {
   GetLocation(hwnd,CSIDL_PERSONAL,szFolderLocation);
   return 0;
   }

   extern "C" int __cdecl GetCommonAppDataLocation(HWND hwnd,char *szFolderLocation) {
   GetLocation(hwnd,CSIDL_COMMON_APPDATA,szFolderLocation);
   return 0;
   }
 
   int GetLocation(HWND hwnd,long key,char *szFolderLocation) {

   ITEMIDLIST *ppItemIDList;
   IShellFolder *pIShellFolder;
   LPCITEMIDLIST pcParentIDList;

   HRESULT wasInitialized = CoInitialize(NULL);

   szFolderLocation[0] = '\0';

   HRESULT rc = SHGetFolderLocation(hwnd,key,NULL,0,&ppItemIDList);

   if ( S_OK != rc ) {
      char szMessage[256];
      sprintf(szMessage,"SHGetFolderLocation returned %ld",rc);
      MessageBox(NULL,szMessage,"Error",MB_OK);
      szFolderLocation[0] = '\0';
      return 0;
   }

   rc = SHBindToParent(ppItemIDList, IID_IShellFolder, (void **) &pIShellFolder, &pcParentIDList);

   if ( S_OK == rc ) {

      STRRET strRet;
      rc = pIShellFolder -> GetDisplayNameOf(pcParentIDList,SHGDN_FORPARSING,&strRet);
      pIShellFolder -> Release();
      if ( S_OK == rc ) {
         WideCharToMultiByte(CP_ACP,0,strRet.pOleStr,-1,szFolderLocation,MAX_PATH,0,0);
      } else {
         char szMessage[256];
         sprintf(szMessage,"GetDisplayNameOf returned %ld",rc);
         MessageBox(NULL,szMessage,"Error",MB_OK);
         szFolderLocation[0] = '\0';
         return 0;
      }
   } else {
      char szMessage[256];
      sprintf(szMessage,"SHBindToParent returned %ld",rc);
      MessageBox(NULL,szMessage,"Error",MB_OK);
      szFolderLocation[0] = '\0';
      return 0;
   }

   if ( S_FALSE == wasInitialized )
      CoUninitialize();

   return 0;
   }


   class Factory : public IClassFactory {

   public:
  
      STDMETHOD (QueryInterface)(REFIID riid,void **ppv);
      STDMETHOD_ (ULONG, AddRef)();
      STDMETHOD_ (ULONG, Release)();
      STDMETHOD (CreateInstance)(IUnknown *punkOuter, REFIID riid, void **ppv);
      STDMETHOD (LockServer)(BOOL fLock);
  
      Factory() : refCount(0) {};
      ~Factory() {};
  
   private:

      int refCount;

   };
  
  
   static Factory objectFactory;
  

   STDAPI DllCanUnloadNow(void) {
   return S_FALSE;
   }
  
  
   STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppObject) {
   *ppObject = NULL;
   if ( CLSID_CursiVisionPrintingBackEnd != rclsid & CLSID_CursiVisionPrintingBackEndAdditional != rclsid ) 
      return CLASS_E_CLASSNOTAVAILABLE;
   return objectFactory.QueryInterface(riid,ppObject);
   }
  
   char *OBJECT_NAME[2];
   char *OBJECT_NAME_V[2];
   GUID OBJECT_CLSID[2];

   STDAPI DllRegisterServer() {

   char *OBJECT_VERSION;
   GUID OBJECT_LIBID;
   char *OBJECT_DESCRIPTION[2];

   OBJECT_NAME[0] = "InnoVisioNate.CursiVisionPrintingBackEnd";
   OBJECT_NAME[1] = "InnoVisioNate.CursiVisionPrintingBackEndAdditional";
   OBJECT_NAME_V[0] = "InnoVisioNate.CursiVisionPrintingBackEnd.1";
   OBJECT_NAME_V[1] = "InnoVisioNate.CursiVisionPrintingBackEndAdditional.1";
   OBJECT_VERSION = "1.0";
   memcpy(&OBJECT_CLSID[0],&CLSID_CursiVisionPrintingBackEnd,sizeof(GUID));
   memcpy(&OBJECT_CLSID[1],&CLSID_CursiVisionPrintingBackEndAdditional,sizeof(GUID));

   memcpy(&OBJECT_LIBID,&LIBID_CursiVisionPrintingBackEnd,sizeof(GUID));
   OBJECT_DESCRIPTION[0] = "CursiVision Printing Tool";
   OBJECT_DESCRIPTION[1] = "CursiVision Printing Tool Additional";

   HRESULT rc = S_OK;
   ITypeLib *ptLib;
   HKEY keyHandle,clsidHandle;
   DWORD disposition;
   char szTemp[256],szCLSID[256];
   LPOLESTR oleString;
  
   for ( long objectIndex = 0; objectIndex < 2; objectIndex++ ) {

      StringFromCLSID(OBJECT_CLSID[objectIndex],&oleString);
      WideCharToMultiByte(CP_ACP,0,oleString,-1,szCLSID,256,0,0);
    
      if ( S_OK != LoadTypeLib(wstrModuleName,&ptLib) )
         rc = ResultFromScode(SELFREG_E_TYPELIB);
      else
         if ( S_OK != RegisterTypeLib(ptLib,wstrModuleName,NULL) )
            rc = ResultFromScode(SELFREG_E_TYPELIB);

      RegOpenKeyEx(HKEY_CLASSES_ROOT,"CLSID",0,KEY_CREATE_SUB_KEY,&keyHandle);
    
         RegCreateKeyEx(keyHandle,szCLSID,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&clsidHandle,&disposition);
         sprintf(szTemp,OBJECT_DESCRIPTION[objectIndex]);
         RegSetValueEx(clsidHandle,NULL,0,REG_SZ,(BYTE *)szTemp,strlen(szTemp));
    
         sprintf(szTemp,"Control");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         sprintf(szTemp,"");
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,strlen(szTemp));
    
         sprintf(szTemp,"ProgID");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         sprintf(szTemp,OBJECT_NAME_V[objectIndex]);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,strlen(szTemp));
    
         sprintf(szTemp,"InprocServer");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szModuleName,strlen(szModuleName));
    
         sprintf(szTemp,"InprocServer32");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szModuleName,strlen(szModuleName));
//         RegSetValueEx(keyHandle,"ThreadingModel",0,REG_SZ,(BYTE *)"Free",5);
         RegSetValueEx(keyHandle,"ThreadingModel",0,REG_SZ,(BYTE *)"Apartment",9);
    
         sprintf(szTemp,"LocalServer");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szModuleName,strlen(szModuleName));
       
         sprintf(szTemp,"TypeLib");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
       
         StringFromCLSID(OBJECT_LIBID,&oleString);
         WideCharToMultiByte(CP_ACP,0,oleString,-1,szTemp,256,0,0);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,strlen(szTemp));
           
         sprintf(szTemp,"ToolboxBitmap32");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
    
         sprintf(szTemp,"Version");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         sprintf(szTemp,OBJECT_VERSION);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,strlen(szTemp));
    
         sprintf(szTemp,"MiscStatus");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         sprintf(szTemp,"0");
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,strlen(szTemp));
    
         sprintf(szTemp,"1");
         RegCreateKeyEx(keyHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         sprintf(szTemp,"%ld",
                    OLEMISC_ALWAYSRUN |
                    OLEMISC_ACTIVATEWHENVISIBLE | 
                    OLEMISC_RECOMPOSEONRESIZE | 
                    OLEMISC_INSIDEOUT |
                    OLEMISC_SETCLIENTSITEFIRST |
                    OLEMISC_CANTLINKINSIDE );
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,strlen(szTemp));
    
      RegCreateKeyEx(HKEY_CLASSES_ROOT,OBJECT_NAME[objectIndex],0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         RegCreateKeyEx(keyHandle,"CurVer",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         sprintf(szTemp,OBJECT_NAME_V[objectIndex]);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,strlen(szTemp));
    
      RegCreateKeyEx(HKEY_CLASSES_ROOT,OBJECT_NAME_V[objectIndex],0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         RegCreateKeyEx(keyHandle,"CLSID",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szCLSID,strlen(szCLSID));
  
   }

   ICatRegister *pICatRegister;

   rc = CoCreateInstance(CLSID_StdComponentCategoriesMgr,NULL,CLSCTX_ALL,IID_ICatRegister,reinterpret_cast<void **>(&pICatRegister));

   CATID categoryId = IID_ICursiVisionBackEnd;

   pICatRegister -> RegisterClassImplCategories(CLSID_CursiVisionPrintingBackEnd,1,&categoryId);

   pICatRegister -> Release();

   return S_OK;
   }
  
  
   STDAPI DllUnregisterServer() {

   CoInitialize(NULL);

   ICatRegister *pICatRegister;
   long rc = CoCreateInstance(CLSID_StdComponentCategoriesMgr,NULL,CLSCTX_ALL,IID_ICatRegister,reinterpret_cast<void **>(&pICatRegister));
   CATID categoryId = IID_ICursiVisionBackEnd;
   pICatRegister -> UnRegisterClassImplCategories(CLSID_CursiVisionPrintingBackEnd,1,&categoryId);
   pICatRegister -> Release();

   OBJECT_NAME[0] = "InnoVisioNate.CursiVisionPrintingBackEnd";
   OBJECT_NAME[1] = "InnoVisioNate.CursiVisionPrintingBackEndAdditional";
   OBJECT_NAME_V[0] = "InnoVisioNate.CursiVisionPrintingBackEnd.1";
   OBJECT_NAME_V[1] = "InnoVisioNate.CursiVisionPrintingBackEndAdditional.1";

   memcpy(&OBJECT_CLSID[0],&CLSID_CursiVisionPrintingBackEnd,sizeof(GUID));
   memcpy(&OBJECT_CLSID[1],&CLSID_CursiVisionPrintingBackEndAdditional,sizeof(GUID));

   HKEY keyHandle;
   char szCLSID[256];
   LPOLESTR oleString;
  
   for ( long objectIndex = 0; objectIndex < 2; objectIndex++ ) {

      StringFromCLSID(OBJECT_CLSID[objectIndex],&oleString);
      WideCharToMultiByte(CP_ACP,0,oleString,-1,szCLSID,256,0,0);
    
      RegOpenKeyEx(HKEY_CLASSES_ROOT,"CLSID",0,KEY_CREATE_SUB_KEY,&keyHandle);

      rc = SHDeleteKey(keyHandle,szCLSID);

      rc = SHDeleteKey(HKEY_CLASSES_ROOT,OBJECT_NAME[objectIndex]);

      rc = SHDeleteKey(HKEY_CLASSES_ROOT,OBJECT_NAME_V[objectIndex]);
   }

   return S_OK;
   }
  
  
   long __stdcall Factory::QueryInterface(REFIID iid, void **ppv) { 
   *ppv = NULL; 
   if ( iid == IID_IUnknown || iid == IID_IClassFactory ) 
      *ppv = this; 
   else 
      return E_NOINTERFACE; 
   AddRef(); 
   return S_OK; 
   } 
  
  
   unsigned long __stdcall Factory::AddRef() { 
   return ++refCount; 
   } 
  
  
   unsigned long __stdcall Factory::Release() { 
   return --refCount;
   } 
  
  
   HRESULT STDMETHODCALLTYPE Factory::CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv) { 

   HRESULT hres;

   *ppv = NULL; 

   PrintingBackEnd *pef = new PrintingBackEnd(punkOuter);

   hres = pef -> QueryInterface(riid,ppv);

   if ( ! *ppv ) 
      delete pef;

   return hres;
   } 
  
  
   long __stdcall Factory::LockServer(int fLock) { 
   return S_OK; 
   }
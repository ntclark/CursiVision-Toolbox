
#include <windows.h>
#include <shlwapi.h>
#include <ShlObj.h>

#include <olectl.h>
#include <comcat.h>
#include <stdio.h>

#define DEFINE_DATA

#include "utilities.h"

#include "VideoBackEnd.h"

#include "Properties_i.c"
#include "pdfEnabler_i.c"
#include "VideoBackEnd_i.c"
#include "CursiVision_i.c"

#include "VisioLoggerIntegration_i.c"
#include "VisioLoggerVideo_i.c"

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

      GetLocation(NULL,CSIDL_COMMON_APPDATA,szTemp);

#if 1
      sprintf(szApplicationDataDirectory,"%s\\CursiVision",szTemp);

      CreateDirectory(szApplicationDataDirectory,NULL);

      sprintf(szApplicationDataDirectory,"%s\\CursiVision\\Settings",szTemp);

      CreateDirectory(szApplicationDataDirectory,NULL);

      sprintf(szApplicationDataDirectory,"%s\\CursiVision",szTemp);

      GetLocation(NULL,CSIDL_PERSONAL,szTemp);

      sprintf(szUserDirectory,"%s\\CursiVision Files",szTemp);
#endif

#if 0
      sprintf(szApplicationDataDirectory,"%s\\InnoVisioNate",szTemp);

      CreateDirectory(szApplicationDataDirectory,NULL);

      sprintf(szApplicationDataDirectory,"%s\\InnoVisioNate\\Settings",szTemp);

      CreateDirectory(szApplicationDataDirectory,NULL);

      sprintf(szApplicationDataDirectory,"%s\\InnoVisioNate",szTemp);
#endif

      }

      break;
  
   case DLL_PROCESS_DETACH:
      break;
  
   }
  
   return TRUE;
   }

  
   class Factory : public IClassFactory {

   public:

      Factory(bool isCV) : isCursiVision(isCV) {};
  
      STDMETHOD (QueryInterface)(REFIID riid,void **ppv);
      STDMETHOD_ (ULONG, AddRef)();
      STDMETHOD_ (ULONG, Release)();
      STDMETHOD (CreateInstance)(IUnknown *punkOuter, REFIID riid, void **ppv);
      STDMETHOD (LockServer)(BOOL fLock);
  
      Factory() : refCount(0) {};
      ~Factory() {};
  
   private:

      int refCount;
      bool isCursiVision;

   };
  
  
   Factory *pFactories[2] = { new Factory(true),new Factory(false) };
  
   STDAPI DllCanUnloadNow(void) {
   return S_FALSE;
   }
  
  
   STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppObject) {

   *ppObject = NULL;

   if ( CLSID_CursiVisionVideoBackEnd == rclsid )
      return pFactories[0] -> QueryInterface(riid,ppObject);

   if ( CLSID_VisioLoggerVideoNewRow == rclsid || CLSID_VisioLoggerVideoAction == rclsid || CLSID_VisioLoggerVideoPreSignature == rclsid ) 
      return pFactories[1] -> QueryInterface(riid,ppObject);

   return CLASS_E_CLASSNOTAVAILABLE;
   }
  
   char *OBJECT_NAME;
   char *OBJECT_NAME_V;
   GUID OBJECT_CLSID;

   STDAPI DllRegisterForVisioLogger();

   STDAPI DllRegisterServer() {

   char *OBJECT_VERSION;
   GUID OBJECT_LIBID;
   char *OBJECT_DESCRIPTION;

   OBJECT_NAME = "InnoVisioNate.CursiVisionVideoBackEnd";
   OBJECT_NAME_V = "InnoVisioNate.CursiVisionVideoBackEnd.1";
   OBJECT_VERSION = "1.0";
   memcpy(&OBJECT_CLSID,&CLSID_CursiVisionVideoBackEnd,sizeof(GUID));
   memcpy(&OBJECT_LIBID,&LIBID_CursiVisionVideoBackEnd,sizeof(GUID));
   OBJECT_DESCRIPTION = "CursiVision Camera Tool";

   HRESULT rc = S_OK;
   ITypeLib *ptLib;
   HKEY keyHandle,clsidHandle;
   DWORD disposition;
   char szTemp[256],szCLSID[256];
   LPOLESTR oleString;
  
   StringFromCLSID(OBJECT_CLSID,&oleString);
   WideCharToMultiByte(CP_ACP,0,oleString,-1,szCLSID,256,0,0);
  
   if ( S_OK != LoadTypeLib(wstrModuleName,&ptLib) )
      rc = ResultFromScode(SELFREG_E_TYPELIB);
   else
      if ( S_OK != RegisterTypeLib(ptLib,wstrModuleName,NULL) )
         rc = ResultFromScode(SELFREG_E_TYPELIB);

   RegOpenKeyEx(HKEY_CLASSES_ROOT,"CLSID",0,KEY_CREATE_SUB_KEY,&keyHandle);
  
      RegCreateKeyEx(keyHandle,szCLSID,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&clsidHandle,&disposition);
      sprintf(szTemp,OBJECT_DESCRIPTION);
      RegSetValueEx(clsidHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
  
      sprintf(szTemp,"Control");
      RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
      sprintf(szTemp,"");
      RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
  
      sprintf(szTemp,"ProgID");
      RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
      sprintf(szTemp,OBJECT_NAME_V);
      RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
  
      sprintf(szTemp,"InprocServer");
      RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
      RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szModuleName,(DWORD)strlen(szModuleName));
  
      sprintf(szTemp,"InprocServer32");
      RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
      RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szModuleName,(DWORD)strlen(szModuleName));
//      RegSetValueEx(keyHandle,"ThreadingModel",0,REG_SZ,(BYTE *)"Free",5);
      RegSetValueEx(keyHandle,"ThreadingModel",0,REG_SZ,(BYTE *)"Apartment",9);
  
      sprintf(szTemp,"LocalServer");
      RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
      RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szModuleName,(DWORD)strlen(szModuleName));
    
      sprintf(szTemp,"TypeLib");
      RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
    
      StringFromCLSID(OBJECT_LIBID,&oleString);
      WideCharToMultiByte(CP_ACP,0,oleString,-1,szTemp,256,0,0);
      RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
        
      sprintf(szTemp,"ToolboxBitmap32");
      RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
//      sprintf(szTemp,"%s, 1",szModuleName);
//      RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szModuleName));
  
      sprintf(szTemp,"Version");
      RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
      sprintf(szTemp,OBJECT_VERSION);
      RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
  
      sprintf(szTemp,"MiscStatus");
      RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
      sprintf(szTemp,"0");
      RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
  
      sprintf(szTemp,"1");
      RegCreateKeyEx(keyHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
      sprintf(szTemp,"%ld",
                 OLEMISC_ALWAYSRUN |
                 OLEMISC_ACTIVATEWHENVISIBLE | 
                 OLEMISC_RECOMPOSEONRESIZE | 
                 OLEMISC_INSIDEOUT |
                 OLEMISC_SETCLIENTSITEFIRST |
                 OLEMISC_CANTLINKINSIDE );
      RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
  
   RegCreateKeyEx(HKEY_CLASSES_ROOT,OBJECT_NAME,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
      RegCreateKeyEx(keyHandle,"CurVer",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
      sprintf(szTemp,OBJECT_NAME_V);
      RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
  
   RegCreateKeyEx(HKEY_CLASSES_ROOT,OBJECT_NAME_V,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
      RegCreateKeyEx(keyHandle,"CLSID",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
      RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szCLSID,(DWORD)strlen(szCLSID));
  

   ICatRegister *pICatRegister;

   rc = CoCreateInstance(CLSID_StdComponentCategoriesMgr,NULL,CLSCTX_ALL,IID_ICatRegister,reinterpret_cast<void **>(&pICatRegister));

   CATID categoryId = IID_ICursiVisionBackEnd;

   pICatRegister -> RegisterClassImplCategories(CLSID_CursiVisionVideoBackEnd,1,&categoryId);

   pICatRegister -> Release();

#if 1
   return DllRegisterForVisioLogger();
#else
   return S_OK;
#endif
   }
  
  
   STDAPI DllUnregisterServer() {

   CoInitialize(NULL);

   ICatRegister *pICatRegister;
   long rc = CoCreateInstance(CLSID_StdComponentCategoriesMgr,NULL,CLSCTX_ALL,IID_ICatRegister,reinterpret_cast<void **>(&pICatRegister));
   CATID categoryId = IID_ICursiVisionBackEnd;
   pICatRegister -> UnRegisterClassImplCategories(CLSID_CursiVisionVideoBackEnd,1,&categoryId);
   pICatRegister -> Release();

   OBJECT_NAME = "InnoVisioNate.CursiVisionVideoBackEnd";
   OBJECT_NAME_V = "InnoVisioNate.CursiVisionVideoBackEnd.1";
   memcpy(&OBJECT_CLSID,&CLSID_CursiVisionVideoBackEnd,sizeof(GUID));

   HKEY keyHandle;
   char szCLSID[256];
   LPOLESTR oleString;
  
   StringFromCLSID(OBJECT_CLSID,&oleString);
   WideCharToMultiByte(CP_ACP,0,oleString,-1,szCLSID,256,0,0);
  
   RegOpenKeyEx(HKEY_CLASSES_ROOT,"CLSID",0,KEY_CREATE_SUB_KEY,&keyHandle);

   rc = SHDeleteKey(keyHandle,szCLSID);

   rc = SHDeleteKey(HKEY_CLASSES_ROOT,OBJECT_NAME);

   rc = SHDeleteKey(HKEY_CLASSES_ROOT,OBJECT_NAME_V);

   return S_OK;
   }
  
   char *VL_OBJECT_NAME[] = {"InnoVisioNate.VideoNewRow","InnoVisioNate.VideoAction","InnoVisionate.VideoPreSignature"};
   char *VL_OBJECT_NAME_V[] = {"InnoVisioNate.VideoNewRow.1","InnoVisioNate.VideoAction.1","InnoVisioNate.VideoPreSignature.1"};
   char *VL_OBJECT_DESCRIPTION[] = {"InnoVisioNate Video tool","InnoVisioNate Video tool","InnoVisioNate Video tool"};
   GUID VL_OBJECT_CLSID[] = {CLSID_VisioLoggerVideoNewRow,CLSID_VisioLoggerVideoAction,CLSID_VisioLoggerVideoPreSignature};

   // VisioLogger registration. I may choose not to use this object for both. If I permanently decide to use this
   // object for both, I would combine these register server functions.

   STDAPI DllRegisterForVisioLogger() {

   char *OBJECT_VERSION;
   GUID OBJECT_LIBID;

   OBJECT_VERSION = "1.0";

   memcpy(&OBJECT_LIBID,&LIBID_VisioLoggerVideo,sizeof(GUID));

   HRESULT rc = S_OK;
   ITypeLib *ptLib;
   HKEY keyHandle,clsidHandle;
   DWORD disposition;
   char szTemp[256],szCLSID[256];
   LPOLESTR oleString;
  
   if ( S_OK != LoadTypeLib(wstrModuleName,&ptLib) )
      rc = ResultFromScode(SELFREG_E_TYPELIB);
   else
      if ( S_OK != RegisterTypeLib(ptLib,wstrModuleName,NULL) )
         rc = ResultFromScode(SELFREG_E_TYPELIB);

   for ( long objectIndex = 0; objectIndex < 3; objectIndex++ ) {

      StringFromCLSID(VL_OBJECT_CLSID[objectIndex],&oleString);

      WideCharToMultiByte(CP_ACP,0,oleString,-1,szCLSID,256,0,0);
    
      RegOpenKeyEx(HKEY_CLASSES_ROOT,"CLSID",0,KEY_CREATE_SUB_KEY,&keyHandle);
    
         RegCreateKeyEx(keyHandle,szCLSID,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&clsidHandle,&disposition);
         sprintf(szTemp,VL_OBJECT_DESCRIPTION[objectIndex]);
         RegSetValueEx(clsidHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
    
         sprintf(szTemp,"Control");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         sprintf(szTemp,"");
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
    
         sprintf(szTemp,"ProgID");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         sprintf(szTemp,VL_OBJECT_NAME_V[objectIndex]);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
    
         sprintf(szTemp,"InprocServer");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szModuleName,(DWORD)strlen(szModuleName));
    
         sprintf(szTemp,"InprocServer32");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szModuleName,(DWORD)strlen(szModuleName));
         //RegSetValueEx(keyHandle,"ThreadingModel",0,REG_SZ,(BYTE *)"Free",5);
         RegSetValueEx(keyHandle,"ThreadingModel",0,REG_SZ,(BYTE *)"Apartment",9);
    
         sprintf(szTemp,"LocalServer");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szModuleName,(DWORD)strlen(szModuleName));
       
         sprintf(szTemp,"TypeLib");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
       
         StringFromCLSID(OBJECT_LIBID,&oleString);
         WideCharToMultiByte(CP_ACP,0,oleString,-1,szTemp,256,0,0);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
           
         sprintf(szTemp,"ToolboxBitmap32");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
//         sprintf(szTemp,"%s, 1",szModuleName);
//         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,strlen(szModuleName));
    
         sprintf(szTemp,"Version");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         sprintf(szTemp,OBJECT_VERSION);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
    
         sprintf(szTemp,"MiscStatus");
         RegCreateKeyEx(clsidHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         sprintf(szTemp,"0");
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
    
         sprintf(szTemp,"1");
         RegCreateKeyEx(keyHandle,szTemp,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         sprintf(szTemp,"%ld",
                    OLEMISC_ALWAYSRUN |
                    OLEMISC_ACTIVATEWHENVISIBLE | 
                    OLEMISC_RECOMPOSEONRESIZE | 
                    OLEMISC_INSIDEOUT |
                    OLEMISC_SETCLIENTSITEFIRST |
                    OLEMISC_CANTLINKINSIDE );
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
    
      RegCreateKeyEx(HKEY_CLASSES_ROOT,VL_OBJECT_NAME[objectIndex],0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         RegCreateKeyEx(keyHandle,"CurVer",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         sprintf(szTemp,VL_OBJECT_NAME_V[objectIndex]);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szTemp,(DWORD)strlen(szTemp));
    
      RegCreateKeyEx(HKEY_CLASSES_ROOT,VL_OBJECT_NAME_V[objectIndex],0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         RegCreateKeyEx(keyHandle,"CLSID",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&keyHandle,&disposition);
         RegSetValueEx(keyHandle,NULL,0,REG_SZ,(BYTE *)szCLSID,(DWORD)strlen(szCLSID));

   }


   ICatRegister *pICatRegister;

   rc = CoCreateInstance(CLSID_StdComponentCategoriesMgr,NULL,CLSCTX_ALL,IID_ICatRegister,reinterpret_cast<void **>(&pICatRegister));

   CATID categoryId = IID_IVisioLoggerNewRow;

   pICatRegister -> RegisterClassImplCategories(CLSID_VisioLoggerVideoNewRow,1,&categoryId);

   categoryId = IID_IVisioLoggerAction;

   pICatRegister -> RegisterClassImplCategories(CLSID_VisioLoggerVideoAction,1,&categoryId);

   categoryId = IID_IVisioLoggerPreSignature;

   pICatRegister -> RegisterClassImplCategories(CLSID_VisioLoggerVideoPreSignature,1,&categoryId);

   pICatRegister -> Release();

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

   VideoBackEnd *pef = new VideoBackEnd(punkOuter);

   hres = pef -> QueryInterfaceSpecial(isCursiVision,riid,ppv);

   if ( ! *ppv ) 
      delete pef;

   return hres;
   } 
  
  
   long __stdcall Factory::LockServer(int fLock) { 
   return S_OK; 
   }
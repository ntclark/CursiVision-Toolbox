
#include <Ws2tcpip.h>

#include "forwardToReceptor.h"
#include "resultDisposition.h"

   HRESULT __stdcall forwardToReceptor::Dispose(BSTR bstrOriginalFile,BSTR bstrResultFileName,BSTR topazSignatureData,BSTR graphicDataFile,BSTR dispositionSettingsFileName,BOOL isTempFile) {

   isProcessing = true;

   if ( showProperties || ! szServerName[0] ) {

SetProperties:

      IUnknown *pIUnknown = NULL;
      QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&pIUnknown));
      pIGProperties -> ShowProperties(hwndParent,pIUnknown);
      pIUnknown -> Release();

   }

   if ( ! doExecute )
      return S_OK;

   char szCommand[4096];
   char szFileName[MAX_PATH];

   WSADATA wsaData;
   WSAStartup(MAKEWORD(2,2),&wsaData);

   addrinfo addressInfo = {0};
   addrinfo *pResolvedAddressInfo = NULL;

   addressInfo.ai_flags = 0L;
   addressInfo.ai_family = AF_UNSPEC;
   addressInfo.ai_socktype = SOCK_STREAM;
   addressInfo.ai_protocol = IPPROTO_TCP;
   addressInfo.ai_addrlen = 0;
   addressInfo.ai_addr = NULL;
   addressInfo.ai_canonname = NULL;
   addressInfo.ai_next = NULL;

   sprintf(szCommand,"%ld",portNumber);

   if ( getaddrinfo(szServerName,szCommand,&addressInfo,&pResolvedAddressInfo) ) {
      sprintf(szCommand,"The server \"%s\" using port # %ld was not found.",szServerName,portNumber);
      MessageBox(NULL,szCommand,"CursiVision Forward To Receptor Error!",MB_ICONEXCLAMATION);
      return E_FAIL;
   }

   long rc = 0L;

   SOCKET connectionSocket = INVALID_SOCKET;

   for ( addrinfo *p = pResolvedAddressInfo; p; p = p -> ai_next ) {

      if ( ! ( SOCK_STREAM == p -> ai_socktype ) )
         continue;

      connectionSocket = socket(p -> ai_family,p -> ai_socktype,p -> ai_protocol);

      if ( ! ( INVALID_SOCKET == connectionSocket ) ) {
         if ( SOCKET_ERROR != connect(connectionSocket,p -> ai_addr,(int)p -> ai_addrlen) )
            break;
      }

      connectionSocket = INVALID_SOCKET;

   }

   if ( INVALID_SOCKET == connectionSocket ) {

      for ( addrinfo *p = pResolvedAddressInfo; p; p = p -> ai_next ) {
         connectionSocket = socket(p -> ai_family,p -> ai_socktype,p -> ai_protocol);
         if ( INVALID_SOCKET == connectionSocket ) {
            sprintf(szCommand,"A socket could not be created to connect to %s on port %ld.",szServerName,portNumber);
            MessageBox(NULL,szCommand,"CursiVision Forward To Receptor Error!",MB_ICONEXCLAMATION);
            freeaddrinfo(pResolvedAddressInfo);
            return E_FAIL;
         }
         rc = connect(connectionSocket,p -> ai_addr,(int)p -> ai_addrlen);
         if ( SOCKET_ERROR != rc )
            break;
      }

   }

   freeaddrinfo(pResolvedAddressInfo);

   if ( rc ) {
      if ( 10060 == WSAGetLastError() ) 
         sprintf(szCommand,"The connection to %s on port %ld timed out.\n"
                           "Please ensure the Firewall on that computer allows access to port %ld\n"
                           "and that the CursiVision Receptor service is running.",szServerName,portNumber,portNumber);
      else
         sprintf(szCommand,"A connection could not be made to %s on port %ld, error code = %ld\n"
                           "Please ensure the Firewall on that computer allows access to port %ld\n"
                           "and that the CursiVision Receptor service is running.",szServerName,portNumber,WSAGetLastError(),portNumber);
      MessageBox(NULL,szCommand,"CursiVision Forward To Receptor Error!",MB_ICONEXCLAMATION | MB_TOPMOST);
      return E_FAIL;
   }

   BSTR translatedDispositionSettingsFile = NULL;

   if ( dispositionSettingsFileName ) {
      struct resultDisposition disposition;
      IGProperties *pProperties;
      HRESULT rc = CoCreateInstance(CLSID_InnoVisioNateProperties,NULL,CLSCTX_ALL,IID_IGProperties,reinterpret_cast<void **>(&pProperties));
      pProperties -> Add(L"result disposition",NULL);
      pProperties -> DirectAccess(L"result disposition",TYPE_BINARY,&disposition,sizeof(resultDisposition));
      pProperties -> put_FileName(dispositionSettingsFileName);
      VARIANT_BOOL bSuccess;
      pProperties -> LoadFile(&bSuccess);
      pProperties -> Release();
      translatedDispositionSettingsFile = SysAllocString(_wtempnam(NULL,NULL));
      FILE *fSettings = _wfopen(translatedDispositionSettingsFile,L"wb");
      fwrite(&disposition,1,sizeof(resultDisposition),fSettings);
      fclose(fSettings);
   }

   BSTR filesToSend[] = {bstrResultFileName,translatedDispositionSettingsFile,L""};
   char fileCommand[][64] = {"filesize","profilesize",0};
   char fileType[][64] = {"signed document","document profile",0};

   for ( long k = 0; 1; k++ ) {

      if ( ! filesToSend[k] ) 
         continue;

      if ( 0 == wcslen(filesToSend[k]) )
         break;

      WideCharToMultiByte(CP_ACP,0,filesToSend[k],-1,szFileName,MAX_PATH,0,0);

      FILE *fPDF = fopen(szFileName,"rb");
      fseek(fPDF,0,SEEK_END);
      long fileSize = ftell(fPDF);
      fseek(fPDF,0,SEEK_SET);

      BYTE *pBinary = new BYTE[fileSize];
      fread(pBinary,fileSize,1,fPDF);
      fclose(fPDF);   

      sprintf(szCommand,"%s:%ld",fileCommand[k],fileSize);
      send(connectionSocket,szCommand,(DWORD)strlen(szCommand),0);

      memset(szCommand,0,sizeof(szCommand));
      rc = recv(connectionSocket,szCommand,1024,0);

      if ( 0 > rc || strncmp(szCommand,"send",4) ) {
         sprintf(szCommand,"The CursiVision Receptor on server %s did not accept the %s for processing.",szServerName,fileType[k]);
         MessageBox(NULL,szCommand,"CursiVision Forward To Receptor Error!",MB_ICONEXCLAMATION);
         delete [] pBinary;
         return E_FAIL;
      }

      long totalBytes = 0;
      for ( totalBytes = 0; totalBytes + 1024 < fileSize; totalBytes += 1024 ) 
         send(connectionSocket,(char *)pBinary + totalBytes,1024,0L);

      if ( totalBytes < fileSize )
         send(connectionSocket,(char *)pBinary + totalBytes,fileSize - totalBytes,0L);

      memset(szCommand,0,sizeof(szCommand));
      rc = recv(connectionSocket,szCommand,1024,0);

      if ( 0 > rc || strncmp(szCommand,"received",8) ) {
         sprintf(szCommand,"The CursiVision Receptor on server %s did not accept the %s for processing.",szServerName,fileType[k]);
         MessageBox(NULL,szCommand,"CursiVision Forward To Receptor Error!",MB_ICONEXCLAMATION);
         delete [] pBinary;
         return E_FAIL;
      }

      delete [] pBinary;
   }

   WideCharToMultiByte(CP_ACP,0,bstrResultFileName,-1,szFileName,MAX_PATH,0,0);

   char *pSlash = strrchr(szFileName,'\\');
   if ( ! pSlash )
      pSlash = strrchr(szFileName,'/');
   if ( pSlash )
      sprintf(szCommand,"name %s",pSlash + 1);
   else
      sprintf(szCommand,"name %s",szFileName);

   send(connectionSocket,szCommand,(DWORD)strlen(szCommand),0);

   memset(szCommand,0,sizeof(szCommand));
   rc = recv(connectionSocket,szCommand,1024,0);

   send(connectionSocket,"overwrite",9,0);
   rc = recv(connectionSocket,szCommand,1024,0);

   if ( szNextServerName[0] ) {
      memset(szCommand,0,sizeof(szCommand));
      sprintf(szCommand,"forward %s",szNextServerName);
      send(connectionSocket,szCommand,1024,0);
      memset(szCommand,0,sizeof(szCommand));
      rc = recv(connectionSocket,szCommand,1024,0);
   }

   if ( saveOnly && szServerStoreLocation[0] ) {
      memset(szCommand,0,sizeof(szCommand));
      sprintf(szCommand,"store %s",szServerStoreLocation);
      send(connectionSocket,szCommand,1024,0);
      memset(szCommand,0,sizeof(szCommand));
      rc = recv(connectionSocket,szCommand,1024,0);
   }

   send(connectionSocket,"go",4,0L);

   memset(szCommand,0,sizeof(szCommand));
   rc = recv(connectionSocket,szCommand,1024,0);

   if ( 0 > rc || stricmp(szCommand,"ok") ) {
      char szTemp[1024];
      sprintf(szTemp,"The CursiVisionReceptor on server %s did not accept the signed document for processing.\nThe server reported:\n\n\t%s",szServerName,szCommand);
      MessageBox(NULL,szTemp,"CursiVision Forward To Receptor Error!",MB_ICONEXCLAMATION);
      return E_FAIL;
   }

   shutdown(connectionSocket,SD_SEND);
   closesocket(connectionSocket);

   if ( translatedDispositionSettingsFile ) {
      DeleteFileW(translatedDispositionSettingsFile);
      SysFreeString(translatedDispositionSettingsFile);
   }

   return S_OK;
   }


   HRESULT __stdcall forwardToReceptor::put_PropertiesFileName(BSTR propertiesFileName) {
   pIGProperties -> put_FileName(propertiesFileName);
   short bSuccess;
   pIGProperties -> LoadFile(&bSuccess);
   pIGProperties -> Save();
   return S_OK;
   }


   HRESULT __stdcall forwardToReceptor::get_PropertiesFileName(BSTR *pPropertiesFileName) {
   return pIGProperties -> get_FileName(pPropertiesFileName);
   }


   HRESULT __stdcall forwardToReceptor::get_CodeName(BSTR *pPropertiesRootName) {
   if ( ! pPropertiesRootName )
      return E_POINTER;
   *pPropertiesRootName = SysAllocString(L"cvbeForward");
   return S_OK;
   }


   HRESULT __stdcall forwardToReceptor::get_SavedDocumentName(BSTR *pDocumentName) {
   return E_NOTIMPL;
   }

   HRESULT __stdcall forwardToReceptor::put_ParentWindow(HWND hp) {
   hwndParent = hp;
   return S_OK;
   }

   HRESULT __stdcall forwardToReceptor::put_CommandLine(BSTR theCommandLine) {
   return E_NOTIMPL;
   }
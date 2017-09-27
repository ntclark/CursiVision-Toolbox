
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ws2tcpip.h>
#include <Mswsock.h>
#include <richedit.h>

char szMessage[1024];
char szLog[2048];
char szCommand[32768];

   DWORD __stdcall loadLog(DWORD_PTR dwCookie,BYTE *pBuffer,LONG bufferSize,LONG *pBytesReturned);

   extern "C" long sendDocument(char *serverName,long ftpPort,char *userName,char *password,char *pszAttachment,HWND hwndLog) {

   char szPort[32];
   sprintf(szPort,"%ld",ftpPort);

   addrinfo validOptions = {0};
   addrinfo *pCommandAddress;
   addrinfo *pDataAddress;

   validOptions.ai_family = AF_INET;
   validOptions.ai_socktype = SOCK_STREAM;

   WSADATA wsaData = {0};
   WORD wsaVersion = MAKEWORD( 1, 1 );

   WSAStartup(wsaVersion,&wsaData);

   long foundHost = getaddrinfo(serverName,szPort,&validOptions,&pCommandAddress);

   if ( foundHost ) {
      sprintf(szMessage,"\n\nThe system was unable to resolve the address for host: %s\n\n"
                           "\nPress Retry to specify the properties or Cancel to exit.",serverName);
      WSACleanup();
      return IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);
   }

   long theCommandSocket = socket(pCommandAddress -> ai_family,pCommandAddress -> ai_socktype,pCommandAddress -> ai_protocol);

   if ( -1L == theCommandSocket ) {
      sprintf(szMessage,"\nERROR: The system was not able to create a socket for the FTP.\n"
                           "\nPress Retry to specify the properties or Cancel to exit.");
      WSACleanup();
      return IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);
   }  

   if ( -1L == connect(theCommandSocket,pCommandAddress -> ai_addr,pCommandAddress -> ai_addrlen) ) {
      sprintf(szMessage,"\n\nERROR: The system was not able to connect the socket for the FTP, \n\n\trc = %ld\n\n"
                           "\nPress Retry to specify the properties or Cancel to exit.",WSAGetLastError());
      WSACleanup();
      return IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);
   }

   char szInput[1024];

   EDITSTREAM editStream;
   memset(&editStream,0,sizeof(EDITSTREAM));
   editStream.pfnCallback = loadLog;
   editStream.dwCookie = NULL;

   memset(szInput,0,sizeof(szInput));
   recv(theCommandSocket,szInput,1024,0L);
   if ( hwndLog ) {
      sprintf(szLog,"\n%s",szInput);
      SendMessage(hwndLog,EM_STREAMIN,(WPARAM)(SF_TEXT | SFF_SELECTION),(LPARAM)&editStream);
      long countcr = 0;
      char *px = szLog;
      while ( strchr(px,'0x0A') ) {
         countcr++;
         px++;
      }
      SendMessage(hwndLog,EM_LINESCROLL,(WPARAM)0,(LPARAM)countcr);
   }

   sprintf(szCommand,"user %s\n",userName);
   send(theCommandSocket,szCommand,strlen(szCommand),0L);

   memset(szInput,0,sizeof(szInput));
   recv(theCommandSocket,szInput,1024,0L);

   if ( hwndLog ) {
      sprintf(szLog,"\n%s",szInput);
      long countcr = 0;
      char *px = szLog;
      while ( strchr(px,'0x0A') ) {
         countcr++;
         px++;
      }
      SendMessage(hwndLog,EM_STREAMIN,(WPARAM)(SF_TEXT | SFF_SELECTION),(LPARAM)&editStream);
      SendMessage(hwndLog,EM_LINESCROLL,(WPARAM)0,(LPARAM)countcr);
   }

   if ( '4' == szInput[0] || '5' == szInput[0] ) {
      sprintf(szMessage,"\n\nERROR: The FTP Server was found, however, it responded with the following to the USER command for %s:\n\n%s\n\n"
                           "\nPress Retry to specify the properties or Cancel to exit.",userName,szInput);
      closesocket(theCommandSocket);
      WSACleanup();
      return IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);

   }

   sprintf(szCommand,"pass %s\n",password);
   send(theCommandSocket,szCommand,strlen(szCommand),0L);

   memset(szInput,0,sizeof(szInput));
   recv(theCommandSocket,szInput,1024,0L);

   if ( hwndLog ) {
      sprintf(szLog,"\n%s",szInput);
      long countcr = 0;
      char *px = szLog;
      while ( strchr(px,'0x0A') ) {
         countcr++;
         px++;
      }
      SendMessage(hwndLog,EM_STREAMIN,(WPARAM)(SF_TEXT | SFF_SELECTION),(LPARAM)&editStream);
      SendMessage(hwndLog,EM_LINESCROLL,(WPARAM)0,(LPARAM)countcr);
   }

   if ( '4' == szInput[0] || '5' == szInput[0] ) {
      sprintf(szMessage,"\n\nERROR: The FTP Server was found, however, it responded with the following to the PASS command:\n\n%s\n\n"
                           "\nPress Retry to specify the properties or Cancel to exit.",szInput);
      closesocket(theCommandSocket);
      WSACleanup();
      return IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);

   }

   sprintf(szCommand,"MODE S\n");
   send(theCommandSocket,szCommand,strlen(szCommand),0L);

   memset(szInput,0,sizeof(szInput));
   recv(theCommandSocket,szInput,1024,0L);
   if ( hwndLog ) {
      sprintf(szLog,"\n%s",szInput);
      long countcr = 0;
      char *px = szLog;
      while ( strchr(px,'0x0A') ) {
         countcr++;
         px++;
      }
      SendMessage(hwndLog,EM_STREAMIN,(WPARAM)(SF_TEXT | SFF_SELECTION),(LPARAM)&editStream);
      SendMessage(hwndLog,EM_LINESCROLL,(WPARAM)0,(LPARAM)countcr);
   }

   sprintf(szCommand,"TYPE Image\n");
   send(theCommandSocket,szCommand,strlen(szCommand),0L);

   memset(szInput,0,sizeof(szInput));
   recv(theCommandSocket,szInput,1024,0L);
   if ( hwndLog ) {
      sprintf(szLog,"\n%s",szInput);
      long countcr = 0;
      char *px = szLog;
      while ( strchr(px,'0x0A') ) {
         countcr++;
         px++;
      }
      SendMessage(hwndLog,EM_STREAMIN,(WPARAM)(SF_TEXT | SFF_SELECTION),(LPARAM)&editStream);
      SendMessage(hwndLog,EM_LINESCROLL,(WPARAM)0,(LPARAM)countcr);
   }

   sprintf(szCommand,"STRU F\n");
   send(theCommandSocket,szCommand,strlen(szCommand),0L);

   memset(szInput,0,sizeof(szInput));
   recv(theCommandSocket,szInput,1024,0L);
   if ( hwndLog ) {
      sprintf(szLog,"\n%s",szInput);
      long countcr = 0;
      char *px = szLog;
      while ( strchr(px,'0x0A') ) {
         countcr++;
         px++;
      }
      SendMessage(hwndLog,EM_STREAMIN,(WPARAM)(SF_TEXT | SFF_SELECTION),(LPARAM)&editStream);
      SendMessage(hwndLog,EM_LINESCROLL,(WPARAM)0,(LPARAM)countcr);
   }

   char szServerUpload[32];

   sprintf(szCommand,"pasv\n");
   send(theCommandSocket,szCommand,strlen(szCommand),0L);

   memset(szInput,0,sizeof(szInput));
   recv(theCommandSocket,szInput,1024,0L);
   if ( hwndLog ) {
      sprintf(szLog,"\n%s",szInput);
      long countcr = 0;
      char *px = szLog;
      while ( strchr(px,'0x0A') ) {
         countcr++;
         px++;
      }
      SendMessage(hwndLog,EM_STREAMIN,(WPARAM)(SF_TEXT | SFF_SELECTION),(LPARAM)&editStream);
      SendMessage(hwndLog,EM_LINESCROLL,(WPARAM)0,(LPARAM)countcr);
   }

   long serverPort[2] = {0L,0L};
   char *px = strchr(szInput,'(') + 1;

   long ipParts[4];

   ipParts[0] = atol(strtok(px,","));
   ipParts[1] = atol(strtok(NULL,","));
   ipParts[2] = atol(strtok(NULL,","));
   ipParts[3] = atol(strtok(NULL,","));
   serverPort[0] = atol(strtok(NULL,","));
   serverPort[1] = atol(strtok(NULL,","));

   sprintf(szPort,"%ld",serverPort[0] * 256 + serverPort[1]);
   sprintf(szServerUpload,"%ld.%ld.%ld.%ld",ipParts[0],ipParts[1],ipParts[2],ipParts[3]);

   foundHost = getaddrinfo(szServerUpload,szPort,&validOptions,&pDataAddress);

   long theDataSocket = socket(pDataAddress -> ai_family,pDataAddress -> ai_socktype,pDataAddress -> ai_protocol);

   if ( -1L == theDataSocket ) {
      sprintf(szMessage,"\nERROR: The system was not able to create a socket for the FTP server.\n"
                           "\nPress Retry to specify the properties or Cancel to exit.");
      WSACleanup();
      return IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);
   }  

   if ( -1L == connect(theDataSocket,pDataAddress -> ai_addr,pDataAddress -> ai_addrlen) ) {
      sprintf(szMessage,"\n\nERROR: The system was not able to connect the socket for the FTP server, \n\n\trc = %ld\n\n"
                           "\nPress Retry to specify the properties or Cancel to exit.",WSAGetLastError());
      WSACleanup();
      return IDCANCEL == MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);
   }

   char *pRootName = strrchr(pszAttachment,'\\');
   if ( ! pRootName )
      pRootName = pszAttachment - 1;

   pRootName++;

   sprintf(szCommand,"STOR %s\n",pRootName);
   send(theCommandSocket,szCommand,strlen(szCommand),0L);

   memset(szInput,0,sizeof(szInput));
   recv(theCommandSocket,szInput,1024,0L);
   if ( hwndLog ) {
      sprintf(szLog,"\n%s",szInput);
      long countcr = 0;
      char *px = szLog;
      while ( strchr(px,'0x0A') ) {
         countcr++;
         px++;
      }
      SendMessage(hwndLog,EM_STREAMIN,(WPARAM)(SF_TEXT | SFF_SELECTION),(LPARAM)&editStream);
      SendMessage(hwndLog,EM_LINESCROLL,(WPARAM)0,(LPARAM)countcr);
   }

   HANDLE hFile = CreateFile(pszAttachment,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
   BOOL rc = TransmitFile(theDataSocket,hFile,0,0,NULL,NULL,0L);
   CloseHandle(hFile);

   closesocket(theDataSocket);

   sprintf(szCommand,"BYE\r\n");
   send(theCommandSocket,szCommand,strlen(szCommand),0L);

//   memset(szInput,0,sizeof(szInput));
//   recv(theCommandSocket,szInput,1024,0L);
   if ( hwndLog ) {
      sprintf(szLog,"\nThe file: %s has been sent",pRootName);
      SendMessage(hwndLog,EM_STREAMIN,(WPARAM)(SF_TEXT | SFF_SELECTION),(LPARAM)&editStream);
      SendMessage(hwndLog,EM_LINESCROLL,(WPARAM)0,(LPARAM)1);
   }

   closesocket(theCommandSocket);

   WSACleanup();

   return 1;
   }

   DWORD __stdcall loadLog(DWORD_PTR dwCookie,BYTE *pBuffer,LONG bufferSize,LONG *pBytesReturned) {
   if ( ! szLog[0] )
      return 1L;
   *pBytesReturned = strlen(szLog);
   memcpy(pBuffer,szLog,*pBytesReturned);
   szLog[0] = '\0';
   return 0L;
   }

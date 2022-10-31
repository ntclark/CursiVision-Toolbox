// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ws2tcpip.h>

#include "..\emailUtilities.h"

#include "TLSEncryption.h"

#define IDX_FROM            0
#define IDX_REPLY_TO        1
#define IDX_SUBJECT         2
#define IDX_CONTENT_TYPE    3
#define IDX_CC              4
#define IDX_MIME_VERSION    5

#define COMMAND_SIZE  1024

#define WINSOCK_ERROR                           \
    DWORD errorVal = WSAGetLastError();         \
    VOID *lpBuffer;                             \
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,errorVal,0L,(LPTSTR)&lpBuffer,1024,NULL);  \
    strcpy_s<1024>(szError,(char *)lpBuffer);   \
    LocalFree(lpBuffer); 

#define DO_SEND(s,r) { if ( useTLS ) pTLS -> send(s,r); else send(s,r); }

    
char szSubject[1024];
char szException[16384];

static char szCommand[1024];

char szMessage[1024];

extern "C" int to64(FILE *infile, FILE *outfile, long limit);

extern "C" int mpack(char *,char *,char *);

extern "C" long sendMail(char *serverName,long smtpPort,char *userName,char *password,
                           char *pszAttachment,char *pszOriginalName,
                           char *fromAddress,char *pszTo,char *pszCC,char *pszBCC,char *pszSubject,char *pszBody,boolean useTLS) {

   char szEmailFileName[MAX_PATH];
   char szPackedAttachmentFileName[MAX_PATH];

   strcpy(szEmailFileName,_tempnam(NULL,NULL));

   strcpy(szPackedAttachmentFileName,_tempnam(NULL,NULL));

   mpack(pszAttachment,"application/pdf",szPackedAttachmentFileName);

   FILE *fTemp = fopen(szPackedAttachmentFileName,"rb");

   fseek(fTemp,0,SEEK_END);

   long attachmentSize = ftell(fTemp);

   BYTE *bAttachment = new BYTE[attachmentSize + 1];

   bAttachment[attachmentSize] = '\0';

   fseek(fTemp,0,SEEK_SET);

   fread(bAttachment,attachmentSize,1,fTemp);

   fclose(fTemp);

   DeleteFile(szPackedAttachmentFileName);

   FILE *fInput = fopen(szEmailFileName,"wb+");

   fprintf(fInput,"From: %s\x0D\x0A",fromAddress);
   fprintf(fInput,"User-Agent: CursiVision E-Mail back end tool\x0D\x0A");
   fprintf(fInput,"MIME-Version: 1.0\x0D\x0A");
   fprintf(fInput,"Reply-to: %s\x0D\x0A",fromAddress);
   fprintf(fInput,"To: %s\x0D\x0A",pszTo);

   fprintf(fInput,"Subject: %s\x0D\x0A",pszSubject);

   fprintf(fInput,"Content-Type: multipart/mixed;\x0D\x0A");
   fprintf(fInput," boundary=\"------------030609080107030205080300\"\x0D\x0A");
   fprintf(fInput,"Content-Language: en-US\x0D\x0A\x0D\x0A");

   fprintf(fInput,"This is a multi-part message in MIME format.\x0D\x0A"
                  "--------------030609080107030205080300\x0D\x0A"
                  "Content-Type: text/plain; charset=ISO-8859-1; format=flowed\x0D\x0A"
                  "Content-Transfer-Encoding: 7bit\x0D\x0A\x0D\x0A");

   if ( ! ( NULL == pszBody ) ) 
      fprintf(fInput,"%s\x0D\x0A",pszBody);

   fprintf(fInput,"\x0D\x0A--------------030609080107030205080300\x0D\x0A");

   char *pBaseName = NULL;
   char *pOriginalName = pszOriginalName;

   if ( strrchr(pOriginalName,'\\') )
      pOriginalName = strrchr(pOriginalName,'\\') + 1;

   fprintf(fInput,"--------------030609080107030205080300\x0D\x0A"
                  "Content-Type: application/pdf;\x0D\x0A"
                  " name=\"%s\"\x0D\x0A"
                  "Content-Transfer-Encoding: base64\x0D\x0A"
                  "Content-Disposition: attachment;\x0D\x0A"
                  " filename=\"%s\"\x0D\x0A\x0D\x0A",pOriginalName,pOriginalName);

   size_t headerSize = ftell(fInput);

   fwrite(bAttachment,1,attachmentSize,fInput);

   delete [] bAttachment;

   fprintf(fInput,"--------------030609080107030205080300--\x0D\x0A\x0D\x0A\x0D\x0A\x0D\x0A");

   char *pszHeader = new char[headerSize + 1];

   pszHeader[headerSize] = '\0';

   fseek(fInput,0,SEEK_SET);

   fread(pszHeader,headerSize,1,fInput);

   fclose(fInput);

   char *pszLowerHeader = new char[headerSize + 1];

   pszLowerHeader[headerSize] = '\0';

   memcpy(pszLowerHeader,pszHeader,headerSize);

   char *p = pszLowerHeader;
   char *pEnd = pszLowerHeader + headerSize;
   while ( p < pEnd && *p ) {
      *p = tolower(*p);
      p++;
   }

   char szHeaderNames[][16] = {"from:","reply-to:","subject:","content-type:","cc:","MIME-Version:","\0"};
   
   char *pHeaderComponents[] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
   
   for ( long k = 0; 1; k++ ) {
       
      if ( ! szHeaderNames[k][0] ) 
         break;

      pHeaderComponents[k] = strstr(pszLowerHeader,szHeaderNames[k]);
      if ( pHeaderComponents[k] ) {
         if ( pHeaderComponents[k] != pszLowerHeader ) {
            if ( *(pHeaderComponents[k] - 1) != '\x0A' ) {
               pHeaderComponents[k] = strchr(pHeaderComponents[k],'\x0A');
               if ( pHeaderComponents[k] ) {
                   pHeaderComponents[k] = strstr(pHeaderComponents[k],szHeaderNames[k]);
                   if ( pHeaderComponents[k] && *(pHeaderComponents[k] - 1) != '\x0A' ) 
                       pHeaderComponents[k] = NULL;
               }
            }
         }
      }
       
   }
   
   if ( ! pHeaderComponents[IDX_FROM] || ! pHeaderComponents[IDX_SUBJECT]  ) {
      sprintf(szMessage,"\nERROR: One or more of the %s or %s fields is missing in an e-mail.\n\tBoth of these are required.\n"
                           "\nPress Retry to specify the properties or Cancel to exit.",
                                szHeaderNames[IDX_FROM],szHeaderNames[IDX_SUBJECT]);
      long rc = MessageBox(NULL,szMessage,"Error!",MB_RETRYCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
      delete [] pszHeader;
      delete [] pszLowerHeader;
      return rc == IDCANCEL;
   }

   for ( long k = 0; 1; k++ ) {

      if ( ! szHeaderNames[k][0] ) 
         break;
   
      if ( ! pHeaderComponents[k] ) 
         continue;

      pHeaderComponents[k] = pszHeader + (pHeaderComponents[k] - pszLowerHeader);

   }

   char szPort[32];
   sprintf(szPort,"%ld",smtpPort);

   addrinfo validOptions = {0};
   addrinfo *pCommandAddress;

    validOptions.ai_family = AF_INET;
    validOptions.ai_socktype = SOCK_STREAM;
    validOptions.ai_protocol = IPPROTO_TCP;

    WSADATA wsaData = {0};
    WORD wsaVersion = MAKEWORD( 2, 2 );

   WSAStartup(wsaVersion,&wsaData);

   long notFoundHost = getaddrinfo(serverName,szPort,&validOptions,&pCommandAddress);
    
   memset(szSubject,0,sizeof(szSubject));
   strncpy(szSubject,pHeaderComponents[IDX_SUBJECT],min(strlen(pHeaderComponents[IDX_SUBJECT]),128));

   if ( strchr(szSubject,'\n') ) 
      *(strchr(szSubject,'\n')) = '\0';
                 

   if ( notFoundHost ) {
      sprintf(szMessage,"\n\nThe system was unable to resolve the address for host: %s\n\n"
                           "\nPress Retry to specify the properties or Cancel to exit.",serverName);
      long rc = MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);
      delete [] pszLowerHeader;
      delete [] pszHeader;
      DeleteFile(szEmailFileName);
      return rc == IDCANCEL;
   }
    
    SOCKET theSocket = socket(pCommandAddress -> ai_family,pCommandAddress -> ai_socktype,pCommandAddress -> ai_protocol);

    if ( -1L == theSocket ) {
        sprintf(szMessage,"\nERROR: Was not able to create a socket for sending email.\n"
                            "\nPress Retry to specify the properties or Cancel to exit.");
        long rc = MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);
        delete [] pszLowerHeader;
        delete [] pszHeader;
        DeleteFile(szEmailFileName);
        return rc == IDCANCEL;
    }  

    if ( -1L == connect(theSocket,pCommandAddress -> ai_addr,(int)pCommandAddress -> ai_addrlen) ) {

        sprintf(szMessage,"\n\nERROR: Was not able to connect the socket for sending email, rc = %ld: %s.\n\n"
                            "\nPress Retry to specify the properties or Cancel to exit.",errno,strerror(errno));
        long rc = MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);
        delete [] pszLowerHeader;
        delete [] pszHeader;
        DeleteFile(szEmailFileName);
        return rc == IDCANCEL;
    }

    char szInput[1024];

    memset(szInput,0,sizeof(szInput));
    recv(theSocket,szInput,1024,0L);

    char outputCommands[2][32];
    char responses[2][16];

    strcpy(outputCommands[0],"EHLO %s\r\n");
    strcpy(responses[0],"250");

    if ( useTLS ) {
         strcpy(outputCommands[1],"STARTTLS\r\n");
         strcpy(responses[1],"220");
    } else {
         strcpy(outputCommands[1],"MAIL From:%s\r\n");
         strcpy(responses[1],"250");
    }
   
   for ( long k = 0; k < 2; k++ ) { 
   
      if ( 0 == k ) 
         sprintf(szCommand,outputCommands[k],serverName);
      else 
         sprintf(szCommand,outputCommands[k],fromAddress);

      send(theSocket,szCommand,(DWORD)strlen(szCommand),0L);
      memset(szInput,0,sizeof(szInput));
      recv(theSocket,szInput,1024,0L);

      if ( strncmp(szInput,responses[k],3) ) {

         sprintf(szMessage,"\nERROR(%s,%ld): The SMTP Server sent an unexpected response to this input\n"
                                    "Sent to server:%sRecieved from server:%sExpected response:%s\n"
                                    "\nPress Retry to specify the properties or Cancel to exit.",
                                    __FILE__,__LINE__,outputCommands[k],szInput,responses[k]);

         long rc = MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);

         closesocket(theSocket);

         delete [] pszLowerHeader;
         delete [] pszHeader;
   
         DeleteFile(szEmailFileName);

         return rc == IDCANCEL;
      }

   }

    TLSEncryption *pTLS = NULL;

    if ( useTLS ) {

        pTLS = new TLSEncryption(theSocket,serverName);

        sprintf_s<COMMAND_SIZE>(szCommand,"EHLO %s\r\n",serverName);
        pTLS -> exchange(szCommand,strlen(szCommand),szInput);

        sprintf_s<COMMAND_SIZE>(szCommand,"AUTH LOGIN\r\n");
        pTLS -> exchange(szCommand,strlen(szCommand),szInput);	
  
        if ( strstr(szInput,"334") ) {
        
            sprintf_s<COMMAND_SIZE>(szCommand,"%s",userName);
            pTLS -> Base64Encode(szCommand,strlen(szCommand));
            pTLS -> exchange(szCommand,strlen(szCommand),szInput);

            if ( strstr(szInput,"334") ) {
                sprintf_s<COMMAND_SIZE>(szCommand,"%s",password);
                pTLS -> Base64Encode(szCommand,strlen(szCommand));
                pTLS -> exchange(szCommand,strlen(szCommand),szInput);
            }

            sprintf_s<COMMAND_SIZE>(szCommand,"MAIL FROM: <%s>\r\n",fromAddress);
            pTLS -> exchange(szCommand,strlen(szCommand),szInput);

        }

    }

   memset(szInput,0,sizeof(szInput));
 
    char *recipientList[] = {pszTo,pHeaderComponents[IDX_CC],pszCC,pszBCC};

    for ( long k = 0; k < sizeof(recipientList) / sizeof(char *); k++ ) {

        if ( ! recipientList[k] )
            continue;
       
        if ( ! recipientList[k][0] )
            continue;
       
        char *pszTokens = new char[strlen(recipientList[k]) + 1];
        memset(pszTokens,0,(strlen(recipientList[k]) + 1) * sizeof(char));
        strcpy(pszTokens,recipientList[k]);
       
        char *p = strchr(pszTokens,'\n');
        if ( p )
            *p = '\0';

        p = strchr(pszTokens,':');
        if ( p )
            p = p + 1;
        else
            p = pszTokens;

        p = strtok(p,",");

        while ( p ) {

            while ( ' ' == *p ) p++;

            sprintf(szCommand,"RCPT TO:<%s>\r\n",p);

            if ( useTLS ) {
              pTLS -> exchange(szCommand,(DWORD)strlen(szCommand),szInput);
            } else {
                send(theSocket,szCommand,(DWORD)strlen(szCommand),0L);
                memset(szInput,0,sizeof(szInput));
                recv(theSocket,szInput,1024,0L);
            }

            if ( szInput[0] ) {

               if ( strncmp(szInput,"250",3) ) {

                  sprintf(szMessage,"\nThe SMTP Server sent an unexpected response to this input\n"
                                             "Sent to server:%sRecieved from server:%s\n"
                                             "\nPress Retry to change the settings or Cancel to exit.",
                                                szCommand,szInput);

                  long rc = MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);

                  closesocket(theSocket);

                  delete [] pszTokens;
                  delete [] pszLowerHeader;
                  delete [] pszHeader;
   
                  DeleteFile(szEmailFileName);

                  return rc == IDCANCEL;

               }
               
           }

           p = strtok(NULL,",");
       }
       
       delete [] pszTokens;
       
   }
   
    sprintf(szCommand,"DATA\r\n");

    if ( useTLS )
        pTLS -> exchange(szCommand,(DWORD)strlen(szCommand),szInput);
    else {
        send(theSocket,szCommand,(DWORD)strlen(szCommand),0L);
        memset(szInput,0,sizeof(szInput));
        recv(theSocket,szInput,1024,0L);
    }

    fInput = fopen(szEmailFileName,"rb");

    fseek(fInput,0,SEEK_END);

    long emailBytes = ftell(fInput);

    fseek(fInput,0,SEEK_SET);

    char *pszEmail = new char[emailBytes + 16L];
   
    fread(pszEmail,emailBytes,1,fInput);

    fclose(fInput);

    if ( useTLS ) {

        char *pStart = (char *)pszEmail;

        char *pEnd = pStart + emailBytes;

        while ( pStart < pEnd ) {
            int n = min(1024,pEnd - pStart);
            pTLS -> send(pStart,n);
            pStart += n;
        }

        pTLS -> exchange((char *)"\r\n.\r\n",strlen("\r\n.\r\n"),szInput);
   
        sprintf_s<COMMAND_SIZE>(szCommand,"QUIT\r\n");

        pTLS -> send(szCommand,(DWORD)strlen(szCommand));

    } else {

        sprintf(pszEmail + emailBytes,"\r\n.\r\n");
        send(theSocket,pszEmail,emailBytes + 5,0L);
        memset(szInput,0,sizeof(szInput));
        recv(theSocket,szInput,1024,0L);
        sprintf(szCommand,"QUIT\r\n");
        send(theSocket,szCommand,(DWORD)strlen(szCommand),0L);

    }

    closesocket(theSocket);

    DeleteFile(szEmailFileName);
   
    delete [] pszLowerHeader;
    delete [] pszHeader;
    delete [] pszEmail;

    return 1;

    }
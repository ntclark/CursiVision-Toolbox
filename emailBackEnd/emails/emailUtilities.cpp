
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ws2tcpip.h>

#include "..\emailUtilities.h"

#define IDX_FROM            0
#define IDX_REPLY_TO        1
#define IDX_SUBJECT         2
#define IDX_CONTENT_TYPE    3
#define IDX_CC              4
#define IDX_MIME_VERSION    5
    
char szSubject[1024];
char szException[16384];
char szCommand[1024];

char szMessage[1024];

extern "C" int to64(FILE *infile, FILE *outfile, long limit);

extern "C" int mpack(char *,char *,char *);

extern "C" long sendMail(char *serverName,long smtpPort,char *userName,char *password,
                           char *pszAttachment,char *pszOriginalName,
                           char *fromAddress,char *pszTo,char *pszCC,char *pszBCC,char *pszSubject,char *pszBody) {

   char szEmailFileName[MAX_PATH];
   char szAttachmentFileName[MAX_PATH];

   strcpy(szEmailFileName,_tempnam(NULL,NULL));
   strcpy(szAttachmentFileName,_tempnam(NULL,NULL));

   mpack(pszAttachment,"application/pdf",szAttachmentFileName);

   FILE *fTemp = fopen(szAttachmentFileName,"rb");
   fseek(fTemp,0,SEEK_END);
   long attachmentSize = ftell(fTemp);
   BYTE *bAttachment = new BYTE[attachmentSize + 1];
   bAttachment[attachmentSize] = '\0';
   fseek(fTemp,0,SEEK_SET);
   fread(bAttachment,attachmentSize,1,fTemp);
   fclose(fTemp);
   DeleteFile(szAttachmentFileName);

   FILE *fInput = fopen(szEmailFileName,"wb+");

   fprintf(fInput,"From: %s\x0D\x0A",fromAddress);
   fprintf(fInput,"User-Agent: CursiVision E-Mail back end tool\x0D\x0A");
   fprintf(fInput,"MIME-Version: 1.0\x0D\x0A");
   fprintf(fInput,"Reply-to: %s\x0D\x0A",fromAddress);
   fprintf(fInput,"To: %s\x0D\x0A",pszTo);

   fprintf(fInput,"Subject: %s\x0D\x0A",pszSubject);

   fprintf(fInput,"Content-Type: multipart/mixed;\x0D\x0A");
   fprintf(fInput," boundary=\"------------030609080107030205080300\"\x0D\x0A\x0D\x0A");

   fprintf(fInput,"This is a multi-part message in MIME format.\x0D\x0A"
                  "--------------030609080107030205080300\x0D\x0A"
                  "Content-Type: text/plain; charset=ISO-8859-1; format=flowed\x0D\x0A"
                  "Content-Transfer-Encoding: 7bit\x0D\x0A\x0D\x0A");

   fprintf(fInput,"%s\x0D\x0A",pszBody);

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

   long cntHeaderBytes = ftell(fInput);

   fwrite(bAttachment,attachmentSize,1,fInput);

   delete [] bAttachment;

   fprintf(fInput,"--------------030609080107030205080300--\x0D\x0A\x0D\x0A\x0D\x0A\x0D\x0A");

   char *pszHeader = new char[cntHeaderBytes + 1];

   pszHeader[cntHeaderBytes] = '\0';

   fseek(fInput,0,SEEK_SET);

   fread(pszHeader,cntHeaderBytes,1,fInput);

   fseek(fInput,0,SEEK_SET);

   char *pszLowerHeader = new char[cntHeaderBytes + 1];

   pszLowerHeader[cntHeaderBytes] = '\0';

   memcpy(pszLowerHeader,pszHeader,cntHeaderBytes);

   char *p = pszLowerHeader;
   char *pEnd = pszLowerHeader + cntHeaderBytes;
   while ( p < pEnd && *p ) {
      *p = tolower(*p);
      p++;
   }

#if 0
   char authenticationString[512];
   
   FILE *fEncodingInput = fopen("temp1","wt");
   FILE *fEncodingOutput = fopen("temp2","wt");
   char szChar[] = {'\0'};
   fwrite(szChar,1,1,fEncodingInput);
   fwrite(userName,1,(DWORD)strlen(userName),fEncodingInput);
   fwrite(szChar,1,1,fEncodingInput);
   fwrite(password,1,(DWORD)strlen(password),fEncodingInput);
   fclose(fEncodingInput);
   fEncodingInput = fopen("temp1","rt");
   to64(fEncodingInput,fEncodingOutput,0);
   fclose(fEncodingInput);
   fclose(fEncodingOutput);
   fEncodingInput = fopen("temp2","rt");
   fgets(authenticationString,512,fEncodingInput);
   fclose(fEncodingInput);

   DeleteFile("temp1");
   DeleteFile("temp2");
#endif

#if 1
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

#endif

#if 0
   struct sockaddr_in socketDetails;
   struct hostent *pMailHost;
#endif

   char szPort[32];
   sprintf(szPort,"%ld",smtpPort);

   addrinfo validOptions = {0};
   addrinfo *pCommandAddress;

   validOptions.ai_family = AF_INET;
   validOptions.ai_socktype = SOCK_STREAM;

   WSADATA wsaData = {0};
   WORD wsaVersion = MAKEWORD( 1, 1 );

   WSAStartup(wsaVersion,&wsaData);

   long foundHost = getaddrinfo(serverName,szPort,&validOptions,&pCommandAddress);
    
#if 1
   memset(szSubject,0,sizeof(szSubject));
   strncpy(szSubject,pHeaderComponents[IDX_SUBJECT],min(strlen(pHeaderComponents[IDX_SUBJECT]),128));
#else
   strcpy(szSubject,pszSubject);
#endif

   if ( strchr(szSubject,'\n') ) 
      *(strchr(szSubject,'\n')) = '\0';
                 
#if 0
   pMailHost = gethostbyname(serverName);
    
   if ( NULL == pMailHost ) {
#else
   if ( foundHost ) {
#endif
      sprintf(szMessage,"\n\nThe system was unable to resolve the address for host: %s\n\n"
                           "\nPress Retry to specify the properties or Cancel to exit.",serverName);
      long rc = MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);
      delete [] pszLowerHeader;
      delete [] pszHeader;
      fclose(fInput);
      DeleteFile(szEmailFileName);
      return rc == IDCANCEL;
   }
    
#if 0
   SOCKET theSocket = socket(PF_INET,SOCK_STREAM,0);
#else
   SOCKET theSocket = socket(pCommandAddress -> ai_family,pCommandAddress -> ai_socktype,pCommandAddress -> ai_protocol);
#endif

   if ( -1L == theSocket ) {
      sprintf(szMessage,"\nERROR: Was not able to create a socket for sending email.\n"
                           "\nPress Retry to specify the properties or Cancel to exit.");
      long rc = MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);
      delete [] pszLowerHeader;
      delete [] pszHeader;
      fclose(fInput);
      DeleteFile(szEmailFileName);
      return rc == IDCANCEL;
   }  

#if 0
   memset(&socketDetails,0,sizeof(struct sockaddr_in));
    
   socketDetails.sin_family = AF_INET;
   socketDetails.sin_port = htons((unsigned short)smtpPort);
   socketDetails.sin_addr = *(struct in_addr *)pMailHost -> h_addr;

   if ( -1L == connect(theSocket, (sockaddr *)&socketDetails, sizeof(struct sockaddr_in)) ) {
#else
   if ( -1L == connect(theSocket,pCommandAddress -> ai_addr,(int)pCommandAddress -> ai_addrlen) ) {
#endif
      sprintf(szMessage,"\n\nERROR: Was not able to connect the socket for sending email, rc = %ld: %s.\n\n"
                           "\nPress Retry to specify the properties or Cancel to exit.",errno,strerror(errno));
      long rc = MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);
      delete [] pszLowerHeader;
      delete [] pszHeader;
      fclose(fInput);
      DeleteFile(szEmailFileName);
      return rc == IDCANCEL;
   }

   char szInput[1024];

   memset(szInput,0,sizeof(szInput));
   recv(theSocket,szInput,1024,0L);

#if 1
   char outputCommands[][32] = {"EHLO %s\r\n","MAIL From:%s\r\n","\0"};
   char responses[][16] = {"250","250","\0"};

   for ( long k = 0; k < 2; k++ ) { 
#else
   char outputCommands[][32] = {"EHLO %s\r\n","AUTH PLAIN\r\n","%s\r\n","\0"};
   char responses[][16] = {"250","334","235","\0"};
   for ( long k = 0; k < 3; k++ ) { 
#endif 
   
      if ( 0 == k ) 
         sprintf(szCommand,outputCommands[k],serverName);
#if 1        
      else if ( 1 == k )
         sprintf(szCommand,outputCommands[k],fromAddress);
#else
      else if ( 1 == k )
         sprintf(szCommand,outputCommands[k]);
      else 
         sprintf(szCommand,outputCommands[k],authenticationString);
#endif                    

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

         fclose(fInput);
   
         DeleteFile(szEmailFileName);

         return rc == IDCANCEL;
      }

   }

#if 1

   memset(szInput,0,sizeof(szInput));
    
#else

   memset(szInput,0,sizeof(szInput));
   read(theSocket,szInput,1024);
   
   sprintf(szCommand,"MAIL FROM:<%s>\r\n",configValues[CFG_ADMINEMAIL]);
   write(theSocket,szCommand,(DWORD)strlen(szCommand));
   memset(szInput,0,sizeof(szInput));
   read(theSocket,szInput,1024);

#if 0
   if ( strncmp(szInput,"250",3) )
       fprintf(fLog, "\nERROR: The SMTP Server sent an unexpected response to this input\n"
                                  "Sent to server:%sRecieved from server:%sExpected response:%s\n",szCommand,szInput,"250");
   else
       fprintf(fLog,"Sent to server:%sResponse:%s",szCommand,szInput);
#endif

#endif
   
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
           send(theSocket,szCommand,(DWORD)strlen(szCommand),0L);
           memset(szInput,0,sizeof(szInput));
           recv(theSocket,szInput,1024,0L);

           if ( szInput[0] ) {
               if ( strncmp(szInput,"250",3) ) {

                   //fprintf(fLog, "\nERROR: The SMTP Server sent an unexpected response to this command\n");

                  sprintf(szMessage,"\nThe SMTP Server sent an unexpected response to this input\n"
                                             "Sent to server:%sRecieved from server:%s\n"
                                             "\nPress Retry to change the settings or Cancel to exit.",
                                                szCommand,szInput);

                  long rc = MessageBox(NULL,szMessage,"Error!",MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_DEFBUTTON1);

                  closesocket(theSocket);

                  delete [] pszTokens;
                  delete [] pszLowerHeader;
                  delete [] pszHeader;

                  fclose(fInput);
   
                  DeleteFile(szEmailFileName);

                  return rc == IDCANCEL;

               }
               //fprintf(fLog,"Sent to server:%sResponse:%s",szCommand,szInput); 
           }

           p = strtok(NULL,",");
       }
       
       delete [] pszTokens;
       
   }
   
   sprintf(szCommand,"DATA\r\n");
   send(theSocket,szCommand,(DWORD)strlen(szCommand),0L);
   memset(szInput,0,sizeof(szInput));
   recv(theSocket,szInput,1024,0L);

#if 0
   if ( strncmp(szInput,"354",3) )
       fprintf(fLog, "\nERROR: The SMTP Server sent an unexpected response to this command\n");
   fprintf(fLog,"Sent to server:%sResponse:%s",szCommand,szInput);
#endif

   fseek(fInput,0,SEEK_END);
   long emailBytes = ftell(fInput);
   fseek(fInput,0,SEEK_SET);

   char *pszEmail = new char[emailBytes + 8];
   
   fread(pszEmail,emailBytes,1,fInput);

   fclose(fInput);

   sprintf(pszEmail + emailBytes,"\r\n.\r\n");
   send(theSocket,pszEmail,emailBytes + 5,0L);

   memset(szInput,0,sizeof(szInput));
   recv(theSocket,szInput,1024,0L);

   delete [] pszEmail;
   
   sprintf(szCommand,"QUIT\r\n");
   send(theSocket,szCommand,(DWORD)strlen(szCommand),0L);

   closesocket(theSocket);

   DeleteFile(szEmailFileName);
   
   delete [] pszLowerHeader;
   delete [] pszHeader;

   return 1;
}
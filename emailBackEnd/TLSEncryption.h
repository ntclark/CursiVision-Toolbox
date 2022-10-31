#pragma once

#define SECURITY_WIN32
#define IO_BUFFER_SIZE  0x10000
#define DLL_NAME TEXT("Secur32.dll")
#define NT4_DLL_NAME TEXT("Security.dll")

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <schannel.h>
#include <security.h>
#include <sspi.h>

    class TLSEncryption {
    public:

        TLSEncryption(SOCKET passedSocket,char *pszServerName) {

            SECURITY_STATUS Status;
            PCCERT_CONTEXT pRemoteCertContext = NULL;

            memset(szServerName,0,sizeof(szServerName));
            strcpy_s<128>(szServerName,pszServerName);

            socket = passedSocket;

            loadSecurityLibrary();

            createCredentials();

            fCredsInitialized = TRUE;

            performHandshake();

        }

        int send(char *buffer,int bufferLength);
        int recv(char *buffer);
        int exchange(char *pToServer,int length,char *pFromServer) {
            send(pToServer,length);
            return recv(pFromServer);
        }

        void Base64Encode(char *pszString,int stringLength);

    
    private:

        boolean loadSecurityLibrary();
        void unloadSecurityLibrary();

        SECURITY_STATUS performHandshake();
        SECURITY_STATUS createCredentials();
        SECURITY_STATUS clientHandshakeLoop(boolean doInitialRead);

        void getNewClientCredentials();

        SECURITY_STATUS readDecrypt(PBYTE pbIoBuffer,char *pszOutput);
        DWORD EncryptSend(BYTE *pbIoBuffer);

        SecPkgContext_StreamSizes Sizes{0};
        DWORD cbIoBufferLength{0};

        CtxtHandle hContext{NULL};
        CredHandle hClientCreds{NULL};
        HCERTSTORE hMyCertStore{NULL};
        SecBuffer extraData{0};

        BOOL fCredsInitialized{FALSE};
        BOOL fContextInitialized{FALSE};
    
        SOCKET socket;
        char szServerName[128];
    };


#include "TLSEncryption.h"

BOOL fVerbose = FALSE;

INT iPortNumber = 465;
char *pszUser = NULL;

DWORD dwProtocol = SP_PROT_TLS1;
ALG_ID  aiKeyExch = 0;

HMODULE g_hSecurity  = NULL;

SCHANNEL_CRED SchannelCred;
PSecurityFunctionTable g_pSSPI;

char szDebugString[1024];

    SECURITY_STATUS TLSEncryption::createCredentials() { 

    TimeStamp  tsExpiry;
    SECURITY_STATUS  status;

    ALG_ID rgbSupportedAlgs[16];

    hMyCertStore = CertOpenSystemStore(0, "MY");

    if( ! hMyCertStore)
    {
        sprintf_s<1024>(szDebugString, "**** Error 0x%x returned by CertOpenSystemStore\n", GetLastError() );
        OutputDebugStringA(szDebugString);
        return SEC_E_NO_CREDENTIALS;
    }

    ZeroMemory( &SchannelCred, sizeof(SchannelCred) );

    SchannelCred.dwVersion  = SCHANNEL_CRED_VERSION;
    SchannelCred.grbitEnabledProtocols = dwProtocol;
    SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS;

    status = g_pSSPI -> AcquireCredentialsHandleA( NULL,(char *)UNISP_NAME_A,SECPKG_CRED_OUTBOUND,NULL,&SchannelCred,NULL,NULL,&hClientCreds,&tsExpiry );

    if ( status != SEC_E_OK ) {
        sprintf_s<1024>(szDebugString,"**** Error 0x%x returned by AcquireCredentialsHandle\n", status);
        OutputDebugStringA(szDebugString);
    }

    return status;
    }


    SECURITY_STATUS TLSEncryption::performHandshake() {

    SecBufferDesc  bufferDescriptor;
    SecBuffer OutBuffers[1];

    DWORD dwSSPIOutFlags, cbData;
    TimeStamp tsExpiry;
    SECURITY_STATUS scRet;

    DWORD dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY | ISC_RET_EXTENDED_ERROR | ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM;

    bufferDescriptor.cBuffers = 1;
    bufferDescriptor.pBuffers = OutBuffers;
    bufferDescriptor.ulVersion = SECBUFFER_VERSION;

    OutBuffers[0].pvBuffer = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer = 0;

    scRet = g_pSSPI -> InitializeSecurityContextA(&hClientCreds,NULL,szServerName,dwSSPIFlags,0,SECURITY_NATIVE_DREP,NULL,0,&hContext,&bufferDescriptor,&dwSSPIOutFlags,&tsExpiry );

    if ( scRet != SEC_I_CONTINUE_NEEDED ) { 
        sprintf_s<1024>(szDebugString,"**** Error %d returned by InitializeSecurityContext (1)\n", scRet); 
        return scRet;
    }

    if ( OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL ) {

        cbData = ::send(socket, (char *)OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer, 0 );
        
        if( cbData == SOCKET_ERROR || cbData == 0 ) {
            sprintf_s<1024>(szDebugString,"**** Error %d sending data to server (1)\n", WSAGetLastError());
            OutputDebugStringA(szDebugString);
            g_pSSPI -> FreeContextBuffer(OutBuffers[0].pvBuffer);
            g_pSSPI -> DeleteSecurityContext(&hContext);
            return SEC_E_INTERNAL_ERROR;
        }

        g_pSSPI -> FreeContextBuffer(OutBuffers[0].pvBuffer);

    }

    scRet = clientHandshakeLoop(TRUE);

    g_pSSPI -> QueryContextAttributes(&hContext, SECPKG_ATTR_STREAM_SIZES, &Sizes );

    cbIoBufferLength = Sizes.cbHeader + Sizes.cbMaximumMessage + Sizes.cbTrailer;

    return scRet;
    }


    int TLSEncryption::send(char *pszOutput,int cbOutput) {

    BYTE *pbIoBuffer = (BYTE *)LocalAlloc(LMEM_FIXED, cbIoBufferLength);
    
    BYTE *pTarget = pbIoBuffer + Sizes.cbHeader;

    strncpy((char *)pTarget,pszOutput,cbOutput);

    pTarget[cbOutput] = 0x0;

    DWORD cbData = EncryptSend(pbIoBuffer);

    LocalFree(pbIoBuffer);

    return cbData;
    }

    DWORD TLSEncryption::EncryptSend(BYTE *pbIoBuffer) {

    SECURITY_STATUS scRet;
    SecBufferDesc buffersDescriptor;
    SecBuffer Buffers[4];
    DWORD cbMessage;
    PBYTE pbMessage;

    pbMessage = pbIoBuffer + Sizes.cbHeader; 

    cbMessage = (DWORD)strlen((char *)pbMessage);

    Buffers[0].pvBuffer = pbIoBuffer;
    Buffers[0].cbBuffer = Sizes.cbHeader;
    Buffers[0].BufferType  = SECBUFFER_STREAM_HEADER;

    Buffers[1].pvBuffer = pbMessage;
    Buffers[1].cbBuffer = cbMessage;
    Buffers[1].BufferType = SECBUFFER_DATA;
                                                                                            
    Buffers[2].pvBuffer = pbMessage + cbMessage;
    Buffers[2].cbBuffer = Sizes.cbTrailer;
    Buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;

    Buffers[3].pvBuffer = SECBUFFER_EMPTY;
    Buffers[3].cbBuffer = SECBUFFER_EMPTY;
    Buffers[3].BufferType = SECBUFFER_EMPTY;

    buffersDescriptor.ulVersion = SECBUFFER_VERSION;
    buffersDescriptor.cBuffers = 4;
    buffersDescriptor.pBuffers = Buffers;

    scRet = g_pSSPI -> EncryptMessage(&hContext, 0, &buffersDescriptor, 0);

    if( FAILED(scRet) ) { 
        sprintf_s<1024>(szDebugString,"**** Error 0x%x returned by EncryptMessage\n", scRet); 
        OutputDebugStringA(szDebugString);
        return scRet; 
    }

    return ::send(socket,(char *)pbIoBuffer,Buffers[0].cbBuffer + Buffers[1].cbBuffer + Buffers[2].cbBuffer,0);
    }


    int TLSEncryption::recv(char *pszBuffer) {
    BYTE *pbIoBuffer = (BYTE *)LocalAlloc(LMEM_FIXED, cbIoBufferLength);
    SECURITY_STATUS scRet = readDecrypt(pbIoBuffer,pszBuffer);
    LocalFree(pbIoBuffer);
    return strlen(pszBuffer);
    }


    SECURITY_STATUS TLSEncryption::readDecrypt(PBYTE pbIoBuffer, char *pszOutput) {
  
    SecBuffer ExtraBuffer = {0};
    SecBuffer *pDataBuffer = NULL;
    SecBuffer *pExtraBuffer = NULL;

    SecBufferDesc bufferDescriptor = {0};
    SecBuffer Buffers[4];

    DWORD cbIoBuffer = 0;
    SECURITY_STATUS scRet = 0;

    while ( TRUE ) {
                
        if ( 0 == cbIoBuffer || scRet == SEC_E_INCOMPLETE_MESSAGE ) {

            DWORD cbData = ::recv(socket, (char *)pbIoBuffer + cbIoBuffer, cbIoBufferLength - cbIoBuffer, 0);
            
            if ( cbData == SOCKET_ERROR ) {
                sprintf_s<1024>(szDebugString,"**** Error %d reading data from server\n", WSAGetLastError());
                OutputDebugStringA(szDebugString);
                scRet = SEC_E_INTERNAL_ERROR;
                break;
            }

            if ( 0 == cbData ) {
                if ( cbIoBuffer ) {
                    sprintf_s<1024>(szDebugString,"**** Server unexpectedly disconnected\n");
                    OutputDebugStringA(szDebugString);
                    return SEC_E_INTERNAL_ERROR;
                }
                break;
            }

            cbIoBuffer += cbData;

        }

        Buffers[0].pvBuffer = pbIoBuffer;
        Buffers[0].cbBuffer = cbIoBuffer;
        Buffers[0].BufferType = SECBUFFER_DATA;

        Buffers[1].BufferType = SECBUFFER_EMPTY;
        Buffers[2].BufferType = SECBUFFER_EMPTY;
        Buffers[3].BufferType = SECBUFFER_EMPTY;

        bufferDescriptor.ulVersion = SECBUFFER_VERSION;
        bufferDescriptor.cBuffers = 4;
        bufferDescriptor.pBuffers = Buffers;

        scRet = g_pSSPI -> DecryptMessage(&hContext, &bufferDescriptor, 0, NULL);

        if ( scRet == SEC_I_CONTEXT_EXPIRED ) 
            break;

        if ( scRet != SEC_E_OK && scRet != SEC_I_RENEGOTIATE && scRet != SEC_I_CONTEXT_EXPIRED ) {
            sprintf_s<1024>(szDebugString,"**** DecryptMessage ");
            OutputDebugStringA(szDebugString);
            return scRet; 
        }

        pDataBuffer  = NULL;

        pExtraBuffer = NULL;

        for ( int k = 1; k < 4; k++ )
        {
            if ( pDataBuffer == NULL && Buffers[k].BufferType == SECBUFFER_DATA  ) 
                pDataBuffer = &Buffers[k];

            if ( pExtraBuffer == NULL && Buffers[k].BufferType == SECBUFFER_EXTRA ) 
                pExtraBuffer = &Buffers[k];
        }

        if ( pDataBuffer )
        {
            DWORD length = pDataBuffer -> cbBuffer;
            if( length ) 
            {
                BYTE *pEnd = (BYTE *)pDataBuffer -> pvBuffer;
                if( pEnd[length - 2] == 13 && pEnd[length - 1] == 10 ) 
                    break;
            }
        }

        if ( pExtraBuffer )
        {
            MoveMemory(pbIoBuffer, pExtraBuffer->pvBuffer, pExtraBuffer->cbBuffer);
            cbIoBuffer = pExtraBuffer -> cbBuffer; // sprintf_s<1024>(szDebugString,"cbIoBuffer= %d  \n", cbIoBuffer);
        }
        else
          cbIoBuffer = 0;

        if ( scRet == SEC_I_RENEGOTIATE ) {
            sprintf_s<1024>(szDebugString,"Server requested renegotiate!\n");
            OutputDebugStringA(szDebugString);
            scRet = clientHandshakeLoop(FALSE);
            if ( scRet != SEC_E_OK )
                return scRet;

            if ( ExtraBuffer.pvBuffer ) { // Move any "extra" data to the input buffer.
                MoveMemory(pbIoBuffer, ExtraBuffer.pvBuffer, ExtraBuffer.cbBuffer);
                cbIoBuffer = ExtraBuffer.cbBuffer;
            }

        }

    } 

    if ( ! ( NULL == pDataBuffer ) ) {
        memcpy((BYTE *)pszOutput,(BYTE *)pDataBuffer -> pvBuffer,pDataBuffer -> cbBuffer );
        pszOutput[pDataBuffer -> cbBuffer] = '\0';
    } else {
        sprintf_s<1024>(szDebugString,"No data available from Read/Decryption");
        OutputDebugStringA(szDebugString);
    }

    return SEC_E_OK;
    }


    void TLSEncryption::Base64Encode(char *pszString,int stringLength) {
    DWORD cbString = 4096;
    char szResult[4096];
    CryptBinaryToStringA((BYTE *)pszString,stringLength,CRYPT_STRING_BASE64,szResult,&cbString);
    strcpy(pszString,szResult);
    return;
    }

    boolean TLSEncryption::loadSecurityLibrary()  {
    INIT_SECURITY_INTERFACE pInitSecurityInterface;
    g_hSecurity = LoadLibrary("Secur32.dll");
    pInitSecurityInterface = (INIT_SECURITY_INTERFACE)GetProcAddress( g_hSecurity, "InitSecurityInterfaceA" );
    g_pSSPI = pInitSecurityInterface();
    return TRUE;
    }


    void TLSEncryption::unloadSecurityLibrary() {
    FreeLibrary(g_hSecurity);
    g_hSecurity = NULL;
    }


#if 0
    static DWORD VerifyServerCertificate( PCCERT_CONTEXT pServerCert, PSTR pszServerName, DWORD dwCertFlags ) {
    HTTPSPolicyCallbackData  polHttps;
    CERT_CHAIN_POLICY_PARA   PolicyPara;
    CERT_CHAIN_POLICY_STATUS PolicyStatus;
    CERT_CHAIN_PARA          ChainPara;
    PCCERT_CHAIN_CONTEXT     pChainContext = NULL;
    
    
    DWORD cchServerName, Status;

    LPSTR rgszUsages[] = { (char *)szOID_PKIX_KP_SERVER_AUTH,(char *)szOID_SERVER_GATED_CRYPTO,(char *)szOID_SGC_NETSCAPE };

    DWORD cUsages = sizeof(rgszUsages) / sizeof(LPSTR);

    PWSTR  pwszServerName = NULL;


    if ( pServerCert == NULL ){ 
        Status = SEC_E_WRONG_PRINCIPAL; 
        goto cleanup;
    }

    // Convert server name to unicode.
    if(pszServerName == NULL || strlen(pszServerName) == 0)
    { Status = SEC_E_WRONG_PRINCIPAL; goto cleanup; }

    cchServerName = MultiByteToWideChar(CP_ACP, 0, pszServerName, -1, NULL, 0);

    pwszServerName = (WCHAR *)LocalAlloc(LMEM_FIXED, cchServerName * sizeof(WCHAR));

    if ( pwszServerName == NULL ) { 
        Status = SEC_E_INSUFFICIENT_MEMORY; 
        goto cleanup; 
    }

    cchServerName = MultiByteToWideChar(CP_ACP, 0, pszServerName, -1, pwszServerName, cchServerName);

    if ( cchServerName == 0 ) { 
        Status = SEC_E_WRONG_PRINCIPAL; 
        goto cleanup; 
    }


    // Build certificate chain.
    ZeroMemory(&ChainPara, sizeof(ChainPara));

    ChainPara.cbSize = sizeof(ChainPara);
    ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_OR;
    ChainPara.RequestedUsage.Usage.cUsageIdentifier = cUsages;
    ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = rgszUsages;

    if ( ! CertGetCertificateChain( NULL,pServerCert,NULL,pServerCert->hCertStore,&ChainPara,0,NULL,&pChainContext ) ) {
        Status = GetLastError();
        sprintf_s<1024>(szDebugString,"Error 0x%x returned by CertGetCertificateChain!\n", Status);
        goto cleanup;
    }


    // Validate certificate chain.
    ZeroMemory(&polHttps, sizeof(HTTPSPolicyCallbackData));

    polHttps.cbStruct           = sizeof(HTTPSPolicyCallbackData);
    polHttps.dwAuthType         = AUTHTYPE_SERVER;
    polHttps.fdwChecks          = dwCertFlags;
    polHttps.pwszServerName     = pwszServerName;

    memset(&PolicyPara, 0, sizeof(PolicyPara));
    PolicyPara.cbSize            = sizeof(PolicyPara);
    PolicyPara.pvExtraPolicyPara = &polHttps;

    memset(&PolicyStatus, 0, sizeof(PolicyStatus));
    PolicyStatus.cbSize = sizeof(PolicyStatus);

    if ( ! CertVerifyCertificateChainPolicy( CERT_CHAIN_POLICY_SSL,pChainContext,&PolicyPara,&PolicyStatus ) ) {
        Status = GetLastError();
        sprintf_s<1024>(szDebugString,"Error 0x%x returned by CertVerifyCertificateChainPolicy!\n", Status);
        goto cleanup;
    }

    if ( PolicyStatus.dwError ){
        Status = PolicyStatus.dwError;
        DisplayWinVerifyTrustError(Status);
        goto cleanup;
    }

    Status = SEC_E_OK;

cleanup:
    if(pChainContext)  CertFreeCertificateChain(pChainContext);
    if(pwszServerName) LocalFree(pwszServerName);

    return Status;
}
#endif

/*****************************************************************************/
static LONG DisconnectFromServer( SOCKET Socket, PCredHandle phCreds, CtxtHandle * phContext ) {

    PBYTE pbMessage;
    DWORD dwType, dwSSPIFlags, dwSSPIOutFlags, cbMessage, cbData, Status;

    SecBufferDesc OutBuffer;
    SecBuffer OutBuffers[1];
    TimeStamp tsExpiry;

    dwType = SCHANNEL_SHUTDOWN;

    OutBuffers[0].pvBuffer   = &dwType;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = sizeof(dwType);

    OutBuffer.cBuffers  = 1;
    OutBuffer.pBuffers  = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    Status = g_pSSPI -> ApplyControlToken(phContext, &OutBuffer);

    if ( FAILED(Status) ) { 
        sprintf_s<1024>(szDebugString,"**** Error 0x%x returned by ApplyControlToken\n", Status); 
        goto cleanup; 
    }

    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT |ISC_REQ_CONFIDENTIALITY |ISC_RET_EXTENDED_ERROR | ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM;

    OutBuffers[0].pvBuffer   = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = 0;

    OutBuffer.cBuffers  = 1;
    OutBuffer.pBuffers  = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    Status = g_pSSPI -> InitializeSecurityContextA( phCreds,phContext,NULL,dwSSPIFlags,0,
                  SECURITY_NATIVE_DREP,NULL,0,phContext,&OutBuffer,&dwSSPIOutFlags,&tsExpiry );

    if ( FAILED(Status) ) { 
        sprintf_s<1024>(szDebugString,"**** Error 0x%x returned by InitializeSecurityContext\n", Status); 
        goto cleanup; 
    }

    pbMessage = (BYTE *)OutBuffers[0].pvBuffer;
    cbMessage = OutBuffers[0].cbBuffer;

    // Send the close notify message to the server.
    if ( pbMessage != NULL && cbMessage != 0 )
    {
        cbData = send(Socket, (char *)pbMessage, cbMessage, 0);

        if ( cbData == SOCKET_ERROR || cbData == 0 )
        {
            Status = WSAGetLastError();
            sprintf_s<1024>(szDebugString,"**** Error %d sending close notify\n", Status);
            OutputDebugStringA(szDebugString);
            goto cleanup;
        }

        g_pSSPI -> FreeContextBuffer(pbMessage);
    }
    

cleanup:

    g_pSSPI -> DeleteSecurityContext(phContext);

    closesocket(Socket); 

    return Status;
}



    void TLSEncryption::getNewClientCredentials() {

    CredHandle hCreds;
    SecPkgContext_IssuerListInfoEx IssuerListInfo;
    PCCERT_CHAIN_CONTEXT pChainContext;
    CERT_CHAIN_FIND_BY_ISSUER_PARA FindByIssuerPara;
    PCCERT_CONTEXT pCertContext;
    TimeStamp tsExpiry;
    SECURITY_STATUS status;

    // Read list of trusted issuers from schannel.
    status = g_pSSPI -> QueryContextAttributes(&hContext,SECPKG_ATTR_ISSUER_LIST_EX, (PVOID)&IssuerListInfo );

    if ( status != SEC_E_OK ) {
        sprintf_s<1024>(szDebugString,"Error 0x%x querying issuer list info\n", status);
        OutputDebugStringA(szDebugString);
        return; 
    }

    // Enumerate the client certificates.
    ZeroMemory(&FindByIssuerPara, sizeof(FindByIssuerPara));

    FindByIssuerPara.cbSize = sizeof(FindByIssuerPara);
    FindByIssuerPara.pszUsageIdentifier = szOID_PKIX_KP_CLIENT_AUTH;
    FindByIssuerPara.dwKeySpec = 0;
    FindByIssuerPara.cIssuer   = IssuerListInfo.cIssuers;
    FindByIssuerPara.rgIssuer  = IssuerListInfo.aIssuers;

    pChainContext = NULL;

    while ( TRUE ) {
        
        pChainContext = CertFindChainInStore( hMyCertStore,X509_ASN_ENCODING,0,CERT_CHAIN_FIND_BY_ISSUER,&FindByIssuerPara,pChainContext );
                
        if ( pChainContext == NULL ) { 
            sprintf_s<1024>(szDebugString,"Error 0x%x finding cert chain\n", GetLastError()); 
            OutputDebugStringA(szDebugString);
            break; 
        }

        // Get pointer to leaf certificate context.
        pCertContext = pChainContext -> rgpChain[0] -> rgpElement[0] -> pCertContext;

        // Create schannel credential.

        SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;
        SchannelCred.cCreds = 1;
        SchannelCred.paCred = &pCertContext;

        status = g_pSSPI -> AcquireCredentialsHandleA(NULL,(char *)UNISP_NAME_A,SECPKG_CRED_OUTBOUND,NULL,&SchannelCred,NULL,NULL,&hCreds,&tsExpiry );

        if ( status != SEC_E_OK ) {
            sprintf_s<1024>(szDebugString,"**** Error 0x%x returned by AcquireCredentialsHandle\n", status); 
            OutputDebugStringA(szDebugString);
            continue;
        }

        sprintf_s<1024>(szDebugString,"\nnew schannel credential created\n");

        g_pSSPI -> FreeCredentialsHandle(&hClientCreds);

        hClientCreds = hCreds;
    }

    return;
    }


    SECURITY_STATUS TLSEncryption::clientHandshakeLoop(boolean doInitialRead) {

    SecBufferDesc   OutBuffer, InBuffer;
    SecBuffer       InBuffers[2], OutBuffers[1];
    DWORD           dwSSPIFlags, dwSSPIOutFlags, cbData, cbIoBuffer;
    TimeStamp       tsExpiry;
    SECURITY_STATUS scRet;
    PUCHAR          IoBuffer;
    BOOL            fDoRead;

    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY | ISC_RET_EXTENDED_ERROR | ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM;

    IoBuffer = (UCHAR *)LocalAlloc(LMEM_FIXED, IO_BUFFER_SIZE);

    if ( IoBuffer == NULL ) { 
        sprintf_s<1024>(szDebugString,"**** Out of memory (1)\n"); 
        return SEC_E_INTERNAL_ERROR; 
    }
    
    cbIoBuffer = 0;

    fDoRead = doInitialRead;

    // Loop until the handshake is finished or an error occurs.

    scRet = SEC_I_CONTINUE_NEEDED;

    while ( scRet == SEC_I_CONTINUE_NEEDED || scRet == SEC_E_INCOMPLETE_MESSAGE || scRet == SEC_I_INCOMPLETE_CREDENTIALS ) {

        if ( 0 == cbIoBuffer || scRet == SEC_E_INCOMPLETE_MESSAGE )
        {
            if ( fDoRead )
            {
                cbData = ::recv(socket, (char *)IoBuffer + cbIoBuffer, IO_BUFFER_SIZE - cbIoBuffer, 0 );

                if ( cbData == SOCKET_ERROR )
                {
                    sprintf_s<1024>(szDebugString,"**** Error %d reading data from server\n", WSAGetLastError());
                    OutputDebugStringA(szDebugString);
                    scRet = SEC_E_INTERNAL_ERROR;
                    break;
                }

                if ( cbData == 0 )
                {
                    sprintf_s<1024>(szDebugString,"**** Server unexpectedly disconnected\n");
                    OutputDebugStringA(szDebugString);
                    scRet = SEC_E_INTERNAL_ERROR;
                    break;
                }

                cbIoBuffer += cbData;
            }
            else
              fDoRead = TRUE;
        }

        // Set up the input buffers. Buffer 0 is used to pass in data
        // received from the server. Schannel will consume some or all
        // of this. Leftover data (if any) will be placed in buffer 1 and
        // given a buffer type of SECBUFFER_EXTRA.
        InBuffers[0].pvBuffer   = IoBuffer;
        InBuffers[0].cbBuffer   = cbIoBuffer;
        InBuffers[0].BufferType = SECBUFFER_TOKEN;

        InBuffers[1].pvBuffer   = NULL;
        InBuffers[1].cbBuffer   = 0;
        InBuffers[1].BufferType = SECBUFFER_EMPTY;

        InBuffer.cBuffers       = 2;
        InBuffer.pBuffers       = InBuffers;
        InBuffer.ulVersion      = SECBUFFER_VERSION;

        // Set up the output buffers. These are initialized to NULL
        // so as to make it less likely we'll attempt to free random
        // garbage later.
        OutBuffers[0].pvBuffer  = NULL;
        OutBuffers[0].BufferType= SECBUFFER_TOKEN;
        OutBuffers[0].cbBuffer  = 0;

        OutBuffer.cBuffers      = 1;
        OutBuffer.pBuffers      = OutBuffers;
        OutBuffer.ulVersion     = SECBUFFER_VERSION;


        scRet = g_pSSPI -> InitializeSecurityContextA(&hClientCreds,&hContext,NULL,dwSSPIFlags,0,SECURITY_NATIVE_DREP,&InBuffer,0,NULL,&OutBuffer,&dwSSPIOutFlags,&tsExpiry );

        // If InitializeSecurityContext was successful (or if the error was
        // one of the special extended ones), send the contends of the output
        // buffer to the server.

        if ( scRet == SEC_E_OK || scRet == SEC_I_CONTINUE_NEEDED || FAILED(scRet) && ( dwSSPIOutFlags & ISC_RET_EXTENDED_ERROR) ) {

            if ( OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL ) {

                cbData = ::send(socket, (char *)OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer, 0 );

                if ( cbData == SOCKET_ERROR || cbData == 0 ) {
                    sprintf_s<1024>(szDebugString, "**** Error %d sending data to server (2)\n",  WSAGetLastError() );
                    OutputDebugStringA(szDebugString);
                    g_pSSPI -> FreeContextBuffer(OutBuffers[0].pvBuffer);
                    g_pSSPI -> DeleteSecurityContext(&hContext);
                    return SEC_E_INTERNAL_ERROR;
                }

                g_pSSPI->FreeContextBuffer(OutBuffers[0].pvBuffer);

                OutBuffers[0].pvBuffer = NULL;
            }
        }



        // If InitializeSecurityContext returned SEC_E_INCOMPLETE_MESSAGE,
        // then we need to read more data from the server and try again.

        if ( scRet == SEC_E_INCOMPLETE_MESSAGE )
            continue;


        // If InitializeSecurityContext returned SEC_E_OK, then the
        // handshake completed successfully.
        if( scRet == SEC_E_OK )
        {
            // If the "extra" buffer contains data, this is encrypted application
            // protocol layer stuff. It needs to be saved. The application layer
            // will later decrypt it with DecryptMessage.

            sprintf_s<1024>(szDebugString,"Handshake was successful\n");

            if(InBuffers[1].BufferType == SECBUFFER_EXTRA)
            {
                extraData.pvBuffer = LocalAlloc( LMEM_FIXED, InBuffers[1].cbBuffer );
                if ( extraData.pvBuffer == NULL) { sprintf_s<1024>(szDebugString,"**** Out of memory (2)\n"); return SEC_E_INTERNAL_ERROR; }

                MoveMemory( extraData.pvBuffer,IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer),InBuffers[1].cbBuffer );

                extraData.cbBuffer   = InBuffers[1].cbBuffer;

                extraData.BufferType = SECBUFFER_TOKEN;

                sprintf_s<1024>(szDebugString, "%d bytes of app data was bundled with handshake data\n", extraData.cbBuffer );
            }
            else
            {
                extraData.pvBuffer   = NULL;
                extraData.cbBuffer   = 0;
                extraData.BufferType = SECBUFFER_EMPTY;
            }

            break;

        }

        // Check for fatal error.
        if(FAILED(scRet)) { sprintf_s<1024>(szDebugString,"**** Error 0x%x returned by InitializeSecurityContext (2)\n", scRet); break; }

        // If InitializeSecurityContext returned SEC_I_INCOMPLETE_CREDENTIALS,
        // then the server just requested client authentication.
        if(scRet == SEC_I_INCOMPLETE_CREDENTIALS)
        {
            // Busted. The server has requested client authentication and
            // the credential we supplied didn't contain a client certificate.
            // This function will read the list of trusted certificate
            // authorities ("issuers") that was received from the server
            // and attempt to find a suitable client certificate that
            // was issued by one of these. If this function is successful,
            // then we will connect using the new certificate. Otherwise,
            // we will attempt to connect anonymously (using our current credentials).

            getNewClientCredentials();

            fDoRead = FALSE;
            scRet = SEC_I_CONTINUE_NEEDED;
            continue;
        }

        if ( InBuffers[1].BufferType == SECBUFFER_EXTRA )
        {
            MoveMemory( IoBuffer, IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer), InBuffers[1].cbBuffer );
            cbIoBuffer = InBuffers[1].cbBuffer;
        }
        else
          cbIoBuffer = 0;
    }

    if ( FAILED(scRet) )
        g_pSSPI -> DeleteSecurityContext(&hContext);

    LocalFree(IoBuffer);

    return scRet;
}


#if 0
/*****************************************************************************/
static SECURITY_STATUS SMTPsession( SOCKET Socket,PCredHandle phCreds,CtxtHandle *phContext)  {
        
    SecPkgContext_StreamSizes Sizes;
    SECURITY_STATUS  scRet;
    PBYTE pbIoBuffer;
    DWORD cbIoBufferLength, cbData;

    scRet = g_pSSPI -> QueryContextAttributes( phContext, SECPKG_ATTR_STREAM_SIZES, &Sizes );

    cbIoBufferLength = Sizes.cbHeader + Sizes.cbMaximumMessage + Sizes.cbTrailer;

    pbIoBuffer = (BYTE *)LocalAlloc(LMEM_FIXED, cbIoBufferLength);

    //scRet = ReadDecrypt( Socket, phCreds, phContext, pbIoBuffer, cbIoBufferLength );
    //if ( scRet != SEC_E_OK ) 
    //    return scRet;

    sprintf((char *)pbIoBuffer + Sizes.cbHeader, "%s",  "EHLO \r\n" );
    cbData = EncryptSend(Socket, phContext, pbIoBuffer, Sizes );

    if ( cbData == SOCKET_ERROR || cbData == 0 ) { 
        sprintf_s<1024>(szDebugString,"**** Error %d sending data to server (3)\n",  WSAGetLastError()); 
        return SEC_E_INTERNAL_ERROR; 
    }

    // Receive a Response
    scRet = ReadDecrypt( Socket, phCreds, phContext, pbIoBuffer, cbIoBufferLength );

    if ( scRet != SEC_E_OK ) 
        return scRet;

    // Build the request - must be < maximum message size

    sprintf((char *)pbIoBuffer+Sizes.cbHeader, "%s",  "QUIT \r\n" );

    // Send a request.

    cbData = EncryptSend( Socket, phContext, pbIoBuffer, Sizes );
    
    if ( cbData == SOCKET_ERROR || cbData == 0 ) { 
        sprintf_s<1024>(szDebugString,"**** Error %d sending data to server (3)\n",  WSAGetLastError()); 
        return SEC_E_INTERNAL_ERROR; 
    }  

    scRet = ReadDecrypt( Socket, phCreds, phContext, pbIoBuffer, cbIoBufferLength );

    if ( scRet != SEC_E_OK ) 
        return scRet;

    return SEC_E_OK;
}
#endif

#if 0
    void tlsHandshake(char *pszServerName,SOCKET socket)  {

    CredHandle hClientCreds;
    CtxtHandle hContext;
    BOOL fCredsInitialized   = FALSE;
    BOOL fContextInitialized = FALSE;

    SecBuffer  ExtraData;
    SECURITY_STATUS Status;

    PCCERT_CONTEXT pRemoteCertContext = NULL;

    LoadSecurityLibrary();


    if ( CreateCredentials( &hClientCreds) ) { 
        sprintf_s<1024>(szDebugString,"Error creating credentials\n"); 
        goto cleanup; 
    }

    fCredsInitialized = TRUE;

    if ( PerformClientHandshake( socket, &hClientCreds, pszServerName, &hContext, &ExtraData ) ) { 
        sprintf_s<1024>(szDebugString,"Error performing handshake\n"); 
        goto cleanup; 
    }

    fContextInitialized = TRUE;

    Status = g_pSSPI -> QueryContextAttributes(&hContext, SECPKG_ATTR_REMOTE_CERT_CONTEXT, (PVOID)&pRemoteCertContext );

    if ( Status != SEC_E_OK ) { 
        sprintf_s<1024>(szDebugString,"Error 0x%x querying remote certificate\n", Status); 
        goto cleanup;
    }

    CertFreeCertificateContext(pRemoteCertContext);

    pRemoteCertContext = NULL;


#if 0
    DisplayConnectionInfo(&hContext); //
#endif

    if ( SMTPsession( socket, &hClientCreds, &hContext ) ) {
        sprintf_s<1024>(szDebugString,"Error SMTP Session \n"); 
        OutputDebugStringA(szDebugString);
        goto cleanup;
    }

#if 0
    // Send a close_notify alert to the server and close down the connection.
    if(DisconnectFromServer(socket, &hClientCreds, &hContext))
    { sprintf_s<1024>(szDebugString,"Error disconnecting from server\n"); goto cleanup; }
    fContextInitialized = FALSE;
    //socket = INVALID_SOCKET; //
#endif

cleanup:

    return;

    // Free the server certificate context.
    if(pRemoteCertContext)
    {
        CertFreeCertificateContext(pRemoteCertContext);
        pRemoteCertContext = NULL;
    }

    // Free SSPI context handle.
    if(fContextInitialized)
    {
        g_pSSPI->DeleteSecurityContext(&hContext);
        fContextInitialized = FALSE;
    }

    // Free SSPI credentials handle.
    if(fCredsInitialized)
    {
        g_pSSPI->FreeCredentialsHandle(&hClientCreds);
        fCredsInitialized = FALSE;
    }

    // Close socket.
    if ( socket != INVALID_SOCKET) closesocket(socket);

    // Shutdown WinSock subsystem.
    WSACleanup();

    // Close "MY" certificate store.
    if(hMyCertStore) CertCloseStore(hMyCertStore, 0);

    UnloadSecurityLibrary();

}

#endif

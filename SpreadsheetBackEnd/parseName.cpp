
#include "SpreadsheetBackEnd.h"


long SpreadsheetBackEnd::parseName(char *pszPDFFile,long index,char *pszName) {

   HRESULT rc = CoCreateInstance(CLSID_PdfEnabler,NULL,CLSCTX_ALL,IID_IPdfEnabler,reinterpret_cast<void **>(&pIPdfEnabler));

   pIPdfEnabler -> Document(&pIPdfDocument);

   BSTR bstrPDFFile = SysAllocStringLen(NULL,MAX_PATH);

   MultiByteToWideChar(CP_ACP,0,pszPDFFile,-1,bstrPDFFile,MAX_PATH);

   pIPdfDocument -> Open(bstrPDFFile,NULL,NULL);

   BSTR bstrInternal = _wtempnam(NULL,L"cvtmp");

   pIPdfDocument -> WriteUncompressed(bstrInternal);

   pIPdfDocument -> Release();

   pIPdfEnabler -> Release();

   SysFreeString(bstrPDFFile);

   char szTempFile[MAX_PATH];

   WideCharToMultiByte(CP_ACP,0,bstrInternal,-1,szTempFile,MAX_PATH,0,0);

   FILE *fInput = fopen(szTempFile,"rb");

   fseek(fInput,0,SEEK_END);

   long sizeBytes = ftell(fInput);

   BYTE *pContents = new BYTE[sizeBytes + 16];

   memset(pContents + sizeBytes,0,16);

   fseek(fInput,0,SEEK_SET);

   fread((void *)pContents,sizeBytes,1,fInput);

   fclose(fInput);

   char *pStart = (char *)pContents;
   char *pEnd = pStart;
   char *p = NULL;
   char *pResult = new char[32768];
   long countFound = 0;
   bool takeNext = false;

   pszName[0] = '\0';

   while ( p = strstr(pStart,"TJ") ) {
      
      memset(pResult,0,32768 * sizeof(char));

      countFound = 0;

      char *pEnd = p;

      p--;
      while ( p > pStart && *p != ']' )
         p--;

      if ( p == pStart ) {
         pStart = pEnd + 2;
         continue;
      }

      char *pEndBracket = p;

      long n = 0;
      while ( p > pStart && ( '[' != *p || n > 0 ) ) {
         if ( ')' == *p ) 
            n++;
         if ( '(' == *p )
            n--;
         p--;
      }

      if ( p == pStart ) {
         pStart = pEnd + 2;
         continue;
      }

      while ( p < pEnd ) {

         p++;

         while ( p < pEnd && ( ( '0' <= *p && *p <= '9' ) || '-' == *p || '.' == *p || ' ' == *p || 0x0D == *p || 0x0A == *p || 0x09 == *p ) )
            p++;

         if ( '(' == *p )
            p++;
         else
            printf("\nhello world");

         if ( p != pEndBracket )
            pResult[countFound++] = *p;

         p++;

         while ( p < pEnd && ')' != *p ) {
            if ( p != pEndBracket )
               pResult[countFound++] = *p;
            p++;
         }

         if ( p == pEnd ) {
            pStart = pEnd + 2;
            continue;
         }

      }

      if ( 0 == countFound ) {
         pStart = pEnd + 2;
         continue;
      }

      if ( takeNext ) {
         char *pLast = pResult + countFound;
         p = pResult;
         while ( p < pLast && ( ' ' == *p || 0x0D == *p || 0x0A == *p || 0x09 == *p ) )
            p++;
         strcpy(pszName,p);
         break;
      }

      if ( ! ( p = strstr(pResult,szNamePrefix[index]) ) ) 
         continue;

      p += strlen(szNamePrefix[index]);

      char *pLast = pResult + countFound;

      while ( p < pLast && ( ' ' == *p || 0x0D == *p || 0x0A == *p || 0x09 == *p ) )
         p++;

      if ( p == pLast ) {
         takeNext = true;
         pStart = pEnd + 2;
         continue;
      }

      strcpy(pszName,p);

      break;

   }

   delete [] pContents;
   delete [] pResult;

   DeleteFileW(bstrInternal);

   if ( ! pszName[0] )
      return 0;

   return 1;
   }
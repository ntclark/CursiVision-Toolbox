
#pragma once

//#import <C:\\Program Files\\Common Files\\System\\ado\\msado15.dll>  rename( "EOF", "AdoNSEOF" )
#import <C:\\Program Files\\Microsoft Office 15\\root\\vfs\\ProgramFilesCommonX86\\Microsoft Shared\\OFFICE15\\ACEDAO.dll> rename( "EOF", "AdoNSEOF" )

   class database {
   public:

      database();
      ~database();

   long connect(char *pszDirectory,char *pszName);
   long disconnect();

   long insert(BSTR pdfDocumentName,BSTR settingsFileName);

   private:

   BSTR bstrConnectionString;

   DAO::ConnectionPtr pConnection;
   DAO::_DBEngine *pEngine;
   DAO::DatabasePtr pDatabasePtr;

   };


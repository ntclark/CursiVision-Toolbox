
extern "C" long sendMail(char *serverName,long smtpPort,char *userName,char *password,
                           char *pszAttachment,char *pszOriginalName,
                           char *pszFrom,char *pszTo,char *pszCC,char *pszBcc,char *pszSubject,char *pszBody,boolean useTLS);


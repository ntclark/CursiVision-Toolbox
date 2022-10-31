
#include "VideoBackEnd.h"


   HRESULT VideoBackEnd::VisioLoggerVideoBackEnd::_IVisioLoggerAction::ActionPerformed(OLE_HANDLE hParent,BSTR actionFieldName,SAFEARRAY *fieldNames,SAFEARRAY *imageFieldNames,SAFEARRAY *documentFieldNames,SAFEARRAY *fieldValues,SAFEARRAY *imageHandles,SAFEARRAY *documentValues,SAFEARRAY *valuesChanged,SAFEARRAY *imagesChanged,SAFEARRAY *documentsChanged) {

   pParent -> isMainMenuAction = false;
   if ( NULL == actionFieldName || 0 == wcslen(actionFieldName) )
      pParent -> isMainMenuAction = true;

   if ( pParent -> szActionField[0] && ! ( NULL == actionFieldName ) && 0 < wcslen(actionFieldName) ) {
      char szTemp[64];
      WideCharToMultiByte(CP_ACP,0,actionFieldName,-1,szTemp,64,0,0);
      if ( strcmp(szTemp,pParent -> szActionField) )
         return S_OK;
   } else if ( ! ( NULL == actionFieldName ) && 0 < wcslen(actionFieldName) )
      return S_OK;

   return pParent -> Handle(hParent,fieldNames,imageFieldNames,documentFieldNames,fieldValues,imageHandles,documentValues,valuesChanged,imagesChanged,documentsChanged,NULL,NULL);
   }


   HRESULT VideoBackEnd::VisioLoggerVideoBackEnd::_IVisioLoggerAction::get_ActionFieldName(BSTR *pResult) {
   long n = strlen(pParent -> szActionField);
   *pResult = SysAllocStringLen(NULL,n);
   MultiByteToWideChar(CP_ACP,0,pParent -> szActionField,-1,*pResult,n);
   return S_OK;
   }


#include "VideoBackEnd.h"

   // IUnknown

   long __stdcall VideoBackEnd::VisioLoggerVideoBackEnd::QueryInterface(REFIID riid,void **ppv) {

   *ppv = NULL;

   if ( riid == IID_IUnknown ) 
      *ppv = static_cast<IUnknown *>(this);

   else if ( riid == IID_IVisioLoggerNewRow )
      *ppv = static_cast<IVisioLoggerNewRow *>(this);

   else if ( riid == IID_IVisioLoggerAction ) 
      *ppv = static_cast<IVisioLoggerAction *>(pIVisioLoggerAction);

   else if ( riid == IID_IVisioLoggerPreSignature ) 
      *ppv = static_cast<IVisioLoggerPreSignature *>(pIVisioLoggerPreSignature);

   if ( riid == IID_IGPropertyPageClient )
      return pIGPropertyPageClient -> QueryInterface(riid,ppv);

   else

   if ( *ppv ) {
      AddRef();
      return S_OK;
   }

   return pParent -> QueryInterface(riid,ppv);
   }

   unsigned long __stdcall VideoBackEnd::VisioLoggerVideoBackEnd::AddRef() {
   return pParent -> AddRef();
   }

   unsigned long __stdcall VideoBackEnd::VisioLoggerVideoBackEnd::Release() { 
   return pParent -> Release();
   }
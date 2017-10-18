// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EnhancedReplicator.h"

   // IUnknown

   long __stdcall theReplicator::QueryInterface(REFIID riid,void **ppv) {

   *ppv = NULL;

   if ( riid == IID_IUnknown )
      *ppv = static_cast<IUnknown *>(this);

   if ( riid == IID_ICursiVisionBackEnd )
      *ppv = static_cast<ICursiVisionBackEnd *>(this);

   if ( *ppv ) {
      AddRef();
      return S_OK;
   }

   if ( riid == IID_IGPropertyPageClient )
      return pIGPropertyPageClient -> QueryInterface(riid,ppv);
   else

   if ( IID_ISpecifyPropertyPages == riid )
      return pIGProperties -> QueryInterface(riid,ppv);
   else

   if ( IID_IPersistStreamInit == riid )
      return pIGProperties -> QueryInterface(riid,ppv);
   else

      return E_NOINTERFACE;

   AddRef();

   return S_OK;
   }

   unsigned long __stdcall theReplicator::AddRef() {
   refCount++;
   return refCount;
   }

   unsigned long __stdcall theReplicator::Release() { 
   refCount--;
   if ( ! refCount ) {
      delete this;
      return 0;
   }
   return refCount;
   }
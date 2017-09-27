
#include "DocumentStorage.h"


   long __stdcall database::ConnectionEvents::QueryInterface(REFIID riid,void **ppv) {
   *ppv = NULL; 
 
   if (riid == __uuidof(IUnknown) || riid == __uuidof(ADODB::ConnectionEventsVt)) 
      *ppv = this;
   else
      return E_NOINTERFACE;
 
   static_cast<IUnknown*>(*ppv) -> AddRef();
  
   return S_OK; 
   }
 
   unsigned long __stdcall database::ConnectionEvents::AddRef() {
   return 1;
   }
 
   unsigned long __stdcall database::ConnectionEvents::Release() {
   return 1;
   }

   long __stdcall database::RecordsetEvents::QueryInterface(REFIID riid,void **ppv) {
   *ppv = NULL; 
 
   if (riid == __uuidof(IUnknown) || riid == __uuidof(ADODB::RecordsetEventsVt)) 
      *ppv = this;
   else
      return E_NOINTERFACE;
 
   static_cast<IUnknown*>(*ppv) -> AddRef();
  
   return S_OK; 
   }
 
   unsigned long __stdcall database::RecordsetEvents::AddRef() {
   return 1;
   }
 
   unsigned long __stdcall database::RecordsetEvents::Release() {
   return 1;
   }


   STDMETHODIMP database::ConnectionEvents::raw_InfoMessage(struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Connection *pConnection)  {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::ConnectionEvents::raw_BeginTransComplete(LONG TransactionLevel,struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Connection *pConnection) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::ConnectionEvents::raw_CommitTransComplete(struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Connection *pConnection) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::ConnectionEvents::raw_RollbackTransComplete(struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Connection *pConnection) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::ConnectionEvents::raw_WillExecute(BSTR *Source,ADODB::CursorTypeEnum *CursorType,ADODB::LockTypeEnum *LockType,long *Options,ADODB::EventStatusEnum *adStatus,struct ADODB::_Command *pCommand,struct ADODB::_Recordset *pRecordset,struct ADODB::_Connection *pConnection) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::ConnectionEvents::raw_ExecuteComplete(LONG RecordsAffected,struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Command *pCommand,struct ADODB::_Recordset *pRecordset,struct ADODB::_Connection *pConnection) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::ConnectionEvents::raw_WillConnect(BSTR *ConnectionString,BSTR *UserID,BSTR *Password,long *Options,ADODB::EventStatusEnum *adStatus,struct ADODB::_Connection *pConnection) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::ConnectionEvents::raw_ConnectComplete(struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Connection *pConnection) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::ConnectionEvents::raw_Disconnect(ADODB::EventStatusEnum *adStatus,struct ADODB::_Connection *pConnection) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };


   STDMETHODIMP database::RecordsetEvents::raw_WillChangeField(LONG cFields,VARIANT Fields,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::RecordsetEvents::raw_FieldChangeComplete(LONG cFields,VARIANT Fields,struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::RecordsetEvents::raw_WillChangeRecord(ADODB::EventReasonEnum adReason,LONG cRecords,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::RecordsetEvents::raw_RecordChangeComplete(ADODB::EventReasonEnum adReason,LONG cRecords,struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset) {
   *adStatus = ADODB::adStatusOK;
   if ( pError ) {
      MessageBoxW(NULL,pError -> Description,L"",MB_OK);
   }
   return S_OK;
   };
   
   STDMETHODIMP database::RecordsetEvents::raw_WillChangeRecordset(ADODB::EventReasonEnum adReason,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::RecordsetEvents::raw_RecordsetChangeComplete(ADODB::EventReasonEnum adReason,struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::RecordsetEvents::raw_WillMove(ADODB::EventReasonEnum adReason,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::RecordsetEvents::raw_MoveComplete(ADODB::EventReasonEnum adReason,struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::RecordsetEvents::raw_EndOfRecordset(VARIANT_BOOL *fMoreData,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::RecordsetEvents::raw_FetchProgress( long Progress,long MaxProgress,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };
   
   STDMETHODIMP database::RecordsetEvents::raw_FetchComplete(struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset) {
   *adStatus = ADODB::adStatusOK;
   return S_OK;
   };



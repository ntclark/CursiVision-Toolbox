
   class ConnectionEvents : public ADODB::ConnectionEventsVt {

   public:

      ConnectionEvents(database *pp) : refCount(0),pParent(pp) {};
      ~ConnectionEvents() {};

      STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
      STDMETHODIMP_(ULONG) AddRef(void);
      STDMETHODIMP_(ULONG) Release(void);

      STDMETHODIMP raw_InfoMessage(struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Connection *pConnection);
      STDMETHODIMP raw_BeginTransComplete(LONG TransactionLevel,struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Connection *pConnection);
      STDMETHODIMP raw_CommitTransComplete(struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Connection *pConnection);
      STDMETHODIMP raw_RollbackTransComplete(struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Connection *pConnection);
      STDMETHODIMP raw_WillExecute(BSTR *Source,ADODB::CursorTypeEnum *CursorType,ADODB::LockTypeEnum *LockType,long *Options,ADODB::EventStatusEnum *adStatus,struct ADODB::_Command *pCommand,struct ADODB::_Recordset *pRecordset,struct ADODB::_Connection *pConnection);
      STDMETHODIMP raw_ExecuteComplete(LONG RecordsAffected,struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Command *pCommand,struct ADODB::_Recordset *pRecordset,struct ADODB::_Connection *pConnection);      
      STDMETHODIMP raw_WillConnect(BSTR *ConnectionString,BSTR *UserID,BSTR *Password,long *Options,ADODB::EventStatusEnum *adStatus,struct ADODB::_Connection *pConnection);
      STDMETHODIMP raw_ConnectComplete(struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Connection *pConnection);
      STDMETHODIMP raw_Disconnect(ADODB::EventStatusEnum *adStatus,struct ADODB::_Connection *pConnection);

   private:

      database *pParent;
      ULONG refCount;

   } *pConnectionEvents;


   class RecordsetEvents : public ADODB::RecordsetEventsVt {
   public:

      RecordsetEvents(database *pp) : refCount(0), pParent(pp) {};
      ~RecordsetEvents() {};

      STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
      STDMETHODIMP_(ULONG) AddRef(void);
      STDMETHODIMP_(ULONG) Release(void);

      STDMETHODIMP raw_WillChangeField(LONG cFields,VARIANT Fields,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset);
      STDMETHODIMP raw_FieldChangeComplete(LONG cFields,VARIANT Fields,struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset);
      STDMETHODIMP raw_WillChangeRecord(ADODB::EventReasonEnum adReason,LONG cRecords,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset);
      STDMETHODIMP raw_RecordChangeComplete(ADODB::EventReasonEnum adReason,LONG cRecords,struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset);
      STDMETHODIMP raw_WillChangeRecordset(ADODB::EventReasonEnum adReason,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset);
      STDMETHODIMP raw_RecordsetChangeComplete(ADODB::EventReasonEnum adReason,struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset);
      STDMETHODIMP raw_WillMove(ADODB::EventReasonEnum adReason,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset);
      STDMETHODIMP raw_MoveComplete(ADODB::EventReasonEnum adReason,struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset);
      STDMETHODIMP raw_EndOfRecordset(VARIANT_BOOL *fMoreData,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset);
      STDMETHODIMP raw_FetchProgress(long Progress,long MaxProgress,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset);
      STDMETHODIMP raw_FetchComplete(struct ADODB::Error *pError,ADODB::EventStatusEnum *adStatus,struct ADODB::_Recordset *pRecordset);

   private:

      database *pParent;
      ULONG refCount;

   } *pRecordsetEvents;


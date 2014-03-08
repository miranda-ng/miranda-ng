unit KOLEdb;
{* This unit is created for KOL to allow to communicate with DB using OLE DB.
|<br> ========================================================================
|<br> Copyright (C) 2001 by Vladimir Kladov.
|<p>
  This unit conains three objects TDataSource, TSession and TQuery to implement
  the most important things: to connect to database, to control transactions,
  to perform commands (queries) and obtain results or update tables.
|</p>
}

interface

uses Windows, mComObj, KOL;

type
  INT64 = I64;
  PInt64 = PI64;

  tagVariant = packed Record
    vt: WORD;
    reserved1,
    reserved2,
    reserved3: WORD;
    case Integer of
    0: ( bVal       : Byte );
    1: ( iVal       : ShortInt );
    2: ( lVal       : Integer );
    3: ( fltVal     : Extended );
    4: ( dblVal     : Double );
    5: ( boolVal    : Bool );
    //6: ( scode      : SCODE );
    //7: ( cyVal      : CY );
    //8: ( date       : Date );
    9: ( bstrVal    : Pointer ); // BSTR => [ Len: Integer; array[ 1..Len ] of WideChar ]
    //10:( pdecVal    : ^Decimal );
    end;

(*
typedef struct tagVARIANT  {
   VARTYPE vt;
   unsigned short wReserved1;
   unsigned short wReserved2;
   unsigned short wReserved3;
   union {
      Byte                    bVal;                 // VT_UI1.
      Short                   iVal;                 // VT_I2.
      long                    lVal;                 // VT_I4.
      float                   fltVal;               // VT_R4.
      double                  dblVal;               // VT_R8.
      VARIANT_BOOL            boolVal;              // VT_BOOL.
      SCODE                   scode;                // VT_ERROR.
      CY                      cyVal;                // VT_CY.
      DATE                    date;                 // VT_DATE.
      BSTR                    bstrVal;              // VT_BSTR.
      DECIMAL                 FAR* pdecVal          // VT_BYREF|VT_DECIMAL.
      IUnknown                FAR* punkVal;         // VT_UNKNOWN.
      IDispatch               FAR* pdispVal;        // VT_DISPATCH.
      SAFEARRAY               FAR* parray;          // VT_ARRAY|*.
      Byte                    FAR* pbVal;           // VT_BYREF|VT_UI1.
      short                   FAR* piVal;           // VT_BYREF|VT_I2.
      long                    FAR* plVal;           // VT_BYREF|VT_I4.
      float                   FAR* pfltVal;         // VT_BYREF|VT_R4.
      double                  FAR* pdblVal;         // VT_BYREF|VT_R8.
      VARIANT_BOOL            FAR* pboolVal;        // VT_BYREF|VT_BOOL.
      SCODE                   FAR* pscode;          // VT_BYREF|VT_ERROR.
      CY                      FAR* pcyVal;          // VT_BYREF|VT_CY.
      DATE                    FAR* pdate;           // VT_BYREF|VT_DATE.
      BSTR                    FAR* pbstrVal;        // VT_BYREF|VT_BSTR.
      IUnknown                FAR* FAR* ppunkVal;   // VT_BYREF|VT_UNKNOWN.
      IDispatch               FAR* FAR* ppdispVal;  // VT_BYREF|VT_DISPATCH.
      SAFEARRAY               FAR* FAR* pparray;    // VT_ARRAY|*.
      VARIANT                 FAR* pvarVal;         // VT_BYREF|VT_VARIANT.
      void                    FAR* byref;           // Generic ByRef.
      char                    cVal;                 // VT_I1.
      unsigned short          uiVal;                // VT_UI2.
      unsigned long           ulVal;                // VT_UI4.
      int                     intVal;               // VT_INT.
      unsigned int            uintVal;              // VT_UINT.
      char FAR *              pcVal;                // VT_BYREF|VT_I1.
      unsigned short FAR *    puiVal;               // VT_BYREF|VT_UI2.
      unsigned long FAR *     pulVal;               // VT_BYREF|VT_UI4.
      int FAR *               pintVal;              // VT_BYREF|VT_INT.
      unsigned int FAR *      puintVal;             //VT_BYREF|VT_UINT.
   };
};
*)

{============= This part of code is grabbed from OLEDB.pas ================}
const
  MAXBOUND = 65535; { High bound for arrays }
  DBSTATUS_S_ISNULL = $00000003;

type
  PIUnknown = ^IUnknown;
  PUintArray = ^TUintArray;
  TUintArray = array[0..MAXBOUND] of UINT;

  HROW = UINT;
  PHROW = ^HROW;
  PPHROW = ^PHROW;

  HACCESSOR = UINT;
  HCHAPTER = UINT;
  DBCOLUMNFLAGS = UINT;
  DBTYPE = Word;
  DBKIND = UINT;
  DBPART = UINT;
  DBMEMOWNER = UINT;
  DBPARAMIO = UINT;
  DBBINDSTATUS = UINT;

const
  IID_NULL            : TGUID = '{00000000-0000-0000-0000-000000000000}';
  IID_IDataInitialize : TGUID = '{2206CCB1-19C1-11D1-89E0-00C04FD7A829}';
  CLSID_MSDAINITIALIZE: TGUID = '{2206CDB0-19C1-11D1-89E0-00C04FD7A829}';

  IID_IDBInitialize   : TGUID = '{0C733A8B-2A1C-11CE-ADE5-00AA0044773D}';
  //IID_IDBProperties : TGUID = '{0C733A8A-2A1C-11CE-ADE5-00AA0044773D}';
  IID_IDBCreateSession: TGUID = '{0C733A5D-2A1C-11CE-ADE5-00AA0044773D}';
  IID_IDBCreateCommand: TGUID = '{0C733A1D-2A1C-11CE-ADE5-00AA0044773D}';
  IID_ICommand        : TGUID = '{0C733A63-2A1C-11CE-ADE5-00AA0044773D}';
  IID_ICommandText    : TGUID = '{0C733A27-2A1C-11CE-ADE5-00AA0044773D}';
  IID_ICommandProperties: TGUID = '{0C733A79-2A1C-11CE-ADE5-00AA0044773D}';
  IID_IRowset         : TGUID = '{0C733A7C-2A1C-11CE-ADE5-00AA0044773D}';
  IID_IRowsetChange   : TGUID = '{0C733A05-2A1C-11CE-ADE5-00AA0044773D}';
  IID_IRowsetUpdate   : TGUID = '{0C733A6D-2A1C-11CE-ADE5-00AA0044773D}';
  IID_IColumnsInfo    : TGUID = '{0C733A11-2A1C-11CE-ADE5-00AA0044773D}';
  IID_IAccessor       : TGUID = '{0C733A8C-2A1C-11CE-ADE5-00AA0044773D}';

  // Added By ECM !!! ==================================================
  IID_ITransaction    : TGUID = '{0FB15084-AF41-11CE-BD2B-204C4F4F5020}';
  IID_ITransactionLocal: TGUID = '{0C733A5F-2A1C-11CE-ADE5-00AA0044773D}';
  IID_ITransactionOptions: TGUID = '{3A6AD9E0-23B9-11CF-AD60-00AA00A74CCD}';
  // ===================================================================

  // for version 1.5 of OLE DB:
  //DBGUID_DBSQL      : TGUID = '{c8b522df-5cf3-11ce-ade5-00aa0044773d}';

  // otherwise:
  DBGUID_DBSQL        : TGUID = '{C8B521FB-5CF3-11CE-ADE5-00AA0044773D}';
  DBGUID_DEFAULT      : TGUID = '{C8B521FB-5CF3-11CE-ADE5-00AA0044773D}';
  DBGUID_SQL          : TGUID = '{C8B522D7-5CF3-11CE-ADE5-00AA0044773D}';

  DBPROPSET_ROWSET    : TGUID = '{C8B522BE-5CF3-11CE-ADE5-00AA0044773D}';

  DB_S_ENDOFROWSET    = $00040EC6;

type

// *********************************************************************//
// Interface: IDBInitialize
// GUID:      {0C733A8B-2A1C-11CE-ADE5-00AA0044773D}
// *********************************************************************//
  IDBInitialize = interface(IUnknown)
    ['{0C733A8B-2A1C-11CE-ADE5-00AA0044773D}']
    function Initialize: HResult; stdcall;
    function Uninitialize: HResult; stdcall;
  end;

// *********************************************************************//
// Interface: IDBCreateCommand
// GUID:      {0C733A1D-2A1C-11CE-ADE5-00AA0044773D}
// *********************************************************************//
  IDBCreateCommand = interface(IUnknown)
    ['{0C733A1D-2A1C-11CE-ADE5-00AA0044773D}']
    function CreateCommand(const punkOuter: IUnknown; const riid: TGUID;
      out ppCommand: IUnknown): HResult; stdcall;
  end;

  (*---
  { Safecall Version }
  IDBCreateCommandSC = interface(IUnknown)
    ['{0C733A1D-2A1C-11CE-ADE5-00AA0044773D}']
    procedure CreateCommand(const punkOuter: IUnknown; const riid: TGUID;
      out ppCommand: IUnknown); safecall;
  end;
  ---*)

// *********************************************************************//
// Interface: IDBCreateSession
// GUID:      {0C733A5D-2A1C-11CE-ADE5-00AA0044773D}
// *********************************************************************//
  IDBCreateSession = interface(IUnknown)
    ['{0C733A5D-2A1C-11CE-ADE5-00AA0044773D}']
    function CreateSession(const punkOuter: IUnknown; const riid: TGUID;
      out ppDBSession: IUnknown): HResult; stdcall;
  end;

  (*---
  { Safecall Version }
  IDBCreateSessionSC = interface(IUnknown)
    ['{0C733A5D-2A1C-11CE-ADE5-00AA0044773D}']
    procedure CreateSession(const punkOuter: IUnknown; const riid: TGUID;
      out ppDBSession: IUnknown); safecall;
  end;
  ---*)

// *********************************************************************//
// Interface: IDataInitialize
// GUID:      {2206CCB1-19C1-11D1-89E0-00C04FD7A829}
// *********************************************************************//
  IDataInitialize = interface(IUnknown)
    ['{2206CCB1-19C1-11D1-89E0-00C04FD7A829}']
    function GetDataSource(const pUnkOuter: IUnknown; dwClsCtx: DWORD;
      pwszInitializationString: POleStr; const riid: TIID;
      var DataSource: IUnknown): HResult; stdcall;
    function GetInitializationString(const DataSource: IUnknown;
      fIncludePassword: Boolean; out pwszInitString: POleStr): HResult; stdcall;
    function CreateDBInstance(const clsidProvider: TGUID;
      const pUnkOuter: IUnknown; dwClsCtx: DWORD; pwszReserved: POleStr;
      riid: TIID; var DataSource: IUnknown): HResult; stdcall;
    function CreateDBInstanceEx(const clsidProvider: TGUID;
      const pUnkOuter: IUnknown; dwClsCtx: DWORD; pwszReserved: POleStr;
      pServerInfo: PCoServerInfo; cmq: ULONG; rgmqResults: Pointer): HResult; stdcall;
    function LoadStringFromStorage(pwszFileName: POleStr;
      out pwszInitializationString: POleStr): HResult; stdcall;
    function WriteStringToStorage(pwszFileName, pwszInitializationString: POleStr;
      dwCreationDisposition: DWORD): HResult; stdcall;
  end;

  (*---
  { Safecall Version }
  IDataInitializeSC = interface(IUnknown)
    ['{2206CCB1-19C1-11D1-89E0-00C04FD7A829}']
    procedure GetDataSource(const pUnkOuter: IUnknown; dwClsCtx: DWORD;
      pwszInitializationString: POleStr; const riid: TIID;
      var DataSource: IUnknown); safecall;
    procedure GetInitializationString(const DataSource: IUnknown;
      fIncludePassword: Boolean; out pwszInitString: POleStr); safecall;
    procedure CreateDBInstance(const clsidProvider: TGUID;
      const pUnkOuter: IUnknown; dwClsCtx: DWORD; pwszReserved: POleStr;
      riid: TIID; var DataSource: IUnknown); safecall;
    procedure CreateDBInstanceEx(const clsidProvider: TGUID;
      const pUnkOuter: IUnknown; dwClsCtx: DWORD; pwszReserved: POleStr;
      pServerInfo: PCoServerInfo; cmq: ULONG; rgmqResults: PMultiQI); safecall;
    procedure LoadStringFromStorage(pwszFileName: POleStr;
      out pwszInitializationString: POleStr); safecall;
    procedure WriteStringToStorage(pwszFileName, pwszInitializationString: POleStr;
      dwCreationDisposition: DWORD); safecall;
  end;
  ---*)

// *********************************************************************//
// Interface: ICommand
// GUID:      {0C733A63-2A1C-11CE-ADE5-00AA0044773D}
// *********************************************************************//
  ICommand = interface(IUnknown)
    ['{0C733A63-2A1C-11CE-ADE5-00AA0044773D}']
    function Cancel: HResult; stdcall;
    function Execute(const punkOuter: IUnknown; const riid: TGUID;
      pParams: Pointer; // var pParams: DBPARAMS;
      pcRowsAffected: PInteger; ppRowset: PIUnknown): HResult; stdcall;
    function GetDBSession(const riid: TGUID; out ppSession: IUnknown): HResult; stdcall;
  end;

  (*
  { Safecall Version }
  ICommandSC = interface(IUnknown)
    ['{0C733A63-2A1C-11CE-ADE5-00AA0044773D}']
    procedure Cancel; safecall;
    procedure Execute(const punkOuter: IUnknown; const riid: TGUID; var pParams: DBPARAMS;
      pcRowsAffected: PInteger; ppRowset: PIUnknown); safecall;
    procedure GetDBSession(const riid: TGUID; out ppSession: IUnknown); safecall;
  end;
  *)

// *********************************************************************//
// Interface: ICommandText
// GUID:      {0C733A27-2A1C-11CE-ADE5-00AA0044773D}
// *********************************************************************//
  ICommandText = interface(ICommand)
    ['{0C733A27-2A1C-11CE-ADE5-00AA0044773D}']
    function GetCommandText(var pguidDialect: TGUID;
      out ppwszCommand: PWideChar): HResult; stdcall;
    function SetCommandText(rguidDialect: PGUID;
      pwszCommand: PWideChar): HResult; stdcall;
  end;

  (*
  { Safecall Version }
  ICommandTextSC = interface(ICommand)
    ['{0C733A27-2A1C-11CE-ADE5-00AA0044773D}']
    procedure GetCommandText(var pguidDialect: TGUID;
      out ppwszCommand: PWideChar); safecall;
    procedure SetCommandText(rguidDialect: PGUID;
      pwszCommand: PWideChar); safecall;
  end;
  *)

// *********************************************************************//
// Interface: IRowset
// GUID:      {0C733A7C-2A1C-11CE-ADE5-00AA0044773D}
// *********************************************************************//
  IRowset = interface(IUnknown)
    ['{0C733A7C-2A1C-11CE-ADE5-00AA0044773D}']
    function AddRefRows(cRows: UINT; rghRows: PUintArray; rgRefCounts: PUintArray;
      rgRowStatus: PUintArray): HResult; stdcall;
    function GetData(HROW: HROW; HACCESSOR: HACCESSOR; pData: Pointer): HResult; stdcall;
    function GetNextRows(hReserved: HCHAPTER; lRowsOffset: Integer; cRows: Integer;
      out pcRowsObtained: UINT; {var prghRows: PUintArray} prghRows: Pointer ): HResult; stdcall;
    function ReleaseRows(cRows: UINT; rghRows: PUintArray; rgRowOptions,
      rgRefCounts, rgRowStatus: PUintArray): HResult; stdcall;
    function RestartPosition(hReserved: HCHAPTER): HResult; stdcall;
  end;

  (*
  { Safecall Version }
  IRowsetSC = interface(IUnknown)
    ['{0C733A7C-2A1C-11CE-ADE5-00AA0044773D}']
    procedure AddRefRows(cRows: UINT; rghRows: PUintArray; rgRefCounts: PUintArray;
      rgRowStatus: PUintArray); safecall;
    procedure GetData(HROW: HROW; HACCESSOR: HACCESSOR; pData: Pointer); safecall;
    procedure GetNextRows(hReserved: HCHAPTER; lRowsOffset: Integer; cRows: Integer;
      out pcRowsObtained: UINT; var prghRows: PUintArray); safecall;
    procedure ReleaseRows(cRows: UINT; rghRows: PUintArray; rgRowOptions,
      rgRefCounts, rgRowStatus: PUintArray); safecall;
    procedure RestartPosition(hReserved: HCHAPTER); safecall;
  end;
  *)

// *********************************************************************//
// Interface: IRowsetChange
// GUID:      {0C733A05-2A1C-11CE-ADE5-00AA0044773D}
// *********************************************************************//
  IRowsetChange = interface(IUnknown)
    ['{0C733A05-2A1C-11CE-ADE5-00AA0044773D}']
    function DeleteRows(hReserved: HCHAPTER; cRows: UINT; rghRows: PUintArray;
      rgRowStatus: PUintArray): HResult; stdcall;
    function SetData(HROW: HROW; HACCESSOR: HACCESSOR; pData: Pointer): HResult; stdcall;
    function InsertRow(hReserved: HCHAPTER; HACCESSOR: HACCESSOR; pData: Pointer;
      phRow: PHROW): HResult; stdcall;
  end;

  (*
  { Safecall Version }
  IRowsetChangeSC = interface(IUnknown)
    ['{0C733A05-2A1C-11CE-ADE5-00AA0044773D}']
    procedure DeleteRows(hReserved: HCHAPTER; cRows: UINT; rghRows: PUintArray;
      rgRowStatus: PUintArray); safecall;
    procedure SetData(HROW: HROW; HACCESSOR: HACCESSOR; pData: Pointer); safecall;
    procedure InsertRow(hReserved: HCHAPTER; HACCESSOR: HACCESSOR; pData: Pointer;
      phRow: PHROW); safecall;
  end;
  *)

// *********************************************************************//
// Interface: IRowsetUpdate
// GUID:      {0C733A6D-2A1C-11CE-ADE5-00AA0044773D}
// *********************************************************************//
  DBPENDINGSTATUS = DWORD;
  PDBPENDINGSTATUS = ^DBPENDINGSTATUS;
  PPDBPENDINGSTATUS = ^PDBPENDINGSTATUS;

  DBROWSTATUS = UINT;
  PDBROWSTATUS = ^DBROWSTATUS;
  PPDBROWSTATUS = ^PDBROWSTATUS;

  IRowsetUpdate = interface(IRowsetChange)
    ['{0C733A6D-2A1C-11CE-ADE5-00AA0044773D}']
    function GetOriginalData(HROW: HROW; HACCESSOR: HACCESSOR; pData: Pointer): HResult; stdcall;
    function GetPendingRows(hReserved: HCHAPTER; dwRowStatus: DBPENDINGSTATUS; pcPendingRows: PUINT;
      prgPendingRows: PPHROW; prgPendingStatus: PPDBPENDINGSTATUS): HResult; stdcall;
    function GetRowStatus(hReserved: HCHAPTER; cRows: UINT; rghRows: PUintArray;
      rgPendingStatus: PUintArray): HResult; stdcall;
    function Undo(hReserved: HCHAPTER; cRows: UINT; rghRows: PUintArray; pcRowsUndone: PUINT;
      prgRowsUndone: PPHROW; prgRowStatus: PPDBROWSTATUS): HResult; stdcall;
    function Update(hReserved: HCHAPTER; cRows: UINT; rghRows: PUintArray; pcRows: PUINT;
      prgRows: PPHROW; prgRowStatus: PPDBROWSTATUS): HResult; stdcall;
  end;

  (*
  { Safecall Version }
  IRowsetUpdateSC = interface(IRowsetChange)
    ['{0C733A6D-2A1C-11CE-ADE5-00AA0044773D}']
    procedure GetOriginalData(HROW: HROW; HACCESSOR: HACCESSOR; pData: Pointer); safecall;
    procedure GetPendingRows(hReserved: HCHAPTER; dwRowStatus: DBPENDINGSTATUS; pcPendingRows: PUINT;
      prgPendingRows: PPHROW; prgPendingStatus: PPDBPENDINGSTATUS); safecall;
    procedure GetRowStatus(hReserved: HCHAPTER; cRows: UINT; rghRows: PUintArray;
      rgPendingStatus: PUintArray); safecall;
    procedure Undo(hReserved: HCHAPTER; cRows: UINT; rghRows: PUintArray; pcRowsUndone: PUINT;
      prgRowsUndone: PPHROW; prgRowStatus: PPDBROWSTATUS); safecall;
    procedure Update(hReserved: HCHAPTER; cRows: UINT; rghRows: PUintArray; pcRows: PUINT;
      prgRows: PPHROW; prgRowStatus: PPDBROWSTATUS); safecall;
  end;
  *)

// *********************************************************************//
// Interface: ICommandProperties
// GUID:      {0C733A79-2A1C-11CE-ADE5-00AA0044773D}
// *********************************************************************//

  DBPROPID = UINT;
  PDBPROPID = ^DBPROPID;
  PDBPropIDArray = ^TDBPropIDArray;
  TDBPropIDArray = array[0..MAXBOUND] of DBPROPID;

  PDBIDGuid = ^TDBIDGuid;
  DBIDGUID = record
    case Integer of
      0: (guid: TGUID);
      1: (pguid: ^TGUID);
  end;
  TDBIDGuid = DBIDGUID;

  PDBIDName = ^TDBIDName;
  DBIDNAME = record
    case Integer of
      0: (pwszName: PWideChar);
      1: (ulPropid: UINT);
  end;
  TDBIDName = DBIDNAME;

  DBPROPOPTIONS = UINT;
  DBPROPSTATUS = UINT;
  PPDBID = ^PDBID;
  PDBID = ^DBID;
  DBID = packed record
    uGuid: DBIDGUID;
    eKind: DBKIND;
    uName: DBIDNAME;
  end;
  TDBID = DBID;

  PDBProp = ^TDBProp;
  DBPROP = packed record
    dwPropertyID: DBPROPID;
    dwOptions: DBPROPOPTIONS;
    dwStatus: DBPROPSTATUS;
    colid: DBID;
    vValue: tagVariant; // OleVariant;
  end;
  TDBProp = DBPROP;

  PDBPropArray = ^TDBPropArray;
  TDBPropArray = array[0..MAXBOUND] of TDBProp;

  PPDBPropSet = ^PDBPropSet;
  PDBPropSet = ^TDBPropSet;
  DBPROPSET = packed record
    rgProperties: PDBPropArray;
    cProperties: UINT;
    guidPropertySet: TGUID;
  end;
  TDBPropSet = DBPROPSET;

  PDBPropIDSet = ^TDBPropIDSet;
  DBPROPIDSET = packed record
    rgPropertyIDs: PDBPropIDArray;
    cPropertyIDs: UINT;
    guidPropertySet: TGUID;
  end;
  TDBPropIDSet = DBPROPIDSET;

  PDBPropIDSetArray = ^TDBPropIDSetArray;
  TDBPropIDSetArray = array[0..MAXBOUND] of TDBPropIDSet;

  PDBPropSetArray = ^TDBPropSetArray;
  TDBPropSetArray = array[0..MAXBOUND] of TDBPropSet;

  ICommandProperties = interface(IUnknown)
    ['{0C733A79-2A1C-11CE-ADE5-00AA0044773D}']
    function GetProperties(cPropertyIDSets: UINT; rgPropertyIDSets: PDBPropIDSetArray;
      var pcPropertySets: UINT; out prgPropertySets: PDBPropSet): HResult; stdcall;
    function SetProperties(cPropertySets: UINT; rgPropertySets: PDBPropSetArray): HResult; stdcall;
  end;

  (*
  { Safecall Version }
  ICommandPropertiesSC = interface(IUnknown)
    ['{0C733A79-2A1C-11CE-ADE5-00AA0044773D}']
    procedure GetProperties(cPropertyIDSets: UINT; rgPropertyIDSets: PDBPropIDSetArray;
      var pcPropertySets: UINT; out prgPropertySets: PDBPropSet); safecall;
    procedure SetProperties(cPropertySets: UINT; rgPropertySets: PDBPropSetArray); safecall;
  end;
  *)

  PDBIDArray = ^TDBIDArray;
  TDBIDArray = array[0..MAXBOUND] of TDBID;

  PDBColumnInfo = ^TDBColumnInfo;
  DBCOLUMNINFO = packed record
    pwszName: PWideChar;
    pTypeInfo: Pointer;
    iOrdinal: UINT;
    dwFlags: DBCOLUMNFLAGS;
    ulColumnSize: UINT;
    wType: DBTYPE;
    bPrecision: Byte;
    bScale: Byte;
    columnid: DBID;
  end;
  TDBColumnInfo = DBCOLUMNINFO;

  PColumnInfo = ^TColumnInfoArray;
  TColumnInfoArray = array[ 0..MAXBOUND ] of TDBColumnInfo;

// *********************************************************************//
// Interface: IColumnsInfo
// GUID:      {0C733A11-2A1C-11CE-ADE5-00AA0044773D}
// *********************************************************************//
  IColumnsInfo = interface(IUnknown)
    ['{0C733A11-2A1C-11CE-ADE5-00AA0044773D}']
    function GetColumnInfo(var pcColumns: UINT; out prgInfo: PDBColumnInfo;
      out ppStringsBuffer: PWideChar): HResult; stdcall;
    function MapColumnIDs(cColumnIDs: UINT; rgColumnIDs: PDBIDArray;
      rgColumns: PUintArray): HResult; stdcall;
  end;

  (*
  { Safecall Version }
  IColumnsInfoSC = interface(IUnknown)
    ['{0C733A11-2A1C-11CE-ADE5-00AA0044773D}']
    procedure GetColumnInfo(var pcColumns: UINT; out prgInfo: PDBColumnInfo;
      out ppStringsBuffer: PWideChar); safecall;
    procedure MapColumnIDs(cColumnIDs: UINT; rgColumnIDs: PDBIDArray;
      rgColumns: PUINTArray); safecall;
  end;
  *)

  PDBBindExt = ^TDBBindExt;
  DBBINDEXT = packed record
    pExtension: PByte;
    ulExtension: UINT;
  end;
  TDBBindExt = DBBINDEXT;

  PDBObject = ^TDBObject;
  DBOBJECT = packed record
    dwFlags: UINT;
    iid: TGUID;
  end;
  TDBObject = DBOBJECT;

  PDBBinding = ^TDBBinding;
  DBBINDING = packed record
    iOrdinal: UINT;
    obValue: UINT;
    obLength: UINT;
    obStatus: UINT;
    pTypeInfo: Pointer; //ITypeInfo; (reserved, should be nil)
    pObject: PDBObject;
    pBindExt: PDBBindExt;
    dwPart: DBPART;
    dwMemOwner: DBMEMOWNER;
    eParamIO: DBPARAMIO;
    cbMaxLen: UINT;
    dwFlags: UINT;
    wType: DBTYPE;
    bPrecision: Byte;
    bScale: Byte;
  end;
  TDBBinding = DBBINDING;

  PDBBindingArray = ^TDBBindingArray;
  TDBBindingArray = array[0..MAXBOUND] of TDBBinding;

const
  DBTYPE_EMPTY       = $00000000;
  DBTYPE_NULL        = $00000001;
  DBTYPE_I2          = $00000002;
  DBTYPE_I4          = $00000003;
  DBTYPE_R4          = $00000004;
  DBTYPE_R8          = $00000005;
  DBTYPE_CY          = $00000006;
  DBTYPE_DATE        = $00000007;
  DBTYPE_BSTR        = $00000008;
  DBTYPE_IDISPATCH   = $00000009;
  DBTYPE_ERROR       = $0000000A;
  DBTYPE_BOOL        = $0000000B;
  DBTYPE_VARIANT     = $0000000C;
  DBTYPE_IUNKNOWN    = $0000000D;
  DBTYPE_DECIMAL     = $0000000E;
  DBTYPE_UI1         = $00000011;
  DBTYPE_ARRAY       = $00002000;
  DBTYPE_BYREF       = $00004000;
  DBTYPE_I1          = $00000010;
  DBTYPE_UI2         = $00000012;
  DBTYPE_UI4         = $00000013;
  DBTYPE_I8          = $00000014;
  DBTYPE_UI8         = $00000015;
  DBTYPE_FILETIME    = $00000040;
  DBTYPE_GUID        = $00000048;
  DBTYPE_VECTOR      = $00001000;
  DBTYPE_RESERVED    = $00008000;
  DBTYPE_BYTES       = $00000080;
  DBTYPE_STR         = $00000081;
  DBTYPE_WSTR        = $00000082;
  DBTYPE_NUMERIC     = $00000083;
  DBTYPE_UDT         = $00000084;
  DBTYPE_DBDATE      = $00000085;
  DBTYPE_DBTIME      = $00000086;
  DBTYPE_DBTIMESTAMP = $00000087;
  DBTYPE_DBFILETIME  = $00000089;
  DBTYPE_PROPVARIANT = $0000008A;
  DBTYPE_VARNUMERIC  = $0000008B;

type
// *********************************************************************//
// Interface: IAccessor
// GUID:      {0C733A8C-2A1C-11CE-ADE5-00AA0044773D}
// *********************************************************************//
  IAccessor = interface(IUnknown)
    ['{0C733A8C-2A1C-11CE-ADE5-00AA0044773D}']
    function AddRefAccessor(HACCESSOR: HACCESSOR; pcRefCount: PUINT): HResult; stdcall;
    function CreateAccessor(dwAccessorFlags: UINT; cBindings: UINT; rgBindings: PDBBindingArray;
      cbRowSize: UINT; var phAccessor: HACCESSOR; rgStatus: PUintArray): HResult; stdcall;
    function GetBindings(HACCESSOR: HACCESSOR; pdwAccessorFlags: PUINT; var pcBindings: UINT;
      out prgBindings: PDBBinding): HResult; stdcall;
    function ReleaseAccessor(HACCESSOR: HACCESSOR; pcRefCount: PUINT): HResult; stdcall;
  end;

  (*
  { Safecall Version }
  IAccessorSC = interface(IUnknown)
    ['{0C733A8C-2A1C-11CE-ADE5-00AA0044773D}']
    procedure AddRefAccessor(HACCESSOR: HACCESSOR; pcRefCount: PUINT); safecall;
    procedure CreateAccessor(dwAccessorFlags: UINT; cBindings: UINT; rgBindings: PDBBindingArray;
      cbRowSize: UINT; var phAccessor: HACCESSOR; rgStatus: PUintArray); safecall;
    procedure GetBindings(HACCESSOR: HACCESSOR; pdwAccessorFlags: PUINT; var pcBindings: UINT;
      out prgBindings: PDBBinding); safecall;
    procedure ReleaseAccessor(HACCESSOR: HACCESSOR; pcRefCount: PUINT); safecall;
  end;
  *)

// Begin Added By ECM !!! =======================================================
  PBoid = ^TBoid;
  BOID = packed record
    rgb_: array[0..15] of Byte;
  end;
  TBoid = BOID;

  PXactTransInfo = ^TXactTransInfo;
  XACTTRANSINFO = packed record
    uow: BOID;
    isoLevel: Integer;
    isoFlags: UINT;
    grfTCSupported: UINT;
    grfRMSupported: UINT;
    grfTCSupportedRetaining: UINT;
    grfRMSupportedRetaining: UINT;
  end;
  TXactTransInfo = XACTTRANSINFO;

  PXactOpt = ^TXactOpt;
  XACTOPT = packed record
    ulTimeout: UINT;
    szDescription: array[0..39] of Shortint;
  end;
  TXActOpt = XACTOPT;

// *********************************************************************//
// Interface: ITransactionOptions
// GUID:      {3A6AD9E0-23B9-11CF-AD60-00AA00A74CCD}
// *********************************************************************//
  ITransactionOptions = interface(IUnknown)
    ['{3A6AD9E0-23B9-11CF-AD60-00AA00A74CCD}']
    function SetOptions(var pOptions: XACTOPT): HResult; stdcall;
    function GetOptions(var pOptions: XACTOPT): HResult; stdcall;
  end;

// *********************************************************************//
// Interface: ITransaction
// GUID:      {0FB15084-AF41-11CE-BD2B-204C4F4F5020}
// *********************************************************************//
  ITransaction = interface(IUnknown)
    ['{0FB15084-AF41-11CE-BD2B-204C4F4F5020}']
    function Commit(fRetaining: BOOL; grfTC: UINT; grfRM: UINT): HResult; stdcall;
    function Abort(pboidReason: PBOID; fRetaining: BOOL; fAsync: BOOL): HResult; stdcall;
    function GetTransactionInfo(out pinfo: XACTTRANSINFO): HResult; stdcall;
  end;

// *********************************************************************//
// Interface: ITransactionLocal
// GUID:      {0C733A5F-2A1C-11CE-ADE5-00AA0044773D}
// *********************************************************************//
  ITransactionLocal = interface(ITransaction)
    ['{0C733A5F-2A1C-11CE-ADE5-00AA0044773D}']
    function GetOptionsObject(out ppOptions: ITransactionOptions): HResult; stdcall;
    function StartTransaction(isoLevel: Integer; isoFlags: UINT;
      const pOtherOptions: ITransactionOptions; pulTransactionLevel: PUINT): HResult; stdcall;
  end;

const
  XACTTC_SYNC_PHASEONE = $00000001;
  XACTTC_SYNC_PHASETWO = $00000002;
  XACTTC_SYNC = $00000002;
  XACTTC_ASYNC_PHASEONE = $00000004;
  XACTTC_ASYNC = $00000004;

// End Added By ECM !!! =========================================================

// Begin Added By azsd !!! ======================================================
(*
type
  PDbNumeric = ^tagDB_NUMERIC;
  tagDB_NUMERIC = packed record
    precision: Byte;
    scale: Byte;
    sign: Byte;
    val: array[0..15] of Byte;
  end;
*)
// End Added By azsd !!! ========================================================

{============= This part of code is designed by me ================}
type
  PDBBINDSTATUSARRAY = ^TDBBINDSTATUSARRAY;
  TDBBINDSTATUSARRAY = array[ 0..MAXBOUND ] of DBBINDSTATUS;

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//  TDataSource - a connection to data base
//,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
type
  PDataSource = ^TDataSource;
  TDataSource = object( TObj )
  {* This object provides a connection with data base. You create it using
     NewDataSource function and passing a connection string to it. The object
     is initializing immediately after creating. You can get know if the
     connection established successfully reading Intitialized property. }
  private
    fSessions: PList;
    fIDBInitialize: IDBInitialize;
    FInitialized: Boolean;
  protected
    function Initialize( const Params: String ): Boolean;
  public
    constructor Create;
    {* Do not call this constructor. Use function NewDataSource instead. }
    destructor Destroy; virtual;
    {* Do not call this destructor. Use Free method instead. When TDataSource
       object is destroyed, all its sessions (and consequensly, all queries)
       are freed automatically. }
    property Initialized: Boolean read FInitialized;
    {* Returns True, if the connection with database is established. Mainly,
       it is not necessary to analizy this flag. If any error occure during
       initialization, CheckOle halts further execution. (But You can use
       another error handler, which does not stop the application). }
  end;

function NewDataSource( const Params: String ): PDataSource;
{* Creates data source objects and initializes it. Pass a connection
   string as a parameter, which determines used provider, database
   location, user identification and other parameters. See demo provided
   or/and read spicifications from database software vendors, which
   parameters to pass. }

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//  TSession - transaction session in a connection
//,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
type
  PSession = ^TSession;
  TSession = object( TObj )
  {* This object is intended to provide session transactions. It always
     must be created as a "child" of TDataSource object, and it owns by
     query objects (of type TQuery). For each TDataSource object, it is
     possible to create several TSession objects, and for each session,
     several TQuery objects can exist. }
  private
    fQueryList: PList;
    fDataSource: PDataSource;
    fCreateCommand: IDBCreateCommand;

    // Added By ECM !!! ==================
    fTransaction: ITransaction;
    fTransactionLocal: ITransactionLocal;
    // ===================================

  protected
  public
    constructor Create;
    {* }
    destructor Destroy; virtual;
    {* Do not call directly, call Free method instead. When TSession object is
       destroyed, all it child queries are freed automatically. }

    // Added By ECM !!! ====================================
    function   StartTransaction(isoLevel: Integer): HRESULT;
    function   Commit(Retaining: BOOL): HRESULT;
    function   Rollback(Retaining: BOOL): HRESULT;
    function   Active: Boolean;
    // =====================================================

    property DataSource: PDataSource read fDataSource;
    {* Returns a pointer to owner TDataSource object. }
  end;

function NewSession( ADataSource: PDataSource ): PSession;
{* Creates session object owned by ADataSource (this last must exist). }

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//  TQuery - a command and resulting rowset(s)
//,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
type
  TRowsetMode = ( rmUpdateImmediate, rmUpdateDelayed, rmReadOnly );
  TFieldType = ( ftInteger, ftReal, ftString, ftDate, ftLargeInt, ftOther );

  PQuery = ^TQuery;
  TQuery = object( TObj )
  {* This is the most important object to work with database. It is always
     must be created as a "child" of TSession object, and allows to perform
     commands, open rowsets, scroll it, update and so on. }
  private
    fSession: PSession;
    fText: String;
    fCommand: ICommandText;
    fCommandProps: ICommandProperties;
    fRowsAffected: Integer;
    fRowSet: IRowset;
    fRowSetChg: IRowsetChange;
    fRowSetUpd: IRowsetUpdate;
    fColCount: UINT;
    fColInfo: PColumnInfo;
    fColNames: PWideChar;
    fBindings: PDBBindingArray;
    fBindStatus: PDBBINDSTATUSARRAY;
    fRowSize: Integer;
    fAccessor: HACCESSOR;
    fRowHandle: THandle;
    fRowBuffers: PList;
    fEOF: Boolean;
    fCurIndex: Integer;
    fChanged: Boolean;
    fMode: TRowsetMode;
    procedure SetText(const Value: String);
    function GetRowCount: Integer;
    function GetColNames(Idx: Integer): String;
    procedure SetCurIndex(const Value: Integer);
    function GetRowsKnown: Integer;
    function GetStrField(Idx: Integer): String;
    procedure SetStrField(Idx: Integer; const Value: String);
    function GetIntField(Idx: Integer): Integer;
    procedure SetIntField(Idx: Integer; const Value: Integer);
    function GetFltField(Idx: Integer): Double;
    procedure SetFltField(Idx: Integer; const Value: Double);
    function GetDField(Idx: Integer): TDateTime;
    procedure SetDField(Idx: Integer; const Value: TDateTime);
    function FieldPtr( Idx: Integer ): Pointer;
    function Changed( Idx: Integer ): Pointer;
    function GetColByName(Name: String): Integer;
    function GetSFieldByName(const Name: String): String;
    procedure SetSFieldByName(const Name: String; const Value: String);
    function GetIFieldByName(const Name: String): Integer;
    procedure SetIFieldByName(const Name: String; Value: Integer);
    function GetRFieldByName(const Name: String): Double;
    procedure SetRFieldByName(const Name: String; const Value: Double);
    function GetDFlfByName(const Name: String): TDateTime;
    procedure SetDFldByName(const Name: String; const Value: TDateTime);
    function GetColType(Idx: Integer): TFieldType;
    function GetColTypeByName(const Name: String): TFieldType;
    function GetIsNull(Idx: Integer): Boolean;
    procedure SetIsNull(Idx: Integer; const Value: Boolean);
    function GetIsNullByName(const Name: String): Boolean;
    procedure SetIsNullByName(const Name: String; const Value: Boolean);
    function GetFByNameAsStr(const Name: String): String;
    function GetFieldAsStr(Idx: Integer): String;
    procedure SetFByNameFromStr(const Name, Value: String);
    procedure SetFieldFromStr(Idx: Integer; const Value: String);
    function GetI64Field(Idx: Integer): Int64;
    function GetI64FldByName(const Name: String): Int64;
    procedure SetI64Field(Idx: Integer; const Value: Int64);
    procedure SetI64FldByName(const Name: String; const Value: Int64);
    function GetFixupNumeric(Idx: Integer): Int64; //add by azsd
    function GetRawType(Idx: Integer): DWORD;
    function GetRawTypeByName(const Name: String): DWORD;
    function GetFieldAsHex(Idx: Integer): Pointer;
    function GetFieldByNameAsHex(const Name: String): Pointer;
  protected
    fDelList: PList;
    procedure ClearRowset;
    procedure ReleaseHandle;
    procedure FetchData;
    procedure NextWOFetch( Skip: Integer );
  public
    destructor Destroy; virtual;
    {* Do not call the destructor directly, call method Free instead. When
       "parent" TSession object is destroyed, all queries owned by the session
       are destroyed automatically. }
    property Session: PSession read fSession;
    {* Returns owner session object. }
    property Text: String read FText write SetText;
    {* Query command text. When You change it, currently opened rowset (if any)
       is closed, so there are no needs to call Close method before preparing
       for new command. Current version does not support passing "parameters",
       so include all values into Text as a part of string. }
    procedure Close;
    {* Closes opened rowset if any. It is not necessary to call close after
       Execute. Also, rowset is closed automatically when another value is
       assigned to Text property. }
    procedure Execute;
    {* Call this method to execute command (stored in Text), which does not
       open a rowset (thus is, "insert", "delete", and "update" SQL statements
       do so). }
    procedure Open;
    {* Call this method for executing command, which opens a rowset (table of
       data). This can be "select" SQL statement, or call to stored procedure,
       which returns result in a table. }
    property RowCount: Integer read GetRowCount;
    {* For commands, such as "insert", "delete" or "update" SQL statements,
       this property returns number of rows affected by a command. For "select"
       statement performed using Open method, this property should return
       a number of rows selected. By for (the most) providers, this value is
       unknown for first time (-1 is returned). To get know how much rows are
       in returned rowset, method Last should be called first. But for large
       data returned this is not efficient way, because actually a loop
       "while not EOF do Next" is performed to do so.
       |<br>
       Tip: to get count of rows, You can call another query, which executes
       "select count(*) where..." SQL statement with the same conditions. }
    property RowsKnown: Integer read GetRowsKnown;
    {* Returns actual number or selected rows, if this is "known" value, or number
       of rows already fetched. }
    property ColCount: UINT read fColCount;
    {* Returns number of columns in opened rowset. }
    property ColNames[ Idx: Integer ]: String read GetColNames;
    {* Return names of columns. }
    property ColByName[ Name: String ]: Integer read GetColByName;
    {* Returns column index by name. Comparing of names is ANSI and case insensitive. }
    property ColType[ Idx: Integer ]: TFieldType read GetColType;
    {* }
    property ColTypeByName[ const Name: String ]: TFieldType read GetColTypeByName;
    {* }
    function FirstColumn: Integer;
    {* by Alexander Shakhaylo. To return an index of the first column,
       containing actual data. (for .mdb, the first can contain special
       control information, but not for .dbf) }
    property RawType[ Idx: Integer ]: DWORD read GetRawType;
    {*}
    property RawTypeByName[ const Name: String ]: DWORD read GetRawTypeByName;
    {*}
    property EOF: Boolean read fEOF;
    {* Returns True, if end of data is achived (usually after calling Next
       or Prev method, or immediately after Open, if there are no rows in opened
       rowset). }
    procedure First;
    {* Resets a position to the start of rowset. This method is called
       automatically when Open is called successfully. }
    procedure Next;
    {* Moves position to the next row if possible. If EOF achived, a position
       is not changed. }
    procedure Prev;
    {* Moves position to a previous row (but if CurIndex > 0). }
    procedure Last;
    {* Moves position to the last row. This method can be unefficient for
       large datasets, because implemented as a loop where method Next is
       called repeteadly, while EOF is not achieved. }
    property Mode: TRowsetMode read fMode write fMode;
    {* }
    procedure Post;
    {* Applyes changes made in a record, writing changed row to database table. }
    procedure Delete;
    {* Deletes a row. In rmUpdateDelayed Mode, rows are only added to a list
       for later deleting it when Update called. }
    procedure Update;
    {* Allows to apply all updates far later, not when Post method is called.
       To use TQuery in this manner, its Mode should be set to rmUpdateDelayed. }
    property CurIndex: Integer read fCurIndex write SetCurIndex;
    {* Index of current row. It is possible to change it directly even if
       specified row is not yet fetched. But check at least what new value is
       stored in CurIndex after such assignment. }
    property SField[ Idx: Integer ]: String read GetStrField write SetStrField;
    {* Access to a string field by index. You should be sure, that a field
       has string type. }
    property SFieldByName[ const Name: String ]: String read GetSFieldByName write SetSFieldByName;
    {* }
    property IField[ Idx: Integer ]: Integer read GetIntField write SetIntField;
    {* Access to a integer field by index. You should be sure, that a field
       has integer type or compatible. }
    property IFieldByName[ const Name: String ]: Integer read GetIFieldByName write SetIFieldByName;
    {* }
    property LField[ Idx: Integer ]: Int64 read GetI64Field write SetI64Field;
    {* }
    property LFieldByName[ const Name: String ]: Int64 read GetI64FldByName write SetI64FldByName;
    {* }
    property RField[ Idx: Integer ]: Double read GetFltField write SetFltField;
    {* Access to a real (Double) field by index. You should be sure, that a field
       has numeric (with floating decimal point) type. }
    property RFieldByName[ const Name: String ]: Double read GetRFieldByName write SetRFieldByName;
    {* }
    property DField[ Idx: Integer ]: TDateTime read GetDField write SetDField;
    {* }
    property DFieldByName[ const Name: String ]: TDateTime read GetDFlfByName write SetDFldByName;
    {* }
    property IsNull[ Idx: Integer ]: Boolean read GetIsNull write SetIsNull;
    {* }
    property IsNullByName[ const Name: String ]: Boolean read GetIsNullByName write SetIsNullByName;
    {* }
    property FieldAsStr[ Idx: Integer ]: String read GetFieldAsStr write SetFieldFromStr;
    {* }
    property FieldByNameAsStr[ const Name: String ]: String read GetFByNameAsStr write SetFByNameFromStr;
    {* }
    property FieldAsHex[ Idx: Integer ]: Pointer read GetFieldAsHex;
    {* Access to field data directly. If you change field data inplace, call
       MarkRecordChanged by yourself. If field IsNull, data found at the address
       provided have no sense. }
    property FieldByNameAsHex[ const Name: String ]: Pointer read GetFieldByNameAsHex;
    {* See FieldByNameAsHex. }
    procedure MarkFieldChanged( Idx: Integer );
    {* See also MarkRecordChangedByName. }
    procedure MarkFieldChangedByName( const Name: String );
    {* When record field changed directly (using FieldAsHex property, for ex.),
       use this method to signal to record set container, that record is changed,
       and to ensure that field no more marked as null. }
  end;

function NewQuery( Session: PSession ): PQuery;
{* Creates query object. }

// Error handling routines:

function CheckOLE( Rslt: HResult ): Boolean;
function CheckOLEex( Rslt: HResult; const OKResults: array of HResult ): Boolean;
procedure DummyOleError( Result: HResult );
var OleError: procedure( Result: HResult ) = DummyOleError;

implementation

type
  PDBNumeric = ^TDBNumeric;
  TDBNUMERIC = packed record
    precision: Byte;
    scale: Byte;
    sign: Byte;
    val: array[0..15] of Byte;
  end;

  PDBVarNumeric = ^TDBVarNumeric;
  TDBVARNUMERIC = packed record
    precision: Byte;
    scale: ShortInt;
    sign: Byte;
    val: ^Byte;
  end;

  PDBDate = ^TDBDate;
  TDBDATE = packed record
    year: Smallint;
    month: Word;
    day: Word;
  end;

  PDBTime = ^TDBTIME;
  TDBTIME = packed record
    hour: Word;
    minute: Word;
    second: Word;
  end;

  PDBTimeStamp = ^TDBTimeStamp;
  TDBTIMESTAMP = packed record
    year: Smallint;
    month: Word;
    day: Word;
    hour: Word;
    minute: Word;
    second: Word;
    fraction: UINT;
  end;

var fIMalloc: IMalloc = nil;

(* procedure DummyOleError( Result: HResult );
begin
  MsgOK( 'OLE DB error ' + Int2Hex( Result, 8 ) );
  Halt;
end; *)

procedure DummyOleError( Result: HResult );
begin
  {$IFNDEF FPC}
  raise Exception.Create( e_Custom, 'OLE DB error ' + Int2Hex( Result, 8 ) );
  {$ENDIF}
end;

function CheckOLE( Rslt: HResult ): Boolean;
begin
  Result := Rslt = 0;
  if not Result then
    OleError( Rslt );
end;

function CheckOLEex( Rslt: HResult; const OKResults: array of HResult ): Boolean;
var I: Integer;
begin
  Result := TRUE;
  for I := Low( OKResults ) to High( OKResults ) do
    if Rslt = OKResults[ I ] then Exit;
  Result := FALSE;
  OleError( Rslt );
end;

{ TDataSource }

function NewDataSource( const Params: String ): PDataSource;
begin
  new( Result, Create );
  Result.Initialize( Params );
end;

constructor TDataSource.Create;
var clsid: TCLSID;
begin
  inherited;
  fSessions := NewList;
  //if CheckOLEex( CoInitialize( nil ), [ S_OK, S_FALSE ] ) then
  OleInit;
  if CheckOLE( CoGetMalloc( MEMCTX_TASK, fIMalloc ) ) then
  if CheckOLE( CLSIDFromProgID( 'SQLOLEDB', clsid ) ) then
    CheckOLE( CoCreateInstance( clsid, nil, CLSCTX_INPROC_SERVER,
      IID_IDBInitialize, fIDBInitialize ) );
end;

destructor TDataSource.Destroy;
var I: Integer;
begin
  for I := fSessions.Count - 1 downto 0 do
    PObj( fSessions.Items[ I ] ).Free;
  fSessions.Free;
  if Initialized then
    CheckOLE( fIDBInitialize.UnInitialize );
  OleUnInit;
  inherited;
end;

function TDataSource.Initialize( const Params: String ): Boolean;
var DI: IDataInitialize;
    Unk: IUnknown;
begin
  Result := FALSE;
  if Initialized then
  begin
    Result := TRUE;
    Exit;
  end;
  if CheckOLE( CoCreateInstance( CLSID_MSDAINITIALIZE, nil,
          CLSCTX_ALL, IID_IDataInitialize, DI ) ) then
  if CheckOLE( DI.GetDataSource( nil, CLSCTX_ALL, StringToOleStr( Params ),
            IID_IDBInitialize, Unk ) ) then
  if CheckOLE( Unk.QueryInterface( IID_IDBInitialize, fIDBInitialize ) ) then
  if CheckOLE( fIDBInitialize.Initialize ) then
  begin
    Result := TRUE;
    FInitialized := Result;
  end;
end;

{ TSession }

function NewSession( ADataSource: PDataSource ): PSession;
var CreateSession: IDBCreateSession;
    Unk: IUnknown;
begin
  new( Result, Create );
  Result.fDataSource := ADataSource;
  ADataSource.fSessions.Add( Result );
  // Modified by ECM !!! ===============================================================================
  if CheckOLE( ADataSource.fIDBInitialize.QueryInterface( IID_IDBCreateSession, CreateSession ) ) then begin
    CheckOLE( CreateSession.CreateSession( nil, IID_IDBCreateCommand,
            IUnknown( Result.fCreateCommand ) ) );

    Unk := Result.fCreateCommand;
    if Assigned(Unk) then begin
      CheckOLE(Unk.QueryInterface(IID_ITransaction,Result.fTransaction));
      CheckOLE(Unk.QueryInterface(IID_ITransactionLocal,Result.fTransactionLocal));
    end;
  end;
  // =================================================================================================
end;

// Added By ECM !!! ==============================================
function TSession.Active: Boolean;
var
  xinfo: TXactTransInfo;
  Ret: HRESULT;
begin
  if not Assigned(fTransaction) then Result := FALSE
  else begin
    FillChar(xinfo,SizeOf(xinfo),0);
    Ret := fTransaction.GetTransactionInfo(xinfo);
    Result := Ret = S_OK;
    CheckOLE(Ret);
  end;
end;

function TSession.Commit(Retaining: BOOL): HRESULT;
begin
  Assert(Assigned(fTransaction));
  Result := fTransaction.Commit(Retaining,XACTTC_SYNC,0);
  CheckOLE(Result);
end;
// ===============================================================

constructor TSession.Create;
begin
  inherited;
  fQueryList := NewList;
end;

destructor TSession.Destroy;
var I: Integer;
begin
  for I := fQueryList.Count - 1 downto 0 do
    PObj( fQueryList.Items[ I ] ).Free;
  fQueryList.Free;
  I := fDataSource.fSessions.IndexOf( @Self );
  fDataSource.fSessions.Delete( I );
  // Add By ECM !!! ================
  // if Active then Rollback(FALSE);
  //================================
  fCreateCommand := nil;
  inherited;
end;

// Added By ECM !!! ===============================================
function TSession.Rollback(Retaining: BOOL): HRESULT;
begin
  Assert(Assigned(fTransaction));
  Result := fTransaction.Abort(nil,Retaining,FALSE);
  CheckOLE(Result);
end;

function TSession.StartTransaction(isoLevel: Integer): HRESULT;
begin
  Assert(Assigned(fTransactionLocal));
  Result := fTransactionLocal.StartTransaction(isoLevel,0,nil,nil);
  CheckOLE(Result);
end;
// ================================================================

{ TQuery }

function NewQuery( Session: PSession ): PQuery;
begin
  new( Result, Create );
  Result.fSession := Session;
  Session.fQueryList.Add( Result );
  CheckOLE( Session.fCreateCommand.CreateCommand( nil, IID_ICommandText,
            IUnknown( Result.fCommand ) ) );
end;

function TQuery.Changed( Idx: Integer ): Pointer;
begin
  fChanged := TRUE;
  Result := Pointer( DWORD( fRowBuffers.Items[ fCurIndex ] ) +
                   fBindings[ Idx ].obStatus );
  PDWORD( Result )^ := 0; // set to NOT NULL
end;

procedure TQuery.ClearRowset;
var I: Integer;
    AccessorIntf: IAccessor;
begin
  ReleaseHandle;

  if fAccessor <> 0 then
  begin
    if CheckOLE( fRowSet.QueryInterface( IID_IAccessor, AccessorIntf ) ) then
      AccessorIntf.ReleaseAccessor( fAccessor, nil );
    fAccessor := 0;
  end;

  if fRowBuffers <> nil then
  begin
    for I := fRowBuffers.Count - 1 downto 0 do
      FreeMem( fRowBuffers.Items[ I ] );
    fRowBuffers.Free;
    fRowBuffers := nil;
  end;
  fRowSize := 0;

  if fBindings <> nil then
  begin
    //for I := 0 to fColCount - 1 do
    //  fBindings[ I ].pTypeInfo := nil;
    FreeMem( fBindings );
    fBindings := nil;
    FreeMem( fBindStatus );
    fBindStatus := nil;
  end;

  if fColInfo <> nil then
    fIMalloc.Free( fColInfo );
  fColInfo := nil;

  if fColNames <> nil then
    fIMalloc.Free( fColNames );
  fColNames := nil;

  fColCount := 0;
  fRowSetUpd := nil;
  fRowSet := nil;
  fRowSetChg := nil;
  fRowsAffected := 0;

  fEOF := TRUE;
end;

procedure TQuery.Close;
begin
  Update;
  ClearRowset;
end;

procedure TQuery.Delete;
var Params, Results: array of DWORD;
begin
  //if fRowHandle = 0 then Exit;
  CASE fMode OF
  rmUpdateImmediate:
    begin
      SetLength( Results, 1 );
      SetLength( Params, 1 );
      Params[ 0 ] := fRowHandle;
      CheckOLE( fRowSetUpd.DeleteRows( 0, 1, @ Params[ 0 ], @ Results[ 0 ] ) );
    end;
  rmUpdateDelayed:
    begin
      if fDelList = nil then
        fDelList := NewList;
      fDelList.Add( Pointer( fRowHandle ) );
    end;
  END;
end;

destructor TQuery.Destroy;
var I: Integer;
begin
  Close; //ClearRowset;
  I := fSession.fQueryList.IndexOf( @Self );
  if I >= 0 then
    fSession.fQueryList.Delete( I );
  fText := '';
  fCommandProps := nil;
  fCommand := nil;
  fDelList.Free;
  inherited;
end;

procedure TQuery.Execute;
begin
  ClearRowset;
  // first set txt to fCommand just before execute
  if CheckOLE( fCommand.SetCommandText( @DBGUID_DBSQL, StringToOleStr( fText ) ) ) then
    CheckOLE( fCommand.Execute( nil, IID_NULL, nil, @fRowsAffected, nil ) );
end;

procedure TQuery.FetchData;
var Buffer: Pointer;
begin
  if fRowHandle = 0 then
    Exit;
  if fRowBuffers.Items[ fCurIndex ] = nil then
  begin
    GetMem( Buffer, fRowSize );
    FillChar( Buffer^, fRowSize, 0 ); //fixup the varnumberic random bytes by azsd
    fRowBuffers.Items[ fCurIndex ] := Buffer;
    CheckOLE( fRowSet.GetData( fRowHandle, fAccessor, fRowBuffers.Items[ fCurIndex ] ) );
  end;
end;

function TQuery.FieldPtr(Idx: Integer): Pointer;
begin
  if (fRowSet = nil) or (fCurIndex < 0) or (DWORD(Idx) >= ColCount) then
    Result := nil
  else
    Result := Pointer( DWORD( fRowBuffers.Items[ fCurIndex ] ) +
                   fBindings[ Idx ].obValue );
end;

procedure TQuery.First;
begin
  if fCurIndex = 0 then Exit;
  ReleaseHandle;
  fCurIndex := -1;
  if CheckOLE( fRowSet.RestartPosition( 0 ) ) then
  begin
    fEOF := FALSE;
    Next;
  end;
end;

function TQuery.FirstColumn: Integer;
var i: integer;
begin
   Result := -1;
   for i := 0 to fColCount - 1 do begin
      if fBindings[i].iOrdinal > 0 then begin
         Result := i;
         exit;
      end;
   end;
end;

function TQuery.GetColByName(Name: String): Integer;
var I: Integer;
begin
  Result := -1;
  for I := 0 to fColCount - 1 do
  begin
    if AnsiCompareStrNoCase( Name, ColNames[ I ] ) = 0 then
    begin
      Result := I;
      break;
    end;
  end;
end;

function TQuery.GetColNames(Idx: Integer): String;
begin
  Result := fColInfo[ Idx ].pwszName;
end;

function TQuery.GetColType(Idx: Integer): TFieldType;
begin
  Result := ftOther;
  if fBindings = nil then Exit;
  case fBindings[ Idx ].wType of
  DBTYPE_I1,  DBTYPE_I2,  DBTYPE_I4, DBTYPE_BOOL,
  DBTYPE_UI1, DBTYPE_UI2, DBTYPE_UI4 : Result := ftInteger;
  DBTYPE_I8,  DBTYPE_UI8      : Result := ftLargeInt;
  DBTYPE_BSTR, DBTYPE_WSTR, DBTYPE_STR: Result := ftString;
  DBTYPE_R4, DBTYPE_R8, DBTYPE_CY,
  DBTYPE_NUMERIC, DBTYPE_VARNUMERIC,
  DBTYPE_DECIMAL              : Result := ftReal;// no need new cate here,moved to GetFieldAsStr
  DBTYPE_DATE, DBTYPE_FILETIME, //DBTYPE_DBFILETIME,
  DBTYPE_DBDATE, DBTYPE_DBTIME,
  DBTYPE_DBTIMESTAMP          : Result := ftDate;
  else   Result := ftOther;
  end;
end;

function TQuery.GetColTypeByName(const Name: String): TFieldType;
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >= 0, 'Incorrect column name (' + Name + ').' );
  Result := ColType[ Idx ];
end;

function TQuery.GetDField(Idx: Integer): TDateTime;
var P: Pointer;
    ST: TSystemTime;
    pD: PDBDate;
    pT: PDBTime;
    TS: PDBTimeStamp;
    pFT: PFileTime;
begin
  P := FieldPtr( Idx );
  if P = nil then
    Result := 0.0
  else
  begin
    FillChar( ST, Sizeof(ST), 0 );
    case fBindings[ Idx ].wType of
    DBTYPE_DATE: Result := PDouble( P )^ + VCLDate0;
    DBTYPE_DBDATE:
      begin
        pD := P;
        ST.wYear := pD.year;
        ST.wMonth := pD.month;
        ST.wDay := pD.day;
        SystemTime2DateTime( ST, Result );
      end;
    DBTYPE_DBTIME:
      begin
        pT := P;
        ST.wYear := 1899;
        ST.wMonth := 12;
        ST.wDay := 31;
        ST.wHour := pT.hour;
        ST.wMinute := pT.minute;
        ST.wSecond := pT.second;
        SystemTime2DateTime( ST, Result );
        Result := Result - VCLDate0;
      end;
    DBTYPE_DBTIMESTAMP:
      begin
        TS := P;
        ST.wYear := TS.year;
        ST.wMonth := TS.month;
        ST.wDay := TS.day;
        ST.wHour := TS.hour;
        ST.wMinute := TS.minute;
        ST.wSecond := TS.second;
        ST.wMilliseconds := TS.fraction div 1000000;
        SystemTime2DateTime( ST, Result );
      end;
    DBTYPE_FILETIME:
      begin
        pFT := P;
        FileTimeToSystemTime( pFT^, ST );
        SystemTime2DateTime( ST, Result );
      end;
    else Result := 0.0;
    end;
  end;
end;

function TQuery.GetDFlfByName(const Name: String): TDateTime;
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  Result := DField[ Idx ];
end;

function TQuery.GetFByNameAsStr(const Name: String): String;
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  Result := FieldAsStr[ Idx ];
end;

function TQuery.GetFieldAsHex(Idx: Integer): Pointer;
begin
  {if IsNull[ Idx ] then
    Result := nil
  else}
    Result := FieldPtr( Idx );
end;

function TQuery.GetFieldAsStr(Idx: Integer): String;
begin
  if IsNull[ Idx ] then
    Result := '(null)'
  else
  case ColType[ Idx ] of
  ftReal:
    //added optimize by azsd
    begin
      case fBindings[ Idx ].wType of
      DBTYPE_NUMERIC,DBTYPE_VARNUMERIC:
        if ShortInt(PDBNumeric(FieldPtr(Idx)).scale)<>0 then
          Result := Double2Str( RField[ Idx ] )
        else
          Result := Int64_2Str( LField[ Idx ] );
      else
        Result := Double2Str( RField[ Idx ] );
      end;
    end;
  ftString:   Result := SField[ Idx ];
  ftDate:     Result := DateTime2StrShort( DField[ Idx ] );
  ftLargeInt: Result := Int64_2Str( LField[ Idx ] );//add by azsd
  //ftInteger:
  else        Result := Int2Str( IField[ Idx ] );
  //else        Result := '(?)';
  end;
end;

function TQuery.GetFieldByNameAsHex(const Name: String): Pointer;
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  Result := FieldAsHex[ Idx ];
end;

function TQuery.GetFltField(Idx: Integer): Double;
var P: Pointer;
begin
  P := FieldPtr( Idx );
  if P = nil then
    Result := 0.0
  else
  case fBindings[ Idx ].wType of
  DBTYPE_R4:      Result := PSingle( P )^;
  DBTYPE_R8:      Result := PDouble( P )^;
  DBTYPE_CY:      Result := PInteger( P )^ * 0.0001;
  //TODO: DBTYPE_DECIMAL
  DBTYPE_NUMERIC, DBTYPE_VARNUMERIC:
    begin
      Result := Int64_2Double(GetFixupNumeric(Idx));
      if PDBNumeric(P).sign=0 then Result := 0 - Result;
      if PDBNumeric(P).scale<>0 then Result := Result * IntPower( 10, 0 - Shortint(PDBNumeric(P).scale));
    end;
  else            Result := 0.0;
  end;
end;

function TQuery.GetFixupNumeric(Idx: Integer): Int64;
var
  P: Pointer;
begin
  P := FieldPtr( Idx );
  Result := MakeInt64( 0, 0 );
  if P=nil then Exit;
  case fBindings[ Idx ].wType of
  DBTYPE_NUMERIC:
    Result := PInt64( DWORD(P)+3 )^; //131 filled with 00
  DBTYPE_VARNUMERIC:
    begin
      Result := PInt64( DWORD(P)+3 )^; //139 containing some shit bytes
      //vn := P;
      //if vn.precision> then
      //fix-up done in Fetchdata
    end;
  else
    Result := MakeInt64( PDWORD( DWORD(P)+3 )^, 0 );
  end;
end;

function TQuery.GetI64Field(Idx: Integer): Int64;
const His: array[ 0..1 ] of Integer = ( 0, -1 and not 255 );
var P: Pointer;
    B: Byte;
begin
  P := FieldPtr( Idx );
  Result := MakeInt64( 0, 0 );
  if P <> nil then
  case fBindings[ Idx ].wType of
  DBTYPE_I8, DBTYPE_UI8, DBTYPE_CY:
              Result := PInt64( P )^;
  DBTYPE_I1:
    begin
      B := PByte( P )^;
      Result := Int2Int64( Integer( B ) or His[ B shr 7 ] );
    end;
  DBTYPE_UI1: Result := MakeInt64( PByte( P )^, 0 );
  DBTYPE_I2:  Result := Int2Int64( PShortInt( P )^ );
  DBTYPE_UI2: Result := MakeInt64( PWord( P )^, 0 );
  DBTYPE_I4:  Result := Int2Int64( PInteger( P )^ );
  DBTYPE_NUMERIC, DBTYPE_VARNUMERIC:
    begin
      if ShortInt(PDBNumeric(P).scale)<>0 then
        Result := Double2Int64( RField[Idx] )
      else
        Result := GetFixupNumeric(Idx);
    end;
  //DBTYPE_UI4:
  else        Result := MakeInt64( PInteger( P )^, 0 );
  end;
end;

function TQuery.GetI64FldByName(const Name: String): Int64;
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  Result := LField[ Idx ];
end;

function TQuery.GetIFieldByName(const Name: String): Integer;
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  Result := IField[ Idx ];
end;

function TQuery.GetIntField(Idx: Integer): Integer;
var P: Pointer;
begin
  P := FieldPtr( Idx );
  if P = nil then
    Result := 0
  else
  case fBindings[ Idx ].wType of
  DBTYPE_I1:   begin
                 Result := PByte( P )^;
                 if LongBool( Result and $80) then
                   Result := Result or not $7F;
               end;
  DBTYPE_UI1:  Result := PByte( P )^;
  DBTYPE_I2, DBTYPE_UI2, DBTYPE_BOOL:   Result := PShortInt( P )^;
  DBTYPE_NUMERIC, DBTYPE_VARNUMERIC:
    begin
      if ShortInt(PDBNumeric(P).scale)<>0 then
        Result := Round( RField[Idx] )
      else
        Result := GetFixupNumeric(Idx).Lo;
    end;
  //DBTYPE_I4, DBTYPE_UI4, DBTYPE_HCHAPTER:
  else         Result := PInteger( P )^;
  end;
end;

function TQuery.GetIsNull(Idx: Integer): Boolean;
var P: PDWORD;
begin
  Result := TRUE;
  if (fRowSet = nil) or (fCurIndex < 0) or (DWORD(Idx) >= ColCount) then
    Exit;
  P := Pointer( DWORD( fRowBuffers.Items[ fCurIndex ] ) +
                   fBindings[ Idx ].obStatus );
  Result := P^ = DBSTATUS_S_ISNULL;
end;

function TQuery.GetIsNullByName(const Name: String): Boolean;
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  Result := IsNull[ Idx ];
end;

function TQuery.GetRawType(Idx: Integer): DWORD;
begin
  Result := 0;
  if fBindings = nil then Exit;
  Result := fBindings[ Idx ].wType;
end;

function TQuery.GetRawTypeByName(const Name: String): DWORD;
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  Result := RawType[ Idx ];
end;

function TQuery.GetRFieldByName(const Name: String): Double;
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  Result := RField[ Idx ];
end;

function TQuery.GetRowCount: Integer;
begin
  {if fRowsAffected = DB_S_ASYNCHRONOUS then
  begin
    // only for asynchronous connections - do not see now
  end;}
  Result := fRowsAffected;
end;

function TQuery.GetRowsKnown: Integer;
begin
  Result := fRowsAffected;
  if Result = 0 then
  if fRowBuffers <> nil then
    Result := fRowBuffers.Count;
end;

function TQuery.GetSFieldByName(const Name: String): String;
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  Result := SField[ Idx ];
end;

function TQuery.GetStrField(Idx: Integer): String;
var P: Pointer;
begin
  P := FieldPtr( Idx );
  if P = nil then
    Result := ''
  else
  if fBindings[ Idx ].wType = DBTYPE_STR then
    Result := PChar( P )
  else
    Result := PWideChar( P );
end;

procedure TQuery.Last;
begin
  while not EOF do
    Next; //WOFetch( 0 );
  if RowsKnown > 0 then
    fCurIndex := RowsKnown;
  Prev;
  //FetchData;
  fEOF := FALSE;
end;

procedure TQuery.MarkFieldChanged(Idx: Integer);
begin
  Changed( Idx );
end;

procedure TQuery.MarkFieldChangedByName(const Name: String);
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  MarkFieldChanged( Idx );
end;

procedure TQuery.Next;
begin
  NextWOFetch( 0 );
  FetchData;
end;

procedure TQuery.NextWOFetch( Skip: Integer );
var Obtained: UINT;
    PHandle: Pointer;
    hr: HResult;
begin
  ReleaseHandle;
  PHandle := @fRowHandle;
  if (fCurIndex = fRowsAffected) and (Skip = -2) then
    hr := fRowSet.GetNextRows( 0, -1, 1, Obtained, @PHandle )
  else
    hr := fRowSet.GetNextRows( 0, Skip, 1, Obtained, @PHandle );
  if hr <> DB_S_ENDOFROWSET then
    CheckOLE( hr );
  Inc( fCurIndex, Skip + 1 );
  if Obtained = 0 then
  begin
    fEOF := TRUE;
    if fRowBuffers <> nil then
      fRowsAffected := fRowBuffers.Count;
  end
    else
  begin
    if fRowBuffers = nil then
      fRowBuffers := NewList;
    if fCurIndex >= fRowBuffers.Count then
      fRowBuffers.Add( nil );
  end;
end;

procedure TQuery.Open;
const
  DB_NULLID: DBID = (uguid: (guid: (D1: 0; D2: 0; D3:0; D4: (0, 0, 0, 0, 0, 0, 0, 0)));
             ekind: 1 {DBKIND_GUID_PROPID}; uname: (ulpropid:0));

var ColInfo: IColumnsInfo;
    AccessorIntf: IAccessor;
    I: Integer;
    OK: Boolean;

    PropSets: array[0..0] of TDBPropset;
    Props: array[ 0..0 ] of TDBProp;
begin
  ClearRowset;
  if CheckOLE( fCommand.SetCommandText( @DBGUID_DBSQL, StringToOleStr( fText ) ) ) then
  begin
    if Mode = rmReadOnly then
    begin
      if not CheckOLE( fCommand.Execute( nil, IID_IROWSET, nil, @fRowsAffected, PIUnknown( @fRowSet ) ) ) then
        Exit;
    end
      else
    begin
      // Add by ECM !!!
      {$IFNDEF IBPROVIDER}
      if fCommandProps = nil then
      begin
        if CheckOLE( fCommand.QueryInterface( IID_ICommandProperties, fCommandProps ) ) then
        begin
          PropSets[0].rgProperties := @ Props[ 0 ];
          PropSets[0].cProperties := 1;
          PropSets[0].guidPropertySet := DBPROPSET_ROWSET;

          Props[0].dwPropertyID := $00000075; //DBPROP_UPDATABILITY
          Props[0].dwOptions := 0; //DBPROPOPTIONS_REQUIRED;
          Props[0].dwStatus := 0; //DBPROPSTATUS_OK;
          Props[0].colid := DB_NULLID;
          Props[0].vValue.vt := VT_I4;
          Props[0].vValue.lVal := 1; //DBPROPVAL_UP_CHANGE;
        end;
      end;
      CheckOLE( fCommandProps.SetProperties( 1, @ PropSets[ 0 ] ) );
      {$ENDIF}
      if not CheckOLE( fCommand.Execute( nil, IID_IROWSETCHANGE, nil, nil, PIUnknown( @ fRowSetChg ) ) ) then
        Exit;
      if not CheckOLE( fRowSetChg.QueryInterface( IID_IROWSET, fRowSet ) ) then
        Exit;
      if Mode = rmUpdateDelayed then
        CheckOLE( fRowSetChg.QueryInterface( IID_IROWSETUPDATE, fRowSetUpd ) );
    end;

    if fRowsAffected = 0 then
      Dec( fRowsAffected ); // RowCount = -1 means that RowCount is an unknown value
    if fRowSetChg <> nil then
    begin
      OK := CheckOLE( fRowSetChg.QueryInterface( IID_IColumnsInfo, ColInfo ) );
    end
      else
    begin
      OK := CheckOLE( fRowSet.QueryInterface( IID_IColumnsInfo, ColInfo ) );
    end;
    if OK then
    if CheckOLE( ColInfo.GetColumnInfo( fColCount, PDBColumnInfo( fColInfo ), fColNames ) ) then
    begin
      fBindings := AllocMem( Sizeof( TDBBinding ) * fColCount);
      for I := 0 to fColCount - 1 do
      begin
            fBindings[ I ].iOrdinal   := fColInfo[ I ].iOrdinal;
            fBindings[ I ].obValue    := fRowSize + 4;
        //  fBindings[ I ].obLength   := 0;
            fBindings[ I ].obStatus   := fRowSize;
        //  fBindings[ I ].pTypeInfo  := nil;
        //  fBindings[ I ].pObject    := nil;
        //  fBindings[ I ].pBindExt   := nil;
            fBindings[ I ].dwPart     := 1 + 4; //DBPART_VALUE + DBPART_STATUS;
        //  fBindings[ I ].dwMemOwner := 0; //DBMEMOWNER_CLIENTOWNED;
        //  fBindings[ I ].eParamIO   := 0; //DBPARAMIO_NOTPARAM;
            fBindings[ I ].cbMaxLen   := fColInfo[ I ].ulColumnSize;
            case fColInfo[ I ].wType of
            DBTYPE_BSTR: Inc( fBindings[ I ].cbMaxLen, 1 );
            DBTYPE_WSTR: fBindings[ I ].cbMaxLen := fBindings[ I ].cbMaxLen * 2  + 2;
            end;
            fBindings[ I ].cbMaxLen := (fBindings[ I ].cbMaxLen + 3) and not 3;
        //  fBindings[ I ].dwFlags    := 0;
            fBindings[ I ].wType      := fColInfo[ I ].wType;
            fBindings[ I ].bPrecision := fColInfo[ I ].bPrecision;
            fBindings[ I ].bScale     := fColInfo[ I ].bScale;
            Inc( fRowSize, fBindings[ I ].cbMaxLen + 4 );
      end;
      fBindStatus := AllocMem( Sizeof( DBBINDSTATUS ) * fColCount );
      if fRowSetChg <> nil then
      begin
        OK := CheckOLE( fRowSetChg.QueryInterface( IID_IAccessor, AccessorIntf ) );
      end
        else
      begin
        OK := CheckOLE( fRowSet.QueryInterface( IID_IAccessor, AccessorIntf ) );
      end;
      if OK then
        CheckOLE(
          AccessorIntf.CreateAccessor(
          2, //DBACCESSOR_ROWDATA, // Accessor will be used to retrieve row data
          fColCount,  // Number of columns being bound
          fBindings,  // Structure containing bind info
          0,          // Not used for row accessors
          fAccessor,  // Returned accessor handle
          PUIntArray( fBindStatus ) // Information about binding validity
          )
        );
      fEOF := FALSE;
      fCurIndex := -1;
      First;
    end;
  end;
end;

procedure TQuery.Post;
var R: HResult;
    {P: PChar;
    I: Integer;}
begin
  if not fChanged then Exit;
  if fRowSetChg = nil then Exit;
  R := fRowSetChg.SetData( fRowHandle, fAccessor, fRowBuffers.Items[ fCurIndex ] );
  if R <> HResult( $00040EDA {DB_S_ERRORSOCCURED} ) then
    CheckOLE( R )
  { // я вижу только статус DBSTATUS_E_INTEGRITYVIOLATION касательно 0-й колонки,
    // которую никто не просил добавлять во время выборки.
  else
  begin
    asm
      int 3
    end;
    for I := 0 to fColCount-1 do
    begin
      P := Pointer( DWORD( fRowBuffers.Items[ fCurIndex ] ) +
                   fBindings[ I ].obStatus );
      ShowMessage( fColInfo[I].pwszName + '.Status=' + Int2Hex( PDWORD( P )^, 8 ) );
    end;
  end};
  fChanged := FALSE;
end;

procedure TQuery.Prev;
begin
  if CurIndex > 0 then
  begin
    NextWOFetch( -2 ); //***
    //Dec( fCurIndex );
    fEOF := FALSE;
    FetchData; //***
  end;
end;

procedure TQuery.ReleaseHandle;
begin
  if fRowHandle <> 0 then
    CheckOLE( fRowSet.ReleaseRows( 1, @fRowHandle, nil, nil, nil ) );
  fRowHandle := 0;
end;

procedure TQuery.SetCurIndex(const Value: Integer);
var OldCurIndex: Integer;
begin
  OldCurIndex := fCurIndex;
  if fCurIndex = Value then
  begin
    if fRowHandle = 0 then
      FetchData;
    if fRowHandle <> 0 then
      Exit;
  end;
  if Value = 0 then
    First
  else
  if Value >= fRowsAffected - 1 then
    Last;

  fEOF := FALSE;
  while (fCurIndex < Value) and not EOF do
    Next;
  while (fCurIndex > Value) and not EOF do
    Prev;

  if fCurIndex = Value then
    FetchData
  else
    fCurIndex := OldCurIndex;
end;

procedure TQuery.SetDField(Idx: Integer; const Value: TDateTime);
var P: Pointer;
    ST: TSystemTime;
    pD: PDBDate;
    pT: PDBTime;
    TS: PDBTimeStamp;
    pFT: PFileTime;
begin
  P := FieldPtr( Idx );
  if P = nil then Exit;
  case fBindings[ Idx ].wType of
  DBTYPE_DATE: PDouble( P )^ := Value - VCLDate0;
  DBTYPE_DBDATE:
    begin
      pD := P;
      DateTime2SystemTime( Value, ST );
      pD.year := ST.wYear;
      pD.month := ST.wMonth;
      pD.day := ST.wDay;
    end;
  DBTYPE_DBTIME:
    begin
      pT := P;
      DateTime2SystemTime( Value, ST );
      pT.hour := ST.wHour;
      pT.minute := ST.wMinute;
      pT.second := ST.wSecond;
    end;
  DBTYPE_DBTIMESTAMP:
    begin
      TS := P;
      DateTime2SystemTime( Value, ST );
      TS.year := ST.wYear;
      TS.month := ST.wMonth;
      TS.day := ST.wDay;
      TS.hour := ST.wHour;
      TS.minute := ST.wMinute;
      TS.second := ST.wSecond;
      TS.fraction := ST.wMilliseconds * 1000;
    end;
  DBTYPE_FILETIME:
    begin
      pFT := P;
      DateTime2SystemTime( Value, ST );
      SystemTimeToFileTime( ST, pFT^ );
    end;
  else Exit;
  end;
  Changed( Idx );
end;

procedure TQuery.SetDFldByName(const Name: String; const Value: TDateTime);
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  DField[ Idx ] := Value;
end;

procedure TQuery.SetFByNameFromStr(const Name, Value: String);
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  FieldAsStr[ Idx ] := Value;
end;

procedure TQuery.SetFieldFromStr(Idx: Integer; const Value: String);
begin
  if StrEq( Value, '(null)' ) and (ColType[ Idx ] <> ftString) then
    IsNull[ Idx ] := TRUE
  else
  case ColType[ Idx ] of
  ftInteger:  IField[ Idx ] := Str2Int( Value );
  ftReal:     RField[ Idx ] := Str2Double( Value );
  ftString:   SField[ Idx ] := Value;
  ftDate:     DField[ Idx ] := Str2DateTimeShort( Value );
  end;
end;

procedure TQuery.SetFltField(Idx: Integer; const Value: Double);
var P: Pointer;
begin
  P := FieldPtr( Idx );
  if P = nil then
    Exit;
  case fBindings[ Idx ].wType of
  DBTYPE_R4:   PExtended( P )^ := Value;
  DBTYPE_R8:   PDouble( P )^ := Value;
  DBTYPE_CY:   PInteger( P )^ := Round( Value * 10000 );
  //TODO: DBTYPE_NUMERIC, DBTYPE_VARNUMERIC, DBTYPE_DECIMAL
  else         Exit;
  end;
  Changed( Idx );
end;

procedure TQuery.SetI64Field(Idx: Integer; const Value: Int64);
begin

end;

procedure TQuery.SetI64FldByName(const Name: String; const Value: Int64);
begin

end;

procedure TQuery.SetIFieldByName(const Name: String; Value: Integer);
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  IField[ Idx ] := Value;
end;

procedure TQuery.SetIntField(Idx: Integer; const Value: Integer);
var P: Pointer;
begin
  P := FieldPtr( Idx );
  if P = nil then
    Exit;
  case fBindings[ Idx ].wType of
  DBTYPE_I1, DBTYPE_UI1: PByte( P )^ := Byte( Value );
  DBTYPE_I2, DBTYPE_UI2: PShortInt( P )^ := Value;
  DBTYPE_BOOL: if Value <> 0 then PShortInt( P )^ := -1
                             else PShortInt( P )^ := 0;
  else PInteger( P )^ := Value;
  end;
  Changed( Idx );
end;

procedure TQuery.SetIsNull(Idx: Integer; const Value: Boolean);
var P: PDWORD;
begin
  if not Value then Exit;
  if (fRowSet = nil) or (fCurIndex < 0) or (DWORD(Idx) >= ColCount) then
    Exit;
  P := Changed( Idx );
  P^ := DBSTATUS_S_ISNULL;
end;

procedure TQuery.SetIsNullByName(const Name: String; const Value: Boolean);
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  IsNull[ Idx ] := Value;
end;

procedure TQuery.SetRFieldByName(const Name: String; const Value: Double);
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  RField[ Idx ] := Value;
end;

procedure TQuery.SetSFieldByName(const Name: String; const Value: String);
var Idx: Integer;
begin
  Idx := ColByName[ Name ];
  Assert( Idx >=0, 'TQuery: incorrect column name (' + Name + ').' );
  SField[ Idx ] := Value;
end;

procedure TQuery.SetStrField(Idx: Integer; const Value: String);
var P: Pointer;
begin
  P := FieldPtr( Idx );
  if P = nil then
    Exit;
  if fBindings[ Idx ].wType = DBTYPE_STR then
    StrLCopy( PChar( P ), @ Value[ 1 ], fBindings[ Idx ].cbMaxLen )
  else
    StringToWideChar( Value, PWideChar( P ), fBindings[ Idx ].cbMaxLen );
  Changed( Idx );
end;

procedure TQuery.SetText(const Value: String);
begin
  // clear here current rowset if any:
  ClearRowset;
  {// set txt to fCommand -- do this at the last moment just before execute
  CheckOLE( fCommand.SetCommandText( DBGUID_DBSQL, StringToOleStr( Value ) ) );}
  FText := Value;
end;

procedure TQuery.Update;
var Params, Results: array of DWORD;
    I: Integer;
begin
  if Mode <> rmUpdateDelayed then Exit;
  if (fDelList <> nil) and (fDelList.Count > 0) then
  begin
    SetLength( Params, fDelList.Count );
    SetLength( Results, fDelList.Count );
    for I := 0 to fDelList.Count-1 do
      Params[ I ] := DWORD( fDelList.Items[ I ] );
    CheckOLE( fRowSetUpd.DeleteRows( 0, fDelList.Count, @ Params[ 0 ], @ Results[ 0 ] ) );
    Free_And_Nil( fDelList );
  end;
  if fRowSetUpd = nil then Exit;
  CheckOLE( fRowSetUpd.Update( 0, 0, nil, nil, nil, nil ) );
end;

end.

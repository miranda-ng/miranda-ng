
{*******************************************************}
{                                                       }
{       Borland Delphi Runtime Library                  }
{       COM object support                              }
{                                                       }
{       Copyright (C) 1997,99 Inprise Corporation       }
{                                                       }
{*******************************************************}

{$IMPORTEDDATA ON}
unit KOLComObj;

{$G+}
{$DEFINE NOWARNINGS}
{$I KOLDEF.inc}

interface

uses Windows, ActiveX, KOL, err {$IFDEF _D6orHigher}, Variants {$ENDIF};

type
{ Forward declarations }

  TComObjectFactory = class;

{ COM server abstract base class }

  TComServerObject = class(TObject)
  protected
    function CountObject(Created: Boolean): Integer; virtual; abstract;
    function CountFactory(Created: Boolean): Integer; virtual; abstract;
    function GetHelpFileName: AnsiString; virtual; abstract;
    function GetServerFileName: AnsiString; virtual; abstract;
    function GetServerKey: AnsiString; virtual; abstract;
    function GetServerName: AnsiString; virtual; abstract;
    function GetStartSuspended: Boolean; virtual; abstract;
    function GetTypeLib: ITypeLib; virtual; abstract;
    procedure SetHelpFileName(const Value: AnsiString); virtual; abstract;
  public
    property HelpFileName: AnsiString read GetHelpFileName write SetHelpFileName;
    property ServerFileName: AnsiString read GetServerFileName;
    property ServerKey: AnsiString read GetServerKey;
    property ServerName: AnsiString read GetServerName;
    property TypeLib: ITypeLib read GetTypeLib;
    property StartSuspended: Boolean read GetStartSuspended;
  end;


{ TMultiReadExclusiveWriteSynchronizer minimizes thread serialization to gain
  read access to a resource shared among threads while still providing complete
  exclusivity to callers needing write access to the shared resource.
  (multithread shared reads, single thread exclusive write)
  Reading is allowed while owning a write lock.
  Read locks can be promoted to write locks.}

  {$IFNDEF _D2orD3}
  TActiveThreadRecord = record
    ThreadID: Integer;
    RecursionCount: Integer;
  end;
  TActiveThreadArray = array of TActiveThreadRecord;

  TMultiReadExclusiveWriteSynchronizer = class
  private
    FLock: TRTLCriticalSection;
    FReadExit: THandle;
    FCount: Integer;
    FSaveReadCount: Integer;
    FActiveThreads: TActiveThreadArray;
    FWriteRequestorID: Integer;
    FReallocFlag: Integer;
    FWriting: Boolean;
    function WriterIsOnlyReader: Boolean;
  public
    constructor Create;
    destructor Destroy; override;
    procedure BeginRead;
    procedure EndRead;
    procedure BeginWrite;
    procedure EndWrite;
  end;
  {$ENDIF}

{ COM class manager }

  TFactoryProc = procedure(Factory: TComObjectFactory) of object;

  TComClassManager = class(TObject)
  private
    FFactoryList: TComObjectFactory;
    {$IFNDEF _D2orD3}
    FLock: TMultiReadExclusiveWriteSynchronizer;
    {$ENDIF}
    procedure AddObjectFactory(Factory: TComObjectFactory);
    procedure RemoveObjectFactory(Factory: TComObjectFactory);
  public
    constructor Create;
    destructor Destroy; override;
    procedure ForEachFactory(ComServer: TComServerObject;
      FactoryProc: TFactoryProc);
    function GetFactoryFromClass(ComClass: TClass): TComObjectFactory;
    function GetFactoryFromClassID(const ClassID: TGUID): TComObjectFactory;
  end;

{ IServerExceptionHandler }
{ This interface allows you to report safecall exceptions that occur in a
  TComObject server to a third party, such as an object that logs errors into
  the system event log or a server monitor residing on another machine.
  Obtain an interface from the error logger implementation and assign it
  to your TComObject's ServerExceptionHandler property.  Each TComObject
  instance can have its own server exception handler, or all instances can
  share the same handler.  The server exception handler can override the
  TComObject's default exception handling by setting Handled to True and
  assigning an OLE HResult code to the HResult parameter.
}

  IServerExceptionHandler = interface
    ['{6A8D432B-EB81-11D1-AAB1-00C04FB16FBC}']
    procedure OnException(
      const ServerClass, ExceptionClass, ErrorMessage: WideString;
      ExceptAddr: Integer; const ErrorIID, ProgID: WideString;
      var Handled: Integer; var Result: HResult); dispid 2;
  end;

{ COM object }

  TComObject = class(TObject, IUnknown, ISupportErrorInfo)
  private
    FController: Pointer;
    FFactory: TComObjectFactory;
    FNonCountedObject: Boolean;
    FRefCount: Integer;
    FServerExceptionHandler: IServerExceptionHandler;
    function GetController: IUnknown;
  protected
    { IUnknown }
    function IUnknown.QueryInterface = ObjQueryInterface;
    function IUnknown._AddRef = ObjAddRef;
    function IUnknown._Release = ObjRelease;
    { IUnknown methods for other interfaces }
    function QueryInterface(const IID: TGUID; out Obj): HResult; stdcall;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
    { ISupportErrorInfo }
    function InterfaceSupportsErrorInfo(const iid: TIID): HResult; stdcall;
  public
    constructor Create;
    constructor CreateAggregated(const Controller: IUnknown);
    constructor CreateFromFactory(Factory: TComObjectFactory;
      const Controller: IUnknown);
    destructor Destroy; override;
    procedure Initialize; virtual;
    function ObjAddRef: Integer; virtual; stdcall;
    function ObjQueryInterface(const IID: TGUID; out Obj): HResult; virtual; stdcall;
    function ObjRelease: Integer; virtual; stdcall;
    function SafeCallException(ExceptObject: TObject;
      ExceptAddr: Pointer): HResult; override;
    property Controller: IUnknown read GetController;
    property Factory: TComObjectFactory read FFactory;
    property RefCount: Integer read FRefCount;
    property ServerExceptionHandler: IServerExceptionHandler
      read FServerExceptionHandler write FServerExceptionHandler;
  end;

{ COM class }

  TComClass = class of TComObject;

{ Instancing mode for COM classes }

  TClassInstancing = (ciInternal, ciSingleInstance, ciMultiInstance);

{ Threading model supported by COM classes }

  TThreadingModel = (tmSingle, tmApartment, tmFree, tmBoth);

{ COM object factory }

  {$IFDEF NOWARNINGS}
    {$WARNINGS OFF}
  {$ENDIF}
  TComObjectFactory = class(TObject, IUnknown, IClassFactory, IClassFactory2)
  private
    FNext: TComObjectFactory;
    FComServer: TComServerObject;
    FComClass: TClass;
    FClassID: TGUID;
    FClassName: AnsiString;
    FDescription: AnsiString;
    FErrorIID: TGUID;
    FInstancing: TClassInstancing;
    FLicString: WideString;
    FRegister: Longint;
    FShowErrors: Boolean;
    FSupportsLicensing: Boolean;
    FThreadingModel: TThreadingModel;
  protected
    function GetProgID: AnsiString; virtual;
    function GetLicenseString: WideString; virtual;
    function HasMachineLicense: Boolean; virtual;
    function ValidateUserLicense(const LicStr: WideString): Boolean; virtual;
    { IUnknown }
    function QueryInterface(const IID: TGUID; out Obj): HResult; stdcall;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
    { IClassFactory }
    function CreateInstance(const UnkOuter: IUnknown; const IID: TGUID;
      out Obj): HResult; stdcall;
    function LockServer(fLock: BOOL): HResult; stdcall;
    { IClassFactory2 }
    function GetLicInfo(var licInfo: TLicInfo): HResult; stdcall;
    function RequestLicKey(dwResrved: Longint; out bstrKey: WideString): HResult; stdcall;
    function CreateInstanceLic(const unkOuter: IUnknown; const unkReserved: IUnknown;
      const iid: TIID; const bstrKey: WideString; out vObject): HResult; stdcall;
  public
    constructor Create(ComServer: TComServerObject; ComClass: TComClass;
      const ClassID: TGUID; const ClassName, Description: AnsiString;
      Instancing: TClassInstancing; ThreadingModel: TThreadingModel {= tmSingle} );
    destructor Destroy; override;
    function CreateComObject(const Controller: IUnknown): TComObject; virtual;
    procedure RegisterClassObject;
    procedure UpdateRegistry(Register: Boolean); virtual;
    property ClassID: TGUID read FClassID;
    property ClassName: AnsiString read FClassName;
    property ComClass: TClass read FComClass;
    property ComServer: TComServerObject read FComServer;
    property Description: AnsiString read FDescription;
    property ErrorIID: TGUID read FErrorIID write FErrorIID;
    property LicString: WideString read FLicString write FLicString;
    property ProgID: AnsiString read GetProgID;
    property Instancing: TClassInstancing read FInstancing;
    property ShowErrors: Boolean read FShowErrors write FShowErrors;
    property SupportsLicensing: Boolean read FSupportsLicensing write FSupportsLicensing;
    property ThreadingModel: TThreadingModel read FThreadingModel;
  end;
  {$IFDEF NOWARNINGS}
    {$WARNINGS ON}
  {$ENDIF}

{ COM objects intended to be aggregated / contained }

  TAggregatedObject = class
  private
    FController: Pointer;
    function GetController: IUnknown;
  protected
    { IUnknown }
    function QueryInterface(const IID: TGUID; out Obj): HResult; stdcall;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
  public
    constructor Create(Controller: IUnknown);
    property Controller: IUnknown read GetController;
  end;

  TContainedObject = class(TAggregatedObject, IUnknown)
  protected
    { IUnknown }
    function QueryInterface(const IID: TGUID; out Obj): HResult; virtual; stdcall;
  end;

{ COM object with type information }

  TTypedComObject = class(TComObject, IProvideClassInfo)
  protected
    { IProvideClassInfo }
    function GetClassInfo(out TypeInfo: ITypeInfo): HResult; stdcall;
  end;

  TTypedComClass = class of TTypedComObject;

  {$IFDEF NOWARNINGS}
    {$WARNINGS OFF}
  {$ENDIF}
  TTypedComObjectFactory = class(TComObjectFactory)
  private
    FClassInfo: ITypeInfo;
  public
    constructor Create(ComServer: TComServerObject;
      TypedComClass: TTypedComClass; const ClassID: TGUID;
      Instancing: TClassInstancing; ThreadingModel: TThreadingModel {= tmSingle} );
    function GetInterfaceTypeInfo(TypeFlags: Integer): ITypeInfo;
    procedure UpdateRegistry(Register: Boolean); override;
    property ClassInfo: ITypeInfo read FClassInfo;
  end;
  {$IFDEF NOWARNINGS}
    {$WARNINGS ON}
  {$ENDIF}

{ OLE Automation object }

  TConnectEvent = procedure (const Sink: IUnknown; Connecting: Boolean) of object;

  TAutoObjectFactory = class;

  TAutoObject = class(TTypedComObject, IDispatch)
  private
    FEventSink: IUnknown;
    FAutoFactory: TAutoObjectFactory;
  protected
    { IDispatch }
    function GetIDsOfNames(const IID: TGUID; Names: Pointer;
      NameCount, LocaleID: Integer; DispIDs: Pointer): HResult; virtual; stdcall;
    function GetTypeInfo(Index, LocaleID: Integer; out TypeInfo): HResult; virtual; stdcall;
    function GetTypeInfoCount(out Count: Integer): HResult; virtual; stdcall;
    function Invoke(DispID: Integer; const IID: TGUID; LocaleID: Integer;
      Flags: Word; var Params; VarResult, ExcepInfo, ArgErr: Pointer): HResult; virtual; stdcall;
    { Other methods }
    procedure EventConnect(const Sink: IUnknown; Connecting: Boolean);
    procedure EventSinkChanged(const EventSink: IUnknown); virtual;
    property AutoFactory: TAutoObjectFactory read FAutoFactory;
    property EventSink: IUnknown read FEventSink write FEventSink;
  public
    procedure Initialize; override;
  end;

{ OLE Automation class }

  TAutoClass = class of TAutoObject;

{ OLE Automation object factory }

  TAutoObjectFactory = class(TTypedComObjectFactory)
  private
    FDispTypeInfo: ITypeInfo;
    FDispIntfEntry: PInterfaceEntry;
    FEventIID: TGUID;
    FEventTypeInfo: ITypeInfo;
  public
    constructor Create(ComServer: TComServerObject; AutoClass: TAutoClass;
      const ClassID: TGUID; Instancing: TClassInstancing;
      ThreadingModel: TThreadingModel {= tmSingle} );
    function GetIntfEntry(Guid: TGUID): PInterfaceEntry; virtual;
    property DispIntfEntry: PInterfaceEntry read FDispIntfEntry;
    property DispTypeInfo: ITypeInfo read FDispTypeInfo;
    property EventIID: TGUID read FEventIID;
    property EventTypeInfo: ITypeInfo read FEventTypeInfo;
  end;

  TAutoIntfObject = class(TInterfacedObject, IDispatch, ISupportErrorInfo)
  private
    FDispTypeInfo: ITypeInfo;
    FDispIntfEntry: PInterfaceEntry;
    FDispIID: TGUID;
  protected
    { IDispatch }
    function GetIDsOfNames(const IID: TGUID; Names: Pointer;
      NameCount, LocaleID: Integer; DispIDs: Pointer): HResult; stdcall;
    function GetTypeInfo(Index, LocaleID: Integer; out TypeInfo): HResult; stdcall;
    function GetTypeInfoCount(out Count: Integer): HResult; stdcall;
    function Invoke(DispID: Integer; const IID: TGUID; LocaleID: Integer;
      Flags: Word; var Params; VarResult, ExcepInfo, ArgErr: Pointer): HResult; stdcall;
    { ISupportErrorInfo }
    function InterfaceSupportsErrorInfo(const iid: TIID): HResult; stdcall;
  public
    constructor Create(const TypeLib: ITypeLib; const DispIntf: TGUID);
    function SafeCallException(ExceptObject: TObject;
      ExceptAddr: Pointer): HResult; override;
    property DispIntfEntry: PInterfaceEntry read FDispIntfEntry;
    property DispTypeInfo: ITypeInfo read FDispTypeInfo;
    property DispIID: TGUID read FDispIID;
  end;

{ OLE exception classes }

  EOleError = Exception; // class(Exception);

  EOleSysError = EOleError; { class(EOleError)
  private
    FErrorCode: HRESULT;
  public
    constructor Create(const Message: AnsiString; ErrorCode: HRESULT;
      HelpContext: Integer);
    property ErrorCode: HRESULT read FErrorCode write FErrorCode;
  end;}

  EOleException = EOleSysError; { class(EOleSysError)
  private
    FSource: AnsiString;
    FHelpFile: AnsiString;
  public
    constructor Create(const Message: AnsiString; ErrorCode: HRESULT;
      const Source, HelpFile: AnsiString; HelpContext: Integer);
    property HelpFile: AnsiString read FHelpFile write FHelpFile;
    property Source: AnsiString read FSource write FSource;
  end;}

  EOleRegistrationError = EOleError; { class(EOleError);}

  { Dispatch call descriptor }

  PCallDesc = ^TCallDesc;
  TCallDesc = packed record
    CallType: Byte;
    ArgCount: Byte;
    NamedArgCount: Byte;
    ArgTypes: array[0..255] of Byte;
  end;

  PDispDesc = ^TDispDesc;
  TDispDesc = packed record
    DispID: Integer;
    ResType: Byte;
    CallDesc: TCallDesc;
  end;

procedure DispatchInvoke(const Dispatch: IDispatch; CallDesc: PCallDesc;
  DispIDs: PDispIDList; Params: Pointer; Result: PVariant);
procedure DispatchInvokeError(Status: Integer; const ExcepInfo: TExcepInfo);

{function HandleSafeCallException(ExceptObject: TObject;
  ExceptAddr: Pointer; const ErrorIID: TGUID; const ProgID,
  HelpFileName: WideString): HResult;}

function CreateComObject(const ClassID: TGUID): IUnknown;
function CreateRemoteComObject(const MachineName: WideString; const ClassID: TGUID): IUnknown;
function CreateOleObject(const ClassName: AnsiString): IDispatch;
function GetActiveOleObject(const ClassName: AnsiString): IDispatch;

procedure OleError(ErrorCode: HResult);
procedure OleCheck(Result: HResult);

function StringToGUID(const S: AnsiString): TGUID;
function GUIDToString(const ClassID: TGUID): AnsiString;

function ProgIDToClassID(const ProgID: AnsiString): TGUID;
function ClassIDToProgID(const ClassID: TGUID): AnsiString;

procedure CreateRegKey(const Key, ValueName, Value: KOLstring);
procedure DeleteRegKey(const Key: KOLstring);
function GetRegStringValue(const Key, ValueName: KOLstring): KOLstring;

function StringToLPOLESTR(const Source: KOLstring): POleStr;

procedure RegisterComServer(const DLLName: KOLstring);
procedure RegisterAsService(const ClassID, ServiceName: KOLstring);

function CreateClassID: KOLstring;

procedure InterfaceConnect(const Source: IUnknown; const IID: TIID;
  const Sink: IUnknown; var Connection: Longint);
procedure InterfaceDisconnect(const Source: IUnknown; const IID: TIID;
  var Connection: Longint);

type
  TCoCreateInstanceExProc = function (const clsid: TCLSID;
    unkOuter: IUnknown; dwClsCtx: Longint; ServerInfo: PCoServerInfo;
    dwCount: Longint; rgmqResults: PMultiQIArray): HResult stdcall;
  TCoInitializeExProc = function (pvReserved: Pointer;
    coInit: Longint): HResult; stdcall;
  TCoAddRefServerProcessProc = function :Longint; stdcall;
  TCoReleaseServerProcessProc = function :Longint; stdcall;
  TCoResumeClassObjectsProc = function :HResult; stdcall;
  TCoSuspendClassObjectsProc = function :HResult; stdcall;

// COM functions that are only available on DCOM updated OSs
// These pointers may be nil on Win95 or Win NT 3.51 systems
var
  CoCreateInstanceEx: TCoCreateInstanceExProc = nil;
  CoInitializeEx: TCoInitializeExProc = nil;
  CoAddRefServerProcess: TCoAddRefServerProcessProc = nil;
  CoReleaseServerProcess: TCoReleaseServerProcessProc = nil;
  CoResumeClassObjects: TCoResumeClassObjectsProc = nil;
  CoSuspendClassObjects: TCoSuspendClassObjectsProc = nil;


{ CoInitFlags determines the COM threading model of the application or current
  thread. This bitflag value is passed to CoInitializeEx in ComServ initialization.
  Assign COINIT_APARTMENTTHREADED or COINIT_MULTITHREADED to this variable before
  Application.Initialize is called by the project source file to select a
  threading model.  Other CoInitializeEx flags (such as COINIT_SPEED_OVER_MEMORY)
  can be OR'd in also.  }
var
  CoInitFlags: Integer = -1;  // defaults to no threading model, call CoInitialize()

function ComClassManager: TComClassManager;

const
  GUID_NULL: TGUID = '{00000000-0000-0000-0000-000000000000}';

implementation

resourcestring
  SCreateRegKeyError = 'Error creating system registry entry';
  SOleError = 'OLE error %.8x';
  SObjectFactoryMissing = 'Object factory for class %s missing';
  STypeInfoMissing = 'Type information missing for class %s';
  SBadTypeInfo = 'Incorrect type information for class %s';
  SDispIntfMissing = 'Dispatch interface missing from class %s';
  SNoMethod = 'Method ''%s'' not supported by automation object';
  SVarNotObject = 'Variant does not reference an automation object';
  SDCOMNotInstalled = 'DCOM not installed';
  SDAXError = 'DAX Error';

  SAutomationWarning = 'COM Server Warning';
  SNoCloseActiveServer1 = 'There are still active COM objects in this ' +
    'application.  One or more clients may have references to these objects, ' +
    'so manually closing ';
  SNoCloseActiveServer2 = 'this application may cause those client ' +
    'application(s) to fail.'#13#10#13#10'Are you sure you want to close this ' +
    'application?';

var
  OleUninitializing: Boolean;

{ Handle a safe call exception }

{function HandleSafeCallException(ExceptObject: TObject;
  ExceptAddr: Pointer; const ErrorIID: TGUID; const ProgID,
  HelpFileName: WideString): HResult;
var
  E: TObject;
  CreateError: ICreateErrorInfo;
  ErrorInfo: IErrorInfo;
begin
  Result := E_UNEXPECTED;
  E := ExceptObject;
  if Succeeded(CreateErrorInfo(CreateError)) then
  begin
    CreateError.SetGUID(ErrorIID);
    if ProgID <> '' then CreateError.SetSource(PWideChar(ProgID));
    if HelpFileName <> '' then CreateError.SetHelpFile(PWideChar(HelpFileName));
    if E is Exception then
    begin
      CreateError.SetDescription(PWideChar(WideString(Exception(E).Message)));
      CreateError.SetHelpContext(Exception(E).HelpContext);
      if (E is EOleSysError) and (EOleSysError(E).ErrorCode < 0) then
        Result := EOleSysError(E).ErrorCode;
    end;
    if CreateError.QueryInterface(IErrorInfo, ErrorInfo) = S_OK then
      SetErrorInfo(0, ErrorInfo);
  end;
end;}

{ TDispatchSilencer }

type
  TDispatchSilencer = class(TInterfacedObject, IUnknown, IDispatch)
  private
    Dispatch: IDispatch;
    DispIntfIID: TGUID;
  public
    constructor Create(ADispatch: IUnknown; const ADispIntfIID: TGUID);
    { IUnknown }
    function QueryInterface(const IID: TGUID; out Obj): HResult; stdcall;
    { IDispatch }
    function GetTypeInfoCount(out Count: Integer): HResult; stdcall;
    function GetTypeInfo(Index, LocaleID: Integer; out TypeInfo): HResult; stdcall;
    function GetIDsOfNames(const IID: TGUID; Names: Pointer;
      NameCount, LocaleID: Integer; DispIDs: Pointer): HResult; stdcall;
    function Invoke(DispID: Integer; const IID: TGUID; LocaleID: Integer;
      Flags: Word; var Params; VarResult, ExcepInfo, ArgErr: Pointer): HResult; stdcall;
  end;

constructor TDispatchSilencer.Create(ADispatch: IUnknown;
  const ADispIntfIID: TGUID);
begin
  inherited Create;
  DispIntfIID := ADispIntfIID;
  OleCheck(ADispatch.QueryInterface(ADispIntfIID, Dispatch));
end;

function TDispatchSilencer.QueryInterface(const IID: TGUID; out Obj): HResult;
begin
  Result := inherited QueryInterface(IID, Obj);
  if Result = E_NOINTERFACE then
    if IsEqualGUID(IID, DispIntfIID) then
    begin
      IDispatch(Obj) := Self;
      Result := S_OK;
    end
    else
      Result := Dispatch.QueryInterface(IID, Obj);
end;

function TDispatchSilencer.GetTypeInfoCount(out Count: Integer): HResult;
begin
  Result := Dispatch.GetTypeInfoCount(Count);
end;

function TDispatchSilencer.GetTypeInfo(Index, LocaleID: Integer; out TypeInfo): HResult;
begin
  Result := Dispatch.GetTypeInfo(Index, LocaleID, TypeInfo);
end;

function TDispatchSilencer.GetIDsOfNames(const IID: TGUID; Names: Pointer;
  NameCount, LocaleID: Integer; DispIDs: Pointer): HResult;
begin
  Result := Dispatch.GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
end;

function TDispatchSilencer.Invoke(DispID: Integer; const IID: TGUID; LocaleID: Integer;
  Flags: Word; var Params; VarResult, ExcepInfo, ArgErr: Pointer): HResult;
begin
  { Ignore error since some containers, such as Internet Explorer 3.0x, will
    return error when the method was not handled, or scripting errors occur }
  Dispatch.Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo,
    ArgErr);
  Result := S_OK;
end;

{$IFNDEF _D2orD3}
{ TMultiReadExclusiveWriteSynchronizer }

constructor TMultiReadExclusiveWriteSynchronizer.Create;
begin
  inherited Create;
  InitializeCriticalSection(FLock);
  FReadExit := CreateEvent(nil, True, True, nil);  // manual reset, start signaled
  SetLength(FActiveThreads, 4);
end;

destructor TMultiReadExclusiveWriteSynchronizer.Destroy;
begin
  BeginWrite;
  inherited Destroy;
  CloseHandle(FReadExit);
  DeleteCriticalSection(FLock);
end;

function TMultiReadExclusiveWriteSynchronizer.WriterIsOnlyReader: Boolean;
var
  I, Len: Integer;
begin
  Result := False;
  if FWriteRequestorID = 0 then Exit;
  // We know a writer is waiting for entry with the FLock locked,
  // so FActiveThreads is stable - no BeginRead could be resizing it now
  I := 0;
  Len := High(FActiveThreads);
  while (I < Len) and
    ((FActiveThreads[I].ThreadID = 0) or (FActiveThreads[I].ThreadID = FWriteRequestorID)) do
    Inc(I);
  Result := I >= Len;
end;

procedure TMultiReadExclusiveWriteSynchronizer.BeginWrite;
begin
  EnterCriticalSection(FLock);  // Block new read or write ops from starting
  if not FWriting then
  begin
    FWriteRequestorID := GetCurrentThreadID;   // Indicate that writer is waiting for entry
    if not WriterIsOnlyReader then              // See if any other thread is reading
      WaitForSingleObject(FReadExit, INFINITE); // Wait for current readers to finish
    FSaveReadCount := FCount;  // record prior read recursions for this thread
    FCount := 0;
    FWriteRequestorID := 0;
    FWriting := True;
  end;
  Inc(FCount);  // allow read recursions during write without signalling FReadExit event
end;

procedure TMultiReadExclusiveWriteSynchronizer.EndWrite;
begin
  Dec(FCount);
  if FCount = 0 then
  begin
    FCount := FSaveReadCount;  // restore read recursion count
    FSaveReadCount := 0;
    FWriting := False;
  end;
  LeaveCriticalSection(FLock);
end;

procedure TMultiReadExclusiveWriteSynchronizer.BeginRead;
var
  I: Integer;
  ThreadID: Integer;
  ZeroSlot: Integer;
  AlreadyInRead: Boolean;
begin
  ThreadID := GetCurrentThreadID;
  // First, do a lightweight check to see if this thread already has a read lock
  while InterlockedExchange(FReallocFlag, ThreadID) <> 0 do  Sleep(0);
  try    // FActiveThreads array is now stable
    I := 0;
    while (I < High(FActiveThreads)) and (FActiveThreads[I].ThreadID <> ThreadID) do
      Inc(I);
    AlreadyInRead := I < High(FActiveThreads);
    if AlreadyInRead then  // This thread already has a read lock
    begin                   // Don't grab FLock, since that could deadlock with
      if not FWriting then  // a waiting BeginWrite
      begin                 // Bump up ref counts and exit
        InterlockedIncrement(FCount);
        Inc(FActiveThreads[I].RecursionCount); // thread safe = unique to threadid
      end;
    end
  finally
    FReallocFlag := 0;
  end;
  if not AlreadyInRead then
  begin   // Ok, we don't already have a lock, so do the hard work of making one
    EnterCriticalSection(FLock);
    try
      if not FWriting then
      begin
        // This will call ResetEvent more than necessary on win95, but still work
        if InterlockedIncrement(FCount) = 1 then
          ResetEvent(FReadExit); // Make writer wait until all readers are finished.
        I := 0;  // scan for empty slot in activethreads list
        ZeroSlot := -1;
        while (I < High(FActiveThreads)) and (FActiveThreads[I].ThreadID <> ThreadID) do
        begin
          if (FActiveThreads[I].ThreadID = 0) and (ZeroSlot < 0) then ZeroSlot := I;
          Inc(I);
        end;
        if I >= High(FActiveThreads) then  // didn't find our threadid slot
        begin
          if ZeroSlot < 0 then  // no slots available.  Grow array to make room
          begin   // spin loop.  wait for EndRead to put zero back into FReallocFlag
            while InterlockedExchange(FReallocFlag, ThreadID) <> 0 do  Sleep(0);
            try
              SetLength(FActiveThreads, High(FActiveThreads) + 3);
            finally
              FReallocFlag := 0;
            end;
          end
          else  // use an empty slot
            I := ZeroSlot;
          // no concurrency issue here.  We're the only thread interested in this record.
          FActiveThreads[I].ThreadID := ThreadID;
          FActiveThreads[I].RecursionCount := 1;
        end
        else  // found our threadid slot.
          Inc(FActiveThreads[I].RecursionCount); // thread safe = unique to threadid
      end;
    finally
      LeaveCriticalSection(FLock);
    end;
  end;
end;

procedure TMultiReadExclusiveWriteSynchronizer.EndRead;
var
  I, ThreadID, Len: Integer;
begin
  if not FWriting then
  begin
    // Remove our threadid from the list of active threads
    I := 0;
    ThreadID := GetCurrentThreadID;
    // wait for BeginRead to finish any pending realloc of FActiveThreads
    while InterlockedExchange(FReallocFlag, ThreadID) <> 0 do  Sleep(0);
    try
      Len := High(FActiveThreads);
      while (I < Len) and (FActiveThreads[I].ThreadID <> ThreadID) do Inc(I);
      assert(I < Len);
      // no concurrency issues here.  We're the only thread interested in this record.
      Dec(FActiveThreads[I].RecursionCount); // threadsafe = unique to threadid
      if FActiveThreads[I].RecursionCount = 0 then
        FActiveThreads[I].ThreadID := 0; // must do this last!
    finally
      FReallocFlag := 0;
    end;
    if (InterlockedDecrement(FCount) = 0) or WriterIsOnlyReader then
      SetEvent(FReadExit);     // release next writer
  end;
end;

procedure FreeAndNil(var Obj);
var
  P: TObject;
begin
  P := TObject(Obj);
  TObject(Obj) := nil;  // clear the reference before destroying the object
  P.Free;
end;
{$ENDIF}

{ TComClassManager }
constructor TComClassManager.Create;
begin
  inherited Create;
  {$IFNDEF _D2orD3}
  FLock := TMultiReadExclusiveWriteSynchronizer.Create;
  {$ENDIF}
end;

destructor TComClassManager.Destroy;
begin
  {$IFNDEF _D2orD3}
  FLock.Free;
  {$ENDIF}
  inherited Destroy;
end;

procedure TComClassManager.AddObjectFactory(Factory: TComObjectFactory);
begin
  {$IFNDEF _D2orD3}
  FLock.BeginWrite;
  try
  {$ENDIF}
    Factory.FNext := FFactoryList;
    FFactoryList := Factory;
  {$IFNDEF _D2orD3}
  finally
    FLock.EndWrite;
  end;
  {$ENDIF}
end;

procedure TComClassManager.ForEachFactory(ComServer: TComServerObject;
  FactoryProc: TFactoryProc);
var
  Factory, Next: TComObjectFactory;
begin
  {$IFNDEF _D2orD3}
  FLock.BeginWrite;  // FactoryProc could add or delete factories from list
  try
  {$ENDIF}
    Factory := FFactoryList;
    while Factory <> nil do
    begin
      Next := Factory.FNext;
      if Factory.ComServer = ComServer then FactoryProc(Factory);
      Factory := Next;
    end;
  {$IFNDEF _D2orD3}
  finally
    FLock.EndWrite;
  end;
  {$ENDIF}
end;

function TComClassManager.GetFactoryFromClass(ComClass: TClass): TComObjectFactory;
begin
  {$IFNDEF _D2orD3}
  FLock.BeginRead;
  try
  {$ENDIF}
    Result := FFactoryList;
    while Result <> nil do
    begin
      if Result.ComClass = ComClass then Exit;
      Result := Result.FNext;
    end;
    raise EOleError.CreateResFmt(e_Ole, Integer( @SObjectFactoryMissing ), [ComClass.ClassName]);
  {$IFNDEF _D2orD3}
  finally
    FLock.EndRead;
  end;
  {$ENDIF}
end;

function TComClassManager.GetFactoryFromClassID(const ClassID: TGUID): TComObjectFactory;
begin
  {$IFNDEF _D2orD3}
  FLock.BeginRead;
  try
  {$ENDIF}
    Result := FFactoryList;
    while Result <> nil do
    begin
      if IsEqualGUID(Result.ClassID, ClassID) then Exit;
      Result := Result.FNext;
    end;
  {$IFNDEF _D2orD3}
  finally
    FLock.EndRead;
  end;
  {$ENDIF}
end;

procedure TComClassManager.RemoveObjectFactory(Factory: TComObjectFactory);
var
  F, P: TComObjectFactory;
begin
  {$IFNDEF _D2orD3}
  FLock.BeginWrite;
  try
  {$ENDIF}
    P := nil;
    F := FFactoryList;
    while F <> nil do
    begin
      if F = Factory then
      begin
        if P <> nil then P.FNext := F.FNext else FFactoryList := F.FNext;
        Exit;
      end;
      P := F;
      F := F.FNext;
    end;
  {$IFNDEF _D2orD3}
  finally
    FLock.EndWrite;
  end;
  {$ENDIF}
end;

{ TComObject }

constructor TComObject.Create;
begin
  FNonCountedObject := True;
  CreateFromFactory(ComClassManager.GetFactoryFromClass(ClassType), nil);
end;

constructor TComObject.CreateAggregated(const Controller: IUnknown);
begin
  FNonCountedObject := True;
  CreateFromFactory(ComClassManager.GetFactoryFromClass(ClassType), Controller);
end;

constructor TComObject.CreateFromFactory(Factory: TComObjectFactory;
  const Controller: IUnknown);
begin
  FRefCount := 1;
  FFactory := Factory;
  FController := Pointer(Controller);
  if not FNonCountedObject then FFactory.ComServer.CountObject(True);
  Initialize;
  Dec(FRefCount);
end;

destructor TComObject.Destroy;
begin
  if not OleUninitializing then
  begin
    if (FFactory <> nil) and not FNonCountedObject then
      FFactory.ComServer.CountObject(False);
    if FRefCount > 0 then CoDisconnectObject(Self, 0);
  end;
end;

function TComObject.GetController: IUnknown;
begin
  Result := IUnknown(FController);
end;

procedure TComObject.Initialize;
begin
end;

function TComObject.SafeCallException(ExceptObject: TObject;
  ExceptAddr: Pointer): HResult;
var
  Msg: AnsiString;
  Handled: Integer;
begin
  Handled := 0;
  if ServerExceptionHandler <> nil then
  begin
    if ExceptObject is Exception then
      Msg := Exception(ExceptObject).Message;
    Result := 0;
    ServerExceptionHandler.OnException(ClassName,
      ExceptObject.ClassName, Msg, Integer(ExceptAddr),
      WideString(GUIDToString(FFactory.ErrorIID)),
      FFactory.ProgID, Handled, Result);
  end;
  if Handled = 0 then
    {Result := HandleSafeCallException(ExceptObject, ExceptAddr,
      FFactory.ErrorIID, FFactory.ProgID, FFactory.ComServer.HelpFileName);}
end;

{ TComObject.IUnknown }

function TComObject.ObjQueryInterface(const IID: TGUID; out Obj): HResult;
begin
  if GetInterface(IID, Obj) then Result := S_OK else Result := E_NOINTERFACE;
end;

function TComObject.ObjAddRef: Integer;
begin
  Result := InterlockedIncrement(FRefCount);
end;

function TComObject.ObjRelease: Integer;
begin
  // InterlockedDecrement returns only 0 or 1 on Win95 and NT 3.51
  // returns actual result on NT 4.0
  Result := InterlockedDecrement(FRefCount);
  if Result = 0 then Destroy;
end;

{ TComObject.IUnknown for other interfaces }

function TComObject.QueryInterface(const IID: TGUID; out Obj): HResult;
begin
  if FController <> nil then
    Result := IUnknown(FController).QueryInterface(IID, Obj) else
    Result := ObjQueryInterface(IID, Obj);
end;

function TComObject._AddRef: Integer;
begin
  if FController <> nil then
    Result := IUnknown(FController)._AddRef else
    Result := ObjAddRef;
end;

function TComObject._Release: Integer;
begin
  if FController <> nil then
    Result := IUnknown(FController)._Release else
    Result := ObjRelease;
end;

{ TComObject.ISupportErrorInfo }

function TComObject.InterfaceSupportsErrorInfo(const iid: TIID): HResult;
begin
  if GetInterfaceEntry(iid) <> nil then
    Result := S_OK else
    Result := S_FALSE;
end;

{ TComObjectFactory }

constructor TComObjectFactory.Create(ComServer: TComServerObject;
  ComClass: TComClass; const ClassID: TGUID; const ClassName,
  Description: AnsiString; Instancing: TClassInstancing;
  ThreadingModel: TThreadingModel);
begin
  IsMultiThread := IsMultiThread or (ThreadingModel <> tmSingle);
  if ThreadingModel in [tmFree, tmBoth] then
    CoInitFlags := COINIT_MULTITHREADED else
  if (ThreadingModel = tmApartment) and (CoInitFlags <> COINIT_MULTITHREADED) then
    CoInitFlags := COINIT_APARTMENTTHREADED;
  ComClassManager.AddObjectFactory(Self);
  FComServer := ComServer;
  FComClass := ComClass;
  FClassID := ClassID;
  FClassName := ClassName;
  FDescription := Description;
  FInstancing := Instancing;
  FErrorIID := IUnknown;
  FShowErrors := True;
  FThreadingModel := ThreadingModel;
  FRegister := -1;
end;

destructor TComObjectFactory.Destroy;
begin
  if FRegister <> -1 then CoRevokeClassObject(FRegister);
  ComClassManager.RemoveObjectFactory(Self);
end;

function TComObjectFactory.CreateComObject(const Controller: IUnknown): TComObject;
begin
  Result := TComClass(FComClass).CreateFromFactory(Self, Controller);
end;

function TComObjectFactory.GetProgID: AnsiString;
begin
  if FClassName <> '' then
    Result := FComServer.ServerName + '.' + FClassName else
    Result := '';
end;

procedure TComObjectFactory.RegisterClassObject;
const
  RegFlags: array[ciSingleInstance..ciMultiInstance] of Integer = (
    REGCLS_SINGLEUSE, REGCLS_MULTIPLEUSE);
  SuspendedFlag: array[Boolean] of Integer = (0, REGCLS_SUSPENDED);
begin
  if FInstancing <> ciInternal then
    OleCheck(CoRegisterClassObject(FClassID, Self, CLSCTX_LOCAL_SERVER,
      RegFlags[FInstancing] or SuspendedFlag[FComServer.StartSuspended], FRegister));
end;

procedure TComObjectFactory.UpdateRegistry(Register: Boolean);
const
  ThreadStrs: array[TThreadingModel] of AnsiString =
    ('', 'Apartment', 'Free', 'Both');
var
  ClassID, ProgID, ServerKeyName, ShortFileName: AnsiString;
begin
  if FInstancing = ciInternal then Exit;
  ClassID := GUIDToString(FClassID);
  ProgID := GetProgID;
  ServerKeyName := 'CLSID\' + ClassID + '\' + FComServer.ServerKey;
  if Register then
  begin
    CreateRegKey('CLSID\' + ClassID, '', Description);
    ShortFileName := FComServer.ServerFileName;
    if {Ansi}Pos(' ', ShortFileName) <> 0 then
      ShortFileName := ExtractShortPathName(ShortFileName);
    CreateRegKey(ServerKeyName, '', ShortFileName);
    if (FThreadingModel <> tmSingle) and IsLibrary then
      CreateRegKey(ServerKeyName, 'ThreadingModel', ThreadStrs[FThreadingModel]);
    if ProgID <> '' then
    begin
      CreateRegKey(ProgID, '', Description);
      CreateRegKey(ProgID + '\Clsid', '', ClassID);
      CreateRegKey('CLSID\' + ClassID + '\ProgID', '', ProgID);
    end;
  end else
  begin
    if ProgID <> '' then
    begin
      DeleteRegKey('CLSID\' + ClassID + '\ProgID');
      DeleteRegKey(ProgID + '\Clsid');
      DeleteRegKey(ProgID);
    end;
    DeleteRegKey(ServerKeyName);
    DeleteRegKey('CLSID\' + ClassID);
  end;
end;

function TComObjectFactory.GetLicenseString: WideString;
begin
  if FSupportsLicensing then Result := FLicString
  else Result := '';
end;

function TComObjectFactory.HasMachineLicense: Boolean;
begin
  Result := True;
end;

function TComObjectFactory.ValidateUserLicense(const LicStr: WideString): Boolean;
begin
  Result := AnsiCompareText(LicStr, FLicString) = 0;
end;

{ TComObjectFactory.IUnknown }

function TComObjectFactory.QueryInterface(const IID: TGUID; out Obj): HResult;
begin
  if GetInterface(IID, Obj) then Result := S_OK else Result := E_NOINTERFACE;
end;

function TComObjectFactory._AddRef: Integer;
begin
  Result := ComServer.CountFactory(True);
end;

function TComObjectFactory._Release: Integer;
begin
  Result := ComServer.CountFactory(False);
end;

{ TComObjectFactory.IClassFactory }

function TComObjectFactory.CreateInstance(const UnkOuter: IUnknown;
  const IID: TGUID; out Obj): HResult;
begin
  Result := CreateInstanceLic(UnkOuter, nil, IID, '', Obj);
end;

function TComObjectFactory.LockServer(fLock: BOOL): HResult;
begin
  Result := CoLockObjectExternal(Self, fLock, True);
  // Keep com server alive until this class factory is unlocked
  ComServer.CountObject(fLock);
end;

{ TComObjectFactory.IClassFactory2 }

function TComObjectFactory.GetLicInfo(var licInfo: TLicInfo): HResult;
begin
  Result := S_OK;
  try
    with licInfo do
    begin
      cbLicInfo := SizeOf(licInfo);
      fRuntimeKeyAvail := (not FSupportsLicensing) or (GetLicenseString <> '');
      fLicVerified := (not FSupportsLicensing) or HasMachineLicense;
    end;
  except
    Result := E_UNEXPECTED;
  end;
end;

function TComObjectFactory.RequestLicKey(dwResrved: Longint; out bstrKey: WideString): HResult;
begin
  // Can't give away a license key on an unlicensed machine
  if not HasMachineLicense then
  begin
    Result := CLASS_E_NOTLICENSED;
    Exit;
  end;
  bstrKey := FLicString;
  Result := NOERROR;
end;

function TComObjectFactory.CreateInstanceLic(const unkOuter: IUnknown;
  const unkReserved: IUnknown; const iid: TIID; const bstrKey: WideString;
  out vObject): HResult; stdcall;
var
  ComObject: TComObject;
begin
  // We can't write to a nil pointer.  Duh.
  if @vObject = nil then
  begin
    Result := E_POINTER;
    Exit;
  end;
  // In case of failure, make sure we return at least a nil interface.
  Pointer(vObject) := nil;
  // Check for licensing.
  if FSupportsLicensing and
    ((bstrKey <> '') and (not ValidateUserLicense(bstrKey))) or
    ((bstrKey = '') and (not HasMachineLicense)) then
  begin
    Result := CLASS_E_NOTLICENSED;
    Exit;
  end;
  // We can only aggregate if they are requesting our IUnknown.
  if (unkOuter <> nil) and not (IsEqualIID(iid, IUnknown)) then
  begin
    Result := CLASS_E_NOAGGREGATION;
    Exit;
  end;
  try
    ComObject := CreateComObject(UnkOuter);
  except
    if FShowErrors and (ExceptObject is Exception) then
      with Exception(ExceptObject) do
      begin
        {if (Message <> '') and (AnsiLastChar(Message) > '.') then
          Message := Message + '.';}
        MessageBox(0, PKOLChar(Message), PKOLChar(KOLString( SDAXError )), MB_OK or MB_ICONSTOP or
          MB_SETFOREGROUND);
      end;
    Result := E_UNEXPECTED;
    Exit;
  end;
  Result := ComObject.ObjQueryInterface(IID, vObject);
  if ComObject.RefCount = 0 then ComObject.Free;
end;

{ TAggregatedObject }

constructor TAggregatedObject.Create(Controller: IUnknown);
begin
  FController := Pointer(Controller);
end;

function TAggregatedObject.GetController: IUnknown;
begin
  Result := IUnknown(FController);
end;

{ TAggregatedObject.IUnknown }

function TAggregatedObject.QueryInterface(const IID: TGUID; out Obj): HResult;
begin
  Result := IUnknown(FController).QueryInterface(IID, Obj);
end;

function TAggregatedObject._AddRef: Integer;
begin
  Result := IUnknown(FController)._AddRef;
end;

function TAggregatedObject._Release: Integer; stdcall;
begin
  Result := IUnknown(FController)._Release;
end;

{ TContainedObject.IUnknown }

function TContainedObject.QueryInterface(const IID: TGUID; out Obj): HResult;
begin
  if GetInterface(IID, Obj) then Result := S_OK else Result := E_NOINTERFACE;
end;

{ TTypedComObject.IProvideClassInfo }

function TTypedComObject.GetClassInfo(out TypeInfo: ITypeInfo): HResult;
begin
  TypeInfo := TTypedComObjectFactory(FFactory).FClassInfo;
  Result := S_OK;
end;

{ TTypedComObjectFactory }

constructor TTypedComObjectFactory.Create(ComServer: TComServerObject;
  TypedComClass: TTypedComClass; const ClassID: TGUID;
  Instancing: TClassInstancing; ThreadingModel: TThreadingModel);
var
  ClassName, Description: WideString;
begin
  if ComServer.TypeLib.GetTypeInfoOfGUID(ClassID, FClassInfo) <> S_OK then
    raise EOleError.CreateResFmt(e_Ole, Integer(@STypeInfoMissing), [TypedComClass.ClassName]);
  OleCheck(FClassInfo.GetDocumentation(MEMBERID_NIL, @ClassName,
    @Description, nil, nil));
  inherited Create(ComServer, TypedComClass, ClassID,
    ClassName, Description, Instancing, ThreadingModel);
end;

function TTypedComObjectFactory.GetInterfaceTypeInfo(
  TypeFlags: Integer): ITypeInfo;
const
  FlagsMask = IMPLTYPEFLAG_FDEFAULT or IMPLTYPEFLAG_FSOURCE;
var
  ClassAttr: PTypeAttr;
  I, TypeInfoCount, Flags: Integer;
  RefType: HRefType;
begin
  OleCheck(FClassInfo.GetTypeAttr(ClassAttr));
  TypeInfoCount := ClassAttr^.cImplTypes;
  ClassInfo.ReleaseTypeAttr(ClassAttr);
  for I := 0 to TypeInfoCount - 1 do
  begin
    OleCheck(ClassInfo.GetImplTypeFlags(I, Flags));
    if Flags and FlagsMask = TypeFlags then
    begin
      OleCheck(ClassInfo.GetRefTypeOfImplType(I, RefType));
      OleCheck(ClassInfo.GetRefTypeInfo(RefType, Result));
      Exit;
    end;
  end;
  Result := nil;
end;

procedure TTypedComObjectFactory.UpdateRegistry(Register: Boolean);
var
  ClassKey: AnsiString;
  TypeLib: ITypeLib;
  TLibAttr: PTLibAttr;
begin
  ClassKey := 'CLSID\' + GUIDToString(FClassID);
  if Register then
  begin
    inherited UpdateRegistry(Register);
    TypeLib := FComServer.TypeLib;
    OleCheck(TypeLib.GetLibAttr(TLibAttr));
    try
      CreateRegKey(ClassKey + '\Version', '', Format('%d.%d',
        [TLibAttr.wMajorVerNum, TLibAttr.wMinorVerNum]));
      CreateRegKey(ClassKey + '\TypeLib', '', GUIDToString(TLibAttr.guid));
    finally
      TypeLib.ReleaseTLibAttr(TLibAttr);
    end;
  end else
  begin
    DeleteRegKey(ClassKey + '\TypeLib');
    DeleteRegKey(ClassKey + '\Version');
    inherited UpdateRegistry(Register);
  end;
end;

{ TAutoObject }

procedure TAutoObject.EventConnect(const Sink: IUnknown;
  Connecting: Boolean);
begin
  if Connecting then
  begin
    OleCheck(Sink.QueryInterface(FAutoFactory.FEventIID, FEventSink));
    EventSinkChanged(TDispatchSilencer.Create(Sink, FAutoFactory.FEventIID));
  end
  else
  begin
    FEventSink := nil;
    EventSinkChanged(nil);
  end;
end;

procedure TAutoObject.EventSinkChanged(const EventSink: IUnknown);
begin
end;

procedure TAutoObject.Initialize;
begin
  FAutoFactory := Factory as TAutoObjectFactory;
  inherited Initialize;
end;

{ TAutoObject.IDispatch }

function TAutoObject.GetIDsOfNames(const IID: TGUID; Names: Pointer;
  NameCount, LocaleID: Integer; DispIDs: Pointer): HResult;
begin
  Result := DispGetIDsOfNames(FAutoFactory.DispTypeInfo,
    Names, NameCount, DispIDs);
end;

function TAutoObject.GetTypeInfo(Index, LocaleID: Integer;
  out TypeInfo): HResult;
begin
  Pointer(TypeInfo) := nil;
  if Index <> 0 then
  begin
    Result := DISP_E_BADINDEX;
    Exit;
  end;
  ITypeInfo(TypeInfo) := TAutoObjectFactory(Factory).DispTypeInfo;
  Result := S_OK;
end;

function TAutoObject.GetTypeInfoCount(out Count: Integer): HResult;
begin
  Count := 1;
  Result := S_OK;
end;

function TAutoObject.Invoke(DispID: Integer; const IID: TGUID; LocaleID: Integer;
  Flags: Word; var Params; VarResult, ExcepInfo, ArgErr: Pointer): HResult;
const
  INVOKE_PROPERTYSET = INVOKE_PROPERTYPUT or INVOKE_PROPERTYPUTREF;
begin
  if Flags and INVOKE_PROPERTYSET <> 0 then Flags := INVOKE_PROPERTYSET;
  Result := TAutoObjectFactory(Factory).DispTypeInfo.Invoke(Pointer(
    Integer(Self) + TAutoObjectFactory(Factory).DispIntfEntry.IOffset),
    DispID, Flags, TDispParams(Params), VarResult, ExcepInfo, ArgErr);
end;

{ TAutoObjectFactory }

constructor TAutoObjectFactory.Create(ComServer: TComServerObject;
  AutoClass: TAutoClass; const ClassID: TGUID;
  Instancing: TClassInstancing; ThreadingModel: TThreadingModel);
var
  TypeAttr: PTypeAttr;
begin
  inherited Create(ComServer, AutoClass, ClassID, Instancing, ThreadingModel);
  FDispTypeInfo := GetInterfaceTypeInfo(IMPLTYPEFLAG_FDEFAULT);
  if FDispTypeInfo = nil then
    raise EOleError.CreateResFmt(e_Ole, Integer(@SBadTypeInfo), [AutoClass.ClassName]);
  OleCheck(FDispTypeInfo.GetTypeAttr(TypeAttr));
  FDispIntfEntry := GetIntfEntry(TypeAttr^.guid);
  FDispTypeInfo.ReleaseTypeAttr(TypeAttr);
  if FDispIntfEntry = nil then
    raise EOleError.CreateResFmt(e_Ole, Integer(@SDispIntfMissing),
          [AutoClass.ClassName]);
  FErrorIID := FDispIntfEntry^.IID;
  FEventTypeInfo := GetInterfaceTypeInfo(IMPLTYPEFLAG_FDEFAULT or
    IMPLTYPEFLAG_FSOURCE);
  if FEventTypeInfo <> nil then
  begin
    OleCheck(FEventTypeInfo.GetTypeAttr(TypeAttr));
    FEventIID := TypeAttr.guid;
    FEventTypeInfo.ReleaseTypeAttr(TypeAttr);
  end;
end;

function TAutoObjectFactory.GetIntfEntry(Guid: TGUID): PInterfaceEntry;
begin
  Result := FComClass.GetInterfaceEntry(Guid);
end;

{ TAutoIntfObject }

constructor TAutoIntfObject.Create(const TypeLib: ITypeLib; const DispIntf: TGUID);
begin
  inherited Create;
  OleCheck(TypeLib.GetTypeInfoOfGuid(DispIntf, FDispTypeInfo));
  FDispIntfEntry := GetInterfaceEntry(DispIntf);
end;

{ TAutoIntfObject.IDispatch }

function TAutoIntfObject.GetIDsOfNames(const IID: TGUID; Names: Pointer;
  NameCount, LocaleID: Integer; DispIDs: Pointer): HResult;
begin
  Result := DispGetIDsOfNames(FDispTypeInfo, Names, NameCount, DispIDs);
end;

function TAutoIntfObject.GetTypeInfo(Index, LocaleID: Integer;
  out TypeInfo): HResult;
begin
  Pointer(TypeInfo) := nil;
  if Index <> 0 then
  begin
    Result := DISP_E_BADINDEX;
    Exit;
  end;
  ITypeInfo(TypeInfo) := FDispTypeInfo;
  Result := S_OK;
end;

function TAutoIntfObject.GetTypeInfoCount(out Count: Integer): HResult;
begin
  Count := 1;
  Result := S_OK;
end;

function TAutoIntfObject.Invoke(DispID: Integer; const IID: TGUID;
  LocaleID: Integer; Flags: Word; var Params; VarResult, ExcepInfo,
  ArgErr: Pointer): HResult;
const
  INVOKE_PROPERTYSET = INVOKE_PROPERTYPUT or INVOKE_PROPERTYPUTREF;
begin
  if Flags and INVOKE_PROPERTYSET <> 0 then Flags := INVOKE_PROPERTYSET;
  Result := FDispTypeInfo.Invoke(Pointer(Integer(Self) +
    FDispIntfEntry.IOffset), DispID, Flags, TDispParams(Params), VarResult,
    ExcepInfo, ArgErr);
end;

function TAutoIntfObject.InterfaceSupportsErrorInfo(const iid: TIID): HResult;
begin
  if IsEqualGUID(DispIID, iid) then
    Result := S_OK else
    Result := S_FALSE;
end;

function TAutoIntfObject.SafeCallException(ExceptObject: TObject;
  ExceptAddr: Pointer): HResult;
begin
  Result := 0; { HandleSafeCallException(ExceptObject, ExceptAddr, DispIID, '', ''); }
end;

const
{ Maximum number of dispatch arguments }

  MaxDispArgs = 64; {!!!}

{ Special variant type codes }

  varStrArg = $0048;

{ Parameter type masks }

  atVarMask  = $3F;
  atTypeMask = $7F;
  atByRef    = $80;

{function TrimPunctuation(const S: AnsiString): AnsiString;
var
  P: PChar;
begin
  Result := S;
  P := AnsiLastChar(Result);
  while (Length(Result) > 0) and (P^ in [#0..#32, '.']) do
  begin
    SetLength(Result, P - PChar(Result));
    P := AnsiLastChar(Result);
  end;
end;}

{ EOleSysError }

{constructor EOleSysError.Create(const Message: AnsiString;
  ErrorCode: HRESULT; HelpContext: Integer);
var
  S: AnsiString;
begin
  S := Message;
  if S = '' then
  begin
    S := SysErrorMessage(ErrorCode);
    if S = '' then FmtStr(S, SOleError, [ErrorCode]);
  end;
  inherited CreateHelp(S, HelpContext);
  FErrorCode := ErrorCode;
end;}

{ EOleException }

{constructor EOleException.Create(const Message: AnsiString; ErrorCode: HRESULT;
  const Source, HelpFile: AnsiString; HelpContext: Integer);
begin
  inherited Create(TrimPunctuation(Message), ErrorCode, HelpContext);
  FSource := Source;
  FHelpFile := HelpFile;
end;}


{ Raise EOleSysError exception from an error code }

procedure OleError(ErrorCode: HResult);
begin
  raise EOleSysError.Create(e_Ole, 'OLE error: ' + Int2Str( ErrorCode ) );
end;

{ Raise EOleSysError exception if result code indicates an error }

procedure OleCheck(Result: HResult);
begin
  if not Succeeded(Result) then OleError(Result);
end;

{ Convert a AnsiString to a GUID }

function StringToGUID(const S: AnsiString): TGUID;
begin
  OleCheck(CLSIDFromString(PWideChar(WideString(S)), Result));
end;

{ Convert a GUID to a AnsiString }

function GUIDToString(const ClassID: TGUID): AnsiString;
var
  P: PWideChar;
begin
  OleCheck(StringFromCLSID(ClassID, P));
  Result := P;
  CoTaskMemFree(P);
end;

{ Convert a programmatic ID to a class ID }

function ProgIDToClassID(const ProgID: AnsiString): TGUID;
begin
  OleCheck(CLSIDFromProgID(PWideChar(WideString(ProgID)), Result));
end;

{ Convert a class ID to a programmatic ID }

function ClassIDToProgID(const ClassID: TGUID): AnsiString;
var
  P: PWideChar;
begin
  OleCheck(ProgIDFromCLSID(ClassID, P));
  Result := P;
  CoTaskMemFree(P);
end;

{ Create registry key }

procedure CreateRegKey(const Key, ValueName, Value: KOLstring);
var
  Handle: HKey;
  Status, Disposition: Integer;
begin
  Status := RegCreateKeyEx(HKEY_CLASSES_ROOT, PKOLChar(Key), 0, '',
    REG_OPTION_NON_VOLATILE, KEY_READ or KEY_WRITE, nil, Handle,
    @Disposition);
  if Status = 0 then
  begin
    Status := RegSetValueEx(Handle, PKOLChar(ValueName), 0, REG_SZ,
      PKOLChar(Value), Length(Value) + 1);
    RegCloseKey(Handle);
  end;
  if Status <> 0 then raise EOleRegistrationError.CreateResFmt(e_Registry,
                            Integer(@SCreateRegKeyError), [ nil ] );
end;

{ Delete registry key }

procedure DeleteRegKey(const Key: KOLstring);
begin
  RegDeleteKey(HKEY_CLASSES_ROOT, PKOLChar(Key));
end;

{ Get registry value }

function GetRegStringValue(const Key, ValueName: KOLstring): KOLstring;
var
  Size: DWord;
  RegKey: HKEY;
begin
  Result := '';
  if RegOpenKey(HKEY_CLASSES_ROOT, PKOLChar(Key), RegKey) = ERROR_SUCCESS then
  try
    Size := 256;
    SetLength(Result, Size);
    if RegQueryValueEx(RegKey, PKOLChar(ValueName), nil, nil, PByte(PKOLChar(Result)), @Size) = ERROR_SUCCESS then
      SetLength(Result, Size - 1) else
      Result := '';
  finally
    RegCloseKey(RegKey);
  end;
end;

function CreateComObject(const ClassID: TGUID): IUnknown;
begin
  OleCheck(CoCreateInstance(ClassID, nil, CLSCTX_INPROC_SERVER or
    CLSCTX_LOCAL_SERVER, IUnknown, Result));
end;

function CreateRemoteComObject(const MachineName: WideString;
  const ClassID: TGUID): IUnknown;
const
  LocalFlags = CLSCTX_LOCAL_SERVER or CLSCTX_REMOTE_SERVER or CLSCTX_INPROC_SERVER;
  RemoteFlags = CLSCTX_REMOTE_SERVER;
var
  MQI: TMultiQI;
  ServerInfo: TCoServerInfo;
  IID_IUnknown: TGuid;
  Flags, Size: DWORD;
  LocalMachine: array [0..MAX_COMPUTERNAME_LENGTH] of KOLchar;
begin
  if @CoCreateInstanceEx = nil then
    raise Exception.CreateResFmt(e_Com, Integer(@SDCOMNotInstalled), [nil]);
  FillChar(ServerInfo, sizeof(ServerInfo), 0);
  ServerInfo.pwszName := PWideChar(MachineName);
  IID_IUnknown := IUnknown;
  MQI.IID := @IID_IUnknown;
  MQI.itf := nil;
  MQI.hr := 0;
  { If a MachineName is specified check to see if it the local machine.
    If it isn't, do not allow LocalServers to be used. }
  if Length(MachineName) > 0 then
  begin
    Size := Sizeof(LocalMachine);  // Win95 is hypersensitive to size
    if GetComputerName(LocalMachine, Size) and
       (AnsiCompareText(LocalMachine, MachineName) = 0) then
      Flags := LocalFlags else
      Flags := RemoteFlags;
  end else
    Flags := LocalFlags;
  OleCheck(CoCreateInstanceEx(ClassID, nil, Flags, @ServerInfo, 1, @MQI));
  OleCheck(MQI.HR);
  Result := MQI.itf;
end;

function CreateOleObject(const ClassName: AnsiString): IDispatch;
var
  ClassID: TCLSID;
begin
  ClassID := ProgIDToClassID(ClassName);
  OleCheck(CoCreateInstance(ClassID, nil, CLSCTX_INPROC_SERVER or
    CLSCTX_LOCAL_SERVER, IDispatch, Result));
end;

function GetActiveOleObject(const ClassName: AnsiString): IDispatch;
var
  ClassID: TCLSID;
  Unknown: IUnknown;
begin
  ClassID := ProgIDToClassID(ClassName);
  OleCheck(GetActiveObject(ClassID, nil, Unknown));
  OleCheck(Unknown.QueryInterface(IDispatch, Result));
end;

function StringToLPOLESTR(const Source: KOLstring): POleStr;
var
  SourceLen: Integer;
  Buffer: PWideChar;
begin
  SourceLen := Length(Source);
  Buffer  := CoTaskMemAlloc((SourceLen+1) * sizeof(WideChar));
  StringToWideChar( Source, Buffer, SourceLen+1 );
  Result := POleStr( Buffer );
end;

function CreateClassID: KOLstring;
var
  ClassID: TCLSID;
  P: PWideChar;
begin
  CoCreateGuid(ClassID);
  StringFromCLSID(ClassID, P);
  Result := P;
  CoTaskMemFree(P);
end;

procedure RegisterComServer(const DLLName: KOLstring);
type
  TRegProc = function: HResult; stdcall;
const
  RegProcName = 'DllRegisterServer'; { Do not localize }
var
  Handle: THandle;
  RegProc: TRegProc;
begin
  {$IFDEF _D2orD3}
  Handle := LoadLibrary( PChar( DLLName ) );
  {$ELSE}
  Handle := SafeLoadLibrary(DLLName);
  {$ENDIF}
  if Handle <= HINSTANCE_ERROR then
    raise Exception.CreateFmt( e_Com, '%s: %s', [SysErrorMessage(GetLastError), DLLName]);
  try
    RegProc := GetProcAddress(Handle, RegProcName);
    if Assigned(RegProc) then OleCheck(RegProc) else RaiseLastWin32Error;
  finally
    FreeLibrary(Handle);
  end;
end;

procedure RegisterAsService(const ClassID, ServiceName: KOLstring);
begin
  CreateRegKey('AppID\' + ClassID, 'LocalService', ServiceName);
  CreateRegKey('CLSID\' + ClassID, 'AppID', ClassID);
end;

{ Connect an IConnectionPoint interface }

procedure InterfaceConnect(const Source: IUnknown; const IID: TIID;
  const Sink: IUnknown; var Connection: Longint);
var
  CPC: IConnectionPointContainer;
  CP: IConnectionPoint;
begin
  Connection := 0;
  if Succeeded(Source.QueryInterface(IConnectionPointContainer, CPC)) then
    if Succeeded(CPC.FindConnectionPoint(IID, CP)) then
      CP.Advise(Sink, Connection);
end;

{ Disconnect an IConnectionPoint interface }

procedure InterfaceDisconnect(const Source: IUnknown; const IID: TIID;
  var Connection: Longint);
var
  CPC: IConnectionPointContainer;
  CP: IConnectionPoint;
begin
  if Connection <> 0 then
    if Succeeded(Source.QueryInterface(IConnectionPointContainer, CPC)) then
      if Succeeded(CPC.FindConnectionPoint(IID, CP)) then
        if Succeeded(CP.Unadvise(Connection)) then Connection := 0;
end;

procedure LoadComExProcs;
var
  Ole32: HModule;
begin
  Ole32 := GetModuleHandle('ole32.dll');
  if Ole32 <> 0 then
  begin
    @CoCreateInstanceEx := GetProcAddress(Ole32, 'CoCreateInstanceEx');
    @CoInitializeEx := GetProcAddress(Ole32, 'CoInitializeEx');
    @CoAddRefServerProcess := GetProcAddress(Ole32, 'CoAddRefServerProcess');
    @CoReleaseServerProcess := GetProcAddress(Ole32, 'CoReleaseServerProcess');
    @CoResumeClassObjects := GetProcAddress(Ole32, 'CoResumeClassObjects');
    @CoSuspendClassObjects := GetProcAddress(Ole32, 'CoSuspendClassObjects');
  end;
end;

procedure SafeCallError(ErrorCode: Integer; ErrorAddr: Pointer);
var
  ErrorInfo: IErrorInfo;
  Source, Description, HelpFile: WideString;
  HelpContext: Longint;
begin
  HelpContext := 0;
  if GetErrorInfo(0, ErrorInfo) = S_OK then
  begin
    ErrorInfo.GetSource(Source);
    ErrorInfo.GetDescription(Description);
    ErrorInfo.GetHelpFile(HelpFile);
    ErrorInfo.GetHelpContext(HelpContext);
  end;
  raise EOleException.Create(e_Ole, Description + Int2Str( ErrorCode ) {, Source,
    HelpFile, HelpContext} ) at ErrorAddr;
end;

{ Call Invoke method on the given IDispatch interface using the given
  call descriptor, dispatch IDs, parameters, and result }

procedure DispatchInvoke(const Dispatch: IDispatch; CallDesc: PCallDesc;
  DispIDs: PDispIDList; Params: Pointer; Result: PVariant);
type
  PVarArg = ^TVarArg;
  TVarArg = array[0..3] of DWORD;
  TStringDesc = record
    BStr: PWideChar;
    PStr: pAnsiString;
  end;
var
  I, J, K, ArgType, ArgCount, StrCount, DispID, InvKind, Status: Integer;
  VarFlag: Byte;
  ParamPtr: ^Integer;
  ArgPtr, VarPtr: PVarArg;
  DispParams: TDispParams;
  ExcepInfo: TExcepInfo;
  Strings: array[0..MaxDispArgs - 1] of TStringDesc;
  Args: array[0..MaxDispArgs - 1] of TVarArg;
begin
  StrCount := 0;
  try
    ArgCount := CallDesc^.ArgCount;
    if ArgCount <> 0 then
    begin
      ParamPtr := Params;
      ArgPtr := @Args[ArgCount];
      I := 0;
      repeat
        Dec(Integer(ArgPtr), SizeOf(TVarData));
        ArgType := CallDesc^.ArgTypes[I] and atTypeMask;
        VarFlag := CallDesc^.ArgTypes[I] and atByRef;
        if ArgType = varError then
        begin
          ArgPtr^[0] := varError;
          ArgPtr^[2] := DWORD(DISP_E_PARAMNOTFOUND);
        end else
        begin
          if ArgType = varStrArg then
          begin
            with Strings[StrCount] do
              if VarFlag <> 0 then
              begin
                BStr := StringToOleStr(pAnsiString(ParamPtr^)^);
                PStr := pAnsiString(ParamPtr^);
                ArgPtr^[0] := varOleStr or varByRef;
                ArgPtr^[2] := Integer(@BStr);
              end else
              begin
                BStr := StringToOleStr(pAnsiString(ParamPtr)^);
                PStr := nil;
                ArgPtr^[0] := varOleStr;
                ArgPtr^[2] := Integer(BStr);
              end;
            Inc(StrCount);
          end else
          if VarFlag <> 0 then
          begin
            if (ArgType = varVariant) and
              (PVarData(ParamPtr^)^.VType = varString) then
              VarCast(PVariant(ParamPtr^)^, PVariant(ParamPtr^)^, varOleStr);
            ArgPtr^[0] := ArgType or varByRef;
            ArgPtr^[2] := ParamPtr^;
          end else
          if ArgType = varVariant then
          begin
            if PVarData(ParamPtr)^.VType = varString then
            begin
              with Strings[StrCount] do
              begin
                BStr := StringToOleStr(AnsiString(PVarData(ParamPtr^)^.VString));
                PStr := nil;
                ArgPtr^[0] := varOleStr;
                ArgPtr^[2] := Integer(BStr);
              end;
              Inc(StrCount);
            end else
            begin
              VarPtr := PVarArg(ParamPtr);
              ArgPtr^[0] := VarPtr^[0];
              ArgPtr^[1] := VarPtr^[1];
              ArgPtr^[2] := VarPtr^[2];
              ArgPtr^[3] := VarPtr^[3];
              Inc(Integer(ParamPtr), 12);
            end;
          end else
          begin
            ArgPtr^[0] := ArgType;
            ArgPtr^[2] := ParamPtr^;
            if (ArgType >= varDouble) and (ArgType <= varDate) then
            begin
              Inc(Integer(ParamPtr), 4);
              ArgPtr^[3] := ParamPtr^;
            end;
          end;
          Inc(Integer(ParamPtr), 4);
        end;
        Inc(I);
      until I = ArgCount;
    end;
    DispParams.rgvarg := @Args;
    DispParams.rgdispidNamedArgs := @DispIDs[1];
    DispParams.cArgs := ArgCount;
    DispParams.cNamedArgs := CallDesc^.NamedArgCount;
    DispID := DispIDs[0];
    InvKind := CallDesc^.CallType;
    if InvKind = DISPATCH_PROPERTYPUT then
    begin
      if Args[0][0] and varTypeMask = varDispatch then
        InvKind := DISPATCH_PROPERTYPUTREF;
      DispIDs[0] := DISPID_PROPERTYPUT;
      Dec(Integer(DispParams.rgdispidNamedArgs), SizeOf(Integer));
      Inc(DispParams.cNamedArgs);
    end else
      if (InvKind = DISPATCH_METHOD) and (ArgCount = 0) and (Result <> nil) then
        InvKind := DISPATCH_METHOD or DISPATCH_PROPERTYGET;
    Status := Dispatch.Invoke(DispID, GUID_NULL, 0, InvKind, DispParams,
      Result, @ExcepInfo, nil);
    if Status <> 0 then DispatchInvokeError(Status, ExcepInfo);
    J := StrCount;
    while J <> 0 do
    begin
      Dec(J);
      with Strings[J] do
        if PStr <> nil then OleStrToStrVar(BStr, PStr^);
    end;
  finally
    K := StrCount;
    while K <> 0 do
    begin
      Dec(K);
      SysFreeString(Strings[K].BStr);
    end;
  end;
end;

{ Call GetIDsOfNames method on the given IDispatch interface }

procedure GetIDsOfNames(const Dispatch: IDispatch; Names: PAnsiChar;
  NameCount: Integer; DispIDs: PDispIDList);

  procedure RaiseNameException;
  begin
    raise EOleError.CreateResFmt(e_Com, Integer( @SNoMethod ), [Names]);
  end;

type
  PNamesArray = ^TNamesArray;
  TNamesArray = array[0..0] of PWideChar;
var
  N, SrcLen, DestLen: Integer;
  Src: PAnsiChar;
  Dest: PWideChar;
  NameRefs: PNamesArray;
  StackTop: Pointer;
  Temp: Integer;
begin
  Src := Names;
  N := 0;
  asm
    MOV  StackTop, ESP
    MOV  EAX, NameCount
    INC  EAX
    SHL  EAX, 2  // sizeof pointer = 4
    SUB  ESP, EAX
    LEA  EAX, NameRefs
    MOV  [EAX], ESP
  end;
  repeat
    SrcLen := StrLen(Src);
    DestLen := MultiByteToWideChar(0, 0, Src, SrcLen, nil, 0) + 1;
    asm
      MOV  EAX, DestLen
      ADD  EAX, EAX
      ADD  EAX, 3      // round up to 4 byte boundary
      AND  EAX, not 3
      SUB  ESP, EAX
      LEA  EAX, Dest
      MOV  [EAX], ESP
    end;
    if N = 0 then NameRefs[0] := Dest else NameRefs[NameCount - N] := Dest;
    MultiByteToWideChar(0, 0, Src, SrcLen, Dest, DestLen);
    Dest[DestLen-1] := #0;
    Inc(Src, SrcLen+1);
    Inc(N);
  until N = NameCount;
  Temp := Dispatch.GetIDsOfNames(GUID_NULL, NameRefs, NameCount,
    GetThreadLocale, DispIDs);
  if Temp = Integer(DISP_E_UNKNOWNNAME) then RaiseNameException else OleCheck(Temp);
  asm
    MOV  ESP, StackTop
  end;
end;

{ Central call dispatcher }

procedure VarDispInvoke(Result: PVariant; const Instance: Variant;
  CallDesc: PCallDesc; Params: Pointer); cdecl;

  procedure RaiseException;
  begin
    raise EOleError.CreateResFmt(e_Com, Integer( @SVarNotObject ), [ nil ] );
  end;

var
  Dispatch: Pointer;
  DispIDs: array[0..MaxDispArgs - 1] of Integer;
begin
  if TVarData(Instance).VType = varDispatch then
    Dispatch := TVarData(Instance).VDispatch
  else if TVarData(Instance).VType = (varDispatch or varByRef) then
    Dispatch := Pointer(TVarData(Instance).VPointer^)
  else RaiseException;
  GetIDsOfNames(IDispatch(Dispatch), @CallDesc^.ArgTypes[CallDesc^.ArgCount],
    CallDesc^.NamedArgCount + 1, @DispIDs);
  if Result <> nil then VarClear(Result^);
  DispatchInvoke(IDispatch(Dispatch), CallDesc, @DispIDs, @Params, Result);
end;

{ Raise exception given an OLE return code and TExcepInfo structure }

procedure DispCallError(Status: Integer; var ExcepInfo: TExcepInfo;
  ErrorAddr: Pointer; FinalizeExcepInfo: Boolean);
var
  E: Exception;
begin
  if Status = Integer(DISP_E_EXCEPTION) then
  begin
    with ExcepInfo do
      E := EOleException.Create(e_Com, bstrDescription {, scode, bstrSource,
        bstrHelpFile, dwHelpContext } );
    if FinalizeExcepInfo then
      Finalize(ExcepInfo);
  end else
    E := EOleSysError.Create(e_com, '' {, Status, 0});
  if ErrorAddr <> nil then
    raise E at ErrorAddr
  else
    raise E;
end;

{ Raise exception given an OLE return code and TExcepInfo structure }

procedure DispatchInvokeError(Status: Integer; const ExcepInfo: TExcepInfo);
begin
  DispCallError(Status, PExcepInfo(@ExcepInfo)^, nil, False);
end;

procedure ClearExcepInfo(var ExcepInfo: TExcepInfo);
begin
  FillChar(ExcepInfo, SizeOf(ExcepInfo), 0);
end;

procedure DispCall(const Dispatch: IDispatch; CallDesc: PCallDesc;
  DispID: Integer; NamedArgDispIDs, Params, Result: Pointer); stdcall;
type
  TExcepInfoRec = record  // mock type to avoid auto init and cleanup code
    wCode: Word;
    wReserved: Word;
    bstrSource: PWideChar;
    bstrDescription: PWideChar;
    bstrHelpFile: PWideChar;
    dwHelpContext: Longint;
    pvReserved: Pointer;
    pfnDeferredFillIn: Pointer;
    scode: HResult;
  end;
var
  DispParams: TDispParams;
  ExcepInfo: TExcepInfoRec;
asm
        PUSH    EBX
        PUSH    ESI
        PUSH    EDI
        MOV     EBX,CallDesc
        XOR     EDX,EDX
        MOV     EDI,ESP
        MOVZX   ECX,[EBX].TCallDesc.ArgCount
        MOV     DispParams.cArgs,ECX
        TEST    ECX,ECX
        JE      @@10
        ADD     EBX,OFFSET TCallDesc.ArgTypes
        MOV     ESI,Params
@@1:    MOVZX   EAX,[EBX].Byte
        TEST    AL,atByRef
        JNE     @@3
        CMP     AL,varVariant
        JE      @@2
        CMP     AL,varDouble
        JB      @@4
        CMP     AL,varDate
        JA      @@4
        PUSH    [ESI].Integer[4]
        PUSH    [ESI].Integer[0]
        PUSH    EDX
        PUSH    EAX
        ADD     ESI,8
        JMP     @@5
@@2:    PUSH    [ESI].Integer[12]
        PUSH    [ESI].Integer[8]
        PUSH    [ESI].Integer[4]
        PUSH    [ESI].Integer[0]
        ADD     ESI,16
        JMP     @@5
@@3:    AND     AL,atTypeMask
        OR      EAX,varByRef
@@4:    PUSH    EDX
        PUSH    [ESI].Integer[0]
        PUSH    EDX
        PUSH    EAX
        ADD     ESI,4
@@5:    INC     EBX
        DEC     ECX
        JNE     @@1
        MOV     EBX,CallDesc
@@10:   MOV     DispParams.rgvarg,ESP
        MOVZX   EAX,[EBX].TCallDesc.NamedArgCount
        MOV     DispParams.cNamedArgs,EAX
        TEST    EAX,EAX
        JE      @@12
        MOV     ESI,NamedArgDispIDs
@@11:   PUSH    [ESI].Integer[EAX*4-4]
        DEC     EAX
        JNE     @@11
@@12:   MOVZX   ECX,[EBX].TCallDesc.CallType
        CMP     ECX,DISPATCH_PROPERTYPUT
        JNE     @@20
        PUSH    DISPID_PROPERTYPUT
        INC     DispParams.cNamedArgs
        CMP     [EBX].TCallDesc.ArgTypes.Byte[0],varDispatch
        JE      @@13
        CMP     [EBX].TCallDesc.ArgTypes.Byte[0],varUnknown
        JNE     @@20
@@13:   MOV     ECX,DISPATCH_PROPERTYPUTREF
@@20:   MOV     DispParams.rgdispidNamedArgs,ESP
        PUSH    EDX                     { ArgErr }
        LEA     EAX,ExcepInfo
        PUSH    EAX                     { ExcepInfo }
        PUSH    ECX
        PUSH    EDX
        CALL    ClearExcepInfo
        POP     EDX
        POP     ECX
        PUSH    Result                  { VarResult }
        LEA     EAX,DispParams
        PUSH    EAX                     { Params }
        PUSH    ECX                     { Flags }
        PUSH    EDX                     { LocaleID }
        PUSH    OFFSET GUID_NULL        { IID }
        PUSH    DispID                  { DispID }
        MOV     EAX,Dispatch
        PUSH    EAX
        MOV     EAX,[EAX]
        CALL    [EAX].Pointer[24]
        TEST    EAX,EAX
        JE      @@30
        LEA     EDX,ExcepInfo
        MOV     CL, 1
        PUSH    ECX
        MOV     ECX,[EBP+4]
        JMP     DispCallError
@@30:   MOV     ESP,EDI
        POP     EDI
        POP     ESI
        POP     EBX
end;

procedure DispCallByID(Result: Pointer; const Dispatch: IDispatch;
  DispDesc: PDispDesc; Params: Pointer); cdecl;
asm
        PUSH    EBX
        MOV     EBX,DispDesc
        XOR     EAX,EAX
        PUSH    EAX
        PUSH    EAX
        PUSH    EAX
        PUSH    EAX
        MOV     EAX,ESP
        PUSH    EAX
        LEA     EAX,Params
        PUSH    EAX
        PUSH    EAX
        PUSH    [EBX].TDispDesc.DispID
        LEA     EAX,[EBX].TDispDesc.CallDesc
        PUSH    EAX
        PUSH    Dispatch
        CALL    DispCall
        MOVZX   EAX,[EBX].TDispDesc.ResType
        MOV     EBX,Result
        JMP     @ResultTable.Pointer[EAX*4]

@ResultTable:
        DD      @ResEmpty
        DD      @ResNull
        DD      @ResSmallint
        DD      @ResInteger
        DD      @ResSingle
        DD      @ResDouble
        DD      @ResCurrency
        DD      @ResDate
        DD      @ResString
        DD      @ResDispatch
        DD      @ResError
        DD      @ResBoolean
        DD      @ResVariant
        DD      @ResUnknown
        DD      @ResDecimal
        DD      @ResError
        DD      @ResByte

@ResSingle:
        FLD     [ESP+8].Single
        JMP     @ResDone

@ResDouble:
@ResDate:
        FLD     [ESP+8].Double
        JMP     @ResDone

@ResCurrency:
        FILD    [ESP+8].Currency
        JMP     @ResDone

@ResString:
        MOV     EAX,[EBX]
        TEST    EAX,EAX
        JE      @@1
        PUSH    EAX
        CALL    SysFreeString
@@1:    MOV     EAX,[ESP+8]
        MOV     [EBX],EAX
        JMP     @ResDone

@ResDispatch:
@ResUnknown:
        MOV     EAX,[EBX]
        TEST    EAX,EAX
        JE      @@2
        PUSH    EAX
        MOV     EAX,[EAX]
        CALL    [EAX].Pointer[8]
@@2:    MOV     EAX,[ESP+8]
        MOV     [EBX],EAX
        JMP     @ResDone

@ResVariant:
        MOV     EAX,EBX
        CALL    System.@VarClear
        MOV     EAX,[ESP]
        MOV     [EBX],EAX
        MOV     EAX,[ESP+4]
        MOV     [EBX+4],EAX
        MOV     EAX,[ESP+8]
        MOV     [EBX+8],EAX
        MOV     EAX,[ESP+12]
        MOV     [EBX+12],EAX
        JMP     @ResDone

@ResSmallint:
@ResInteger:
@ResBoolean:
@ResByte:
        MOV     EAX,[ESP+8]

@ResDecimal:
@ResEmpty:
@ResNull:
@ResError:
@ResDone:
        ADD     ESP,16
        POP     EBX
end;

var
  ComClassManagerVar: TObject;
  SaveInitProc: Pointer;
  NeedToUninitialize: Boolean;

function ComClassManager: TComClassManager;
begin
  if ComClassManagerVar = nil then
    ComClassManagerVar := TComClassManager.Create;
  Result := TComClassManager(ComClassManagerVar);
end;

procedure InitComObj;
begin
  if SaveInitProc <> nil then TProcedure(SaveInitProc);
  if (CoInitFlags <> -1) and Assigned(KOLComObj.CoInitializeEx) then
  begin
    NeedToUninitialize := Succeeded(KOLComObj.CoInitializeEx(nil, CoInitFlags));
    IsMultiThread := IsMultiThread or
      ((CoInitFlags and COINIT_APARTMENTTHREADED) <> 0) or
      (CoInitFlags = COINIT_MULTITHREADED);  // this flag has value zero
  end
  else
    NeedToUninitialize := Succeeded(CoInitialize(nil));
end;


initialization
begin
  LoadComExProcs;
  VarDispProc := @VarDispInvoke;
  DispCallByIDProc := @DispCallByID;
  SafeCallErrorProc := @SafeCallError;
  if not IsLibrary then
  begin
    SaveInitProc := InitProc;
    InitProc := @InitComObj;
  end;
end;

finalization
begin
  OleUninitializing := True;
  ComClassManagerVar.Free;
  SafeCallErrorProc := nil;
  DispCallByIDProc := nil;
  VarDispProc := nil;
  if NeedToUninitialize then CoUninitialize;
end;

end.

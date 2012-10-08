{  =============================================================================

    UnitName     : XIe
    Ver          : 1.1
    Create Date  : 09.07.2007
    Last Edit    : 19.01.2011 by Pavel Perminov
    Author       : Dmitry Mirovodin
                   http://www.hcsoft.spb.ru
                   mirovodin@mail.ru
                   support@hcsoft.spb.ru

   ==========================================================================  }

unit XIE;

interface

uses
  Windows, ActiveX, URLMon;

type

  TIEWrapperOnProcess =  Procedure (const ProgressProcent: Byte; const StatusID : Cardinal; Const StatusText : String ) of object;


  TBindStatusCallBack = Class(TObject, IUnknown, IBindStatusCallback)
  private
    fOnProcess : TIEWrapperOnProcess;
    function QueryInterface(const IID: TGUID; out Obj): HRESULT; stdcall;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
  protected
    function OnStartBinding(dwReserved: DWORD; pib: IBinding): HResult; stdcall;
    function GetPriority(out nPriority): HResult; stdcall;
    function OnLowResource(reserved: DWORD): HResult; stdcall;
    function OnProgress(ulProgress, ulProgressMax, ulStatusCode: ULONG; szStatusText: LPCWSTR): HResult; stdcall;
    function OnStopBinding(hresult: HResult; szError: LPCWSTR): HResult; stdcall;
    function GetBindInfo(out grfBINDF: DWORD; var bindinfo: TBindInfo): HResult; virtual; stdcall; 
    function OnDataAvailable(grfBSCF: DWORD; dwSize: DWORD; formatetc: PFormatEtc; stgmed: PStgMedium): HResult; stdcall;
    function OnObjectAvailable(const iid: TGUID; punk: IUnknown): HResult; stdcall;
  public
    constructor Create();
    Property OnProcess : TIEWrapperOnProcess Read fOnProcess Write fOnProcess;
    Class function ProcessStatusIdToString(Const StatusId: Cardinal):String;
  end;

  TIEWrapper = Class (TObject)
  protected
    fBindStatusCallback : TBindStatusCallback;
    function GetOnProcess : TIEWrapperOnProcess;
    procedure SetOnProcess( Value : TIEWrapperOnProcess);
    function CheckRequest(const Request : String): boolean; Virtual;
  public
    constructor Create(); virtual;
    destructor Destroy(); override;
    function  OpenRequest(const Request : String):String;
    function LoadFile(const Request : String; const FileName: String): boolean;
    property OnProcess : TIEWrapperOnProcess Read GetOnProcess Write SetOnProcess;
  end;


implementation

uses
  SysUtils;

{
const
  BINDF_ASYNCHRONOUS            = $00000001;
  BINDF_ASYNCSTORAGE            = $00000002;
  BINDF_NOPROGRESSIVERENDERING  = $00000004;
  BINDF_OFFLINEOPERATION        = $00000008;
  BINDF_GETNEWESTVERSION        = $00000010;
  BINDF_NOWRITECACHE            = $00000020;
  BINDF_NEEDFILE                = $00000040;
  BINDF_PULLDATA                = $00000080;
  BINDF_IGNORESECURITYPROBLEM   = $00000100;
  BINDF_RESYNCHRONIZE           = $00000200;
  BINDF_HYPERLINK               = $00000400;
  BINDF_NO_UI                   = $00000800;
  BINDF_SILENTOPERATION         = $00001000;
  BINDF_PRAGMA_NO_CACHE         = $00002000;
  BINDF_GETCLASSOBJECT          = $00004000;
  BINDF_RESERVED_1              = $00008000;
  BINDF_FREE_THREADED           = $00010000;
  BINDF_DIRECT_READ             = $00020000;
  BINDF_FORMS_SUBMIT            = $00040000;
  BINDF_GETFROMCACHE_IF_NET_FAIL= $00080000;
  BINDF_FROMURLMON              = $00100000;
  BINDF_FWD_BACK                = $00200000;
  BINDF_PREFERDEFAULTHANDLER    = $00400000;
  BINDF_RESERVED_3              = $00800000;
}

// ========================================================================== //

constructor TBindStatusCallback.Create();
begin
 inherited Create;
end;

// -----------------------------------------------------------------------------

function TBindStatusCallback.QueryInterface(const IID: TGUID; out Obj): HResult;
begin
  if GetInterface(IID, Obj) then Result := S_OK
    else Result := E_NOINTERFACE;
end;

// -----------------------------------------------------------------------------

function TBindStatusCallback._AddRef: Integer;
begin
  Result := -1;
end;

// -----------------------------------------------------------------------------

function TBindStatusCallback._Release: Integer;
begin
  Result := -1;
end;

// -----------------------------------------------------------------------------

Class function TBindStatusCallback.ProcessStatusIdToString(Const StatusId: Cardinal):String;
begin
  case StatusId of
    BINDSTATUS_FINDINGRESOURCE : result := 'BINDSTATUS_FINDINGRESOURCE';
    BINDSTATUS_CONNECTING   : Result := 'BINDSTATUS_CONNECTING';
    BINDSTATUS_REDIRECTING  : Result := 'BINDSTATUS_REDIRECTING';
    BINDSTATUS_BEGINDOWNLOADDATA : Result := 'BINDSTATUS_BEGINDOWNLOADDATA';
    BINDSTATUS_DOWNLOADINGDATA : Result := 'BINDSTATUS_DOWNLOADINGDATA';
    BINDSTATUS_ENDDOWNLOADDATA : Result := 'BINDSTATUS_ENDDOWNLOADDATA';
    BINDSTATUS_BEGINDOWNLOADCOMPONENTS : Result := 'BINDSTATUS_BEGINDOWNLOADCOMPONENTS';
    BINDSTATUS_INSTALLINGCOMPONENTS  : Result := 'BINDSTATUS_INSTALLINGCOMPONENTS';
    BINDSTATUS_ENDDOWNLOADCOMPONENTS  : Result := 'BINDSTATUS_ENDDOWNLOADCOMPONENTS';
    BINDSTATUS_USINGCACHEDCOPY : Result := 'BINDSTATUS_USINGCACHEDCOPY';
    BINDSTATUS_SENDINGREQUEST : Result := 'BINDSTATUS_SENDINGREQUEST';
    BINDSTATUS_CLASSIDAVAILABLE : Result := 'BINDSTATUS_CLASSIDAVAILABLE';
    BINDSTATUS_MIMETYPEAVAILABLE : Result := 'BINDSTATUS_MIMETYPEAVAILABLE';
    BINDSTATUS_CACHEFILENAMEAVAILABLE : Result := 'BINDSTATUS_CACHEFILENAMEAVAILABLE';
    BINDSTATUS_BEGINSYNCOPERATION : Result := 'BINDSTATUS_BEGINSYNCOPERATION';
    BINDSTATUS_ENDSYNCOPERATION : Result := 'BINDSTATUS_ENDSYNCOPERATION';
    BINDSTATUS_BEGINUPLOADDATA : Result := 'BINDSTATUS_BEGINUPLOADDATA';
    BINDSTATUS_UPLOADINGDATA : Result:= 'BINDSTATUS_UPLOADINGDATA';
    BINDSTATUS_ENDUPLOADDATA : Result:= 'BINDSTATUS_ENDUPLOADDATA';
    BINDSTATUS_PROTOCOLCLASSID : Result := 'BINDSTATUS_PROTOCOLCLASSID';
    BINDSTATUS_ENCODING : Result:= 'BINDSTATUS_ENCODING';
    BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE : Result := 'BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE';
    BINDSTATUS_CLASSINSTALLLOCATION : Result := 'BINDSTATUS_CLASSINSTALLLOCATION';
    BINDSTATUS_DECODING : Result := 'BINDSTATUS_DECODING';
    BINDSTATUS_LOADINGMIMEHANDLER : Result := 'BINDSTATUS_LOADINGMIMEHANDLER';
    BINDSTATUS_CONTENTDISPOSITIONATTACH : Result := 'BINDSTATUS_CONTENTDISPOSITIONATTACH';
    BINDSTATUS_FILTERREPORTMIMETYPE : Result := 'BINDSTATUS_FILTERREPORTMIMETYPE';
    BINDSTATUS_CLSIDCANINSTANTIATE : Result := 'BINDSTATUS_CLSIDCANINSTANTIATE';
    BINDSTATUS_IUNKNOWNAVAILABLE : Result := 'BINDSTATUS_IUNKNOWNAVAILABLE';
    BINDSTATUS_DIRECTBIND : Result := 'BINDSTATUS_DIRECTBIND';
    BINDSTATUS_RAWMIMETYPE : Result := 'BINDSTATUS_RAWMIMETYPE';
    BINDSTATUS_PROXYDETECTING : Result := 'BINDSTATUS_PROXYDETECTING';
    BINDSTATUS_ACCEPTRANGES : Result := 'BINDSTATUS_ACCEPTRANGES';
    BINDSTATUS_COOKIE_SENT : Result := 'BINDSTATUS_COOKIE_SENT';
    BINDSTATUS_COMPACT_POLICY_RECEIVED : Result := 'BINDSTATUS_COMPACT_POLICY_RECEIVED';
    BINDSTATUS_COOKIE_SUPPRESSED : Result := 'BINDSTATUS_COOKIE_SUPPRESSED';
    BINDSTATUS_COOKIE_STATE_UNKNOWN : Result := 'BINDSTATUS_COOKIE_STATE_UNKNOWN';
    BINDSTATUS_COOKIE_STATE_ACCEPT : Result := 'BINDSTATUS_COOKIE_STATE_ACCEPT';
    BINDSTATUS_COOKIE_STATE_REJECT : Result := 'BINDSTATUS_COOKIE_STATE_REJECT';
    BINDSTATUS_COOKIE_STATE_PROMPT : Result := 'BINDSTATUS_COOKIE_STATE_PROMPT';
    BINDSTATUS_COOKIE_STATE_LEASH  : Result := 'BINDSTATUS_COOKIE_STATE_LEASH';
    BINDSTATUS_COOKIE_STATE_DOWNGRADE : Result := 'BINDSTATUS_COOKIE_STATE_DOWNGRADE';
    BINDSTATUS_POLICY_HREF : Result := 'BINDSTATUS_POLICY_HREF';
    BINDSTATUS_P3P_HEADER : Result := 'BINDSTATUS_P3P_HEADER';
    BINDSTATUS_SESSION_COOKIE_RECEIVED : Result := 'BINDSTATUS_SESSION_COOKIE_RECEIVED';
    BINDSTATUS_PERSISTENT_COOKIE_RECEIVED : Result := 'BINDSTATUS_PERSISTENT_COOKIE_RECEIVED';
    BINDSTATUS_SESSION_COOKIES_ALLOWED : Result := 'BINDSTATUS_SESSION_COOKIES_ALLOWED';
   else
    Result := 'N/A Code : ' + IntToStr(StatusId);
  end;
end;

// -----------------------------------------------------------------------------

function TBindStatusCallback.OnStartBinding(dwReserved: DWORD; pib: IBinding): HResult;
begin
  Result := E_NOTIMPL;
end;

// -----------------------------------------------------------------------------

function TBindStatusCallback.GetPriority(out nPriority): HResult;
begin
  Result := E_NOTIMPL;
end;

// -----------------------------------------------------------------------------

function TBindStatusCallback.OnLowResource(reserved: DWORD): HResult;
begin
  Result := E_NOTIMPL;
end;

// -----------------------------------------------------------------------------

function TBindStatusCallback.OnProgress(ulProgress, ulProgressMax, ulStatusCode: ULONG; szStatusText: LPCWSTR): HResult;
var
  Proc : Byte;
begin
  if Assigned(fOnProcess) then
  begin
    if ulStatusCode = BINDSTATUS_ENDDOWNLOADDATA then Proc := 100 else
     if ulProgressMax = 0 then Proc := 0 else
       Proc := Trunc( ulProgress * 100 / ulProgressMax);

    fOnProcess(Proc, ulStatusCode, szStatusText);
  end;  
  Result := E_NOTIMPL;
end;

// -----------------------------------------------------------------------------

function TBindStatusCallback.OnStopBinding(hresult: HResult; szError: LPCWSTR): HResult;
begin
  Result := E_NOTIMPL;
end;

// -----------------------------------------------------------------------------

function TBindStatusCallback.GetBindInfo(out grfBINDF: DWORD; var bindinfo: TBindInfo): HResult;
begin
  Result := E_NOTIMPL;
//  grfBINDF := BINDF_GETNEWESTVERSION;
//  Result :=BINDF_GETNEWESTVERSION;
end;

// -----------------------------------------------------------------------------

function TBindStatusCallback.OnDataAvailable(grfBSCF: DWORD; dwSize: DWORD; formatetc: PFormatEtc; stgmed: PStgMedium): HResult;
begin
  Result := E_NOTIMPL;
end;

// -----------------------------------------------------------------------------

function TBindStatusCallback.OnObjectAvailable(const iid: TGUID; punk: IUnknown): HResult;
begin
  Result := E_NOTIMPL;
end;

// ========================================================================== //

Constructor TIEWrapper.Create;
begin
  fBindStatusCallback := TBindStatusCallback.Create;
end;

// -----------------------------------------------------------------------------

function TIEWrapper.GetOnProcess : TIEWrapperOnProcess;
begin
  Result:=fBindStatusCallback.OnProcess;
end;

// -----------------------------------------------------------------------------

procedure TIEWrapper.SetOnProcess( Value : TIEWrapperOnProcess);
begin
  fBindStatusCallback.OnProcess := Value;
end;

// -----------------------------------------------------------------------------

function TIEWrapper.CheckRequest(const Request : String): boolean;
begin
  result := False;
  if Length(Request)>0 then Result := True;
end;

// -----------------------------------------------------------------------------

function TIEWrapper.OpenRequest(const Request : string):String;
Var
  Stream            : IStream;
  StreamInfo        : STATSTG;
  BuffSize          : Integer;
  P                 : Pointer;
begin
  Result := '';
  if not CheckRequest(Request) then Exit;
  Stream := nil;
  if URLOpenBlockingStream(nil, PChar(Request), Stream,  0, fBindStatusCallback) = S_OK then
    Begin
      ZeroMemory(@StreamInfo, SizeOf(StreamInfo));
      If Stream.Stat(StreamInfo, 0) = S_OK Then
        Begin
          If StreamInfo.cbSize > 0 Then
            Begin
              BuffSize := StreamInfo.cbSize;
              GetMem(P, BuffSize);
              try
                ZeroMemory(P, SizeOf(BuffSize));
                Stream.Read(P, buffsize, Nil);
                Result := PCHAR(P);
              finally
                FreeMem(P);
              end;
            End;
        End;
      Stream := nil;
    End;
end;

// -----------------------------------------------------------------------------

function TIEWrapper.LoadFile(const Request : String; const FileName: String): boolean;
begin
  Result := false;
  if not CheckRequest(Request) then Exit;
  if URLDownloadToFile(nil, PChar(Request), PCHAR(FileName), 0, fBindStatusCallback) = S_OK then
    Result := True;
end;

// -----------------------------------------------------------------------------

destructor TIEWrapper.Destroy();
begin
  fBindStatusCallback.Free;
  fBindStatusCallback := nil;
  inherited Destroy;
end;

// ========================================================================== //

end.

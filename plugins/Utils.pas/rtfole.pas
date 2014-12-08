unit rtfole;

interface

uses
  Windows, RichEdit, ActiveX,
  tlb_richedit;

const
  IID_IOleObject          : TGUID = '{00000112-0000-0000-C000-000000000046}';
  IID_IRichEditOle        : TGUID = '{00020D00-0000-0000-C000-000000000046}';
  IID_IRichEditOleCallback: TGUID = '{00020D03-0000-0000-C000-000000000046}';

type
  TReObject = packed record
    cbStruct: DWORD;          // Size of structure
    cp      : Integer;        // Character position of object
    clsid   : TCLSID;         // Class ID of object
    poleobj : IOleObject;     // OLE object interface
    pstg    : IStorage;       // Associated storage interface
    polesite: IOLEClientSite; // Associated client site interface
    sizel   : TSize;          // Size of object (may be 0,0)
    dvaspect: DWORD;          // Display aspect to use
    dwFlags : DWORD;          // Object status flags
    dwUser  : DWORD;          // Dword for user's use
  end;

const
  // Flags to specify which interfaces should be returned in the structure above
  REO_GETOBJ_NO_INTERFACES  = $00000000;
  REO_GETOBJ_POLEOBJ        = $00000001;
  REO_GETOBJ_PSTG           = $00000002;
  REO_GETOBJ_POLESITE       = $00000004;
  REO_GETOBJ_ALL_INTERFACES = $00000007;

  // Place object at selection
  REO_CP_SELECTION  = ULONG(-1);

  // Use character position to specify object instead of index
  REO_IOB_SELECTION = ULONG(-1);
  REO_IOB_USE_CP    = ULONG(-1);

  // Object flags
  REO_NULL            = $00000000; // No flags
  REO_READWRITEMASK   = $0000003F; // Mask out RO bits
  REO_DONTNEEDPALETTE = $00000020; // Object doesn't need palette
  REO_BLANK           = $00000010; // Object is blank
  REO_DYNAMICSIZE     = $00000008; // Object defines size always
  REO_INVERTEDSELECT  = $00000004; // Object drawn all inverted if sel
  REO_BELOWBASELINE   = $00000002; // Object sits below the baseline
  REO_RESIZABLE       = $00000001; // Object may be resized
  REO_LINK            = $80000000; // Object is a link (RO)
  REO_STATIC          = $40000000; // Object is static (RO)
  REO_SELECTED        = $08000000; // Object selected (RO)
  REO_OPEN            = $04000000; // Object open in its server (RO)
  REO_INPLACEACTIVE   = $02000000; // Object in place active (RO)
  REO_HILITED         = $01000000; // Object is to be hilited (RO)
  REO_LINKAVAILABLE   = $00800000; // Link believed available (RO)
  REO_GETMETAFILE     = $00400000; // Object requires metafile (RO)

  // flags for IRichEditOle::GetClipboardData(),
  // IRichEditOleCallback::GetClipboardData() and
  // IRichEditOleCallback::QueryAcceptData()
  RECO_PASTE  = $00000000; // paste from clipboard
  RECO_DROP   = $00000001; // drop
  RECO_COPY   = $00000002; // copy to the clipboard
  RECO_CUT    = $00000003; // cut to the clipboard
  RECO_DRAG   = $00000004; // drag

type  
  TImageDataObject = class(TInterfacedObject,IDataObject)
  private
    FBmp:hBitmap;
    FMedium:TStgMedium;
    FFormatEtc: TFormatEtc;
    procedure SetBitmap(bmp:hBitmap);
    function GetOleObject(OleClientSite:IOleClientSite; Storage:IStorage):IOleObject;
    // IDataObject
    function GetData(const formatetcIn: TFormatEtc; out medium: TStgMedium): HResult; stdcall;
    function GetDataHere(const formatetc: TFormatEtc; out medium: TStgMedium): HResult; stdcall;
    function QueryGetData(const formatetc: TFormatEtc): HResult; stdcall;
    function GetCanonicalFormatEtc(const formatetc: TFormatEtc; out formatetcOut: TFormatEtc): HResult; stdcall;
{$IFDEF FPC}
    function SetData(const formatetc: TFormatEtc; const medium: TStgMedium; fRelease: BOOL): HResult; stdcall;
    function EnumFormatEtc(dwDirection: dword; out enumFormatEtc: IEnumFormatEtc): HResult; stdcall;
    function DAdvise(const formatetc: TFormatEtc; advf: dword; const advSink: IAdviseSink; out dwConnection: dword): HResult; stdcall;
    function DUnadvise(dwConnection: dword): HResult; stdcall;
{$ELSE}
    function SetData(const formatetc: TFormatEtc; var medium: TStgMedium; fRelease: BOOL): HResult; stdcall;
    function EnumFormatEtc(dwDirection: Longint; out enumFormatEtc: IEnumFormatEtc): HResult; stdcall;
    function DAdvise(const formatetc: TFormatEtc; advf: Longint; const advSink: IAdviseSink; out dwConnection: Longint): HResult; stdcall;
    function DUnadvise(dwConnection: Longint): HResult; stdcall;
{$ENDIF}
    function EnumDAdvise(out enumAdvise: IEnumStatData): HResult; stdcall;
  public
    destructor Destroy; override;
    function InsertBitmap(Wnd: HWND; Bitmap: hBitmap; cp: Cardinal): Boolean;
  end;

  IRichEditOle = interface(IUnknown)
    ['{00020d00-0000-0000-c000-000000000046}']
    function GetClientSite(out clientSite: IOleClientSite): HResult; stdcall;
    function GetObjectCount: HResult; stdcall;
    function GetLinkCount: HResult; stdcall;
    function GetObject(iob: Longint; out ReObject: TReObject; dwFlags: DWORD): HResult; stdcall;
    function InsertObject(var ReObject: TReObject): HResult; stdcall;
    function ConvertObject(iob: Longint; rclsidNew: TIID; lpstrUserTypeNew: LPCSTR): HResult; stdcall;
    function ActivateAs(rclsid: TIID; rclsidAs: TIID): HResult; stdcall;
    function SetHostNames(lpstrContainerApp: LPCSTR; lpstrContainerObj: LPCSTR): HResult; stdcall;
    function SetLinkAvailable(iob: Longint; fAvailable: BOOL): HResult; stdcall;
    function SetDvaspect(iob: Longint; dvaspect: DWORD): HResult; stdcall;
    function HandsOffStorage(iob: Longint): HResult; stdcall;
    function SaveCompleted(iob: Longint; const stg: IStorage): HResult; stdcall;
    function InPlaceDeactivate: HResult; stdcall;
    function ContextSensitiveHelp(fEnterMode: BOOL): HResult; stdcall;
    function GetClipboardData(var chrg: TCharRange; reco: DWORD; out dataobj: IDataObject): HResult; stdcall;
    function ImportDataObject(dataobj: IDataObject; cf: TClipFormat; hMetaPict: HGLOBAL): HResult; stdcall;
  end;

  IRichEditOleCallback = interface(IUnknown)
    ['{00020d03-0000-0000-c000-000000000046}']
    function GetNewStorage(out stg: IStorage): HResult; stdcall;
    function GetInPlaceContext(out Frame: IOleInPlaceFrame; out Doc: IOleInPlaceUIWindow; lpFrameInfo: POleInPlaceFrameInfo): HResult; stdcall;
    function ShowContainerUI(fShow: BOOL): HResult; stdcall;
    function QueryInsertObject(const clsid: TCLSID; const stg: IStorage; cp: Longint): HResult; stdcall;
    function DeleteObject(const oleobj: IOleObject): HResult; stdcall;
    function QueryAcceptData(const dataobj: IDataObject; var cfFormat: TClipFormat; reco: DWORD; fReally: BOOL; hMetaPict: HGLOBAL): HResult; stdcall;
    function ContextSensitiveHelp(fEnterMode: BOOL): HResult; stdcall;
    function GetClipboardData(const chrg: TCharRange; reco: DWORD; out dataobj: IDataObject): HResult; stdcall;
    function GetDragDropEffect(fDrag: BOOL; grfKeyState: DWORD; var dwEffect: DWORD): HResult; stdcall;
    function GetContextMenu(seltype: Word; const oleobj: IOleObject; const chrg: TCharRange; out menu: HMENU): HResult; stdcall;
  end;

  TRichEditOleCallback = class(TObject, IUnknown, IRichEditOleCallback)
    private
      FRefCount: Longint;
    public
      constructor Create;
      destructor Destroy; override;
      function QueryInterface(
      {$IFDEF FPC_HAS_CONSTREF}constref{$ELSE}const{$ENDIF} iid: TGUID; out Obj): HResult; stdcall;
      function _AddRef: Longint; stdcall;
      function _Release: Longint; stdcall;

      function GetNewStorage(out stg: IStorage): HResult; stdcall;
      function GetInPlaceContext(out Frame: IOleInPlaceFrame; out Doc: IOleInPlaceUIWindow; lpFrameInfo: POleInPlaceFrameInfo): HResult; stdcall;
      function GetClipboardData(const chrg: TCharRange; reco: DWORD; out dataobj: IDataObject): HResult; stdcall;
      function GetContextMenu(seltype: Word; const oleobj: IOleObject; const chrg: TCharRange; out menu: HMENU): HResult; stdcall;
      function ShowContainerUI(fShow: BOOL): HResult; stdcall;
      function QueryInsertObject(const clsid: TCLSID; const stg: IStorage; cp: Longint): HResult;  stdcall;
      function DeleteObject(const oleobj: IOleObject): HResult;  stdcall;
      function QueryAcceptData(const dataobj: IDataObject; var cfFormat: TClipFormat; reco: DWORD; fReally: BOOL; hMetaPict: HGLOBAL): HResult;  stdcall;
      function ContextSensitiveHelp(fEnterMode: BOOL): HResult;  stdcall;
      function GetDragDropEffect(fDrag: BOOL; grfKeyState: DWORD; var dwEffect: DWORD): HResult;  stdcall;
  end;

function RichEdit_SetOleCallback(Wnd: HWND; const Intf: IRichEditOleCallback): Boolean;
function RichEdit_GetOleInterface(Wnd: HWND; out Intf: IRichEditOle): Boolean;
function RichEdit_InsertBitmap(Wnd: HWND; Bitmap: hBitmap; cp: Cardinal): Boolean;

procedure OleCheck(OleResult: HResult);
procedure ReleaseObject(var Obj);

procedure InitRichEditLibrary;

const
  RichEditClass:pAnsiChar = nil;

implementation

{
type
  EOleError = class(Exception);

const
  SOleError = 'OLE2 error occured. Error code: %.8xH';
}
{ OLE Specific }

function FailedHR(hr: HResult): Boolean;
begin
  Result := Failed(hr);
end;

function OleErrorMsg(ErrorCode: HResult): String;
begin
  Result:='';
//!!  FmtStr(Result, SOleError, [Longint(ErrorCode)]);
end;

procedure OleError(ErrorCode: HResult);
begin
//!!  raise EOleError.Create(OleErrorMsg(ErrorCode));
end;

procedure OleCheck(OleResult: HResult);
begin
  if FailedHR(OleResult) then OleError(OleResult);
//  if not Succeeded(OleResult) then OleError(OleResult);
end;

procedure ReleaseObject(var Obj);
begin
  if IUnknown(Obj) <> nil then IUnknown(Obj) := nil;
end;

procedure CreateStorage(var Storage: IStorage);
var
  LockBytes: ILockBytes;
begin
  OleCheck(CreateILockBytesOnHGlobal(0, True, LockBytes));
  try
    OleCheck(StgCreateDocfileOnILockBytes(LockBytes,
      STGM_READWRITE or STGM_SHARE_EXCLUSIVE or STGM_CREATE, 0, Storage));
  finally
    ReleaseObject(LockBytes);
  end;
end;

{ TRichEditOleCallback }

constructor TRichEditOleCallback.Create;
begin
  inherited Create;
end;

destructor TRichEditOleCallback.Destroy;
begin
  inherited Destroy;
end;

function TRichEditOleCallback.QueryInterface(
{$IFDEF FPC_HAS_CONSTREF}constref{$ELSE}const{$ENDIF} iid: TGUID; out Obj): HResult; stdcall;
begin
  if GetInterface(iid, Obj) then
    Result := S_OK
  else
    Result := E_NOINTERFACE;
end;

function TRichEditOleCallback._AddRef: Longint;
begin
  Inc(FRefCount);
  Result := FRefCount;
end;

function TRichEditOleCallback._Release: Longint;
begin
  Dec(FRefCount);
  Result := FRefCount;
end;

function TRichEditOleCallback.GetNewStorage(out stg: IStorage): HResult;
begin
  try
    CreateStorage(stg);
    Result := S_OK;
  except
    Result:= E_OUTOFMEMORY;
  end;
{
  OleCheck(CreateILockBytesOnHGlobal(0, True, LockBytes));
  OleCheck(StgCreateDocfileOnILockBytes(LockBytes,
    STGM_READWRITE or STGM_SHARE_EXCLUSIVE or STGM_CREATE, 0, Result));
}
end;

function TRichEditOleCallback.GetInPlaceContext(
  out Frame: IOleInPlaceFrame; out Doc: IOleInPlaceUIWindow;
  lpFrameInfo: POleInPlaceFrameInfo): HResult;
begin
  Result := E_NOTIMPL;
{
  Doc := nil; //Document window is same as frame window
  FrameInfo.hWndFrame     := 0; // Form.Handle;
  FrameInfo.fMDIApp       := False;
  FrameInfo.hAccel        := 0;
  FrameInfo.cAccelEntries := 0;
}
end;

function TRichEditOleCallback.QueryInsertObject(const clsid: TCLSID; const stg: IStorage; cp: Longint): HResult;
begin
  Result := NOERROR;
end;

const
  OLECLOSE_NOSAVE = 1;

function TRichEditOleCallback.DeleteObject(const oleobj: IOleObject): HResult;
begin
  if Assigned(oleobj) then oleobj.Close(OLECLOSE_NOSAVE);
  Result := NOERROR;
end;

function TRichEditOleCallback.QueryAcceptData(const dataobj: IDataObject; var cfFormat: TClipFormat; reco: DWORD; fReally: BOOL; hMetaPict: HGLOBAL): HResult;
begin
  Result := S_OK;
end;

function TRichEditOleCallback.ContextSensitiveHelp(fEnterMode: BOOL): HResult;
begin
  Result := E_NOTIMPL;
end;

function TRichEditOleCallback.GetClipboardData(const chrg: TCharRange; reco: DWORD; out dataobj: IDataObject): HResult;
begin
  Result := E_NOTIMPL;
end;

function TRichEditOleCallback.GetDragDropEffect(fDrag: BOOL; grfKeyState: DWORD; var dwEffect: DWORD): HResult;
begin
  Result := E_NOTIMPL;
end;

function TRichEditOleCallback.GetContextMenu(seltype: Word; const oleobj: IOleObject; const chrg: TCharRange; out menu: HMENU): HResult;
begin
  Result := E_NOTIMPL;
{
  Menu := 0
}
end;

function TRichEditOleCallback.ShowContainerUI(fShow: BOOL): HResult;
begin
  Result := E_NOTIMPL;
end;


function RichEdit_SetOleCallback(Wnd: HWND; const Intf: IRichEditOleCallback): Boolean;
begin
  Result := SendMessage(Wnd, EM_SETOLECALLBACK, 0, LPARAM(Intf)) <> 0;
end;

function RichEdit_GetOleInterface(Wnd: HWND; out Intf: IRichEditOle): Boolean;
begin
  Result := SendMessage(Wnd, EM_GETOLEINTERFACE, 0, LPARAM(@Intf)) <> 0;
end;

{ TImageDataObject }

{$IFDEF FPC}
function TImageDataObject.DAdvise(const formatetc: TFormatEtc; advf: dword; const advSink: IAdviseSink; out dwConnection: dword): HResult;
{$ELSE}
function TImageDataObject.DAdvise(const formatetc: TFormatEtc; advf: longint; const advSink: IAdviseSink; out dwConnection: longint): HResult;
{$ENDIF}
begin
  Result := E_NOTIMPL;
end;

{$IFDEF FPC}
function TImageDataObject.DUnadvise(dwConnection: dword): HResult;
{$ELSE}
function TImageDataObject.DUnadvise(dwConnection: longint): HResult;
{$ENDIF}
begin
  Result := E_NOTIMPL;
end;

function TImageDataObject.EnumDAdvise(out enumAdvise: IEnumStatData): HResult;
begin
  Result := E_NOTIMPL;
end;

{$IFDEF FPC}
function TImageDataObject.EnumFormatEtc(dwDirection: dword; out enumFormatEtc: IEnumFormatEtc): HResult;
{$ELSE}
function TImageDataObject.EnumFormatEtc(dwDirection: longint; out enumFormatEtc: IEnumFormatEtc): HResult;
{$ENDIF}
begin
  Result := E_NOTIMPL;
end;

function TImageDataObject.GetCanonicalFormatEtc(const formatetc: TFormatEtc; out formatetcOut: TFormatEtc): HResult;
begin
  Result := E_NOTIMPL;
end;

function TImageDataObject.GetDataHere(const formatetc: TFormatEtc; out medium: TStgMedium): HResult;
begin
  Result := E_NOTIMPL;
end;

function TImageDataObject.QueryGetData(const formatetc: TFormatEtc): HResult;
begin
  Result := E_NOTIMPL;
end;

destructor TImageDataObject.Destroy;
begin
  ReleaseStgMedium(FMedium);
  inherited;
end;

function TImageDataObject.GetData(const formatetcIn: TFormatEtc; out medium: TStgMedium): HResult;
begin
  FillChar(medium,SizeOf(medium),0);
  medium.tymed         := TYMED_GDI;
  medium.hBitmap       := FMedium.hBitmap;
(*
{$IFDEF FPC}
  medium.punkForRelease := nil;
{$ELSE}
  medium.unkForRelease := nil;
{$ENDIF}
*)
  Result:=S_OK;
end;

{$IFDEF FPC}
function TImageDataObject.SetData(const formatetc: TFormatEtc; const medium: TStgMedium; fRelease: BOOL): HResult;
{$ELSE}
function TImageDataObject.SetData(const formatetc: TFormatEtc; var medium: TStgMedium; fRelease: BOOL): HResult;
{$ENDIF}
begin
  FFormatEtc := formatetc;
  FMedium := medium;
  Result:= S_OK;
end;

procedure TImageDataObject.SetBitmap(bmp: hBitmap);
var
  stgm: TStgMedium;
  fm: TFormatEtc;
begin
  FillChar(stgm,SizeOf(stgm),0);
  stgm.tymed         := TYMED_GDI;
  stgm.hBitmap       := bmp;
(*
{$IFDEF FPC}
  stgm.pUnkForRelease := nil;
{$ELSE}
  stgm.UnkForRelease := nil;
{$ENDIF}
*)
  fm.cfFormat := CF_BITMAP;
  fm.ptd      := nil;
  fm.dwAspect := DVASPECT_CONTENT;
  fm.lindex   := -1;
  fm.tymed    := TYMED_GDI;
  SetData(fm, stgm, FALSE);
end;

const
  OLERENDER_FORMAT = 2;

function TImageDataObject.GetOleObject(OleClientSite: IOleClientSite; Storage: IStorage):IOleObject;
begin
  if (FMedium.hBitmap = 0) then
    Result := nil
  else
    OleCreateStaticFromData(Self, IID_IOleObject, OLERENDER_FORMAT, @FFormatEtc, OleClientSite,
      Storage, Result);
end;

function TImageDataObject.InsertBitmap(Wnd: HWND; Bitmap: hBitmap; cp: Cardinal): Boolean;
var
  RichEditOLE: IRichEditOLE;
  OleClientSite: IOleClientSite;
  Storage: IStorage;
  OleObject: IOleObject;
  ReObject: TReObject;
  clsid: TGUID;
begin
  Result := false;
  if Bitmap = 0 then
    exit;
  if not RichEdit_GetOleInterface(Wnd, RichEditOle) then
    exit;
  FBmp := CopyImage(Bitmap, IMAGE_BITMAP, 0, 0, 0);
  try
    SetBitmap(FBmp);
    RichEditOle.GetClientSite(OleClientSite);
    Storage := nil;
    try
      CreateStorage(Storage);
      if not(Assigned(OleClientSite) and Assigned(Storage)) then
        exit;
      try
        OleObject := GetOleObject(OleClientSite, Storage);
        if OleObject = nil then
          exit;
        OleSetContainedObject(OleObject, True);
        OleObject.GetUserClassID(clsid);
        ZeroMemory(@ReObject, SizeOf(ReObject));
        ReObject.cbStruct := SizeOf(ReObject);
        ReObject.clsid    := clsid;
        ReObject.cp       := cp;
        ReObject.dvaspect := DVASPECT_CONTENT;
        ReObject.poleobj  := OleObject;
        ReObject.polesite := OleClientSite;
        ReObject.pstg     := Storage;
        Result := (RichEditOle.InsertObject(ReObject) = NOERROR);
      finally
        ReleaseObject(OleObject);
      end;
    finally
      ReleaseObject(OleClientSite);
      ReleaseObject(Storage);
    end;
  finally
    DeleteObject(FBmp);
    ReleaseObject(RichEditOLE);
  end;
end;

function RichEdit_InsertBitmap(Wnd: HWND; Bitmap: hBitmap; cp: Cardinal): Boolean;
begin
  with TImageDataObject.Create do
  try
    Result := InsertBitmap(Wnd,Bitmap,cp);
  finally
    Free;
  end
end;

const
  RichEditLibnames: array[ 0..3 ] of PAnsiChar =
      ( 'msftedit', 'riched20', 'riched32', 'riched' );
  RichEditClasses: array[ 0..3 ] of PAnsiChar =
      ( 'RichEdit50W', 'RichEdit20A', 'RichEdit', 'RichEdit'  );

const
  FRichEditModule:THANDLE = 0;

procedure InitRichEditLibrary;
var
  SaveErrMode:integer;
  i:integer;
begin
  if FRichEditModule = 0 then
  begin
    SaveErrMode := SetErrorMode(SEM_NOOPENFILEERRORBOX or SEM_FAILCRITICALERRORS);

    for i:=0 to HIGH(RichEditLibNames) do
    begin
      FRichEditModule := LoadLibraryA(RichEditLibNames[i]);
      if FRichEditModule > HINSTANCE_ERROR then
      begin
        RichEditClass := RichEditClasses[i];
        break;
      end
      else
        FRichEditModule := 0;
    end;

    if FRichEditModule = 0 then
      RichEditClass := RichEditClasses[HIGH(RichEditClasses)];

    SetErrorMode(SaveErrMode);
  end;
end;

initialization
  InitRichEditLibrary;

finalization
  if FRichEditModule <> 0 then FreeLibrary(FRichEditModule);

end.

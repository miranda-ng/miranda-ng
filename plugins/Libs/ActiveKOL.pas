{This version is compatible with KOL 3.00+ -- VK}

unit ActiveKOL;

interface

uses
  windows, messages, KOL, ActiveX, KOLComObj, err;

{$I KOLDEF.INC}
{$IFDEF _D6orHigher}
  //{$WARN SYMBOL_DEPRECATED OFF}
  {$WARN SYMBOL_PLATFORM OFF}
  {$IFDEF _D7orHigher}
  {$WARN UNSAFE_TYPE OFF}
  {$WARN UNSAFE_CAST OFF}
  {$WARN UNSAFE_CODE OFF}
  {$ENDIF}
{$ENDIF}

{$IFNDEF _D5orHigher}
const
  sNoRunningObject = 'Unable to retrieve a pointer to a running object registered with OLE for %s/%s';
{$ENDIF}

type
  POleCtl = ^TOleCtl;

  TEventDispatch = class(TObject, IUnknown, IDispatch)
  private
    FControl: POleCtl;
  protected
    { IUnknown }
    function QueryInterface(const IID: TGUID; out Obj): HResult; stdcall;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
    { IDispatch }
    function GetTypeInfoCount(out Count: Integer): HResult; stdcall;
    function GetTypeInfo(Index, LocaleID: Integer; out TypeInfo): HResult; stdcall;
    function GetIDsOfNames(const IID: TGUID; Names: Pointer;
      NameCount, LocaleID: Integer; DispIDs: Pointer): HResult; stdcall;
    function Invoke(DispID: Integer; const IID: TGUID; LocaleID: Integer;
      Flags: Word; var Params; VarResult, ExcepInfo, ArgErr: Pointer): HResult; stdcall;
    property Control: POleCtl read FControl;
  public
    constructor Create(Control: POleCtl);
  end;

  {$IFNDEF _D5orHigher}
  TOleEnum = type Integer;
  //{$NODEFINE TOleEnum}
  {$ENDIF}

  TGetStrProc = procedure(const S: string) of object;

  TEnumValue = record
    Value: Longint;
    Ident: string;
  end;

  PEnumValueList = ^TEnumValueList;
  TEnumValueList = array[0..32767] of TEnumValue;

  PEnumPropDesc = ^TEnumPropDesc;
  TEnumPropDesc = object(TObj)
  private
    FDispID: Integer;
    FValueCount: Integer;
    FValues: PEnumValueList;
  public
    constructor Create(DispID, ValueCount: Integer;
      const TypeInfo: ITypeInfo);
    destructor Destroy; virtual;
    procedure GetStrings(Proc: TGetStrProc);
    function StringToValue(const S: string): Integer;
    function ValueToString(V: Integer): string;
  end;

  PControlData = ^TControlData;
  TControlData = record
    ClassID: TGUID;
    EventIID: TGUID;
    EventCount: Longint;
    EventDispIDs: Pointer;
    LicenseKey: Pointer;
    Flags: DWORD;
    Version: Integer;
    FontCount: Integer;
    FontIDs: PDispIDList;
    PictureCount: Integer;
    PictureIDs: PDispIDList;
    Reserved: Integer;
    InstanceCount: Integer;
    EnumPropDescs: PList;
  end;

  PControlData2 = ^TControlData2;
  TControlData2 = record
    ClassID: TGUID;
    EventIID: TGUID;
    EventCount: Longint;
    EventDispIDs: Pointer;
    LicenseKey: Pointer;
    Flags: DWORD;
    Version: Integer;
    FontCount: Integer;
    FontIDs: PDispIDList;
    PictureCount: Integer;
    PictureIDs: PDispIDList;
    Reserved: Integer;
    InstanceCount: Integer;
    EnumPropDescs: PList;
    FirstEventOfs: Cardinal;
  end;

  TOleCtlIntfClass = class of TOleCtlIntf;
  TOleCtlIntf = class( TObject, IUnknown, IOleClientSite,
    IOleControlSite, IOleInPlaceSite, IOleInPlaceFrame, IDispatch,
    IPropertyNotifySink, ISimpleFrameSite)
  private
    FRefCount: Integer;
    fOleCtl: POleCtl;
    procedure GetEventMethod(DispID: TDispID; var Method: TMethod);
  protected
    { IUnknown }
    function QueryInterface(const IID: TGUID; out Obj): HResult; stdcall; //override;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
    { IOleClientSite }
    function SaveObject: HResult; stdcall;
    function GetMoniker(dwAssign: Longint; dwWhichMoniker: Longint;
      out mk: IMoniker): HResult; stdcall;
    function GetContainer(out container: IOleContainer): HResult; stdcall;
    function ShowObject: HResult; stdcall;
    function OnShowWindow(fShow: BOOL): HResult; stdcall;
    function RequestNewObjectLayout: HResult; stdcall;
    { IOleControlSite }
    function OnControlInfoChanged: HResult; stdcall;
    function LockInPlaceActive(fLock: BOOL): HResult; stdcall;
    function GetExtendedControl(out disp: IDispatch): HResult; stdcall;
    function TransformCoords(var ptlHimetric: TPoint; var ptfContainer: TPointF;
      flags: Longint): HResult; stdcall;
    function IOleControlSite.TranslateAccelerator = OleControlSite_TranslateAccelerator;
    function OleControlSite_TranslateAccelerator(msg: PMsg;
      grfModifiers: Longint): HResult; stdcall;
    function OnFocus(fGotFocus: BOOL): HResult; stdcall;
    function ShowPropertyFrame: HResult; stdcall;
    { IOleWindow }
    function ContextSensitiveHelp(fEnterMode: BOOL): HResult; stdcall;
    { IOleInPlaceSite }
    function IOleInPlaceSite.GetWindow = OleInPlaceSite_GetWindow;
    function OleInPlaceSite_GetWindow(out wnd: HWnd): HResult; stdcall;
    function CanInPlaceActivate: HResult; stdcall;
    function OnInPlaceActivate: HResult; stdcall;
    function OnUIActivate: HResult; stdcall;
    function GetWindowContext(out frame: IOleInPlaceFrame;
      out doc: IOleInPlaceUIWindow; out rcPosRect: TRect;
      out rcClipRect: TRect; out frameInfo: TOleInPlaceFrameInfo): HResult;
      stdcall;
    function Scroll(scrollExtent: TPoint): HResult; stdcall;
    function OnUIDeactivate(fUndoable: BOOL): HResult; stdcall;
    function OnInPlaceDeactivate: HResult; stdcall;
    function DiscardUndoState: HResult; stdcall;
    function DeactivateAndUndo: HResult; stdcall;
    function OnPosRectChange(const rcPosRect: TRect): HResult; stdcall;
    { IOleInPlaceUIWindow }
    function GetBorder(out rectBorder: TRect): HResult; stdcall;
    function RequestBorderSpace(const borderwidths: TRect): HResult; stdcall;
    function SetBorderSpace(pborderwidths: PRect): HResult; stdcall;
    function SetActiveObject(const activeObject: IOleInPlaceActiveObject;
      pszObjName: POleStr): HResult; stdcall;
    { IOleInPlaceFrame }
    function IOleInPlaceFrame.GetWindow = OleInPlaceFrame_GetWindow;
    function OleInPlaceFrame_GetWindow(out wnd: HWnd): HResult; stdcall;
    function InsertMenus(hmenuShared: HMenu;
      var menuWidths: TOleMenuGroupWidths): HResult; stdcall;
    function SetMenu(hmenuShared: HMenu; holemenu: HMenu;
      hwndActiveObject: HWnd): HResult; stdcall;
    function RemoveMenus(hmenuShared: HMenu): HResult; stdcall;
    function SetStatusText(pszStatusText: POleStr): HResult; stdcall;
    function EnableModeless(fEnable: BOOL): HResult; stdcall;
    function IOleInPlaceFrame.TranslateAccelerator = OleInPlaceFrame_TranslateAccelerator;
    function OleInPlaceFrame_TranslateAccelerator(var msg: Windows.TMsg;
      wID: Word): HResult; stdcall;
    { IDispatch }
    function GetTypeInfoCount(out Count: Integer): HResult; stdcall;
    function GetTypeInfo(Index, LocaleID: Integer; out TypeInfo): HResult; stdcall;
    function GetIDsOfNames(const IID: TGUID; Names: Pointer;
      NameCount, LocaleID: Integer; DispIDs: Pointer): HResult; stdcall;
    function Invoke(DispID: Integer; const IID: TGUID; LocaleID: Integer;
      Flags: Word; var Params; VarResult, ExcepInfo, ArgErr: Pointer): HResult; stdcall;
    { ISimpleFrameSite }
    function PreMessageFilter(wnd: HWnd; msg, wp, lp: Integer;
      out res: Integer; out Cookie: Longint): HResult; stdcall;
    function PostMessageFilter(wnd: HWnd; msg, wp, lp: Integer;
      out res: Integer; Cookie: Longint): HResult; stdcall;
    { IPropertyNotifySink }
    function OnChanged(dispid: TDispID): HResult; virtual; stdcall;
    function OnRequestEdit(dispid: TDispID): HResult; virtual; stdcall;
  public
    property OleCtl: POleCtl read fOleCtl;
    constructor Create; virtual;
  end;


  TOnGetIntfClass = function(): TOleCtlIntfClass of object;


  TOleCtl = object( TControl )
  private
    FOnGetIntfClass: TOnGetIntfClass;
    function GetOleObject: Variant;
    procedure CreateInstance;
    function GetOnLeave: TOnEvent;
    procedure SetOnLeave(const Value: TOnEvent);
    procedure HookControlWndProc;
    procedure SetUIActive(Active: Boolean);
    procedure CreateControl;
    procedure DestroyStorage;
    procedure DestroyControl;
    procedure StandardEvent(DispID: TDispID; var Params: TDispParams);
    //procedure SetMouseDblClk(const Value: TOnMouse);
    procedure SetOnChar(const Value: TOnChar);
  protected
    //{$IFDEF DELPHI_CODECOMPLETION_BUG}
    fNotAvailable: Boolean;
    //{$ENDIF}
    {$IFNDEF USE_NAMES}
    fName: String;
    {$ENDIF}
    FControlData: PControlData;
    FOleObject: IOleObject;
    FMiscStatus: Longint;
    FFonts: PList;
    FPictures: PList;
    FEventDispatch: TEventDispatch;
    fOleCtlIntf: TOleCtlIntf;
    FPersistStream: IPersistStreamInit;
    FOleInPlaceObject: IOleInPlaceObject;
    FOleInPlaceActiveObject: IOleInPlaceActiveObject;
    FOleControl: IOleControl;
    FUpdatingColor: Boolean;
    FUpdatingFont: Boolean;
    FUpdatingEnabled: Boolean;
    FObjectData: HGlobal;
    FControlDispatch: IDispatch;
    FPropBrowsing: IPerPropertyBrowsing;
    FPropConnection: Longint;
    FEventsConnection: Longint;
    fCreatingWnd: Boolean;
    procedure Init; virtual;
    procedure InitControlData; virtual;
    procedure InitControlInterface(const Obj: IUnknown); virtual;
    property ControlData: PControlData read FControlData write FControlData;
    function GetMainMenu: HMenu;
    procedure InvokeEvent(DispID: TDispID; var Params: TDispParams);
    procedure D2InvokeEvent(DispID: TDispID; var Params: TDispParams);
    procedure DoHandleException;
    procedure CreateEnumPropDescs;
    procedure DestroyEnumPropDescs;

    property OnGetIntfClass: TOnGetIntfClass read FOnGetIntfClass write FOnGetIntfClass;
  public
    function GetByteProp(Index: Integer): Byte;
    function GetColorProp(Index: Integer): TColor;
    function GetTColorProp(Index: Integer): TColor;
    function GetCompProp(Index: Integer): Comp;
    function GetCurrencyProp(Index: Integer): Currency;
    function GetDoubleProp(Index: Integer): Double;
    function GetIDispatchProp(Index: Integer): IDispatch;
    function GetIntegerProp(Index: Integer): Integer;
    function GetIUnknownProp(Index: Integer): IUnknown;
    function GetWordBoolProp(Index: Integer): WordBool;
    function GetTDateTimeProp(Index: Integer): TDateTime;
    function GetTFontProp(Index: Integer): PGraphicTool;
    function GetOleBoolProp(Index: Integer): TOleBool;
    function GetOleDateProp(Index: Integer): TOleDate;
    function GetOleEnumProp(Index: Integer): TOleEnum;
    function GetTOleEnumProp(Index: Integer): TOleEnum;
    function GetOleVariantProp(Index: Integer): OleVariant;
    //function GetTPictureProp(Index: Integer): TPicture;
    procedure GetProperty(Index: Integer; var Value: TVarData);
    function GetShortIntProp(Index: Integer): ShortInt;
    function GetSingleProp(Index: Integer): Single;
    function GetSmallintProp(Index: Integer): Smallint;
    function GetStringProp(Index: Integer): string;
    function GetVariantProp(Index: Integer): Variant;
    function GetWideStringProp(Index: Integer): WideString;
    function GetWordProp(Index: Integer): Word;
    procedure SetByteProp(Index: Integer; Value: Byte);
    procedure SetColorProp(Index: Integer; Value: TColor);
    procedure SetTColorProp(Index: Integer; Value: TColor);
    procedure SetCompProp(Index: Integer; const Value: Comp);
    procedure SetCurrencyProp(Index: Integer; const Value: Currency);
    procedure SetDoubleProp(Index: Integer; const Value: Double);
    procedure SetIDispatchProp(Index: Integer; const Value: IDispatch);
    procedure SetIntegerProp(Index: Integer; Value: Integer);
    procedure SetIUnknownProp(Index: Integer; const Value: IUnknown);
    procedure SetName(const Value: String); virtual;
    procedure SetWordBoolProp(Index: Integer; Value: WordBool);
    procedure SetTDateTimeProp(Index: Integer; const Value: TDateTime);
    procedure SetTFontProp(Index: Integer; Value:PGraphicTool);
    procedure SetOleBoolProp(Index: Integer; Value: TOleBool);
    procedure SetOleDateProp(Index: Integer; const Value: TOleDate);
    procedure SetOleEnumProp(Index: Integer; Value: TOleEnum);
    procedure SetTOleEnumProp(Index: Integer; Value: TOleEnum);
    procedure SetOleVariantProp(Index: Integer; const Value: OleVariant);
    procedure SetParent(AParent: PControl); virtual;
    //procedure SetTPictureProp(Index: Integer;  Value: TPicture);
    procedure SetProperty(Index: Integer; const Value: TVarData);
    procedure SetShortIntProp(Index: Integer; Value: Shortint);
    procedure SetSingleProp(Index: Integer; const Value: Single);
    procedure SetSmallintProp(Index: Integer; Value: Smallint);
    procedure SetStringProp(Index: Integer; const Value: string);
    procedure SetVariantProp(Index: Integer; const Value: Variant);
    procedure SetWideStringProp(Index: Integer; const Value: WideString);
    procedure SetWordProp(Index: Integer; Value: Word);

    function GetEnumPropDesc(DispID: Integer): PEnumPropDesc;

    property  DragCursor: Boolean read fNotAvailable;
    property  DragMode  : Boolean read fNotAvailable;
    property  ParentShowHint: Boolean read fNotAvailable;
    property  PopupMenu: Boolean read fNotAvailable;
    property  ShowHint: Boolean read fNotAvailable;
    property  OnDragDrop: Boolean read fNotAvailable;
    property  OnDragOver: Boolean read fNotAvailable;
    property  OnEndDrag: Boolean read fNotAvailable;
    property  OnStartDrag: Boolean read fNotAvailable;

    property  OnExit: TOnEvent read GetOnLeave write SetOnLeave;
    property  OleObject: Variant read GetOleObject;

    property  Name: String read fName write fName;
    function CreateWindow: Boolean; virtual;
    procedure DblClk;
    procedure KeyDown(var Key: Longint; AShift: DWORD);
    procedure KeyUp(var Key: Longint; AShift: DWORD);
    procedure KeyPress(var Key: KOLChar);
    procedure MouseDown(Button: TMouseButton; AShift: DWORD;
      X, Y: Integer);
    procedure MouseMove(AShift: DWORD; X, Y: Integer);
    procedure MouseUp(Button: TMouseButton; AShift: DWORD;
      X, Y: Integer);

    property OnKeyPress: TOnChar
             read {$IFDEF EVENTS_DYNAMIC} Get_OnChar {$ELSE} EV.fOnChar {$ENDIF}
             write SetOnChar;
    property OnDblClick: TOnMouse index idx_fOnMouseDblClk
             read {$IFDEF EVENTS_DYNAMIC} Get_OnMouseEvent {$ELSE} EV.fOnMouseDblClk {$ENDIF}
             write SetOnMouseEvent; // SetMouseDblClk;

    destructor Destroy; virtual;

  end;

{$IFNDEF _D2orD3}
type
  TVariantArray = Array of OleVariant;
  TOleServer    = class;
  TConnectKind  = (ckRunningOrNew,          // Attach to a running or create a new instance of the server
                   ckNewInstance,           // Create a new instance of the server
                   ckRunningInstance,       // Attach to a running instance of the server
                   ckRemote,                // Bind to a remote instance of the server
                   ckAttachToInterface);    // Don't bind to server, user will provide interface via 'CpnnectTo'

  TServerEventDispatch = class(TObject, IUnknown, IDispatch)
  private
    FServer: TOleServer;
    InternalRefCount : Integer;
  protected
    { IUnknown }
    function QueryInterface(const IID: TGUID; out Obj): HResult; stdcall;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
    { IDispatch }
    function GetTypeInfoCount(out Count: Integer): HResult; stdcall;
    function GetTypeInfo(Index, LocaleID: Integer; out TypeInfo): HResult; stdcall;
    function GetIDsOfNames(const IID: TGUID; Names: Pointer;
      NameCount, LocaleID: Integer; DispIDs: Pointer): HResult; stdcall;
    function Invoke(DispID: Integer; const IID: TGUID; LocaleID: Integer;
      Flags: Word; var Params; VarResult, ExcepInfo, ArgErr: Pointer): HResult; stdcall;
    property Server: TOleServer read FServer;
    function ServerDisconnect :Boolean;
  public
    constructor Create(Server: TOleServer);
  end;

  PServerData = ^TServerData;
  TServerData = record
    ClassID: TGUID;                   // CLSID of CoClass
    IntfIID: TGUID;                   // IID of default interface
    EventIID: TGUID;                  // IID of default source interface
    LicenseKey: Pointer;              // Pointer to license string (not implemented)
    Version: Integer;                 // Version of this structure
    InstanceCount: Integer;           // Instance of the Server running
  end;

  TOleServer = class(TObject, IUnknown)
  private
    FServerData:        PServerData;
    FRefCount:          Longint;
    FEventDispatch:     TServerEventDispatch;
    FEventsConnection:  Longint;
    FAutoConnect:       Boolean;
    FRemoteMachineName: string;
    FConnectKind:       TConnectKind;

  protected
      { IUnknown }
    function QueryInterface(const IID: TGUID; out Obj): HResult; stdcall; //override;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;

    procedure Loaded; //override;
    procedure InitServerData; virtual; abstract;

    function  GetServer: IUnknown; virtual;

    procedure ConnectEvents(const Obj: IUnknown);
    procedure DisconnectEvents(const Obj: Iunknown);
    procedure InvokeEvent(DispID: TDispID; var Params: TVariantArray); virtual;

    function  GetConnectKind: TConnectKind;
    procedure SetConnectKind(ck: TConnectKind);

    function  GetAutoConnect: Boolean;
    procedure SetAutoConnect(flag: Boolean);

    property  ServerData: PServerData read FServerData write FServerData;
    property  EventDispatch: TServerEventDispatch read FEventDispatch write FEventDispatch;

  public
    constructor Create; //(AOwner: TComponent); override;
    destructor Destroy; override;

    // NOTE: If derived class is generated by TLIBIMP or ImportTypeLibraryCodeGenerator,
    //       the derived class will also expose a 'ConnectTo(interface)' function.
    //       You must invoke that method if you're using 'ckAttachToInterface' connection
    //       kind.
    procedure Connect; virtual; abstract;
    procedure Disconnect; virtual; abstract;

  published
    property AutoConnect: Boolean read GetAutoConnect write SetAutoConnect;
    property ConnectKind: TConnectKind read GetConnectKind write SetConnectKind;
    property RemoteMachineName: string read FRemoteMachineName write FRemoteMachineName;
  end;
{$ENDIF}

var
  EmptyParam: OleVariant; { "Empty parameter" standard constant which can be
                            passed as an optional parameter on a dual interface. }


implementation

uses
  OleConst;

const
  // The following flags may be or'd into the TControlData.Reserved field to override
  // default behaviors.

  // cdForceSetClientSite:
  //   Call SetClientSite early (in constructor) regardless of misc status flags
  cdForceSetClientSite = 1;

  // cdDeferSetClientSite:
  //   Don't call SetClientSite early.  Takes precedence over cdForceSetClientSite and misc status flags
  cdDeferSetClientSite = 2;

const
  cfBackColor = $00000001;
  cfForeColor = $00000002;
  cfFont      = $00000004;
  cfEnabled   = $00000008;
  cfCaption   = $00000010;
  cfText      = $00000020;

const
  MaxDispArgs = 32;

type

  PDispInfo = ^TDispInfo;
  TDispInfo = packed record
    DispID: TDispID;
    ResType: Byte;
    CallDesc: TCallDesc;
  end;

  TArgKind = (akDWord, akSingle, akDouble);

  PEventArg = ^TEventArg;
  TEventArg = record
    Kind: TArgKind;
    Data: array[0..1] of Integer;
  end;

  TEventInfo = record
    Method: TMethod;
    Sender: TObject;
    ArgCount: Integer;
    Args: array[0..MaxDispArgs - 1] of TEventArg;
  end;

function StringToVarOleStr(const S: string): Variant;
begin
  VarClear(Result);
  TVarData(Result).VOleStr := StringToOleStr(S);
  TVarData(Result).VType := varOleStr;
end;

{ TEnumPropDesc }

constructor TEnumPropDesc.Create(DispID, ValueCount: Integer;
  const TypeInfo: ITypeInfo);
var
  I: Integer;
  VarDesc: PVarDesc;
  XName: WideString;
begin
  FDispID := DispID;
  FValueCount := ValueCount;
  FValues := AllocMem(ValueCount * SizeOf(TEnumValue));
  for I := 0 to ValueCount - 1 do
  begin
    OleCheck(TypeInfo.GetVarDesc(I, VarDesc));
    try
      OleCheck(TypeInfo.GetDocumentation(VarDesc^.memid, @XName,
        nil, nil, nil));
      with FValues^[I] do
      begin
        Value := TVarData(VarDesc^.lpVarValue^).VInteger;
        Ident := XName;
        while (Length(Ident) > 1) and (Ident[1] = '_') do
          Delete(Ident, 1, 1);
      end;
    finally
      TypeInfo.ReleaseVarDesc(VarDesc);
    end;
  end;
end;

destructor TEnumPropDesc.Destroy;
begin
  if FValues <> nil then
  begin
    Finalize(FValues^[0], FValueCount);
    FreeMem(FValues, FValueCount * SizeOf(TEnumValue));
  end;
  inherited;
end;

procedure TEnumPropDesc.GetStrings(Proc: TGetStrProc);
var
  I: Integer;
begin
  for I := 0 to FValueCount - 1 do
    with FValues^[I] do Proc(Format('%d - %s', [Value, Ident]));
end;

function TEnumPropDesc.StringToValue(const S: string): Integer;
var
  I: Integer;
begin
  I := 1;
  while (I <= Length(S)) and (S[I] in ['0'..'9', '-']) do Inc(I);
  if I > 1 then
  begin
    Result := Str2Int(Copy(S, 1, I - 1));
    for I := 0 to FValueCount - 1 do
      if Result = FValues^[I].Value then Exit;
  end else
    for I := 0 to FValueCount - 1 do
      with FValues^[I] do
        if AnsiCompareText(S, Ident) = 0 then
        begin
          Result := Value;
          Exit;
        end;
  raise EOleError.CreateResFmt(e_Ole, Integer( @SBadPropValue ), [S]);
end;

function TEnumPropDesc.ValueToString(V: Integer): string;
var
  I: Integer;
begin
  for I := 0 to FValueCount - 1 do
    with FValues^[I] do
      if V = Value then
      begin
        Result := Format('%d - %s', [Value, Ident]);
        Exit;
      end;
  Result := Int2Str(V);
end;

{ TOleCtl }

procedure TOleCtl.CreateControl;
var
  Stream: IStream;
  CS: IOleClientSite;
  X: Integer;
begin
  if FOleControl = nil then
    try
      try  // work around ATL bug
        X := FOleObject.GetClientSite(CS);
      except
        X := -1;
      end;
      if (X <> 0) or (CS = nil) then
        OleCheck(FOleObject.SetClientSite(fOleCtlIntf));
      if FObjectData = 0 then OleCheck(FPersistStream.InitNew) else
      begin
        OleCheck(CreateStreamOnHGlobal(FObjectData, False, Stream));
        OleCheck(FPersistStream.Load(Stream));
        DestroyStorage;
      end;
      OleCheck(FOleObject.QueryInterface(IOleControl, FOleControl));
      OleCheck(FOleObject.QueryInterface(IDispatch, FControlDispatch));
      FOleObject.QueryInterface(IPerPropertyBrowsing, FPropBrowsing);
      InterfaceConnect(FOleObject, IPropertyNotifySink,
        fOleCtlIntf, FPropConnection);
      InterfaceConnect(FOleObject, FControlData^.EventIID,
        FEventDispatch, FEventsConnection);
      if FControlData^.Flags and cfBackColor <> 0 then
        fOleCtlIntf.OnChanged(DISPID_BACKCOLOR);
      if FControlData^.Flags and cfEnabled <> 0 then
        fOleCtlIntf.OnChanged(DISPID_ENABLED);
      if FControlData^.Flags and cfFont <> 0 then
        fOleCtlIntf.OnChanged(DISPID_FONT);
      if FControlData^.Flags and cfForeColor <> 0 then
        fOleCtlIntf.OnChanged(DISPID_FORECOLOR);
      FOleControl.OnAmbientPropertyChange(DISPID_UNKNOWN);
      fOleCtlIntf.RequestNewObjectLayout;
    except
      DestroyControl;
      raise;
    end;
end;

procedure TOleCtl.CreateEnumPropDescs;

  function FindMember(DispId: Integer): Boolean;
  begin
    Result := GetEnumPropDesc(DispId) <> nil;
  end;
  {var
    I: Integer;
  begin
    for I := 0 to FControlData^.EnumPropDescs.Count - 1 do
      if TEnumPropDesc(FControlData^.EnumPropDescs).FDispID = DispID then
      begin
        Result := True;
        Exit;
      end;
    Result := False;
  end;}

  procedure CreateEnum(TypeDesc: TTypeDesc; const TypeInfo: ITypeInfo;
    DispId: Integer);
  var
    RefInfo: ITypeInfo;
    RefAttr: PTypeAttr;
    epd: PEnumPropDesc;
  begin
    if TypeDesc.vt <> VT_USERDEFINED then Exit;
    OleCheck(TypeInfo.GetRefTypeInfo(TypeDesc.hreftype, RefInfo));
    OleCheck(RefInfo.GetTypeAttr(RefAttr));
    try
      if RefAttr^.typekind = TKIND_ENUM then
      begin
        new( epd, Create(Dispid, RefAttr^.cVars, RefInfo) );
        FControlData^.EnumPropDescs.Add( epd );
      end;
    finally
      RefInfo.ReleaseTypeAttr(RefAttr);
    end;
  end;

  procedure ProcessTypeInfo(const TypeInfo: ITypeInfo);
  var
    I: Integer;
    RefInfo: ITypeInfo;
    TypeAttr: PTypeAttr;
    VarDesc: PVarDesc;
    FuncDesc: PFuncDesc;
    RefType: HRefType;
  begin
    OleCheck(TypeInfo.GetTypeAttr(TypeAttr));
    try
      if IsEqualGUID(TypeAttr^.guid, IDispatch) then Exit;
      if ((TypeAttr.typekind = TKIND_INTERFACE) or
        (TypeAttr.wTypeFlags and TYPEFLAG_FDUAL <> 0)) and
        (TypeAttr.wTypeFlags and TYPEFLAG_FNONEXTENSIBLE <> 0) then
      begin
        OleCheck(TypeInfo.GetRefTypeOfImplType(0, RefType));
        OleCheck(TypeInfo.GetRefTypeInfo(RefType, RefInfo));
        ProcessTypeInfo(RefInfo);
      end;
      for I := 0 to TypeAttr^.cVars - 1 do
      begin
        OleCheck(TypeInfo.GetVarDesc(I, VarDesc));
        try
          CreateEnum(VarDesc^.elemdescVar.tdesc, TypeInfo, VarDesc^.memid);
        finally
          TypeInfo.ReleaseVarDesc(VarDesc);
        end;
      end;
      for I := 0 to TypeAttr^.cFuncs - 1 do
      begin
        OleCheck(TypeInfo.GetFuncDesc(I, FuncDesc));
        try
          if not FindMember(FuncDesc^.memid) then
            case FuncDesc^.invkind of
              INVOKE_PROPERTYGET:
                CreateEnum(FuncDesc^.elemdescFunc.tdesc, TypeInfo, FuncDesc^.memid);
              INVOKE_PROPERTYPUT:
                CreateEnum(FuncDesc^.lprgelemdescParam[FuncDesc.cParams - 1].tdesc,
                  TypeInfo, FuncDesc^.memid);
              INVOKE_PROPERTYPUTREF:
                if FuncDesc^.lprgelemdescParam[FuncDesc.cParams - 1].tdesc.vt = VT_PTR then
                  CreateEnum(FuncDesc^.lprgelemdescParam[FuncDesc.cParams - 1].tdesc.ptdesc^,
                    TypeInfo, FuncDesc^.memid);
            end;
        finally
          TypeInfo.ReleaseFuncDesc(FuncDesc);
        end;
      end;
    finally
      TypeInfo.ReleaseTypeAttr(TypeAttr);
    end;
  end;

var
  TypeInfo: ITypeInfo;
begin
  CreateControl;
  FControlData^.EnumPropDescs := NewList;
  try
    OleCheck(FControlDispatch.GetTypeInfo(0, 0, TypeInfo));
    ProcessTypeInfo(TypeInfo);
  except
    DestroyEnumPropDescs;
    raise;
  end;
end;

procedure TOleCtl.CreateInstance;
var
  ClassFactory2: IClassFactory2;
  LicKeyStr: WideString;

  procedure LicenseCheck(Status: HResult; const Ident: string);
  begin
    if Status = CLASS_E_NOTLICENSED then
      raise EOleError.CreateFmt(e_Ole, Ident, [SubClassName]);
    OleCheck(Status);
  end;

begin
  if (FControlData^.LicenseKey <> nil) then
  begin
    OleCheck(CoGetClassObject(FControlData^.ClassID, CLSCTX_INPROC_SERVER or
      CLSCTX_LOCAL_SERVER, nil, IClassFactory2, ClassFactory2));
    LicKeyStr := PWideChar(FControlData^.LicenseKey);
    LicenseCheck(ClassFactory2.CreateInstanceLic(nil, nil, IOleObject,
      LicKeyStr, FOleObject), SInvalidLicense);
  end else
    LicenseCheck(CoCreateInstance(FControlData^.ClassID, nil,
      CLSCTX_INPROC_SERVER or CLSCTX_LOCAL_SERVER, IOleObject,
      FOleObject), SNotLicensed);
end;

procedure CallEventMethod(const EventInfo: TEventInfo);
asm
        PUSH    EBX
        PUSH    ESI
        PUSH    EBP
        MOV     EBP,ESP
        MOV     EBX,EAX
        MOV     EDX,[EBX].TEventInfo.ArgCount
        TEST    EDX,EDX
        JE      @@5
        XOR     EAX,EAX
        LEA     ESI,[EBX].TEventInfo.Args
@@1:    MOV     AL,[ESI].TEventArg.Kind
        CMP     AL,1
        JA      @@2
        JE      @@3
        TEST    AH,AH
        JNE     @@3
        MOV     ECX,[ESI].Integer[4]
        MOV     AH,1
        JMP     @@4
@@2:    PUSH    [ESI].Integer[8]
@@3:    PUSH    [ESI].Integer[4]
@@4:    ADD     ESI,12
        DEC     EDX
        JNE     @@1
@@5:    MOV     EDX,[EBX].TEventInfo.Sender
        MOV     EAX,[EBX].TEventInfo.Method.Data
        CALL    [EBX].TEventInfo.Method.Code
        MOV     ESP,EBP
        POP     EBP
        POP     ESI
        POP     EBX
end;

type
  PVarArg = ^TVarArg;
  TVarArg = array[0..3] of DWORD;

function TOleCtl.CreateWindow: Boolean;
begin
  Result := FALSE;
  if fHandle <> 0 then
  begin
    Result := TRUE;
    Exit;
  end;
  if fCreatingWnd then
    Exit;
  fCreatingWnd := TRUE;
  try
    CreateControl;
    if FMiscStatus and OLEMISC_INVISIBLEATRUNTIME = 0 then
    begin
      FOleObject.DoVerb(OLEIVERB_INPLACEACTIVATE, nil, fOleCtlIntf, 0,
        ParentWindow, BoundsRect);
      if FOleInPlaceObject = nil then
        raise EOleError.CreateResFmt(e_Ole, Integer( @SCannotActivate ), [nil]);
      HookControlWndProc;
      if  {$IFDEF USE_FLAGS} not(F3_Visible in fStyle.f3_Style)
          {$ELSE} not fVisible {$ENDIF}
      and IsWindowVisible(fHandle) then
          ShowWindow(fHandle, SW_HIDE);
      Result := TRUE;
    end
      else
      Result := inherited CreateWindow;
  finally
    fCreatingWnd := FALSE;
  end;
end;

procedure TOleCtl.D2InvokeEvent(DispID: TDispID; var Params: TDispParams);
type
  TStringDesc = record
    PStr: Pointer;
    BStr: PBStr;
  end;
var
  I, J, K, ArgType, ArgCount, StrCount: Integer;
  ArgPtr: PEventArg;
  ParamPtr: PVarArg;
  Strings: array[0..MaxDispArgs - 1] of TStringDesc;
  EventInfo: TEventInfo;
begin
  fOleCtlIntf.GetEventMethod(DispID, EventInfo.Method);
  if Integer(EventInfo.Method.Code) >= $10000 then
  begin
    StrCount := 0;
    try
      ArgCount := Params.cArgs;
      EventInfo.Sender := fOleCtlIntf;
      EventInfo.ArgCount := ArgCount;
      if ArgCount <> 0 then
      begin
        ParamPtr := @Params.rgvarg^[EventInfo.ArgCount];
        ArgPtr := @EventInfo.Args;
        I := 0;
        repeat
          Dec(Integer(ParamPtr), SizeOf(TVarArg));
          ArgType := ParamPtr^[0] and $0000FFFF;
          if ArgType and varTypeMask = varOleStr then
          begin
            ArgPtr^.Kind := akDWord;
            with Strings[StrCount] do
            begin
              PStr := nil;
              if ArgType and varByRef <> 0 then
              begin
                OleStrToStrVar(PBStr(ParamPtr^[2])^, string(PStr));
                BStr := PBStr(ParamPtr^[2]);
                ArgPtr^.Data[0] := Integer(@PStr);
              end else
              begin
                OleStrToStrVar(TBStr(ParamPtr^[2]), string(PStr));
                BStr := nil;
                ArgPtr^.Data[0] := Integer(PStr);
              end;
            end;
            Inc(StrCount);
          end else
          begin
            case ArgType of
              varSingle:
                begin
                  ArgPtr^.Kind := akSingle;
                  ArgPtr^.Data[0] := ParamPtr^[2];
                end;
              varDouble..varDate:
                begin
                  ArgPtr^.Kind := akDouble;
                  ArgPtr^.Data[0] := ParamPtr^[2];
                  ArgPtr^.Data[1] := ParamPtr^[3];
                end;
              varDispatch:
                begin
                  ArgPtr^.Kind := akDWord;
                  ArgPtr^.Data[0] := Integer(ParamPtr)
                end;
            else
              ArgPtr^.Kind := akDWord;
              if (ArgType and varArray) <> 0 then
                ArgPtr^.Data[0] := Integer(ParamPtr)
              else
                ArgPtr^.Data[0] := ParamPtr^[2];
            end;
          end;
          Inc(Integer(ArgPtr), SizeOf(TEventArg));
          Inc(I);
        until I = EventInfo.ArgCount;
      end;
      CallEventMethod(EventInfo);
      J := StrCount;
      while J <> 0 do
      begin
        Dec(J);
        with Strings[J] do
          if BStr <> nil then BStr^ := StringToOleStr(string(PStr));
      end;
    except
      DoHandleException;
    end;
    K := StrCount;
    while K <> 0 do
    begin
      Dec(K);
      string(Strings[K].PStr) := '';
    end;
  end;
end;

procedure TOleCtl.DblClk;
var MouseData: TMouseEventData;
    P: TPoint;
begin
  {$IFDEF NIL_EVENTS}
  if Assigned(EV.fOnMouseDblClk) then
  {$ENDIF}
  begin
    MouseData.Button := mbLeft;
    MouseData.Shift := 0;
    GetCursorPos( P );
    P := Screen2Client( P );
    MouseData.X := P.x;
    MouseData.Y := P.y;
    EV.fOnMouseDblClk(@Self, MouseData);
  end;
end;

destructor TOleCtl.Destroy;

  procedure FreeList(var L: PList);
  begin
    if L = nil then Exit;
    L.Release;
    L := nil;
  end;

begin
  SetUIActive(False);
  if FOleObject <> nil then FOleObject.Close(OLECLOSE_NOSAVE);
  DestroyControl;
  DestroyStorage;
  FPersistStream := nil;
  if FOleObject <> nil then FOleObject.SetClientSite(nil);
  FOleObject := nil;
  FEventDispatch.Free;
  FreeList(FFonts);
  FreeList(FPictures);
  Dec(FControlData^.InstanceCount);
  if FControlData^.InstanceCount = 0 then
    DestroyEnumPropDescs;
  fOleCtlIntf.Free;
  inherited Destroy;
end;

procedure TOleCtl.DestroyControl;
begin
  InterfaceDisconnect(FOleObject, FControlData^.EventIID, FEventsConnection);
  InterfaceDisconnect(FOleObject, IPropertyNotifySink, FPropConnection);
  FPropBrowsing := nil;
  FControlDispatch := nil;
  FOleControl := nil;
end;

procedure TOleCtl.DestroyEnumPropDescs;
var
  I: Integer;
begin
  with FControlData^ do
    if EnumPropDescs <> nil then
    begin
      for I := 0 to EnumPropDescs.Count - 1 do
        PEnumPropDesc(EnumPropDescs.Items[I]).Free;
      EnumPropDescs.Free;
      EnumPropDescs := nil;
    end;
end;

procedure TOleCtl.DestroyStorage;
begin
  if FObjectData <> 0 then
  begin
    GlobalFree(FObjectData);
    FObjectData := 0;
  end;
end;

procedure TOleCtl.DoHandleException;
begin
  //Application.HandleException(Self);
  //TODO: replace Application.HandleException with something
end;

function TOleCtl.GetByteProp(Index: Integer): Byte;
begin
  Result := GetIntegerProp(Index);
end;

function TOleCtl.GetColorProp(Index: Integer): TColor;
begin
  Result := GetIntegerProp(Index);
end;

function TOleCtl.GetCompProp(Index: Integer): Comp;
begin
  Result := GetDoubleProp(Index);
end;

function TOleCtl.GetCurrencyProp(Index: Integer): Currency;
var
  Temp: TVarData;
begin
  GetProperty(Index, Temp);
  Result := Temp.VCurrency;
end;

function TOleCtl.GetDoubleProp(Index: Integer): Double;
var
  Temp: TVarData;
begin
  GetProperty(Index, Temp);
  Result := Temp.VDouble;
end;

procedure TOleCtlIntf.GetEventMethod(DispID: TDispID; var Method: TMethod);
{begin // test for D4 - it works...
  Method.Code := nil;
  Method.Data := nil;
end;}
const
  szOleCtl = sizeof( TOleCtl );
asm
        PUSH    EBX
        PUSH    ESI
        PUSH    EDI
        PUSH    ECX
        MOV     EBX,EAX
        MOV     ECX,[EBX].fOleCtl
        ///////////////////////// fix of events handling
        MOV     EBX, ECX       // by Alexey Izyumov
        ///////////////////////// Octouber, 2001
        MOV     ECX,[ECX].TOleCtl.FControlData
        MOV     EDI,[ECX].TControlData.EventCount
        MOV     ESI,[ECX].TControlData.EventDispIDs
        XOR     EAX,EAX
        JMP     @@1
@@0:    CMP     EDX,[ESI].Integer[EAX*4]
        JE      @@2
        INC     EAX
@@1:    CMP     EAX,EDI
        JNE     @@0
        XOR     EAX,EAX
        XOR     EDX,EDX
        JMP     @@3
@@2:    PUSH    EAX
        CMP     [ECX].TControlData.Version, 401
        JB      @@2a
        MOV     EAX, [ECX].TControlData2.FirstEventOfs
        TEST    EAX, EAX
        JNE     @@2b
@@2a:   {MOV     EAX, [EBX]
        CALL    TObject.ClassParent
        CALL    TObject.InstanceSize}
        MOV     EAX, szOleCtl
        ADD     EAX, 7
        AND     EAX, not 7  // 8 byte alignment
@@2b:   ADD     EBX, EAX
        POP     EAX
        MOV     EDX,[EBX][EAX*8].TMethod.Data
        MOV     EAX,[EBX][EAX*8].TMethod.Code
@@3:    POP     ECX
        MOV     [ECX].TMethod.Code,EAX
        MOV     [ECX].TMethod.Data,EDX
        POP     EDI
        POP     ESI
        POP     EBX
end;

function TOleCtl.GetEnumPropDesc(DispID: Integer): PEnumPropDesc;
var
  I: Integer;
begin
  with FControlData^ do
  begin
    if EnumPropDescs = nil then CreateEnumPropDescs;
    for I := 0 to EnumPropDescs.Count - 1 do
    begin
      Result := EnumPropDescs.Items[I];
      if Result.FDispID = DispID then Exit;
    end;
    Result := nil;
  end;
end;

function TOleCtl.GetIDispatchProp(Index: Integer): IDispatch;
var
  Temp: TVarData;
begin
  GetProperty(Index, Temp);
  Result := IDispatch(Temp.VDispatch);
end;

function TOleCtl.GetIntegerProp(Index: Integer): Integer;
var
  Temp: TVarData;
begin
  GetProperty(Index, Temp);
  Result := Temp.VInteger;
end;

function TOleCtl.GetIUnknownProp(Index: Integer): IUnknown;
var
  Temp: TVarData;
begin
  GetProperty(Index, Temp);
  Result := IUnknown(Temp.VUnknown);
end;

function TOleCtl.GetMainMenu: HMenu;
var
  Form: PControl;
begin
  Result := 0;
  Form := ParentForm;
  if Form <> nil then
    //if Form.FormStyle <> fsMDIChild then
      Result := Form.Menu
    {else
      if Application.MainForm <> nil then
        Result := Application.MainForm.Menu};
end;

function TOleCtl.GetOleBoolProp(Index: Integer): TOleBool;
var
  Temp: TVarData;
begin
  GetProperty(Index, Temp);
  Result := Temp.VBoolean;
end;

function TOleCtl.GetOleDateProp(Index: Integer): TOleDate;
var
  Temp: TVarData;
begin
  GetProperty(Index, Temp);
  Result := Temp.VDate;
end;

function TOleCtl.GetOleEnumProp(Index: Integer): TOleEnum;
begin
  Result := GetIntegerProp(Index);
end;

function TOleCtl.GetOleObject: Variant;
begin
  CreateControl;
  Result := Variant(FOleObject as IDispatch);
end;

function TOleCtl.GetOleVariantProp(Index: Integer): OleVariant;
begin
  VarClear(Result);
  GetProperty(Index, TVarData(Result));
end;

function TOleCtl.GetOnLeave: TOnEvent;
begin
  Result := OnExit;
end;

var  // init to zero, never written to
  DispParams: TDispParams = ();

procedure TOleCtl.GetProperty(Index: Integer; var Value: TVarData);
var
  Status: HResult;
  ExcepInfo: TExcepInfo;
begin
  CreateControl;
  Value.VType := varEmpty;
  Status := FControlDispatch.Invoke(Index, GUID_NULL, 0,
    DISPATCH_PROPERTYGET, DispParams, @Value, @ExcepInfo, nil);
  if Status <> 0 then DispatchInvokeError(Status, ExcepInfo);
end;

function TOleCtl.GetShortIntProp(Index: Integer): ShortInt;
begin
  Result := GetIntegerProp(Index);
end;

function TOleCtl.GetSingleProp(Index: Integer): Single;
var
  Temp: TVarData;
begin
  GetProperty(Index, Temp);
  Result := Temp.VSingle;
end;

function TOleCtl.GetSmallintProp(Index: Integer): Smallint;
var
  Temp: TVarData;
begin
  GetProperty(Index, Temp);
  Result := Temp.VSmallint;
end;

function TOleCtl.GetStringProp(Index: Integer): string;
begin
  Result := GetVariantProp(Index);
end;

function TOleCtl.GetTColorProp(Index: Integer): TColor;
begin
  Result := GetIntegerProp(Index);
end;

function TOleCtl.GetTDateTimeProp(Index: Integer): TDateTime;
var
  Temp: TVarData;
begin
  GetProperty(Index, Temp);
  Result := Temp.VDate;
end;

function TOleCtl.GetTFontProp(Index: Integer): PGraphicTool;
{var
  I: Integer;}
begin
  Result := nil;
  {for I := 0 to FFonts.Count-1 do
    if FControlData^.FontIDs^[I] = Index then
    begin
      Result := TFont(FFonts[I]);
      if Result.FontAdapter = nil then
        SetOleFont(Result, GetIDispatchProp(Index) as IFontDisp);
    end;}
  //TODO: implement TFont later
end;

function TOleCtl.GetTOleEnumProp(Index: Integer): TOleEnum;
begin
  Result := GetIntegerProp(Index);
end;

function TOleCtl.GetVariantProp(Index: Integer): Variant;
begin
  Result := GetOleVariantProp(Index);
end;

function TOleCtl.GetWideStringProp(Index: Integer): WideString;
var
  Temp: TVarData;
begin
  Result := '';
  GetProperty(Index, Temp);
  Pointer(Result) := Temp.VOleStr;
end;

function TOleCtl.GetWordBoolProp(Index: Integer): WordBool;
var
  Temp: TVarData;
begin
  GetProperty(Index, Temp);
  Result := Temp.VBoolean;
end;

function TOleCtl.GetWordProp(Index: Integer): Word;
begin
  Result := GetIntegerProp(Index);
end;

procedure TOleCtl.HookControlWndProc;
var
  WndHandle: HWnd;
begin
  if (FOleInPlaceObject <> nil) and (fHandle = 0) then
  begin
    WndHandle := 0;
    FOleInPlaceObject.GetWindow(WndHandle);
    if WndHandle = 0 then
      raise EOleError.CreateResFmt(e_Ole, Integer(@SNoWindowHandle), [nil]);
    fHandle := WndHandle;
    fDefWndProc := Pointer(GetWindowLong(fHandle, GWL_WNDPROC));
    CreatingWindow := @Self;
    SetWindowLong(fHandle, GWL_WNDPROC, Longint(@WndFunc));
    SendMessage(fHandle, WM_NULL, 0, 0);
  end;
end;

procedure TOleCtl.Init;
var
  I: Integer;
  intfClass: TOleCtlIntfClass;
begin
  OleInit;
  inherited;
  // overriding this method, we allow for constructor to initialize
  // the object.
  fControlClassName := 'OleCtl'; // ClassName
  {$IFDEF USE_FLAGS} include( fFlagsG3, G3_IsControl );
  {$ELSE} fIsControl := TRUE; {$ENDIF}
  fStyle.Value := WS_VISIBLE or WS_CLIPCHILDREN or WS_CLIPSIBLINGS or
      WS_CHILD; // or WS_BORDER or WS_THICKFRAME;

  //AttachProc( WndProcCtrl ); for test only

  // The rest of initialization -- moved from OleCtrls
  InitControlData;
  Inc(FControlData^.InstanceCount);
  if FControlData^.FontCount > 0 then
  begin
    FFonts := NewList;
    //FFonts.Count := FControlData^.FontCount;
    for I := 0 to FControlData^.FontCount-1 do
      FFonts.Add( NewFont );
  end;
  {if FControlData^.PictureCount > 0 then
  begin
    FPictures := NewList;
    //FPictures.Count := FControlData^.PictureCount;
    for I := 0 to FControlData^.PictureCount-1 do
    begin
      FPictures.Add( NewPicture );
      TPicture(FPictures[I]).OnChange := PictureChanged;
    end;
  end;}
  FEventDispatch := TEventDispatch.Create(@Self);
  CreateInstance;
  InitControlInterface(FOleObject);
  OleCheck(FOleObject.GetMiscStatus(DVASPECT_CONTENT, FMiscStatus));

  if (Assigned(OnGetIntfClass)) then
    intfClass := OnGetIntfClass()
  else
    intfClass := TOleCtlIntf;
  fOleCtlIntf := intfClass.Create;
  fOleCtlIntf.fOleCtl := @Self;

  if (FControlData^.Reserved and cdDeferSetClientSite) = 0 then
    if ((FMiscStatus and OLEMISC_SETCLIENTSITEFIRST) <> 0) or
      ((FControlData^.Reserved and cdForceSetClientSite) <> 0) then
      OleCheck(FOleObject.SetClientSite(fOleCtlIntf));
  OleCheck(FOleObject.QueryInterface(IPersistStreamInit, FPersistStream));
  if  FMiscStatus and OLEMISC_INVISIBLEATRUNTIME <> 0 then
      {$IFDEF USE_FLAGS} exclude( fStyle.f3_Style, F3_Visible );
      {$ELSE} fVisible := False; {$ENDIF}
  {if FMiscStatus and OLEMISC_SIMPLEFRAME <> 0 then
    ControlStyle := [csAcceptsControls, csDoubleClicks, csNoStdEvents] else
    ControlStyle := [csDoubleClicks, csNoStdEvents];}
  if FMiscStatus and OLEMISC_SIMPLEFRAME = 0 then
    fExStyle := 0; // clear WS_EX_CONTROLPARENT
  TabStop := FMiscStatus and (OLEMISC_ACTSLIKELABEL or
    OLEMISC_NOUIACTIVATE) = 0;
  OleCheck(fOleCtlIntf.RequestNewObjectLayout);
end;

procedure TOleCtl.InitControlData;
begin
  // nothing here. Originally, this method was abstract.
  // Since TOleControl class became TOleCtl object, abstract methods
  // are not available. So, make this method empty to override it
  // in descendant objects, which represent Active-X controls.
end;

procedure TOleCtl.InitControlInterface(const Obj: IUnknown);
begin
  // This method is to override it in derived Active-X control holder.
end;

procedure TOleCtl.InvokeEvent(DispID: TDispID; var Params: TDispParams);
var
  EventMethod: TMethod;
begin
  if ControlData.Version < 300 then
    D2InvokeEvent(DispID, Params)
  else
  begin
    fOleCtlIntf.GetEventMethod(DispID, EventMethod);
    if Integer(EventMethod.Code) < $10000 then Exit;

    try
      asm
                PUSH    EBX
                PUSH    ESI
                MOV     ESI, Params
                MOV     EBX, [ESI].TDispParams.cArgs
                TEST    EBX, EBX
                JZ      @@7
                MOV     ESI, [ESI].TDispParams.rgvarg
                MOV     EAX, EBX
                SHL     EAX, 4     // count * sizeof(TVarArg)
                XOR     EDX, EDX
                ADD     ESI, EAX   // EDI = Params.rgvarg^[ArgCount]
        @@1:    SUB     ESI, 16    // Sizeof(TVarArg)
                MOV     EAX, dword ptr [ESI]
                CMP     AX, varSingle
                JA      @@3
                JE      @@4
        @@2:    TEST    DL,DL
                JNE     @@2a
                MOV     ECX, ESI
                INC     DL
                TEST    EAX, varArray
                JNZ     @@6
                MOV     ECX, dword ptr [ESI+8]
                JMP     @@6
        @@2a:   TEST    EAX, varArray
                JZ      @@5
                PUSH    ESI
                JMP     @@6
        @@3:    CMP     AX, varDate
                JA      @@2
        @@4:    PUSH    dword ptr [ESI+12]
        @@5:    PUSH    dword ptr [ESI+8]
        @@6:    DEC     EBX
                JNE     @@1
        @@7:    MOV     EDX, Self
                MOV     EAX, EventMethod.Data
                CALL    EventMethod.Code
                POP     ESI
                POP     EBX
      end;
    except
      DoHandleException;
    end;
  end;
end;

procedure TOleCtl.KeyDown(var Key: Longint; AShift: DWORD);
begin
  if Assigned(EV.fOnKeyDown) then EV.fOnKeyDown(@Self, Key, AShift);
end;

procedure TOleCtl.KeyPress(var Key: KOLChar);
begin
  if Assigned(EV.fOnChar) then EV.fOnChar(@Self, Key, 0);
end;

procedure TOleCtl.KeyUp(var Key: Longint; AShift: DWORD);
begin
  if Assigned(EV.fOnKeyUp) then EV.fOnKeyUp(@Self, Key, AShift);
end;

procedure TOleCtl.MouseDown(Button: TMouseButton; AShift: DWORD; X,
  Y: Integer);
begin
  //TODO: mouse
end;

procedure TOleCtl.MouseMove(AShift: DWORD; X, Y: Integer);
begin
  //TODO: mouse
end;

procedure TOleCtl.MouseUp(Button: TMouseButton; AShift: DWORD; X,
  Y: Integer);
begin
  //TODO: mouse
end;

procedure TOleCtl.SetByteProp(Index: Integer; Value: Byte);
begin
  SetIntegerProp(Index, Value);
end;

procedure TOleCtl.SetColorProp(Index: Integer; Value: TColor);
begin
  SetIntegerProp(Index, Value);
end;

procedure TOleCtl.SetCompProp(Index: Integer; const Value: Comp);
var
  Temp: TVarData;
begin
  Temp.VType := VT_I8;
  Temp.VDouble := Value;
  SetProperty(Index, Temp);
end;

procedure TOleCtl.SetCurrencyProp(Index: Integer; const Value: Currency);
var
  Temp: TVarData;
begin
  Temp.VType := varCurrency;
  Temp.VCurrency := Value;
  SetProperty(Index, Temp);
end;

procedure TOleCtl.SetDoubleProp(Index: Integer; const Value: Double);
var
  Temp: TVarData;
begin
  Temp.VType := varDouble;
  Temp.VDouble := Value;
  SetProperty(Index, Temp);
end;

procedure TOleCtl.SetIDispatchProp(Index: Integer; const Value: IDispatch);
var
  Temp: TVarData;
begin
  Temp.VType := varDispatch;
  Temp.VDispatch := Pointer(Value);
  SetProperty(Index, Temp);
end;

procedure TOleCtl.SetIntegerProp(Index, Value: Integer);
var
  Temp: TVarData;
begin
  Temp.VType := varInteger;
  Temp.VInteger := Value;
  SetProperty(Index, Temp);
end;

procedure TOleCtl.SetIUnknownProp(Index: Integer; const Value: IUnknown);
var
  Temp: TVarData;
begin
  Temp.VType := VT_UNKNOWN;
  Temp.VUnknown := Pointer(Value);
  SetProperty(Index, Temp);
end;

(*procedure TOleCtl.SetMouseDblClk(const Value: TOnMouse);
begin
  {$IFDEF EVENTS_DYNAMIC} ProvideUniqueEvents {$ELSE} EV {$ENDIF}
  .fOnMouseDblClk := Value;
end;*)

procedure TOleCtl.SetName(const Value: String);
var
  OldName: string;
  DispID: Integer;
begin
  OldName := Name;
  Name := Value; //inherited SetName(Value);
  if FOleControl <> nil then
  begin
    FOleControl.OnAmbientPropertyChange(DISPID_AMBIENT_DISPLAYNAME);
    if FControlData^.Flags and (cfCaption or cfText) <> 0 then
    begin
      if FControlData^.Flags and cfCaption <> 0 then
        DispID := DISPID_CAPTION else
        DispID := DISPID_TEXT;
      if OldName = GetStringProp(DispID) then SetStringProp(DispID, Value);
    end;
  end;
end;

procedure TOleCtl.SetOleBoolProp(Index: Integer; Value: TOleBool);
var
  Temp: TVarData;
begin
  Temp.VType := varBoolean;
  if Value then
    Temp.VBoolean := WordBool(-1) else
    Temp.VBoolean := WordBool(0);
  SetProperty(Index, Temp);
end;

procedure TOleCtl.SetOleDateProp(Index: Integer; const Value: TOleDate);
var
  Temp: TVarData;
begin
  Temp.VType := varDate;
  Temp.VDate := Value;
  SetProperty(Index, Temp);
end;

procedure TOleCtl.SetOleEnumProp(Index: Integer; Value: TOleEnum);
begin
  SetIntegerProp(Index, Value);
end;

procedure TOleCtl.SetOleVariantProp(Index: Integer;
  const Value: OleVariant);
begin
  SetProperty(Index, TVarData(Value));
end;

procedure TOleCtl.SetOnChar(const Value: TOnChar);
begin
  {$IFDEF EVENTS_DYNAMIC} ProvideUniqueEvents {$ELSE} EV {$ENDIF}
  .fOnChar := Value;
end;

procedure TOleCtl.SetOnLeave(const Value: TOnEvent);
begin
  OnExit := Value;
end;

procedure TOleCtl.SetParent(AParent: PControl);
var
  CS: IOleClientSite;
  X: Integer;
begin
  inherited Parent := AParent;
  if (AParent <> nil) then
  begin
    try  // work around ATL bug
      X := FOleObject.GetClientSite(CS);
    except
      X := -1;
    end;
    if (X <> 0) or (CS = nil) then
      OleCheck(FOleObject.SetClientSite(fOleCtlIntf));
    if FOleControl <> nil then
      FOleControl.OnAmbientPropertyChange(DISPID_UNKNOWN);
  end;
end;

procedure TOleCtl.SetProperty(Index: Integer; const Value: TVarData);
const
  DispIDArgs: Longint = DISPID_PROPERTYPUT;
var
  Status, InvKind: Integer;
  DispParams: TDispParams;
  ExcepInfo: TExcepInfo;
begin
  CreateControl;
  DispParams.rgvarg := @Value;
  DispParams.rgdispidNamedArgs := @DispIDArgs;
  DispParams.cArgs := 1;
  DispParams.cNamedArgs := 1;
  if Value.VType <> varDispatch then
    InvKind := DISPATCH_PROPERTYPUT else
    InvKind := DISPATCH_PROPERTYPUTREF;
  Status := FControlDispatch.Invoke(Index, GUID_NULL, 0,
    InvKind, DispParams, nil, @ExcepInfo, nil);
  if Status <> 0 then DispatchInvokeError(Status, ExcepInfo);
end;

procedure TOleCtl.SetShortIntProp(Index: Integer; Value: Shortint);
begin
  SetIntegerProp(Index, Value);
end;

procedure TOleCtl.SetSingleProp(Index: Integer; const Value: Single);
var
  Temp: TVarData;
begin
  Temp.VType := varSingle;
  Temp.VSingle := Value;
  SetProperty(Index, Temp);
end;

procedure TOleCtl.SetSmallintProp(Index: Integer; Value: Smallint);
var
  Temp: TVarData;
begin
  Temp.VType := varSmallint;
  Temp.VSmallint := Value;
  SetProperty(Index, Temp);
end;

procedure TOleCtl.SetStringProp(Index: Integer; const Value: string);
var
  Temp: TVarData;
begin
  Temp.VType := varOleStr;
  Temp.VOleStr := StringToOleStr(Value);
  try
    SetProperty(Index, Temp);
  finally
    SysFreeString(Temp.VOleStr);
  end;
end;

procedure TOleCtl.SetTColorProp(Index: Integer; Value: TColor);
begin
  SetIntegerProp(Index, Value);
end;

procedure TOleCtl.SetTDateTimeProp(Index: Integer; const Value: TDateTime);
var
  Temp: TVarData;
begin
  Temp.VType := varDate;
  Temp.VDate := Value;
  SetProperty(Index, Temp);
end;

procedure TOleCtl.SetTFontProp(Index: Integer; Value: PGraphicTool);
{var
  I: Integer;
  F: TFont;
  Temp: IFontDisp;}
begin
  {for I := 0 to FFonts.Count-1 do
    if FControlData^.FontIDs^[I] = Index then
    begin
      F := TFont(FFonts[I]);
      F.Assign(Value);
      if F.FontAdapter = nil then
      begin
        GetOleFont(F, Temp);
        SetIDispatchProp(Index, Temp);
      end;
    end;}
  //TODO: implement TFont property later
end;

procedure TOleCtl.SetTOleEnumProp(Index: Integer; Value: TOleEnum);
begin
  SetIntegerProp(Index, Value);
end;

procedure TOleCtl.SetUIActive(Active: Boolean);
var
  Form: POleCtl; // declare it as POleCtl, though it is only PControl
                 // - to access its protected fields
begin
  Form := POleCtl( ParentForm );
  if  Form <> nil then
      if  Active then
      begin
          {if (Form.ActiveOleControl <> nil) and
            (Form.ActiveOleControl <> Self) then
            Form.ActiveOleControl.Perform(CM_UIDEACTIVATE, 0, 0);
          Form.ActiveOleControl := Self;}
          if  (Form.DF.fCurrentControl <> nil) and
              (Form.DF.fCurrentControl <> @Self) then
              Form.DF.fCurrentControl.Perform(CM_UIDEACTIVATE, 0, 0);
          Form.DF.fCurrentControl := @Self;
      end else
          if  Form.DF.fCurrentControl = @Self then
              Form.DF.fCurrentControl := nil;
end;

procedure TOleCtl.SetVariantProp(Index: Integer; const Value: Variant);
begin
  SetOleVariantProp(Index, Value);
end;

procedure TOleCtl.SetWideStringProp(Index: Integer;
  const Value: WideString);
var
  Temp: TVarData;
begin
  Temp.VType := varOleStr;
  if Value <> '' then
    Temp.VOleStr := PWideChar(Value)
  else
    Temp.VOleStr := nil;
  SetProperty(Index, Temp);
end;

procedure TOleCtl.SetWordBoolProp(Index: Integer; Value: WordBool);
var
  Temp: TVarData;
begin
  Temp.VType := varBoolean;
  if Value then
    Temp.VBoolean := WordBool(-1) else
    Temp.VBoolean := WordBool(0);
  SetProperty(Index, Temp);
end;

procedure TOleCtl.SetWordProp(Index: Integer; Value: Word);
begin
  SetIntegerProp(Index, Value);
end;

procedure TOleCtl.StandardEvent(DispID: TDispID; var Params: TDispParams);
type
  PVarDataList = ^TVarDataList;
  TVarDataList = array[0..3] of TVarData;
const
  {ShiftMap: array[0..7] of TShiftState = (
    [],
    [ssShift],
    [ssCtrl],
    [ssShift, ssCtrl],
    [ssAlt],
    [ssShift, ssAlt],
    [ssCtrl, ssAlt],
    [ssShift, ssCtrl, ssAlt]);
  MouseMap: array[0..7] of TShiftState = (
    [],
    [ssLeft],
    [ssRight],
    [ssLeft, ssRight],
    [ssMiddle],
    [ssLeft, ssMiddle],
    [ssRight, ssMiddle],
    [ssLeft, ssRight, ssMiddle]);}
  ShiftMap: array[0..7] of DWord = (
    0,
    MK_SHIFT,
    MK_CONTROL,
    MK_SHIFT or MK_CONTROL,
    MK_ALT,
    MK_SHIFT or MK_ALT,
    MK_CONTROL or MK_ALT,
    MK_SHIFT or MK_CONTROL or MK_ALT);
  MouseMap: array[0..7] of DWORD = (
    0,
    MK_LBUTTON,
    MK_RBUTTON,
    MK_LBUTTON or MK_RBUTTON,
    MK_MBUTTON,
    MK_LBUTTON or MK_MBUTTON,
    MK_RBUTTON or MK_MBUTTON,
    MK_LBUTTON or MK_RBUTTON or MK_MBUTTON);
  ButtonMap: array[0..7] of TMouseButton = (
    mbLeft, mbLeft, mbRight, mbLeft, mbMiddle, mbLeft, mbRight, mbLeft);
var
  Args: PVarDataList;
  AShift: DWORD;
  Button: TMouseButton;
  X, Y: Integer;
  Key: Longint;
  Ch: KOLChar;
begin
  Args := PVarDataList(Params.rgvarg);
  try
    case DispID of
      DISPID_CLICK:
        Click;
      DISPID_DBLCLICK:
        DblClk;
      DISPID_KEYDOWN, DISPID_KEYUP:
        if Params.cArgs >= 2 then
        begin
          Key := Variant(Args^[1]);
          X := Variant(Args^[0]);
          case DispID of
            DISPID_KEYDOWN: KeyDown(Key, X);
            DISPID_KEYUP:   KeyUp(Key, X);
          end;
          if ((Args^[1].vType and varByRef) <> 0) then
            Word(Args^[1].VPointer^) := Key;
        end;
      DISPID_KEYPRESS:
       if Params.cArgs > 0 then
       begin
         Ch := KOLChar(Integer(Variant(Args^[0])));
         KeyPress(Ch);
         if ((Args^[0].vType and varByRef) <> 0) then
           KOLChar(Args^[0].VPointer^) := Ch;
       end;
      {DISPID_KEYPRESS:
        if Params.cArgs > 0 then
        begin
          Ch := KOLChar(Integer(Variant(Args^[0])));
          KeyPress(Ch);
          if ((Args^[0].vType and varByRef) <> 0) then
            KOLChar(Args^[0].VPointer^) := Ch;
        end;}
      DISPID_MOUSEDOWN, DISPID_MOUSEMOVE, DISPID_MOUSEUP:
        if Params.cArgs >= 4 then
        begin
          X := Integer(Variant(Args^[3])) and 7;
          Y := Integer(Variant(Args^[2])) and 7;
          Button := ButtonMap[X];
          AShift := ShiftMap[Y] + MouseMap[X];
          X := Variant(Args^[1]);
          Y := Variant(Args^[0]);
          case DispID of
            DISPID_MOUSEDOWN:
              MouseDown(Button, AShift, X, Y);
            DISPID_MOUSEMOVE:
              MouseMove(AShift, X, Y);
            DISPID_MOUSEUP:
              MouseUp(Button, AShift, X, Y);
          end;
        end;
    end;
  except
    DoHandleException;
  end;
end;

{$IFNDEF _D2orD3}
{ TServerEventDispatch }
constructor TServerEventDispatch.Create(Server: TOleServer);
begin
  FServer := Server;
  InternalRefCount := 1;
end;

{ TServerEventDispatch.IUnknown }
function TServerEventDispatch.QueryInterface(const IID: TGUID; out Obj): HResult;
begin
  if GetInterface(IID, Obj) then
  begin
    Result := S_OK;
    Exit;
  end;
  if IsEqualIID(IID, FServer.FServerData^.EventIID) then
  begin
    GetInterface(IDispatch, Obj);
    Result := S_OK;
    Exit;
  end;
  Result := E_NOINTERFACE;
end;

function TServerEventDispatch._AddRef: Integer;
begin
  if FServer <> nil then FServer._AddRef;
  InternalRefCount := InternalRefCount + 1;
  Result := InternalRefCount;
end;

function TServerEventDispatch._Release: Integer;
begin
  if FServer <> nil then FServer._Release;
  InternalRefCount := InternalRefCount -1;
  Result := InternalRefCount;
end;

{ TServerEventDispatch.IDispatch }
function TServerEventDispatch.GetTypeInfoCount(out Count: Integer): HResult;
begin
  Count := 0;
  Result:= S_OK;
end;

function TServerEventDispatch.GetTypeInfo(Index, LocaleID: Integer; out TypeInfo): HResult;
begin
  Pointer(TypeInfo) := nil;
  Result := E_NOTIMPL;
end;

function TServerEventDispatch.GetIDsOfNames(const IID: TGUID; Names: Pointer;
  NameCount, LocaleID: Integer; DispIDs: Pointer): HResult;
begin
  Result := E_NOTIMPL;
end;

function TServerEventDispatch.Invoke(DispID: Integer; const IID: TGUID;
  LocaleID: Integer; Flags: Word; var Params;
  VarResult, ExcepInfo, ArgErr: Pointer): HResult;
var
  ParamCount, I: integer;
  VarArray : TVariantArray;
begin
  // Get parameter count
  ParamCount := TDispParams(Params).cArgs;
  // Set our array to appropriate length
  SetLength(VarArray, ParamCount);
  // Copy over data
  for I := Low(VarArray) to High(VarArray) do
    VarArray[High(VarArray)-I] := OleVariant(TDispParams(Params).rgvarg^[I]);
  // Invoke Server proxy class
  if FServer <> nil then FServer.InvokeEvent(DispID, VarArray);
  // Clean array
  SetLength(VarArray, 0);
  // Pascal Events return 'void' - so assume success!
  Result := S_OK;
end;

function TServerEventDispatch.ServerDisconnect : Boolean;
begin
  FServer := nil;
  if FServer <> nil then
    Result := false
  else Result := true;
end;

{TOleServer}
constructor TOleServer.Create; //(AOwner: TComponent);
begin
  inherited; // Create(AOwner);
  // Allow derived class to initialize ServerData structure pointer
  InitServerData;
  // Make sure derived class set ServerData pointer to some valid structure
  Assert(FServerData <> nil);
  // Increment instance count (not used currently)
  Inc(FServerData^.InstanceCount);
  // Create Event Dispatch Handler
  FEventDispatch := TServerEventDispatch.Create(Self);
end;

destructor TOleServer.Destroy;
begin
  // Disconnect from the Server (NOTE: Disconnect must handle case when we're no longer connected)
  Disconnect;
  // Free Events dispatcher
  FEventDispatch.ServerDisconnect;
  if (FEventDispatch._Release = 0) then FEventDispatch.Free;
  // Decrement refcount
  Dec(FServerData^.InstanceCount);
  inherited Destroy;
end;

procedure TOleServer.Loaded;
begin
  {inherited Loaded;}

  // Load Server if user requested 'AutoConnect' and we're not in Design mode
  {if not (csDesigning in ComponentState) then}
    if AutoConnect then
        Connect;
end;

procedure TOleServer.InvokeEvent(DispID: TDispID; var Params: TVariantArray);
begin
  // To be overriden in derived classes to do dispatching
end;

function TOleServer.GetServer: IUnknown;
var
  HR: HResult;
  ErrorStr: string;
begin
  case ConnectKind of
    ckNewInstance:
      Result := CreateComObject(FServerData^.ClassId);

    ckRunningInstance:
    begin
      HR := GetActiveObject(FServerData^.ClassId, nil, Result);
      if not Succeeded(HR) then
      begin
        ErrorStr := Format(sNoRunningObject, [ClassIDToProgID(FServerData^.ClassId),
                                              GuidToString(FServerData^.ClassId)]);
        raise EOleSysError.Create( e_Ole, ErrorStr {, HR, 0} );
      end;
    end;

    ckRunningOrNew:
      if not Succeeded(GetActiveObject(FServerData^.ClassId, nil, Result)) then
        Result := CreateComObject(FServerData^.ClassId);

    ckRemote:
      {Highly inefficient: requires at least two round trips - GetClassObject + QI}
      Result := CreateRemoteComObject(RemoteMachineName, FServerData^.ClassID);
  end;
end;

procedure TOleServer.ConnectEvents(const Obj: IUnknown);
begin
  KOLComObj.InterfaceConnect(Obj, FServerData^.EventIID, FEventDispatch, FEventsConnection);
end;

procedure TOleServer.DisconnectEvents(const Obj: Iunknown);
begin
  KOLComObj.InterfaceDisconnect(Obj, FServerData^.EventIID, FEventsConnection);
end;

function  TOleServer.GetConnectKind: TConnectKind;
begin
  // Should the setting of a RemoteMachine name override the Connection Kind ??
  if RemoteMachineName <> '' then
    Result := ckRemote
  else
    Result := FConnectKind;
end;

procedure TOleServer.SetConnectKind(cK: TConnectKind);
begin
  // Should we validate that we have a RemoteMachineName for ckRemote ??
  FConnectKind := cK;
end;

function  TOleServer.GetAutoConnect: Boolean;
begin
  // If user wants to provide the interface to connect to, then we won't
  // 'automatically' connect to a server.
  if ConnectKind = ckAttachToInterface then
    Result := False
  else
    Result := FAutoConnect;
end;

procedure TOleServer.SetAutoConnect(flag: Boolean);
begin
  FAutoConnect := flag;
end;

{ TOleServer.IUnknown }
function TOleServer.QueryInterface(const IID: TGUID; out Obj): HResult;
begin
  if GetInterface(IID, Obj) then
    Result := S_OK
  else
    Result := E_NOINTERFACE;
end;

function TOleServer._AddRef: Integer;
begin
  Inc(FRefCount);
  Result := FRefCount;
end;

function TOleServer._Release: Integer;
begin
  Dec(FRefCount);
  Result := FRefCount;
end;
{$ENDIF _D2orD3}

{ TEventDispatch }

constructor TEventDispatch.Create(Control: POleCtl);
begin
  FControl := Control;
end;

{ TEventDispatch.IUnknown }

function TEventDispatch.QueryInterface(const IID: TGUID; out Obj): HResult;
begin
  if GetInterface(IID, Obj) then
  begin
    Result := S_OK;
    Exit;
  end;
  if IsEqualIID(IID, FControl.FControlData^.EventIID) then
  begin
    GetInterface(IDispatch, Obj);
    Result := S_OK;
    Exit;
  end;
  Result := E_NOINTERFACE;
end;

function TEventDispatch._AddRef: Integer;
begin
  Result := FControl.fOleCtlIntf._AddRef;
end;

function TEventDispatch._Release: Integer;
begin
  Result := FControl.fOleCtlIntf._Release;
end;

{ TEventDispatch.IDispatch }

function TEventDispatch.GetTypeInfoCount(out Count: Integer): HResult;
begin
  Count := 0;
  Result := S_OK;
end;

function TEventDispatch.GetTypeInfo(Index, LocaleID: Integer;
  out TypeInfo): HResult;
begin
  Pointer(TypeInfo) := nil;
  Result := E_NOTIMPL;
end;

function TEventDispatch.GetIDsOfNames(const IID: TGUID; Names: Pointer;
  NameCount, LocaleID: Integer; DispIDs: Pointer): HResult;
begin
  Result := E_NOTIMPL;
end;

function TEventDispatch.Invoke(DispID: Integer; const IID: TGUID;
  LocaleID: Integer; Flags: Word; var Params;
  VarResult, ExcepInfo, ArgErr: Pointer): HResult;
begin
  if (DispID >= DISPID_MOUSEUP) and (DispID <= DISPID_CLICK) then
    FControl.StandardEvent(DispID, TDispParams(Params)) else
    FControl.InvokeEvent(DispID, TDispParams(Params));
  Result := S_OK;
end;

{ TOleCtlIntf }

function TOleCtlIntf._AddRef: Integer;
begin
  //{$IFDEF _D2orD3}
  //Result := inherited _AddRef;
  //{$ELSE}
  Inc(FRefCount);
  Result := FRefCount;
  //{$ENDIF}
end;

function TOleCtlIntf._Release: Integer;
begin
  //{$IFDEF _D2orD3}
  //Result := inherited _Release;
  //{$ELSE}
  Dec(FRefCount);
  Result := FRefCount;
  //{$ENDIF}
end;

function TOleCtlIntf.CanInPlaceActivate: HResult;
begin
  Result := S_OK;
end;

function TOleCtlIntf.ContextSensitiveHelp(fEnterMode: BOOL): HResult;
begin
  Result := S_OK;
end;

function TOleCtlIntf.DeactivateAndUndo: HResult;
begin
  fOleCtl.FOleInPlaceObject.UIDeactivate;
  Result := S_OK;
end;

function TOleCtlIntf.DiscardUndoState: HResult;
begin
  Result := E_NOTIMPL;
end;

function TOleCtlIntf.EnableModeless(fEnable: BOOL): HResult;
begin
  Result := S_OK;
end;

function TOleCtlIntf.GetBorder(out rectBorder: TRect): HResult;
begin
  Result := INPLACE_E_NOTOOLSPACE;
end;

function TOleCtlIntf.GetContainer(out container: IOleContainer): HResult;
begin
  Result := E_NOINTERFACE;
end;

function TOleCtlIntf.GetExtendedControl(out disp: IDispatch): HResult;
begin
  Result := E_NOTIMPL;
end;

function TOleCtlIntf.GetIDsOfNames(const IID: TGUID; Names: Pointer;
  NameCount, LocaleID: Integer; DispIDs: Pointer): HResult;
begin
  Result := E_NOTIMPL;
end;

function TOleCtlIntf.GetMoniker(dwAssign, dwWhichMoniker: Integer;
  out mk: IMoniker): HResult;
begin
  Result := E_NOTIMPL;
end;

function TOleCtlIntf.GetTypeInfo(Index, LocaleID: Integer;
  out TypeInfo): HResult;
begin
  Pointer(TypeInfo) := nil;
  Result := E_NOTIMPL;
end;

function TOleCtlIntf.GetTypeInfoCount(out Count: Integer): HResult;
begin
  Count := 0;
  Result := S_OK;
end;

function TOleCtlIntf.GetWindowContext(out frame: IOleInPlaceFrame;
  out doc: IOleInPlaceUIWindow; out rcPosRect, rcClipRect: TRect;
  out frameInfo: TOleInPlaceFrameInfo): HResult;
begin
  frame := Self;
  doc := nil;
  rcPosRect := fOleCtl.BoundsRect;
  rcClipRect := MakeRect( 0, 0, 32767, 32767 );
  with frameInfo do
  begin
    fMDIApp := False;
    hWndFrame := fOleCtl.ParentForm.GetWindowHandle;
                 //GetTopParentHandle;
                 // now it is not possible to make alien window to be parent for KOL window 
    hAccel := 0;
    cAccelEntries := 0;
  end;
  Result := S_OK;
end;

function TOleCtlIntf.InsertMenus(hmenuShared: HMenu;
  var menuWidths: TOleMenuGroupWidths): HResult;
{var
  Menu: TMainMenu;}
begin
  {Menu := GetMainMenu;
  if Menu <> nil then
    Menu.PopulateOle2Menu(hmenuShared, [0, 2, 4], menuWidths.width);}
  //TODO: implement menu populate
  Result := S_OK;
end;

function TOleCtlIntf.Invoke(DispID: Integer; const IID: TGUID;
  LocaleID: Integer; Flags: Word; var Params; VarResult, ExcepInfo,
  ArgErr: Pointer): HResult;
{var
  F: PGraphicTool;}
begin
  if (Flags and DISPATCH_PROPERTYGET <> 0) and (VarResult <> nil) then
  begin
    Result := S_OK;
    case DispID of
      DISPID_AMBIENT_BACKCOLOR:
        PVariant(VarResult)^ := fOleCtl.Color;
      DISPID_AMBIENT_DISPLAYNAME:
        PVariant(VarResult)^ := StringToVarOleStr( fOleCtl.Name );
      DISPID_AMBIENT_FONT:
      begin
        {if (fOleCtl.Parent <> nil) and fOleCtl.ParentFont then
          F := Parent.Font // TOleControl(Parent).Font
        else
          F := Font;
        PVariant(VarResult)^ := FontToOleFont(F);}
        //TODO: implement Font later
      end;
      DISPID_AMBIENT_FORECOLOR:
        PVariant(VarResult)^ := fOleCtl.fTextColor; // Font.Color;
      DISPID_AMBIENT_LOCALEID:
        PVariant(VarResult)^ := Integer(GetUserDefaultLCID);
      DISPID_AMBIENT_MESSAGEREFLECT:
        PVariant(VarResult)^ := True;
      DISPID_AMBIENT_USERMODE:
        PVariant(VarResult)^ := TRUE; // not (csDesigning in ComponentState);
      DISPID_AMBIENT_UIDEAD:
        PVariant(VarResult)^ := FALSE; // csDesigning in ComponentState;
      DISPID_AMBIENT_SHOWGRABHANDLES:
        PVariant(VarResult)^ := False;
      DISPID_AMBIENT_SHOWHATCHING:
        PVariant(VarResult)^ := False;
      DISPID_AMBIENT_SUPPORTSMNEMONICS:
        PVariant(VarResult)^ := True;
      DISPID_AMBIENT_AUTOCLIP:
        PVariant(VarResult)^ := True;
    else
      Result := DISP_E_MEMBERNOTFOUND;
    end;
  end else
    Result := DISP_E_MEMBERNOTFOUND;
end;

function TOleCtlIntf.LockInPlaceActive(fLock: BOOL): HResult;
begin
  Result := E_NOTIMPL;
end;

function TOleCtlIntf.OleControlSite_TranslateAccelerator(msg: PMsg;
  grfModifiers: Integer): HResult;
begin
  Result := E_NOTIMPL;
end;

function TOleCtlIntf.OleInPlaceFrame_GetWindow(out wnd: HWnd): HResult;
begin
  wnd := fOleCtl.ParentForm.GetWindowHandle; // GetTopParentHandle;
  Result := S_OK;
end;

function TOleCtlIntf.OleInPlaceFrame_TranslateAccelerator(var msg: Windows.TMsg;
  wID: Word): HResult;
begin
  Result := S_FALSE;
end;

function TOleCtlIntf.OleInPlaceSite_GetWindow(out wnd: HWnd): HResult;
begin
  Result := S_OK;
  wnd := fOleCtl.ParentWindow;
  if wnd = 0 then Result := E_FAIL;
end;

function TOleCtlIntf.OnChanged(dispid: TDispID): HResult;
begin
  try
    case dispid of
      DISPID_BACKCOLOR:
        if not fOleCtl.FUpdatingColor then
        begin
          fOleCtl.FUpdatingColor := True;
          try
            fOleCtl.fColor := fOleCtl.GetIntegerProp(DISPID_BACKCOLOR);
          finally
            fOleCtl.FUpdatingColor := False;
          end;
        end;
      DISPID_ENABLED:
        if not fOleCtl.FUpdatingEnabled then
        begin
          fOleCtl.FUpdatingEnabled := True;
          try
            fOleCtl.Enabled := fOleCtl.GetWordBoolProp(DISPID_ENABLED);
          finally
            fOleCtl.FUpdatingEnabled := False;
          end;
        end;
      DISPID_FONT:
        if not fOleCtl.FUpdatingFont then
        begin
          fOleCtl.FUpdatingFont := True;
          try
            //OleFontToFont(GetVariantProp(DISPID_FONT), Font);
            // font - implement later
          finally
            fOleCtl.FUpdatingFont := False;
          end;
        end;
      DISPID_FORECOLOR:
        if not fOleCtl.FUpdatingFont then
        begin
          fOleCtl.FUpdatingFont := True;
          try
            fOleCtl.fTextColor := fOleCtl.GetIntegerProp(DISPID_FORECOLOR);
            //Font.Color := GetIntegerProp(DISPID_FORECOLOR);
          finally
            fOleCtl.FUpdatingFont := False;
          end;
        end;
    end;
  except  // control sent us a notification for a dispid it doesn't have.
    //on EOleError do ;
  end;
  Result := S_OK;
end;

function TOleCtlIntf.OnControlInfoChanged: HResult;
begin
  Result := E_NOTIMPL;
end;

function TOleCtlIntf.OnFocus(fGotFocus: BOOL): HResult;
begin
  Result := E_NOTIMPL;
end;

function TOleCtlIntf.OnInPlaceActivate: HResult;
begin
  fOleCtl.FOleObject.QueryInterface( IOleInPlaceObject,
                                     fOleCtl.FOleInPlaceObject);
  fOleCtl.FOleObject.QueryInterface( IOleInPlaceActiveObject,
                                     fOleCtl.FOleInPlaceActiveObject);
  Result := S_OK;
end;

function TOleCtlIntf.OnInPlaceDeactivate: HResult;
begin
  fOleCtl.FOleInPlaceActiveObject := nil;
  fOleCtl.FOleInPlaceObject := nil;
  Result := S_OK;
end;

function TOleCtlIntf.OnPosRectChange(const rcPosRect: TRect): HResult;
begin
  fOleCtl.FOleInPlaceObject.SetObjectRects(rcPosRect, MakeRect(0, 0, 32767, 32767));
  Result := S_OK;
end;

function TOleCtlIntf.OnRequestEdit(dispid: TDispID): HResult;
begin
  Result := S_OK;
end;

function TOleCtlIntf.OnShowWindow(fShow: BOOL): HResult;
begin
  Result := S_OK;
end;

function TOleCtlIntf.OnUIActivate: HResult;
begin
  fOleCtl.SetUIActive(True);
  Result := S_OK;
end;

function TOleCtlIntf.OnUIDeactivate(fUndoable: BOOL): HResult;
begin
  SetMenu(0, 0, 0);
  fOleCtl.SetUIActive(False);
  Result := S_OK;
end;

function TOleCtlIntf.PostMessageFilter(wnd: HWnd; msg, wp, lp: Integer;
  out res: Integer; Cookie: Integer): HResult;
begin
  Result := S_OK;
end;

function TOleCtlIntf.PreMessageFilter(wnd: HWnd; msg, wp, lp: Integer;
  out res, Cookie: Integer): HResult;
begin
  Result := S_OK;
end;

function TOleCtlIntf.QueryInterface(const IID: TGUID; out Obj): HResult;
begin
  if GetInterface(IID, Obj) then Result := S_OK else Result := E_NOINTERFACE;
end;

function TOleCtlIntf.RemoveMenus(hmenuShared: HMenu): HResult;
begin
  while GetMenuItemCount(hmenuShared) > 0 do
    RemoveMenu(hmenuShared, 0, MF_BYPOSITION);
  Result := S_OK;
end;

function TOleCtlIntf.RequestBorderSpace(
  const borderwidths: TRect): HResult;
begin
  Result := INPLACE_E_NOTOOLSPACE;
end;

function TOleCtlIntf.RequestNewObjectLayout: HResult;
var
  Extent: TPoint;
  W, H: Integer;
  DC: HDC;
  PixelsPerInch: Integer;
begin
  Result := fOleCtl.FOleObject.GetExtent(DVASPECT_CONTENT, Extent);
  if Result <> S_OK then Exit;

  W := fOleCtl.Width;
  H := fOleCtl.Height;
  if (W = 0) or (H = 0) then
  begin
    DC := GetDC(0);
    PixelsPerInch := GetDeviceCaps(DC, LOGPIXELSY);
    ReleaseDC(0, DC);

    W := MulDiv(Extent.X, PixelsPerInch, 2540);
    H := MulDiv(Extent.Y, PixelsPerInch, 2540);
    if (fOleCtl.FMiscStatus and OLEMISC_INVISIBLEATRUNTIME <> 0) and
       (fOleCtl.FOleControl = nil) then
    begin
      if W > 32 then W := 32;
      if H > 32 then H := 32;
    end;
  end;
  fOleCtl.SetBoundsRect( MakeRect( fOleCtl.Left, fOleCtl.Top,
                         fOleCtl.Left + W, fOleCtl.Top + H ) );
end;

function TOleCtlIntf.SaveObject: HResult;
begin
  Result := S_OK;
end;

function TOleCtlIntf.Scroll(scrollExtent: TPoint): HResult;
begin
  Result := E_NOTIMPL;
end;

function TOleCtlIntf.SetActiveObject(
  const activeObject: IOleInPlaceActiveObject;
  pszObjName: POleStr): HResult;
begin
  Result := S_OK;
end;

function TOleCtlIntf.SetBorderSpace(pborderwidths: PRect): HResult;
begin
  Result := E_NOTIMPL;
end;

function TOleCtlIntf.SetMenu(hmenuShared, holemenu: HMenu;
  hwndActiveObject: HWnd): HResult;
var
  Menu: HMenu;
begin
  Menu := fOleCtl.GetMainMenu;
  Result := S_OK;
  if Menu <> 0 then
  begin
    //Menu.SetOle2MenuHandle(hmenuShared);
    Result := OleSetMenuDescriptor( holemenu,
              fOleCtl.ParentForm.GetWindowHandle,
              hwndActiveObject, nil, nil);
  end;
end;

function TOleCtlIntf.SetStatusText(pszStatusText: POleStr): HResult;
begin
  Result := S_OK;
end;

function TOleCtlIntf.ShowObject: HResult;
begin
  fOleCtl.HookControlWndProc;
  Result := S_OK;
end;

function TOleCtlIntf.ShowPropertyFrame: HResult;
begin
  Result := E_NOTIMPL;
end;

function TOleCtlIntf.TransformCoords(var ptlHimetric: TPoint;
  var ptfContainer: TPointF; flags: Integer): HResult;
var DC: HDC;
    PixelsPerInch: Integer;
begin
  DC := GetDC(0);
  PixelsPerInch := GetDeviceCaps(DC, LOGPIXELSY);
  ReleaseDC(0, DC);

  if flags and XFORMCOORDS_HIMETRICTOCONTAINER <> 0 then
  begin
    ptfContainer.X := MulDiv(ptlHimetric.X, PixelsPerInch, 2540);
    ptfContainer.Y := MulDiv(ptlHimetric.Y, PixelsPerInch, 2540);
  end else
  begin
    ptlHimetric.X := Integer(Round(ptfContainer.X * 2540 / PixelsPerInch));
    ptlHimetric.Y := Integer(Round(ptfContainer.Y * 2540 / PixelsPerInch));
  end;
  Result := S_OK;
end;

constructor TOleCtlIntf.Create;
begin
  inherited;
end;


end.

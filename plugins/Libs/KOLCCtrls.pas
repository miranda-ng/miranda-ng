unit KOLCCtrls;
{$UNDEF UNICODE}

interface

uses
  Windows, Messages, ShellAPI, KOL;

{ ====== TRACKBAR CONTROL CONSTANTS =================== }

const
  TRACKBAR_CLASS    = 'msctls_trackbar32';

  TBS_AUTOTICKS     = $0001;
  TBS_VERT          = $0002;
  TBS_HORZ          = $0000;
  TBS_TOP           = $0004;
  TBS_BOTTOM        = $0000;
  TBS_LEFT          = $0004;
  TBS_RIGHT         = $0000;
  TBS_BOTH          = $0008;
  TBS_NOTICKS       = $0010;
  TBS_ENABLESELRANGE = $0020;
  TBS_FIXEDLENGTH   = $0040;
  TBS_NOTHUMB       = $0080;
  TBS_TOOLTIPS      = $0100;

  TBM_GETPOS        = WM_USER;
  TBM_GETRANGEMIN   = WM_USER + 1;
  TBM_GETRANGEMAX   = WM_USER + 2;
  TBM_GETTIC        = WM_USER + 3;
  TBM_SETTIC        = WM_USER + 4;
  TBM_SETPOS        = WM_USER + 5;
  TBM_SETRANGE      = WM_USER + 6;
  TBM_SETRANGEMIN   = WM_USER + 7;
  TBM_SETRANGEMAX   = WM_USER + 8;
  TBM_CLEARTICS     = WM_USER + 9;
  TBM_SETSEL        = WM_USER + 10;
  TBM_SETSELSTART   = WM_USER + 11;
  TBM_SETSELEND     = WM_USER + 12;
  TBM_GETPTICS      = WM_USER + 14;
  TBM_GETTICPOS     = WM_USER + 15;
  TBM_GETNUMTICS    = WM_USER + 16;
  TBM_GETSELSTART   = WM_USER + 17;
  TBM_GETSELEND     = WM_USER + 18;
  TBM_CLEARSEL      = WM_USER + 19;
  TBM_SETTICFREQ    = WM_USER + 20;
  TBM_SETPAGESIZE   = WM_USER + 21;
  TBM_GETPAGESIZE   = WM_USER + 22;
  TBM_SETLINESIZE   = WM_USER + 23;
  TBM_GETLINESIZE   = WM_USER + 24;
  TBM_GETTHUMBRECT  = WM_USER + 25;
  TBM_GETCHANNELRECT = WM_USER + 26;
  TBM_SETTHUMBLENGTH = WM_USER + 27;
  TBM_GETTHUMBLENGTH = WM_USER + 28;
  TBM_SETTOOLTIPS   = WM_USER + 29;
  TBM_GETTOOLTIPS   = WM_USER + 30;
  TBM_SETTIPSIDE    = WM_USER + 31;

  // TrackBar Tip Side flags
  TBTS_TOP          = 0;
  TBTS_LEFT         = 1;
  TBTS_BOTTOM       = 2;
  TBTS_RIGHT        = 3;

  TBM_SETBUDDY      = WM_USER + 32; // wparam = BOOL fLeft; (or right)
  TBM_GETBUDDY      = WM_USER + 33; // wparam = BOOL fLeft; (or right)
  TBM_SETUNICODEFORMAT = CCM_SETUNICODEFORMAT;
  TBM_GETUNICODEFORMAT = CCM_GETUNICODEFORMAT;

  TB_LINEUP         = 0;
  TB_LINEDOWN       = 1;
  TB_PAGEUP         = 2;
  TB_PAGEDOWN       = 3;
  TB_THUMBPOSITION  = 4;
  TB_THUMBTRACK     = 5;
  TB_TOP            = 6;
  TB_BOTTOM         = 7;
  TB_ENDTRACK       = 8;

  // custom draw item specs
  TBCD_TICS         = $0001;
  TBCD_THUMB        = $0002;
  TBCD_CHANNEL      = $0003;

  { ^^^^^^^^ TRACKBAR CONTROL ^^^^^^^^ }

type
  PTrackbar = ^TTrackbar;
  TTrackbarOption = (trbAutoTicks, trbEnableSelRange, trbFixedLength,
    trbNoThumb, trbNoTicks, trbTooltips, trbTopLeftMarks,
    trbVertical, trbNoBorder, trbBoth);
  TTrackbarOptions = set of TTrackbarOption;

  TOnScroll = procedure(Sender: PTrackbar; Code: Integer) of object;
  {* Code:
  |<pre>
  TB_THUMBTRACK    Slider movement (the user dragged the slider)
  TB_THUMBPOSITION WM_LBUTTONUP following a TB_THUMBTRACK notification message
  TB_BOTTOM        VK_END
  TB_ENDTRACK      WM_KEYUP (the user released a key that sent a relevant virtual key code)
  TB_LINEDOWN      VK_RIGHT or VK_DOWN
  TB_LINEUP        VK_LEFT or VK_UP
  TB_PAGEDOWN      VK_NEXT (the user clicked the channel below or to the right of the slider)
  TB_PAGEUP        VK_PRIOR (the user clicked the channel above or to the left of the slider)
  TB_TOP           VK_HOME
  |</pre>
  }

  TTrackbar = object(TControl)
  private
    function GetOnScroll: TOnScroll;
    procedure SetOnScroll(const Value: TOnScroll);
    function GetVal(const Index: Integer): Integer;
    procedure SetVal(const Index, Value: Integer);
    procedure SetThumbLen(const Index, Value: Integer);
  protected
  public
    property OnScroll: TOnScroll read GetOnScroll write SetOnScroll;
    property RangeMin: Integer index $80010007 read GetVal write SetVal;
    property RangeMax: Integer index $80020008 read GetVal write SetVal;
    property PageSize: Integer index $00160015 read GetVal write SetVal;
    property LineSize: Integer index $00180017 read GetVal write SetVal;
    property Position: Integer index $80000005 read GetVal write SetVal;
    property NumTicks: Integer index $00100000 read GetVal;
    property SelStart: Integer index $0011000B read GetVal write SetVal;
    property SelEnd: Integer index $0012000C read GetVal write SetVal;
    property ThumbLen: Integer index $001B0000 read GetVal write SetThumbLen;

    function ChannelRect: TRect;
  end;

  PTrackbarData = ^TTrackbarData;
  TTrackbarData = packed record
    FOnScroll: TOnScroll;
  end;

  TKOLTrackbar = PTrackbar;

  { SPC CONTROLS }

  TSortBy = (sbName, sbExtention);

  PSPCDirectoryEdit = ^TSPCDirectoryEdit;
  TSPCDirectoryEditBox = PSPCDirectoryEdit;
  TSPCDirectoryEdit = object(TObj)
  private
    { Private declarations }
    fCreated: Boolean;
    fBorder: Integer;
    fControl: PControl;
    fEdit: PControl;
    fButton: PControl;
    fDirList: POpenDirDialog;
    fFont: PGraphicTool;
    fPath: string;
    fTitle: string;
    fCaptionEmpty: string;
    fOnChange: TOnEvent;
    fColor: TColor;
    function GetTop: Integer;
    procedure SetTop(Value: Integer);
    function GetLeft: Integer;
    procedure SetLeft(Value: Integer);
    function GetHeight: Integer;
    procedure SetHeight(Value: Integer);
    function GetWidth: Integer;
    procedure SetWidth(Value: Integer);
    procedure DoClick(Sender: PObj);
    procedure SetPath(Value: string);
  protected
    { Protected declarations }
  public
    destructor Destroy; virtual;
    procedure Initialize;
    function SetAlign(Value: TControlAlign): PSPCDirectoryEdit; overload;
    function SetPosition(X, Y: integer): PSPCDirectoryEdit; overload;
    function SetSize(X, Y: integer): PSPCDirectoryEdit; overload;
    function GetFont: PGraphicTool;
    property Border: Integer read fBorder write fBorder;
    { Public declarations }
    property Font: PGraphicTool read GetFont;
    property Color: TColor read fColor write fColor;
    property Title: string read fTitle write fTitle;
    property Path: string read fPath write SetPath;
    property OnChange: TOnEvent read fOnChange write fOnChange;
    property CaptionEmpty: string read fCaptionEmpty write fCaptionEmpty;
    property Height: Integer read GetHeight write SetHeight;
    property Width: Integer read GetWidth write SetWidth;
    property Top: Integer read GetTop write SetTop;
    property Left: Integer read GetLeft write SetLeft;
  end;

  TCase = (ctDefault, ctLower, ctUpper);

  PSPCFileList = ^TSPCFileList;
  TSPCFileListBox = PSPCFileList;
  TSPCFileList = object(TObj)
  private
    { Private declarations }
    fColor: TColor;
    fIcons: PImageList;
    fFilters: string;
    fIntegralHeight: Boolean;
    fFileList: PDirList;
    fControl: PControl;
    fPath: KOLString;
    fFont: PGraphicTool;
    FOnSelChange: TOnEvent;
    fDoCase: TCase;
    fHasBorder: Boolean;
    fOnPaint: TOnPaint;
    fExecuteOnDblClk: Boolean;
    fSortBy: TSortBy;
    FOnMouseDblClick: TOnMouse;
    function GetVisible: Boolean; // Edited
    procedure SetVisible(Value: Boolean); // Edited
    function GetFocused: Boolean;
    procedure SetFocused(Value: Boolean);
    function GetTop: Integer;
    procedure SetTop(Value: Integer);
    function GetLeft: Integer;
    procedure SetLeft(Value: Integer);
    function GetHeight: Integer;
    procedure SetHeight(Value: Integer);
    function GetWidth: Integer;
    procedure SetWidth(Value: Integer);
    procedure DoSelChange(Sender: PObj);
    procedure SetPath(Value: KOLString);
    procedure SetFilters(Value: string);
    procedure SetIntegralHeight(Value: Boolean);
    function GetCurIndex: Integer;
    procedure SetCurIndex(Value: Integer);
    procedure SetHasBorder(Value: Boolean);
    function GetSelected(Index: Integer): Boolean;
    procedure SetSelected(Index: Integer; Value: Boolean);
    function GetItem(Index: Integer): string;
    function DrawOneItem(Sender: PObj; DC: HDC; const Rect: TRect; ItemIdx: Integer; DrawAction: TDrawAction; ItemState: TDrawState): Boolean;
    procedure DoMouseDblClk(Sender: PControl; var Mouse: TMouseEventData);
    procedure SetFont(Value: PGraphicTool);
    procedure SetSortBy(Value: TSortBy);
  protected
    { Protected declarations }
  public
    property _SortBy: TSortBy read fSortBy write SetSortBy;
    property OnMouseDblClk: TOnMouse read FOnMouseDblClick write FOnMouseDblClick;
    destructor Destroy; virtual;
    function GetFileName: string;
    function GetFullFileName: string;
    property Selected[Index: Integer]: Boolean read GetSelected write SetSelected;
    property Items[Index: Integer]: string read GetItem;
    function TotalSelected: Integer;
    function SetPosition(X, Y: integer): PSPCFileList; overload;
    function SetSize(X, Y: integer): PSPCFileList; overload;
    function SetAlign(Value: TControlAlign): PSPCFileList; overload;
    function GetFont: PGraphicTool;
    { Public declarations }
    property Color: TColor read fColor write fColor;
    property Font: PGraphicTool read GetFont write SetFont;
    property IntegralHeight: Boolean read fIntegralHeight write SetIntegralHeight;
    property Path: KOLstring read fPath write SetPath;
    property Filters: string read fFilters write SetFilters;
    property OnSelChange: TOnEvent read FOnSelChange write FOnSelChange;
    property OnPaint: TOnPaint read FOnPaint write FOnPaint;
    property CurIndex: Integer read GetCurIndex write SetCurIndex;
    function Count: LongInt;
    property DoCase: TCase read fDoCase write fDoCase;
    property HasBorder: Boolean read fHasBorder write SetHasBorder;
    property Height: Integer read GetHeight write SetHeight;
    property Width: Integer read GetWidth write SetWidth;
    property Top: Integer read GetTop write SetTop;
    property Left: Integer read GetLeft write SetLeft;
    property Visible: Boolean read GetVisible write SetVisible; // Edited
    property Focused: Boolean read GetFocused write SetFocused;
    property ExecuteOnDblClk: Boolean read fExecuteOnDblClk write fExecuteOnDblClk;
    procedure SortByName;
    procedure SortByExtention;
  end;

  PSPCDirectoryList = ^TSPCDirectoryList;
  TSPCDirectoryListBox = PSPCDirectoryList;
  TSPCDirectoryList = object(TObj)
  private
    { Private declarations }
    fColor: TColor;
    fDoIndent: Boolean;
    fTotalTree: Integer;
    fDIcons: PImageList;
    fFOLDER: PIcon;
    fInitialized: Integer;
    fCreated: Boolean;
    fIntegralHeight: Boolean;
    fDirList: PDirList;
    fCurIndex: Integer;
    fControl: PControl;
    fPath: string;
    fFont: PGraphicTool;
    FOnMouseDblClick: TOnMouse;
    fLVBkColor: Integer;
    fOnChange: TOnEvent;
    fFileListBox: PSPCFileList;
    function GetTop: Integer;
    procedure SetTop(Value: Integer);
    function GetLeft: Integer;
    procedure SetLeft(Value: Integer);
    function GetHeight: Integer;
    procedure SetHeight(Value: Integer);
    function GetWidth: Integer;
    procedure SetWidth(Value: Integer);
    procedure DoMouseDblClick(Sender: PControl; var Mouse: TMouseEventData);
    procedure SetPath(Value: string);
    procedure SetFileListBox(Value: PSPCFileList);
  protected
    { Protected declarations }
  public
    destructor Destroy; virtual;
    property FileListBox: PSPCFileList read fFileListBox write SetFileListBox;
    function SetAlign(Value: TControlAlign): PSPCDirectoryList; overload;
    function SetPosition(X, Y: integer): PSPCDirectoryList; overload;
    function SetSize(X, Y: integer): PSPCDirectoryList; overload;
    function GetFont: PGraphicTool;
    property Color: TColor read fColor write fColor;
    { Public declarations }
    property Font: PGraphicTool read GetFont;
    property IntegralHeight: Boolean read fIntegralHeight write fIntegralHeight;
    property Path: string read fPath write SetPath;
    property DoIndent: Boolean read fDoIndent write fDoIndent;
    property OnMouseDblClk: TOnMouse read FOnMouseDblClick write FOnMouseDblClick;
    property CurIndex: Integer read fCurIndex write fCurIndex;
    property LVBkColor: Integer read fLVBkColor write fLVBkColor;
    property OnChange: TOnEvent read fOnChange write fOnChange;
    property Height: Integer read GetHeight write SetHeight;
    property Width: Integer read GetWidth write SetWidth;
    property Top: Integer read GetTop write SetTop;
    property Left: Integer read GetLeft write SetLeft;
  end;

  PSPCDriveCombo = ^TSPCDriveCombo;
  TSPCDriveComboBox = PSPCDriveCombo;
  TSPCDriveCombo = object(TObj)
  private
    { Private declarations }
    fIcons: PImageList;
    fColor: TColor;
    fInitialized: Integer;
    fCurIndex: Integer;
    fControl: PControl;
    fDrive: KOLChar;
    fFont: PGraphicTool;
    fLVBkColor: Integer;
    fOnChange: TOnEvent;
    //    fOnChangeInternal: TOnEvent;
    fAOwner: PControl;
    fDirectoryListBox: PSPCDirectoryList;
    function GetTop: Integer;
    procedure SetTop(Value: Integer);
    function GetLeft: Integer;
    procedure SetLeft(Value: Integer);
    function GetHeight: Integer;
    procedure SetHeight(Value: Integer);
    function GetWidth: Integer;
    procedure SetWidth(Value: Integer);
    procedure SetDrive(Value: KOLChar);
    procedure BuildList;
    procedure DoChange(Obj: PObj);
    //    procedure DoChangeInternal(Obj: PObj);
    function DoMeasureItem(Sender: PObj; Idx: Integer): Integer;
    function DrawOneItem(Sender: PObj; DC: HDC; const Rect: TRect; ItemIdx: Integer; DrawAction: TDrawAction; ItemState: TDrawState): Boolean;
  protected
    { Protected declarations }
  public
    destructor Destroy; virtual;
    function SetAlign(Value: TControlAlign): PSPCDriveCombo; overload;
    function SetPosition(X, Y: integer): PSPCDriveCombo; overload;
    function SetSize(X, Y: integer): PSPCDriveCombo; overload;
    function GetFont: PGraphicTool;
    procedure SetFont(Value: PGraphicTool);
    property Color: TColor read fColor write fColor;
    { Public declarations }
    property DirectoryListBox: PSPCDirectoryList read fDirectoryListBox write fDirectoryListBox;
    property Font: PGraphicTool read GetFont write SetFont;
    property Drive: KOLChar read fDrive write SetDrive;
    property CurIndex: Integer read fCurIndex write fCurIndex;
    property LVBkColor: Integer read fLVBkColor write fLVBkColor;
    property OnChange: TOnEvent read fOnChange write fOnChange;
    property Height: Integer read GetHeight write SetHeight;
    property Width: Integer read GetWidth write SetWidth;
    property Top: Integer read GetTop write SetTop;
    property Left: Integer read GetLeft write SetLeft;
  end;

  TFilterItem = class
  private
    fFull: string;
    fDescription: string;
    fFilter: string;
  public
  published
    property Full: string read fFull write fFull;
    property Description: string read fDescription write fDescription;
    property Filter: string read fFilter write fFilter;
  end;

  PSPCFilterCombo = ^TSPCFilterCombo;
  TSPCFilterComboBox = PSPCFilterCombo;
  TSPCFilterCombo = object(TObj)
  private
    { Private declarations }
    fColor: TColor;
    fCurIndex: Integer;
    fControl: PControl;
    fFont: PGraphicTool;
    fLVBkColor: Integer;
    fOnChange: TOnEvent;
    fFilterItems: PList;
    fFilter: string;
    fCreated: Boolean;
    fInitialized: Integer;
    fFileListBox: PSPCFileList;
    ftext: string;
    function GetTop: Integer;
    procedure SetTop(Value: Integer);
    function GetLeft: Integer;
    procedure SetLeft(Value: Integer);
    function GetHeight: Integer;
    procedure SetHeight(Value: Integer);
    function GetWidth: Integer;
    procedure SetWidth(Value: Integer);
    function GetFilterItem(Index: Integer): TFilterItem;
    procedure SetFilter(Value: string);
    procedure SetCurIndex(Value: Integer);
    function GetCurIndex: Integer;
    procedure DoChange(Obj: PObj);
    function DoMeasureItem(Sender: PObj; Idx: Integer): Integer;
    function GetItem(Index: Integer): string;
    procedure SetItem(Index: Integer; Value: string);
    function GetFilter: string;
  protected
    { Protected declarations }
  public
    destructor Destroy; virtual;
    procedure Update;
    procedure Add(fNewFilter: string);
    procedure DeleteItem(Index: Integer);
    function Count: Integer;
    procedure BuildList;
    property FileListBox: PSPCFileList read fFileListBox write fFileListBox;
    function SetAlign(Value: TControlAlign): PSPCFilterCombo; overload;
    function SetPosition(X, Y: integer): PSPCFilterCombo; overload;
    function SetSize(X, Y: integer): PSPCFilterCombo; overload;
    function GetFont: PGraphicTool;
    procedure SetFont(Value: PGraphicTool);
    property Filter: string read GetFilter write SetFilter;
    property Color: TColor read fColor write fColor;
    { Public declarations }
    property Text: string read fText write fText;
    property Font: PGraphicTool read GetFont write SetFont;
    property CurIndex: Integer read GetCurIndex write SetCurIndex;
    property LVBkColor: Integer read fLVBkColor write fLVBkColor;
    property OnChange: TOnEvent read fOnChange write fOnChange;
    property Items[Index: Integer]: string read GetItem write SetItem;
    property Filters[Index: Integer]: TFilterItem read GetFilterItem;
    property Height: Integer read GetHeight write SetHeight;
    property Width: Integer read GetWidth write SetWidth;
    property Top: Integer read GetTop write SetTop;
    property Left: Integer read GetLeft write SetLeft;
  end;

  PSPCStatus = ^TSPCStatus;
  TSPCStatusBar = PSPCStatus;
  TSPCStatus = object(TControl)
  private
    { Private declarations }
    fControl: PControl;
    function GetTop: Integer;
    procedure SetTop(Value: Integer);
    function GetLeft: Integer;
    procedure SetLeft(Value: Integer);
    function GetHeight: Integer;
    procedure SetHeight(Value: Integer);
    function GetWidth: Integer;
    procedure SetWidth(Value: Integer);
    procedure SetSimpleStatusText(Value: string);
    function GetSimpleStatusText: string;
  protected
    { Protected declarations }
  public
    destructor Destroy; virtual;
    function SetAlign(Value: TControlAlign): PSPCStatus; overload;
    function SetPosition(X, Y: integer): PSPCStatus; overload;
    function SetSize(X, Y: integer): PSPCStatus; overload;
    function GetFont: PGraphicTool;
    procedure SetFont(Value: PGraphicTool);
    { Public declarations }
    property Font: PGraphicTool read GetFont write SetFont;
    property SimpleStatusText: string read GetSimpleStatusText write SetSimpleStatusText;
    property Height: Integer read GetHeight write SetHeight;
    property Width: Integer read GetWidth write SetWidth;
    property Top: Integer read GetTop write SetTop;
    property Left: Integer read GetLeft write SetLeft;
    //    property SizeGrip;
  end;

function NewTrackbar(AParent: PControl; Options: TTrackbarOptions; OnScroll: TOnScroll): PTrackbar;

function CheckBit(Value, Index: LongInt): Boolean;
function GetLastPos(c: char; s: string): Integer;
function NewTSPCDirectoryEditBox(AOwner: PControl): PSPCDirectoryEdit;
function NewTSPCDirectoryListBox(AOwner: PControl; Style: TListViewStyle; Options: TListViewOptions; ImageListSmall, ImageListNormal, ImageListState: PImageList): PSPCDirectoryList;
function NewTSPCDriveComboBox(AOwner: PControl; Options: TComboOptions): PSPCDriveCombo;
function NewTSPCFileListBox(AOwner: PControl; Options: TListOptions): PSPCFileList;
function NewTSPCFilterComboBox(AOwner: PControl; Options: TComboOptions): PSPCFilterCombo;
function NewTSPCStatusBar(AOwner: PControl): PSPCStatus;

implementation

function WndProcTrackbarParent(Sender: PControl; var Msg: TMsg; var Rslt: Integer): Boolean;
var
  D                 : PTrackbarData;
  Trackbar          : PTrackbar;
begin
  Result := False;
  if (Msg.message = WM_HSCROLL) or (Msg.message = WM_VSCROLL) then
    if (Msg.lParam <> 0) then begin
      Trackbar := Pointer({$IFDEF USE_PROP}
        GetProp(Msg.lParam, ID_SELF)
{$ELSE}
        GetWindowLong(Msg.lParam, GWL_USERDATA)
{$ENDIF});
      if Assigned(Trackbar) then begin
        D := Trackbar.CustomData;
        if Assigned(D.FOnScroll) then
          D.FOnScroll(Trackbar, Msg.wParam);
      end;
    end;
end;

function TTrackbar.ChannelRect: TRect;
begin
  Perform( TBM_GETCHANNELRECT, 0, Integer( @ Result ) );
end;

function NewTrackbar(AParent: PControl; Options: TTrackbarOptions; OnScroll: TOnScroll): PTrackbar;
const
  TrackbarOptions   : array[TTrackbarOption] of Integer = (TBS_AUTOTICKS,
    TBS_ENABLESELRANGE, TBS_FIXEDLENGTH, TBS_NOTHUMB, TBS_NOTICKS, TBS_TOOLTIPS,
    TBS_TOP, TBS_VERT, 0, TBS_BOTH);
var
  aStyle            : DWORD;
  D                 : PTrackbarData;
  W, H              : Integer;
begin
  DoInitCommonControls(ICC_BAR_CLASSES);
  aStyle := MakeFlags(@Options, TrackbarOptions) or WS_CHILD or WS_VISIBLE;
  Result := PTrackbar(_NewCommonControl(AParent, TRACKBAR_CLASS, aStyle,
    not (trbNoBorder in Options), nil));
  W := 200;
  H := 40;
  if (trbVertical in Options) then begin
    H := W;
    W := 40;
  end;
  Result.Width := W;
  Result.Height := H;
  GetMem(D, Sizeof(D^));
  Result.CustomData := D;
  D.FOnScroll := OnScroll;
  AParent.AttachProc(WndProcTrackbarParent);
end;

{ TTrackbar }

function TTrackbar.GetOnScroll: TOnScroll;
var
  D                 : PTrackbarData;
begin
  D := CustomData;
  Result := D.FOnScroll;
end;

function TTrackbar.GetVal(const Index: Integer): Integer;
begin
  Result := Perform(WM_USER + (HiWord(Index) and $7FFF), 0, 0);
end;

procedure TTrackbar.SetOnScroll(const Value: TOnScroll);
var
  D                 : PTrackbarData;
begin
  D := CustomData;
  D.FOnScroll := Value;
end;

procedure TTrackbar.SetThumbLen(const Index, Value: Integer);
begin
  Perform(TBM_SETTHUMBLENGTH, Value, 0);
end;

procedure TTrackbar.SetVal(const Index, Value: Integer);
begin
  Perform(WM_USER + LoWord(Index), Index shr 31, Value);
end;

{ TSPCDirectoryEdit }

function NewTSPCDirectoryEditBox;
var
  p                 : PSPCDirectoryEdit;
  c                 : PControl;
begin
  c := NewPanel(AOwner, esNone);
  c.ExStyle := c.ExStyle or WS_EX_CLIENTEDGE;
  New(p, create);
  AOwner.Add2AutoFree(p);
  p.fControl := c;
  p.fFont := NewFont;
  p.fCreated := False;
  Result := p;
end;

function TSPCDirectoryEdit.SetAlign(Value: TControlAlign): PSPCDirectoryEdit;
begin
  fControl.Align := Value;
  Result := @Self;
end;

destructor TSPCDirectoryEdit.Destroy;
begin
  fFont.Free;
  inherited;
end;

function TSPCDirectoryEdit.SetPosition(X, Y: integer): PSPCDirectoryEdit;
begin
  fControl.Left := X;
  fControl.Top := Y;
  Result := @self;
end;

function TSPCDirectoryEdit.SetSize(X, Y: integer): PSPCDirectoryEdit;
begin
  fControl.Width := X;
  fControl.Height := Y;
  Result := @self;
end;

function TSPCDirectoryEdit.GetFont;
begin
  Result := fFont;
end;

procedure TSPCDirectoryEdit.Initialize;
begin
  fEdit := NewEditBox(fControl, [eoReadOnly]);
  fEdit.Font.FontHeight := -11;
  fControl.Height := fEdit.Height - 1;
  fEdit.Left := 0;
  fEdit.Top := 1;
  fEdit.Height := 17;
  fEdit.Width := fControl.Width - 21;
  fEdit.HasBorder := False;
  fEdit.Color := fColor;
  fEdit.Font.Assign(Font);
  fButton := NewBitBtn(fControl, '...', [], glyphLeft, 0, 1);
  fButton.Font.FontHeight := -11;
  fButton.VerticalAlign := vaCenter;
  fButton.LikeSpeedButton;
  fButton.Width := 17;
  fButton.Height := 17;
  fButton.Top := 0;
  fButton.Left := fEdit.Width;
  fButton.OnClick := DoClick;
  fDirList := NewOpenDirDialog(Title, []);
  fDirList.CenterOnScreen := True;
end;

procedure TSPCDirectoryEdit.SetPath(Value: string);
begin
  if DirectoryExists(Value) then fPath := Value else fPath := '';
  if Length(fPath) = 0 then fEdit.Text := CaptionEmpty else fEdit.Text := fPath;
  if Assigned(fOnChange) then if fCreated then fOnChange(@Self) else fCreated := True;
end;

procedure TSPCDirectoryEdit.DoClick;
begin
  fDirList.InitialPath := Path;
  if fDirList.Execute then begin
    Path := fDirList.Path;
    fEdit.Text := fDirList.Path;
  end;
end;

function TSPCDirectoryEdit.GetHeight: Integer;
begin
  Result := fControl.Height;
end;

procedure TSPCDirectoryEdit.SetHeight(Value: Integer);
begin
  fControl.Height := Value;
end;

function TSPCDirectoryEdit.GetWidth: Integer;
begin
  Result := fControl.Width;
end;

procedure TSPCDirectoryEdit.SetWidth(Value: Integer);
begin
  fControl.Width := Value;
end;

function TSPCDirectoryEdit.GetTop: Integer;
begin
  Result := fControl.Top;
end;

procedure TSPCDirectoryEdit.SetTop(Value: Integer);
begin
  fControl.Top := Value;
end;

function TSPCDirectoryEdit.GetLeft: Integer;
begin
  Result := fControl.Left;
end;

procedure TSPCDirectoryEdit.SetLeft(Value: Integer);
begin
  fControl.Left := Value;
end;

{ TSPCDirectoryList }

function NewTSPCDirectoryListBox;
var
  p                 : PSPCDirectoryList;
  c                 : PControl;
  Shell32           : LongInt;
begin
  c := NewListView(AOwner, lvsDetailNoHeader, [], ImageListSmall, ImageListNormal, ImageListState);
  New(p, create);
  AOwner.Add2AutoFree(p);
  p.fControl := c;
  p.fControl.OnMouseDblClk := p.DoMouseDblClick;
  p.fControl.lvOptions := [lvoRowSelect, lvoInfoTip, lvoAutoArrange];
  p.fCreated := False;
  p.fDirList := NewDirList('', '', 0);
  p.fFont := NewFont;
  p.fDIcons := NewImageList(AOwner);
  p.fDIcons.LoadSystemIcons(True);
  Shell32 := LoadLibrary('shell32.dll');
  p.fFOLDER := NewIcon;
  p.fFOLDER.LoadFromResourceID(Shell32, 4, 16);
  p.fDIcons.ReplaceIcon(0, p.fFOLDER.Handle);
  p.fFOLDER.LoadFromResourceID(Shell32, 5, 16);
  p.fDIcons.ReplaceIcon(1, p.fFOLDER.Handle);
  FreeLibrary(Shell32);
  p.fFOLDER.Free;
  p.fControl.ImageListSmall := p.fDIcons;
  p.fInitialized := 0;
  Result := p;
end;

function TSPCDirectoryList.SetAlign(Value: TControlAlign): PSPCDirectoryList;
begin
  fControl.Align := Value;
  Result := @Self;
end;

procedure TSPCDirectoryList.DoMouseDblClick;
var
  s                 : string;
  i                 : Integer;
begin
  if fControl.lvCurItem > -1 then begin
    s := '';
    if fControl.LVCurItem <= fTotalTree - 1 then begin
      for i := 0 to fControl.LVCurItem do s := s + fControl.lvItems[i, 0] + '\';
    end else begin
      for i := 0 to fTotalTree - 1 do s := s + fControl.lvItems[i, 0] + '\';
      s := s + fControl.lvItems[fControl.lvCurItem, 0];
    end;
    Path := s;
    if Assigned(fOnMouseDblClick) then fOnMouseDblClick(@Self, Mouse);
  end;
end;

destructor TSPCDirectoryList.Destroy;
begin
  fFont.Free;
  inherited;
end;

function TSPCDirectoryList.SetPosition(X, Y: integer): PSPCDirectoryList;
begin
  fControl.Left := X;
  fControl.Top := Y;
  Result := @self;
end;

function TSPCDirectoryList.SetSize(X, Y: integer): PSPCDirectoryList;
begin
  fControl.Width := X;
  fControl.Height := Y;
  Result := @self;
end;

function TSPCDirectoryList.GetFont;
begin
  Result := fFont;
end;

procedure TSPCDirectoryList.SetPath(Value: string);
var
  TPath, fValue     : string;
  i, z              : Integer;
  LastDir           : Cardinal;
  fImgIndex         : Integer;
  Code              : Cardinal;
  fDriveShown       : Boolean;
begin
  fValue := Value;
  fControl.lvBkColor := fColor;
  fControl.lvTextBkColor := fColor;
  if Length(fValue) = 1 then fValue := fValue + ':\';
  if not fCreated then begin
    fCreated := True;
    fControl.LVColAdd('', taRight, fControl.Width);
    //    if fIntegralHeight then
    //    begin
    //      fControl.Height:=(fControl.Height div 16)*16+1;
    //    end;
  end;
  fControl.Clear;
  if DirectoryExists(fValue) then begin
    LastDir := 0;
    fTotalTree := 0;
    if fValue[Length(fValue)] = '\' then TPath := fValue else TPath := fValue + '\';
    fPath := TPath;
    fDriveShown := False;
    fImgIndex := -1;
    repeat
      if fTotalTree > 0 then fImgIndex := 1;
      if not fDriveShown then begin
        fDriveShown := True;
        fImgIndex := FileIconSystemIdx(Copy(TPath, 1, 3));
      end;
      fControl.LVAdd(Copy(TPath, 1, Pos('\', TPath) - 1), fImgIndex, [], 0, 0, 0);
      fControl.LVItemIndent[LastDir] := LastDir;
      Delete(TPath, 1, Pos('\', TPath));
      if DoIndent then Inc(LastDir);
      Inc(fTotalTree);
    until Length(TPath) = 0;
    fDirList.ScanDirectory(fValue, '*.*', FILE_ATTRIBUTE_NORMAL);
    fDirList.Sort([sdrByName]);
    z := -1;
    for i := 0 to fDirList.Count - 1 do begin
      Code := fDirList.Items[i].dwFileAttributes;
      if Code = (Code or $10) then
        if not (fDirList.Names[i] = '.') then
          if not (fDirList.Names[i] = '..') then begin
            Inc(z);
            fControl.LVAdd(fDirList.Names[i], 0, [], 0, 0, 0);
            if DoIndent then fControl.LVItemIndent[z + fTotalTree] := LastDir else fControl.LVItemIndent[z + fTotalTree] := 1;
          end;
    end;
  end else begin
    fPath := '';
  end;
  Inc(fInitialized);
  if fInitialized > 2 then fInitialized := 2;
  if Assigned(OnChange) then if fInitialized = 2 then OnChange(@Self);
  if Assigned(fFileListBox) then fFileListBox.Path := Path;
  fControl.LVColWidth[0] := -2;
end;

function TSPCDirectoryList.GetHeight: Integer;
begin
  Result := fControl.Height;
end;

procedure TSPCDirectoryList.SetHeight(Value: Integer);
begin
  fControl.Height := Value;
end;

function TSPCDirectoryList.GetWidth: Integer;
begin
  Result := fControl.Width;
end;

procedure TSPCDirectoryList.SetWidth(Value: Integer);
begin
  fControl.Width := Value;
end;

function TSPCDirectoryList.GetTop: Integer;
begin
  Result := fControl.Top;
end;

procedure TSPCDirectoryList.SetTop(Value: Integer);
begin
  fControl.Top := Value;
end;

function TSPCDirectoryList.GetLeft: Integer;
begin
  Result := fControl.Left;
end;

procedure TSPCDirectoryList.SetLeft(Value: Integer);
begin
  fControl.Left := Value;
end;

procedure TSPCDirectoryList.SetFileListBox(Value: PSPCFileList);
begin
  fFileListBox := Value;
  fFileListBox.Path := Path;
end;

{ TSPCDriveCombo }

function CheckBit;
var
  fL                : LongInt;
begin
  fL := Value;
  fL := fL shr Index;
  fL := fL and $01;
  Result := (fL = 1);
end;

function NewTSPCDriveComboBox;
var
  p                 : PSPCDriveCombo;
  c                 : PControl;
begin
  c := NewComboBox(AOwner, [coReadOnly, coOwnerDrawVariable]);
  New(p, create);
  AOwner.Add2AutoFree(p);
  p.fControl := c;
  p.fFont := NewFont;
  p.fFont.FontHeight := -8;
  p.fControl.Font.Assign(p.fFont);
  p.fIcons := NewImageList(AOwner);
  p.fIcons.LoadSystemIcons(True);
  p.fAOwner := AOwner;
  p.fControl.OnDrawItem := p.DrawOneItem;
  p.fControl.OnChange := p.DoChange;
  p.fControl.OnMeasureItem := p.DoMeasureItem;
  p.BuildList;
  p.fInitialized := 0;
  p.fControl.Color := $FF0000;
  Result := p;
end;

procedure TSPCDriveCombo.DoChange(Obj: PObj);
begin
  Drive := fControl.Items[fControl.CurIndex][1];
  SetCurrentDirectory(PKOLChar(Drive + ':\'));
  if Assigned(fOnChange) then fOnChange(@Self);
  if Assigned(fDirectoryListBox) then fDirectoryListBox.Path := Drive;
end;

destructor TSPCDriveCombo.Destroy;
begin
  fFont.Free;
  inherited;
end;

function TSPCDriveCombo.SetAlign(Value: TControlAlign): PSPCDriveCombo;
begin
  fControl.Align := Value;
  Result := @Self;
end;

function TSPCDriveCombo.SetPosition(X, Y: integer): PSPCDriveCombo;
begin
  fControl.Left := X;
  fControl.Top := Y;
  Result := @self;
end;

function TSPCDriveCombo.SetSize(X, Y: integer): PSPCDriveCombo;
begin
  fControl.Width := X;
  fControl.Height := Y;
  Result := @self;
end;

function TSPCDriveCombo.GetFont;
begin
  Result := fFont;
end;

procedure TSPCDriveCombo.SetFont(Value: PGraphicTool);
begin
  fFont := Value;
  fControl.Font.Assign(Value);
end;

procedure TSPCDriveCombo.SetDrive;
var
  fC                : KOLChar;
begin
  fControl.Font.Assign(fFont);
  fControl.Color := fColor;
  fC := Value;
  if fControl.SearchFor(fc, 0, True) > -1 then begin
    fDrive := fC;
    fControl.CurIndex := fControl.SearchFor(fc, 0, True);
  end;
  Inc(fInitialized);
  if fInitialized > 2 then fInitialized := 2;
  if Assigned(fOnChange) then if fInitialized = 2 then fOnChange(@Self);
end;

function VolumeID(DriveChar: KOLChar): string;
var
  NotUsed, VolFlags : DWORD;
  Buf               : array[0..MAX_PATH] of KOLChar;
begin
  if GetVolumeInformation(PKOLChar(DriveChar + ':\'), Buf, DWORD(sizeof(Buf)), nil, NotUsed, VolFlags, nil, 0) then
    Result := buf//Copy(Buf, 1, StrLen(Buf))
  else
    Result := '';
end;

function dr_property(path: KOLString): KOLString;
var
  Cpath             : PKOLChar;
  Spath             : KOLChar;
begin
  Result := '';
  Cpath := PKOLChar(Copy(path, 1, 2));
  Spath := Cpath[0];
  case GetDriveType(Cpath) of
    0: Result := '<unknown>'; //Не известен
    1: Result := '<disabled>'; //Не существует :)
    DRIVE_REMOVABLE: Result := 'Removable'; //Флопик
    DRIVE_FIXED: if Length(VolumeID(Spath)) > 0 then Result := VolumeID(Spath) else Result := 'Local Disk'; //HDD
    DRIVE_REMOTE: if Length(VolumeID(Spath)) > 0 then Result := VolumeID(Spath) else Result := 'Net Disk'; //Внешний носитель
    //         DRIVE_REMOTE: if Length(VolumeID(Spath))>0 then Result:=NetworkVolume(Spath) else Result:='Net Disk';//Внешний носитель
    DRIVE_CDROM: if Length(VolumeID(Spath)) > 0 then Result := VolumeID(Spath) else Result := 'Compact Disc'; //CD
    DRIVE_RAMDISK: if Length(VolumeID(Spath)) > 0 then Result := VolumeID(Spath) else Result := 'Removable Disk'; //Внешний носитель
  end;
end;

procedure TSPCDriveCombo.BuildList;
var
  b                 : Byte;
  fFlags            : LongInt;
  fDir              : string;
  //  a                 : integer;
  fFullPath         : string;
  fdr_property      : string;
begin
  GetDir(0, fDir);
  fControl.Clear;
  fFlags := GetLogicalDrives;
  for b := 0 to 25 do if Boolean(fFlags and (1 shl b)) then begin
      fFullPath := Chr(b + $41) + ':';
      fdr_property := dr_property(fFullPath);
      {a :=}fControl.Add(Chr(b + $41) + '  ' + fdr_property);
    end;
  fControl.CurIndex := fControl.SearchFor(fDir[1], 0, True);
  fControl.Update;
end;

function TSPCDriveCombo.DrawOneItem(Sender: PObj; DC: HDC; //aded by tamerlan311
  const Rect: TRect; ItemIdx: Integer; DrawAction: TDrawAction;
  ItemState: TDrawState): Boolean;
var
  T_Rect            : TRect;
  B_Rect            : TRect;
  Ico               : Integer;
begin
  SetBkMode(DC, opaque);
  if ItemIdx > -1 then begin
    //PControl(Sender).CanResize := True;
    T_Rect := Rect;
    B_Rect := Rect;
    T_Rect.Left := Rect.Left + 19;
    B_Rect.Left := Rect.Left + 18;
    PControl(Sender).Canvas.Pen.PenMode := pmCopy;
    PControl(Sender).Canvas.Pen.Color := $0000FF;
    PControl(Sender).Brush.Color := clWindow;
    if (odsFocused in ItemState) or (odsSelected in ItemState) then begin
      SetBkMode(DC, TRANSPARENT);
      PControl(Sender).Canvas.Brush.color := clWindow;
      FillRect(DC, T_Rect, PControl(Sender).Canvas.Brush.Handle);
      if (not (odsFocused in ItemState)) and ((odsSelected in ItemState)) then begin
        PControl(Sender).Canvas.Brush.color := clInactiveBorder;
        SetTextColor(DC, Font.Color);
        fIcons.DrawingStyle := [];
      end else begin
        PControl(Sender).Canvas.Brush.color := clHighLight;
        SetTextColor(DC, $FFFFFF);
        fIcons.DrawingStyle := [dsBlend50];
      end;
      FillRect(DC, T_Rect, PControl(Sender).Canvas.Brush.Handle);
    end else begin
      SetTextColor(DC, Font.Color);
      PControl(Sender).Canvas.Brush.color := clWindow;
      SelectObject(DC, PControl(Sender).Canvas.Brush.Handle);
      FillRect(DC, B_Rect, PControl(Sender).Canvas.Brush.Handle);
      fIcons.DrawingStyle := [];
    end;
    Ico := FileIconSystemIdx(PControl(Sender).Items[ItemIdx][1] + ':\');
    fIcons.Draw(Ico, DC, Rect.Left + 1, Rect.Top);
    DrawText(DC, PKOLChar(PControl(Sender).Items[ItemIdx]), Length(PControl(Sender).Items[ItemIdx]), T_Rect, DT_SINGLELINE or DT_VCENTER or DT_NOPREFIX);
  end;
 // PControl(Sender).Update;
  Result := True;         ///
end;

function TSPCDriveCombo.GetHeight: Integer;
begin
  Result := fControl.Height;
end;

procedure TSPCDriveCombo.SetHeight(Value: Integer);
begin
  fControl.Height := Value;
end;

function TSPCDriveCombo.GetWidth: Integer;
begin
  Result := fControl.Width;
end;

procedure TSPCDriveCombo.SetWidth(Value: Integer);
begin
  fControl.Width := Value;
end;

function TSPCDriveCombo.GetTop: Integer;
begin
  Result := fControl.Top;
end;

procedure TSPCDriveCombo.SetTop(Value: Integer);
begin
  fControl.Top := Value;
end;

function TSPCDriveCombo.GetLeft: Integer;
begin
  Result := fControl.Left;
end;

procedure TSPCDriveCombo.SetLeft(Value: Integer);
begin
  fControl.Left := Value;
end;

function TSPCDriveCombo.DoMeasureItem(Sender: PObj; Idx: Integer): Integer;
begin
  Result := 16;
end;

{ TSPCFileList }

function NewTSPCFileListBox;
var
  p                 : PSPCFileList;
begin
  Options := Options + [loOwnerDrawFixed];
  New(p, Create);
  AOwner.Add2AutoFree(p);
  p.fControl := NewListBox(AOwner, Options);
  //   p.fControl.OnMouseDblClk:=p.DoMouseDblClick;
  p.fControl.OnChange := p.DoSelChange;
  p.fControl.Font.FontHeight := -8;
  p.fFileList := NewDirList('', '', 0);
  p.fControl.OnDrawItem := p.DrawOneItem;
  p.fFont := NewFont;
  p.fIcons := NewImageList(nil);
  p.fIcons.LoadSystemIcons(true);
  p.fControl.OnMouseDblClk := p.DoMouseDblClk;
  p.fControl.Font.FontHeight := -11;
  Result := p;
end;

function TSPCFileList.SetAlign(Value: TControlAlign): PSPCFileList;
begin
  fControl.Align := Value;
  Result := @Self;
end;

procedure TSPCFileList.SetFilters(Value: string);
begin
  fFilters := Value;
  Path := Path;
end;

procedure TSPCFileList.DoSelChange;
begin
  if Assigned(fOnSelChange) then fOnSelChange(@Self);
end;

destructor TSPCFileList.Destroy;
begin
  fFont.Free;
  inherited;
end;

function TSPCFileList.SetPosition(X, Y: integer): PSPCFileList;
begin
  fControl.Left := X;
  fControl.Top := Y;
  Result := @self;
end;

function TSPCFileList.SetSize(X, Y: integer): PSPCFileList;
begin
  fControl.Width := X;
  fControl.Height := Y;
  Result := @self;
end;

function TSPCFileList.GetFont;
begin
  Result := fControl.Font;
end;

procedure TSPCFileList.SetFont(Value: PGraphicTool);
begin
  fControl.Font.Assign(Value);
end;

procedure TSPCFileList.SetPath(Value: KOLstring);
var
  i                 : Integer;
  fValue            : string;
begin
  fValue := Value;
  if Length(fValue) > 0 then begin
    if not (fValue[Length(fValue)] = '\') then fValue := fValue + '\';
  end;
  if DirectoryExists(fValue) then begin
    fFileList.Clear;
    fFileList.ScanDirectoryEx(FileShortPath(fValue), Filters, FILE_ATTRIBUTE_NORMAL and not FILE_ATTRIBUTE_DIRECTORY);
    fControl.Clear;
    fControl.Color := fColor;
    case _SortBy of
      sbName: fFileList.Sort([sdrByName]);
      sbExtention: fFileList.Sort([sdrByExt]);
    end;
    for i := 1 to fFileList.Count do if not fFileList.IsDirectory[i - 1] then fControl.Add(fFileList.Names[i - 1]);
    fPath := fValue;
    if fDoCase = ctLower then for i := 0 to fControl.Count - 1 do fControl.Items[i] := LowerCase(fControl.Items[i]);
    if fDoCase = ctUpper then for i := 0 to fControl.Count - 1 do fControl.Items[i] := UpperCase(fControl.Items[i]);
  end else begin
    fControl.Clear;
    fPath := '';
  end;
  if fIntegralHeight then begin
    fControl.Height := Round(fControl.Height / 16) * 16 + 4;
  end;
end;

procedure TSPCFileList.SetIntegralHeight;
begin
  fIntegralHeight := Value;
  if fIntegralHeight then begin
    fControl.Height := (fControl.Height div 14) * 14 + 6;
  end;
end;

function TSPCFileList.GetFileName: string;
begin
  Result := fControl.Items[fControl.CurIndex];
end;

function TSPCFileList.GetFullFileName: string;
begin
  Result := Path + fControl.Items[fControl.CurIndex]
end;

function TSPCFileList.Count: LongInt;
begin
  Result := fControl.Count;
end;

function TSPCFileList.GetCurIndex: Integer;
begin
  Result := fControl.CurIndex;
end;

procedure TSPCFileList.SetCurIndex(Value: Integer);
begin
  fControl.CurIndex := Value;
end;

procedure TSPCFileList.SetHasBorder(Value: Boolean);
var
  NewStyle          : DWORD;
begin
  if Value then
    fControl.Style := fControl.Style or WS_THICKFRAME
  else begin
    NewStyle := fControl.Style and not (WS_BORDER or WS_THICKFRAME or WS_DLGFRAME or WS_CAPTION
      or WS_MINIMIZEBOX or WS_MAXIMIZEBOX or WS_SYSMENU or WS_HSCROLL);
    if not fControl.IsControl then NewStyle := NewStyle or WS_POPUP;
    fControl.Style := NewStyle;
    fControl.ExStyle := fControl.ExStyle and not (WS_EX_CONTROLPARENT or WS_EX_DLGMODALFRAME
      or WS_EX_WINDOWEDGE or WS_EX_CLIENTEDGE);
  end;
end;

function TSPCFileList.GetSelected(Index: Integer): Boolean;
begin
  if Index > Count - 1 then Result := False else Result := fControl.ItemSelected[Index];
end;

procedure TSPCFileList.SetSelected(Index: Integer; Value: Boolean);
begin
  if Index <= Count - 1 then fControl.ItemSelected[Index] := Value;
end;

function TSPCFileList.TotalSelected: Integer;
var
  i                 : Integer;
begin
  Result := 0;
  if fControl.Count = 0 then Result := -1 else begin
    for i := 0 to fControl.Count - 1 do if fControl.ItemSelected[i] then Result := Result + 1;
  end;
end;

function TSPCFileList.GetItem(Index: Integer): string;
begin
  Result := fControl.Items[Index];
end;

function TSPCFileList.GetHeight: Integer;
begin
  Result := fControl.Height;
end;

procedure TSPCFileList.SetHeight(Value: Integer);
begin
  fControl.Height := Value;
end;

function TSPCFileList.GetWidth: Integer;
begin
  Result := fControl.Width;
end;

procedure TSPCFileList.SetWidth(Value: Integer);
begin
  fControl.Width := Value;
end;

function TSPCFileList.GetTop: Integer;
begin
  Result := fControl.Top;
end;

procedure TSPCFileList.SetTop(Value: Integer);
begin
  fControl.Top := Value;
end;

function TSPCFileList.GetVisible: Boolean; // Edited
begin
  Result := FControl.Visible;
end;

procedure TSPCFileList.SetVisible(Value: Boolean); // Edited
begin
  FControl.Visible := Value;
end;

function TSPCFileList.GetLeft: Integer;
begin
  Result := fControl.Left;
end;

procedure TSPCFileList.SetLeft(Value: Integer);
begin
  fControl.Left := Value;
end;

function TSPCFileList.GetFocused: Boolean;
begin
  Result := fControl.Focused;
end;

procedure TSPCFileList.SetFocused(Value: Boolean);
begin
  fControl.Focused := Value;
end;

function TSPCFileList.DrawOneItem(Sender: PObj; DC: HDC;
  const Rect: TRect; ItemIdx: Integer; DrawAction: TDrawAction;
  ItemState: TDrawState): Boolean;
var
  T_Rect, B_Rect    : TRect;
  Ico               : Integer;
begin
  SetBkMode(DC, opaque);
  if ItemIdx > -1 then begin
    PControl(Sender).CanResize := True;
    T_Rect := Rect;
    B_Rect := Rect;
    T_Rect.Left := Rect.Left + 19;
    B_Rect.Left := Rect.Left + 18;
    PControl(Sender).Canvas.Pen.PenMode := pmCopy;
    PControl(Sender).Canvas.Pen.Color := $0000FF;
    PControl(Sender).Brush.Color := clWindow;
    if (odsFocused in ItemState) or (odsSelected in ItemState) then begin
      SetBkMode(DC, transparent);
      PControl(Sender).Canvas.Brush.color := clWindow;
      FillRect(DC, T_Rect, PControl(Sender).Canvas.Brush.Handle);
      if (not (odsFocused in ItemState)) and ((odsSelected in ItemState)) then begin
        PControl(Sender).Canvas.Brush.color := clInactiveBorder;
        SetTextColor(DC, Font.Color);
        fIcons.DrawingStyle := [];
      end
      else begin
        PControl(Sender).Canvas.Brush.color := clHighLight;
        SetTextColor(DC, $FFFFFF);
        fIcons.DrawingStyle := [dsBlend50];
      end;
      FillRect(DC, T_Rect, PControl(Sender).Canvas.Brush.Handle);
    end else begin
      SetTextColor(DC, Font.Color);
      PControl(Sender).Canvas.Brush.color := clWindow;
      SelectObject(DC, PControl(Sender).Canvas.Brush.Handle);
      FillRect(DC, B_Rect, PControl(Sender).Canvas.Brush.Handle);
      fIcons.DrawingStyle := [];
    end;
    Ico := FileIconSystemIdx(Path + PControl(Sender).Items[ItemIdx]);
    fIcons.Draw(Ico, DC, Rect.Left + 1, Rect.Top);
    DrawText(DC, PKOLChar(PControl(Sender).Items[ItemIdx]), Length(PControl(Sender).Items[ItemIdx]), T_Rect, DT_SINGLELINE or DT_VCENTER or DT_NOPREFIX);
  end;
  PControl(Sender).Update;
  Result := True;         ///
end;

procedure TSPCFileList.DoMouseDblClk(Sender: PControl; var Mouse: TMouseEventData);
begin
  if ExecuteOnDblClk then
    {$IFDEF UNICODE_CTRLS}
    ShellExecuteW
    {$ELSE}
    ShellExecuteA
    {$ENDIF}
    (fControl.Handle, nil, PKOLChar(Path + Sender.Items[CurIndex]), '', '', SW_SHOW)
  else
    if Assigned(fOnMouseDblClick) then fOnMouseDblClick(@Self, Mouse);
end;

procedure TSPCFileList.SetSortBy(Value: TSortBy);
begin
  fSortBy := Value;
  Path := Path;
end;

procedure TSPCFileList.SortByName;
begin
  _SortBy := sbName;
end;

procedure TSPCFileList.SortByExtention;
begin
  _SortBy := sbExtention;
end;

{ TSPCFilterCombo }

function GetLastPos(c: char; s: string): Integer;
var
  i                 : Integer;
begin
  Result := 0;
  for i := 1 to Length(s) do if s[i] = c then Result := i;
end;

function NewTSPCFilterComboBox;
var
  p                 : PSPCFilterCombo;
  c                 : PControl;
begin
  c := NewComboBox(AOwner, [coReadOnly]);
  New(p, create);
  AOwner.Add2AutoFree(p);
  p.fControl := c;
  p.fFont := NewFont;
  p.fControl.Font.Assign(p.fFont);
  p.Font.FontHeight := -8;
  p.fControl.Font.FontHeight := -8;
  p.fControl.OnChange := p.DoChange;
  p.fControl.OnMeasureItem := p.DoMeasureItem;
  p.fFilterItems := NewList;
  p.fCreated := False;
  p.fInitialized := 0;
  Result := p;
end;

function TSPCFilterCombo.SetAlign(Value: TControlAlign): PSPCFilterCombo;
begin
  fControl.Align := Value;
  Result := @Self;
end;

procedure TSPCFilterCombo.Add;
begin
  fFilterItems.Add(TFilterItem.Create);
  TFilterItem(fFilterItems.Items[fFilterItems.Count - 1]).Description := Copy(fNewFilter, 1, Pos('|', fNewFilter) - 1);
  TFilterItem(fFilterItems.Items[fFilterItems.Count - 1]).Filter := Copy(fNewFilter, Pos('|', fNewFilter) + 1, Length(fNewFilter) - Pos('|', fNewFilter));
  BuildList;
end;

procedure TSPCFilterCombo.DeleteItem;
begin
  fFilterItems.Delete(Index);
end;

function TSPCFilterCombo.Count: Integer;
begin
  Result := fFilterItems.Count;
end;

function TSPCFilterCombo.GetFilterItem;
begin
  Result := fFilterItems.Items[Index];
end;

procedure TSPCFilterCombo.Update;
begin
  DoChange(@Self);
end;

procedure TSPCFilterCombo.DoChange(Obj: PObj);
begin
  Filter := TFilterItem(fFilterItems.Items[fControl.CurIndex]).Filter;
  if Assigned(fOnChange) then fOnChange(@Self);
  if Assigned(fFileListBox) then fFileListBox.Filters := Filter;
end;

destructor TSPCFilterCombo.Destroy;
begin
  fFont.Free;
  inherited;
end;

function TSPCFilterCombo.SetPosition(X, Y: integer): PSPCFilterCombo;
begin
  fControl.Left := X;
  fControl.Top := Y;
  Result := @self;
end;

function TSPCFilterCombo.SetSize(X, Y: integer): PSPCFilterCombo;
begin
  fControl.Width := X;
  fControl.Height := Y;
  Result := @self;
end;

function TSPCFilterCombo.GetFont;
begin
  Result := fFont;
  fControl.Color := $FFFFFF;
end;

procedure TSPCFilterCombo.SetFont(Value: PGraphicTool);
begin
  fFont := Value;
end;

procedure TSPCFilterCombo.BuildList;
var
  i                 : Integer;
begin
  fControl.Color := Color;
  fControl.Font.Assign(Font);
  fControl.Clear;
  if fFilterItems.Count > 0 then
    for i := 1 to fFilterItems.Count do fControl.Add(TFilterItem(fFilterItems.Items[i - 1]).Description);
end;

procedure TSPCFilterCombo.SetFilter(Value: string);
begin
  fFilter := Value;
  if Assigned(fOnChange) then fOnChange(@Self);
end;

procedure TSPCFilterCombo.SetCurIndex(Value: Integer);
begin
  fCurIndex := Value;
  fControl.CurIndex := Value;
  Inc(fInitialized);
  if fInitialized > 2 then fInitialized := 2;
  if Assigned(fOnChange) then if fInitialized = 2 then fOnChange(@Self);
end;

function TSPCFilterCombo.GetHeight: Integer;
begin
  Result := fControl.Height;
end;

procedure TSPCFilterCombo.SetHeight(Value: Integer);
begin
  fControl.Height := Value;
end;

function TSPCFilterCombo.GetWidth: Integer;
begin
  Result := fControl.Width;
end;

procedure TSPCFilterCombo.SetWidth(Value: Integer);
begin
  fControl.Width := Value;
end;

function TSPCFilterCombo.GetTop: Integer;
begin
  Result := fControl.Top;
end;

procedure TSPCFilterCombo.SetTop(Value: Integer);
begin
  fControl.Top := Value;
end;

function TSPCFilterCombo.GetLeft: Integer;
begin
  Result := fControl.Left;
end;

procedure TSPCFilterCombo.SetLeft(Value: Integer);
begin
  fControl.Left := Value;
end;

function TSPCFilterCombo.DoMeasureItem(Sender: PObj; Idx: Integer): Integer;
begin
  Result := 16;
end;

function TSPCFilterCombo.GetItem(Index: Integer): string;
begin
  Result := fControl.Items[Index];
end;

procedure TSPCFilterCombo.SetItem(Index: Integer; Value: string);
begin
  if Index + 1 > fFilterItems.Count then fFilterItems.Add(TFilterItem.Create);
  TFilterItem(fFilterItems.Items[Index]).Description := Copy(Value, 1, Pos('|', Value) - 1);
  TFilterItem(fFilterItems.Items[Index]).Filter := Copy(Value, Pos('|', Value) + 1, Length(Value) - Pos('|', Value));
  BuildList;
end;

function TSPCFilterCombo.GetFilter: string;
begin
  Result := TFilterItem(fFilterItems.Items[fControl.CurIndex]).Filter;
end;

function TSPCFilterCombo.GetCurIndex: Integer;
begin
  Result := fControl.CurIndex;
end;

{ TSPCStatus }

function NewTSPCStatusBar;
var
  p                 : PSPCStatus;
  c                 : PControl;
  Style             : DWord;
begin
  Style := $00000000;
  Style := Style or WS_VISIBLE or WS_CHILD or WS_CLIPSIBLINGS or WS_CLIPCHILDREN; //msctls_statusbar32
  c := _NewControl(AOwner, 'msctls_statusbar32', Style, True, nil);
  //  c:=_NewStatusBar(AOwner);
  c.Style := Style;
  c.ExStyle := c.ExStyle xor WS_EX_CLIENTEDGE;
  c.BringToFront;
  New(p, create);
  p.fControl := c;
  Result := p;
end;

destructor TSPCStatus.Destroy;
begin
  fFont.Free;
  inherited;
end;

function TSPCStatus.SetAlign(Value: TControlAlign): PSPCStatus;
begin
  fControl.Align := Value;
  Result := @Self;
end;

function TSPCStatus.SetPosition(X, Y: integer): PSPCStatus;
begin
  fControl.Left := X;
  fControl.Top := Y;
  Result := @self;
end;

function TSPCStatus.SetSize(X, Y: integer): PSPCStatus;
begin
  fControl.Width := X;
  fControl.Height := Y;
  Result := @self;
end;

function TSPCStatus.GetFont;
begin
  Result := fControl.Font;
end;

procedure TSPCStatus.SetFont(Value: PGraphicTool);
begin
  fControl.Font.Assign(Value);
end;

function TSPCStatus.GetHeight: Integer;
begin
  Result := fControl.Height;
end;

procedure TSPCStatus.SetHeight(Value: Integer);
begin
  fControl.Height := Value;
end;

function TSPCStatus.GetWidth: Integer;
begin
  Result := fControl.Width;
end;

procedure TSPCStatus.SetWidth(Value: Integer);
begin
  fControl.Width := Value;
end;

function TSPCStatus.GetTop: Integer;
begin
  Result := fControl.Top;
end;

procedure TSPCStatus.SetTop(Value: Integer);
begin
  fControl.Top := Value;
end;

function TSPCStatus.GetLeft: Integer;
begin
  Result := fControl.Left;
end;

procedure TSPCStatus.SetLeft(Value: Integer);
begin
  fControl.Left := Value;
end;

procedure TSPCStatus.SetSimpleStatusText(Value: string);
begin
  fControl.Caption := Value;
end;

function TSPCStatus.GetSimpleStatusText: string;
begin
  Result := fControl.Caption;
end;

end.


unit KOLCCtrls;

interface

{$I KOLDEF.INC}

uses
  windows, messages, KOL;

{ ====== TRACKBAR CONTROL CONSTANTS =================== }

const
  TRACKBAR_CLASS = 'msctls_trackbar32';

  TBS_AUTOTICKS           = $0001;
  TBS_VERT                = $0002;
  TBS_HORZ                = $0000;
  TBS_TOP                 = $0004;
  TBS_BOTTOM              = $0000;
  TBS_LEFT                = $0004;
  TBS_RIGHT               = $0000;
  TBS_BOTH                = $0008;
  TBS_NOTICKS             = $0010;
  TBS_ENABLESELRANGE      = $0020;
  TBS_FIXEDLENGTH         = $0040;
  TBS_NOTHUMB             = $0080;
  TBS_TOOLTIPS            = $0100;

  TBM_GETPOS              = WM_USER;
  TBM_GETRANGEMIN         = WM_USER+1;
  TBM_GETRANGEMAX         = WM_USER+2;
  TBM_GETTIC              = WM_USER+3;
  TBM_SETTIC              = WM_USER+4;
  TBM_SETPOS              = WM_USER+5;
  TBM_SETRANGE            = WM_USER+6;
  TBM_SETRANGEMIN         = WM_USER+7;
  TBM_SETRANGEMAX         = WM_USER+8;
  TBM_CLEARTICS           = WM_USER+9;
  TBM_SETSEL              = WM_USER+10;
  TBM_SETSELSTART         = WM_USER+11;
  TBM_SETSELEND           = WM_USER+12;
  TBM_GETPTICS            = WM_USER+14;
  TBM_GETTICPOS           = WM_USER+15;
  TBM_GETNUMTICS          = WM_USER+16;
  TBM_GETSELSTART         = WM_USER+17;
  TBM_GETSELEND           = WM_USER+18;
  TBM_CLEARSEL            = WM_USER+19;
  TBM_SETTICFREQ          = WM_USER+20;
  TBM_SETPAGESIZE         = WM_USER+21;
  TBM_GETPAGESIZE         = WM_USER+22;
  TBM_SETLINESIZE         = WM_USER+23;
  TBM_GETLINESIZE         = WM_USER+24;
  TBM_GETTHUMBRECT        = WM_USER+25;
  TBM_GETCHANNELRECT      = WM_USER+26;
  TBM_SETTHUMBLENGTH      = WM_USER+27;
  TBM_GETTHUMBLENGTH      = WM_USER+28;
  TBM_SETTOOLTIPS         = WM_USER+29;
  TBM_GETTOOLTIPS         = WM_USER+30;
  TBM_SETTIPSIDE          = WM_USER+31;

  // TrackBar Tip Side flags
  TBTS_TOP                = 0;
  TBTS_LEFT               = 1;
  TBTS_BOTTOM             = 2;
  TBTS_RIGHT              = 3;

  TBM_SETBUDDY            = WM_USER+32; // wparam = BOOL fLeft; (or right)
  TBM_GETBUDDY            = WM_USER+33; // wparam = BOOL fLeft; (or right)
  TBM_SETUNICODEFORMAT    = CCM_SETUNICODEFORMAT;
  TBM_GETUNICODEFORMAT    = CCM_GETUNICODEFORMAT;

  TB_LINEUP               = 0;
  TB_LINEDOWN             = 1;
  TB_PAGEUP               = 2;
  TB_PAGEDOWN             = 3;
  TB_THUMBPOSITION        = 4;
  TB_THUMBTRACK           = 5;
  TB_TOP                  = 6;
  TB_BOTTOM               = 7;
  TB_ENDTRACK             = 8;

  // custom draw item specs
  TBCD_TICS    = $0001;
  TBCD_THUMB   = $0002;
  TBCD_CHANNEL = $0003;

{ ^^^^^^^^ TRACKBAR CONTROL ^^^^^^^^ }

type
  PTrackbar = ^TTrackbar;
  TTrackbarOption = ( trbAutoTicks, trbEnableSelRange, trbFixedLength,
                   trbNoThumb, trbNoTicks, trbTooltips, trbTopLeftMarks,
                   trbVertical, trbNoBorder );
  TTrackbarOptions = Set Of TTrackbarOption;

  TOnScroll = procedure( Sender: PTrackbar; Code: Integer ) of object;
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

  TTrackbar = object( TControl )
  private
    function GetOnScroll: TOnScroll;
    procedure SetOnScroll(const Value: TOnScroll);
    function GetVal( const Index: Integer ): Integer;
    procedure SetVal(const Index, Value: Integer);
    procedure SetThumbLen(const Index, Value: Integer);
    procedure SetTickFreq(const Value: Integer);
    procedure SetNumTicks(const Index, Value: Integer);
  protected
  public
    property OnScroll: TOnScroll read GetOnScroll write SetOnScroll;
    property RangeMin: Integer index $80010007 read GetVal write SetVal;
    property RangeMax: Integer index $80020008 read GetVal write SetVal;
    property PageSize: Integer index $00160015 read GetVal write SetVal;
    {* to scroll with PgUp/PgDn }
    property LineSize: Integer index $00180017 read GetVal write SetVal;
    {* to scroll with arrow keys }
    property Position: Integer index $80000005 read GetVal write SetVal;
    property NumTicks: Integer index $00100000 read GetVal write SetNumTicks;
    {* set approximately via TickFreq, returns actual tick mark count }
    property TickFreq: Integer                             write SetTickFreq;
    {* 2 means that one tick will be drawn for 2 tick marks }
    property SelStart: Integer index $0011000B read GetVal write SetVal;
    property SelEnd  : Integer index $0012000C read GetVal write SetVal;
    property ThumbLen: Integer index $001B0000 read GetVal write SetThumbLen;
    {* trbFixedLength should be on to have effect }
    function ChannelRect: TRect;
  end;

  TKOLTrackbar = PTrackbar;

function NewTrackbar( AParent: PControl; Options: TTrackbarOptions; OnScroll: TOnScroll ): PTrackbar;

implementation

type
  PTrackbarData = ^TTrackbarData;
  TTrackbarData = packed record
    FOnScroll: TOnScroll;
  end;

function WndProcTrackbarParent( Sender: PControl; var Msg: TMsg; var Rslt: LRESULT ): Boolean;
var D: PTrackbarData;
    Trackbar: PTrackbar;
begin
  Result := FALSE;
  if (Msg.message = WM_HSCROLL) or (Msg.message = WM_VSCROLL) then
  if (Msg.lParam <> 0) then
  begin
    {$IFDEF USE_PROP}
    Trackbar := Pointer( GetProp( Msg.lParam, ID_SELF ) );
    {$ELSE}
    Trackbar := Pointer( GetWindowLongPtr( Msg.lParam, GWLP_USERDATA ) );
    {$ENDIF}
    if Trackbar <> nil then
    begin
      D := Trackbar.CustomData;
      if Assigned( D.FOnScroll ) then
        D.FOnScroll( Trackbar, Msg.wParam );
    end;
  end;
end;

function NewTrackbar( AParent: PControl; Options: TTrackbarOptions; OnScroll: TOnScroll ): PTrackbar;
const TrackbarOptions: array[ TTrackbarOption ] of Integer = ( TBS_AUTOTICKS,
      TBS_ENABLESELRANGE, TBS_FIXEDLENGTH, TBS_NOTHUMB, TBS_NOTICKS, TBS_TOOLTIPS,
      TBS_TOP, TBS_VERT, 0 );
var aStyle: DWORD;
    D: PTrackbarData;
    W, H: Integer;
begin
  DoInitCommonControls( ICC_BAR_CLASSES );
  aStyle := MakeFlags( @Options, TrackbarOptions ) or WS_CHILD or WS_VISIBLE;
  Result := PTrackbar( _NewCommonControl( AParent, TRACKBAR_CLASS, aStyle,
            not (trbNoBorder in Options), nil ) );
  W := 200;
  H := 40;
  if trbVertical in Options then
  begin
    H := W;
    W := 40;
  end;
  Result.Width := W;
  Result.Height := H;
  GetMem( D, Sizeof( D^ ) );
  Result.CustomData := D;
  D.FOnScroll := OnScroll;
  AParent.AttachProc( WndProcTrackbarParent );
end;

{ TTrackbar }

function TTrackbar.ChannelRect: TRect;
begin
  Perform( TBM_GETCHANNELRECT, 0, LPARAM( @ Result ) );
end;

function TTrackbar.GetOnScroll: TOnScroll;
var D: PTrackbarData;
begin
  D := CustomData;
  Result := D.FOnScroll;
end;

function TTrackbar.GetVal( const Index: Integer ): Integer;
begin
  Result := Perform( WM_USER + ( HiWord( Index ) and $7FFF ), 0, 0 );
end;

procedure TTrackbar.SetNumTicks(const Index, Value: Integer);
begin
  TickFreq := (RangeMax - RangeMin) div Value;
end;

procedure TTrackbar.SetOnScroll(const Value: TOnScroll);
var D: PTrackbarData;
begin
  D := CustomData;
  D.FOnScroll := Value;
end;

procedure TTrackbar.SetThumbLen(const Index, Value: Integer);
begin
  Perform( TBM_SETTHUMBLENGTH, Value, 0 );
end;

procedure TTrackbar.SetTickFreq(const Value: Integer);
begin
  Perform( TBM_SETTICFREQ, Value, 0 );
end;

procedure TTrackbar.SetVal(const Index, Value: Integer);
begin
  Perform( WM_USER + LoWord( Index ), Index shr 31, Value );
end;

end.

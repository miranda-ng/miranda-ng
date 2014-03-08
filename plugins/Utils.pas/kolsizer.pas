unit KOLSizer;
//
// purpose: KOL control sizercontrol and design grid
//  author: © 2004, Thaddy de Koning
// Remarks: Tnx in part to Marco Cantu for the sizer idea in DDH3
//          copyrighted freeware.
//
interface

uses
  Windows, Messages, Kol;

const
  DESIGNER_NORESIZE = 1;

type
  PDesigner=^TDesigner;
  TDesigner=object(TStrlistEx)
  private
    fOwner:pControl;
    fSpacing:Cardinal;
    FOldPaint:TOnPaint;
    fActive: boolean;
    fSizer:PControl;
    FOnControlChange: TonEvent;
//    FOnDblClick:TOnEvent;
//    FOnMouseDown:TOnMouse;
    fCurrent: pControl;
//    FAction:integer;

    procedure setactive(const Value: boolean);
    function PrepareClassname(aControl: PControl): KOLString;
    function UniqueName(aName: KOLString; flags:cardinal): KOLString;
    procedure SetCurrent(const Value: pControl);
    procedure InternalControlChange(sender:pObj);
    procedure Setspacing(Space:cardinal = 8);
    function  GetFlags(aControl:pControl):cardinal;
  protected
    procedure init;virtual;
    procedure DoKeyUp( Sender: PControl; var Key: Longint; Shift: dword);
    procedure DoChar( Sender: PControl; var Key: KOLChar; Shift: dword);
  public
    destructor destroy;virtual;
    procedure Connect(aName: KOLString; aControl: pControl; flags:cardinal=0);
    procedure DisConnect(aControl: pControl);
    procedure Paintgrid(sender:pControl;DC:HDC);

    property Spacing:cardinal read fSpacing write setspacing;
    property Active:boolean read fActive write setactive;
//    property Action:integer read FAction write Faction;
    property Current:pControl read fCurrent write SetCurrent;
    property OnControlChange:TOnEvent Read FOnControlChange write FOnControlChange;
//    property OnDblClick:TonEvent read fOnDblClick write FOnDblClick;
//    property OnMouseDown:TOnMouse read FOnMouseDown write FOnMouseDown;
  end;

function NewSizerControl(AControl: PControl;aDesigner:PDesigner;flags:cardinal=0):PControl;
function NewDesigner(aOwner:pControl):pDesigner;

implementation

const
  FlagDelimeterChar='@';

const
  // Size and move commands for SysCommand
  SZ_LEFT        = $F001;
  SZ_RIGHT       = $F002;
  SZ_TOP         = $F003;
  SZ_TOPLEFT     = $F004;
  SZ_TOPRIGHT    = $F005;
  SZ_BOTTOM      = $F006;
  SZ_BOTTOMLEFT  = $F007;
  SZ_BOTTOMRIGHT = $F008;
  SZ_MOVE        = $F012;

type
  TPosInfo = record
    Rect     :Trect;
    Pos      :integer;
    Direction:integer;
  end;

  PSizerdata=^ TSizerdata;
  TSizerdata= object(Tobj)
    FControl :PControl;
    FPosInfo :array [0..7] of TPosInfo;
    Szflags  :cardinal;
    Direction:longint;

    procedure DoPaint(sender:pControl;DC:HDC);
  end;

  PHack =^ THack;
  THack = object(Tcontrol)
  end;

var
  LocalDesigner:PDesigner=nil;

function DesignHandlerProc(Sender: PControl; var Msg: TMsg; var Rslt: integer): Boolean;
//var MouseData:TMouseEventData;
begin
  Result:=false;
  case msg.message of
{
      WM_KEYUP,WM_SYSCHAR,WM_SYSKEYUP,
      WM_CHAR: begin
//        if loword(msg.wParam)=VK_TAB then
          Messagebox(0,'222','',0);
      end;
}    WM_LBUTTONDOWN: begin
      if LocalDesigner.fOwner<>Sender then LocalDesigner.Current:=Sender;
      Result:=true;
      {
      if assigned(Localdesigner.OnMousedown) then
      // Borrowed from KOL.pas
      // enables us to pass on KOL mouse events back to the designer
      // without having to connect to true KOL eventproperties.
      with MouseData do
      begin
        Shift := Msg.wParam;
        if GetKeyState(VK_MENU) < 0 then
          Shift := Shift or MK_ALT;
        X := LoWord(Msg.lParam);
        Y := HiWord(Msg.lParam);
        Button := mbNone;
        StopHandling := true;
        Rslt := 0; // needed ?
        LocalDesigner.OnMousedown(sender,Mousedata);
        Result:=true
      end;
       }
    end
  end;
end;

// TSizerControl methods
function WndProcSizer( Sender: PControl; var Msg: TMsg; var Rslt: integer ): Boolean;
var
  Pt: TPoint;
  i: integer;
  R:Trect;
  Data:PSizerData;
begin
  Data:=PSizerData(Sender.CustomObj);
  Result:=True;
  with Sender^, Data^ do
  begin
    case msg.message of
{
      WM_KEYUP,WM_SYSCHAR,WM_SYSKEYUP,
      WM_CHAR: begin
//        if loword(msg.wParam)=VK_TAB then
          Messagebox(0,'111','',0);
      end;
}
      WM_NCHITTEST: begin
        Pt := MakePoint(loword(Msg.lparam), hiword(Msg.lparam));
        Pt := Screen2Client (Pt);
        Rslt:=0;
        for i := 0 to 7 do
          if PtInRect (FPosInfo [i].rect, Pt) then
          begin
            // The value of rslt is passed on and makes
            // the system select the correct cursor
            // without us having to do anything more.
            Rslt     :=FPosInfo[i].pos;
            Direction:=FPosInfo[i].direction;
            break;
          end;
        if Rslt = 0 then
          Result:=False;
      end;

      WM_SIZE: begin
        R := BoundsRect;
        InflateRect (R, -2, -2);
        Fcontrol.BoundsRect := R;
        FPosInfo[0].rect:=MakeRect (0            ,0             ,5            ,5);
        FPosInfo[1].rect:=MakeRect (Width div 2-3,0             ,Width div 2+2,5);
        FPosInfo[2].rect:=MakeRect (Width-5      ,0             ,Width        ,5);
        FPosInfo[3].rect:=MakeRect (Width-5      ,Height div 2-3,Width        ,Height div 2+2);
        FPosInfo[4].rect:=MakeRect (Width-5      ,Height-5      ,Width        ,Height);
        FPosInfo[5].rect:=MakeRect (Width div 2-3,Height-5      ,Width div 2+2,Height);
        FPosInfo[6].rect:=MakeRect (0            ,Height-5      ,5            ,Height);
        FPosInfo[7].rect:=MakeRect (0            ,Height div 2-3,5            ,Height div 2+2);
      end;

      WM_NCLBUTTONDOWN: if (Szflags and DESIGNER_NORESIZE)=0 then
        Perform (WM_SYSCOMMAND, Direction, 0);

      WM_LBUTTONDOWN: Perform (WM_SYSCOMMAND, SZ_MOVE, 0);

      WM_MOVE: begin
        R := BoundsRect;
        InflateRect (R, -2, -2);
        fControl.Invalidate;
        fControl.BoundsRect := R;
      end;

    else
      Result:=false;
    end;
  end;
end;

function NewSizerControl(AControl: PControl;aDesigner:PDesigner;flags:cardinal):PControl;
var
  R: TRect;
  Data:PSizerData;
begin
  New(Data,Create);
  Result:={NewPanel(aControl,esNone);//}NewPaintBox(aControl);
  Result.ExStyle:=Result.ExStyle or WS_EX_TRANSPARENT;
//  Result.TabStop:=true;
//  Result.OnChar:=aDesigner.DoChar;
//  Result.OnKeyDown:=aDesigner.DoKeyUp;
//  Result.OnKeyUp:=aDesigner.DoKeyUp;
  if aDesigner.fowner<>aControl then
    With result^, Data^  do
    begin
      Szflags  := flags;
      FControl := AControl;
      // set the size and position
      R := aControl.BoundsRect;
      InflateRect (R, 2, 2);
      BoundsRect := R;
      // set the parent
      Parent := aControl.Parent;
      // create the list of positions
      FPosInfo [0].pos := htTopLeft    ; FPosInfo [0].direction := SZ_TOPLEFT;
      FPosInfo [1].pos := htTop        ; FPosInfo [1].direction := SZ_TOP;
      FPosInfo [2].pos := htTopRight   ; FPosInfo [2].direction := SZ_TOPRIGHT;
      FPosInfo [3].pos := htRight      ; FPosInfo [3].direction := SZ_RIGHT;
      FPosInfo [4].pos := htBottomRight; FPosInfo [4].direction := SZ_BOTTOMRIGHT;
      FPosInfo [5].pos := htBottom     ; FPosInfo [5].direction := SZ_BOTTOM;
      FPosInfo [6].pos := htBottomLeft ; FPosInfo [6].direction := SZ_BOTTOMLEFT;
      FPosInfo [7].pos := htLeft       ; FPosInfo [7].direction := SZ_LEFT;
      CustomObj:=Data;
      OnPaint:=DoPaint;
      AttachProc(WndProcSizer);
      Bringtofront;
      Focused:=true
    end;
end;

procedure TSizerData.DoPaint(sender:pControl;DC:HDC);
var
  i: integer;
begin
  // I simply use the current pen and brush
  for i := 0 to  7 do
    with pSizerdata(sender.Customobj).FPosInfo[i].Rect do
      Rectangle(DC, Left, Top, Right, Bottom);
end;

{ TDesigner }
function NewDesigner(aOwner:pControl):pDesigner;
begin
  if Assigned(LocalDesigner) then
  begin
    result:=LocalDesigner;
  end
  else
  begin
    New(Result,Create);
    with result^ do
    begin
      Fowner:=aOwner;
      Connect('',Fowner);
      FOldPaint:=Fowner.OnPaint;
      LocalDesigner:=Result;
      //Result.Current:=aOwner;
    end
  end
end;

procedure TDesigner.init;
begin
  inherited;
  Fspacing:=8;
end;

procedure TDesigner.PaintGrid(Sender: pControl; DC: HDC);
var
  i, j: integer;
begin
  i := 0;
  j := 0;
  Sender.Canvas.FillRect(Sender.Canvas.ClipRect);
  if Assigned(FOldPaint) then FOldPaint(Sender,DC);
  repeat
    repeat
      MoveToEx(Dc,i, j,nil);
      LineTo(Dc,i + 1,j);
      inc(i, fSpacing);
    until i > Sender.ClientWidth;
    i := 0;
    inc(j, fSpacing);
  until j > Sender.ClientHeight;
end;

procedure TDesigner.SetSpacing(Space: cardinal);
begin
  fSpacing:=Space;
  fOwner.invalidate;
end;

destructor TDesigner.destroy;
begin
  SetActive(false);
  FOwner.OnPaint:=FOldPaint;
  inherited;
end;

//Note: Make shure that whatever happens, all pointers are nil or valid!
//      Took a long time to debug spurious crashes.
//      So this is not excessively safe.
procedure TDesigner.SetActive(const Value: boolean);
var
  i:integer;
begin
  FActive := Value;
  if FActive then
  begin
    fOwner.OnPaint:=PaintGrid;
    if count > 1 then
    begin
      if Assigned(fCurrent) then
        fSizer:=NewSizerControl(fCurrent,@self,GetFlags(fCurrent));
      for i:=0 to count -1 do
       if not PControl(Objects[i]).IsprocAttached(DesignHandlerProc) then
         PControl(Objects[i]).AttachProc(DesignHandlerProc);
    end;
  end
  else
  begin
    if count > 0 then // always coz Owner is first
      for i:=0 to count -1 do
        PControl(Objects[i]).DetachProc(DesignHandlerProc);
    if Assigned(fSizer) then
    begin
      fSizer.free;
      fSizer:=nil;
    end;
    fCurrent:=nil;
    fOwner.OnPaint:=FOldPaint;
  end;
  fOwner.Invalidate;
end;

procedure TDesigner.Connect(aName: KOLString; aControl: pControl; flags:cardinal=0);
begin
  if (IndexOfObj(aControl) = -1) then
  begin
    if aName = '' then
      aName := PrepareClassName(aControl);
    AddObject(UniqueName(aName,flags), Cardinal(aControl));
    InternalControlChange(aControl);
    SetCurrent(aControl);
    if Active then
      if not aControl.IsprocAttached(DesignHandlerProc) then
        aControl.AttachProc(DesignHandlerProc);
  end;
end;

procedure TDesigner.DisConnect(aControl: pControl);
var
  index: integer;
begin
  index := IndexOfObj(aControl);
  if index = -1 then
    exit;
  Delete(index);

  InternalControlChange(nil);
end;

function TDesigner.GetFlags(aControl:pControl):cardinal;
var
  idx,dummy:integer;
  tmpstr:KOLString;
begin
  idx:=IndexOfObj(aControl);
  tmpstr:=Items[idx];
  idx:=IndexOfChar(tmpstr,FlagDelimeterChar);
  if idx<0 then result:=0
  else
  begin
    val(copy(tmpstr,idx+1,15),result,dummy);
  end;
end;

procedure TDesigner.SetCurrent(const Value: pControl);
begin
  if Assigned(fSizer) then
  begin
    fSizer.free;
    fsizer:=nil;
  end;
  if Value <> nil then
  begin

    fCurrent := Value;
    if fActive and (fCurrent<>nil) and (fCurrent<>fOwner) then
      fSizer:=NewSizerControl(Value,@self,GetFlags(Value));

    InternalControlChange(Value);
  end;
end;

procedure TDesigner.InternalControlChange(sender: pObj);
begin
  if fActive then
    if Assigned(OnControlChange)then
      FOnControlChange(sender);
end;

procedure TDesigner.DoChar( Sender: PControl; var Key: KOLChar; Shift: dword);
begin
//   messagebox(0,'444','',0);
end;

procedure TDesigner.DoKeyUp(Sender: PControl; var Key: integer; Shift: dword);

  procedure DeleteControl(Index:integer);
  var
    i: integer;
    C:PControl;
  begin
    C:=PControl(Objects[index]);
    // delete children, not owner
    if C.ChildCount>0 then
      for i:=C.ChildCount-1 downto 0 do
        if C<>fOwner then DeleteControl(i);

    if C<>fOwner then
    begin
      C.free;
      Delete(0);
    end;
  end;

var
  i:integer;
begin
//   if Key = VK_TAB then
//   messagebox(0,'333','',0);

   if Key = VK_DELETE then
   begin
     i:=IndexOfObj(LocalDesigner.Current);
     if i<>-1 then
     begin
       DeleteControl(i);
       InternalControlChange(nil);
       PostMessage(Sender.Handle,WM_CLOSE,0,0); //???
     end;
   end;
end;

  // Converts  an object name to a Delphi compatible control name that
  // is unique for the designer, i.e 'Button' becomes 'Button1',
  // the next button becomes 'Button2', always unless the
  // control is already named by the user in which case the name is preserved
  // unless there are conficts. In that case the control is silently
  // renamed with a digit suffix without raising exceptions.
  // Deleted names are re-used.
  // It's not a beauty but it works.
  // (A severe case of programming 48 hours without sleep)

function TDesigner.UniqueName(aName: KOLString; flags:cardinal): KOLString;
var
  I, J: integer;
  T: KOLString;
begin
  // Strip obj_ prefix and all other prefix+underscores from
  // subclassname property: 'obj_BUTTON' becomes 'Button'
  T := LowerCase(aName);
  while T <> '' do aName := Parse(T, '_');

//  aName[1]:=UpCase(aName[1]);
  //Propercase it
  T := aName[1];
  T := UpperCase(T);
  aName[1] := T[1];

  Result := aName;
  // Add at least a 1 to the name if the last char
  // is not a digit.
  if not (AnsiChar(aName[length(aName)]) in ['0'..'9']) then
    Result := Format('%s%d', [aName, 1]);
  J := 1;
  repeat
    I := IndexOf(Result);
    if I > -1 then
    begin
      inc(J);
      Result := Format('%s%d', [aName, J]);
    end;
  until I = -1;
  if flags<>0 then
  begin
    Str(flags,T);
    Result:=Result+FlagDelimeterChar+T;
  end;
end;

// This is probably not complete yet.
function TDesigner.PrepareClassName(aControl: PControl): KOLString;
begin
  Result := aControl.subclassname;
  with aControl^ do
    if subClassname = 'obj_STATIC' then
    begin
      // Only place where panel and label differ
      // consistently???
      // why not aControl.SizeRedraw ??
      if pHack(aControl).SizeRedraw = True then
        Result := 'obj_LABEL'
      else
        Result := 'obj_PANEL'
    end

    else if subclassname = 'obj_BUTTON' then
    begin
      if      Boolean(Style and BS_AUTOCHECKBOX) then Result := 'obj_CHECKBOX'
      else if Boolean(style and BS_RADIOBUTTON ) then Result := 'obj_RADIOBOX'
      else if Boolean(style and BS_OWNERDRAW   ) then Result := 'obj_BITBTN'
      else if Boolean(style and BS_GROUPBOX    ) then Result := 'obj_GROUPBOX';
    end

    else if IndexOfStr(UpperCase(subclassname), 'RICHEDIT')>-1 then
      Result := 'obj_RICHEDIT';
end;

end.

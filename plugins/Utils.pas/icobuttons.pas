unit IcoButtons;

interface

uses windows, KOL;

const
  AST_NORMAL  = 0;
  AST_HOVERED = 1;
  AST_PRESSED = 2;

type
  tGetIconProc = function(action:integer;stat:integer=AST_NORMAL):cardinal;
  tActionProc  = function(action:integer):integer;

type
  pIcoButton = ^tIcoButton;
  tIcoButton = object(TControl)
  private
    function  GetGetIconProc:tGetIconProc;
    procedure SetGetIconProc (val:tGetIconProc);
    procedure SetDoActionProc(val:tActionProc);
    procedure SetCheckFlag(val:boolean);
    function  GetCheckFlag:boolean;
    procedure SetAction(val:integer);
    function  GetAction:integer;
    function  GetState:integer;
    procedure myPaint(Sender: PControl; DC: HDC);
    procedure myMouseDown (Sender:PControl; var Mouse:TMouseEventData);
    procedure myMouseUp   (Sender:PControl; var Mouse:TMouseEventData);
    procedure myMouseEnter(Sender: PObj);
    procedure myMouseLeave(Sender: PObj);
    procedure myCtrlBtnClick(Sender: PObj);
  public

    procedure RefreshIcon;
    property GetIconProc : tGetIconProc read GetGetIconProc write SetGetIconProc;
    property DoActionProc: tActionProc  write SetDoActionProc;

    property AsCheckbox: boolean read GetCheckFlag write SetCheckFlag;
    property Action    : integer read GetAction    write SetAction;
    property State     : integer read GetState;
  end;

function CreateIcoButton(AOwner: PControl; pGetIconProc:tGetIconProc;
         pActionProc:tActionProc; action:integer=0; repeattime:integer=0):pIcoButton;

function CreateIcoButtonHandle(AOwner: PControl; pActionProc:tActionProc;
         ico_normal:HICON; ico_hovered:HICON=0; ico_pressed:HICON=0;
         action:integer=0; repeattime:integer=0):pIcoButton;

implementation

uses messages;

type
  pIcoBtnData = ^tIcoBtnData;
  tIcoBtnData = record
    rptvalue:cardinal;
    rpttimer:cardinal;
    checking: boolean;

    ico_normal :PIcon;
    ico_hovered:PIcon;
    ico_pressed:PIcon;
    active     :PIcon; // one of ico_*

    Action:integer;

    GetIcon : tGetIconProc;
    DoAction: tActionProc;
  end;

function tIcoButton.GetGetIconProc:tGetIconProc;
begin
  result:=pIcoBtnData(CustomData).GetIcon;
end;

procedure tIcoButton.SetGetIconProc(val:tGetIconProc);
begin
  pIcoBtnData(CustomData).GetIcon:=val;
end;

procedure tIcoButton.SetDoActionProc(val:tActionProc);
begin
  pIcoBtnData(CustomData).DoAction:=val;
end;

procedure tIcoButton.SetCheckFlag(val:boolean);
begin
  pIcoBtnData(CustomData).checking:=val;
end;

function tIcoButton.GetCheckFlag:boolean;
begin
  result:=pIcoBtnData(CustomData).checking;
end;

procedure tIcoButton.SetAction(val:integer);
begin
  pIcoBtnData(CustomData).Action:=val;
end;

function tIcoButton.GetAction:integer;
begin
  result:=pIcoBtnData(CustomData).Action;
end;

function tIcoButton.GetState:integer;
begin
  with pIcoBtnData(CustomData)^ do
  if      active=ico_pressed then result:=AST_PRESSED
  else if active=ico_hovered then result:=AST_HOVERED
  else {if active=ico_normal then}result:=AST_NORMAL;
end;

procedure tIcoButton.myCtrlBtnClick(Sender: PObj);
var
  D: PIcoBtnData;
begin
  D:=PControl(Sender).CustomData;
  if @D.DoAction<>nil then
    D.DoAction(D.action);
end;

procedure tIcoButton.myMouseEnter(Sender: PObj);
var
  D: PIcoBtnData;
begin
  D:=PControl(Sender).CustomData;
  if D.ico_hovered<>nil then
  begin
    D.active:=D.ico_hovered;
    PControl(Sender).Update;
//    PControl(Sender).Parent.Update; //??
  end;
end;

procedure tIcoButton.myMouseLeave(Sender: PObj);
var
  D: PIcoBtnData;
begin
  D:=PControl(Sender).CustomData;
  if D.active=D.ico_hovered then //!!!! for case when mouse button pressed and mouse moved
    D.active:=D.ico_normal;
  PControl(Sender).Update;
//  PControl(Sender).Parent.Update; //??
end;

procedure TimerProc(wnd:HWND;uMsg:uint;idEvent:uint_ptr;dwTime:dword); stdcall;
begin
  PControl(IdEvent).OnClick(PControl(IdEvent));
end;

procedure tIcoButton.myMouseDown(Sender:PControl; var Mouse:TMouseEventData);
var
  D: PIcoBtnData;
begin
  if Mouse.Button<>mbLeft then exit;
  D:=Sender.CustomData;
  if D.checking then
  begin
    if D.active=D.ico_pressed then
      D.active:=D.ico_normal
    else
      D.active:=D.ico_pressed;
  end
  else
  begin
    if D.ico_pressed<>nil then
      D.active:=D.ico_pressed
    else
      Sender.SetPosition(Sender.Position.X-2,Sender.Position.Y-2);

    if D.rptvalue<>0 then
    begin
      D.rpttimer:=SetTimer(Sender.Handle,uint(Sender),D.rptvalue,@TimerProc);
//      D.rpttimer:=SetTimer(Sender.GetWindowHandle,1,D.rptvalue,nil);
    end;
  end;
  Sender.Update;
end;

procedure tIcoButton.myMouseUp(Sender:PControl; var Mouse:TMouseEventData);
var
  D: PIcoBtnData;
  tp:TPOINT;
begin
  if Mouse.Button<>mbLeft then exit;
  D:=Sender.CustomData;
  if not D.checking then
  begin
    if D.rpttimer<>0 then
    begin
      KillTimer(Sender.Handle,D.rpttimer);
      D.rpttimer:=0;
    end;

    if D.ico_pressed<>nil then
    begin
      tp.X:=Mouse.X;
      tp.Y:=Mouse.Y;
      // mouse still above button?
      if (D.ico_hovered<>nil) and PtInRect(Sender.BoundsRect,tp) then
        D.active:=D.ico_hovered
      else
        D.active:=D.ico_normal;
    end
    else
      Sender.SetPosition(Sender.Position.X+2,Sender.Position.Y+2);
    Sender.Update;
  end;
end;

procedure Destroy(dummy:PControl;sender:PObj);
var
  D: PIcoBtnData;
begin
  D:=pIcoButton(sender).CustomData;
  D.ico_normal.Free;
  if D.ico_hovered<>nil then D.ico_hovered.Free;
  if D.ico_pressed<>nil then D.ico_pressed.Free;

  if D.rpttimer<>0 then
  begin
    KillTimer(0,D.rpttimer);
    D.rpttimer:=0;
  end;
end;

procedure tIcoButton.RefreshIcon;
var
  D: PIcoBtnData;
begin
  D:=CustomData;
  if @D.GetIcon=nil then exit;

  D.ico_normal.Handle:=D.GetIcon(D.action,AST_NORMAL);
  D.ico_normal.ShareIcon:=true;
  if D.ico_hovered<>nil then
  begin
    D.ico_hovered.Handle:=D.GetIcon(D.action,AST_HOVERED);
    D.ico_hovered.ShareIcon:=true;
  end;
  if D.ico_pressed<>nil then
  begin
    D.ico_pressed.Handle:=D.GetIcon(D.action,AST_PRESSED);
    D.ico_pressed.ShareIcon:=true;
  end;
end;

procedure tIcoButton.myPaint(Sender: PControl; DC: HDC);
var
  D: PIcoBtnData;
begin
  D:=Sender.CustomData;
  D.active.Draw(DC,0,0);
end;

function CreateIcoButton(AOwner: PControl; pGetIconProc:tGetIconProc;
         pActionProc:tActionProc; action:integer=0; repeattime:integer=0):pIcoButton;
var
  ico:HICON;
  D: PIcoBtnData;
begin
  // first, checking what icons are available
  ico:=pGetIconProc(action,AST_NORMAL);
  if ico=0 then
  begin
    result:=nil;
    exit;
  end;

  Result:=pIcoButton(NewBitBtn(AOwner,'',[bboNoBorder,bboNoCaption],glyphOver,0,0));
  if result=nil then exit;

  Result.LikeSpeedButton.Flat:=true;
  Result.Transparent:=true;

  GetMem(D,SizeOf(TIcoBtnData));
  Result.CustomData:=D;

  Result.OnMouseDown :=Result.myMouseDown;
  Result.OnMouseUp   :=Result.myMouseUp;
  Result.OnMouseEnter:=Result.myMouseEnter;
  Result.OnMouseLeave:=Result.myMouseLeave;
  Result.OnClick     :=Result.myCtrlBtnClick;
  Result.OnPaint     :=Result.myPaint;

  Result.AsCheckbox:=false;
  Result.action:=action;

  D.rptvalue:=repeattime;
  D.rpttimer:=0;

  Result.DoActionProc:=pActionProc;
  Result.GetIconProc :=pGetIconProc;

  D.ico_normal:=NewIcon;
  D.ico_normal.Handle   :=ico;
  D.ico_normal.ShareIcon:=true;
  D.active:=D.ico_normal;

  ico:=D.GetIcon(action,AST_HOVERED);
  if ico<>0 then
  begin
    D.ico_hovered:=NewIcon;
    D.ico_hovered.Handle   :=ico;
    D.ico_hovered.ShareIcon:=true;
  end
  else
    D.ico_hovered:=nil;
  ico:=D.GetIcon(action,AST_PRESSED);
  if ico<>0 then
  begin
    D.ico_pressed:=NewIcon;
    D.ico_pressed.Handle   :=ico;
    D.ico_pressed.ShareIcon:=true;
  end
  else
    D.ico_pressed:=nil;

  Result.SetSize(16,16);
  Result.SetPosition(0,0);
  Result.OnDestroy:=TOnEvent(MakeMethod(nil,@DEstroy));
end;

function CreateIcoButtonHandle(AOwner: PControl; pActionProc:tActionProc;
         ico_normal:HICON; ico_hovered:HICON=0; ico_pressed:HICON=0;
         action:integer=0; repeattime:integer=0):pIcoButton;
var
  D: PIcoBtnData;
begin
  if ico_normal=0 then
  begin
    result:=nil;
    exit;
  end;

  Result:=pIcoButton(NewBitBtn(AOwner,'',[bboNoBorder,bboNoCaption],glyphOver,0,0));
  if result=nil then exit;
  Result.LikeSpeedButton.Flat:=true;
  Result.Transparent:=true;

  GetMem(D,SizeOf(TIcoBtnData));
  Result.CustomData:=D;

  Result.OnMouseDown :=Result.myMouseDown;
  Result.OnMouseUp   :=Result.myMouseUp;
  Result.OnMouseEnter:=Result.myMouseEnter;
  Result.OnMouseLeave:=Result.myMouseLeave;
  Result.OnClick     :=Result.myCtrlBtnClick;
  Result.OnPaint     :=Result.myPaint;

  Result.AsCheckbox:=false;
  Result.action:=action;

  D.rptvalue:=repeattime;
  D.rpttimer:=0;

  Result.GetIconProc :=nil;
  Result.DoActionProc:=pActionProc;

  D.ico_normal:=NewIcon;
  D.ico_normal.Handle   :=ico_normal;
  D.ico_normal.ShareIcon:=true;
  D.active:=D.ico_normal;

  if ico_hovered<>0 then
  begin
    D.ico_hovered:=NewIcon;
    D.ico_hovered.Handle   :=ico_hovered;
    D.ico_hovered.ShareIcon:=true;
  end
  else
    D.ico_hovered:=nil;

  if ico_pressed<>0 then
  begin
    D.ico_pressed:=NewIcon;
    D.ico_pressed.Handle   :=ico_pressed;
    D.ico_pressed.ShareIcon:=true;
  end
  else
    D.ico_pressed:=nil;

  Result.SetSize(16,16);
  Result.SetPosition(0,0);
  Result.OnDestroy:=TOnEvent(MakeMethod(nil,@Destroy));
end;

end.

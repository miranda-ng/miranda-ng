{Trackbar}
type
  pAWKTrackbar = ^tAWKTrackbar;
  tAWKTrackbar = object(TObj)
    total:integer;
    UpdInterval:integer;
    OldMouseDown,
    OldMouseUp:TOnMouse;

    procedure CtrlResize(Sender: PObj);
    procedure Erase(Sender: PControl; DC: HDC);
    procedure Paint(Sender: PControl; DC: HDC);
    procedure Scroll(Sender:PTrackbar; Code:integer);
    procedure PressButton  (Sender: PControl;var Mouse: TMouseEventData);
    procedure UnPressButton(Sender: PControl;var Mouse: TMouseEventData);
    procedure DragButton   (Sender: PControl;var Mouse: TMouseEventData);
  end;

procedure ResetTrackbar(Trackbar:PControl);
begin
  if Trackbar=nil then exit;

  with pTrackbar(Trackbar)^ do
  begin
    RangeMin:=0;
    RangeMax:=0;
    Position:=0;
  end;
end;

procedure TrackbarSetRange(Trackbar:PTrackbar;timer:integer;total:integer=-1);
var
  D:pAWKTrackbar;
  lpercent:real;
begin
  if Trackbar=nil then exit;

  with Trackbar^ do
  begin
    D:=pointer(CustomObj);
    if total<0 then // changing timer only
    begin
      total:=D.total;
      if RangeMax>0 then
        lpercent:=position/RangeMax
      else
        lpercent:=0;
    end
    else            // for new track
    begin
      D.total:=total;
      lpercent:=0;
    end;
    D.UpdInterval:=timer;
    total:=(total*1000) div timer;
    RangeMax:=total;
    LineSize:=total div 100;
    PageSize:=total div 10;
    Position:=round(lpercent*total);
  end;
end;

procedure SetTrackbarPosition(Trackbar:PTrackbar;pos:integer);
begin
  if Trackbar=nil then exit;
//??  if Sender.ChildCount=0 then exit;
  if pIcoButton(Trackbar.Children[0]).State<>AST_PRESSED then
    Trackbar.Position:=pos;

  Trackbar.Update;
end;

function CoordToPos(Trackbar:PTrackbar;x:integer):integer;
var
  range:integer;
  rmin,rmax:integer;
  offsetthumb,width:integer;
  rc:TRect;
begin
  result:=0;
  if Trackbar=nil then exit;

  rmin:=Trackbar.RangeMin;
  rmax:=Trackbar.RangeMax;
  range:=rmax-rmin; // logic width

  offsetthumb:=Trackbar.ThumbLen div 2;

  rc:=Trackbar.ChannelRect;
  width:= (rc.right-rc.left)-(offsetthumb*2)-1;
  result:=(range*(x-rc.left-offsetthumb)) div width;

  inc(result,rmin);
  if result>rmax then
    result:=rmax
  else if result<rmin then
    result:=rmin;
end;

procedure tAWKTrackbar.PressButton(Sender: PControl;var Mouse: TMouseEventData);
begin
  pAWKTrackbar(Sender.Parent.CustomObj).OldMouseDown(Sender,Mouse);

  pIcoButton(Sender)^.Action:=PTrackbar(Sender.Parent).Position;
end;

procedure tAWKTrackbar.UnPressButton(Sender: PControl;var Mouse: TMouseEventData);
begin
  pAWKTrackbar(Sender.Parent.CustomObj).OldMouseUp(Sender,Mouse);

  CallService(MS_WAT_PRESSBUTTON,WAT_CTRL_SEEK,
    pIcoButton(Sender)^.Action*pAWKTrackbar(Sender.Parent.CustomObj).UpdInterval div 1000);
  pIcoButton(Sender)^.Action:=-1;
end;

procedure tAWKTrackbar.DragButton(Sender: PControl;var Mouse: TMouseEventData);
var
  pos:integer;
begin
  with pIcoButton(Sender)^ do
    if State=AST_PRESSED then
    begin
      pos:=CoordToPos(PTrackbar(Sender.Parent),Sender.Left+Mouse.X);
      if Action<>pos then
      begin
        Action:=pos;
        PTrackbar(Sender.Parent).Position:=pos;
      end;
    end;
end;

procedure tAWKTrackbar.Scroll(Sender:PTrackbar; Code:integer);
begin
  if code=TB_ENDTRACK then
  begin
    CallService(MS_WAT_PRESSBUTTON,WAT_CTRL_SEEK,
      Sender.Position*pAWKTrackbar(Sender.CustomObj).UpdInterval div 1000);
  end;
end;

procedure tAWKTrackbar.CtrlResize(Sender: PObj);
var
  tmp:integer;
begin
  tmp:=PControl(Sender).Parent.Width-16;
  if (PTrackbar(Sender)^.Width)>tmp then
    PTrackbar(Sender)^.Width:=tmp;
 //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // need to move slider here
//  PControl(Sender).Update;
end;

procedure tAWKTrackbar.Erase(Sender: PControl; DC: HDC);
begin
//  Sender.Parent.Update;
end;

procedure tAWKTrackbar.Paint(Sender: PControl; DC: HDC);
var
  rc, rc1:TRECT;
  w:integer;
begin
  SendMessage(Sender.Handle,TBM_GETTHUMBRECT,0,tlparam(@rc));

  w:=rc.right-rc.left;
  if w<>16 then
    rc.left:=rc.left+(w div 2)-8;

  copyRect(rc1,Sender.BoundsRect);
  rc1.Right :=rc1.Right-rc1.Left-8;
  rc1.Left  :=4;
  rc1.Top   :=7{((rc1.Bottom-rc1.Top) div 2)-2};
  rc1.Bottom:=rc1.Top+4;
  DrawEdge(DC,rc1,EDGE_SUNKEN,BF_RECT or BF_ADJUST);
  if Sender.ChildCount>0 then
    Sender.Children[0].Left:=rc.Left
  else
  begin
    rc.right:=rc.left+8;
    DrawFrameControl(DC,rc,DFC_BUTTON,DFCS_BUTTONPUSH);
  end;
end;

procedure RefreshTrackbarIcons(Owner:PControl);
begin
  if Owner.ChildCount>0 then
    pIcoButton(Owner.Children[0]).RefreshIcon;
end;

function MakeNewTrackBar(AOwner:PControl):PTrackbar;
var
  D:pAWKTrackbar;
  btn:pIcoButton;
begin
  New(D, Create);
  result:=NewTrackbar(AOwner,[trbNoTicks,trbBoth,trbNoBorder],D.Scroll);

  with result^ do
  begin
    Transparent:=true;
    CustomObj:=D;
    SetSize(AOwner.Width-16,18);
    SetPosition({AOwner.Left+}8,{AOwner.Top+}AOwner.Height-18);
    Anchor(true,false,true,true);
    ThumbLen:=16;

    RangeMin:=0;
    RangeMax:=100;

    btn:=CreateIcoButton(result,GetIcon,DoAction,WAT_CTRL_SEEK);

    if btn<>nil then
    begin
      btn.ResetEvent(idx_fOnClick);
      D.OldMouseDown:=btn.OnMouseDown;
      btn.OnMouseDown:=D.PressButton;
      D.OldMouseUp:=btn.OnMouseUp;
      btn.OnMouseUp  :=D.UnPressButton;
      btn.OnMouseMove:=D.DragButton;
    end;

    OnResize    :=D.CtrlResize;
    OnEraseBkGnd:=D.Erase;
    OnPaint     :=D.Paint;

//    OnScroll    :=D.Scroll;
  end;

end;

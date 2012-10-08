{mRadio frame}
const
  frm_back:pAnsiChar = 'Frame background';
const
  FrameWnd:HWND = 0;
  FrameId:integer = -1;
  OldEditProc:pointer=nil;
  pattern:pWideChar=nil;
  current: THANDLE = 0;
var
  FrameCtrl:PControl;
  colorhook:THANDLE;
  hbr:HBRUSH;
  frm_bkg:TCOLORREF;

function RadioFrameProc( Sender: PControl; var Msg: TMsg; var Rslt:Integer ): boolean;
begin
  result:=false;
  rslt:=0;

  case Msg.message of

    WM_DESTROY: begin
      hVolFrmCtrl:=0;
      DeleteObject(hbr);
    end;

    WM_SHOWWINDOW: begin
      Rslt:=1;
      with Sender^ do
        hVolFrmCtrl:=Children[1].Handle;
    end;

    WM_CTLCOLORSTATIC: begin
//      SetBkColor(msg.wParam, frm_bkg);
      Rslt:=hbr;
      result:=True;
    end;

    WM_CONTEXTMENU: begin
      CallService('mRadio/Settings',0,0);
    end;

    WM_HSCROLL: begin
       case loword(msg.wParam) of
         SB_THUMBTRACK: begin
//           gVolume:=Hiword(msg.wParam);
           Service_RadioSetVolume(Hiword(msg.wParam){gVolume},2);
         end;
         SB_ENDSCROLL: begin
//           gVolume:=SendMessage(msg.lParam,TBM_GETPOS,0,0);
           Service_RadioSetVolume({Hiword(msg.wParam)}SendMessage(msg.lParam,TBM_GETPOS,0,0){gVolume},2)
         end;
       end;
    end;

  end;
end;

function DoAction(action:integer):integer;
begin
  result:=Service_RadioMute(0,1);
end;

function ColorReload(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  cid:TColourID;
//  wnd:HWND;
//  i:integer;
begin
  result:=0;
  cid.cbSize:=SizeOf(cid);
  StrCopy(cid.group,cPluginName);
  StrCopy(cid.name ,frm_back);
  frm_bkg:=CallService(MS_COLOUR_GETA,dword(@cid),0);
  if hbr<>0 then DeleteObject(hbr);
  hbr:=CreateSolidBrush(frm_bkg);

  FrameCtrl.Children[1].Width := FrameCtrl.Children[1].Width-1;
  FrameCtrl.Children[1].Width := FrameCtrl.Children[1].Width+1;

  FrameCtrl.InvalidateEx;
  FrameCtrl.Update;
end;

function GetIcon(action:integer;stat:integer=AST_NORMAL):cardinal;
begin
  case stat of
    AST_NORMAL : result:=CallService(MS_SKIN2_GETICON,0,dword(IcoBtnOn));
    AST_PRESSED: result:=CallService(MS_SKIN2_GETICON,0,dword(IcoBtnOff));
  else
    result:=0;
  end;
end;

procedure MyErase(DummySelf, Sender:PControl; DC:HDC);
begin
end;

procedure MyEraseFrame(DummySelf, Sender:PControl; DC:HDC);
var
  rc:TRECT;
begin
  GetClientRect(Sender.Handle, rc);
  Sender.Canvas.Brush.Color := frm_bkg;
  Sender.Canvas.FillRect(rc);
end;

function CreateFrameWindow(parent:HWND):THANDLE;
var
  TB:PTrackbar;
begin
  result:=0;

  FrameCtrl:=NewAlienPanel(parent,esNone);
  if FrameCtrl<>nil then
  begin
    result:=FrameCtrl.GetWindowHandle;
    with FrameCtrl^ do
    begin
      // Button
      btnMute:=CreateIcoButton(FrameCtrl,GetIcon,DoAction,IDC_RADIO_MUTE);
      with btnMute^ do
      begin
        SetSize(16,16);
        SetPosition(FrameCtrl.Width-18,0);
        Anchor(false,true,true,false);

        AsCheckbox:=true;
      end;

      // Trackbar
      TB:=NewTrackbar(FrameCtrl,[trbTooltips,trbNoTicks,trbBoth,trbNoBorder],nil);
      with TB^ do
      begin
        SetSize(FrameCtrl.Width-24,18);
        SetPosition(2,0);
        Anchor(true,true,true,false);

        RangeMin:=0;
        RangeMax:=100;
        PageSize:=20;

        OnEraseBkGnd:=TOnPaint(MakeMethod(nil, @MyErase));
      end;

      AttachProc(@RadioFrameProc);
    end;

    with FrameCtrl^ do
    begin
      MinWidth :=80;
      MinHeight:=20;
      Height:=18;
      Anchor(true,true,true,true);
//      OnMessage:=TOnMessage(MakeMethod(nil, @MyEraseFrame));
      OnEraseBkGnd:=TOnPaint(MakeMethod(nil, @MyEraseFrame));
{
      OnPaint    :=FrameCtrl.Paint;
      OnResize   :=FrameCtrl.FrameResize;
      OnMouseDown:=FrameCtrl.MouseDown;
}
    end;
  end;
end;

procedure CreateFrame(parent:HWND);
var
  Frame:TCLISTFrame;
//  wnd:HWND;
//  tmp:cardinal;
  tr:TRECT;
  cid:TColourID;
begin
  if ServiceExists(MS_CLIST_FRAMES_ADDFRAME)=0 then
    exit;
  if parent=0 then
    parent:=CallService(MS_CLUI_GETHWND,0,0);

  FrameWnd:=CreateFrameWindow(parent);

  if FrameWnd<>0 then
  begin
    GetWindowRect(FrameWnd,tr);
    FillChar(Frame,SizeOf(Frame),0);
    with Frame do
    begin
      cbSize  :=SizeOf(Frame);
      hWnd    :=FrameWnd;
      hIcon   :=0;
      align   :=alTop;
      height  :=tr.bottom-tr.top+2;
      if IsAnsi then
      begin
        Flags   :=F_VISIBLE or F_NOBORDER;
        name.a  :=cPluginName;
        TBName.a:=cPluginName + ' volume control';
      end
      else
      begin
        Flags   :=F_VISIBLE or F_NOBORDER or F_UNICODE;
        name.w  :=cPluginName;
        TBName.w:=cPluginName + ' volume control';
      end;
    end;

    FrameId:=CallService(MS_CLIST_FRAMES_ADDFRAME,dword(@Frame),0);
    if FrameId>=0 then
    begin
      CallService(MS_CLIST_FRAMES_UPDATEFRAME,FrameId, FU_FMPOS);

      cid.cbSize:=SizeOf(cid);
      cid.flags :=0;
      StrCopy(cid.group,cPluginName);
      StrCopy(cid.dbSettingsGroup,cPluginName);

      StrCopy(cid.name   ,frm_back);
      StrCopy(cid.setting,'frame_back');
      cid.defcolour:=COLOR_3DFACE;
      cid.order    :=0;
      ColourRegister(@cid);

      hbr:=0;
      colorhook:=HookEvent(ME_COLOUR_RELOAD,@ColorReload);
      ColorReload(0,0);
      Service_RadioSetVolume(gVolume,0);
    end;
  end;
end;

procedure DestroyFrame;
begin
  if FrameId>=0 then
  begin
    UnhookEvent(colorhook);
    CallService(MS_CLIST_FRAMES_REMOVEFRAME,FrameId,0);
    FrameId:=-1;
  end;
  FrameCtrl.Free;
  //  DestroyWindow(FrameWnd);
  FrameWnd:=0;
end;

{CList frame}
unit KOLFrame;

interface

implementation

uses windows,kol,commdlg,messages,common,commctrl, KOLCCtrls,
     wat_api,wrapper,global,m_api,dbsettings,waticons,mirutils,
     icobuttons,textblock,kolsizer;

{$include resource.inc}

{$R frm.res}

{$include frm_data.inc}
{$include frm_vars.inc}

procedure MouseDown(DummySelf, Sender:PControl;var Mouse:TMouseEventData);
var
  wnd:HWND;
begin
  wnd:=GetParent(Sender.GetWindowHandle);
  SendMessage(wnd,WM_SYSCOMMAND,
     SC_MOVE or HTCAPTION,MAKELPARAM(Mouse.x,Mouse.y));
end;

// ---------------- frame functions ----------------

procedure SetFrameTitle(title:pointer;icon:HICON;addflag:integer=FO_UNICODETEXT);
var
  D:PWATFrameData;
begin
  D:=FrameCtrl.CustomData;
  CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,
      (D.FrameId shl 16)+FO_TBNAME+addflag,tlparam(title));
  CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,(D.FrameId shl 16)+FO_ICON,icon);
  CallService(MS_CLIST_FRAMES_UPDATEFRAME,D.FrameId,FU_TBREDRAW);
end;

// -----------------------

function IsFrameMinimized(FrameId:integer):bool;
begin
  result:=(CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS,
          (FrameId shl 16)+FO_FLAGS,0) and F_UNCOLLAPSED)=0;
end;

function IsFrameFloated(FrameId:integer):bool;
begin
  result:=CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS,
          (FrameId shl 16)+FO_FLOATING,0)>0;
end;

function IsFrameHidden(FrameId:integer):bool;
begin
  result:=(CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS,
          (FrameId shl 16)+FO_FLAGS,0) and F_VISIBLE)=0;
end;

procedure HideFrame(FrameId:integer);
begin
  if not IsFrameHidden(FrameId) then
  begin
    CallService(MS_CLIST_FRAMES_SHFRAME,FrameId,0);
    HiddenByMe:=true;
  end;
end;

function ShowFrame(FrameId:integer):integer;
begin
  result:=0;
  if IsFrameHidden(FrameId) then
    if HiddenByMe then
    begin
      CallService(MS_CLIST_FRAMES_SHFRAME,FrameId,0);
      HiddenByMe:=false;
    end
    else
      result:=1;
end;

{$include frm_rc.inc}
{$include frm_icogroup.inc}
{$include frm_trackbar.inc}
{$include frm_text.inc}
{$include frm_frame.inc}
{$include frm_designer.inc}

{$include frm_dlg1.inc}
{$include frm_dlg2.inc}

// ---------------- basic frame functions ----------------

function NewPlStatus(wParam:WPARAM;lParam:LPARAM):int;cdecl;
const
  needToChange:boolean=true;
var
  bufw:array [0..511] of WideChar;
//  FrameWnd:HWND;
  Cover:pAnsiChar;
  D:PWATFrameData;
begin
  result:=0;
//  FrameWnd:=FrameCtrl.Form.GetWindowHandle;
  D:=FrameCtrl.CustomData;

  case wParam of
    WAT_EVENT_PLAYERSTATUS: begin
      case integer(loword(lParam)) of
        WAT_PLS_STOPPED : begin
          if D.HideNoMusic then
            HideFrame(D.FrameId)
          else
            ShowFrame(D.FrameId); // if was hidden with "no player"
        end;
        WAT_PLS_NOTFOUND: begin
          if D.HideNoPlayer then
            HideFrame(D.FrameId);

          SetFrameTitle(PluginShort,0,0); // frame update code there
        end;
      else
        exit;
      end;
      FrameCtrl.ResetFrame;
    end;

    WAT_EVENT_NEWTRACK: begin
      // cover
      if D.UseCover then
        if (pSongInfo(lParam)^.Cover<>nil) and (pSongInfo(lParam)^.Cover^<>#0) then
        begin
          GetShortPathNameW(pSongInfo(lParam)^.Cover,bufw,SizeOf(bufw));
          WideToAnsi(bufw,Cover);
          FrameCtrl.RefreshPicture(Cover);
          mFreeMem(Cover);
        end;

      // trackbar
      TrackbarSetRange(D.Trackbar,D.UpdInterval,pSongInfo(lParam)^.total);

      if (D.UpdTimer=0) and (D.UpdInterval>0) then
        D.UpdTimer:=SetTimer(0,0,D.UpdInterval,@FrameTimerProc);

      // text
      UpdateTextBlock(D,true);

      ShowFrame(D.FrameId);
    end;

    WAT_EVENT_NEWPLAYER: begin
      SetFrameTitle(pSongInfo(lParam)^.player,pSongInfo(lParam)^.icon);
      // new player must call "no music" at least, so we have chance to show frame
    end;

    WAT_EVENT_PLUGINSTATUS: begin
      case lParam of
        dsEnabled: begin
          ShowFrame(D.FrameId);
          // plus - start frame and text timers
          if D.UpdInterval>0 then
            D.UpdTimer:=SetTimer(0,0,D.UpdInterval,@FrameTimerProc);
        end;

        dsPermanent: begin
          HideFrame(D.FrameId);

          // plus - stop frame and text timers
          if D.UpdTimer<>0 then
          begin
            KillTimer(0,D.UpdTimer);
            D.UpdTimer:=0;
          end;
        end;
      end;
    end;
  end;
end;

function IconChanged(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  result:=0;
  if PWATFrameData(FrameCtrl.CustomData).FrameId<>0 then
  begin
    FrameCtrl.RefreshAllFrameIcons;
    ShowWindow(FrameCtrl.GetWindowHandle,SW_HIDE);
    ShowWindow(FrameCtrl.GetWindowHandle,SW_SHOW);
  end;
end;

//??const opt_FrmHeight :PAnsiChar = 'frame/frmheight';

function CreateFrame(parent:HWND):boolean;
var
  CLFrame:TCLISTFrame;
  rc:TRECT;
  FrameWnd:HWND;
begin
  result:=false;
  if ServiceExists(MS_CLIST_FRAMES_ADDFRAME)=0 then
    exit;

  if parent=0 then
    parent:=CallService(MS_CLUI_GETHWND,0,0);

  FrameWnd:=CreateFrameWindow(parent);

  if FrameWnd<>0 then
  begin
    FillChar(CLFrame,SizeOf(CLFrame),0);
    with CLFrame do
    begin
      cbSize  :=SizeOf(CLFrame);
      hWnd    :=FrameWnd;
      hIcon   :=0;
      align   :=alTop;
      GetClientRect(FrameWnd,rc);
//??      height  :=DBReadWord(0,PluginShort,opt_FrmHeight,rc.bottom-rc.top);
      Flags   :=0;//{F_VISIBLE or} F_SHOWTB;
      name.a  :=PluginShort;
      TBName.a:=PluginShort;
    end;
    FrameHeight:=CLFrame.height;

    PWATFrameData(FrameCtrl.CustomData).FrameId:=CallService(MS_CLIST_FRAMES_ADDFRAME,twparam(@CLFrame),0);
    if PWATFrameData(FrameCtrl.CustomData).FrameId>=0 then
    begin
      plStatusHook:=HookEvent(ME_WAT_NEWSTATUS,@NewPlStatus);
    end;
  end;
  result:=FrameWnd<>0;
end;

procedure DestroyFrame;
var
  id:integer;
begin
  if (FrameCtrl<>nil) and (PWATFrameData(FrameCtrl.CustomData).FrameId>=0) then
  begin
    UnhookEvent(plStatusHook);

    id:=PWATFrameData(FrameCtrl.CustomData).FrameId;
    FrameCtrl.Free;
    FrameCtrl:=nil;
    CallService(MS_CLIST_FRAMES_REMOVEFRAME,Id,0);
  end;
end;

const
  opt_ModStatus:PAnsiChar = 'module/frame';

function GetModStatus:integer;
begin
  result:=DBReadByte(0,PluginShort,opt_ModStatus,1);
end;

procedure SetModStatus(stat:integer);
begin
  DBWriteByte(0,PluginShort,opt_modStatus,stat);
end;

// ---------------- base interface procedures ----------------

function InitProc(aGetStatus:boolean=false):integer;
begin
  FrameCtrl:=nil;
  result:=0;
  if aGetStatus then
  begin
    if GetModStatus=0 then
      exit;
  end
  else
    SetModStatus(1);

  result:=ord(CreateFrame(0));
  if result<>0 then
    sic:=HookEvent(ME_SKIN2_ICONSCHANGED,@IconChanged);
end;

procedure DeInitProc(aSetDisable:boolean);
begin
  if aSetDisable then
    SetModStatus(0);

  if sic<>0 then
  begin
    UnhookEvent(sic);
    sic:=0;
  end;
  DestroyFrame;
end;

function AddOptionsPage(var tmpl:pAnsiChar;var proc:pointer;var name:PAnsiChar):integer;
const
  count:integer=2;
begin
  if count=0 then
    count:=2;
  if count=2 then
  begin
    tmpl:=PAnsiChar(IDD_OPTION_FRAME);
    proc:=@FrameViewDlg;
    name:='Frame (main)';
  end
  else
  begin
    tmpl:=PAnsiChar(IDD_OPTION_FRAME2);
    proc:=@FrameTextDlg;
    name:='Frame (text)';
  end;

  dec(count);
  result:=count;
end;

var
  Frame:twModule;

procedure Init;
begin
  Frame.Next      :=ModuleLink;
  Frame.Init      :=@InitProc;
  Frame.DeInit    :=@DeInitProc;
  Frame.AddOption :=@AddOptionsPage;
  Frame.Check     :=nil;
  Frame.ModuleName:='Frame';
  ModuleLink      :=@Frame;
end;

begin
  Init;
end.

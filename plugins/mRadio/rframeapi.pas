{mRadio frame}
unit rframeapi;

interface

uses
  windows;

const
  hVolFrmCtrl:HWND=0;
  hVolFrmMute:HWND=0;

procedure CreateFrame(parent:HWND);
procedure DestroyFrame;

implementation

uses
  messages,commctrl,
  m_api,common,
  rglobal;

{$include mr_rc.inc}

const
  frm_back:pAnsiChar = 'Frame background';
const
  FrameWnd:HWND = 0;
  FrameId:integer = -1;
var
  colorhook:THANDLE;
  hbr:HBRUSH;
  frm_bkg:TCOLORREF;
  OldSliderWndProc:pointer;

function QSDlgResizer(Dialog:HWND;lParam:LPARAM;urc:PUTILRESIZECONTROL):int; cdecl;
begin
  case urc^.wId of
    IDC_RADIO_OPEN: result:=RD_ANCHORX_LEFT  or RD_ANCHORY_CENTRE;
    IDC_RADIO_MUTE: result:=RD_ANCHORX_RIGHT or RD_ANCHORY_CENTRE;
    IDC_RADIO_VOL : result:=RD_ANCHORX_WIDTH or RD_ANCHORY_CENTRE;
  else
    result:=0;
  end;
end;

function SliderWndProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
begin
  if hMessage=WM_ERASEBKGND then
    result:=1
  else
    result:=CallWindowProc(OldSliderWndProc, Dialog, hMessage, wParam, lParam);
end;

procedure MakeTooltip(Dialog:HWND);
var
  hwndTooltip:HWND;
  ti:TTOOLINFOW;
begin
  hwndTooltip:=CreateWindowW(TOOLTIPS_CLASS,nil,TTS_ALWAYSTIP,
      integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
      integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
      Dialog,0,hInstance,nil);

  FillChar(ti,SizeOf(ti),0);
  ti.cbSize  :=sizeof(TOOLINFO);
  ti.uFlags  :=TTF_IDISHWND or TTF_SUBCLASS;
  ti.hwnd    :=Dialog;
  ti.hinst   :=hInstance;
  ti.uId     :=GetDlgItem(Dialog,IDC_RADIO_MUTE);
  ti.lpszText:=pWideChar(TranslateW('Mute'));
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,tlparam(@ti));
end;

function RadioFrameProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  urd:TUTILRESIZEDIALOG;
  rc:TRECT;
  tmp:pAnsiChar;
begin
  result:=0;
  case hMessage of
    WM_DESTROY: begin
      hVolFrmCtrl:=0;
      hVolFrmMute:=0;
      DeleteObject(hbr);
      hbr:=0;
    end;

    WM_INITDIALOG: begin
      SendDlgItemMessage(Dialog,IDC_RADIO_MUTE, BUTTONSETASFLATBTN,0,0);
//      SetButtonIcon(hMuteFrmCtrl,IcoBtnOn);

      MakeTooltip(Dialog);

      hVolFrmCtrl:=GetDlgItem(Dialog,IDC_RADIO_VOL);
      hVolFrmMute:=GetDlgItem(Dialog,IDC_RADIO_MUTE);
      SendMessage(hVolFrmCtrl,TBM_SETRANGE,0,MAKELONG(0,100));
      SendMessage(hVolFrmCtrl,TBM_SETPAGESIZE,0,20);
      SendMessage(hVolFrmCtrl,TBM_SETPOS,1,gVolume);

      OldSliderWndProc:=pointer(SetWindowLongPtrW(hVolFrmCtrl,GWL_WNDPROC,LONG_PTR(@SliderWndProc)));
    end;

    WM_SIZE: begin
      FillChar(urd,SizeOf(TUTILRESIZEDIALOG),0);
      urd.cbSize    :=SizeOf(urd);
      urd.hwndDlg   :=Dialog;
      urd.hInstance :=hInstance;
      urd.lpTemplate:=MAKEINTRESOURCEA(IDD_FRAME);
      urd.lParam    :=0;
      urd.pfnResizer:=@QSDlgResizer;
      CallService(MS_UTILS_RESIZEDIALOG,0,tlparam(@urd));
    end;

    WM_ERASEBKGND: begin
      GetClientRect(Dialog,rc);
      FillRect(wParam,rc,hbr);
      result:=1;
    end;

    WM_DRAWITEM: begin
      case wParam of
        IDC_RADIO_OPEN: begin
          result:=1;
          DrawIconEx(PDRAWITEMSTRUCT(lParam)^.hDC,0,0,
              CallService(MS_SKIN2_GETICON,0,TLPARAM(IcoBtnOpen)),
              16,16,0,hbr,DI_NORMAL);
        end;

        IDC_RADIO_MUTE: begin
          result:=1;
          if gVolume<0 then
            tmp:=IcoBtnOff
          else
            tmp:=IcoBtnOn;
          DrawIconEx(PDRAWITEMSTRUCT(lParam)^.hDC,0,0,
              CallService(MS_SKIN2_GETICON,0,TLPARAM(tmp)),
              16,16,0,hbr,DI_NORMAL);
        end;
      end;
    end;

    WM_CTLCOLORBTN: begin
      if (THANDLE(lParam)=GetDlgItem(Dialog,IDC_RADIO_MUTE)) or
         (THANDLE(lParam)=GetDlgItem(Dialog,IDC_RADIO_OPEN)) then
      begin
        SetBkColor(wParam, frm_bkg);
        result:=hbr;
      end;
    end;

    WM_CTLCOLORSTATIC: begin
      if THANDLE(lParam)=hVolFrmCtrl then
      begin
        SetBkColor(wParam, frm_bkg);
        result:=hbr;
      end;
    end;

    WM_CONTEXTMENU: begin
      CallService(cPluginName + '/Settings',0,0);
    end;

    WM_COMMAND: begin
      case wParam shr 16 of

        BN_CLICKED: begin
          case loword(wParam) of
            IDC_RADIO_OPEN: begin
              CallService(MS_RADIO_QUICKOPEN,0,0);
            end;
            IDC_RADIO_MUTE: begin
              CallService(MS_RADIO_MUTE,0,1);
            end;
          end;
        end;
      end;
    end;

    WM_HSCROLL: begin
//      gVolume:=SendMessage(lParam,TBM_GETPOS,0,0);
      CallService(MS_RADIO_SETVOL,SendMessage(lParam,TBM_GETPOS,0,0){gVolume},2)
    end;

  else
    result:=DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;

function ColorReload(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  cid:TColourID;
  wnd:HWND;
begin
  result:=0;
  cid.cbSize:=SizeOf(cid);
  StrCopy(cid.group,cPluginName);
  StrCopy(cid.name ,frm_back);
  frm_bkg:=CallService(MS_COLOUR_GETA,twparam(@cid),0);
  if hbr<>0 then DeleteObject(hbr);
  hbr:=CreateSolidBrush(frm_bkg);

  wnd:=GetFocus();
	InvalidateRect(hVolFrmCtrl,nil,true);
	SetFocus(hVolFrmCtrl);
  RedrawWindow(FrameWnd,nil,0,RDW_INVALIDATE or RDW_ALLCHILDREN or RDW_ERASE);
  SetFocus(wnd);
end;

procedure CreateFrame(parent:HWND);
var
  Frame:TCLISTFrame;
  wnd:HWND;
  tmp:cardinal;
  tr:TRECT;
  cid:TColourID;
begin
  if ServiceExists(MS_CLIST_FRAMES_ADDFRAME)=0 then
    exit;
  if parent=0 then
    parent:=CallService(MS_CLUI_GETHWND,0,0);

  if FrameWnd=0 then
    FrameWnd:=CreateDialog(hInstance,MAKEINTRESOURCE(IDD_FRAME),parent,@RadioFrameProc);

  if FrameWnd<>0 then
  begin
    GetWindowRect(FrameWnd,tr);
    FillChar(Frame,SizeOf(Frame),0);
    with Frame do
    begin
      cbSize  :=SizeOf(Frame);
      hWnd    :=FrameWnd;
      hIcon   :=CallService(MS_SKIN2_GETICON,0,lparam(IcoBtnSettings));
      align   :=alTop;
      height  :=tr.bottom-tr.top+2;
      Flags   :=F_VISIBLE or F_NOBORDER or F_UNICODE;
      name.w  :=cPluginName;
      TBName.w:=TranslateW(cPluginName + ' volume control');
    end;

    FrameId:=CallService(MS_CLIST_FRAMES_ADDFRAME,wparam(@Frame),0);
    if FrameId>=0 then
    begin
      CallService(MS_CLIST_FRAMES_UPDATEFRAME,FrameId, FU_FMPOS);

      wnd:=CallService(MS_CLUI_GETHWND{MS_CLUI_GETHWNDTREE},0,0);
      tmp:=SendMessage(wnd,CLM_GETEXSTYLE,0,0);
      SendMessage(wnd,CLM_SETEXSTYLE,tmp or CLS_EX_SHOWSELALWAYS,0);

      cid.cbSize:=SizeOf(cid);
      cid.flags :=0;
      StrCopy(cid.group,cPluginName);
      StrCopy(cid.dbSettingsGroup,cPluginName);

      StrCopy(cid.name   ,frm_back);
      StrCopy(cid.setting,'frame_back');
      cid.defcolour:=GetSysColor(COLOR_3DFACE);
      cid.order    :=0;
      ColourRegister(@cid);

      hbr:=0;
      colorhook:=HookEvent(ME_COLOUR_RELOAD,@ColorReload);
      ColorReload(0,0);
      CallService(MS_RADIO_SETVOL,gVolume,0);
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
  DestroyWindow(FrameWnd);
  FrameWnd:=0;
end;

end.

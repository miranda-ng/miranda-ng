unit sr_frame;

interface

uses windows;

procedure CreateFrame(parent:HWND);
procedure DestroyFrame;

implementation

uses commctrl,Messages,m_api,common,wrapper,mirutils,sr_global;

{$include resource.inc}

const
  frm_back:pAnsiChar = 'Frame background';
const
  FrameWnd:HWND = 0;
  FrameId:integer = -1;
  OldEditProc:pointer=nil;
  pattern:pWideChar=nil;
var
  colorhook:THANDLE;
  hbr:HBRUSH;
  frm_bkg:TCOLORREF;

function NewEditProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
begin
//  result:=0;
  case hMessage of
    WM_CHAR: begin
      if wParam=27 then
      begin
        // clear edit field
        SendMessage(Dialog,WM_SETTEXT,0,0);
        result:=0;
      end
      else if wParam=13 then
      begin
        CallService(QS_SHOWSERVICE,twparam(pattern),0);
        result:=0;
      end
      else
        result:=1;
      if result=0 then
        exit;
    end;
  end;
  result:=CallWindowProc(OldEditProc,Dialog,hMessage,wParam,lParam);
end;

function QSDlgResizer(Dialog:HWND;lParam:LPARAM;urc:PUTILRESIZECONTROL):int; cdecl;
begin
  case urc^.wId of
    IDC_FRAME_OPEN: result:=RD_ANCHORX_LEFT  or RD_ANCHORY_CENTRE;
    IDC_FRAME_EDIT: result:=RD_ANCHORX_WIDTH or RD_ANCHORY_CENTRE;
  else
    result:=0;
  end;
end;

function QSFrameProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  urd:TUTILRESIZEDIALOG;
  rc:TRECT;
  TI:TTOOLINFOW;
  hwndTooltip:HWND;
begin
  result:=0;
  case hMessage of
    WM_DESTROY: begin
      DeleteObject(hbr);
      hbr:=0;
    end;

    WM_INITDIALOG: begin
      OldEditProc:=pointer(SetWindowLongPtrW(GetDlgItem(Dialog,IDC_FRAME_EDIT),
         GWL_WNDPROC,LONG_PTR(@NewEditProc)));

      hwndTooltip:=CreateWindowW(TOOLTIPS_CLASS,nil,TTS_ALWAYSTIP,
          integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
          integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
          Dialog,0,hInstance,nil);

      FillChar(TI,SizeOf(TI),0);
      TI.cbSize  :=sizeof(TOOLINFO);
      TI.uFlags  :=TTF_IDISHWND or TTF_SUBCLASS;
      TI.hwnd    :=Dialog;
      TI.hinst   :=hInstance;
      TI.uId     :=GetDlgItem(Dialog,IDC_FRAME_OPEN);
      TI.lpszText:=pWideChar(TranslateW('Open QS window'));
      SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,tlparam(@TI));

      SetButtonIcon(GetDlgItem(Dialog,IDC_FRAME_OPEN),QS_QS);
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

    WM_COMMAND: begin
      case wParam shr 16 of
        EN_CHANGE: begin
          mFreeMem(pattern);
          pattern:=GetDlgText(Dialog,IDC_FRAME_EDIT);
          if pattern<>nil then
            CharLowerW(pattern);
        end;

        BN_CLICKED: begin
          case loword(wParam) of
            IDC_FRAME_OPEN: CallService(QS_SHOWSERVICE,twparam(pattern),0);
          end;
        end;
      end;
    end;

  else
    result:=DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;

function ColorReload(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  cid:TColourID;
begin
  result:=0;
  cid.cbSize:=SizeOf(cid);
  StrCopy(cid.group,'QuickSearch');
  StrCopy(cid.name ,frm_back);
  frm_bkg:=CallService(MS_COLOUR_GETA,twparam(@cid),0);
  DeleteObject(hbr);
  hbr:=CreateSolidBrush(frm_bkg);

  RedrawWindow(FrameWnd,nil,0,RDW_ERASE);
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

  hbr:=0;

  if parent=0 then
    parent:=CallService(MS_CLUI_GETHWND,0,0);

  if FrameWnd=0 then
    FrameWnd:=CreateDialog(hInstance,MAKEINTRESOURCE(IDD_FRAME),parent,@QSFrameProc);

  if FrameWnd<>0 then
  begin
    GetWindowRect(FrameWnd,tr);
    FillChar(Frame,SizeOf(Frame),0);
    with Frame do
    begin
      cbSize  :=SizeOf(Frame);
      hWnd    :=FrameWnd;
      hIcon   :=CallService(MS_SKIN2_GETICON,0,tlparam(QS_QS));
      align   :=alTop;
      height  :=tr.bottom-tr.top+2;
      Flags   :=F_NOBORDER or F_UNICODE;
      name.w  :='Quick search';
      TBName.w:='Quick search';
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
      StrCopy(cid.group,'QuickSearch');
      StrCopy(cid.dbSettingsGroup,qs_module);

      StrCopy(cid.name   ,frm_back);
      StrCopy(cid.setting,'frame_back');
      cid.defcolour:=GetSysColor(COLOR_3DFACE);
      cid.order    :=0;
      ColourRegister(@cid);

      colorhook:=HookEvent(ME_COLOUR_RELOAD,@ColorReload);
      ColorReload(0,0);
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
  mFreeMem(pattern);
end;

end.

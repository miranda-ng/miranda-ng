unit sr_frame;

interface

uses windows;

procedure CreateFrame(parent:HWND);
procedure DestroyFrame;

implementation

uses commctrl,Messages,m_api,common,wrapper,mirutils,sr_global;

{.$include resource.inc}

const
  frm_back:pAnsiChar = 'Frame background';
const
  FrameWnd:HWND = 0;
  FrameId:integer = -1;
  OldEditProc:pointer=nil;
  pattern:pWideChar=nil;
  current: THANDLE = 0;
var
  colorhook:THANDLE;
  hbr:HBRUSH;
  frm_bkg:TCOLORREF;

function CheckContact(wnd:HWND; hContact:THANDLE):THANDLE;
var
  buf:array [0..127] of WideChar;
begin
  result:=0;

  SendMessage(wnd,CLM_GETITEMTEXT,hContact,lparam(@buf));

  if StrPosW(CharLowerW(@buf),pattern)<>nil then
    result:=hContact;
end;

function GetNextContact(wnd:HWND; root,hContact:THANDLE; direction,skip:integer):THANDLE;
var
  tmp:THANDLE;
//  buf:array [0..127] of WideChar;
begin
  result:=0;
  repeat

    if skip=0 then
    begin
      case SendMessage(wnd,CLM_GETITEMTYPE,hContact,0) of
        CLCIT_GROUP: begin
          if SendMessage(wnd,CLM_GETEXPAND,hContact,0)=CLE_EXPAND then // expanded only
          begin
            tmp:=SendMessage(wnd,CLM_GETNEXTITEM,CLGN_CHILD,hContact);
            if tmp<>0 then
            begin
              hContact:=tmp;
              if direction=CLGN_PREVIOUS then
              begin // set on last contact in group
                repeat
                  tmp:=SendMessage(wnd,CLM_GETNEXTITEM,CLGN_NEXT,hContact);
                  if tmp<>0 then
                    hContact:=tmp
                  else
                    break;
                until false;
              end;
              continue;
            end;
          end;
        end;
        CLCIT_CONTACT: begin
          result:=CheckContact(wnd,hContact);
          if result<>0 then
            break;
        end;
      else // CLCIT_INVALID for example
        break;
      end;
    end
    else
      skip:=0;

      if hContact=HCONTACT_ISGROUP then
//    if (hContact=root) and (direction=CLGN_PREVIOUS) then // 1st group, no need to process "root=clist"
      break;

    tmp:=SendMessage(wnd,CLM_GETNEXTITEM,direction,hContact);
    if tmp=0 then
    begin
      hContact:=SendMessage(wnd,CLM_GETNEXTITEM,CLGN_PARENT,hContact);
      // not here but where group insert
      skip:=1;
      continue;
      //      hContact:=SendMessage(wnd,CLM_GETNEXTITEM,direction,hContact);
      if hContact=0 then
        break;
    end
    else
      hContact:=tmp;
  until false;
end;

function SearchContact(direction:integer;skip:integer=1):integer;
var
  root,hContact:THANDLE;
  wnd:HWND;
begin
  result:=0;
  if (pattern=nil) or (pattern^=#0) then
  begin
    current:=0; // next seach - from start
    exit; // pattern empty
  end;

  wnd:=CallService(MS_CLUI_GETHWNDTREE,0,0);

  root:=SendMessage(wnd,CLM_GETNEXTITEM,CLGN_ROOT,0);
  if current=0 then
    hContact:=root
  else
    hContact:=current;

  result:=GetNextContact(wnd,root,hContact,direction,skip);

  if result<>0 then
    SetCListSelContact(result);
end;

function NewEditProc(Dialog:HWnd;hMessage:UINT;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
begin
//  result:=0;
  case hMessage of
    WM_CHAR: if wParam=27 then
    begin
      // clear edit field
      SendMessage(Dialog,WM_SETTEXT,0,0);
      // go to top?
      current:=0;
    end;

    WM_KEYDOWN: begin
      case wParam of
        VK_PRIOR,VK_UP: begin
          current:=SearchContact(CLGN_PREVIOUS);
        end;
        VK_NEXT,VK_DOWN: begin
          current:=SearchContact(CLGN_NEXT);
        end;
      end;
    end;
  end;
  result:=CallWindowProc(OldEditProc,Dialog,hMessage,wParam,lParam);
end;

function QSDlgResizer(Dialog:HWND;lParam:LPARAM;urc:PUTILRESIZECONTROL):int; cdecl;
begin
  case urc^.wId of
    IDC_FRAME_OPEN: result:=RD_ANCHORX_LEFT  or RD_ANCHORY_CENTRE;
    IDC_FRAME_EDIT: result:=RD_ANCHORX_WIDTH or RD_ANCHORY_CENTRE;
    IDC_FRAME_PREV: result:=RD_ANCHORX_RIGHT or RD_ANCHORY_CENTRE;
    IDC_FRAME_NEXT: result:=RD_ANCHORX_RIGHT or RD_ANCHORY_CENTRE;
  else
    result:=0;
  end;
end;

function QSFrameProc(Dialog:HWnd;hMessage:UINT;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
var
  urd:TUTILRESIZEDIALOG;
  tmp:THANDLE;
  rc:TRECT;
  ti:TTOOLINFOW;
  hwndTooltip:HWND;
begin
  result:=0;
  case hMessage of
    WM_DESTROY: begin
      DeleteObject(hbr);
    end;

    WM_INITDIALOG: begin
      OldEditProc:=pointer(SetWindowLongPtrW(GetDlgItem(Dialog,IDC_FRAME_EDIT),
         GWL_WNDPROC,LONG_PTR(@NewEditProc)));

      hwndTooltip:=CreateWindowW(TOOLTIPS_CLASS,nil,TTS_ALWAYSTIP,
          integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
          integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
          Dialog,0,hInstance,nil);

      FillChar(ti,SizeOf(ti),0);
      ti.cbSize  :=sizeof(TOOLINFO);
      ti.uFlags  :=TTF_IDISHWND or TTF_SUBCLASS;
      ti.hwnd    :=Dialog;
      ti.hinst   :=hInstance;
      ti.uId     :=GetDlgItem(Dialog,IDC_FRAME_PREV);

      ti.lpszText:=pWideChar(TranslateW('Previous item'));
      SendMessage(hwndTooltip,TTM_ADDTOOLW,0,tlparam(@ti));
      ti.uId     :=GetDlgItem(Dialog,IDC_FRAME_NEXT);
      ti.lpszText:=pWideChar(TranslateW('Next item'));
      SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,tlparam(@ti));
      ti.uId     :=GetDlgItem(Dialog,IDC_FRAME_OPEN);
      ti.lpszText:=pWideChar(TranslateW('Open main window'));
      SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,tlparam(@ti));
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

          tmp:=SearchContact(CLGN_NEXT,0);
          if tmp=0 then
            tmp:=SearchContact(CLGN_PREVIOUS,0);
          current:=tmp;
        end;

        BN_CLICKED: begin
          tmp:=0;
          case loword(wParam) of
            IDC_FRAME_PREV: tmp:=SearchContact(CLGN_PREVIOUS);
            IDC_FRAME_NEXT: tmp:=SearchContact(CLGN_NEXT);
            IDC_FRAME_OPEN: CallService(QS_SHOWSERVICE,twparam(pattern),0);
          end;
          if tmp<>0 then current:=tmp;
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
      hIcon   :=0;
      align   :=alTop;
      height  :=tr.bottom-tr.top+2;
      Flags   :=F_VISIBLE or F_NOBORDER or F_UNICODE;
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
      cid.defcolour:=COLOR_3DFACE;
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

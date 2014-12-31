unit editwrapper;

interface

uses windows,m_api;

// exported flags
const
  EF_SCRIPT = 1;         // right now, just "Variables" script
  EF_ALL    = EF_SCRIPT; // what can be changed in runtime
  EF_FORCES = $80;
  EF_FORCET = $40;
  EF_FORCE  = EF_FORCES or EF_FORCET;

function MakeEditField(Dialog:HWND; id:uint):HWND;
procedure SetEditFlags(wnd:HWND; mask, flags:dword); overload;
procedure SetEditFlags(Dialog:HWND; id:uint; mask,flags:dword); overload;
function GetEditFlags(wnd:HWND):dword; overload;
function GetEditFlags(Dialog:HWND; id:uint):dword; overload;
function EnableEditField(wnd:HWND; enable:boolean):boolean; overload;
function EnableEditField(Dialog:HWND; id:uint; enable:boolean):boolean; overload;
function ShowEditField(wnd:HWND; mode:integer):boolean;overload;
function ShowEditField(Dialog:HWND; id:uint; mode:integer):boolean;overload;

{
  -1 - cancel
   1 - script
   0 - new text
}
function ShowEditBox(parent:HWND;var text:PWideChar;title:PWideChar):int_ptr;


implementation

uses messages,commctrl,common,wrapper;

{$R editwrapper.res}
{$include 'i_text_const.inc'}

// internal flags
const
  EF_WRAP = 2;

// EditFields & EditForm -> Button -> [EditField,WinProc,flags]
type
  pUserData = ^tUserData;
  tUserData = record
    SavedProc    :pointer;
    LinkedControl:HWND;
    LinkedCtrlId :integer;
    flags        :dword;
  end;

procedure SetButtonTitle(btnwnd:HWND);
var
  title:PWideChar;
  ptr:pUserData;
begin
  ptr:=pUserData(GetWindowLongPtrW(btnwnd,GWLP_USERDATA));
  if (ptr^.flags and EF_SCRIPT)<>0 then
    title:='S'
  else
    title:='T';
  SendMessageW(btnwnd,WM_SETTEXT,0,tlParam(title));
end;

function EditDlgResizer(Dialog:HWND;lParam:LPARAM;urc:PUTILRESIZECONTROL):int; cdecl;
begin
  case urc^.wId of
    IDCANCEL:         result:=RD_ANCHORX_RIGHT or RD_ANCHORY_BOTTOM;
    IDOK:             result:=RD_ANCHORX_RIGHT or RD_ANCHORY_BOTTOM;
    IDC_SCRIPT_HELP:  result:=RD_ANCHORX_RIGHT or RD_ANCHORY_TOP;
    IDC_TEXT_SCRIPT:  result:=RD_ANCHORX_LEFT  or RD_ANCHORY_TOP;
    IDC_TEXT_WRAP:    result:=RD_ANCHORX_LEFT  or RD_ANCHORY_BOTTOM;
    IDC_TEXT_EDIT_W:  result:=RD_ANCHORX_WIDTH or RD_ANCHORY_HEIGHT;
    IDC_TEXT_EDIT_NW: result:=RD_ANCHORX_WIDTH or RD_ANCHORY_HEIGHT;
  else
    result:=0;
  end;
end;

// if need to change button text, will pass button (not edit field) handle as parameter
function EditWndProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  pc:PWideChar;
  btnwnd:HWND;
  ptr:pUserData;
  wnd,wnd1:HWND;
  vhi:TVARHELPINFO;
  cr:integer;
  idshow,idhide:integer;
  urd:TUTILRESIZEDIALOG;
begin
  result:=0;

  case hMessage of
    WM_DESTROY: begin
    end;

    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);
      ptr:=pUserData(GetWindowLongPtrW(HWND(lParam),GWLP_USERDATA));

      SetWindowLongPtrW(Dialog,GWLP_USERDATA,lParam);
      pc:=GetDlgText(ptr^.LinkedControl);

      if (ptr^.flags and (EF_FORCES or EF_FORCET))<>0 then
        EnableWindow(GetDlgItem(Dialog,IDC_TEXT_SCRIPT),false);

      if (ptr^.flags and EF_SCRIPT)<>0 then
        CheckDlgButton(Dialog,IDC_TEXT_SCRIPT,BST_CHECKED);

      if (ptr^.flags and EF_WRAP)<>0 then
      begin
        CheckDlgButton(Dialog,IDC_TEXT_WRAP,BST_CHECKED);
        idshow:=IDC_TEXT_EDIT_W;
        idhide:=IDC_TEXT_EDIT_NW;
      end
      else
      begin
        idshow:=IDC_TEXT_EDIT_NW;
        idhide:=IDC_TEXT_EDIT_W;
      end;

      SetDlgItemTextW(Dialog,idshow,pc);
      mFreeMem(pc);
      ShowWindow(GetDlgItem(Dialog,idshow),SW_SHOW);
      ShowWindow(GetDlgItem(Dialog,idhide),SW_HIDE);

      wnd:=GetDlgItem(Dialog,IDC_SCRIPT_HELP);
      if ServiceExists(MS_VARS_FORMATSTRING)<>0 then
      begin
        SendMessage(wnd,BM_SETIMAGE,IMAGE_ICON,
            CallService(MS_VARS_GETSKINITEM,0,VSI_HELPICON));
        SendMessage(wnd,BUTTONADDTOOLTIP,
            CallService(MS_VARS_GETSKINITEM,0,VSI_HELPTIPTEXT),0);
{
        SendMessage(wnd,BM_SETIMAGE,IMAGE_ICON,
            CallService(MS_SKIN_LOADICON,SKINICON_OTHER_HELP,0));
}
      end
      else
      begin
        ShowWindow(wnd,SW_HIDE);
      end;
    end;

    WM_GETMINMAXINFO: begin
      with PMINMAXINFO(lParam)^ do
      begin
        ptMinTrackSize.x:=200;
        ptMinTrackSize.y:=180;
      end;
    end;

    WM_SIZE: begin
      FillChar(urd,SizeOf(TUTILRESIZEDIALOG),0);
      urd.cbSize    :=SizeOf(urd);
      urd.hwndDlg   :=Dialog;
      urd.hInstance :=hInstance;
      urd.lpTemplate:='IDD_EDITCONTROL';
      urd.lParam    :=0;
      urd.pfnResizer:=@EditDlgResizer;
      CallService(MS_UTILS_RESIZEDIALOG,0,tlparam(@urd));
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        BN_CLICKED: begin
          btnwnd:=GetWindowLongPtrW(Dialog,GWLP_USERDATA);
          ptr:=pUserData(GetWindowLongPtrW(btnwnd,GWLP_USERDATA));

          case loword(wParam) of
            IDC_TEXT_WRAP: begin
              if IsDlgButtonChecked(Dialog,IDC_TEXT_WRAP)<>BST_UNCHECKED then
              begin
                ptr^.flags:=ptr^.flags or EF_WRAP;
                idshow:=IDC_TEXT_EDIT_W;
                idhide:=IDC_TEXT_EDIT_NW;
              end
              else
              begin
                ptr^.flags:=ptr^.flags and not EF_WRAP;
                idshow:=IDC_TEXT_EDIT_NW;
                idhide:=IDC_TEXT_EDIT_W;
              end;
              wnd :=GetDlgItem(Dialog,idhide);
              wnd1:=GetDlgItem(Dialog,idshow);
              pc:=GetDlgText(wnd);
              SetDlgItemTextW(Dialog,idshow,pc);
              mFreeMem(pc);
              cr:=hiword(SendMessage(wnd,EM_GETSEL,0,0));
              SendMessage(wnd1,EM_SETSEL,-1,cr);
              ShowWindow(wnd1,SW_SHOW);
              ShowWindow(wnd ,SW_HIDE);

//              SetWindowLongPtr(wnd,GWL_STYLE,GetWindowLongPtr(wnd,GWL_STYLE) xor WS_HSCROLL);
//              SetWindowLongPtr(wnd,GWL_STYLE,GetWindowLongPtr(wnd,GWL_STYLE) xor ES_AUTOHSCROLL);

            end;

            IDC_SCRIPT_HELP: begin
              FillChar(vhi,SizeOf(vhi),0);
              with vhi do
              begin
                cbSize:=SizeOf(vhi);
                flags :=VHF_FULLDLG or VHF_SETLASTSUBJECT;
                if (ptr^.flags and EF_WRAP)<>0 then
                  hwndCtrl:=GetDlgItem(Dialog,IDC_TEXT_EDIT_W)
                else
                  hwndCtrl:=GetDlgItem(Dialog,IDC_TEXT_EDIT_NW);
              end;
              CallService(MS_VARS_SHOWHELPEX,Dialog,tlparam(@vhi));
            end;

            IDOK: begin
              ptr^.flags:=ptr^.flags and not EF_ALL; // /clear changing flags
              if IsDlgButtonChecked(Dialog,IDC_TEXT_SCRIPT)<>BST_UNCHECKED then
                ptr^.flags:=ptr^.flags or EF_SCRIPT;
              SetButtonTitle(btnwnd);
              //??
              if (ptr^.flags and EF_WRAP)<>0 then
                pc:=GetDlgText(Dialog,IDC_TEXT_EDIT_W)
              else
                pc:=GetDlgText(Dialog,IDC_TEXT_EDIT_NW);
              SendMessageW(ptr^.LinkedControl,WM_SETTEXT,0,tlParam(pc));
              mFreeMem(pc);

              // !! cheat
              SendMessage(GetParent(ptr^.LinkedControl),WM_COMMAND,
                  (EN_CHANGE shl 16)+ptr^.LinkedCtrlId,ptr^.LinkedControl);

              EndDialog(Dialog,1);
            end;

            IDCANCEL: begin // clear result / restore old value
              EndDialog(Dialog,0);
            end;
          end;
        end;
      end;
    end;
{
    WM_NOTIFY: begin
      case integer(PNMHdr(lParam)^.code) of
        PSN_APPLY: begin
        end;
      end;
    end;
}
  end;
end;

//----- Edit button processing -----

function EditControlProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  oldproc:pointer;
  ptr:pUserData;
begin
  ptr:=pUserData(GetWindowLongPtrW(Dialog,GWLP_USERDATA));
  oldproc:=ptr^.SavedProc;

  case hMessage of
    WM_DESTROY: begin
      SetWindowLongPtrW(Dialog,GWLP_WNDPROC,long_ptr(oldproc));
      mFreeMem(ptr);
    end;

    WM_INITDIALOG: begin
    end;

    WM_LBUTTONDOWN: begin
      DialogBoxParamW(hInstance,'IDD_EDITCONTROL',GetParent(Dialog),@EditWndProc,Dialog);
      result:=0;
      exit;
    end;
  end;

  result:=CallWindowProc(oldproc,Dialog,hMessage,wParam,lParam)
end;

function MakeEditField(Dialog:HWND; id:uint):HWND;
var
  rc,rcp:TRECT;
  ctrl:HWND;
  pu:pUserData;
begin
  ctrl:=GetDlgItem(Dialog,id);
  GetWindowRect(ctrl,rc ); // screen coords
  GetWindowRect(Dialog ,rcp); // screen coords of parent

  result:=CreateWindowW('BUTTON',nil,WS_CHILD+WS_VISIBLE+BS_PUSHBUTTON+BS_CENTER+BS_VCENTER,
          rc.left-rcp.left, rc.top-rcp.top+(rc.bottom-rc.top-16) div 2, 16,16,
          Dialog,0,hInstance,nil);
  if result<>0 then
  begin
    SetWindowLongPtrW(ctrl,GWLP_USERDATA,long_ptr(result));
    mGetMem(pu,SizeOf(tUserData));
    pu^.SavedProc    :=pointer(SetWindowLongPtrW(result,GWL_WNDPROC,long_ptr(@EditControlProc)));
    pu^.LinkedControl:=ctrl;
    pu^.LinkedCtrlId :=id;
    pu^.flags        :=0;
    SetWindowLongPtrW(result,GWLP_USERDATA,long_ptr(pu));
    inc(rc.left,20);
    MoveWindow(ctrl,
      rc.left-rcp.left, rc.top-rcp.top, rc.right-rc.left, rc.bottom-rc.top,
      false);
    SetButtonTitle(result);
  end;
end;

procedure SetEditFlags(wnd:HWND; mask, flags:dword);
var
  btnwnd:HWND;
  pu:pUserData;
begin
  if mask<>0 then
  begin
    btnwnd:=GetWindowLongPtrW(wnd,GWLP_USERDATA);
    if btnwnd<>0 then
    begin
      pu:=pUserData(GetWindowLongPtrW(btnwnd,GWLP_USERDATA));
      pu^.flags:=pu^.flags and not mask;
      pu^.flags:=pu^.flags or flags;
      if      (pu^.flags and EF_FORCES)<>0 then pu^.flags:=pu^.flags or EF_SCRIPT
      else if (pu^.flags and EF_FORCET)<>0 then pu^.flags:=pu^.flags and not EF_SCRIPT;
      SetButtonTitle(btnwnd);
    end;
  end;
end;
procedure SetEditFlags(Dialog:HWND; id:uint; mask, flags:dword);
begin
  SetEditFlags(GetDlgItem(Dialog,id),mask,flags);
end;

function GetEditFlags(wnd:HWND):dword;
var
  btnwnd:HWND;
  pu:pUserData;
begin
  btnwnd:=GetWindowLongPtrW(wnd,GWLP_USERDATA);
  if btnwnd<>0 then
  begin
    pu:=pUserData(GetWindowLongPtrW(btnwnd,GWLP_USERDATA));
    result:=pu^.flags and EF_ALL;
  end
  else
    result:=0;
end;
function GetEditFlags(Dialog:HWND; id:uint):dword;
begin
  result:=GetEditFlags(GetDlgItem(Dialog,id));
end;

function EnableEditField(wnd:HWND; enable:boolean):boolean;
var
  btnwnd:HWND;
begin
  btnwnd:=GetWindowLongPtrW(wnd,GWLP_USERDATA);
  if btnwnd<>0 then
    EnableWindow(btnwnd,enable);
  result:=EnableWindow(wnd,enable);
end;
function EnableEditField(Dialog:HWND; id:uint; enable:boolean):boolean;
begin
  result:=EnableEditField(GetDlgItem(Dialog,id),enable);
end;

function ShowEditField(wnd:HWND; mode:integer):boolean;
var
  btnwnd:HWND;
begin
  btnwnd:=GetWindowLongPtrW(wnd,GWLP_USERDATA);
  if btnwnd<>0 then
    ShowWindow(btnwnd,mode);
  result:=ShowWindow(wnd,mode);
end;
function ShowEditField(Dialog:HWND; id:uint; mode:integer):boolean;
begin
  result:=ShowEditField(GetDlgItem(Dialog,id),mode);
end;

//----- Separate Edit window -----

type
  pResultText = ^tResultText;
  tResultText = record
    text:PWideChar;
    typ :integer;
  end;
  pSepDlgParam = ^tSepDlgParam;
  tSepDlgParam = record
    title:PWideChar;
    text :PWideChar;
  end;

function EditWndProcSep(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  pc:PWideChar;
  wnd,wnd1:HWND;
  vhi:TVARHELPINFO;
  p:pResultText;
  cr:integer;
  idshow,idhide:integer;
  urd:TUTILRESIZEDIALOG;
begin
  result:=0;

  case hMessage of
    WM_DESTROY: begin
    end;

    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);

      SetWindowTextW(Dialog,pSepDlgParam(lParam)^.title);

      idshow:=IDC_TEXT_EDIT_NW;
      idhide:=IDC_TEXT_EDIT_W;
      SetDlgItemTextW(Dialog,idshow,pSepDlgParam(lParam)^.text);
      ShowWindow(GetDlgItem(Dialog,idshow),SW_SHOW);
      ShowWindow(GetDlgItem(Dialog,idhide),SW_HIDE);

      wnd:=GetDlgItem(Dialog,IDC_SCRIPT_HELP);
      if ServiceExists(MS_VARS_FORMATSTRING)<>0 then
      begin
        SendMessage(wnd,BM_SETIMAGE,IMAGE_ICON,
            CallService(MS_VARS_GETSKINITEM,0,VSI_HELPICON));
        SendMessage(wnd,BUTTONADDTOOLTIP,
            CallService(MS_VARS_GETSKINITEM,0,VSI_HELPTIPTEXT),0);
{
        SendMessage(wnd,BM_SETIMAGE,IMAGE_ICON,
            CallService(MS_SKIN_LOADICON,SKINICON_OTHER_HELP,0));
}
      end
      else
      begin
        ShowWindow(wnd,SW_HIDE);
      end;
    end;

    WM_GETMINMAXINFO: begin
      with PMINMAXINFO(lParam)^ do
      begin
        ptMinTrackSize.x:=200;
        ptMinTrackSize.y:=180;
      end;
    end;

    WM_SIZE: begin
      FillChar(urd,SizeOf(TUTILRESIZEDIALOG),0);
      urd.cbSize    :=SizeOf(urd);
      urd.hwndDlg   :=Dialog;
      urd.hInstance :=hInstance;
      urd.lpTemplate:='IDD_EDITCONTROL';
      urd.lParam    :=0;
      urd.pfnResizer:=@EditDlgResizer;
      CallService(MS_UTILS_RESIZEDIALOG,0,tlparam(@urd));
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        BN_CLICKED: begin
          case loword(wParam) of
            IDC_TEXT_WRAP: begin
              if IsDlgButtonChecked(Dialog,IDC_TEXT_WRAP)<>BST_UNCHECKED then
              begin
                idshow:=IDC_TEXT_EDIT_W;
                idhide:=IDC_TEXT_EDIT_NW;
              end
              else
              begin
                idshow:=IDC_TEXT_EDIT_NW;
                idhide:=IDC_TEXT_EDIT_W;
              end;
              wnd :=GetDlgItem(Dialog,idhide);
              wnd1:=GetDlgItem(Dialog,idshow);
              pc:=GetDlgText(wnd);
              SetDlgItemTextW(Dialog,idshow,pc);
              mFreeMem(pc);
              cr:=hiword(SendMessage(wnd,EM_GETSEL,0,0));
              SendMessage(wnd1,EM_SETSEL,-1,cr);
              ShowWindow(wnd1,SW_SHOW);
              ShowWindow(wnd ,SW_HIDE);

//              SetWindowLongPtr(wnd,GWL_STYLE,GetWindowLongPtr(wnd,GWL_STYLE) xor WS_HSCROLL);
//              SetWindowLongPtr(wnd,GWL_STYLE,GetWindowLongPtr(wnd,GWL_STYLE) xor ES_AUTOHSCROLL);

            end;

            IDC_SCRIPT_HELP: begin
              FillChar(vhi,SizeOf(vhi),0);
              with vhi do
              begin
                cbSize:=SizeOf(vhi);
                flags :=VHF_FULLDLG or VHF_SETLASTSUBJECT;
                if IsDlgButtonChecked(Dialog,IDC_TEXT_WRAP)<>BST_UNCHECKED then
                  hwndCtrl:=GetDlgItem(Dialog,IDC_TEXT_EDIT_W)
                else
                  hwndCtrl:=GetDlgItem(Dialog,IDC_TEXT_EDIT_NW);
              end;
              CallService(MS_VARS_SHOWHELPEX,Dialog,tlparam(@vhi));
            end;

            IDOK: begin
              mGetMem(p,SizeOf(tResultText));

              if IsDlgButtonChecked(Dialog,IDC_TEXT_SCRIPT)<>BST_UNCHECKED then
                p^.typ:=1
              else
                p^.typ:=0;

              if IsDlgButtonChecked(Dialog,IDC_TEXT_WRAP)<>BST_UNCHECKED then
                p^.text:=GetDlgText(Dialog,IDC_TEXT_EDIT_W)
              else
                p^.text:=GetDlgText(Dialog,IDC_TEXT_EDIT_NW);

              EndDialog(Dialog,uint_ptr(p));
            end;

            IDCANCEL: begin // clear result / restore old value
              EndDialog(Dialog,0);
            end;
          end;
        end;
      end;
    end;
{
    WM_NOTIFY: begin
      case integer(PNMHdr(lParam)^.code) of
        PSN_APPLY: begin
        end;
      end;
    end;
}
  end;
end;

function ShowEditBox(parent:HWND;var text:PWideChar;title:PWideChar):int_ptr;
var
  tmp:pResultText;
  par:tSepDlgParam;
begin
  par.title:=title;
  par.text :=text;
  result:=DialogBoxParamW(hInstance,'IDD_EDITCONTROL',parent,@EditWndProcSep,tlparam(@par));
  if result<>0 then
  begin
    tmp:=pResultText(result);
    text  :=tmp^.text;
    result:=tmp^.typ;
    mFreeMem(tmp);
  end
  else
    result:=-1;
end;

end.

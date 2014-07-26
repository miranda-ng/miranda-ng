{
  Parameter: CBN_EDITCHANGE on fields changing, BN_CLICKED on Struct changes
  Result   : CBN_EDITCHANGE on type   changing, BN_CLICKED on option changes
}
unit sparam;

interface

uses windows, m_api;

const
  // parameter flags
  ACF_NUMBER   = $00000001; // Param is number
  ACF_UNICODE  = $00000002; // Param is Unicode string
  ACF_CURRENT  = $00000004; // Param is ignored, used current user handle
                            // from current message window
  ACF_RESULT   = $00000008; // Param is previous action result
  ACF_PARAM    = $00000010; // Param is Call parameter
  ACF_STRUCT   = $00000020;
  ACF_PARTYPE  = ACF_NUMBER  or ACF_UNICODE or
                 ACF_CURRENT or ACF_RESULT  or
                 ACF_PARAM   or ACF_STRUCT;
  ACF_SIGNED       = $00002000; // for future
  ACF_TEMPLATE     = $00000800;
  ACF_SCRIPT_PARAM = $00001000;
  // dummy
  ACF_STRING  = 0;
  ACF_RNUMBER = 0;

  // result flags
  ACF_RSTRING  = $00010000; // Service result is string
  ACF_RUNICODE = $00020000; // Service result is Widestring
  ACF_RSTRUCT  = $00040000; // Service result in structure
  ACF_RFREEMEM = $00080000; // Need to free memory
  ACF_RHEXNUM  = $00100000; // Show number as hex
  ACF_RSIGNED  = $00200000; // Show number as signed

  ACF_RTYPE    = ACF_RSTRING or ACF_RUNICODE or
                 ACF_RSTRUCT or ACF_RFREEMEM or
                 ACF_RHEXNUM or ACF_RSIGNED;

  // parameter / result block creation flags
  ACF_NOSTATIC = $01000000; // No label text in block
  ACF_NOBORDER = $02000000; // No group border around block
  ACF_NOSTRUCT = $04000000; // don't add structure as param type
  ACF_NOVISUAL = $08000000; // don't show number view styles

function CreateParamBlock(parent:HWND;x,y,width:integer;flags:dword=0):THANDLE;
function ClearParamFields(Dialog:HWND):HWND;
function FillParam       (Dialog:HWND;txt:pAnsiChar):integer;
function SetParamValue   (Dialog:HWND;    flags:dword;    value:pointer):boolean;
function GetParamValue   (Dialog:HWND;var flags:dword;var value:pointer):boolean;
function SetParamLabel   (Dialog:HWND; lbl:pWideChar):HWND;

procedure ClearParam    (flags:dword; var param);
function  DuplicateParam(flags:dword; var sparam,dparam):dword;
function  TranslateParam(param:uint_ptr;flags:dword;hContact:TMCONTACT):uint_ptr;

function CreateResultBlock(parent:HWND;x,y,width:integer;flags:dword=0):THANDLE;
function ClearResultFields(Dialog:HWND):HWND;
function SetResultValue(Dialog:HWND;flags:dword):integer;
function GetResultValue(Dialog:HWND):dword;

implementation

uses
  messages,
  common, editwrapper, wrapper, syswin,
  sedit, strans,
  mirutils;

const
  IDC_FLAG_PAR  = 2150;
  IDC_EDIT_PAR  = 2151;
  IDC_STRUCT    = 2152;
  IDC_LABEL_PAR = 2153;
  IDC_GROUP_PAR = 2154;

  IDC_RES_TYPE    = 2160;
  IDC_RES_FREEMEM = 2161;
  IDC_RES_SIGNED  = 2162;
  IDC_RES_HEXNUM  = 2163;


procedure InsertString(wnd:HWND;num:dword;str:PAnsiChar);
var
  buf:array [0..127] of WideChar;
begin
  SendMessageW(wnd,CB_SETITEMDATA,
      SendMessageW(wnd,CB_ADDSTRING,0,
          lparam(TranslateW(FastAnsiToWideBuf(str,buf)))),
      num);
end;


//----- Dialog functions -----

procedure MakeParamTypeList(wnd:HWND; flags:dword);
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);
  InsertString(wnd,ACF_NUMBER ,'number value');
  InsertString(wnd,ACF_STRING ,'ANSI string');
  InsertString(wnd,ACF_UNICODE,'Unicode string');
  InsertString(wnd,ACF_CURRENT,'current contact');
  InsertString(wnd,ACF_RESULT ,'last result');
  InsertString(wnd,ACF_PARAM  ,'parameter');
  if (flags and ACF_NOSTRUCT)=0 then
    InsertString(wnd,ACF_STRUCT ,'structure');
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

function IsParamNumber(txt:pAnsiChar):boolean;
begin
  if (txt[0] in ['0'..'9']) or ((txt[0]='-') and (txt[1] in ['0'..'9'])) or
    ((txt[0]='$') and (txt[1] in sHexNum)) or
    ((txt[0]='0') and (txt[1]='x') and (txt[2] in sHexNum)) then
    result:=true
  else
    result:=false;
end;

// Set parameter type by parameter template
function FixParam(buf:PAnsiChar):integer;
begin
  if      StrCmp(buf,'hContact'    )=0 then result:=ACF_CURRENT
  else if StrCmp(buf,'parameter'   )=0 then result:=ACF_PARAM
  else if StrCmp(buf,'result'      )=0 then result:=ACF_RESULT
  else if StrCmp(buf,'structure'   )=0 then result:=ACF_STRUCT
  else if StrCmp(buf,'Unicode text')=0 then result:=ACF_UNICODE
  else                                      result:=ACF_STRING;
end;

function FixParamControls(Dialog:HWND;atype:dword):dword;
var
  wnd,wnd1:HWND;
  pcw:pWideChar;
begin
  result:=atype;

  wnd :=GetDlgItem(Dialog,IDC_EDIT_PAR);
  wnd1:=GetDlgItem(Dialog,IDC_STRUCT);

  if atype=ACF_STRUCT then
  begin
    ShowEditField(wnd ,SW_HIDE);
    ShowWindow   (wnd1,SW_SHOW);
  end
  else
  begin
    ShowEditField(wnd ,SW_SHOW);
    ShowWindow   (wnd1,SW_HIDE);

    if atype in [ACF_CURRENT,ACF_RESULT,ACF_PARAM] then
      EnableEditField(wnd,false)
    else
    begin
      if atype=ACF_NUMBER then //!!
      begin
        if SendMessageW(wnd,WM_GETTEXTLENGTH,0,0)=0 then
        begin
          pcw:='0';
          SendMessageW(wnd,WM_SETTEXT,0,TLParam(pcw));
        end;
      end;
      EnableEditField(wnd,true);
    end;

  end;
end;

// get line from template
function GetParamLine(src:pAnsiChar;dst:pWideChar;var ltype:integer):pAnsiChar;
var
  pp,pc:pAnsiChar;
  j:integer;
  savechar:AnsiChar;
begin
  pc:=StrScan(src,'|');

  if pc<>nil then
  begin
    savechar:=pc^;
    pc^:=#0;
  end;

  if IsParamNumber(src) then
  begin
    j:=0;
    pp:=src;
    repeat
      dst[j]:=WideChar(pp^);
      inc(j); inc(pp);
    until (pp^=#0) or (pp^=' ');
    dst[j]:=WideChar(pp^); // anyway, #0 or " " needs
    if pp^<>#0 then
    begin
      dst[j+1]:='-'; dst[j+2]:=' '; inc(j,3);
      FastAnsitoWideBuf(pp+1,dst+j);
      StrCopyW(dst+j,TranslateW(dst+j));
    end;
    ltype:=ACF_NUMBER;
  end
  else
  begin
    ltype:=FixParam(src);
    StrCopyW(dst,TranslateW(FastAnsitoWideBuf(src,dst)));
  end;

  if pc<>nil then
  begin
    pc^:=savechar;
    inc(pc);
  end;

  result:=pc;
end;

// Set parameter value by parameter template
function FillParam(Dialog:HWND;txt:pAnsiChar):integer;
var
  bufw:array [0..2047] of WideChar;
  wnd:HWND;
  p,pc:PAnsiChar;
  ltype:integer;
begin
  wnd:=GetDlgItem(Dialog,IDC_EDIT_PAR);
  SendMessage(wnd,CB_RESETCONTENT,0,0);
  if (txt<>nil) and (txt^<>#0) then
  begin
    result:=-1;
    p:=txt;
    repeat
      pc:=GetParamLine(p,bufw,ltype);
      if result<0 then
        result:=ltype;
      SendMessageW(wnd,CB_ADDSTRING,0,lparam(@bufw));

      if result=ACF_STRUCT then
        break
      else
        p:=pc;
    until pc=nil;
  end
  else
    result:=ACF_NUMBER;
  SendMessage(wnd,CB_SETCURSEL,0,0);

  CB_SelectData(GetDlgItem(Dialog,IDC_FLAG_PAR),result);
  FixParamControls(Dialog,result);
end;

function ClearParamFields(Dialog:HWND):HWND;
var
  wnd:HWND;
begin
  wnd:=GetDlgItem(Dialog,IDC_EDIT_PAR);
  SendMessage    (wnd,CB_RESETCONTENT,0,0);
  SetEditFlags   (wnd,EF_ALL,0);
  CB_SelectData(Dialog,IDC_FLAG_PAR,ACF_NUMBER);
  FixParamControls(Dialog,ACF_NUMBER);
  result:=Dialog;
end;

function DlgParamProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  wnd,wnd1:HWND;
  proc:pointer;
  pcw:pWideChar;
  pc,pc1:pAnsiChar;
  i:integer;
begin
  result:=0;

  case hMessage of
    WM_DESTROY: begin
      pc:=pAnsiChar(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_STRUCT),GWLP_USERDATA));
      mFreeMem(pc);
    end;

    WM_SHOWWINDOW: begin
      // hide window by ShowWindow function
      if (lParam=0) and (wParam=0) then
      begin
        pc:=pAnsiChar(SetWindowLongPtrW(GetDlgItem(Dialog,IDC_STRUCT),GWLP_USERDATA,0));
        mFreeMem(pc);
      end;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        CBN_EDITCHANGE,
        EN_CHANGE: begin
          SendMessage(GetParent(Dialog),WM_COMMAND,CBN_EDITCHANGE shl 16,Dialog);
        end;

        CBN_SELCHANGE:  begin
          SendMessage(GetParent(Dialog),WM_COMMAND,CBN_EDITCHANGE shl 16,Dialog);
          case loword(wParam) of
            IDC_FLAG_PAR: begin
              wnd :=GetDlgItem(Dialog,IDC_EDIT_PAR);
              wnd1:=GetDlgItem(Dialog,IDC_STRUCT);

              i:=CB_GetData(GetDlgItem(Dialog,loword(wParam)));

              if i=ACF_STRUCT then
              begin
                ShowEditField(wnd ,SW_HIDE);
                ShowWindow   (wnd1,SW_SHOW);
              end
              else
              begin
                ShowEditField(wnd ,SW_SHOW);
                ShowWindow   (wnd1,SW_HIDE);

                if i in [ACF_CURRENT,ACF_RESULT,ACF_PARAM] then
                  EnableEditField(wnd,false)
                else
                begin
                  if i=ACF_NUMBER then
                  begin
                    pcw:='0';
                    SendMessageW(wnd,WM_SETTEXT,0,TLParam(pcw));
                  end;
                  EnableEditField(wnd,true);
                end;

              end;
            end;
          end;
        end;

        BN_CLICKED: begin
          case loword(wParam) of
            IDC_STRUCT: begin
              pc:=pAnsiChar(GetWindowLongPtrW(lParam,GWLP_USERDATA));
//!!!!
              pc1:=EditStructure(pc{,Dialog});
              if pc1<>nil then
              begin
                mFreeMem(pc);
                SetWindowLongPtrW(lParam,GWLP_USERDATA,long_ptr(pc1));
                SendMessage(GetParent(Dialog),WM_COMMAND,BN_CLICKED shl 16,Dialog);
              end;
            end;
          else
            // can be just edit field wrapper button, no need to react
          end;
        end;

      end;
    end;
  else
    proc:=pointer(GetWindowLongPtrW(Dialog,GWLP_USERDATA));
    result:=CallWindowProc(proc,Dialog,hMessage,wParam,lParam)
  end;
end;

//----- Common interface functions -----

function CreateParamBlock(parent:HWND;x,y,width:integer;flags:dword=0):THANDLE;
var
  hf:HFONT;
  group,ctrl:HWND;
  proc:pointer;
  rc:TRECT;
  fullline:bool;
  gx,dx,dy,xo,yo:integer;
  ux,uy:integer;
begin
  hf:=SendMessageW(parent,WM_GETFONT,0,0);
  GetUnitSize(parent,ux,uy);

  // block
  SetRect(rc,x,y,x+width,y+31);
  dx:=rc.right-rc.left;
  dy:=rc.bottom-rc.top;

  result:=CreateWindowExW(WS_EX_CONTROLPARENT,'STATIC',nil,WS_CHILD+WS_VISIBLE,
          x,y,dx,dy, parent,0,hInstance,nil);
  proc:=pointer(SetWindowLongPtrW(result,GWLP_WNDPROC,long_ptr(@DlgParamProc)));
  SetWindowLongPtrW(result,GWLP_USERDATA,long_ptr(proc));

  yo:=0;

  // group border
  if (flags and ACF_NOBORDER)=0 then
  begin
    group:=CreateWindowW('BUTTON','Param',WS_CHILD+WS_VISIBLE+WS_GROUP+BS_GROUPBOX,
          0,0,dx,dy, result,IDC_GROUP_PAR,hInstance,nil);
    SendMessageW(group,WM_SETFONT,hf,0);
    gx:=4;
    inc(yo,12);
  end
  else
  begin
    gx:=0;
  end;

  // label
  if (flags and ACF_NOSTATIC)=0 then
  begin

    if width<=150 then
    begin
      fullline:=true;
      rc.bottom:=11*uy div 8;
      xo:=dx-gx*2;
    end
    else
    begin
      fullline:=false;
      rc.bottom:=14*uy div 8; // same as param type combobox
      xo:=dx div 3;
    end;
    ctrl:=CreateWindowW('STATIC','Param type',WS_CHILD+WS_VISIBLE+SS_RIGHT+SS_CENTERIMAGE,
          gx,yo,xo-gx,rc.bottom, result,IDC_LABEL_PAR,hInstance,nil);
    SendMessageW(ctrl,WM_SETFONT,hf,0);
  end
  else
  begin
    fullline:=true;
  end;

  // param type
  rc.bottom:=14*uy div 8;
  if fullline then
  begin
    xo:=gx;
    if (flags and ACF_NOSTATIC)=0 then
      inc(yo,rc.bottom);
  end;
  ctrl:=CreateWindowW('COMBOBOX',nil,WS_CHILD+WS_VISIBLE+WS_VSCROLL+CBS_DROPDOWNLIST+CBS_AUTOHSCROLL,
        xo+2,yo,dx-xo-gx-2,56, result,IDC_FLAG_PAR,hInstance,nil);

  SendMessageW(ctrl,WM_SETFONT,hf,0);
  MakeParamTypeList(ctrl,flags);
  inc(yo,rc.bottom+2);

  // param value
  rc.bottom:=14*uy div 8;

  ctrl:=CreateWindowW('COMBOBOX',nil,WS_CHILD+WS_VISIBLE+WS_VSCROLL+CBS_DROPDOWN+CBS_AUTOHSCROLL,
        gx,yo,dx-gx*2,76, result,IDC_EDIT_PAR,hInstance,nil);
  SendMessageW(ctrl,WM_SETFONT,hf,0);
  MakeEditField(result,IDC_EDIT_PAR);

  ctrl:=CreateWindowW('BUTTON','Structure',WS_CHILD+WS_VISIBLE+BS_PUSHBUTTON,
        gx,yo,dx-gx*2,rc.bottom, result,IDC_STRUCT,hInstance,nil);
  SendMessageW(ctrl,WM_SETFONT,hf,0);
  inc(yo,rc.bottom+4);

  // resize group and dialog
  MoveWindow(result,x,y,dx,yo,false);
  if (flags and ACF_NOBORDER)=0 then
    MoveWindow(group,0,0,dx,yo,false);

  ClearParamFields(result);
end;

// if separate
function DestroyBlock(block:pointer):integer;
begin
  result:=0;
end;

function SetParamLabel(Dialog:HWND; lbl:pWideChar):HWND;
var
  wnd:HWND;
begin
  result:=Dialog;

  if Dialog=0 then
    exit;

  wnd:=GetDlgItem(Dialog,IDC_LABEL_PAR);
  if wnd<>0 then
    SendMessageW(wnd,WM_SETTEXT,0,LPARAM(lbl))
  else
  begin
    wnd:=GetDlgItem(Dialog,IDC_GROUP_PAR);
    if wnd<>0 then
      SendMessageW(wnd,WM_SETTEXT,0,LPARAM(lbl))
  end;
end;

function SetParamValue(Dialog:HWND;flags:dword;value:pointer):boolean;
var
  wnd,wnd1:HWND;
  pc:pAnsiChar;
  pcw:pWideChar;
  vtype:integer;
begin
  if Dialog=0 then
  begin
    result:=false;
    exit;
  end;

  result:=true;

  wnd:=GetDlgItem(Dialog,IDC_EDIT_PAR);
  if (flags and ACF_TEMPLATE)<>0 then
  begin
    vtype:=FillParam(Dialog,value);
  end
  else if (flags and ACF_PARAM)<>0 then
  begin
    SendMessageW(wnd,WM_SETTEXT,0,LPARAM(TranslateW('Parameter')));
    EnableWindow(wnd,false);
    vtype:=ACF_PARAM;
  end
  else if (flags and ACF_RESULT)<>0 then
  begin
    SendMessageW(wnd,WM_SETTEXT,0,LPARAM(TranslateW('Last result')));
    EnableWindow(wnd,false);
    vtype:=ACF_RESULT;
  end
  else if (flags and ACF_CURRENT)<>0 then
  begin
    SendMessageW(wnd,WM_SETTEXT,0,LPARAM(TranslateW('Current user')));
    EnableWindow(wnd,false);
    vtype:=ACF_CURRENT;
  end
  else if (flags and ACF_STRUCT)<>0 then
  begin
    vtype:=ACF_STRUCT;

    ShowEditField(wnd,SW_HIDE);
    wnd1:=GetDlgItem(Dialog,IDC_STRUCT);
    ShowWindow(wnd1,SW_SHOW);
    // delete old value
    pc:=pAnsiChar(GetWindowLongPtrW(wnd1,GWLP_USERDATA));
    mFreeMem(pc);
    // set newly allocated
    SetWindowLongPtrW(wnd1,GWLP_USERDATA,long_ptr(StrDup(pc,pAnsiChar(value))));
//!!!!!!!!
  end
  else if (flags and ACF_NUMBER)<>0 then
  begin
    vtype:=ACF_NUMBER;
    if value=nil then
      pcw:='0'
    else
      pcw:=value;
    SendMessageW(wnd,WM_SETTEXT,0,LPARAM(pcw));
  end
  else if (flags and ACF_UNICODE)<>0 then
  begin
    vtype:=ACF_UNICODE;
    SendMessageW(wnd,WM_SETTEXT,0,LPARAM(value));
  end
  else
  begin
    vtype:=ACF_STRING;
    SendMessageW(wnd,WM_SETTEXT,0,LPARAM(value));
  end;
  SetEditFlags(wnd,EF_SCRIPT,ord((flags and ACF_SCRIPT_PARAM)<>0));
  
  CB_SelectData(GetDlgItem(Dialog,IDC_FLAG_PAR),vtype);
  FixParamControls(Dialog,vtype);
end;

function GetParamValue(Dialog:HWND;var flags:dword;var value:pointer):boolean;
var
  wnd:HWND;
begin
  if Dialog=0 then
  begin
    result:=false;
    exit;
  end;

  result:=true;
  flags:=0;
  value:=nil;
  wnd:=GetDlgItem(Dialog,IDC_EDIT_PAR);
  case CB_GetData(GetDlgItem(Dialog,IDC_FLAG_PAR)) of
    ACF_PARAM: begin
      flags:=flags or ACF_PARAM
    end;
    ACF_RESULT: begin
      flags:=flags or ACF_RESULT
    end;
    ACF_CURRENT: begin
      flags:=flags or ACF_CURRENT
    end;
    ACF_NUMBER: begin
      flags:=flags or ACF_NUMBER;
      value:=GetDlgText(wnd);
    end;
    ACF_STRUCT: begin
      flags:=flags or ACF_STRUCT;
      StrDup(pAnsiChar(value),
          pAnsiChar(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_STRUCT),GWLP_USERDATA)));
    end;
    ACF_UNICODE: begin
      flags:=flags or ACF_UNICODE;
      value:=GetDlgText(wnd);
    end;
    ACF_STRING: value:=GetDlgText(wnd);
  end;
  if (GetEditFlags(wnd) and EF_SCRIPT)<>0 then
     flags:=flags or ACF_SCRIPT_PARAM;
end;

//----- Additional functions -----

procedure ClearParam(flags:dword; var param);
begin
  if (flags and (ACF_CURRENT or ACF_RESULT or ACF_PARAM))=0 then
    mFreeMem(pointer(param));
end;

function DuplicateParam(flags:dword; var sparam,dparam):dword;
var
  tmpdst:array [0..2047] of WideChar;
  ltype:integer;
begin
  mFreeMem(dparam);

  if (flags and ACF_TEMPLATE)<>0 then
  begin
    flags:=flags and not (ACF_TEMPLATE or ACF_PARTYPE);
    GetParamLine(pAnsiChar(sparam),tmpdst,ltype);
    case ltype of
      ACF_NUMBER: begin
        flags:=flags or ACF_NUMBER;
        StrDupW(pWideChar(dparam),pWideChar(@tmpdst));
      end;
      ACF_STRING: begin
        flags:=flags or ACF_STRING;
        StrDupW(pWideChar(dparam),pWideChar(@tmpdst));
      end;
      ACF_UNICODE: begin
        flags:=flags or ACF_UNICODE;
        StrDupW(pWideChar(dparam),pWideChar(@tmpdst));
      end;
      ACF_STRUCT: begin
        flags:=flags or ACF_STRUCT;
        StrDup(pAnsiChar(dparam),pAnsiChar(sparam)+10); //10=StrLen('structure|')
      end;
      ACF_CURRENT: flags:=flags or ACF_CURRENT;
      ACF_RESULT : flags:=flags or ACF_RESULT;
      ACF_PARAM  : flags:=flags or ACF_PARAM;
    end;
  end
  else if (flags and (ACF_CURRENT or ACF_RESULT or ACF_PARAM))=0 then
  begin
    if (flags and ACF_NUMBER)<>0 then
      StrDupW(pWideChar(dparam),pWideChar(sparam))
    else if (flags and ACF_STRUCT)<>0 then
      StrDup(pAnsiChar(dparam),pAnsiChar(sparam))
    else if (flags and  ACF_UNICODE)<>0 then
      StrDupW(pWideChar(dparam),pWideChar(sparam))
    else
      StrDupW(pWideChar(dparam),pWideChar(sparam));
  end;
  result:=flags;
end;

function TranslateParam(param:uint_ptr;flags:dword;hContact:TMCONTACT):uint_ptr;
var
  tmp1:pWideChar;
begin
  if (flags and ACF_SCRIPT_PARAM)<>0 then
    result:=uint_ptr(ParseVarString(pWideChar(param),hContact));

  tmp1:=pWideChar(result);
  if (flags and ACF_NUMBER)=0 then
  begin
    if (flags and ACF_UNICODE)=0 then
      WideToAnsi(tmp1,pAnsiChar(result),MirandaCP)
    else
      StrDupW(pWideChar(result),tmp1);
  end
  else
    result:=NumToInt(tmp1);

  if (flags and ACF_SCRIPT_PARAM)<>0 then
    mFreeMem(tmp1);
end;

//===== result block =====

function ClearResultFields(Dialog:HWND):HWND;
var
  w:HWND;
begin
  result:=Dialog;

  if Dialog=0 then
    exit;

  CheckDlgButton(Dialog,IDC_RES_FREEMEM,BST_UNCHECKED);
  ShowWindow(GetDlgItem(Dialog,IDC_RES_FREEMEM),SW_HIDE);
  CB_SelectData(Dialog,IDC_RES_TYPE,ACF_RNUMBER);

  w:=GetDlgItem(Dialog,IDC_RES_HEXNUM);
  if w<>0 then
  begin
    ShowWindow(w,SW_SHOW);
    ShowWindow(GetDlgItem(Dialog,IDC_RES_SIGNED),SW_SHOW);
  end;
end;

procedure MakeResultTypeList(wnd:HWND;flags:dword);
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);
  InsertString(wnd,ACF_RNUMBER ,'number value');
  InsertString(wnd,ACF_RSTRING ,'ANSI string');
  InsertString(wnd,ACF_RUNICODE,'Unicode string');
  if (flags and ACF_NOSTRUCT)=0 then
    InsertString(wnd,ACF_RSTRUCT ,'structure');
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

function DlgResultProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  proc:pointer;
  wnd:HWND;
  b:bool;
  i,j:integer;
begin
  result:=0;

  case hMessage of
    WM_COMMAND: begin
      case wParam shr 16 of
        BN_CLICKED: begin
          SendMessage(GetParent(Dialog),WM_COMMAND,BN_CLICKED shl 16,Dialog);
          case loword(wParam) of
            IDC_RES_SIGNED: begin
              if IsDlgButtonChecked(Dialog,IDC_RES_SIGNED)=BST_UNCHECKED then
              begin
                b:=true;
              end
              else
              begin
                b:=false;
              end;
              EnableWindow(GetDlgItem(Dialog,IDC_RES_HEXNUM),b);
            end;
            IDC_RES_HEXNUM: begin
              if IsDlgButtonChecked(Dialog,IDC_RES_HEXNUM)=BST_UNCHECKED then
              begin
                b:=true;
              end
              else
              begin
                b:=false;
              end;
              EnableWindow(GetDlgItem(Dialog,IDC_RES_SIGNED),b);
            end;
          end;
        end;

        CBN_SELCHANGE:  begin
          SendMessage(GetParent(Dialog),WM_COMMAND,CBN_EDITCHANGE shl 16,Dialog);
          case loword(wParam) of
            IDC_RES_TYPE: begin
              case CB_GetData(lParam) of
                ACF_RNUMBER: begin
                  i:=SW_HIDE;
                  j:=SW_SHOW;
                end;
                ACF_RSTRUCT: begin
                  i:=SW_HIDE;
                  j:=SW_HIDE;
                end;
                ACF_RSTRING,ACF_RUNICODE: begin
                  i:=SW_SHOW;
                  j:=SW_HIDE;
                end;
              end;
              ShowWindow(GetDlgItem(Dialog,IDC_RES_FREEMEM),i);
              wnd:=GetDlgItem(Dialog,IDC_RES_HEXNUM);
              if wnd<>0 then
              begin
                ShowWindow(wnd,j);
                ShowWindow(GetDlgItem(Dialog,IDC_RES_SIGNED),j);
              end;
            end;
          end;
        end;
      end;
    end;
  else
    proc:=pointer(GetWindowLongPtrW(Dialog,GWLP_USERDATA));
    result:=CallWindowProc(proc,Dialog,hMessage,wParam,lParam)
  end;
end;

function CreateResultBlock(parent:HWND;x,y,width:integer;flags:dword=0):THANDLE;
var
  hf:HFONT;
  ctrl,group:HWND;
  proc:pointer;
  rc:TRECT;
  fullline:bool;
  dx,dy,yo,gx,xo:integer;
  ux,uy:integer;
begin
  hf:=SendMessageW(parent,WM_GETFONT,0,0);
  GetUnitSize(parent,ux,uy);

  // block body
  SetRect(rc,x,y,x+width,y+53);
  dx:=rc.right-rc.left;
  dy:=rc.bottom-rc.top;

  result:=CreateWindowExW(WS_EX_CONTROLPARENT,'STATIC',nil,WS_CHILD+WS_VISIBLE,
          x,y,dx,dy, parent,0,hInstance,nil);
  proc:=pointer(SetWindowLongPtrW(result,GWLP_WNDPROC,long_ptr(@DlgResultProc)));
  SetWindowLongPtrW(result,GWLP_USERDATA,long_ptr(proc));

  yo:=0;

  // group border
  if (flags and ACF_NOBORDER)=0 then
  begin
    group:=CreateWindowW('BUTTON','Result',WS_CHILD+WS_VISIBLE+WS_GROUP+BS_GROUPBOX,
          0,0,dx,dy, result,0,hInstance,nil);
    SendMessageW(group,WM_SETFONT,hf,0);
    gx:=4;
    inc(yo,12);
  end
  else
  begin
    gx:=0;
  end;

  // label
  if (flags and ACF_NOSTATIC)=0 then
  begin
    if width<=150 then
    begin
      fullline:=true;
      rc.bottom:=11*uy div 8;
      xo:=dx-gx*2;
    end
    else
    begin
      fullline:=false;
      rc.bottom:=14*uy div 8; // same as param type combobox
      xo:=dx div 3;
    end;
    ctrl:=CreateWindowW('STATIC','Result type',WS_CHILD+WS_VISIBLE+SS_RIGHT+SS_CENTERIMAGE,
          gx,yo,xo-gx,rc.bottom, result,IDC_LABEL_PAR,hInstance,nil);
    SendMessageW(ctrl,WM_SETFONT,hf,0);
  end
  else
  begin
    fullline:=true;
  end;

  // result type
  rc.bottom:=14*uy div 8;
  if fullline then
  begin
    xo:=gx;
    if (flags and ACF_NOSTATIC)=0 then
      inc(yo,rc.bottom);
  end;
  ctrl:=CreateWindowW('COMBOBOX',nil,WS_CHILD+WS_VISIBLE+WS_VSCROLL+CBS_DROPDOWNLIST+CBS_AUTOHSCROLL,
        xo+2,yo,dx-xo-gx-2,76, result,IDC_RES_TYPE,hInstance,nil);

  SendMessageW(ctrl,WM_SETFONT,hf,0);
  MakeResultTypeList(ctrl,flags);
  inc(yo,rc.bottom+2);

  // 'free memory' checkbox
  rc.bottom:=11*uy div 8;

  ctrl:=CreateWindowW('BUTTON','Free memory',WS_CHILD+WS_VISIBLE+BS_AUTOCHECKBOX,
        gx,yo,dx-gx*2,rc.bottom, result,IDC_RES_FREEMEM,hInstance,nil);
  SendMessageW(ctrl,WM_SETFONT,hf,0);
  inc(yo,rc.bottom+4);

  if (flags and ACF_NOVISUAL)=0 then
  begin
    dec(yo,rc.bottom+4);

    ctrl:=CreateWindowW('BUTTON','Signed',WS_CHILD+WS_VISIBLE+BS_AUTOCHECKBOX,
          gx,yo,dx-gx*2,rc.bottom, result,IDC_RES_SIGNED,hInstance,nil);
    SendMessageW(ctrl,WM_SETFONT,hf,0);
    inc(yo,rc.bottom+2);

    ctrl:=CreateWindowW('BUTTON','As hex',WS_CHILD+WS_VISIBLE+BS_AUTOCHECKBOX,
          gx,yo,dx-gx*2,rc.bottom, result,IDC_RES_HEXNUM,hInstance,nil);
    SendMessageW(ctrl,WM_SETFONT,hf,0);
    inc(yo,rc.bottom+4);
  end;

  // resize group and dialog
  MoveWindow(result,x,y,dx,yo,false);
  if (flags and ACF_NOBORDER)=0 then
    MoveWindow(group,0,0,dx,yo,false);

  ClearResultFields(result);
end;

function SetResultValue(Dialog:HWND;flags:dword):integer;
var
  w:HWND;
  btn:cardinal;
  sh,sh1:integer;
begin
  if Dialog=0 then
  begin
    result:=ACF_RNUMBER;
    exit;
  end;

  // RESULT
  sh :=SW_HIDE;
  sh1:=SW_HIDE;
  w:=GetDlgItem(Dialog,IDC_RES_HEXNUM);
  if (flags and ACF_RSTRUCT)<>0 then
    result:=ACF_RSTRUCT
  else if (flags and (ACF_RSTRING or ACF_RUNICODE))<>0 then
  begin
    sh:=SW_SHOW;

    if (flags and ACF_RFREEMEM)<>0 then
      btn:=BST_CHECKED
    else
      btn:=BST_UNCHECKED;
    CheckDlgButton(Dialog,IDC_RES_FREEMEM,btn);

    if (flags and ACF_RUNICODE)<>0 then
      result:=ACF_RUNICODE
    else
      result:=ACF_RSTRING;
  end
  else
  begin
    result:=ACF_RNUMBER;
    if w<>0 then
    begin
      sh1:=SW_SHOW;
      if (flags and ACF_RSIGNED)<>0 then
        btn:=BST_CHECKED
      else
        btn:=BST_UNCHECKED;
      CheckDlgButton(Dialog,IDC_RES_SIGNED,btn);
      if (flags and ACF_RHEXNUM)<>0 then
        btn:=BST_CHECKED
      else
        btn:=BST_UNCHECKED;
      CheckDlgButton(Dialog,IDC_RES_HEXNUM,btn);
    end;
  end;
  ShowWindow(GetDlgItem(Dialog,IDC_RES_FREEMEM),sh);
  if w<>0 then
  begin
    ShowWindow(w,sh1);
    ShowWindow(GetDlgItem(Dialog,IDC_RES_SIGNED),sh1);
  end;
  CB_SelectData(Dialog,IDC_RES_TYPE,result);
end;

function GetResultValue(Dialog:HWND):dword;
begin
  if Dialog=0 then
  begin
    result:=ACF_RNUMBER;
    exit;
  end;

  case CB_GetData(GetDlgItem(Dialog,IDC_RES_TYPE)) of
    ACF_RSTRING: begin
      result:=ACF_RSTRING;
      if IsDlgButtonChecked(Dialog,IDC_RES_FREEMEM)=BST_CHECKED then
        result:=result or ACF_RFREEMEM;
    end;
    ACF_RUNICODE: begin
      result:={!!atavizm ACF_RSTRING or }ACF_RUNICODE;
      if IsDlgButtonChecked(Dialog,IDC_RES_FREEMEM)=BST_CHECKED then
        result:=result or ACF_RFREEMEM;
    end;
    ACF_RSTRUCT: result:=ACF_RSTRUCT;
  else
    result:=ACF_RNUMBER;
    if GetDlgItem(Dialog,IDC_RES_HEXNUM)<>0 then
    begin
      if IsDlgButtonChecked(Dialog,IDC_RES_SIGNED)=BST_CHECKED then
        result:=result or ACF_RSIGNED
      else if IsDlgButtonChecked(Dialog,IDC_RES_HEXNUM)=BST_CHECKED then
        result:=result or ACF_RHEXNUM;
    end;
  end;

end;

end.

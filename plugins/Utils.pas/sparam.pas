{
  Parameter: CBN_EDITCHANGE on fields changing, BN_CLICKED on Struct changes
  Result   : CBN_EDITCHANGE on type   changing, BN_CLICKED on option changes
  ACF_FLAG_TEMPLATE saved in CB param of IDC_EDIT_PAR
  parameter type depends of ACF_FLAG_PAR
}
unit sparam;

interface

uses windows, m_api, awkservices;

//----- Parameter dialog -----

function CreateParamBlock(parent:HWND;x,y,width:integer;flags:dword=0):THANDLE;
function ClearParamFields(Dialog:HWND):HWND;
function FillParam       (Dialog:HWND;txt:PAnsiChar):integer;
function SetParamValue   (Dialog:HWND;    flags:dword;    value:pointer):boolean;
function GetParamValue   (Dialog:HWND;var flags:dword;var value:pointer):boolean;
function SetParamLabel   (Dialog:HWND; lbl:PWideChar):HWND;

//----- Parameter value -----

procedure SaveParamValue(    flags:dword;     param:pointer; module,setting:PAnsiChar);
procedure LoadParamValue(var flags:dword; var param:pointer; module,setting:PAnsiChar);
procedure ClearParam    (    flags:dword; var param);
function  DuplicateParam(    flags:dword; var sparam,dparam):dword;
{??
function  TranslateParam(param:uint_ptr;flags:dword;hContact:TMCONTACT):uint_ptr;
}

//----- Execution -----

function  PrepareParameter(flags:dword;const aparam:LPARAM; const data:tSubstData):LPARAM;
procedure ReleaseParameter(flags:dword;var   aparam:LPARAM);

//----- result dialog -----

function CreateResultBlock(parent:HWND;x,y,width:integer;flags:dword=0):THANDLE;
function ClearResultFields(Dialog:HWND):HWND;
function SetResultValue   (Dialog:HWND;flags:dword):integer;
function GetResultValue   (Dialog:HWND):dword;

//----- Old flags converting -----

function ConvertParamFlags (flags:dword):dword;
function ConvertResultFlags(flags:dword):dword;

implementation

uses
  messages,
  common, editwrapper, wrapper, syswin,
  sedit, strans,
  dbsettings,mirutils,mircontacts;

//----- Parameter dialog -----

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
  InsertString(wnd, ACF_TYPE_NUMBER , 'number value');
  InsertString(wnd, ACF_TYPE_STRING , 'ANSI string');
  InsertString(wnd, ACF_TYPE_UNICODE, 'Unicode string');
  if (flags and ACF_BLOCK_NOCURRENT)=0 then
    InsertString(wnd, ACF_TYPE_CURRENT, 'current contact');
  InsertString(wnd, ACF_TYPE_RESULT , 'last result');
  InsertString(wnd, ACF_TYPE_PARAM  , 'parameter');
  if (flags and ACF_BLOCK_NOSTRUCT)=0 then
    InsertString(wnd, ACF_TYPE_STRUCT, 'structure');
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

function IsParamNumber(txt:PAnsiChar):boolean;
begin
  if (txt[0] in ['0'..'9']) or ((txt[0]='-') and (txt[1] in ['0'..'9'])) or
    ((txt[0]='$') and (txt[1] in sHexNum)) or
    ((txt[0]='0') and (txt[1]='x') and (txt[2] in sHexNum)) then
    result:=true
  else
    result:=false;
end;

// Set parameter type by parameter template for non-numbers
function FixParam(buf:PAnsiChar):integer;
begin
  if      StrCmp(buf,'hContact'    )=0 then result:=ACF_TYPE_CURRENT
  else if StrCmp(buf,'parameter'   )=0 then result:=ACF_TYPE_PARAM
  else if StrCmp(buf,'result'      )=0 then result:=ACF_TYPE_RESULT
  else if StrCmp(buf,'structure'   )=0 then result:=ACF_TYPE_STRUCT
  else if StrCmp(buf,'Unicode text')=0 then result:=ACF_TYPE_UNICODE
  else                                      result:=ACF_TYPE_STRING;
end;

function FixParamControls(Dialog:HWND;atype:dword):dword;
var
  wnd,wnd1:HWND;
  pcw:PWideChar;
begin
  result:=atype;

  wnd :=GetDlgItem(Dialog,IDC_EDIT_PAR);
  wnd1:=GetDlgItem(Dialog,IDC_STRUCT);

  if atype=ACF_TYPE_STRUCT then
  begin
    ShowEditField(wnd ,SW_HIDE);
    ShowWindow   (wnd1,SW_SHOW);
  end
  else
  begin
    ShowEditField(wnd ,SW_SHOW);
    ShowWindow   (wnd1,SW_HIDE);

    if atype in [ACF_TYPE_CURRENT,ACF_TYPE_RESULT,ACF_TYPE_PARAM] then
      EnableEditField(wnd,false)
    else
    begin
      if atype=ACF_TYPE_NUMBER then
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
function GetParamLine(src:PAnsiChar;dst:PWideChar;var ltype:dword):PAnsiChar;
var
  pp,pc:PAnsiChar;
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
      FastAnsiToWideBuf(pp+1,dst+j);
      StrCopyW(dst+j,TranslateW(dst+j));
    end;
    ltype:=ACF_TYPE_NUMBER;
  end
  else
  begin
    ltype:=FixParam(src);
    StrCopyW(dst,TranslateW(FastAnsiToWideBuf(src,dst)));
  end;

  if pc<>nil then
  begin
    pc^:=savechar;
    inc(pc);
  end;

  result:=pc;
end;

// Set parameter value by parameter template
function FillParam(Dialog:HWND;txt:PAnsiChar):integer;
var
  bufw:array [0..2047] of WideChar;
  wnd:HWND;
  p,pc:PAnsiChar;
  flags,ltype:dword;
begin
  wnd:=GetDlgItem(Dialog,IDC_EDIT_PAR);
  flags:=CB_GetData(wnd);
  CB_SetData(wnd,flags or ACF_FLAG_TEMPLATE);
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

      if result=ACF_TYPE_STRUCT then
        break
      else
        p:=pc;
    until pc=nil;
  end
  else
    result:=ACF_TYPE_NUMBER;
  SendMessage(wnd,CB_SETCURSEL,0,0);

  CB_SelectData(GetDlgItem(Dialog,IDC_FLAG_PAR),result);
  //!!!! need to set ACF_FLAG_TEMPLATE here
  FixParamControls(Dialog,result);
end;

function ClearParamFields(Dialog:HWND):HWND;
var
  wnd:HWND;
begin
  wnd:=GetDlgItem(Dialog,IDC_EDIT_PAR);
  SendMessage    (wnd,CB_RESETCONTENT,0,0);
  SetEditFlags   (wnd,EF_ALL,0);
  CB_SelectData(Dialog,IDC_FLAG_PAR,ACF_TYPE_NUMBER);
  FixParamControls(Dialog,ACF_TYPE_NUMBER);
  result:=Dialog;
end;

function DlgParamProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  wnd,wnd1:HWND;
  proc:pointer;
  pcw:PWideChar;
  pc,pc1:PAnsiChar;
  i:integer;
begin
  result:=0;

  case hMessage of
    WM_DESTROY: begin
      pc:=PAnsiChar(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_STRUCT),GWLP_USERDATA));
      mFreeMem(pc);
    end;

    WM_SHOWWINDOW: begin
      // hide window by ShowWindow function
      if (lParam=0) and (wParam=0) then
      begin
        pc:=PAnsiChar(SetWindowLongPtrW(GetDlgItem(Dialog,IDC_STRUCT),GWLP_USERDATA,0));
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

              if i=ACF_TYPE_STRUCT then
              begin
                ShowEditField(wnd ,SW_HIDE);
                ShowWindow   (wnd1,SW_SHOW);
              end
              else
              begin
                ShowEditField(wnd ,SW_SHOW);
                ShowWindow   (wnd1,SW_HIDE);

                if i in [ACF_TYPE_CURRENT,ACF_TYPE_RESULT,ACF_TYPE_PARAM] then
                  EnableEditField(wnd,false)
                else
                begin
                  if i=ACF_TYPE_NUMBER then
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
              pc:=PAnsiChar(GetWindowLongPtrW(lParam,GWLP_USERDATA));
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
  if (flags and ACF_BLOCK_NOBORDER)=0 then
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
  if (flags and ACF_BLOCK_NOSTATIC)=0 then
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
    if (flags and ACF_BLOCK_NOSTATIC)=0 then
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
  if (flags and ACF_BLOCK_NOBORDER)=0 then
    MoveWindow(group,0,0,dx,yo,false);

  ClearParamFields(result);
end;

// if separate
function DestroyBlock(block:pointer):integer;
begin
  result:=0;
end;

function SetParamLabel(Dialog:HWND; lbl:PWideChar):HWND;
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
  pc:PAnsiChar;
  pcw:PWideChar;
  vtype:integer;
begin
  if Dialog=0 then
  begin
    result:=false;
    exit;
  end;

  result:=true;

  wnd:=GetDlgItem(Dialog,IDC_EDIT_PAR);
  if (flags and ACF_FLAG_TEMPLATE)<>0 then
  begin
    vtype:=FillParam(Dialog,value);
  end
  else
  begin
    CB_SetData(wnd,flags and not ACF_TYPE_MASK);
    vtype:=flags and ACF_TYPE_MASK;
    case vtype of
      ACF_TYPE_PARAM: begin
        SendMessageW(wnd,WM_SETTEXT,0,LPARAM(TranslateW('Parameter')));
        EnableWindow(wnd,false);
      end;
      ACF_TYPE_RESULT: begin
        SendMessageW(wnd,WM_SETTEXT,0,LPARAM(TranslateW('Last result')));
        EnableWindow(wnd,false);
      end;
      ACF_TYPE_CURRENT: begin
        SendMessageW(wnd,WM_SETTEXT,0,LPARAM(TranslateW('Current user')));
        EnableWindow(wnd,false);
      end;
      ACF_TYPE_STRUCT: begin
        ShowEditField(wnd,SW_HIDE);
        wnd1:=GetDlgItem(Dialog,IDC_STRUCT);
        ShowWindow(wnd1,SW_SHOW);
        // delete old value
        pc:=PAnsiChar(GetWindowLongPtrW(wnd1,GWLP_USERDATA));
        mFreeMem(pc);
        // set newly allocated
        SetWindowLongPtrW(wnd1,GWLP_USERDATA,long_ptr(StrDup(pc,PAnsiChar(value))));
    //!!!!!!!!
      end;
      ACF_TYPE_NUMBER: begin
        if value=nil then
          pcw:='0'
        else
          pcw:=value;
        SendMessageW(wnd,WM_SETTEXT,0,LPARAM(pcw));
      end;
      ACF_TYPE_UNICODE,
      ACF_TYPE_STRING: begin
        SendMessageW(wnd,WM_SETTEXT,0,LPARAM(value));
      end;
    end;
  end;
  SetEditFlags(wnd,EF_SCRIPT,ord((flags and ACF_FLAG_SCRIPT)<>0));
  
  CB_SelectData(GetDlgItem(Dialog,IDC_FLAG_PAR),vtype);
  FixParamControls(Dialog,vtype);
end;

//?? preserve ACF_TEMPLATE flag??
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
  value:=nil;
  wnd:=GetDlgItem(Dialog,IDC_EDIT_PAR);
  flags:=CB_GetData(GetDlgItem(Dialog,IDC_FLAG_PAR));
  case flags of
    ACF_TYPE_UNICODE,
    ACF_TYPE_STRING,
    ACF_TYPE_NUMBER: begin
      value:=GetDlgText(wnd);
    end;
    ACF_TYPE_STRUCT: begin
      StrDup(PAnsiChar(value),
          PAnsiChar(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_STRUCT),GWLP_USERDATA)));
    end;
  end;
  if (GetEditFlags(wnd) and EF_SCRIPT)<>0 then
     flags:=flags or ACF_FLAG_SCRIPT;
  // for example, ACF_FLAG_TEMPLATE
  flags:=flags or CB_GetData(wnd);
end;

//----- Parameter value -----

const
  ioflags:PAnsiChar = 'flags';
  iovalue:PAnsiChar = 'value';

procedure SaveParamValue(flags:dword; param:pointer; module,setting:PAnsiChar);
var
  buf:array [0..127] of AnsiChar;
  p:PAnsiChar;
begin
  p:=StrCopyE(buf,setting); p^:='/'; inc(p);
  StrCopy(p,ioflags); DBWriteDWord(0,module,buf,flags);
  StrCopy(p,iovalue);
  case flags and ACF_TYPE_MASK of
    ACF_TYPE_NUMBER,
    ACF_TYPE_STRING,
    ACF_TYPE_UNICODE: DBWriteUnicode(0,module,buf,param);
    ACF_TYPE_STRUCT : DBWriteString (0,module,buf,param);
  end;
end;

procedure LoadParamValue(var flags:dword; var param:pointer; module,setting:PAnsiChar);
var
  buf:array [0..127] of AnsiChar;
  p:PAnsiChar;
begin
  p:=StrCopyE(buf,setting); p^:='/'; inc(p);
  StrCopy(p,ioflags); flags:=DBReadDWord(0,module,buf);
  StrCopy(p,iovalue);
  case flags and ACF_TYPE_MASK of
    ACF_TYPE_NUMBER,
    ACF_TYPE_STRING,
    ACF_TYPE_UNICODE: param:=DBReadUnicode(0,module,buf);
    ACF_TYPE_STRUCT : param:=DBReadString (0,module,buf);
  end;
end;

procedure ClearParam(flags:dword; var param);
begin
{
  if not ((flags and ACF_TYPE_MASK) in [ACF_TYPE_CURRENT, ACF_TYPE_RESULT, ACF_TYPE_PARAM]) then
    mFreeMem(pointer(param));
}
  case flags of
    ACF_TYPE_NUMBER,
    ACF_TYPE_STRING,
    ACF_TYPE_UNICODE,
    ACF_TYPE_STRUCT : mFreeMem(pointer(param));
  end;
end;

//----- Execution -----

function PrepareParameter(flags:dword;const aparam:LPARAM; const data:tSubstData):LPARAM;
var
  tmp1:PWideChar;
  typ:dword;
begin
  typ:=flags and ACF_TYPE_MASK;
  case typ of
    ACF_TYPE_STRUCT: begin
      result:=uint_ptr(MakeStructure(pAnsiChar(aparam),
          data.Parameter,data.LastResult,data.ResultType=ACF_TYPE_NUMBER))
    end;

    ACF_TYPE_PARAM: begin
      result:=data.Parameter;
    end;

    ACF_TYPE_RESULT: begin
      result:=data.LastResult;
    end;

    ACF_TYPE_CURRENT: begin
      result:=WndToContact(WaitFocusedWndChild(GetForegroundwindow){GetFocus});
    end;

    ACF_TYPE_NUMBER,
    ACF_TYPE_STRING,
    ACF_TYPE_UNICODE: begin
      if (flags and ACF_FLAG_SCRIPT)<>0 then
        tmp1:=ParseVarString(pWideChar(aparam),data.Parameter)
      else
        tmp1:=pWideChar(aparam);

      if typ<>ACF_TYPE_NUMBER then
      begin
        if typ<>ACF_TYPE_UNICODE then
          WideToAnsi(tmp1,pAnsiChar(result),MirandaCP)
        else
          StrDupW(pWideChar(result),tmp1);
      end
      else
        result:=NumToInt(tmp1);

      if (flags and ACF_FLAG_SCRIPT)<>0 then
        mFreeMem(tmp1);
    end;
  else
    result:=0;
  end;
end;

procedure ReleaseParameter(flags:dword;var aparam:LPARAM);
begin
  case flags and ACF_TYPE_MASK of
    ACF_TYPE_STRING,
    ACF_TYPE_UNICODE: mFreeMem(pointer(aparam));
    ACF_TYPE_STRUCT : FreeStructure(aparam);
  end;
end;

//----- Additional functions -----

function DuplicateParam(flags:dword; var sparam,dparam):dword;
var
  tmpdst:array [0..2047] of WideChar;
  ltype:dword;
begin
  mFreeMem(dparam);

  if (flags and ACF_FLAG_TEMPLATE)<>0 then
  begin
    flags:=flags and not (ACF_FLAG_TEMPLATE or ACF_TYPE_MASK);
    GetParamLine(PAnsiChar(sparam),tmpdst,ltype);
    flags:=flags or ltype;
    case ltype of
      ACF_TYPE_NUMBER,
      ACF_TYPE_STRING,
      ACF_TYPE_UNICODE: begin
        StrDupW(PWideChar(dparam),PWideChar(@tmpdst));
      end;
      ACF_TYPE_STRUCT: begin
        StrDup(PAnsiChar(dparam),PAnsiChar(sparam)+10); //10=StrLen('structure|')
      end;
    end;
  end
  else
  begin
    case flags and ACF_TYPE_MASK of
      ACF_TYPE_NUMBER,
      ACF_TYPE_STRING,
      ACF_TYPE_UNICODE: begin
        StrDupW(PWideChar(dparam),PWideChar(sparam))
      end;
      ACF_TYPE_STRUCT: begin
        StrDup(PAnsiChar(dparam),PAnsiChar(sparam))
      end;
    end;
  end;
  result:=flags;
end;

{??
function TranslateParam(param:uint_ptr;flags:dword;hContact:TMCONTACT):uint_ptr;
var
  tmp1:PWideChar;
begin
  if (flags and ACF_SCRIPT_PARAM)<>0 then
    result:=uint_ptr(ParseVarString(PWideChar(param),hContact));

  tmp1:=PWideChar(result);
  case flags and ACF_TYPE_MASK of
    ACF_TYPE_NUMBER: begin
      result:=NumToInt(tmp1);
    end;
    ACF_TYPE_UNICODE: begin
      StrDupW(PWideChar(result),tmp1);
    end;
    ACF_TYPE_STRING,
    ACF_TYPE_STRUCT: begin
      WideToAnsi(tmp1,PAnsiChar(result),MirandaCP)
    end;
  end
  else

  if (flags and ACF_SCRIPT_PARAM)<>0 then
    mFreeMem(tmp1);
end;
}


//----- Result dialog -----

function ClearResultFields(Dialog:HWND):HWND;
var
  w:HWND;
begin
  result:=Dialog;

  if Dialog=0 then
    exit;

  CheckDlgButton(Dialog,IDC_RES_FREEMEM,BST_UNCHECKED);
  ShowWindow(GetDlgItem(Dialog,IDC_RES_FREEMEM),SW_HIDE);
  CB_SelectData(Dialog,IDC_RES_TYPE,ACF_TYPE_NUMBER);

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
  InsertString(wnd, ACF_TYPE_NUMBER , 'number value');
  InsertString(wnd, ACF_TYPE_STRING , 'ANSI string');
  InsertString(wnd, ACF_TYPE_UNICODE, 'Unicode string');
  if (flags and ACF_BLOCK_NOSTRUCT)=0 then
    InsertString(wnd, ACF_TYPE_STRUCT, 'structure');
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
                ACF_TYPE_NUMBER: begin
                  i:=SW_HIDE;
                  j:=SW_SHOW;
                end;
                ACF_TYPE_STRUCT: begin
                  i:=SW_HIDE;
                  j:=SW_HIDE;
                end;
                ACF_TYPE_STRING,
                ACF_TYPE_UNICODE: begin
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
  if (flags and ACF_BLOCK_NOBORDER)=0 then
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
  if (flags and ACF_BLOCK_NOSTATIC)=0 then
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
    if (flags and ACF_BLOCK_NOSTATIC)=0 then
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

  if (flags and ACF_BLOCK_NOVISUAL)=0 then
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
  if (flags and ACF_BLOCK_NOBORDER)=0 then
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
    result:=ACF_TYPE_NUMBER;
    exit;
  end;

  // RESULT
  result:=flags and ACF_TYPE_MASK;
  
  sh :=SW_HIDE;
  sh1:=SW_HIDE;
  w:=GetDlgItem(Dialog,IDC_RES_HEXNUM);

  case result of
    ACF_TYPE_STRING,
    ACF_TYPE_UNICODE: begin
      sh:=SW_SHOW;

      if (flags and ACF_FLAG_FREEMEM)<>0 then
        btn:=BST_CHECKED
      else
        btn:=BST_UNCHECKED;
      CheckDlgButton(Dialog,IDC_RES_FREEMEM,btn);
    end;

    ACF_TYPE_NUMBER: begin
      if w<>0 then
      begin
        sh1:=SW_SHOW;
        if (flags and ACF_FLAG_SIGNED)<>0 then
          btn:=BST_CHECKED
        else
          btn:=BST_UNCHECKED;
        CheckDlgButton(Dialog,IDC_RES_SIGNED,btn);
        if (flags and ACF_FLAG_HEXNUM)<>0 then
          btn:=BST_CHECKED
        else
          btn:=BST_UNCHECKED;
        CheckDlgButton(Dialog,IDC_RES_HEXNUM,btn);
      end;
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
    result:=ACF_TYPE_NUMBER;
    exit;
  end;

  result:=CB_GetData(GetDlgItem(Dialog,IDC_RES_TYPE));
  case result of
    ACF_TYPE_STRING,
    ACF_TYPE_UNICODE: begin
      if IsDlgButtonChecked(Dialog,IDC_RES_FREEMEM)=BST_CHECKED then
        result:=result or ACF_FLAG_FREEMEM;
    end;
    ACF_TYPE_NUMBER: begin
      if GetDlgItem(Dialog,IDC_RES_HEXNUM)<>0 then
      begin
        if IsDlgButtonChecked(Dialog,IDC_RES_SIGNED)=BST_CHECKED then
          result:=result or ACF_FLAG_SIGNED
        else if IsDlgButtonChecked(Dialog,IDC_RES_HEXNUM)=BST_CHECKED then
          result:=result or ACF_FLAG_HEXNUM;
      end;
    end;
  end;

end;

//--------------------------------

const
  // parameter flags
  ACF_OLD_NUMBER   = $00000001;
  ACF_OLD_UNICODE  = $00000002;
  ACF_OLD_CURRENT  = $00000004;

  ACF_OLD_RESULT   = $00000008;
  ACF_OLD_PARAM    = $00000010;
  ACF_OLD_STRUCT   = $00000020;
  ACF_PARTYPE  = ACF_OLD_NUMBER  or ACF_OLD_UNICODE or
                 ACF_OLD_CURRENT or ACF_OLD_RESULT  or
                 ACF_OLD_PARAM   or ACF_OLD_STRUCT;
  ACF_OLD_TEMPLATE     = $00000800;
  ACF_OLD_SCRIPT_PARAM = $00001000;
  // dummy
  ACF_OLD_STRING  = 0;
  ACF_OLD_RNUMBER = 0;

  ACF_OLD_RSTRING  = $00010000;
  ACF_OLD_RUNICODE = $00020000;
  ACF_OLD_RSTRUCT  = $00040000;
  ACF_OLD_RFREEMEM = $00080000;
  ACF_OLD_RHEXNUM  = $00100000;
  ACF_OLD_RSIGNED  = $00200000;

  ACF_RTYPE = ACF_OLD_RSTRING or ACF_OLD_RUNICODE or ACF_OLD_RSTRUCT;

  ACF_OLD_SCRIPT_SERVICE = $00800000;

function ConvertParamFlags(flags:dword):dword;
begin
  case flags and ACF_PARTYPE of
    ACF_OLD_NUMBER : result:=ACF_TYPE_NUMBER;
    ACF_OLD_UNICODE: result:=ACF_TYPE_UNICODE;
    ACF_OLD_CURRENT: result:=ACF_TYPE_CURRENT;
    ACF_OLD_RESULT : result:=ACF_TYPE_RESULT;
    ACF_OLD_PARAM  : result:=ACF_TYPE_PARAM;
    ACF_OLD_STRUCT : result:=ACF_TYPE_STRUCT;
    ACF_OLD_STRING : result:=ACF_TYPE_STRING;
  else
    result:=ACF_TYPE_NUMBER;
  end;
  if (flags and ACF_OLD_TEMPLATE    )<>0 then result:=result or ACF_FLAG_TEMPLATE;
  if (flags and ACF_OLD_SCRIPT_PARAM)<>0 then result:=result or ACF_FLAG_SCRIPT;
end;

function ConvertResultFlags(flags:dword):dword;
begin
  case flags and ACF_RTYPE of
    ACF_OLD_RNUMBER : result:=ACF_TYPE_NUMBER;
    ACF_OLD_RUNICODE: result:=ACF_TYPE_UNICODE;
    ACF_OLD_RSTRUCT : result:=ACF_TYPE_STRUCT;
    ACF_OLD_RSTRING : result:=ACF_TYPE_STRING;
  else
    result:=ACF_TYPE_NUMBER;
  end;
  if (flags and ACF_OLD_RFREEMEM)<>0 then result:=result or ACF_FLAG_FREEMEM;
  if (flags and ACF_OLD_RHEXNUM )<>0 then result:=result or ACF_FLAG_HEXNUM;
  if (flags and ACF_OLD_RSIGNED )<>0 then result:=result or ACF_FLAG_SIGNED;
  if (flags and ACF_OLD_SCRIPT_SERVICE)<>0 then result:=result or ACF_FLAG_SCRIPT;
end;

end.

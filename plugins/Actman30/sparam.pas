unit sparam;

interface

uses windows,iac_global;

const
  ACF_PARNUM   = $00000001; // Param is number
  ACF_UNICODE  = $00000002; // Param is Unicode string
  ACF_CURRENT  = $00000004; // Param is ignored, used current user handle
                            // from current message window
  ACF_RESULT   = $00000008; // Param is previous action result
  ACF_PARAM    = $00000010; // Param is Call parameter
  ACF_STRUCT   = $00000020;
  ACF_PARTYPE  = ACF_PARNUM  or ACF_UNICODE or
                 ACF_CURRENT or ACF_RESULT  or
                 ACF_PARAM   or ACF_STRUCT;
  ACF_TEMPLATE     = $00000800;
  ACF_SCRIPT_PARAM = $00001000;
  // dummy
  ACF_STRING = 0;

  ACF_RSTRING  = $00010000; // Service result is string
  ACF_RUNICODE = $00020000; // Service result is Widestring
  ACF_RSTRUCT  = $00040000; // Service result in structure
  ACF_RFREEMEM = $00080000; // Need to free memory

function  CreateParamBlock(parent:HWND;x,y,width:integer):THANDLE;
procedure ClearParamFields(Dialog:HWND);
function  FillParam       (Dialog:HWND;txt:pAnsiChar):integer;
function  SetParamValue   (Dialog:HWND;    flags:dword;    value:pointer):boolean;
function  GetParamValue   (Dialog:HWND;var flags:dword;var value:pointer):boolean;

procedure ClearParam    (flags:dword; var param);
function  DuplicateParam(flags:dword; var sparam,dparam):dword;

procedure PreProcess(flags:dword;var l_param:LPARAM;const WorkData:tWorkData);

function  CreateResultBlock(parent:HWND;x,y,width:integer):THANDLE;
procedure ClearResultFields(Dialog:HWND);
function  SetResultValue(Dialog:HWND;flags:dword):integer;
function  GetResultValue(Dialog:HWND):dword;

implementation

uses
  messages,
  common, editwrapper, wrapper, syswin,
  m_api, sedit, strans, mirutils;

{$include i_sparam_const.inc}
{$resource sparam.res}

const
  ptNumber  = 0;
  ptString  = 1;
  ptUnicode = 2;
  ptCurrent = 3;
  ptResult  = 4;
  ptParam   = 5;
  ptStruct  = 6;

const
  sresInt    = 0;
  sresString = 1;
  sresWide   = 2;
  sresStruct = 3;

//----- Processing functions -----

procedure PreProcess(flags:dword;var l_param:LPARAM;const WorkData:tWorkData);
var
  tmp1:pWideChar;
begin
  with WorkData do
  begin
    if (flags and ACF_STRUCT)<>0 then
    begin
      l_param:=uint_ptr(MakeStructure(pAnsiChar(l_param),Parameter,LastResult,ResultType))
    end
    else if (flags and ACF_PARAM)<>0 then
    begin
      l_param:=Parameter;
    end
    else if (flags and ACF_RESULT)<>0 then
    begin
      l_param:=LastResult;
    end
    else if (flags and ACF_CURRENT)<>0 then
    begin
      l_param:=WndToContact(WaitFocusedWndChild(GetForegroundwindow){GetFocus});
    end
    else
    begin
      if (flags and ACF_SCRIPT_PARAM)<>0 then
        l_param:=uint_ptr(ParseVarString(pWideChar(l_param),Parameter));

      tmp1:=pWideChar(l_param);
      if (flags and ACF_PARNUM)=0 then
      begin
        if (flags and ACF_UNICODE)=0 then
          WideToAnsi(tmp1,pAnsiChar(l_param),MirandaCP)
        else
          StrDupW(pWideChar(l_param),tmp1);
      end
      else
        l_param:=NumToInt(tmp1);

      if (flags and ACF_SCRIPT_PARAM)<>0 then
        mFreeMem(tmp1);
    end;
  end;
end;

procedure PostProcess(flags:dword;var l_param:LPARAM; var WorkData:tWorkData);
var
  code:integer;
  len:integer;
  pc:pAnsiChar;
begin
  if (flags and ACF_STRUCT)<>0 then
  begin
    with WorkData do
    begin
      LastResult:=GetStructureResult(l_param,@code,@len);
      case code of
{
        SST_LAST: begin
          result:=LastResult;
        end;
}
        SST_PARAM: begin //??
          LastResult:=Parameter;
          ResultType:=rtInt;
        end;
        SST_BYTE,SST_WORD,SST_DWORD,
        SST_QWORD,SST_NATIVE: begin
          ResultType:=rtInt;
        end;
        SST_BARR: begin
          StrDup(pAnsiChar(pc),pAnsiChar(LastResult),len);
          AnsiToWide(pAnsiChar(pc),PWideChar(LastResult),MirandaCP);
          mFreeMem(pAnsiChar(pc));
          ResultType:=rtWide;
        end;
        SST_WARR: begin
          StrDupW(pWideChar(LastResult),pWideChar(LastResult),len);
          ResultType:=rtWide;
        end;
        SST_BPTR: begin
          AnsiToWide(pAnsiChar(LastResult),pWideChar(LastResult),MirandaCP);
          ResultType:=rtWide;
        end;
        SST_WPTR: begin
          StrDupW(pWideChar(LastResult),pWideChar(LastResult));
          ResultType:=rtWide;
        end;
      end;
      FreeStructure(l_param);
      l_param:=0;
    end
  end;
end;

//----- Dialog functions -----

procedure MakeParamTypeList(wnd:HWND);
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);
  InsertString(wnd,ptNumber ,'number value');
  InsertString(wnd,ptString ,'ANSI string');
  InsertString(wnd,ptUnicode,'Unicode string');
  InsertString(wnd,ptCurrent,'current contact');
  InsertString(wnd,ptResult ,'last result');
  InsertString(wnd,ptParam  ,'parameter');
  InsertString(wnd,ptStruct ,'structure');
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
  if      StrCmp(buf,'hContact'    )=0 then result:=ptCurrent
  else if StrCmp(buf,'parameter'   )=0 then result:=ptParam
  else if StrCmp(buf,'result'      )=0 then result:=ptResult
  else if StrCmp(buf,'structure'   )=0 then result:=ptStruct
  else if StrCmp(buf,'Unicode text')=0 then result:=ptUnicode
  else                                      result:=ptString;
end;

// get line from template
function GetParamLine(src:pAnsiChar;dst:pWideChar;var ltype:integer):pAnsiChar;
var
  pp,pc:pAnsiChar;
  savechar:AnsiChar;
  j:integer;
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
    ltype:=ptNumber;
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

      if result=ptStruct then
        break
      else
        p:=pc;
    until pc=nil;
  end
  else
    result:=ptNumber;
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

procedure ClearParamFields(Dialog:HWND);
var
  wnd:HWND;
begin
  ShowWindow(GetDlgItem(Dialog,IDC_STRUCT),SW_HIDE);
  wnd:=GetDlgItem(Dialog,IDC_EDIT_PAR);
  ShowEditField  (wnd,SW_SHOW);
  EnableEditField(wnd,true);
  SendMessage    (wnd,CB_RESETCONTENT,0,0);
  SetEditFlags   (wnd,EF_ALL,0);
  CB_SelectData(Dialog,IDC_FLAG_PAR,ptNumber);
end;

function ParamDlgResizer(Dialog:HWND;lParam:LPARAM;urc:PUTILRESIZECONTROL):int; cdecl;
begin
  case urc^.wId of
    IDC_FLAG_PAR: result:=RD_ANCHORX_WIDTH or RD_ANCHORY_TOP; //RD_ANCHORX_RIGHT
    IDC_EDIT_PAR: result:=RD_ANCHORX_WIDTH or RD_ANCHORY_TOP;
    IDC_STRUCT:   result:=RD_ANCHORX_WIDTH or RD_ANCHORY_TOP;
    IDC_STAT_PAR: result:=RD_ANCHORX_LEFT  or RD_ANCHORY_TOP;
//    IDC_CLOSE:    result:=RD_ANCHORX_WIDTH or RD_ANCHORY_TOP;
  else
    result:=0;
  end;
end;

function DlgParamProc(Dialog:HWnd;hMessage:uint;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
var
  wnd,wnd1:HWND;
  i:integer;
  pcw:pWideChar;
  pc:pAnsiChar;
  urd:TUTILRESIZEDIALOG;
begin
  result:=0;

  case hMessage of
    WM_DESTROY: begin
      pc:=pAnsiChar(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_STRUCT),GWLP_USERDATA));
      mFreeMem(pc);
    end;

    WM_INITDIALOG: begin
      MakeEditField(Dialog,IDC_EDIT_PAR);

      MakeParamTypeList(GetDlgItem(Dialog,IDC_FLAG_PAR));
    end;

    WM_SIZE: begin
      FillChar(urd,SizeOf(TUTILRESIZEDIALOG),0);
      urd.cbSize    :=SizeOf(urd);
      urd.hwndDlg   :=Dialog;
      urd.hInstance :=hInstance;
      urd.lpTemplate:=MAKEINTRESOURCEA('IDD_SPARAM');
      urd.lParam    :=0;
      urd.pfnResizer:=@ParamDlgResizer;
      CallService(MS_UTILS_RESIZEDIALOG,0,tlparam(@urd));
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

              if i=ptStruct then
              begin
                ShowEditField(wnd ,SW_HIDE);
                ShowWindow   (wnd1,SW_SHOW);
              end
              else
              begin
                ShowEditField(wnd ,SW_SHOW);
                ShowWindow   (wnd1,SW_HIDE);

                if i in [ptCurrent,ptResult,ptParam] then
                  EnableEditField(wnd,false)
                else
                begin
                  if i=ptNumber then
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
      end;
    end;
  end;
end;

//----- Common interface functions -----

function CreateParamBlock(parent:HWND;x,y,width:integer):THANDLE;
var
  rc,rc1:TRECT;
begin
  SetRect(rc,x,y,x+width,y+0{dlgheight});
  MapDialogRect(parent,rc);

  result:=CreateDialogW(hInstance,'IDD_SPARAM',parent,@DlgParamProc);
  GetClientRect(result,rc1);
  SetWindowPos(result,0,
      x,y{rc.left,rc.top},rc.right-rc.left,rc1.bottom-rc1.top,
      SWP_NOZORDER);
end;

// if separate
function DestroyBlock(block:pointer):integer;
begin
  result:=0;
end;

function SetParamValue(Dialog:HWND;flags:dword;value:pointer):boolean;
var
  wnd,wnd1:HWND;
  pc:pAnsiChar;
  vtype:integer;
begin
  result:=true;
//?? Check for "Apply" activation

  wnd:=GetDlgItem(Dialog,IDC_EDIT_PAR);
  if (flags and ACF_TEMPLATE)<>0 then
  begin
    vtype:=FillParam(Dialog,value);
  end
  else if (flags and ACF_PARAM)<>0 then
  begin
    SendMessageW(wnd,WM_SETTEXT,0,LPARAM(TranslateW('Parameter')));
    EnableWindow(wnd,false);
    vtype:=ptParam;
  end
  else if (flags and ACF_RESULT)<>0 then
  begin
    SendMessageW(wnd,WM_SETTEXT,0,LPARAM(TranslateW('Last result')));
    EnableWindow(wnd,false);
    vtype:=ptResult;
  end
  else if (flags and ACF_CURRENT)<>0 then
  begin
    SendMessageW(wnd,WM_SETTEXT,0,LPARAM(TranslateW('Current user')));
    EnableWindow(wnd,false);
    vtype:=ptCurrent;
  end
  else if (flags and ACF_STRUCT)<>0 then
  begin
    vtype:=ptStruct;

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
  else if (flags and ACF_PARNUM)<>0 then
  begin
    vtype:=ptNumber;
    SendMessageW(wnd,WM_SETTEXT,0,LPARAM(value));
  end
  else if (flags and ACF_UNICODE)<>0 then
  begin
    vtype:=ptUnicode;
    SendMessageW(wnd,WM_SETTEXT,0,LPARAM(value));
  end
  else
  begin
    vtype:=ptString;
    SendMessageW(wnd,WM_SETTEXT,0,LPARAM(value));
  end;
  SetEditFlags(wnd,EF_SCRIPT,ord((flags and ACF_SCRIPT_PARAM)<>0));
  
  CB_SelectData(GetDlgItem(Dialog,IDC_FLAG_PAR),vtype);
end;

function GetParamValue(Dialog:HWND;var flags:dword;var value:pointer):boolean;
var
  wnd:HWND;
begin
  result:=true;
  flags:=0;
  value:=nil;
  wnd:=GetDlgItem(Dialog,IDC_EDIT_PAR);
  case CB_GetData(GetDlgItem(Dialog,IDC_FLAG_PAR)) of
    ptParam: begin
      flags:=flags or ACF_PARAM
    end;
    ptResult: begin
      flags:=flags or ACF_RESULT
    end;
    ptCurrent: begin
      flags:=flags or ACF_CURRENT
    end;
    ptNumber: begin
      flags:=flags or ACF_PARNUM;
      value:=GetDlgText(wnd);
    end;
    ptStruct: begin
      flags:=flags or ACF_STRUCT;
      StrDup(pAnsiChar(value),
          pAnsiChar(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_STRUCT),GWLP_USERDATA)));
    end;
    ptUnicode: begin
      flags:=flags or ACF_UNICODE;
      value:=GetDlgText(wnd);
    end;
    ptString: value:=GetDlgText(wnd);
  end;
  if (GetEditFlags(wnd) and EF_SCRIPT)<>0 then
     flags:=flags or ACF_SCRIPT_PARAM;
end;

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
      ptNumber: begin
        flags:=flags or ACF_PARNUM;
        StrDupW(pWideChar(dparam),pWideChar(@tmpdst));
      end;
      ptString: begin
        flags:=flags or ACF_STRING;
        StrDupW(pWideChar(dparam),pWideChar(@tmpdst));
      end;
      ptUnicode: begin
        flags:=flags or ACF_UNICODE;
        StrDupW(pWideChar(dparam),pWideChar(@tmpdst));
      end;
      ptStruct: begin
        flags:=flags or ACF_STRUCT;
        StrDup(pAnsiChar(dparam),pAnsiChar(sparam)+10); //10=StrLen('structure|')
      end;
      ptCurrent: flags:=flags or ACF_CURRENT;
      ptResult : flags:=flags or ACF_RESULT;
      ptParam  : flags:=flags or ACF_PARAM;
    end;
  end
  else if (flags and (ACF_CURRENT or ACF_RESULT or ACF_PARAM))=0 then
  begin
    if (flags and ACF_PARNUM)<>0 then
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

//----- result block -----

procedure MakeResultTypeList(wnd:HWND);
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);
  InsertString(wnd,sresInt   ,'Integer');
  InsertString(wnd,sresString,'String');
  InsertString(wnd,sresWide  ,'Wide String');
  InsertString(wnd,sresStruct,'Structure');
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

function ResultDlgResizer(Dialog:HWND;lParam:LPARAM;urc:PUTILRESIZECONTROL):int; cdecl;
begin
  case urc^.wId of
    IDC_RES_TYPE   : result:=RD_ANCHORX_WIDTH or RD_ANCHORY_TOP; //RD_ANCHORX_RIGHT
    IDC_RES_FREEMEM: result:=RD_ANCHORX_WIDTH or RD_ANCHORY_TOP;
    IDC_RES_STAT   : result:=RD_ANCHORX_LEFT  or RD_ANCHORY_TOP;
    IDC_RES_GROUP  : result:=RD_ANCHORX_WIDTH or RD_ANCHORY_TOP;
  else
    result:=0;
  end;
end;

procedure ClearResultFields(Dialog:HWND);
begin
  CheckDlgButton(Dialog,IDC_RES_FREEMEM,BST_UNCHECKED);
  ShowWindow(GetDlgItem(Dialog,IDC_RES_FREEMEM),SW_HIDE);
  CB_SelectData(Dialog,IDC_RES_TYPE,sresInt);
end;

function DlgResultProc(Dialog:HWnd;hMessage:uint;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
var
  urd:TUTILRESIZEDIALOG;
  i:integer;
begin
  result:=0;

  case hMessage of
    WM_INITDIALOG: begin
      MakeResultTypeList(GetDlgItem(Dialog,IDC_RES_TYPE));
    end;

    WM_SIZE: begin
      FillChar(urd,SizeOf(TUTILRESIZEDIALOG),0);
      urd.cbSize    :=SizeOf(urd);
      urd.hwndDlg   :=Dialog;
      urd.hInstance :=hInstance;
      urd.lpTemplate:=MAKEINTRESOURCEA('IDD_SRESULT');
      urd.lParam    :=0;
      urd.pfnResizer:=@ResultDlgResizer;
      CallService(MS_UTILS_RESIZEDIALOG,0,tlparam(@urd));
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        CBN_SELCHANGE:  begin
          case loword(wParam) of
            IDC_RES_TYPE: begin
              case CB_GetData(lParam) of
                sresInt,sresStruct: begin
                  i:=SW_HIDE;
                end;
                sresString,sresWide: begin
                  i:=SW_SHOW;
                end;
              end;
              ShowWindow(GetDlgItem(Dialog,IDC_RES_FREEMEM),i);
            end;
          end;
        end;
      end;
    end;
  end;
end;

function CreateResultBlock(parent:HWND;x,y,width:integer):THANDLE;
var
  rc,rc1:TRECT;
begin
  SetRect(rc,x,y,x+width,y+0{dlgheight});
  MapDialogRect(parent,rc);

  result:=CreateDialogW(hInstance,'IDD_SRESULT',parent,@DlgResultProc);
  GetClientRect(result,rc1);
  SetWindowPos(result,0,
      x,y{rc.left,rc.top},rc.right-rc.left,rc1.bottom-rc1.top,
      SWP_NOZORDER);
end;

function SetResultValue(Dialog:HWND;flags:dword):integer;
var
  btn:integer;
begin
  // RESULT
  if (flags and ACF_RSTRUCT)<>0 then
    result:=sresStruct
  else if (flags and ACF_RSTRING)<>0 then
  begin
    if (flags and ACF_RFREEMEM)<>0 then
      btn:=BST_CHECKED
    else
      btn:=BST_UNCHECKED;
    CheckDlgButton(Dialog,IDC_RES_FREEMEM,btn);

    if (flags and ACF_RUNICODE)<>0 then
      result:=sresWide
    else
      result:=sresString;
  end
  else
  begin
    result:=sresInt;
  end;
  CB_SelectData(Dialog,IDC_RES_TYPE,result);
end;

function GetResultValue(Dialog:HWND):dword;
begin
  case CB_GetData(GetDlgItem(Dialog,IDC_RES_TYPE)) of
    sresString: begin
      result:=ACF_RSTRING;
      if IsDlgButtonChecked(Dialog,IDC_RES_FREEMEM)=BST_CHECKED then
        result:=result or ACF_RFREEMEM;
    end;
    sresWide: begin
      result:=ACF_RSTRING or ACF_RUNICODE;
      if IsDlgButtonChecked(Dialog,IDC_RES_FREEMEM)=BST_CHECKED then
        result:=result or ACF_RFREEMEM;
    end;
    sresStruct: result:=ACF_RSTRUCT;
  else
    result:=0;
  end;

end;

end.

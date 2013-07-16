unit iac_service;

interface

implementation

uses
  windows, messages, commctrl,
  global, iac_global,
  m_api,
  sedit,strans,mApiCardM,
  mirutils,dbsettings, editwrapper,
  syswin,wrapper,common;

{$include i_cnst_service.inc}
{$resource iac_service.res}

const
  ACF_PARNUM  = $00000001; // Param is number
  ACF_UNICODE = $00000002; // Param is Unicode string
  ACF_CURRENT = $00000004; // Param is ignored, used current user handle
                           // from current message window
  ACF_RESULT  = $00000008; // Param is previous action result
  ACF_PARAM   = $00000010; // Param is Call parameter
  ACF_STRUCT  = $00000020;
  ACF_PARTYPE = ACF_PARNUM  or ACF_UNICODE or
                ACF_CURRENT or ACF_RESULT  or
                ACF_PARAM   or ACF_STRUCT;

  ACF_RSTRING  = $00010000; // Service result is string
  ACF_RUNICODE = $00020000; // Service result is Widestring
  ACF_RSTRUCT  = $00040000; // Service result in structure
  ACF_RFREEMEM = $00080000; // Need to free memory

  ACF_SCRIPT_PARAM   = $00001000;
  ACF_SCRIPT_SERVICE = $00002000;
  // dummy
  ACF_STRING = 0;

const
  opt_service  = 'service';
  opt_flags2   = 'flags2';
  opt_wparam   = 'wparam';
  opt_lparam   = 'lparam';
const
  ioService   = 'service';
  ioType      = 'type';
  ioResult    = 'result';
  ioCurrent   = 'current';
  ioParam     = 'param';
  ioStruct    = 'struct';
  ioValue     = 'value';
  ioNumber    = 'number';
  ioUnicode   = 'unicode';
  ioVariables = 'variables';
  ioWParam    = 'WPARAM';
  ioLParam    = 'LPARAM';
  ioOutput    = 'OUTPUT';
  ioFree      = 'free';
  ioAnsi      = 'ansi';
  ioInt       = 'int';

type
  tServiceAction = class(tBaseAction)
  private
    service:PAnsiChar;
    wparam :pWideChar;
    lparam :pWideChar;
    flags2 :dword;
  public
    constructor Create(uid:dword);
    destructor Destroy; override;
//    function  Clone:tBaseAction; override;
    function  DoAction(var WorkData:tWorkData):LRESULT; override;
    procedure Save(node:pointer;fmt:integer); override;
    procedure Load(node:pointer;fmt:integer); override;
  end;

//----- Support functions -----

//----- Object realization -----

constructor tServiceAction.Create(uid:dword);
begin
  inherited Create(uid);
end;

procedure ClearParam(flags:dword; var param);
begin
  if (flags and (ACF_CURRENT or ACF_RESULT or ACF_PARAM))=0 then
    mFreeMem(pointer(param));
end;

destructor tServiceAction.Destroy;
begin
  mFreeMem(service);
  ClearParam(flags ,wparam);
  ClearParam(flags2,lparam);

  inherited Destroy;
end;
{
function tServiceAction.Clone:tBaseAction;
begin
  result:=tServiceAction.Create(0);
  Duplicate(result);

  tServiceAction(result).flags2 :=flags2;
  StrDup(tServiceAction(result).service,service);

  if (flags and (ACF_PARNUM or ACF_RESULT or ACF_PARAM))=0 then
    StrDup(pAnsiChar(tServiceAction(result).wparam),pAnsiChar(wparam))
  else if ((flags and ACF_PARNUM)<>0) and ((flags and ACF_SCRIPT_PARAM)<>0) then
    StrDup(pAnsiChar(tServiceAction(result).wparam),pAnsiChar(wparam))
  else
    tServiceAction(result).wparam:=wparam;

  if (flags2 and (ACF_PARNUM or ACF_RESULT or ACF_PARAM))=0 then
    StrDup(pAnsiChar(tServiceAction(result).lparam),pAnsiChar(lparam))
  else if ((flags2 and ACF_PARNUM)<>0) and ((flags and ACF_SCRIPT_PARAM)<>0) then
    StrDup(pAnsiChar(tServiceAction(result).lparam),pAnsiChar(lparam))
  else
    tServiceAction(result).lparam:=lparam;
end;
}
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

function tServiceAction.DoAction(var WorkData:tWorkData):LRESULT;
var
  buf:array [0..255] of AnsiChar;
  lservice:pAnsiChar;
  lwparam,llparam:TLPARAM;
  res:int_ptr;
begin
  result:=0;

  lservice:=service;
  lwparam :=TLPARAM(wparam);
  llparam :=TLPARAM(lparam);
  // Service name processing
  if (flags and ACF_SCRIPT_SERVICE)<>0 then
    lservice:=ParseVarString(lservice,WorkData.Parameter);
    
  StrCopy(buf,lservice);
  if StrPos(lservice,protostr)<>nil then
    if CallService(MS_DB_CONTACT_IS,WorkData.Parameter,0)=0 then
    begin
      if (flags and ACF_SCRIPT_SERVICE)<>0 then
        mFreeMem(lservice);
      exit;
    end
    else
      StrReplace(buf,protostr,GetContactProtoAcc(WorkData.Parameter));

  if ServiceExists(buf)<>0 then
  begin

    PreProcess(flags ,lwparam,WorkData);
    PreProcess(flags2,llparam,WorkData);

    res:=CallServiceSync(buf,lwparam,llparam);
    ClearResult(WorkData);

    // result type processing
    if (flags and ACF_RSTRING)<>0 then
    begin
//!! delete old or not?
      if (flags and ACF_RUNICODE)=0 then
        AnsiToWide(pAnsiChar(res),pWideChar(WorkData.LastResult),MirandaCP)
      else
        StrDupW(pWideChar(WorkData.LastResult),pWideChar(res));
      WorkData.ResultType:=rtWide;

      if (flags and ACF_RFREEMEM)<>0 then
        mFreeMem(pAnsiChar(res)); //?? Miranda MM??
    end
    else if (flags and ACF_RSTRUCT)=0 then
      WorkData.ResultType:=rtInt
    else if (flags and ACF_RSTRUCT)<>0 then
    begin
      PostProcess(flags ,lwparam,WorkData);
      PostProcess(flags2,llparam,WorkData);
    end;

    // free string (ansi+unicode) parameters
    if ((flags and ACF_PARTYPE)=ACF_STRING) or
       ((flags and ACF_PARTYPE)=ACF_UNICODE) then
      mFreeMem(pointer(lwparam));
    if ((flags2 and ACF_PARTYPE)=ACF_STRING) or
       ((flags2 and ACF_PARTYPE)=ACF_UNICODE) then
      mFreeMem(pointer(llparam));
  end;
  if (flags and ACF_SCRIPT_SERVICE)<>0 then
    mFreeMem(lservice);
end;

procedure LoadParam(section:PAnsiChar;flags:dword; var param:pointer);
begin
  if (flags and (ACF_CURRENT or ACF_RESULT or ACF_PARAM))=0 then
  begin
    if (flags and ACF_STRUCT)<>0 then
      param:=DBReadUTF8(0,DBBranch,section,nil)
    else
      param:=DBReadUnicode(0,DBBranch,section,nil);
  end;
end;

function ReadParam(act:HXML; var param:pWideChar;isvar:boolean):dword;
var
  tmp:pWideChar;
begin
  result:=0;
  if act=0 then
    exit;
  with xmlparser do
  begin
    tmp:=getAttrValue(act,ioType);
    if      lstrcmpiw(tmp,ioCurrent)=0 then result:=result or ACF_CURRENT
    else if lstrcmpiw(tmp,ioResult )=0 then result:=result or ACF_RESULT
    else if lstrcmpiw(tmp,ioParam  )=0 then result:=result or ACF_PARAM
    else if lstrcmpiw(tmp,ioStruct )=0 then
    begin
      result:=result or ACF_STRUCT;
//!!!!      param:=ReadStruct(act);
    end
    else
    begin
      StrDupW(pWideChar(param),getAttrValue(act,ioValue));

      if      lstrcmpiw(tmp,ioNumber )=0 then result:=result or ACF_PARNUM
      else if lstrcmpiw(tmp,ioUnicode)=0 then result:=result or ACF_UNICODE;
//      else if lstrcmpiw(tmp,ioAnsi)=0 then;
    end;
  end;
end;
{
function ReadParamINI(node:pointer;prefix:pAnsiChar;var param:pWideChar;isvar:boolean):dword;
var
  pc,pc1:pAnsiChar;
  buf:array [0..63] of AnsiChar;
begin
  result:=0;
  pc1:=StrCopyE(buf,prefix);
  pc:=GetParamSectionStr(node,StrCopy(pc1,ioType));
  if      lstrcmpi(pc,ioCurrent)=0 then result:=result or ACF_CURRENT
  else if lstrcmpi(pc,ioResult )=0 then result:=result or ACF_RESULT
  else if lstrcmpi(pc,ioParam  )=0 then result:=result or ACF_PARAM
  else if lstrcmpi(pc,ioStruct )=0 then
  begin
    result:=result or ACF_STRUCT;
//!!!!      param:=ReadStruct(act);
  end
  else
  begin
    UTF8ToWide(GetParamSectionInt(node,StrCopy(pc1,ioValue)),param);

    if      lstrcmpi(pc,ioNumber )=0 then result:=result or ACF_PARNUM
    else if lstrcmpi(pc,ioUnicode)=0 then result:=result or ACF_UNICODE;
//      else if lstrcmpi(pc,ioAnsi)=0 then;
  end;
end;
}
procedure tServiceAction.Load(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
  sub:HXML;
  tmp:pWideChar;
begin
  inherited Load(node,fmt);

  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));

      StrCopy(pc,opt_service); service:=DBReadString(0,DBBranch,section,nil);
      StrCopy(pc,opt_flags2 ); flags2 :=DBReadDword (0,DBBranch,section);

      StrCopy(pc,opt_wparam); LoadParam(section,flags ,pointer(wparam));
      StrCopy(pc,opt_lparam); LoadParam(section,flags2,pointer(lparam));
    end;

    1: begin
      with xmlparser do
      begin
        FastWideToAnsi(getAttrValue(HXML(node),ioService),service);
//!!!!        StrDupW(service,getAttrValue(HXML(node),ioService));
        if StrToInt(getAttrValue(HXML(node),ioVariables))=1 then
          flags:=flags or ACF_SCRIPT_SERVICE;

        sub:=getNthChild(HXML(node),ioWParam,0);
        if StrToInt(getAttrValue(sub,ioVariables))=1 then
          flags:=flags or ACF_SCRIPT_PARAM;
        flags:=flags or ReadParam(sub,wparam,(flags and ACF_SCRIPT_PARAM)<>0);

        sub:=getNthChild(HXML(node),ioLParam,0);
        if StrToInt(getAttrValue(sub,ioVariables))=1 then
          flags2:=flags2 or ACF_SCRIPT_PARAM;
        flags2:=flags2 or ReadParam(sub,lparam,(flags2 and ACF_SCRIPT_PARAM)<>0);

        sub:=getNthChild(HXML(node),ioOutput,0);
        if StrToInt(getAttrValue(sub,ioFree))=1 then flags:=flags or ACF_RFREEMEM;

        tmp:=getAttrValue(sub,ioType);
        if      lstrcmpiw(tmp,ioUnicode)=0 then flags:=flags or ACF_RUNICODE
        else if lstrcmpiw(tmp,ioAnsi   )=0 then flags:=flags or ACF_RSTRING
        else if lstrcmpiw(tmp,ioStruct )=0 then flags:=flags or ACF_RSTRUCT
        else if lstrcmpiw(tmp,ioInt    )=0 then ;
      end;
    end;
{
    2: begin
      StrDup(service,GetParamSectionStr(node,ioService));
//!!!!      UTF8ToWide(GetParamSectionStr(node,ioService),service);
      if GetParamSectionInt(node,ioVariables)=1 then
        flags:=flags or ACF_SCRIPT_SERVICE;

      if GetParamSectionInt(node,ioWParam+'.'+ioVariables))=1 then
        flags:=flags or ACF_SCRIPT_PARAM;
      flags:=flags or ReadParamINI(node,ioWParam+'.',wparam,(flags and ACF_SCRIPT_PARAM)<>0);

      if GetParamSectionInt(node,ioLParam+'.'+ioVariables))=1 then
        flags2:=flags2 or ACF_SCRIPT_PARAM;
      flags2:=flags2 or ReadParamINI(node,ioLParam+'.',lparam,(flags2 and ACF_SCRIPT_PARAM)<>0);

      if GetParamSectionInt(node,ioFree)=1 then flags:=flags or ACF_RFREEMEM;

      pc:=GetParamSectionStr(node,ioType);
      if      lstrcmpi(pñ,ioUnicode)=0 then flags:=flags or ACF_RUNICODE
      else if lstrcmpi(pñ,ioAnsi   )=0 then flags:=flags or ACF_RSTRING
      else if lstrcmpi(pñ,ioStruct )=0 then flags:=flags or ACF_RSTRUCT
//      else if lstrcmpi(pñ,ioInt    )=0 then ;
    end;
}
  end;
end;

procedure SaveParam(section:PAnsiChar;flags:dword; param:pointer);
begin
  if (flags and (ACF_CURRENT or ACF_RESULT or ACF_PARAM))=0 then
  begin
    if pointer(param)<>nil then
    begin
      if (flags and ACF_STRUCT)<>0 then
        DBWriteUTF8(0,DBBranch,section,param)
      else
        DBWriteUnicode(0,DBBranch,section,param);
    end;
  end;
end;

procedure tServiceAction.Save(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Save(node,fmt);

  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));

      StrCopy(pc,opt_service); DBWriteString(0,DBBranch,section,service);
      StrCopy(pc,opt_flags2 ); DBWriteDWord (0,DBBranch,section,flags2);

      StrCopy(pc,opt_wparam); SaveParam(section,flags ,pointer(wparam));
      StrCopy(pc,opt_lparam); SaveParam(section,flags2,pointer(lparam));
    end;
{
    1: begin
    end;
}
  end;
end;

//----- Dialog realization -----

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
  sresStruct = 2;

procedure MakeResultTypeList(wnd:HWND);
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);
  InsertString(wnd,sresInt   ,'Integer');
  InsertString(wnd,sresString,'String');
  InsertString(wnd,sresStruct,'Structure');
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

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

var
  ApiCard:tmApiCard;
  
function FixParam(Dialog:HWND;buf:PAnsiChar;flag:integer):integer;
begin
  if      StrCmp(buf,Translate('hContact'    ))=0 then result:=ptCurrent
  else if StrCmp(buf,Translate('parameter'   ))=0 then result:=ptParam
  else if StrCmp(buf,Translate('result'      ))=0 then result:=ptResult
  else if StrCmp(buf,Translate('structure'   ))=0 then result:=ptStruct
  else if StrCmp(buf,Translate('Unicode text'))=0 then result:=ptUnicode
  else
  begin
    if (buf[0] in ['0'..'9']) or ((buf[0]='-') and (buf[1] in ['0'..'9'])) or
      ((buf[0]='$') and (buf[1] in sHexNum)) or
      ((buf[0]='0') and (buf[1]='x') and (buf[2] in sHexNum)) then
      result:=ptNumber
    else
      result:=ptString;
  end;

  CB_SelectData(Dialog,flag,result);
//    SendDlgItemMessage(Dialog,flag,CB_SETCURSEL,result,0);
  SendMessage(Dialog,WM_COMMAND,(CBN_SELCHANGE shl 16) or flag,GetDlgItem(Dialog,flag));
end;

procedure ReloadService(Dialog:HWND;setvalue:boolean);
var
  pc:pAnsiChar;
  buf,buf1:array [0..127] of AnsiChar;
  wnd:hwnd;
  i:integer;
  struct:pAnsiChar;
//    bufw:array [0..MaxDescrLen] of WideChar;
begin
  wnd:=GetDlgItem(Dialog,IDC_EDIT_SERVICE);
  SendMessageA(wnd,CB_GETLBTEXT,SendMessage(wnd,CB_GETCURSEL,0,0),tlparam(@buf));
  ApiCard.Service:=@buf;

  pc:=ApiCard.FillParams(GetDlgItem(Dialog,IDC_EDIT_WPAR),true);
  if pc<>nil then
  begin
    if GetDlgItemTextA(Dialog,IDC_EDIT_WPAR,buf1,SizeOf(buf1))>0 then
      case FixParam(Dialog,@buf1,IDC_FLAG_WPAR) of
        ptStruct: begin
          if setvalue then
          begin
            struct:=pAnsiChar(SetWindowLongPtrW(GetDlgItem(Dialog,IDC_WSTRUCT),GWLP_USERDATA,
            long_ptr(StrDup(struct,StrScan(pc,'|')+1))));
            mFreeMem(struct);
          end;

{          struct:=pAnsiChar(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_WSTRUCT),GWLP_USERDATA));
          mFreeMem(struct);
          StrDup(struct,StrScan(pc,'|')+1);
          SetWindowLongPtrW(GetDlgItem(Dialog,IDC_WSTRUCT),GWLP_USERDATA,long_ptr(struct));
//            AnsiToWide(StrScan(pc,'|')+1,wstruct,MirandaCP);
}
        end;
      end;
    mFreeMem(pc);
  end;

  pc:=ApiCard.FillParams(GetDlgItem(Dialog,IDC_EDIT_LPAR),false);
  if pc<>nil then
  begin
    if GetDlgItemTextA(Dialog,IDC_EDIT_LPAR,buf1,SizeOf(buf1))>0 then
      case FixParam(Dialog,@buf1,IDC_FLAG_LPAR) of
        ptStruct: begin
          if setvalue then
          begin
            struct:=pAnsiChar(SetWindowLongPtrW(GetDlgItem(Dialog,IDC_LSTRUCT),GWLP_USERDATA,
            long_ptr(StrDup(struct,StrScan(pc,'|')+1))));
            mFreeMem(struct);
          end;
{
          struct:=pAnsiChar(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_LSTRUCT),GWLP_USERDATA));
          mFreeMem(struct);
          StrDup(struct,StrScan(pc,'|')+1);
          SetWindowLongPtrW(GetDlgItem(Dialog,IDC_LSTRUCT),GWLP_USERDATA,long_ptr(struct));
//            AnsiToWide(StrScan(pc,'|')+1,lstruct,MirandaCP);
}
        end;
      end;
    mFreeMem(pc);
  end;

  pc:=ApiCard.ResultType;
  i:=sresInt;
  if pc<>nil then
  begin
    if      lstrcmpia(pc,'struct')=0 then i:=sresStruct
    else if lstrcmpia(pc,'str')=0 then
    begin
      i:=sresString;
      CheckDlgButton(Dialog,IDC_RES_UNICODE,BST_UNCHECKED);
    end
    else if lstrcmpia(pc,'wide')=0 then
    begin
      i:=sresString;
      CheckDlgButton(Dialog,IDC_RES_UNICODE,BST_CHECKED);
    end;
    mFreeMem(pc);
  end;
  CB_SelectData(Dialog,IDC_SRV_RESULT,i);
//    ApiCard.Show;
end;

// true - need to show structure
function SetParam(Dialog:HWND; aflags:dword; id:integer; aparam:pWideChar):integer;
var
  wnd:HWND;
begin
  wnd:=GetDlgItem(Dialog,id);
  if (aflags and ACF_PARAM)<>0 then
  begin
    EnableWindow(wnd,false);
    result:=ptParam;
  end
  else if (aflags and ACF_RESULT)<>0 then
  begin
    EnableWindow(wnd,false);
    result:=ptResult;
  end
  else if (aflags and ACF_CURRENT)<>0 then
  begin
    EnableWindow(wnd,false);
    result:=ptCurrent;
  end
  else if (aflags and ACF_PARNUM)<>0 then
  begin
    result:=ptNumber;
    SetDlgItemTextW(Dialog,id,aparam);
  end
  else if (aflags and ACF_STRUCT)<>0 then
  begin
    result:=ptStruct;
  end
  else if (aflags and ACF_UNICODE)<>0 then
  begin
    result:=ptUnicode;
    SetDlgItemTextW(Dialog,id,aparam);
  end
  else
  begin
    result:=ptString;
    SetDlgItemTextW(Dialog,id,aparam);
  end;
  SetEditFlags(wnd,EF_SCRIPT,ord((aflags and ACF_SCRIPT_PARAM)<>0));
end;

procedure ClearFields(Dialog:HWND);
var
  wnd:HWND;
begin
  ShowWindow(GetDlgItem(Dialog,IDC_WSTRUCT),SW_HIDE);
  wnd:=GetDlgItem(Dialog,IDC_EDIT_WPAR);
  ShowEditField  (wnd,SW_SHOW);
  EnableEditField(wnd,true);
  SendMessage    (wnd,CB_RESETCONTENT,0,0);
//??  SetDlgItemTextW(Dialog,IDC_EDIT_WPAR,nil);
  CB_SelectData(GetDlgItem(Dialog,IDC_FLAG_WPAR),ptNumber);
  SetEditFlags(wnd,EF_ALL,0);

  ShowWindow  (GetDlgItem(Dialog,IDC_LSTRUCT),SW_HIDE);
  wnd:=GetDlgItem(Dialog,IDC_EDIT_LPAR);
  ShowEditField  (wnd,SW_SHOW);
  EnableEditField(wnd,true);
  SendMessage    (wnd,CB_RESETCONTENT,0,0);
//??  SetDlgItemTextW(Dialog,IDC_EDIT_LPAR,nil);
  CB_SelectData(GetDlgItem(Dialog,IDC_FLAG_LPAR),ptNumber);
  SetEditFlags(wnd,EF_ALL,0);

  ShowWindow(GetDlgItem(Dialog,IDC_RES_FREEMEM),SW_HIDE);
  ShowWindow(GetDlgItem(Dialog,IDC_RES_UNICODE),SW_HIDE);
  CheckDlgButton(Dialog,IDC_RES_FREEMEM,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_RES_UNICODE,BST_UNCHECKED);

  CB_SelectData(Dialog,IDC_SRV_RESULT,sresInt);
end;

function DlgProc(Dialog:HWnd;hMessage:UINT;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
const
  NoProcess:boolean=true;
var
  i:integer;
  pc,pc1:pAnsiChar;
  wnd,wnd1:HWND;
  pcw:PWideChar;
begin
  result:=0;

  case hMessage of
    WM_DESTROY: begin
      ApiCard.Free;
      pc:=pAnsiChar(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_WSTRUCT),GWLP_USERDATA));
      mFreeMem(pc);
      pc:=pAnsiChar(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_LSTRUCT),GWLP_USERDATA));
      mFreeMem(pc);
    end;
    
    WM_INITDIALOG: begin
      MakeResultTypeList(GetDlgItem(Dialog,IDC_SRV_RESULT));
      MakeParamTypeList(GetDlgItem(Dialog,IDC_FLAG_WPAR));
      MakeParamTypeList(GetDlgItem(Dialog,IDC_FLAG_LPAR));

      TranslateDialogDefault(Dialog);

//??
      MakeEditField(Dialog,IDC_EDIT_SERVICE);
      MakeEditField(Dialog,IDC_EDIT_WPAR);
      MakeEditField(Dialog,IDC_EDIT_LPAR);

      ApiCard:=CreateServiceCard(Dialog);
      ApiCard.FillList(GetDlgItem(Dialog,IDC_EDIT_SERVICE),
        DBReadByte(0,DBBranch,'SrvListMode'));
    end;

    WM_ACT_SETVALUE: begin
      NoProcess:=true;
      ClearFields(Dialog);

      with tServiceAction(lParam) do
      begin
        if CB_SelectData(Dialog,IDC_EDIT_SERVICE,Hash(service,StrLen(service)))<>CB_ERR then
//        if SendDlgItemMessageA(Dialog,IDC_EDIT_SERVICE,CB_SELECTSTRING,twparam(-1),tlparam(service))<>CB_ERR then
          ReloadService(Dialog,false)
        else
          SetDlgItemTextA(Dialog,IDC_EDIT_SERVICE,service);
//!!
        SetEditFlags(GetDlgItem(Dialog,IDC_EDIT_SERVICE),EF_SCRIPT,
              ord((flags and ACF_SCRIPT_SERVICE)<>0));

        // RESULT
        if (flags and ACF_RSTRUCT)<>0 then
          i:=sresStruct
        else if (flags and ACF_RSTRING)<>0 then
        begin
          i:=sresString;
          if (flags and ACF_RUNICODE)<>0 then CheckDlgButton(Dialog,IDC_RES_UNICODE,BST_CHECKED);
          if (flags and ACF_RFREEMEM)<>0 then CheckDlgButton(Dialog,IDC_RES_FREEMEM,BST_CHECKED);
        end
        else
        begin
          i:=sresInt;
        end;
        CB_SelectData(Dialog,IDC_SRV_RESULT,i);

        // WPARAM
        i:=SetParam(Dialog,flags,IDC_EDIT_WPAR,pWideChar(wparam));
        if i=ptStruct then
        begin
          ShowEditField(GetDlgItem(Dialog,IDC_EDIT_WPAR),SW_HIDE);
          ShowWindow   (GetDlgItem(Dialog,IDC_WSTRUCT  ),SW_SHOW);

{
          p:=pAnsiChar(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_WSTRUCT),GWLP_USERDATA));
          mFreeMem(p);
}
          SetWindowLongPtrW(GetDlgItem(Dialog,IDC_WSTRUCT),GWLP_USERDATA,
            long_ptr(StrDup(pc,pAnsiChar(wparam))));
        end;
        CB_SelectData(GetDlgItem(Dialog,IDC_FLAG_WPAR),i);

        // LPARAM
        i:=SetParam(Dialog,flags2,IDC_EDIT_LPAR,pWideChar(lparam));
        if i=ptStruct then
        begin
          ShowEditField(GetDlgItem(Dialog,IDC_EDIT_LPAR),SW_HIDE);
          ShowWindow   (GetDlgItem(Dialog,IDC_LSTRUCT  ),SW_SHOW);

{
          p:=pAnsiChar(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_LSTRUCT),GWLP_USERDATA));
          mFreeMem(p);
}
          SetWindowLongPtrW(GetDlgItem(Dialog,IDC_LSTRUCT),GWLP_USERDATA,
            long_ptr(StrDup(pc,pAnsiChar(lparam))));
        end;
        CB_SelectData(GetDlgItem(Dialog,IDC_FLAG_LPAR),i);

      end;
      NoProcess:=false;
    end;

    WM_ACT_RESET: begin
      NoProcess:=true;
      ClearFields(Dialog);
      SetDlgItemTextW(Dialog,IDC_EDIT_SERVICE,nil);
      SetDlgItemTextW(Dialog,IDC_EDIT_WPAR,'0');
      SetDlgItemTextW(Dialog,IDC_EDIT_LPAR,'0');
{
      ShowWindow(GetDlgItem(Dialog,IDC_WSTRUCT),SW_HIDE);
      ShowWindow(GetDlgItem(Dialog,IDC_LSTRUCT),SW_HIDE);
}
      NoProcess:=false;
    end;

    WM_ACT_SAVE: begin
      with tServiceAction(lParam) do
      begin
        //WPARAM
        wnd:=GetDlgItem(Dialog,IDC_EDIT_WPAR);
        case CB_GetData(GetDlgItem(Dialog,IDC_FLAG_WPAR)) of
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
            wparam:=GetDlgText(wnd);
          end;
          ptStruct: begin
            flags:=flags or ACF_STRUCT;
            StrDup(pAnsiChar(wparam),
                pAnsiChar(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_WSTRUCT),GWLP_USERDATA)));
          end;
          ptUnicode: begin
            flags:=flags or ACF_UNICODE;
            wparam:=GetDlgText(wnd);
          end;
          ptString: wparam:=GetDlgText(wnd);
        end;
        if (GetEditFlags(wnd) and EF_SCRIPT)<>0 then
           flags:=flags or ACF_SCRIPT_PARAM;

        // LPARAM
        wnd:=GetDlgItem(Dialog,IDC_EDIT_LPAR);
        case CB_GetData(GetDlgItem(Dialog,IDC_FLAG_LPAR)) of
          ptParam: begin
            flags2:=flags2 or ACF_PARAM
          end;
          ptResult: begin
            flags2:=flags2 or ACF_RESULT
          end;
          ptCurrent: begin
            flags2:=flags2 or ACF_CURRENT
          end;
          ptNumber: begin
            flags2:=flags2 or ACF_PARNUM;
            lparam:=GetDlgText(wnd);
          end;
          ptStruct: begin
            flags2:=flags2 or ACF_STRUCT;
            StrDup(pAnsiChar(lparam),
                pAnsiChar(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_LSTRUCT),GWLP_USERDATA)));
          end;
          ptUnicode: begin
            flags2:=flags2 or ACF_UNICODE;
            lparam:=GetDlgText(wnd);
          end;
          ptString: lparam:=GetDlgText(wnd);
        end;
        if (GetEditFlags(wnd) and EF_SCRIPT)<>0 then
           flags2:=flags2 or ACF_SCRIPT_PARAM;

        // RESULT
        case CB_GetData(GetDlgItem(Dialog,IDC_SRV_RESULT)) of
          sresString: begin
            flags:=flags or ACF_RSTRING;
            if IsDlgButtonChecked(Dialog,IDC_RES_UNICODE)=BST_CHECKED then
              flags:=flags or ACF_RUNICODE;
            if IsDlgButtonChecked(Dialog,IDC_RES_FREEMEM)=BST_CHECKED then
              flags:=flags or ACF_RFREEMEM;
          end;
          sresStruct: flags:=flags or ACF_RSTRUCT;
        end;

        service:=ApiCard.NameFromList(GetDlgItem(Dialog,IDC_EDIT_SERVICE));
//!!
        if (GetEditFlags(Dialog,IDC_EDIT_SERVICE) and EF_SCRIPT)<>0 then
           flags:=flags or ACF_SCRIPT_SERVICE;
      end;
    end;

    WM_SHOWWINDOW: begin
      // hide window by ShowWindow function
      if (lParam=0) and (wParam=0) then
      begin
        pc:=pAnsiChar(SetWindowLongPtrW(GetDlgItem(Dialog,IDC_WSTRUCT),GWLP_USERDATA,0));
        mFreeMem(pc);
        pc:=pAnsiChar(SetWindowLongPtrW(GetDlgItem(Dialog,IDC_LSTRUCT),GWLP_USERDATA,0));
        mFreeMem(pc);
      end;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
{        CBN_EDITUPDATE,
}
        CBN_EDITCHANGE,
        EN_CHANGE: if not NoProcess then
            SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);

        CBN_SELCHANGE:  begin
          case loword(wParam) of
            IDC_SRV_RESULT: begin
              i:=CB_GetData(lParam);
              case i of
                sresInt,sresStruct: begin
                  ShowWindow(GetDlgItem(Dialog,IDC_RES_FREEMEM),SW_HIDE);
                  ShowWindow(GetDlgItem(Dialog,IDC_RES_UNICODE),SW_HIDE);
                end;
                sresString: begin
                  ShowWindow(GetDlgItem(Dialog,IDC_RES_FREEMEM),SW_SHOW);
                  ShowWindow(GetDlgItem(Dialog,IDC_RES_UNICODE),SW_SHOW);
                end;
              end;
            end;

            IDC_FLAG_WPAR,IDC_FLAG_LPAR: begin
              if loword(wParam)=IDC_FLAG_WPAR then
              begin
                wnd :=GetDlgItem(Dialog,IDC_EDIT_WPAR);
                wnd1:=GetDlgItem(Dialog,IDC_WSTRUCT);
              end
              else
              begin
                wnd :=GetDlgItem(Dialog,IDC_EDIT_LPAR);
                wnd1:=GetDlgItem(Dialog,IDC_LSTRUCT);
              end;
              i:=CB_GetData(GetDlgItem(Dialog,loword(wParam)));

              if i=ptStruct then
              begin
                ShowEditField(wnd,SW_HIDE);
                ShowWindow(wnd1,SW_SHOW);
              end
              else
              begin
                ShowEditField(wnd,SW_SHOW);
                ShowWindow(wnd1,SW_HIDE);
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

            IDC_EDIT_SERVICE: ReloadService(Dialog,true);
          end;
          if not NoProcess then
            SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
        end;

        BN_CLICKED: begin
          case loword(wParam) of
            IDC_WSTRUCT, IDC_LSTRUCT: begin
              pc:=pAnsiChar(GetWindowLongPtrW(lParam,GWLP_USERDATA));
//!!!!
              pc1:=EditStructure(pAnsiChar(pc),Dialog);
              if pc1<>nil then
              begin
                mFreeMem(pc);
                SetWindowLongPtrW(lParam,GWLP_USERDATA,long_ptr(pc1));
                SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
              end;
            end;
          else
            SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
          end;
        end;

      end;
    end;

    WM_HELP: begin
      pc:=ApiCard.NameFromList(GetDlgItem(Dialog,IDC_EDIT_SERVICE));
      ApiCard.Service:=pc;
      mFreeMem(pc);
      ApiCard.Show;

      result:=1;
    end;

  end;
end;

//----- Export/interface functions -----

var
  vc:tActModule;

function CreateAction:tBaseAction;
begin
  result:=tServiceAction.Create(vc.Hash);
end;

function CreateDialog(parent:HWND):HWND;
begin
  result:=CreateDialogW(hInstance,'IDD_ACTSERVICE',parent,@DlgProc);
end;

procedure Init;
begin
  vc.Next    :=ModuleLink;

  vc.Name    :='Service';
  vc.Dialog  :=@CreateDialog;
  vc.Create  :=@CreateAction;
  vc.Icon    :='IDI_SERVICE';
  vc.Hash    :=0;

  ModuleLink :=@vc;
end;

begin
  Init;
end.

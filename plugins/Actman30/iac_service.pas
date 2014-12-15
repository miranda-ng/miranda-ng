unit iac_service;

interface

implementation

uses
  windows, messages, commctrl,
  global, iac_global,
  m_api,
  sedit,strans,mApiCardM,
  mirutils,mircontacts,dbsettings, editwrapper,
  sparam,srvblock,
  syswin,wrapper,common;

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
{
  ACF_SCRIPT_PARAM set in "sparam" unit
  ACF_SCRIPT_SERVICE set in "srvblock" unit
  // dummy
  ACF_STRING = 0;

const // V2
  ACF2_SRV_WPAR = $00000001;
  ACF2_SRV_LPAR = $00000002;
  ACF2_SRV_SRVC = $00000004;
  ACF2_FREEMEM  = $00000100;
  ACF_OLD_WPARNUM  = $00000001;
  ACF_OLD_LPARNUM  = $00000002;
  ACF_OLD_WUNICODE = $00000004;
  ACF_OLD_LUNICODE = $00000008;
  ACF_OLD_WCURRENT = $00000010;
  ACF_OLD_LCURRENT = $00000020;
  ACF_OLD_WPARHEX  = $00000040;
  ACF_OLD_LPARHEX  = $00000080;

  ACF_OLD_WRESULT  = $00010000;
  ACF_OLD_LRESULT  = $00020000;
  ACF_OLD_WPARAM   = $00040000;
  ACF_OLD_LPARAM   = $00080000;
  ACF_OLD_WSTRUCT  = $00100000;
  ACF_OLD_LSTRUCT  = $00200000;

  ACF_OLD_STRING   = $00000800;
  ACF_OLD_UNICODE  = $00001000;
  ACF_OLD_STRUCT   = $00008000;

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
    if (flags and (ACF_RSTRING or ACF_UNICODE))<>0 then
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
  section:array [0..127] of AnsiChar;
  buf:array [0..31] of WideChar;
  pc:pAnsiChar;
  sub:HXML;
  tmp:pWideChar;
  lflags,lflags2:dword;
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

    100: begin
      pc:=StrCopyE(section,pAnsiChar(node));

      StrCopy(pc,opt_service); service:=DBReadString(0,DBBranch,section,nil);
      StrCopy(pc,opt_flags2 ); flags2 :=DBReadDword (0,DBBranch,section);


      if (flags and (ACF_OLD_WCURRENT or ACF_OLD_WRESULT or ACF_OLD_WPARAM))=0 then
      begin
        StrCopy(pc,opt_wparam);

        if (flags and ACF_OLD_WSTRUCT)<>0 then
          wparam:=PWideChar(DBReadUTF8(0,DBBranch,section,nil))
        else if ((flags and ACF_OLD_WPARNUM)=0) or ((flags2 and ACF2_SRV_WPAR)<>0) then
          wparam:=DBReadUnicode(0,DBBranch,section,nil)
        else
          StrDupW(wparam,IntToStr(buf,DBReadDWord(0,DBBranch,section)));
      end;

      if (flags and (ACF_OLD_LCURRENT or ACF_OLD_LRESULT or ACF_OLD_LPARAM))=0 then
      begin
        StrCopy(pc,opt_lparam);

        if (flags and ACF_OLD_LSTRUCT)<>0 then
          lparam:=PWideChar(DBReadUTF8(0,DBBranch,section,nil))
        else if ((flags and ACF_OLD_LPARNUM)=0) or ((flags2 and ACF2_SRV_LPAR)<>0) then
          lparam:=DBReadUnicode(0,DBBranch,section,nil)
        else
          StrDupW(lparam,IntToStr(buf,DBReadDWord(0,DBBranch,section)));
      end;

      lflags :=flags;
      lflags2:=flags2;
      flags :=flags and not ACF_MASK;
      flags2:=0;

      if (lflags2 and ACF2_SRV_SRVC)<>0 then flags:=flags or ACF_SCRIPT_SERVICE;
      
      if (lflags  and ACF_OLD_WPARNUM )<>0 then flags:=flags or ACF_PARNUM;
      if (lflags  and ACF_OLD_WUNICODE)<>0 then flags:=flags or ACF_UNICODE;
      if (lflags  and ACF_OLD_WCURRENT)<>0 then flags:=flags or ACF_CURRENT;
      if (lflags  and ACF_OLD_WRESULT )<>0 then flags:=flags or ACF_RESULT;
      if (lflags  and ACF_OLD_WPARAM  )<>0 then flags:=flags or ACF_PARAM;
      if (lflags  and ACF_OLD_WSTRUCT )<>0 then flags:=flags or ACF_STRUCT;
      if (lflags2 and ACF2_SRV_WPAR   )<>0 then flags:=flags or ACF_SCRIPT_PARAM;

      if (lflags  and ACF_OLD_LPARNUM )<>0 then flags2:=flags2 or ACF_PARNUM;
      if (lflags  and ACF_OLD_LUNICODE)<>0 then flags2:=flags2 or ACF_UNICODE;
      if (lflags  and ACF_OLD_LCURRENT)<>0 then flags2:=flags2 or ACF_CURRENT;
      if (lflags  and ACF_OLD_LRESULT )<>0 then flags2:=flags2 or ACF_RESULT;
      if (lflags  and ACF_OLD_LPARAM  )<>0 then flags2:=flags2 or ACF_PARAM;
      if (lflags  and ACF_OLD_LSTRUCT )<>0 then flags2:=flags2 or ACF_STRUCT;
      if (lflags2 and ACF2_SRV_LPAR   )<>0 then flags2:=flags2 or ACF_SCRIPT_PARAM;

      if (lflags  and ACF_OLD_STRING )<>0 then flags:=flags or ACF_RSTRING;
      if (lflags  and ACF_OLD_UNICODE)<>0 then flags:=flags or ACF_RUNICODE;
      if (lflags  and ACF_OLD_STRUCT )<>0 then flags:=flags or ACF_RSTRUCT;
      if (lflags2 and ACF2_FREEMEM   )<>0 then flags:=flags or ACF_RFREEMEM;

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
    13: begin
    end;
  end;
end;

//----- Dialog realization -----

function DlgProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  ServiceBlock:HWND;
  rc:TRECT;
  sv:tServiceValue;
begin
  result:=0;

  case hMessage of
    WM_DESTROY: begin
    end;
    
    WM_INITDIALOG: begin
      GetClientRect(Dialog,rc);
      ServiceBlock:=CreateServiceBlock(Dialog,0,0,rc.right,rc.bottom,ACF_NOVISUAL or ACF_SCRIPT_EXPAND);
      SetWindowLongPtrW(Dialog,GWLP_USERDATA,ServiceBlock);
      SetServiceListMode(ServiceBlock,DBReadByte(0,DBBranch,'SrvListMode'));

      TranslateDialogDefault(Dialog);
    end;

    WM_ACT_SETVALUE: begin
      ServiceBlock:=GetWindowLongPtrW(Dialog,GWLP_USERDATA);
      with tServiceAction(lParam) do
      begin
        sv.service:=service;
        sv.w_flag :=flags;
        sv.wparam :=wparam;
        sv.l_flag :=flags2;
        sv.lparam :=lparam;
        sv.flags  :=flags;
      end;
      SetSrvBlockValue(ServiceBlock,sv);
    end;

    WM_ACT_RESET: begin
      ClearServiceBlock(GetWindowLongPtrW(Dialog,GWLP_USERDATA));
    end;

    WM_ACT_SAVE: begin
      with tServiceAction(lParam) do
      begin
        ServiceBlock:=GetWindowLongPtrW(Dialog,GWLP_USERDATA);
        GetSrvBlockValue(ServiceBlock,sv);

        service:=sv.service;
        wparam :=sv.wparam;
        lparam :=sv.lparam;
        flags2 :=sv.l_flag;
        flags  :=sv.flags or sv.w_flag;
      end;
    end;

    //??
    WM_SHOWWINDOW: begin
{
      // hide window by ShowWindow function
      if (lParam=0) and (wParam=0) then
      begin
        pc:=pAnsiChar(SetWindowLongPtrW(GetDlgItem(Dialog,IDC_WSTRUCT),GWLP_USERDATA,0));
        mFreeMem(pc);
        pc:=pAnsiChar(SetWindowLongPtrW(GetDlgItem(Dialog,IDC_LSTRUCT),GWLP_USERDATA,0));
        mFreeMem(pc);
      end;
}
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        CBN_EDITCHANGE,
        BN_CLICKED: 
          SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
      end;
    end;

    WM_HELP: begin
      ServiceBlock:=GetWindowLongPtrW(Dialog,GWLP_USERDATA);
      SendMessage(ServiceBlock,WM_HELP,0,0);

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

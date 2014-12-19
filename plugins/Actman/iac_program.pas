unit iac_program;

interface

implementation

uses
  editwrapper,
  windows, messages, commctrl,
  global, iac_global, m_api, wrapper, syswin,
  mirutils, mircontacts, common, dbsettings;

{$include i_cnst_program.inc}
{$resource iac_program.res}

const
  ACF_CURPATH  = $00000001; // Current (not program) path
  ACF_PRTHREAD = $00000002; // parallel Program
  ACF_PRG_PRG  = $00000004; // script for program path
  ACF_PRG_ARG  = $00000008; // script for program args

const // V2
  ACF_OLD_CURPATH  = $00000002;
  ACF_OLD_PRTHREAD = $00000004;
  ACF2_PRG_PRG     = $00000001;
  ACF2_PRG_ARG     = $00000002;

const
  opt_prg      = 'program';
  opt_args     = 'arguments';
  opt_time     = 'time';
  opt_show     = 'show';
const
  ioArgs         = 'args';
  ioProgram      = 'program';
  ioCurrent      = 'current';
  ioParallel     = 'parallel';
  ioWait         = 'wait';
  ioFileVariable = 'modvariables';
  ioArgVariable  = 'argvariables';
  ioWindow       = 'window';
  ioHidden       = 'hidden';
  ioMinimized    = 'minimized';
  ioMaximized    = 'maximized';
type
  tProgramAction = class(tBaseAction)
  private
    prgname:pWideChar;
    args   :pWideChar;
    show   :dword;
    time   :dword;
  public
    constructor Create(uid:dword);
    destructor Destroy; override;
//    function  Clone:tBaseAction; override;
    function  DoAction(var WorkData:tWorkData):LRESULT; override;
    procedure Save(node:pointer;fmt:integer); override;
    procedure Load(node:pointer;fmt:integer); override;
  end;

//----- Support functions -----

function replany(var str:pWideChar;aparam:LPARAM;alast:pWideChar):boolean;
var
  buf:array [0..31] of WideChar;
  tmp:pWideChar;
begin
  if StrScanW(str,'<')<>nil then
  begin
    result:=true;
    mGetMem(tmp,2048);
    StrCopyW(tmp,str);
    StrReplaceW(tmp,'<param>',IntToStr(buf,aparam));
    StrReplaceW(tmp,'<last>' ,alast);

    str:=tmp;
  end
  else
    result:=false;
end;

//----- Object realization -----

constructor tProgramAction.Create(uid:dword);
begin
  inherited Create(uid);

  show   :=0;
  time   :=0;
  prgname:=nil;
  args   :=nil;
end;

destructor tProgramAction.Destroy;
begin
  mFreeMem(prgname);
  mFreeMem(args);

  inherited Destroy;
end;
{
function tProgramAction.Clone:tBaseAction;
begin
  result:=tProgramAction.Create(0);
  Duplicate(result);

  tProgramAction(result).show   :=show;
  tProgramAction(result).time   :=time;
  StrDupW(tProgramAction(result).prgname,prgname);
  StrDupW(tProgramAction(result).args   ,args);
end;
}
function tProgramAction.DoAction(var WorkData:tWorkData):LRESULT;
var
  tmp,tmpp,lpath:PWideChar;
  replPrg ,replArg :PWideChar;
  replPrg1,replArg1:PWideChar;
  pd:LPARAM;
  vars1,vars2,prgs,argss:boolean;
  buf:array [0..31] of WideChar;
begin
  result:=0;

  if WorkData.ResultType=rtInt then
  begin
    StrDupW(pWideChar(WorkData.LastResult),IntToStr(buf,WorkData.LastResult));
    WorkData.ResultType:=rtWide;
  end;

  replPrg:=prgname;
  prgs   :=replany(replPrg,WorkData.Parameter,pWideChar(WorkData.LastResult));

  replArg:=args;
  argss  :=replany(replArg,WorkData.Parameter,pWideChar(WorkData.LastResult));

  if ((flags and ACF_PRG_PRG)<>0) or
     ((flags and ACF_PRG_ARG)<>0) then
  begin
    if CallService(MS_DB_CONTACT_IS,WorkData.Parameter,0)<>0 then
      pd:=WorkData.Parameter
    else
      pd:=WndToContact(WaitFocusedWndChild(GetForegroundwindow){GetFocus});
    if (pd=0) and (WorkData.Parameter<>0) then
      pd:=WorkData.Parameter;
  end;

  if (flags and ACF_PRG_ARG)<>0 then
  begin
    vars2:=true;
    tmp :=ParseVarString(replArg,pd,pWideChar(WorkData.LastResult));
  end
  else
  begin
    vars2:=false;
    tmp :=replArg;
  end;

  if (flags and ACF_PRG_PRG)<>0 then
  begin
    vars1:=true;
    tmpp :=ParseVarString(replPrg,pd,pWideChar(WorkData.LastResult));
  end
  else
  begin
    vars1:=false;
    tmpp:=replPrg;
  end;
  
  if StrScanW(tmpp,'%')<>nil then
  begin
    mGetMem(replPrg1,8192*SizeOf(WideChar));
    ExpandEnvironmentStringsW(tmpp,replPrg1,8191);
    if vars1 then mFreeMem(tmpp);
    if prgs  then mFreeMem(replPrg);
    tmpp :=replPrg1;
    prgs :=false;
    vars1:=true;
  end;
  if StrScanW(tmp,'%')<>nil then
  begin
    mGetMem(replArg1,8192*SizeOf(WideChar));
    ExpandEnvironmentStringsW(tmp,replArg1,8191);
    if vars2 then mFreeMem(tmp);
    if argss then mFreeMem(replArg);
    tmp  :=replArg1;
    argss:=false;
    vars2:=true;
  end;

  if (flags and ACF_CURPATH)=0 then
    lpath:=ExtractW(tmpp,false)
  else
    lpath:=nil;

  if (flags and ACF_PRTHREAD)<>0 then
    time:=0
  else if time=0 then
    time:=INFINITE;
  WorkData.LastResult:=ExecuteWaitW(tmpp,tmp,lpath,show,time,@pd);
  WorkData.ResultType:=rtInt;

  if vars2 then mFreeMem(tmp);
  if vars1 then mFreeMem(tmpp);

  if prgs  then mFreeMem(replPrg);
  if argss then mFreeMem(replArg);

  mFreeMem(lpath);
end;

procedure tProgramAction.Load(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
  tmp:pWideChar;
  flags2,lflags:dword;
begin
  inherited Load(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_prg ); prgname:=DBReadUnicode(0,DBBranch,section,nil);
      StrCopy(pc,opt_args); args   :=DBReadUnicode(0,DBBranch,section,nil);
      StrCopy(pc,opt_time); time   :=DBReadDWord  (0,DBBranch,section,0);
      StrCopy(pc,opt_show); show   :=DBReadDWord  (0,DBBranch,section,SW_SHOW);
    end;

    100: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_prg ); prgname:=DBReadUnicode(0,DBBranch,section,nil);
      StrCopy(pc,opt_args); args   :=DBReadUnicode(0,DBBranch,section,nil);
      StrCopy(pc,opt_time); time   :=DBReadDWord  (0,DBBranch,section,0);
      StrCopy(pc,opt_show); show   :=DBReadDWord  (0,DBBranch,section,SW_SHOW);

      StrCopy(pc,'flags2'); flags2:=DBReadDWord(0,DBBranch,section,0);
      lflags:=flags;
      flags:=flags and not ACF_MASK;

      if (lflags and ACF_OLD_CURPATH )<>0 then flags:=flags or ACF_CURPATH;
      if (lflags and ACF_OLD_PRTHREAD)<>0 then flags:=flags or ACF_PRTHREAD;

      if (flags2 and ACF2_PRG_PRG)<>0 then flags:=flags or ACF_PRG_PRG;
      if (flags2 and ACF2_PRG_ARG)<>0 then flags:=flags or ACF_PRG_ARG;
    end;

    1: begin
      with xmlparser do
      begin
        StrDupW(prgname,getText(HXML(node)));
        StrDupW(args,getAttrValue(HXML(node),ioArgs));
        if StrToInt(getAttrValue(HXML(node),ioCurrent))=1 then
          flags:=flags or ACF_CURPATH;

        if StrToInt(getAttrValue(HXML(node),ioParallel))=1 then
          flags:=flags or ACF_PRTHREAD
        else
          time:=StrToInt(getAttrValue(HXML(node),ioWait));

        if StrToInt(getAttrValue(HXML(node),ioFileVariable))=1 then
          flags:=flags or ACF_PRG_PRG;

        if StrToInt(getAttrValue(HXML(node),ioArgVariable))=1 then
          flags:=flags or ACF_PRG_ARG;

        tmp:=getAttrValue(HXML(node),ioWindow);
        if      lstrcmpiw(tmp,ioHidden   )=0 then show:=SW_HIDE
        else if lstrcmpiw(tmp,ioMinimized)=0 then show:=SW_SHOWMINIMIZED
        else if lstrcmpiw(tmp,ioMaximized)=0 then show:=SW_SHOWMAXIMIZED
        else                                      show:=SW_SHOWNORMAL;
      end;
    end;
{
    2: begin
      UTF8ToWide(GetParamSectionStr(node,ioProgram),prgname);
      UTF8ToWide(GetParamSectionStr(node,ioArgs   ),args);
      if GetParamSectionInt(node,ioCurrent)=1 then
        flags:=flags or ACF_CURPATH;

      if GetParamSectionInt(node,ioParallel)=1 then
        flags:=flags or ACF_PRTHREAD
      else
        time:=GetParamSectionInt(node,ioWait);

      if GetParamSectionInt(node,ioFileVariable)=1 then
        flags:=flags or ACF_PRG_PRG;

      if GetParamSectionInt(node,ioArgVariable)=1 then
        flags:=flags or ACF_PRG_ARG;

      pc:=GetParamSectionStr(node,ioWindow);
      if      lstrcmpi(pc,ioHidden   )=0 then show:=SW_HIDE
      else if lstrcmpi(pc,ioMinimized)=0 then show:=SW_SHOWMINIMIZED
      else if lstrcmpi(pc,ioMaximized)=0 then show:=SW_SHOWMAXIMIZED
      else                                     show:=SW_SHOWNORMAL;
    end;
}
  end;
end;

procedure tProgramAction.Save(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Save(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_prg ); DBWriteUnicode(0,DBBranch,section,prgname);
      StrCopy(pc,opt_args); DBWriteUnicode(0,DBBranch,section,args);
      StrCopy(pc,opt_time); DBWriteDWord  (0,DBBranch,section,time);
      StrCopy(pc,opt_show); DBWriteDWord  (0,DBBranch,section,show);
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

procedure MakeFileEncList(wnd:HWND);
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);
{
  InsertString(wnd,0,'Ansi');
  InsertString(wnd,1,'UTF8');
  InsertString(wnd,2,'UTF8+sign');
  InsertString(wnd,3,'UTF16');
  InsertString(wnd,4,'UTF16+sign');
}
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

procedure ClearFields(Dialog:HWND);
begin
  CheckDlgButton(Dialog,IDC_FLAG_NORMAL,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_HIDDEN,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_MINIMIZE,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_MAXIMIZE,BST_UNCHECKED);

  CheckDlgButton(Dialog,IDC_FLAG_CURPATH,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_CONTINUE,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_PARALLEL,BST_UNCHECKED);
end;

function FillFileName(Dialog:HWND;idc:integer):boolean;
var
  pw,ppw:pWideChar;
begin
  mGetMem(pw,1024*SizeOf(WideChar));
  ppw:=GetDlgText(Dialog,idc);
  result:=ShowDlgW(pw,ppw);
  if result then
  begin
    SetDlgItemTextW(Dialog,idc,pw);
    SetEditFlags(Dialog,idc,EF_SCRIPT,0);
  end;
  mFreeMem(ppw);
  mFreeMem(pw);
end;

function DlgProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  NoProcess:boolean=true;
begin
  result:=0;

  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);

      MakeEditField(Dialog,IDC_EDIT_PRGPATH);
      MakeEditField(Dialog,IDC_EDIT_PRGARGS);
    end;

    WM_ACT_SETVALUE: begin
      NoProcess:=true;
      ClearFields(Dialog);

      with tProgramAction(lParam) do
      begin
        SetDlgItemTextW(Dialog,IDC_EDIT_PRGPATH ,prgname);
        SetDlgItemTextW(Dialog,IDC_EDIT_PRGARGS ,args);

        SetEditFlags(Dialog,IDC_EDIT_PRGPATH,EF_SCRIPT,ord((flags and ACF_PRG_PRG)<>0));
        SetEditFlags(Dialog,IDC_EDIT_PRGARGS,EF_SCRIPT,ord((flags and ACF_PRG_ARG)<>0));

        SetDlgItemInt(Dialog,IDC_EDIT_PROCTIME,time,false);
        case show of
          SW_HIDE         : CheckDlgButton(Dialog,IDC_FLAG_HIDDEN,BST_CHECKED);
          SW_SHOWMINIMIZED: CheckDlgButton(Dialog,IDC_FLAG_MINIMIZE,BST_CHECKED);
          SW_SHOWMAXIMIZED: CheckDlgButton(Dialog,IDC_FLAG_MAXIMIZE,BST_CHECKED);
        else
          {SW_SHOWNORMAL   :} CheckDlgButton(Dialog,IDC_FLAG_NORMAL,BST_CHECKED);
        end;
        if (flags and ACF_CURPATH)<>0 then
          CheckDlgButton(Dialog,IDC_FLAG_CURPATH,BST_CHECKED);
        if (flags and ACF_PRTHREAD)<>0 then
          CheckDlgButton(Dialog,IDC_FLAG_PARALLEL,BST_CHECKED)
        else
          CheckDlgButton(Dialog,IDC_FLAG_CONTINUE,BST_CHECKED);
      end;
      NoProcess:=false;
    end;

    WM_ACT_RESET: begin
      NoProcess:=true;
      ClearFields(Dialog);

      SetDlgItemTextW(Dialog,IDC_EDIT_PRGPATH,nil);
      SetDlgItemTextW(Dialog,IDC_EDIT_PRGARGS,nil);
      SetEditFlags(Dialog,IDC_EDIT_PRGPATH,EF_ALL,0);
      SetEditFlags(Dialog,IDC_EDIT_PRGARGS,EF_ALL,0);

      CheckDlgButton(Dialog,IDC_FLAG_PARALLEL,BST_CHECKED);
      CheckDlgButton(Dialog,IDC_FLAG_NORMAL  ,BST_CHECKED);
      SetDlgItemInt(Dialog,IDC_EDIT_PROCTIME,0,false);
      NoProcess:=false;
    end;

    WM_ACT_SAVE: begin
      with tProgramAction(lParam) do
      begin
        {mFreeMem(prgname); }prgname:=GetDlgText(Dialog,IDC_EDIT_PRGPATH);
        {mFreeMem(args   ); }args   :=GetDlgText(Dialog,IDC_EDIT_PRGARGS);
{
        p:=GetDlgText(IDC_EDIT_PRGPATH);
        if p<>nil then
        begin
          CallService(MS_UTILS_PATHTORELATIVE,dword(p),dword(@buf));
          StrDupW(prgname,@buf);
          mFreeMem(p);
        end;
}
        if IsDlgButtonChecked(Dialog,IDC_FLAG_PARALLEL)=BST_CHECKED then
          flags:=flags or ACF_PRTHREAD;
        if IsDlgButtonChecked(Dialog,IDC_FLAG_CURPATH)=BST_CHECKED then
          flags:=flags or ACF_CURPATH;

        time:=GetDlgItemInt(Dialog,IDC_EDIT_PROCTIME,pbool(nil)^,false);

        if IsDlgButtonChecked(Dialog,IDC_FLAG_MINIMIZE)=BST_CHECKED then
          show:=SW_SHOWMINIMIZED
        else if IsDlgButtonChecked(Dialog,IDC_FLAG_MAXIMIZE)=BST_CHECKED then
          show:=SW_SHOWMAXIMIZED
        else if IsDlgButtonChecked(Dialog,IDC_FLAG_HIDDEN)=BST_CHECKED then
          show:=SW_HIDE
        else //if IsDlgButtonChecked(Dialog,IDC_FLAG_NORMAL)=BST_CHECKED then
          show:=SW_SHOWNORMAL;

        if (GetEditFlags(Dialog,IDC_EDIT_PRGPATH) and EF_SCRIPT)<>0 then flags:=flags or ACF_PRG_PRG;
        if (GetEditFlags(Dialog,IDC_EDIT_PRGARGS) and EF_SCRIPT)<>0 then flags:=flags or ACF_PRG_ARG;
      end;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        EN_CHANGE: if not NoProcess then
            SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);

        BN_CLICKED: begin
          case loword(wParam) of
            IDC_PROGRAM: begin
              if not FillFileName(Dialog,IDC_EDIT_PRGPATH) then
                exit;
            end;
          end;
          SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
        end;
      end;
    end;

    WM_HELP: begin
      MessageBoxW(0,
        TranslateW('Text <last> replacing'#13#10+
          'by last result'#13#10#13#10+
          'Text <param> replacing'#13#10+
          'by parameter'),
        TranslateW('Text'),0);
      result:=1;
    end;

  end;
end;

//----- Export/interface functions -----

var
  vc:tActModule;

function CreateAction:tBaseAction;
begin
  result:=tProgramAction.Create(vc.Hash);
end;

function CreateDialog(parent:HWND):HWND;
begin
  result:=CreateDialogW(hInstance,'IDD_ACTPROGRAM',parent,@DlgProc);
end;

procedure Init;
begin
  vc.Next    :=ModuleLink;

  vc.Name    :='Program';
  vc.Dialog  :=@CreateDialog;
  vc.Create  :=@CreateAction;
  vc.Icon    :='IDI_PROGRAM';
  vc.Hash    :=0;

  ModuleLink :=@vc;
end;

begin
  Init;
end.

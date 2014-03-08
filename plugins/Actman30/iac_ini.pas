unit iac_ini;

interface

implementation

uses
  windows,messages,commctrl,
  iac_global,global,
  common,m_api,wrapper,
  dbsettings,editwrapper,mirutils;

{$include i_cnst_ini.inc}
{$resource iac_ini.res}

const
  opt_file   :PAnsiChar = 'inifile';
  opt_section:PAnsiChar = 'section';
  opt_param  :PAnsiChar = 'param';
  opt_value  :PAnsiChar = 'value';

const
  ACF_INI_WRITE   = $00000001;
  ACF_INI_DELETE  = $00000002;
  ACF_INI_LR      = $00000004;
  ACF_INI_UTF     = $00000008;
  ACF_INI_FILE    = $00000010;
  ACF_INI_SECTION = $00000020;
  ACF_INI_PARAM   = $00000040;
  ACF_INI_VALUE   = $00000080;
type
  tINIAction = class(tBaseAction)
    inifile  :pWideChar;
    section  :pWideChar;
    parameter:pWideChar;
    value    :pWideChar;

    constructor Create(uid:dword);
    destructor Destroy; override;
//    function  Clone:tBaseAction; override;
    function  DoAction(var WorkData:tWorkData):LRESULT; override;
    procedure Save(node:pointer;fmt:integer); override;
    procedure Load(node:pointer;fmt:integer); override;
  end;

//----- Support functions -----

//----- Object realization -----

constructor tINIAction.Create(uid:dword);
begin
  inherited Create(uid);
end;

destructor tINIAction.Destroy;
begin
  mFreeMem(inifile);
  mFreeMem(section);
  mFreeMem(parameter);
  mFreeMem(value);

  inherited Destroy;
end;
{
function tINIAction.Clone:tBaseAction;
begin
  result:=tServiceAction.Create(0);
  Duplicate(result);

end;
}
function tINIAction.DoAction(var WorkData:tWorkData):LRESULT;
var
  linifile,
  lsection,
  lparam,
  lvalue:pWideChar;
  ainifile,
  asection,
  aparam,
  avalue:pAnsiChar;
  buf:pAnsiChar;
  cond:bool;
begin
  result:=0;
  cond:=true;

  if (flags and ACF_INI_FILE)<>0 then
    linifile:=ParseVarString(inifile,WorkData.Parameter,pWideChar(WorkData.LastResult))
  else
    linifile:=inifile;
  if (linifile=nil) or (linifile^=#0) then
    cond:=false;

  if cond then
  begin
    if (flags and ACF_INI_SECTION)<>0 then
      lsection:=ParseVarString(section,WorkData.Parameter,pWideChar(WorkData.LastResult))
    else
      lsection:=section;
    if (lsection=nil) or (lsection^=#0) then
      cond:=false;
  end
  else
    lsection:=nil;

  if cond then
  begin
    if (flags and ACF_INI_PARAM)<>0 then
      lparam:=ParseVarString(parameter,WorkData.Parameter,pWideChar(WorkData.LastResult))
    else
      lparam:=parameter;
  end
  else
    lparam:=nil;

  if cond then
  begin
    if (flags and ACF_INI_DELETE)<>0 then
    begin
      WritePrivateProfileStringW(lsection,lparam,nil,linifile);
    end
    else
    begin
      if (lparam<>nil) and (lparam^<>#0) then
      begin
        if (flags and ACF_INI_LR)<>0 then
          lvalue:=pWideChar(WorkData.LastResult)
        else if (flags and ACF_INI_VALUE)<>0 then
          lvalue:=ParseVarString(value,WorkData.Parameter,pWideChar(WorkData.LastResult))
        else
          lvalue:=value;

        WideToAnsi(linifile,ainifile);
        WideToAnsi(lsection,asection);
        WideToAnsi(lparam  ,aparam);

        if (flags and ACF_INI_WRITE)<>0 then
        begin
          if (flags and ACF_INI_UTF)=0 then
            WideToAnsi(lvalue,avalue,MirandaCP)
          else
            WideToUTF8(lvalue,avalue);

          WritePrivateProfileStringA(asection,aparam,avalue,ainifile);

          mFreeMem(avalue);
        end

        else // single line only (Windows-way)
        begin
          mGetMem(buf,4096); buf^:=#0;
          GetPrivateProfileStringA(asection,aparam,avalue,buf,4096,ainifile);
          ClearResult(WorkData);

          if GetTextFormat(pByte(buf),StrLen(buf))=CP_UTF8 then
            UTF8ToWide(buf,pWideChar(WorkData.LastResult))
          else
            AnsiToWide(buf,pWideChar(WorkData.LastResult),MirandaCP);
          WorkData.ResultType:=rtWide;
          mFreeMem(buf);
        end;

        mFreeMem(ainifile);
        mFreeMem(asection);
        mFreeMem(aparam);

        if ((flags and ACF_INI_VALUE)<>0) and
           ((flags and ACF_INI_LR   ) =0) then mFreeMem(lvalue);
      end;
    end;
  end;

  if (flags and ACF_INI_FILE   )<>0 then mFreeMem(linifile);
  if (flags and ACF_INI_SECTION)<>0 then mFreeMem(lsection);
  if (flags and ACF_INI_PARAM  )<>0 then mFreeMem(lparam);
end;

procedure tINIAction.Load(node:pointer;fmt:integer);
var
  lsection: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Load(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(lsection,pAnsiChar(node));
      StrCopy(pc,opt_file   ); inifile  :=DBReadUnicode(0,DBBranch,lsection,nil);
      StrCopy(pc,opt_section); section  :=DBReadUnicode(0,DBBranch,lsection,nil);
      StrCopy(pc,opt_param  ); parameter:=DBReadUnicode(0,DBBranch,lsection,nil);
      StrCopy(pc,opt_value  ); value    :=DBReadUnicode(0,DBBranch,lsection,nil);
    end;
{
    1: begin
    end;
}
  end;
end;

procedure tINIAction.Save(node:pointer;fmt:integer);
var
  lsection: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Save(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(lsection,pAnsiChar(node));
      StrCopy(pc,opt_file   ); DBWriteUnicode(0,DBBranch,lsection,inifile);
      StrCopy(pc,opt_section); DBWriteUnicode(0,DBBranch,lsection,section);
      StrCopy(pc,opt_param  ); DBWriteUnicode(0,DBBranch,lsection,parameter);
      StrCopy(pc,opt_value  ); DBWriteUnicode(0,DBBranch,lsection,value);
    end;
{
    1: begin
    end;
}
  end;
end;

//----- Dialog realization -----

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

procedure ClearFields(Dialog:HWND);
begin
  CheckDlgButton(Dialog,IDC_INI_READ  ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_INI_WRITE ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_INI_DELETE,BST_UNCHECKED);

  CheckDlgButton(Dialog,IDC_INI_LR ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_INI_UTF,BST_UNCHECKED);

  EnableEditField(Dialog,IDC_INI_VALUE,true);
end;

function DlgProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  NoProcess:boolean=true;
begin
  result:=0;

  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);

      MakeEditField(Dialog,IDC_INI_PATH);
      MakeEditField(Dialog,IDC_INI_SECTION);
      MakeEditField(Dialog,IDC_INI_PARAM);
      MakeEditField(Dialog,IDC_INI_VALUE);
    end;

    WM_ACT_SETVALUE: begin
      NoProcess:=true;
      ClearFields(Dialog);
      with tINIAction(lParam) do
      begin
        SetDlgItemTextW(Dialog,IDC_INI_PATH   ,inifile);
        SetDlgItemTextW(Dialog,IDC_INI_SECTION,section);
        SetDlgItemTextW(Dialog,IDC_INI_PARAM  ,parameter);
        SetDlgItemTextW(Dialog,IDC_INI_VALUE  ,value);

        SetEditFlags(Dialog,IDC_INI_PATH   ,EF_SCRIPT,ord((flags and ACF_INI_FILE   )<>0));
        SetEditFlags(Dialog,IDC_INI_SECTION,EF_SCRIPT,ord((flags and ACF_INI_SECTION)<>0));
        SetEditFlags(Dialog,IDC_INI_PARAM  ,EF_SCRIPT,ord((flags and ACF_INI_PARAM  )<>0));

        SetEditFlags(Dialog,IDC_INI_VALUE,EF_SCRIPT,ord((flags and ACF_INI_VALUE)<>0));
        if ((flags and ACF_INI_DELETE)<>0) or
           ((flags and ACF_INI_LR    )<>0) then
          EnableEditField(Dialog,IDC_INI_VALUE,false);

        if (flags and ACF_INI_WRITE)<>0 then
          CheckDlgButton(Dialog,IDC_INI_WRITE,BST_CHECKED)
        else if (flags and ACF_INI_DELETE)<>0 then
          CheckDlgButton(Dialog,IDC_INI_DELETE,BST_CHECKED)
        else
          CheckDlgButton(Dialog,IDC_INI_READ,BST_CHECKED);

        EnableWindow(GetDlgItem(Dialog,IDC_INI_LR),(flags and ACF_INI_DELETE)=0);
        if (flags and ACF_INI_LR)<>0 then
          CheckDlgButton(Dialog,IDC_INI_LR,BST_CHECKED);

        EnableWindow(GetDlgItem(Dialog,IDC_INI_UTF),(flags and ACF_INI_WRITE)<>0);
        if (flags and ACF_INI_UTF)<>0 then
          CheckDlgButton(Dialog,IDC_INI_UTF,BST_CHECKED);
      end;
      NoProcess:=false;
    end;

    WM_ACT_RESET: begin
      NoProcess:=true;
      ClearFields(Dialog);

      SetDlgItemTextW(Dialog,IDC_INI_PATH   ,nil);
      SetDlgItemTextW(Dialog,IDC_INI_SECTION,nil);
      SetDlgItemTextW(Dialog,IDC_INI_PARAM  ,nil);
      SetDlgItemTextW(Dialog,IDC_INI_VALUE  ,nil);
      SetEditFlags(Dialog,IDC_INI_PATH   ,EF_ALL,0);
      SetEditFlags(Dialog,IDC_INI_SECTION,EF_ALL,0);
      SetEditFlags(Dialog,IDC_INI_PARAM  ,EF_ALL,0);
      SetEditFlags(Dialog,IDC_INI_VALUE  ,EF_ALL,0);

      EnableWindow(GetDlgItem(Dialog,IDC_INI_LR ),true);
      EnableWindow(GetDlgItem(Dialog,IDC_INI_UTF),false);

      CheckDlgButton(Dialog,IDC_INI_READ,BST_CHECKED);
      NoProcess:=false;
    end;

    WM_ACT_SAVE: begin
      with tINIAction(lParam) do
      begin
        if IsDlgButtonChecked(Dialog,IDC_INI_WRITE)<>BST_UNCHECKED then
          flags:=flags or ACF_INI_WRITE
        else if IsDlgButtonChecked(Dialog,IDC_INI_DELETE)<>BST_UNCHECKED then
          flags:=flags or ACF_INI_DELETE;

        if IsDlgButtonChecked(Dialog,IDC_INI_LR )<>BST_UNCHECKED then flags:=flags or ACF_INI_LR;
        if IsDlgButtonChecked(Dialog,IDC_INI_UTF)<>BST_UNCHECKED then flags:=flags or ACF_INI_UTF;

        inifile  :=GetDlgText(Dialog,IDC_INI_PATH);
        section  :=GetDlgText(Dialog,IDC_INI_SECTION);
        parameter:=GetDlgText(Dialog,IDC_INI_PARAM);
        value    :=GetDlgText(Dialog,IDC_INI_VALUE);
        if (GetEditFlags(Dialog,IDC_INI_PATH   ) and EF_SCRIPT)<>0 then flags:=flags or ACF_INI_FILE;
        if (GetEditFlags(Dialog,IDC_INI_SECTION) and EF_SCRIPT)<>0 then flags:=flags or ACF_INI_SECTION;
        if (GetEditFlags(Dialog,IDC_INI_PARAM  ) and EF_SCRIPT)<>0 then flags:=flags or ACF_INI_PARAM;
        if (GetEditFlags(Dialog,IDC_INI_VALUE  ) and EF_SCRIPT)<>0 then flags:=flags or ACF_INI_VALUE;
      end;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        EN_CHANGE: if not NoProcess then
          SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);

        BN_CLICKED: begin
          case loword(wParam) of
            IDC_INI_INIBTN: begin
              if not FillFileName(Dialog,IDC_INI_PATH) then
                exit;
            end;

            IDC_INI_READ,
            IDC_INI_WRITE,
            IDC_INI_DELETE: begin
              EnableEditField(Dialog,IDC_INI_VALUE,loword(wParam)<>IDC_INI_DELETE);

              EnableWindow(GetDlgItem(Dialog,IDC_INI_LR),
                IsDlgButtonChecked(Dialog,IDC_INI_DELETE)=BST_UNCHECKED);
              EnableWindow(GetDlgItem(Dialog,IDC_INI_UTF),
                IsDlgButtonChecked(Dialog,IDC_INI_WRITE)<>BST_UNCHECKED);
            end;

            IDC_INI_LR: begin
              EnableEditField(Dialog,IDC_INI_VALUE,IsDlgButtonChecked(Dialog,IDC_INI_LR)=BST_UNCHECKED);
            end;
          end;

          if not NoProcess then
            SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
        end;
      end;
    end;

    WM_HELP: begin
      result:=1;
    end;

  end;
end;

//----- Export/interface functions -----

var
  vc:tActModule;

function CreateAction:tBaseAction;
begin
  result:=tINIAction.Create(vc.Hash);
end;

function CreateDialog(parent:HWND):HWND;
begin
  result:=CreateDialogW(hInstance,'IDD_INI',parent,@DlgProc);
end;

procedure Init;
begin
  vc.Next    :=ModuleLink;

  vc.Name    :='INI';
  vc.Dialog  :=@CreateDialog;
  vc.Create  :=@CreateAction;
  vc.Icon    :='IDI_INI';

  ModuleLink :=@vc;
end;

begin
  Init;
end.

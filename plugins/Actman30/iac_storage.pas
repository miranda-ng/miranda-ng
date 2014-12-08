unit iac_storage;

interface

implementation

uses
  windows, messages,
  common, wrapper,
  mirutils,m_api,dbsettings, inouttext,
  global,iac_global;

{$include i_cnst_storage.inc}
{$resource iac_storage.res}

const
  opt_number = 'number';
const
  ioNumber = 'number';
  ioOper   = 'oper';
  ioCopy   = 'copy';
const
  ACF_COPYFROM = $00000001;
type
  tStorageAction = class(tBaseAction)
    Number:integer;

    constructor Create(uid:dword);
    destructor Destroy; override;
//    function  Clone:tBaseAction; override;
    function  DoAction(var WorkData:tWorkData):LRESULT; override;
    procedure Save(node:pointer;fmt:integer); override;
    procedure Load(node:pointer;fmt:integer); override;
  end;

//----- Support functions -----

//----- Object realization -----

constructor tStorageAction.Create(uid:dword);
begin
  inherited Create(uid);

  Number:=0;
end;

destructor tStorageAction.Destroy;
begin

  inherited Destroy;
end;
{
function tStorageAction.Clone:tBaseAction;
begin
  result:=.Create(0);
  Duplicate(result);

end;
}
function tStorageAction.DoAction(var WorkData:tWorkData):LRESULT;
var
  num:integer;
  st,dt:pByte;
  sv,dv:^uint_ptr;
begin
  result:=0;

  if Number in [0..9] then
  begin
    // copy to slot
    if (flags and ACF_COPYFROM)=0 then
    begin
      num:=Number;
      st:=@WorkData.ResultType;
      dt:=@WorkData.Storage[Number].rtype;
      sv:=@WorkData.LastResult;
      dv:=@WorkData.Storage[Number].value;
    end
    // copy from slot
    else
    begin
      num:=-1;
      dt:=@WorkData.ResultType;
      st:=@WorkData.Storage[Number].rtype;
      dv:=@WorkData.LastResult;
      sv:=@WorkData.Storage[Number].value;
    end;

    ClearResult(WorkData,num);
    dt^:=st^;

    if WorkData.ResultType=rtInt then // Number
      dv^:=sv^

    else if WorkData.ResultType=rtWide then // Unicode
      StrDupW(pWideChar(dv^),pWideChar(sv^))
    
    else if WorkData.ResultType<>rtUnkn then // Ansi and UTF8
      StrDup(pAnsiChar(dv^),pAnsiChar(sv^));
  end;

{
  // copy to slot
  if (flags and ACF_COPYFROM)=0 then
  begin
    if Number in [0..9] then
    begin
      ClearResult(WorkData,Number);
      WorkData.Storage[Number].rtype:=WorkData.ResultType;

      if WorkData.ResultType=rtInt then // Number
        WorkData.Storage[Number].value:=WorkData.LastResult

      else if WorkData.ResultType=rtWide then // Unicode
        StrDupW(pWideChar(WorkData.Storage[Number].value),pWideChar(WorkData.LastResult))
      
      else if WorkData.ResultType<>rtUnkn then // Ansi and UTF8
        StrDup(pAnsiChar(WorkData.Storage[Number].value),pAnsiChar(WorkData.LastResult));

    end;
  end
  // copy from slot
  else
  begin
    if Number in [0..9] then
    begin
      ClearResult(WorkData);
      WorkData.ResultType:=WorkData.Storage[Number].rtype;

      if WorkData.ResultType=rtInt then // Number
        WorkData.LastResult:=WorkData.Storage[Number].value

      else if WorkData.ResultType=rtWide then // Unicode
        StrDupW(pWideChar(WorkData.LastResult),pWideChar(WorkData.Storage[Number].value))
      
      else if WorkData.ResultType<>rtUnkn then // Ansi and UTF8
        StrDup(pAnsiChar(WorkData.LastResult),pAnsiChar(WorkData.Storage[Number].value));

    end;
  end;
}
end;

procedure tStorageAction.Load(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Load(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_number); Number:=DBReadByte(0,DBBranch,section,0);
    end;

    1: begin
      with xmlparser do
      begin
        if lstrcmpiw(getAttrValue(HXML(node),ioOper),ioCopy)=1 then
           flags:=flags or ACF_COPYFROM;
        Number:=StrToInt(getAttrValue(HXML(node),ioNumber));
      end;
    end;
  end;
end;

procedure tStorageAction.Save(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Save(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_number); DBWriteByte(0,DBBranch,section,Number);
    end;
{
    1: begin
    end;
}
    13: begin
      tTextExport(node).AddDWord('slot',Number);
      tTextExport(node).AddFlag ('copy',(flags or ACF_COPYFROM)<>0);
    end;
  end;
end;

//----- Dialog realization -----

procedure FillStorageCombo(wnd:HWND);
var
  i:integer;
  buf:array [0..31] of AnsiChar;
  p:pAnsiChar;
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);

  p:=StrCopyE(buf,'Slot #')-1;
  for i:=0 to 9 do
  begin
    p^:=AnsiChar(ORD('0')+i);
    InsertString(wnd,i,buf);
  end;

  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

procedure ClearFields(Dialog:HWND);
begin
  CheckDlgButton(Dialog,IDC_FLAG_TO  ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_FROM,BST_UNCHECKED);
end;

function DlgProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
begin
  result:=0;

  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);

      FillStorageCombo(GetDlgItem(Dialog,IDC_STORAGELIST));
    end;

    WM_ACT_SETVALUE: begin
      ClearFields(Dialog);
      with tStorageAction(lParam) do
      begin
        if (flags and ACF_COPYFROM)=0 then
          CheckDlgButton(Dialog,IDC_FLAG_TO  ,BST_CHECKED)
        else
          CheckDlgButton(Dialog,IDC_FLAG_FROM,BST_CHECKED);

        CB_SelectData(GetDlgItem(Dialog,IDC_STORAGELIST),Number);
      end;
    end;

    WM_ACT_RESET: begin
      ClearFields(Dialog);
      CheckDlgButton(Dialog,IDC_FLAG_TO,BST_CHECKED);
      CB_SelectData(GetDlgItem(Dialog,IDC_STORAGELIST),0);
    end;

    WM_ACT_SAVE: begin
      with tStorageAction(lParam) do
      begin
        if IsDlgButtonChecked(Dialog,IDC_FLAG_FROM)<>BST_UNCHECKED then
          flags:=flags or ACF_COPYFROM;

        Number:=CB_GetData(GetDlgItem(Dialog,IDC_STORAGELIST));
      end;
    end;
{
    WM_COMMAND: begin
      case wParam shr 16 of
      end;
    end;
}
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
  result:=tStorageAction.Create(vc.Hash);
end;

function CreateDialog(parent:HWND):HWND;
begin
  result:=CreateDialogW(hInstance,'IDD_ACTSTORAGE',parent,@DlgProc);
end;

procedure Init;
begin
  vc.Next    :=ModuleLink;

  vc.Name    :='Storage';
  vc.Dialog  :=@CreateDialog;
  vc.Create  :=@CreateAction;
  vc.Icon    :='IDI_STORAGE';

  ModuleLink :=@vc;
end;

begin
  Init;
end.

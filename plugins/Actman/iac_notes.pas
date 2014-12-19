unit iac_notes;

interface

implementation

uses
  windows, messages, commctrl,
  common, wrapper,
  mirutils, dbsettings, m_api,
  global, iac_global;

{$include i_cnst_notes.inc}
{$resource iac_notes.res}

const
  opt_text = 'text';

type
  tNotesAction = class(tBaseAction)
  private
    note:PWideChar;
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

constructor tNotesAction.Create(uid:dword);
begin
  inherited Create(uid);
end;

destructor tNotesAction.Destroy;
begin

  inherited Destroy;
end;

function tNotesAction.DoAction(var WorkData:tWorkData):LRESULT;
begin
  result:=0;
end;

procedure tNotesAction.Load(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Load(node,0);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_text); note:=DBReadUnicode(0,DBBranch,section,nil);
    end;
{
    1: begin
    end;
}
  end;
end;

procedure tNotesAction.Save(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Save(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_text); DBWriteUnicode(0,DBBranch,section,note);
    end;
{
    1: begin
    end;
}
  end;
end;

//----- Dialog realization -----

procedure ClearFields(Dialog:HWND);
begin
  SetDlgItemTextW(Dialog,IDC_TXT_TEXT,nil);
end;

function DlgProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  NoProcess:boolean=true;
begin
  result:=0;

  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);
    end;

    WM_ACT_SETVALUE: begin
      NoProcess:=true;
      ClearFields(Dialog);
      with tNotesAction(lParam) do
      begin
        SetDlgItemTextW(Dialog,IDC_TXT_TEXT,note);
      end;
      NoProcess:=false;
    end;

    WM_ACT_RESET: begin
      NoProcess:=true;
      ClearFields(Dialog);
      NoProcess:=false;
    end;

    WM_ACT_SAVE: begin
      with tNotesAction(lParam) do
      begin
        flags:=0;
        note:=GetDlgText(Dialog,IDC_TXT_TEXT);
      end;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        EN_CHANGE: if not NoProcess then
            SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
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
  result:=tNotesAction.Create(vc.Hash);
end;

function CreateDialog(parent:HWND):HWND;
begin
  result:=CreateDialogW(hInstance,'IDD_ACTNOTES',parent,@DlgProc);
end;

procedure Init;
begin
  vc.Next    :=ModuleLink;

  vc.Name    :='Notes';
  vc.Dialog  :=@CreateDialog;
  vc.Create  :=@CreateAction;
  vc.Icon    :='IDI_NOTES';

  ModuleLink :=@vc;
end;

begin
  Init;
end.

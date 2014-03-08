unit iac_;

interface

implementation

uses windows, iac_global, mirutils;


type
   = class(tBaseAction)

    constructor Create(uid:dword);
    destructor Destroy; override;
    function  Clone:tBaseAction; override;
    function  DoAction(var WorkData:tWorkData):LRESULT; override;
    procedure Save(node:pointer;fmt:integer); override;
    procedure Load(node:pointer;fmt:integer); override;
  end;

//----- Support functions -----

//----- Object realization -----

constructor .Create(uid:dword);
begin
  inherited Create(uid);
end;

destructor .Destroy;
begin

  inherited Destroy;
end;

function .Clone:tBaseAction;
begin
  result:=.Create(0);
  Duplicate(result);

end;

function .DoAction(var WorkData:tWorkData):LRESULT;
begin
  result:=0;
end;

procedure .Load(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Load(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
    end;
{
    1: begin
    end;
}
  end;
end;

procedure .Save(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Save(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
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
end;

function DlgProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
begin
  result:=0;

  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);
    end;

    WM_ACT_SETVALUE: begin
      ClearFields(Dialog);
      with (lParam) do
      begin
      end;
    end;

    WM_ACT_RESET: begin
      ClearFields(Dialog);
    end;

    WM_ACT_SAVE: begin
      with (lParam) do
      begin
      end;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
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
  result:=.Create(vc.Hash);
end;

function CreateDialog(parent:HWND):HWND;
begin
  result:=CreateDialogW(hInstance,,parent,@DlgProc);
end;

procedure Init;
begin
  vc.Next    :=ModuleLink;

  vc.Name    :=;
  vc.Dialog  :=@CreateDialog;
  vc.Create  :=@CreateAction;
  vc.Icon    :=;

  ModuleLink :=@vc;
end;

begin
  Init;
end.

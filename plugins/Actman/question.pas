unit question;

interface
uses windows,messages;

function QuestionDlg(Dialog:HWnd;hMessage:UINT;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;

implementation

uses m_api;

{$include i_const.inc}

const
  imp_yes    = 1;
  imp_yesall = 2;
  imp_no     = 3;
  imp_noall  = 4;
  imp_append = 5;

function QuestionDlg(Dialog:HWnd;hMessage:UINT;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  i:integer;
begin
  result:=0;
  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);
      SetDlgItemTextW(Dialog, IDC_ASK,pWideChar(lParam));
      result:=1;
    end;
    WM_COMMAND: begin
      case loword(wParam) of
        IDOK      : i:=imp_yes;
        IDCANCEL  : i:=imp_no;
        IDC_YESALL: i:=imp_yesall;
        IDC_NOALL : i:=imp_noall;
        IDC_APPEND: i:=imp_append;
      else
        i:=0;
      end;
      if i<>0 then
      begin
        EndDialog(Dialog,i);
        result:=1;
      end;
    end;
  end;
end;

end.

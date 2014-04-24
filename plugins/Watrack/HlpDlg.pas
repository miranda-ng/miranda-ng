{help dialogs}
unit HlpDlg;

interface

uses windows;

const
  sFormatHelp:PWideChar = 'Text format codes'#13#10'{b}text{/b}'#9'bold'#13#10+
  '{i}text{/i}'#9'italic'#13#10'{u}text{/u}'#9'undeline'#13#10+
  '{cf##}text{/cf}'#9'text color'#13#10'{bg##}text{/bg}'#9+
  'background color'#13#10'text - user text'#13#10+
  '## - color number (1-16)'#13#10'Color 0 is background color'#13#10+
  'Color 17 is default text color';

function ShowColorHelpDlg(parent:HWND):integer;

implementation

uses messages,m_api;

{$include res\i_const.inc}

const
  colors:array [0..15] of dword = (
    $00FFFFFF,$00000000,$007F0000,$00009300,
    $000000FF,$0000007F,$009C009C,$00007FFC,
    $0000FFFF,$0000FC00,$00939300,$00FFFF00,
    $00FC0000,$00FF00FF,$007F7F7F,$00D2D2D2
   );

const
  COLORDLG = 'COLOR';

function ColorHelpDlg(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  ps:tPaintStruct;
  br:hBrush;
  dc:hDC;
  rc:tRect;
  i,j:integer;
begin
  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);
    end;
    WM_COMMAND:
      if (wParam shr 16)=BN_CLICKED then
        case loword(wParam) of
          IDOK, IDCANCEL: DestroyWindow(Dialog);
        end;
    WM_PAINT: begin
      dc:=BeginPaint(Dialog,ps);
      SetBkColor(dc,GetSysColor(COLOR_BTNFACE));
      for i:=0 to 1 do
      begin
        for j:=0 to 7 do
        begin
          with rc do
          begin
            left  :=32+i*66;
            top   :=10+j*18;
            right :=56+i*66;
            bottom:=24+j*18;
          end;
          br:=CreateSolidBrush(colors[i*8+j]);
          Rectangle(dc,rc.left-1,rc.top-1,rc.right+1,rc.bottom+1);
          FillRect(dc,rc,br);
          DeleteObject(br);
        end;
      end;
      EndPaint(Dialog,ps);
    end;
  end;
  result:=0;
end;

function ShowColorHelpDlg(parent:HWND):integer;
begin
  result:=CreateDialogW(hInstance,COLORDLG,parent,@ColorHelpDlg);
end;

end.

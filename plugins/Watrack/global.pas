{WATrack global datas}
unit Global;

interface

uses windows,messages,wat_api;

const
  hwndTooltip:HWND=0;

var
  UserCP:dword;

const
  DLGED_INIT = $1000; // dialog init, not activate Apply button

const
  dsWait      = -1;
  dsEnabled   = 0;
  dsTemporary = 1;
  dsPermanent = 2;

// --- type definition ---
type
  pwModule = ^twModule;
  twModule = record
    Next      :pwModule;
    Init      :function(aGetStatus:boolean=false):integer;
    DeInit    :procedure(aSetDisable:boolean);
    AddOption :function(var tmpl:pAnsiChar;var proc:pointer;var name:pAnsiChar):integer;
    Check     :function(load:boolean):boolean;
    ModuleName:pWideChar;
    ModuleStat:integer; // filling by the way
    Button    :HWND;    // checkboxes for switch on/off
//    AddOption:function(parent:HWND;var Dlg:integer;var name:pWideChar):integer;
  end;

const
  PluginName  = 'Winamp Track';
  PluginShort:PAnsiChar = 'WATrack';

const
  ModuleLink:pwModule=nil;

const
  DisablePlugin :integer=0;
  hHookWATStatus:THANDLE=0;

// --- global functions ---

procedure MakeHint (wnd:HWND;id:integer;txt:pAnsiChar);
procedure MakeHintW(wnd:HWND;id:integer;txt:pWideChar);

implementation

uses common,commctrl,mirutils,m_api;//,templates;

procedure MakeHint(wnd:HWND;id:integer;txt:pAnsiChar);
var
  ti:TTOOLINFOW;
begin
//  FillChar(ti,SizeOf(ti),0);
  ti.cbSize  :=sizeof(TTOOLINFOW);
  ti.uFlags  :=TTF_IDISHWND or TTF_SUBCLASS;
  ti.hwnd    :=wnd;
  ti.hinst   :=hInstance;
  ti.uId     :=GetDlgItem(wnd,id);
  ti.lpszText:=TranslateA2W(txt);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
  mFreeMem(ti.lpszText);
end;

procedure MakeHintW(wnd:HWND;id:integer;txt:pWideChar);
var
  ti:TTOOLINFOW;
begin
//  FillChar(ti,SizeOf(ti),0);
  ti.cbSize  :=sizeof(TTOOLINFOW);
  ti.uFlags  :=TTF_IDISHWND or TTF_SUBCLASS;
  ti.hwnd    :=wnd;
  ti.hinst   :=hInstance;
  ti.uId     :=GetDlgItem(wnd,id);
  ti.lpszText:=TranslateW(txt);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
end;

end.

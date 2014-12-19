unit hooks;

interface

implementation

uses
  windows, commctrl, messages,
  mirutils, common, dbsettings, m_api, wrapper,
  global, mApiCardM;

{$R hooks.res}

{$include i_hook.inc}
{$include i_hconst.inc}
{$include i_options.inc}
{$include i_opt_dlg.inc}

// ------------ base interface functions -------------

procedure Init;
begin

  MessageWindow:=CreateWindowExW(0,'STATIC',nil,0,1,1,1,1,HWND_MESSAGE,0,hInstance,nil);
  if MessageWindow<>0 then
    SetWindowLongPtrW(MessageWindow,GWL_WNDPROC,LONG_PTR(@HookWndProc));

  if LoadHooks=0 then
  begin
    MaxHooks:=8;
    GetMem  (HookList ,MaxHooks*SizeOf(tHookRec));
    FillChar(HookList^,MaxHooks*SizeOf(tHookRec),0);
  end
  else
    SetAllHooks;
end;

procedure DeInit;
begin
  ClearHooks;
  if MessageWindow<>0 then
    DestroyWindow(MessageWindow);
end;

function AddOptionPage(var tmpl:pAnsiChar;var proc:pointer;var name:PAnsiChar):integer;
begin
  result:=0;
  tmpl:=PAnsiChar(IDD_HOOKS);
  proc:=@DlgProcOpt;
  name:='Hooks';
end;

var
  amLink:tActionLink;

procedure InitLink;
begin
  amLink.Next     :=ActionLink;
  amLink.Init     :=@Init;
  amLink.DeInit   :=@DeInit;
  amLink.AddOption:=@AddOptionPage;
  ActionLink      :=@amLink;
end;

initialization
  InitLink;
end.

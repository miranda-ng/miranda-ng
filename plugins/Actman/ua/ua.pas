unit ua;

interface

procedure Init;
procedure DeInit;
function AddOptionPage(var tmpl:pAnsiChar;var proc:pointer;var name:PAnsiChar):integer;

implementation

uses
  windows, commctrl, messages,
  mirutils, common, dbsettings, io, m_api, wrapper,
  global;

{$R ua.res}

{$include m_actman.inc}

{$include i_uconst.inc}
{$include i_uavars.inc}

// in - Action ID, out - action (group) number
function GetUABranch(setting:pAnsiChar;id:cardinal):pAnsiChar;
var
  i:integer;
  p,p1:pAnsiChar;
begin
  result:=nil;
  p1:=StrCopyE(setting,opt_groups);
  for i:=0 to CallService(MS_ACT_GETLIST,0,0)-1 do
  begin
    p:=StrEnd(IntToStr(p1,i));
    p^:='/'; inc(p);
    StrCopy(p,opt_id);
    if DBReadDWord(0,DBBranch,setting)=id then
    begin
      p^:=#0;
      result:=p;
      break;
    end;
  end;
end;

var
  amLink:tActionLink;

{$include i_uaplaces.inc}
{$include i_options.inc}
{$include i_opt_dlg.inc}
{$include i_ua.inc}
{$include i_inoutxm.inc}

// ------------ base interface functions -------------

var
  iohook:THANDLE;
  hontabloaded,
  honttbloaded,
  ontabbtnpressed,
  onactchanged:THANDLE;
  hPreBuildMMenu,
  hPreBuildCMenu,
  hPreBuildTMenu:THANDLE;

procedure Init;
begin
  GetModuleFileNameW(hInstance,szMyPath,MAX_PATH);

  hServiceWithLParam:=CreateServiceFunction(SERVICE_WITH_LPARAM_NAME,@ServiceCallWithLParam);
  hTTBService       :=CreateServiceFunction(TTB_SERVICE_NAME        ,@TTBServiceCall);
  CheckPlacesAbility;

  CreateUActionList;

  honttbloaded   :=HookEvent(ME_TTB_MODULELOADED ,@OnTTBLoaded);
  hontabloaded   :=HookEvent(ME_MSG_TOOLBARLOADED,@OnTabBBLoaded);
  ontabbtnpressed:=HookEvent(ME_MSG_BUTTONPRESSED,@OnTabButtonPressed);
  onactchanged   :=HookEvent(ME_ACT_CHANGED      ,@ActListChange);

  hPreBuildMMenu:=HookEvent(ME_CLIST_PREBUILDMAINMENU   , PreBuildMainMenu);
  hPreBuildCMenu:=HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);
  hPreBuildTMenu:=HookEvent(ME_CLIST_PREBUILDTRAYMENU   , PreBuildTrayMenu);

  iohook:=HookEvent(ME_ACT_INOUT,@ActInOut);
end;

procedure DeInit;
begin
  SetLength(arMenuRec,0);

  UnhookEvent(hPreBuildMMenu);
  UnhookEvent(hPreBuildCMenu);
  UnhookEvent(hPreBuildTMenu);

  UnhookEvent(honttbloaded);
  UnhookEvent(hontabloaded);
  UnhookEvent(ontabbtnpressed);
  UnhookEvent(onactchanged);
  UnhookEvent(iohook);
  DestroyServiceFunction(hServiceWithLParam);
  DestroyServiceFunction(hTTBService);
end;

function AddOptionPage(var tmpl:pAnsiChar;var proc:pointer;var name:PAnsiChar):integer;
begin
  result:=0;
  tmpl:=PAnsiChar(IDD_UA);
  proc:=@DlgProcOpt;
  name:='Use Actions';
end;

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

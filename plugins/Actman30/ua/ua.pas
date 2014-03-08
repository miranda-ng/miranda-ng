unit ua;

interface

implementation

uses
  windows, commctrl, messages,
  mirutils, common, dbsettings, io, m_api, wrapper, editwrapper,
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

procedure CheckPlacesAbility;
var
  i:integer;
begin
  for i:=0 to NumTypes-1 do
  begin
    with NamesArray[i] do
    begin
      enable:=(service=nil) or (ServiceExists(service)<>0);
    end;
  end;
end;

procedure Init;
begin
  GetModuleFileNameW(hInstance,szMyPath,MAX_PATH);

  CreateServiceFunction(SERVICE_WITH_LPARAM_NAME,@ServiceCallWithLParam);
  CreateServiceFunction(TTB_SERVICE_NAME        ,@TTBServiceCall);
  CheckPlacesAbility;

  CreateUActionList;

  HookEvent(ME_TTB_MODULELOADED ,@OnTTBLoaded);
  HookEvent(ME_MSG_TOOLBARLOADED,@OnTabBBLoaded);
  HookEvent(ME_MSG_BUTTONPRESSED,@OnTabButtonPressed);
  HookEvent(ME_ACT_CHANGED      ,@ActListChange);

  HookEvent(ME_CLIST_PREBUILDMAINMENU   , PreBuildMainMenu);
  HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);
  HookEvent(ME_CLIST_PREBUILDTRAYMENU   , PreBuildTrayMenu);

  HookEvent(ME_ACT_INOUT,@ActInOut);
end;

procedure DeInit;
var
  i:integer;
begin
  if Length(UActionList)>0 then
  begin
    for i:=HIGH(UActionList) downto 0 do
    begin
      DeleteUAction(i,false);
    end;
    SetLength(UActionList,0);
  end;
  SetLength(arMenuRec,0);
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

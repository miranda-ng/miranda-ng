{$include compilers.inc}
{$IFDEF COMPILER_16_UP}
  {$WEAKLINKRTTI ON}
  {.$RTTI EXPLICIT METHODS([]) PROPERTIES([]) FIELDS([])}
{$ENDIF}
{$IMAGEBASE $13200000}
library actman;
{%File 'i_const.inc'}
{%File 'i_opt_dlg2.inc'}
{%File 'i_opt_dlg.inc'}
{%File 'i_options.inc'}
{%File 'i_services.inc'}
{%File 'i_vars.inc'}
{%File 'i_inoutxm.inc'}
{%File 'tasks\i_opt_dlg.inc'}
{%File 'tasks\i_options.inc'}
{%File 'tasks\i_task.inc'}
{%File 'hooks\i_options.inc'}
{%File 'hooks\i_hook.inc'}
{%File 'hooks\i_opt_dlg.inc'}
{%File 'ua\i_opt_dlg.inc'}
{%File 'ua\i_inoutxm.inc'}
{%File 'ua\i_options.inc'}
{%File 'ua\i_ua.inc'}
{%File 'ua\i_uaplaces.inc'}
{%File 'ua\i_uconst.inc'}

uses
//  fastmm4,
  Windows,
  m_api,
  messages,
  commctrl,
  common,
  wrapper,
  dbsettings,
  mirutils,
  awkservices,
  global,
  lowlevelc,
  dlgshare,
  iac_global,
  iac_settings,
  iac_storage,
  iac_dbrw,
  iac_messagebox,
  iac_text,
  iac_jump,
  iac_inout,
  iac_service,
  iac_program,
  iac_chain,
  iac_contact,
  iac_call,
  iac_ini,
  iac_notes,
  ua in 'ua\ua.pas',
  hooks in 'hooks\hooks.pas',
  scheduler in 'tasks\scheduler.pas';

{$r options.res}
{$r version.res}

const
  PluginName  = 'Action Manager';
var
  hevaction,hHookChanged,hevinout:THANDLE;

{$include i_const.inc}

{$include i_options.inc}
{$include i_services.inc}
{$include i_opt_dlg.inc}
{.$include i_inoutxm.inc}

function PreShutdown(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  ptr:pActionLink;
begin
  result:=0;

  ptr:=ActionLink;
  while ptr<>nil do
  begin
    if @ptr^.DeInit<>nil then
      ptr^.DeInit;
    ptr:=ptr^.Next;
  end;

  MacroList.Clear;
  MacroList.Free;

  DestroyHookableEvent(hHookChanged);
  DestroyHookableEvent(hevinout);
  DestroyHookableEvent(hevaction);

end;

procedure RegisterActTypes;
var
  p:pActModule;
  sid:TSKINICONDESC;
  buf:array [0..63] of AnsiChar;
  pc:pAnsiChar;
begin
  FillChar(sid,SizeOf(sid),0);
  sid.cx:=16;
  sid.cy:=16;
  sid.szSection.a:='Actions';
  sid.pszName    :=@buf;
  pc:=StrCopyE(buf,IcoLibPrefix);
  p:=ModuleLink;

  while p<>nil do
  begin
    if p^.Hash=0 then
      p^.Hash:=Hash(p^.Name,StrLen(p^.Name));
    
    sid.hDefaultIcon   :=LoadImageA(hInstance,p^.Icon,IMAGE_ICON,16,16,0);
    sid.szDescription.a:=p^.Name;
    StrCopy(pc,p^.Name);
    Skin_AddIcon(@sid);
 
   DestroyIcon(sid.hDefaultIcon);

    p:=p^.Next;
  end;
end;

// This function implements autostart action execution after all others plugins loading
function DoAutostart(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  i:integer;
  Macro:pMacroRecord;
  section:array [0..127] of AnsiChar;
  p,p1:pAnsiChar;
begin
  Result:=0;
  CallService(MS_ACT_RUNBYNAME,TWPARAM(AutoStartName),0);

  p1:=StrCopyE(section,opt_group);
  for i:=0 to MacroList.Count-1 do
  begin
    Macro:=MacroList[i];
    if (Macro^.flags and ACF_FIRSTRUN)<>0 then
    begin
      CallService(MS_ACT_RUNBYID,TWPARAM(Macro^.id),0);
      Macro^.flags:=Macro^.flags and not ACF_FIRSTRUN;
      p:=StrEnd(IntToStr(p1,i));
      p^:='/'; inc(p);
      StrCopy(p,opt_flags); DBWriteDWord(0,DBBranch,section,Macro^.flags);
    end;
  end;
end;

function DoAddButton(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  Srmm_AddButton(pBBButton(lParam));
  result := 0;
end;

function DoCallContactService(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  ccs:PCCSDATA;
begin
  ccs := PCCSDATA(lParam);
  result := Proto_ChainSend(0, ccs);
end;

function DoOpenUrl(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  Utils_OpenUrl(PAnsiChar(lParam), byte(wParam));
  result := 0;
end;

function DoSetStatus(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  Clist_SetStatusMode(wParam);
  result := 0;
end;

function OnModulesLoaded(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  ptr:pActionLink;
begin
  Result:=0;

  RegisterActTypes;

  LoadMacros;
  RegisterIcons;

  HookEvent(ME_OPT_INITIALISE ,@OnOptInitialise);
  HookEvent(ME_SYSTEM_PRESHUTDOWN{ME_SYSTEM_OKTOEXIT},@PreShutdown);
  NotifyEventHooks(hHookChanged,twparam(ACTM_LOADED),0);

  IsMultiThread:=true;
  // Load additional modules
  ptr:=ActionLink;
  while ptr<>nil do
  begin
    if @ptr^.Init<>nil then
      ptr^.Init;
    ptr:=ptr^.Next;
  end;

  // cheat
  HookEvent(ME_SYSTEM_MODULESLOADED,@DoAutostart);
end;

function Load:int; cdecl;
begin
  Result:=0;
  Langpack_Register;

  hHookChanged:=CreateHookableEvent(ME_ACT_CHANGED);
  hevinout    :=CreateHookableEvent(ME_ACT_INOUT);
  hevaction   :=CreateHookableEvent(ME_ACT_ACTION);

  CreateServiceFunction(MS_ACT_FREELIST ,@ActFreeList);
  CreateServiceFunction(MS_ACT_GETLIST  ,@ActGetList);
  CreateServiceFunction(MS_ACT_RUNBYID  ,@ActRun);
  CreateServiceFunction(MS_ACT_RUNBYNAME,@ActRunGroup);
  CreateServiceFunction(MS_ACT_RUNPARAMS,@ActRunParam);
  CreateServiceFunction(MS_ACT_SELECT   ,@ActSelect);

  CreateServiceFunction('Utils/OpenURL',@DoOpenUrl);
  CreateServiceFunction('Proto/CallContactService', @DoCallContactService);
  CreateServiceFunction('Actman/ButtonsBar/AddButton', @DoAddButton);
  CreateServiceFunction('CList/SetStatusMode', @DoSetStatus);

  HookEvent(ME_SYSTEM_MODULESLOADED,@OnModulesLoaded);
end;

function Unload: int; cdecl;
begin
  Result:=0;
end;

exports
  Load, Unload;

begin
  DisableThreadLibraryCalls(hInstance);

  PluginInfo.cbSize     :=SizeOf(TPLUGININFOEX);
  PluginInfo.shortName  :='Action manager';
  PluginInfo.version    :=$00030001;
  PluginInfo.description:='Plugin for manage hotkeys to open contact window, insert text, '+
                          'run program and call services';
  PluginInfo.author     :='Awkward';
  PluginInfo.copyright  :='(c) 2007-13 Awkward';
  PluginInfo.homepage   :='https://miranda-ng.org/p/Actman';
  PluginInfo.flags      :=UNICODE_AWARE;
  PluginInfo.uuid       :=MIID_ACTMAN;
end.

library testdll;

uses
  m_api, Windows;

var
  PluginInterfaces:array [0..1] of MUUID;

function MirandaPluginInfoEx(mirandaVersion:dword):PPLUGININFOEX; cdecl;
begin
  result:=@PluginInfo;
  PluginInfo.cbSize     :=SizeOf(TPLUGININFOEX);
  PluginInfo.shortName  :='Plugin Template';
  PluginInfo.version    :=$00000001;
  PluginInfo.description:='The long description of your plugin, to go in the plugin options dialog';
  PluginInfo.author     :='J. Random Hacker';
  PluginInfo.authorEmail:='noreply@sourceforge.net';
  PluginInfo.copyright  :='(c) 2003 J. Random Hacker';
  PluginInfo.homepage   :='http://miranda-icq.sourceforge.net/';
  PluginInfo.flags      :=UNICODE_AWARE;
  PluginInfo.uuid       :=MIID_TESTPLUGIN;//'{08B86253-EC6E-4d09-B7A9-64ACDF0627B8}';
end;

function PluginMenuCommand(wParam: WPARAM; lParam: LPARAM):int_ptr; cdecl;
begin
  Result:=0;
  // this is called by Miranda, thus has to use the cdecl calling convention
  // all services and hooks need this.
  MessageBox(0, 'Just groovy, baby!', 'Plugin-o-rama', MB_OK);
end;

var
  onloadhook:THANDLE;

function OnModulesLoaded(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  mi:TCListMenuItem;
begin
  Result:=0;
  UnhookEvent(onloadhook);

  CreateServiceFunction('TestPlug/MenuCommand', @PluginMenuCommand);
  FillChar(mi,SizeOf(mi),0);
  mi.cbSize    :=SizeOf(mi);
  mi.position  :=$7FFFFFFF;
  mi.flags     :=0;
  mi.hIcon     :=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
  mi.szName.a  :='&Test Plugin...';
  mi.pszService:='TestPlug/MenuCommand';
  Menu_AddMainMenuItem(@mi)
end;

function Load():int; cdecl;
begin
  Langpack_register;
  onloadhook:=HookEvent(ME_SYSTEM_MODULESLOADED,@OnModulesLoaded);

  Result:=0;
end;

function Unload:int; cdecl;
begin
  Result:=0;
end;

function MirandaPluginInterfaces:PMUUID; cdecl;
begin
  PluginInterfaces[0]:=MIID_TESTPLUGIN;
  PluginInterfaces[1]:=MIID_LAST;
  result:=@PluginInterfaces;
end;

exports
  Load, Unload,
  MirandaPluginInterfaces,MirandaPluginInfoEx;

begin
end.

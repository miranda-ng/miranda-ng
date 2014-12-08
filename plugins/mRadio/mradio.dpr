{.$DEFINE CHANGE_NAME_BUFFERED}
{$include compilers.inc}
{$IFDEF COMPILER_16_UP}
  {$WEAKLINKRTTI ON}
  {.$RTTI EXPLICIT METHODS([]) PROPERTIES([]) FIELDS([])}
{$ENDIF}
{$IMAGEBASE $13300000}
library mradio;

uses
//  FastMM4,
  Windows,messages,commctrl
  ,common,io,wrapper,wrapdlgs,syswin
  ,Dynamic_Bass
  ,m_api,dbsettings,mirutils,mircontacts,memini,
  rbass,rglobal,rccenter,rframeapi,roptions;

{$include mr_rc.inc}
{$r mradio.res}

{$include i_search.inc}
{$include i_variables.inc}
{$include i_service.inc}
{$include i_myservice.inc}
{$include i_hotkey.inc}
{$include i_tray.inc}
{$include i_visual.inc}
{$include i_optdlg.inc}

function MirandaPluginInfoEx(mirandaVersion:dword):PPLUGININFOEX; cdecl;
begin
  result:=@PluginInfo;
  PluginInfo.cbSize     :=SizeOf(TPLUGININFOEX);
  PluginInfo.shortName  :='mRadio Mod';
  PluginInfo.version    :=$00000204;
  PluginInfo.description:='This plugin plays and records Internet radio streams.'+
                          ' Also local media files can be played.';
  PluginInfo.author     :='Awkward';
  PluginInfo.authorEmail:='panda75@bk.ru; awk1975@ya.ru';
  PluginInfo.copyright  :='(c) 2007-2014 Awkward';
  PluginInfo.homepage   :='http://code.google.com/p/delphi-miranda-plugins/';
  PluginInfo.flags      :=UNICODE_AWARE;
  PluginInfo.uuid       :=MIID_MRADIO;
end;

function OnModulesLoaded(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  nlu:TNETLIBUSER;
  buf:array [0..MAX_PATH-1] of WideChar;
  szTemp:array [0..255] of AnsiChar absolute buf;
  pc:pWideChar;
begin
  // buf = miranda directory
  // must be same as %miranda_path% of MS_UTILS_REPLACEVARS

  GetModuleFileNameW(0,buf,MAX_PATH-1);
  pc:=StrEndW(buf);
  repeat
    dec(pc);
  until pc^='\';
  inc(pc);
  pc^:=#0;

  // INI file path
  StrCopyW(pc,'plugins\mradio.ini');
  FastWideToAnsi(buf,storage);
  mGetMem(storagep,MAX_PATH+32);
  CallService(MS_DB_GETPROFILEPATH,MAX_PATH-1,tlparam(storagep));
  StrCat(storagep,'\mradio.ini');

  DBWriteDWord(0,PluginName,optVersion,PluginInfo.version); //??

  RegisterIcons;
  CreateMenu;
  CreateMIMTrayMenu;

  //-- network (for proxy)
  FillChar(nlu,SizeOf(nlu),0);
  StrCopy(szTemp,Translate('%s server connection'));
  StrReplace(szTemp,'%s',PluginName);
  nlu.szDescriptiveName.a:=szTemp;
  nlu.cbSize             :=SizeOf(nlu);
  nlu.flags              :=NUF_HTTPCONNS or NUF_NOHTTPSOPTION or NUF_OUTGOING;
  nlu.szSettingsModule   :=PluginName;
  hNetLib:=CallService(MS_NETLIB_REGISTERUSER,0,tlparam(@nlu));

//  CallService(MS_RADIO_COMMAND,MRC_RECORD,2); record off - not so necessary

  LoadPresets;
  LoadSettings;

  SetStatus(0,ID_STATUS_OFFLINE);

  CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_NOSTATION);

  IsMultiThread:=true;

  RegisterVariables;

  HookEvent(ME_DB_CONTACT_SETTINGCHANGED,@OnSettingsChanged);
  HookEvent(ME_DB_CONTACT_DELETED       ,@OnContactDeleted);
  randomize;
  CreateFrame(0);

  RegisterHotKey;

  result:=0;
end;

function PreShutdown(wParam:WPARAM;lParam:LPARAM):int;cdecl;
{
var
  buf:array [0..MAX_PATH-1] of AnsiChar;
  fdata:WIN32_FIND_DATAA;
  p:pAnsiChar;
  fi:THANDLE;
}
begin
  RemoveTrayItems;

  CallService(MS_RADIO_COMMAND,MRC_STOP,1);
  UnregisterHotKey;

  DestroyProtoServices;
  DestroyFrame();

  MyUnloadBass;

  DBWriteByte(0,PluginName,optVolume,gVolume);

  DestroyHookableEvent(hhRadioStatus);

  CallService(MS_NETLIB_CLOSEHANDLE,hNetLib,0);

  mFreeMem(storage);
  mFreeMem(storagep);

  FreePresets;
  FreeSettings;
{
  //delete cover files
  buf[0]:=#0;
  GetTempPathA(MAX_PATH,buf);
  p:=StrEnd(buf);
  StrCopy(p,'mrAvt*.*');

  fi:=FindFirstFileA(buf,fdata);
  if fi<>THANDLE(INVALID_HANDLE_VALUE) then
  begin
    repeat
      StrCopy(p,fdata.cFileName);
      DeleteFileA(buf);
    until not FindNextFileA(fi,fdata);
    FindClose(fi);
  end;
}
  result:=0;
end;

function Load(): int; cdecl;
var
  desc:TPROTOCOLDESCRIPTOR;
begin
  Langpack_register;

  // register protocol
  FillChar(desc,SizeOf(desc),0);
  desc.cbSize:=PROTOCOLDESCRIPTOR_V3_SIZE;//SizeOf(desc);
  desc.szName:=PluginName;
  desc._type :=PROTOTYPE_VIRTUAL;
  CallService(MS_PROTO_REGISTERMODULE,0,lparam(@desc));

  // hooks and services
  hhRadioStatus:=CreateHookableEvent(ME_RADIO_STATUS);

  CreateServiceFunction(MS_RADIO_PLAYSTOP ,@Service_RadioPlayStop);
  CreateServiceFunction(MS_RADIO_RECORD   ,@Service_RadioRecord);
  CreateServiceFunction(MS_RADIO_SETTINGS ,@Service_RadioSettings);
  CreateServiceFunction(MS_RADIO_SETVOL   ,@Service_RadioSetVolume);
  CreateServiceFunction(MS_RADIO_GETVOL   ,@Service_RadioGetVolume);
  CreateServiceFunction(MS_RADIO_MUTE     ,@Service_RadioMute);
  CreateServiceFunction(MS_RADIO_COMMAND  ,@ControlCenter);
  CreateServiceFunction(MS_RADIO_EQONOFF  ,@Service_EqOnOff);

  CreateServiceFunction(MS_RADIO_TRAYMENU ,@CreateTrayMenu);

  CreateServiceFunction(MS_RADIO_QUICKOPEN,@QuickOpen);

  CreateServiceFunction(MS_RADIO_EXPORT   ,@ExportAll);
  CreateServiceFunction(MS_RADIO_IMPORT   ,@ImportAll);


  CreateProtoServices;

  HookEvent(ME_SYSTEM_MODULESLOADED     ,@OnModulesLoaded);
  HookEvent(ME_SYSTEM_OKTOEXIT          ,@PreShutdown);
  HookEvent(ME_CLIST_DOUBLECLICKED      ,@Service_RadioPlayStop{@DblClickProc});
  HookEvent(ME_OPT_INITIALISE           ,@OnOptInitialise);
  HookEvent(ME_CLIST_PREBUILDCONTACTMENU,@OnContactMenu);

  PluginStatus:=ID_STATUS_OFFLINE;

  Result:=0;
end;

function Unload: int; cdecl;
begin
  Result:=0;
end;

exports
  Load, Unload,
  MirandaPluginInfoEx;

begin
  DisableThreadLibraryCalls(hInstance);
end.

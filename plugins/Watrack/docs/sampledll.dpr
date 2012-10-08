library testdll;

uses m_api, Windows,common;

{$include m_helpers.inc}
{$include m_music.inc}

const
  PluginInfo:TPLUGININFOEX=(
    cbSize     :sizeof(TPLUGININFOEX);
    shortName  :'Plugin Template';
    version    :$00000001;
    description:'The long description of your plugin, to go in the plugin options dialog';
    author     :'J. Random Hacker';
    authorEmail:'noreply@sourceforge.net';
    copyright  :'(c) 2003 J. Random Hacker';
    homepage   :'http://miranda-icq.sourceforge.net/';
    flags      :UNICODE_AWARE;
    replacesDefaultModule:0;
    uuid:'{00000000-0000-0000-0000-000000000000}'
  );
var
  PluginInterfaces:array [0..1] of MUUID;

var
  hook:integer;
  oldproc:tReadFormatProc;

// -------- format --------
Function mp3proc(var dst:tSongInfo):boolean;cdecl;
begin
{
  MP3 Handler here
}
//messagebox(0,'ok','',0);
// Example for old handler
  if (int(@oldproc)<>WAT_RES_OK) and (int(@oldproc)<>WAT_RES_ERROR) then
    result:=oldproc(dst)
  else
    result:=true;
end;

// ---------- Player ----------
var
 plwnd:HWND;

function Check(flags:integer):HWND;cdecl;
begin
  result:=1;
  plwnd:=12;
end;

function GetFileName:pWideChar;cdecl;
begin
  result:=nil;
end;

function GetInfo(var SongInfo:tSongInfo;flags:integer):integer;cdecl;
begin
  PluginLink^.CallService(MS_WAT_WINAMPINFO,integer(@SongInfo),flags);
  SongInfo.plyver:=$1234;
  result:=0;
end;

function Command(command:integer;value:integer):integer;cdecl;
begin
  result:=PluginLink^.CallService(MS_WAT_WINAMPCOMMAND,plwnd,
    command+(value shl 16));
end;

function OnWATLoaded(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  tmp:tMusicFormat;
  tmp1:tPlayerCell;
begin
  PluginLink^.UnhookEvent(hook);

  FillChar(tmp,SizeOf(tMusicFormat),0);
  lstrcpy(tmp.ext,'MP3');
  tmp.proc:=mp3proc;
  oldproc:=tReadFormatProc(PluginLink^.CallService(MS_WAT_FORMAT,
    WAT_ACT_REGISTER+WAT_ACT_REPLACE,dword(@tmp)));

  FillChar(tmp1,SizeOf(tPlayerCell),0);
  tmp1.desc   :='Sampler';
  tmp1.flags  :=0;// WAT_OPT_WINAMPAPI
  tmp1.Check  :=@Check;
  tmp1.GetInfo:=@GetInfo;
  tmp1.Command:=@Command;
  tmp1.GetName:=@GetFileName;
  PluginLink^.CallService(MS_WAT_PLAYER,WAT_ACT_REGISTER+WAT_ACT_REPLACE,dword(@tmp1));
  result:=0;
end;

function OnModuleLoaded(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  hook:=HookEvent(ME_WAT_MODULELOADED,@OnWATLoaded);
  result:=0;
end;

function MirandaPluginInfo(mirandaVersion:DWORD):PPLUGININFO; cdecl;
begin
  result:=@PluginInfo;
  PluginInfo.cbSize:=SizeOf(TPLUGININFO);
end;

function MirandaPluginInfoEx(mirandaVersion:DWORD):PPLUGININFOEX; cdecl;
begin
  result:=@PluginInfo;
  PluginInfo.cbSize:=SizeOf(TPLUGININFOEX);
end;

function Load(link: PPLUGINLINK): int; cdecl;
begin
  PLUGINLINK := Pointer(link);
  InitMMI;
  Result:=0;
  hook:=HookEvent(ME_SYSTEM_MODULESLOADED,@OnModuleLoaded);
end;

function Unload: int; cdecl;
begin
  Result:=0;
end;

function MirandaPluginInterfaces:PMUUID; cdecl;
begin
  PluginInterfaces[0]:=PluginInfo.uuid;
  PluginInterfaces[1]:=MIID_LAST;
  result:=@PluginInterfaces;
end;

exports
  Load, Unload,
  MirandaPluginInfo
  ,MirandaPluginInterfaces,MirandaPluginInfoEx;

begin
end.

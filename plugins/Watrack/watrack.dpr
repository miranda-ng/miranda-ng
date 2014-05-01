{$include compilers.inc}
{$IFDEF COMPILER_16_UP}
  {$WEAKLINKRTTI ON}
  {.$RTTI EXPLICIT METHODS([]) PROPERTIES([]) FIELDS([])}
{$ENDIF}
{$IMAGEBASE $13000000}
library WATrack;
uses
// FastMM not compatible with FPC, internal for delphi xe
//  {$IFNDEF COMPILER_16_UP}{$IFNDEF FPC}fastmm4,{$ENDIF}{$ENDIF}
  m_api,dbsettings,{activex,}winampapi,
  Windows,messages,commctrl,//uxtheme,
  srv_format,srv_player,srv_getinfo,wat_api,wrapper,
  common,syswin,HlpDlg,mirutils
  ,global,waticons,io,macros, msninfo
  ,myshows   in 'myshows\myshows.pas'
  ,lastfm    in 'lastfm\lastfm.pas'
  ,statlog   in 'stat\statlog.pas'
  ,popups    in 'popup\popups.pas'
  ,proto     in 'proto\proto.pas'
  ,status    in 'status\status.pas'
  ,tmpl      in 'status\tmpl.pas'
  ,templates in 'templates\templates.pas'
{$IFDEF KOL_MCK}
  ,kolframe  in 'kolframe\kolframe.pas'
{$ENDIF}
  {$include lst_players.inc}
  {$include lst_formats.inc}
;

{$include res\i_const.inc}

{$Resource res\watrack.res}

{$include i_vars.inc}

const
  MenuDisablePos = 500050000;

function MirandaPluginInfoEx(mirandaVersion:dword):PPLUGININFOEX; cdecl;
begin
  result:=@PluginInfo;
  PluginInfo.cbSize     :=SizeOf(TPLUGININFOEX);
  PluginInfo.shortName  :=PluginName;
  PluginInfo.version    :=$0000060C;
  PluginInfo.description:='Paste played music info into message window or status text';
  PluginInfo.author     :='Awkward';
  PluginInfo.authorEmail:='panda75@bk.ru; awk1975@ya.ru';
  PluginInfo.copyright  :='(c) 2005-2012 Awkward';
  PluginInfo.homepage   :='http://code.google.com/p/delphi-miranda-plugins/';
  PluginInfo.flags      :=UNICODE_AWARE;
  PluginInfo.uuid       :=MIID_WATRACK;
end;

{$include i_options.inc}
{$include i_timer.inc}
{$include i_gui.inc}
{$include i_opt_dlg.inc}
{$include i_cover.inc}

function ReturnInfo(enc:WPARAM;cp:LPARAM=CP_ACP):pointer;
begin
  if enc<>WAT_INF_UNICODE then
  begin
    ClearSongInfoData(tSongInfo(SongInfoA));
    move(SongInfo,SongInfoA,SizeOf(tSongInfo));
    with SongInfoA do
    begin
      if enc=WAT_INF_ANSI then
      begin
        WideToAnsi(SongInfo.artist ,artist ,cp);
        WideToAnsi(SongInfo.title  ,title  ,cp);
        WideToAnsi(SongInfo.album  ,album  ,cp);
        WideToAnsi(SongInfo.genre  ,genre  ,cp);
        WideToAnsi(SongInfo.comment,comment,cp);
        WideToAnsi(SongInfo.year   ,year   ,cp);
        WideToAnsi(SongInfo.mfile  ,mfile  ,cp);
        WideToAnsi(SongInfo.wndtext,wndtext,cp);
        WideToAnsi(SongInfo.player ,player ,cp);
        WideToAnsi(SongInfo.txtver ,txtver ,cp);
        WideToAnsi(SongInfo.lyric  ,lyric  ,cp);
        WideToAnsi(SongInfo.cover  ,cover  ,cp);
        WideToAnsi(SongInfo.url    ,url    ,cp);
      end
      else
      begin
        WideToUTF8(SongInfo.artist ,artist);
        WideToUTF8(SongInfo.title  ,title);
        WideToUTF8(SongInfo.album  ,album);
        WideToUTF8(SongInfo.genre  ,genre);
        WideToUTF8(SongInfo.comment,comment);
        WideToUTF8(SongInfo.year   ,year);
        WideToUTF8(SongInfo.mfile  ,mfile);
        WideToUTF8(SongInfo.wndtext,wndtext);
        WideToUTF8(SongInfo.player ,player);
        WideToUTF8(SongInfo.txtver ,txtver);
        WideToUTF8(SongInfo.lyric  ,lyric);
        WideToUTF8(SongInfo.cover  ,cover);
        WideToUTF8(SongInfo.url    ,url);
      end;
    end;
    result:=@SongInfoA;
  end
  else
    result:=@SongInfo;
end;

function WATReturnGlobal(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
begin
  if wParam=0 then wParam:=WAT_INF_UNICODE;
  if lParam=0 then lParam:=MirandaCP;

  result:=int_ptr(ReturnInfo(wParam,lParam));
end;

function WATGetFileInfo(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
//  si:TSongInfo;
  dst:pSongInfo;
  extw:array [0..7] of WideChar;
  f:THANDLE;
  p:PWideChar;
begin
  result:=1;
  if (lParam=0) or (pSongInfo(lParam).mfile=nil) then exit;
  dst:=pointer(lParam);

  p:=dst^.mfile;
  ClearTrackInfo(dst^);
  dst^.mfile:=p;

//  FillChar(dst,SizeOf(dst),0);
//  FillChar(si,SizeOf(si),0);
{
  if flags and WAT_INF_ANSI<>0 then
    AnsiToWide(dst^.mfile,si.mfile)
  else if flags and WAT_INF_UTF<>0 then
    UTFToWide(dst^.mfile,si.mfile)
  else
    si.mfile:=dst^.mfile;
}
  f:=Reset(dst^.mfile);
  if THANDLE(f)<>INVALID_HANDLE_VALUE then
    GetFileTime(f,nil,nil,@dst^.date);
  CloseHandle(f);
  dst^.fsize:=GetFSize(dst^.mfile);
  GetExt(dst^.mfile,extw);
  if GetFileFormatInfo(dst^)<>WAT_RES_NOTFOUND then
  begin
    with dst^ do
    begin
      if (cover=nil) or (cover^=#0) then
        GetCover(cover,mfile);
      if (lyric=nil) or (lyric^=#0) then
        GetLyric(lyric,mfile);
    end;
    result:=0;
//    ReturnInfo(si,dst,wParam and $FF);
  end;
end;

function WATGetMusicInfo(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
type
  ppointer = ^pointer;
const
  giused:cardinal=0;
var
  flags:cardinal;
  buf:PWideChar;

  newplayer,
  newstatus,
  newtrack :bool;

  OldPlayerStatus:integer;
begin
  result:=WAT_RES_NOTFOUND;
  if DisablePlugin=dsPermanent then
    exit;

  //----- Return old info if main timer -----

  if giused<>0 then
  begin
    result:=WAT_RES_OK;
    if lParam<>0 then
      ppointer(lParam)^:=ReturnInfo(wParam and $FF);
    exit;
  end;

  giused:=1;

  OldPlayerStatus:=WorkSI.status;

  newstatus:=false;
  newtrack :=false;

  //----- Checking player -----

  flags:=0;
  if CheckAll<>BST_UNCHECKED then flags:=flags or WAT_OPT_CHECKALL;

  result:=GetPlayerInfo(WorkSI,flags);

  newplayer:=result=WAT_RES_NEWPLAYER;
  if newplayer then
    result:=WAT_RES_OK;

  //----- Checking player status -----

  if result=WAT_RES_OK then
  begin
    // player stopped - no need file info
    if WorkSI.status=WAT_PLS_STOPPED then
    begin
      newstatus:=OldPlayerStatus<>WAT_PLS_STOPPED;
    end
    //----- Get file (no file, new file, maybe new) -----
    else
    begin
      // file info will be replaced (name most important only)
      flags:=0;
      if CheckUnknown<>BST_UNCHECKED then flags:=flags or WAT_OPT_UNKNOWNFMT;
      if CheckTime   <>BST_UNCHECKED then flags:=flags or WAT_OPT_CHECKTIME;
      if UseImplant  <>BST_UNCHECKED then flags:=flags or WAT_OPT_IMPLANTANT;
      if KeepOld     <>BST_UNCHECKED then flags:=flags or WAT_OPT_KEEPOLD;

      // requirement - old file name
      result:=GetFileInfo(WorkSI,flags,0);

      if (WorkSI.status=WAT_PLS_UNKNOWN ) and  // player in unknown state
         (result       =WAT_RES_NOTFOUND) then // and known media not found
        WorkSI.status:=WAT_PLS_STOPPED;

      newstatus:=OldPlayerStatus<>WorkSI.status;

      // now time for changes (window text, volume)
      // just when music presents
      if WorkSI.status<>WAT_PLS_STOPPED then
      begin
        GetChangingInfo(WorkSI,flags);
        // full info requires
        // "no music" case blocked
{??
       WorkSI.status=WAT_PLS_PLAYING and result=WAT_RES_UNKNOWN
}
        if (result=WAT_RES_NEWFILE) or           // new file
           (result=WAT_RES_UNKNOWN) or           // unknown file (enabled by flag in GetFileInfo)
           ((result=WAT_RES_OK) and              // if not new but...
           (((wParam and WAT_INF_CHANGES)=0) or  // ... ask for full info
           (StrPosW(WorkSI.mfile,'://')<>nil) or // ... or remote file
           isContainer(WorkSI.mfile))) then      // ... or container like CUE
        begin
          // requirement: old artist/title for remote files
          newtrack:=result=WAT_RES_NEWFILE;
          result:=GetInfo(WorkSI,flags);
          if not newtrack then
            newtrack:=result=WAT_RES_NEWFILE;
        end;
      end;
    end;

  end
  //----- Player not found -----
  else
  begin
    if OldPlayerStatus<>WorkSI.status then
    begin
      WorkSI.status:=WAT_PLS_NOTFOUND;
      newstatus:=true;
    end;
  end;

  //----- Copy all data to public (WorkSI to SongInfo) -----

  SongInfo.status:=WorkSI.status;

  if WorkSI.status=WAT_PLS_NOTFOUND then
  begin
    ClearSongInfoData(WorkSI);

    ClearSongInfoData(SongInfo);

    if lParam<>0 then
      ppointer(lParam)^:=nil;
  end
  else
  begin
    if not newplayer then // for another player instance
      SongInfo.plwnd:=WorkSI.plwnd
    else
    begin
      ClearPlayerInfo(SongInfo);
      CopyPlayerInfo (WorkSI,SongInfo);
    end;

    if (WorkSI.status=WAT_PLS_STOPPED) or // no music
       (result=WAT_RES_NOTFOUND) then     // or unknown media file
    begin
      ClearFileInfo    (WorkSI);
      ClearChangingInfo(WorkSI);
      ClearTrackInfo   (WorkSI);

      ClearFileInfo    (SongInfo);
      ClearChangingInfo(SongInfo);
      ClearTrackInfo   (SongInfo);
    end
    else
    begin
      ClearChangingInfo(SongInfo);
      CopyChangingInfo (WorkSI,SongInfo);

      if newtrack then
      begin
        // covers
        if (WorkSI.cover=nil) or (WorkSI.cover^=#0) then
          GetCover(WorkSI.cover,WorkSI.mfile)
        else
        begin
          mGetMem(buf,MAX_PATH*SizeOf(WideChar));
          GetTempPathW(MAX_PATH,buf);
          if StrCmpW(buf,WorkSI.cover,StrLenW(buf))=0 then
          begin
            GetExt(WorkSI.cover,StrCatEW(buf,'\wat_cover.'));
            DeleteFileW(buf);
            MoveFileW(WorkSI.cover,buf);
            mFreeMem(WorkSI.cover);
            WorkSI.cover:=buf;
          end
          else
            mFreeMem(buf);
        end;
        // lyric
        if (WorkSI.lyric=nil) or (WorkSI.lyric^=#0) then
          GetLyric(WorkSI.lyric,WorkSI.mfile);

        ClearFileInfo(SongInfo);
        CopyFileInfo (WorkSI,SongInfo);

        ClearTrackInfo(SongInfo);
        CopyTrackInfo (WorkSI,SongInfo);
      end;
    end;

    if lParam<>0 then
      ppointer(lParam)^:=ReturnInfo(wParam and $FF);

  end;

  //----- Events -----

  if newplayer then NotifyEventHooks(hHookWATStatus,WAT_EVENT_NEWPLAYER   ,tlparam(@SongInfo));
  if newstatus then NotifyEventHooks(hHookWATStatus,WAT_EVENT_PLAYERSTATUS,SongInfo.status);
  if newtrack  then NotifyEventHooks(hHookWATStatus,WAT_EVENT_NEWTRACK    ,tlparam(@SongInfo));

  giused:=0;
end;

function PressButton(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  flags:integer;
begin
  if DisablePlugin=dsPermanent then
    result:=0
  else
  begin
    flags:=0;
    if UseImplant<>BST_UNCHECKED then flags:=flags or WAT_OPT_IMPLANTANT;
    if mmkeyemu  <>BST_UNCHECKED then flags:=flags or WAT_OPT_APPCOMMAND;
    if CheckAll  <>BST_UNCHECKED then flags:=flags or WAT_OPT_CHECKALL;
    result:=SendCommand(wParam,lParam,flags);
  end;
end;

function WATPluginStatus(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  f1:integer;
begin
  if wParam=2 then
  begin
    result:=PluginInfo.version;
    exit;
  end;
  if DisablePlugin=dsPermanent then
    result:=1
  else
    result:=0;
  if (integer(wParam)<0) or (wParam=MenuDisablePos) then
  begin
    if result=0 then
      wParam:=1
    else
      wParam:=0;
  end;
  case wParam of
    0: begin
      if DisablePlugin=dsPermanent then //??
      begin
        StartTimer;
        DisablePlugin:=dsEnabled;
      end;
      f1:=0;
    end;
    1: begin
      StopTimer;
      DisablePlugin:=dsPermanent;
      f1:=CMIF_CHECKED;
    end;
  else
    exit;
  end;
  DBWriteByte(0,PluginShort,opt_disable,DisablePlugin);

  ChangeMenuIcons(f1);

  NotifyEventHooks(hHookWATStatus,WAT_EVENT_PLUGINSTATUS,DisablePlugin);
end;

function WaitAllModules(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  ptr:pwModule;
begin
  result:=0;

  CallService(MS_SYSTEM_REMOVEWAIT,wParam,0);

  ptr:=ModuleLink;
  while ptr<>nil do
  begin
    if @ptr^.Init<>nil then
      ptr^.ModuleStat:=ptr^.Init(true);
    ptr:=ptr^.Next;
  end;

  if mTimer<>0 then
    TimerProc(0,0,0,0);

  StartTimer;

  NotifyEventHooks(hHookWATLoaded,0,0);
  CloseHandle(hEvent);
end;

procedure DoTheDew(load:boolean);
var
  ptr:pwModule;
  newstate:boolean;
begin
  ptr:=ModuleLink;
  while ptr<>nil do
  begin
    if @ptr^.Check<>nil then
      ptr^.Check(load);
    ptr:=ptr^.Next;
  end;

  // TTB
  newstate:=ServiceExists(MS_TTB_ADDBUTTON)<>0;
  if newstate=(ttbState<>0) then
    exit;

  if ttbState=0 then
  begin
{
    onloadhook:=0;
    OnTTBLoaded(0,0);
    if ttbState=0 then
}
    HookEvent(ME_TTB_MODULELOADED,@OnTTBLoaded);
  end
  else
  begin
    if ServiceExists(MS_TTB_REMOVEBUTTON)>0 then
      CallService(MS_TTB_REMOVEBUTTON,WPARAM(ttbState),0);
    ttbState:=0;
  end;
end;

function OnPluginLoad(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  DoTheDew(true);
  result:=0;
end;

function OnPluginUnload(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  DoTheDew(false);
  result:=0;
end;

type
  tdbetd = record
    flags:dword;
    event:int;
    descr:pAnsiChar;
  end;

const
  cdbetd: array [0..3] of tdbetd = (
    (flags:DETF_HISTORY or DETF_MSGWINDOW; event:EVENTTYPE_WAT_REQUEST; descr:'WATrack: information request'),
    (flags:DETF_HISTORY or DETF_MSGWINDOW; event:EVENTTYPE_WAT_ANSWER ; descr:nil),
    (flags:DETF_HISTORY or DETF_MSGWINDOW; event:EVENTTYPE_WAT_ERROR  ; descr:'WATrack: request denied'),
    (flags:DETF_HISTORY or DETF_NONOTIFY ; event:EVENTTYPE_WAT_MESSAGE; descr:nil)
  );

function OnModulesLoaded(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  p:PAnsiChar;
  dbetd:TDBEVENTTYPEDESCR;
  i:integer;
begin

  hTimer:=0;

//!!  OleInitialize(nil);

  if RegisterIcons then
    HookEvent(ME_SKIN2_ICONSCHANGED,@IconChanged);

  CreateMenus;

  if ServiceExists(MS_TTB_ADDBUTTON)<>0 then
    HookEvent(ME_TTB_MODULELOADED,@OnTTBLoaded)
  else
    ttbState:=0;

  ProcessFormatLink;
  ProcessPlayerLink;
  // next 2 blocks here coz GetAddonFileName uses some Miranda things
  p:=GetAddonFileName(nil,'player','plugins','ini');
  if p<>nil then
  begin
    LoadFromFile(p);
    mFreeMem(p);
  end;

  p:=GetAddonFileName(nil,'watrack_icons','icons','dll');
  if p<>nil then
  begin
    SetPlayerIcons(p);
    mFreeMem(p);
  end;

  IsMultiThread:=true;

  // Register WATrack events
  dbetd.cbSize     :=SizeOf(TDBEVENTTYPEDESCR);
  dbetd.module     :=PluginShort;
  dbetd.textService:=nil;
  dbetd.iconService:=nil;
  dbetd.eventIcon  :=0;

  for i:=0 to HIGH(cdbetd) do
  begin
    dbetd.flags      :=cdbetd[i].flags;
    dbetd.eventType  :=cdbetd[i].event;
    dbetd.descr      :=cdbetd[i].descr;
    CallService(MS_DB_EVENT_REGISTERTYPE,0,TLPARAM(@dbetd));
  end;

  // Load WATrack modules
  hEvent:=CreateEvent(nil,true,true,nil);
  if hEvent<>0 then
  begin
    p:='WAT_INIT';
    CreateServiceFunction(p,@WaitAllModules);
    CallService(MS_SYSTEM_WAITONHANDLE,hEvent,tlparam(p));
  end;

  loadopt;
  if DisablePlugin=dsPermanent then
    CallService(MS_WAT_PLUGINSTATUS,1,0);

  StartMSNHook;

  result:=0;

  HookEvent(ME_SYSTEM_MODULELOAD  ,@OnPluginLoad);
  HookEvent(ME_SYSTEM_MODULEUNLOAD,@OnPluginUnLoad);
end;

procedure FreeVariables;
begin
  ClearSongInfoData(SongInfo);
  ClearSongInfoData(tSongInfo(SongInfoA));
  ClearSongInfoData(WorkSI);
  mFreeMem(CoverPaths);
  ClearFormats;
  ClearPlayers;
end;

function PreShutdown(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  buf:array [0..511] of WideChar;
  fdata:WIN32_FIND_DATAW;
  fi:THANDLE;
  p:PWideChar;
  ptr:pwModule;
begin
  StopMSNHook;

  NotifyEventHooks(hHookWATStatus,WAT_EVENT_PLAYERSTATUS,WAT_PLS_NOTFOUND);

  if hwndTooltip<>0 then
    DestroyWindow(hwndTooltip);

  if ttbState<>0 then
  begin
    if ServiceExists(MS_TTB_REMOVEBUTTON)>0 then
      CallService(MS_TTB_REMOVEBUTTON,TWPARAM(ttbState),0);
    ttbState:=0;
  end;

  StopTimer;
  ptr:=ModuleLink;
  while ptr<>nil do
  begin
    if @ptr^.DeInit<>nil then
      ptr^.DeInit(false);
    ptr:=ptr^.Next;
  end;

  FreeVariables;

  DestroyHookableEvent(hHookWATLoaded);
  DestroyHookableEvent(hHookWATStatus);

//!!  OleUnInitialize;

  //delete cover files
  buf[0]:=#0;
  GetTempPathW(511,buf);
  p:=StrEndW(buf);
  StrCopyW(p,'wat_cover.*');

  fi:=FindFirstFileW(buf,fdata);
  if fi<>THANDLE(INVALID_HANDLE_VALUE) then
  begin
    repeat
      StrCopyW(p,fdata.cFileName);
      DeleteFileW(buf);
    until not FindNextFileW(fi,fdata);
    FindClose(fi);
  end;

  result:=0;
end;

function Load():int; cdecl;
begin
  result:=0;
  Langpack_register;

  DisablePlugin:=dsPermanent;

  hHookWATLoaded:=CreateHookableEvent(ME_WAT_MODULELOADED);
  hHookWATStatus:=CreateHookableEvent(ME_WAT_NEWSTATUS);

  HookEvent(ME_SYSTEM_OKTOEXIT,@PreShutdown);
  HookEvent(ME_OPT_INITIALISE ,@OnOptInitialise);

  CreateServiceFunction(MS_WAT_GETFILEINFO  ,@WATGetFileInfo);
  CreateServiceFunction(MS_WAT_RETURNGLOBAL ,@WATReturnGlobal);

  CreateServiceFunction(MS_WAT_GETMUSICINFO ,@WATGetMusicInfo);
  CreateServiceFunction(MS_WAT_PLUGINSTATUS ,@WATPluginStatus);
  CreateServiceFunction(MS_WAT_PRESSBUTTON  ,@PressButton);
  CreateServiceFunction(MS_WAT_WINAMPINFO   ,@WinampGetInfo);
  CreateServiceFunction(MS_WAT_WINAMPCOMMAND,@WinampCommand);

  CreateServiceFunction(MS_WAT_FORMAT,@ServiceFormat);
  CreateServiceFunction(MS_WAT_PLAYER,@ServicePlayer);

  FillChar(SongInfoA,SizeOf(SongInfoA),0);
  FillChar(SongInfo ,SizeOf(SongInfo ),0);
  FillChar(WorkSI   ,SizeOf(SongInfo ),0);
  HookEvent(ME_SYSTEM_MODULESLOADED,@OnModulesLoaded);
end;

function Unload:int; cdecl;
begin
  result:=0;
end;

exports
  Load, Unload,
  MirandaPluginInfoEx;

begin
  DisableThreadLibraryCalls(hInstance);
end.

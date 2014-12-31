{mRadio protocol support}
unit pl_mradio;
{$include compilers.inc}
{$r mradio.res}
interface

implementation
uses m_api,dbsettings,windows,common,srv_player,wat_api,io;

const
  strUnknown:PwideChar = 'Unknown';
  playername:PAnsiChar = 'mRadio';
const
  CurrentStation:THANDLE=THANDLE(-1);
const
  ChangesHook:THANDLE=0;
const
  MS_RADIO_COMMAND = 'mRadio/Command';
  MS_RADIO_SETVOL  = 'mRadio/SetVol';
  ME_RADIO_STATUS:PAnsiChar = 'mRadio/Status';
const
  MRC_STOP   = 0;
  MRC_PLAY   = 1;
  MRC_PAUSE  = 2;
  MRC_PREV   = 3;
  MRC_NEXT   = 4;
  MRC_STATUS = 5;
  MRC_SEEK   = 6;
const
  RD_STATUS_NOSTATION  = 0;   // no active station found
  RD_STATUS_PLAYING    = 1;   // media is playing
  RD_STATUS_PAUSED     = 2;   // media is paused
  RD_STATUS_STOPPED    = 3;   // media is stopped (only for playlists)
  RD_STATUS_CONNECT    = 4;   // plugin try to connect to the station
  RD_STATUS_ABORT      = 5;   // plugin want to abort while try to connect
  // next is for events only
  RD_STATUS_POSITION   = 107; // position was changed
  RD_STATUS_MUTED      = 108; // Mute/Unmute command was sent
  RD_STATUS_RECORD     = 109; // "Record" action called
  RD_STATUS_NEWTRACK   = 110; // new track/station
  RD_STATUS_NEWTAG     = 111; // tag data changed
  RD_STATUS_NEWSTATION = 112; // new station (contact)
  // next command is for users
  RD_STATUS_GET        = 6;   // to get current status
const
  prevfile:PWideChar=nil;

function ClearmRadio:integer; cdecl;
begin
  result:=0;
  if ChangesHook>0 then
  begin
    UnhookEvent(ChangesHook);
    ChangesHook:=0;
    CurrentStation:=THANDLE(-1);
  end;
  mFreeMem(prevfile);
end;

function SettingsChanged(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  result:=0;
  case wParam of
    // clear station
    RD_STATUS_NOSTATION: CurrentStation:=THANDLE(-1);
    // get new url
    RD_STATUS_NEWSTATION: CurrentStation:=lParam;
  end;
end;

function Fill:integer;
var
  i:integer;
begin
  CurrentStation:=db_find_first(playername);
  while CurrentStation<>0 do
  begin
    i:=DBReadWord(CurrentStation,playername,'Status',word(-1));
    if i=ID_STATUS_ONLINE then
    begin
      result:=1;
      exit;
    end;
    CurrentStation:=db_find_next(CurrentStation,playername);
  end;
  result:=WAT_RES_NOTFOUND;
end;

function InitmRadio:integer;
begin
  if ChangesHook=0 then
  begin
    ChangesHook:=HookEvent(ME_RADIO_STATUS,@SettingsChanged);
    result:=Fill;
  end
  else if (CurrentStation<>0) and (CurrentStation<>THANDLE(-1)) then
    result:=1
  else
    result:=WAT_RES_NOTFOUND;
end;

function Check(wnd:HWND;flags:integer):HWND;cdecl;
begin
  if CallProtoService(playername,PS_GETSTATUS,0,0)=ID_STATUS_ONLINE then
  begin
    result:=InitmRadio
  end
  else
  begin
    result:=HWND(WAT_RES_NOTFOUND);
    ClearmRadio;
  end;
end;

function GetKbps:integer;
var
  pc:PWideChar;
begin
  pc:=DBReadUnicode(CurrentStation,playername,'Bitrate','0');
  result:=StrToInt(pc);
  mFreeMem(pc);
end;

function GetFileName(wnd:HWND;flags:integer):pWideChar; cdecl;
begin
  result:=DBReadUnicode(0,playername,'ActiveURL',nil)
end;

function GetGenre:pWideChar;
begin
  result:=DBReadUnicode(CurrentStation,playername,'Genre',nil);
end;

function GetWndText:pWideChar;
begin
  result:=DBReadUnicode(CurrentStation,strCList,'StatusMsg',strUnknown);
end;

function GetTitle:pWideChar;
begin
  result:=DBReadUnicode(0,playername,'Title');
  if result=nil then
    result:=DBReadUnicode(CurrentStation,strCList,'StatusMsg',strUnknown);
end;

function GetArtist:pWideChar;
begin
  result:=DBReadUnicode(0,playername,'Artist');
  if result=nil then
    result:=DBReadUnicode(CurrentStation,strCList,'MyHandle',strUnknown);
end;

function GetVolume:cardinal;
begin
  result:=DBReadByte(0,playername,'Volume',0);
  result:=(result shl 16)+round((result shl 4)/100);
end;

procedure SetVolume(value:cardinal);
begin
  CallService(MS_RADIO_SETVOL,(value*100) shr 4,0);
end;

function VolDn:integer;
var
  val:dword;
begin
  result:=GetVolume;
  val:=loword(result);
  if val>0 then
    SetVolume(val-1);
end;

function VolUp:integer;
var
  val:dword;
begin
  result:=GetVolume;
  val:=loword(result);
  if val<16 then
    SetVolume(val+1);
end;

function GetCover:pWideChar;
var
  ptr:PavatarCacheEntry;
begin
  result:=nil;
  if ServiceExists(MS_AV_GETAVATARBITMAP)<>0 then
  begin
    ptr:=PavatarCacheEntry(CallService(MS_AV_GETAVATARBITMAP,CurrentStation,0));
    if ptr<>nil then
      StrDupW(result,ptr^.szFilename);
  end;
end;

function GetVersionText(ver:dword):pWideChar;
var
  s:array [0..31] of WideChar;
  p:pWideChar;
begin
  p:=@s;
  IntToStr(p,ver shr 12);
  while p^<>#0 do inc(p);
  p^:='.';
  IntToStr(p+1,(ver shr 8) and $F);
  while p^<>#0 do inc(p);
  p^:='.';
  IntToStr(p+1,(ver shr 4) and $F);
  while p^<>#0 do inc(p);
  p^:='.';
  IntToStr(p+1,ver and $F);
  StrDupW(result,PWideChar(@s));
end;

function GetStatus:integer; cdecl;
begin
  if CurrentStation<>0 then
  begin
    result:=WAT_PLS_PLAYING;
    case CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_GET) of
      RD_STATUS_PAUSED : result:=WAT_PLS_PAUSED;
      RD_STATUS_STOPPED: begin
        result:=WAT_PLS_STOPPED;
        mFreeMem(prevfile);
      end;
      RD_STATUS_NOSTATION,
      RD_STATUS_ABORT  : result:=WAT_PLS_UNKNOWN;
    end;
  end
  else
    result:=WAT_PLS_STOPPED;
end;

function GetInfo(var SongInfo:tSongInfo;flags:integer):integer;cdecl;
var
  isRemote:bool;
begin
  result:=0;
  if (flags and WAT_OPT_PLAYERDATA)<>0 then
  begin
    if SongInfo.plyver=0 then
    begin
      SongInfo.plyver:=DBReadDWord(0,playername,'version');
      SongInfo.txtver:=GetVersionText(SongInfo.plyver);
    end;
  end
  else if CurrentStation<>0 then
    with SongInfo do
    begin
      if (flags and WAT_OPT_CHANGES)<>0 then
      begin
        volume:=GetVolume;
        mFreeMem(wndtext);
        wndtext:=GetWndText;
        time:=CallService(MS_RADIO_COMMAND,MRC_SEEK,-1);
      end
      else
      begin
        isRemote:=StrPosW(mfile,'://')<>nil;
        if (prevfile=nil) or isRemote or (StrCmpW(prevfile,mfile)<>0) then
        begin
          ClearTrackInfo(SongInfo);
          mFreeMem(prevfile);
          StrDupW(prevfile,mfile);

          if not isRemote then
          begin
            CallService(MS_WAT_GETFILEINFO,0,lparam(@SongInfo));
            fsize:=GetFSize(mfile);
          end;
        end;

//!!
        if kbps  =0   then kbps  :=GetKbps;
        if genre =nil then genre :=GetGenre;
        if title =nil then title :=GetTitle;
        if artist=nil then artist:=GetArtist;
        if cover =nil then cover :=GetCover;
      end;
    end;
end;

function Command(wnd:HWND;cmd:integer;value:integer):integer;cdecl;
begin
  result:=0;
  case cmd of
    WAT_CTRL_PREV : result:=CallService(MS_RADIO_COMMAND,MRC_PREV,0);
    WAT_CTRL_PLAY : result:=CallService(MS_RADIO_COMMAND,MRC_PLAY,0);
    WAT_CTRL_PAUSE: result:=CallService(MS_RADIO_COMMAND,MRC_PAUSE,0);
    WAT_CTRL_STOP : result:=CallService(MS_RADIO_COMMAND,MRC_STOP,0);
    WAT_CTRL_NEXT : result:=CallService(MS_RADIO_COMMAND,MRC_NEXT,0);
    WAT_CTRL_VOLDN: result:=VolDn;
    WAT_CTRL_VOLUP: result:=VolUp;
    WAT_CTRL_SEEK : result:=CallService(MS_RADIO_COMMAND,MRC_SEEK,value);
  end;
end;

const
  plRec:tPlayerCell=(
    Desc     :'mRadio';
    flags    :WAT_OPT_PLAYERINFO or WAT_OPT_SINGLEINST or WAT_OPT_HASURL or WAT_OPT_LAST;
    Icon     :0;
    Init     :nil;
    DeInit   :@ClearmRadio;
    Check    :@Check;
    GetStatus:@GetStatus;
    GetName  :@GetFileName;
    GetInfo  :@GetInfo;
    Command  :@Command;
    URL      :'https://code.google.com/p/delphi-miranda-plugins/';
    Notes    :nil);

var
  LocalPlayerLink:twPlayer;

procedure InitLink;
begin
  if plRec.Icon=0 then
    plRec.Icon:=LoadImage(hInstance,'ICO_MRADIO',IMAGE_ICON,16,16,0);

  LocalPlayerLink.Next:=PlayerLink;
  LocalPlayerLink.This:=@plRec;
  PlayerLink          :=@LocalPlayerLink;
end;

initialization
  InitLink;
finalization
  if plRec.Icon<>0 then DestroyIcon(plRec.Icon);
end.

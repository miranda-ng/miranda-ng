{Apollo player}
unit pl_Apollo;
{$include compilers.inc}

interface

implementation
uses windows,winampapi,messages,common,srv_player,wat_api;

const
  ApolloClass = 'Apollo - Main Window';

const
  WM_APOLLO_COMMAND = WM_USER+3;
  APOLLO_GETVERSION                 = 0;
  APOLLO_GETSTATUS                  = 1;
  APOLLO_GETPLAYLISTPOSITION        = 16;
  APOLLO_GETCURRENTTRACKNUMBER      = 17;
  APOLLO_SETPLAYBACKPOSITION        = 18;
  APOLLO_GETPLAYBACKPOSITION        = 19;
  APOLLO_GETPLAYBACKCOUNTDOWN       = 33;
  APOLLO_GETCURRENTLYPLAYEDFILENAME = 24;
  APOLLO_GETCURRENTLYPLAYEDTITLE    = 25;
  APOLLO_GETPLAYLISTENTRY           = 26;
  APOLLO_GETPLAYLISTTITLE           = 27;
//  APOLLO_OPENURL       = 4;
  APOLLO_OPENFILE      = 2;
  APOLLO_PREVIOUSTRACK = 10;
  APOLLO_STOP          = 11;
  APOLLO_PLAY          = 12;
  APOLLO_PAUSE         = 13;
  APOLLO_NEXTTRACK     = 14;
  APOLLO_SETVOLUME     = 20;
  APOLLO_GETVOLUME     = 21;

function Check(wnd:HWND;flags:integer):HWND;cdecl;
begin
  if wnd<>0 then
  begin
    result:=0;
    exit;
  end;
  result:=FindWindow(ApolloClass,NIL)
end;

function GetVersion(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_GETVERSION,0);
end;

function GetVersionText(ver:integer):PWideChar;
begin
  mGetMem(result,5*SizeOf(WideChar));
  IntToStr(result,ver);
end;

function GetStatus(wnd:HWND):integer; cdecl;
begin
  result:=SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_GETSTATUS,0);
  if result>1 then
    result:=2;
end;

function GetFileName(wnd:HWND;flags:integer):pWideChar;cdecl;
var
  tmpwnd:HWND;
  ps:array [0..255] of AnsiChar;
begin
  if GetStatus(wnd)<>WAT_PLS_STOPPED then
  begin
    tmpwnd:=SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_GETCURRENTLYPLAYEDFILENAME,0);
    ps[0]:=#0;
    SendMessageA(tmpwnd,WM_GETTEXT,255,lparam(@ps));
    if ps[0]<>#0 then
    begin
      mGetMem(result,(StrLen(ps)+1)*SizeOf(WideChar));
      AnsiToWide(ps,result);
      exit;
    end;
  end;
  result:=nil;
end;

function GetWndText(wnd:HWND):pWideChar;
var
  tmpwnd:HWND;
  ps:array [0..255] of AnsiChar;
begin
  tmpwnd:=SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_GETCURRENTLYPLAYEDTITLE,0);
  SendMessageA(tmpwnd,WM_GETTEXT,255,lparam(@ps));
  mGetMem(result,(StrLen(ps)+1)*SizeOf(WideChar));
  AnsiToWide(ps,result);
end;

function Play(wnd:HWND;fname:PWideChar=nil):integer;
var
  cds:COPYDATASTRUCT;
begin
  if (fname<>nil) and (fname^<>#0) then
  begin
    cds.dwData:=APOLLO_OPENFILE;
    WideToAnsi(fname,PAnsiChar(cds.lpData));
    cds.cbData:=StrLen(PAnsiChar(cds.lpData))+1;
    SendMessage(wnd,WM_COPYDATA,0,lparam(@cds));
    mFreeMem(cds.lpData);
  end;
  result:=SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_PLAY,0);
end;

function Pause(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_PAUSE,0);
end;

function Stop(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_STOP,0);
end;

function Next(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_NEXTTRACK,0);
end;

function Prev(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_PREVIOUSTRACK,0);
end;

function GetVolume(wnd:HWND):cardinal;
begin
  result:=SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_GETVOLUME,0);
  result:=(result shl 16)+(result shr 12);
end;

procedure SetVolume(wnd:HWND;value:cardinal);
begin
  SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_SETVOLUME,value shl 12);
end;

function VolDn(wnd:HWND):integer;
var
  val:integer;
begin
  result:=GetVolume(wnd);
  val:=loword(result);
  if val>0 then
    SetVolume(wnd,val-1);
end;

function VolUp(wnd:HWND):integer;
var
  val:integer;
begin
  result:=GetVolume(wnd);
  val:=loword(result);
  if val<16 then
    SetVolume(wnd,val+1);
end;

function Seek(wnd:HWND;value:integer):integer;
begin
  result:=SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_GETPLAYBACKPOSITION,0);
  if value>0 then
    SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_SETPLAYBACKPOSITION,value);
end;

function GetRemoteTitle(wnd:HWND):pWideChar;
var
  tmpwnd:HWND;
  ps:array [0..255] of AnsiChar;
  num:integer;
begin
  num   :=SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_GETPLAYLISTPOSITION,0);
  tmpwnd:=SendMessage(wnd,WM_APOLLO_COMMAND,APOLLO_GETPLAYLISTTITLE   ,num);
  SendMessageA(tmpwnd,WM_GETTEXT,255,lparam(@ps));
  mGetMem(result,(StrLen(ps)+1)*SizeOf(WideChar));
  AnsiToWide(ps,result);
end;

function GetInfo(var SongInfo:tSongInfo;flags:integer):integer;cdecl;
begin
  result:=0;

  if (flags and WAT_OPT_PLAYERDATA)<>0 then
  begin
    if SongInfo.plyver=0 then
    begin
      SongInfo.plyver:=GetVersion    (SongInfo.plwnd);
      SongInfo.txtver:=GetVersionText(SongInfo.plyver);
    end;
    if SongInfo.winampwnd=0 then
      SongInfo.winampwnd:=WinampFindWindow(SongInfo.plwnd);
    exit;
  end;

  if SongInfo.winampwnd<>0 then
    result:=WinampGetInfo(int_ptr(@SongInfo),flags);

  if (flags and WAT_OPT_CHANGES)<>0 then
  begin
    with SongInfo do
    begin
      wndtext:=GetWndText(plwnd);
      volume :=GetVolume (plwnd);
    end
  end
  else
  begin
    with SongInfo do
    begin
      if (status<>WAT_PLS_STOPPED) and
         (mfile<>nil) and (StrPosW(mfile,'://')<>nil) and (album=nil) then
        album:=GetRemoteTitle(plwnd);
    end;
  end;
end;

function Command(wnd:HWND;cmd:integer;value:int_ptr):integer;cdecl;
begin
  case cmd of
    WAT_CTRL_PREV : result:=Prev(wnd);
    WAT_CTRL_PLAY : result:=Play(wnd,pWideChar(value));
    WAT_CTRL_PAUSE: result:=Pause(wnd);
    WAT_CTRL_STOP : result:=Stop(wnd);
    WAT_CTRL_NEXT : result:=Next(wnd);
    WAT_CTRL_VOLDN: result:=VolDn(wnd);
    WAT_CTRL_VOLUP: result:=VolUp(wnd);
    WAT_CTRL_SEEK : result:=Seek(wnd,value);
  else
    result:=0;
  end;
end;

const
  plRec:tPlayerCell=(
    Desc     :'Apollo';
    flags    :WAT_OPT_HASURL;
    Icon     :0;
    Init     :nil;
    DeInit   :nil;
    Check    :@Check;
    GetStatus:@GetStatus;
    GetName  :@GetFileName;
    GetInfo  :@GetInfo;
    Command  :@Command;
    URL      :'http://www.iki.fi/hy/apollo/';
    Notes    :nil);

var
  LocalPlayerLink:twPlayer;

procedure InitLink;
begin
  LocalPlayerLink.Next:=PlayerLink;
  LocalPlayerLink.This:=@plRec;
  PlayerLink          :=@LocalPlayerLink;
end;

initialization
//  ServicePlayer(WAT_ACT_REGISTER,dword(@plRec));
  InitLink;
end.

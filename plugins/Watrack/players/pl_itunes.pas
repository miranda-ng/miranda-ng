{iTunes player}
unit pl_iTunes;
{$include compilers.inc}

interface

implementation
uses windows,common,srv_player,wat_api
  {$IFDEF DELPHI_7_UP}
  ,variants
  {$ENDIF}
  {$IFDEF KOL_MCK}
  ,kolcomobj
  {$ELSE}
  ,ComObj
  {$ENDIF}
;

const
  iTunesClass = 'iTunes';
  iTunesTitle = 'iTunes';
  COMName = 'iTunes.Application';

function Check(wnd:HWND;flags:integer):HWND;cdecl;
begin
  if wnd<>0 then
  begin
    result:=0;
    exit;
  end;
  result:=FindWindow(iTunesClass,iTunesTitle);
end;

function GetFileName(wnd:HWND;flags:integer):PWideChar;cdecl;
var
  v:variant;
begin
  try
    v:=CreateOleObject(COMName);
    StrDupW(result,PWideChar(WideString(v.CurrentTrack.Location)));
  except
    result:=nil;
  end;
  v:=Null;
end;

function SplitVersion(p:pWideChar):integer;
begin
  result:=StrToInt(p);
  while (p^>='0') and (p^<='9') do inc(p); inc(p);
  result:=result*16+StrToInt(p);
  while (p^>='0') and (p^<='9') do inc(p); inc(p);
  result:=(result*16+StrToInt(p))*16;
  while (p^>='0') and (p^<='9') do inc(p); inc(p);
  result:=result*16+StrToInt(p);
end;

function GetVersion(const ver:pWideChar):integer;
begin
  if (ver<>nil) and (ver^<>#0) then
    result:=SplitVersion(ver)
  else
    result:=0;
end;

function GetVersionText(const v:variant):PWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.Version)));
  except
    result:=nil;
  end;
end;

function GetTotalTime(const v:variant):integer;
begin
  try
    result:=v.CurrentTrack.Duration;
  except
    result:=0;
  end;
end;

function GetElapsedTime(const v:variant):integer;
begin
  try
    result:=v.PlayerPosition;
  except
    result:=0;
  end;
end;

function GetStatus(wnd:HWND):integer; cdecl;
var
  tmp:integer;
  v:variant;
begin
  try
    v:=CreateOleObject(COMName);
    tmp:=v.PlayerState;
    if tmp=1 then
      result:=WAT_PLS_PLAYING
    else
      result:=WAT_PLS_STOPPED;
  except
    result:=WAT_PLS_UNKNOWN;
  end;
  v:=Null;
end;

function GetKbps(const v:variant):integer;
begin
  try
    result:=v.CurrentTrack.BitRate;
  except
    result:=0;
  end;
end;

function GetKhz(const v:variant):integer;
begin
  try
    result:=v.CurrentTrack.SampleRate;
  except
    result:=0;
  end;
end;

function GetTrack(const v:variant):integer;
begin
  try
    result:=v.CurrentTrack.TrackNumber;
  except
    result:=0;
  end;
end;

function GetAlbum(const v:variant):pWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.CurrentTrack.Album)));
  except
    result:=nil;
  end;
end;

function GetYear(const v:variant):pWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.CurrentTrack.Year)));
  except
    result:=nil;
  end;
end;

function GetGenre(const v:variant):pWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.CurrentTrack.Genre)));
  except
    result:=nil;
  end;
end;

function GetArtist(const v:variant):pWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.CurrentTrack.Artist)));
  except
    result:=nil;
  end;
end;

function GetTitle(const v:variant):pWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.CurrentStreamTitle)));
  except
    result:=nil;
  end;
end;

function GetComment(const v:variant):pWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.CurrentTrack.Comment)));
  except
    result:=nil;
  end;
end;

function GetWndText(const v:variant):pWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.Windows.Name)));
  except
    result:=nil;
  end;
end;

function Play(const v:variant;fname:PWideChar=nil):integer;
begin
  try
//    v.PlayFile(fname);
    v.BackTrack;
    result:=v.Play;
  except
    result:=0;
  end;
end;

function Pause(const v:variant):integer;
begin
  try
    result:=v.PlayPause;
  except
    result:=0;
  end;
end;

function Stop(const v:variant):integer;
begin
  try
    result:=v.Stop;
  except
    result:=0;
  end;
end;

function Next(const v:variant):integer;
begin
  try
    result:=v.NextTrack;
  except
    result:=0;
  end;
end;

function Prev(const v:variant):integer;
begin
  try
    result:=v.PreviousTrack;
  except
    result:=0;
  end;
end;

function Seek(const v:variant;value:integer):integer;
begin
  try
    result:=v.PlayerPosition;
    if value>0 then
      v.PlayerPosition:=value
    else
      result:=0;
  except
    result:=0;
  end;
end;

function GetVolume(const v:variant):cardinal;
begin
  try
    result:=v.SoundVolume;
    result:=(result shl 16)+round((result shl 4)/100);
  except
    result:=0;
  end;
end;

procedure SetVolume(const v:variant;value:cardinal);
begin
  try
    v.SoundVolume:=integer((value*100) shr 4);
  except
  end;
end;

function VolDn(const v:variant):integer;
var
  val:integer;
begin
  result:=GetVolume(v);
  val:=loword(result);
  if val>0 then
    SetVolume(v,val-1);
end;

function VolUp(const v:variant):integer;
var
  val:integer;
begin
  result:=GetVolume(v);
  val:=loword(result);
  if val<16 then
    SetVolume(v,val+1);
end;

function GetInfo(var SongInfo:tSongInfo;flags:integer):integer;cdecl;
var
  v:variant;
begin
  result:=0;
  with SongInfo do
  begin
    try
      v:=CreateOleObject(COMName);
      if (flags and WAT_OPT_PLAYERDATA)<>0 then
      begin
        if plyver=0 then
        begin
          txtver:=GetVersionText(v);
          plyver:=GetVersion(txtver);
        end;
      end
      else if (flags and WAT_OPT_CHANGES)<>0 then
      begin
        volume:=GetVolume(v);
        if status<>WAT_PLS_STOPPED then
          time:=GetElapsedTime(v);
      end
      else
      begin
        if total  =0   then total  :=GetTotalTime(v);
        if track  =0   then track  :=GetTrack(v);
        if year   =NIL then year   :=GetYear(v);
        if genre  =NIL then genre  :=GetGenre(v);
        if artist =NIL then artist :=GetArtist(v);
        if album  =NIL then album  :=GetAlbum(v);
        if comment=NIL then comment:=GetComment(v);
        if kbps   =0   then kbps   :=GetKbps(v);
        if khz    =0   then khz    :=GetKhz(v);
      end;
//      wndtext:=GetWndText(v);
    except
    end;
    v:=Null;
//    if title=NIL then title:=GetTitle; // only for streaming audio
  end;
end;

function Command(wnd:HWND;cmd:integer;value:int_ptr):integer;cdecl;
var
  v:Variant;
begin
  result:=0;
  try
    v:=CreateOleObject(COMName);
    case cmd of
      WAT_CTRL_PREV : result:=Prev (v);
      WAT_CTRL_PLAY : result:=Play (v,pWideChar(value));
      WAT_CTRL_PAUSE: result:=Pause(v);
      WAT_CTRL_STOP : result:=Stop (v);
      WAT_CTRL_NEXT : result:=Next (v);
      WAT_CTRL_VOLDN: result:=VolDn(v);
      WAT_CTRL_VOLUP: result:=VolUp(v);
      WAT_CTRL_SEEK : result:=Seek (v,value);
    end;
  except
  end;
  v:=Null;
end;

const
  plRec:tPlayerCell=(
    Desc     :'iTunes';
    flags    :WAT_OPT_SINGLEINST or WAT_OPT_HASURL;
    Icon     :0;
    Init     :nil;
    DeInit   :nil;
    Check    :@Check;
    GetStatus:@GetStatus;
    GetName  :@GetFileName;
    GetInfo  :@GetInfo;
    Command  :@Command;
    URL      :'http://www.itunes.com/';
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

{Foobar2000 player}
unit pl_Foobar;
{$include compilers.inc}

interface

implementation
uses {$IFDEF KOL_MCK}err,{$ENDIF}
   windows,common,syswin,wrapper,srv_player,messages,wat_api,winampapi
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
  COMName:PWideChar = 'Foobar2000.Application.0.7';
const
  dummywnd  = 'uninteresting';
const
  FooExe    = 'FOOBAR2000.EXE';
  FooPrefix = 'foobar2000 v';
const
  FooBarClassExt = '{97E27FAA-C0B3-4b8e-A693-ED7881E99FC1}';
  FooBarClassNew = '{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}';
  FooBarClassAdd = '{E7076D1C-A7BF-4f39-B771-BCBE88F2A2A8}';
(*
  class1='{B73733CA-9B0A-4f53-93FA-AC95D4FF2166}';
  text1='Cthulhu fhtagn!';

  '{53229DFC-A273-45cd-A3A4-161FA9FC6414}';
  '{641C2469-355C-4d6f-9663-E714382DA462}';
*)
var
  WinampWindow:HWND;

function proc(awnd:HWND;param:pdword):boolean; stdcall;
var
  s:array [0..255] of AnsiChar;
begin
  result:=true;
  if (awnd<>param^) and (GetClassNameA(awnd,s,255)>0) then
  begin
    s[Length(FooBarClassNew)]:=#0;
    if (StrCmp(s,FooBarClassExt)=0) or
       (StrCmp(s,FooBarClassNew)=0) or
       (StrCmp(s,FooBarClassAdd)=0) then
    begin
      GetWindowTextA(awnd,s,255);
      if StrCmp(s,dummywnd)<>0 then
        param^:=awnd;
//      if WinampWindow<>0 then
//        result:=false;
    end

    else if (WinampWindow=0) and (StrCmp(s,WinampClass)=0) then
    begin
      WinampWindow:=awnd;
    end;

  end;
end;

function Check(wnd:HWND;flags:integer):HWND;cdecl;
var
  tmp,EXEName:PAnsiChar;
  lwnd:HWND;
  ltmp:bool;
begin
  if wnd<>0 then
  begin
    result:=0;
    exit;
  end;
  lwnd:=0;
  repeat
    lwnd:=FindWindowEx(0,lwnd,nil,dummywnd);
    if lwnd=0 then
      break;
    tmp:=Extract(GetEXEByWnd(lwnd,EXEName),true);
    mFreeMem(EXEName);
    ltmp:=lstrcmpia(tmp,FooExe)=0;
    mFreeMem(tmp);
    if ltmp then
    begin
      WinampWindow:=0;
      EnumThreadWindows(GetWindowThreadProcessId(lwnd,nil),@proc,int_ptr(@lwnd));
      break;
    end;
  until false;
  result:=lwnd;
end;

function GetYear(const v:variant):PWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.Playback.FormatTitle('[%year%]'))));
  except
    result:=nil;
  end;
end;

function GetArtist(const v:variant):PWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.Playback.FormatTitle('[%artist%]'))));
  except
    result:=nil;
  end;
end;

function GetTitle(const v:variant):PWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.Playback.FormatTitle('[%title%]'))));
  except
    result:=nil;
  end;
end;

function GetAlbum(const v:variant):PWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.Playback.FormatTitle('[%album%]'))));
  except
    result:=nil;
  end;
end;

function GetGenre(const v:variant):PWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.Playback.FormatTitle('[%genre%]'))));
  except
    result:=nil;
  end;
end;

function GetBitrate(const v:variant):integer;
begin
  try
    result:=v.Playback.FormatTitle('%bitrate%');
  except
    result:=0;
  end;
end;

function GetSamplerate(const v:variant):integer;
begin
  try
    result:=v.Playback.FormatTitle('%samplerate%');
  except
    result:=0;
  end;
end;

function GetChannels(const v:variant):integer;
var
  s:WideString;
begin
  result:=0;
  try
    s:=v.Playback.FormatTitle('%channels%');
    if StrCmpW(pWideChar(s),'mono')=0 then
      result:=1
    else if StrCmpW(pWideChar(s),'Stereo')=0 then
      result:=2;
  except
  end;
end;

function GetCodec(const v:variant):integer;
var
  s:WideString;
  i:integer;
begin
  result:=0;
  try
    s:=v.Playback.FormatTitle('%codec%');
    i:=Length(s);
    if i>0 then result:=ORD(s[1]);
    if i>1 then result:=result+(ORD(s[2]) shl 8);
    if i>2 then result:=result+(ORD(s[3]) shl 16);
    if i>3 then result:=result+(ORD(s[4]) shl 24);
  except
  end;
end;

function SplitVersion(p:pWideChar):integer;
begin
  result:=StrToInt(p);
  while (p^>='0') and (p^<='9') do inc(p); inc(p);
  result:=result*16+StrToInt(p);
  while (p^>='0') and (p^<='9') do inc(p); inc(p);
  result:=result*16+StrToInt(p);
  while (p^>='0') and (p^<='9') do inc(p);
  if p^<>#0 then inc(p);
  if (p^>='0') and (p^<='9') then
  begin
    result:=result*16+StrToInt(p);
    while (p^>='0') and (p^<='9') do inc(p);
  end;
  if p^<>#0 then
  begin
    inc(p);
    while (p^<>#0) and (p^<>' ') do inc(p);
    if p^=' ' then
    begin
      inc(p);
      result:=result*16+StrToInt(p);
    end;
  end;
end;

function GetVersion(const ver:pWideChar):integer;
begin
  if (ver=nil) or (ver^=#0) then
    result:=0
  else
    result:=SplitVersion(ver);
end;

function GetVersionText(const v:variant):PWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.Name))+length(FooPrefix));
  except
    result:=nil;
  end;
end;

function GetTotalTime(const v:variant):integer;
begin
  try
    result:=v.Playback.Length;
  except
    result:=0;
  end;
end;

function GetElapsedTime(const v:variant):integer;
begin
  try
    result:=v.Playback.Position;
  except
    result:=0;
  end;
end;

function GetStatus(wnd:HWND):integer; cdecl;
var
  tmp:boolean;
  v:variant;
  winampwnd:HWND;
begin
  try
    result:=WAT_PLS_STOPPED;
    v:=GetActiveOleObject(COMName);
    tmp:=v.Playback.IsPaused;
    if tmp then
      result:=WAT_PLS_PAUSED
    else
    begin
      tmp:=v.Playback.IsPlaying;
      if tmp then
        result:=WAT_PLS_PLAYING;
    end;
  except
    winampwnd:=WinampFindWindow(wnd);
    if winampwnd<>0 then
      result:=WinampGetStatus(winampwnd)
    else
      result:=WAT_PLS_UNKNOWN;
  end;
  v:=null;
end;

function GetWndText(wnd:HWND):pWideChar;
var
  i:integer;
begin
  result:=GetDlgText(wnd);
  if result<>nil then
  begin
    i:=StrIndexW(result,'[foobar');
    if i<>0 then
    begin
      dec(i);
      repeat
        dec(i);
        if ord(result[i])>ord(' ') then break;
      until i<0;
      result[i+1]:=#0;//if at end
    end;
  end;
end;

function GetFileName(wnd:HWND;flags:integer):PWideChar;cdecl;
var
  v:variant;
begin
  try
    v:=GetActiveOleObject(COMName);
//    v:=CreateOleObject(COMName);
    StrDupW(result,PWideChar(WideString(v.Playback.FormatTitle('%path%'))));
  except
    result:=nil;
  end;
  v:=Null;
end;

function Play(const v:variant;fname:PWideChar=nil):integer;
begin
  try
    result:=v.Playback.Start(false);
  except
    result:=0;
  end;
end;

function Pause(const v:variant):integer;
begin
  try
    result:=v.Playback.Pause;
  except
    result:=0;
  end;
end;

function Stop(const v:variant):integer;
begin
  try
    result:=v.Playback.Stop;
  except
    result:=0;
  end;
end;

function Next(const v:variant):integer;
begin
  try
    result:=v.Playback.Next;
  except
    result:=0;
  end;
end;

function Prev(const v:variant):integer;
begin
  try
    result:=v.Playback.Previous;
  except
    result:=0;
  end;
end;

function GetVolume(const v:variant):cardinal;
begin
  try
    result:=v.Playback.Settings.Volume+100;
    result:=(result shl 16)+round((result shl 4) / 100);
  except
    result:=0;
  end;
end;

procedure SetVolume(const v:variant;value:cardinal);
begin
  try
    v.Playback.Settings.Volume:=integer(((loword(value)*100) shr 4)-100);
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

function Seek(const v:variant;value:integer):integer;
begin
  try
    result:=v.Playback.Position;
    if (value>0) and (v.Playback.CanSeek) and (value<v.Playback.Length) then
      v.Playback.Seek(value)
    else
      result:=0;
  except
    result:=0;
  end;
end;

function GetInfo(var SongInfo:tSongInfo;flags:integer):integer;cdecl;
var
  v:variant;
begin
  result:=0;
  with SongInfo do
  begin
    try
      v:=GetActiveOleObject(COMName);
//        v:=CreateOleObject(COMName);
      if (flags and WAT_OPT_PLAYERDATA)<>0 then
      begin
        if SongInfo.plyver=0 then
        begin
          SongInfo.txtver:=GetVersionText(v);
          SongInfo.plyver:=GetVersion(txtver);
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
        if kbps    =0   then kbps    :=GetBitrate(v);
        if khz     =0   then khz     :=GetSamplerate(v);
        if channels=0   then channels:=GetChannels(v);
        if codec   =0   then codec   :=GetCodec(v);
        if total   =0   then total   :=GetTotalTime(v);
        if year    =NIL then year    :=GetYear(v);
        if artist  =NIL then artist  :=GetArtist(v);
        if title   =NIL then title   :=GetTitle(v);
        if album   =NIL then album   :=GetAlbum(v);
        if genre   =NIL then genre   :=GetGenre(v);
      end;
    except
      SongInfo.winampwnd:=WinampWindow;
      if SongInfo.winampwnd<>0 then
      begin
        result:=WinampGetInfo(int_ptr(@SongInfo),flags);
      end;
    end;
    v:=Null;
    if (flags and WAT_OPT_CHANGES)<>0 then
      wndtext:=GetWndText(SongInfo.plwnd);
  end;
end;

function Command(wnd:HWND;cmd:integer;value:int_ptr):integer;cdecl;
//var
//  c:integer;
var
  v:Variant;
begin
  result:=0;
  try
    v:=GetActiveOleObject(COMName);
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
    if WinampWindow<>0 then
      result:=WinampCommand(WinampWindow,cmd+(value shl 16))
{
    else
    begin
      case cmd of
        WAT_CTRL_PREV : c:=ORD('B');
        WAT_CTRL_PLAY : c:=ORD('C');
        WAT_CTRL_PAUSE: c:=ORD('X');
        WAT_CTRL_STOP : c:=ORD('Z');
        WAT_CTRL_NEXT : c:=ORD('V');
        WAT_CTRL_VOLDN: c:=VK_SUBTRACT;
        WAT_CTRL_VOLUP: c:=VK_ADD;
        else
          exit;
      end;
      PostMessageW(wnd,WM_KEYDOWN,c,1);
    end;
}
  end;
  v:=Null;
end;

const
  plRec:tPlayerCell=(
    Desc     :'foobar2000';
    flags    :WAT_OPT_SINGLEINST or WAT_OPT_HASURL;
    Icon     :0;
    Init     :nil;
    DeInit   :nil;
    Check    :@Check;
    GetStatus:@GetStatus;
    GetName  :@GetFileName;
    GetInfo  :@GetInfo;
    Command  :@Command;
    URL      :'http://www.foobar2000.org/';
    Notes    :'For more full info WinampSpam or foo_comserver (more powerful) '#13#10+
              'components needs to be installed.');

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

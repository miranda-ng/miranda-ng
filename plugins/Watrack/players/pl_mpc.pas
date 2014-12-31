{Media Player Classic}
unit pl_MPC;
{$include compilers.inc}

interface

implementation
uses windows,common,wrapper,srv_player,wat_api;

const
  MPCClass98 = 'MediaPlayerClassicA';
  MPCClassXP = 'MediaPlayerClassicW';
  MPCTail    = ' - Media Player Classic';

function Check(wnd:HWND;flags:integer):HWND;cdecl;
begin
  result:=FindWindowEx(0,wnd,MPCClassXP,NIL);
  if result=0 then
    result:=FindWindowEx(0,wnd,MPCClass98,NIL);
end;

function chwnd(awnd:HWND;Param:pdword):boolean; stdcall;
var
  s:array [0..31] of AnsiChar;
  i:integer;
begin
  FillChar(s,SizeOf(s),0);
  GetWindowTextA(awnd,s,30);
  i:=StrIndex(PAnsiChar(@s),' / ');
  if i<>0 then
  begin
    if Param^=0 then
    begin
      s[i-1]:=#0;
      Param^:=TimeToInt(s);
    end
    else
    begin
      Param^:=TimeToInt(s+i+2);
    end;
    result:=false;
  end
  else
    result:=true;
end;

function GetElapsedTime(wnd:HWND):integer;
begin
  result:=0;
  if EnumChildWindows(wnd,@chwnd,int_ptr(@result)) then
    result:=0;
end;

function GetTotalTime(wnd:HWND):integer;
begin
  result:=1;
  if EnumChildWindows(wnd,@chwnd,int_ptr(@result)) then
    result:=0;
end;

function GetWndText(wnd:HWND):pWidechar;
var
  p:pWideChar;
begin
  result:=GetDlgText(wnd);
  if result<>nil then
  begin
    p:=StrPosW(result,MPCTail);
    if p<>nil then
      p^:=#0;
  end;
end;

function GetInfo(var SongInfo:tSongInfo;flags:integer):integer;cdecl;
begin
  result:=0;
  with SongInfo do
  begin
    if (flags and WAT_OPT_CHANGES)<>0 then
    begin
      time   :=GetElapsedTime(SongInfo.plwnd);
      wndtext:=GetWndText(SongInfo.plwnd);
    end
    else if total=0 then
      total:=GetTotalTime(SongInfo.plwnd);
  end;
end;

const
  plRec:tPlayerCell=(
    Desc     :'MPC';
    flags    :WAT_OPT_HASURL;
    Icon     :0;
    Init     :nil;
    DeInit   :nil;
    Check    :@Check;
    GetStatus:nil;
    GetName  :nil;
    GetInfo  :@GetInfo;
    Command  :nil;
    URL      :'http://gabest.org/';
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

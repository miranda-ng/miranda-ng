{Winamp-like - base class}
unit winampapi;
{$include compilers.inc}

interface

uses windows,messages;

const
  WinampClass = 'Winamp v1.x';
  WinampTail  = ' - Winamp';

function WinampGetStatus(wnd:HWND):integer;
function WinampGetWindowText(wnd:HWND):pWideChar;
function WinampFindWindow(wnd:HWND):HWND;
function WinampCommand(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
function WinampGetInfo(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;

const
  WM_WA_IPC = WM_USER;
  IPC_GETVERSION       = 0;
  IPC_ISPLAYING        = 104;
  IPC_GETINFO          = 126;
  IPC_GETOUTPUTTIME    = 105;
  IPC_WRITEPLAYLIST    = 120;
  IPC_GETLISTLENGTH    = 124;
  IPC_GETLISTPOS       = 125;
  IPC_ISFULLSTOP       = 400; //!!
  IPC_INETAVAILABLE    = 242; //!!
  IPC_GETPLAYLISTFILE  = 211;

  IPC_IS_PLAYING_VIDEO = 501;

  IPC_PLAYFILE    = 100;
  IPC_STARTPLAY   = 102;
  IPC_SETVOLUME   = 122; // -666 returns the current volume.
  IPC_GET_SHUFFLE = 250;
  IPC_SET_SHUFFLE = 252;
  IPC_JUMPTOTIME  = 106;

const
  WINAMP_PREV       = 40044;
  WINAMP_PLAY       = 40045;
  WINAMP_PAUSE      = 40046;
  WINAMP_STOP       = 40047;
  WINAMP_NEXT       = 40048;
  WINAMP_VOLUMEUP   = 40058; // turns the volume up a little
  WINAMP_VOLUMEDOWN = 40059; // turns the volume down a little

implementation

uses common,wat_api;

function WinampFindWindow(wnd:HWND):HWND;
var
  pr,pr1:dword;
begin
  GetWindowThreadProcessId(wnd,@pr);
  result:=0;
  repeat
    result:=FindWindowEx(0,result,WinampClass,nil);
    if result<>0 then
    begin
      GetWindowThreadProcessId(result,@pr1);
      if pr=pr1 then
        break;
    end
    else
      break;
  until false;
end;

// ----------- Get player info ------------

function GetVersion(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_WA_IPC,0,IPC_GETVERSION);
end;

function GetVersionText(wnd:HWND):pWideChar;
var
  ver:integer;
  s:array [0..31] of WideChar;
  p:pWideChar;
begin
  ver:=GetVersion(wnd);
  p:=@s;
  IntToStr(p,ver shr 12);
  while p^<>#0 do inc(p);
  p^:='.';
  IntToStr(p+1,(ver shr 4) and $F);
  while p^<>#0 do inc(p);
  p^:='.';
  IntToStr(p+1,ver and $F);
  StrDupW(result,PWideChar(@s));
end;

function WinampGetStatus(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_WA_IPC,0,IPC_ISPLAYING);
  // 0 - stopped, 1 - playing
  case result of
    0: result:=WAT_PLS_STOPPED;
    1: result:=WAT_PLS_PLAYING;
  else
    if result>1 then
      result:=WAT_PLS_PAUSED;
  end;
{
  if result=0 then // !! only for remote media!
  begin
    result:=SendMessage(wnd,WM_WA_IPC,0,IPC_ISFULLSTOP);
    if result<>0 then
      result:=WAT_PLS_STOPPED
    else
      result:=WAT_PLS_PLAYING;
  end;
}
end;

function WinampGetWindowText(wnd:HWND):pWideChar;
var
  a:cardinal;
  pc:pWideChar;
begin
  a:=GetWindowTextLengthW(wnd);
  mGetMem(result,(a+1)*SizeOf(WideChar));
  if GetWindowTextW(wnd,result,a+1)>0 then
  begin
    pc:=StrPosW(result,WinampTail);
    if pc<>nil then
    begin
      pc^:=#0;
      pc:=StrPosW(result,'. ');
      if pc<>nil then
        StrCopyW(result,pc+2);
    end;
  end;
end;

// --------- Get file info ----------

function GetKbps(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_WA_IPC,1,IPC_GETINFO);
  if result>1000 then
    result:=result div 1000;
end;

function GetKhz(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_WA_IPC,0,IPC_GETINFO);
  if result>1000 then
    result:=result div 1000;
end;

function GetChannels(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_WA_IPC,2,IPC_GETINFO);
end;

function GetTotalTime(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_WA_IPC,1,IPC_GETOUTPUTTIME);
end;

function GetElapsedTime(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_WA_IPC,0,IPC_GETOUTPUTTIME) div 1000;
end;

function GetVolume(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_WA_IPC,-666,IPC_SETVOLUME);
  result:=(result shl 16)+(result shr 4);
end;

function WinampGetInfo(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  wnd:HWND;
begin
  result:=0;
  with pSongInfo(wParam)^ do
  begin
    if winampwnd<>0 then
      wnd:=winampwnd
    else
      wnd:=plwnd;

    if (lParam and WAT_OPT_PLAYERDATA)<>0 then
    begin
      if plyver=0 then
      begin
        plyver:=GetVersion(wnd);
        txtver:=GetVersionText(wnd);
      end;
    end
    else if (lParam and WAT_OPT_CHANGES)<>0 then
    begin
      volume:=GetVolume(wnd);
      if status<>WAT_PLS_STOPPED then
        time:=GetElapsedTime(wnd);
//      wndtext:=WinampGetWindowText(wnd);
    end
    else
    begin
      if kbps    =0 then kbps    :=GetKbps(wnd);
      if khz     =0 then khz     :=GetKhz(wnd);
      if channels=0 then channels:=GetChannels(wnd);
      if total   =0 then total   :=GetTotalTime(wnd);
    end;
  end;
end;

// ------- Commands ----------

function Play(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_COMMAND,WINAMP_PLAY,0);
end;

function Pause(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_COMMAND,WINAMP_PAUSE,0);
end;

function Stop(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_COMMAND,WINAMP_STOP,0);
end;

function Next(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_COMMAND,WINAMP_NEXT,0);
end;

function Prev(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_COMMAND,WINAMP_PREV,0);
end;

function VolDn(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_COMMAND,WINAMP_VOLUMEDOWN,0);
end;

function VolUp(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_COMMAND,WINAMP_VOLUMEUP,0);
end;

procedure SetVolume(wnd:HWND;value:cardinal);
begin
  SendMessage(wnd,WM_WA_IPC,value shl 4,IPC_SETVOLUME);
end;

function Seek(wnd:HWND;value:integer):integer;
begin
  result:=SendMessage(wnd,WM_WA_IPC,0,IPC_GETOUTPUTTIME) div 1000;
    SendMessage(wnd,WM_WA_IPC,value*1000,IPC_JUMPTOTIME);
end;

function WinampCommand(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  wnd:HWND;
begin
  wnd:=wParam;
  case LoWord(lParam) of
    WAT_CTRL_PREV : result:=Prev (wnd);
    WAT_CTRL_PLAY : result:=Play (wnd);
    WAT_CTRL_PAUSE: result:=Pause(wnd);
    WAT_CTRL_STOP : result:=Stop (wnd);
    WAT_CTRL_NEXT : result:=Next (wnd);
    WAT_CTRL_VOLDN: result:=VolDn(wnd);
    WAT_CTRL_VOLUP: result:=VolUp(wnd);
    WAT_CTRL_SEEK : result:=Seek (wnd,lParam shr 16);
  else
    result:=0;
  end;
end;

end.

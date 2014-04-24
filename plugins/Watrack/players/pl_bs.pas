{BSPlayer player}
unit pl_BS;
{$include compilers.inc}

interface

implementation
uses windows,messages,common,srv_player,wat_api;

const
  HWND_MESSAGE = HWND(-3);
const
  BSPlayerClass = 'BSPlayer';
const
  WM_BSP_CMD = WM_USER+2;
  BSP_GETVERSION = $10000;
  BSP_GetMovLen  = $10100;
  BSP_GetMovPos  = $10101;
  BSP_GetStatus  = $10102;
  BSP_Seek       = $10103;

//  BSP_LoadPlaylist = $1010C;
  BSP_SetVol      = $10104;
  BSP_GetVol      = $10105;
  BSP_OpenFile    = $10108;
  BSP_GetFileName = $1010B;

  BSP_VolUp   = 1;
  BSP_VolDown = 2;
  BSP_Play    = 20;
  BSP_Pause   = 21;
  BSP_Stop    = 22;
  BSP_Prev    = 25;
  BSP_Next    = 28;

const
  bspwnd:HWND = 0;

function HiddenWindProc(wnd:HWND; msg:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
begin
  result:=DefWindowProc(wnd,msg,wParam,lParam);
end;

function Check(wnd:HWND;flags:integer):HWND;cdecl;
begin
  if wnd<>0 then
  begin
    result:=0;
    exit;
  end;
  result:=FindWindow(BSPlayerClass,NIL);
  if result=0 then
  begin
    if bspwnd<>0 then
    begin
      DestroyWindow(bspwnd);

      bspwnd:=0;
    end;
  end
  else if bspwnd=0 then
  begin
    bspwnd:=CreateWindowExW(0,'STATIC',nil,0,1,1,1,1,HWND_MESSAGE,0,hInstance,nil);
    if bspwnd<>0 then
      setwindowlongPtrW(bspwnd,GWL_WNDPROC,LONG_PTR(@HiddenWindProc));
  end;
end;

function GetVersion(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_BSP_CMD,BSP_GETVERSION,0)
end;

function GetVersionText(ver:integer):pWideChar;
begin
  mGetMem(result,11*SizeOf(WideChar));
  IntToHex(result,ver shr 24,2);
  result[2]:='.';
  IntToHex(result+3,(ver shr 16) and $FF,2);
  result[5]:='.';
  IntToHex(result+6,ver and $FFFF);
end;

function GetElapsedTime(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_BSP_CMD,BSP_GetMovPos,0) div 1000;
end;

function GetTotalTime(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_BSP_CMD,BSP_GetMovLen,0) div 1000;
end;

function GetStatus(wnd:HWND):integer; cdecl;
begin
  result:=SendMessage(wnd,WM_BSP_CMD,BSP_GetStatus,0);
  if      result=1 then result:=2
  else if result=2 then result:=1;
  if result>2 then result:=2;
end;

function GetFileName(wnd:HWND;flags:integer):pWideChar;
var
  cds:tcopyDataStruct;
  buf:array [0..255] of AnsiChar;
  adr:pointer;
begin
  adr:=@buf;
  cds.dwData:=BSP_GetFileName;
  cds.lpData:=@adr;
  cds.cbData:=4;
  SendMessage(wnd,WM_COPYDATA,bspwnd,lparam(@cds));

  AnsiToWide(buf,result);
end;

function Play(wnd:HWND;fname:PWideChar=nil):integer;
var
  cds:COPYDATASTRUCT;
begin
  if (fname<>nil) and (fname^<>#0) then
  begin
    cds.dwData:=BSP_OpenFile;
    WideToAnsi(fname,PAnsiChar(cds.lpData));
    cds.cbData:=StrLen(PAnsiChar(cds.lpData))+1;
    SendMessage(wnd,WM_COPYDATA,0{!!!},lparam(@cds));
    mFreeMem(cds.lpData);
  end;
  result:=SendMessage(wnd,WM_BSP_CMD,BSP_Play,0);
end;

function Pause(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_BSP_CMD,BSP_Pause,0);
end;

function Stop(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_BSP_CMD,BSP_Stop,0);
end;

function Next(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_BSP_CMD,BSP_Next,0);
end;

function Prev(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_BSP_CMD,BSP_Prev,0);
end;

function VolDn(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_BSP_CMD,BSP_VolDown,0);
end;

function VolUp(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_BSP_CMD,BSP_VolUp,0);
end;

function GetVolume(wnd:HWND):cardinal;
begin
  result:=SendMessage(wnd,WM_BSP_CMD,BSP_GetVol,0);
  result:=(result shl 16)+((result shl 4) div 25);
end;

procedure SetVolume(wnd:HWND;value:cardinal);
begin
  SendMessage(wnd,WM_BSP_CMD,BSP_SetVol,(value*25) shr 4);
end;

function Seek(wnd:HWND;value:integer):integer;
begin
  result:=SendMessage(wnd,WM_BSP_CMD,BSP_GetMovPos,0) div 1000;
  SendMessage(wnd,WM_BSP_CMD,BSP_Seek,value*1000);
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
    exit;
  end;

  with SongInfo do
  begin
    if (flags and WAT_OPT_CHANGES)<>0 then
    begin
      volume:=GetVolume(plwnd);
      if status<>WAT_PLS_STOPPED then
        time:=GetElapsedTime(plwnd);
    end
    else
    begin
      if total=0 then
        total:=GetTotalTime(plwnd);
    end;
  end;
end;

function Command(wnd:HWND;cmd:integer;value:int_ptr):integer;cdecl;
begin
  case cmd of
    WAT_CTRL_PREV : result:=Prev (wnd);
    WAT_CTRL_PLAY : result:=Play (wnd,pWideChar(value));
    WAT_CTRL_PAUSE: result:=Pause(wnd);
    WAT_CTRL_STOP : result:=Stop (wnd);
    WAT_CTRL_NEXT : result:=Next (wnd);
    WAT_CTRL_VOLDN: result:=VolDn(wnd);
    WAT_CTRL_VOLUP: result:=VolUp(wnd);
    WAT_CTRL_SEEK : result:=Seek (wnd,value);
  else
    result:=0;
  end;
end;

const
  plRec:tPlayerCell=(
    Desc     :'BSPlayer';
    flags    :WAT_OPT_HASURL;
    Icon     :0;
    Init     :nil;
    DeInit   :nil;
    Check    :@Check;
    GetStatus:@GetStatus;
    GetName  :@GetFileName;
    GetInfo  :@GetInfo;
    Command  :@Command;
    URL      :'http://www.bsplayer.org/';
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

{BeholderTV}
unit pl_behold;
{$include compilers.inc}

interface

implementation

uses windows,messages,common,wrapper,srv_player,wat_api;

const
  WM_BHCMD            = WM_USER+200;
  WMBH_CHNLUP         = WM_USER+203; // Переключить на следующий канал
  WMBH_CHNLDOWN       = WM_USER+204; // Переключить на предыдущий канал
  WMBH_VOLUMEUP       = WM_USER+210; // Увеличить выбранный уровень
  WMBH_VOLUMEDOWN     = WM_USER+211; // Уменьшить выбранный уровень
  WMBH_FREEZE         = WM_USER+232; // Триггер стоп-кадра
  WMBH_SETVOLUME      = WM_USER+280; // Установить уровень громкости (LParam = 0..65535)
  WMBH_GETVOLUME      = WM_USER+281; // Получить текущий уровень громкости (использовать SendMessage, Result = 0..65535)
  WMBH_GETVERSION     = WM_USER+285; // Получить новер версии ПО (использовать SendMessage)

const
  TitleWndClass = 'TApplication';
  EXEName       = 'BEHOLDTV.EXE';

var
  TitleWnd:HWND;

function enumproc(wnd:HWND; lParam:LPARAM):bool; stdcall;
var
  buf:array [0..64] of AnsiChar;
begin
  result:=true;
  if GetClassNameA(wnd,buf,63)<>0 then
  begin
    if StrCmp(buf,TitleWndClass)=0 then
    begin
      TitleWnd:=wnd;
      result:=false;
    end
  end;
end;

function Check(wnd:HWND;flags:integer):HWND;cdecl;
begin
  if wnd<>0 then
  begin
    result:=0;
    exit;
  end;
  result:=FindWindow('TMain','BeholdTV');
  if result<>0 then
    EnumThreadWindows(GetWindowThreadProcessId(result,nil),@enumproc,0);
end;

function GetVersion(wnd:HWND):integer;
begin
  result:=dword(SendMessage(wnd,WM_BHCMD,WMBH_GETVERSION,0));
  result:=((result shr 16) shl 8)+LoWord(result);
end;

function GetVersionText(ver:integer):PWideChar; //!!
begin
  mGetMem(result,10*SizeOf(WideChar));
  IntToStr(result+1,ver);
  result[0]:=result[1];
  result[1]:='.';
end;

function GetFileName(wnd:HWND;flags:integer):pWideChar;cdecl;
begin
  result:=nil;
end;

function Pause(wnd:HWND):integer;
begin
  result:=0;
  PostMessage(wnd,WM_BHCMD,WMBH_FREEZE,0);
end;

function Next(wnd:HWND):integer;
begin
  result:=0;
  PostMessage(wnd,WM_BHCMD,WMBH_CHNLUP,0);
end;

function Prev(wnd:HWND):integer;
begin
  result:=0;
  PostMessage(wnd,WM_BHCMD,WMBH_CHNLDOWN,0);
end;

function GetVolume(wnd:HWND):cardinal;
begin
  result:=word(SendMessage(wnd,WM_BHCMD,WMBH_GETVOLUME,0));
  result:=(result shl 16)+(result shr 12);
end;

procedure SetVolume(wnd:HWND;value:cardinal);
begin
  SendMessage(wnd,WM_BHCMD,WMBH_SETVOLUME,value shl 12);
end;

function VolDn(wnd:HWND):integer;
begin
  result:=word(SendMessage(wnd,WM_BHCMD,WMBH_VOLUMEDOWN,0));
end;

function VolUp(wnd:HWND):integer;
begin
  result:=word(SendMessage(wnd,WM_BHCMD,WMBH_VOLUMEUP,0));
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

  if (flags and WAT_OPT_CHANGES)<>0 then
    SongInfo.wndtext:=GetDlgText(TitleWnd);
end;

function Command(wnd:HWND;cmd:integer;value:integer):integer;cdecl;
begin
  case cmd of
    WAT_CTRL_PREV : result:=Prev(wnd);
//    WAT_CTRL_PLAY : result:=Play(wnd,pWideChar(value));
    WAT_CTRL_PAUSE: result:=Pause(wnd);
//    WAT_CTRL_STOP : result:=Stop(wnd);
    WAT_CTRL_NEXT : result:=Next(wnd);
    WAT_CTRL_VOLDN: result:=VolDn(wnd);
    WAT_CTRL_VOLUP: result:=VolUp(wnd);
//    WAT_CTRL_SEEK : result:=Seek(wnd,value);
  else
    result:=0;
  end;
end;

const
  plRec:tPlayerCell=(
    Desc     :'BeholdTV';
    flags    :WAT_OPT_HASURL;
    Icon     :0;
    Init     :nil;
    DeInit   :nil;
    Check    :@Check;
    GetStatus:nil;
    GetName  :@GetFileName;
    GetInfo  :@GetInfo;
    Command  :@Command;
    URL      :nil;
    Notes    :'Still experimental, no tested. Can work not properly');

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

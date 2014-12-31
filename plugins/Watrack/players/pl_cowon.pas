{COWON JetAudio player}
unit pl_cowon;
{$include compilers.inc}

interface

implementation
uses windows,winampapi,wrapper,messages,common,srv_player,wat_api;

const
  HWND_MESSAGE = HWND(-3);
const
  HOSTWND_CLASS	= 'TLB_JETAUDIO';

  CowonClass   = 'COWON Jet-Audio MainWnd Class';
  CowonTitle   = 'Afx:400000:8:0:0:'; // for example 'Afx:400000:8:0:0:18c300fd'
  ControlClass = 'COWON Jet-Audio Remocon Class';
  ControlName  = 'Jet-Audio Remote Control';
  PluginClass  = 'Streamzap.WMX';
  PluginName   = 'Jet-Audio Remote Plugin';
const
  MCI_STRING_OFFSET  = 512;
  MCI_MODE_NOT_READY = MCI_STRING_OFFSET + 12;
  MCI_MODE_STOP      = MCI_STRING_OFFSET + 13;
  MCI_MODE_PLAY      = MCI_STRING_OFFSET + 14;
  MCI_MODE_RECORD    = MCI_STRING_OFFSET + 15;
  MCI_MODE_SEEK      = MCI_STRING_OFFSET + 16;
  MCI_MODE_PAUSE     = MCI_STRING_OFFSET + 17;
  MCI_MODE_OPEN      = MCI_STRING_OFFSET + 18;

  WM_REMOCON_GETSTATUS   = WM_APP+740;
  WM_REMOCON_SENDCOMMAND = WM_APP+741;

  JRC_ID_STOP       = 5102;
  JRC_ID_PLAY       = 5104; // Track Number (>=1). Use 0 for normal playback.
  JRC_ID_PREV_TRACK = 5107;
  JRC_ID_NEXT_TRACK = 5108;
  JRC_ID_VOL_DOWN   = 5134;
  JRC_ID_VOL_UP     = 5135;
  JRC_ID_SET_VOLUME = 5180; // Volume Value (0 - 32)
  JRC_ID_SEEK       = 5192; // New position (second)
  JRC_ID_RANDOMMODE = 5117;

  GET_STATUS_STATUS         = 1;
  GET_STATUS_CUR_TRACK      = 8;
  GET_STATUS_CUR_TIME       = 9;
  GET_STATUS_MAX_TIME       = 10;
  GET_STATUS_TRACK_FILENAME = 11;
  GET_STATUS_TRACK_TITLE    = 12;
  GET_STATUS_TRACK_ARTIST   = 13;

  GET_STATUS_VOLUME = 127;

  JRC_COPYDATA_ID_ALBUMNAME      = $1000;
  JRC_COPYDATA_ID_GETVER         = $1002;
  JRC_COPYDATA_ID_TRACK_FILENAME = $3000;
  JRC_COPYDATA_ID_TRACK_TITLE    = $3001;
  JRC_COPYDATA_ID_TRACK_ARTIST   = $3002;

  PLAY_NORMAL = 0;
  PLAY_RANDOM = 1;

  GET_STATUS_JETAUDIO_VER1 = 995;
  GET_STATUS_JETAUDIO_VER2 = 996;
  GET_STATUS_JETAUDIO_VER3 = 997;

const
  TitleWnd:HWND = 0;
  HostWnd :HWND = 0;
  tmpstr  :pWideChar=nil;

function HiddenWindProc(wnd:HWND; msg:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  cds:PCOPYDATASTRUCT;
begin
  case msg of
    WM_COPYDATA: begin
      cds:=PCOPYDATASTRUCT(lParam);
      case cds^.dwData of
        JRC_COPYDATA_ID_TRACK_FILENAME,
        JRC_COPYDATA_ID_TRACK_TITLE   ,
        JRC_COPYDATA_ID_TRACK_ARTIST  : AnsiToWide(cds^.lpData,tmpstr);
      end;
      result:=1;
      exit;
    end;
  end;
  result:=DefWindowProc(wnd,msg,wParam,lParam);
end;

function Init:integer;cdecl;
begin
  HostWnd:=CreateWindowExW(0,'STATIC',nil,0,1,1,1,1,HWND_MESSAGE,0,hInstance,nil);
  if HostWnd<>0 then
    SetWindowLongPtrW(HostWnd,GWL_WNDPROC,LONG_PTR(@HiddenWindProc));
	result:=HostWnd;
end;

function DeInit:integer;cdecl;
begin
  result:=0;
  if HostWnd<>0 then
  begin
    DestroyWindow(HostWnd);
    HostWnd:=0;
  end;
end;

function chwnd(awnd:HWND;param:pdword):boolean; stdcall;
var
  s:array [0..255] of AnsiChar;
begin
  result:=true;
  if GetClassNameA(awnd,s,255)>0 then
  begin
    if StrCmp(s,CowonTitle,Length(CowonTitle))=0 then
    begin
      param^:=awnd;
      result:=false;
    end;
  end;
end;

function Check(wnd:HWND;flags:integer):HWND;cdecl;
begin
  if wnd<>0 then
  begin
    result:=0;
    exit;
  end;
  result:=FindWindow(ControlClass,ControlName);
{
  if result=0 then
    result:=FindWindow(PluginClass,PluginName);
}
  if (result<>0) {and (result<>wnd)} then
    if EnumWindows(@chwnd,int_ptr(@TitleWnd)) then
      TitleWnd:=0;
end;

function GetWndText:pWideChar;
var
  p:pWideChar;
begin
  result:=nil;
  if TitleWnd<>0 then
  begin
    result:=GetDlgText(TitleWnd);
    if result<>nil then
    begin
      if StrScanW(result,'[')<>nil then
      begin
        p:=StrScanW(result,']');
        if p<>nil then
        begin
          StrCopyW(result,p+1);
        end;
      end;
    end;
  end;
end;

function GetVersion(wnd:HWND):integer;
begin
  result:=               SendMessage(wnd,WM_REMOCON_GETSTATUS,0,GET_STATUS_JETAUDIO_VER1);
  result:=(result shl 8)+SendMessage(wnd,WM_REMOCON_GETSTATUS,0,GET_STATUS_JETAUDIO_VER2);
  result:=(result shl 8)+SendMessage(wnd,WM_REMOCON_GETSTATUS,0,GET_STATUS_JETAUDIO_VER3);
end;

function GetVersionText(ver:integer):PWideChar;
{var
  ver:integer;
begin
  ver:=GetVersion;
  mGetMem(result,11*SizeOf(WideChar));
  IntToHex(result,ver shr 16,2);
  result[2]:='.';
  IntToHex(result+3,(ver shr 8) and $FF,2);
  result[5]:='.';
  IntToHex(result+6,ver and $FF);
}
var
  s:array [0..31] of WideChar;
  i:integer;
begin
  i:=StrLenW(IntToStr(s,ver shr 16));
  s[i]:='.';
  i:=integer(StrLenW(IntToStr(pWideChar(@s[i+1]),(ver shr 8) and $FF)))+i+1;
  s[i]:='.';
  IntToStr(pWideChar(@s[i+1]),ver and $FF);
  StrDupW(result,PWideChar(@s));
end;

function GetStatus(wnd:HWND):integer; cdecl;
begin
  result:=SendMessage(wnd,WM_REMOCON_GETSTATUS,0,GET_STATUS_STATUS);
  case result of
    MCI_MODE_STOP : result:=WAT_PLS_STOPPED;
    MCI_MODE_PAUSE: result:=WAT_PLS_PAUSED;
    MCI_MODE_PLAY : result:=WAT_PLS_PLAYING;
  else
    result:=WAT_PLS_UNKNOWN;
  end;
end;

function GetFileName(wnd:HWND;flags:integer):pWideChar;cdecl;
begin
  SendMessage(wnd,WM_REMOCON_GETSTATUS,HostWnd,GET_STATUS_TRACK_FILENAME);
  result:=tmpstr;
end;

function GetArtist(wnd:HWND):pWideChar;
begin
  SendMessage(wnd,WM_REMOCON_GETSTATUS,HostWnd,GET_STATUS_TRACK_ARTIST);
  result:=tmpstr;
end;

function GetTitle(wnd:HWND):pWideChar;
begin
  SendMessage(wnd,WM_REMOCON_GETSTATUS,HostWnd,GET_STATUS_TRACK_TITLE);
  result:=tmpstr;
end;

function GetTrack(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_REMOCON_GETSTATUS,0,GET_STATUS_CUR_TRACK);
end;

function GetTotalTime(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_REMOCON_GETSTATUS,0,GET_STATUS_MAX_TIME) div 1000;
end;

function GetElapsedTime(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_REMOCON_GETSTATUS,0,GET_STATUS_CUR_TIME) div 1000;
end;

function Play(wnd:HWND;fname:PWideChar=nil):integer;
begin
  result:=SendMessage(wnd,WM_REMOCON_SENDCOMMAND,0,JRC_ID_PLAY);
end;

function Pause(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_REMOCON_SENDCOMMAND,0,JRC_ID_PLAY);
end;

function Stop(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_REMOCON_SENDCOMMAND,0,JRC_ID_STOP);
end;

function Next(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_REMOCON_SENDCOMMAND,0,JRC_ID_NEXT_TRACK);
end;

function Prev(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_REMOCON_SENDCOMMAND,0,JRC_ID_PREV_TRACK);
end;

function VolDn(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_REMOCON_SENDCOMMAND,0,JRC_ID_VOL_DOWN);
end;

function VolUp(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_REMOCON_SENDCOMMAND,0,JRC_ID_VOL_UP);
end;

function GetVolume(wnd:HWND):cardinal;
begin
  result:=SendMessage(wnd,WM_REMOCON_GETSTATUS,0,GET_STATUS_VOLUME);
  result:=(result shl 16)+(result shr 1);
end;

procedure SetVolume(wnd:HWND;value:cardinal);
begin
  SendMessage(wnd,WM_REMOCON_SENDCOMMAND,0,JRC_ID_SET_VOLUME+(value shl 17));
end;

function Seek(wnd:HWND;value:integer):integer;
begin
  result:=SendMessage(wnd,WM_REMOCON_SENDCOMMAND,0,JRC_ID_SEEK+(value shl 16));
end;
{
function Shuffle(setOn:integer):integer;
begin
  result:=SendMessage(wnd,WM_REMOCON_GETSTATUS,0,GETSTATUS_COMPONENT_RANDOMMODE);
  SendMessage(wnd,WM_REMOCON_SENDCOMMAND,JRC_ID_RANDOMMODE+(SetOn shl 16));
end;
}

function GetInfo(var SongInfo:tSongInfo;flags:integer):integer;cdecl;
var
  p:pWideChar;
begin
  result:=0;
//  result:=CallService(MS_WAT_WINAMPINFO,integer(@SongInfo),flags);
//  result:=WinampGetInfo(integer(@SongInfo),flags);
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
      wndtext:=GetWndText;
      volume :=GetVolume(plwnd);
      if status<>WAT_PLS_STOPPED then
//        if time=0 then
          time:=GetElapsedTime(plwnd);
    end
    else
    begin

      if artist=NIL then
      begin
        artist:=GetArtist(plwnd);
        if artist^=#0 then
          mFreeMem(artist);
      end;
      if title =NIL then
      begin
        title:=GetTitle(plwnd);
        p:=ExtractW(mfile,true);
        if (title^=#0) or (StrCmpW(title,p)=0) then
          mFreeMem(title);
        mFreeMem(p);
      end;

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
    Desc     :'Cowon JetAudio';
    flags    :WAT_OPT_HASURL;
    Icon     :0;
    Init     :@Init;
    DeInit   :@DeInit;
    Check    :@Check;
    GetStatus:@GetStatus;
    GetName  :@GetFileName;
    GetInfo  :@GetInfo;
    Command  :@Command;
    URL      :'http://www.jetaudio.com/';
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

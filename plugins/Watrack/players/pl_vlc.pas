{Video Lan player}
unit pl_VLC;
{$include compilers.inc}

interface

implementation
uses windows,common,srv_player,wat_api,syswin,wrapper
  {$IFDEF DELPHI_7_UP}
  ,variants
  {$ENDIF}
  {$IFDEF KOL_MCK}
  ,kolcomobj
  {$ELSE}
  ,ComObj
  {$ENDIF}
;

{
    procedure play; safecall;
    procedure pause; safecall;
    procedure stop; safecall;
    procedure playlistNext; safecall;
    procedure playlistPrev; safecall;
    property Playing: WordBool read Get_Playing;
    property Position: Single read Get_Position write Set_Position;
    property Time: SYSINT read Get_Time write Set_Time;
    property Length: SYSINT read Get_Length;
    (0)1-97(100)
    property Volume: SYSINT read Get_Volume write Set_Volume;
    property VersionInfo: WideString read Get_VersionInfo;
}

const
//  GuidOld: TGUID = '{E23FE9C6-778E-49D4-B537-38FCDE4887D8}';
  VLCClass = 'wxWindowClassNR';
  VLCName  = 'VLC media player';
  COMName  = 'VideoLAN.VLCPlugin.1'; // IVLCControl

//  GuidNew: TGUID = '{9BE31822-FDAD-461B-AD51-BE1D1C159921}';
  VLCClassSkin = 'SkinWindowClass';
  VLCClassNew  = 'QWidget';
  VLCEXEName   = 'VLC.EXE';
  COMNameNew   = 'VideoLAN.VLCPlugin2'; // IVLCControl2

function Check(wnd:HWND;flags:integer):HWND;cdecl;
var
  tmp,EXEName:PAnsiChar;
begin
  if wnd<>0 then
  begin
    result:=0;
    exit;
  end;
  result:=FindWindow(VLCClass,VLCName);
  if result=0 then
    result:=FindWindow(VLCClassSkin,nil); // VLCName
  if result=0 then
    result:=FindWindow(VLCClassNew,nil);
  if result<>0 then
  begin
    tmp:=Extract(GetEXEByWnd(result,EXEName),true);
    if lstrcmpia(tmp,VLCEXEName)<>0 then
      result:=0;
    mFreeMem(tmp);
    mFreeMem(EXEName);
  end;
{  if result<>0 then
  begin
    tmp:=Extract(GetEXEByWnd(result,EXEName),true);
    if lstrcmpia(tmp,'VLC.EXE')<>0 then
      result:=0;
    mFreeMem(tmp);
    mFreeMem(EXEName);
  end;
}
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
  try
    result:=SplitVersion(ver);
  except
    result:=0;
  end;
end;

function GetVersionText(const v:variant):PWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.VersionInfo)));
  except
    result:=nil;
  end;
end;

function GetWndText(wnd:HWND):pWideChar;
var
  p:pWideChar;
begin
  result:=GetDlgText(wnd);
{
need to clear  " - lalala VLC" at the end
}
  if result<>nil then
  begin
    p:=StrRScanW(result,'-');
    if p<>nil then // found
    begin
      if (p>result) and ((p-1)^=' ') and ((p+1)^=' ') then
        (p-1)^:=#0;
    end;
  end;
end;

{
function GetTotalTime:integer;
var
  v:variant;
begin
  try
    v:=CreateOleObject(COMName);
    result:=v.Length;
  except
    result:=inherited GetTotalTime;
  end;
  v:=Null;
end;

function GetElapsedTime:integer;
var
  v:variant;
begin
  try
    v:=CreateOleObject(COMName);
    result:=v.Time;
  except
    result:=inherited GetElapsedTime;
  end;
  v:=Null;
end;

function GetStatus:integer; cdecl;
var
  v:variant;
  tmp:boolean;
begin
  try
    v:=CreateOleObject(COMName);
    tmp:=v.Playing;
    if tmp then
      result:=WAT_PLS_PLAYING
    else
      result:=WAT_PLS_STOPPED;
  except
    result:=inherited GetStatus;
  end;
  v:=Null;
end;

function Play(fname:PWideChar=nil):integer;
var
  v:variant;
begin
  try
    v:=CreateOleObject(COMName);
    result:=v.play;
  except
    result:=inherited Play(fname);
  end;
  v:=Null;
end;

function Pause:integer;
var
  v:variant;
begin
  try
    v:=CreateOleObject(COMName);
    result:=v.pause;
  except
    result:=inherited Pause;
  end;
  v:=Null;
end;

function Stop:integer;
var
  v:variant;
begin
  try
    v:=CreateOleObject(COMName);
    result:=v.stop;
  except
    result:=inherited Stop;
  end;
  v:=Null;
end;

function Next:integer;
var
  v:variant;
begin
  try
    v:=CreateOleObject(COMName);
    result:=v.playlistNext;
  except
    result:=inherited Next;
  end;
  v:=Null;
end;

function Prev:integer;
var
  v:variant;
begin
  try
    v:=CreateOleObject(COMName);
    result:=v.playlistPrev;
  except
    result:=inherited Prev;
  end;
  v:=Null;
end;

function Seek(value:integer):integer;
var
  v:variant;
begin
  try
    v:=CreateOleObject(COMName);
    result:=v.Position;
    if value>0 then
      v.Position:=value
    else
      result:=0;
  except
    result:=inherited Seek(value);
  end;
  v:=Null;
end;

function GetVolume:cardinal;
var
  v:variant;
begin
  try
    v:=CreateOleObject(COMName);
    result:=v.Volume;
    result:=(result shl 16)+((result shl 4) div 100);
  except
    result:=0;
  end;
  v:=Null;
end;

procedure SetVolume(value:cardinal);
var
  v:variant;
begin
  try
    v:=CreateOleObject(COMName);
    v.Volume:=(value*100) shr 4;
    end;
  except
    result:=inherited SetVolume(value);
  end;
  v:=Null;
end;

function VolDn:integer;
var
  val:integer;
begin
  result:=GetVolume;
  val:=loword(result);
  if val>0 then
    SetVolume(val-1);
end;

function VolUp:integer;
var
  val:integer;
begin
  result:=GetVolume;
  val:=loword(result);
  if val<16 then
    SetVolume(val+1);
end;

}
function GetInfo(var SongInfo:tSongInfo;flags:integer):integer;cdecl;
var
  v:variant;
begin
  result:=0;
  if (flags and WAT_OPT_PLAYERDATA)<>0 then
  begin
    if SongInfo.plyver=0 then
    begin
      try
        try
          v:=CreateOleObject(COMName);
        except
          try
            v:=CreateOleObject(COMNameNew);
          except
            v:=Null;
          end;
        end;
        if v<>Null then
          with SongInfo do
          begin
            txtver:=GetVersionText(v);
            plyver:=GetVersion(txtver);
          end;
      except
      end;
      v:=Null;
      if (flags and WAT_OPT_CHANGES)<>0 then
        SongInfo.wndtext:=GetWndText(SongInfo.plwnd);
    end;
  end;
end;
{
function Command(wnd:HWND;cmd:integer;value:integer):integer;cdecl;
begin
  result:=0;
  case cmd of
    WAT_CTRL_PREV : result:=Prev;
    WAT_CTRL_PLAY : result:=Play(pWideChar(value));
    WAT_CTRL_PAUSE: result:=Pause;
    WAT_CTRL_STOP : result:=Stop;
    WAT_CTRL_NEXT : result:=Next;
    WAT_CTRL_VOLDN: result:=VolDn;
    WAT_CTRL_VOLUP: result:=VolUp;
    WAT_CTRL_SEEK : result:=Seek(value);
  end;
end;
}
const
  plRec:tPlayerCell=(
    Desc     :'VideoLAN player';
    flags    :WAT_OPT_HASURL;
    Icon     :0;
    Init     :nil;
    DeInit   :nil;
    Check    :@Check;
    GetStatus:nil;
    GetName  :nil;
    GetInfo  :@GetInfo;
    Command  :nil;
    URL      :'http://www.videolan.org/';
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

{Windows Media Player}
unit pl_WMP;
{$include compilers.inc}

interface

implementation
uses windows,common,messages,srv_player,wat_api
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
  WMPOld:boolean=false;

const
//  CLASS_MP :TGUID = '{22D6F312-B0F6-11D0-94AB-0080C74C7E95}';
//  CLASS_WMP:TGUID = '{6BF52A52-394A-11D3-B153-00C04F79FAA6}'; CLASS_WindowsMediaPlayer
  MPCOMName  = 'MediaPlayer.MediaPlayer.1';
  WMPCOMName = 'WMPlayer.OCX.7';
const
  MPClass  = 'Media Player 2';
  WMPClass = 'WMPlayerApp';

function Check(wnd:HWND;flags:integer):HWND;cdecl;
begin
  result:=FindWindowEx(0,wnd,MPClass,NIL);
  if result=0 then
  begin
    result:=FindWindowEx(0,wnd,WMPClass,NIL); //?
    WMPOld:=false;
  end
  else
    WMPOld:=true;
end;

{ Version detect
  fHasWMP64 = (WMP64.FileName="")  ' WMP64 was create above via OBJECT tag else this returns False.
  fHasWMP7 = (WMP7.URL = "")       ' WMP7 or later was create above via OBJECT tag else this returns False.
}

const
  MPVersion:PWideChar = '6.4';

function GetVersionText(flags:integer):PWideChar;
var
  v:variant;
begin
  if WMPOld then
    StrDupW(result,MPVersion)
  else
  begin
    try
      v:=CreateOleObject(WMPCOMName);
      StrDupW(result,pWideChar(Widestring(v.versionInfo)));
    except
      result:=nil;
    end;
//    VarClear(v);
    v:=Null;
  end;
end;

function GetInfo(var SongInfo:tSongInfo;flags:integer):integer;cdecl;
begin
  result:=0;
  if (flags and WAT_OPT_PLAYERDATA)<>0 then
  begin
    if SongInfo.txtver=nil then
      SongInfo.txtver:=GetVersionText(flags);
  end;
end;

{
function Command(wnd:HWND;cmd:integer;value:integer):integer;cdecl;
var
  c:integer;
begin
  result:=0;
  case cmd of
//    WAT_CTRL_PREV : c:=VK_B;
//    WAT_CTRL_PLAY : c:=VK_C;
//    WAT_CTRL_PAUSE: c:=VK_X;
//    WAT_CTRL_STOP : c:=VK_Z;
//    WAT_CTRL_NEXT : c:=VK_V;
    WAT_CTRL_VOLDN: c:=VK_F9;
    WAT_CTRL_VOLUP: c:=VK_F10;
    else
      exit;
  end;
  PostMessageW(wnd,WM_KEYDOWN,c,1);
end;
}
const
  plRec:tPlayerCell=(
    Desc     :'WMP';
    flags    :WAT_OPT_APPCOMMAND or WAT_OPT_SINGLEINST;
    Icon     :0;
    Init     :nil;
    DeInit   :nil;
    Check    :@Check;
    GetStatus:nil;
    GetName  :nil;
    GetInfo  :@GetInfo;
    Command  :nil;
    URL      :'http://www.microsoft.com/windows/windowsmedia/players.aspx';
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

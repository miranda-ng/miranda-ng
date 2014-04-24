{MediaMonkey player}
unit pl_mmonkey;
{$include compilers.inc}

interface

implementation
uses windows,messages,winampapi,common,srv_player,wat_api
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
  COMName:PWideChar = 'SongsDB.SDBApplication';

const
  WM_WA_IPC      = WM_USER;
  IPC_GETVERSION = 0;

{
const
  MMonkeyName = 'MediaMonkey';
}
function Check(wnd:HWND;flags:integer):HWND;cdecl;
//var
//  i:integer;
{
  EXEName:pWideChar;
  tmp:pWideChar;
}
//  v:Variant;
begin
  if wnd<>0 then
  begin
    result:=0;
    exit;
  end;
  result:=FindWindow('TFMainWindow','MediaMonkey');
  if result=0 then
    result:=FindWindow('TFMainWindow.UnicodeClass','MediaMonkey');
{
  wnd:=FindWindow(WinAmpClass,NIL);
  if wnd<>0 then
  begin
    if (SendMessage(wnd,WM_WA_IPC,0,IPC_GETVERSION) and $FF0F)<>$990B then
      wnd:=result;
  end;
}
{
  wnd:=FindWindow(WinAmpClass,NIL);
  if wnd<>0 then
  begin
    i:=SendMessage(wnd,WM_WA_IPC,0,IPC_GETVERSION) and $FF0F;
    if i=$990B then
    begin

      try
//        v:=GetActiveOleObject(COMName);
        v:=CreateOleObject(COMName);
        if not v.IsRunning then
          wnd:=0;
      except
      end;
      v:=Null;

    end
    else
      wnd:=0;
  end;
  result:=wnd;
{
  begin
    EXEName:=GetEXEByWnd(wnd);
    tmp:=Extract(EXEName,true);
    mFreeMem(EXEName);
    result:=StrCmpW(tmp,MMonkeyName,length(MMonkeyName))=0;
    mFreeMem(tmp);
  end;
}
end;

function GetVersion(const v:variant):integer;
begin
  try
    result:=(v.VersionHi shl 8)+(v.VersionLo shl 4)+v.VersionRelease;
  except
    result:=0;
  end;
end;

function GetVersionText(const v:variant):PWideChar;
begin
  try
    StrDupW(result,PWideChar(WideString(v.VersionString)));
  except
    result:=nil;
  end;
end;

function GetFileName(wnd:HWND;flags:integer):pWideChar;cdecl;
var
  v:Variant;
begin
  try
//    SDB:=GetActiveOleObject(COMName);
    v:=CreateOleObject(COMName);
    StrDupW(result,PWideChar(WideString(v.Player.CurrentSong.Path)));
  except
    result:=nil;
  end;
  v:=Null;
end;

function GetStatus(wnd:HWND):integer; cdecl;
begin
  result:=WinampGetStatus(wnd)
end;

function GetInfo(var SongInfo:tSongInfo;flags:integer):integer;cdecl;
var
  v:variant;
begin
  if (flags and WAT_OPT_PLAYERDATA)<>0 then
  begin
    if SongInfo.plyver=0 then
    begin
      try
        v:=CreateOleObject(COMName);
        with SongInfo do
        begin
          plyver:=GetVersion(v);
          txtver:=GetVersionText(v);
        end;
      except
      end;
      v:=Null;
    end;
  end;
  result:=WinampGetInfo(int_ptr(@SongInfo),flags);
end;

function Command(wnd:HWND;cmd:integer;value:integer):integer;cdecl;
begin
  result:=WinampCommand(wnd,cmd+(value shl 16));
end;

const
  plRec:tPlayerCell=(
    Desc     :'MediaMonkey';
    flags    :WAT_OPT_SINGLEINST or WAT_OPT_HASURL;
    Icon     :0;
    Init     :nil;
    DeInit   :nil;
    Check    :@Check;
    GetStatus:@GetStatus;
    GetName  :@GetFileName;
    GetInfo  :@GetInfo;
    Command  :@Command;
    URL      :'http://www.mediamonkey.com/';
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

{Winamp-like - base class}
unit pl_WinAmp;
{$include compilers.inc}

interface

implementation

uses winampapi,windows,common,messages,syswin,srv_player,wat_api;

{
#define IPC_GETPLAYLISTTITLE 212
/* (requires Winamp 2.04+, only usable from plug-ins (not external apps))
** char *name=SendMessage(hwnd_winamp,WM_WA_IPC,index,IPC_GETPLAYLISTTITLE);
**
** IPC_GETPLAYLISTTITLE gets the title of the playlist entry [index].
** returns a pointer to it. returns NULL on error.
*/
}
// class = BaseWindow_RootWnd
// title = Main Window

// ---------- check player ------------

function Check(wnd:HWND;aflags:integer):HWND;cdecl;
var
  tmp,EXEName:PAnsiChar;
begin
  result:=FindWindowEx(0,wnd,WinampClass,NIL);
  if result<>0 then
  begin
    tmp:=Extract(GetEXEByWnd(result,EXEName),true);
    if lstrcmpia(tmp,'WINAMP.EXE')<>0 then
      result:=0;
    mFreeMem(tmp);
    mFreeMem(EXEName);
  end;
end;

function WAnyCheck(wnd:HWND;aflags:integer):HWND;cdecl;
begin
  result:=FindWindowEx(0,wnd,WinampClass,NIL);
end;

// ----------- Get info ------------

function GetStatus(wnd:HWND):integer; cdecl;
begin
  result:=WinampGetStatus(wnd)
end;

function GetWidth(wnd:HWND):integer;
begin
  result:=LOWORD(SendMessage(wnd,WM_WA_IPC,3,IPC_GETINFO));
end;

function GetHeight(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_WA_IPC,3,IPC_GETINFO) shr 16;
end;

function GetInfo(var SongInfo:tSongInfo;aflags:integer):integer;cdecl;
begin
  SongInfo.winampwnd:=SongInfo.plwnd;
  result:=WinampGetInfo(int_ptr(@SongInfo),aflags);
  with SongInfo do
  begin
    if (aflags and WAT_OPT_CHANGES)<>0 then
      wndtext:=WinampGetWindowText(winampwnd)
    else
    begin
{
      if ((loword(plyver) shr 12)>=5) and 
         (SendMessage(wnd,WM_WA_IPC,0,IPC_IS_PLAYING_VIDEO)>1) then
      begin
        if width =0 then width :=GetWidth(wnd);
        if height=0 then Height:=GetHeight(wnd);
      end;
}
    end;
  end;
end;

// ------- Commands ----------

function Command(wnd:HWND;cmd:integer;value:integer):integer;cdecl;
begin
  result:=WinampCommand(wnd,cmd+(value shl 16));
end;

// ------- In-process code -------

function GetFileName(wnd:HWND;flags:integer):pWideChar;cdecl;
var
  fpos,fname:int_ptr;
  pid:dword;
  op:THANDLE;
  buf:array [0..1023] of AnsiChar;
  tmp:{$IFDEF FPC}longword{$ELSE}ulong_ptr{$ENDIF};
begin
  result:=nil;
  if (flags and WAT_OPT_IMPLANTANT)<>0 then
  begin
    if SendMessage(wnd,WM_WA_IPC,0,IPC_ISPLAYING)<>WAT_PLS_STOPPED then
    begin
      fpos :=SendMessage(wnd,WM_USER,0   ,IPC_GETLISTPOS);
      fname:=SendMessage(wnd,WM_USER,fpos,IPC_GETPLAYLISTFILE);
      GetWindowThreadProcessId(wnd,@pid);
      op:=OpenProcess(PROCESS_VM_READ,false,pid);
      if op<>0 then
      begin
        ReadProcessMemory(op,PByte(fname),@buf,SizeOf(buf),tmp);
        CloseHandle(op);
        if tmp>0 then
          AnsiToWide(buf,result);
      end;
    end;
  end;
end;

const
  plRec:tPlayerCell=(
    Desc     :'Winamp';
    flags    :WAT_OPT_ONLYONE or WAT_OPT_WINAMPAPI or WAT_OPT_HASURL;
    Icon     :0;
    Init     :nil;
    DeInit   :nil;
    Check    :@Check;
    GetStatus:@GetStatus;
    GetName  :@GetFileName;
    GetInfo  :@GetInfo;
    Command  :@Command;
    URL      :'http://www.winamp.com/';
    Notes    :nil);

const
  plRecClone:tPlayerCell=(
    Desc     :'Winamp Clone';
    flags    :WAT_OPT_ONLYONE or WAT_OPT_WINAMPAPI or WAT_OPT_LAST;
    Icon     :0;
    Init     :nil;
    DeInit   :nil;
    Check    :@WAnyCheck;
    GetStatus:@GetStatus;
    GetName  :nil;
    GetInfo  :@WinampGetInfo;
    Command  :@Command;
    URL      :nil;
    Notes    :'All "unknown" players using Winamp API');

var
  LocalPlayerLink,
  LocalPlayerLinkC:twPlayer;

procedure InitLink;
begin
  LocalPlayerLink.Next:=PlayerLink;
  LocalPlayerLink.This:=@plRec;
  PlayerLink          :=@LocalPlayerLink;

  LocalPlayerLinkC.Next:=PlayerLink;
  LocalPlayerLinkC.This:=@plRecClone;
  PlayerLink           :=@LocalPlayerLinkC;
end;

initialization
//  ServicePlayer(WAT_ACT_REGISTER,dword(@plRec));
//  ServicePlayer(WAT_ACT_REGISTER,dword(@plRecClone));
  InitLink;
end.

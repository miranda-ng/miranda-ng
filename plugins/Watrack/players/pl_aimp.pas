{AIMP player}
unit pl_AIMP;
{$include compilers.inc}

interface

implementation
uses windows,messages,common,srv_player,wat_api,winampapi;

const
  WM_AIMP_COMMAND = WM_USER + $75;
  WM_AIMP_GET_VERSION = 4;
  WM_AIMP_STATUS_GET  = 1;
  WM_AIMP_STATUS_SET  = 2;
  WM_AIMP_CALLFUNC    = 3;
const
  AIMP_STS_Player = 4;
  AIMP_STS_VOLUME = 1;
  AIMP_STS_POS    = 31;
const
  AIMP_PLAY  = 15;
  AIMP_PAUSE = 16;
  AIMP_STOP  = 17;
  AIMP_NEXT  = 18;
  AIMP_PREV  = 19;

const
  AIMP2_RemoteClass:PAnsiChar = 'AIMP2_RemoteInfo';
const
  AIMP2_RemoteFileSize = 2048;

type
  PAIMP2FileInfo = ^TAIMP2FileInfo;
  TAIMP2FileInfo = packed record
    cbSizeOF    :dword;
    //
    nActive     :LongBool;
    nBitRate    :dword;
    nChannels   :dword;
    nDuration   :dword;
    nFileSize   :Int64;
    nRating     :dword;
    nSampleRate :dword;
    nTrackID    :dword;
    //
    nAlbumLen   :dword;
    nArtistLen  :dword;
    nDateLen    :dword;
    nFileNameLen:dword;
    nGenreLen   :dword;
    nTitleLen   :dword;
    //
    sAlbum      :dword; // size of pointer for 32 bit system
    sArtist     :dword;
    sDate       :dword;
    sFileName   :dword;
    sGenre      :dword;
    sTitle      :dword;
  end;

function Check(wnd:HWND;flags:integer):HWND;cdecl;
begin
  if wnd<>0 then
  begin
    result:=0;
    exit;
  end;
  result:=FindWindowA(AIMP2_RemoteClass,AIMP2_RemoteClass);
end;

function GetVersionText(ver:integer):pWideChar;
begin
  if (ver and $F00)<>0 then
  begin
    mGetMem(result,8*SizeOf(WideChar));
    result[0]:=WideChar((ver div 1000)+ORD('0'));
    ver:=ver mod 1000;
    result[1]:='.';
    result[2]:=WideChar((ver div 100)+ORD('0'));
    ver:=ver mod 100;
    result[3]:='.';
    result[4]:=WideChar((ver div 10)+ORD('0'));
    result[5]:='.';
    result[6]:=WideChar((ver mod 10)+ORD('0'));
    result[7]:=#0;
  end
  else
    result:=nil;
end;

function GetVersion(wnd:HWND):integer;
begin
  result:=SendMessage(wnd,WM_AIMP_COMMAND,WM_AIMP_GET_VERSION,0);
end;

function GetStatus(wnd:HWND):integer; cdecl;
begin
  result:=SendMessage(wnd,WM_AIMP_COMMAND,WM_AIMP_STATUS_GET,AIMP_STS_Player);
end;

function GetVolume(wnd:HWND):cardinal;
begin
  result:=SendMessage(wnd,WM_AIMP_COMMAND,WM_AIMP_STATUS_GET,AIMP_STS_VOLUME);
  result:=(result shl 16)+round((result shl 4)/100);
end;

procedure SetVolume(wnd:HWND;value:cardinal);
begin
  SendMessage(wnd,WM_AIMP_COMMAND,WM_AIMP_STATUS_SET,
      (AIMP_STS_VOLUME shl 16)+((value*100) shr 4));
end;

function VolDn(wnd:HWND):integer;
var
  val:dword;
begin
  result:=GetVolume(wnd);
  val:=loword(result);
  if val>0 then
    SetVolume(wnd,val-1);
end;

function VolUp(wnd:HWND):integer;
var
  val:dword;
begin
  result:=GetVolume(wnd);
  val:=loword(result);
  if val<16 then
    SetVolume(wnd,val+1);
end;

function GetFileName(wnd:HWND;flags:integer):pWideChar;cdecl;
var
  FFile:THANDLE;
  pStr:pointer;
  s:integer;
  p:PAnsiChar;
  pw,pw1:pWideChar;
begin
  result:=nil;
  s:=AIMP2_RemoteFileSize;
  p:=AIMP2_RemoteClass;
  FFile:=OpenFileMappingA(FILE_MAP_READ,True,p);
  if FFile<>0 then
  begin
    pStr:=MapViewOfFile(FFile,FILE_MAP_READ,0,0,s);
    if pStr<>nil then
    begin
      try
        with PAIMP2FileInfo(pStr)^ do
        begin
          StrDupW(result,
            pWideChar(PAnsiChar(pStr)+SizeOf(TAIMP2FileInfo)+
               (nAlbumLen+nArtistLen+nDateLen)*SizeOf(WideChar)),
            nFileNameLen);
            // Delete rest index (like "filename.cue:3")
            pw :=StrRScanW(result,':');
            if pw<>nil then
            begin
              pw1:=StrScanW (result,':');
              if pw<>pw1 then
                pw^:=#0;
            end;
        end;
      except
      end;
      UnmapViewOfFile(pStr);
    end;
    CloseHandle(FFile);
  end;
end;

procedure TranslateRadio(var SongInfo:tSongInfo);
var
  pc,pc1:pWideChar;
begin
{
  artist - album - title (radio)
}
  with SongInfo  do
  begin
    if (artist=nil) and (title<>nil) then
    begin
      // Radio title
      if (StrEndW(title)-1)^=')' then
      begin
        pc:=StrRScanW(title,'(');
        if (pc<>nil) and (pc>title) and ((pc-1)^=' ') then
        begin
           if comment=nil then
           begin
             StrDupW(comment,pc+1);
             (StrEndW(comment)-1)^:=#0;
           end;
           (pc-1)^:=#0;
        end;
      end;
      // artist - title
      pc:=StrPosW(title,' - ');
      if pc<>nil then
      begin
        if artist=nil then
        begin
          pc^:=#0;
          inc(pc,3);
          StrDupW(artist,title);
        end;
        // artist - album - title
        pc1:=StrPosW(pc,' - ');
        if pc1<>nil then
        begin
          if album=nil then
          begin
            pc1^:=#0;
            StrDupW(album,pc);
            pc:=pc1+3;
          end;
        end;
        pc1:=title;
        StrDupW(title,pc);
        mFreeMem(pc1);
      end;
    end;
  end;
end;

function GetInfo(var SongInfo:tSongInfo;flags:integer):integer;cdecl;
var
  FFile:THANDLE;
  s:integer;
  p:PAnsiChar;
  pStr:PAIMP2FileInfo;
begin
  result:=0;
  if (flags and WAT_OPT_PLAYERDATA)<>0 then
  begin
    if SongInfo.plyver=0 then
    begin
      SongInfo.plyver:=GetVersion    (SongInfo.plwnd);
      SongInfo.txtver:=GetVersionText(SongInfo.plyver);
    end;
    if SongInfo.winampwnd=0 then
      SongInfo.winampwnd:=WinampFindWindow(SongInfo.plwnd);
    exit;
  end;

  if SongInfo.winampwnd<>0 then
    WinampGetInfo(int_ptr(@SongInfo),flags);
    
  if (flags and WAT_OPT_CHANGES)=0 then
  begin
    s:=AIMP2_RemoteFileSize;
    p:=AIMP2_RemoteClass;
    FFile:=OpenFileMappingA(FILE_MAP_READ,True,p);
    if FFile<>0 then
    begin
      pStr:=MapViewOfFile(FFile,FILE_MAP_READ,0,0,s);
      if pStr<>nil then
      begin
        try
          with SongInfo do
          begin
            with pStr^ do
            begin
              if channels=0 then channels:=nChannels;
              if kbps    =0 then kbps    :=nBitRate    div 1000;
              if khz     =0 then khz     :=nSampleRate div 1000;
              if total   =0 then total   :=nduration;
              if fsize   =0 then fsize   :=nFileSize;
              if track   =0 then track   :=nTrackID;

              with PAIMP2FileInfo(pStr)^ do
              begin
                if (artist=nil) and (nArtistLen>0) then
                begin
                  StrDupW(artist,
                    pWideChar(PAnsiChar(pStr)+SizeOf(TAIMP2FileInfo))+
                       nAlbumLen,nArtistLen);
                end;
                if (album=nil) and (nAlbumLen>0) then
                begin
                  StrDupW(album,
                    pWideChar(PAnsiChar(pStr)+SizeOf(TAIMP2FileInfo)),
                    nAlbumLen);
                end;
                if (title=nil) and (nTitleLen>0) then
                begin
                  StrDupW(title,
                    pWideChar(PAnsiChar(pStr)+SizeOf(TAIMP2FileInfo))+
                       nAlbumLen+nArtistLen+nDateLen+nFileNameLen+nGenreLen,
                    nTitleLen);
                end;
                if (year=nil) and (nDateLen>0) then
                begin
                  StrDupW(year,
                    pWideChar(PAnsiChar(pStr)+SizeOf(TAIMP2FileInfo))+
                       nAlbumLen+nArtistLen,
                    nDateLen);
                end;
                if (genre=nil) and (nGenreLen>0) then
                begin
                  StrDupW(genre,
                    pWideChar(PAnsiChar(pStr)+SizeOf(TAIMP2FileInfo))+
                       nAlbumLen+nArtistLen+nDateLen+nFileNameLen,
                    nGenreLen);
                end;

                if StrPosW(mfile,'://')<>nil then
                  TranslateRadio(SongInfo);
              end;
            end;
          end;
        except
        end;
        UnmapViewOfFile(pStr);
      end;
      CloseHandle(FFile);
    end;
  end
  else // request AIMP changed data: volume
  begin
    SongInfo.time:=SendMessage(SongInfo.plwnd,WM_AIMP_COMMAND,WM_AIMP_STATUS_GET,AIMP_STS_POS);
    SongInfo.volume:=GetVolume(SongInfo.plwnd);
  end;
end;

function Command(wnd:HWND;cmd:integer;value:integer):integer;cdecl;
var
  WinampWindow:HWND;
begin
  WinampWindow:=WinampFindWindow(wnd);
  if WinampWindow<>0 then
    result:=WinampCommand(WinampWindow,cmd+(value shl 16))
  else
  begin
    result:=0;
    case cmd of
      WAT_CTRL_PREV : SendMessage(wnd,WM_AIMP_COMMAND,WM_AIMP_CALLFUNC,AIMP_PREV);
      WAT_CTRL_PLAY : SendMessage(wnd,WM_AIMP_COMMAND,WM_AIMP_CALLFUNC,AIMP_PLAY);
      WAT_CTRL_PAUSE: SendMessage(wnd,WM_AIMP_COMMAND,WM_AIMP_CALLFUNC,AIMP_PAUSE);
      WAT_CTRL_STOP : SendMessage(wnd,WM_AIMP_COMMAND,WM_AIMP_CALLFUNC,AIMP_STOP);
      WAT_CTRL_NEXT : SendMessage(wnd,WM_AIMP_COMMAND,WM_AIMP_CALLFUNC,AIMP_NEXT);
      WAT_CTRL_VOLDN: result:=VolDn(wnd);
      WAT_CTRL_VOLUP: result:=VolUp(wnd);
      WAT_CTRL_SEEK : begin
        SendMessage(wnd,WM_AIMP_COMMAND,WM_AIMP_STATUS_SET,
                      (AIMP_STS_POS shl 16)+value);
      end;
    end;
  end;
end;

const
  plRec:tPlayerCell=(
    Desc     :'AIMP';
    flags    :WAT_OPT_APPCOMMAND or WAT_OPT_HASURL;
    Icon     :0;
    Init     :nil;
    DeInit   :nil;
    Check    :@Check;
    GetStatus:@GetStatus;
    GetName  :@GetFileName;
    GetInfo  :@GetInfo;
    Command  :@Command;
    URL      :'http://www.aimp.ru/';
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

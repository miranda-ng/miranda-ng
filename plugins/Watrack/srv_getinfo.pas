unit srv_getinfo;

interface

uses wat_api;

function GetPlayerInfo(var dst:tSongInfo;flags:cardinal):integer;
{
  WAT_RES_OK
  WAT_RES_NEWPLAYER
  WAT_RES_NOTFOUND
}

function GetFileInfo(var dst:tSongInfo;flags:cardinal;timeout:cardinal):integer;
{
  WAT_RES_OK
  WAT_RES_NEWFILE
  WAT_RES_UNKNOWN
  WAT_RES_NOTFOUND
}

function GetChangingInfo(var dst:tSongInfo;flags:cardinal):integer;
{
  WAT_RES_OK
}

function GetInfo(var dst:tSongInfo;flags:cardinal):integer;
{
  WAT_RES_OK
  WAT_RES_NEWFILE
}
function GetFileFormatInfo(var dst:tSongInfo):integer;
{
  WAT_RES_OK
  WAT_RES_NOTFOUND
}


implementation

uses
  windows,
  common, msninfo, syswin, wrapper, io, winampapi,
  srv_player, srv_format;


//----- get player info -----

function DefGetVersionText(ver:integer):pWideChar;
begin
  if ver<>0 then
  begin
    mGetMem(result,10*SizeOf(WideChar));
    IntToHex(result,ver);
  end
  else
    result:=nil;
end;

function GetPlayerInfo(var dst:tSongInfo;flags:cardinal):integer;
var
  plwnd:HWND;
  pl:pPlayerCell;
  PlayerChanged:bool;
begin
  plwnd:=CheckAllPlayers(flags,dst.status,PlayerChanged);

  if plwnd<>HWND(WAT_RES_NOTFOUND) then
  begin
    if PlayerChanged then
    begin
      ClearPlayerInfo(dst);

      pl:=GetActivePlayer;
      AnsiToWide(pl^.Desc,dst.player);
      dst.plwnd:=plwnd;
      FastAnsiToWide(pl^.URL,dst.url);
      if pl^.icon<>0 then
        dst.icon:=CopyIcon(pl^.icon);

      if pl^.GetInfo<>nil then
        tInfoProc(pl^.GetInfo)(dst,flags or WAT_OPT_PLAYERDATA)
      else if (pl^.flags and WAT_OPT_WINAMPAPI)<>0 then
        WinampGetInfo(wparam(@dst),flags or WAT_OPT_PLAYERDATA);
      
     if (pl^.flags and WAT_OPT_PLAYERINFO)=0 then
       if dst.txtver=NIL then dst.txtver:=DefGetVersionText(dst.plyver);

      result:=WAT_RES_NEWPLAYER;
    end
    else
    begin
      dst.plwnd:=plwnd; // to prevent same player, another instance
      result:=WAT_RES_OK;
    end
  end
  else
    result:=WAT_RES_NOTFOUND;
end;

//----- get file info -----

function GetFileInfo(var dst:tSongInfo;flags:cardinal;timeout:cardinal):integer;
var
  ftime:int64;
  f:THANDLE;
  fname:pWideChar;
  pl:pPlayerCell;
  remote,FileChanged:boolean;
  tmp:integer;
begin
  pl:=GetActivePlayer;

  if pl^.GetName<>nil then
    fname:=tNameProc(pl^.GetName)(dst.plwnd,flags)
  else
    fname:=nil;

  if (fname=nil) and (dst.plwnd<>0) then
  begin
   tmp:=0;
   if (flags and WAT_OPT_KEEPOLD)<>0 then tmp:=tmp or gffdOld;
   fname:=GetFileFromWnd(dst.plwnd,KnownFileType,tmp,timeout);
  end;

  if fname<>nil then
  begin
    remote:=StrPosW(fname,'://')<>nil;
    // file changing time (local/lan only)
    if not remote then
    begin
      f:=Reset(fname);

      if f<>THANDLE(INVALID_HANDLE_VALUE) then
      begin
        GetFileTime(f,nil,nil,@ftime);
        CloseHandle(f);
      end;
    end;
    // same file
    if (dst.mfile<>nil) and (lstrcmpiw(dst.mfile,fname)=0) then
    begin
      if (not remote) and ((flags and WAT_OPT_CHECKTIME)<>0) then
        FileChanged:=dst.date<>ftime
      else
        FileChanged:=false;
    end
    else  // new filename
    begin
      FileChanged:=true;
    end;

    // if not proper ext (we don't working with it)
    //!!!! check for remotes
    if (not remote) and (CheckExt(fname)=WAT_RES_NOTFOUND) then
    begin
      if (flags and WAT_OPT_UNKNOWNFMT)<>0 then
      begin
        ClearFileInfo(dst);

        dst.mfile:=fname;
        dst.date :=ftime;
        dst.fsize:=GetFSize(dst.mfile);

        result:=WAT_RES_UNKNOWN;
      end
      else
      begin
        mFreeMem(fname);
        result:=WAT_RES_NOTFOUND;
      end;
    end
    else if FileChanged {or isContainer(fname)} then
    begin
      ClearFileInfo(dst);

      dst.mfile:=fname; //!! must be when format recognized or remote
      dst.date :=ftime; //!!
      dst.fsize:=GetFSize(dst.mfile);
      result:=WAT_RES_NEWFILE;
    end
    else
    begin
      result:=WAT_RES_OK;
      mFreeMem(fname);
    end;
  end
  else
  begin
    result:=WAT_RES_NOTFOUND;
  end;
end;

//----- get changing info -----

function DefGetWndText(pl:pPlayerCell; wnd:HWND):pWideChar;
var
  p:pWideChar;
begin
  if wnd<>0 then
  begin
    result:=GetDlgText(wnd);
    if result<>nil then
    begin
      if (pl^.flags and WAT_OPT_TEMPLATE)<>0 then
      begin
        with pTmplCell(pl^.Check)^ do
        begin
          if p_prefix<>nil then
          begin
            p:=StrPosW(result,p_prefix);
            if p=result then
              StrCopyW(result,result+StrLenW(p_prefix));
          end;
          if p_postfix<>nil then
          begin
            p:=StrPosW(result,p_postfix);
            if p<>nil then
              p^:=#0;
          end;
        end;
      end;
    end;
  end
  else
    result:=nil;
end;

function GetChangingInfo(var dst:tSongInfo;flags:cardinal):integer;
var
  pl:pPlayerCell;
begin
  result:=WAT_RES_OK;

  ClearChangingInfo(dst);

  pl:=GetActivePlayer;

  if pl^.GetInfo<>nil then
    tInfoProc(pl^.GetInfo)(dst,flags or WAT_OPT_CHANGES)
  else if (pl^.flags and WAT_OPT_WINAMPAPI)<>0 then
    WinampGetInfo(wparam(@dst),flags or WAT_OPT_CHANGES);

  if (pl^.flags and WAT_OPT_PLAYERINFO)=0 then
    if dst.wndtext=NIL then dst.wndtext:=DefGetWndText(pl,dst.plwnd);
end;

//----- get track info -----

function GetSeparator(str:pWideChar):dword;
begin
  result:=StrIndexW(str,' '#$2013' ');
  if result=0 then
    result:=StrIndexW(str,' - ');
  if result<>0 then
  begin
    result:=result-1 + (3 SHL 16);
    exit;
  end;
  result:=StrIndexW(str,#$2013);
  if result=0 then
    result:=StrIndexW(str,'-');
  if result>0 then
    result:=result-1 + (1 SHL 16);
end;

function DefGetTitle(wnd:HWND;fname,wndtxt:pWideChar):pWideChar;
var
  i:integer;
  tmp:pWideChar;
begin
  if fname<>nil then
    tmp:=DeleteKnownExt(ExtractW(fname,true))
  else
    tmp:=wndtxt;
  if tmp=nil then
  begin
    result:=nil;
    exit;
  end;
  StrDupW(result,tmp);
  i:=GetSeparator(result);
  if i>0 then
    StrCopyW(result,result+LoWord(i)+HiWord(i));
  if fname<>nil then
    mFreeMem(tmp);
end;

function DefGetArtist(wnd:HWND;fname,wndtxt:pWideChar):pWideChar;
var
  i:integer;
  tmp:pWideChar;
begin
  if fname<>nil then
    tmp:=DeleteKnownExt(ExtractW(fname,true))
  else
    tmp:=wndtxt;
  if tmp=nil then
  begin
    result:=nil;
    exit;
  end;
  StrDupW(result,tmp);
  i:=GetSeparator(result);
  if i>0 then
    result[LoWord(i)]:=#0;
  if fname<>nil then
    mFreeMem(tmp);
end;

function GetFileFormatInfo(var dst:tSongInfo):integer;
var
  fmt:pMusicFormat;
begin
  result:=CheckExt(dst.mfile);
  if result=WAT_RES_OK then
  begin
    fmt:=GetActiveFormat;
    fmt^.proc(dst);
  end;
end;

function GetInfo(var dst:tSongInfo;flags:cardinal):integer;
var
  oldartist,oldtitle:pWideChar;
  fname:pWideChar;
  pl:pPlayerCell;
  lmsnInfo:pMSNInfo;
  remote:boolean;
begin
  result:=WAT_RES_OK;

  remote:=StrPosW(dst.mfile,'://')<>nil;

//  if remote or ((plyLink^[0].flags and WAT_OPT_PLAYERINFO)<>0) then
  oldartist:=dst.artist; oldtitle:=dst.title;

  ClearTrackInfo(dst);

  // info from player
  pl:=GetActivePlayer;
  if pl^.GetInfo<>nil then
    tInfoProc(pl^.GetInfo)(dst,flags and not WAT_OPT_CHANGES)
  else if (pl^.flags and WAT_OPT_WINAMPAPI)<>0 then
    WinampGetInfo(wparam(@dst),flags and not WAT_OPT_CHANGES);

  // info from file
  GetFileFormatInfo(dst);

  if (pl^.flags and WAT_OPT_PLAYERINFO)=0 then
    with dst do
    begin
      if remote then
        fname:=nil
      else
        fname:=mfile;

      lmsnInfo:=GetMSNInfo;

      if lmsnInfo<>nil then
      begin
        if artist=NIL then StrDupW(artist,lmsnInfo.msnArtist);
        if title =NIL then StrDupW(title ,lmsnInfo.msnTitle);
        if album =NIL then StrDupW(album ,lmsnInfo.msnAlbum);
      end;

      if artist=NIL then artist:=DefGetArtist(plwnd,fname,wndtext);
      if title =NIL then title :=DefGetTitle (plwnd,fname,wndtext);
    end;

  if remote or ((pl^.flags and WAT_OPT_PLAYERINFO)<>0) or
     isContainer(dst.mfile) then
  begin
    if (oldartist=oldtitle) or
       ((oldartist<>nil) and (StrCmpW(dst.artist,oldartist)<>0)) or
       ((oldtitle <>nil) and (StrCmpW(dst.title ,oldtitle )<>0)) then
    begin
      result:=WAT_RES_NEWFILE;
    end;
  end;
end;

end.

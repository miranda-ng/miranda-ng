{}
procedure Replace(dst:pWideChar;macro:integer;value:PWideChar);
var
  buf:array [0..63] of WideChar;
  pc:pWideChar;
begin
  buf[0]:='%';
  pc:=vars[macro].alias;
  if pc=nil then
    pc:=vars[macro].name;
  StrCopyW(buf+1,pc);
  pc:=StrEndW(buf);
  pc^:='%';
  (pc+1)^:=#0;
  StrReplaceW(dst,buf,value);
end;

function ReplaceAll(s:PWideChar):pWideChar;
var
  tmp:integer;
  pp,p:pWideChar;
  ws:array [0..127] of WideChar;
  ls:pWideChar;
  i:integer;
  tmpstr:pWideChar;
  Info:pSongInfo;
begin
  Info:=pointer(CallService(MS_WAT_RETURNGLOBAL,0,0));
  mGetMem(ls,32768);
  StrCopyW(ls,s);
  StrReplaceW(ls,'{tab}',#9);

  StrCopyW(ws,Info^.player);
  case PlayerCaps of
    1: LowerCase(ws);
    2: UpperCase(ws);
  end;
  Replace(ls,mn_player,ws);
  Replace(ls,mn_file  ,Info^.mfile);
  Replace(ls,mn_year  ,Info^.year);
  Replace(ls,mn_genre ,Info^.genre);
  GetExt(Info^.mfile,ws);
  if LoCaseType=BST_CHECKED then
    LowerCase(ws)
  else
    UpperCase(ws);
  Replace(ls,mn_type ,ws);
  Replace(ls,mn_track,IntToStr(ws,Info^.track));
// codec
  ws[0]:=WideChar( Info^.codec and $FF);
  ws[1]:=WideChar((Info^.codec shr  8) and $FF);
  ws[2]:=WideChar((Info^.codec shr 16) and $FF);
  ws[3]:=WideChar((Info^.codec shr 24) and $FF);
  ws[4]:=#0;
  //fps
  IntToStr(ws,Info^.fps div 100);
  i:=0;
  repeat
    inc(i);
  until ws[i]=#0;
  ws[i]:='.';
  IntToStr(pWideChar(@ws[i+1]),Info^.fps mod 100);
  Replace(ls,mn_fps    ,ws);
  Replace(ls,mn_txtver ,Info^.txtver);
  Replace(ls,mn_height ,IntToStr(ws,Info^.height));
  Replace(ls,mn_width  ,IntToStr(ws,Info^.width));
  Replace(ls,mn_kbps   ,IntToStr(ws,Info^.kbps));
  Replace(ls,mn_bitrate,ws);
  if Info^.vbr<>0 then
    p:=chVBR
  else if WriteCBR=0 then
    p:=nil
  else
    p:=chCBR;
  Replace(ls,mn_vbr       ,p);
  Replace(ls,mn_khz       ,IntToStr(ws,Info^.khz));
  Replace(ls,mn_samplerate,ws);
  Replace(ls,mn_channels  ,IntToStr(ws,Info^.channels));
  case Info^.channels of
    1:   p:=chMono;
    2:   p:=chStereo;
    5,6: p:=ch51;
  end;
  Replace(ls,mn_mono,p);
  Replace(ls,mn_size,
    IntToK(ws,Info^.fsize,FSizeMode,FSPrecision,FSizePost));
  Replace(ls,mn_length,IntToTime(ws,Info^.total));
  Replace(ls,mn_total ,ws);
  case Info^.status of
    WAT_PLS_PLAYING: pp:=splPlaying;
    WAT_PLS_PAUSED : pp:=splPaused;
  else
    {WAT_PLS_STOPPED:} pp:=splStopped;
  end;
  Replace(ls,mn_status,TranslateW(pp));
  Replace(ls,mn_nstatus,pp);
  Replace(ls,mn_lyric ,Info^.lyric);
  Replace(ls,mn_cover ,Info^.cover);
  Replace(ls,mn_volume,IntToStr(ws,loword(Info^.volume)));

  mGetMem(tmpstr,32767);

  StrCopyW(tmpstr,Info^.artist);
  if ReplaceSpc=BST_CHECKED then CharReplaceW(tmpstr ,'_',' ');
  Replace(ls,mn_artist,tmpstr);

  StrCopyW(tmpstr,Info^.title);
  if ReplaceSpc=BST_CHECKED then CharReplaceW(tmpstr ,'_',' ');
  Replace(ls,mn_title,tmpstr);

  StrCopyW(tmpstr,Info^.album);
  if ReplaceSpc=BST_CHECKED then CharReplaceW(tmpstr ,'_',' ');
  Replace(ls,mn_album,tmpstr);

  StrCopyW(tmpstr,Info^.comment);
  if ReplaceSpc=BST_CHECKED then CharReplaceW(tmpstr ,'_',' ');
  Replace(ls,mn_comment,tmpstr);

  StrCopyW(tmpstr,Info^.wndtext);
  if ReplaceSpc=BST_CHECKED then CharReplaceW(tmpstr ,'_',' ');
  Replace(ls,mn_wndtext,tmpstr);

  mFreeMem(tmpstr);

  Replace(ls,mn_version,IntToHex(ws,Info^.plyver));
  Replace(ls,mn_time   ,IntToTime(ws,Info^.time));
  if Info^.total>0 then
    tmp:=(Info^.time*100) div Info^.total
  else
    tmp:=0;
  Replace(ls,mn_percent,IntToStr(ws,tmp));
  Replace(ls,mn_playerhome,Info^.url);

  result:=ls;
end;

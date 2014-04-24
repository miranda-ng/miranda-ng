{Variables support}
function GetField(ai:PARGUMENTSINFO):int_ptr; cdecl;
var
  i,j:integer;
  res,ws:pWideChar;
  s:array [0..31] of WideChar;
  rs:boolean;
  si:pSongInfo;
begin
  i:=0;
  repeat
    ws:=vars[i].alias;
    if ws=nil then
      ws:=vars[i].name;
    if lstrcmpiw(PWideChar(ai^.argv^),ws)=0 then
      break;
    inc(i);
  until i=numvars;
  ws:=nil;
  j:=-1;
  rs:=true;
  si:=pointer(CallService(MS_WAT_RETURNGLOBAL,0,0));
  case i of
    mn_wndtext: ws:=si^.wndtext;
    mn_artist : ws:=si^.artist;
    mn_title  : ws:=si^.title;
    mn_album  : ws:=si^.album;
    mn_genre  : ws:=si^.genre;
    mn_file   : begin ws:=si^.mfile; rs:=false; end;
    mn_year   : ws:=si^.year;
    mn_comment: ws:=si^.comment;
    mn_player : begin
      StrCopyW(s,si^.player);
      case PlayerCaps of
        1: LowerCase(s);
        2: UpperCase(s);
      end;
      ws:=@s;
    end;
    mn_lyric  : ws:=si^.lyric;
    mn_cover  : ws:=si^.cover;
    mn_txtver : ws:=si^.txtver;
    mn_type: begin
      GetExt(si^.mfile,s);
      if LoCaseType=BST_CHECKED then
        LowerCase(s);
//      else
//        UpperCase(s);
      ws:=@s;
    end;
    mn_size: begin
      IntToK(s,si^.fsize,FSizeMode,FSPrecision,FSizePost);
      ws:=@s;
    end;
    mn_fps: begin
      IntToStr(s,si^.fps div 100);
      ws:=@s;
      while ws^<>#0 do inc(ws);
      ws^:='.';
      IntToStr(ws+1,si^.fps mod 100);
      ws:=@s;
    end;
    mn_codec: begin
      s[0]:=WideChar( si^.codec and $FF);
      s[1]:=WideChar((si^.codec shr  8) and $FF);
      s[2]:=WideChar((si^.codec shr 16) and $FF);
      s[3]:=WideChar((si^.codec shr 24) and $FF);
      s[4]:=#0;
      ws:=@s;
    end;
    mn_vbr: if si^.vbr<>0 then
        ws:=chVBR
      else if WriteCBR<>0 then
        ws:=chCBR;
    mn_status: case si^.status of
      WAT_PLS_STOPPED: ws:=TranslateW(splStopped);
      WAT_PLS_PLAYING: ws:=TranslateW(splPlaying);
      WAT_PLS_PAUSED : ws:=TranslateW(splPaused);
    end;
    mn_nstatus: case si^.status of
      WAT_PLS_STOPPED: ws:=splStopped;
      WAT_PLS_PLAYING: ws:=splPlaying;
      WAT_PLS_PAUSED : ws:=splPaused;
    end;
    mn_mono: begin
      case si^.channels of
        1:   ws:=chMono;
        2:   ws:=chStereo;
        5,6: ws:=ch51;
      end;
    end;
    mn_playerhome: ws:=si^.url;
  else
    begin
      case i of
        mn_volume    : j:=loword(si^.volume);
        mn_width     : j:=si^.width;
        mn_height    : j:=si^.height;
        mn_kbps,
        mn_bitrate   : j:=si^.kbps;
        mn_khz,
        mn_samplerate: j:=si^.khz;
        mn_channels  : j:=si^.channels;
        mn_track     : j:=si^.track;
        mn_percent: begin
          if si^.total>0 then
            j:=(si^.time*100) div si^.total
          else
            j:=0;
        end;
      else
        begin
          case i of
            mn_total,
            mn_length : IntToTime(s,si^.total);
            mn_time   : IntToTime(s,si^.time);
            mn_version: IntToHex (s,si^.plyver);
          else
            result:=0;
            exit;
          end;
          ws:=@s;
        end;
      end;
    end;
  end;
  if (ws=nil) and (j>=0) then
  begin
    IntToStr(s,j);
    ws:=@s;
  end;

  StrDupW(ws,ws);
  If rs and (ReplaceSpc=BST_CHECKED) then
    CharReplaceW(ws,'_',' ');
  i:=StrLenW(ws);
  mGetMem(res,(i+1)*SizeOf(WideChar));
  if ws<>nil then
  begin
    StrCopyW(res,ws);
    mFreeMem(ws);
  end
  else
    res[0]:=#0;
  result:=int_ptr(res);
end;

function FreeField(szReturn:PAnsiChar):int; cdecl;
begin
  mFreeMem(szReturn);
  result:=1;
end;

procedure RegisterVariables;
const
  Prefix:PAnsiChar = 'WATrack'#9;
var
  rt:TTOKENREGISTER;
  i,j:integer;
  s:array [0..127] of AnsiChar;
  p:pvar;
begin
  if not isVarsInstalled then
    exit;

  rt.cbSize          :=SizeOf(rt);
  rt.memType         :=TR_MEM_OWNER;
  rt.flags           :=TRF_FIELD or TRF_CLEANUP or
                       TRF_UNICODE or TRF_PARSEFUNC or TRF_CLEANUPFUNC;
  rt.szService       :=@GetField;
  rt.szCleanupService:=@FreeField;
  j:=StrLen(Prefix);
  move(Prefix^,s,j);
  rt.szHelpText:=@s;
  for i:=0 to numvars-1 do
  begin
    p:=@vars[i];
    rt.szTokenString.w:=p.alias;
    if rt.szTokenString.w=nil then
      rt.szTokenString.w:=p.name;
    if p.help<>nil then
      StrCopy(s+j,p.help);
    CallService(MS_VARS_REGISTERTOKEN,0,lparam(@rt));
  end;
end;

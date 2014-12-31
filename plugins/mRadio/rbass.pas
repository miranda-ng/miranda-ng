{BASS dll code}
unit rbass;

interface

uses
  windows;

procedure BassError(text:PWideChar);
procedure OpenURL(url:PWideChar); cdecl;
procedure StopStation;
function GetMusicFormat:PAnsiChar;
function ConstructFilter:pointer;

procedure EQ_ON;
procedure EQ_OFF;
procedure SetSndVol(arg:integer);
procedure SetBassConfig;

procedure MyFreeBass;
function MyInitBass:bool;
procedure MyUnloadBass;
function MyLoadBass:bool;
function CheckBassStatus:bool;

implementation

uses
  common, m_api, io
  ,dbsettings, mirutils, wrapdlgs
  ,Dynamic_Bass,dynbasswma
  ,rglobal
  ;

const
  signMP3    = $FBFF;
  signID3    = $00334449;
  signOGG    = $5367674F;
  OGGHdrSize = 26; // +1=NumSegments
const
  BASSName = 'bass.dll';
  StationHeader:PByte=nil;
var
  hdrlen:integer;
  syncMETA,
  syncOGG,
  syncWMA,
  syncWMA1,
  syncEND:HSYNC;
  SaveHeader:bool;

const
  proxy:pAnsiChar = nil;
const
  hRecord:THANDLE = 0;

procedure BassError(text:PWideChar);
begin
  MessageboxW(0,TranslateW(text),TranslateW('Sorry!'),MB_ICONERROR)
end;

// now called from BASS_Init only, so no dll load check
procedure ErrorCustom(text:pWideChar=nil);
var
  buf:array [0..255] of WideChar;
  pcw:pWideChar;
  idx:integer;
begin
  idx:=BASS_ErrorGetCode();
  if (idx<0) or (idx>BASS_ERROR_MAXNUMBER) then
  begin
    if text=nil then
      pcw:='Unknown error'
    else
      pcw:=text;
  end
  else
    pcw:=FastAnsiToWideBuf(BASS_ERRORS[idx],@buf);

  MessageBoxW(0,TranslateW(pcw),
      TranslateW('Oops! BASS error'),MB_ICONERROR)
end;



procedure SetBassConfig;
begin
  if BassStatus=rbs_null then exit;

  BASS_SetConfig(BASS_CONFIG_NET_PREBUF ,sPreBuf);
  BASS_SetConfig(BASS_CONFIG_NET_BUFFER ,sBuffer);
  BASS_SetConfig(BASS_CONFIG_NET_TIMEOUT,sTimeout);
end;

procedure SetSndVol(arg:integer);
begin
  if arg<0 then
    arg:=gVolume
  else
    gVolume:=arg;

  // no BASS dll - no channel
  if chan<>0 then
  begin
    if arg<0 then arg:=0;
    BASS_ChannelSetAttribute(chan,BASS_ATTRIB_VOL,arg/100);
  end;
end;

procedure EQ_OFF;
var
  i:cardinal;
begin
  // no BASS dll - no channel
  if chan<>0 then
    for i:=0 to 9 do
      BASS_ChannelRemoveFX(chan,eq[i].fx);
  isEQ_OFF:=BST_CHECKED;
end;

procedure EQ_ON;
var
  i:cardinal;
begin
  // no BASS dll - no channel
  if chan<>0 then
  begin
    for i:=0 to 9 do
      eq[i].fx:=BASS_ChannelSetFX(chan,BASS_FX_DX8_PARAMEQ,1);
    for i:=0 to 9 do
      BASS_FXSetParameters(eq[i].fx,@eq[i].param);
    isEQ_OFF:=BST_UNCHECKED;
  end;
end;

procedure MyStopBass;
begin
  if BassStatus=rbs_null then exit;

  if ActiveContact<>0 then
  begin
    if syncMETA<>0 then
    begin
      BASS_ChannelRemoveSync(chan,syncMETA);
      syncMETA:=0
    end;
    if syncEND<>0 then
    begin
      BASS_ChannelRemoveSync(chan,syncEND);
      syncEND:=0
    end;
    if syncWMA<>0 then
    begin
      BASS_ChannelRemoveSync(chan,syncWMA);
      syncWMA:=0
    end;
    if syncOGG<>0 then
    begin
      BASS_ChannelRemoveSync(chan,syncOGG);
      syncOGG:=0
    end;
    if syncWMA1<>0 then
    begin
      BASS_ChannelRemoveSync(chan,syncWMA1);
      syncWMA1:=0
    end;
  end;
end;

procedure MyFreeBass;
begin
  if BassStatus=rbs_init then
  begin
    MyStopBass;
    BASS_Free;

    BassStatus:=rbs_load;
  end;
end;

procedure StopStation;
begin
  if chan<>0 then
    BASS_StreamFree(chan); // close old stream
  chan:=0;
  mFreeMem(StationHeader);
  mFreeMem(ActiveURL);
  DBDeleteSetting(ActiveContact,strCList,optStatusMsg);
  MyStopBass;
end;

function GetDeviceNumber:integer;
var
  i:integer;
  info:BASS_DEVICEINFO;
begin
  // default device
  result:=-1;
  i:=1; //  0 is always the "no sound" device
  repeat
    if not BASS_GetDeviceInfo(i,info) then
      break;
    if (info.flags and BASS_DEVICE_ENABLED)<>0 then
      if StrCmp(info.name,usedevice)=0 then
      begin
        // not default device
        if (info.flags and BASS_DEVICE_DEFAULT)=0 then
          result:=i;
        break;
      end;
    inc(i);
  until false;
end;

function MyInitBass:bool;
var
  num:integer;
begin
  if BassStatus=rbs_null then
  begin
    result:=false;
    exit;
    // or can do this:
    MyLoadBass;
    if BassStatus=rbs_null then
    begin
      result:=false;
      exit;
    end;
  end;

  if BassStatus=rbs_init then
  begin
    result:=true;
    exit;
  end;

  num:=GetDeviceNumber;
  // Bass_ErrorGetCode=BASS_ERROR_NO3D
  result:=BASS_Init(num,44100,BASS_DEVICE_3D,0,nil);
  if (not result) and (Bass_ErrorGetCode()=BASS_ERROR_NO3D) then
    result:=BASS_Init(num,44100,0,0,nil);
  // not default device choosed - check default now
  if (not result) and (num>=0) then
  begin
    result:=BASS_Init(-1,44100,BASS_DEVICE_3D,0,nil);
    if (not result) and (Bass_ErrorGetCode()=BASS_ERROR_NO3D) then
      result:=BASS_Init(-1,44100,0,0,nil);
  end;

  // BASS interface compatibility
  if not result then
  begin
    if BASS_ErrorGetCode()=BASS_ERROR_ALREADY then
    begin
      result:=true;
    end;
  end;

  if not result then
  begin
    ErrorCustom('Can''t initialize device');
  end
  else
  begin
    num:=DBReadByte(0,PluginName,optEAXType,0);
    if num=0 then
      BASS_SetEAXParameters(-1,0,-1,-1)
    else
      BASS_SetEAXPreset(EAXItems[num].code);
  end;

  if result then
    BassStatus:=rbs_init;
end;

procedure MyUnloadBass;
begin
  MyFreeBass;
  if BassStatus=rbs_load then
  begin
    mFreeMem(proxy);

    BASS_PluginFree(0);
    Unload_BASSDLL;

    BassStatus:=rbs_null;
  end;
end;

function MyLoadBass:bool;
var
  pc,custom:PWideChar;
  basspath:PWideChar;
  buf:array [0..MAX_PATH-1] of WideChar;
  fh:THANDLE;
  fd:TWin32FindDataW;
begin
  if BassStatus<>rbs_null then
  begin
    result:=true;
    exit;
  end;

  mGetMem(basspath,1024);

  // trying to load Bass.dll from custom
  custom:=DBReadUnicode(0,PluginName,optBASSPath,nil);
  if custom<>nil then
  begin
    pc:=StrCopyEW(basspath,custom);
    if (pc-1)^<>'\' then
    begin
      pc^:='\';
      inc(pc);
    end;
    StrCopyW(pc,BASSName);

    result:=Load_BASSDLL(basspath);
  end
  else
    result:=false;

  if not result then
  begin
    GetModuleFileNameW(0,buf,MAX_PATH-1);
    pc:=StrEndW(buf);
    repeat
      dec(pc);
    until pc^='\';
    inc(pc);
    pc^:=#0;

    pc:=StrCopyW(StrCopyEW(basspath,buf),BASSName);    // %miranda_path%\
    result:=Load_BASSDLL(basspath);
    if not result then
    begin
      pc:=StrCopyW(StrCopyEW(pc,'plugins\'),BASSName); // %miranda_path%\plugins\
      result:=Load_BASSDLL(basspath);
      if not result then
      begin
        pc:=StrCopyW(StrCopyEW(pc,'bass\'),BASSName);  // %miranda_path%\plugins\bass\
        result:=Load_BASSDLL(basspath);
      end;
    end;
  end;

  // not found but custom path is empty
  if (not result) and (custom=nil) then
  begin
    if MessageboxW(0,TranslateW('BASS.DLL not found! Choose BASS.dll path manually'),
        cPluginName,MB_YESNO)=IDYES then
    begin
      pc:=nil;
      if SelectDirectory(TranslateW('Choose BASS.dll path'),pc,0) then
      begin
        //!! if options page opened, need to change edit field

        PathToRelativeW(pc,buf);
        pc:=StrCopyEW(basspath,buf);
        if (pc-1)^<>'\' then
        begin
          pc^:='\';
          inc(pc);
        end;
        pc^:=#0;
        DBWriteUnicode(0,PluginName,optBASSPath,basspath);
        StrCopyW(pc,BASSName);

        result:=Load_BASSDLL(basspath);
      end;
    end;
  end;

  // check Bass.dll version
  if result then
  begin
    if (BASS_GetVersion shr 16)<BASSVERSION then
    begin
      Unload_BASSDLL;
      result:=false;
      BassError('Wrong version of BASS.DLL');
    end
    else
    begin
      // load Bass plugins
      pc:=StrCopyW(pc,'bass*.dll');
      fh:=FindFirstFileW(basspath,fd);
      if fh<>THANDLE(INVALID_HANDLE_VALUE) then
      begin
        repeat
          StrCopyW(pc,fd.cFileName);
          if BASS_PluginLoad(pAnsiChar(basspath),BASS_UNICODE)=0 then
;//            break; // ignore if unsuccefull plugin load
        until not FindNextFileW(fh,fd);
        FindClose(fh);
      end;

      // enable ASX processing (if WMA loaded)
      BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST, 2); // 2 - enable internet and local playlists
    end;
  end
  else
  begin
    BassError('BASS.DLL not found!');
  end;

  mFreeMem(custom);
  mFreeMem(basspath);

  if result then
    BassStatus:=rbs_load;
end;

function CheckBassStatus:bool;
begin
  if BassStatus=rbs_null then
    MyLoadBass;

  if BassStatus<>rbs_null then
  begin
    SetBassConfig;

    mFreeMem(proxy);
    proxy:=GetProxy(hNetLib);
    BASS_SetConfigPtr(BASS_CONFIG_NET_PROXY,proxy);
  end;

  if BassStatus=rbs_load then
    MyInitBass;

  result:=BassStatus<>rbs_null;
end;

function GetMusicFormat:PAnsiChar;
var
  bci:BASS_CHANNELINFO;
begin
  BASS_ChannelGetInfo(chan,bci);
  case bci.ctype of
    BASS_CTYPE_STREAM_OGG: result:='OGG';
    BASS_CTYPE_STREAM_MP1,
    BASS_CTYPE_STREAM_MP2,
    BASS_CTYPE_STREAM_MP3: result:='MP3';
    BASS_CTYPE_STREAM_WMA,
    BASS_CTYPE_STREAM_WMA_MP3: result:='WMA';
    {BASS_CTYPE_STREAM_AAC,}$10b00: result:='AAC';
    {BASS_CTYPE_STREAM_MP4:}$10b01: result:='MP4';
    {BASS_CTYPE_STREAM_AC3:}$11000: result:='AC3';
  else
    result:=nil;
  end;
end;

function GetFileExt(buf:pWideChar;sign:pointer):pWideChar;
var
  pc:pAnsiChar;
begin
  result:=buf;
  pc:=GetMusicFormat;
  if pc=nil then
  begin
    StrCopyW(buf,'sav');
    if sign<>nil then
    begin
      if pdword(sign)^=signOGG then
        StrCopyW(buf,'ogg')
      else if ((pdword(sign)^ and $00FFFFFF)=signID3) or (pword(sign)^=signMP3) then
        StrCopyW(buf,'mp3');
    end;
  end
  else
  begin
    FastAnsiToWideBuf(pc,buf);
    LowerCase(buf);
  end;
end;

function MakeFileName(sign:pointer):pWideChar;
var
  p,pcw:PWideChar;
  buf:pWideChar;
begin
// allocate buffer
  mGetMem(buf,MAX_PATH*SizeOf(WideChar));
// path
  if recpath<>nil then
  begin
    ConvertFileName(recpath,buf,ActiveContact);
//    pcw:=ParseVarString(recpath,ActiveContact);
//    CallService(MS_UTILS_PATHTOABSOLUTEW,WPARAM(pcw),LPARAM(buf));
//    mFreeMem(pcw);
    if not ForceDirectories(buf) then
    begin
      result:=nil;
      exit;
    end;
    pcw:=StrEndW(buf);
    if (pcw-1)^<>'\' then
    begin
      pcw^:='\';
      inc(pcw);
    end;
  end
  else
    pcw:=buf;
// name
//!!
  p:=MakeMessage;
  pcw:=StrCopyEW(pcw,p);
  mFreeMem(p);
  if (pcw=buf) or ((pcw-1)^='\') then
    pcw:=StrEndW(IntToHex(pcw,GetCurrentTime));
// ext
  pcw^:='.'; inc(pcw);
  GetFileExt(pcw,sign);

  result:=buf;
end;



procedure StatusProc(buffer:pointer;len,user:dword); stdcall;
var
  pc:pWideChar;
  pb:PByte;
  i,sum:integer;
  flag:bool;
  doRecord:bool;
begin
  flag:=true;
  doRecord:=CallService(MS_RADIO_COMMAND,MRC_RECORD,LPARAM(-1))<>0;
  if (buffer<>nil) and (len<>0) and SaveHeader then
  begin
    SaveHeader:=false;
    if pdword(buffer)^=signOGG then // if header ALL in buffer
    begin
      pb:=buffer;
      flag:=false;
      repeat
        inc(pb,OGGHdrSize);
        i:=pb^; //patterns
        sum:=0;
        inc(pb);
        while i>0 do
        begin
          inc(sum,pb^);
          inc(pb);
          dec(i);
        end;
        inc(pb,sum); //here must be next sign
        flag:=not flag;
      until not flag;
      hdrlen:=PAnsiChar(pb)-PAnsiChar(buffer);
      mGetMem(StationHeader,hdrlen);
      move(buffer^,StationHeader^,hdrlen);
    end;
  end;
  if (buffer=nil) or not doRecord then // end of stream or stop record
  begin
    if not doRecord or (doContRec=BST_UNCHECKED) then
      if hRecord<>0 then
      begin
        if buffer<>nil then // write tail
          BlockWrite(hRecord,buffer^,len);
        CloseHandle(hRecord);
        hRecord:=0;
      end;
  end
  else
  begin
    if len=0 then // HTTP or ICY tags
    begin
{
while PAnsiChar(buffer)^<>#0 do
begin
messagebox(0,PAnsiChar(buffer),'ICY-HTTP',0);
while PAnsiChar(buffer)^<>#0 do inc(PAnsiChar(buffer)); inc(PAnsiChar(buffer));
end;
}
    end
    else
    begin
      if doRecord then
      begin
        if hRecord=0 then
        begin
          pc:=MakeFileName(StationHeader);
          if pc<>nil then
            hRecord:=Rewrite(pc)
          else
            hRecord:=THANDLE(INVALID_HANDLE_VALUE);
          if hRecord=THANDLE(INVALID_HANDLE_VALUE) then
            hRecord:=0
          else if flag and (StationHeader<>nil) then
          begin
            BlockWrite(hRecord,StationHeader^,hdrlen);
// permissible to skip to the next Page (OggS) but this is not necessary
          end;
          mFreeMem(pc);
        end;
        if hRecord<>0 then
          BlockWrite(hRecord,buffer^,len);
      end;
    end;
  end;
end;

{$IFDEF Debug}
procedure logmeta(tag,a,b:pansiChar);
var
  f:thandle;
  p:pansichar;
begin
  f:=Append(pansichar('mradio.log'));
  BlockWrite(f,tag^,StrLen(tag));
  p:=#13#10; BlockWrite(f,p^,StrLen(p));
  BlockWrite(f,a^,StrLen(a));
  p:=#13#10; BlockWrite(f,p^,StrLen(p));
  BlockWrite(f,b^,StrLen(b));
  p:=#13#10; BlockWrite(f,p^,StrLen(p));
  p:=#13#10; BlockWrite(f,p^,StrLen(p));
  CloseHandle(f);
end;
{$ENDIF}

function DoMeta(meta:PAnsiChar;TagType:int_ptr):Boolean;
var
  pcw:pWideChar;
  buf:array [0..511] of AnsiChar;
  artist,title:PAnsiChar;
  oldartist,oldtitle:pAnsiChar;
  ppc,pc:pAnsiChar;
  idx,lcp:integer;
  needtofree:Boolean;
  CurDescrW:PWideChar;
  old:boolean;

//  tag:PAnsiChar;
  gotartist,gottitle:boolean; // indicate what we got artist/title
begin
  result:=false;

  if meta=nil then
    meta:=BASS_ChannelGetTags(chan,TagType);
  if meta<>nil then
  begin
    // for cases when artist or title presents but empty
    gotartist:=false;
    gottitle :=false;
    needtofree:=false;
    lcp:=CP_UTF8;
    buf[0]:=#0;
    artist:=nil;
    title :=nil;
    CurDescrW:=nil;

//tag:=meta;
    case TagType of
      BASS_TAG_WMA_META: begin
        pc:=StrPos(meta,'data=');
        if pc=meta then
        begin
          pc:=StrPos(meta,'artist=');
          if pc<>nil then
          begin
            gotartist:=true;
            mGetMem(artist,256);
            Decode(artist,pc+7);
          end;

          pc:=StrPos(meta,'title=');
          if pc<>nil then
          begin
            gottitle:=true;
            mGetMem(title,256);
            Decode(title,pc+6);
          end;

          pc:=StrPos(meta,'album=');
          if pc<>nil then
          begin
          end;

          pc:=StrPos(meta,'duration=');
          if pc<>nil then
          begin
          end;

          if not gotartist then
          begin
            if not gottitle then
            begin
              pc:=StrPos(meta,'caption=');
              if pc<>nil then
              begin
                gottitle:=true;
                mGetMem(title,256);
                Decode(title,pc+8);
              end;
            end;

            // analize title/caption for artist-title
            if gottitle then
            begin
              pc:=StrPos(title,' - ');
              if pc=nil then
                pc:=StrScan(title,'-');
              if pc<>nil then
              begin
                artist:=title;
                if pc^=' ' then
                  title:=pc+3
                else
                  title:=pc+1;
                pc^:=#0;
                CurDescrW:=pWideChar(artist);
              end
              else
                CurDescrW:=pWideChar(title);
            end;
          end
          else
            needtofree:=true;

          // to avoid mem leak and wrong tag process
          result:=true;
        end;
        StatusProc(nil,0,0); // split records here
      end;

      BASS_TAG_META: begin
//tag:='SHOUTCAST';
        // SHOUTCAST StreamTitle='xxx';StreamUrl='xxx';
        // "Station=xyz" meta tag="Trackinfo"
        pc:=StrPos(meta,'StreamTitle=');
        if pc<>nil then
        begin
          inc(pc,13);
          ppc:=StrScan(pc,';');
          if (ppc-pc-1)>0 then
          begin
            StrCopy(buf,pc,ppc-pc-1);
            lcp:=GetTextFormat(@buf,ppc-pc-1);
          end;
        end;
        if buf[0]<>#0 then
        begin
          case lcp of
            CP_UTF8: UTF8ToWide(buf,CurDescrW);
            CP_ACP : AnsiToWide(buf,CurDescrW,MirandaCP);
          end;
        end;

        gottitle:=true;
        title:=pAnsiChar(CurDescrW);
        pcw:=StrPosW(CurDescrW,' - ');
        if pcw=nil then
          pcw:=StrScanW(CurDescrW,'-');
        if pcw<>nil then
        begin
          artist:=pAnsiChar(CurDescrW);
          if pcw^=' ' then
            title:=pAnsiChar(pcw+3)
          else
            title:=pAnsiChar(pcw+1);
          pcw^:=#0;
        end;

        lcp:=CP_UNICODE;

        StatusProc(nil,0,0); // split records here
        result:=true;
      end;

      BASS_TAG_ID3: begin // not realized, anyway - at the end of track
      end;

      BASS_TAG_ID3V2: begin
      end;

      BASS_TAG_APE, // not sure, need to check. maybe better process BASS_TAG_APEBINARY
      BASS_TAG_WMA,
      BASS_TAG_OGG: begin
//tag:='OGG';
        while meta^<>#0 do
        begin
          CharLowerA(StrCopy(buf,meta,10));
          if StrCmp(buf,'title',5)=0 then
          begin
            title:=meta+6;
            gottitle:=true;
          end
          else if StrCmp(buf,'artist',6)=0 then
          begin
            artist:=meta+7;
            gotartist:=true;
          end;
          if gotartist and gottitle then
            break;
          while meta^<>#0 do inc(meta); inc(meta);
        end;

        if (not gotartist) and gottitle then
        begin
          pc:=StrPos(title,' - ');
          if pc=nil then
            pc:=StrScan(title,'-');
          if pc<>nil then
          begin
            needtofree:=true;
            StrDup(artist,title,pc-title);
            if pc^=' ' then
              idx:=3
            else
              idx:=1;
            StrDup(title,pc+idx);
          end;
        end;
        buf[0]:=#0;
      end;
    end;

    old:=true;
    if gotartist or gottitle then
    begin
      // check for old
      oldartist:=nil;
      oldtitle :=nil;
      case lcp of
        CP_UTF8: begin
          if gotartist then
          begin
            oldartist:=DBReadUTF8(0,PluginName,optArtist);
            if StrCmp(artist,oldartist)<>0 then
              old:=false;
          end;

          if old and gottitle then
          begin
            oldtitle:=DBReadUTF8(0,PluginName,optTitle);
            if StrCmp(title,oldtitle)<>0 then
              old:=false;
          end;

          if not old then
          begin
            DBWriteUTF8(0,PluginName,optArtist,artist);
            DBWriteUTF8(0,PluginName,optTitle ,title);
          end;
        end;

        CP_UNICODE:begin
          if gotartist then
          begin
            oldartist:=pAnsiChar(DBReadUnicode(0,PluginName,optArtist));
            if StrCmpW(pWideChar(artist),pWideChar(oldartist))<>0 then
              old:=false;
          end;

          if old and gottitle then
          begin
            oldtitle:=pAnsiChar(DBReadUnicode(0,PluginName,optTitle));
            if StrCmpW(pWideChar(title),pWideChar(oldtitle))<>0 then
              old:=false;
          end;

          if not old then
          begin
            DBWriteUnicode(0,PluginName,optArtist,pWideChar(artist));
            DBWriteUnicode(0,PluginName,optTitle ,pWideChar(title));
          end;
        end;
      end;
{$IFDEF Debug}
//logmeta(tag,artist,title);
{$ENDIF}
      mFreeMem(oldartist);
      mFreeMem(oldtitle);
      mFreeMem(CurDescrW);
      if needtofree then
      begin
        mFreeMem(artist);
        mFreeMem(title );
      end;
    end;

    if not old then
      CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_NEWTAG);
  end;
end;

procedure MetaSync(handle:HSYNC;channel,data:dword;user:pointer); stdcall;
//var tagtype:integer;
begin
(*
  if      handle=syncOGG  then tagtype:=BASS_TAG_OGG
  else if handle=syncWMA  then tagtype:=BASS_TAG_WMA
  else if handle=syncWMA1 then tagtype:=BASS_TAG_WMA_META
  else {if handle=syncMETA then} tagtype:=BASS_TAG_META;
*)
  DoMeta(nil{PAnsiChar(data)},int_ptr(user){tagtype});
end;

procedure EndSync(handle:HSYNC;channel,data:dword;user:pointer); stdcall;
var
  lContact:cardinal;
begin
  if RemoteSong then
  begin
    lContact:=ActiveContact;
    CallService(MS_RADIO_COMMAND,MRC_STOP,0);
    CallService(MS_RADIO_COMMAND,MRC_PLAY,lContact)
  end
  else if plist<>nil           then CallService(MS_RADIO_COMMAND,MRC_NEXT,0)
  else if doLoop=BST_UNCHECKED then CallService(MS_RADIO_COMMAND,MRC_STOP,0);
end;

type
  tICYField = record
    name  :PAnsiChar;
    branch:PAnsiChar;
    option:PAnsiChar;
  end;
const
  NumICYFields = 4;
  ICYFields: array [0..NumICYFields-1] of tICYField = (
    (name:'icy-name:'      ; branch:'CList'     ; option:'MyHandle'),
    (name:'icy-genre:'     ; branch:cPluginName ; option:'Genre'),
    (name:'icy-br:'        ; branch:cPluginName ; option:'Bitrate'),
    (name:'icy-description'; branch:cPluginName ; option:'About')
  );

procedure ProcessStationData;
var
  icy:PAnsiChar;
  i,len:integer;
begin
  // get the broadcast name and bitrate
  icy:=BASS_ChannelGetTags(chan,BASS_TAG_ICY);
  if icy=nil then
    icy:=BASS_ChannelGetTags(chan,BASS_TAG_HTTP); // no ICY tags, try HTTP
  if icy<>nil then
  begin
    while icy^<>#0 do
    begin
      for i:=0 to NumICYFields-1 do
      begin
        with ICYFields[i] do
        begin
          len:=StrLen(name);
          if StrCmp(icy,name,len)=0 then
          begin
            if DBReadStringLength(ActiveContact,branch,option)=0 then
              DBWriteString(ActiveContact,branch,option,icy+len);
            break;
          end;
        end;
      end;
      while icy^<>#0 do inc(icy); inc(icy);
    end;
  end;
end;

procedure OpenURL(url:PWideChar); cdecl;
var
  len,progress:dword;
  flags:dword;
  i:integer;
  EAXUsed:bool;
  ansiurl:array [0..511] of AnsiChar;
begin
  if not CheckBassStatus then exit;
  if BassStatus<>rbs_init then exit; //!! check for record

  if plist=nil then
    CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_CONNECT);

  EAXUsed:=DBReadByte(0,PluginName,optEAXType,0)<>0;

{}
  for i:=0 to NumTries-1 do
  begin
    if EAXUsed then
      flags:=BASS_STREAM_STATUS or BASS_SAMPLE_3D or BASS_SAMPLE_MONO
    else
    begin
      if ForcedMono<>BST_UNCHECKED then
        flags:=BASS_STREAM_STATUS or BASS_SAMPLE_MONO
      else
        flags:=BASS_STREAM_STATUS;
    end;

    flags:=flags or BASS_UNICODE;
    if RemoteSong then
    begin
      SaveHeader:=true;
      chan:=BASS_StreamCreateURL(url,0,flags,@StatusProc,nil)
    end
    else
    begin
      if (plist=nil) and (doLoop<>BST_UNCHECKED) then
        flags:=flags or BASS_SAMPLE_LOOP;
      chan:=BASS_StreamCreateFile(FALSE,url,0,0,flags);
    end;

    if (chan=0) and EAXUsed then
    begin
      flags:=flags and not (BASS_SAMPLE_3D or BASS_SAMPLE_MONO);
      if ForcedMono<>BST_UNCHECKED then
        flags:=flags or BASS_SAMPLE_MONO;

      if RemoteSong then
        chan:=BASS_StreamCreateURL({ansi}url,0,flags,@StatusProc,nil)
      else
        chan:=BASS_StreamCreateFile(FALSE,url,0,0,flags);
    end;

    if (chan=0) and RemoteSong then
    begin
      if BASS_ErrorGetCode=BASS_ERROR_FILEOPEN then
      begin
        flags:=flags and not BASS_UNICODE;
        chan:=BASS_StreamCreateURL(FastWideToAnsiBuf(url,ansiurl),0,flags,@StatusProc,nil)
      end;
    end;

    if chan<>0 then break;
  end;
{}

  if chan=0 then
  begin
    if (CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_GET)=RD_STATUS_ABORT) or
       (plist=nil) then
    begin
      CallService(MS_RADIO_COMMAND,MRC_STOP,1);
    end
    else if plist<>nil then
      CallService(MS_RADIO_COMMAND,MRC_NEXT,0);
  end
  else
  begin

    CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_NEWTRACK);

    if RemoteSong then
    begin
      if isEQ_OFF=BST_UNCHECKED then
        EQ_ON;

{$IFDEF CHANGE_NAME_BUFFERED}
      icy:=DBReadString(ActiveContact,strCList,optMyHandle);
      mGetMem(url,StrLen(icy)+6);
      StrCopy(url+6,icy);
      mFreeMem(icy);
      url[0]:='[';
      url[1]:=#0;
      url[4]:=']';
      url[5]:=' ';
{$ENDIF}
      progress:=0;
      repeat
        if CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_GET)=RD_STATUS_ABORT then
        begin
          CallService(MS_RADIO_COMMAND,MRC_STOP,1);
          exit;
        end;
        len:=BASS_StreamGetFilePosition(chan,BASS_FILEPOS_END);
        if len=DW_ERROR then
          break;
        progress:=BASS_StreamGetFilePosition(chan, BASS_FILEPOS_DOWNLOAD);
        if progress=dword(-1) then
          break;
        progress:=(progress-
          BASS_StreamGetFilePosition(chan,BASS_FILEPOS_CURRENT))*100 div len;
        // percentage of buffer filled
{$IFDEF CHANGE_NAME_BUFFERED}
        IntToStr(url+1,progress,2);
        url[3]:='%';
        DBWriteString(ActiveContact,strCList,optMyHandle,url);
{$ENDIF}
      until progress>sPreBuf;
{$IFDEF CHANGE_NAME_BUFFERED}
      if url[1]<>#0 then
        DBWriteString(ActiveContact,strCList,optMyHandle,url+6);
      mFreeMem(url);
{$ENDIF}

      ProcessStationData; // process ICY-headers

     // get the stream title and set sync for subsequent titles
      DoMeta(nil,BASS_TAG_META);

      syncMETA:=BASS_ChannelSetSync(chan,BASS_SYNC_META      ,0,@MetaSync,pointer(BASS_TAG_META));
      syncOGG :=BASS_ChannelSetSync(chan,BASS_SYNC_OGG_CHANGE,0,@MetaSync,pointer(BASS_TAG_OGG));
      syncWMA :=BASS_ChannelSetSync(chan,BASS_SYNC_WMA_CHANGE,0,@MetaSync,pointer(BASS_TAG_WMA));
      syncWMA1:=BASS_ChannelSetSync(chan,BASS_SYNC_WMA_META  ,0,@MetaSync,pointer(BASS_TAG_WMA_META));
    end
    else
    begin
      if not DoMeta(nil,BASS_TAG_OGG) then
        if not DoMeta(nil,BASS_TAG_ID3V2) then
          if not DoMeta(nil,BASS_TAG_ID3) then
           if not DoMeta(nil,BASS_TAG_APE) then
      ;

    end;

    syncEND:=BASS_ChannelSetSync(chan,BASS_SYNC_END,0,@EndSync,nil);

    SetSndVol(-1);
    // play it!
    BASS_ChannelPlay(chan,FALSE);
  end;
end;

function MakeFilter(dst,descr,full,filter:PWideChar;show:bool=true):pWideChar;
var
  p:PWideChar;
begin
  if full<>nil then
  begin
    p:=StrEndW(full);
    p^:=';';
    StrCopyW(p+1,filter);
  end;

  dst:=StrCopyEW(dst,TranslateW(descr));
  if show then
  begin
    dst^    :=' ';
    (dst+1)^:='(';
    dst:=StrCopyEW(dst+2,filter);
    dst^:=')';
    inc(dst);
    dst^:=#0;
  end;
  inc(dst);
  result:=StrCopyEW(dst,filter)+1;
end;

function ConstructFilter:pointer;
var
  pc:pWideChar;
  ph:PDWord;
  info:PBASS_PLUGININFO;
  i:integer;
  full:array [0..511] of WideChar;
  tmpbuf1,tmpbuf2:array [0..127] of WideChar;
begin
  mGetMem(pc,4096);
//  FillChar(pc^,4096,0);
  result:=pc;
  full[0]:=#0;
  pc:=MakeFilter(pc,'All files'     ,nil ,'*.*',false);
  pc:=MakeFilter(pc,'Playlist files',full,'*.pls;*.m3u;*.m3u8;*.asx');
  pc:=MakeFilter(pc,'BASS built-in' ,full,'*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif');

  if BassStatus=rbs_null then
    MyLoadBass;

  if BassStatus<>rbs_null then
  begin
    ph:=pointer(BASS_PluginGetInfo(0));
    if ph<>nil then
    begin
      while ph^<>0 do
      begin
        info:=BASS_PluginGetInfo(ph^);
        for i:=0 to info^.formatc-1 do
  //!! need to translate Ansi -> wide
          with info^.Formats[i] do
          begin
            pc:=MakeFilter(pc,FastAnsiToWideBuf(name,tmpbuf1),full,FastAnsiToWideBuf(exts,tmpbuf2));
          end;
        inc(ph);
      end;
    end;
  end;
  pc:=MakeFilter(pc,'All supported formats',nil,full,false);
  pc^:=#0;
end;

end.

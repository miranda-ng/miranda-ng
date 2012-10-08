{}

function GetStatusText(status:integer;toCList:boolean=false):PWideChar;
begin
  case status of
    RD_STATUS_PAUSED : result:='paused';
    RD_STATUS_STOPPED: if toCList then result:=nil else result:='stopped';
    RD_STATUS_CONNECT: result:='connecting';
    RD_STATUS_ABORT  : result:='aborting';
    RD_STATUS_PLAYING: if toCList then result:=nil else result:='playing';
  else
    result:=nil;
  end;
end;

procedure ConstructMsg(artist,title:PWideChar;status:integer=-1);
var
  buf:PWideChar;
  tstrlen,astrlen,statuslen:integer;
  sstatus:PWideChar;
  p:pWideChar;
begin
  astrlen:=StrLenW(artist);
  
  tstrlen:=0;
  // if need to remove duplicate
  if (title<>nil) and (title^<>#0) then
  begin
    if (astrlen=0) or (StrCmpW(artist,title)<>0) then
      tstrlen:=StrLenW(title);
  end;
  if (astrlen>0) and (tstrlen>0) then
    inc(tstrlen,3);

  statuslen:=0;
  if status>=0 then
  begin
    sstatus:=TranslateW(GetStatusText(status,true));
    if sstatus<>nil then
      statuslen:=StrLenW(sstatus)+3;
  end
  else
    sstatus:=nil;

  if (astrlen+tstrlen+statuslen)>0 then
  begin
    mGetMem(buf,(astrlen+tstrlen+statuslen+1)*SizeOf(WideChar));
    p:=buf;
    if astrlen>0 then
      p:=StrCopyEW(p,artist);

    if tstrlen>0 then
    begin
      if astrlen>0 then
      begin
        p^:=' '; inc(p);
        p^:='-'; inc(p);
        p^:=' '; inc(p);
      end;
      p:=StrCopyEW(p,title);
    end;

    if statuslen>0 then
    begin
      p^:=' '; inc(p);
      p^:='('; inc(p);
      p:=StrCopyEW(p,sstatus);
      p^:=')'; inc(p);
      p^:=#0;
    end;

    DBWriteUnicode(ActiveContact,strCList,optStatusMsg,buf);
    mFreeMem(buf);
  end
  else
    DBDeleteSetting(ActiveContact,strCList,optStatusMsg);
end;

{$IFDEF Debug}
procedure log(a,b:lparam);
var
  f:thandle;
  buf:array [0..31] of ansichar;
  p:pansichar;
begin
  f:=Append(pansichar('mradio.log'));
  p:=IntToStr(buf,a); BlockWrite(f,p^,StrLen(p));
  p:=#13#10; BlockWrite(f,p^,StrLen(p));
  p:=IntToStr(buf,b); BlockWrite(f,p^,StrLen(p));
  p:=#13#10; BlockWrite(f,p^,StrLen(p));
  p:=#13#10; BlockWrite(f,p^,StrLen(p));
  CloseHandle(f);
end;
{$ENDIF}

function ControlCenter(code:WPARAM;arg:LPARAM):int_ptr; cdecl;
const
  PlayStatus:int_ptr=RD_STATUS_NOSTATION;
  doRecord:boolean=false;
var
  tmpbuf,buf1:array [0..MAX_PATH-1] of WideChar;
  plFile:pWideChar;
  plLocal:boolean;
  artist,title:pWideChar;
begin
  result:=0;
{$IFDEF Debug}
  log(code,arg);
{$ENDIF}
  case code of
    MRC_PLAY : begin
      // play new station?
      if arg<>0 then
      begin
        ActiveURLw:=DBReadUnicode(arg,PluginName,optStationURL);
        // no URL for this contact
        if (ActiveURLw=nil) or (ActiveURLw^=#0) then exit;

        ActiveContact:=arg;
        RemoteSong:=StrPosW(ActiveURLw,'://')<>nil;

        if isPlaylist(ActiveURLw)<>0 then
        begin

          if RemoteSong then
          begin
            GetTempPathW(MAX_PATH,tmpbuf);
            GetTempFileNameW(tmpbuf,'mr',GetCurrentTime,buf1);
            if not GetFile(ActiveURLw,buf1,hNetLib) then
            begin
              exit;
            end;
            plFile:=@buf1;
            plLocal:=false;
          end
          else
          begin
            plFile:=ActiveURLw;
            plLocal:=true;
          end;

          plist:=CreatePlaylist(plFile);
          if not plLocal then
            DeleteFileW(plFile);

          if plist<>nil then
          begin
            if plist.GetCount=0 then
            begin
              plist.Free;
              plist:=nil;
              ActiveContact:=0;
              exit;
            end;
            plist.Shuffle:=doShuffle<>BST_UNCHECKED;

            if not plist.Shuffle then
            begin
              if PlayFirst=BST_UNCHECKED then
                plist.Track:=DBReadWord(ActiveContact,PluginName,optCurElement);
            end;
            
            mFreeMem(ActiveURLw);
            ActiveURLw:=plist.GetSong;
          end;
          RemoteSong:=StrPosW(ActiveURLw,'://')<>nil; // coz activeuRLw can be changed
        end
        else
          plist:=nil;

        if (ActiveURLw<>nil) and (ActiveURLw^<>#0) then
        begin
          if RemoteSong then
          begin
            if GetWorkOfflineStatus<>0 then
            begin
              BassError('Can''t connect to net. Please, clear ''Work offline'' option in Internet settings');
              ControlCenter(MRC_STATUS,RD_STATUS_NOSTATION);
              exit;
            end;
          end;
          ControlCenter(MRC_STATUS,RD_STATUS_NEWSTATION);
          if Assigned(plist) then
            ControlCenter(MRC_STATUS,RD_STATUS_CONNECT);
          CloseHandle(mir_forkthread(@OpenURL,ActiveURLw));
        end;
      end
      // play current from start
      else if chan<>0 then
      begin
        BASS_ChannelPlay(chan,true);
        code:=MRC_STATUS;
        arg :=RD_STATUS_PLAYING;
      end
      // play playlist entry?
      else if Assigned(plist) and (ActiveURLw<>nil) and (ActiveURLw^<>#0) then
      begin
        CloseHandle(mir_forkthread(@OpenURL,ActiveURLw));
      end;
    end;

    MRC_RECORD: begin
      if arg=LPARAM(-1) then
        result:=ord(doRecord)
      else
      begin
        case arg of
          1: doRecord:=true;  // force start
          2: doRecord:=false; // force stop
        else
          doRecord:=not doRecord;
        end;
        result:=ord(doRecord);

        code:=MRC_STATUS;
        arg :=RD_STATUS_RECORD+(result shl 16);
      end;
    end;

    MRC_PAUSE: begin
      if chan<>0 then
      begin
        case ControlCenter(MRC_STATUS,RD_STATUS_GET) of
          RD_STATUS_PLAYING: begin
            BASS_ChannelPause(chan);
            arg:=1;
          end;
          RD_STATUS_PAUSED: begin
            BASS_ChannelPlay(chan,false);
            arg:=0;
          end;
        end;
        code:=MRC_STATUS;
        arg :=RD_STATUS_PAUSED+(arg shl 16);
      end
      else
      begin
        ActiveContact:=LoadContact(PluginName,optLastStn);
        if ActiveContact<>0 then
          ControlCenter(MRC_PLAY,ActiveContact);
      end;
    end;

    MRC_STOP: begin
      ControlCenter(MRC_RECORD,2);

      code:=MRC_STATUS;
      // stop playlist track but not station
      if Assigned(plist) and (arg=0) then
      begin
        if chan<>0 then
          BASS_ChannelStop(chan);

        arg:=RD_STATUS_STOPPED;
      end
      else // forced or "normal" stop
      begin
        if Assigned(plist) then
        begin
          DBWriteWord(ActiveContact,PluginName,optCurElement,plist.Track);
          plist.Free;
          plist:=nil;
        end;
        ControlCenter(MRC_STATUS,RD_STATUS_STOPPED);
        // Save station for next autoplay
        SaveContact(ActiveContact,PluginName,optLastStn);
        StopStation;

        arg:=RD_STATUS_NOSTATION;
      end;
    end;

    MRC_MUTE: begin
      CallService(MS_RADIO_MUTE,0,arg);
      code:=MRC_STATUS;
      arg :=RD_STATUS_MUTED;
    end;

    MRC_NEXT: begin
      if Assigned(plist) then
      begin
        StopStation;
        ActiveURLw:=plist.Next;
        ControlCenter(MRC_PLAY,0);
      end;
    end;

    MRC_PREV: begin
      if Assigned(plist) then
      begin
        StopStation;
        ActiveURLw:=plist.Previous;
        ControlCenter(MRC_PLAY,0)
      end;
    end;

    MRC_SEEK: begin
      if not RemoteSong and (chan<>0) then
      begin
        if integer(arg)=-1 then // get position
        begin
          result:=trunc(BASS_ChannelBytes2Seconds(chan,BASS_ChannelGetPosition(chan,BASS_POS_BYTE)));
          if result<0 then
            result:=0;
        end
        else
        begin
          BASS_ChannelSetPosition(chan,BASS_ChannelSeconds2Bytes(chan,arg),BASS_POS_BYTE);

          code:=MRC_STATUS;
          arg :=RD_STATUS_POSITION+(arg shl 16);
        end;
      end;
    end;
  end;

  if code=MRC_STATUS then
  begin
    if arg=RD_STATUS_GET then
      result:=PlayStatus
    else
    begin

      code:=(arg and $FFFF);
      arg :=hiword(arg);
      // this is just events
      case code of
        RD_STATUS_MUTED: begin
        end;

        RD_STATUS_RECORD: begin
        end;

        RD_STATUS_POSITION: begin
        end;

      else
        // these statuses are for events and some tasks
        artist:=nil;
        title :=nil;
        case code of
          RD_STATUS_NOSTATION: begin
            SetStatus(ActiveContact,ID_STATUS_OFFLINE);
            ActiveContact:=0;

            DBDeleteSetting(0,PluginName,optActiveURL);

            PlayStatus:=RD_STATUS_NOSTATION;
            // empty message
          end;

          RD_STATUS_STOPPED: begin
            if ActiveContact<>0 then //!! fools proof
              DBDeleteSetting(ActiveContact,strCList,optStatusMsg);

            DBDeleteSetting(0,PluginName,optTitle);
            DBDeleteSetting(0,PluginName,optArtist);

            PlayStatus:=RD_STATUS_STOPPED;
            // empty message
          end;

          RD_STATUS_ABORT: begin
            PlayStatus:=RD_STATUS_ABORT;
            // status as message
          end;

          RD_STATUS_CONNECT: begin
            SetStatus(ActiveContact,ID_STATUS_AWAY);
            PlayStatus:=RD_STATUS_CONNECT;
            // status as message
          end;

          RD_STATUS_NEWSTATION: begin
            arg:=ActiveContact;
            PlayStatus:=RD_STATUS_PLAYING;
          end;

          RD_STATUS_NEWTRACK: begin
            SetStatus(ActiveContact,ID_STATUS_ONLINE);
            DBWriteUnicode(0,PluginName,optActiveURL,ActiveURLw);

            DBWriteString(0,PluginName,optActiveCodec,GetMusicFormat);
            arg :=lparam(ActiveURLw);

            // for case when tags was in meta
            artist:=DBReadUnicode(0,PluginName,optArtist);
            title :=DBReadUnicode(0,PluginName,optTitle);
            PlayStatus:=RD_STATUS_PLAYING;
            // status as message
          end;

          RD_STATUS_NEWTAG: begin
          // must be updated tags
            artist:=DBReadUnicode(0,PluginName,optArtist);
            title :=DBReadUnicode(0,PluginName,optTitle);

            PlayStatus:=RD_STATUS_PLAYING; // maybe keep RD_STATUS_NEWTAG?
          end;

          RD_STATUS_PLAYING: begin
            SetStatus(ActiveContact,ID_STATUS_ONLINE);
            PlayStatus:=RD_STATUS_PLAYING;
            // status as message
          end;

          RD_STATUS_PAUSED: begin
            if arg=0 then
            begin
              PlayStatus:=RD_STATUS_PLAYING;
              if StrPosW(ActiveURLw,'://')=nil then //local only
              begin
                artist:=DBReadUnicode(0,PluginName,optArtist);
                title :=DBReadUnicode(0,PluginName,optTitle);
              end;
            end
            else
              PlayStatus:=RD_STATUS_PAUSED;
            // status as message
          end;
        else
          exit;
        end;
        ConstructMsg(artist,title,PlayStatus);
        mFreeMem(artist);
        mFreeMem(title);
      end;
      
      NotifyEventHooks(hhRadioStatus,code,arg);
    end;

  end;
end;

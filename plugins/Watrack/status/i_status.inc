{Status and XStatus processing}

// XStatus
const
  xsnum = 31;

function ListenProc(wParam:WPARAM;lParam:LPARAM):int_ptr; cdecl;
begin
  // ignoring incoming ListeningTo info, uses internal
  if PLISTENINGTOINFO(lParam).cbSize=SizeOf(TLISTENINGTOINFO) then
  begin
    result:=int_ptr(GetMacros(tmpl_tunes,0)); // need real proto number here
    if result=-1 then result:=0;
  end
  else
  begin
    result:=CallService(MS_WAT_REPLACETEXT,0,wParam);
  end;
end;

function IsOurStatus(protomask,status:dword):boolean;
var
  mask:dword;
begin
{  if      status=ID_STATUS_OFFLINE    then mask:=M_STAT_OFFLINE
  else }
       if status=ID_STATUS_ONLINE     then mask:=psf_online
  else if status=ID_STATUS_INVISIBLE  then mask:=psf_invisible
  else if status=ID_STATUS_AWAY       then mask:=psf_shortaway
  else if status=ID_STATUS_NA         then mask:=psf_longaway
  else if status=ID_STATUS_DND        then mask:=psf_heavydnd
  else if status=ID_STATUS_OCCUPIED   then mask:=psf_lightdnd
  else if status=ID_STATUS_FREECHAT   then mask:=psf_freechat
  else if status=ID_STATUS_ONTHEPHONE then mask:=psf_onthephone
  else if status=ID_STATUS_OUTTOLUNCH then mask:=psf_outtolunch
  else mask:=0;
  result:=(protomask and mask)<>0;
end;

function NewPlStatus(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  si:pSongInfo;
  i,j:integer;
  xstat:integer;
  txt,title:pWideChar;
  proto,ls:PAnsiChar;
  mask,status:dword;
  todo,isvideo:bool;
  oldstatus:integer;
  staudio,stvideo:integer;
  lti:TLISTENINGTOINFO;
  buf,buf1:array [0..31] of WideChar;
  doClear:boolean;
begin
  result:=0;
  si:=pSongInfo(lParam);

  case wParam of
    WAT_EVENT_NEWTRACK: begin
      isvideo:=pSongInfo(lParam)^.width<>0;
      doClear:=false;
    end;

    WAT_EVENT_PLAYERSTATUS: begin
      LastStatus:=lParam;
      if not (lParam in [WAT_PLS_NOTFOUND,WAT_PLS_STOPPED]) then
        exit;
      doClear:=true;
    end;
  else
    exit; // not accept template changes
  end;

  i:=GetNumProto;
  for j:=1 to i do
  begin
    proto:=GetProtoName(j);
    mask:=GetProtoSetting(j);

    if (SimpleMode<>BST_UNCHECKED) or ((mask and psf_enabled)<>0) then
    begin

      if (UseStatus<>BST_UNCHECKED) or (UseExtStatus<>BST_UNCHECKED) then
      begin
        status:=CallProtoService(proto,PS_GETSTATUS,0,0);
        todo  :=(SimpleMode<>BST_UNCHECKED) or IsOurStatus(mask,status);
      end
      else
      begin
        status:=ID_STATUS_ONLINE;
        todo  :=true;
      end;

      if (UseListeningTo<>BST_UNCHECKED) and
         IsTunesSupported(j) then
//         (ProtoServiceExists(proto,PS_SET_LISTENINGTO)<>0) then
      begin
        if doClear then
          CallProtoService(proto,PS_SET_LISTENINGTO,0,0)
        else if (wParam=WAT_EVENT_NEWTRACK) then
        begin
          lti.cbSize    :=SizeOf(lti);
          if si.width<>0 then
            lti.szType.W:='Video'
          else
            lti.szType.W:='Music';
          lti.szArtist.W:=si.artist;
          lti.szAlbum .W:=si.album;
          lti.szTitle .W:=si.title;
          lti.szTrack .W:=IntToStr(buf1,si.track);
          lti.szYear  .W:=si.year;
          lti.szGenre .W:=si.genre;
          lti.szLength.W:=IntToStr(buf,si.total);
          lti.szPlayer.W:=si.player;
          lti.dwFlags   :=LTI_UNICODE;

          CallProtoService(proto,PS_SET_LISTENINGTO,0,tlparam(@lti));
        end;
      end;

      if UseStatus<>BST_UNCHECKED then
        if todo then
        begin
          if SimpleMode<>BST_UNCHECKED then
            txt:=GetMacros(tmpl_stext,0)
          else
            txt:=GetMacros(tmpl_stext,j);
          if uint_ptr(txt)<>uint_ptr(-1) then
          begin
            if (txt=nil) or (txt^=#0) then
              ls:=nil
            else
              WideToAnsi(txt,ls,UserCP);
            SetStatus(proto,-status,ls);
            mFreeMem(ls);
            mFreeMem(txt);
          end;
        end;

      if UseExtStatus<>BST_UNCHECKED then
      begin
        if todo or (XIndepended<>BST_UNCHECKED) then
        begin
          if IsXStatusSupported(j) then
          begin
            if doClear then // player status changed to no music/no player
            begin

              if KeepStatus=BST_UNCHECKED then
              begin
                // just restoring savedstatus if was. no text changing
                if OldXStatus[j]<>255 then
                begin
                  oldstatus:=OldXStatus[j];
                  OldXStatus[j]:=255;
                  SetXStatus(proto,oldstatus,pWideChar(-1),pWideChar(-1));
                end;
              end;

            end
            else
            begin
              if SimpleMode<>BST_UNCHECKED then
                txt:=GetMacros(tmpl_xtext,0)
              else
                txt:=GetMacros(tmpl_xtext,j);
              if uint_ptr(txt)<>uint_ptr(-1) then // status template presents
              begin
                // XStatus for audio/video
                if SimpleMode<>BST_UNCHECKED then
                begin
                  stvideo := 8;
                  staudio := 11;
                end
                else
                begin
                  mask:=GetProtoSetting(j,true);
                  staudio:=mask and $FF;
                  stvideo:=(mask shr 8) and $FF;
                end;
                // Check, what we able to do something
                oldstatus:=GetXStatus(proto,nil,nil);
                if XStatusSet<>0 then // no matter which xstatus
                begin
                  if not ((oldstatus=staudio) or (oldstatus=stvideo) or // music
                         ((oldstatus=0) and (XStatusSet=2))) then // empty
                  begin
                    mFreeMem(txt);
                    continue; //!! do nothing!
                  end;
                end;
                if isvideo then
                  xstat:=stvideo
                else
                  xstat:=staudio;

                if xstat=0 then  // not choosed, keep old (current)
                  xstat:=oldstatus
                else
                begin
                  if OldXStatus[j]=255 then
                    OldXStatus[j]:=oldstatus;
                end;

                if ClearXStat<>BST_UNCHECKED then
                  SetXStatus(proto,0);

                if SimpleMode<>BST_UNCHECKED then
                  title:=GetMacros(tmpl_xtitle,0)
                else
                  title:=GetMacros(tmpl_xtitle,j);
                SetXStatus(proto,xstat,txt,title);
                mFreeMem(title);
                mFreeMem(txt);
              end;
            end;
          end;
        end;
      end;

    end;
  end;
end;

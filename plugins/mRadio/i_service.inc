{services}

function Service_GetCaps(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
begin
  case wParam of
    PFLAGNUM_1:
      result:=PF1_EXTSEARCH or PF1_MODEMSGRECV or PF1_ADDSEARCHRES or PF1_BASICSEARCH;
    PFLAGNUM_2:
      result:=PF2_ONLINE or PF2_INVISIBLE or PF2_SHORTAWAY;
    PFLAGNUM_3:
      result:=PF2_ONLINE;// or PF2_INVISIBLE or PF2_SHORTAWAY;
    PFLAGNUM_4:
      result:=PF4_NOCUSTOMAUTH or PF4_AVATARS;
    PFLAG_UNIQUEIDTEXT:
      result:=int_ptr(Translate('Radio station URL'));
    PFLAG_UNIQUEIDSETTING:
      result:=int_ptr(optStationURL)
    else
      result:=0;
  end
end;

function Service_GetName(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
begin
  if lParam<>0 then
    StrCopy(PAnsiChar(lParam),Translate(PluginName),wParam);
  result:=0;
end;

procedure GetAwayMsgProc(hContact:TMCONTACT); cdecl;
var
  buf,p:PWideChar;
begin
  if isVarsInstalled then
  begin
    buf:=ParseVarString(StatusTmpl,ActiveContact);
  end
  else
  begin
    mGetMem(buf,1024);
    StrCopyW(buf,StatusTmpl);
    if StrPosW(buf,'%radio_name%')<>nil then
    begin
      p:=DBReadUnicode(ActiveContact,strCList,optMyHandle);
      StrReplaceW(buf,'%radio_name%',p);
      mFreeMem(p);
    end;
    if StrPosW(buf,'%radio_url%')<>nil then
    begin
      p:=DBReadUnicode(ActiveContact,PluginName,optStationURL);
      StrReplaceW(buf,'%radio_url%',p);
      mFreeMem(p);
    end;
    if StrPosW(buf,'%radio_active%')<>nil then
    begin
      StrReplaceW(buf,'%radio_active%',ActiveURL);
    end;
    if StrPosW(buf,'%radio_genre%')<>nil then // saved as String
    begin
      p:=DBReadUnicode(ActiveContact,PluginName,optGenre);
      StrReplaceW(buf,'%radio_genre%',p);
      mFreeMem(p);
    end;
    if StrPosW(buf,'%radio_bitrate%')<>nil then // saved as String
    begin
      p:=DBReadUnicode(ActiveContact,PluginName,optBitrate);
      StrReplaceW(buf,'%radio_bitrate%',p);
      mFreeMem(p);
    end;
    if StrPosW(buf,'%radio_title%')<>nil then
    begin
      p:=MakeMessage;
      StrReplaceW(buf,'%radio_title%',p);
      mFreeMem(p);
    end;
    if StrPosW(buf,'%radio_codec%')<>nil then
    begin
      p:=DBReadUnicode(ActiveContact,PluginName,optActiveCodec);
      StrReplaceW(buf,'%radio_codec%',p);
      mFreeMem(p);
    end;
    if StrPosW(buf,'%radio_status%')<>nil then
    begin
      StrReplaceW(buf,'%radio_status%',
         TranslateW(GetStatusText(
             CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_GET))));
    end;
  end;

  ProtoBroadcastAck(PluginName,hContact,ACKTYPE_AWAYMSG,ACKRESULT_SUCCESS,AckHandle,lParam(buf));
  mFreeMem(buf);
end;

function Service_GetAwayMsg(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  ccs:PCCSDATA;
begin
  ccs:=PCCSDATA(lParam);
  if (ccs^.hContact<>0) and (PluginStatus=ID_STATUS_ONLINE) and
     (DBReadWord(ccs^.hContact,PluginName,optStatus,ID_STATUS_OFFLINE)=ID_STATUS_ONLINE) then
  begin
    {CloseHandle}(mir_forkthread(@GetAwayMsgProc,pointer(uint_ptr(ccs^.hContact))));
    result:=AckHandle;
  end
  else
    result:=0;
end;

function Service_GetStatus(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
begin
  result:=PluginStatus;
end;

function Service_LoadIcon(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
begin
  case loword(wParam) of
    PLI_PROTOCOL: result:=LoadImage(hInstance,MAKEINTRESOURCE(IDI_MAIN),IMAGE_ICON,16,16,LR_SHARED);
//    PLI_ONLINE  : result:=0;
//    PLI_OFFLINE : result:=0;
  else
    result:=0;
  end;
end;

function Service_SetStatus(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  OldStatus:integer;
begin
  result:=0;
  OldStatus:=PluginStatus;
  if wParam<>ID_STATUS_OFFLINE then
    wParam:=ID_STATUS_ONLINE;

  if wParam=TWPARAM(PluginStatus) then
    exit;

  PluginStatus:=ID_STATUS_OFFLINE;

  if wParam<>ID_STATUS_OFFLINE then
  begin
//    if MyInitBASS then
    begin
      PluginStatus:=ID_STATUS_ONLINE;

      if (ActiveContact<>0) and (AuConnect<>0) then
        CallService(MS_RADIO_COMMAND,MRC_PLAY,ActiveContact);
    end;
  end
  else //offline
  begin
    CallService(MS_RADIO_COMMAND,MRC_STOP,0);
  end;
  SetStatus(0,ID_STATUS_OFFLINE);

  ProtoBroadcastAck(PluginName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,OldStatus,PluginStatus);
end;

// really, can be replaced by helper function. this one more effective
function CreateProtoService(serviceName:PAnsiChar;pFunc:pointer):THANDLE;
var
  temp:array [0..MAXMODULELABELLENGTH-1] of AnsiChar;
begin
  StrCopy(StrCopyE(temp,PluginName),serviceName);
  result:=CreateServiceFunction(temp,pFunc);
end;

var
  prh0,prh1,prh2,prh3,prh4,prh5,prh6,prh7,prh8,prh9:THANDLE;

procedure DestroyProtoServices;
begin
  DestroyServiceFunction(prh0);
  DestroyServiceFunction(prh1);
  DestroyServiceFunction(prh2);
  DestroyServiceFunction(prh3);
  DestroyServiceFunction(prh4);
  DestroyServiceFunction(prh5);
  DestroyServiceFunction(prh6);
  DestroyServiceFunction(prh7);
  DestroyServiceFunction(prh8);
  DestroyServiceFunction(prh9);
end;

procedure CreateProtoServices;
begin
  prh0:=CreateProtoService(PS_GETCAPS          ,@Service_GetCaps);
  prh1:=CreateProtoService(PS_ADDTOLIST        ,@Service_AddToList);
  prh2:=CreateProtoService(PS_CREATEADVSEARCHUI,@Service_ExtSearchUI);
  prh3:=CreateProtoService(PS_SEARCHBYADVANCED ,@Service_SearchByAdvanced);
  prh4:=CreateProtoService(PS_BASICSEARCH      ,@Service_SearchBasic);
  prh5:=CreateProtoService(PS_GETNAME          ,@Service_GetName);
  prh6:=CreateProtoService(PS_LOADICON         ,@Service_LoadIcon);
  prh7:=CreateProtoService(PS_GETSTATUS        ,@Service_GetStatus);
  prh8:=CreateProtoService(PS_SETSTATUS        ,@Service_SetStatus);
  prh9:=CreateProtoService(PSS_GETAWAYMSG      ,@Service_GetAwayMsg);
end;

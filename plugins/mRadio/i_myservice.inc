{My services}

function Service_RadioPlayStop(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  p:PAnsiChar;
  lnew:bool;
  hContact:TMCONTACT;
  cni:TCONTACTINFO;
  i:integer;
begin
  result:=0;
  if lParam=0 then
  begin
    if wParam=0 then
      wParam:=GetCListSelContact;
    p:=GetContactProtoAcc(wParam);
    if (p=nil) or (StrCmp(p,PluginName)<>0) then
      exit;
    hContact:=wParam;
  end
  // wParam = station name
  else
  begin
    FillChar(cni,SizeOf(cni),0);
    cni.cbSize  :=sizeof(cni);
    if lParam=1 then
      cni.dwFlag:=CNF_DISPLAY
    else
      cni.dwFlag:=CNF_DISPLAY or CNF_UNICODE;
    cni.szProto :=PluginName;

    hContact:=db_find_first(PluginName);
    while hContact<>0 do
    begin
      cni.hContact:=hContact;
      if CallService(MS_CONTACT_GETCONTACTINFO,0,tlparam(@cni))=0 then
      begin
        if lParam=1 then
          i:=StrCmp(pAnsiChar(wParam),cni.retval.szVal.a)
        else
          i:=StrCmpW(pWideChar(wParam),cni.retval.szVal.w);
        mir_free(cni.retval.szVal.w);
        if i=0 then
          break;
      end;
      hContact:=db_find_next(hContact,PluginName);
    end;
  end;

  if hContact<>0 then
  begin
    result:=1;
    if PluginStatus=ID_STATUS_OFFLINE then
      Service_SetStatus(ID_STATUS_ONLINE,0);

    case CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_GET) of
      RD_STATUS_CONNECT: begin //break while connect
        CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_ABORT);
        exit;
      end;
      RD_STATUS_ABORT: exit;
    end;

    lnew:=ActiveContact<>hContact;

    if ActiveContact<>0 then
      CallService(MS_RADIO_COMMAND,MRC_STOP,1);

    if lnew then
      CallService(MS_RADIO_COMMAND,MRC_PLAY,hContact);
  end;
end;

function Service_RadioSettings(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  ood:TOPENOPTIONSDIALOG;
begin
  result:=0;
  ood.cbSize:=SizeOf(ood);
  ood.pszGroup:='Network';
  ood.pszPage :=PluginName;
  ood.pszTab  :=Translate('Advanced');
  CallService(MS_OPT_OPENOPTIONS,0,tlparam(@ood));
end;

function Service_RadioRecord(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  if lParam<>0 then lParam:=TLPARAM(-1) else lParam:=0;

  result:=CallService(MS_RADIO_COMMAND,MRC_RECORD,lParam);
end;

function Service_RadioGetVolume(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  result:=gVolume;
end;

function Service_RadioSetVolume(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  result:=Service_RadioGetVolume(0,0);
  SetSndVol(wParam);

  DBWriteByte(0,PluginName,optVolume,ABS(wParam));
  if lParam<>2 then // not from Frame
    if hVolFrmCtrl<>0 then
      SendMessage(hVolFrmCtrl,TBM_SETPOS,1,ABS(wParam));

  if lParam<>1 then // not from Settings
    if hVolCtrl<>0 then
      SendMessage(hVolCtrl,TBM_SETPOS,1,ABS(wParam));

end;

function Service_RadioMute(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  if gVolume=0 then
    gVolume:=-1
  else
    gVolume:=-gVolume;
  Result:=Service_RadioSetVolume(gVolume,0);
  if hVolFrmMute<>0 then
  begin
    InvalidateRect(hVolFrmMute,nil,true);
    RedrawWindow(hVolFrmMute,nil,0,RDW_INVALIDATE or RDW_ALLCHILDREN or RDW_ERASE);
  end;
  if hVolFrmCtrl<>0 then
    EnableWindow(hVolFrmCtrl,gVolume>=0);
end;

function Service_EqOnOff(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  result:=ord(isEQ_OFF=BST_UNCHECKED);
  case wParam of
    0: begin
      if isEQ_OFF=BST_UNCHECKED then
        EQ_OFF
      else
        EQ_ON;
    end;
    1: EQ_ON;
  else
    EQ_OFF;
  end;
  if eq[0].wnd<>0 then // if options opened (can use hVolCtrl)
  begin
    CheckDlgButton(GetParent(eq[0].wnd),IDC_EQOFF,isEQ_OFF);
  end;
end;

//----- Import-export -----

function ImportOneStation(group:PAnsiChar;section:pointer):TMCONTACT;
var
  p:pWideChar;
  pc:pAnsiChar;
begin
  result:=0;
  pc:=GetParamSectionStr(section,'URL');
  if pc<>nil then
  begin
    result:=CallService(MS_DB_CONTACT_ADD,0,0);
    if result<>0 then
    begin
      CallService(MS_PROTO_ADDTOCONTACT,result,lparam(PluginName));
      DBWriteString(result,PluginName,optStationURL,pc);
      DBWriteString(result,PluginName,optFirstName ,pc);

      pc:=GetParamSectionStr(section,optBitrate,'0');
      DBWriteString(result,PluginName,optBitrate,pc);
      DBWriteWord  (result,PluginName,optAge    ,StrToInt(pc));

      pc:=GetParamSectionStr(section,'Name',GetSectionName(section));
      DBWriteString(result,strCList  ,optMyHandle,pc);
      DBWriteString(result,PluginName,optNick    ,pc);

      pc:=GetParamSectionStr(section,optGenre,'unknown');
      DBWriteString(result,PluginName,optGenre   ,pc);
      DBWriteString(result,PluginName,optLastName,pc);

      SetStatus(result,ID_STATUS_OFFLINE);

      if group=nil then
        group:=GetParamSectionStr(section,optGroup);

      AnsiToWide(group,p,MirandaCP);
      CreateGroupW(p,result);
      mFreeMem(p);
      CallService(MS_IGNORE_IGNORE,result,IGNOREEVENT_ALL);
    end;
  end;
end;

function ImportAll(wParam:WPARAM;lParam:LPARAM):int; cdecl;
var
  dst:array [0..MAX_PATH-1] of AnsiChar;
  pc:pAnsiChar;
  lstorage,section,list:pointer;
begin
  result:=0;
  if lParam<>0 then
    StrCopy(dst,PAnsiChar(lParam));
  if (lParam<>0) or ShowDlg(dst,'radio.ini',nil,true) then
  begin
    lstorage:=OpenStorage(dst);
    if lstorage<>nil then
    begin
      list:=GetSectionList(lstorage);

      pc:=list;
      while pc^<>#0 do
      begin
        section:=SearchSection(lstorage,pc);
        if ImportOneStation(pAnsiChar(wParam),section)<>0 then inc(result);
        while pc^<>#0 do inc(pc);
        inc(pc);
      end;

      FreeSectionList(list);

      CloseStorage(lstorage);
    end;
  end;
end;

procedure ExportRadioContact(num:integer;fname:PAnsiChar;hContact:TMCONTACT);
var
  pc:pAnsiChar;
  section:array [0..15] of AnsiChar;
begin
  IntToStr(section,num);
  pc:=DBReadString(hContact,strCList,optMyHandle);
  WritePrivateProfileStringA(section,'Name',pc,fname);
  mFreeMem(pc);

  pc:=DBReadString(hContact,PluginName,optStationURL);
  WritePrivateProfileStringA(section,'URL',pc,fname);
  mFreeMem(pc);

  pc:=DBReadString(hContact,PluginName,optGenre);
  if pc<>nil then
  begin
    WritePrivateProfileStringA(section,optGenre,pc,fname);
    mFreeMem(pc);
  end;

  pc:=DBReadString(hContact,PluginName,optBitrate);
  if pc<>nil then
  begin
    WritePrivateProfileStringA(section,optBitrate,pc,fname);
    mFreeMem(pc);
  end;

  pc:=DBReadString(hContact,strCList,optGroup);
  if pc<>nil then
  begin
    WritePrivateProfileStringA(section,optGroup,pc,fname);
    mFreeMem(pc);
  end;
end;

function ExportAll(wParam:WPARAM;lParam:LPARAM):int; cdecl;
var
  dst:array [0..MAX_PATH-1] of AnsiChar;
  hContact:TMCONTACT;
begin
  result:=0;
  if lParam<>0 then
    StrCopy(dst,PAnsiChar(lParam));
  if (lParam<>0) or ShowDlg(dst,'radio.ini',nil,false) then
  begin
    if (wParam<>0) and (CallService(MS_DB_CONTACT_IS,wParam,0)<>0) then
    begin
      result:=1;
      ExportRadioContact(result,dst,wParam)
    end
    else
    begin
      hContact:=db_find_first(PluginName);
      while hContact<>0 do
      begin
        inc(result);
        ExportRadioContact(result,dst,hContact);
        hContact:=db_find_next(hContact,PluginName);
      end;
    end;
  end;
end;

function QuickOpen(wParam:WPARAM;lParam:LPARAM):int; cdecl;
var
  filter:pWideChar;
  buf:array [0..MAX_PATH-1] of WideChar;
begin
  filter:=ConstructFilter;
  if ShowDlgW(@buf,nil,filter) then
  begin
    result:=CallService(MS_DB_CONTACT_ADD,0,0);
    if result<>0 then
    begin
      CallService(MS_PROTO_ADDTOCONTACT,result,tlparam(PluginName));
      //
//      DBWriteByte(result,strCList,'Hidden'   ,0);
      DBWriteByte(result,strCList,'NotOnList',1);
      // URL
      DBWriteUnicode(result,PluginName,optStationURL,@buf);
      DBWriteUnicode(result,PluginName,optFirstName ,@buf);
      // Name
      DBWriteUnicode(result,strCList  ,optMyHandle,@buf);
      DBWriteUnicode(result,PluginName,optNick    ,@buf);

      SetStatus(result,ID_STATUS_OFFLINE);

      CallService(MS_IGNORE_IGNORE,result,IGNOREEVENT_USERONLINE{IGNOREEVENT_ALL});

      CallService(MS_RADIO_COMMAND,MRC_PLAY,result);
    end;
  end
  else
  begin
    result:=0;
  end;
  mFreeMem(filter);
end;

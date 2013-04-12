{My services}

function Service_RadioPlayStop(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  p:PAnsiChar;
  lnew:bool;
  hContact:THANDLE;
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
  if eq[0].wnd<>0 then // if options opened
  begin
    CheckDlgButton(GetParent(eq[0].wnd),IDC_EQOFF,isEQ_OFF);
  end;
end;

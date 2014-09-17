{}
var
  doTray:bool;
var
  trayradioparent:THANDLE;
  trayprotoh:THANDLE;
  trayparent:THANDLE;
  traymute  :THANDLE;
  trayplay  :THANDLE;
  srvtrayplaypause:THANDLE;
  srvtraystop:THANDLE;
const
  trayStations:TSortedList = (items:nil; realCount:0; limit:0; increment:8; sortFunc: nil);
  
type
  pTrayRadioStation = ^tTrayRadioStation;
  tTrayRadioStation = record
    name    :pWideChar;
    hContact:THANDLE;
    service :THANDLE;
    menuitem:THANDLE;
    presents:int; // 0 - not used, 1 - ok, 2 - new
  end;

function MyStrSort(para1:pointer; para2:pointer):int; cdecl;
begin
  result:=lstrcmpiW{StrCmpW}(pTrayRadioStation(para1).name,pTrayRadioStation(para2).name);
end;

function ChooseStation(wParam:WPARAM;lParam,lParam1:LPARAM):int; cdecl;
begin
  result:=Service_RadioPlayStop(lParam1,0);
end;

procedure MakeStationsMenu;
var
  hContact:Cardinal;
  p:pWideChar;
  i,idx:integer;
  tmp:pTrayRadioStation;
  srch:tTrayRadioStation;
  pc:pAnsiChar;
  buf:array [0..63] of AnsiChar;
  mi:TCListMenuItem;
begin
  trayStations.sortFunc:=@MyStrSort;

  // clear presents flag
  if trayStations.realCount>0 then
    for i:=0 to trayStations.realCount-1 do
      pTrayRadioStation(trayStations.Items[i]).presents:=0;

  // Fill list
  FillChar(srch,SizeOf(srch),0);
  hContact:=db_find_first(PluginName);
  while hContact<>0 do
  begin
     p:=DBReadUnicode(hContact,strCList,'MyHandle',nil);
     if p<>nil then
     begin
       srch.name:=p;
       // search in list
       if List_GetIndex(@trayStations,@srch,@idx)<>0 then
         // found - set mark
         pTrayRadioStation(trayStations.Items[idx]).presents:=1
       else // add if not found
       begin
         mGetMem(tmp,SizeOf(tTrayRadioStation));
         tmp.name    :=p;
         tmp.presents:=2;
         tmp.hContact:=hContact;
         List_InsertPtr(@trayStations,tmp);
       end;
     end;
    hContact:=db_find_next(hContact,PluginName);
  end;
  // delete obsolete elements
  for i:=trayStations.realCount-1 downto 0 do
  begin
    tmp:=pTrayRadioStation(trayStations.Items[i]);
    if tmp.presents=0 then
    begin
      mFreeMem(tmp.name);
      DestroyServiceFunction(tmp.service);
      CallService(MO_REMOVEMENUITEM,tmp.menuitem,0);
      mFreeMem(tmp);
      List_Remove(@trayStations,i);
    end;
  end;

  // build menu from sorted list
  FillChar(mi,SizeOf(mi),0);
  mi.cbSize:=sizeof(mi);
  mi.Flags :=CMIF_KEEPUNTRANSLATED or CMIF_UNICODE or CMIF_ROOTHANDLE;
  mi.szPopupName:=TChar(trayparent);
  pc:=StrCopyE(@buf,'mRadio/Choose');
  for i:=0 to trayStations.realCount-1 do
  begin
    tmp:=pTrayRadioStation(trayStations.Items[i]);
    if tmp.presents=2 then
    begin
      IntToStr(pc,tmp.hContact);
      tmp.service:=CreateServiceFunctionParam(@buf,@ChooseStation,tmp.hContact);
      mi.position  :=i;
      mi.pszService:=@buf;
      mi.szName.w  :=tmp.name;
      if doTray then
        tmp.menuitem:=Menu_AddTrayMenuItem(@mi)
      else
        tmp.menuitem:=Menu_AddMainMenuItem(@mi);
    end;
  end;
end;

function TrayPlayPause(wParam:WPARAM;lParam:LPARAM):int_ptr; cdecl;
begin
  result:=CallService(MS_RADIO_COMMAND,MRC_PAUSE,0);
end;

function TrayStop(wParam:WPARAM;lParam:LPARAM):int_ptr; cdecl;
begin
  result:=CallService(MS_RADIO_COMMAND,MRC_STOP,0);
end;

function TrayProto(wParam:WPARAM;lParam:LPARAM):int_ptr; cdecl;
begin
  if PluginStatus=ID_STATUS_ONLINE then
    wParam:=ID_STATUS_OFFLINE
  else
    wParam:=ID_STATUS_ONLINE;
  result:=Service_SetStatus(wParam,lParam);
end;

procedure CreateTrayMenu();
var
  mi:TCListMenuItem;
  playstr:pWideChar;
  ico:pAnsiChar;
begin
  FillChar(mi, sizeof(mi), 0);
  mi.cbSize  :=sizeof(mi);
  mi.flags   :=CMIF_UNICODE;
  mi.szName.w:=cPluginName;
  mi.hIcon   :=CallService(MS_SKIN2_GETICON,0,lparam(IcoBtnSettings));
  if doTray then
    trayradioparent:=Menu_AddTrayMenuItem(@mi)
  else
    trayradioparent:=Menu_AddMainMenuItem(@mi);

  FillChar(mi, sizeof(mi), 0);
  mi.cbSize  :=sizeof(mi);
  mi.szPopupName:=TChar(trayradioparent);

  if PluginStatus=ID_STATUS_ONLINE then
  begin
    mi.szName.w  :='mRadio off';
  end
  else
  begin
    mi.szName.w  :='mRadio on';
  end;
  mi.flags:=CMIF_UNICODE or CMIF_ROOTHANDLE;
  mi.position  :=0;
  srvtraystop:=CreateServiceFunction('mRadio/TrayProto',@TrayProto);
  mi.pszService:='mRadio/TrayProto';
  if doTray then
    trayprotoh:=Menu_AddTrayMenuItem(@mi)
  else
    trayprotoh:=Menu_AddMainMenuItem(@mi);

  if gVolume<0 then
  begin
    mi.flags:=CMIF_UNICODE or CMIF_ROOTHANDLE or CMIF_CHECKED;
    ico:=IcoBtnOff;
  end
  else
  begin
    mi.flags:=CMIF_UNICODE or CMIF_ROOTHANDLE;
    ico:=IcoBtnOn;
  end;
  mi.hIcon     :=CallService(MS_SKIN2_GETICON,0,tlparam(ico));
  mi.szName.w  :='Mute';
  mi.pszService:=MS_RADIO_MUTE;
  mi.position  :=1;
  if doTray then
    traymute:=Menu_AddTrayMenuItem(@mi)
  else
    traymute:=Menu_AddMainMenuItem(@mi);

  mi.hIcon:=0;
  mi.flags:=CMIF_UNICODE or CMIF_ROOTHANDLE;
  if CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_GET)<>RD_STATUS_PLAYING then
  begin
    playstr:='Play'
  end
  else
  begin
    playstr:='Pause';
  end;
//  mi.hIcon:=CallService(MS_SKIN2_GETICON,0,tlparam(ico));
  mi.szName.w  :=playstr;
  mi.position  :=2;
  srvtrayplaypause:=CreateServiceFunction('mRadio/TrayPlayPause',@TrayPlayPause);
  mi.pszService:='mRadio/TrayPlayPause';
  if doTray then
    trayplay:=Menu_AddTrayMenuItem(@mi)
  else
    trayplay:=Menu_AddTrayMenuItem(@mi);

  mi.szName.w  :='Stop';
  mi.position  :=3;
  srvtraystop:=CreateServiceFunction('mRadio/TrayStop',@TrayStop);
  mi.pszService:='mRadio/TrayStop';
  if doTray then
    Menu_AddTrayMenuItem(@mi)
  else
    Menu_AddMainMenuItem(@mi);

  mi.szName.w  :='Quick Open';
  mi.position  :=4;
  mi.pszService:=MS_RADIO_QUICKOPEN;
  mi.hIcon     :=CallService(MS_SKIN2_GETICON,0,tlparam(IcoBtnOpen));
  if doTray then
    Menu_AddTrayMenuItem(@mi)
  else
    Menu_AddMainMenuItem(@mi);

  mi.hIcon    :=0;
  mi.szName.w :='Play Station';
  mi.position :=1000;
  mi.pszService:=nil;
  if doTray then
    trayparent:=Menu_AddTrayMenuItem(@mi)
  else
    trayparent:=Menu_AddMainMenuItem(@mi);
end;

function TrayPrebuild(wParam:WPARAM;lParam:LPARAM):int; cdecl;
var
  mi:tClistMenuItem;
  playstr:pWideChar;
  ico:pAnsiChar;
begin
  FillChar(mi,sizeof(mi),0);
  mi.cbSize:=sizeof(mi);

  mi.flags:=CMIM_NAME or CMIF_UNICODE;
  if PluginStatus=ID_STATUS_ONLINE then
  begin
    playstr:='mRadio off';
  end
  else
  begin
    playstr:='mRadio on';
  end;
  mi.szName.w:=playstr;
  CallService(MS_CLIST_MODIFYMENUITEM,trayprotoh,tlparam(@mi));

  if gVolume<0 then
  begin
    ico:=IcoBtnOff;
  end
  else
  begin
    ico:=IcoBtnOn;
  end;
  mi.flags:=CMIM_FLAGS or CMIM_ICON;
  mi.hIcon:=CallService(MS_SKIN2_GETICON,0,tlparam(ico));
  CallService(MS_CLIST_MODIFYMENUITEM,traymute,tlparam(@mi));
  mi.hIcon:=0;

  mi.flags:=CMIM_NAME or CMIF_UNICODE;
  if CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_GET)<>RD_STATUS_PLAYING then
    playstr:='Play'
  else
    playstr:='Pause';
  mi.szName.w:=playstr;
  CallService(MS_CLIST_MODIFYMENUITEM,trayplay,tlparam(@mi));

  MakeStationsMenu();
  result:=0;
end;

procedure CreateMIMTrayMenu;
begin
  doTray:=ServiceExists(MS_CLIST_ADDTRAYMENUITEM)<>0;

  CreateTrayMenu();
  MakeStationsMenu();
  if doTray then
    HookEvent(ME_CLIST_PREBUILDTRAYMENU,@TrayPrebuild)
  else
    HookEvent(ME_CLIST_PREBUILDMAINMENU,@TrayPrebuild)
end;

procedure RemoveTrayItems;
var
  i:integer;
  tmp:pTrayRadioStation;
begin
  // remove stations
  for i:=trayStations.realCount-1 downto 0 do
  begin
    tmp:=pTrayRadioStation(trayStations.Items[i]);
    mFreeMem(tmp.name);
    DestroyServiceFunction(tmp.service);
    mFreeMem(tmp);
  end;
  List_Destroy(@trayStations);
  DestroyServiceFunction(srvtrayplaypause);
  DestroyServiceFunction(srvtraystop);

  CallService(MO_REMOVEMENUITEM,trayradioparent,0);
end;

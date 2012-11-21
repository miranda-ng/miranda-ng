{}
var
  trayradioparent:THANDLE;
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
  hContact:=CallService(MS_DB_CONTACT_FINDFIRST,0,0);
  while hContact<>0 do
  begin
     if StrCmp(PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO,hContact,0)),cPluginName)=0 then
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
     end;
    hContact:=CallService(MS_DB_CONTACT_FINDNEXT,hContact,0);
  end;

  // delete obsolete elements
  for i:=trayStations.realCount-1 downto 0 do
  begin
    tmp:=pTrayRadioStation(trayStations.Items[i]);
    if tmp.presents=0 then
    begin
      mFreeMem(tmp.name);
      DestroyServiceFunction(tmp.service);
      CallService(MS_CLIST_REMOVETRAYMENUITEM,tmp.menuitem,0);
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
      tmp.menuitem:=Menu_AddTrayMenuItem(@mi);
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

procedure CreateTrayMenu();
var
  mi:TCListMenuItem;
  playstr:pWideChar;
begin
  FillChar(mi, sizeof(mi), 0);
  mi.cbSize  :=sizeof(mi);
  mi.flags   :=CMIF_UNICODE;
  mi.szName.w:=cPluginName;
  mi.hIcon   :=CallService(MS_SKIN2_GETICON,0,lparam(IcoBtnSettings));
  trayradioparent:=Menu_AddTrayMenuItem(@mi);

  FillChar(mi, sizeof(mi), 0);
  mi.cbSize  :=sizeof(mi);
  mi.szPopupName:=TChar(trayradioparent);

  if gVolume<0 then
    mi.flags:=CMIF_UNICODE or CMIF_ROOTHANDLE or CMIF_CHECKED
  else
    mi.flags:=CMIF_UNICODE or CMIF_ROOTHANDLE;
  mi.szName.w  :='Mute';
  mi.pszService:=MS_RADIO_MUTE;
  mi.position  :=1;
  traymute:=Menu_AddTrayMenuItem(@mi);

  mi.flags:=CMIF_UNICODE or CMIF_ROOTHANDLE;
  if CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_GET)<>RD_STATUS_PLAYING then
    playstr:='Play'
  else
    playstr:='Pause';
  mi.szName.w  :=playstr;
  mi.position  :=2;
  srvtrayplaypause:=CreateServiceFunction('mRadio/TrayPlayPause',@TrayPlayPause);
  mi.pszService:='mRadio/TrayPlayPause';
  trayplay:=Menu_AddTrayMenuItem(@mi);

  mi.szName.w  :='Stop';
  mi.position  :=3;
  srvtraystop:=CreateServiceFunction('mRadio/TrayStop',@TrayStop);
  mi.pszService:='mRadio/TrayStop';
  Menu_AddTrayMenuItem(@mi);

  mi.szName.w :='Play Station';
  mi.position :=1000;
  mi.pszService:=nil;
  trayparent:=Menu_AddTrayMenuItem(@mi);
end;

function TrayPrebuild(wParam:WPARAM;lParam:LPARAM):int; cdecl;
var
  mi:tClistMenuItem;
  playstr:pWideChar;
begin
  FillChar(mi,sizeof(mi),0);
  mi.cbSize:=sizeof(mi);
  if gVolume<0 then
    mi.flags:=CMIM_FLAGS or CMIF_CHECKED
  else
    mi.flags:=CMIM_FLAGS;
  CallService(MS_CLIST_MODIFYMENUITEM,traymute,tlparam(@mi));

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
  if ServiceExists(MS_CLIST_ADDTRAYMENUITEM)<>0 then
//    if hiddenwindow<>0 then
    begin
      CreateTrayMenu();
      MakeStationsMenu();
      HookEvent(ME_CLIST_PREBUILDTRAYMENU,@TrayPrebuild);
    end;
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
//    CallService(MS_CLIST_REMOVETRAYMENUITEM,tmp.menuitem,0);
    mFreeMem(tmp);
  end;
  List_Destroy(@trayStations);
  DestroyServiceFunction(srvtrayplaypause);
  DestroyServiceFunction(srvtraystop);

  if ServiceExists(MS_CLIST_REMOVETRAYMENUITEM)<>0 then
    CallService(MS_CLIST_REMOVETRAYMENUITEM,trayradioparent,0);

end;

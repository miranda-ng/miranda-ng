{}
type
  pTrayRadioStation = ^tTrayRadioStation;
  tTrayRadioStation = record
    name:pWideChar;
    handle:THANDLE;
  end;

function MyStrSort(para1:pointer; para2:pointer):int; cdecl;
begin
  result:=StrCmpW(pTrayRadioStation(para1).name,pTrayRadioStation(para2).name);
end;

function MakeStationsMenu:HMENU;
var
  hContact:Cardinal;
  sl:TSortedList;
  p:pWideChar;
  i:integer;
  flag:integer;
  tmp:pTrayRadioStation;
begin
  result:=CreatePopupMenu;
  if result<>0 then
  begin
    FillChar(sl,SizeOf(sl),0);
    sl.increment:=16;
    sl.sortFunc:=@MyStrSort;
    hContact:=CallService(MS_DB_CONTACT_FINDFIRST,0,0);
    while hContact<>0 do
    begin
       if StrCmp(PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO,hContact,0)),cPluginName)=0 then
       begin
         p:=DBReadUnicode(hContact,strCList,'MyHandle',nil);
         if p<>nil then
         begin
           mGetMem(tmp,SizeOf(tTrayRadioStation));
           tmp.name:=p;
           tmp.handle:=hContact;
           List_InsertPtr(@sl,tmp);
         end;
       end;
      hContact:=CallService(MS_DB_CONTACT_FINDNEXT,hContact,0);
    end;

    for i:=0 to sl.realCount-1 do
    begin
      if (i=0) or ((i mod 20)<>0) then
        flag:=MF_STRING
      else
        flag:=MF_STRING or MF_MENUBARBREAK;
      tmp:=sl.Items[i];
      AppendMenuW(result,flag,tmp.handle,tmp.name);
      mFreeMem(tmp.name);
      mFreeMem(tmp);
    end;
    List_Destroy(@sl);
  end;
end;

function CreateTrayMenu(wParam:WPARAM;lParam:LPARAM):int; cdecl;
const
  startid = 100;
var
  menu:HMENU;
  flag,id:integer;
  pt:TPOINT;
  playstr:pWideChar;
begin
  id:=0;
  menu:=CreatePopupMenu;
  if menu<>0 then
  begin
    if gVolume<0 then
      flag:=MF_STRING+MF_CHECKED
    else
      flag:=MF_STRING+MF_UNCHECKED;

    if CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_GET)<>RD_STATUS_PAUSED then
      playstr:='Pause'
    else
      playstr:='Play';

    AppendMenuW(menu,flag     ,startid+1,TranslateW('Mute'));
    AppendMenuW(menu,MF_STRING,startid+2,TranslateW(playstr));
    AppendMenuW(menu,MF_STRING,startid+3,TranslateW('Stop'));
    AppendMenuW(menu,MF_SEPARATOR,0,nil);
    AppendMenuW(menu,MF_POPUP,MakeStationsMenu,TranslateW('Play Station'));
    GetCursorPos(pt);
    id:=integer(TrackPopupMenu(menu,TPM_RETURNCMD+TPM_NONOTIFY,pt.x,pt.y,0,hiddenwindow,nil));
    case id of
      0: ; // nothing
      startid+1: begin // mute
        Service_RadioMute(0,0);
      end;
      startid+2: begin // play/pause
        CallService(MS_RADIO_COMMAND,MRC_PAUSE,0);
      end;
      startid+3: begin // stop
        CallService(MS_RADIO_COMMAND,MRC_STOP,0);
      end;
    else // choose station
      Service_RadioPlayStop(id,0);
    end;
    DestroyMenu(menu);
  end;
  result:=id;
end;

procedure CreateMIMTrayMenu;
var
  mi:TCListMenuItem;
begin
  if ServiceExists(MS_CLIST_ADDTRAYMENUITEM)<>0 then
//    if hiddenwindow<>0 then
    begin
      FillChar(mi, sizeof(mi), 0);
      mi.cbSize    :=sizeof(mi);
      mi.pszService:=MS_RADIO_TRAYMENU;
      mi.szName.a  :=cPluginName;
      mi.hIcon     :=CallService(MS_SKIN2_GETICON,0,lparam(IcoBtnSettings));
      Menu_AddTrayMenuItem(@mi);
    end;
end;


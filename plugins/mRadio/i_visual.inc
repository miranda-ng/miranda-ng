{Visual part}
var
  hCMenuItemRec,
  hCMenuItemPlay:THANDLE;

function OnContactMenu(hContact:WPARAM;lParam:LPARAM):int;cdecl;
begin
  if StrCmp(Proto_GetProtoName(hContact),PluginName)<>0 then
  begin
    Menu_ShowItem(hCMenuItemRec, 0);
    Menu_ShowItem(hCMenuItemPlay, 0);
  end
  else
  begin
    // play/Stop
    if THANDLE(hContact)<>ActiveContact then
      Menu_ModifyItem(hCMenuItemPlay, 'Start broadcasting', IcoLib_GetIcon(IcoBtnOn,0))
    else
      Menu_ModifyItem(hCMenuItemPlay, 'Stop broadcasting', IcoLib_GetIcon(IcoBtnOff,0));

    // record
    if Service_RadioRecord(0,1)<>0 then
      Menu_ModifyItem(hCMenuItemRec, 'Stop record', IcoLib_GetIcon(IcoBtnRecDn,0))
    else
      Menu_ModifyItem(hCMenuItemRec, 'Start record', IcoLib_GetIcon(IcoBtnRecUp,0));
  end;

  result:=0;
end;

procedure CreateMenu;
var
  mi:TCListMenuItem;
begin
  FillChar(mi, sizeof(mi), 0);
  mi.pszService:=MS_RADIO_RECORD;
  mi.szName.a  :='Start/Stop Record';
  hCMenuItemRec:=Menu_AddContactMenuItem(@mi);

  mi.hIcon      :=IcoLib_GetIcon(IcoBtnOn,0);
  mi.pszService :=MS_RADIO_PLAYSTOP;
  mi.szName.a   :='Start/Stop broadcasting';
  hCMenuItemPlay:=Menu_AddContactMenuItem(@mi);
end;

procedure RegisterIcons;
var
  sid:TSKINICONDESC;
begin
  FillChar(sid,SizeOf(TSKINICONDESC),0);
  sid.cx:=16;
  sid.cy:=16;
  sid.szSection.a:='Protocols/mRadio';
  sid.hDefaultIcon   :=LoadImage(hInstance,MAKEINTRESOURCE(BTN_RECUP),IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoBtnRecUp;
  sid.szDescription.a:='Start record';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);

  sid.hDefaultIcon   :=LoadImage(hInstance,MAKEINTRESOURCE(BTN_RECDN),IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoBtnRecDn;
  sid.szDescription.a:='Stop record';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);

  sid.hDefaultIcon   :=LoadImage(hInstance,MAKEINTRESOURCE(IDI_MAIN),IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoBtnSettings;
  sid.szDescription.a:='Settings';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);

  sid.hDefaultIcon   :=LoadImage(hInstance,MAKEINTRESOURCE(IDI_ON),IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoBtnOn;
  sid.szDescription.a:='Broadcast ON';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);

  sid.hDefaultIcon   :=LoadImage(hInstance,MAKEINTRESOURCE(IDI_OFF),IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoBtnOff;
  sid.szDescription.a:='Broadcast OFF';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);

  sid.hDefaultIcon   :=LoadImage(hInstance,MAKEINTRESOURCE(IDI_OPEN),IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoBtnOpen;
  sid.szDescription.a:='Quick Open';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);

  sid.hDefaultIcon   :=LoadImage(hInstance,MAKEINTRESOURCE(IDI_ADD),IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoBtnAdd;
  sid.szDescription.a:='Add EQ preset';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);

  sid.hDefaultIcon   :=LoadImage(hInstance,MAKEINTRESOURCE(IDI_DEL),IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoBtnDel;
  sid.szDescription.a:='Delete EQ preset';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);
end;

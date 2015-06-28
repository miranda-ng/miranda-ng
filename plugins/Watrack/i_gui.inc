{some visual stuff}

function OnTTBLoaded(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  ttb:m_api.TTBButton;
begin
  FillChar(ttb,SizeOf(ttb),0);
  ttb.dwFlags:=TTBBF_VISIBLE or TTBBF_SHOWTOOLTIP;

  // plugin status button
  if DisablePlugin<>dsPermanent then
    ttb.dwFlags:=ttb.dwFlags or TTBBF_PUSHED;

  ttb.hIconDn   :=IcoLib_GetIcon(IcoBtnEnable,0);
  ttb.hIconUp   :=IcoLib_GetIcon(IcoBtnDisable,0);
  ttb.wParamUp  :=1;
  ttb.pszService:=MS_WAT_PLUGINSTATUS;
  ttb.name      :='WATrack status';
  ttbState:=TopToolbar_AddButton(@ttb);
  if ttbState=THANDLE(-1) then
    ttbState:=0
  else
    CallService(MS_TTB_SETBUTTONOPTIONS,(ttbState shl 16)+TTBO_TIPNAME,
        tlparam(Translate('Disable WATrack')));
  result:=0;
end;

procedure CreateMenus;
var
  mi:TMO_MenuItem;
begin
  FillChar(mi, sizeof(mi), 0);
  mi.root      :=Menu_CreateRoot(MO_MAIN, 'Watrack', 0, 0, 0);
  mi.hIcon     :=IcoLib_GetIcon(IcoBtnEnable,0);
  mi.szName.a  :='Disable WATrack';
  mi.pszService:=MS_WAT_PLUGINSTATUS;
  mi.position  :=MenuDisablePos;
  hMenuDisable:=Menu_AddMainMenuItem(@mi);
end;

procedure ChangeMenuIcons(f1:cardinal);
var
  p:PAnsiChar;
begin
  if f1<>0 then
    Menu_ModifyItem(hMenuDisable, 'Enable WATrack', IcoLib_GetIcon(IcoBtnDisable,0), f1)
  else
    Menu_ModifyItem(hMenuDisable, 'Disable WATrack', IcoLib_GetIcon(IcoBtnEnable,0));

  if ServiceExists(MS_TTB_SETBUTTONSTATE)<>0 then
  begin
    if f1<>0 then
    begin
      p:='Enable WATrack';
      CallService(MS_TTB_SETBUTTONSTATE,ttbState,0)
    end
    else
    begin
      p:='Disable WATrack';
      CallService(MS_TTB_SETBUTTONSTATE,ttbState,TTBST_PUSHED);
    end;
    CallService(MS_TTB_SETBUTTONOPTIONS,(ttbState shl 16)+TTBO_TIPNAME,
        lparam(Translate(p)));
  end;
end;

function IconChanged(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  ttb:m_api.TTBButton;
begin
  result:=0;
  Menu_ModifyItem(hMenuDisable, nil, IcoLib_GetIcon(IcoBtnEnable,0));

  // toptoolbar
  if ServiceExists(MS_TTB_GETBUTTONOPTIONS)<>0 then
  begin
    CallService(MS_TTB_GETBUTTONOPTIONS,(ttbState shl 16)+TTBO_ALLDATA,tlparam(@ttb));
    ttb.hIconDn:=IcoLib_GetIcon(IcoBtnEnable,0);
    ttb.hIconUp:=IcoLib_GetIcon(IcoBtnDisable,0);
    CallService(MS_TTB_SETBUTTONOPTIONS,(ttbState shl 16)+TTBO_ALLDATA,tlparam(@ttb));
  end;

end;

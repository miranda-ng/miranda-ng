{some visual stuff}

function OnTTBLoaded(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  ttb:m_api.TTBButton;
begin
  FillChar(ttb,SizeOf(ttb),0);
  ttb.cbSize :=SizeOf(ttb);
  ttb.dwFlags:=TTBBF_VISIBLE or TTBBF_SHOWTOOLTIP;

  // plugin status button
  if DisablePlugin<>dsPermanent then
    ttb.dwFlags:=ttb.dwFlags or TTBBF_PUSHED;

  ttb.hIconDn   :=CallService(MS_SKIN2_GETICON,0,tlparam(IcoBtnEnable));
  ttb.hIconUp   :=CallService(MS_SKIN2_GETICON,0,tlparam(IcoBtnDisable));
  ttb.wParamUp  :=1;
//  ttb.wParamDown    :=0;
  ttb.pszService:=MS_WAT_PLUGINSTATUS;
  ttb.name      :='WATrack status';
//  ttb.tooltipDn:='Disable Plugin';
//  ttb.tooltipUp:='Enable Plugin';
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
  mi:TCListMenuItem;
begin
  FillChar(mi, sizeof(mi), 0);
  mi.cbSize       :=sizeof(mi);
  mi.szPopupName.a:=PluginShort;

  mi.hIcon        :=CallService(MS_SKIN2_GETICON,0,lparam(IcoBtnEnable));
  mi.szName.a     :='Disable WATrack';
  mi.pszService   :=MS_WAT_PLUGINSTATUS;
  mi.popupPosition:=MenuDisablePos;
  hMenuDisable:=Menu_AddMainMenuItem(@mi);
end;

procedure ChangeMenuIcons(f1:cardinal);
var
  mi:tClistMenuItem;
  p:PAnsiChar;
begin
  FillChar(mi,sizeof(mi),0);
  mi.cbSize:=sizeof(mi);
  mi.flags :=CMIM_NAME+CMIM_FLAGS+CMIM_ICON+f1;
  if f1<>0 then
  begin
    mi.hIcon   :=CallService(MS_SKIN2_GETICON,0,lparam(IcoBtnDisable));
    mi.szName.a:='Enable WATrack';
  end
  else
  begin
    mi.hIcon   :=CallService(MS_SKIN2_GETICON,0,lparam(IcoBtnEnable));
    mi.szName.a:='Disable WATrack';
  end;
  CallService(MS_CLIST_MODIFYMENUITEM,hMenuDisable,lparam(@mi));

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
  mi:TCListMenuItem;
  ttb:m_api.TTBButton;
begin
  result:=0;
  FillChar(mi,SizeOf(mi),0);
  mi.cbSize:=sizeof(mi);
  mi.flags :=CMIM_ICON;

  mi.hIcon:=CallService(MS_SKIN2_GETICON,0,tlparam(IcoBtnEnable));
  CallService(MS_CLIST_MODIFYMENUITEM,hMenuDisable,tlparam(@mi));

// toptoolbar
  if ServiceExists(MS_TTB_GETBUTTONOPTIONS)<>0 then
  begin
{
    CallService(MS_TTB_GETBUTTONOPTIONS,(ttbInfo shl 16)+TTBO_ALLDATA,tlparam(@ttb));
    ttb.hIconUp:=CallService(MS_SKIN2_GETICON,0,tlparam(IcoBtnInfo));
    ttb.hIconDn:=ttb.hIconUp;
    CallService(MS_TTB_SETBUTTONOPTIONS,(ttbInfo shl 16)+TTBO_ALLDATA,tlparam(@ttb));
}
    CallService(MS_TTB_GETBUTTONOPTIONS,(ttbState shl 16)+TTBO_ALLDATA,tlparam(@ttb));
    ttb.hIconDn:=CallService(MS_SKIN2_GETICON,0,tlparam(IcoBtnEnable));
    ttb.hIconUp:=CallService(MS_SKIN2_GETICON,0,tlparam(IcoBtnDisable));
    CallService(MS_TTB_SETBUTTONOPTIONS,(ttbState shl 16)+TTBO_ALLDATA,tlparam(@ttb));
  end;

end;

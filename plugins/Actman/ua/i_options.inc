{}
const
  opt_TTBTooltip        :pAnsiChar = 'MTBTooltip';
  opt_TTBTooltipPressed :pAnsiChar = 'MTBTooltipPressed';
  opt_TTBShowWhenVars   :pAnsiChar = 'MTBVarStr';

  opt_TabBTooltip       :pAnsiChar = 'TabBTooltip';
  opt_TabBTooltipPressed:pAnsiChar = 'TabBTooltipPressed';

  opt_MenuPopup       :pAnsiChar = 'MenuPopup';
  opt_MenuNameVars    :pAnsiChar = 'MenuName';
  opt_MenuShowWhenVars:pAnsiChar = 'MenuVarStr';
  opt_MenuOptions     :pAnsiChar = 'MenuOptions';

{}
procedure DeleteUASettings(idx:integer);
var
  setting:array [0..63] of AnsiChar;
  p,pm:pAnsiChar;
  lmenu:tMenuType;
begin
  with UActionList[idx] do
  begin
    p:=GetUABranch(setting,dwActID);
    if p<>nil then
    begin
      p:=StrCopyE(p,opt_UA);
      p^:='/'; inc(p);
{!!
      p^:='*'; inc(p); p^:=#0;
      DBDeleteGroup(0,DBBranch,setting);
!!}
      StrCopy(p,opt_Flags); DBDeleteSetting(0,DBBranch,setting);
    
      StrCopy(p,opt_TTBTooltip        ); DBDeleteSetting(0,DBBranch,setting);
      StrCopy(p,opt_TTBTooltipPressed ); DBDeleteSetting(0,DBBranch,setting);
      StrCopy(p,opt_TTBShowWhenVars   ); DBDeleteSetting(0,DBBranch,setting);

      StrCopy(p,opt_TabBTooltip       ); DBDeleteSetting(0,DBBranch,setting);
      StrCopy(p,opt_TabBTooltipPressed); DBDeleteSetting(0,DBBranch,setting);

      for lmenu:=main_menu to HIGH(tMenuType) do
      begin
        pm:=p;
        pm^:=AnsiChar(ORD(lmenu)+ORD('0')); inc(pm);
        pm^:='_'; inc(pm);
        StrCopy(pm,opt_MenuPopup       ); DBDeleteSetting(0,DBBranch,setting);
        StrCopy(pm,opt_MenuNameVars    ); DBDeleteSetting(0,DBBranch,setting);
        StrCopy(pm,opt_MenuShowWhenVars); DBDeleteSetting(0,DBBranch,setting);
        StrCopy(pm,opt_MenuOptions     ); DBDeleteSetting(0,DBBranch,setting);
      end;
    end;
  end;
end;

procedure addSaveUA(setting:pAnsiChar;txt:pWideChar); overload;
begin
  if (txt=nil) or (txt^=#0) then DBDeleteSetting(0,DBBranch,setting)
  else DBWriteUnicode(0,DBBranch,setting,txt);
end;

procedure addSaveUA(setting:pAnsiChar;txt:pAnsiChar); overload;
begin
  if (txt=nil) or (txt^=#0) then DBDeleteSetting(0,DBBranch,setting)
  else DBWriteString(0,DBBranch,setting,txt);
end;

procedure SaveUA(idx:integer);
var
  setting:array [0..63] of AnsiChar;
  p,pm:pAnsiChar;
  lmenu:tMenuType;
begin
  with UActionList[idx] do
  begin
    p:=GetUABranch(setting,dwActID);
    if p<>nil then
    begin
      p:=StrCopyE(p,opt_UA);
      p^:='/'; inc(p);

      StrCopy(p,opt_Flags); DBWriteDWord(0,DBBranch,setting,
          flags and not (UAF_REALTIME OR UAF_SPECIAL));

      StrCopy(p,opt_TTBTooltip       ); addSaveUA(setting,szTTBTooltip);
      StrCopy(p,opt_TTBTooltipPressed); addSaveUA(setting,szTTBTooltipPressed);
      StrCopy(p,opt_TTBShowWhenVars  ); addSaveUA(setting,szTTBShowWhenVars);

      StrCopy(p,opt_TabBTooltip       ); addSaveUA(setting,szTabBTooltip);
      StrCopy(p,opt_TabBTooltipPressed); addSaveUA(setting,szTabBTooltipPressed);

      for lmenu:=main_menu to HIGH(tMenuType) do
      begin
        pm:=p;
        pm^:=AnsiChar(ORD(lmenu)+ORD('0')); inc(pm);
        pm^:='_'; inc(pm);
        with UAMenuItem[lmenu] do
        begin
          StrCopy(pm,opt_MenuPopup       ); addSaveUA(setting,szMenuPopup);
          StrCopy(pm,opt_MenuNameVars    ); addSaveUA(setting,szMenuNameVars);
          StrCopy(pm,opt_MenuShowWhenVars); addSaveUA(setting,szMenuShowWhenVars);
          StrCopy(pm,opt_MenuOptions     ); DBWriteWord(0,DBBranch,setting,menu_opt);
        end;
      end;
    end;
  end;
end;

procedure SaveUAs;
var
  i:integer;
begin
  for i:=0 to HIGH(UActionList) do
    SaveUA(i);
end;

function LoadUA(idx:integer):integer;
var
  setting:array [0..63] of AnsiChar;
  p,pm:pAnsiChar;
  lmenu:tMenuType;
begin
  result:=0;
  with UActionList[idx] do
  begin
    p:=GetUABranch(setting,dwActID);
    if p<>nil then
    begin
      p:=StrCopyE(p,opt_UA);
      p^:='/'; inc(p);

      StrCopy(p,opt_Flags);
      flags:=DBReadDWord(0,DBBranch,setting,dword(UAF_SPECIAL));
      if flags=dword(UAF_SPECIAL) then // no settings
      begin
        flags:=0;
        exit;
      end;
      // no need to use previous "pressed" state
      if (flags and UAF_SAVESTATE)=0 then
        flags:=flags and not UAF_PRESSED;

      flags:=flags and not UAF_REALTIME;
      result:=1;

      StrCopy(p,opt_TTBTooltip       ); szTTBTooltip       :=DBReadString (0,DBBranch,setting);
      StrCopy(p,opt_TTBTooltipPressed); szTTBTooltipPressed:=DBReadString (0,DBBranch,setting);
      StrCopy(p,opt_TTBShowWhenVars  ); szTTBShowWhenVars  :=DBReadUnicode(0,DBBranch,setting);
                                        
      StrCopy(p,opt_TabBTooltip       ); szTabBTooltip       :=DBReadUnicode(0,DBBranch,setting);
      StrCopy(p,opt_TabBTooltipPressed); szTabBTooltipPressed:=DBReadUnicode(0,DBBranch,setting);

      for lmenu:=main_menu to HIGH(tMenuType) do
      begin
        pm:=p;
        pm^:=AnsiChar(ORD(lmenu)+ORD('0')); inc(pm);
        pm^:='_'; inc(pm);
        with UAMenuItem[lmenu] do
        begin
          StrCopy(pm,opt_MenuPopup       ); szMenuPopup       :=DBReadUnicode(0,DBBranch,setting);
          StrCopy(pm,opt_MenuNameVars    ); szMenuNameVars    :=DBReadUnicode(0,DBBranch,setting);
          StrCopy(pm,opt_MenuShowWhenVars); szMenuShowWhenVars:=DBReadUnicode(0,DBBranch,setting);
          StrCopy(pm,opt_MenuOptions     ); menu_opt          :=DBReadWord   (0,DBBranch,setting);
        end;
      end;
    end;
  end;
end;
(*
function LoadUAs:integer;
{
var
  section:array [0..63] of AnsiChar;
  p:PAnsiChar;
  i:integer;
}
begin
  result:=0;
{
  MaxTasks:=DBReadByte(0,opt_tasks,opt_count);
  result:=MaxTasks;
  if MaxTasks>0 then
  begin
    GetMem  (TaskList ,MaxTasks*SizeOf(tTaskRec));
    FillChar(TaskList^,MaxTasks*SizeOf(tTaskRec),0);
    for i:=0 to MaxTasks-1 do
    begin
      p:=StrEnd(IntToStr(section,i));
      with TaskList[i] do
      begin
        StrCopy(p,opt_flags ); flags    :=DBReadDWord  (0,opt_tasks,section);
        StrCopy(p,opt_name  ); name     :=DBReadUnicode(0,opt_tasks,section);
        StrCopy(p,opt_action); action   :=DBReadDWord  (0,opt_tasks,section);
      end;
    end;
  end;
}
end;
*)
procedure SetAllActionUsers(var ActionItem:tMyActionItem; initial:boolean);
var
  luse:boolean;
  lmenu:tMenuType;
begin
  if NamesArray[uaHotkey].enable then
  begin
    if (ActionItem.flags and UAF_REGHOTKEY)<>0 then
      AddCoreHotkey(ActionItem)
    else
      DeleteCoreHotkey(ActionItem);
  end;
  if not initial then
  begin
    if NamesArray[uaTTB].enable then
    begin
      DeleteTTBButton(ActionItem); // no modify command there, just delete, then insert back
      if (ActionItem.flags and UAF_REGTTBB)<>0 then
        AddTTBButton(ActionItem);
    end;

    if NamesArray[uaTAB].enable then
    begin
      if (ActionItem.flags and UAF_REGTABB)<>0 then
        AddTabBBButton(ActionItem)
      else
        DeleteTabBBButton(ActionItem);
    end;
  end;

  luse:=false;
  for lmenu:=main_menu to HIGH(tMenuType) do
  begin
    if NamesArray[uaMenu+ORD(lmenu)].enable then
    begin
      if (ActionItem.UAMenuItem[lmenu].menu_opt and UAF_MENUUSE)<>0 then
      begin
        luse:=true;
        if ActionItem.UAMenuItem[lmenu].changed then
          DeleteMenuItem(ActionItem,lmenu);
        CreateMenuItem(ActionItem,lmenu);
      end
      else
        DeleteMenuItem(ActionItem,lmenu);
    end;
  end;

  if (not luse) and (ActionItem.hMenuService<>0) then
  begin
    DestroyServiceFunction(ActionItem.hMenuService);
    ActionItem.hMenuService:=0;
  end;
end;

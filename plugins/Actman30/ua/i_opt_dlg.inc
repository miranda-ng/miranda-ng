{}
const
  settings:HWND = 0;

var
  hIC:THANDLE;

//----- Static (no option-depended) control view -----

procedure HideAllControls(Dialog:HWND);
begin
  ShowWindow(GetDlgItem(Dialog,IDC_UA_GLOBAL     ),SW_HIDE);

  ShowWindow(GetDlgItem(Dialog,IDC_UA_BLOCK      ),SW_HIDE);
  ShowWindow(GetDlgItem(Dialog,IDC_UA_MENU       ),SW_HIDE);
  ShowWindow(GetDlgItem(Dialog,IDC_UA_NOTRANSLATE),SW_HIDE);
  ShowWindow(GetDlgItem(Dialog,IDC_UA_SEPARATE   ),SW_HIDE);
  ShowWindow(GetDlgItem(Dialog,IDC_UA_POPUPT     ),SW_HIDE);
  ShowWindow(GetDlgItem(Dialog,IDC_UA_VARNAMEST  ),SW_HIDE);
//  ShowWindow(GetDlgItem(Dialog,IDC_UA_NOTRANSLATE),SW_HIDE);

  ShowWindow(GetDlgItem(Dialog,IDC_UA_TTNORMALT  ),SW_HIDE);
  ShowWindow(GetDlgItem(Dialog,IDC_UA_TTPRESSEDT ),SW_HIDE);

  ShowWindow(GetDlgItem(Dialog,IDC_UA_SHOWVART   ),SW_HIDE);


  ShowEditField(Dialog,IDC_UA_SHOWVARV  ,SW_HIDE);
  ShowEditField(Dialog,IDC_UA_VARNAMESV ,SW_HIDE);
  ShowEditField(Dialog,IDC_UA_POPUPV    ,SW_HIDE);
  ShowEditField(Dialog,IDC_UA_TTNORMALV ,SW_HIDE);
  ShowEditField(Dialog,IDC_UA_TTPRESSEDV,SW_HIDE);
end;

//  Show or hide option items ("static options")
procedure ShowHideControls(Dialog:HWND;atype:integer;item:integer=-1);
var
  show:integer;
begin
  HideAllControls(Dialog);
  if atype>=0 then
  begin
    if LoByte(atype)<>uaHotkey then
      show:=SW_SHOW
    else
      show:=SW_HIDE;
    ShowWindow(GetDlgItem(Dialog,IDC_UA_TWOSTATE),show);
    ShowWindow(GetDlgItem(Dialog,IDC_UA_SAVSTATE),show);
    ShowWindow(GetDlgItem(Dialog,IDC_UA_COMMON  ),show);

    case LoByte(atype) of
      uaTTB: begin
        ShowWindow(GetDlgItem(Dialog,IDC_UA_SHOWVART  ),SW_SHOW);
        ShowEditField(Dialog,IDC_UA_SHOWVARV  ,SW_SHOW);

        ShowWindow(GetDlgItem(Dialog,IDC_UA_TTNORMALT ),SW_SHOW);
        ShowWindow(GetDlgItem(Dialog,IDC_UA_TTPRESSEDT),SW_SHOW);
        ShowEditField(Dialog,IDC_UA_TTNORMALV ,SW_SHOW);
        ShowEditField(Dialog,IDC_UA_TTPRESSEDV,SW_SHOW);
      end;
      uaTAB: begin
        ShowWindow(GetDlgItem(Dialog,IDC_UA_GLOBAL    ),SW_SHOW);

        ShowWindow(GetDlgItem(Dialog,IDC_UA_TTNORMALT ),SW_SHOW);
        ShowWindow(GetDlgItem(Dialog,IDC_UA_TTPRESSEDT),SW_SHOW);
        ShowEditField(Dialog,IDC_UA_TTNORMALV ,SW_SHOW);
        ShowEditField(Dialog,IDC_UA_TTPRESSEDV,SW_SHOW);
      end;
      uaHotkey: begin
      end;
      uaMenu: begin
        if tMenuType(HiByte(atype))=contact_menu then
          ShowWindow(GetDlgItem(Dialog,IDC_UA_GLOBAL),SW_SHOW);

        ShowWindow(GetDlgItem(Dialog,IDC_UA_BLOCK     ),SW_SHOW);
        ShowWindow(GetDlgItem(Dialog,IDC_UA_MENU      ),SW_SHOW);

//!!        ShowWindow(GetDlgItem(Dialog,IDC_UA_SEPARATE  ),SW_SHOW);
//!!        ShowWindow(GetDlgItem(Dialog,IDC_UA_SEPARATE  ),SW_SHOW);

        ShowWindow(GetDlgItem(Dialog,IDC_UA_SHOWVART  ),SW_SHOW);
        ShowEditField(Dialog,IDC_UA_SHOWVARV  ,SW_SHOW);

        ShowWindow(GetDlgItem(Dialog,IDC_UA_POPUPT    ),SW_SHOW);
        ShowWindow(GetDlgItem(Dialog,IDC_UA_VARNAMEST ),SW_SHOW);
        ShowEditField(Dialog,IDC_UA_VARNAMESV ,SW_SHOW);
        ShowEditField(Dialog,IDC_UA_POPUPV    ,SW_SHOW);

//        ShowWindow(GetDlgItem(Dialog,IDC_UA_NOTRANSLATE),SW_SHOW);
      end;
    end;
  end;
end;

procedure EnableDisableCheck(Dialog:HWND);
var
  wnd:HWND;
  item:integer;
  two,enable:boolean;
begin
  wnd:=GetDlgItem(Dialog,IDC_UA_PLACELIST);
  item:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
  enable:=ListView_GetCheckState(wnd,item)<>0;

  // common settings
  two:=IsDlgButtonChecked(Dialog,IDC_UA_TWOSTATE)<>BST_UNCHECKED;
  EnableWindow(GetDlgItem(Dialog,IDC_UA_SAVSTATE),two);

  // all others
  EnableWindow(GetDlgItem(Dialog,IDC_UA_BLOCK      ),enable);
  EnableWindow(GetDlgItem(Dialog,IDC_UA_MENU       ),enable);

  EnableWindow(GetDlgItem(Dialog,IDC_UA_NOTRANSLATE),enable);
  EnableWindow(GetDlgItem(Dialog,IDC_UA_SEPARATE   ),enable);
  EnableWindow(GetDlgItem(Dialog,IDC_UA_POPUPT     ),enable);
  EnableWindow(GetDlgItem(Dialog,IDC_UA_VARNAMEST  ),enable);

  EnableWindow(GetDlgItem(Dialog,IDC_UA_TTNORMALT  ),enable);
  EnableWindow(GetDlgItem(Dialog,IDC_UA_TTPRESSEDT ),enable);

  EnableWindow(GetDlgItem(Dialog,IDC_UA_SHOWVART   ),enable);

  EnableEditField(Dialog,IDC_UA_SHOWVARV  ,enable);
  EnableEditField(Dialog,IDC_UA_VARNAMESV ,enable);
  EnableEditField(Dialog,IDC_UA_POPUPV    ,enable);
  EnableEditField(Dialog,IDC_UA_TTNORMALV ,enable);

  EnableEditField(Dialog,IDC_UA_TTPRESSEDV,enable and two);
end;

// Clear all screen buttons/text fields (reset)
procedure ClearControls(Dialog:HWND);
var
  s:HWND;
begin
  s:=settings;
  settings:=0;
  // common
  CheckDlgButton (Dialog,IDC_UA_TWOSTATE,BST_UNCHECKED);
  CheckDlgButton (Dialog,IDC_UA_SAVSTATE,BST_UNCHECKED);

  // menu items
  SetDlgItemTextW(Dialog,IDC_UA_BLOCK      ,'>');
  CheckDlgButton (Dialog,IDC_UA_SEPARATE   ,BST_UNCHECKED);
  CheckDlgButton (Dialog,IDC_UA_NOTRANSLATE,BST_UNCHECKED);
  SetDlgItemTextW(Dialog,IDC_UA_POPUPV     ,nil);
  SetDlgItemTextW(Dialog,IDC_UA_VARNAMESV  ,nil);

  SetDlgItemTextW(Dialog,IDC_UA_SHOWVARV   ,nil);
  // buttons
  SetDlgItemTextW(Dialog,IDC_UA_TTNORMALV ,nil);
  SetDlgItemTextW(Dialog,IDC_UA_TTPRESSEDV,nil);

  // contact related
  CheckDlgButton (Dialog,IDC_UA_GLOBAL,BST_UNCHECKED);

  settings:=s;
end;

procedure InitActionSettings(Dialog:HWND);
begin
  MakeEditField(Dialog,IDC_UA_TTNORMALV);
  MakeEditField(Dialog,IDC_UA_TTPRESSEDV);

  MakeEditField(Dialog,IDC_UA_POPUPV);
  MakeEditField(Dialog,IDC_UA_VARNAMESV);

  MakeEditField(Dialog,IDC_UA_SHOWVARV);
  SetEditFlags(GetDlgItem(Dialog,IDC_UA_SHOWVARV),EF_FORCE,EF_FORCES);
end;

function CompareItem(lParam1,lParam2:LPARAM;SortType:LPARAM):int; stdcall;
begin
  result:=UActionList[lParam1].wSortIndex-UActionList[lParam2].wSortIndex;
end;

procedure FillActionList(wnd:HWND);
var
  i:integer;
  li:LV_ITEMW;
  il:HIMAGELIST;
  lmenu:tMenuType;
begin
  wnd:=GetDlgItem(wnd,IDC_UA_ACTIONLIST);
  SendMessage(wnd,LVM_DELETEALLITEMS,0,0);

  il:=ImageList_Create(16,16,ILC_COLOR32 or ILC_MASK,0,1);
  for i:=0 to HIGH(UActionList) do
  begin   
    li.mask    :=LVIF_TEXT+LVIF_PARAM+LVIF_IMAGE;
    li.iSubItem:=0;
    li.iItem   :=i;
    li.lParam  :=i;
    li.pszText :=UActionList[i].szActDescr;
    li.iImage:=ImageList_ReplaceIcon(il, -1,
        HICON(CallService(MS_SKIN2_GETICONBYHANDLE,0,LPARAM(UActionList[i].hIcolibIcon))));
    li.iItem   :=SendMessageW(wnd,LVM_INSERTITEMW,0,LPARAM(@li));

    for lmenu:=main_menu to HIGH(tMenuType) do
      UActionList[i].UAMenuItem[lmenu].changed:=false;
  end;
  ImageList_Destroy(SendMessage(wnd,LVM_SETIMAGELIST,LVSIL_SMALL,il));

  SendMessage(wnd,LVM_SORTITEMS,0,LPARAM(@CompareItem));

  ListView_SetItemState(wnd,0,
    LVIS_FOCUSED or LVIS_SELECTED,
    LVIS_FOCUSED or LVIS_SELECTED);
end;

// refresh icons in UA list (at least)
function IconChanged(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  i:integer;
  li:LV_ITEMW;
  il:HIMAGELIST;
  wnd:HWND;
begin
  result:=0;
  wnd:=GetDlgItem(settings,IDC_UA_ACTIONLIST);

  il:=ImageList_Create(16,16,ILC_COLOR32 or ILC_MASK,0,1);
  for i:=0 to HIGH(UActionList) do
  begin
    li.mask    :=LVIF_IMAGE;
    li.iSubItem:=0;
    li.iItem   :=i;
    li.iImage:=ImageList_ReplaceIcon(il, -1,
        HICON(CallService(MS_SKIN2_GETICONBYHANDLE,0,TLPARAM(UActionList[i].hIcolibIcon))));
    SendMessageW(wnd,LVM_SETITEM,0,TLPARAM(@li));
  end;
  ImageList_Destroy(SendMessage(wnd,LVM_SETIMAGELIST,LVSIL_SMALL,il));
//!!refresh?
end;

//----- Dynamic (option-depended) view -----

procedure ShowSubAction(Dialog:HWND;aType:integer;item:integer=-1);
var
  UA:pMyActionItem;
  s:HWND;
  i:integer;
begin
  s:=settings;
  settings:=0;
  ClearControls(Dialog);

  // get UAction number
  item:=LV_GetLParam(GetDlgItem(Dialog,IDC_UA_ACTIONLIST),item);

  UA:=@UActionList[item];

  // common settings
  if (UA.flags and UAF_2STATE)<>0 then
    CheckDlgButton(Dialog,IDC_UA_TWOSTATE,BST_CHECKED);

  if (UA.flags and UAF_SAVESTATE)<>0 then
    CheckDlgButton(Dialog,IDC_UA_SAVSTATE,BST_CHECKED);

  if (UA.flags and UAF_GLOBAL)=0 then
    CheckDlgButton(Dialog,IDC_UA_GLOBAL,BST_CHECKED);

  // Show real UA settings
  case LoByte(aType) of
    uaTTB: begin // CList modern toolbar
      SetDlgItemTextA(Dialog,IDC_UA_TTNORMALV ,UA.szTTBTooltip);
      SetDlgItemTextA(Dialog,IDC_UA_TTPRESSEDV,UA.szTTBTooltipPressed);
      SetDlgItemTextW(Dialog,IDC_UA_SHOWVARV  ,UA.szTTBShowWhenVars);

      if (UA.flags and UAF_TTBTTUV)<>0 then i:=EF_SCRIPT else i:=0;
      SetEditFlags(Dialog,IDC_UA_TTNORMALV ,EF_ALL,i);
      if (UA.flags and UAF_TTBTTPV)<>0 then i:=EF_SCRIPT else i:=0;
      SetEditFlags(Dialog,IDC_UA_TTPRESSEDV,EF_ALL,i);
    end;

    uaTAB: begin // TabSRMM toolbar
      SetDlgItemTextW(Dialog,IDC_UA_TTNORMALV ,UA.szTabBTooltip);
      SetDlgItemTextW(Dialog,IDC_UA_TTPRESSEDV,UA.szTabBTooltipPressed);
    end;

    uaMenu: begin
      with UA.UAMenuItem[tMenuType(HiByte(aType))] do
      begin
        if (menu_opt and UAF_MENUSEP)<>0 then
          CheckDlgButton(Dialog,IDC_UA_SEPARATE,BST_CHECKED);
        if (menu_opt and UAF_NOTRANS)<>0 then
          CheckDlgButton(Dialog,IDC_UA_NOTRANSLATE,BST_CHECKED);
        SetDlgItemTextW(Dialog,IDC_UA_POPUPV   ,szMenuPopup);
        SetDlgItemTextW(Dialog,IDC_UA_VARNAMESV,szMenuNameVars);
        SetDlgItemTextW(Dialog,IDC_UA_SHOWVARV ,szMenuShowWhenVars);
        if (menu_opt and UAF_MNAMVAR)<>0 then i:=EF_SCRIPT else i:=0;
        SetEditFlags(Dialog,IDC_UA_VARNAMESV,EF_ALL,i);
        if (menu_opt and UAF_MSUBVAR)<>0 then i:=EF_SCRIPT else i:=0;
        SetEditFlags(Dialog,IDC_UA_POPUPV,EF_ALL,i);
      end;
    end;

    uaHotkey: begin // Hotkey
      // Settings in Customize/Hotkeys
    end;
  end;
  ShowHideControls(Dialog,aType,-1);
  EnableDisableCheck(Dialog);
  settings:=s;
end;

function isPlaceActive(idx,place:integer):boolean;
begin
  result:=false;
  with UActionList[idx] do
    case LoByte(place) of
      uaTTB    : result:=(flags and UAF_REGTTBB)<>0;
      uaTAB    : result:=(flags and UAF_REGTABB)<>0;
      uaHotkey : result:=(flags and UAF_REGHOTKEY)<>0;
      uaMenu: begin
        result:=(UAMenuItem[tMenuType(HiByte(place))].menu_opt and UAF_MENUUSE)<>0
      end;
    end;
end;

procedure ShowAction(Dialog:HWND;item:integer=-1);
var
  i,j:integer;
  wnd:HWND;
  li:LV_ITEMW;
  buf:array [0..255] of WideChar;
  lset:HWND;
begin
  wnd:=GetDlgItem(Dialog,IDC_UA_PLACELIST);
  SendMessage(wnd,LVM_DELETEALLITEMS,0,0);
  j:=LV_GetLParam(GetDlgItem(Dialog,IDC_UA_ACTIONLIST),item);
  if j>=0 then
  begin
    with UActionList[j] do
    begin
      lset:=settings;
      settings:=0;
      // make "places" list
      for i:=0 to NumTypes-1 do
      begin
        with NamesArray[i] do
        begin
          if enable then // cached ability flag
          begin
            li.mask    :=LVIF_TEXT+LVIF_PARAM;
            li.iSubItem:=0;
            li.iItem   :=i;
            li.lParam  :=atype; //!!!!!! need to add subtype
            li.pszText :=TranslateW(FastAnsiToWideBuf(name,buf));
            li.iItem   :=SendMessageW(wnd,LVM_INSERTITEMW,0,LPARAM(@li));

            ListView_SetCheckState(wnd,li.iItem,isPlaceActive(j,atype));
          end;
        end;
      end;
      ListView_SetItemState(wnd,0,
        LVIS_FOCUSED or LVIS_SELECTED,
        LVIS_FOCUSED or LVIS_SELECTED);
      // show first selected "place"
      ShowSubAction(Dialog,LV_GetLParam(wnd));
      settings:=lset;
    end;
  end
  else
  begin
    ClearControls(Dialog);
    ShowHideControls(Dialog,-1,-1);
  end;
end;

//----- Saving -----

procedure SetChangedFlag(Dialog:HWND);
var
  num,atype:integer;
begin
  num  :=LV_GetLParam(GetDlgItem(Dialog,IDC_UA_ACTIONLIST),-1);
  atype:=LV_GetLParam(GetDlgItem(Dialog,IDC_UA_PLACELIST ),-1);
  UActionList[num].UAMenuItem[tMenuType(HiByte(atype))].changed:=true;
end;

procedure SaveMenuSubAction(Dialog:HWND;var MenuItem:tUAMenuItem);
begin
  with MenuItem do
  begin
    mFreeMem(szMenuPopup       ); szMenuPopup       :=GetDlgText(Dialog,IDC_UA_POPUPV);
    mFreeMem(szMenuNameVars    ); szMenuNameVars    :=GetDlgText(Dialog,IDC_UA_VARNAMESV);
    mFreeMem(szMenuShowWhenVars); szMenuShowWhenVars:=GetDlgText(Dialog,IDC_UA_SHOWVARV);
    menu_opt:=0;
    if IsDlgButtonchecked(Dialog,IDC_UA_NOTRANSLATE)<>BST_UNCHECKED then
      menu_opt:=menu_opt or UAF_NOTRANS;
    if IsDlgButtonchecked(Dialog,IDC_UA_SEPARATE)<>BST_UNCHECKED then
      menu_opt:=menu_opt or UAF_MENUSEP;
    if (GetEditFlags(Dialog,IDC_UA_VARNAMESV) and EF_SCRIPT)<>0 then
      menu_opt:=menu_opt or UAF_MNAMVAR;
    if (GetEditFlags(Dialog,IDC_UA_POPUPV) and EF_SCRIPT)<>0 then
      menu_opt:=menu_opt or UAF_MSUBVAR;
  end;
end;

procedure SetPlaceActive(idx,place:integer;active:boolean);
begin
  with UActionList[idx] do
    case LoByte(place) of
      uaTTB   : if active then flags:=flags or UAF_REGTTBB   else flags:=flags and not UAF_REGTTBB;
      uaTAB   : if active then flags:=flags or UAF_REGTABB   else flags:=flags and not UAF_REGTABB;
      uaHotkey: if active then flags:=flags or UAF_REGHOTKEY else flags:=flags and not UAF_REGHOTKEY;
      uaMenu  :
        with UAMenuItem[tMenuType(HiByte(place))] do
         if active then menu_opt:=menu_opt or UAF_MENUUSE
                   else menu_opt:=menu_opt and not UAF_MENUUSE;
    end;
end;

procedure SaveAction(Dialog:HWND;item:integer=-1;atype:integer=-1);
var
  i,num:integer;
  wnd:HWND;
begin
  num:=LV_GetLParam(GetDlgItem(Dialog,IDC_UA_ACTIONLIST),item);
  if num<0 then exit;

  wnd:=GetDlgItem(Dialog,IDC_UA_PLACELIST);
  atype:=LV_GetLParam(wnd,atype);
  with UActionList[num] do
  begin
    // main flags
    flags:=flags and not UAF_USING;
    // common section
    if IsDlgButtonChecked(Dialog,IDC_UA_TWOSTATE)<>BST_UNCHECKED then
      flags:=flags or UAF_2STATE
    else
      flags:=flags and not UAF_2STATE;

    if IsDlgButtonChecked(Dialog,IDC_UA_SAVSTATE)<>BST_UNCHECKED then
      flags:=flags or UAF_SAVESTATE
    else
      flags:=flags and not UAF_SAVESTATE;

    if IsDlgButtonChecked(Dialog,IDC_UA_GLOBAL)=BST_UNCHECKED then
      flags:=flags or UAF_GLOBAL
    else
      flags:=flags and not UAF_GLOBAL;

    // custom data
    case LoByte(atype) of
      uaTTB: begin // CList modern toolbar
        mFreeMem(szTTBTooltip       ); szTTBTooltip       :=GetDlgText(Dialog,IDC_UA_TTNORMALV ,true);
        mFreeMem(szTTBTooltipPressed); szTTBTooltipPressed:=GetDlgText(Dialog,IDC_UA_TTPRESSEDV,true);
        mFreeMem(szTTBShowWhenVars  ); szTTBShowWhenVars  :=GetDlgText(Dialog,IDC_UA_SHOWVARV);
        if (GetEditFlags(Dialog,IDC_UA_TTNORMALV ) and EF_SCRIPT)<>0 then
          flags:=flags or UAF_TTBTTUV;
        if (GetEditFlags(Dialog,IDC_UA_TTPRESSEDV) and EF_SCRIPT)<>0 then
          flags:=flags or UAF_TTBTTPV;
      end;

      uaTAB: begin // TabSRMM toolbar
        mFreeMem(szTabBTooltip       ); szTabBTooltip       :=GetDlgText(Dialog,IDC_UA_TTNORMALV);
        mFreeMem(szTabBTooltipPressed); szTabBTooltipPressed:=GetDlgText(Dialog,IDC_UA_TTPRESSEDV);
      end;

      uaMenu: SaveMenuSubAction(Dialog,UAMenuItem[tMenuType(HiByte(atype))]);

      uaHotkey: begin // Hotkey
        // Settings in Customize/Hotkeys
      end;
    end;
    for i:=0 to SendMessage(wnd,LVM_GETITEMCOUNT,0,0)-1 do
    begin
      SetPlaceActive(num,LV_GetLParam(wnd,i),ListView_GetCheckState(wnd,i)<>0);
    end;
  //just after Action (not place) changes
  if item<0 then
    SaveUA(num);
  end;
end;

procedure ChangeBlock(Dialog:HWND);
var
  pc:pWideChar;
  sh_main,sh_add:integer;
begin
  if IsWindowVisible(GetDlgItem(Dialog,IDC_UA_POPUPV)) then
  begin
    pc:='<';
    sh_main:=SW_HIDE;
    sh_add :=SW_SHOW;
  end
  else
  begin
    pc:='>';
    sh_main:=SW_SHOW;
    sh_add :=SW_HIDE;
  end;
  ShowWindow(GetDlgItem(Dialog,IDC_UA_NOTRANSLATE),sh_add);
  ShowWindow(GetDlgItem(Dialog,IDC_UA_SEPARATE   ),sh_add);
  ShowWindow(GetDlgItem(Dialog,IDC_UA_POPUPT     ),sh_main);
  ShowWindow(GetDlgItem(Dialog,IDC_UA_VARNAMEST  ),sh_main);
  ShowEditField(Dialog,IDC_UA_VARNAMESV,sh_main);
  ShowEditField(Dialog,IDC_UA_POPUPV   ,sh_main);
  
  SetDlgItemTextW(Dialog,IDC_UA_BLOCK,pc);
  if sh_main=SW_SHOW then
    pc:='Main settings'
  else
    pc:='Custom settings';
  SetDlgItemTextW(Dialog,IDC_UA_MENU,TranslateW(pc));
end;

//----- Main Dialog -----

function DlgProcOpt(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  wnd:HWND;
  lv:LV_COLUMNW;
  i:integer;
  twostate:bool;
begin
  result:=0;
  case hMessage of
    WM_CLOSE: begin
      settings:=0;
      UnhookEvent(hIC);
    end;

    WM_INITDIALOG: begin
      settings:=0;
      TranslateDialogDefault(Dialog);

      InitActionSettings(Dialog);

      wnd:=GetDlgItem(Dialog,IDC_UA_PLACELIST);
      SendMessage(wnd,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_CHECKBOXES,LVS_EX_CHECKBOXES);
      SendMessage(wnd,LVM_SETUNICODEFORMAT,1,0);
      zeromemory(@lv,sizeof(lv));
      lv.mask:=LVCF_WIDTH;
      lv.cx  :=110;
      SendMessageW(wnd,LVM_INSERTCOLUMNW ,0,tlparam(@lv));
      SendMessageW(wnd,LVM_SETCOLUMNWIDTH,0,LVSCW_AUTOSIZE_USEHEADER);

      wnd:=GetDlgItem(Dialog,IDC_UA_ACTIONLIST);
//      SendMessage(wnd,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_CHECKBOXES,LVS_EX_CHECKBOXES);
      SendMessage(wnd,LVM_SETUNICODEFORMAT,1,0);
      zeromemory(@lv,sizeof(lv));
      lv.mask:=LVCF_WIDTH;
      lv.cx  :=110;
      SendMessageW(wnd,LVM_INSERTCOLUMNW ,0,tlparam(@lv));
      SendMessageW(wnd,LVM_SETCOLUMNWIDTH,0,LVSCW_AUTOSIZE_USEHEADER);
      FillActionList(Dialog);
      ShowAction(Dialog,-1);

      settings:=Dialog;
      hIC:=HookEvent(ME_SKIN2_ICONSCHANGED,@IconChanged);
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        EN_CHANGE: begin
          if settings<>0 then
          begin
            SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
            if loword(wParam)=IDC_UA_POPUPV then
              SetChangedFlag(Dialog);
          end;
        end;

        BN_CLICKED: begin
          if settings<>0 then
          begin
            case loword(wParam) of
              IDC_UA_TWOSTATE: begin
                twostate:=IsDlgButtonChecked(Dialog,IDC_UA_TWOSTATE)<>BST_UNCHECKED;
                EnableWindow(GetDlgItem(Dialog,IDC_UA_SAVSTATE),twostate);

                if LoByte(LV_GetLParam(GetDlgItem(Dialog,IDC_UA_PLACELIST))) in [uaTTB,uaTAB] then
                  EnableEditField(Dialog,IDC_UA_TTPRESSEDV,
                    twostate and IsWindowEnabled(GetDlgItem(Dialog,IDC_UA_TTNORMALV))
                  );

                ShowHideControls(Dialog,
                    LV_GetLParam(GetDlgItem(Dialog,IDC_UA_PLACELIST)),-1);

                if twostate then
                  AddIcolibIconP(
                      UActionList[LV_GetLParam(GetDlgItem(Dialog,IDC_UA_ACTIONLIST),-1)])
                else
                  DeleteIcolibIconP(
                      UActionList[LV_GetLParam(GetDlgItem(Dialog,IDC_UA_ACTIONLIST),-1)]);

                SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
              end;

//was commented. why?
              IDC_UA_SAVSTATE,
              IDC_UA_GLOBAL: SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);

              IDC_UA_BLOCK: begin
                ChangeBlock(Dialog);
              end;

              IDC_UA_NOTRANSLATE,
              IDC_UA_SEPARATE: begin
                SetChangedFlag(Dialog);
                SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
              end;
            end;
          end;
        end;
      end;
    end;

    WM_NOTIFY: begin
      case integer(PNMHdr(lParam)^.code) of
        PSN_APPLY: begin
          SaveAction(Dialog);
          SaveUAs;
          FillChar(arMenuRec[0],Length(arMenuRec)*SizeOf(tuaMenuRecA),0);
          for i:=0 to HIGH(UActionList) do
          begin
            SetAllActionUsers(UActionList[i],false);
          end;
        end;

        LVN_ITEMCHANGED: begin
          if settings=0 then exit;
          if PNMLISTVIEW(lParam)^.uChanged=LVIF_STATE then
          begin
            i:=(PNMLISTVIEW(lParam)^.uOldState and LVNI_FOCUSED)-
               (PNMLISTVIEW(lParam)^.uNewState and LVNI_FOCUSED);

            if i>0 then // old focus
            begin
              if wParam=IDC_UA_ACTIONLIST then
                SaveAction(Dialog,PNMLISTVIEW(lParam)^.iItem)
              else //if wParam=IDC_UA_PLACELIST then
                SaveAction(Dialog,-1,PNMLISTVIEW(lParam)^.iItem);
            end
            else if i<0 then // new focus
            begin
              if wParam=IDC_UA_ACTIONLIST then
                ShowAction(Dialog,PNMLISTVIEW(lParam)^.iItem)
              else//if wParam=IDC_UA_PLACELIST then
                ShowSubAction(Dialog,
                  LV_GetLParam(GetDlgItem(Dialog,IDC_UA_PLACELIST),
                               PNMLISTVIEW(lParam)^.iItem));
            end
            // checkboxes
            else if (settings<>0) and
               ((PNMLISTVIEW(lParam)^.uOldState or PNMLISTVIEW(lParam)^.uNewState)=$3000) then
            begin
              // which type
              wnd:=GetDlgItem(Dialog,IDC_UA_PLACELIST);
              if PNMLISTVIEW(lParam)^.iItem<>
                 SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED) then
                ListView_SetItemState(wnd,PNMLISTVIEW(lParam)^.iItem,
                    LVIS_FOCUSED or LVIS_SELECTED,
                    LVIS_FOCUSED or LVIS_SELECTED)
              else
                EnableDisableCheck(Dialog);

              SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
            end;
          end;
        end;
      end;
    end;
  end;
end;

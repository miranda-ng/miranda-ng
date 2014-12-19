{}
const
  settings:HWND = 0;
const
  NumControls = 17;

  IDsArray:array [0..NumControls-1] of integer =(
    // Menu settings controls
    IDC_UA_SEPARATE ,IDC_UA_POPUPT   ,IDC_UA_POPUPV,
    IDC_UA_VARNAMEST,IDC_UA_VARNAMESV,IDC_UA_VARNAMESH,
    IDC_UA_SHOWVART ,IDC_UA_SHOWVARV ,IDC_UA_SHOWVARH,
    IDC_UA_TWOSTATE ,IDC_UA_SAVSTATE ,IDC_UA_COMMON,
    // toolbar settings controls
    IDC_UA_TTNORMALT,IDC_UA_TTNORMALV,IDC_UA_TTPRESSEDT,IDC_UA_TTPRESSEDV,
    IDC_UA_GLOBAL
    );

  // Show-hide controls by place type
  SHArray:array [0..NumTypes-1, 0..NumControls-1] of integer = (
    // CList Modern toolbar
    (SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE,SW_HIDE,SW_HIDE, SW_SHOW,SW_SHOW,SW_SHOW,
     SW_SHOW,SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW,SW_SHOW, SW_HIDE),
    // TabSRMM toolbar
    (SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE,SW_HIDE,SW_HIDE,
     SW_SHOW,SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW,SW_SHOW, SW_SHOW),
    // Core Hotkey
    (SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE,SW_HIDE,SW_HIDE,
     SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE),
    // Main menu
    (SW_SHOW,SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW,
     SW_SHOW,SW_SHOW,SW_SHOW, SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE),
    // Contact menu
    (SW_SHOW,SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW,
     SW_SHOW,SW_SHOW,SW_SHOW, SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_SHOW),
    // Tray menu
    (SW_SHOW,SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW,
     SW_SHOW,SW_SHOW,SW_SHOW, SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE),
    // Protocol menu
    (SW_SHOW,SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW,
     SW_SHOW,SW_SHOW,SW_SHOW, SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE),
    // Status menu
    (SW_SHOW,SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW,
     SW_SHOW,SW_SHOW,SW_SHOW, SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE)
    );
  // additional show/hide controls check by Variables installings (1 - need to check)
  SHVarArray:array [0..NumControls-1] of byte = (
    0,0,0,    0,0,1,    1,1,1,
    0,0,0,0,0,0,0, 0);
  // additional enable/disable controls check (1 - always enable)
  EnDisArray:array [0..NumControls-1] of byte = (
    0,0,0,    0,0,0,    0,0,0,
    1,1,1,0,0,0,0, 1);

var
  hIC:THANDLE;

procedure CheckPlacesAbility;
var
  i:integer;
begin
  for i:=0 to NumTypes-1 do
  begin
    with NamesArray[i] do
    begin
      enable:=(service=nil) or (ServiceExists(service)<>0);
    end;
  end;
end;

function CompareItem(lParam1,lParam2:LPARAM;SortType:LPARAM):int; stdcall;
begin
  result:=UActionList[lParam1].wSortIndex-UActionList[lParam2].wSortIndex;
end;

//  Show or hide option items
procedure SetupControls(Dialog:HWND;atype:integer;item:integer=-1);
var
  i: cardinal;
  typ:integer;
  wnd,wnd1:HWND;
  enable:boolean;
begin
  if atype<0 then
  begin
    for i:=0 to NumControls-1 do
    begin
      ShowWindow(GetDlgItem(Dialog,IDsArray[i]),SW_HIDE);
    end;
  end
  else
  begin
    wnd1:=GetDlgItem(Dialog,IDC_UA_PLACELIST);
    if item<0 then
      item:=SendMessage(wnd1,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
    enable:=ListView_GetCheckState(wnd1,item)<>0;

    for i:=0 to NumControls-1 do
    begin
      typ:=SHArray[LoByte(atype)+HiByte(atype)][i];
      if typ=SW_SHOW then
        if (SHVarArray[i]<>0) and (not IsVarsInstalled) then
          typ:=SW_HIDE;
      wnd:=GetDlgItem(Dialog,IDsArray[i]);
      ShowWindow(wnd,typ);
      EnableWindow(wnd,enable or (EnDisArray[i]<>0));
    end;

    // common settings
    EnableWindow(GetDlgItem(Dialog,IDC_UA_SAVSTATE),
        IsDlgButtonChecked(Dialog,IDC_UA_TWOSTATE)<>BST_UNCHECKED);

    // personal settings
    case LoByte(atype) of
      uaTTB, uaTAB: begin
        enable:=false;
        if IsDlgButtonChecked(Dialog,IDC_UA_TWOSTATE)<>BST_UNCHECKED then
          if IsWindowEnabled(GetDlgItem(Dialog,IDC_UA_TTNORMALV)) then
            enable:=true;
        EnableWindow(GetDlgItem(Dialog,IDC_UA_TTPRESSEDV),enable);
      end;
    end;
  end;
end;

// Clear all screen buttons/text fields (reset)
procedure ClearControls(Dialog:HWND);
var
  s:HWND;
begin
  s:=settings;
  settings:=0;
  CheckDlgButton (Dialog,IDC_UA_TWOSTATE ,BST_UNCHECKED);
  CheckDlgButton (Dialog,IDC_UA_SAVSTATE ,BST_UNCHECKED);

  CheckDlgButton (Dialog,IDC_UA_SEPARATE ,BST_UNCHECKED);
  SetDlgItemTextW(Dialog,IDC_UA_POPUPV   ,nil);
  SetDlgItemTextW(Dialog,IDC_UA_VARNAMESV,nil);
  SetDlgItemTextW(Dialog,IDC_UA_SHOWVARV ,nil);
  settings:=s;
end;

procedure ShowSubAction(Dialog:HWND;aType:integer;item:integer=-1);
var
  UA:pMyActionItem;
  s:HWND;
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
        SetDlgItemTextW(Dialog,IDC_UA_POPUPV   ,szMenuPopup);
        SetDlgItemTextW(Dialog,IDC_UA_VARNAMESV,szMenuNameVars);
        SetDlgItemTextW(Dialog,IDC_UA_SHOWVARV ,szMenuShowWhenVars);
      end;
    end;

    uaHotkey: begin // Hotkey
      // Settings in Customize/Hotkeys
    end;
  end;
  SetupControls(Dialog,aType,-1);
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
    SetupControls(Dialog,-1,-1);
  end;
end;

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
    if IsDlgButtonchecked(Dialog,IDC_UA_SEPARATE)<>BST_UNCHECKED then
      menu_opt:=menu_opt or UAF_MENUSEP;
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
    li.iImage:=ImageList_AddIcon(il,
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
    li.iImage:=ImageList_AddIcon(il,
        HICON(CallService(MS_SKIN2_GETICONBYHANDLE,0,TLPARAM(UActionList[i].hIcolibIcon))));
    SendMessageW(wnd,LVM_SETITEM,0,TLPARAM(@li));
  end;
  ImageList_Destroy(SendMessage(wnd,LVM_SETIMAGELIST,LVSIL_SMALL,il));
//!!refresh?
end;

function DlgProcOpt(Dialog:HWnd;hMessage:UINT;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
var
  wnd:HWND;
  lv:LV_COLUMNW;
  i:integer;
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

//      if isVarsInstalled then
      begin
        SendDlgItemMessage(Dialog,IDC_UA_VARNAMESH,BM_SETIMAGE,IMAGE_ICON,
          CallService(MS_VARS_GETSKINITEM,0,VSI_HELPICON));
        SendDlgItemMessage(Dialog,IDC_UA_SHOWVARH,BM_SETIMAGE,IMAGE_ICON,
          CallService(MS_VARS_GETSKINITEM,0,VSI_HELPICON));
//        SendDlgItemMessage(Dialog,IDC_UA_VARNAMESH,BUTTONSETASFLATBTN,0,0);
//        SendDlgItemMessage(Dialog,IDC_UA_SHOWVARH ,BUTTONSETASFLATBTN,0,0);
      end;
      
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
                SetupControls(Dialog,
                    LV_GetLParam(GetDlgItem(Dialog,IDC_UA_PLACELIST)),-1);
  {
                EnableWindow(GetDlgItem(Dialog,IDC_UA_TTPRESSEDV),
                  IsDlgButtonChecked(Dialog,IDC_UA_TWOSTATE)<>BST_UNCHECKED);
  }
                if IsDlgButtonChecked(Dialog,IDC_UA_TWOSTATE)=BST_UNCHECKED then
                  DeleteIcolibIconP(
                      UActionList[LV_GetLParam(GetDlgItem(Dialog,IDC_UA_ACTIONLIST),-1)])
                else
                  AddIcolibIconP(
                      UActionList[LV_GetLParam(GetDlgItem(Dialog,IDC_UA_ACTIONLIST),-1)]);

                SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
              end;

              IDC_UA_VARNAMESH: ShowVarHelp(Dialog,IDC_UA_VARNAMESV);
              IDC_UA_SHOWVARH : ShowVarHelp(Dialog,IDC_UA_SHOWVARV);

              IDC_UA_SAVSTATE,
              IDC_UA_GLOBAL: SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);

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
                SetupControls(Dialog,LV_GetLParam(wnd,PNMLISTVIEW(lParam)^.iItem),
                     PNMLISTVIEW(lParam)^.iItem);

              SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
            end;
          end;
        end;
      end;
    end;
  end;
end;

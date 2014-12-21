{}

const
  inoutfilter:pWideChar = 'XML files'#0'*.xml'#0'All files'#0'*.*'#0#0;
const
  MaxDescrLen = 128;
const
  hlpVariables    = 30;
  hlpAdvVariables = 31;

var
  DontReact:bool;
  OldGroupTableProc,
  OldActTableProc:pointer;
  EventMask:dword;

procedure ChangeListNotify(list:integer);
var
  p:pActModule;
begin
  p:=ModuleLink;
  while p<>nil do
  begin
    SendMessage(p^.DlgHandle,WM_ACT_LISTCHANGE,list,0);
    p:=p^.Next;
  end;
end;

procedure ResetDialogs;
var
  p:pActModule;
begin
  p:=ModuleLink;
  while p<>nil do
  begin
    SendMessage(p^.DlgHandle,WM_ACT_RESET,0,0);
    p:=p^.Next;
  end;
end;

procedure ClearDialogData;
var
  p:pActModule;
begin
  p:=ModuleLink;
  while p<>nil do
  begin
    if p^.DlgHandle<>0 then ShowWindow(p^.DlgHandle,SW_HIDE);
    p:=p^.Next;
  end;
end;

function CheckChanges:dword;
var
  i,j:integer;
  found:boolean;
begin
  result:=0;
  // 1 - search new items (EditMacroList elements which don't exists in MacroList)
  found:=true;
  for i:=0 to EditMacroList.Count-1 do
  begin
    if (EditMacroList[i]^.flags and ACF_ASSIGNED)<>0 then
    begin
      found:=false;
      for j:=0 to MacroList.Count-1 do
      begin
        if (MacroList[j]^.flags and ACF_ASSIGNED)<>0 then
        begin
          if EditMacroList[i]^.id=MacroList[j]^.id then
          begin
            found:=true;
            break;
          end;
        end;
      end;
      if not found then break;
    end;
  end;
  if not found then
    result:=result or ACTM_NEW;
  // 2 - search deleted items (MacroList elements which don't exists in EditMacroList)
  found:=true;
  for i:=0 to MacroList.Count-1 do
  begin
    if (MacroList[i]^.flags and ACF_ASSIGNED)<>0 then
    begin
      found:=false;
      for j:=0 to EditMacroList.Count-1 do
      begin
        if (EditMacroList[j]^.flags and ACF_ASSIGNED)<>0 then
        begin
          if MacroList[i]^.id=EditMacroList[j]^.id then
          begin
            found:=true;
            break;
          end;
        end;
      end;
      if not found then break;
    end;
  end;
  if not found then
    result:=result or ACTM_DELETE;
  // 3 - resort? (maybe ANY EditMacroList position changes)
end;

procedure SetChanged(wnd:HWND;atype:integer);
begin
  SendMessage(GetParent(wnd),PSM_CHANGED,0,0);
//  if ((atype and etMacro)<>0) and (NewGroupList =GroupList ) then NewGroupList :=CloneGroupList;
//  if ((atype and etACT  )<>0) and (NewActionList=ActionList) then NewActionList:=CloneActionList;
end;

procedure SetStart;
begin
  EditMacroList:=MacroList.Clone();
end;

// Cancel button pressed OR Edit window closed (After apply action)
// No main list changed (except flags clearance - not necessary)
// Edit macro list freeing with actions marked as introduced
procedure SetCancel;
var
  i,j:integer;
begin
  // Delete changed data
  EditMacroList.Clear(ACF_INTRODUCED);
  EditMacroList.Free;

  // Clear editing flags
  for i:=0 to MacroList.Count-1 do
  begin
    with MacroList[i]^ do
      if (flags and ACF_ASSIGNED)<>0 then
      begin
        for j:=0 to ActionCount-1 do
        begin
          ActionList^[j].flags:=ActionList^[j].flags and not ACF_REPLACED;
        end;
      end;
  end;
end;

// SAVE: fill edit macro list by dialog options,
// Free main macro list with actions marked as replaced
// copy content to main macro list (state as at option editing start)
// clear "introduced" and "replaced" flags
procedure SetSave(Dialog:HWND);
var
  TmpMacroList:tMacroList;
//  TmpMacro:pMacroRecord;
  wnd:HWND;
  li:LV_ITEMW;
  i,j:integer;
  TmpMacroCount:integer;
begin
  wnd:=MacroListWindow;
  TmpMacroCount:=SendMessageW(wnd,LVM_GETITEMCOUNT,0,0);

  // need to clear flags and resort
  if TmpMacroCount>0 then
  begin
    // Clear actions editing flags
    for i:=0 to EditMacroList.Count-1 do
    begin
      with EditMacroList[i]^ do
        if (flags and ACF_ASSIGNED)<>0 then
        begin
          for j:=0 to ActionCount-1 do
          begin
            ActionList^[j].flags:=ActionList^[j].flags and not ACF_INTRODUCED;
          end;
        end;
    end;

    TmpMacroList:=tMacroList.Create(TmpMacroCount);

    // resort/copy macros
    li.mask    :=LVIF_PARAM;
    li.iSubItem:=0;
    for i:=0 to TmpMacroCount-1 do
    begin
      li.iItem:=i;
      SendMessageW(wnd,LVM_GETITEMW,0,lparam(@li)); // GetLParam(wnd,i);
      move(EditMacroList[loword(li.lParam)]^,TmpMacroList[i]^,SizeOf(tMacroRecord));

      // macro flags
      TmpMacroList[i]^.flags:=
          (TmpMacroList[i]^.flags and not ACF_MACROFLAG) or
          (li.lParam and ACF_MACROFLAG);
{!!
      TmpMacro:=TmpMacroList[i];
      if (li.lParam and ACF_FIRSTRUN)<>0 then
        TmpMacro^.flags:=TmpMacro^.flags or ACF_FIRSTRUN
      else
        TmpMacro^.flags:=TmpMacro^.flags and not ACF_FIRSTRUN;

      if (li.lParam and ACF_SINGLEINST)<>0 then
        TmpMacro^.flags:=TmpMacro^.flags or ACF_SINGLEINST
      else
        TmpMacro^.flags:=TmpMacro^.flags and not ACF_SINGLEINST;
!!}
      li.lParam:=(li.lParam and (not $FFFF)) or i;
      SendMessageW(wnd,LVM_SETITEMW,0,lparam(@li));
    end;

    EditMacroList.Free;
    EditMacroList:=TmpMacroList;
  end;

  MacroList.Clear(ACF_REPLACED);
  MacroList.Free;
  MacroList:=EditMacroList.Clone;
end;

// Fill action type combobox
function FillActTypeList(list:HWND):HWND;
var
  cbei:TCOMBOBOXEXITEMW;
  il:HIMAGELIST;
  i:integer;
  buf:array [0..127] of WideChar;
  p:pActModule;
  rc:TRECT;
  dlg:HWND;
begin
  il:=ImageList_Create(16,16,ILC_COLOR32 or ILC_MASK,0,1);
  SendMessage(list,CB_RESETCONTENT,0,0);

  FillChar(cbei,SizeOf(cbei),0);
  cbei.mask:=CBEIF_IMAGE or CBEIF_SELECTEDIMAGE or CBEIF_TEXT or CBEIF_LPARAM;
  p:=ModuleLink;
  i:=0;
  // initial action dialogs position
  SetRect(rc,134,22,0,0);
  dlg:=GetParent(list);
  MapDialogRect(dlg,rc);
  while p<>nil do
  begin
    StrCopy(StrCopyE(@buf,IcoLibPrefix),p^.Name);
    ImageList_ReplaceIcon(il,-1,CallService(MS_SKIN2_GETICON,0,lparam(@buf)));

    cbei.pszText       :=TranslateW(FastAnsiToWideBuf(p^.Name,buf));
    cbei.iItem         :=i;
    cbei.lParam        :=p^.Hash;
    cbei.iImage        :=i;
    cbei.iSelectedImage:=i;
    if SendMessageW(list,CBEM_INSERTITEMW,0,lparam(@cbei))=-1 then
      break;
    p^.DlgHandle:=p^.Dialog(dlg);
    SetWindowPos(p^.DlgHandle,0,rc.left,rc.top,0,0,SWP_NOZORDER or SWP_NOSIZE or SWP_HIDEWINDOW);
    p:=p^.Next;
    inc(i);
  end;
  ImageList_Destroy(SendMessage(list,CBEM_SETIMAGELIST,0,il));

  SendMessage(list,CB_SETCURSEL,0,0);
  result:=GetLink(CB_GetData(list,0))^.DlgHandle;
end;

//----- Action list stuff -----

procedure SHActButtons(Dialog:HWND;mode:integer);
begin
  ShowWindow(GetDlgItem(Dialog,IDC_STAT_ACTION),mode);
  ShowWindow(GetDlgItem(Dialog,IDC_ACTION_TYPE),mode);
  if mode=SW_HIDE then
    ClearDialogData;
end;

procedure CheckActListDirectionButtons(Dialog:HWND);
var
  wnd:HWND;
  dir:integer;
  okup,okdown:boolean;
begin
  wnd:=ActionListWindow;

  dir:=LV_CheckDirection(wnd);
  okup  :=odd(loword(dir));
  okdown:=(loword(dir) and 2)<>0;

  EnableWindow(GetDlgItem(Dialog,IDC_ACTION_UP  ),okup);
  EnableWindow(GetDlgItem(Dialog,IDC_ACTION_DOWN),okdown);
  SendMessage(wnd,LVM_ENSUREVISIBLE,hiword(dir)-1,0);
end;

procedure CheckActListButtons(Dialog:HWND);
var
  mode:integer;
  wnd:HWND;
  b:boolean;
begin
  wnd:=ActionListWindow;
  b:=SendMessage(wnd,LVM_GETITEMCOUNT,0,0)>0;
  if b then
{
    ListView_SetItemState(wnd,next,
        LVIS_FOCUSED or LVIS_SELECTED,
        LVIS_FOCUSED or LVIS_SELECTED);
}
    mode:=SW_SHOW
  else
    mode:=SW_HIDE;

  SHActButtons(Dialog,mode);

  EnableWindow(wnd,b);
  EnableWindow(GetDlgItem(Dialog,IDC_ACTION_HELP  ),b);
  EnableWindow(GetDlgItem(Dialog,IDC_GROUP_TEST   ),b);
  EnableWindow(GetDlgItem(Dialog,IDC_ACTION_DELETE),b);
  EnableWindow(GetDlgItem(Dialog,IDC_ACTION_TYPE  ),b);

  CheckActListDirectionButtons(Dialog);
end;

// Fill dialog fields by choosen action
procedure FillAction(Dialog:HWND;act:tBaseAction);
var
  p:pActModule;
  i:LRESULT;
  wnd:HWND;
begin
  if act<>nil then
  begin
    p:=GetLink(act.UID);
    if p<>nil then
      SendMessage(p^.DlgHandle,WM_ACT_SETVALUE,0,tlparam(act));
  end
  // for newly added and activated action
  else
    p:=ModuleLink;

  if p<>nil then
  begin
    wnd:=GetDlgItem(Dialog,IDC_ACTION_TYPE);
    i:=CB_SelectData(wnd,tlparam(p^.Hash));
    ShowWindow(p^.DlgHandle,SW_SHOW);
    // Saving "normal" action type
    if p^.Hash>10 then
    begin
      SetWindowLongPtrW(wnd,GWLP_USERDATA,i);
    end;
  end;
end;

// Fill Chain list
procedure FillActionList(Dialog:HWND;Macro:pMacroRecord);
var
  idx:integer;
  wnd:HWND;
  li:LV_ITEMW;
begin
  wnd:=ActionListWindow;
  SendMessage(wnd,LVM_DELETEALLITEMS,0,0);

  if (Macro<>nil) and (Macro^.ActionCount>0) then
  begin
    li.mask    :=LVIF_TEXT or LVIF_PARAM;
    li.iSubitem:=0;
    idx:=0;
    while idx<Macro^.ActionCount do
    begin
      li.pszText:=Macro^.ActionList^[idx].ActionDescr;
      li.iItem  :=idx;
      li.lParam :=tlparam(Macro^.ActionList^[idx]);
      SendMessageW(wnd,LVM_INSERTITEMW,0,tlparam(@li));
      ListView_SetCheckState(wnd,idx,(Macro^.ActionList^[idx].flags and ACF_DISABLED)=0);
      inc(idx);
    end;

    Listview_SetItemState(wnd,0,
        LVIS_FOCUSED or LVIS_SELECTED,
        LVIS_FOCUSED or LVIS_SELECTED);

    FillAction(Dialog,Macro^.ActionList^[0]);
  end;
  CheckActListButtons(Dialog);
end;

// change current action name ONLY IN LIST
procedure ChangeActionName(Dialog:HWND; num:integer=-1;str:PWideChar=nil);
var
  li:LV_ITEMW;
  wnd:HWND;
  buf:array [0..127] of WideChar;
begin
  wnd:=ActionListWindow;
  if num<0 then
    li.iItem:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED)

  else
    li.iItem:=num;
  if li.iItem>=0 then
  begin
    li.iSubItem:=0;
    // getting SubAction number
    li.mask:=LVIF_PARAM;
    SendMessage(wnd,LVM_GETITEM,0,tlparam(@li));

    // changing to default name
    if str=nil then
    begin
      str:=StrCopyEW(@buf,TranslateW('Action'));
      str^:=' '; inc(str);
      IntToStr(str,SendMessage(wnd,LVM_GETITEMCOUNT,0,0));
      str:=@buf;
    end;

    li.mask   :=LVIF_TEXT;
    li.pszText:=str;
    SendMessageW(wnd,LVM_SETITEMW,0,tlparam(@li));
  end;
end;

procedure SaveAction(Dialog:HWND;listnum:integer;actptr:tBaseAction);
var
  wnd:HWND;
//  li:LV_ITEMW;
//  arr: array [0..255] of WideChar;
  p:pActModule;
begin
  wnd:=GetDlgItem(Dialog,IDC_ACTION_TYPE);
  p:=GetLink(CB_GetData(wnd));
  if p^.Hash<10 then
  begin
    p:=GetLink(CB_GetData(wnd,GetWindowLongPtrW(wnd,GWLP_USERDATA)));
  end;

  wnd:=ActionListWindow;

  if listnum<0 then
  begin
    listnum:=SendMessageW(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
    if listnum<0 then
      exit;
    actptr:=tBaseAction(LV_GetLParam(wnd,listnum));
  end;

  // changed existing action
  if actptr<>nil then
  begin
    if (actptr.flags and ACF_INTRODUCED)<>0 then // new but changed
      actptr.Free
    else // old
      actptr.flags:=actptr.flags or ACF_REPLACED;
  end;

  // Action saving
  actptr:=p.Create;
  SendMessage(p.DlgHandle,WM_ACT_SAVE,0,tlparam(actptr));
  actptr.flags:=actptr.flags or ACF_INTRODUCED;
{ maybe do it at full chain saving?
  
  li.iItem     :=listnum;
  li.mask      :=LVIF_TEXT;
  li.iSubItem  :=0;
  li.pszText   :=@arr;
  li.cchTextMax:=SizeOf(arr) div SizeOf(WideChar);
  SendDlgItemMessageW(Dialog,IDC_ACTION_LIST,LVM_GETITEMW,0,tlparam(@li));
  StrDupW(actptr.ActionDescr,arr);

  if ListView_GetCheckState(wnd,listnum)=0 then
    actptr.flags:=actptr.flags or ACF_DISABLED;
}
  LV_SetLParam(wnd,tlparam(actptr),listnum);
end;

procedure BuildActionChain(Dialog:HWND;group:integer=-1);
var
  item:integer;
  wnd:HWND;
  idx:integer;
  li:LV_ITEMW;
  arr: array [0..255] of WideChar;
begin
  idx:=smallint(loword(LV_GetLParam(MacroListWindow,group)));
  if idx>=0 then // we have macro to save
  begin
    SaveAction(Dialog,-1,nil);

    with EditMacroList[idx]^ do
    begin
      if ActionCount>0 then
      begin
        ActionCount:=0;
        FreeMem(ActionList);
        ActionList:=nil;
      end;
    end;

    wnd:=ActionListWindow;
    with EditMacroList[idx]^ do
    begin
      ActionCount:=SendMessageW(wnd,LVM_GETITEMCOUNT,0,0);
      if ActionCount>0 then // have some actions - need to build list
      begin
        // fix count for empty actions
        item:=0;
        idx:=0;
        while item<ActionCount do
        begin
          // if created but not changed action (fools proof)
          if LV_GetLParam(wnd,item)<>0 then
            Inc(idx);
          Inc(item);
        end;
        // if REAL amount to save>0 (macro not empty)
        if idx>0 then
        begin
          GetMem(ActionList,SizeOf(tBaseAction)*idx);

          li.mask      :=LVIF_TEXT;
          li.iSubItem  :=0;
          li.pszText   :=@arr;
          li.cchTextMax:=SizeOf(arr) div SizeOf(WideChar);

          idx:=0;
          for item:=0 to ActionCount-1 do
          begin
            ActionList^[idx]:=tBaseAction(LV_GetLParam(wnd,item));
            if ActionList^[idx]<>nil then
            begin
              // set name and enabled/disabled flag
              li.iItem:=item;
              SendDlgItemMessageW(Dialog,IDC_ACTION_LIST,LVM_GETITEMW,0,tlparam(@li));
              mFreeMem(ActionList^[idx].ActionDescr);
              StrDupW(ActionList^[idx].ActionDescr,arr);

              if ListView_GetCheckState(wnd,item)=0 then
                ActionList^[idx].flags:=ActionList^[item].flags or ACF_DISABLED
              else
                ActionList^[idx].flags:=ActionList^[item].flags and not ACF_DISABLED;
              inc(idx);
            end;
          end;
        end;
        ActionCount:=idx; // fix for empty actions
      end;
    end;
  end;
end;

// action (chain) table procedure (key hook)
function NewActTableProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  i:integer;
begin
  result:=0;
  case hMessage of
    WM_KEYDOWN: begin
      if (lParam and (1 shl 30))=0 then
      begin
        case wParam of
          VK_F2: begin
            i:=SendMessage(Dialog,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
            if i>=0 then
              PostMessageW(Dialog,LVM_EDITLABELW,i,0);
            exit;
          end;
          VK_UP: begin
            if (GetKeyState(VK_CONTROL) and $8000)<>0 then
            begin
              PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_ACTION_UP,0);
              exit;
            end;
          end;
          VK_DOWN: begin
            if (GetKeyState(VK_CONTROL) and $8000)<>0 then
            begin
              PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_ACTION_DOWN,0);
              exit;
            end;
          end;
          VK_INSERT: begin
            PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_ACTION_NEW,0);
            exit;
          end;
          VK_DELETE: begin
            PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_ACTION_DELETE,0);
            exit;
          end;
        end;
      end;
    end;
  end;
  result:=CallWindowProc(OldActTableProc,Dialog,hMessage,wParam,lParam);
end;

//----- Macro list stuff -----

// action group table procedure (key hook)
function NewGroupTableProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  i:integer;
begin
  result:=0;
  case hMessage of
    WM_KEYDOWN: begin
      if (lParam and (1 shl 30))=0 then
      begin
        case wParam of
          VK_F2: begin
            i:=SendMessage(Dialog,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
            if i>=0 then
              PostMessageW(Dialog,LVM_EDITLABELW,i,0);
            exit;
          end;
          VK_INSERT: begin
            PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_GROUP_NEW,0);
            exit;
          end;
          VK_DELETE: begin
            PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_GROUP_DELETE,0);
            exit;
          end;
          VK_UP: begin
            if (GetKeyState(VK_CONTROL) and $8000)<>0 then
            begin
              PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_GROUP_UP,0);
              exit;
            end;
          end;
          VK_DOWN: begin
            if (GetKeyState(VK_CONTROL) and $8000)<>0 then
            begin
              PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_GROUP_DOWN,0);
              exit;
            end;
          end;
        end;
      end;
    end;
  end;
  result:=CallWindowProc(OldGroupTableProc,Dialog,hMessage,wParam,lParam);
end;

function AddMacro(Dialog:HWND;MacroNum:cardinal):integer;
var
  li:LV_ITEMW;
  list:HWND;
begin
  with EditMacroList[MacroNum]^ do
  begin
    list:=MacroListWindow;
    li.mask     :=LVIF_PARAM+LVIF_TEXT;
    li.iItem    :=SendMessage(list,LVM_GETNEXTITEM,-1,LVNI_FOCUSED)+1;
    li.iSubItem :=0;
    li.lParam   :=MacroNum;
    li.pszText  :=@descr;
    li.iItem    :=SendMessageW(list,LVM_INSERTITEMW,0,lparam(@li));
    if li.iItem>0 then
      dec(li.iItem);
    ListView_SetItemState(list,li.iItem,
        LVIS_FOCUSED or LVIS_SELECTED,
        LVIS_FOCUSED or LVIS_SELECTED);
    result:=li.iItem;
  end;
end;

function MoveMacro(list:HWND;num:integer;incr:integer):integer;
var
  i,j:integer;
begin
  if num<0 then
  begin
    result:=-1;
    j:=SendMessage(list,LVM_GETITEMCOUNT,0,0)-1;
    if incr<0 then // up, from beginning
    begin
      for i:=0 to j do
      begin
        if SendMessage(list,LVM_GETITEMSTATE,i,LVIS_SELECTED)<>0 then
        begin
          if i=0 then break; // first selected is first already, nothing to move
          LV_MoveItem(list,incr,i);
          if result<0 then result:=i+incr;
        end;
      end;
    end
    else           // down, from the end
    begin
      for i:=j downto 0 do
      begin
        if SendMessage(list,LVM_GETITEMSTATE,i,LVIS_SELECTED)<>0 then
        begin
          if i=j then break; // last selected is last already, nothing to move
          LV_MoveItem(list,incr,i);
          if result<0 then result:=i+incr;
        end;
      end;
    end;
  end
  else
  begin
    LV_MoveItem(list,incr,num);
    result:=num;
  end;
end;

// enable/disable navigation chain buttons
procedure CheckMacroListDirectionButtons(Dialog:HWND);
var
  wnd:HWND;
  dir:integer;
  okup,okdown:boolean;
begin
  wnd:=MacroListWindow;
  dir:=LV_CheckDirection(wnd);
  okup  :=odd(loword(dir));
  okdown:=(loword(dir) and 2)<>0;

  EnableWindow(GetDlgItem(Dialog,IDC_GROUP_UP  ),okup);
  EnableWindow(GetDlgItem(Dialog,IDC_GROUP_DOWN),okdown);
  SendMessage(wnd,LVM_ENSUREVISIBLE,hiword(dir)-1,0);
end;

procedure CheckMacroList(Dialog:HWND);
var
  wnd:HWND;
  b:boolean;
begin
  wnd:=MacroListWindow;

  b:=SendMessage(wnd,LVM_GETITEMCOUNT,0,0)>0;
  if not b then
    CheckActListButtons(Dialog);

  EnableWindow(wnd,b);
  EnableWindow(GetDlgItem(Dialog,IDC_ACTION_NEW  ),b);
  EnableWindow(GetDlgItem(Dialog,IDC_GROUP_DELETE),b);
  EnableWindow(GetDlgItem(Dialog,IDC_GROUP_EXPORT),b);

  CheckMacroListDirectionButtons(Dialog);
end;

// Fill action group list and disable chain controls
function FillMacroList(Dialog:HWND):integer;
var
  CurMacro:pMacroRecord;
  i:integer;
  list:HWND;
  lvi:TLVITEMW;
begin
  SendDlgItemMessage(Dialog,IDC_ACTION_LIST,LVM_DELETEALLITEMS,0,0);
  CheckActListButtons(Dialog);

  list:=MacroListWindow;

  SendMessage(list,LVM_DELETEALLITEMS,0,0);
  result:=-1;
  if EditMacroList.Count>0 then
  begin
    CurMacro:=EditMacroList[0];
    lvi.mask:=LVIF_TEXT+LVIF_PARAM;
    lvi.iSubItem:=0;
    for i:=0 to EditMacroList.Count-1 do
    begin
      if (CurMacro^.flags and ACF_ASSIGNED)<>0 then
      begin
        lvi.iItem :=i;

        lvi.lParam:=LPARAM(i) or LPARAM(CurMacro^.flags and ACF_MACROFLAG);
{!!
        if (CurMacro^.flags and ACF_FIRSTRUN)<>0 then
          lvi.lParam:=lvi.lParam or ACF_FIRSTRUN;
        if (CurMacro^.flags and ACF_VOLATILE)<>0 then
          lvi.lParam:=lvi.lParam or ACF_VOLATILE;
        if (CurMacro^.flags and ACF_SINGLEINST)<>0 then
          lvi.lParam:=lvi.lParam or ACF_SINGLEINST;
!!}
        lvi.pszText:=@(CurMacro^.descr);
        SendMessageW(list,LVM_INSERTITEMW,0,tlparam(@lvi));
        inc(result);
      end;
      inc(CurMacro);
    end;
  end;

  CheckMacroList(Dialog);
//??  SendMessage(list,CB_SETCURSEL,0,0);
  SendMessage(list,LVM_SETCOLUMNWIDTH,0,LVSCW_AUTOSIZE_USEHEADER);

  ListView_SetItemState(list,0,
    LVIS_FOCUSED or LVIS_SELECTED,
    LVIS_FOCUSED or LVIS_SELECTED);
end;

//----- Other stuff -----

procedure SetButtonIcons2(Dialog:HWND);
var
  ti:TTOOLINFOW;
  hwndTooltip:HWND;
begin
  hwndTooltip:=CreateWindowW(TOOLTIPS_CLASS,nil,TTS_ALWAYSTIP,
      integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
      integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
      Dialog,0,hInstance,nil);

  FillChar(ti,SizeOf(ti),0);
  ti.cbSize  :=sizeof(TOOLINFO);
  ti.uFlags  :=TTF_IDISHWND or TTF_SUBCLASS;
  ti.hwnd    :=Dialog;
  ti.hinst   :=hInstance;

  ti.uId     :=GetDlgItem(Dialog,IDC_ACTION_HELP);
  ti.lpszText:=TranslateW('Help');
  SendMessage(ti.uId,BM_SETIMAGE,IMAGE_ICON,
      CallService(MS_SKIN_LOADICON,SKINICON_OTHER_HELP,0));
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
  
  ti.uId     :=GetDlgItem(Dialog,IDC_ACTION_NEW);
  ti.lpszText:=TranslateW('New');
  OptSetButtonIcon(ti.uId,ACI_NEW);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
{
  hNew:=GetDlgItem(Dialog,IDC_NEW);
  SendMessage  (hNew,BUTTONADDTOOLTIP,TWPARAM(TranslateW('New')),BATF_UNICODE);
  OptSetButtonIcon(hNew,ACI_NEW);
}

  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_NEW);
  OptSetButtonIcon(ti.uId,ACI_NEW);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_ACTION_UP);
  ti.lpszText:=TranslateW('Up');
  OptSetButtonIcon(ti.uId,ACI_UP);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_UP);
  OptSetButtonIcon(ti.uId,ACI_UP);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_ACTION_DOWN);
  ti.lpszText:=TranslateW('Down');
  OptSetButtonIcon(ti.uId,ACI_DOWN);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_DOWN);
  OptSetButtonIcon(ti.uId,ACI_DOWN);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_ACTION_DELETE);
  ti.lpszText:=TranslateW('Delete');
  OptSetButtonIcon(ti.uId,ACI_DELETE);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_DELETE);
  OptSetButtonIcon(ti.uId,ACI_DELETE);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
{
  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_RELOAD);
  ti.lpszText:=TranslateW('Reload');
  OptSetButtonIcon(ti.uId,ACI_RELOAD);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
}
  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_TEST);
  ti.lpszText:=TranslateW('Test');
  OptSetButtonIcon(ti.uId,ACI_TEST);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
{!!
  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_IMPORT);
  ti.lpszText:=TranslateW('Import');
  OptSetButtonIcon(ti.uId,ACI_IMPORT);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_EXPORT);
  ti.lpszText:=TranslateW('Export');
  OptSetButtonIcon(ti.uId,ACI_EXPORT);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
}
end;

function DlgProcOpt2(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  wnd:HWND;
  p:pActModule;
  i,j,num:int_ptr;
  lvflag:integer;
  li:LV_ITEMW;
  lv:LV_COLUMNW;
begin
  result:=0;
  case hMessage of
    WM_DESTROY: begin
      SetCancel;
    end;

    WM_INITDIALOG: begin
      SetStart;
      DontReact :=true;
      // variant - put after all dialogs adds (to translate them too)
      TranslateDialogDefault(Dialog);
      // For some actions
      MacroListWindow :=GetDlgItem(Dialog,IDC_MACRO_LIST);
      ActionListWindow:=GetDlgItem(Dialog,IDC_ACTION_LIST);

      SetButtonIcons2(Dialog);
      wnd:=ActionListWindow;
      SendMessage(wnd,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_CHECKBOXES,LVS_EX_CHECKBOXES);
      SendMessage(wnd,LVM_SETUNICODEFORMAT,1,0);
      FillChar(lv,SizeOf(lv),0);
      lv.mask:=LVCF_WIDTH;
      lv.cx  :=110;
      SendMessageW(wnd,LVM_INSERTCOLUMNW ,0,tlparam(@lv));
      SendMessageW(wnd,LVM_SETCOLUMNWIDTH,0,LVSCW_AUTOSIZE_USEHEADER);

      // action type combobox
      FillActTypeList(GetDlgItem(Dialog,IDC_ACTION_TYPE));
{!!
      // XML import/export service
      if ServiceExists(MS_SYSTEM_GET_XI)=0 then
      begin
        EnableWindow(GetDlgItem(Dialog,IDC_GROUP_EXPORT),false);
        EnableWindow(GetDlgItem(Dialog,IDC_GROUP_IMPORT),false);
      end;
}      
      OldActTableProc  :=pointer(SetWindowLongPtrW(wnd,GWL_WNDPROC,long_ptr(@NewActTableProc)));
      OldGroupTableProc:=pointer(SetWindowLongPtrW(MacroListWindow,
            GWL_WNDPROC,long_ptr(@NewGroupTableProc)));

      ClearDialogData;
      ResetDialogs;

      // fill group list
      wnd:=MacroListWindow;
      SendMessage(wnd,LVM_SETUNICODEFORMAT,1,0);
      lv.mask:=LVCF_WIDTH;
      lv.cx  :=110;
      SendMessageW(wnd,LVM_INSERTCOLUMNW,0,tlparam(@lv));
      FillMacroList(Dialog);
      ChangeListNotify(1);

      // fill current group
      FillActionList(Dialog,EditMacroList[0]);
      ChangeListNotify(2);
      EventMask:=0;
      DontReact:=false;
    end;

    WM_COMMAND: begin
      if DontReact then exit;
      case wParam shr 16 of
{
        EN_CHANGE: begin
// check for group renaming
          if loword(wParam)<>IDC_EDIT_FORMAT then
          begin
            SetChanged(Dialog,etACT);
          end;
        end;
}
        CBN_SELCHANGE:  begin
          case loword(wParam) of
            IDC_ACTION_TYPE: begin
              ClearDialogData;
              p:=GetLink(CB_GetData(lParam));
              ShowWindow(p^.DlgHandle,SW_SHOW);

              // Saving prevoius "normal" action type
              if p^.Hash>10 then
              begin
                SetWindowLongPtrW(lParam,GWLP_USERDATA,
                    SendMessage(lParam,CB_GETCURSEL,0,0));
                SetChanged(Dialog,etACT);
              end;
            end;
          end;
        end;

        BN_CLICKED: begin
          case loword(wParam) of
            IDC_GROUP_RELOAD, // don't affect to saved (DB) datas
            IDC_ACTION_HELP,
            IDC_GROUP_EXPORT,
            IDC_GROUP_TEST: ;

            IDC_GROUP_UP,
            IDC_GROUP_DOWN,
            IDC_GROUP_NEW : SetChanged(Dialog,etMacro);

            IDC_GROUP_DELETE, // action deleting with subactions
            IDC_GROUP_IMPORT,
            IDC_ACTION_NEW,
            IDC_ACTION_DELETE,
            IDC_ACTION_UP,
            IDC_ACTION_DOWN: begin
              SetChanged(Dialog,etMacro+etACT);
            end;
          else
            SetChanged(Dialog,etACT);
          end;

          case loword(wParam) of
{
            IDC_GROUP_EXPORT: begin
              if ShowDlgW(xmlfilename,xmlfilename,TranslateW(inoutfilter),false) then
              begin
                wnd:=MacroListWindow;
                for i:=0 to ListView_GetItemCount(wnd)-1 do
                begin
                  if ListView_GetItemState(wnd,i,LVIS_SELECTED)<>0 then
                  begin
                    with EditMacroList[LV_GetLParam(wnd,i)] do
                      if (flags and ACF_ASSIGNED)<>0 then // must be always true
                        flags:=flags or ACF_EXPORT;
                  end
                  else
                    with EditMacroList[i] do
                      if (flags and (ACF_EXPORT or ACF_ASSIGNED))=
                                    (ACF_EXPORT or ACF_ASSIGNED) then
                        flags:=flags and not ACF_EXPORT;
                end;
                i:=ACIO_EXPORT or ACIO_SELECTED;
                if GetFSize(xmlfilename)>0 then
                  if MessageBoxW(Dialog,TranslateW('Append data to file?'),
                     PluginName,MB_YESNO+MB_ICONWARNING)=IDYES then
                    i:=i or ACIO_APPEND;
//!!!!!!!!!!!!!!!
// we MUST Export EditMacroList, NOT MacroList
// OR
// Use "Apply" code before
                CallService(MS_ACT_INOUT,i,TLPARAM(@xmlfilename));
                for i:=0 to MaxGroups-1 do
                  with EditMacroList[i] do
                    if (flags and (ACF_EXPORT or ACF_ASSIGNED))=
                                  (ACF_EXPORT or ACF_ASSIGNED) then
                      flags:=flags and not ACF_EXPORT;
              end;
            end;
}
            IDC_GROUP_IMPORT: begin
              if ShowDlgW(xmlfilename,xmlfilename,TranslateW(inoutfilter)) then
              begin
                // "save" selected macro (in edit list only)
                BuildActionChain(Dialog);

                if Import(EditMacroList,xmlfilename,0)<>0 then
//                if CallService(MS_ACT_INOUT,0,TLPARAM(@xmlfilename))<>0 then
                begin
                  // copy from WM_INITDIALOG
                  FillMacroList(Dialog);
                  ChangeListNotify(1);

                  // fill current group
                  FillActionList(Dialog,EditMacroList[0]);
                  ChangeListNotify(2);

                  CheckMacroListDirectionButtons(Dialog);
                end;
              end;
            end;

            IDC_GROUP_TEST: begin
              li.mask    :=LVIF_PARAM;
              li.iSubItem:=0;
              // Get selected macro
              li.iItem   :=SendMessage(MacroListWindow,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
              // "save" selected macro (in edit list only)
              BuildActionChain(Dialog,li.iItem);
              // Get lParam = EditMacroList element
              SendMessageW(MacroListWindow,LVM_GETITEMW,0,tlparam(@li));

              ActionStarter(EditMacroList[loword(li.lParam)],nil,ACTP_SAMETHREAD or ACTP_WAIT);
            end;

            IDC_GROUP_NEW: begin
              if AddMacro(Dialog,EditMacroList.NewMacro())>=0 then
              begin
                CheckMacroList(Dialog);
                ChangeListNotify(1);
              end;
            end;
            IDC_GROUP_DELETE: begin
              DontReact:=true;
              wnd:=MacroListWindow;
              for i:=ListView_GetItemCount(wnd)-1 downto 0 do
              begin
                if ListView_GetItemState(wnd,i,LVIS_SELECTED)<>0 then
                begin
                  num:=Loword(LV_GetLParam(wnd,i));
                  // mark old to delete on apply (must be first)
                  with EditMacroList[num]^ do
                    for j:=0 to ActionCount-1 do
                      ActionList[j].flags:=ActionList[j].flags or ACF_REPLACED;
                  // delete new actions
                  FreeMacro(EditMacroList[num],ACF_INTRODUCED);

                  SendMessage(wnd,LVM_DELETEITEM,i,0);
                end;
              end;
              Listview_SetItemState(wnd,-1,0,LVIS_FOCUSED or LVIS_SELECTED);
              DontReact:=false;
              SendDlgItemMessage(Dialog,IDC_ACTION_LIST,LVM_DELETEALLITEMS,0,0);
              Listview_SetItemState(wnd,0,LVIS_FOCUSED or LVIS_SELECTED,
                                          LVIS_FOCUSED or LVIS_SELECTED);
              CheckMacroList(Dialog);
              ChangeListNotify(1);
            end;

            IDC_ACTION_HELP: begin
              SendMessage(
                GetLink(CB_GetData(GetDlgItem(Dialog,IDC_ACTION_TYPE)))^.DlgHandle,
                WM_HELP,0,0);
            end;

            IDC_ACTION_NEW: begin
              wnd:=ActionListWindow;
              li.mask    :=LVIF_PARAM;
              i          :=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
              li.iItem   :=i+1;
              li.iSubItem:=0;
              li.lParam  :=0;{NewAction(NewActionList,NewMaxActions);}
              SendMessageW(wnd,LVM_INSERTITEMW,0,tlparam(@li));
              ListView_SetCheckState(wnd,li.iItem,true);
              if li.iItem=0 then
              begin
                ListView_SetItemState(wnd,0,LVIS_FOCUSED or LVIS_SELECTED,
                  LVIS_FOCUSED or LVIS_SELECTED);
                SHActButtons(Dialog,SW_SHOW);
                SendDlgItemMessage(Dialog,IDC_ACTION_TYPE,CB_SETCURSEL,0,0);
              end;
              ChangeActionName(Dialog,li.iItem);

              CheckActListButtons(Dialog);
              ChangeListNotify(2);
            end;
            IDC_ACTION_DELETE: begin
              DontReact:=true;
              wnd:=ActionListWindow;
              for i:=ListView_GetItemCount(wnd)-1 downto 0 do
              begin
                if ListView_GetItemState(wnd,i,LVIS_SELECTED)<>0 then
                begin
                  j:=LV_GetLParam(wnd,i);
                  if j<>0 then
                  begin
                    if (tBaseAction(j).flags and ACF_INTRODUCED)<>0 then
                      tBaseAction(j).Free
                    else
                      tBaseAction(j).flags:=tBaseAction(j).flags or ACF_REPLACED;
                  end;
                  SendMessage(wnd,LVM_DELETEITEM,i,0);
                end;
              end;
              Listview_SetItemState(wnd,-1,0,LVIS_FOCUSED or LVIS_SELECTED);
              DontReact:=false;
              Listview_SetItemState(wnd,0,LVIS_FOCUSED or LVIS_SELECTED,
                                          LVIS_FOCUSED or LVIS_SELECTED);
              CheckActListButtons(Dialog);
              ChangeListNotify(2);
            end;

            IDC_GROUP_UP: begin
              MoveMacro(MacroListWindow,-1,-1);
              CheckMacroListDirectionButtons(Dialog);
              EventMask:=EventMask or ACTM_SORT;
              ChangeListNotify(1);
            end;
            IDC_GROUP_DOWN: begin
              MoveMacro(MacroListWindow,-1,1);
              CheckMacroListDirectionButtons(Dialog);
              EventMask:=EventMask or ACTM_SORT;
              ChangeListNotify(1);
            end;

            IDC_ACTION_UP: begin
              MoveMacro(ActionListWindow,-1,-1);
              CheckActListDirectionButtons(Dialog);
              ChangeListNotify(2);
            end;
            IDC_ACTION_DOWN: begin
              MoveMacro(ActionListWindow,-1,1);
              CheckActListDirectionButtons(Dialog);
              ChangeListNotify(2);
            end;

          end;
        end;
      end;
    end;

    WM_HELP: begin
      SendMessage(
        GetLink(CB_GetData(GetDlgItem(Dialog,IDC_ACTION_TYPE)))^.DlgHandle,
        WM_HELP,0,lParam);
      result:=1;
    end;

    WM_NOTIFY: begin
      case integer(PNMHdr(lParam)^.code) of
        PSN_APPLY: begin
          BuildActionChain(Dialog);

          EventMask:=EventMask or CheckChanges();
          SetSave(Dialog);
          SaveMacros;

          if EventMask<>0 then //??
          begin
            NotifyEventHooks(hHookChanged,EventMask,0);
            EventMask:=0;
          end;

        end;

        NM_DBLCLK: begin
          if PNMListView(lParam)^.iItem>=0 then
            PostMessageW(PNMHdr(lParam)^.hWndFrom,LVM_EDITLABELW,
                        PNMListView(lParam)^.iItem,0);
        end;

        LVN_ITEMCHANGED: begin
          if DontReact then exit; // bug when group moved avoid

          if wParam=IDC_MACRO_LIST then
          begin
            if PNMLISTVIEW(lParam)^.uChanged=LVIF_STATE then
            begin
              lvflag:=(PNMLISTVIEW(lParam)^.uOldState and LVNI_FOCUSED)-
                      (PNMLISTVIEW(lParam)^.uNewState and LVNI_FOCUSED);

              if lvflag>0 then // old focus
                BuildActionChain(Dialog,PNMLISTVIEW(lParam)^.iItem)
              else if lvflag<0 then // new focus
              begin
                DontReact:=true;

                ClearDialogData;
                ResetDialogs;
                FillActionList(Dialog,EditMacroList[Loword(PNMLISTVIEW(lParam)^.lParam)]);
                CheckMacroListDirectionButtons(Dialog);
                ChangeListNotify(2);

                DontReact:=false;
              end
              else
              begin
{??
                lvflag:=(PNMLISTVIEW(lParam)^.uOldState and LVNI_SELECTED)-
                        (PNMLISTVIEW(lParam)^.uNewState and LVNI_SELECTED);

                if lvflag<>0 then
                  CheckMacroListDirectionButtons(Dialog);
}
              end;
            end;
          end

          else if wParam=IDC_ACTION_LIST then
          begin
            lvflag:=(PNMLISTVIEW(lParam)^.uOldState and LVNI_FOCUSED)-
                    (PNMLISTVIEW(lParam)^.uNewState and LVNI_FOCUSED);
            if lvflag>0 then // old focus
            begin
              SaveAction(Dialog,
                         PNMLISTVIEW(lParam)^.iItem,
                         tBaseAction(PNMLISTVIEW(lParam)^.lParam));
            end
            else if lvflag<0 then // new focus
            begin
              DontReact:=true;

              ClearDialogData;
              ResetDialogs;
              FillAction(Dialog,tBaseAction(PNMLISTVIEW(lParam)^.lParam));
              CheckActListDirectionButtons(Dialog);

              DontReact:=false;
            end
            else
            begin // checkboxes
{??
              lvflag:=(PNMLISTVIEW(lParam)^.uOldState and LVNI_SELECTED)-
                      (PNMLISTVIEW(lParam)^.uNewState and LVNI_SELECTED);

              if lvflag<>0 then
                CheckMacroListDirectionButtons(Dialog);
}
              if (PNMLISTVIEW(lParam)^.uOldState or PNMLISTVIEW(lParam)^.uNewState)=$3000 then
              begin
                SetChanged(Dialog,etACT);
              end;
            end;
          end;
        end;

        LVN_ENDLABELEDITW: begin
          if DontReact then exit;

          if wParam=IDC_MACRO_LIST then
          begin
            with PLVDISPINFOW(lParam)^ do
            begin
              if item.pszText<>nil then
              begin
                EventMask:=EventMask or ACTM_RENAME;
                SetChanged(Dialog,etMacro);
                item.mask:=LVIF_TEXT;
                if pWideChar(item.pszText)^=#0 then
                  pWideChar(item.pszText):=NoDescription;
                SendMessageW(hdr.hWndFrom,LVM_SETITEMW,0,tlparam(@item));

                with EditMacroList[loword(item.lParam)]^ do
                begin
                  StrCopyW(descr,item.pszText,MacroNameLen-1);
                end;
                ChangeListNotify(1);

                result:=1;
              end;
            end;
          end
          else if wParam=IDC_ACTION_LIST then
          begin
            with PLVDISPINFOW(lParam)^ do
            begin
              if item.pszText<>nil then
              begin
                SetChanged(Dialog,etACT);
                ChangeActionName(Dialog,item.iItem,pWideChar(item.pszText));
                ChangeListNotify(2);
                result:=1;
              end;
            end;
          end;
        end;

      end;
    end;
  else
//  {result:=}DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
//  {result:=}DefWindowProc(Dialog,hMessage,wParam,lParam);
end;

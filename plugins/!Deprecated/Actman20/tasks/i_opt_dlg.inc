{}
const
  settings:HWND = 0;

var
  OldTableProc:pointer;
  onactchanged:THANDLE;

const
  ACI_NEW    :PAnsiChar = 'ACI_New';
  ACI_DELETE :PAnsiChar = 'ACI_Delete';

procedure CheckTaskList(Dialog:HWND;enable:boolean);
begin
  if not enable then
    enable:=SendMessage(GetDlgItem(Dialog,IDC_TASK_NAME),LVM_GETITEMCOUNT,0,0)>0;

  EnableWindow(GetDlgItem(Dialog,IDC_TASK_ABSOLUTE   ),enable);
  EnableWindow(GetDlgItem(Dialog,IDC_TASK_DATEV      ),enable);
  EnableWindow(GetDlgItem(Dialog,IDC_TASK_DAYSV      ),enable);
  EnableWindow(GetDlgItem(Dialog,IDC_TASK_TIMEV      ),enable);
  EnableWindow(GetDlgItem(Dialog,IDC_TASK_REPEAT     ),enable);
  EnableWindow(GetDlgItem(Dialog,IDC_TASK_INTERVAL   ),enable);
  EnableWindow(GetDlgItem(Dialog,IDC_TASK_BREAK      ),enable);
  EnableWindow(GetDlgItem(Dialog,IDC_TASK_EVENT      ),enable);
  EnableWindow(GetDlgItem(Dialog,IDC_TASK_ONCE       ),enable);
  if not enable then
  begin
    ShowWindow(GetDlgItem(Dialog,IDC_TASK_DAYST),SW_HIDE);
    ShowWindow(GetDlgItem(Dialog,IDC_TASK_DAYSV),SW_HIDE);
  end;
end;

procedure FillTaskList(wnd:HWND);
var
  i:integer;
  li:LV_ITEMW;
begin
  SendMessage(wnd,LVM_DELETEALLITEMS,0,0);
  for i:=0 to MaxTasks-1 do
  begin
    with TaskList[i] do
    begin
      if (flags and ACF_ASSIGNED)<>0 then
      begin
        li.mask    :=LVIF_TEXT+LVIF_PARAM;
        li.iSubItem:=0;
        li.iItem   :=i;
        li.lParam  :=i;
        li.pszText :=name;
        li.iItem   :=SendMessageW(wnd,LVM_INSERTITEMW,0,LPARAM(@li));
        ListView_SetCheckState(wnd,li.iItem,(flags and ACF_DISABLED)=0);
      end;
    end;
  end;
  ListView_SetItemState(wnd,0,
    LVIS_FOCUSED or LVIS_SELECTED,
    LVIS_FOCUSED or LVIS_SELECTED);
end;

procedure ClearTaskData(Dialog:HWND);
var
  st:TSystemTime;
begin
  SendMessage(GetDlgItem(Dialog,IDC_TASK_ABSOLUTE),CB_SETCURSEL,0,0);
  CheckDlgButton(Dialog,IDC_TASK_BREAK      ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_TASK_EVENT      ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_TASK_ONCE       ,BST_UNCHECKED);
  EnableWindow(GetDlgItem(Dialog,IDC_TASK_ONCE),false);

  SetDlgItemInt(Dialog,IDC_TASK_DAYSV ,1,false);
  SetDlgItemInt(Dialog,IDC_TASK_REPEAT,0,false);

  FillChar(st,SizeOf(st),0);
  SendDlgItemMessage(Dialog,IDC_TASK_TIMEV   ,DTM_SETSYSTEMTIME,GDT_VALID,lParam(@st));
  SendDlgItemMessage(Dialog,IDC_TASK_INTERVAL,DTM_SETSYSTEMTIME,GDT_VALID,lParam(@st));
{
  ShowWindow(GetDlgItem(Dialog,IDC_TASK_DATET),SW_SHOW);
  ShowWindow(GetDlgItem(Dialog,IDC_TASK_DATEV),SW_SHOW);
  ShowWindow(GetDlgItem(Dialog,IDC_TASK_DAYST),SW_HIDE);
  ShowWindow(GetDlgItem(Dialog,IDC_TASK_DAYSV),SW_HIDE);
}
end;

procedure ShowDateType(Dialog:HWND;start:integer);
var
  sh1,sh2,sh3:integer;
begin
  case start of
    1: begin // start after
      sh1:=SW_HIDE;
      sh2:=SW_SHOW;
      sh3:=SW_SHOW;
    end;
    2: begin // start from
      sh1:=SW_SHOW;
      sh2:=SW_HIDE;
      sh3:=SW_SHOW;
    end;
  else
    begin
//    3: begin // start immediately
      sh1:=SW_HIDE;
      sh2:=SW_HIDE;
      sh3:=SW_HIDE;
    end;
  end;

  ShowWindow(GetDlgItem(Dialog,IDC_TASK_DATET),sh1);
  ShowWindow(GetDlgItem(Dialog,IDC_TASK_DATEV),sh1);
  ShowWindow(GetDlgItem(Dialog,IDC_TASK_DAYST),sh2);
  ShowWindow(GetDlgItem(Dialog,IDC_TASK_DAYSV),sh2);
  ShowWindow(GetDlgItem(Dialog,IDC_TASK_TIMET),sh3);
  ShowWindow(GetDlgItem(Dialog,IDC_TASK_TIMEV),sh3);
end;

procedure ShowTaskData(Dialog:HWND; item:integer=-1);
var
  st:TSystemTime;
  lwnd:HWND;
  start:integer;
begin
  lwnd:=settings;
  settings:=0;

  ClearTaskData(Dialog);

  with TaskList[LV_GetLParam(GetDlgItem(Dialog,IDC_TASK_NAME),item)] do
  begin
    // flags

    if (flags and TCF_NONZEROBREAK)<>0 then
      CheckDlgButton(Dialog,IDC_TASK_BREAK,BST_CHECKED);
    if (flags and TCF_MAKEEVENT)<>0 then
      CheckDlgButton(Dialog,IDC_TASK_EVENT,BST_CHECKED);
    if (flags and TCF_EVENTONCE)<>0 then
      CheckDlgButton(Dialog,IDC_TASK_ONCE,BST_CHECKED);

    EnableWindow(GetDlgItem(Dialog,IDC_TASK_ONCE),
      IsDlgButtonChecked(Dialog,IDC_TASK_EVENT)<>BST_UNCHECKED);

    // action
    CB_SelectData(GetDlgItem(Dialog,IDC_TASK_ACTION),action);
    // times
    FileTimeToSystemTime(starttime,st);

    if (flags and TCF_IMMEDIATELY)<>0 then
    begin
      start:=3;
    end
    else if (flags and TCF_ABSOLUTE)<>0 then
    begin
      start:=2;
      SendDlgItemMessage(Dialog,IDC_TASK_DATEV,DTM_SETSYSTEMTIME,GDT_VALID,lParam(@st))
    end
    else
    begin
      start:=1;
      SetDlgItemInt(Dialog,IDC_TASK_DAYSV,dayoffset,false);
    end;
    CB_SelectData(GetDlgItem(Dialog,IDC_TASK_ABSOLUTE),start);

    SendDlgItemMessage(Dialog,IDC_TASK_TIMEV,DTM_SETSYSTEMTIME,GDT_VALID,lParam(@st));

    SetDlgItemInt(Dialog,IDC_TASK_REPEAT,count,true);

    FileTimeToSystemTime(interval,st);
    SendDlgItemMessage(Dialog,IDC_TASK_INTERVAL,DTM_SETSYSTEMTIME,GDT_VALID,lParam(@st));
    SetDlgItemInt(Dialog,IDC_TASK_INTDAYS,intdays,false);
  end;

  ShowDateType(Dialog,start);

  settings:=lwnd;
end;

procedure SaveTaskData(Dialog:HWND; item:integer=-1);
var
  wnd:HWND;
  li:LV_ITEM;
  st,st1:TSystemTime;
  tmp:longbool;
begin
  wnd:=GetDlgItem(Dialog,IDC_TASK_NAME);

  if item<0 then
    li.iItem:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED) // LVNI_SELECTED
  else
    li.iItem:=item;

  if li.iItem>=0 then
  begin
    li.mask      :=LVIF_PARAM;
    li.iSubItem  :=0;
    SendMessageW(wnd,LVM_GETITEMW,0,LPARAM(@li));

    with TaskList[li.lParam] do
    begin
      if (flags and ACF_ASSIGNED)<>0  then
      begin
        flags:=ACF_ASSIGNED;
        // flags
        if ListView_GetCheckState(wnd,li.iItem)=0 then
          flags:=flags or ACF_DISABLED;

        if IsDlgButtonChecked(Dialog,IDC_TASK_BREAK)<>BST_UNCHECKED then
          flags:=flags or TCF_NONZEROBREAK;
        if IsDlgButtonChecked(Dialog,IDC_TASK_EVENT)<>BST_UNCHECKED then
        begin
          flags:=flags or TCF_MAKEEVENT;
          if IsDlgButtonChecked(Dialog,IDC_TASK_ONCE )<>BST_UNCHECKED then
            flags:=flags or TCF_EVENTONCE;
        end;
        // action
        action:=CB_GetData(GetDlgItem(Dialog,IDC_TASK_ACTION));
        // times
        SendDlgItemMessage(Dialog,IDC_TASK_TIMEV,DTM_GETSYSTEMTIME,0,lParam(@st));

        case CB_GetData(GetDlgItem(Dialog,IDC_TASK_ABSOLUTE)) of
          1: begin
            dayoffset:=GetDlgItemInt(Dialog,IDC_TASK_DAYSV,tmp,false);
          end;
          2: begin
            flags:=flags or TCF_ABSOLUTE;
            SendDlgItemMessage(Dialog,IDC_TASK_DATEV,DTM_GETSYSTEMTIME,0,lParam(@st1));
            st.wYear     :=st1.wYear;
            st.wMonth    :=st1.wMonth;
            st.wDayOfWeek:=st1.wDayOfWeek;
            st.wDay      :=st1.wDay;
          end;
          3: begin
            flags:=flags or TCF_IMMEDIATELY;
          end;
        end;
        SystemTimeToFileTime(st,starttime);

        count:=GetDlgItemInt(Dialog,IDC_TASK_REPEAT,tmp,true);

        SendDlgItemMessage(Dialog,IDC_TASK_INTERVAL,DTM_GETSYSTEMTIME,0,lParam(@st));
        SystemTimeToFileTime(st,interval);
        intdays:=GetDlgItemInt(Dialog,IDC_TASK_INTDAYS,tmp,false);
      end;
    end;
  end;
end;

function NewTask(Dialog:HWND;item:integer=-1):integer;
var
  wnd:HWND;
  li:LV_ITEMW;
begin
  wnd:=GetDlgItem(Dialog,IDC_TASK_NAME);
  if item<0 then
    li.iItem :=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED)+1
  else
    li.iItem :=item;
  li.iSubItem:=0;
  li.mask    :=LVIF_TEXT + LVIF_PARAM;
  li.lParam  :=CreateNewTask;
  li.pszText :=TranslateW('Task sample');
  result:=SendMessageW(wnd,LVM_INSERTITEMW,0,LPARAM(@li));

  ListView_SetCheckState(wnd,li.iItem,
    (TaskList[li.lParam].flags and ACF_DISABLED)=0);
  StrDupW(TaskList[li.lParam].name,li.pszText);

  CheckTaskList(Dialog,true);

  if li.iItem=0 then
    Listview_SetItemState(wnd,0,LVIS_FOCUSED or LVIS_SELECTED,
                                LVIS_FOCUSED or LVIS_SELECTED);
end;

function DeleteTask(Dialog:HWND):integer;
var
  li:LV_ITEM;
  wnd:HWND;
  i:integer;
begin
  result:=0;
  wnd:=GetDlgItem(Dialog,IDC_TASK_NAME);
  for i:=ListView_GetItemCount(wnd)-1 downto 0 do
  begin
    if ListView_GetItemState(wnd,i,LVIS_SELECTED)<>0 then
    begin
      li.iItem     :=i;
      li.mask      :=LVIF_PARAM;
      li.iSubItem  :=0;
      SendMessageW(wnd,LVM_GETITEMW,0,LPARAM(@li));

      TaskList[li.lParam].flags:=TaskList[li.lParam].flags and not ACF_ASSIGNED;

      SendMessage(wnd,LVM_DELETEITEM,i,0);
    end;
  end;
  Listview_SetItemState(wnd,0,LVIS_FOCUSED or LVIS_SELECTED,
                              LVIS_FOCUSED or LVIS_SELECTED);

  CheckTaskList(Dialog,false);
end;

function NewHKTableProc(Dialog:HWnd;hMessage:UINT;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
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
            PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_TASK_NEW,0);
            exit;
          end;

          VK_DELETE: begin
            PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_TASK_DELETE,0);
            exit;
          end;
        end;
      end;
    end;
  end;
  result:=CallWindowProc(OldTableProc,Dialog,hMessage,wParam,lParam);
end;

procedure FillStartTimeList(wnd:HWND);
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);
  CB_AddStrDataW(wnd,TranslateW('Starting after'   ),1);
  CB_AddStrDataW(wnd,TranslateW('Starting from'    ),2);
  CB_AddStrDataW(wnd,TranslateW('Start immediately'),3);
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

procedure FillActionList(wnd:HWND);
var
  ptr,ptr1:pChain;
  i,cnt:integer;
begin
  cnt:=CallService(MS_ACT_GETLIST,0,LPARAM(@ptr));
  SendMessage(wnd,CB_RESETCONTENT,0,0);
  if cnt>0 then
  begin
    ptr1:=ptr;
    inc(pbyte(ptr),4);
    for i:=0 to cnt-1 do
    begin
      CB_AddStrDataW(wnd,ptr^.descr,ptr^.id);
      inc(ptr);
    end;

    CallService(MS_ACT_FREELIST,0,LPARAM(ptr1));
    SendMessage(wnd,CB_SETCURSEL,0,0);
  end;
end;

function ActListChange(wParam:WPARAM;lParam:LPARAM):integer; cdecl;
begin
  result:=0;
  if settings<>0 then
    FillActionList(GetDlgItem(settings,IDC_TASK_ACTION));
end;

procedure SetIcons(Dialog:HWND);
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
  ti.hwnd    :=dialog;
  ti.hinst   :=hInstance;
{
  ti.uId     :=GetDlgItem(Dialog,IDC_EVENT_HELP);
  ti.lpszText:=TranslateW('Help');
  SendMessage(ti.uId,BM_SETIMAGE,IMAGE_ICON,
      CallService(MS_SKIN_LOADICON,SKINICON_OTHER_HELP,0));
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,LPARAM(@ti));
}  
  ti.uId     :=GetDlgItem(Dialog,IDC_TASK_NEW);
  ti.lpszText:=TranslateW('New');
  SetButtonIcon(ti.uId,ACI_NEW);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,LPARAM(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_TASK_DELETE);
  ti.lpszText:=TranslateW('Delete');
  SetButtonIcon(ti.uId,ACI_DELETE);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,LPARAM(@ti));
end;

function DlgProcOpt(Dialog:HWnd;hMessage:UINT;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
var
  wnd:HWND;
  lv:LV_COLUMNW;
  li:LV_ITEMW;
  i:integer;
begin
  result:=0;
  case hMessage of
    WM_CLOSE: begin
      UnhookEvent(onactchanged);
      settings:=0;
    end;

    WM_INITDIALOG: begin
      settings:=0;
      wnd:=GetDlgItem(Dialog,IDC_TASK_NAME);
      SendMessage(wnd,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_CHECKBOXES,LVS_EX_CHECKBOXES);
      SendMessage(wnd,LVM_SETUNICODEFORMAT,1,0);
      zeromemory(@lv,sizeof(lv));
      lv.mask:=LVCF_WIDTH;
      lv.cx  :=110;
      SendMessageW(wnd,LVM_INSERTCOLUMNW ,0,tlparam(@lv));
      SendMessageW(wnd,LVM_SETCOLUMNWIDTH,0,LVSCW_AUTOSIZE_USEHEADER);

      CreateUpDownControl(
        WS_CHILD+WS_BORDER+WS_VISIBLE+UDS_ARROWKEYS+UDS_SETBUDDYINT+UDS_ALIGNRIGHT,
        190,112,14,18,
        Dialog, IDC_TASK_UPDOWN, hInstance, GetDlgItem(Dialog,IDC_TASK_REPEAT),
        10000, -1, 0);

      OldTableProc:=pointer(SetWindowLongPtrW(wnd,GWL_WNDPROC,LONG_PTR(@NewHKTableProc)));
      TranslateDialogDefault(Dialog);

      SetIcons(Dialog);

      FillActionList(GetDlgItem(Dialog,IDC_TASK_ACTION));
      FillStartTimeList(GetDlgItem(Dialog,IDC_TASK_ABSOLUTE));
      FillTaskList(wnd);
      CheckTaskList(Dialog,false);
      onactchanged:=HookEvent(ME_ACT_CHANGED,@ActListChange);
      settings:=Dialog;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        CBN_SELCHANGE: begin
          ShowDateType(Dialog,CB_GetData(lParam));
        end;

        EN_CHANGE: begin
        end;

        BN_CLICKED: begin
          case loword(wParam) of
            IDC_TASK_NEW   : NewTask(Dialog);
            IDC_TASK_DELETE: DeleteTask(Dialog);

            IDC_TASK_EVENT: begin
              EnableWindow(GetDlgItem(Dialog,IDC_TASK_ONCE),
                IsDlgButtonChecked(Dialog,IDC_TASK_EVENT)<>BST_UNCHECKED);
            end;
          end;
        end;
      end;
      if settings<>0 then
        SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
    end;

    WM_NOTIFY: begin
      case integer(PNMHdr(lParam)^.code) of
        PSN_APPLY: begin
          SaveTaskData(Dialog);
          SaveTasks;
          SetAllTasks;
        end;

        DTN_DATETIMECHANGE: begin
          if settings<>0 then
            SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
        end;

        NM_DBLCLK: begin
          if PNMListView(lParam)^.iItem>=0 then
            PostMessageW(PNMHdr(lParam)^.hWndFrom,LVM_EDITLABELW,
                        PNMListView(lParam)^.iItem,0);
        end;

        LVN_ITEMCHANGED: begin
          if PNMLISTVIEW(lParam)^.uChanged=LVIF_STATE then
          begin
            i:=(PNMLISTVIEW(lParam)^.uOldState and LVNI_FOCUSED)-
               (PNMLISTVIEW(lParam)^.uNewState and LVNI_FOCUSED);

            if i>0 then // old focus
              SaveTaskData(Dialog,PNMLISTVIEW(lParam)^.iItem)
            else if i<0 then // new focus
            begin
              ShowTaskData(Dialog,PNMLISTVIEW(lParam)^.iItem);
            end
            else if (settings<>0) and
               ((PNMLISTVIEW(lParam)^.uOldState or PNMLISTVIEW(lParam)^.uNewState)=$3000) then
              SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
          end;
        end;

        LVN_ENDLABELEDITW: begin
          with PLVDISPINFOW(lParam)^ do
          begin
            if item.pszText<>nil then
            begin
              item.mask:=LVIF_TEXT;
              SendMessageW(hdr.hWndFrom,LVM_SETITEMW,0,TLPARAM(@item));

              li.iItem     :=item.iItem;
              li.mask      :=LVIF_PARAM;
              li.iSubItem  :=0;
              SendMessageW(hdr.hWndFrom,LVM_GETITEMW,0,TLPARAM(@li));
              with TaskList[li.lParam] do
              begin
                mFreeMem(name);
                StrDupW (name,item.pszText);
              end;
            end;
          end;
          result:=1;
        end;
      end;
    end;
  end;
end;

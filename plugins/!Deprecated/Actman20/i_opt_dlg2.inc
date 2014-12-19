{}

const
  inoutfilter:pWideChar = 'XML files'#0'*.xml'#0'All files'#0'*.*'#0#0;
const
  NoChainText:PWideChar = 'not defined';
const
  ActionNames:array [0..ACT_MAXTYPE] of pWideChar=(
    'Unknown','Contact','Service','Program','Text','Advanced','Action','Profile','Message');
const
  checknames:array [BST_UNCHECKED..BST_CHECKED] of PAnsiChar=(
    ACI_VAR_UNCHECKED,ACI_VAR_CHECKED);
const
  MaxDescrLen = 128;
const
  hlpContact      = 0;
  hlpService      = 1;
  hlpProgram      = 2;
  hlpText         = 3;
  hlpAdvance      = 4;
  hlpChain        = 5;
  hlpDBRW         = 6;
  hlpMessage      = 7;
  hlpVariables    = 30;
  hlpAdvVariables = 31;
const
  ptNumber  = 0;
  ptString  = 1;
  ptUnicode = 2;
  ptCurrent = 3;
  ptResult  = 4;
  ptParam   = 5;
  ptStruct  = 6;

var
  wstruct,lstruct:pAnsiChar;
  DontReact:bool;
  OldGroupTableProc,
  OldActTableProc:pointer;
  ChMask:dword;

function GetNumValue(wnd:HWND;usevar:boolean;var dst):boolean;
var
  tmp:pWideChar;
begin
  result:=false;
  pWideChar(dst):=GetDlgText(wnd);
  if Pointer(dst)=nil then exit;
  if not usevar then
  begin
    tmp:=PWideChar(dst);
    if pWideChar(dst)^='$' then
    begin
      integer(dst):=HexToInt(pWideChar(dst)+1);
      result:=true;
    end
    else
      integer(dst):=StrToInt(pWideChar(dst));
    mFreeMem(tmp);
  end;
end;

function GetGroupName(id:dword):pWideChar;
var
  i:integer;
begin
  for i:=0 to NewMaxGroups-1 do
  begin
    if ((NewGroupList^[i].flags and ACF_ASSIGNED)<>0) and (id=NewGroupList^[i].id) then
    begin
      result:=NewGroupList^[i].descr;
      exit;
    end;
  end;
  result:=NoChainText;
end;

function AddGroup(Dialog:HWND;HKnum:dword):integer;
var
  li:LV_ITEMW;
  list:HWND;
begin
  with NewGroupList^[HKnum] do
  begin
    if (flags and ACF_ASSIGNED)<>0 then
    begin
      list:=GetDlgItem(Dialog,IDC_ACTION_GROUP);
      li.mask     :=LVIF_PARAM+LVIF_TEXT;
      li.iItem    :=SendMessage(list,LVM_GETNEXTITEM,-1,LVNI_FOCUSED)+1;
      li.iSubItem :=0;
      li.lParam   :=HKnum;
      if descr=nil then
        li.pszText:=NoDescription
      else
        li.pszText:=descr;
      li.iItem    :=SendMessageW(list,LVM_INSERTITEMW,0,lparam(@li));
      if li.iItem>0 then
        dec(li.iItem);
      ListView_SetItemState(list,li.iItem,LVIS_FOCUSED or LVIS_SELECTED,
          LVIS_FOCUSED or LVIS_SELECTED);
      result:=li.iItem;
    end
    else
      result:=-1;
  end;
end;

// Fill action type combobox
procedure FillActTypeList(list:hwnd);
var
  cbei:TCOMBOBOXEXITEMW;
  il:HIMAGELIST;
  i:integer;
  buf:array [0..127] of WideChar;
begin
  SendMessage(list,CB_RESETCONTENT,0,0);
  il:=ImageList_Create(16,16,ILC_COLOR32 or ILC_MASK,0,1);

  cbei.mask:=CBEIF_IMAGE or CBEIF_SELECTEDIMAGE or CBEIF_TEXT; //!!
  for i:=0 to ACT_MAXTYPE-1 do
  begin
    ImageList_AddIcon(il,CallService(MS_SKIN2_GETICON,0,lparam(ActIds[i].icon)));

    cbei.pszText       :=TranslateW(FastAnsiToWideBuf(ActIds[i].text,buf));
    cbei.iItem         :=i;
    cbei.iImage        :=i;
    cbei.iSelectedImage:=i;
    if SendMessageW(list,CBEM_INSERTITEMW,0,lparam(@cbei))=-1 then break;
  end;
  ImageList_Destroy(SendMessage(list,CBEM_SETIMAGELIST,0,il));
  SendMessage(list,CB_SETCURSEL,0,0);
end;

procedure FillSubList(Dialog:hwnd);
var
  list,wnd:HWND;
  i,act:integer;
  arr:array [0..127] of WideChar;
  li:LV_ITEMW;
begin
  wnd:=GetDlgItem(Dialog,IDC_ADV_VAL2);

  SendMessage(wnd,CB_RESETCONTENT,0,0);

  list:=GetDlgItem(Dialog,IDC_ACTION_LIST);
  act:=SendMessageW(list,LVM_GETITEMCOUNT,0,0);
  i:=0;
  li.mask      :=LVIF_TEXT;
  li.iSubItem  :=0;
  li.pszText   :=@arr;
  li.cchTextMax:=SizeOf(arr) div SizeOf(WideChar);
  while i<act do
  begin
    li.iItem:=i;
    SendMessageW(list,LVM_GETITEMW,0,lparam(@li));
    SendMessageW(wnd,CB_ADDSTRING,0,lparam(PWideChar(@arr)));
    inc(i);
  end;
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

procedure FillChainList(Dialog:hwnd);
var
  wnd:HWND;
  i:integer;
//  num:integer;
begin
  wnd:=GetDlgItem(Dialog,IDC_GROUP_LIST);
//  for current chain exclude
//  num:=SendDlgItemMessage(Dialog,IDC_ACTION_GROUP,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);

  SendMessage(wnd,CB_RESETCONTENT,0,0);
  SendMessage(wnd,CB_SETITEMDATA,
    SendMessageW(wnd,CB_ADDSTRING,0,lparam(TranslateW(NoChainText))),0);
  for i:=0 to NewMaxGroups-1 do
  begin
    if (NewGroupList^[i].flags and (ACF_ASSIGNED or ACF_VOLATILE))=ACF_ASSIGNED then
    begin
      SendMessage(wnd,CB_SETITEMDATA,
        SendMessageW(wnd,CB_ADDSTRING,0,lparam(NewGroupList^[i].descr)),
        NewGroupList^[i].id);
    end;
  end;
end;

// action group table procedure (key hook)
function NewGroupTableProc(Dialog:HWnd;hMessage:uint;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
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

// action (chain) table procedure (key hook)
function NewActTableProc(Dialog:HWnd;hMessage:uint;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
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

// miranda button icon paint
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
  SetButtonIcon(ti.uId,ACI_NEW);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_NEW);
  SetButtonIcon(ti.uId,ACI_NEW);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_ACTION_UP);
  ti.lpszText:=TranslateW('Up');
  SetButtonIcon(ti.uId,ACI_UP);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_UP);
  SetButtonIcon(ti.uId,ACI_UP);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_ACTION_DOWN);
  ti.lpszText:=TranslateW('Down');
  SetButtonIcon(ti.uId,ACI_DOWN);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_DOWN);
  SetButtonIcon(ti.uId,ACI_DOWN);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_ACTION_DELETE);
  ti.lpszText:=TranslateW('Delete');
  SetButtonIcon(ti.uId,ACI_DELETE);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_DELETE);
  SetButtonIcon(ti.uId,ACI_DELETE);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_RELOAD);
  ti.lpszText:=TranslateW('Reload');
  SetButtonIcon(ti.uId,ACI_RELOAD);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_TEST);
  ti.lpszText:=TranslateW('Test');
  SetButtonIcon(ti.uId,ACI_TEST);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_CNT_APPLY);
  ti.lpszText:=TranslateW('Apply format');
  SetButtonIcon(ti.uId,ACI_FORMAT);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_IMPORT);
  ti.lpszText:=TranslateW('Import');
  SetButtonIcon(ti.uId,ACI_IMPORT);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_GROUP_EXPORT);
  ti.lpszText:=TranslateW('Export');
  SetButtonIcon(ti.uId,ACI_EXPORT);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.lpszText:=TranslateW('Use Variables');
  ti.uId:=GetDlgItem(Dialog,IDC_SRV_WPAR);
  SetButtonIcon(ti.uId,ACI_VAR_UNCHECKED);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

{
  pc:=TranslateW('');
  wnd:=GetDlgItem(Dialog,IDC_NEW);
  SendMessage(hNew,BUTTONADDTOOLTIP,TWPARAM(pc),BATF_UNICODE);
  SetButtonIcon(wnd,QS_NEW);
}
  ti.uId:=GetDlgItem(Dialog,IDC_SRV_LPAR);
  SetButtonIcon(ti.uId,ACI_VAR_UNCHECKED);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId:=GetDlgItem(Dialog,IDC_SRV_SRVC);
  SetButtonIcon(ti.uId,ACI_VAR_UNCHECKED);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId:=GetDlgItem(Dialog,IDC_PRG_PRG);
  SetButtonIcon(ti.uId,ACI_VAR_UNCHECKED);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId:=GetDlgItem(Dialog,IDC_PRG_ARG);
  SetButtonIcon(ti.uId,ACI_VAR_UNCHECKED);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId:=GetDlgItem(Dialog,IDC_TXT_FILE);
  SetButtonIcon(ti.uId,ACI_VAR_UNCHECKED);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId:=GetDlgItem(Dialog,IDC_TXT_TEXT);
  SetButtonIcon(ti.uId,ACI_VAR_UNCHECKED);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId:=GetDlgItem(Dialog,IDC_RW_MVAR);
  SetButtonIcon(ti.uId,ACI_VAR_UNCHECKED);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId:=GetDlgItem(Dialog,IDC_RW_SVAR);
  SetButtonIcon(ti.uId,ACI_VAR_UNCHECKED);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId:=GetDlgItem(Dialog,IDC_RW_TVAR);
  SetButtonIcon(ti.uId,ACI_VAR_UNCHECKED);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId:=GetDlgItem(Dialog,IDC_MSG_TTL);
  SetButtonIcon(ti.uId,ACI_VAR_UNCHECKED);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId:=GetDlgItem(Dialog,IDC_MSG_TXT);
  SetButtonIcon(ti.uId,ACI_VAR_UNCHECKED);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

end;

function MoveLVItem(list:HWND;num:integer;incr:integer):integer;
var
  li:LV_ITEM;
  buf:array [0..127] of WideChar;
begin
  li.mask      :=LVIF_PARAM+LVIF_STATE+LVIF_TEXT;
  li.iItem     :=num;
  li.iSubItem  :=0;
  li.StateMask :=dword(-1);
  li.pszText   :=@buf;
  li.cchTextMax:=127;
  SendMessageW(list,LVM_GETITEMW,0,lparam(@li));
  SendMessageW(list,LVM_DELETEITEM,li.iItem,0);
  inc(li.iItem,incr);

  SendMessageW(list,LVM_INSERTITEMW,0,lparam(@li));
  SendMessageW(list,LVM_SETITEMSTATE,li.iItem,lparam(@li));
  result:=li.iItem;
end;

function MoveGroup(list:HWND;num:integer=-1;incr:integer=0):integer;
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
          if i=0 then break;
          LV_MoveItem(list,incr,i);
//          MoveLVItem(list,i,incr);
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
          if i=j then break;
          LV_MoveItem(list,incr,i);
//          MoveLVItem(list,i,incr);
          if result<0 then result:=i+incr;
        end;
      end;
    end;
  end
  else
  begin
    MoveLVItem(list,num,incr);
    result:=num;
  end;
end;

{$include i_dlglists.inc}

procedure SetNumValue(wnd:HWND;value:dword;isvar:boolean;ishex:boolean);
var
  buf:array [0..31] of WideChar;
begin
  if isvar then
    SendMessageW(wnd,WM_SETTEXT,0,value)
  else if ishex then
  begin
    buf[0]:='$';
    IntToHex(PWideChar(@buf[1]),value);
    SendMessageW(wnd,WM_SETTEXT,0,tlparam(@buf));
  end
  else
    SendMessageW(wnd,WM_SETTEXT,0,tlparam(IntToStr(buf,value)));
end;

function DlgProcOpt2(Dialog:HWnd;hMessage:uint;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;

  {$include i_visual.inc}

  procedure ShowHelp(code:integer);
  var
    buf:PAnsiChar;
    vhi:TVARHELPINFO;
  begin
    case code of
      hlpVariables: begin
        FillChar(vhi,SizeOf(vhi),0);
        with vhi do
        begin
          cbSize:=SizeOf(vhi);
          flags:=VHF_NOINPUTDLG;
        end;
        CallService(MS_VARS_SHOWHELPEX,Dialog,tlparam(@vhi));
      end;
      hlpAdvVariables: begin
        FillChar(vhi,SizeOf(vhi),0);
        with vhi do
        begin
          cbSize       :=SizeOf(vhi);
          flags        :=VHF_FULLDLG or VHF_SETLASTSUBJECT;
          hwndCtrl     :=GetDlgItem(Dialog,IDC_ADV_VARS);
          szSubjectDesc:='test your variables';
        end;
        CallService(MS_VARS_SHOWHELPEX,Dialog,tlparam(@vhi));
      end;
      hlpContact: begin
        MessageBoxW(0,
          TranslateW('Select contact to open its window'),
          TranslateW('Contacts'),0);
      end;
      hlpService: begin
        buf:=GetDlgText(Dialog,IDC_EDIT_SERVICE,true);
//!!        if buf<>nil then
        begin
          ApiCard.Service:=buf;
          mFreeMem(buf);
          ApiCard.Show;
        end;
      end;
      hlpProgram: begin
        MessageBoxW(0,
          TranslateW('Text <last> replacing'#13#10+
            'by last result'#13#10#13#10+
            'Text <param> replacing'#13#10+
            'by parameter'),
          TranslateW('Text'),0);
      end;
      hlpText: begin
        MessageBoxW(0,
          TranslateW('^s - selected (and replaced) part'#13#10+
          '^e - replaced by empty string'#13#10+
          '^v - paste text from Clipboard'#13#10+
          '^t - replaced by tabulation'#13#10+
          '^l - replaced by last result as unicode'#13#10+
          '^h - replaced by last result as hex'#13#10+
          '^a - in the end: autosend'#13#10+
          '^f(name[,str])'#13#10+
          '     paste line from text file.'#13#10+
          '     brackets contents must be w/o spaces'),
          TranslateW('Text'),0);
      end;
      hlpAdvance: begin
      end;
      hlpChain: begin
        MessageBoxW(0,
          TranslateW('You can select another group from combobox, '+
                     'then it will be executed, after that current '+
                     'action group will be continued.'),
          TranslateW('Macros'),0);
      end;
      hlpDBRW: begin
      end;
      hlpMessage: begin
        MessageBoxW(0,
          TranslateW(
            'Text <last> replacing'#13#10+
            'by last result'#13#10#13#10+
            'Returns:'#13#10+
            '--------'#13#10+
            'OK'#9'= 1'#13#10+
            'CANCEL'#9'= 2'#13#10+
            'ABORT'#9'= 3'#13#10+
            'RETRY'#9'= 4'#13#10+
            'IGNORE'#9'= 5'#13#10+
            'YES'#9'= 6'#13#10+
            'NO'#9'= 7'#13#10+
            'CLOSE'#9'= 8'),
          TranslateW('MessageBox'),0);
      end;
    end;
  end;

  // enable/disable navigation chain buttons
  procedure CheckGrpList(num:integer=-1);
  var
    wnd:HWND;
    dir:integer;
    okup,okdown:boolean;
  begin
    wnd:=GetDlgItem(Dialog,IDC_ACTION_GROUP);
//    if num<0 then
    begin
      dir:=LV_CheckDirection(wnd);
      okup  :=odd(loword(dir));
      okdown:=(loword(dir) and 2)<>0;
    end;
{
    else
    begin
      okup  :=num>0;
      okdown:=(num+1)<SendMessage(wnd,LVM_GETITEMCOUNT,0,0);
      SendMessage(wnd,LVM_ENSUREVISIBLE,num,0);
    end;
}
    EnableWindow(GetDlgItem(Dialog,IDC_GROUP_UP  ),okup);
    EnableWindow(GetDlgItem(Dialog,IDC_GROUP_DOWN),okdown);
    SendMessage(wnd,LVM_ENSUREVISIBLE,hiword(dir)-1,0);
(*
    wnd:=GetDlgItem(Dialog,IDC_ACTION_GROUP);
    if num<0 then
      num:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
    EnableWindow(GetDlgItem(Dialog,IDC_GROUP_UP),num>0);
    EnableWindow(GetDlgItem(Dialog,IDC_GROUP_DOWN),
        (num+1)<SendMessage(wnd,LVM_GETITEMCOUNT,0,0));
    SendMessage(wnd,LVM_ENSUREVISIBLE,num,0);
    result:=num;
*)
  end;
  procedure CheckActList(num:integer=-1);
  var
    wnd:HWND;
    dir:integer;
    okup,okdown:boolean;
  begin
    wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
//    if num<0 then
    begin
      dir:=LV_CheckDirection(wnd);
      okup  :=odd(loword(dir));
      okdown:=(loword(dir) and 2)<>0;
    end;
{
    else
    begin
      okup  :=num>0;
      okdown:=(num+1)<SendMessage(wnd,LVM_GETITEMCOUNT,0,0);
      SendMessage(wnd,LVM_ENSUREVISIBLE,num,0);
    end;
}
    EnableWindow(GetDlgItem(Dialog,IDC_ACTION_UP  ),okup);
    EnableWindow(GetDlgItem(Dialog,IDC_ACTION_DOWN),okdown);
    SendMessage(wnd,LVM_ENSUREVISIBLE,hiword(dir)-1,0);
(*
    wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
    if num<0 then
      num:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
    EnableWindow(GetDlgItem(Dialog,IDC_ACTION_UP),num>0);
    EnableWindow(GetDlgItem(Dialog,IDC_ACTION_DOWN),
        (num+1)<SendMessage(wnd,LVM_GETITEMCOUNT,0,0));
    SendMessage(wnd,LVM_ENSUREVISIBLE,num,0);
*)
  end;

  // change current action name (by type)
  procedure ChangeActionName(num:integer=-1;acttype:integer=0;str:PWideChar=nil);
  var
    li:LV_ITEMW;
    wnd:HWND;
    str1:pWideChar;
  begin
    wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
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
        if num>=0 then // new item - screen only
          str1:=TranslateW(ActionNames[ACT_CONTACT])
        else // change action type
        begin
          str1:=NewActionList[li.lParam].descr;
          if str1=nil then // not in memory yet
            str1:=TranslateW(ActionNames[acttype])
          else
            exit;
        end;
      end
      else  // rename
      begin
        str1:=str;
        mFreeMem(NewActionList[li.lParam].descr);
        StrDupW (NewActionList[li.lParam].descr,str);
      end;

// screen
      li.mask   :=LVIF_TEXT;
      li.pszText:=str1;
      SendMessageW(wnd,LVM_SETITEMW,0,tlparam(@li));
    end;
  end;

  // Fill Chain list
  procedure MakeActionList(HK:pHKRecord);
  var
    i,idx:integer;
    wnd:HWND;
    li:LV_ITEMW;
    p:pWideChar;
  begin
    wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
    SendMessage(wnd,LVM_DELETEALLITEMS,0,0);

    EnableWindow(GetDlgItem(Dialog,IDC_ACTION_UP),false);
    if (HK=nil) or (HK^.firstAction=0) then
    begin
      SHWindows;
      SHActButtons(SW_HIDE);
      EnableWindow(GetDlgItem(Dialog,IDC_ACTION_LIST  ),false);
      EnableWindow(GetDlgItem(Dialog,IDC_GROUP_TEST   ),false);
      EnableWindow(GetDlgItem(Dialog,IDC_ACTION_HELP  ),false);
      EnableWindow(GetDlgItem(Dialog,IDC_ACTION_DELETE),false);
      EnableWindow(GetDlgItem(Dialog,IDC_ACTION_DOWN  ),false);
      exit;
    end;
    SHActButtons(SW_SHOW);

    li.mask    :=LVIF_TEXT or LVIF_PARAM;
    li.iSubitem:=0;
    i:=HK^.firstAction;
    idx:=0;
    repeat
      p:=NewActionList^[i].descr;
      if p=nil then
        p:=TranslateW(ActionNames[NewActionList^[i].actionType]);
      li.pszText:=p;
      li.iItem  :=idx;
      li.lParam :=i;
      SendMessageW(wnd,LVM_INSERTITEMW,0,tlparam(@li));
      ListView_SetCheckState(wnd,idx,(NewActionList^[i].flags and ACF_DISABLED)=0);

      i:=NewActionList^[i].next;
      inc(idx);
    until i=0;

    Listview_SetItemState(wnd,0,
        LVIS_FOCUSED or LVIS_SELECTED,
        LVIS_FOCUSED or LVIS_SELECTED);
    EnableWindow(GetDlgItem(Dialog,IDC_ACTION_LIST  ),true);
    EnableWindow(GetDlgItem(Dialog,IDC_GROUP_TEST   ),true);
    EnableWindow(GetDlgItem(Dialog,IDC_ACTION_HELP  ),true);
    EnableWindow(GetDlgItem(Dialog,IDC_ACTION_DELETE),true);
    EnableWindow(GetDlgItem(Dialog,IDC_ACTION_DOWN),idx>1);
//    FillSubList(Dialog);
    FillAction(HK^.firstAction);
    CheckActList(0);
  end;

  procedure SaveAction(listnum,actnum:integer);
  var
    wnd:HWND;
    i:integer;
    tmp:pWideChar;
    arr: array [0..255] of WideChar;
    li:LV_ITEMW;
  begin
    if (ChMask and ACTM_ACT)=0 then exit;
    ChMask:=ChMask and not ACTM_ACT;
    wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
    if listnum<0 then
    begin
      listnum:=SendMessageW(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
      if listnum<0 then
        exit;
    end;
    if actnum<0 then
      actnum:=LV_GetLParam(wnd,listnum);
    StrDupW(tmp,NewActionList^[actnum].descr); // keeping old name
    FreeAction(@NewActionList^[actnum]);
    with NewActionList^[actnum] do
    begin
      flags :=ACF_ASSIGNED;
      flags2:=0;

      descr:=tmp;
      if descr=nil then
      begin
        li.iItem     :=listnum;
        li.mask      :=LVIF_TEXT;
        li.iSubItem  :=0;
        li.pszText   :=@arr;
        li.cchTextMax:=SizeOf(arr) div SizeOf(WideChar);
        SendDlgItemMessageW(Dialog,IDC_ACTION_LIST,LVM_GETITEMW,0,tlparam(@li));
        StrDupW(descr,arr);
      end;

      if ListView_GetCheckState(wnd,listnum)=0 then
        flags:=flags or ACF_DISABLED;

      actionType:=ActIds[SendDlgItemMessage(Dialog,IDC_ACTION_TYPE,CB_GETCURSEL,0,0)].code;
      case actionType of
        ACT_CONTACT: begin
          contact:=SendDlgItemMessage(Dialog,IDC_CONTACTLIST,CB_GETITEMDATA,
              SendDlgItemMessage(Dialog,IDC_CONTACTLIST,CB_GETCURSEL,0,0),0);
          if IsDlgButtonChecked(Dialog,IDC_CNT_KEEP)=BST_CHECKED then
            flags:=flags or ACF_KEEPONLY;
        end;

        ACT_SERVICE: begin
          if IsDlgButtonChecked(Dialog,IDC_SRV_WPAR)=BST_CHECKED then
            flags2:=flags2 or ACF2_SRV_WPAR;
          if IsDlgButtonChecked(Dialog,IDC_SRV_LPAR)=BST_CHECKED then
            flags2:=flags2 or ACF2_SRV_LPAR;
          if IsDlgButtonChecked(Dialog,IDC_SRV_SRVC)=BST_CHECKED then
            flags2:=flags2 or ACF2_SRV_SRVC;

          case CB_GetData(GetDlgItem(Dialog,IDC_FLAG_WPAR)) of
            ptParam: begin
              flags:=flags or ACF_WPARAM
            end;
            ptResult: begin
              flags:=flags or ACF_WRESULT
            end;
            ptCurrent: begin
              flags:=flags or ACF_WPARNUM or ACF_WCURRENT
            end;
            ptNumber: begin
              flags:=flags or ACF_WPARNUM;
              if GetNumValue(GetDlgItem(Dialog,IDC_EDIT_WPAR),
                 (flags2 and ACF2_SRV_WPAR)<>0,wparam) then
                flags2:=flags2 or ACF2_SRV_WHEX;
//              wparam:=GetDlgItemInt(Dialog,IDC_EDIT_WPAR,pbool(nil)^,true);
            end;
            ptStruct: begin
              flags:=flags or ACF_WSTRUCT;
              StrDup(pAnsiChar(wparam),wstruct);
            end;
            ptUnicode: begin
              flags:=flags or ACF_WUNICODE;
              pointer(wparam):=GetDlgText(Dialog,IDC_EDIT_WPAR);
            end;
            ptString: pointer(wparam):=GetDlgText(Dialog,IDC_EDIT_WPAR,true);
          end;

          case CB_GetData(GetDlgItem(Dialog,IDC_FLAG_LPAR)) of
            ptParam: begin
              flags:=flags or ACF_LPARAM
            end;
            ptResult: begin
              flags:=flags or ACF_LRESULT
            end;
            ptCurrent: begin
              flags:=flags or ACF_LPARNUM or ACF_LCURRENT
            end;
            ptNumber: begin
              flags:=flags or ACF_LPARNUM;
              if GetNumValue(GetDlgItem(Dialog,IDC_EDIT_LPAR),
                 (flags2 and ACF2_SRV_LPAR)<>0,lparam) then
                flags2:=flags2 or ACF2_SRV_LHEX;
//              lparam:=GetDlgItemInt(Dialog,IDC_EDIT_LPAR,pbool(nil)^,true);
            end;
            ptStruct: begin
              flags:=flags or ACF_LSTRUCT;
              StrDup(pAnsiChar(lparam),lstruct);
            end;
            ptUnicode: begin
              flags:=flags or ACF_LUNICODE;
              pointer(lparam):=GetDlgText(Dialog,IDC_EDIT_LPAR);
            end;
            ptString: pointer(lparam):=GetDlgText(Dialog,IDC_EDIT_LPAR,true);
          end;

          if IsDlgButtonChecked(Dialog,IDC_RES_INSERT)=BST_CHECKED then
            flags:=flags or ACF_INSERT;
          if IsDlgButtonChecked(Dialog,IDC_RES_MESSAGE)=BST_CHECKED then
            flags:=flags or ACF_MESSAGE;
          if IsDlgButtonChecked(Dialog,IDC_RES_POPUP)=BST_CHECKED then
            flags:=flags or ACF_POPUP;

          case CB_GetData(GetDlgItem(Dialog,IDC_SRV_RESULT)) of
            sresHex: flags:=flags or ACF_HEX;
            sresInt: begin
              if IsDlgButtonChecked(Dialog,IDC_RES_SIGNED)=BST_CHECKED then
                flags:=flags or ACF_SIGNED;
            end;
            sresString: begin
              flags:=flags or ACF_STRING;
              if IsDlgButtonChecked(Dialog,IDC_RES_UNICODE)=BST_CHECKED then
                flags:=flags or ACF_UNICODE;
              if IsDlgButtonChecked(Dialog,IDC_RES_FREEMEM)=BST_CHECKED then
                flags2:=flags2 or ACF2_FREEMEM;
            end;
            sresStruct: flags:=flags or ACF_STRUCT;
          end;

          service:=GetDlgText(Dialog,IDC_EDIT_SERVICE,true);

        end;

        ACT_PROGRAM: begin
          prgname:=GetDlgText(Dialog,IDC_EDIT_PRGPATH);
          args:=GetDlgText(Dialog,IDC_EDIT_PRGARGS);
          if IsDlgButtonChecked(Dialog,IDC_FLAG_PARALLEL)=BST_CHECKED then
            flags:=flags or ACF_PRTHREAD;
          if IsDlgButtonChecked(Dialog,IDC_FLAG_CURPATH)=BST_CHECKED then
            flags:=flags or ACF_CURPATH;
          time:=GetDlgItemInt(Dialog,IDC_EDIT_PROCTIME,pbool(nil)^,false);
          if IsDlgButtonChecked(Dialog,IDC_FLAG_MINIMIZE)=BST_CHECKED then
            show:=SW_SHOWMINIMIZED
          else if IsDlgButtonChecked(Dialog,IDC_FLAG_MAXIMIZE)=BST_CHECKED then
            show:=SW_SHOWMAXIMIZED
          else if IsDlgButtonChecked(Dialog,IDC_FLAG_HIDDEN)=BST_CHECKED then
            show:=SW_HIDE
          else //if IsDlgButtonChecked(Dialog,IDC_FLAG_NORMAL)=BST_CHECKED then
            show:=SW_SHOWNORMAL;

          if IsDlgButtonChecked(Dialog,IDC_PRG_PRG)=BST_CHECKED then
            flags2:=flags2 or ACF2_PRG_PRG;
          if IsDlgButtonChecked(Dialog,IDC_PRG_ARG)=BST_CHECKED then
            flags2:=flags2 or ACF2_PRG_ARG;
        end;

        ACT_TEXT: begin
          if IsDlgButtonChecked(Dialog,IDC_FLAG_CLIP)<>BST_UNCHECKED then
          begin
            flags:=flags or ACF_CLIPBRD;
            if IsDlgButtonChecked(Dialog,IDC_CLIP_COPYTO)<>BST_UNCHECKED then
              flags:=flags or ACF_COPYTO;
          end
          else
          begin
            if IsDlgButtonChecked(Dialog,IDC_TXT_TEXT)=BST_CHECKED then
              flags2:=flags2 or ACF2_TXT_TEXT;
            text:=GetDlgText(Dialog,IDC_EDIT_INSERT);
            if IsDlgButtonChecked(Dialog,IDC_FLAG_FILE)<>BST_UNCHECKED then
            begin
              flags:=flags or ACF_FILE;
              case CB_GetData(GetDlgItem(Dialog,IDC_FILE_ENC)) of
                0: flags:=flags or ACF_ANSI;
                1: flags:=flags or ACF_UTF8;
                2: flags:=flags or ACF_UTF8 or ACF_SIGN;
                3: flags:=flags or 0;
                4: flags:=flags or ACF_SIGN;
              end;

              if IsDlgButtonChecked(Dialog,IDC_TXT_FILE)=BST_CHECKED then
                flags2:=flags2 or ACF2_TXT_FILE;
              tfile:=GetDlgText(Dialog,IDC_FILE_PATH);
              if IsDlgButtonChecked(Dialog,IDC_FILE_APPEND)<>BST_UNCHECKED then
                flags:=flags or ACF_FAPPEND
              else if IsDlgButtonChecked(Dialog,IDC_FILE_WRITE)<>BST_UNCHECKED then
                flags:=flags or ACF_FWRITE;
            end;
          end;
        end;

        ACT_ADVANCE: begin
          condition:=ADV_COND_NOP;
          if      IsDlgButtonChecked(Dialog,IDC_FLAG_GT )=BST_CHECKED then
            condition:=ADV_COND_GT
          else if IsDlgButtonChecked(Dialog,IDC_FLAG_LT )=BST_CHECKED then
            condition:=ADV_COND_LT
          else if IsDlgButtonChecked(Dialog,IDC_FLAG_EQ )=BST_CHECKED then
            condition:=ADV_COND_EQ;
          value:=GetDlgItemInt(Dialog,IDC_ADV_VALUE,pbool(nil)^,false);

          if IsDlgButtonChecked(Dialog,IDC_FLAG_NOT)=BST_CHECKED then
            condition:=condition or ADV_COND_NOT;

          if      IsDlgButtonChecked(Dialog,IDC_FLAG_BREAK)=BST_CHECKED then
            action:=ADV_ACT_BREAK
          else if IsDlgButtonChecked(Dialog,IDC_FLAG_JUMP )=BST_CHECKED then
            action:=ADV_ACT_JUMP
          else
            action:=ADV_ACT_NOP;

          case action of
            ADV_ACT_JUMP: operval:=GetDlgText(Dialog,IDC_ADV_VAL2);
          end;

          if IsDlgButtonChecked(Dialog,IDC_FLAG_VARS)<>BST_UNCHECKED then
          begin
            varval:=GetDlgText(Dialog,IDC_ADV_VARS);
            action:=action or ADV_ACT_VARS;
            if IsDlgButtonChecked(Dialog,IDC_ADV_ASINT)<>BST_UNCHECKED then
              flags:=flags or ACF_VARASINT;
          end
;{//!!executively!!
          else }if IsDlgButtonChecked(Dialog,IDC_FLAG_MATH)=BST_CHECKED then
          begin
            mathval:=GetDlgItemInt(Dialog,IDC_ADV_VAL1,pbool(nil)^,true);
            action :=action or ADV_ACT_MATH;
            oper   :=CB_GetData(GetDlgItem(Dialog,IDC_ADV_OPER));
//            oper   :=SendDlgItemMessage(Dialog,IDC_ADV_OPER,CB_GETCURSEL,0,0);
          end;
        end;

        ACT_CHAIN: begin
           wnd:=GetDlgItem(Dialog,IDC_GROUP_LIST);
           i:=SendMessage(wnd,CB_GETCURSEL,0,0);
           if i>0 then
             id:=SendMessage(wnd,CB_GETITEMDATA,i,0)
           else
             id:=0;
        end;

        ACT_RW: begin
          if IsDlgButtonChecked(Dialog,IDC_RW_CURRENT)=BST_CHECKED then
            flags:=flags or ACF_CURRENT
          else if IsDlgButtonChecked(Dialog,IDC_RW_RESULT)=BST_CHECKED then
            flags:=flags or ACF_RESULT
          else if IsDlgButtonChecked(Dialog,IDC_RW_PARAM)=BST_CHECKED then
            flags:=flags or ACF_PARAM
          else
            dbcontact:=SendDlgItemMessage(Dialog,IDC_CONTACTLIST,CB_GETITEMDATA,
                SendDlgItemMessage(Dialog,IDC_CONTACTLIST,CB_GETCURSEL,0,0),0);
          dbmodule :=GetDlgText(Dialog,IDC_RW_MODULE ,true);
          dbsetting:=GetDlgText(Dialog,IDC_RW_SETTING,true);
          if IsDlgButtonChecked(Dialog,IDC_RW_MVAR)=BST_CHECKED then
            flags2:=flags2 or ACF2_RW_MVAR;
          if IsDlgButtonChecked(Dialog,IDC_RW_SVAR)=BST_CHECKED then
            flags2:=flags2 or ACF2_RW_SVAR;

          if IsDlgButtonChecked(Dialog,IDC_RW_WRITE)=BST_CHECKED then
            flags:=flags or ACF_DBWRITE
          else if IsDlgButtonChecked(Dialog,IDC_RW_DELETE)=BST_CHECKED then
            flags:=flags or ACF_DBDELETE;

          if IsDlgButtonChecked(Dialog,IDC_RW_LAST)=BST_CHECKED then
            flags:=flags or ACF_LAST;

          if IsDlgButtonChecked(Dialog,IDC_RW_TVAR)=BST_CHECKED then
            flags2:=flags2 or ACF2_RW_TVAR;

          i:=CB_GetData(GetDlgItem(Dialog,IDC_RW_DATATYPE));
          case i of
            0: flags:=flags or ACF_DBBYTE;
            1: flags:=flags or ACF_DBWORD;
            2: flags:=flags or 0;
            3: flags:=flags or ACF_DBANSI;
            4: flags:=flags or ACF_DBUTEXT;
          end;
          if i<3 then
          begin
            if (flags and ACF_LAST)=0 then
            begin
              if GetNumValue(GetDlgItem(Dialog,IDC_RW_VALUE),
                 (flags2 and ACF2_RW_TVAR)<>0,dbvalue) then
                flags2:=flags2 or ACF2_RW_HEX;
            end;
          end
          else
          begin
            if (flags and ACF_LAST)=0 then
              pWideChar(dbvalue):=GetDlgText(Dialog,IDC_RW_TEXT);
          end;

          if (IsDlgButtonChecked(Dialog,IDC_RW_LAST)=BST_CHECKED) or
             ((flags and ACF_LAST)<>0) then
            flags:=flags or ACF_LAST;
        end;

        ACT_MESSAGE: begin
          pWideChar(msgtitle):=GetDlgText(Dialog,IDC_MSG_TITLE);
          pWideChar(msgtext ):=GetDlgText(Dialog,IDC_MSG_TEXT);
          if IsDlgButtonChecked(Dialog,IDC_MSG_KEEP)=BST_CHECKED then
            flags:=flags or ACF_MSG_KEEP;
          if IsDlgButtonChecked(Dialog,IDC_MSG_TTL)=BST_CHECKED then
            flags2:=flags2 or ACF2_MSG_TTL;
          if IsDlgButtonChecked(Dialog,IDC_MSG_TXT)=BST_CHECKED then
            flags2:=flags2 or ACF2_MSG_TXT;

          if      IsDlgButtonChecked(Dialog,IDC_MSGB_OC )=BST_CHECKED then boxopts:=MB_OKCANCEL
          else if IsDlgButtonChecked(Dialog,IDC_MSGB_ARI)=BST_CHECKED then boxopts:=MB_ABORTRETRYIGNORE
          else if IsDlgButtonChecked(Dialog,IDC_MSGB_YNC)=BST_CHECKED then boxopts:=MB_YESNOCANCEL
          else if IsDlgButtonChecked(Dialog,IDC_MSGB_YN )=BST_CHECKED then boxopts:=MB_YESNO
          else if IsDlgButtonChecked(Dialog,IDC_MSGB_RC )=BST_CHECKED then boxopts:=MB_RETRYCANCEL
          else{if IsDlgButtonChecked(Dialog,IDC_MSGB_OK )=BST_CHECKED then}boxopts:=MB_OK;

          if      IsDlgButtonChecked(Dialog,IDC_MSGI_ERROR)=BST_CHECKED then boxopts:=boxopts or MB_ICONHAND
          else if IsDlgButtonChecked(Dialog,IDC_MSGI_QUEST)=BST_CHECKED then boxopts:=boxopts or MB_ICONQUESTION
          else if IsDlgButtonChecked(Dialog,IDC_MSGI_WARN )=BST_CHECKED then boxopts:=boxopts or MB_ICONWARNING
          else if IsDlgButtonChecked(Dialog,IDC_MSGI_INFO )=BST_CHECKED then boxopts:=boxopts or MB_ICONINFORMATION
          ;//else if IsDlgButtonChecked(Dialog,IDC_MSGI_NONE)=BST_CHECKED then ;
        end;

      end;
    end;
  end;

  procedure BuildActionChain(group:integer=-1);
  var
    i,j,item:integer;
    wnd:HWND;
    act:pHKAction;
    idx:integer;
  begin
    if (ChMask and ACTM_ACTS)=0 then exit;
    ChMask:=ChMask and not ACTM_ACTS;
    SaveAction(-1,-1);
    wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
    i:=SendMessageW(wnd,LVM_GETITEMCOUNT,0,0);
    idx:=LV_GetLParam(GetDlgItem(Dialog,IDC_ACTION_GROUP),group);
    if i>0 then
    begin
      j:=LV_GetLParam(wnd,0);
      NewGroupList^[idx].firstAction:=j;
      act:=@NewActionList^[j];
      for item:=1 to i-1 do
      begin
        j:=LV_GetLParam(wnd,item);
        act^.next:=j;
        act:=@NewActionList^[j];
      end;
      act^.next:=0;
    end
    else
    begin
      if idx>=0 then
        NewGroupList^[idx].firstAction:=0;
    end;
  end;

  procedure CheckActionList(next:integer);
  var
    i:integer;
    wnd:HWND;
  begin
    wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
    i:=SendMessage(wnd,LVM_GETITEMCOUNT,0,0);
    if i>0 then
    begin
      EnableWindow(GetDlgItem(Dialog,IDC_ACTION_HELP),true);
      if next=i then
        dec(next);
      ListView_SetItemState(wnd,next,
          LVIS_FOCUSED or LVIS_SELECTED,
          LVIS_FOCUSED or LVIS_SELECTED);
    end
    else
    begin
      SHWindows;
      SHActButtons(SW_HIDE);
      EnableWindow(wnd,false);
      EnableWindow(GetDlgItem(Dialog,IDC_ACTION_HELP  ),false);
      EnableWindow(GetDlgItem(Dialog,IDC_GROUP_TEST   ),false);
      EnableWindow(GetDlgItem(Dialog,IDC_ACTION_DELETE),false);
      next:=0;
    end;
    CheckActList(next);
  end;

  procedure CheckGroupList(next:integer);
  var
    i:integer;
    wnd:HWND;
    li:LV_ITEMW;
  begin
    wnd:=GetDlgItem(Dialog,IDC_ACTION_GROUP);
    i:=SendMessage(wnd,LVM_GETITEMCOUNT,0,0);
    if i>0 then
    begin
      if next=i then
        dec(next);
      ListView_SetItemState(wnd,next,LVIS_FOCUSED or LVIS_SELECTED,
        LVIS_FOCUSED or LVIS_SELECTED);
      li.iItem   :=next;
      li.iSubItem:=0;
      li.mask    :=LVIF_PARAM;
      SendMessage(wnd,LVM_GETITEM,0,tlparam(@li));
    end
    else
    begin
      EnableWindow(wnd,false);
      EnableWindow(GetDlgItem(Dialog,IDC_GROUP_DELETE),false);
      CheckActionList(0);
      EnableWindow(GetDlgItem(Dialog,IDC_ACTION_NEW),false);
    end;
    CheckGrpList(next);
  end;

  // Fill action group list and disable chain controls
  function FillGroupList{(Dialog:hwnd)}:integer;
  var
    CurGroup:pHKRecord;
    i:integer;
    list:HWND;
    lvi:TLVITEMW;
  begin
    SendDlgItemMessage(Dialog,IDC_ACTION_LIST,LVM_DELETEALLITEMS,0,0);
    CheckActionList(-1);
    list:=GetDlgItem(Dialog,IDC_ACTION_GROUP);

    SendMessage(list,LVM_DELETEALLITEMS,0,0);
    CurGroup:=@NewGroupList^;
    result:=-1;
    lvi.mask:=LVIF_TEXT+LVIF_PARAM;
    lvi.iSubItem:=0;
    for i:=0 to NewMaxGroups-1 do
    begin
      if (CurGroup^.flags and ACF_ASSIGNED)<>0 then
      begin
        lvi.iItem :=i;
        lvi.lParam:=i;
        if CurGroup^.descr=nil then
          lvi.pszText:=NoDescription
        else
          lvi.pszText:=CurGroup^.descr;
        SendMessageW(list,LVM_INSERTITEMW,0,tlparam(@lvi));
        inc(result);
      end;
      inc(CurGroup);
    end;

    EnableWindow(GetDlgItem(Dialog,IDC_ACTION_GROUP ),result>=0);
    EnableWindow(GetDlgItem(Dialog,IDC_ACTION_LIST  ),result>=0);
    EnableWindow(GetDlgItem(Dialog,IDC_ACTION_NEW   ),result>=0);
    EnableWindow(GetDlgItem(Dialog,IDC_GROUP_DELETE ),result>=0);
    if result<0 then
    begin
      EnableWindow(GetDlgItem(Dialog,IDC_GROUP_TEST   ),false);
      EnableWindow(GetDlgItem(Dialog,IDC_ACTION_HELP  ),false);
      EnableWindow(GetDlgItem(Dialog,IDC_ACTION_DELETE),false);
      EnableWindow(GetDlgItem(Dialog,IDC_ACTION_UP    ),false);
      EnableWindow(GetDlgItem(Dialog,IDC_ACTION_DOWN  ),false);
      EnableWindow(GetDlgItem(Dialog,IDC_STAT_GROUPS  ),false);
      EnableWindow(GetDlgItem(Dialog,IDC_ACTION_TYPE  ),false);
    end
    else
      result:=0;
    SendMessage(list,CB_SETCURSEL,0,0);
    SendMessage(list,LVM_SETCOLUMNWIDTH,0,LVSCW_AUTOSIZE_USEHEADER);

    ListView_SetItemState(list,0,
      LVIS_FOCUSED or LVIS_SELECTED,
      LVIS_FOCUSED or LVIS_SELECTED);
  end;

  procedure SHMath(show:boolean);
  var
    wnd:HWND;
  begin
    EnableWindow(GetDlgItem(Dialog,IDC_ADV_ASINT),not show);
    EnableWindow(GetDlgItem(Dialog,IDC_ADV_VARS ),not show);

    wnd:=GetDlgItem(Dialog,IDC_ADV_OPER);
    EnableWindow(wnd,show);
    EnableWindow(GetDlgItem(Dialog,IDC_ADV_VAL1),
       show and (CB_GetData(wnd)<>lresult(aeNot)));//(SendMessage(wnd,CB_GETCURSEL,0,0)<>0));
  end;

  procedure SetMBRadioIcon(h:THANDLE;id:dword;icon:uint_ptr);
  begin
    SendDlgItemMessage(Dialog,id,BM_SETIMAGE,IMAGE_ICON,
      LoadImage(h,MAKEINTRESOURCE(icon),IMAGE_ICON,16,16,0{LR_SHARED}));
  //  SendDlgItemMessage(Dialog,id,BM_SETIMAGE,IMAGE_ICON,LoadIcon(0,icon));
  end;

  procedure SetMBRadioIcons;
  var
    h:THANDLE;
  begin
    h:=LoadLibrary('user32.dll');
  //  SetMBRadioIcon(IDC_MSGI_NONE,IDI_); //?
    SetMBRadioIcon(h,IDC_MSGI_ERROR,103{IDI_HAND});
    SetMBRadioIcon(h,IDC_MSGI_QUEST,102{IDI_QUESTION});
    SetMBRadioIcon(h,IDC_MSGI_WARN ,101{IDI_EXCLAMATION});
    SetMBRadioIcon(h,IDC_MSGI_INFO ,104{IDI_ASTERISK});
    FreeLibrary(h);
  end;

  procedure FillFileName(idc:integer);
  var
    pw,ppw:pWideChar;
  begin
    mGetMem(pw,1024*SizeOf(WideChar));
    ppw:=GetDlgText(Dialog,idc);
    if ShowDlgW(pw,ppw) then
      SetDlgItemTextW(Dialog,idc,pw);
    mFreeMem(ppw);
    mFreeMem(pw);
  end;

var
  wnd,wnd1,wnd2:HWND;
  i,j:int_ptr;
  lvflag:integer;
  pc:pAnsiChar;
  li:LV_ITEMW;
  lv:LV_COLUMNW;
  b:boolean;
  ico:HICON;
begin
  result:=0;
  case hMessage of
    WM_DESTROY: begin
      ApiCard.Free;
      SetCancel;
      mFreeMem(wstruct);
      mFreeMem(lstruct);
    end;

    WM_INITDIALOG: begin
      ApiCard:=CreateServiceCard(Dialog);
      wstruct:=nil;
      lstruct:=nil;
      SetStart;
      DontReact :=true;
      SHWindows;
      TranslateDialogDefault(Dialog);
      SetButtonIcons2(Dialog);
      wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
      SendMessage(wnd,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_CHECKBOXES,LVS_EX_CHECKBOXES);
      SendMessage(wnd,LVM_SETUNICODEFORMAT,1,0);
      zeromemory(@lv,sizeof(lv));
      lv.mask:=LVCF_WIDTH;
      lv.cx  :=110;
      SendMessageW(wnd,LVM_INSERTCOLUMNW ,0,tlparam(@lv));
      SendMessageW(wnd,LVM_SETCOLUMNWIDTH,0,LVSCW_AUTOSIZE_USEHEADER);

      MakeResultTypeList(GetDlgItem(Dialog,IDC_SRV_RESULT));
      MakeParamTypeList (GetDlgItem(Dialog,IDC_FLAG_WPAR));
      MakeParamTypeList (GetDlgItem(Dialog,IDC_FLAG_LPAR));
      MakeMathOperList  (GetDlgItem(Dialog,IDC_ADV_OPER));
      MakeFileEncList   (GetDlgItem(Dialog,IDC_FILE_ENC));
      MakeDataTypeList  (GetDlgItem(Dialog,IDC_RW_DATATYPE));

      // service list for RunService
      ApiCard.FillList(GetDlgItem(Dialog,IDC_EDIT_SERVICE));
      // contact list for ContactMessage
      FillContactList(GetDlgItem(Dialog,IDC_CONTACTLIST),fCLfilter,fCLformat);
      // action type combobox
      FillActTypeList(GetDlgItem(Dialog,IDC_ACTION_TYPE));

      if isVarsInstalled then
      begin
        ico:=CallService(MS_VARS_GETSKINITEM,0,VSI_HELPICON);
        SendDlgItemMessage(Dialog,IDC_HLP_FVARS,BM_SETIMAGE,IMAGE_ICON,ico);
        SendDlgItemMessage(Dialog,IDC_HLP_VARS ,BM_SETIMAGE,IMAGE_ICON,ico);
        SendDlgItemMessage(Dialog,IDC_ADV_HVARS,BM_SETIMAGE,IMAGE_ICON,ico);
      end;

      if ServiceExists(MS_SYSTEM_GET_XI)=0 then
      begin
        EnableWindow(GetDlgItem(Dialog,IDC_GROUP_EXPORT),false);
        EnableWindow(GetDlgItem(Dialog,IDC_GROUP_IMPORT),false);
      end;

      OldActTableProc  :=pointer(SetWindowLongPtrW(wnd,GWL_WNDPROC,long_ptr(@NewActTableProc)));
      OldGroupTableProc:=pointer(SetWindowLongPtrW(GetDlgItem(Dialog,IDC_ACTION_GROUP),
            GWL_WNDPROC,long_ptr(@NewGroupTableProc)));

      // fill group list
      wnd:=GetDlgItem(Dialog,IDC_ACTION_GROUP);
      SendMessage(wnd,LVM_SETUNICODEFORMAT,1,0);
      lv.mask:=LVCF_WIDTH;
      lv.cx  :=110;
      SendMessageW(wnd,LVM_INSERTCOLUMNW,0,tlparam(@lv));
      FillGroupList{(Dialog)};
      FillChainList(Dialog);

      SetMBRadioIcons;

      // fill current group
      MakeActionList(@NewGroupList^);
      ChMask:=0;
      CheckGrpList(-1);
      DontReact:=false;
    end;

    WM_COMMAND: begin
      if DontReact then exit;
      case wParam shr 16 of
        CBN_EDITCHANGE: begin
          ChMask:=ChMask or ACTM_ACT or ACTM_ACTS;
          SetChanged(Dialog,etACT);
        end;
        EN_CHANGE: begin
// check for group renaming
          if loword(wParam)<>IDC_EDIT_FORMAT then
          begin
            ChMask:=ChMask or ACTM_ACT or ACTM_ACTS;
            SetChanged(Dialog,etACT);
          end;
        end;
        CBN_SELCHANGE:  begin
          ChMask:=ChMask or ACTM_ACT or ACTM_ACTS;
          SetChanged(Dialog,etACT);
          case loword(wParam) of
            IDC_SRV_RESULT: begin
              i:=CB_GetData(lParam);
              case i of
                sresHex,sresInt,sresStruct: begin
                  SHControl(IDC_RES_FREEMEM,SW_HIDE);
                  SHControl(IDC_RES_UNICODE,SW_HIDE);
                  if i=sresInt then
                    SHControl(IDC_RES_SIGNED,SW_SHOW)
                  else
                    SHControl(IDC_RES_SIGNED,SW_HIDE);
                end;
                sresString: begin
                  SHControl(IDC_RES_FREEMEM,SW_SHOW);
                  SHControl(IDC_RES_UNICODE,SW_SHOW);
                  SHControl(IDC_RES_SIGNED ,SW_HIDE);
                end;
              end;
            end;

            IDC_ADV_OPER: begin
              EnableWindow(GetDlgItem(Dialog,IDC_ADV_VAL1),
                (CB_GetData(lParam)<>lresult(aeNot)));
//                SendMessage(lParam,CB_GETCURSEL,0,0)<>0);
            end;

            IDC_RW_DATATYPE: begin
              if CB_GetData(GetDlgItem(Dialog,IDC_RW_DATATYPE))>2 then
              begin
                SHControl(IDC_RW_TEXT ,SW_SHOW);
                SHControl(IDC_RW_VALUE,SW_HIDE);
              end
              else
              begin
                SHControl(IDC_RW_TEXT ,SW_HIDE);
                SHControl(IDC_RW_VALUE,SW_SHOW);
              end;
            end;

            IDC_ACTION_TYPE: begin
              i:=SendMessage(lParam,CB_GETCURSEL,0,0);
              SHWindows(ActIds[i].code);
              ChangeActionName(-1,i+1);
              case ActIds[i].code of
                ACT_CONTACT: EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),true);
                ACT_RW: begin
                  EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),
                    (IsDlgButtonChecked(Dialog,IDC_RW_MANUAL)=BST_CHECKED));

                  if CB_GetData(GetDlgItem(Dialog,IDC_RW_DATATYPE))>2 then
                  begin
                    SHControl(IDC_RW_TEXT ,SW_SHOW);
                    SHControl(IDC_RW_VALUE,SW_HIDE);
                  end
                  else
                  begin
                    SHControl(IDC_RW_TEXT ,SW_HIDE);
                    SHControl(IDC_RW_VALUE,SW_SHOW);
                  end;

                end;
                ACT_ADVANCE: FillSubList(Dialog);
              end;
            end;
{
            IDC_EDIT_WPAR,IDC_EDIT_LPAR: begin
              SendMessage(lParam,CB_GETLBTEXT,
                 SendMessage(lParam,CB_GETCURSEL,0,0),TLPARAM(@buf));
              if loword(wParam)=IDC_EDIT_WPAR then
                FixParam(buf,IDC_EDIT_WPAR,IDC_FLAG_WPAR)
              else
                FixParam(buf,IDC_EDIT_LPAR,IDC_FLAG_LPAR)
            end;
}
            IDC_FLAG_WPAR,IDC_FLAG_LPAR: begin
              if loword(wParam)=IDC_FLAG_WPAR then
              begin
                wnd :=GetDlgItem(Dialog,IDC_EDIT_WPAR);
                wnd1:=GetDlgItem(Dialog,IDC_WSTRUCT);
                wnd2:=GetDlgItem(Dialog,IDC_SRV_WPAR);
              end
              else
              begin
                wnd :=GetDlgItem(Dialog,IDC_EDIT_LPAR);
                wnd1:=GetDlgItem(Dialog,IDC_LSTRUCT);
                wnd2:=GetDlgItem(Dialog,IDC_SRV_LPAR);
              end;
              i:=CB_GetData(GetDlgItem(Dialog,loword(wParam)));

              if i=ptStruct then
              begin
                ShowWindow(wnd ,SW_HIDE);
                ShowWindow(wnd2,SW_HIDE);
                ShowWindow(wnd1,SW_SHOW);
              end
              else
              begin
                ShowWindow(wnd ,SW_SHOW);
                ShowWindow(wnd2,SW_SHOW);
                ShowWindow(wnd1,SW_HIDE);
                if i in [ptCurrent,ptResult,ptParam] then
                  EnableWindow(wnd,false)
                else
                begin
                  EnableWindow(wnd,true);
{
                flag:=GetWindowLongPtr(wnd,GWL_STYLE);
                if i=ptNumber then
                  flag:=flag or ES_NUMBER
                else
                  flag:=flag and not ES_NUMBER;
                SetWindowLongPtr(wnd,GWL_STYLE,flag);
}
                end;
              end;
            end;
            IDC_EDIT_SERVICE: ReloadService;
          end;
        end;
        BN_CLICKED: begin
          case loword(wParam) of
            IDC_GROUP_RELOAD, // don't affect to saved (DB) datas
            IDC_WSTRUCT,      // 'Changed' on process
            IDC_LSTRUCT,      // 'Changed' on process
            IDC_HLP_FVARS,
            IDC_HLP_VARS,
            IDC_ADV_HVARS,
            IDC_ACTION_HELP,
            IDC_GROUP_EXPORT,
            IDC_CNT_APPLY,
            IDC_CNT_FILTER,
            IDC_GROUP_TEST: ;

            IDC_GROUP_UP,
            IDC_GROUP_DOWN,
            IDC_GROUP_NEW : SetChanged(Dialog,etHK);

            IDC_GROUP_DELETE, // action deleting with subactions
            IDC_GROUP_IMPORT,
            IDC_ACTION_NEW,
            IDC_ACTION_DELETE,
            IDC_ACTION_UP,
            IDC_ACTION_DOWN: begin
              ChMask:=ChMask or ACTM_ACT or ACTM_ACTS;
              SetChanged(Dialog,etHK+etACT);
            end;
          else
            ChMask:=ChMask or ACTM_ACT or ACTM_ACTS;
            SetChanged(Dialog,etACT);
          end;

          case loword(wParam) of
            IDC_SRV_WPAR,
            IDC_SRV_LPAR,
            IDC_SRV_SRVC,
            IDC_PRG_PRG ,
            IDC_PRG_ARG ,
            IDC_TXT_FILE,
            IDC_TXT_TEXT,
            IDC_RW_MVAR ,
            IDC_RW_SVAR ,
            IDC_RW_TVAR ,
            IDC_MSG_TTL ,
            IDC_MSG_TXT : SetButtonIcon(lParam,checknames[IsDlgButtonChecked(Dialog,loword(wParam))]);
          end;

          case loword(wParam) of
            IDC_CNT_FILTER,
            IDC_CNT_APPLY: begin
              if loword(wParam)=IDC_CNT_APPLY then
              begin
                mFreeMem(fCLformat);
                fCLformat:=GetDlgText(Dialog,IDC_EDIT_FORMAT);
                DBWriteUnicode(0,DBBranch,'CLformat',fCLformat);
              end
              else
              begin
                fCLfilter:=IsDlgButtonChecked(Dialog,IDC_CNT_FILTER)<>BST_UNCHECKED;
                DBWriteByte(0,DBBranch,'CLfilter',ord(fCLfilter));
              end;
// Saving and restoring contact after list rebuild
              wnd:=GetDlgItem(Dialog,IDC_CONTACTLIST);
              i:=SendMessage(wnd,CB_GETITEMDATA,SendMessage(wnd,CB_GETCURSEL,0,0),0);

              FillContactList(wnd,fCLfilter,fCLformat);

              SendMessage(wnd,CB_SETCURSEL,FindContact(wnd,i),0);
            end;
            IDC_HLP_FVARS,
            IDC_HLP_VARS : ShowHelp(hlpVariables);
            IDC_ADV_HVARS: ShowHelp(hlpAdvVariables);

            IDC_GROUP_EXPORT: begin
              if ShowDlgW(xmlfilename,xmlfilename,inoutfilter,false) then
              begin
                wnd:=GetDlgItem(Dialog,IDC_ACTION_GROUP);
                for i:=0 to ListView_GetItemCount(wnd)-1 do
                begin
                  if ListView_GetItemState(wnd,i,LVIS_SELECTED)<>0 then
                  begin
                    with GroupList[LV_GetLParam(wnd,i)] do
                      if (flags and ACF_ASSIGNED)<>0 then // must be always true
                        flags:=flags or ACF_EXPORT;
                  end
                  else
                    with GroupList[i] do
                      if (flags and (ACF_EXPORT or ACF_ASSIGNED))=
                                    (ACF_EXPORT or ACF_ASSIGNED) then
                        flags:=flags and not ACF_EXPORT;
                end;
                i:=ACIO_EXPORT or ACIO_SELECTED;
                if GetFSize(xmlfilename)>0 then
                  if MessageBoxW(Dialog,TranslateW('Append data to file?'),
                     PluginName,MB_YESNO+MB_ICONWARNING)=IDYES then
                    i:=i or ACIO_APPEND;
                CallService(MS_ACT_INOUT,i,TLPARAM(@xmlfilename));
//                Export({GetLParam(GetDlgItem(Dialog,IDC_ACTION_GROUP)),}xmlfilename,i);
                for i:=0 to MaxGroups-1 do
                  with GroupList[i] do
                    if (flags and (ACF_EXPORT or ACF_ASSIGNED))=
                                  (ACF_EXPORT or ACF_ASSIGNED) then
                      flags:=flags and not ACF_EXPORT;
              end;
            end;

            IDC_GROUP_IMPORT: begin
              if ShowDlgW(xmlfilename,xmlfilename,inoutfilter) then
              begin
//                if Import(xmlfilename) then
                if CallService(MS_ACT_INOUT,0,TLPARAM(@xmlfilename))<>0 then
                begin
                  ChMask:=ChMask or ACTM_NEW;
                  SendMessage(Dialog,WM_COMMAND,IDC_GROUP_RELOAD+(BN_CLICKED shl 16),
                    GetDlgItem(Dialog,IDC_GROUP_RELOAD));
//(in reload)       FillChainList(Dialog);
                end;
              end;
            end;

            IDC_WSTRUCT, IDC_LSTRUCT: begin
              if loword(wParam)=IDC_WSTRUCT then
                pc:=wstruct
              else
                pc:=lstruct;
//!!!!
              pAnsiChar(j):=EditStructure(pAnsiChar(pc),Dialog);
              if j<>0 then
              begin
                ChMask:=ChMask or ACTM_ACT or ACTM_ACTS;
                SetChanged(Dialog,etACT);
                mFreeMem(pAnsiChar(pc));
                pc:=pAnsiChar(j);

                if loword(wParam)=IDC_WSTRUCT then
                  wstruct:=pc
                else
                  lstruct:=pc;
              end;
            end;

            IDC_PROGRAM: begin
              FillFileName(IDC_EDIT_PRGPATH);
            end;
            IDC_FILE_FILEBTN: begin
              FillFileName(IDC_FILE_PATH);
            end;

            IDC_GROUP_TEST: begin
              wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
              li.mask    :=LVIF_PARAM;
              li.iSubItem:=0;
              li.iItem   :=0;
              SendMessageW(wnd,LVM_GETITEMW,0,tlparam(@li));
              j:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);

              ActionStarterWait(li.lParam);
              // doubling from  "reload" button
              wnd:=GetDlgItem(Dialog,IDC_ACTION_GROUP);
              i:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
              NewGroupList :=GroupList;
              NewMaxGroups :=MaxGroups;
              NewActionList:=ActionList;
              NewMaxActions:=MaxActions;
              FillGroupList{(Dialog)};
              FillChainList(Dialog);

              Listview_SetItemState(wnd,0,0,LVIS_FOCUSED or LVIS_SELECTED);
              Listview_SetItemState(wnd,i,
                   LVIS_FOCUSED or LVIS_SELECTED,
                   LVIS_FOCUSED or LVIS_SELECTED);
              SendMessage(wnd,LVM_ENSUREVISIBLE,i,0);

              wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
              Listview_SetItemState(wnd,0,0,LVIS_FOCUSED or LVIS_SELECTED);
              Listview_SetItemState(wnd,j,
                   LVIS_FOCUSED or LVIS_SELECTED,
                   LVIS_FOCUSED or LVIS_SELECTED);
              SendMessage(wnd,LVM_ENSUREVISIBLE,j,0);

            end;
            IDC_GROUP_NEW: begin
              i:=AddGroup(Dialog,NewGroup(NewGroupList,NewMaxGroups));
              if i>=0 then
              begin
                ChMask:=ChMask or ACTM_NEW or ACTM_SORT;
                EnableWindow(GetDlgItem(Dialog,IDC_ACTION_GROUP),true);
                EnableWindow(GetDlgItem(Dialog,IDC_GROUP_DELETE),true);
                EnableWindow(GetDlgItem(Dialog,IDC_ACTION_NEW  ),true);
                FillChainList(Dialog);
                CheckGrpList(i);
              end;
            end;
            IDC_GROUP_DELETE: begin
              wnd:=GetDlgItem(Dialog,IDC_ACTION_GROUP);
              for i:=ListView_GetItemCount(wnd)-1 downto 0 do
              begin
                if ListView_GetItemState(wnd,i,LVIS_SELECTED)<>0 then
                begin
                  ChMask:=ChMask or ACTM_DELETE or ACTM_SORT;

                  with NewGroupList^[LV_GetLParam(wnd,i)] do
                  begin
                    flags:=0;
                    mFreeMem(descr);
                    FreeActions(NewActionList,firstAction);
                  end;

                  SendMessage(wnd,LVM_DELETEITEM,i,0);
                end;
              end;
              SendDlgItemMessage(Dialog,IDC_ACTION_LIST,LVM_DELETEALLITEMS,0,0);
              FillChainList(Dialog);
              Listview_SetItemState(wnd,0,LVIS_FOCUSED or LVIS_SELECTED,
                                          LVIS_FOCUSED or LVIS_SELECTED);
              CheckGroupList(-1);
{
              i:=SendMessageW(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED); //??
              if i>=0 then
              begin
                ChMask:=ChMask or ACTM_DELETE;
                j:=GetLParam(wnd,i);

                with NewGroupList^[j] do
                begin
                  flags:=0;
                  mFreeMem(descr);
                  FreeActions(NewActionList,firstAction);
                end;
                SendDlgItemMessage(Dialog,IDC_ACTION_LIST,LVM_DELETEALLITEMS,0,0);

//??                i:=ListView_GetNextItem(wnd,-1,LVNI_FOCUSED);
                SendMessage(wnd,LVM_DELETEITEM,i,0);
                FillChainList(Dialog);
                CheckGroupList(i);
              end;
}
            end;
            IDC_GROUP_RELOAD: begin
              ChMask:=ChMask or ACTM_RELOAD;
              SetCancel;
              NewGroupList :=GroupList;
              NewMaxGroups :=MaxGroups;
              NewActionList:=ActionList;
              NewMaxActions:=MaxActions;
              FillGroupList{(Dialog)};
              FillChainList(Dialog);
            end;

            IDC_ACTION_HELP: begin
              ShowHelp(SendDlgItemMessage(Dialog,IDC_ACTION_TYPE,CB_GETCURSEL,0,0));
            end;

            IDC_ACTION_NEW: begin
              wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
              li.mask    :=LVIF_PARAM;
              i          :=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
              li.iItem   :=i+1;
              li.iSubItem:=0;
              li.lParam  :=NewAction(NewActionList,NewMaxActions);
              SendMessageW(wnd,LVM_INSERTITEMW,0,tlparam(@li));
              ListView_SetCheckState(wnd,li.iItem,true);
              if li.iItem=0 then
              begin
                ListView_SetItemState(wnd,0,LVIS_FOCUSED or LVIS_SELECTED,
                  LVIS_FOCUSED or LVIS_SELECTED);
                SHActButtons(SW_SHOW);
              end;
              ChangeActionName(li.iItem);
              EnableWindow(GetDlgItem(Dialog,IDC_ACTION_LIST  ),true);
              EnableWindow(GetDlgItem(Dialog,IDC_ACTION_HELP  ),true);
              EnableWindow(GetDlgItem(Dialog,IDC_GROUP_TEST   ),true);
              EnableWindow(GetDlgItem(Dialog,IDC_ACTION_DELETE),true);
              EnableWindow(GetDlgItem(Dialog,IDC_ACTION_TYPE  ),true);
              CheckActList(i);

              wnd:=GetDlgItem(Dialog,IDC_ADV_VAL2);
              i:=SendMessage(wnd,CB_GETCURSEL,0,0);
              FillSubList(Dialog);
              SendMessage(wnd,CB_SETCURSEL,i,0);
            end;
            IDC_ACTION_DELETE: begin
              wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
              for i:=ListView_GetItemCount(wnd)-1 downto 0 do
              begin
                if ListView_GetItemState(wnd,i,LVIS_SELECTED)<>0 then
                begin
                  FreeAction(@NewActionList^[LV_GetLParam(wnd,i)]);
                  SendMessage(wnd,LVM_DELETEITEM,i,0);
                end;
              end;
              Listview_SetItemState(wnd,0,LVIS_FOCUSED or LVIS_SELECTED,
                                          LVIS_FOCUSED or LVIS_SELECTED);
              CheckActionList(0);
{
              wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
              li.iItem:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED); //??
              if li.iItem>=0 then
              begin
                li.mask    :=LVIF_PARAM;
                li.iSubItem:=0;
                SendMessageW(wnd,LVM_GETITEM,0,TLPARAM(@li));
                FreeAction(@NewActionList^[li.lParam]);

                SendMessage(wnd,LVM_DELETEITEM,li.iItem,0);
                CheckActionList(li.iItem);
              end;
}
            end;

            IDC_GROUP_UP: begin
              wnd:=GetDlgItem(Dialog,IDC_ACTION_GROUP);
//              i:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
//              if i>0 then
              begin
                CheckGrpList(MoveGroup(wnd,-1,-1));
                ChMask:=ChMask or ACTM_SORT;
              end;
            end;
            IDC_GROUP_DOWN: begin
              wnd:=GetDlgItem(Dialog,IDC_ACTION_GROUP);
//              i:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
//              if i<(SendMessage(wnd,LVM_GETITEMCOUNT,0,0)-1) then
              begin
                CheckGrpList(MoveGroup(wnd,-1,1));
                ChMask:=ChMask or ACTM_SORT;
              end;
            end;

            IDC_ACTION_UP: begin
              wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
//              i:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
//              if i>0 then
                CheckActList(MoveGroup(wnd,-1,-1));
            end;
            IDC_ACTION_DOWN: begin
              wnd:=GetDlgItem(Dialog,IDC_ACTION_LIST);
//              i:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
//              if i<(SendMessage(wnd,LVM_GETITEMCOUNT,0,0)-1) then
                CheckActList(MoveGroup(wnd,-1,1));
            end;

            IDC_FILE_WRITE, IDC_FILE_READ, IDC_FILE_APPEND:
                EnableWindow(GetDlgItem(Dialog,IDC_EDIT_INSERT),
                             loword(wParam)<>IDC_FILE_APPEND);

            IDC_RW_DELETE,
            IDC_RW_READ,
            IDC_RW_WRITE: begin
              b:=loword(wParam)<>IDC_RW_DELETE;
              EnableWindow(GetDlgItem(Dialog,IDC_RW_DATATYPE),b);
              EnableWindow(GetDlgItem(Dialog,IDC_RW_LAST    ),b);
              EnableWindow(GetDlgItem(Dialog,IDC_RW_VALUE   ),b);
              EnableWindow(GetDlgItem(Dialog,IDC_RW_TEXT    ),b);
            end;

            IDC_FLAG_CLIP,IDC_FLAG_FILE,IDC_FLAG_MESSAGE: begin
              b:=loword(wParam)=IDC_FLAG_CLIP;
//!! +text read = disabled
              EnableWindow(GetDlgItem(Dialog,IDC_CLIP_COPYTO),b);
              EnableWindow(GetDlgItem(Dialog,IDC_CLIP_PASTE ),b);
              b:=b or ((loword(wParam)=IDC_FLAG_FILE) and
                 (IsDlgButtonChecked(Dialog,IDC_FILE_READ)<>BST_UNCHECKED));
              EnableWindow(GetDlgItem(Dialog,IDC_EDIT_INSERT),not b);

              b:=loword(wParam)=IDC_FLAG_FILE;
              EnableWindow(GetDlgItem(Dialog,IDC_FILE_ENC    ),b);
              EnableWindow(GetDlgItem(Dialog,IDC_FILE_PATH   ),b);
              EnableWindow(GetDlgItem(Dialog,IDC_FILE_FILEBTN),b);
              EnableWindow(GetDlgItem(Dialog,IDC_FILE_READ   ),b);
              EnableWindow(GetDlgItem(Dialog,IDC_FILE_WRITE  ),b);
              EnableWindow(GetDlgItem(Dialog,IDC_FILE_APPEND ),b);
            end;

            IDC_FLAG_JUMP: begin
              EnableWindow(GetDlgItem(Dialog,IDC_ADV_VAL2),true);
            end;

            IDC_FLAG_BREAK,IDC_FLAG_ANOP:
            begin
              EnableWindow(GetDlgItem(Dialog,IDC_ADV_VAL2),false);
            end;

            IDC_FLAG_VARS: begin
              if IsDlgButtonChecked(Dialog,IDC_FLAG_VARS)<>BST_UNCHECKED then
              begin
                SHMath(false);
                CheckDlgButton(Dialog,IDC_FLAG_MATH,BST_UNCHECKED);
              end
              else
              begin
                EnableWindow(GetDlgItem(Dialog,IDC_ADV_ASINT),false);
                EnableWindow(GetDlgItem(Dialog,IDC_ADV_VARS ),false);
              end;
            end;
            IDC_FLAG_MATH: begin
              if IsDlgButtonChecked(Dialog,IDC_FLAG_MATH)<>BST_UNCHECKED then
              begin
                SHMath(true);
                CheckDlgButton(Dialog,IDC_FLAG_VARS,BST_UNCHECKED);
              end
              else
              begin
                EnableWindow(GetDlgItem(Dialog,IDC_ADV_OPER),false);
                EnableWindow(GetDlgItem(Dialog,IDC_ADV_VAL1),false);
              end;
            end;

            IDC_RW_CURRENT, IDC_RW_RESULT, IDC_RW_PARAM: begin
              EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),false);
            end;
            IDC_RW_MANUAL: EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),true);

            IDC_RW_LAST: begin
              b :=IsDlgButtonChecked(Dialog,IDC_RW_LAST )=BST_UNCHECKED;
              EnableWindow(GetDlgItem(Dialog,IDC_RW_VALUE), b);
              EnableWindow(GetDlgItem(Dialog,IDC_RW_TEXT ), b);
            end;
          end;
        end;
      end;
    end;

    WM_HELP: begin
      if (PHELPINFO(lParam)^.iContextType=HELPINFO_WINDOW) then
        ShowHelp(SendDlgItemMessage(Dialog,IDC_ACTION_TYPE,CB_GETCURSEL,0,0));
    end;

    WM_NOTIFY: begin
      case integer(PNMHdr(lParam)^.code) of
        PSN_APPLY: begin
          BuildActionChain;
          SetSave(Dialog,LV_GetLParam(GetDlgItem(Dialog,IDC_ACTION_GROUP)));

          if ActionList<>NewActionList then
          begin
            DestroyActions(ActionList,MaxActions);
            ActionList:=NewActionList;
            MaxActions:=NewMaxActions;
          end;

          SaveGroups;

          if ChMask<>0 then //??
          begin
            NotifyEventHooks(hHookChanged,ChMask,0);
            ChMask:=0;
          end;

        end;

        NM_DBLCLK: begin
          if PNMListView(lParam)^.iItem>=0 then
            PostMessageW(PNMHdr(lParam)^.hWndFrom,LVM_EDITLABELW,
                        PNMListView(lParam)^.iItem,0);
        end;

        LVN_ITEMCHANGED: begin
          if DontReact then exit; // bug when group moved avoid

          if wParam=IDC_ACTION_GROUP then
          begin
            if PNMLISTVIEW(lParam)^.uChanged=LVIF_STATE then
            begin
              lvflag:=(PNMLISTVIEW(lParam)^.uOldState and LVNI_FOCUSED)-
                      (PNMLISTVIEW(lParam)^.uNewState and LVNI_FOCUSED);

              if lvflag>0 then // old focus
                BuildActionChain(PNMLISTVIEW(lParam)^.iItem)
              else if lvflag<0 then // new focus
              begin
                DontReact:=true;

                ClearDialogData;
                MakeActionList(@NewGroupList^[PNMLISTVIEW(lParam)^.lParam]);
                CheckGrpList(PNMLISTVIEW(lParam)^.iItem);

                DontReact:=false;
              end
              else
              begin
                lvflag:=(PNMLISTVIEW(lParam)^.uOldState and LVNI_SELECTED)-
                        (PNMLISTVIEW(lParam)^.uNewState and LVNI_SELECTED);
                if lvflag<>0 then
                  CheckGrpList(PNMLISTVIEW(lParam)^.iItem);
              end;
            end;
          end
          else if wParam=IDC_ACTION_LIST then
          begin
            lvflag:=(PNMLISTVIEW(lParam)^.uOldState and LVNI_FOCUSED)-
                    (PNMLISTVIEW(lParam)^.uNewState and LVNI_FOCUSED);
            if lvflag>0 then // old focus
              SaveAction(PNMLISTVIEW(lParam)^.iItem,
                         PNMLISTVIEW(lParam)^.lParam)
            else if lvflag<0 then // new focus
            begin
              DontReact:=true;

              ClearDialogData;
              j:=PNMLISTVIEW(lParam)^.lParam;
              FillAction(j);
              CheckActList(PNMLISTVIEW(lParam)^.iItem);

              DontReact:=false;
            end
            else
            begin // checkboxes
              lvflag:=(PNMLISTVIEW(lParam)^.uOldState and LVNI_SELECTED)-
                      (PNMLISTVIEW(lParam)^.uNewState and LVNI_SELECTED);
              if lvflag<>0 then
                CheckGrpList(PNMLISTVIEW(lParam)^.iItem);

              if (PNMLISTVIEW(lParam)^.uOldState or PNMLISTVIEW(lParam)^.uNewState)=$3000 then
              begin
                if PNMLISTVIEW(lParam)^.uOldState=$1000 then
                  i:=0
                else
                  i:=ACF_DISABLED;
                j:=PNMLISTVIEW(lParam)^.lParam;
                NewActionList^[j].flags:=(NewActionList^[j].flags and not ACF_DISABLED) or dword(i);
                ChMask:=ChMask or ACTM_ACT or ACTM_ACTS;
                SetChanged(Dialog,etACT);
              end;
            end;
          end;
        end;

        LVN_ENDLABELEDITW: begin
          if DontReact then exit;
          if wParam=IDC_ACTION_GROUP then
          begin
            with PLVDISPINFOW(lParam)^ do
            begin
              if item.pszText<>nil then
              begin
                ChMask:=ChMask or ACTM_RENAME;
                SetChanged(Dialog,etHK);
                item.mask:=LVIF_TEXT;
                if pWideChar(item.pszText)^=#0 then
                  pWideChar(item.pszText):=NoDescription;
                SendMessageW(hdr.hWndFrom,LVM_SETITEMW,0,tlparam(@item));

                with NewGroupList^[item.lParam] do
                begin
                  mFreeMem(descr);
                  StrDupW(descr,item.pszText);
                end;

                FillChainList(Dialog);
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
                ChMask:=ChMask or ACTM_ACT or ACTM_ACTS;
                SetChanged(Dialog,etACT);
                ChangeActionName(item.iItem,0,pWideChar(item.pszText));
                result:=1;
              end;
//??
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

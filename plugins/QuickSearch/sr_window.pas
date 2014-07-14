unit sr_window;

interface

uses windows,m_api;

function OpenSrWindow(apattern:PWideChar;flags:LPARAM):boolean;
function CloseSrWindow(save:boolean=true):boolean;

procedure RegisterColors;

const
  grid:HWND = 0;

implementation

uses messages,commctrl,sr_global,common,dbsettings,mirutils,
    wrapper,protocols,sparam,srvblock,editwrapper;

const
  IDM_STAYONTOP = WM_USER+1;

const
  flt_show_offline = $100;
const
  strCListDel:PAnsiChar='CList/DeleteContactCommand';
const
  hIconF   :HICON   = 0;
  hIconM   :HICON   = 0;
  mainwnd  :HWND    = 0;
  StatusBar:HWND    = 0;
const
  OldLVProc  :pointer = nil;
  OldLVHProc :pointer = nil;
  OldEditProc:pointer = nil;
const
  QSF_INLIST  = $0001; // in constant list
  QSF_ACTIVE  = $0002; // contact in listview
  QSF_DELETED = $0004; // contact deleted
  QSF_PATTERN = $0008; // pattern check passed
  QSF_ACCDEL  = $0010; // account deleted
  QSF_ACCOFF  = $0020; // account disabled
  QSF_META    = $0040; // contact is metacontact
  QSF_SUBMETA = $0080; // contact is part of metacontact

type
  pQSRec = ^tQSRec;
  tQSRec = record // cell
    text:PWideChar;
    data:uint_ptr;
  end;
  pQSFRec = ^tQSFRec;
  tQSFRec = record // row (contact)
    contact:TMCONTACT;
    proto  :uint_ptr;
    flags  :dword;
    status :dword;
//--- Metacontacts only ---
    wparam :WPARAM; 
    lparam :LPARAM;
  end;
var
  MainBuf:array of array of tQSRec;
  FlagBuf:array of tQSFRec;
  LastMeta:integer;
  tablecolumns:integer;
var
  tstrMale,
  tstrFemale,
  tstrUnknown:PWideChar;

const
  TIMERID_HOVER = 10;
const
  TTShowed:bool=false;
  TTInstalled:bool = false;

var
  AdvFilter:cardinal;

{$i i_ok.inc}

//-----  -----

function FindItem(num:integer):integer;
var
  fi:LV_FINDINFO;
begin
  if num>=0 then
  begin
    FillChar(fi,SizeOf(fi),0);
    fi.flags :=LVFI_PARAM;
    fi.lParam:=num;
    result:=SendMessage(grid,LVM_FINDITEM,wparam(-1),lparam(@fi));
  end
  else
    result:=num;
end;

function GetLVSubItem(x,y:integer):integer;
var
  pinfo:LV_HITTESTINFO;
begin
  pinfo.flags:=0;
  pinfo.pt.x:=x;
  pinfo.pt.y:=y;
  ScreenToClient(grid,pinfo.pt);
  result:=-1;
  if integer(SendMessage(grid,LVM_SUBITEMHITTEST,0,tlparam(@pinfo)))<>-1 then
  begin
    if (pinfo.flags and LVHT_ONITEM)<>0 then
    begin
      result:=pinfo.iSubItem;
    end;
  end;
end;

procedure AddContactToList(hContact:TMCONTACT;num:integer);
var
  li:LV_ITEMW;
  i:integer;
begin
  FillChar(li,SizeOf(li),0);
  li.iItem :=100000; //!! need append
  li.mask  :=LVIF_IMAGE or LVIF_PARAM;
  li.iImage:=CallService(MS_CLIST_GETCONTACTICON,hContact,0);
  li.lParam:=num;
  li.iItem :=SendMessageW(grid,LVM_INSERTITEMW,0,lparam(@li));

  li.iImage:=0;
  li.iSubItem:=0;
  for i:=0 to qsopt.numcolumns-1 do
  begin
    with qsopt.columns[i] do
    begin
      if (flags and COL_ON)<>0 then
      begin
        // Client icons preprocess
        li.pszText :=MainBuf[num,i].text;
        if (((flags and COL_CLIENT)<>0) and
            ((qsopt.flags and QSO_CLIENTICONS)<>0) and
            (li.pszText<>NIL)) OR
           ((flags and (COL_XSTATUS or COL_GENDER))<>0) then
          li.mask:=LVIF_IMAGE or LVIF_TEXT
        else
          li.mask:=LVIF_TEXT;
        SendMessageW(grid,LVM_SETITEMW,0,tlparam(@li));
        inc(li.iSubItem);
      end;
    end;
  end;
end;


procedure ProcessLine(num:integer;test:boolean=true);
var
  p:pQSFRec;
  l:boolean;
begin
  p:=@FlagBuf[num];
  if (p^.flags and QSF_DELETED)<>0 then
    exit;

  if test then
  begin
    l:=CheckPatternW(num);
    if l then
      p^.flags:=p^.flags or QSF_PATTERN
    else
      p^.flags:=p^.flags and not QSF_PATTERN;
  end
  else
    l:=(p^.flags and QSF_PATTERN)<>0;//true;

  if l then
  begin
    if (p^.flags and QSF_ACTIVE)=0 then
    begin
      if ((qsopt.flags and QSO_SHOWOFFLINE)<>0) or (p^.status<>ID_STATUS_OFFLINE) then
      begin
        // check for proto in combo
        if (LoByte(AdvFilter)=0) or (p^.proto=LoByte(AdvFilter)) then
        begin
          p^.flags:=p^.flags or QSF_ACTIVE;
          AddContactToList(p^.contact,num);
        end;
      end;
    end
  end
  else
  begin
    if (p^.flags and QSF_ACTIVE)<>0 then
    begin
      p^.flags:=p^.flags and not QSF_ACTIVE;
      ListView_DeleteItem(grid,FindItem(num));
    end;
  end;
end;


function CompareItem(lParam1,lParam2:LPARAM;SortType:LPARAM):int; stdcall;
var
  res1,res2:pQSRec;
  i1,i2:uint_ptr;
  typ1,typ2:boolean;
begin
  result:=0;
  if SortType=StatusSort then //sort by status
  begin
    i1:=FlagBuf[lParam1].status;
    i2:=FlagBuf[lParam2].status;
    // offline - to the end
    if i1=ID_STATUS_OFFLINE then i1:=ID_STATUS_OFFLINE+64;
    if i2=ID_STATUS_OFFLINE then i2:=ID_STATUS_OFFLINE+64;
    // not string parameters
    typ1:=false;
    typ2:=false;
  end
  else
  begin
    res1:=@MainBuf[lParam1,SortType];
    res2:=@MainBuf[lParam2,SortType];
    i1  := res1^.data;
    i2  := res2^.data;
    typ1:=i1=uint_ptr(-1);
    typ2:=i2=uint_ptr(-1);

    if (typ1 and typ2) then // string & string
    begin
      if (res2.text=nil) and (res1.text=nil) then // nil
        result:=0
      else if res2.text=nil then
        result:=1
      else if res1.text=nil then
        result:=-1
      else
        result:=lstrcmpiw(res1.text,res2.text);
    end
    else if typ1 or typ2 then // string & num
    begin
      if typ1 then
        result:=1
      else
        result:=-1;
    end;
  end;
  if not (typ1 or typ2) then // not strings
  begin
    if i1>i2 then
      result:=1
    else if i1<i2 then
      result:=-1
    else
      result:=0;
  end;
  if (qsopt.flags and QSO_SORTASC)=0 then
    result:=-result;
end;

procedure Sort;
begin
  if qsopt.columnsort>=tablecolumns then
    qsopt.columnsort:=StatusSort;

  SendMessage(grid,LVM_SORTITEMS,ListViewToColumn(qsopt.columnsort),LPARAM(@CompareItem));
//  ListView_SortItems(grid,@CompareItem,GetQSColumn(qsopt.columnsort));

  if (qsopt.columnsort<>StatusSort) and ((qsopt.flags and QSO_SORTBYSTATUS)<>0) then
    SendMessage(grid,LVM_SORTITEMS,StatusSort,LPARAM(@CompareItem));
//    ListView_SortItems(grid,@CompareItem,StatusSort);
end;

function AdvancedFilter:integer;
var
  p:pQSFRec;
  i:integer;
  show:boolean;
begin
  result:=0;

  SendMessage(grid,WM_SETREDRAW,0,0);

  for i:=0 to HIGH(FlagBuf) do
  begin
    p:=@FlagBuf[i];
    // firstly = proto
    show:=(LoByte(AdvFilter)=0) or (p^.proto=LoByte(AdvFilter));
    // secondary = show/hide offline
    show:=show and ((p^.status<>ID_STATUS_OFFLINE) or ((AdvFilter and flt_show_offline)<>0));

    if (p^.flags and QSF_PATTERN)<>0 then
    begin
      if show then
      begin
        if (p^.flags and QSF_ACTIVE)=0 then
          ProcessLine(i,false);
      end
      else
      begin
        p^.flags:=p^.flags and not QSF_ACTIVE;
        ListView_DeleteItem(grid,FindItem(i));
      end;
    end;
  end;

  SendMessage(grid,WM_SETREDRAW,1,0);
  InvalidateRect(grid,nil,false);

  Sort;
  UpdateSB;
end;

procedure FillGrid;
var
  cnt:integer;
begin

  SendMessage(grid,WM_SETREDRAW,0,0);

  MakePatternW;

  for cnt:=0 to HIGH(FlagBuf) do
    ProcessLine(cnt);

  SendMessage(grid,WM_SETREDRAW,1,0);
  InvalidateRect(grid,nil,false);

  Sort;
  UpdateSB;

  AdvancedFilter; //!!

  ListView_SetItemState(grid,0,LVIS_FOCUSED or LVIS_SELECTED,
    LVIS_FOCUSED or LVIS_SELECTED);
end;


//----- contacts actions -----

function GetFocusedhContact:TMCONTACT;
var
  i:integer;
begin
  i:=LV_GetLParam(grid);
  if i=-1 then
    result:=0
  else
    result:=FlagBuf[i].contact;
end;

procedure ShowContactMsgDlg(hContact:TMCONTACT);
begin
  if hContact<>0 then
  begin
    ShowContactDialog(hContact);
    if (qsopt.flags and QSO_AUTOCLOSE)<>0 then DestroyWindow(mainwnd);
  end;
end;

procedure DeleteOneContact(hContact:TMCONTACT);
begin
  if ServiceExists(strCListDel)>0 then
    CallService(strCListDel,hContact,0)
  else
    CallService(MS_DB_CONTACT_DELETE,hContact,0);
end;

procedure DeleteByList;
var
  i,j:integer;
begin
  j:=ListView_GetItemCount(grid)-1;

  i:=MessageBoxW(0,TranslateW('Do you really want to delete selected contacts?'),
     TranslateW('Warning'),MB_OKCANCEL+MB_ICONWARNING);

  if i=IDOK then
  begin
    SendMessage(grid,WM_SETREDRAW,0,0);
    for i:=j downto 0 do
    begin
      if ListView_GetItemState(grid,i,LVIS_SELECTED)<>0 then
        CallService(MS_DB_CONTACT_DELETE,FlagBuf[LV_GetLParam(grid,i)].contact,0);
    end;
    SendMessage(grid,WM_SETREDRAW,1,0);
  end;
end;

procedure ConvertToMeta;
var
  hMeta:TMCONTACT;
  tmp:TMCONTACT;
  i,j:integer;
begin
  j:=ListView_GetItemCount(grid)-1;

  hMeta:=0;
  for i:=j downto 0 do // check
  begin
    if ListView_GetItemState(grid,i,LVIS_SELECTED)<>0 then
    begin
      tmp:=db_mc_getMeta(FlagBuf[LV_GetLParam(grid,i)].contact);
      if tmp<>0 then
        if hMeta=0 then
          hMeta:=tmp
        else if tmp<>hMeta then
        begin
          MessageBoxW(0,
            TranslateW('Some of selected contacts in different metacontacts already'),
            'Quick Search',MB_ICONERROR);
          exit;
        end;
    end;
  end;

  if hMeta<>0 then
  begin
    i:=MessageBoxW(0,
      TranslateW('One or more contacts in same Meta already. Try to convert anyway?'),
      'Quick Search',MB_YESNO+MB_ICONWARNING);
    if i<>IDYES then
      exit;
  end;

  // convert if needed
  for i:=j downto 0 do
  begin
    if ListView_GetItemState(grid,i,LVIS_SELECTED)<>0 then
    begin
      if hMeta=0 then
        hMeta:=CallService(MS_MC_CONVERTTOMETA,FlagBuf[LV_GetLParam(grid,i)].contact,0)
      else
        CallService(MS_MC_ADDTOMETA,FlagBuf[LV_GetLParam(grid,i)].contact,hMeta);
    end;
  end;
end;

procedure UpdateLVCell(item,column:integer;text:pWideChar=pWideChar(-1));
var
  li:LV_ITEMW;
  contact:TMCONTACT;
  row:integer;
begin
  contact:=FlagBuf[LV_GetLParam(grid,item)].contact;
  // get buffer row from LV item
  row:=FindBufNumber(contact);
  // get cell text
  if text=pWideChar(-1) then
  begin
    mFreeMem(MainBuf[row,column].text);
    LoadOneItem(contact,@qsopt.columns[column],0,MainBuf[row,column]);
    text:=MainBuf[row,column].text;
  end;

  // rewrite LV cell
  zeromemory(@li,sizeof(li));
  li.mask    :=LVIF_TEXT;
  li.iItem   :=item;
  li.iSubItem:=ColumnToListview(column); // buffer column to LV subitem
  li.pszText :=text;
  SendMessageW(grid,LVM_SETITEMW,0,tlparam(@li));

  // if need to filter and sort, do it
  if (qsopt.columns[column].flags and COL_FILTER)<>0 then
    ProcessLine(row);
  if qsopt.columnsort=li.iSubItem then
    Sort;
end;

procedure MoveToGroup(group:PWideChar);
var
  contact:TMCONTACT;
  i,j,grcol,row:integer;
begin
  j:=ListView_GetItemCount(grid)-1;
  // search group column in QS window (if presents)
  grcol:=-1;
  for i:=0 to qsopt.numcolumns-1 do
  begin
    with qsopt.columns[i] do
      if (flags and COL_GROUP)<>0 then
      begin
        if (flags and COL_ON)=0 then
          flags:=flags and not COL_INIT
        else
          grcol:=i;
        break;
      end
  end;
  // move to new group and changing in LV if needs
  for i:=0 to j do
  begin
    if ListView_GetItemState(grid,i,LVIS_SELECTED)<>0 then
    begin
      contact:=FlagBuf[LV_GetLParam(grid,i)].contact;
      // change settings
      DBWriteUnicode(contact,strCList,'Group',group);
      // update buffer and LV
      if ((qsopt.flags and QSO_AUTOCLOSE)=0) and (grcol>=0) then
      begin
        row:=FindBufNumber(contact);

        mFreeMem(MainBuf[row,grcol].text);
        StrDupW (MainBuf[row,grcol].text,group);

//        LoadOneItem(contact,qsopt.columns[grcol],0,MainBuf[row,grcol]);
        UpdateLVCell(i,grcol);
      end;
    end;
  end;
end;

procedure MoveToContainer(container:PWideChar);
var
  contact:TMCONTACT;
  i,j,grcol,row:integer;
begin
  j:=ListView_GetItemCount(grid)-1;
  // search container column in QS window (if presents)
  grcol:=-1;

  for i:=0 to qsopt.numcolumns-1 do
  begin
    with qsopt.columns[i] do
      if (flags and COL_CNTNR)<>0 then
      begin
        if (flags and COL_ON)=0 then
          flags:=flags and not COL_INIT
        else
          grcol:=i;
        break;
      end
  end;

  // attach to new container and changing in LV if needs
  for i:=0 to j do
  begin
    if ListView_GetItemState(grid,i,LVIS_SELECTED)<>0 then
    begin
      contact:=FlagBuf[LV_GetLParam(grid,i)].contact;
      if container^=#0 then
        DBDeleteSetting(contact,'Tab_SRMsg','containerW')
      else
        DBWriteUnicode(contact,'Tab_SRMsg','containerW',container);
      if ((qsopt.flags and QSO_AUTOCLOSE)=0) and (grcol>=0) then
      begin
        row:=FindBufNumber(contact);

        mFreeMem(MainBuf[row,grcol].text);
        StrDupW (MainBuf[row,grcol].text,container);

//        LoadOneItem(contact,qsopt.columns[grcol],0,MainBuf[row,grcol]);
        UpdateLVCell(i,grcol);
      end;
    end;
  end;
end;

// right now - memory column order, not screen
procedure CopyMultiLinesW;
var
  p,buf:PWideChar;
  idx:integer;
  i,j,k:integer;
  tmpcnt,cnt:integer;
begin
{
   lv:LV_COLUMNW;
   buf:array [0..127] of WideChar;

   lv.mask      :=LVCF_TEXT;
   lv.cchTextMax:=128;
   lv.pszText   :=@buf;

   SendMessageW(LVM_GETCOLUMNW,i,LPARAM(@lv));

   use lv.pszText, not qsopt.columns[i].title
}
  // calculate buffer size, column order not important
  cnt:=0;

  k:=0;
  while k<qsopt.numcolumns do
  begin
    if not IsColumnMinimized(k) then
      inc(cnt,StrLenW(TranslateW(qsopt.columns[k].title))+1);
    Inc(k);
  end;
  if cnt>0 then
    inc(cnt,2);

  j:=ListView_GetItemCount(grid)-1;
  tmpcnt:=cnt;
  for i:=0 to j do
  begin
    if ListView_GetItemState(grid,i,LVIS_SELECTED)<>0 then
    begin
      k:=0;
      idx:=LV_GetLParam(grid,i);
      while k<qsopt.numcolumns do
      begin
        if not IsColumnMinimized(k) then
          inc(cnt,StrLenW(MainBuf[idx,k].text)+1);
        Inc(k);
      end;
    end;
    if tmpcnt<>cnt then
      inc(cnt,2);
  end;
  if cnt=0 then
    exit;

  inc(cnt);
  mGetMem(buf,cnt*SizeOf(WideChar));
  p:=buf;

  // fill info (need visual column order)
  k:=0;
  while k<qsopt.numcolumns do
  begin
    if not IsColumnMinimized(k) then
    begin
      p:=StrCopyEW(p,TranslateW(qsopt.columns[k].title));
      p^:=#9;
      inc(p);
    end;
    inc(k);
  end;
  (p-1)^:=#13;
  p^    :=#10;
  inc(p);

  for i:=0 to j do
  begin
    if ListView_GetItemState(grid,i,LVIS_SELECTED)<>0 then
    begin
      k:=0;
      idx:=LV_GetLParam(grid,i);
      while k<qsopt.numcolumns do
      begin
        if not IsColumnMinimized(k) then
        begin
          p:=StrCopyEW(p,MainBuf[idx,k].text);
          p^:=#9;
          inc(p);
        end;
        inc(k);
      end;
      (p-1)^:=#13;
      p^    :=#10;
      inc(p);
    end;
  end;
  p^:=#0;
  CopyToClipboard(buf,false);
  mFreeMem(buf);
end;
{
procedure CopySingleLine;
var
  p,pp:pWideChar;
  cnt,num,i:integer;
begin
  // same as multiline - memory column order
  cnt:=0;
  num:=LV_GetLParam(grid,ListView_GetNextItem(grid,-1,LVNI_FOCUSED));
  i:=0;
  while i<qsopt.numcolumns do
  begin
    if not (qsopt.skipminimized and IsColumnMinimized(i)) then
    begin
      p:=TranslateW(qsopt.columns[i].title);
      inc(cnt,StrLenW(p)+1);
      if (StrEndW(p)-1)^<>':' then
        inc(cnt);

      inc(cnt,StrLenW(MainBuf[num,i].text)+2);
    end;
  end;
  if cnt=0 then
    exit;
  mGetMem(pp,(cnt+1)*SizeOf(WideChar));
  p:=pp;

  i:=0;
  while i<qsopt.numcolumns do
  begin
    if not (qsopt.skipminimized and IsColumnMinimized(i)) then
    begin
      p:=StrCopyEW(p,TranslateW(qsopt.columns[i].title));
      if (p-1)^<>':' then
      begin
        p^:=':';
        inc(p);
      end;
      p^:=' '; inc(p);
      p:=StrCopyEW(p,MainBuf[num,i].text);
      p^:=#13; (p+1)^:=#10; inc(p,2);
    end;
  end;
  p^:=#0;

  CopyToClipboard(pp,false);
  mFreeMem(pp);
end;
}
const
  srvhandle:THANDLE=0;
  mnuhandle:THANDLE=0;
  cmcolumn :integer=-1;

function ColChangeFunc(wParam:WPARAM;lParam:LPARAM):int_ptr; cdecl;
var
  pc,pc1:pWideChar;
  p:pAnsiChar;
  tbuf:array [0..255] of WideChar;
  lmodule:pAnsiChar;
  contact:integer;
  col:pcolumnitem;
  qsr:pQSRec;
begin
  col:=@qsopt.columns[cmcolumn];
  StrCopyW(StrCopyEW(@tbuf,TranslateW('Editing of column ')),col.title);
  contact:=FindBufNumber(wParam);
  qsr:=@MainBuf[contact,cmcolumn];
  pc:=qsr.text;
  result:=ShowEditBox(grid,pc,@tbuf);
  if result=-1 then
    exit
  else if result=1 then
  begin
    pc1:=pc;
    pc:=ParseVarString(pc1,wParam);
    mFreeMem(pc1);
  end;
  // change buffer value
  mFreeMem(qsr.text);
  qsr.text:=pc;
  if col.datatype<>QSTS_STRING then
  begin
    qsr.data:=NumToInt(qsr.text);
  end;
  // change database setting value
  if col.module<>nil then
    lmodule:=col.module
  else
  begin
    lmodule:=GetProtoName(FlagBuf[contact].proto);
  end;

  case col.datatype of
    QSTS_BYTE: begin
      DBWriteByte(wParam,lmodule,
          col.setting,qsr.data);
    end;
    QSTS_WORD: begin
      DBWriteWord(wParam,lmodule,
          col.setting,qsr.data);
    end;
    QSTS_DWORD,QSTS_SIGNED,QSTS_HEXNUM: begin
      DBWriteDWord(wParam,lmodule,
          col.setting,dword(qsr.data));
    end;
    QSTS_STRING: begin
      case DBGetSettingType(wParam,lmodule,col.setting) of
        DBVT_ASCIIZ: begin
          WideToAnsi(qsr.text,p,MirandaCP);
          DBWriteString(wParam,lmodule,col.setting,p);
          mFreeMem(p);
        end;
        DBVT_UTF8: begin
          WidetoUTF8(qsr.text,p);
          DBWriteUTF8(wParam,lmodule,col.setting,p);
          mFreeMem(p);
        end;
        DBVT_DELETED,
        DBVT_WCHAR: begin
          DBWriteUnicode(wParam,lmodule,
              col.setting,qsr.text);
        end;
      end;
    end;
  end;

  UpdateLVCell(SendMessage(grid,LVM_GETNEXTITEM,-1,LVNI_FOCUSED),cmcolumn,qsr.text);
end;

function ShowContactMenu(wnd:HWND;hContact:TMCONTACT;col:integer=-1):HMENU;
var
  mi:TCListMenuItem;
  pt:tpoint;
  doit:bool;
begin
  if hContact<>0 then
  begin
    doit:=false;
    if col>=0 then
    begin
      col:=ListViewToColumn(col);
      if (qsopt.columns[col].setting_type=QST_SETTING) and
         // right now, not time or IP
         (qsopt.columns[col].datatype<>QSTS_IP) and 
         (qsopt.columns[col].datatype<>QSTS_TIMESTAMP) then
      begin
        doit:=true;

        if srvhandle=0 then
          srvhandle:=CreateServiceFunction('QS/dummy',@ColChangeFunc);

        cmcolumn:=col;

        FillChar(mi,SizeOf(mi),0);
        mi.cbSize:=SizeOf(mi);
        if mnuhandle=0 then
        begin
          mi.flags     :=CMIF_UNICODE;
          mi.szName.w  :='Change setting through QS';
          mi.pszService:='QS/dummy';
          mnuhandle:=Menu_AddContactMenuItem(@mi);
        end
        else
        begin
          mi.flags :=CMIM_FLAGS;
          CallService(MS_CLIST_MODIFYMENUITEM,mnuhandle,LPARAM(@mi));
        end;
      end;
    end;

    GetCursorPos(pt);
    result:=CallService(MS_CLIST_MENUBUILDCONTACT,hContact,0);
    if result<>0 then
    begin
      TrackPopupMenu(result,0,pt.x,pt.y,0,wnd,nil);
      DestroyMenu(result);
    end;
    // Due to stupid miranda logic, we need to clear tails at service processing, not earlier
    if doit then
    begin
      mi.cbSize:=SizeOf(mi);
      mi.flags :=CMIM_FLAGS or CMIF_HIDDEN;
      CallService(MS_CLIST_MODIFYMENUITEM,mnuhandle,LPARAM(@mi));
    end;

  end
  else
    result:=0;
end;

function MyStrSort(para1:pointer; para2:pointer):int; cdecl;
begin
  result:=StrCmpW(pWideChar(para1),pWideChar(para2));
end;

function MakeContainerMenu(idxfrom:integer=100):HMENU;
var
  sl:TSortedList;
  i:integer;
  b:array [0..15] of AnsiChar;
  p:pWideChar;
begin
  result:=CreatePopupMenu;
  AppendMenuW(result,MF_STRING,idxfrom,TranslateW('default'));
  AppendMenuW(result,MF_SEPARATOR,0,nil);
  FillChar(sl,SizeOf(sl),0);
  sl.increment:=16;
  sl.sortFunc:=@MyStrSort;
  i:=1;
  repeat
    p:=DBReadUnicode(0,'TAB_ContainersW',IntToStr(b,i),nil);
    if p=nil then break;
    List_InsertPtr(@sl,p);
    inc(i);
  until false;
  inc(idxfrom);
  for i:=0 to sl.realCount-1 do
  begin
    p:=pWideChar(sl.Items[i]);
    AppendMenuW(result,MF_STRING,idxfrom+i,p);
    mFreeMem(p);
  end;
  List_Destroy(@sl);
end;

procedure ShowMultiPopup(cnt:integer);
var
  mmenu,grpmenu,cntmenu:HMENU;
  p:PWideChar;
  pt:TPOINT;
  buf:array [0..255] of WideChar;
  i:integer;
begin
  mmenu:=CreatePopupMenu;
  if mmenu=0 then
    exit;

  StrCopyW(buf,TranslateW('Selected'));
  p:=@buf;
  while p^<>#0 do inc(p);
  p^:=' '; inc(p);

  IntToStr(p,cnt);

  while p^<>#0 do inc(p);
  p^:=' '; inc(p);
  StrCopyW(p,TranslateW('contacts'));
  AppendMenuW(mmenu,MF_DISABLED+MF_STRING,0,buf);
  AppendMenuW(mmenu,MF_SEPARATOR,0,nil);
  AppendMenuW(mmenu,MF_STRING,101,TranslateW('&Delete'));
  AppendMenuW(mmenu,MF_STRING,102,TranslateW('&Copy'));
  if ServiceExists(MS_MC_CONVERTTOMETA)<>0 then
    AppendMenuW(mmenu,MF_STRING,103,TranslateW('C&onvert to Meta'));

  cntmenu:=MakeContainerMenu(300);
  AppendMenuW(mmenu,MF_POPUP,cntmenu,TranslateW('Attach to &Tab container'));

  grpmenu:=MakeGroupMenu(400);
  AppendMenuW(mmenu,MF_POPUP,grpmenu,TranslateW('&Move to Group'));
//    grpmenu:=CallService(MS_CLIST_GROUPBUILDMENU,0,0);

  GetCursorPos(pt);
  i:=integer(TrackPopupMenu(mmenu,TPM_RETURNCMD+TPM_NONOTIFY,pt.x,pt.y,0,mainwnd,nil));
  case i of
    101: DeleteByList;
    102: begin
      CopyMultiLinesW({ListView_GetSelectedCount(grid)})
    end;
    103: ConvertToMeta;
    300..399: begin
      if i=300 then // default container, just delete setting
        buf[0]:=#0
      else
      begin
        GetMenuStringW(cntmenu,i,buf,SizeOf(buf),MF_BYCOMMAND);
      end;
      MoveToContainer(buf);
    end;
    400..499: begin
      if i=400 then // root group
        buf[0]:=#0
      else
      begin
        GetMenuStringW(grpmenu,i,buf,SizeOf(buf),MF_BYCOMMAND);
      end;
      MoveToGroup(buf);
    end;
  end;
  DestroyMenu(mmenu);
  if (qsopt.flags and QSO_AUTOCLOSE)<>0 then
    CloseSrWindow;
end;

//----- ListView Columns -----

procedure ColumnClick(wnd:HWND;num:integer);
var
  hdi:THDITEM;
  header:HWND;
begin
  header:=ListView_GetHeader(wnd);

  zeromemory(@hdi,sizeof(hdi));
  // clear sort mark
  hdi.mask:=HDI_FORMAT;
  SendMessage(header,HDM_GETITEM,qsopt.columnsort,lparam(@hdi));
  hdi.fmt:=hdi.fmt and not (HDF_SORTDOWN or HDF_SORTUP);
  SendMessage(header,HDM_SETITEM,qsopt.columnsort,lparam(@hdi));

  if qsopt.columnsort<>num then
  begin
    qsopt.flags:=qsopt.flags or QSO_SORTASC;
    qsopt.columnsort:=num;
  end
  else
    qsopt.flags:=qsopt.flags xor QSO_SORTASC;

  // set new sort mark
  SendMessage(header,HDM_GETITEM,qsopt.columnsort,lparam(@hdi));
  if (qsopt.flags and QSO_SORTASC)=0 then
    hdi.fmt:=hdi.fmt or HDF_SORTDOWN
  else
    hdi.fmt:=hdi.fmt or HDF_SORTUP;
  SendMessage(header,HDM_SETITEM,qsopt.columnsort,lparam(@hdi));

  Sort;
end;

procedure FillLVColumn(column,lvcolumn:integer);
var
  li:LV_ITEMW;
  i:integer;
begin
  FillChar(li,SizeOf(li),0);
  for i:=0 to ListView_GetItemCount(grid)-1 do
  begin
    li.iItem   :=i;
    li.mask    :=LVIF_PARAM;
    li.iSubItem:=0;
    SendMessage(grid,LVM_GETITEM,0,lparam(@li));

    li.pszText :=MainBuf[li.lParam,column].text;
    // Client icons preprocess
    if (((qsopt.columns[column].flags and COL_CLIENT)<>0) and
        ((qsopt.flags and QSO_CLIENTICONS)<>0) and
       (li.pszText<>NIL)) OR
       ((qsopt.columns[column].flags and (COL_XSTATUS or COL_GENDER))<>0) then
      li.mask:=LVIF_IMAGE or LVIF_TEXT
    else
      li.mask:=LVIF_TEXT;
    li.iSubItem:=lvcolumn;
    SendMessageW(grid,LVM_SETITEMW,0,lparam(@li));
  end;
end;

procedure addcolumn(num:integer;column:pcolumnitem);
var
  lvcol:LV_COLUMNW;
  hdi:THDITEM;
begin
  zeromemory(@lvcol,sizeof(lvcol));
  lvcol.mask      :=LVCF_TEXT or LVCF_WIDTH;
  lvcol.pszText   :=TranslateW(column.title);
  lvcol.cx        :=column.width;
  SendMessageW(grid,LVM_INSERTCOLUMNW,num,lparam(@lvcol));

  // set checkbox in column header
  hdi.mask:=HDI_FORMAT;
  if (column.flags and COL_FILTER)<>0 then
    hdi.fmt:=HDF_LEFT or HDF_STRING or HDF_CHECKBOX or HDF_CHECKED
  else
    hdi.fmt:=HDF_LEFT or HDF_STRING or HDF_CHECKBOX;
  SendMessage(ListView_GetHeader(grid),HDM_SETITEM,num,tlparam(@hdi));
end;

procedure MakeColumnMenu;
var
  column:pcolumnitem;
  menu:HMENU;
  pt:TPOINT;
  flag,id,i,j:integer;
begin
  menu:=CreatePopupMenu;
  if menu<>0 then
  begin
    for id:=0 to qsopt.numcolumns-1 do
    begin
      if (qsopt.columns[id].flags and COL_ON)<>0 then
        flag:=MF_CHECKED or MF_STRING
      else
        flag:=MF_UNCHECKED or MF_STRING;
      AppendMenuW(menu,flag,100+id,TranslateW(qsopt.columns[id].title));
    end;
    GetCursorPos(pt);
    id:=integer(TrackPopupMenu(menu,TPM_RETURNCMD+TPM_NONOTIFY,pt.x,pt.y,0,mainwnd,nil));
    if id>=100 then
    begin
      dec(id,100);
      column:=@qsopt.columns[id];
      // show column
      if (column.flags and COL_ON)=0 then
      begin
        column.flags:=column.flags or COL_ON;
        // memory
        if (column.flags and COL_INIT)=0 then
        begin
          for i:=0 to HIGH(MainBuf) do // contacts
          begin
            LoadOneItem(FlagBuf[i].contact,column,FlagBuf[i].proto,MainBuf[i,id]);
          end;
          column.flags:=column.flags or COL_INIT;
        end;
        // screen
        i:=ColumnToListView(id);
        addcolumn(i,column);

        // fill new column
        FillLVColumn(id,i);
      end
      else
      // hide column
      begin
        j:=0;

        for i:=0 to qsopt.numcolumns-1 do
        begin
          if (qsopt.columns[i].flags and COL_ON)<>0 then
            inc(j);
        end;
        // keep at least one visible column (1 + this)
        if j>2 then
        begin
          SendMessage(grid,LVM_DELETECOLUMN,ColumnToListView(id),0);
          column.flags:=column.flags and not COL_ON;
        end;
      end;
    end;
    DestroyMenu(menu);
  end;
end;

function NewLVHProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
begin
  case hMessage of
    WM_RBUTTONUP: begin
      result:=0;
      exit;
    end;

    WM_RBUTTONDOWN: begin
      MakeColumnMenu;
    end;
  end;
  result:=CallWindowProc(OldLVHProc,Dialog,hMessage,wParam,lParam);
end;

var
  HintWnd:HWND;

function NewLVProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  OldHItem   :integer=0;
  OldHSubItem:integer=0;
var
  p:PWideChar;
  buf :array [0..255] of WideChar; //!! for spec columns and patterns now only
  buf1:array [0..127] of AnsiChar;
  tmpCursor:TPOINT;
  pinfo:LV_HITTESTINFO;
  TI:TToolInfoW;
  ics:TCUSTOM_STATUS;

  info:TCLCINFOTIP;
//  qsr:tQSRec;
  i,num:integer;
begin
  result:=0;
  case hMessage of

    WM_DESTROY: begin
      if TTInstalled then
        KillTimer(Dialog,TIMERID_HOVER);
    end;

    WM_LBUTTONDBLCLK: begin
      ShowContactMsgDlg(GetFocusedhContact);
      exit;
    end;

    WM_CHAR: begin
      if wParam=27 then // ESC
      begin
        PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDCANCEL,0);
        exit;
      end;
      case wParam of
        1: begin
          ListView_SetItemState(grid,-1,LVIS_SELECTED,LVIS_SELECTED);
        end;
        // Ctrl-C
        3: begin
//          i:=ListView_GetSelectedCount(grid);
          CopyMultiLinesW();
          exit;
        end;
        // backspace
        8: begin
          if pattern<>nil then
          begin
            StrCopyW(buf,pattern);
            p:=StrEndW(buf);
            (p-1)^:=#0;
            SetDlgItemTextW(mainwnd,IDC_E_SEARCHTEXT,buf);
          end;
        end;
        // letters
        32..127: begin
          if pattern<>nil then
            StrCopyW(buf,pattern)
          else
            buf[0]:=#0;
          p:=StrEndW(buf);
          p^:=WideChar(wParam);
          (p+1)^:=#0;
          SetDlgItemTextW(mainwnd,IDC_E_SEARCHTEXT,buf);
        end;
      end
    end;

    WM_TIMER: begin
      if wParam=TIMERID_HOVER then
      begin
				KillTimer(Dialog,TIMERID_HOVER);
				if GetForegroundWindow<>mainwnd then exit;
        i:=LV_GetLParam(grid,OldHItem);
        FillChar(info,SizeOf(info),0);
        with info do
        begin
          cbSize    :=SizeOf(info);
          hItem     :=FlagBuf[i].contact;
          GetCursorPos(ptCursor);
          tmpCursor :=ptCursor;
{
          ptCursor.x:=loword(lParam);
          ptCursor.y:=hiword(lParam);
}
          SendMessage(grid,LVM_GETITEMRECT,OldHItem,tlparam(@rcItem));
          ScreenToClient(grid,tmpCursor);
          if not PtInRect(rcItem,tmpCursor) then exit;
        end;
//        mGetMem(txt,16384*SizeOf(WideChar));
{
          p:=txt;
          for cnt:=0 to HIGH(MainBuf[0]) do
          begin
            if (qsopt.columns[cnt].flags and COL_ON)=0 then
            begin
              LoadOneItem(info.hItem,cnt,FlagBuf[i].proto,qsr);
              if qsr.text<>nil then
              begin
                if qsr.text^<>#0 then
                begin
//!! need: buffer free space check here
num:=StrLenW(qsopt.columns[cnt].title)+StrLenW(qsr.text)+4;
if (16384-num)>(p-txt) then
begin

                  p:=StrCopyEW(p,qsopt.columns[cnt].title);
                  p^:=':'; inc(p); p^:=' '; inc(p);
                  p:=StrCopyEW(p,qsr.text);
                  p^:=#13; inc(p); p^:=#10; inc(p);
end
else
begin
  mFreeMem(qsr.text);
  break;
end;
                end;
                mFreeMem(qsr.text);
              end;
            end;
          end;
          p^:=#0;
}
        CallService(MS_TIPPER_SHOWTIPW,0{twparam(txt)},tlparam(@info));
//        mFreeMem(txt);
        TTShowed:=true;
      end;
    end;

    WM_MOUSEMOVE: begin
      pinfo.pt.x:=loword(lParam);
      pinfo.pt.y:=hiword(lParam);
      pinfo.flags:=0;
      if integer(SendMessage(grid,LVM_SUBITEMHITTEST,0,tlparam(@pinfo)))<>-1 then
      begin
        if ((pinfo.flags and LVHT_ONITEM)<>0) and
            ((pinfo.iItem<>OldHItem) or (pinfo.iSubItem<>OldHSubItem)) then
        begin
          OldHSubItem:=pinfo.iSubItem;
          OldHItem   :=pinfo.iItem;

          if TTInstalled then
          begin
            if TTShowed then
            begin
              TTShowed:=false;
              CallService(MS_TIPPER_HIDETIP,0,0);
            end;
            KillTimer(Dialog, TIMERID_HOVER);
            if OldHSubItem=0 then
            begin
              SetTimer(Dialog, TIMERID_HOVER, 450, nil);
              exit;
            end;
          end;
//!!
          with TI do
          begin
            cbSize:=SizeOf(TI);
            uFlags:=TTF_SUBCLASS+TTF_IDISHWND;
            hWnd  :=mainwnd;
            uId   :=Dialog;
            hInst :=0;
          end;

          num:=ListViewToColumn(OldHSubItem);
          if (qsopt.columns[num].flags and
             (COL_XSTATUS or COL_GENDER))<>0 then
          begin
            i:=LV_GetLParam(grid,OldHItem);
//            TTShowed:=true;
            if (qsopt.columns[num].flags and COL_GENDER)<>0 then
            begin
              case MainBuf[i,num].data of
                77: TI.lpszText:=tstrMale;
                70: TI.lpszText:=tstrFemale;
              else
                TI.lpszText:=tstrUnknown;
              end;
            end
            else // if (qsopt.columns[num].flags and COL_XSTATUS)<>0 then
            begin
              StrCopyW(buf,MainBuf[i,num].text);
              ics.flags:=CSSF_DEFAULT_NAME or CSSF_MASK_NAME or CSSF_UNICODE;

              StrCopy(StrCopyE(buf1,GetProtoName(FlagBuf[i].proto)),PS_GETCUSTOMSTATUSEX);

              i:=StrToInt(buf);
              ics.wParam:=@i;
              ics.cbSize:=SizeOf(ics);
              ics.szName.w:=@buf;

              CallService(buf1,0,tlparam(@ics));
              TI.lpszText:=TranslateW(@buf);
            end;
          end
          else
          begin
            TI.lpszText:=nil;
//            TTShowed:=false;
          end;
          SendMessageW(HintWnd,TTM_SETTOOLINFOW,0,tlparam(@TI));
        end
      end;
    end;

    WM_KEYUP: begin
      case wParam of
        VK_RETURN: begin
          if ListView_GetSelectedCount(grid)=1 then
            ShowContactMsgDlg(GetFocusedhContact);
          exit;
        end;
        VK_INSERT: begin
          CallService(MS_FINDADD_FINDADD,0,0);
          exit;
        end;
        VK_DELETE: begin
          lParam:=ListView_GetSelectedCount(grid);
          if lParam>1 then
            DeleteByList
          else if lParam=1 then
            DeleteOneContact(GetFocusedhContact);
          exit;
        end;
        VK_F5: begin
          PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_REFRESH,0);
          exit;
        end;
      end;
    end;

    WM_NOTIFY: begin
      case integer(PNMHdr(lParam)^.code) of
        HDN_ITEMSTATEICONCLICK: begin
          if ((PHDNotify(lParam)^.pitem^.mask and HDI_FORMAT  )<>0) and
             ((PHDNotify(lParam)^.pitem^.fmt  and HDF_CHECKBOX)<>0) then
          begin
            i:=ListViewToColumn(PHDNotify(lParam)^.Item);

            if (PHDNotify(lParam)^.pitem^.fmt and HDF_CHECKED)=0 then // OLD state
            begin
              qsopt.columns[i].flags:=qsopt.columns[i].flags or COL_FILTER;
              PHDNotify(lParam)^.pitem^.fmt:=PHDNotify(lParam)^.pitem^.fmt or HDF_CHECKED
            end
            else
            begin
              qsopt.columns[i].flags:=qsopt.columns[i].flags and not COL_FILTER;
              PHDNotify(lParam)^.pitem^.fmt:=PHDNotify(lParam)^.pitem^.fmt and not HDF_CHECKED
            end;
            SendMessage(
              PHDNotify(lParam)^.hdr.hWndFrom,HDM_SETITEM,
              PHDNotify(lParam)^.Item,tlparam(PHDNotify(lParam)^.pitem));
//            result:=1;
            FillGrid;
            exit;
          end;
        end;
        HDN_ENDDRAG: begin
        end;
      end;
    end;
  end;
  result:=CallWindowProc(OldLVProc,Dialog,hMessage,wParam,lParam);
end;

//----- Single cell painting -----

procedure SetCellColor(lplvcd:PNMLVCUSTOMDRAW;idx:integer);
begin
  if (qsopt.flags and QSO_COLORIZE)<>0 then
  begin
    with FlagBuf[idx] do 
    begin
      if (flags and QSF_ACCDEL)<>0 then
      begin
        lplvcd^.clrTextBk:=QSColors[bkg_del].color;
        lplvcd^.clrText  :=QSColors[fgr_del].color;
      end
      else if (flags and QSF_ACCOFF)<>0 then
      begin
        lplvcd^.clrTextBk:=QSColors[bkg_dis].color;
        lplvcd^.clrText  :=QSColors[fgr_dis].color;
      end
      else if (flags and QSF_META)<>0 then
      begin
        lplvcd^.clrTextBk:=QSColors[bkg_meta].color;
        lplvcd^.clrText  :=QSColors[fgr_meta].color;
      end
      else if (flags and QSF_SUBMETA)<>0 then
      begin
        lplvcd^.clrTextBk:=QSColors[bkg_sub].color;
        lplvcd^.clrText  :=QSColors[fgr_sub].color;
      end
      else if (flags and QSF_INLIST)=0 then
      begin
        lplvcd^.clrTextBk:=QSColors[bkg_hid].color;
        lplvcd^.clrText  :=QSColors[fgr_hid].color;
      end
      else
        idx:=-1;
    end;
  end
  else
    idx:=-1;
  if idx<0 then
  begin
    if ((qsopt.flags and QSO_DRAWGRID)=0) and odd(lplvcd^.nmcd.dwItemSpec) then
    begin
      lplvcd^.clrTextBk:=QSColors[bkg_odd].color;
      lplvcd^.clrText  :=QSColors[fgr_odd].color;
    end
    else
    begin
      lplvcd^.clrTextBk:=QSColors[bkg_norm].color;
      lplvcd^.clrText  :=QSColors[fgr_norm].color;
    end;
  end;
end;

function ProcessCustomDraw(lParam:LPARAM):integer;
var
  lplvcd:PNMLVCUSTOMDRAW;
  h:HICON;
  MirVerW:pWideChar;
  buf:array [0..255] of AnsiChar;
  rc:TRECT;
  i,j,sub:integer;
begin
  lplvcd:=pointer(lParam);
  result:=CDRF_DODEFAULT;
  case lplvcd^.nmcd.dwDrawStage of
    CDDS_PREPAINT: begin
      result:=CDRF_NOTIFYITEMDRAW;
      exit;
    end;
    CDDS_ITEMPREPAINT: begin
      result:=CDRF_NOTIFYSUBITEMDRAW;

      SetCellColor(lplvcd,lplvcd^.nmcd.lItemlParam);

      exit;
    end;
    CDDS_SUBITEM or CDDS_ITEMPREPAINT: begin

      i:=lplvcd^.nmcd.lItemlParam;
      SetCellColor(lplvcd,i);

      sub:=ListViewToColumn(lplvcd^.iSubItem);

      if (qsopt.columns[sub].flags and COL_GENDER)<>0 then
      begin
        ListView_GetSubItemRect(grid,lplvcd^.nmcd.dwItemSpec,lplvcd^.iSubItem,LVIR_ICON,@rc);

        case MainBuf[i,sub].data of
          70: h:=hIconF;
          77: h:=hIconM;
        else
          h:=0;
        end;
        if h<>0 then
        begin
          DrawIconEx(lplvcd^.nmcd.hdc,rc.left+1,rc.top,h,16,16,0,0,DI_NORMAL);
        end;
        result:=CDRF_SKIPDEFAULT;
      end

      else if (qsopt.columns[sub].flags and COL_XSTATUS)<>0 then
      begin
        j:=StrToInt(MainBuf[i,sub].text);
        if j>0 then
        begin
          StrCopy(StrCopyE(buf,GetProtoName(FlagBuf[i].proto)),PS_GETCUSTOMSTATUSICON);
          if ServiceExists(buf)<>0 then
          begin
            h:=CallService(buf,j,LR_SHARED);

            ListView_GetSubItemRect(grid,lplvcd^.nmcd.dwItemSpec,lplvcd^.iSubItem,LVIR_ICON,@rc);
            DrawIconEx(lplvcd^.nmcd.hdc,rc.left+1,rc.top,h,16,16,0,0,DI_NORMAL);
          end;
        end;
        result:=CDRF_SKIPDEFAULT;
      end

      else if ((qsopt.flags and QSO_CLIENTICONS)<>0) and
           ((qsopt.columns[sub].flags and COL_CLIENT)<>0) then
        result:=CDRF_NOTIFYPOSTPAINT;
    end;

    CDDS_SUBITEM or CDDS_ITEMPOSTPAINT: begin
      sub:=ListViewToColumn(lplvcd^.iSubItem);
      if (qsopt.columns[sub].flags and COL_CLIENT)<>0 then
      begin
        MirVerW:=MainBuf[lplvcd^.nmcd.lItemlParam,sub].text;

//!!
        if (MirVerW<>nil) and (MirVerW[0]<>#0) and (ServiceExists(MS_FP_GETCLIENTICONW)<>0) then
        begin
          h:=CallService(MS_FP_GETCLIENTICONW,tlparam(MirVerW),0);
          ListView_GetSubItemRect(grid,lplvcd^.nmcd.dwItemSpec,lplvcd^.iSubItem,LVIR_ICON,@rc);
          DrawIconEx(lplvcd^.nmcd.hdc,rc.left+1,rc.top,h,16,16,0,0,DI_NORMAL);
          DestroyIcon(h);
        end;
        result:=CDRF_SKIPDEFAULT;
      end;
    end;
  end;
end;


function NewEditProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  li:LV_ITEM;
  count,current,next,perpage:integer;
begin
  result:=0;
  case hMessage of 
    WM_CHAR: if wParam=27 then
    begin
      PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDCANCEL,0);
      exit;
    end;
    WM_KEYUP: if wParam=VK_RETURN then
    begin
      if ListView_GetSelectedCount(grid)=1 then
        ShowContactMsgDlg(GetFocusedhContact);
      exit;
    end;
    WM_KEYDOWN: begin
      count  :=ListView_GetItemCount(grid);
      current:=ListView_GetNextItem(grid,-1,LVNI_FOCUSED);
      next:=-1;
      if count>0 then
        case wParam of
          VK_NEXT,VK_PRIOR: begin
            perpage:=ListView_GetCountPerPage(grid);
            if wParam=VK_NEXT then
              next:=Min(current+perpage,count)
            else
              next:=Max(current-perpage,0);
          end;
          VK_UP: begin
            if current>0 then
              next:=current-1
          end;
          VK_DOWN: begin
            if current<count-1 then
            next:=current+1
          end;
          VK_F5: begin
            PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_REFRESH,0);
            exit;
          end;
        end;
      if next>=0 then
      begin
        li.statemask:=LVIS_SELECTED;
        li.state:=0;
        SendMessage(grid,LVM_SETITEMSTATE,twparam(-1),tlparam(@li));
        ListView_SetItemState(grid,next,LVIS_FOCUSED or LVIS_SELECTED,
            LVIS_FOCUSED or LVIS_SELECTED);
        SendMessage(grid,LVM_ENSUREVISIBLE,next,0);
        result:=0;
        exit;
      end;
    end;
  end;
  result:=CallWindowProc(OldEditProc,Dialog,hMessage,wParam,lParam);
end;

procedure ClearBuffers;
var
  w,h:integer;
begin
  for w:=0 to HIGH(MainBuf) do
    for h:=0 to HIGH(MainBuf[0]) do
      mFreeMem(MainBuf[w,h].text);

  SetLength(MainBuf,0);
  SetLength(FlagBuf,0);
end;

procedure SetSpecialColumns(num:integer);
begin
  with qsopt.columns[num] do
  begin
    if setting_type=QST_SETTING then
    begin
      if (datatype=QSTS_STRING) and
         (StrCmp(module ,'CList')=0) and
         (StrCmp(setting,'Group')=0) then
      begin
        flags:=flags or COL_GROUP
      end

      else if (datatype=QSTS_STRING) and
         (StrCmp(module ,'Tab_SRMsg' )=0) and
         (StrCmp(setting,'containerW')=0) then
      begin
        flags:=flags or COL_CNTNR
      end

      else if (datatype=QSTS_BYTE) and
         (lstrcmpia(setting,'XStatusId')=0) then
      begin
        flags:=flags or COL_XSTATUS;
      end

      else if (datatype=QSTS_STRING) and
         (StrCmp(setting,'MirVer')=0) and
         (ServiceExists(MS_FP_GETCLIENTICONW)<>0) then
        flags:=flags or COL_CLIENT;

    end
    else if (setting_type=QST_CONTACTINFO) and (cnftype=CNF_GENDER) then
    begin
      if hIconF=0 then hIconF:=CallService(MS_SKIN2_GETICON,0,tlparam(QS_FEMALE));
      if hIconM=0 then hIconM:=CallService(MS_SKIN2_GETICON,0,tlparam(QS_MALE));
      flags:=flags or COL_GENDER;
      tstrMale   :=TranslateW('Male');
      tstrFemale :=TranslateW('Female');
      tstrUnknown:=TranslateW('Unknown');
    end;

    qsopt.columns[num].flags:=flags;
  end;
end;

// Set columns and clear listview
procedure PrepareTable(reset:boolean=false);
var
  lvcol:LV_COLUMNW;
  hdi:THDITEM;
  i:integer;
  old:integer;
begin
  SendMessage(grid,LVM_DELETEALLITEMS,0,0);

  zeromemory(@hdi,sizeof(hdi));
  hdi.mask:=HDI_FORMAT;

  old:=tablecolumns;
  tablecolumns:=0;
  zeromemory(@lvcol,sizeof(lvcol));
  lvcol.mask:=LVCF_TEXT or LVCF_WIDTH;
  for i:=0 to qsopt.numcolumns-1 do
  begin
    with qsopt.columns[i] do
    begin
      if (flags and COL_ON)<>0 then
      begin
        addcolumn(tablecolumns,@qsopt.columns[i]);
        inc(tablecolumns);
      end;

      SetSpecialColumns(i);
    end;
  end;

  if reset then
  begin
    for i:=old+tablecolumns-1 downto tablecolumns do
    begin
      SendMessage(grid,LVM_DELETECOLUMN,i,0);
    end;
  end;

  ListView_SetItemCount(grid,HIGH(FlagBuf)+1);
end;

//----- Miranda Events -----

procedure ChangeStatusPicture(row:integer; hContact:THANDLE; Pic:integer);
var
  li:LV_ITEMW;
begin
  row:=FindItem(row);
  if row>=0 then
  begin
    li.iItem   :=row;
    li.iSubItem:=0;
    li.mask    :=LVIF_IMAGE;
    li.iImage  :=Pic;//CallService(MS_CLIST_GETCONTACTICON,hContact,0);
    SendMessageW(grid,LVM_SETITEMW,0,lparam(@li));
  end;
end;

function OnStatusChanged(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  j:integer;
  oldstat,newstat:integer;
begin
  result:=0;

  j:=FindBufNumber(wParam);
  if j>=0 then
  begin
    oldstat:=FlagBuf[j].status;
    newstat:=DBReadWord(wParam,GetProtoName(FlagBuf[j].proto),'Status',ID_STATUS_OFFLINE);
    FlagBuf[j].status:=newstat;

    if (oldstat<>ID_STATUS_OFFLINE) and (newstat<>ID_STATUS_OFFLINE) then
      ChangeStatusPicture(j,wParam,lParam)
    else if (oldstat=ID_STATUS_OFFLINE) {and (newstat<>ID_STATUS_OFFLINE)} then
    begin
      if (qsopt.flags and QSO_SHOWOFFLINE)<>0 then
        ChangeStatusPicture(j,wParam,lParam)
      else
        ProcessLine(j,true) // why false? need to filter!
    end
    else if {(oldstat<>ID_STATUS_OFFLINE) and} (newstat=ID_STATUS_OFFLINE) then
    begin
      if (qsopt.flags and QSO_SHOWOFFLINE)<>0 then
        ChangeStatusPicture(j,wParam,lParam)
      else
      begin
        FlagBuf[j].flags:=FlagBuf[j].flags and not QSF_ACTIVE;
        ListView_DeleteItem(grid,FindItem(j));
      end;
    end;

  // refresh table to new filtering
    if (qsopt.flags and QSO_SORTBYSTATUS)<>0 then
      Sort;
    UpdateSB;
  end;
end;

function OnContactAdded(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  i:integer;
begin
  result:=0;
  // refresh table to add contact
  i:=Length(MainBuf);
  SetLength(MainBuf,i+1);
  SetLength(MainBuf[i],qsopt.numcolumns);
  FillChar(MainBuf[i][0],qsopt.numcolumns*SizeOf(tQSRec),0);
  SetLength(FlagBuf,i+1);

  AddContact(i,wParam);
  ProcessLine(i);
  Sort;
  UpdateSB;
end;

function OnContactDeleted(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  i,j:integer;
begin
  result:=0;
  i:=FindBufNumber(wParam);
  if i>=0 then
  begin
    FlagBuf[i].flags:=(FlagBuf[i].flags or QSF_DELETED) and not QSF_ACTIVE;
    for j:=0 to HIGH(MainBuf[0]) do
      mFreeMem(MainBuf[i,j].text);
    i:=FindItem(i);
    if i>=0 then
      ListView_DeleteItem(grid,i);
    UpdateSB;
  end;
end;
{
function OnAccountChanged(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  result:=0;

  case wParam of
    PRAC_ADDED: begin
    end;
    PRAC_REMOVED: begin
    end;
    PRAC_CHECKED: begin
      with PPROTOACCOUNT(lParam)^ do
      begin
        if bIsEnabled<>0 then
        begin
        end
        else
        begin
        end;
      end;
    end;
  end;
end;
}
//----- Main window procedure with support -----

function FindAddDlgResizer(Dialog:HWND;lParam:LPARAM;urc:PUTILRESIZECONTROL):int; cdecl;
begin
  case urc^.wId of
    IDCANCEL:           result:=RD_ANCHORX_RIGHT or RD_ANCHORY_TOP;
    IDC_REFRESH:        result:=RD_ANCHORX_RIGHT or RD_ANCHORY_TOP;
    IDC_CH_SHOWOFFLINE: result:=RD_ANCHORX_LEFT  or RD_ANCHORY_TOP;
    IDC_CH_COLORIZE:    result:=RD_ANCHORX_LEFT  or RD_ANCHORY_TOP;
    IDC_CB_PROTOCOLS:   result:=RD_ANCHORX_LEFT  or RD_ANCHORY_TOP;
    IDC_E_SEARCHTEXT:   result:=RD_ANCHORX_WIDTH or RD_ANCHORY_TOP;
    IDC_LIST:           result:=RD_ANCHORX_WIDTH or RD_ANCHORY_HEIGHT;
    IDC_STATUSBAR:      result:=RD_ANCHORX_WIDTH or RD_ANCHORY_BOTTOM;
  else
    result:=0;
  end;
end;

procedure FillProtoCombo(cb:HWND);
var
  i:integer;
begin
  SendMessage(cb,CB_RESETCONTENT,0,0);
  CB_AddStrDataW(cb,TranslateW('All'));
  for i:=1 to GetNumProto do
  begin
    CB_AddStrDataW(cb,GetProtoAccName(i),i);
  end;
  SendMessage(cb,CB_SETCURSEL,0,0);
end;

var
  hAdd,
  hDelete,
//  hAccount,
  hChange:THANDLE;

procedure SaveColumnOrder;
var
  tmpcolumns:array [0..MaxColumnAmount-1] of integer;
  lvc:LV_COLUMNW;
  i,idx,num,cnt:integer;
begin
  DBDeleteGroup(0,qs_module,'item*');
  idx:=0;
  lvc.mask:=LVCF_ORDER or LVCF_WIDTH;
  for i:=0 to qsopt.numcolumns-1 do
  begin
    if qsopt.columns[i].setting_type<>0 then
    begin
      if (qsopt.columns[i].flags and COL_ON)<>0 then
      begin
        SendMessageW(grid,LVM_GETCOLUMN,idx,tlparam(@lvc));
        qsopt.columns[i].width:=lvc.cx;
        tmpcolumns[lvc.iOrder]:=i;
        inc(idx);
      end;
    end;
  end;
  idx:=0;
  cnt:=0;
  for i:=0 to qsopt.numcolumns-1 do
  begin
    if qsopt.columns[i].setting_type<>0 then
    begin
      if (qsopt.columns[i].flags and COL_ON)<>0 then
      begin
        num:=tmpcolumns[idx];
        inc(idx);
      end
      else
        num:=i;
      savecolumn(cnt,qsopt.columns[num]);
      inc(cnt);
    end
  end;
end;

function QSMainWndProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  smenu:HMENU;
  header:HWND;
  hdi:THDITEM;
  w,h:uint_ptr;
  tmp:LONG_PTR;
  buf:array [0..255] of WideChar;
  colarr:array [0..127] of integer absolute buf;
  rc:TRECT;
  pt:TPOINT;
  TI:tToolInfoW;
  urd:TUTILRESIZEDIALOG;
begin
  result:=0;
  case hMessage of
    WM_DESTROY: begin
      if srvhandle<>0 then DestroyServiceFunction(srvhandle);
      if mnuhandle<>0 then CallService(MO_REMOVEMENUITEM,mnuhandle,0);

      UnhookEvent(hAdd);
      UnhookEvent(hDelete);
      UnhookEvent(hChange);
//      UnhookEvent(hAccount);

      UnhookEvent(colorhook);

      mainwnd:=0;

      StatusBar:=0;
      GetWindowRect(Dialog,rc);

      CopyRect(qsopt.grrect,rc);

      // save column width/order
      if grid<>0 then
        SaveColumnOrder
      else
        grid:=0;

      saveopt_wnd;

      ListView_SetImageList(GetDlgItem(Dialog,IDC_LIST),0,LVSIL_SMALL);

      if (qsopt.flags and QSO_SAVEPATTERN)<>0 then
      begin
        DBWriteUnicode(0,qs_module,'pattern',pattern);
      end;

      mFreeMem(patstr);
      mFreeMem(pattern);

      ClearBuffers;

    end;

    WM_INITDIALOG: begin
      srvhandle:=0;
      mnuhandle:=0;

      SetWindowTextW(Dialog,'Quick Search');

      StatusBar:=GetDlgItem(Dialog,IDC_STATUSBAR);

      smenu:=GetSystemMenu(Dialog,false);
      InsertMenu (smenu,5,MF_BYPOSITION or MF_SEPARATOR,0,nil);
      InsertMenuW(smenu,6,MF_BYPOSITION or MF_STRING,
        IDM_STAYONTOP,TranslateW('Stay on Top'));

      if (qsopt.flags and QSO_STAYONTOP)<>0 then
      begin
        CheckMenuItem(smenu,IDM_STAYONTOP,MF_BYCOMMAND or MF_CHECKED);
        SetWindowPos(Dialog,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE or SWP_NOSIZE);
      end;
      AdvFilter:=0;
      CheckDlgButton(Dialog,IDC_CH_SHOWOFFLINE,ORD((qsopt.flags and QSO_SHOWOFFLINE)<>0));
      if (qsopt.flags and QSO_SHOWOFFLINE)<>0 then
      begin
        AdvFilter:=AdvFilter or flt_show_offline;
      end;

      CheckDlgButton(Dialog,IDC_CH_COLORIZE,ORD((qsopt.flags and QSO_COLORIZE)<>0));

      // Window
      mainwnd:=Dialog;
      tmp:=GetWindowLongPtrW(Dialog,GWL_EXSTYLE);
      if (qsopt.flags and QSO_TOOLSTYLE)<>0 then
        tmp:=tmp or WS_EX_TOOLWINDOW
      else
        tmp:=tmp and not WS_EX_TOOLWINDOW;
      SetWindowLongPtrW(Dialog,GWL_EXSTYLE,tmp);

      SendMessage(Dialog,WM_SETICON,ICON_SMALL,//LoadIcon(hInstance,PAnsiChar(IDI_QS))
        CallService(MS_SKIN2_GETICON,0,tlparam(QS_QS)));
      grid:=GetDlgItem(Dialog,IDC_LIST);

      // ListView
      ListView_SetImageList(grid,
         CallService(MS_CLIST_GETICONSIMAGELIST,0,0),LVSIL_SMALL);

      tmp:=LVS_EX_FULLROWSELECT or LVS_EX_SUBITEMIMAGES or LVS_EX_HEADERDRAGDROP or
           LVS_EX_LABELTIP or LVS_EX_DOUBLEBUFFER;
      if (qsopt.flags and QSO_DRAWGRID)<>0 then
        tmp:=tmp or LVS_EX_GRIDLINES;
      SendMessage(grid,LVM_SETEXTENDEDLISTVIEWSTYLE,0,tmp);

      // ListView header
      header:=ListView_GetHeader(grid);
      SetWindowLongPtrW(header,GWL_STYLE,
        GetWindowLongPtrW(header,GWL_STYLE) or HDS_CHECKBOXES);

      OldLVProc  :=pointer(SetWindowLongPtrW(grid,GWL_WNDPROC,LONG_PTR(@NewLVProc)));
      OldEditProc:=pointer(SetWindowLongPtrW(GetDlgItem(Dialog,IDC_E_SEARCHTEXT),
         GWL_WNDPROC,LONG_PTR(@NewEditProc)));

      OldLVHProc:=pointer(SetWindowLongPtrW(
          SendMessage(grid,LVM_GETHEADER,0,0),
          GWL_WNDPROC,LONG_PTR(@NewLVHProc)));

      FillProtoCombo(GetDlgItem(Dialog,IDC_CB_PROTOCOLS));

      PrepareTable;

      if pattern<>nil then
      begin
        SetDlgItemTextW(Dialog,IDC_E_SEARCHTEXT,pattern)
      end
      else
      begin
        buf[0]:=#0;
        SetDlgItemTextW(Dialog,IDC_E_SEARCHTEXT,@buf);
        FillGrid;
      end;

      // Show sorting column
      zeromemory(@hdi,sizeof(hdi));
      hdi.mask:=HDI_FORMAT;
      SendMessageW(header,HDM_GETITEM,qsopt.columnsort,tlparam(@hdi));
      if (qsopt.flags and QSO_SORTASC)<>0 then
        hdi.fmt:=hdi.fmt or HDF_SORTDOWN
      else
        hdi.fmt:=hdi.fmt or HDF_SORTUP;
      SendMessageW(header,HDM_SETITEM,qsopt.columnsort,tlparam(@hdi));


      TranslateDialogDefault(Dialog);

      SnapToScreen(qsopt.grrect);
      with qsopt.grrect do
        MoveWindow(Dialog,left,top,right-left,bottom-top,false);

      with TI do
      begin
        cbSize     :=SizeOf(TI);
        uFlags     :=TTF_SUBCLASS+TTF_IDISHWND;
        hWnd       :=Dialog;
        uId        :=grid;
        hInst      :=0;
        lpszText   :=nil;
      end;
      HintWnd:=CreateWindowExW(0,TOOLTIPS_CLASS,nil,0,
          integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
          integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
          Dialog,0,HInstance,NIL);

      SendMessageW(HintWnd,TTM_ADDTOOLW,0,tlparam(@TI));
      colorhook:=HookEvent(ME_COLOUR_RELOAD,@ColorReload);

      hAdd   :=HookEvent(ME_DB_CONTACT_ADDED        ,@OnContactAdded);
      hDelete:=HookEvent(ME_DB_CONTACT_DELETED      ,@OnContactDeleted);
      hChange:=HookEvent(ME_CLIST_CONTACTICONCHANGED,@OnStatusChanged);
//      hAccount:=HookEvent(ME_PROTO_ACCLISTCHANGED    ,@OnAccountChanged);
    end;

    WM_GETMINMAXINFO: begin
      with PMINMAXINFO(lParam)^ do
      begin
        ptMinTrackSize.x:=300;
        ptMinTrackSize.y:=160;
      end;
    end;

    WM_SIZE: begin
      SendMessage(StatusBar,WM_SIZE,0,0);
      FillChar(urd,SizeOf(TUTILRESIZEDIALOG),0);
      urd.cbSize    :=SizeOf(urd);
      urd.hwndDlg   :=Dialog;
      urd.hInstance :=hInstance;
      urd.lpTemplate:=MAKEINTRESOURCEA(IDD_MAIN);
      urd.lParam    :=0;
      urd.pfnResizer:=@FindAddDlgResizer;
      CallService(MS_UTILS_RESIZEDIALOG,0,tlparam(@urd));
    end;

    WM_SYSCOMMAND: begin
      if wParam=IDM_STAYONTOP then
      begin
        if (qsopt.flags and QSO_STAYONTOP)<>0 then
        begin
          h:=MF_BYCOMMAND or MF_UNCHECKED;
          w:=HWND_NOTOPMOST;
        end
        else
        begin
          h:=MF_BYCOMMAND or MF_CHECKED;
          w:=HWND_TOPMOST;
        end;
        CheckMenuItem(GetSystemMenu(Dialog,false),IDM_STAYONTOP,h);
        SetWindowPos(Dialog,w,0,0,0,0,SWP_NOMOVE or SWP_NOSIZE);
        qsopt.flags:=qsopt.flags xor QSO_STAYONTOP;
        exit;
      end;
    end;

    WM_CONTEXTMENU: begin
      if wParam=tWPARAM(GetDlgItem(Dialog,IDC_LIST)) then
      begin
        w:=ListView_GetSelectedCount(grid);
        if w>1 then
          ShowMultiPopup(w)
        else
        begin
          ShowContactMenu(Dialog,GetFocusedhContact,
            GetLVSubItem(loword(lParam),hiword(lParam)));
        end;
      end;
    end;

    WM_MEASUREITEM:
      CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);
    WM_DRAWITEM:
      CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);

    WM_MOUSEMOVE: begin
      if TTInstalled then
      begin
        GetWindowRect(grid,rc);
        pt.x:=loword(lParam);
        pt.y:=hiword(lParam);
        ClientToScreen(Dialog,pt);
        if not PtInRect(rc,pt) then
        begin
          if TTShowed then
          begin
            TTShowed:=false;
            CallService(MS_TIPPER_HIDETIP,0,0);
          end;
          KillTimer(grid,TIMERID_HOVER);
        end;
      end;
    end;

    WM_KEYDOWN: begin
      case wParam of
        VK_F5: begin
          PostMessage(Dialog,WM_COMMAND,(BN_CLICKED shl 16)+IDC_REFRESH,0);
          exit;
        end;
      end;
    end;

    WM_COMMAND: begin
      if CallService(MS_CLIST_MENUPROCESSCOMMAND,
         MAKEWPARAM(LOWORD(wParam),MPCF_CONTACTMENU),
         GetFocusedhContact)<>0 then
      begin
        if (qsopt.flags and QSO_AUTOCLOSE)<>0 then
          CloseSrWindow;
        exit;
      end;

      case wParam shr 16 of
        CBN_SELCHANGE: begin
          AdvFilter:=(AdvFilter and not $FF) or cardinal(CB_GetData(lParam));
          AdvancedFilter;
        end;

        EN_CHANGE: begin
          GetDlgItemTextW(Dialog,IDC_E_SEARCHTEXT,buf,sizeOf(buf));
          mFreeMem(pattern);
          StrDupW(pattern,buf);
          if pattern<>nil then
            CharLowerW(pattern);
          FillGrid; //!!
        end;

        BN_CLICKED: begin
          case loword(wParam) of
            IDC_CH_SHOWOFFLINE: begin
              if IsDlgButtonChecked(Dialog,IDC_CH_SHOWOFFLINE)<>BST_UNCHECKED then
              begin
                qsopt.flags:=qsopt.flags or QSO_SHOWOFFLINE;
                AdvFilter:=AdvFilter or flt_show_offline
              end
              else
              begin
                qsopt.flags:=qsopt.flags and not QSO_SHOWOFFLINE;
                AdvFilter:=AdvFilter and not flt_show_offline;
              end;
              AdvancedFilter;
            end;

            IDC_CH_COLORIZE: begin
              if IsDlgButtonChecked(Dialog,IDC_CH_COLORIZE)=BST_UNCHECKED then
                qsopt.flags:=qsopt.flags and not QSO_COLORIZE
              else
                qsopt.flags:=qsopt.flags or QSO_COLORIZE;
              RedrawWindow(grid,nil,0,RDW_INVALIDATE);
            end;

            IDC_REFRESH: begin
              ClearBuffers;
              PrepareToFill;
              PrepareTable(true);
              FillGrid;
            end;

            IDCANCEL: CloseSrWindow();
          end;
        end;
      end;
    end;

    WM_NOTIFY: begin
      case integer(PNMHdr(lParam)^.code) of
        LVN_COLUMNCLICK: begin
          ColumnClick(PNMListView(lParam)^.hdr.hwndFrom,PNMListView(lParam)^.iSubItem);
        end;
        NM_CUSTOMDRAW: begin
          if PNMHdr(lParam)^.hwndFrom=grid then
          begin
            SetWindowLongPtrW(Dialog,DWL_MSGRESULT,ProcessCustomDraw(lParam));
            result:=1;
          end;
        end;
      end;
    end;

//  else
//    result:=DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;

//----- base QS window functions -----

function CloseSrWindow(save:boolean=true):boolean;
begin
  if mainwnd<>0 then
  begin
    result:=true;
    //!! cheat
    if not save then
      grid:=0;

    DestroyWindow(mainwnd);

    FreeProtoList;
  end
  else
    result:=false;
end;

function BringToFront:integer;
var
  wp:TWINDOWPLACEMENT;
begin
  result:=1;
  wp.length:=SizeOf(TWINDOWPLACEMENT);
  GetWindowPlacement(mainwnd,@wp);
  if wp.showCmd=SW_SHOWMINIMIZED then
    ShowWindow(mainwnd,SW_RESTORE);
  SetForegroundWindow(mainwnd);
end;

function OpenSRWindow(apattern:PWideChar;flags:LPARAM):boolean;
var
  i,j:integer;
begin
  result:=true;
  if mainwnd<>0 then
  begin
    BringToFront;
    exit;
  end;

  j:=0;
  for i:=0 to qsopt.numcolumns-1 do
  begin
    if (qsopt.columns[i].flags and COL_ON)<>0 then
      inc(j);
  end;
  // no even one visible column
  if j=0 then
    exit;

  TTInstalled := ServiceExists(MS_TIPPER_SHOWTIP)<>0;
  // too lazy to move pattern and flags to thread
  if apattern<>nil then
  begin
    if flags=0 then
      StrDupW(pattern,apattern)
    else
      AnsiToWide(PAnsiChar(apattern),pattern);
    CharLowerW(pattern);
  end
  else if (qsopt.flags and QSO_SAVEPATTERN)<>0 then
    pattern:=DBReadUnicode(0,qs_module,'pattern',nil)
  else
    pattern:=nil;

  CreateProtoList;
  if PrepareToFill then
  begin
    ColorReload(0,0);
    loadopt_wnd;
    CreateDialogW(hInstance,PWideChar(IDD_MAIN),0,@QSMainWndProc);
  end;
end;

end.

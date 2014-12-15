{structure editor}
unit sedit;

interface

uses windows;

function EditStructure(struct:pAnsiChar;parent:HWND=0):pAnsiChar;

implementation

uses io,messages, commctrl, common, wrapper, strans, memini
  {$IFDEF Miranda}, m_api, mirutils{$ENDIF};
{
  <STE_* set> <len> <data>
}
{$r structopts.res}
{$include i_struct_const.inc}

{$IFDEF Miranda}
const
  ACI_NEW    :PAnsiChar = 'ACI_New';
  ACI_UP     :PAnsiChar = 'ACI_Up';
  ACI_DOWN   :PAnsiChar = 'ACI_Down';
  ACI_DELETE :PAnsiChar = 'ACI_Delete';

const
  API_STRUCT_FILE:pAnsiChar = 'plugins\services.ini';
  namespace = 'Structure';
{$ENDIF}

type
  pint_ptr = ^int_ptr;
  TWPARAM = WPARAM;
  TLPARAM = LPARAM;

const
  col_alias=0;
  col_type =1;
  col_len  =2;
  col_flag =3;
  col_data =4;
var
  OldLVProc:pointer;
{$IFDEF Miranda}
  storage:pointer;
{$ENDIF}

function GetTypeIndex(etype:integer):integer;
var
  j:integer;
begin
  j:=0;
  while j<MaxStructTypes do
  begin
    if StructElems[j].typ=etype then break;
    inc(j);
  end;
  if j<MaxStructTypes then
    result:=j
  else
    result:=SST_UNKNOWN;
end;

procedure InsertString(wnd:HWND;num:dword;str:PAnsiChar);
var
  buf:array [0..127] of WideChar;
begin
  SendMessageW(wnd,CB_SETITEMDATA,
      SendMessageW(wnd,CB_ADDSTRING,0,
        lparam({$IFDEF Miranda}TranslateW{$ENDIF}(FastAnsiToWideBuf(str,buf)))),
      num);
end;

{$IFDEF Miranda}
procedure RegisterIcon(var sid:TSKINICONDESC;id,name:PAnsiChar;descr:PAnsiChar);
var
  buf:array [0..63] of WideChar;
begin
  sid.hDefaultIcon   :=LoadImageA(hInstance,id,IMAGE_ICON,16,16,0);
  sid.pszName        :=name;
  sid.szDescription.w:=FastAnsiToWideBuf(descr,buf);
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);
end;

procedure RegisterIcons;
var
  sid:TSKINICONDESC;
begin
  if CallService(MS_SKIN2_GETICON,0,LPARAM(ACI_NEW))<>0 then
    exit;

  FillChar(sid,SizeOf(TSKINICONDESC),0);
  sid.cbSize     :=SizeOf(TSKINICONDESC);
  sid.cx         :=16;
  sid.cy         :=16;
  sid.flags      :=SIDF_UNICODE;
  sid.szSection.w:='Actions';

  RegisterIcon(sid,'IDI_NEW'    ,ACI_NEW    ,'New');
  RegisterIcon(sid,'IDI_DELETE' ,ACI_DELETE ,'Delete');
  RegisterIcon(sid,'IDI_UP'     ,ACI_UP     ,'Up');
  RegisterIcon(sid,'IDI_DOWN'   ,ACI_DOWN   ,'Down');
end;
{$ENDIF}
procedure SetDataButtonIcons(Dialog:HWND);
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
  ti.uId     :=GetDlgItem(Dialog,IDC_DATA_NEW);
{$IFDEF Miranda}
  ti.lpszText:=TranslateW('New');
  SetButtonIcon(ti.uId,ACI_NEW);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
  ti.uId     :=GetDlgItem(Dialog,IDC_DATA_UP);
  ti.lpszText:=TranslateW('Up');
  SetButtonIcon(ti.uId,ACI_UP);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
  ti.uId     :=GetDlgItem(Dialog,IDC_DATA_DOWN);
  ti.lpszText:=TranslateW('Down');
  SetButtonIcon(ti.uId,ACI_DOWN);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
  ti.uId     :=GetDlgItem(Dialog,IDC_DATA_DELETE);
  ti.lpszText:=TranslateW('Delete');
  SetButtonIcon(ti.uId,ACI_DELETE);
{$ELSE}
  ti.lpszText:='New';
  SendMessageW(ti.uId, BM_SETIMAGE, IMAGE_ICON,
    LoadImage(hInstance,'IDI_NEW',IMAGE_ICON,16,16,0));
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
  ti.uId     :=GetDlgItem(Dialog,IDC_DATA_UP);
  ti.lpszText:='Up';
  SendMessageW(ti.uId, BM_SETIMAGE, IMAGE_ICON,
    LoadImage(hInstance,'IDI_UP',IMAGE_ICON,16,16,0));
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
  ti.uId     :=GetDlgItem(Dialog,IDC_DATA_DOWN);
  ti.lpszText:='Down';
  SendMessageW(ti.uId, BM_SETIMAGE, IMAGE_ICON,
    LoadImage(hInstance,'IDI_DOWN',IMAGE_ICON,16,16,0));
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
  ti.uId     :=GetDlgItem(Dialog,IDC_DATA_DELETE);
  ti.lpszText:='Delete';
  SendMessageW(ti.uId, BM_SETIMAGE, IMAGE_ICON,
    LoadImage(hInstance,'IDI_DELETE',IMAGE_ICON,16,16,0));
{$ENDIF}
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
end;

function NewLVProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
begin
  result:=0;
  case hMessage of
    WM_KEYDOWN: begin
      if (lParam and (1 shl 30))=0 then
      begin
        case wParam of
          VK_UP: begin
            if (GetKeyState(VK_CONTROL) and $8000)<>0 then
            begin
              SendMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_DATA_UP,0);
              exit;
            end;
          end;
          VK_DOWN: begin
            if (GetKeyState(VK_CONTROL) and $8000)<>0 then
            begin
              SendMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_DATA_DOWN,0);
              exit;
            end;
          end;
          VK_INSERT: begin
            SendMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_DATA_NEW,0);
            exit;
          end;
          VK_DELETE: begin
            SendMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_DATA_DELETE,0);
            exit;
          end;
        end;
      end;
    end;
  end;
  result:=CallWindowProc(OldLVProc,Dialog,hMessage,wParam,lParam);
end;

function MakeLVStructList(list:HWND):HWND;
var
  lv:LV_COLUMNW;
begin
  SendMessage(list,LVM_SETUNICODEFORMAT,1,0);
  SendMessage(list,LVM_SETEXTENDEDLISTVIEWSTYLE,
    LVS_EX_FULLROWSELECT or LVS_EX_GRIDLINES or LVS_EX_CHECKBOXES,
    LVS_EX_FULLROWSELECT or LVS_EX_GRIDLINES or LVS_EX_CHECKBOXES);

  zeromemory(@lv,sizeof(lv));
  lv.mask:=LVCF_TEXT or LVCF_WIDTH;
  lv.cx  :=22; lv.pszText:={$IFDEF Miranda}TranslateW{$ENDIF}('alias');
  SendMessageW(list,LVM_INSERTCOLUMNW,col_alias,lparam(@lv)); // alias
  lv.cx  :=62; lv.pszText:={$IFDEF Miranda}TranslateW{$ENDIF}('type');
  SendMessageW(list,LVM_INSERTCOLUMNW,col_type ,lparam(@lv)); // type
  lv.cx  :=32; lv.pszText:={$IFDEF Miranda}TranslateW{$ENDIF}('length');
  SendMessageW(list,LVM_INSERTCOLUMNW,col_len  ,lparam(@lv)); // length

  lv.cx  :=20; lv.pszText:={$IFDEF Miranda}TranslateW{$ENDIF}('');
  SendMessageW(list,LVM_INSERTCOLUMNW,col_flag ,lparam(@lv)); // flags

  lv.cx  :=72; lv.pszText:={$IFDEF Miranda}TranslateW{$ENDIF}('data');
  SendMessageW(list,LVM_INSERTCOLUMNW,col_data ,lparam(@lv)); // value

  SendMessageW(list,LVM_SETCOLUMNWIDTH,col_data,LVSCW_AUTOSIZE_USEHEADER);

  OldLVProc:=pointer(SetWindowLongPtrW(list,GWL_WNDPROC,long_ptr(@NewLVProc)));
  result:=list;
end;

procedure FillDataTypeList(wnd:HWND);
var
  i:integer;
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);

  for i:=0 to MaxStructTypes-1 do
    InsertString(wnd,StructElems[i].typ,StructElems[i].full);

  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

procedure FillAlignTypeList(wnd:HWND);
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);

  InsertString(wnd,0,'Native' );
  InsertString(wnd,1,'Packed' );
  InsertString(wnd,2,'2 bytes');
  InsertString(wnd,4,'4 bytes');
  InsertString(wnd,8,'8 bytes');

  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

//----- Data show -----

function InsertLVLine(list:HWND):integer;
var
  li:TLVITEMW;
begin
  li.mask    :=0;//LVIF_PARAM;
  li.iItem   :=SendMessage(list,LVM_GETNEXTITEM,-1,LVNI_FOCUSED)+1;
  li.iSubItem:=0;
  result:=SendMessageW(list,LVM_INSERTITEMW,0,lparam(@li));
end;

// fill table line by data from structure
procedure FillLVLine(list:HWND;item:integer;const element:tOneElement);
var
  tmp1:array [0..31] of WideChar;
  li:TLVITEMW;
  i,llen:integer;
  p,pc:pAnsiChar;
  pw:pWideChar;
begin
  if (element.flags and SF_RETURN)<>0 then
    ListView_SetCheckState(list,item,true);

  li.iItem:=item;
  li.mask:=LVIF_TEXT;

  // type
  p:=StructElems[GetTypeIndex(element.etype)].short;
  llen:=0;
  while p^<>#0 do
  begin
    tmp1[llen]:=WideChar(p^);
    inc(p);
    inc(llen);
  end;
  tmp1[llen]:=#0;
  li.iSubItem:=col_type;
  li.pszText :=@tmp1;
  SendMessageW(list,LVM_SETITEMW,0,lparam(@li));

  // flags
  llen:=0;

  if (element.flags and SF_SIZE)<>0 then
  begin
    tmp1[llen]:=char_size; inc(llen);
  end;

{$IFDEF Miranda}
  if (element.flags and SF_SCRIPT)<>0 then
  begin
    tmp1[llen]:=char_script; inc(llen);
  end;
  if (element.flags and SF_MMI)<>0 then
  begin
    tmp1[llen]:=char_mmi; inc(llen);
  end;
{$ENDIF}
  tmp1[llen]:=#0;
  li.iSubItem:=col_flag;
  li.pszText :=@tmp1;
  SendMessageW(list,LVM_SETITEMW,0,lparam(@li));

  // alias
  if element.alias[0]<>#0 then
  begin
    pc:=@element.alias;
    while pc^<>#0 do
    begin
      tmp1[llen]:=WideChar(pc^);
      inc(llen);
      inc(pc);
    end;
    tmp1[llen]:=#0;
    li.iSubItem:=col_alias;
    li.pszText :=@tmp1;
    SendMessageW(list,LVM_SETITEMW,0,lparam(@li));
  end;

  case element.etype of
    SST_LAST,SST_PARAM: begin
      llen:=0;
    end;

    SST_BYTE,SST_WORD,SST_DWORD,
    SST_QWORD,SST_NATIVE: begin
{$IFDEF Miranda}
      if (element.flags and SF_SCRIPT)<>0 then
      begin
        li.iSubItem:=col_data;
        UTF8ToWide(element.text,pw);
        llen:=StrLenW(pw);
        li.pszText :=pw;
        SendMessageW(list,LVM_SETITEMW,0,lparam(@li));
        mFreeMem(pw);
      end
      else
{$ENDIF}
      begin
        pc:=@element.svalue;
        llen:=0;
        while pc^<>#0 do
        begin
          tmp1[llen]:=WideChar(pc^);
          inc(llen);
          inc(pc);
        end;
        if llen>0 then //??
        begin
          tmp1[llen]:=#0;
          li.iSubItem:=col_data;
          li.pszText :=@tmp1;
          SendMessageW(list,LVM_SETITEMW,0,lparam(@li));
        end;
      end;
    end;

    SST_BARR,SST_WARR,SST_BPTR,SST_WPTR: begin
      // like for numbers, array length
      if element.len>0 then //??
      begin
        IntToStr(tmp1,element.len);
        li.iSubItem:=col_len;
        li.pszText :=@tmp1;
        SendMessageW(list,LVM_SETITEMW,0,lparam(@li));
      end;

      if element.text<>nil then
      begin
        li.iSubItem:=col_data;
        if (element.etype in [SST_WARR,SST_WPTR])
{$IFDEF Miranda}
           or ((element.flags and SF_SCRIPT)<>0)
{$ENDIF}
        then
        begin
          UTF8ToWide(element.text,pw);
        end
        else
        begin
          AnsiToWide(element.text,pw);
        end;
        li.pszText :=pw;
        llen:=StrLenW(pw);
        SendMessageW(list,LVM_SETITEMW,0,lparam(@li));
        mFreeMem(pw);
      end;
    end;
  end;

  i:=element.etype+(llen shl 16);
  LV_SetLParam(list,i,item);

  ListView_SetItemState(list,item,LVIS_FOCUSED or LVIS_SELECTED,
    LVIS_FOCUSED or LVIS_SELECTED);
end;

// Fill table by structure
procedure FillLVStruct(list:HWND;txt:PAnsiChar);
var
  p:pansiChar;
  element:tOneElement;
begin
  SendMessage(list,LVM_DELETEALLITEMS,0,0);
  if txt^ in sNum then
    txt:=StrScan(txt,char_separator)+1;
  while txt^<>#0 do
  begin
    p:=StrScan(txt,char_separator);
    GetOneElement(txt,element,false);
    FillLVLine(list,InsertLVLine(list),element);
    FreeElement(element);

    if p=nil then break;
    txt:=p+1;
  end;
  ListView_SetItemState(list,0,LVIS_FOCUSED or LVIS_SELECTED,
    LVIS_FOCUSED or LVIS_SELECTED);
end;

//----- Data save -----

function GetLVRow(var dst:pAnsiChar;list:HWND;item:integer):integer;
var
  li:TLVITEMW;
  buf:array [0..63] of WideChar;
  pc:pWideChar;
  pc1:pAnsiChar;
  len:integer;
  {$IFDEF Miranda}isScript:boolean;{$ENDIF}
begin
  li.iItem:=item;
  
  // result value check and element type
  li.mask      :=LVIF_PARAM or LVIF_STATE;
  li.iSubItem  :=0;
  li.stateMask :=LVIS_STATEIMAGEMASK;
  SendMessageW(list,LVM_GETITEMW,item,lparam(@li));
  result:=loword(li.lParam); // element type
  len   :=hiword(li.lParam); // text length

  if (li.state shr 12)>1 then // "return" value
  begin
    dst^:=char_return;
    inc(dst);
  end;

  li.mask      :=LVIF_TEXT;
  li.iSubItem  :=col_flag;
  li.cchTextMax:=32;
  li.pszText   :=@buf;
{$IFDEF Miranda}
  isScript:=false;
{$ENDIF}
  if SendMessage(list,LVM_GETITEMTEXTW,item,lparam(@li))>0 then
  begin
    if StrScanW(buf,char_size)<>nil then
    begin
      dst^:=char_size;
      inc(dst);
    end;
{$IFDEF Miranda}
    if StrScanW(buf,char_script)<>nil then
    begin
      dst^:=char_script;
      inc(dst);
      isScript:=true;
    end;

    if StrScanW(buf,char_mmi)<>nil then
    begin
      dst^:=char_mmi;
      inc(dst);
    end;
{$ENDIF}
  end;
{
  // type text (can skip and use type code)
  li.mask      :=LVIF_TEXT;
  li.cchTextMax:=HIGH(buf);
  li.pszText   :=@buf; 
  li.iSubItem  :=col_type;
  SendMessageW(list,LVM_GETITEMTEXTW,item,lparam(@li));
  dst:=StrEnd(FastWideToAnsiBuf(@buf,dst));
}
  dst:=StrCopyE(dst,StructElems[GetTypeIndex(result)].short);
  // alias
  li.mask      :=LVIF_TEXT;
  li.cchTextMax:=HIGH(buf);
  li.pszText   :=@buf; 

  li.iSubItem  :=col_alias;
  if SendMessageW(list,LVM_GETITEMTEXTW,item,lparam(@li))>0 then
  begin
    dst^:=' '; inc(dst);
    pc:=@buf;
    while pc^<>#0 do
    begin
      dst^:=AnsiChar(pc^); inc(dst); inc(pc);
    end;
  end;

  case result of
    SST_LAST,SST_PARAM: exit;

    SST_BYTE,SST_WORD,SST_DWORD,
    SST_QWORD,SST_NATIVE: begin
      li.iSubItem  :=col_data;
      li.cchTextMax:=32;
{$IFDEF Miranda}
      if isScript then
      begin
        mGetMem(pc,(len+1)*SizeOf(WideChar));
        li.pszText:=pc;
      end
      else
{$ENDIF}
      li.pszText   :=@buf;
      if SendMessage(list,LVM_GETITEMTEXTW,item,lparam(@li))>0 then
      begin
        dst^:=' '; inc(dst);
{$IFDEF Miranda}
        if isScript then
        begin
          WideToUTF8(pc,pc1);
          dst:=StrCopyE(dst,pc1);
          mFreeMem(pc1);
          mFreeMem(pc);
        end
        else
{$ENDIF}
        begin
          pc:=@buf;
          while pc^<>#0 do
          begin
            dst^:=AnsiChar(pc^); inc(dst); inc(pc);
          end;
//        StrCopyW(dst,buf);
        end;
      end;
    end;

    SST_BARR,SST_WARR,SST_BPTR,SST_WPTR: begin
      dst^:=' '; inc(dst);
      // length
      li.iSubItem  :=col_len;
      li.cchTextMax:=32;
      li.pszText   :=@buf;
      if SendMessage(list,LVM_GETITEMTEXTW,item,lparam(@li))>0 then
      begin
        pc:=@buf;
        while pc^<>#0 do
        begin
          dst^:=AnsiChar(pc^); inc(dst); inc(pc);
        end;
      end
      else
        dst:=StrEnd(IntToStr(dst,len));

      if len>0 then
      begin
//        dst:=StrEnd(dst);
        li.iSubItem  :=col_data;
        li.cchTextMax:=len+1;
        mGetMem(pc,(len+1)*SizeOf(WideChar));
        li.pszText   :=pc;
        SendMessage(list,LVM_GETITEMTEXTW,item,lparam(@li));
        if pc^<>#0 then
        begin
          dst^:=' '; inc(dst);
          if (result in [SST_WARR,SST_WPTR])
{$IFDEF Miranda}
            or isScript
{$ENDIF}
           then
            WideToUTF8(pc,pc1)
          else
            WideToAnsi(pc,pc1);

          dst:=StrCopyE(dst,pc1);
          mFreeMem(pc1);
        end;
        mFreeMem(pc);
      end;
    end;
  end;
//  dst:=StrEnd(dst);
end;

function SaveStructure(list:HWND;align:integer):pAnsiChar;
var
  p:PAnsiChar;
  i:integer;
begin
  mGetMem(p,32768);
  result:=p;
  FillChar(p^,32768,0);
  IntToStr(result,align);
  inc(result);
  result^:=char_separator;
  inc(result);

  for i:=0 to SendMessage(list,LVM_GETITEMCOUNT,0,0)-1 do
  begin
    GetLVRow(result,list,i);
    result^:=char_separator; inc(result);
  end;
  dec(result); result^:=#0;
  i:=(result+2-p);
  mGetMem(result,i);
  move(p^,result^,i);
  mFreeMem(p);
end;
{$IFDEF Miranda}
function StructEditDlgResizer(Dialog:HWND;lParam:LPARAM;urc:PUTILRESIZECONTROL):int; cdecl;
begin
  case urc^.wId of
    IDC_DATA_FULL:   result:=RD_ANCHORX_WIDTH or RD_ANCHORY_HEIGHT;
    IDC_DATA_TMPL:   result:=RD_ANCHORX_WIDTH or RD_ANCHORY_BOTTOM;
    IDC_DATA_EDIT:   result:=RD_ANCHORX_RIGHT or RD_ANCHORY_HEIGHT;
    IDC_DATA_EDTN:   result:=RD_ANCHORX_RIGHT or RD_ANCHORY_TOP;
    IDC_DATA_TYPE:   result:=RD_ANCHORX_RIGHT or RD_ANCHORY_TOP;
    IDC_DATA_LEN:    result:=RD_ANCHORX_RIGHT or RD_ANCHORY_TOP;
    IDC_DATA_SLEN:   result:=RD_ANCHORX_RIGHT or RD_ANCHORY_TOP;

    IDC_VAR_HELP:    result:=RD_ANCHORX_RIGHT or RD_ANCHORY_BOTTOM;
    IDC_DATA_VARS:   result:=RD_ANCHORX_RIGHT or RD_ANCHORY_BOTTOM;
    IDC_DATA_MMI:    result:=RD_ANCHORX_RIGHT or RD_ANCHORY_BOTTOM;
    IDC_DATA_SIZE:   result:=RD_ANCHORX_RIGHT or RD_ANCHORY_BOTTOM;

    IDC_DATA_NEW:    result:=RD_ANCHORX_RIGHT or RD_ANCHORY_TOP;
    IDC_DATA_UP:     result:=RD_ANCHORX_RIGHT or RD_ANCHORY_TOP;
    IDC_DATA_DOWN:   result:=RD_ANCHORX_RIGHT or RD_ANCHORY_TOP;
    IDC_DATA_DELETE: result:=RD_ANCHORX_RIGHT or RD_ANCHORY_TOP;
    IDC_DATA_INFO:   result:=RD_ANCHORX_RIGHT or RD_ANCHORY_BOTTOM;
    IDC_DATA_PASTE:  result:=RD_ANCHORX_RIGHT or RD_ANCHORY_BOTTOM;

    IDC_DATA_ALIGN : result:=RD_ANCHORX_RIGHT or RD_ANCHORY_TOP;
    IDC_DATA_SALGN : result:=RD_ANCHORX_RIGHT or RD_ANCHORY_TOP;
    IDC_DATA_SEP   : result:=RD_ANCHORX_RIGHT or RD_ANCHORY_TOP;

    IDC_DATA_CHANGE: result:=RD_ANCHORX_RIGHT or RD_ANCHORY_BOTTOM;
    IDOK:            result:=RD_ANCHORX_RIGHT or RD_ANCHORY_BOTTOM;
    IDCANCEL:        result:=RD_ANCHORX_RIGHT or RD_ANCHORY_BOTTOM;
  else
    result:=0;
  end;
end;
{$ENDIF}
procedure CheckReturns(wnd:HWND;item:integer);
var
  li:TLVITEMW;
  i:integer;
begin
  li.mask     :=LVIF_STATE;
  li.iSubItem :=0;
  li.stateMask:=LVIS_STATEIMAGEMASK;
  li.state    :=1 shl 12;
  for i:=0 to SendMessageW(wnd,LVM_GETITEMCOUNT,0,0)-1 do
  begin
    if i<>item then
    begin
      SendMessageW(wnd,LVM_SETITEMSTATE,i,lparam(@li));
{
      li.iItem:=i;
      SendMessageW(list,LVM_GETITEMSTATE,i,lparam(@li));
      if (li.state shr 12)>1 then
      begin
        li.state:=1 shl 12;
        SendMessageW(wnd,LVM_SETITEMSTATE,i,lparam(@li));
      end;
}
    end;
  end;
end;

// enable/disable navigation chain buttons
procedure CheckList(Dialog:HWND; num:integer=-1);
begin
  if num<0 then
    num:=SendDlgItemMessage(Dialog,IDC_DATA_FULL,LVM_GETNEXTITEM,WPARAM(-1),LVNI_FOCUSED);
  EnableWindow(GetDlgItem(Dialog,IDC_DATA_UP),num>0);
  EnableWindow(GetDlgItem(Dialog,IDC_DATA_DOWN),
      (num+1)<SendDlgItemMessage(Dialog,IDC_DATA_FULL,LVM_GETITEMCOUNT,0,0));
end;

procedure FillLVData(Dialog:HWND;list:HWND;item:integer);
var
  buf:array [0..15] of WideChar;
  dtype,i:integer;
  p:pWideChar;
  b,b1:boolean;
  idcshow,idchide:integer;
  li:TLVITEMW;
  {$IFDEF Miranda}vflag,{$ENDIF}mflag,
  len:integer;
  wnd:HWND;
begin
  len:=LV_GetLParam(list,item);
  dtype:=loword(len);
  len  :=hiword(len);
  idcshow:=IDC_DATA_EDIT;
  idchide:=IDC_DATA_EDTN;

  buf[0]:=#0;
  case dtype of
    SST_BYTE,SST_WORD,SST_DWORD,
    SST_QWORD,SST_NATIVE: begin
      idchide:=IDC_DATA_EDIT;
      idcshow:=IDC_DATA_EDTN;
      b :=true;
      b1:=false;
    end;

    SST_BARR,SST_WARR,SST_BPTR,SST_WPTR: begin
      b :=true;
      b1:=true;

      li.iSubItem  :=col_len;
      li.cchTextMax:=15;
      li.pszText   :=@buf;
      SendMessage(list,LVM_GETITEMTEXTW,item,lparam(@li));
    end;
  else
    b :=false;
    b1:=false;
  end;
  SetDlgItemTextW(Dialog,IDC_DATA_LEN,@buf);

  p:=@buf;
  li.cchTextMax:=15;
  li.pszText   :=@buf;
  if b then
  begin
{$IFDEF Miranda}
    li.iSubItem:=col_flag;
    vflag:=BST_UNCHECKED;
    i:=SW_HIDE;
    mflag:=BST_UNCHECKED;
    if SendMessage(list,LVM_GETITEMTEXTW,item,lparam(@li))>0 then
    begin
      if StrScanW(p,char_script)<>nil then
      begin
        b1:=true;
        vflag:=BST_CHECKED;
        i:=SW_SHOW;

        idchide:=IDC_DATA_EDTN;
        idcshow:=IDC_DATA_EDIT;
      end;

      if StrScanW(p,char_mmi)<>nil then
        mflag:=BST_CHECKED;
    end;
    ShowWindow(GetDlgItem(Dialog,IDC_VAR_HELP),i);
    CheckDlgButton(Dialog,IDC_DATA_VARS,vflag);
    CheckDlgButton(Dialog,IDC_DATA_MMI ,mflag);
{$ENDIF}
    if b1 then
    begin
      mGetMem(p,(len+1)*SizeOf(WideChar));
      li.cchTextMax:=len+1;
      li.pszText   :=p;
      i:=SW_HIDE;
    end
    else
    begin
      i:=SW_SHOW;

      if StrScanW(p,char_size)<>nil then
        mflag:=BST_CHECKED
      else
        mflag:=BST_UNCHECKED;
      CheckDlgButton(Dialog,IDC_DATA_SIZE,mflag);

      EnableWindow(GetDlgItem(Dialog,IDC_DATA_EDTN),mflag=BST_UNCHECKED);
    end;
    ShowWindow(GetDlgItem(Dialog,IDC_DATA_SIZE),i);

    li.iSubItem:=col_data;
    SendMessage(list,LVM_GETITEMTEXTW,item,lparam(@li));
  end;
  SetDlgItemTextW(Dialog,idchide,'');
  SetDlgItemTextW(Dialog,idcshow,p);

  if b1 then
    mFreeMem(p);

  wnd:=GetDlgItem(Dialog,IDC_DATA_TYPE);
  CB_SelectData(wnd,dtype);
  SendMessage(Dialog,WM_COMMAND,(CBN_SELENDOK shl 16)+IDC_DATA_TYPE,wnd);
end;

// Fill table row by data from edit fields
procedure FillLVRow(Dialog:HWND;list:HWND;item:integer);
var
  ltype,j,idc:integer;
  idx:integer;
  wnd:HWND;
  buf:array [0..63] of WideChar;
  tmp:pWideChar;
begin
  // type
  wnd:=GetDlgItem(Dialog,IDC_DATA_TYPE);
  ltype:=SendMessage(wnd,CB_GETITEMDATA,SendMessage(wnd,CB_GETCURSEL,0,0),0);
  j:=GetTypeIndex(ltype);

  LV_SetItemW(list,FastAnsiToWideBuf(StructElems[j].short,buf),item,col_type);

  // flags
  idx:=0;

  if IsDlgButtonChecked(Dialog,IDC_DATA_SIZE)<>BST_UNCHECKED then
  begin
    buf[idx]:=char_size; inc(idx);
  end;
{$IFDEF Miranda}
  if IsDlgButtonChecked(Dialog,IDC_DATA_VARS)<>BST_UNCHECKED then
  begin
    buf[idx]:=char_script; inc(idx);
  end;

  if IsDlgButtonChecked(Dialog,IDC_DATA_MMI)<>BST_UNCHECKED then
  begin
    buf[idx]:=char_mmi; inc(idx);
  end;
{$ENDIF}
  buf[idx]:=#0;
  LV_SetItemW(list,@buf,item,col_flag);
  
  // values
  tmp:=nil;
  case ltype of
    SST_LAST,SST_PARAM: begin
      j:=0;
    end;

    SST_BYTE,SST_WORD,SST_DWORD,
    SST_QWORD,SST_NATIVE: begin
{$IFDEF Miranda}
      if IsDlgButtonChecked(Dialog,IDC_DATA_VARS)<>BST_UNCHECKED then
        idc:=IDC_DATA_EDIT
      else
{$ENDIF}
        idc:=IDC_DATA_EDTN;

      tmp:=GetDlgText(Dialog,idc);
      j:=StrLenW(tmp);
      LV_SetItemW(list,tmp,item,col_data);
    end;

    SST_BARR,SST_WARR,SST_BPTR,SST_WPTR: begin

      SendDlgItemMessageW(Dialog,IDC_DATA_LEN,WM_GETTEXT,15,lparam(@buf));
      LV_SetItemW(list,buf,item,col_len);

      tmp:=GetDlgText(Dialog,IDC_DATA_EDIT);
      j:=StrLenW(tmp);
      LV_SetItemW(list,tmp,item,col_data);
    end;
  end;
  ltype:=ltype or (j shl 16);
  mFreeMem(tmp);
  LV_SetLParam(list,ltype,item);
end;

{$IFDEF Miranda}
procedure FillTemplates(wnd:HWND;lstorage:pointer);
var
  p,pp:pAnsiChar;
  i:integer;
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);

  p:=GetSectionList(lstorage,namespace);
  pp:=p;
  i:=0;
  while p^<>#0 do
  begin
    CB_AddStrData(wnd,p,int_ptr(SearchSection(lstorage,p,namespace)), i);

    while p^<>#0 do inc(p); inc(p);
    inc(i);
  end;
  FreeSectionList(pp);
  if i>0 then
    SendMessage(wnd,CB_SETCURSEL,0,0);
end;
{$ENDIF}

procedure ReadableForm(wnd:HWND; struct:pAnsiChar);
var
  p,pc,buf:pAnsiChar;
  element:tOneElement;
begin
  GetMem(buf,StrLen(struct)*2);
  pc:=buf;

  struct:=StrScan(struct,char_separator)+1;
  while struct^<>#0 do
  begin
    p:=StrScan(struct,char_separator);
    GetOneElement(struct,element,false);

    pc:=StrCopyE(pc,element.talias);
    if element.etype in [SST_BARR,SST_WARR] then
    begin
      pc^:=' '; inc(pc);
      pc^:='['; inc(pc);
      pc:=StrEnd(IntToStr(pc,element.len));
      pc^:=']'; inc(pc);
    end;
    pc^:=' '; inc(pc);
    pc:=StrCopyE(pc,element.alias);
{
//    if (element.typ IN [SST_BYTE,SST_WORD,SST_DWORD, SST_QWORD, SST_NATIVE]) then
    pc^:=' '; inc(pc);
    pc^:='='; inc(pc);
    pc^:=' '; inc(pc);
    pc:=StrCopyE(pc,element.sValue);
}
    pc^:=#13; inc(pc);
    pc^:=#10; inc(pc);

    FreeElement(element);

    if p=nil then break;
    struct:=p+1;
  end;
  pc^:=#0;

  SendMessageA(wnd,WM_SETTEXT,0,LPARAM(buf));
  FreeMem(buf);
end;

function StructHelp(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  tmp:pWideChar;
  pc:pAnsiChar;
begin
  result:=0;
  case hMessage of
    WM_CLOSE: begin
      DestroyWindow(Dialog); //??
    end;

    WM_INITDIALOG: begin
{$IFDEF Miranda}
      TranslateDialogDefault(Dialog);
{$ENDIF}
      result:=1;

      if lParam<>0 then
      begin
        SetDlgItemTextA(Dialog,IDC_HLP_NAME,GetSectionName(pointer(lParam)));

        SetDlgItemTextA(Dialog,IDC_HLP_PLUGIN,GetParamSectionStr(pointer(lParam),'plugin'));

        FastAnsiToWide(GetParamSectionStr(pointer(lParam),'descr','Undefined'),tmp);
        SetDlgItemTextW(Dialog,IDC_HLP_DESCR,{$IFDEF Miranda}TranslateW{$ENDIF}(tmp));
        mFreeMem(tmp);

        pc:=GetParamSectionStr(pointer(lParam),'full',nil);
        if pc=nil then
          pc:=GetParamSectionStr(pointer(lParam),'short',nil);
        if pc<>nil then
          ReadableForm(GetDlgItem(Dialog,IDC_HLP_STRUCT),pc);
      end;
    end;

    WM_COMMAND: begin
      if (wParam shr 16)=BN_CLICKED then
      begin
        case loword(wParam) of
          IDOK,IDCANCEL: begin
            DestroyWindow(Dialog);
          end;
        end;
      end;
    end;
  else
//!!    result:=DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;

function StructEdit(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  wnd:HWND;
  i:integer;
  li:TLVITEMW;
  b,b1,b2:boolean;
  idchide,idcshow,csize:integer;
{$IFDEF Miranda}
  pc:pAnsiChar;
  urd:TUTILRESIZEDIALOG;
{$ELSE}
  rc,rc1:TRECT;
{$ENDIF}
begin
  result:=0;
  case hMessage of

    WM_DESTROY: begin
{$IFDEF Miranda}
      CloseStorage(storage);
{$ENDIF}
    end;

    WM_INITDIALOG: begin
      result:=1;
{$IFDEF Miranda}
      TranslateDialogDefault(Dialog);
      RegisterIcons;
      storage:=OpenStorage(API_STRUCT_FILE);
      if storage<>nil then
        FillTemplates(GetDlgItem(Dialog,IDC_DATA_TMPL),storage);
      if isVarsInstalled then
        SendDlgItemMessage(Dialog,IDC_VAR_HELP,BM_SETIMAGE,IMAGE_ICON,
          CallService(MS_VARS_GETSKINITEM,0,VSI_HELPICON));
{$ENDIF}
      wnd:=GetDlgItem(Dialog,IDC_DATA_FULL);
      MakeLVStructList(wnd);
      SetDataButtonIcons(Dialog);
      FillDataTypeList (GetDlgItem(Dialog,IDC_DATA_TYPE));
      FillAlignTypeList(GetDlgItem(Dialog,IDC_DATA_ALIGN));
      if lParam<>0 then
      begin
        FillLVStruct(wnd,pAnsiChar(lParam)) // fill lv with current structure
      end
      else
        SendMessage(Dialog,WM_COMMAND,(CBN_SELCHANGE shl 16)+IDC_DATA_TYPE,
            GetDlgItem(Dialog,IDC_DATA_TYPE));
      CheckList(Dialog,-1);
      SendMessage(Dialog,WM_COMMAND,(CBN_SELENDOK shl 16)+IDC_DATA_TYPE,
                  GetDlgItem(Dialog,IDC_DATA_TYPE));
    end;

    WM_GETMINMAXINFO: begin
      with PMINMAXINFO(lParam)^ do
      begin
        ptMinTrackSize.x:=500;
        ptMinTrackSize.y:=300;
      end;
    end;

    WM_SIZE: begin
{$IFDEF Miranda}
      FillChar(urd,SizeOf(TUTILRESIZEDIALOG),0);
      urd.cbSize    :=SizeOf(urd);
      urd.hwndDlg   :=Dialog;
      urd.hInstance :=hInstance;
      urd.lpTemplate:='IDD_STRUCTURE';//MAKEINTRESOURCEA(IDD_STRUCTURE);
      urd.lParam    :=0;
      urd.pfnResizer:=@StructEditDlgResizer;
      CallService(MS_UTILS_RESIZEDIALOG,0,TLPARAM(@urd));
{$ELSE}
      GetWindowRect(Dialog,rc);

      wnd:=GetDlgItem(Dialog,IDC_DATA_EDIT);
      GetWindowRect(wnd,rc1);
      SetWindowPos(wnd,0,0,0,rc.right-rc1.left-8,rc1.bottom-rc1.top,
          SWP_NOMOVE or SWP_NOZORDER or SWP_SHOWWINDOW);
      
      wnd:=GetDlgItem(Dialog,IDC_DATA_FULL);
      GetWindowRect(wnd,rc1);
      SetWindowPos(wnd,0,0,0,rc1.right-rc1.left, rc.bottom-rc1.top-8,
          SWP_NOMOVE or SWP_NOZORDER or SWP_SHOWWINDOW);
{$ENDIF}
    end;

    WM_COMMAND: begin
      case wParam shr 16 of

        CBN_SELENDOK{CBN_SELCHANGE}:  begin
          case loword(wParam) of
{$IFDEF Miranda}
            IDC_DATA_TMPL: begin
            end;
{$ENDIF}
            IDC_DATA_TYPE: begin
              i:=CB_GetData(lParam);
              case i of
                SST_LAST,SST_PARAM: begin
                  b :=false;
                  b1:=false;
                end;

                SST_BYTE,SST_WORD,SST_DWORD,
                SST_QWORD,SST_NATIVE: begin
                  b :=true;
                  b1:=false;
                end;

                SST_BARR,SST_WARR,SST_BPTR,SST_WPTR: begin
                  b :=true;
                  b1:=true;
                end;
              else
                b :=false;
                b1:=false;
              end;
              EnableWindow(GetDlgItem(Dialog,IDC_DATA_EDIT),b);
              EnableWindow(GetDlgItem(Dialog,IDC_DATA_EDTN),b);
              EnableWindow(GetDlgItem(Dialog,IDC_DATA_LEN ),b1);

              csize:=SW_HIDE;
              if b then
              begin
                if not b1 then
//                if i IN [SST_BYTE,SST_WORD,SST_DWORD,SST_QWORD,SST_NATIVE] then
                begin
                  csize:=SW_SHOW;
                  b2:=IsDlgButtonChecked(Dialog,IDC_DATA_SIZE)=BST_UNCHECKED;
{$IFDEF Miranda}
                  if b2 and (IsDlgButtonChecked(Dialog,IDC_DATA_VARS)<>BST_UNCHECKED) then
                  begin
                    idchide:=IDC_DATA_EDTN;
                    idcshow:=IDC_DATA_EDIT;
                  end
                  else
{$ENDIF}
                  begin
                    idchide:=IDC_DATA_EDIT;
                    idcshow:=IDC_DATA_EDTN;
                  end;
                end
                else
                begin
                  idchide:=IDC_DATA_EDTN;
                  idcshow:=IDC_DATA_EDIT;
                  b2:=true;
                end;
                ShowWindow(GetDlgItem(Dialog,idcshow),SW_SHOW);
                ShowWindow(GetDlgItem(Dialog,idchide),SW_HIDE);

                EnableWindow(GetDlgItem(Dialog,IDC_DATA_EDTN),b2);
              end;
              ShowWindow(GetDlgItem(Dialog,IDC_DATA_SIZE),csize);

{$IFDEF Miranda}
              if i IN [SST_PARAM,SST_LAST] then
                ShowWindow(GetDlgItem(Dialog,IDC_DATA_VARS),SW_HIDE)
              else
                ShowWindow(GetDlgItem(Dialog,IDC_DATA_VARS),SW_SHOW);

              if i IN [SST_BPTR,SST_WPTR] then
                ShowWindow(GetDlgItem(Dialog,IDC_DATA_MMI),SW_SHOW)
              else
                ShowWindow(GetDlgItem(Dialog,IDC_DATA_MMI),SW_HIDE);
{$ENDIF}
            end;
          end;
        end;

        BN_CLICKED: begin
          case loword(wParam) of
{$IFDEF Miranda}
            IDC_DATA_INFO: begin
               CreateDialogParamW(hInstance,'IDD_STRUCTHELP',//MAKEINTRESOURCEW(IDD_HELP),
                  0{Dialog},@StructHelp,CB_GetData(GetDlgItem(Dialog,IDC_DATA_TMPL)));
            end;

            IDC_DATA_PASTE: begin
              wnd:=GetDlgItem(Dialog,IDC_DATA_TMPL);
              pc:=GetParamSectionStr(pointer(CB_GetData(wnd)),'full',nil);
              if pc=nil then
                pc:=GetParamSectionStr(pointer(CB_GetData(wnd)),'short',nil);
              if pc<>nil then
              begin
                FillLVStruct(GetDlgItem(Dialog,IDC_DATA_FULL),pc); // fill lv with current structure
              end;
            end;

            IDC_VAR_HELP: begin
              ShowVarHelp(Dialog,IDC_DATA_EDIT);
            end;
            IDC_DATA_VARS: begin
              if (not isVarsInstalled) or
                 (IsDlgbuttonChecked(Dialog,IDC_DATA_VARS)=BST_UNCHECKED) then
                idcshow:=SW_HIDE
              else
                idcshow:=SW_SHOW;
              ShowWindow(GetDlgItem(Dialog,IDC_VAR_HELP),idcshow);


              i:=CB_GetData(GetDlgItem(Dialog,IDC_DATA_TYPE));
              if i IN [SST_BYTE,SST_WORD,SST_DWORD,SST_QWORD,SST_NATIVE] then
              begin
                if IsDlgButtonChecked(Dialog,IDC_DATA_VARS)<>BST_UNCHECKED then
                begin
                  idchide:=IDC_DATA_EDTN;
                  idcshow:=IDC_DATA_EDIT;
                end
                else
                begin
                  idchide:=IDC_DATA_EDIT;
                  idcshow:=IDC_DATA_EDTN;
                end;
                ShowWindow(GetDlgItem(Dialog,idcshow),SW_SHOW);
                ShowWindow(GetDlgItem(Dialog,idchide),SW_HIDE);
              end;
            end;
{$ENDIF}
            IDC_DATA_SIZE: begin
              EnableWindow(GetDlgItem(Dialog,IDC_DATA_EDTN),
                  IsDlgButtonChecked(Dialog,IDC_DATA_SIZE)=BST_UNCHECKED);
            end;

            IDC_DATA_NEW: begin
              wnd:=GetDlgItem(Dialog,IDC_DATA_FULL);
              i:=InsertLVLine(wnd);
              FillLVRow(Dialog,wnd,i);
              EnableWindow(GetDlgItem(Dialog,IDC_DATA_DELETE),true);
//              CheckList(Dialog,i);
//              if SendMessage(wnd,LVM_GETITEMCOUNT,0,0)=1 then
              begin
                li.mask     :=LVIF_STATE;
                li.iItem    :=i;
                li.iSubItem :=0;
                li.StateMask:=LVIS_FOCUSED+LVIS_SELECTED;
                li.State    :=LVIS_FOCUSED+LVIS_SELECTED;
                SendMessageW(wnd,LVM_SETITEMW,0,TLPARAM(@li));
              end;
              CheckList(Dialog);
            end;

            IDC_DATA_DELETE: begin
              wnd:=GetDlgItem(Dialog,IDC_DATA_FULL);
              i:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED); //??
              if i<>-1 then
              begin
                SendMessage(wnd,LVM_DELETEITEM,i,0);
                CheckList(Dialog,-1);
              end;

//            SendMessageW(Dialog,LVM_DELETEITEM,ListView_GetNextItem(Dialog,-1,LVNI_FOCUSED),0);
//select next and set field (auto?)
{
    i:=SendMessage(wnd,LVM_GETITEMCOUNT,0,0);
    if i>0 then
    begin
      if next=i then
        dec(next);
      ListView_SetItemState(wnd,next,LVIS_FOCUSED or LVIS_SELECTED,
        LVIS_FOCUSED or LVIS_SELECTED);
}
            end;

            IDC_DATA_UP: begin
              wnd:=GetDlgItem(Dialog,IDC_DATA_FULL);
              li.iItem:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
//              if li.iItem>0 then
                LV_MoveItem(wnd,-1,li.iItem);
                CheckList(Dialog);
            end;

            IDC_DATA_DOWN: begin
              wnd:=GetDlgItem(Dialog,IDC_DATA_FULL);
              li.iItem:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
//              if li.iItem<(SendMessage(wnd,LVM_GETITEMCOUNT,0,0)-1) then
                LV_MoveItem(wnd,1,li.iItem);
                CheckList(Dialog);
            end;

            IDOK: begin // save result
              EndDialog(Dialog,int_ptr(
                  SaveStructure(GetDlgItem(Dialog,IDC_DATA_FULL),
                    CB_GetData(GetDlgItem(Dialog,IDC_DATA_ALIGN))
                  )));
            end;

            IDCANCEL: begin // clear result / restore old value
              EndDialog(Dialog,0);
            end;

            IDC_DATA_CHANGE: begin
              wnd:=GetDlgItem(Dialog,IDC_DATA_FULL);
              if SendMessage(wnd,LVM_GETITEMCOUNT,0,0)=0 then
              begin
                PostMessage(Dialog,hMessage,IDC_DATA_NEW,lParam);
                exit;
              end;
              i:=SendMessage(wnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED); //??
              if i<>-1 then
                FillLVRow(Dialog,wnd,i);
            end;

          end;
        end;
      end;
    end;

    WM_NOTIFY: begin
      if integer(PNMHdr(lParam)^.code)=PSN_APPLY then
      begin
      end
      else if wParam=IDC_DATA_FULL then
      begin
        case integer(PNMHdr(lParam)^.code) of
          LVN_ITEMCHANGED: begin
            i:=(PNMLISTVIEW(lParam)^.uOldState and LVNI_FOCUSED)-
               (PNMLISTVIEW(lParam)^.uNewState and LVNI_FOCUSED);
            if i>0 then // old focus - do nothing
            else if i<0 then // new focus - fill fields
            begin
              //save
              FillLVData(Dialog,PNMHdr(lParam)^.hwndFrom,PNMLISTVIEW(lParam)^.iItem);
              CheckList(Dialog,PNMLISTVIEW(lParam)^.iItem);
            end
            else
            begin
              if (PNMLISTVIEW(lParam)^.uOldState or PNMLISTVIEW(lParam)^.uNewState)=$3000 then
              begin
                if PNMLISTVIEW(lParam)^.uOldState=$1000 then // check
                  CheckReturns(GetDlgItem(Dialog,IDC_DATA_FULL),PNMLISTVIEW(lParam)^.iItem);
              end;
            end;
          end;

          LVN_ENDLABELEDITW: begin
            with PLVDISPINFO(lParam)^ do
            begin
              if item.pszText<>nil then
              begin
                item.mask:=LVIF_TEXT;
                SendMessageW(hdr.hWndFrom,LVM_SETITEMW,0,TLPARAM(@item));
                result:=1;
              end;
            end;
          end;

          NM_DBLCLK: begin
            if PNMListView(lParam)^.iItem>=0 then
            begin
              SendMessage(PNMHdr(lParam)^.hWndFrom,LVM_EDITLABEL,
                          PNMListView(lParam)^.iItem,0);
            end;
          end;

        end;
      end;
    end;
  else
//!!    result:=DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;

function EditStructure(struct:pAnsiChar;parent:HWND=0):pAnsiChar;
begin
  InitCommonControls;

  result:=pAnsiChar(uint_ptr(DialogBoxParamW(hInstance,'IDD_STRUCTURE',
                 parent,@StructEdit,LPARAM(struct))));

  if uint_ptr(result)=uint_ptr(-1) then
    result:=nil;
end;

end.

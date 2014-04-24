{$include compilers.inc}
unit wrapper;

interface

uses windows;

function CreateHiddenWindow(proc:pointer=nil):HWND;

function DoInitCommonControls(dwICC:dword):boolean;

function GetScreenRect:TRect;
procedure SnapToScreen(var rc:TRect;dx:integer=0;dy:integer=0{;
          minw:integer=240;minh:integer=100});

function GetDlgText(Dialog:HWND;idc:integer;getAnsi:boolean=false):pointer; overload;
function GetDlgText(wnd:HWND;getAnsi:boolean=false):pointer; overload;

function StringToGUID(const astr:PAnsiChar):TGUID; overload;
function StringToGUID(const astr:PWideChar):TGUID; overload;

// Comboboxes
function CB_SelectData(cb:HWND;data:lparam):LRESULT; overload;
function CB_SelectData(Dialog:HWND;id:cardinal;data:lparam):LRESULT; overload;
function CB_GetData   (cb:HWND;idx:integer=-1):LRESULT; overload;
function CB_AddStrData (cb:HWND;astr:pAnsiChar;data:lparam=0;idx:integer=-1):HWND; overload;
function CB_AddStrData (Dialog:HWND;id:cardinal;astr:pAnsiChar;data:lparam=0;idx:integer=-1):HWND; overload;
function CB_AddStrDataW(cb:HWND;astr:pWideChar;data:lparam=0;idx:integer=-1):HWND; overload;
function CB_AddStrDataW(Dialog:HWND;id:cardinal;astr:pWideChar;data:lparam=0;idx:integer=-1):HWND; overload;

// CommCtrl - ListView
Procedure ListView_GetItemTextA(list:HWND;i:WPARAM;iSubItem:integer;pszText:pointer;cchTextMax:integer);
Procedure ListView_GetItemTextW(list:HWND;i:WPARAM;iSubItem:integer;pszText:pointer;cchTextMax:integer);
function  LV_GetLParam  (list:HWND;item:integer=-1):LRESULT;
function  LV_SetLParam  (list:HWND;lParam:LPARAM;item:integer=-1):LRESULT;
function  LV_ItemAtPos(list:HWND;pt:TPOINT;var SubItem:dword):integer; overload;
function  LV_ItemAtPos(list:HWND;x,y:integer;var SubItem:dword):integer; overload;
procedure LV_SetItem (list:HWND;str:PAnsiChar;item:integer;subitem:integer=0);
procedure LV_SetItemW(list:HWND;str:PWideChar;item:integer;subitem:integer=0);
function  LV_MoveItem(list:HWND;direction:integer;item:integer=-1):integer;
function  LV_GetColumnCount(list:HWND):LRESULT;
function  LV_CheckDirection(list:HWND):integer; // bit 0 - can move up, bit 1 - down

// CommDLG - Dialogs
function ShowDlg (dst:PAnsiChar;fname:PAnsiChar=nil;Filter:PAnsiChar=nil;open:boolean=true):boolean;
function ShowDlgW(dst:PWideChar;fname:PWideChar=nil;Filter:PWideChar=nil;open:boolean=true):boolean;

procedure GetUnitSize(wnd:HWND; var baseUnitX, baseUnitY: integer);

implementation

uses messages,common,commctrl,commdlg;

const
  EmptyGUID:TGUID = '{00000000-0000-0000-0000-000000000000}';

{$IFNDEF FPC}
const
  LVM_SORTITEMSEX = LVM_FIRST + 81;
{$ENDIF}

{$IFNDEF DELPHI_7_UP}
const
  SM_XVIRTUALSCREEN  = 76;
  SM_YVIRTUALSCREEN  = 77;
  SM_CXVIRTUALSCREEN = 78;
  SM_CYVIRTUALSCREEN = 79;
{$ENDIF}

//----- Hidden Window functions -----
const
  HWND_MESSAGE = HWND(-3);
const
  hiddenwindow:HWND = 0;
  hwndcount:integer=0;

function HiddenWindProc(wnd:HWND; msg:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
begin
  if msg=WM_CLOSE then
  begin
    dec(hwndcount);
    if hwndcount>0 then // not all references gone
    begin
      result:=0;
      exit
    end
    else
      hiddenwindow:=0
  end;

  result:=DefWindowProcW(wnd,msg,wParam,lParam);
end;

function CreateHiddenWindow(proc:pointer=nil):HWND;
begin
  if proc=nil then
  begin
    if hiddenwindow<>0 then
    begin
      result:=hiddenwindow;
      inc(hwndcount); // one reference more
    end
    else
    begin
      result:=CreateWindowExW(0,'STATIC',nil,0,
         1,1,1,1,HWND_MESSAGE,0,hInstance,nil);
      if result<>0 then
        SetWindowLongPtrW(result,GWL_WNDPROC,LONG_PTR(@HiddenWindProc));

      hiddenwindow:=result;
    end
  end
  else
  begin
    result:=CreateWindowExW(0,'STATIC',nil,0,
       1,1,1,1,HWND_MESSAGE,0,hInstance,nil);
    if result<>0 then
      SetWindowLongPtrW(result,GWL_WNDPROC,LONG_PTR(proc));
  end;
end;
//----- End of hidden window functions -----

function DoInitCommonControls(dwICC:dword):boolean;
var
  ICC: TInitCommonControlsEx;
begin
  if dwICC=0 then
    dwICC:=ICC_STANDARD_CLASSES or ICC_WIN95_CLASSES;
  ICC.dwSize:= Sizeof(ICC);
  ICC.dwICC := dwICC;
  result:=InitCommonControlsEx(ICC);
end;

function GetScreenRect:TRect;
begin
  result.left  := GetSystemMetrics( SM_XVIRTUALSCREEN  );
  result.top   := GetSystemMetrics( SM_YVIRTUALSCREEN  );
  result.right := GetSystemMetrics( SM_CXVIRTUALSCREEN ) + result.left;
  result.bottom:= GetSystemMetrics( SM_CYVIRTUALSCREEN ) + result.top;
end;

procedure SnapToScreen(var rc:TRect;dx:integer=0;dy:integer=0{;
          minw:integer=240;minh:integer=100});
var
  rect:TRect;
begin
  rect:=GetScreenRect;
  if rc.right >rect.right  then rc.right :=rect.right -dx;
  if rc.bottom>rect.bottom then rc.bottom:=rect.bottom-dy;
  if rc.left  <rect.left   then rc.left  :=rect.left;
  if rc.top   <rect.top    then rc.top   :=rect.top;
end;

function GetDlgText(wnd:HWND;getAnsi:boolean=false):pointer;
var
  a:cardinal;
begin
  result:=nil;
  if getAnsi then
  begin
    a:=SendMessageA(wnd,WM_GETTEXTLENGTH,0,0)+1;
    if a>1 then
    begin
      mGetMem(PAnsiChar(result),a);
      SendMessageA(wnd,WM_GETTEXT,a,lparam(result));
    end;
  end
  else
  begin
    a:=SendMessageW(wnd,WM_GETTEXTLENGTH,0,0)+1;
    if a>1 then
    begin
      mGetMem(pWideChar(result),a*SizeOf(WideChar));
      SendMessageW(wnd,WM_GETTEXT,a,lparam(result));
    end;
  end;
end;

function GetDlgText(Dialog:HWND;idc:integer;getAnsi:boolean=false):pointer;
begin
  result:=GetDlgText(GetDlgItem(Dialog,idc),getAnsi);
end;

//----- Combobox functions -----

function CB_SelectData(cb:HWND;data:lparam):LRESULT; overload;
var
  i:integer;
begin
  result:=CB_ERR;
  for i:=0 to SendMessage(cb,CB_GETCOUNT,0,0)-1 do
  begin
    if data=lparam(SendMessage(cb,CB_GETITEMDATA,i,0)) then
    begin
      result:=i;
      break;
    end;
  end;
  result:=SendMessage(cb,CB_SETCURSEL,result,0);
end;

function CB_SelectData(Dialog:HWND;id:cardinal;data:lparam):LRESULT; overload;
begin
  result:=CB_SelectData(GetDlgItem(Dialog,id),data);
end;

function CB_GetData(cb:HWND;idx:integer=-1):LRESULT;
begin
  if idx<0 then
    idx:=SendMessage(cb,CB_GETCURSEL,0,0);
  if idx<0 then
    result:=0
  else
    result:=SendMessage(cb,CB_GETITEMDATA,idx,0);
end;

function CB_AddStrData(cb:HWND;astr:pAnsiChar;data:lparam=0;idx:integer=-1):HWND;
begin
  result:=cb;
  if idx<0 then
    idx:=SendMessageA(cb,CB_ADDSTRING,0,lparam(astr))
  else
    idx:=SendMessageA(cb,CB_INSERTSTRING,idx,lparam(astr));
  SendMessageA(cb,CB_SETITEMDATA,idx,data);
end;

function CB_AddStrData(Dialog:HWND;id:cardinal;astr:pAnsiChar;data:lparam=0;idx:integer=-1):HWND;
begin
  result:=CB_AddStrData(GetDlgItem(Dialog,id),astr,data,idx);
end;

function CB_AddStrDataW(cb:HWND;astr:pWideChar;data:lparam=0;idx:integer=-1):HWND;
begin
  result:=cb;
  if idx<0 then
    idx:=SendMessageW(cb,CB_ADDSTRING,0,lparam(astr))
  else
    idx:=SendMessageW(cb,CB_INSERTSTRING,idx,lparam(astr));
  SendMessage(cb,CB_SETITEMDATA,idx,data);
end;

function CB_AddStrDataW(Dialog:HWND;id:cardinal;astr:pWideChar;data:lparam=0;idx:integer=-1):HWND;
begin
  result:=CB_AddStrDataW(GetDlgItem(Dialog,id),astr,data,idx);
end;

function StringToGUID(const astr:PAnsiChar):TGUID;
var
  i:integer;
begin
  result:=EmptyGUID;
  if StrLen(astr)<>38 then exit;
  result.D1:=HexToInt(PAnsiChar(@astr[01]),8);
  result.D2:=HexToInt(PAnsiChar(@astr[10]),4);
  result.D3:=HexToInt(PAnsiChar(@astr[15]),4);

  result.D4[0]:=HexToInt(PAnsiChar(@astr[20]),2);
  result.D4[1]:=HexToInt(PAnsiChar(@astr[22]),2);
  for i:=2 to 7 do
  begin
    result.D4[i]:=HexToInt(PAnsiChar(@astr[21+i*2]),2);
  end;
end;

function StringToGUID(const astr:PWideChar):TGUID;
var
  i:integer;
begin
  result:=EmptyGUID;
  if StrLenW(astr)<>38 then exit;
  result.D1:=HexToInt(pWideChar(@astr[01]),8);
  result.D2:=HexToInt(pWideChar(@astr[10]),4);
  result.D3:=HexToInt(pWideChar(@astr[15]),4);

  result.D4[0]:=HexToInt(pWideChar(@astr[20]),2);
  result.D4[1]:=HexToInt(pWideChar(@astr[22]),2);
  for i:=2 to 7 do
  begin
    result.D4[i]:=HexToInt(pWideChar(@astr[21+i*2]),2);
  end;
end;

//----- ListView functions -----

Procedure ListView_GetItemTextA(list:HWND;i:WPARAM;iSubItem:integer;pszText:pointer;cchTextMax:integer);
Var
  lvi:LV_ITEMA;
Begin
  lvi.iSubItem  :=iSubItem;
  lvi.cchTextMax:=cchTextMax;
  lvi.pszText   :=pszText;
  SendMessageA(list,LVM_GETITEMTEXT,i,LPARAM(@lvi));
end;

Procedure ListView_GetItemTextW(list:HWND;i:WPARAM;iSubItem:integer;pszText:pointer;cchTextMax:integer);
Var
  lvi:LV_ITEMW;
Begin
  lvi.iSubItem  :=iSubItem;
  lvi.cchTextMax:=cchTextMax;
  lvi.pszText   :=pszText;
  SendMessageW(list,LVM_GETITEMTEXT,i,LPARAM(@lvi));
end;

procedure LV_SetItem(list:HWND;str:PAnsiChar;item:integer;subitem:integer=0);
var
  li:LV_ITEMA;
begin
//  zeromemory(@li,sizeof(li));
  li.mask    :=LVIF_TEXT;
  li.pszText :=str;
  li.iItem   :=item;
  li.iSubItem:=subitem;
  SendMessageA(list,LVM_SETITEMA,0,lparam(@li));
end;

procedure LV_SetItemW(list:HWND;str:PWideChar;item:integer;subitem:integer=0);
var
  li:LV_ITEMW;
begin
//  zeromemory(@li,sizeof(li));
  li.mask    :=LVIF_TEXT;
  li.pszText :=str;
  li.iItem   :=item;
  li.iSubItem:=subitem;
  SendMessageW(list,LVM_SETITEMW,0,lparam(@li));
end;

function LV_GetLParam(list:HWND;item:integer=-1):LRESULT;
var
  li:LV_ITEMW;
begin
  if item<0 then
  begin
    item:=SendMessage(list,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
    if item<0 then
    begin
      result:=-1;
      exit;
    end;
  end;
  li.iItem   :=item;
  li.mask    :=LVIF_PARAM;
  li.iSubItem:=0;
  SendMessageW(list,LVM_GETITEMW,0,lparam(@li));
  result:=li.lParam;
end;

function LV_SetLParam(list:HWND;lParam:LPARAM;item:integer=-1):LRESULT;
var
  li:LV_ITEMW;
begin
  if item<0 then
  begin
    item:=SendMessage(list,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
    if item<0 then
    begin
      result:=-1;
      exit;
    end;
  end;
  li.iItem   :=item;
  li.mask    :=LVIF_PARAM;
  li.lParam  :=lParam;
  li.iSubItem:=0;
  SendMessageW(list,LVM_SETITEMW,0,windows.lparam(@li));
  result:=lParam;
end;

function LV_ItemAtPos(list:HWND;Pt:TPOINT;var SubItem:dword):integer;
var
  HTI:LV_HITTESTINFO;
begin
  HTI.pt.x := pt.X;
  HTI.pt.y := pt.Y;
  SendMessage(list,LVM_SUBITEMHITTEST,0,lparam(@HTI));
  Result :=HTI.iItem;
  if @SubItem<>nil then
    SubItem:=HTI.iSubItem;
end;

function LV_ItemAtPos(list:HWND;x,y:integer;var SubItem:dword):integer; overload;
var
  HTI:LV_HITTESTINFO;
begin
  HTI.pt.x := x;
  HTI.pt.y := y;
  SendMessage(list,LVM_SUBITEMHITTEST,0,lparam(@HTI));
  Result :=HTI.iItem;
  if @SubItem<>nil then
    SubItem:=HTI.iSubItem;
end;

function LV_Compare(lParam1,lParam2,param:LPARAM):integer; stdcall;
var
  olditem,neibor:integer;
begin
  result:=lParam1-lParam2;
  neibor :=hiword(param);
  olditem:=loword(param);
  if neibor>olditem then
  begin
    if (lParam1=olditem) and (lParam2<=neibor) then
      result:=1;
  end
  else
  begin
    if (lParam2=olditem) and (lParam1>=neibor) then
      result:=1;
  end;
end;

function LV_MoveItem(list:HWND;direction:integer;item:integer=-1):integer;
begin
  if ((direction>0) and (item=(SendMessage(list,LVM_GETITEMCOUNT,0,0)-1))) or
     ((direction<0) and (item=0)) then
  begin
    result:=item;
    exit;
  end;

  if item<0 then
    item:=SendMessage(list,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
  SendMessageW(list,LVM_SORTITEMSEX,wparam(item)+(wparam(item+direction) shl 16),lparam(@LV_Compare));
  result:=item+direction;
end;

function LV_GetColumnCount(list:HWND):LRESULT;
begin
  result:=SendMessage(SendMessage(list,LVM_GETHEADER,0,0),HDM_GETITEMCOUNT,0,0);
end;

function LV_CheckDirection(list:HWND):integer;
var
  i,cnt{,selcnt}:integer;
  stat,first,last,focus: integer;
begin
  first :=-1;
  last  :=-1;
  focus :=-1;
  cnt   :=SendMessage(list,LVM_GETITEMCOUNT,0,0)-1;
//  selcnt:=SendMessage(list,LVM_GETSELECTEDCOUNT,0,0);
  for i:=0 to cnt do
  begin
    stat:=SendMessage(list,LVM_GETITEMSTATE,i,LVIS_SELECTED or LVIS_FOCUSED);
    if (stat and LVIS_SELECTED)<>0 then
    begin
      if (stat and LVIS_FOCUSED)<>0 then
        focus:=i;
      if first<0 then first:=i;
      last:=i;
    end;
  end;
  result:=0;
  if focus<0 then
    focus:=first;
  if focus>=0 then
    result:=result or ((focus+1) shl 16);
  if first>0 then // at least one selected and not first
  begin
    result:=(result or 1){ or (first+1) shl 16};
  end;
  if (last>=0) and (last<cnt) then
    result:=result or 2;
end;

//----- CommDlg procedures -----

function ShowDlg(dst:PAnsiChar;fname:PAnsiChar=nil;Filter:PAnsiChar=nil;open:boolean=true):boolean;
var
  NameRec:OpenFileNameA;
begin
  if dst=nil then
  begin
    result:=false;
    exit;
  end;
  FillChar(NameRec,SizeOf(NameRec),0);
  with NameRec do
  begin
    LStructSize:=SizeOf(NameRec);
    if fname=nil then
      dst[0]:=#0
    else if fname<>dst then
      StrCopy(dst,fname);
//    lpstrInitialDir:=dst;
    if Filter<>nil then
    begin
      lpstrDefExt:=StrEnd(Filter)+1;
      inc(lpstrDefExt,2); // skip "*."
    end;
    lpStrFile  :=dst;
    lpStrFilter:=Filter;
    NMaxFile   :=511;
    Flags      :=OFN_EXPLORER or OFN_OVERWRITEPROMPT;// or OFN_HIDEREADONLY;
  end;
  if open then
    result:=GetOpenFileNameA({$IFDEF FPC}@{$ENDIF}NameRec)
  else
    result:=GetSaveFileNameA({$IFDEF FPC}@{$ENDIF}NameRec);
end;

function ShowDlgW(dst:PWideChar;fname:PWideChar=nil;Filter:PWideChar=nil;open:boolean=true):boolean;
var
  NameRec:OpenFileNameW;
begin
  if dst=nil then
  begin
    result:=false;
    exit;
  end;
  FillChar(NameRec,SizeOf(NameRec),0);
  with NameRec do
  begin
    LStructSize:=SizeOf(NameRec);
    if fname=nil then
      dst[0]:=#0
    else if fname<>dst then
      StrCopyW(dst,fname);
//    lpstrInitialDir:=dst;
    if Filter<>nil then
    begin
      lpstrDefExt:=StrEndW(Filter)+1;
      inc(lpstrDefExt,2); // skip "*."
    end;
    lpStrFile  :=dst;
    lpStrFilter:=Filter;
    NMaxFile   :=511;
    Flags      :=OFN_EXPLORER or OFN_OVERWRITEPROMPT;// or OFN_HIDEREADONLY;
  end;
  if open then
    result:=GetOpenFileNameW({$IFDEF FPC}@{$ENDIF}NameRec)
  else
    result:=GetSaveFileNameW({$IFDEF FPC}@{$ENDIF}NameRec)
end;

procedure GetUnitSize(wnd:HWND; var baseUnitX, baseUnitY: integer);
var
  dc  :HDC;
  hfo :HFONT;
  tm  :TTEXTMETRIC;
  size:TSIZE;
  tmp :pWideChar;
begin
  dc:=GetDC(wnd);
  hfo:=SelectObject(dc,SendMessage(wnd,WM_GETFONT,0,0));
  GetTextMetrics(dc,tm);
  tmp:='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';
  GetTextExtentPoint32W(dc,tmp,52,size);
  SelectObject(dc,hfo);
  ReleaseDC(wnd,dc);
  baseUnitX:=(size.cx div 26+1) div 2;
  baseUnitY:=tm.tmHeight;
end;

end.

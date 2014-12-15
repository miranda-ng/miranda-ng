unit cbex;

interface

uses windows;

// build combobox with xstatus icons and names

function AddCBEx(wnd:HWND;proto:PAnsiChar):HWND;

implementation

uses messages,commctrl,m_api,common;

function AddCBEx(wnd:HWND;proto:PAnsiChar):HWND;
var
  cbei:TCOMBOBOXEXITEMW;
  total,cnt:integer;
  il:HIMAGELIST;
  icon:HICON;
  buf,buf1:array [0..127] of AnsiChar;
  b:array [0..63] of WideChar;
  ics:TCUSTOM_STATUS;
begin
  result:=0;
  SendMessage(wnd,CB_RESETCONTENT,0,0);
  StrCopy(StrCopyE(buf,proto),PS_GETCUSTOMSTATUSICON);

  if ServiceExists(@buf)=0 then
    exit;

  il:=ImageList_Create(16,16,ILC_COLOR32 or ILC_MASK,0,1);
  if il=0 then exit;

  cnt:=0;
  StrCopy(StrCopyE(buf1,proto),PS_GETCUSTOMSTATUSEX);

  cbei.mask:=CBEIF_IMAGE or CBEIF_SELECTEDIMAGE or CBEIF_TEXT; //!!
  ics.cbSize  :=SizEOf(ics);
  ics.flags   :=CSSF_STATUSES_COUNT;
  ics.szName.w:=@b;
  ics.wParam  :=@total;
  CallService(buf1,0,lParam(@ics));
  ics.flags :=CSSF_DEFAULT_NAME or CSSF_MASK_NAME or CSSF_UNICODE;

  while cnt<=total do
  begin
    if cnt=0 then
    begin
      ImageList_AddIcon(il,CallService(MS_SKIN_LOADICON,SKINICON_OTHER_SMALLDOT,0));
      cbei.pszText:=TranslateW('None');
    end
    else
    begin
      icon:=CallService(@buf,cnt,LR_SHARED);
      if icon=0 then break;
      if ImageList_AddIcon(il,icon)=-1 then break;
      ics.wParam:=@cnt;
      CallService(@buf1,0,lparam(@ics));
      cbei.pszText:=TranslateW(@b);
    end;
    cbei.iItem         :=cnt;
    cbei.iImage        :=cnt;
    cbei.iSelectedImage:=cnt;
    if SendMessageW(wnd,CBEM_INSERTITEMW,0,lparam(@cbei))=-1 then break;
    inc(cnt);
//    DestroyIcon(icon);
  end;

  if cnt=0 then
    ImageList_Destroy(il)
  else
  begin
    ImageList_Destroy(SendMessage(wnd,CBEM_SETIMAGELIST,0,il));
    SendMessage(wnd,CB_SETCURSEL,0,0);
    result:=wnd;
  end;
end;

end.

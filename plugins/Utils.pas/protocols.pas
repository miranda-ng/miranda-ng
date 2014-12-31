unit protocols;

interface

uses windows,m_api;

function FindProto(proto:PAnsiChar):uint_ptr;

function GetStatusNum(status:integer):integer;
function GetNumProto:cardinal;

function  GetProtoSetting(ProtoNum:uint_ptr;param:boolean=false):LPARAM;
procedure SetProtoSetting(ProtoNum:uint_ptr;mask:LPARAM;param:boolean=false);

function IsTunesSupported  (ProtoNum:uint_ptr):bool;
function IsXStatusSupported(ProtoNum:uint_ptr):bool;
function IsChatSupported   (ProtoNum:uint_ptr):bool;

function GetProtoStatus   (ProtoNum:uint_ptr):integer;
function GetProtoStatusNum(ProtoNum:uint_ptr):integer;
function GetProtoName     (ProtoNum:uint_ptr):PAnsiChar;
function GetProtoAccName  (ProtoNum:uint_ptr):PWideChar;

procedure FillProtoList  (list:HWND;withIcons:bool=false);
procedure CheckProtoList (list:HWND);
procedure FillStatusList (proto:uint_ptr;list:HWND;withIcons:bool=false);
procedure CheckStatusList(list:HWND;ProtoNum:uint_ptr);

function  CreateProtoList(deepscan:boolean=false):integer;
procedure FreeProtoList;

function SetStatus(proto:PAnsiChar;status:integer;txt:PAnsiChar=pointer(-1)):integer;
function SetXStatus(proto:PAnsiChar;newstatus:integer;
                    txt:PWideChar=nil;title:PWideChar=nil):integer;
function GetXStatus(proto:PAnsiChar;txt:pointer=nil;title:pointer=nil):integer;

const
  psf_online     = $0001;
  psf_invisible  = $0002;
  psf_shortaway  = $0004;
  psf_longaway   = $0008;
  psf_lightdnd   = $0010;
  psf_heavydnd   = $0020;
  psf_freechat   = $0040;
  psf_outtolunch = $0080;
  psf_onthephone = $0100;
  psf_enabled    = $0800;
  psf_all        = $08FF;
  // protocol properties
  psf_chat       = $1000;
  psf_icq        = $2000;
  psf_tunes      = $4000;
  psf_deleted    = $8000;

implementation

uses commctrl,common,dbsettings;

const
  defproto = '- default -';

const
  NumStatus = 10;
  StatCodes:array [0..NumStatus-1] of integer=(
    ID_STATUS_OFFLINE,
    ID_STATUS_ONLINE,
    ID_STATUS_INVISIBLE,
    ID_STATUS_AWAY,
    ID_STATUS_NA,
    ID_STATUS_OCCUPIED,
    ID_STATUS_DND,
    ID_STATUS_FREECHAT,
    ID_STATUS_OUTTOLUNCH,
    ID_STATUS_ONTHEPHONE);
const
  StatNames:array [0..NumStatus-1] of PWideChar=(
  'Default'{'Offline'},'Online','Invisible','Away','N/A','Occupied','DND',
  'Free for chat','Out to lunch','On the Phone');

type
  pMyProto = ^tMyProto;
  tMyProto = record
    name    :PAnsiChar; // internal account name
    descr   :PWideChar; // public   account name
//    xstat   :integer; // old ICQ XStatus
    enabled :integer;
    status  :integer; // mask
    param   :LPARAM;
  end;
  pMyProtos = ^tMyProtos;
  tMyProtos = array [0..100] of tMyProto;

var
  protos:pMyProtos;
  NumProto:cardinal;
  hAccounts:THANDLE;

function FindProto(proto:PAnsiChar):uint_ptr;
var
  i:integer;
begin
  if uint_ptr(proto)<=100 then
  begin
    result:=uint_ptr(proto);
    exit;
  end;
  for i:=1 to NumProto do
  begin
    if StrCmp(proto,protos^[i].name)=0 then
    begin
      result:=i;
      exit;
    end;
  end;
  result:=0;
end;

function IsTunesSupported(ProtoNum:uint_ptr):bool;
begin
  ProtoNum:=FindProto(PAnsiChar(ProtoNum));
  if (ProtoNum<=NumProto) and ((protos^[ProtoNum].status and psf_tunes)<>0) then
    result:=true
  else
    result:=false;
end;

function IsXStatusSupported(ProtoNum:uint_ptr):bool;
begin
  ProtoNum:=FindProto(PAnsiChar(ProtoNum));
  if (ProtoNum<=NumProto) and ((protos^[ProtoNum].status and psf_icq)<>0) then
    result:=true
  else
    result:=false;
end;

function IsChatSupported(ProtoNum:uint_ptr):bool;
begin
  ProtoNum:=FindProto(PAnsiChar(ProtoNum));
  if (ProtoNum<=NumProto) and ((protos^[ProtoNum].status and psf_chat)<>0) then
    result:=true
  else
    result:=false;
end;

function GetProtoSetting(ProtoNum:uint_ptr;param:boolean=false):LPARAM;
begin
  ProtoNum:=FindProto(PAnsiChar(ProtoNum));
  if ProtoNum<=NumProto then
  begin
    if param then
      result:=protos^[ProtoNum].param
    else
      result:=protos^[ProtoNum].enabled
  end
  else
    result:=0;
end;

procedure SetProtoSetting(ProtoNum:uint_ptr;mask:LPARAM;param:boolean=false);
begin
  ProtoNum:=FindProto(PAnsiChar(ProtoNum));
  if ProtoNum<=NumProto then
  begin
    if param then
      protos^[ProtoNum].param:=mask
    else
      protos^[ProtoNum].enabled:=mask;
  end;
end;

function GetStatusNum(status:integer):integer;
var
  i:integer;
begin
  for i:=0 to NumStatus-1 do
    if StatCodes[i]=status then
    begin
      result:=i;
      exit;
    end;
  result:=0; //-1
end;

function GetProtoStatus(ProtoNum:uint_ptr):integer;
begin
  ProtoNum:=FindProto(PAnsiChar(ProtoNum));
  result:=CallProtoService(protos^[ProtoNum].name,PS_GETSTATUS,0,0);
end;

function GetProtoStatusNum(ProtoNum:uint_ptr):integer;
begin
  ProtoNum:=FindProto(PAnsiChar(ProtoNum));
  result:=GetStatusNum(GetProtoStatus(ProtoNum));
end;

function GetNumProto:cardinal;
begin
  result:=NumProto;
end;

function GetProtoName(ProtoNum:uint_ptr):PAnsiChar;
begin
  if ProtoNum<=NumProto then
    result:=protos^[ProtoNum].name
  else
    result:=nil;
end;

function GetProtoAccName(ProtoNum:uint_ptr):PWideChar;
begin
  if ProtoNum<=NumProto then
    result:=protos^[ProtoNum].descr
  else
    result:=nil;
end;

procedure FillProtoList(list:HWND;withIcons:bool=false);
var
  item:TLVITEMW;
  lvc:TLVCOLUMN;
  i,newItem:integer;
  cli:PCLIST_INTERFACE;
begin
  FillChar(lvc,SizeOf(lvc),0);
  ListView_SetExtendedListViewStyle(list, LVS_EX_CHECKBOXES);
  if withIcons then
  begin
    cli:=PCLIST_INTERFACE(CallService(MS_CLIST_RETRIEVE_INTERFACE,0,0));
    SetWindowLongPtrW(list,GWL_STYLE,
        GetWindowLongPtrW(list,GWL_STYLE) or LVS_SHAREIMAGELISTS);
    ListView_SetImageList(list,
      CallService(MS_CLIST_GETICONSIMAGELIST,0,0),LVSIL_SMALL);
    lvc.mask:=LVCF_FMT+LVCF_IMAGE
  end
  else
  begin
    cli:=nil;
    lvc.mask:=LVCF_FMT;
  end;

  lvc.fmt :={LVCFMT_IMAGE or} LVCFMT_LEFT;
  ListView_InsertColumn(list,0,lvc);

  FillChar(item,SizeOf(item),0);
  if withIcons then
    item.mask:=LVIF_TEXT+LVIF_IMAGE
  else
    item.mask:=LVIF_TEXT;
  for i:=0 to NumProto do
  begin
    item.iItem:=i;
    item.pszText:=protos^[i].descr;
    if withIcons and (i>0) then
      item.iImage:=cli^.pfnIconFromStatusMode(protos^[i].name,ID_STATUS_ONLINE,0);
    newItem:=SendMessageW(list,LVM_INSERTITEMW,0,lParam(@item));
    if newItem>=0 then
      ListView_SetCheckState(list,newItem,(protos^[i].enabled and psf_enabled)<>0)
  end;
  ListView_SetItemState (list,0,LVIS_FOCUSED or LVIS_SELECTED,LVIS_FOCUSED or LVIS_SELECTED);

  ListView_SetColumnWidth(list,0,LVSCW_AUTOSIZE);
end;

procedure CheckProtoList(list:HWND);
var
  i:integer;
begin
  for i:=1 to ListView_GetItemCount(list) do
  begin
    with protos^[i] do
      if ListView_GetCheckState(list,i)=BST_CHECKED then
        enabled:=enabled or psf_enabled
      else
        enabled:=enabled and not psf_enabled;
  end;
end;

procedure FillStatusList(proto:uint_ptr;list:HWND;withIcons:bool=false);

  procedure AddString(num:integer;enabled:boolean;cli:PCLIST_INTERFACE);
  var
    item:LV_ITEMW;
    newItem:integer;
  begin
    FillChar(item,SizeOf(item),0);
    item.iItem  :=num;
    item.lParam :=StatCodes[num];
    if cli<>nil then
    begin
      item.mask   :=LVIF_TEXT+LVIF_PARAM+LVIF_IMAGE;
      item.iImage:=cli^.pfnIconFromStatusMode(protos^[proto].name,item.lParam,0);
    end
    else
      item.mask   :=LVIF_TEXT+LVIF_PARAM;
    item.pszText:=TranslateW(StatNames[num]);
    newItem:=SendMessageW(list,LVM_INSERTITEMW,0,lparam(@item));
    if newItem>=0 then
      ListView_SetCheckState(list,newItem,enabled);
  end;

var
  lvc:TLVCOLUMN;
  cli:PCLIST_INTERFACE;
begin
  if proto=0 then
    withIcons:=false;
  ListView_DeleteAllItems(list);
  ListView_DeleteColumn(list,0);
  FillChar(lvc,SizeOf(lvc),0);
  ListView_SetExtendedListViewStyle(list, LVS_EX_CHECKBOXES);
  if withIcons then
  begin
    cli:=PCLIST_INTERFACE(CallService(MS_CLIST_RETRIEVE_INTERFACE,0,0));
    SetWindowLongPtrW(list,GWL_STYLE,
        GetWindowLongPtrW(list,GWL_STYLE) or LVS_SHAREIMAGELISTS);
    ListView_SetImageList(list,
      CallService(MS_CLIST_GETICONSIMAGELIST,0,0),LVSIL_SMALL);
    lvc.mask:=LVCF_FMT+LVCF_IMAGE
  end
  else
  begin
    cli:=nil;
    SetWindowLongPtrW(list,GWL_STYLE,
        GetWindowLongPtrW(list,GWL_STYLE) and not LVS_SHAREIMAGELISTS);
//    ListView_SetImageList(list,0,LVSIL_SMALL);
    lvc.mask:=LVCF_FMT;
  end;
  lvc.fmt:={LVCFMT_IMAGE or} LVCFMT_LEFT;
  ListView_InsertColumn(list,0,lvc);

  AddString(0,true,nil);
  ListView_SetItemState (list,0,LVIS_FOCUSED or LVIS_SELECTED,$000F);
  with protos^[proto] do
  begin
    if (status and psf_online    )<>0 then AddString(1,(enabled and psf_online    )<>0,cli);
    if (status and psf_invisible )<>0 then AddString(2,(enabled and psf_invisible )<>0,cli);
    if (status and psf_shortaway )<>0 then AddString(3,(enabled and psf_shortaway )<>0,cli);
    if (status and psf_longaway  )<>0 then AddString(4,(enabled and psf_longaway  )<>0,cli);
    if (status and psf_lightdnd  )<>0 then AddString(5,(enabled and psf_lightdnd  )<>0,cli);
    if (status and psf_heavydnd  )<>0 then AddString(6,(enabled and psf_heavydnd  )<>0,cli);
    if (status and psf_freechat  )<>0 then AddString(7,(enabled and psf_freechat  )<>0,cli);
    if (status and psf_outtolunch)<>0 then AddString(8,(enabled and psf_outtolunch)<>0,cli);
    if (status and psf_onthephone)<>0 then AddString(9,(enabled and psf_onthephone)<>0,cli);
  end;
  ListView_SetColumnWidth(list,0,LVSCW_AUTOSIZE);
end;

procedure CheckStatusList(list:HWND;ProtoNum:uint_ptr);

  procedure SetStatusMask(stat:integer;state:bool);
  var
    i:integer;
  begin
    case stat of
      ID_STATUS_ONLINE:     i:=psf_online;
      ID_STATUS_INVISIBLE:  i:=psf_invisible;
      ID_STATUS_AWAY:       i:=psf_shortaway;
      ID_STATUS_NA:         i:=psf_longaway;
      ID_STATUS_OCCUPIED:   i:=psf_lightdnd;
      ID_STATUS_DND:        i:=psf_heavydnd;
      ID_STATUS_FREECHAT:   i:=psf_freechat;
      ID_STATUS_OUTTOLUNCH: i:=psf_outtolunch;
      ID_STATUS_ONTHEPHONE: i:=psf_onthephone;
    else
      exit;
    end;
    with protos^[ProtoNum] do
      if state then
        enabled:=enabled or i
      else
        enabled:=enabled and not i;
  end;

var
  i:integer;
  item:TLVITEM;
begin
  for i:=1 to ListView_GetItemCount(list)-1 do //skip default
  begin
    item.iItem:=i;
    item.mask:=LVIF_PARAM;
    ListView_GetItem(list,item);
    SetStatusMask(item.lParam,ListView_GetCheckState(list,i)=BST_CHECKED)
  end;
end;

function AccListChanged(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  i:integer;
begin
  result:=0;
  case wParam of
    PRAC_ADDED: begin
    end;
    PRAC_CHANGED: begin
      i:=FindProto(PPROTOACCOUNT(lParam).szModuleName);
      if i>0 then
        protos^[i].descr:=PPROTOACCOUNT(lParam).tszAccountName.w;
    end;
    PRAC_REMOVED: begin
    end;
  end;
end;

function CreateProtoList(deepscan:boolean=false):integer;
var
  protoCount,i:integer;
  proto:^PPROTOACCOUNT;
  buf:array [0..127] of AnsiChar;
  flag:integer;
  p:pAnsichar;
//  hContract:THANDLE;
begin
  CallService(MS_PROTO_ENUMACCOUNTS,wparam(@protoCount),lparam(@proto));

  mGetMem(protos,(protoCount+1)*SizeOf(tMyProto)); // 0 - default
  NumProto:=0;
  with protos^[0] do
  begin
    name   :=defproto;
    descr  :=defproto;
    status :=-1;
    enabled:=-1;
  end;
  for i:=1 to protoCount do
  begin
    inc(NumProto);
    with protos^[NumProto] do
    begin
      name :=proto^^.szModuleName;
      descr:=proto^^.tszAccountName.w;

      enabled:=psf_all;//psf_enabled;
      status :=0;
//        xstat  :=-1;
      flag:=CallProtoService(name,PS_GETCAPS,PFLAGNUM_2,0);
      if (flag and PF2_ONLINE)    <>0 then status:=status or psf_online;
      if (flag and PF2_INVISIBLE) <>0 then status:=status or psf_invisible;
      if (flag and PF2_SHORTAWAY) <>0 then status:=status or psf_shortaway;
      if (flag and PF2_LONGAWAY)  <>0 then status:=status or psf_longaway;
      if (flag and PF2_LIGHTDND)  <>0 then status:=status or psf_lightdnd;
      if (flag and PF2_HEAVYDND)  <>0 then status:=status or psf_heavydnd;
      if (flag and PF2_FREECHAT)  <>0 then status:=status or psf_freechat;
      if (flag and PF2_OUTTOLUNCH)<>0 then status:=status or psf_outtolunch;
      if (flag and PF2_ONTHEPHONE)<>0 then status:=status or psf_onthephone;

      flag:=CallProtoService(name,PS_GETCAPS,PFLAGNUM_1,0);
      if ((flag and PF1_CHAT)<>0) or
         (DBReadByte(0,name,'CtcpChatAccept',13)<>13) or // IRC
         (DBReadByte(0,name,'Jud',13)<>13) then          // Jabber
//        flag:=CallProtoService(name,PS_GETCAPS,PFLAGNUM_1,0);
//        if (flag and PF1_CHAT)<>0 then
        status:=status or psf_chat;

      p:=StrCopyE(buf,name);
      StrCopy(p,PS_GETCUSTOMSTATUSEX);
      if ServiceExists(buf)<>0 then
        status:=status or psf_icq;

      StrCopy(p,PS_SET_LISTENINGTO);
      if ServiceExists(buf)<>0 then
        status:=status or psf_tunes;

    end;
    inc(proto);
  end;

{
  if deepscan then
  begin
    hContact:=db_find_first();
    while hContact<>0 do
    begin
      i:=NumProto;
      while i>0 do
      begin
        if StrCmp()=0 then
          break;
        dec(i);
      end;

      hContact:=db_find_next(hContact);
    end;
  end;
}
  result:=NumProto;

  hAccounts:=HookEvent(ME_PROTO_ACCLISTCHANGED,@AccListChanged);
end;

procedure FreeProtoList;
begin
  UnhookEvent(hAccounts);
  mFreeMem(protos);
  NumProto:=0;
end;

function SetStatus(proto:PAnsiChar;status:integer;txt:PAnsiChar=pointer(-1)):integer;
//var  nas:TNAS_PROTOINFO;
begin
  if status>0 then
    result:=CallProtoService(proto,PS_SETSTATUS,status,0)
  else
    result:=-1;
  if txt<>PAnsiChar(-1) then
  begin
//    if ServiceExists(MS_NAS_SETSTATEA)=0 then
      result:=CallProtoService(proto,PS_SETAWAYMSG,abs(status),lparam(txt))
(*
    else
    begin
  {
      nas.Msg.w:=mmi.malloc((StrLenW(txt)+1)*SizeOf(WideChar));
      nas.Msg.w^:=#0;
      StrCopyW(nas.Msg.w,txt);
  }
      StrDup(nas.Msg.a,txt);
      nas.Flags  :=0;
      nas.cbSize :=SizeOf(nas);
      nas.szProto:=proto;
      nas.status :=abs(status){0};
      result:=CallService(MS_NAS_SETSTATEA,LPARAM(@nas),1);
    end;
*)
  end;
end;

function SetXStatus(proto:PAnsiChar;newstatus:integer;
                    txt:PWideChar=nil;title:PWideChar=nil):integer;
var
  ics:TCUSTOM_STATUS;
begin
  result:=0;
  if IsXStatusSupported(uint_ptr(proto)) then
  begin
    with ics do
    begin
      cbSize:=SizeOf(ics);
      flags:=CSSF_UNICODE;
      if newstatus>=0 then
      begin
        flags:=flags or CSSF_MASK_STATUS;
        status:=@newstatus;
      end;
      if title<>PWideChar(-1) then
      begin
        flags:=flags or CSSF_MASK_NAME;
        szName.w:=title;
      end;
      if txt<>PWideChar(-1) then
      begin
        flags:=flags or CSSF_MASK_MESSAGE;
        szMessage.w:=txt;
      end;
    end;
    result:=CallProtoService(proto,PS_SETCUSTOMSTATUSEX,0,lparam(@ics));
  end;
end;

function GetXStatus(proto:PAnsiChar;txt:pointer=nil;title:pointer=nil):integer;
var
{
  buf:array [0..127] of AnsiChar;
  pc:PAnsiChar;
  param:array [0..63] of AnsiChar;
}
  ics:TCUSTOM_STATUS;
  i,j:integer;
begin
  result:=0;
  if IsXStatusSupported(uint_ptr(proto)) then
  begin

    if (title<>nil) or (txt<>nil) then
    begin
      with ics do
      begin
        cbSize:=SizeOf(ics);
        flags:=CSSF_STR_SIZES;
        wParam:=@i;
        lParam:=@j;
      end;
      CallProtoService(proto,PS_GETCUSTOMSTATUSEX,0,LPARAM(@ics));
    end;

    with ics do
    begin
      flags:=CSSF_MASK_STATUS;
      status:=@result;
    end;

    if title<>nil then
    begin
      mGetMem(title^,(i+1)*SizeOf(WideChar));
      with ics do
      begin
        flags   :=flags or CSSF_MASK_NAME or CSSF_UNICODE;
        szName.w:=PWideChar(title^);
      end;
    end;

    if txt<>nil then
    begin
      mGetMem(txt^,(j+1)*SizeOf(WideChar));
      with ics do
      begin
        flags:=flags or CSSF_MASK_MESSAGE or CSSF_UNICODE;
        szMessage.w:=PWideChar(txt^);
      end;
    end;

    CallProtoService(proto,PS_GETCUSTOMSTATUSEX,0,LPARAM(@ics));

{
    StrCopy(buf,proto);
    StrCat (buf,PS_GETCUSTOMSTATUS);
    result:=CallService(buf,0,0);
    if (txt<>nil) or (title<>nil) then
    begin
      move('XStatus',param,7);
      IntToStr(param+7,result);
      pc:=strend(param);

      if txt<>nil then
      begin
        StrCopy(pc,'Msg'); PWideChar(txt^):=DBReadUnicode(0,proto,param,nil);
      end;
      if title<>nil then
      begin
        StrCopy(pc,'Name'); PWideChar(title^):=DBReadUnicode(0,proto,param,nil);
      end;
    end;
}
  end;
end;

end.

{Statistic}
unit proto;
{$include compilers.inc}
interface
{$Resource proto.res}
implementation

uses
  windows,messages,commctrl,
  common,m_api,mirutils,dbsettings,wrapper,
  global,wat_api;

{$include i_proto_rc.inc}

const
  ShareOptText = 'ShareMusic';
const
  IcoBtnContext:PAnsiChar='WATrack_Context';
const
  MenuUserInfoPos = 500050000;

const
  wpRequest = 'WAT###0_';
  wpAnswer  = 'WAT###1_';
  wpError   = 'WAT###2_';
  wpMessage = 'WAT###3_';
  wpRequestNew = 'ASKWAT';

const
  SendRequestText:PAnsiChar =
    'WATrack internal info - sorry!';
{
    'If you see this message, probably you have no "WATrack" plugin installed or uses old '+
    'version. See http://miranda-im.org/download/details.php?action=viewfile&id=2345 or '+
    'http://awkward.miranda.im/ (beta versions) for more information and download.';
}
const
  hmInRequest  = $0001;
  hmOutRequest = $0002;
  hmInInfo     = $0004;
  hmOutInfo    = $0008;
  hmInError    = $0010;
  hmOutError   = $0020;
  hmIRequest   = $0040;
  hmISend      = $0080;

var
  hSRM,
  hGCI,
  icchangedhook,
  hAddUserHook,
  hContactMenuItem,
  contexthook:THANDLE;
  ProtoText:pWideChar;
  HistMask:cardinal;

{$include i_proto_opt.inc}
{$include i_proto_dlg.inc}

procedure AddEvent(hContact:TMCONTACT;atype,flag:integer;data:pointer;size:integer;time:dword=0);
var
  dbeo:TDBEVENTINFO;
begin
  FillChar(dbeo,SizeOf(dbeo),0);
  with dbeo do
  begin
    cbSize   :=SizeOf(dbeo);
    eventType:=atype;
    szModule :=PluginShort;
    if data=nil then
    begin
      PAnsiChar(data):='';
      size:=1;
    end;
    pBlob    :=data;
    cbBlob   :=size;
    flags    :=flag;
    if time<>0 then
      Timestamp:=time
    else
      Timestamp:=GetCurrentTime;
  end;
  db_event_add(hContact, @dbeo);
end;

{SEND-time text translation}
(*
const
  BufSize = 16384;

function FormatToBBW(src:PWideChar):PWideChar;
var
  buf:array [0..32] of WideChar;
  p:PWideChar;
  i,j:integer;
begin
  result:=src;
  StrReplaceW(src,'{b}'  ,'[b]');
  StrReplaceW(src,'{/b}' ,'[/b]');
  StrReplaceW(src,'{u}'  ,'[u]');
  StrReplaceW(src,'{/u}' ,'[/u]');
  StrReplaceW(src,'{i}'  ,'[i]');
  StrReplaceW(src,'{/i}' ,'[/i]');
  StrReplaceW(src,'{/cf}','[/color]');
  StrReplaceW(src,'{/bg}','');
  StrCopyW(buf,'[color=');
  repeat
    i:=StrPosW(src,'{cf');
    if i=0 then break;
    j:=i;
    dec(i);
    while (src[j]<>#0) and (src[j]<>'}') do inc(j);
    if src[j]='}' then inc(j);
    case StrToInt(src+i+3) of
      4,10:  p:='green]';
      5,6:   p:='red]';
      7,14:  p:='magenta]';
      3,11,
      12,13: p:='blue]';
      8,9:   p:='yellow]';
      2,15:  p:='black]';
    else
      {1,16:}  p:='white]';
    end;
    StrCopyW(buf+7,p);
    StrCopyW(src+i,src+j);
    StrInsertW(buf,src,i);
  until false;
  repeat
    i:=StrIndex(src,'{bg');
    if i=0 then break;
    j:=i;
    dec(i);
    while (src[j]<>#0) and (src[j]<>'}') do inc(j);
    if src[j]='}' then inc(j);
    StrCopyW(src+i,src+j);
  until false;
end;

function SendMessageProcW(wParam:WPARAM; lParam:LPARAM):integer; cdecl;
var
  ccs:PCCSDATA;
  uns,s,ss:pWideChar;
  p:PAnsiChar;
  present:boolean;
  i:integer;
begin
  if DisablePlugin<>dsPermanent then
  begin
    ccs:=PCCSDATA(lParam);
    if ccs^.wParam=0 then
      present:=StrPos('%music%',PAnsiChar(ccs^.lParam))<>nil
    else // not needed?
    begin
      uns:=PWideChar(ccs^.lParam+StrLen(PAnsiChar(ccs^.lParam))+1);
      present:=StrPos(uns,'%music%')<>nil;
    end;

    if present then
    begin
      if CallService(MS_WAT_GETMUSICINFO,0,0)=WAT_RES_NOTFOUND then
        s:=nil
      else
      begin
        if SimpleMode<>BST_UNCHECKED then
          i:=0
        else
          i:=CallService(MS_PROTO_GETCONTACTBASEPROTO,ccs^.hContact,0);
        s:=GetMacros(TM_MESSAGE,i);
      end;
  //    if s<>nil then // for empty strings
      begin
        mGetMem(ss,BufSize*SizeOf(pWideChar));
        FillChar(ss^,BufSize*SizeOf(pWideChar),0);
        if ccs^.wParam=0 then
          AnsiToWide(PAnsiChar(ccs^.lParam),uns,UserCP);
        StrCopyW(ss,uns);
        if ccs^.wParam=0 then
          mFreeMem(uns);
        StrReplaceW(ss,'%music%',s);
        mFreeMem(s);
        if StrPos(ss,'{')<>nil then
          FormatToBBW(ss);
        s:=PWideChar(ccs^.lParam);
        WideToAnsi(ss,p,UserCP);
        if ccs^.wParam=0 then
        begin
          ccs^.lParam:=tlparam(p);
        end
        else
        begin
          move(PAnsiChar(ss)^,(PAnsiChar(ss)+StrLen(p)+1)^,
            (StrLenW(ss)+1)*SizeOf(WideChar));
          StrCopy(PAnsiChar(ss),p);
          ccs^.lParam:=tlparam(ss);
        end;
        result:=CallService(MS_PROTO_CHAINSEND,wParam,lParam);
        mFreeMem(p);
        ccs^.lParam:=tlparam(s);
        mFreeMem(ss);
        exit;
      end;
    end;
  end;
  result:=CallService(MS_PROTO_CHAINSEND,wParam,lParam);
end;
*)

function ReceiveMessageProcW(wParam:WPARAM; lParam:LPARAM):int_ptr; cdecl;
const
  bufsize = 4096*SizeOf(WideChar);
var
  ccs:PCCSDATA;
  s:pWideChar;
  buf:PWideChar;
  data:PByte;
  dataSize:int;
  encodedStr:PAnsiChar;
//  pos_artist,pos_title,pos_album:PwideChar;
  pos_template:pWideChar;
  curpos:pWideChar;
  encbuf:pWideChar;
  i:integer;
  textpos:PWideChar;
  pc:PAnsiChar;
  isNewRequest:bool;
  si:pSongInfo;
begin
  ccs:=PCCSDATA(lParam);
  result:=0;
  mGetMem(buf,bufsize);

  isNewRequest:=StrCmp(PPROTORECVEVENT(ccs^.lParam)^.szMessage.a,
     wpRequestNew,Length(wpRequestNew))=0;

  if isNewRequest or
     (StrCmp(PPROTORECVEVENT(ccs^.lParam)^.szMessage.a,
             wpRequest,Length(wpRequest))=0) then
  begin
    StrCopy(PAnsiChar(buf),PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO,ccs^.hContact,0)));
    i:=DBReadWord(ccs^.hContact,PAnsiChar(buf),'ApparentMode');
    StrCat(PAnsiChar(buf),PS_GETSTATUS);
    if (i=ID_STATUS_OFFLINE) or
       ((i=0) and (CallService(PAnsiChar(buf),0,0)=ID_STATUS_INVISIBLE)) then
    begin
      result:=CallService(MS_PROTO_CHAINRECV,wParam,lParam);
    end
    else if DBReadByte(ccs^.hContact,strCList,ShareOptText,0)<>0 then
// or (NotListedAllow and (DBReadByte(ccs^.hContact,strCList,'NotOnList',0))
    begin
      if (HistMask and hmInRequest)<>0 then
        AddEvent(ccs^.hContact,EVENTTYPE_WAT_REQUEST,DBEF_READ,nil,0,
                PPROTORECVEVENT(ccs^.lParam)^.Timestamp);
      if GetContactStatus(ccs^.hContact)<>ID_STATUS_OFFLINE then
      begin
//!! Request Answer
        curpos:=nil;
        if DisablePlugin<>dsPermanent then
        begin
          if CallService(MS_WAT_GETMUSICINFO,0,0)=WAT_RES_NOTFOUND then
          begin
            s:=#0#0#0'No player found at this time';
            textpos:=s+3;
          end
          else
          begin
            if not isNewRequest then
            begin
              FillChar(buf^,bufsize,0);
              si:=pSongInfo(CallService(MS_WAT_RETURNGLOBAL,0,0));
              StrCopyW(buf   ,si^.artist); curpos:=StrEndW(buf)+1;
              StrCopyW(curpos,si^.title);  curpos:=StrEndW(curpos)+1;
              StrCopyW(curpos,si^.album);  curpos:=StrEndW(curpos)+1;
            end
            else
              curpos:=buf;
//!! check to DisableTemporary

            s:=PWideChar(CallService(MS_WAT_REPLACETEXT,0,tlparam(ProtoText)));
            textpos:=StrCopyW(curpos,s);
            mFreeMem(s);
            curpos:=StrEndW(curpos)+1;
          end;
        end
        else
        begin
          s:=#0#0#0'Sorry, but i don''t use WATrack right now!';
          textpos:=s+3;
        end;
// encode
        if not isNewRequest then
        begin
          if curpos<>nil then
          begin
            data:=PByte(buf);
            dataSize:=PAnsiChar(curpos)-PAnsiChar(buf);
          end
          else
          begin
            data:=PByte(s);
            dataSize:=(StrLenW(textpos)+3+1)*SizeOf(PWideChar);
          end;
          encodedStr:=mir_base64_encode(data,dataSize);
          mGetMem(encbuf,Length(encodedStr)+1+Length(wpAnswer));
          StrCopy(PAnsiChar(encbuf),wpAnswer);
          StrCopy(PAnsiChar(encbuf)+Length(wpAnswer),encodedStr);
          mFreeMem(encodedStr);

          if (HistMask and hmOutInfo)<>0 then
            AddEvent(ccs^.hContact,EVENTTYPE_WAT_ANSWER,DBEF_SENT,data,dataSize);
          CallContactService(ccs^.hContact,PSS_MESSAGE,0,tlparam(encbuf));
        end
        else
        begin
          i:=WideToCombo(textpos,encbuf,UserCP);
          if (HistMask and hmOutInfo)<>0 then
            AddEvent(ccs^.hContact,EVENTTYPE_WAT_MESSAGE,DBEF_SENT,encbuf,i);
//          if CallContactService(ccs^.hContact,PSS_MESSAGEW,PREF_UNICODE,tlparam(encbuf))=
//             ACKRESULT_FAILED then
            CallContactService(ccs^.hContact,PSS_MESSAGE,PREF_UNICODE,tlparam(encbuf));
        end;
        mFreeMem(encbuf);
      end;
    end
    else
    begin
      if (HistMask and hmIRequest)<>0 then
        AddEvent(ccs^.hContact,EVENTTYPE_WAT_REQUEST,DBEF_READ,nil,0,
                 PPROTORECVEVENT(ccs^.lParam)^.Timestamp);
      if (HistMask and hmISend)<>0 then
      begin
//!! Request Error Answer
        if isNewRequest then
          pc:=PAnsiChar(buf)
        else
        begin
          StrCopy(PAnsiChar(buf),wpError);
          pc:=PAnsiChar(buf)+Length(wpError);
        end;
        StrCopy(pc,'Sorry, but you have no permission to obtain this info!');
        CallContactService(ccs^.hContact,PSS_MESSAGE,0,tlparam(buf));
        if (HistMask and hmOutError)<>0 then
        begin
          AddEvent(ccs^.hContact,EVENTTYPE_WAT_ERROR,DBEF_SENT,nil,0,
                   PPROTORECVEVENT(ccs^.lParam)^.Timestamp);
        end;
      end;
    end;
  end
  else if StrCmp(PPROTORECVEVENT(ccs^.lParam)^.szMessage.a,wpAnswer,Length(wpAnswer))=0 then
  begin
// decode
    data:=mir_base64_decode(PPROTORECVEVENT(ccs^.lParam)^.szMessage.a+Length(wpAnswer),dataSize);

    curpos:=pWideChar(data);           // pos_artist:=curpos;
    while curpos^<>#0 do inc(curpos); inc(curpos); // pos_title :=curpos;
    while curpos^<>#0 do inc(curpos); inc(curpos); // pos_album :=curpos;
    while curpos^<>#0 do inc(curpos); inc(curpos);
    pos_template:=curpos;

    if (HistMask and hmInInfo)<>0 then
      AddEvent(ccs^.hContact,EVENTTYPE_WAT_ANSWER,DBEF_READ,
          data,dataSize,
          PPROTORECVEVENT(ccs^.lParam)^.Timestamp);
//  Action

    StrCopyW(buf,TranslateW('Music Info from '));
    StrCatW (buf,PWideChar(CallService(MS_CLIST_GETCONTACTDISPLAYNAME,ccs^.hContact,GCDNF_UNICODE)));

    MessageBoxW(0,TranslateW(pos_template),buf,MB_ICONINFORMATION);

    mFreeMem(data);
  end
  else if StrCmp(PPROTORECVEVENT(ccs^.lParam)^.szMessage.a,wpError,Length(wpError))=0 then
  begin
    if (HistMask and hmInError)<>0 then
      AddEvent(ccs^.hContact,EVENTTYPE_WAT_ERROR,DBEF_READ,nil,0,
               PPROTORECVEVENT(ccs^.lParam)^.Timestamp);
{
    AnsiToWide(PAnsiChar(CallService(MS_CLIST_GETCONTACTDISPLAYNAME,ccs^.hContact,0)),s);
    StrCopyW(buf,s);
    StrCatW (buf,TranslateW(' answer you'));
    mFreeMem(s);
}
    MessageBoxA(0,Translate(PPROTORECVEVENT(ccs^.lParam)^.szMessage.a+Length(wpError)),
               Translate('You Get Error'),MB_ICONERROR);
  end
  else
    result:=CallService(MS_PROTO_CHAINRECV,wParam,lParam);
  mFreeMem(buf);
end;

function SendRequest(hContact:WPARAM;lParam:LPARAM):int_ptr; cdecl;
var
  buf:array [0..2047] of AnsiChar;
begin
  result:=0;
  StrCopy(buf,wpRequest);
  StrCopy(buf+Length(wpRequest),SendRequestText);
  CallContactService(hContact,PSS_MESSAGE,0,tlparam(@buf));
  if (HistMask and hmOutRequest)<>0 then
    AddEvent(hContact,EVENTTYPE_WAT_REQUEST,DBEF_SENT,nil,0);
end;

procedure RegisterContacts;
var
  hContact:TMCONTACT;
begin
  hContact:=db_find_first();
  while hContact<>0 do
  begin
    if not IsChat(hContact) then
      CallService(MS_PROTO_ADDTOCONTACT,hContact,lparam(PluginShort));
    hContact:=db_find_next(hContact);
  end;
end;

function HookAddUser(hContact:WPARAM;lParam:LPARAM):int; cdecl;
begin
  result:=0;
  if not IsChat(hContact) then
    CallService(MS_PROTO_ADDTOCONTACT,hContact,tlparam(PluginShort));
end;

function OnContactMenu(hContact:WPARAM;lParam:LPARAM):int;cdecl;
var
  mi:TCListMenuItem;
begin
  FillChar(mi,SizeOf(mi),0);
  mi.cbSize:=sizeof(mi);
  if IsMirandaUser(hContact)<=0 then
    mi.flags:=CMIF_NOTOFFLINE or CMIF_NOTOFFLIST or CMIM_FLAGS or CMIF_HIDDEN
  else
    mi.flags:=CMIF_NOTOFFLINE or CMIF_NOTOFFLIST or CMIM_FLAGS;
  CallService(MS_CLIST_MODIFYMENUITEM,hContactMenuItem,tlparam(@mi));
  result:=0;
end;

procedure SetProtocol;
var
  desc:TPROTOCOLDESCRIPTOR;
begin
  desc.cbSize:=PROTOCOLDESCRIPTOR_V3_SIZE;//SizeOf(desc);
  desc.szName:=PluginShort;
  desc._type :=PROTOTYPE_TRANSLATION;

  CallService(MS_PROTO_REGISTERMODULE,0,lparam(@desc));
//  CreateProtoServiceFunction(PluginShort,PSS_MESSAGE ,@SendMessageProcW);
//  CreateProtoServiceFunction(PluginShort,PSS_MESSAGEW,@SendMessageProcW);
  hSRM:=CreateProtoServiceFunction(PluginShort,PSR_MESSAGE ,@ReceiveMessageProcW);
//  CreateProtoServiceFunction(PluginShort,PSR_MESSAGEW,@ReceiveMessageProcW);
end;

function IconChanged(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  mi:TCListMenuItem;
begin
  result:=0;
  FillChar(mi,SizeOf(mi),0);
  mi.cbSize:=sizeof(mi);
  mi.flags :=CMIM_ICON;

  mi.hIcon:=CallService(MS_SKIN2_GETICON,0,tlparam(IcoBtnContext));
  CallService(MS_CLIST_MODIFYMENUITEM,hContactMenuItem,tlparam(@mi));
end;

procedure RegisterIcons;
var
  sid:TSKINICONDESC;
begin
  FillChar(sid,SizeOf(TSKINICONDESC),0);
  sid.cbSize:=SizeOf(TSKINICONDESC);
  sid.cx:=16;
  sid.cy:=16;
  sid.szSection.a:=PluginShort;

  sid.hDefaultIcon   :=LoadImage(hInstance,MAKEINTRESOURCE(BTN_CONTEXT),IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoBtnContext;
  sid.szDescription.a:='Context Menu';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);
//!!
  icchangedhook:=HookEvent(ME_SKIN2_ICONSCHANGED,@IconChanged);
end;

// ------------ base interface functions -------------

function InitProc(aGetStatus:boolean=false):integer;
var
  mi:TCListMenuItem;
begin
  if aGetStatus then
  begin
    if GetModStatus=0 then
    begin
      result:=0;
      exit;
    end;
  end
  else
    SetModStatus(1);
  result:=1;

  ReadOptions;
  RegisterIcons;

  FillChar(mi, sizeof(mi), 0);
  mi.cbSize       :=sizeof(mi);
  mi.szPopupName.a:=PluginShort;
  mi.flags        :=CMIF_NOTOFFLINE or CMIF_NOTOFFLIST;
//  mi.popupPosition:=MenuUserInfoPos;
  mi.hIcon        :=CallService(MS_SKIN2_GETICON,0,lparam(IcoBtnContext));
  mi.szName.a     :='Get user''s Music Info';
  mi.pszService   :=MS_WAT_GETCONTACTINFO;
  hContactMenuItem:=Menu_AddContactMenuItem(@mi);

  SetProtocol;
  RegisterContacts;
  hGCI:=CreateServiceFunction(MS_WAT_GETCONTACTINFO,@SendRequest);
  contexthook :=HookEvent(ME_CLIST_PREBUILDCONTACTMENU,@OnContactMenu);
  hAddUserHook:=HookEvent(ME_DB_CONTACT_ADDED         ,@HookAddUser);
end;

procedure DeInitProc(aSetDisable:boolean);
begin
  if aSetDisable then
    SetModStatus(0);

  UnhookEvent(hAddUserHook);
  UnhookEvent(contexthook);
  UnhookEvent(icchangedhook);

  DestroyServiceFunction(hSRM);
  DestroyServiceFunction(hGCI);
  mFreeMem(ProtoText);
end;

function AddOptionsPage(var tmpl:pAnsiChar;var proc:pointer;var name:PAnsiChar):integer;
begin
  tmpl:='MISC';
  proc:=@DlgProcOptions;
  name:='Misc';
  result:=0;
end;

var
  vproto:twModule;

procedure Init;
begin
  vproto.Next      :=ModuleLink;
  vproto.Init      :=@InitProc;
  vproto.DeInit    :=@DeInitProc;
  vproto.AddOption :=@AddOptionsPage;
  vproto.Check     :=nil;
  vproto.ModuleName:='Protocol';
  ModuleLink       :=@vproto;
end;

begin
  Init;
end.

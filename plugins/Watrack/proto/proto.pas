{Statistic}
unit proto;
{$include compilers.inc}
interface
{$Resource proto.res}
implementation

uses
  windows,messages,commctrl,
  common,m_api,mirutils,mircontacts,dbsettings,wrapper,
  global,wat_api;

{$include resource.inc}
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
  hContactMenuItem: THANDLE;
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
    StrCopy(PAnsiChar(buf),Proto_GetProtoName(ccs^.hContact));
    i:=DBReadWord(ccs^.hContact,PAnsiChar(buf),'ApparentMode');
    StrCat(PAnsiChar(buf),PS_GETSTATUS);
    if (i=ID_STATUS_OFFLINE) or
       ((i=0) and (CallService(PAnsiChar(buf),0,0)=ID_STATUS_INVISIBLE)) then
    begin
      result:=Proto_ChainRecv(wParam,ccs);
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
          mFreeMem(encbuf);
        end
        else
        begin
          WideToUTF8(textpos,encodedStr);
          if (HistMask and hmOutInfo)<>0 then
            AddEvent(ccs^.hContact,EVENTTYPE_WAT_MESSAGE,DBEF_SENT or DBEF_UTF,encodedStr,StrLen(encodedStr));
          CallContactService(ccs^.hContact,PSS_MESSAGE,0,tlparam(encodedStr));
          mFreeMem(encodedStr);
        end;
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
    StrCatW(buf,cli^.pfnGetContactDisplayName(ccs^.hContact,0));

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
    result:=Proto_ChainRecv(wParam,ccs);
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
      Proto_AddToContact(hContact,PluginShort);
    hContact:=db_find_next(hContact);
  end;
end;

function HookAddUser(hContact:WPARAM;lParam:LPARAM):int; cdecl;
begin
  result:=0;
  if not IsChat(hContact) then
    Proto_AddToContact(hContact,PluginShort);
end;

function OnContactMenu(hContact:WPARAM;lParam:LPARAM):int;cdecl;
begin
  if IsMirandaUser(hContact)<=0 then
    Menu_ShowItem(hContactMenuItem, 0)
  else
    Menu_ShowItem(hContactMenuItem, 1);
  result:=0;
end;

procedure SetProtocol;
var
  desc:TPROTOCOLDESCRIPTOR;
begin
  desc.cbSize:=PROTOCOLDESCRIPTOR_V3_SIZE;//SizeOf(desc);
  desc.szName:=PluginShort;
  desc._type :=PROTOTYPE_TRANSLATION;
  Proto_RegisterModule(@desc);

  hSRM:=CreateProtoServiceFunction(PluginShort,PSR_MESSAGE ,@ReceiveMessageProcW);
end;

procedure RegisterIcons;
var
  sid:TSKINICONDESC;
begin
  FillChar(sid,SizeOf(TSKINICONDESC),0);
  sid.cx:=16;
  sid.cy:=16;
  sid.szSection.a:=PluginShort;

  sid.hDefaultIcon   :=LoadImage(hInstance,MAKEINTRESOURCE(BTN_CONTEXT),IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoBtnContext;
  sid.szDescription.a:='Context Menu';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);
end;

// ------------ base interface functions -------------

function InitProc(aGetStatus:boolean=false):integer;
var
  mi:TMO_MenuItem;
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
  mi.root         :=Menu_CreateRoot(MO_MAIN, 'Watrack', 0, 0, 0);
  mi.flags        :=CMIF_NOTOFFLINE or CMIF_NOTOFFLIST;
  mi.hIcon        :=IcoLib_GetIcon(IcoBtnContext,0);
  mi.szName.a     :='Get user''s Music Info';
  mi.pszService   :=MS_WAT_GETCONTACTINFO;
  hContactMenuItem:=Menu_AddContactMenuItem(@mi);

  SetProtocol;
  RegisterContacts;
  hGCI:=CreateServiceFunction(MS_WAT_GETCONTACTINFO,@SendRequest);
  HookEvent(ME_CLIST_PREBUILDCONTACTMENU,@OnContactMenu);
  HookEvent(ME_DB_CONTACT_ADDED         ,@HookAddUser);
end;

procedure DeInitProc(aSetDisable:boolean);
begin
  if aSetDisable then
    SetModStatus(0);

  DestroyServiceFunction(hSRM);
  DestroyServiceFunction(hGCI);
  mFreeMem(ProtoText);
end;

function AddOptionsPage(var tmpl:pAnsiChar;var proc:pointer;var name:PAnsiChar):integer;
begin
  tmpl:=PAnsiChar(IDD_OPT_MISC);
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

{$Include compilers.inc}
unit mirutils;

interface

uses windows,m_api;

// for miranda services
const
  rtUnkn = 0;
  rtInt  = 1;
  rtWide = 2;
  rtAnsi = 3;
  rtUTF8 = 4;


// icons
function SetButtonIcon(btn:HWND;name:PAnsiChar):HICON;
function RegisterSingleIcon(resname,ilname,descr,group:PAnsiChar):int;

// others

function ConvertFileName(src:pAnsiChar;hContact:TMCONTACT=0):pAnsiChar; overload;
function ConvertFileName(src:pWideChar;hContact:TMCONTACT=0):pWideChar; overload;
function ConvertFileName(src:pAnsiChar;dst:pAnsiChar;hContact:TMCONTACT=0):pAnsiChar; overload;
function ConvertFileName(src:pWideChar;dst:pWideChar;hContact:TMCONTACT=0):pWideChar; overload;

procedure ShowPopupW(text:pWideChar;title:pWideChar=nil);
function GetAddonFileName(prefix,altname,path:PAnsiChar;ext:PAnsiChar):PAnsiChar;
function TranslateA2W(sz:PAnsiChar):PWideChar;
function MirandaCP:integer;

function isVarsInstalled:bool;
function ParseVarString(astr:pAnsiChar;aContact:TMCONTACT=0;extra:pAnsiChar=nil):pAnsiChar; overload;
function ParseVarString(astr:pWideChar;aContact:TMCONTACT=0;extra:pWideChar=nil):pWideChar; overload;
function ShowVarHelp(dlg:HWND;id:integer=0):integer;

function  IsChat(hContact:TMCONTACT):bool;
procedure SendToChat(hContact:TMCONTACT;pszText:PWideChar);

function LoadContact(group,setting:PAnsiChar):TMCONTACT;
function SaveContact(hContact:TMCONTACT;group,setting:PAnsiChar):integer;

function SetCListSelContact(hContact:TMCONTACT):TMCONTACT;
function GetCListSelContact:TMCONTACT; {$IFDEF DELPHI_10_UP}inline;{$ENDIF}
function GetContactProtoAcc(hContact:TMCONTACT):PAnsiChar;
function  IsMirandaUser(hContact:TMCONTACT):integer; // >0=Miranda; 0=Not miranda; -1=unknown
procedure ShowContactDialog(hContact:TMCONTACT;DblClk:boolean=true;anystatus:boolean=true);
function  FindContactHandle(proto:pAnsiChar;const dbv:TDBVARIANT;is_chat:boolean=false):TMCONTACT;
function  WndToContact(wnd:HWND):TMCONTACT; overload;
function  WndToContact:TMCONTACT; overload;
function  GetContactStatus(hContact:TMCONTACT):integer;
// -2 - deleted account, -1 - disabled account, 0 - hidden
// 1 - metacontact, 2 - submetacontact, positive - active
// proto - ASSIGNED buffer
function  IsContactActive(hContact:TMCONTACT;proto:pAnsiChar=nil):integer;

function CreateGroupW(name:pWideChar;hContact:TMCONTACT):integer;

function MakeGroupMenu(idxfrom:integer=100):HMENU;
function GetNewGroupName(parent:HWND):pWideChar;

const
  MAX_REDIRECT_RECURSE = 4;

function SendRequest(url:PAnsiChar;rtype:int;args:pAnsiChar=nil;hNetLib:THANDLE=0):pAnsiChar;

function GetFile(url:PAnsiChar;save_file:PAnsiChar;
                 hNetLib:THANDLE=0;recurse_count:integer=0):bool; overload;
// next is just wrapper
function GetFile(url:PWideChar;save_file:PWideChar;
                 hNetLib:THANDLE=0;recurse_count:integer=0):bool; overload;

function GetProxy(hNetLib:THANDLE):PAnsiChar;
function LoadImageURL(url:pAnsiChar;size:integer=0):HBITMAP;

implementation

uses
  Messages,
  dbsettings,freeimage,
  common,io,syswin;

const
  clGroup = 'Group';
// Save / Load contact
const
  opt_cproto   = 'cproto';
  opt_cuid     = 'cuid';
  opt_ischat   = 'ischat';

function SetButtonIcon(btn:HWND;name:PAnsiChar):HICON;
begin
  result:=CallService(MS_SKIN2_GETICON,0,LPARAM(name));
  SendMessage(btn,BM_SETIMAGE,IMAGE_ICON,result);
end;

function ConvertFileName(src:pWideChar;dst:pWideChar;hContact:TMCONTACT=0):pWideChar; overload;
var
  pc:pWideChar;
begin
  result:=dst;
  dst^:=#0;
  if (src<>nil) and (src^<>#0) then
  begin
    if isVarsInstalled then
    begin
      pc:=ParseVarString(src,hContact);
      src:=pc;
    end
    else
      pc:=nil;
    PathToAbsoluteW(src,dst);
    mFreeMem(pc);
  end;
end;

function ConvertFileName(src:pWideChar;hContact:TMCONTACT=0):pWideChar; overload;
var
  buf1:array [0..511] of WideChar;
begin
  if (src<>nil) and (src^<>#0) then
    StrDupW(result,ConvertFileName(src,buf1,hContact))
  else
    result:=nil;
end;

function ConvertFileName(src:pAnsiChar;dst:pAnsiChar;hContact:TMCONTACT=0):pAnsiChar; overload;
var
  pc:pAnsiChar;
begin
  result:=dst;
  dst^:=#0;
  if (src<>nil) and (src^<>#0) then
  begin
    if isVarsInstalled then
    begin
      pc:=ParseVarString(src,hContact);
      src:=pc;
    end
    else
      pc:=nil;
    PathToAbsolute(src,dst);
    mFreeMem(pc);
  end;
end;

function ConvertFileName(src:pAnsiChar;hContact:TMCONTACT=0):pAnsiChar; overload;
var
  buf1:array [0..511] of AnsiChar;
begin
  if (src<>nil) and (src^<>#0) then
    StrDup(result,ConvertFileName(src,buf1,hContact))
  else
    result:=nil;
end;

const
  MirCP:integer=-1;

function MirandaCP:integer;
begin
  if MirCP<0 then
    MirCP:=CallService(MS_LANGPACK_GETCODEPAGE,0,0);
  result:=MirCP;
end;

function IsChat(hContact:TMCONTACT):bool;
begin
  result:=DBReadByte(hContact,
    PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO,hContact,0)),
    'ChatRoom',0)=1;
end;

function isVarsInstalled:bool;
{$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=ServiceExists(MS_VARS_FORMATSTRING)<>0;
end;

function ParseVarString(astr:pAnsiChar;aContact:TMCONTACT=0;extra:pAnsiChar=nil):pAnsiChar;
var
  tfi:TFORMATINFO;
  tmp,pc:pAnsiChar;
  dat:TREPLACEVARSDATA;
begin
  if (astr=nil) or (astr^=#0) then exit;

  if ServiceExists(MS_UTILS_REPLACEVARS)<>0 then
  begin
    FillChar(dat,SizeOf(TREPLACEVARSDATA),0);
    dat.cbSize :=SizeOf(TREPLACEVARSDATA);
    pc:=pAnsiChar(CallService(MS_UTILS_REPLACEVARS,wparam(astr),lparam(@dat)));
    astr:=pc;
  end
  else
    pc:=nil;

  if isVarsInstalled then
  begin
    FillChar(tfi,SizeOf(tfi),0);
    with tfi do
    begin
      cbSize       :=SizeOf(TFORMATINFO);
      szFormat.a   :=astr;
      szExtraText.a:=extra;
      hContact     :=aContact;
    end;
    tmp:=pointer(CallService(MS_VARS_FORMATSTRING,wparam(@tfi),0));
    StrDup(result,tmp);
    mir_free(tmp);
  end
  else
  begin
    StrDup(result,astr);
  end;
  mir_free(pc);
end;

function ParseVarString(astr:pWideChar;aContact:TMCONTACT=0;extra:pWideChar=nil):pWideChar;
var
  tfi:TFORMATINFO;
  tmp,pc:pWideChar;
  dat:TREPLACEVARSDATA;
begin
  if (astr=nil) or (astr^=#0) then exit;

  if ServiceExists(MS_UTILS_REPLACEVARS)<>0 then
  begin
    FillChar(dat,SizeOf(TREPLACEVARSDATA),0);
    dat.cbSize :=SizeOf(TREPLACEVARSDATA);
    dat.dwflags:=RVF_UNICODE;
    pc:=pWideChar(CallService(MS_UTILS_REPLACEVARS,wparam(astr),lparam(@dat)));
    astr:=pc;
  end
  else
    pc:=nil;

  if isVarsInstalled then
  begin
    FillChar(tfi,SizeOf(tfi),0);
    with tfi do
    begin
      cbSize       :=SizeOf(TFORMATINFO);
      flags        :=FIF_UNICODE;
      szFormat.w   :=astr;
      szExtraText.w:=extra;
      hContact     :=aContact;
    end;
    tmp:=pointer(CallService(MS_VARS_FORMATSTRING,wparam(@tfi),0));
    StrDupW(result,tmp);
    mir_free(tmp);
  end
  else
  begin
    StrDupW(result,astr);
  end;
  mir_free(pc); // forced!
end;

function ShowVarHelp(dlg:HWND;id:integer=0):integer;
var
  vhi:TVARHELPINFO;
begin
  FillChar(vhi,SizeOf(vhi),0);
  with vhi do
  begin
    cbSize:=SizeOf(vhi);
    if id=0 then
      flags:=VHF_NOINPUTDLG
    else
    begin
      flags   :=VHF_FULLDLG or VHF_SETLASTSUBJECT;
      hwndCtrl:=GetDlgItem(dlg,id);
    end;
  end;
  result:=CallService(MS_VARS_SHOWHELPEX,dlg,lparam(@vhi));
end;

procedure ShowPopupW(text:pWideChar;title:pWideChar=nil);
var
  ppdu:TPOPUPDATAW;
begin
  if ServiceExists(MS_POPUP_ADDPOPUPW)=0 then
    exit;

  FillChar(ppdu,SizeOf(TPOPUPDATAW),0);
  StrCopyW(ppdu.lpwzText,text,MAX_SECONDLINE-1);
  if title<>nil then
    StrCopyW(ppdu.lpwzContactName,title,MAX_CONTACTNAME-1)
  else
    ppdu.lpwzContactName[0]:=' ';
  CallService(MS_POPUP_ADDPOPUPW,wparam(@ppdu),APF_NO_HISTORY);
end;

function TranslateA2W(sz:PAnsiChar):PWideChar;
var
  tmp:pWideChar;
begin
  mGetMem(tmp,(StrLen(sz)+1)*SizeOf(WideChar));
  Result:=TranslateW(FastAnsiToWideBuf(sz,tmp));
  if Result<>tmp then
  begin
    StrDupW(Result,Result);
    mFreeMem(tmp);
  end;
end;

function GetContactProtoAcc(hContact:TMCONTACT):PAnsiChar;
begin
  if ServiceExists(MS_PROTO_GETCONTACTBASEACCOUNT)<>0 then
    result:=PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEACCOUNT,hContact,0))
  else
    result:=PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO,hContact,0));
end;

function IsMirandaUser(hContact:TMCONTACT):integer; // >0=Miranda; 0=Not miranda; -1=unknown
var
  sz:PAnsiChar;
begin
  sz:=PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO,hContact,0));
  sz:=DBReadString(hContact,sz,'MirVer');
  if sz<>nil then
  begin
    result:=int_ptr(StrPos(sz,'Miranda'));
    mFreeMem(sz);
  end
  else
    result:=-1;
end;

function SetCListSelContact(hContact:TMCONTACT):TMCONTACT;
var
  wnd:HWND;
begin
  wnd:=CallService(MS_CLUI_GETHWNDTREE,0,0);
  result:=hContact;
//  hContact:=SendMessage(wnd,CLM_FINDCONTACT  ,hContact,0);
  SendMessage(wnd,CLM_SELECTITEM   ,hContact,0);
//  SendMessage(wnd,CLM_ENSUREVISIBLE,hContact,0);
end;

function GetCListSelContact:TMCONTACT;
begin
  result:=SendMessageW(CallService(MS_CLUI_GETHWNDTREE,0,0),CLM_GETSELECTION,0,0);
end;

function LoadContact(group,setting:PAnsiChar):TMCONTACT;
var
  p,proto:pAnsiChar;
  section:array [0..63] of AnsiChar;
  dbv:TDBVARIANT;
  is_chat:boolean;
begin
  p:=StrCopyE(section,setting);
  StrCopy(p,opt_cproto); proto  :=DBReadString(0,group,section);
  StrCopy(p,opt_ischat); is_chat:=DBReadByte  (0,group,section,0)<>0;
  StrCopy(p,opt_cuid  );
  if is_chat then
    dbv.szVal.W:=DBReadUnicode(0,group,section,@dbv)
  else
    DBReadSetting(0,group,section,@dbv);

  result:=FindContactHandle(proto,dbv,is_chat);

  mFreeMem(proto);
  if not is_chat then
    DBFreeVariant(@dbv)
  else
    mFreeMem(dbv.szVal.W);
end;

function SaveContact(hContact:TMCONTACT;group,setting:PAnsiChar):integer;
var
  p,proto,uid:pAnsiChar;
  cws:TDBVARIANT;
  section:array [0..63] of AnsiChar;
  pw:pWideChar;
  is_chat:boolean;
begin
  result:=0;
  proto:=GetContactProtoAcc(hContact);
  if proto<>nil then
  begin
    p:=StrCopyE(section,setting);
    is_chat:=IsChat(hContact);
    if is_chat then
    begin
      pw:=DBReadUnicode(hContact,proto,'ChatRoomID');
      StrCopy(p,opt_cuid); DBWriteUnicode(0,group,section,pw);
      mFreeMem(pw);
      result:=1;
    end
    else
    begin
      uid:=pAnsiChar(CallProtoService(proto,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0));
      if uid<>pAnsiChar(CALLSERVICE_NOTFOUND) then
      begin
        if DBReadSetting(hContact,proto,uid,@cws)=0 then
        begin
          StrCopy(p,opt_cuid); DBWriteSetting(0,group,section,@cws);
          DBFreeVariant(@cws);
          result:=1;
        end;
      end;
    end;
    if result<>0 then
    begin
      StrCopy(p,opt_cproto); DBWriteString(0,group,section,proto);
      StrCopy(p,opt_ischat); DBWriteByte  (0,group,section,ord(is_chat));
    end;
  end;
end;

function WndToContact(wnd:HWND):TMCONTACT; overload;
var
  hContact:TMCONTACT;
  mwid:TMessageWindowInputData;
  mwod:TMessageWindowOutputData;
begin
  wnd:=GetParent(wnd); //!!
  hContact:=db_find_first();
  with mwid do
  begin
    cbSize:=SizeOf(mwid);
    uFlags:=MSG_WINDOW_UFLAG_MSG_BOTH;
  end;
  mwod.cbSize:=SizeOf(mwod);
  while hContact<>0 do
  begin
    mwid.hContact:=hContact;
    if CallService(MS_MSG_GETWINDOWDATA,wparam(@mwid),lparam(@mwod))=0 then
    begin
      if {((mwod.uState and MSG_WINDOW_STATE_FOCUS)<>0) and} (mwod.hwndWindow=wnd) then
      begin
        result:=mwid.hContact;
        exit;
      end
    end;
    hContact:=db_find_next(hContact);
  end;
  result:=0;
end;

function WndToContact:TMCONTACT; overload;
var
  wnd:HWND;
begin
  wnd:=GetFocus;
  if wnd=0 then
    wnd:=WaitFocusedWndChild(GetForegroundWindow);
  if wnd<>0 then
    result:=WndToContact(wnd)
  else
    result:=0;
  if result=0 then
    result:=GetCListSelContact;
end;

function GetContactStatus(hContact:TMCONTACT):integer;
var
  szProto:PAnsiChar;
begin
  szProto:=PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO,hContact,0));
  if szProto=NIL then
    result:=ID_STATUS_OFFLINE
  else
    result:=DBReadWord(hContact,szProto,'Status',ID_STATUS_OFFLINE);
end;

function CheckPath(filename,profilepath,path:PAnsiChar):PAnsiChar;
var
  buf:array [0..511] of AnsiChar;
  f:THANDLE;
  p:PAnsiChar;
begin
  result:=nil;
  if profilepath<>nil then
    StrCopy(buf,profilepath)
  else
    buf[0]:=#0;
  StrCat(buf,filename);
  f:=Reset(buf);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
  begin
    if path<>nil then
    begin
      PathToAbsolute(path,buf);
      p:=StrEnd(buf);
      if p^<>'\' then
      begin
        p^:='\';
        inc(p);
        p^:=#0;
      end;
    end
    else if profilepath=nil then
      exit
    else
      buf[0]:=#0;
    StrCat(buf,filename); //path\prefix+name
    f:=Reset(buf);
  end;
  if f<>THANDLE(INVALID_HANDLE_VALUE) then
  begin
    CloseHandle(f);
    StrDup(result,buf);
  end;
end;

function GetAddonFileName(prefix,altname,path:PAnsiChar;ext:PAnsiChar):PAnsiChar;
var
  profilepath:array [0..511] of AnsiChar;
  altfilename,filename:array [0..127] of AnsiChar;
  p:PAnsiChar;
begin
  CallService(MS_DB_GETPROFILEPATH,300,lparam(@profilepath));
  p:=StrEnd(profilepath);
  p^:='\'; inc(p);
  p^:=#0;
  filename[0]:=#0;
  altfilename[0]:=#0;
  if prefix<>nil then
  begin
    StrCopy(filename,prefix);
    p:=StrEnd(filename);
    CallService(MS_DB_GETPROFILENAME,SizeOf(filename)-integer(p-pAnsiChar(@filename)),lparam(p));
    ChangeExt(filename,ext);
    result:=CheckPath(filename,profilepath,path);
  end
  else
    result:=nil;

  if (result=nil) and (altname<>nil) then
  begin
    StrCopy(altfilename,altname);
    ChangeExt(altfilename,ext);
    result:=CheckPath(altfilename,profilepath,path);
  end;
  if result=nil then
  begin
    if filename[0]<>#0 then
      StrCat(profilepath,filename)
    else
      StrCat(profilepath,altfilename);
    StrDup(result,profilepath);
  end;
end;

procedure ShowContactDialog(hContact:TMCONTACT;DblClk:boolean=true;anystatus:boolean=true);
var
  pc:array [0..127] of AnsiChar;
begin
{
CallService(MS_CLIST_CONTACTDOUBLECLICKED,hContact,0);
}
  if (hContact<>0) and (CallService(MS_DB_CONTACT_IS,hContact,0)<>0) then
  begin
    if StrCopy(pc,PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO,hContact,0)))<>nil then
      if DblClk or (DBReadByte(hContact,pc,'ChatRoom',0)=1) then // chat room
      begin
        if not anystatus then
        begin
          StrCat(pc,PS_GETSTATUS);
          anystatus:=(CallService(pc,0,0)<>ID_STATUS_OFFLINE);
        end;
        if anystatus then
        begin
          CallService(MS_CLIST_CONTACTDOUBLECLICKED,hContact,0); //??
        // if chat exist, open chat
        // else create new session
        end;
      end
      else
      begin
        if ServiceExists(MS_MSG_CONVERS)<>0 then // Convers compat.
          CallService(MS_MSG_CONVERS,hContact,0)
        else
          CallService(MS_MSG_SENDMESSAGE,hContact,0)
      end;
  end;
end;

procedure SendChatText(pszID:pointer;pszModule:PAnsiChar;pszText:pointer);
var
  gcd:TGCDEST;
  gce:TGCEVENT;
begin
  gcd.pszModule:=pszModule;
  gcd.iType    :=GC_EVENT_SENDMESSAGE;
  gcd.szID.w   :=pszID;

  FillChar(gce,SizeOf(TGCEVENT),0);
  gce.cbSize  :=SizeOf(TGCEVENT);
  gce.pDest   :=@gcd;
  gce.bIsMe   :=true;
  gce.szText.w:=pszText;
  gce.dwFlags :=GCEF_ADDTOLOG;
  gce.time    :=GetCurrentTime;

  CallServiceSync(MS_GC_EVENT,0,lparam(@gce));
end;

procedure SendToChat(hContact:TMCONTACT;pszText:PWideChar);
var
  gci:TGC_INFO;
  pszModule:PAnsiChar;
  i,cnt:integer;
begin
  pszModule:=PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO,hContact,0));
  cnt:=CallService(MS_GC_GETSESSIONCOUNT,0,lparam(pszModule));
  i:=0;
  gci.pszModule:=pszModule;
  while i<cnt do
  begin
    gci.iItem:=i;
    gci.Flags:=GCF_BYINDEX+GCF_HCONTACT+GCF_ID;
    CallService(MS_GC_GETINFO,0,lparam(@gci));
    if gci.hContact=hContact then
    begin
      SendChatText(gci.pszID.w,pszModule,pszText);
      break;
    end;
    inc(i);
  end;
end;

function FindContactHandle(proto:pAnsiChar;const dbv:TDBVARIANT;is_chat:boolean=false):TMCONTACT;
var
  uid:pAnsiChar;
  ldbv:TDBVARIANT;
  hContact:TMCONTACT;
  pw:pWideChar;
begin
  result:=0;
  uid:=nil;
  if not is_chat then
  begin
    uid:=pAnsiChar(CallProtoService(proto,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0));
    if uid=pAnsiChar(CALLSERVICE_NOTFOUND) then exit;
  end;

  hContact:=db_find_first();
  while hContact<>0 do
  begin
    if is_chat then
    begin
      if IsChat(hContact) then
      begin
        pw:=DBReadUnicode(hContact,proto,'ChatRoomID');
        if StrCmpW(pw,dbv.szVal.W)=0 then result:=hContact;
        mFreeMem(pw);
      end
    end
    else
    begin
      if DBReadSetting(hContact,proto,uid,@ldbv)=0 then
      begin
        if dbv._type=ldbv._type then
        begin
          case dbv._type of
//            DBVT_DELETED: ;
            DBVT_BYTE   : if dbv.bVal=ldbv.bVal then result:=hContact;
            DBVT_WORD   : if dbv.wVal=ldbv.wVal then result:=hContact;
            DBVT_DWORD  : if dbv.dVal=ldbv.dVal then result:=hContact;
            DBVT_UTF8,
            DBVT_ASCIIZ : if StrCmp (dbv.szVal.A,ldbv.szVal.A)=0 then result:=hContact;
            DBVT_WCHAR  : if StrCmpW(dbv.szVal.W,ldbv.szVal.W)=0 then result:=hContact;
            DBVT_BLOB   : begin
              if dbv.cpbVal = ldbv.cpbVal then
              begin
                if CompareMem(dbv.pbVal,ldbv.pbVal,dbv.cpbVal) then
                  result:=hContact;
              end;
            end;
          end;
        end;
        DBFreeVariant(@ldbv);
      end;
    end;
    // added 2011.04.20
    if result<>0 then break;
    hContact:=db_find_next(hContact);
  end;
end;

function IsContactActive(hContact:TMCONTACT;proto:pAnsiChar=nil):integer;
var
  p:PPROTOACCOUNT;
  name: array [0..31] of AnsiChar;
begin

  if db_get_static(hContact,'Protocol','p',@name,SizeOf(name))=0 then
  begin
    result:=0;

    if ServiceExists(MS_PROTO_GETACCOUNT)<>0 then
    begin
      p:=PPROTOACCOUNT(CallService(MS_PROTO_GETACCOUNT,0,lparam(@name)));
      if p=nil then
        result:=-2 // deleted
      else if (not p^.bIsEnabled) or p^.bDynDisabled then
        result:=-1; // disabled
    end
    else
    begin
      if CallService(MS_PROTO_ISPROTOCOLLOADED,0,lparam(@name))=0 then
        result:=-1;
    end;

    if (result=0) and (DBReadByte(hContact,strCList,'Hidden',0)=0) then
    begin
      result:=255;
      if db_mc_getMeta(hContact)<>0 then
        result:=2;
      if StrCmp(
         PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO,hContact,0)),
         META_PROTO)=0 then
       result:=1;
    end;
    if proto<>nil then
      StrCopy(proto,@name);
  end
  else
  begin
    result:=-2;
    if proto<>nil then
      proto^:=#0;
  end;
end;

// Import plugin function adaptation
function CreateGroupW(name:pWideChar;hContact:TMCONTACT):integer;
var
  groupId:integer;
  groupIdStr:array [0..10] of AnsiChar;
  grbuf:array [0..127] of WideChar;
  p:pWideChar;
begin
  if (name=nil) or (name^=#0) then
  begin
    result:=0;
    exit;
  end;

  StrCopyW(@grbuf[1],name);
  grbuf[0]:=WideChar(1 or GROUPF_EXPANDED);

  // Check for duplicate & find unused id
  groupId:=0;
  repeat
    p:=DBReadUnicode(0,'CListGroups',IntToStr(groupIdStr,groupId));
    if p=nil then
      break;

    if StrCmpW(p+1,@grbuf[1])=0 then
    begin
      if hContact<>0 then
        DBWriteUnicode(hContact,strCList,clGroup,@grbuf[1]);

      mFreeMem(p);
      result:=0;
      exit;
    end;

    mFreeMem(p);
    inc(groupId);
  until false;

  DBWriteUnicode(0,'CListGroups',groupIdStr,grbuf);

  if hContact<>0 then
    DBWriteUnicode(hContact,strCList,clGroup,@grbuf[1]);

  p:=StrRScanW(grbuf,'\');
  if p<>nil then
  begin
    p^:=#0;
    CreateGroupW(grbuf+1,0);
  end;

  result:=1;
end;

function MyStrSort(para1:pointer; para2:pointer):int; cdecl;
begin
  result:=StrCmpW(pWideChar(para1),pWideChar(para2));
end;

function MakeGroupMenu(idxfrom:integer=100):HMENU;
var
  sl:TSortedList;
  i:integer;
  b:array [0..15] of AnsiChar;
  p:pWideChar;
begin
  result:=CreatePopupMenu;
  i:=0;
  AppendMenuW(result,MF_STRING,idxfrom,TranslateW('<Root Group>'));
  AppendMenuW(result,MF_SEPARATOR,0,nil);
  FillChar(sl,SizeOf(sl),0);
  sl.increment:=16;
  sl.sortFunc:=@MyStrSort;
  repeat
    p:=DBReadUnicode(0,'CListGroups',IntToStr(b,i),nil);
    if p=nil then break;
    List_InsertPtr(@sl,p+1);
    inc(i);
  until false;
  inc(idxfrom);
  for i:=0 to sl.realCount-1 do
  begin
    AppendMenuW(result,MF_STRING,idxfrom+i,pWideChar(sl.Items[i]));
    p:=pWideChar(sl.Items[i])-1;
    mFreeMem(p);
  end;
  List_Destroy(@sl);
end;

function GetNewGroupName(parent:HWND):pWideChar;
var
  mmenu:HMENU;
  i:integer;
  buf:array [0..63] of WideChar;
  pt:TPoint;
begin
  result:=nil;
  mmenu:=MakeGroupMenu(100);
  GetCursorPos(pt);
  i:=integer(TrackPopupMenu(mmenu,TPM_RETURNCMD+TPM_NONOTIFY,pt.x,pt.y,0,parent,nil));
  if i>100 then // no root or cancel
  begin
    GetMenuStringW(mmenu,i,buf,HIGH(buf)+1,MF_BYCOMMAND);
    StrDupW(result,buf);
  end;
  DestroyMenu(mmenu);
end;

function SendRequest(url:PAnsiChar;rtype:int;args:pAnsiChar=nil;hNetLib:THANDLE=0):pAnsiChar;
var
  nlu:TNETLIBUSER;
  req :TNETLIBHTTPREQUEST;
  resp:PNETLIBHTTPREQUEST;
  hTmpNetLib:THANDLE;
  nlh:array [0..1] of TNETLIBHTTPHEADER;
  buf:array [0..31] of AnsiChar;
begin
  result:=nil;

  FillChar(req,SizeOf(req),0);
  req.cbSize     :=NETLIBHTTPREQUEST_V1_SIZE;//SizeOf(req);
  req.requestType:=rtype;
  req.szUrl      :=url;
  req.flags      :=NLHRF_NODUMP or NLHRF_HTTP11;
  if args<>nil then
  begin
    nlh[0].szName :='Content-Type';
    nlh[0].szValue:='application/x-www-form-urlencoded';
    nlh[1].szName :='Content-Length';
    nlh[1].szValue:=IntToStr(buf,StrLen(args));
    req.headers     :=@nlh;
    req.headersCount:=2;
    req.pData       :=args;
    req.dataLength  :=StrLen(args);
  end;

  if hNetLib=0 then
  begin
    FillChar(nlu,SizeOf(nlu),0);
    nlu.cbSize          :=SizeOf(nlu);
    nlu.flags           :=NUF_HTTPCONNS or NUF_NOHTTPSOPTION or NUF_OUTGOING or NUF_NOOPTIONS;
    nlu.szSettingsModule:='dummy';
    hTmpNetLib:=CallService(MS_NETLIB_REGISTERUSER,0,lparam(@nlu));
  end
  else
    hTmpNetLib:=hNetLib;

  resp:=pointer(CallService(MS_NETLIB_HTTPTRANSACTION,hTmpNetLib,lparam(@req)));

  if resp<>nil then
  begin
    if resp^.resultCode=200 then
    begin
      StrDup(result,resp.pData,resp.dataLength);
    end
    else
    begin
      result:=pAnsiChar(int_ptr(resp^.resultCode and $0FFF));
    end;
    CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,lparam(resp));
  end;

  if (hNetLib=0) and (nlu.cbSize<>0) then
    CallService(MS_NETLIB_CLOSEHANDLE,hTmpNetLib,0);
end;

(*
static int __inline NLog(AnsiChar *msg) {
  return CallService(MS_NETLIB_LOG, (WPARAM)hNetlibUser, (LPARAM)msg);
}
*)
function GetFile(url:PAnsiChar;save_file:PAnsiChar;
                 hNetLib:THANDLE=0;recurse_count:integer=0):bool;
var
  nlu:TNETLIBUSER;
  req :TNETLIBHTTPREQUEST;
  resp:PNETLIBHTTPREQUEST;
  hSaveFile:THANDLE;
  i:integer;
begin
  result:=false;
  if recurse_count>MAX_REDIRECT_RECURSE then
    exit;
  if (url=nil) or (url^=#0) or (save_file=nil) or (save_file^=#0) then
    exit;

  FillChar(req,SizeOf(req),0);
  req.cbSize     :=NETLIBHTTPREQUEST_V1_SIZE;//SizeOf(req);
  req.requestType:=REQUEST_GET;
  req.szUrl      :=url;
  req.flags      :=NLHRF_NODUMP;


  FillChar(nlu,SizeOf(nlu),0);
  if hNetLib=0 then
  begin
    nlu.cbSize          :=SizeOf(nlu);
    nlu.flags           :=NUF_HTTPCONNS or NUF_NOHTTPSOPTION or NUF_OUTGOING or NUF_NOOPTIONS;
    nlu.szSettingsModule:='dummy';
    hNetLib:=CallService(MS_NETLIB_REGISTERUSER,0,lparam(@nlu));
  end;

  resp:=pointer(CallService(MS_NETLIB_HTTPTRANSACTION,hNetLib,lparam(@req)));

  if resp<>nil then
  begin
    if resp^.resultCode=200 then
    begin
      hSaveFile:=Rewrite(save_file);
      if hSaveFile<>THANDLE(INVALID_HANDLE_VALUE) then
      begin
        BlockWrite(hSaveFile,resp^.pData^,resp^.dataLength);
        CloseHandle(hSaveFile);
        result:=true;
      end
    end
    else if (resp.resultCode>=300) and (resp.resultCode<400) then
    begin
      // get new location
      for i:=0 to resp^.headersCount-1 do
      begin
        //MessageBox(0,resp^.headers[i].szValue, resp^.headers[i].szName,MB_OK);
        if StrCmp(resp^.headers^[i].szName,'Location')=0 then
        begin
          result:=GetFile(resp^.headers^[i].szValue,save_file,hNetLib,recurse_count+1);
          break;
        end
      end;
    end
    else
    begin
{
      _stprintf(buff, TranslateT("Failed to download \"%s\" - Invalid response, code %d"), plugin_name, resp->resultCode);

      ShowError(buff);
      AnsiChar *ts = GetAString(buff);
      NLog(ts);
}
    end;
    CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,lparam(resp));

    if nlu.cbSize<>0 then
      CallService(MS_NETLIB_CLOSEHANDLE,hNetLib,0);
  end;
end;

function GetFile(url:PWideChar;save_file:PWideChar;
                 hNetLib:THANDLE=0;recurse_count:integer=0):bool;
var
  aurl,asave:array [0..MAX_PATH-1] of AnsiChar;
begin
  FastWideToAnsiBuf(url,aurl);
  FastWideToAnsiBuf(save_file,asave);
  result:=GetFile(aurl,asave,hNetLib,0);
end;

function GetProxy(hNetLib:THANDLE):PAnsiChar;
var
  nlus:TNETLIBUSERSETTINGS;
  pc:PAnsiChar;
  proxy:array [0..127] of AnsiChar;
begin
  result:=nil;
  nlus.cbSize:=SizeOf(nlus);
  if CallService(MS_NETLIB_GETUSERSETTINGS,hNetLib,lparam(@nlus))<>0 then
  begin
    if nlus.useProxy<>0 then
    begin
      if nlus.proxyType<>PROXYTYPE_IE then
      begin
        pc:=@proxy;
        if nlus.szProxyServer<>nil then
        begin
          if nlus.useProxyAuth<>0 then
          begin
            if nlus.szProxyAuthUser<>nil then
            begin
              pc:=StrCopyE(proxy,nlus.szProxyAuthUser);
              if nlus.szProxyAuthPassword<>nil then
              begin
                pc^:=':'; inc(pc);
                pc:=StrCopyE(pc,nlus.szProxyAuthPassword);
              end;
              pc^:='@';
              inc(pc);
            end;
          end;
          pc:=StrCopyE(pc,nlus.szProxyServer);
          if nlus.wProxyPort<>0 then
          begin
            pc^:=':'; inc(pc);
            IntToStr(pc,nlus.wProxyPort);
          end;
        end;
        StrDup(result,proxy);
      end
      else // use IE proxy
      begin
        mGetMem(result,1);
        result^:=#0;
      end;
    end;
  end;
end;

function LoadImageURL(url:pAnsiChar;size:integer=0):HBITMAP;
var
  nlu:TNETLIBUSER;
  req :TNETLIBHTTPREQUEST;
  resp:PNETLIBHTTPREQUEST;
  hNetLib:THANDLE;
  im:TIMGSRVC_MEMIO;
begin
  result:=0;
  if (url=nil) or (url^=#0) then
    exit;

  FillChar(req,SizeOf(req),0);
  req.cbSize     :=NETLIBHTTPREQUEST_V1_SIZE;//SizeOf(req);
  req.requestType:=REQUEST_GET;
  req.szUrl      :=url;
  req.flags      :=NLHRF_NODUMP;

  FillChar(nlu,SizeOf(nlu),0);
  nlu.cbSize          :=SizeOf(nlu);
  nlu.flags           :=NUF_HTTPCONNS or NUF_NOHTTPSOPTION or NUF_OUTGOING or NUF_NOOPTIONS;
  nlu.szSettingsModule:='dummy';
  hNetLib:=CallService(MS_NETLIB_REGISTERUSER,0,lparam(@nlu));

  resp:=pointer(CallService(MS_NETLIB_HTTPTRANSACTION,hNetLib,lparam(@req)));

  if resp<>nil then
  begin
    if resp^.resultCode=200 then
    begin
      im.iLen :=resp.dataLength;
      im.pBuf :=resp.pData;
      im.flags:=size shl 16;
      im.fif  :=FIF_JPEG;
      result  :=CallService(MS_IMG_LOADFROMMEM,wparam(@im),0);
//      if result<>0 then
//        DeleteObject(SendMessage(wnd,STM_SETIMAGE,IMAGE_BITMAP,result)); //!!
    end;
    CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,lparam(resp));
  end;
  CallService(MS_NETLIB_CLOSEHANDLE,hNetLib,0);
end;

function RegisterSingleIcon(resname,ilname,descr,group:pAnsiChar):int;
var
  sid:TSKINICONDESC;
begin
  FillChar(sid,SizeOf(TSKINICONDESC),0);
  sid.cbSize     :=SizeOf(TSKINICONDESC);
  sid.cx         :=16;
  sid.cy         :=16;
  sid.flags      :=0;
  sid.szSection.a:=group;

  sid.hDefaultIcon   :=LoadImageA(hInstance,resname,IMAGE_ICON,16,16,0);
  sid.pszName        :=ilname;
  sid.szDescription.a:=descr;
  result:=Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);
end;

end.

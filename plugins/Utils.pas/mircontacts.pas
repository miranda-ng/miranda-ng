{$INCLUDE compilers.inc}
unit mircontacts;

interface

uses
  Windows,
  m_api;

//----- Contact info -----

function GetContactStatus(hContact:TMCONTACT):integer;

//----- Contact type check -----

function IsChat(hContact:TMCONTACT):bool;
function IsMirandaUser(hContact:TMCONTACT):integer; // >0=Miranda; 0=Not miranda; -1=unknown

//----- Save / Load contact -----

function LoadContact(group,setting:PAnsiChar):TMCONTACT;
function SaveContact(hContact:TMCONTACT;group,setting:PAnsiChar):integer;
function FindContactHandle(Proto:PAnsiChar;const dbv:TDBVARIANT;is_chat:boolean=false):TMCONTACT;

//----- Another functions -----

function GetCListSelContact:TMCONTACT;

function WndToContact(wnd:HWND):TMCONTACT; overload;
function WndToContact:TMCONTACT; overload;

procedure ShowContactDialog(hContact:TMCONTACT;DblClk:boolean=true;anystatus:boolean=true);

//----- List of contacts (combobox) -----

procedure FillContactList(list:HWND;filter:boolean=true;format:PWideChar=nil);
function  FindContact    (list:HWND;contact:TMCONTACT):integer;


implementation

uses
  messages,
  common, syswin, datetime,
  dbsettings;

//----- Contact info -----

function GetContactStatus(hContact:TMCONTACT):integer;
var
  szProto:PAnsiChar;
begin
  szProto:=Proto_GetBaseAccountName(hContact);
  if szProto=nil then
    result:=ID_STATUS_OFFLINE
  else
    result:=DBReadWord(hContact,szProto,'Status',ID_STATUS_OFFLINE);
end;

//----- Contact type check -----

function IsChat(hContact:TMCONTACT):bool;
begin
  result:=DBReadByte(hContact,Proto_GetBaseAccountName(hContact),'ChatRoom',0)=1;
end;

function IsMirandaUser(hContact:TMCONTACT):integer; // >0=Miranda; 0=Not miranda; -1=unknown
var
  sz:PAnsiChar;
begin
  sz:=DBReadString(hContact,Proto_GetBaseAccountName(hContact),'MirVer');
  if sz<>nil then
  begin
    result:=int_ptr(StrPos(sz,'Miranda'));
    mFreeMem(sz);
  end
  else
    result:=-1;
end;

function IsContactActive(hContact:TMCONTACT;Proto:PAnsiChar=nil):integer;
var
  p:PPROTOACCOUNT;
  name: array [0..31] of AnsiChar;
begin

  if db_get_static(hContact,'Protocol','p',@name,SizeOf(name))=0 then
  begin
    result:=0;

    p:=Proto_GetAccount(@name);
    if p=nil then
      result:=-2 // deleted
    else if (not p^.bIsEnabled) or p^.bDynDisabled then
      result:=-1; // disabled

    if (result=0) and (DBReadByte(hContact,strCList,'Hidden',0)=0) then
    begin
      result:=255;
      if db_mc_getMeta(hContact)<>0 then
        result:=2;
      if StrCmp(Proto_GetBaseAccountName(hContact),META_PROTO)=0 then
        result:=1;
    end;
    if Proto<>nil then
      StrCopy(Proto,@name);
  end
  else
  begin
    result:=-2;
    if Proto<>nil then
      Proto^:=#0;
  end;
end;

//----- Save / Load contact -----

const
  opt_cproto = 'cproto';
  opt_cuid   = 'cuid';
  opt_ischat = 'ischat';

function FindContactHandle(Proto:PAnsiChar;const dbv:TDBVARIANT;is_chat:boolean=false):TMCONTACT;
var
  uid:PAnsiChar;
  ldbv:TDBVARIANT;
  hContact:TMCONTACT;
  pw:PWideChar;
begin
  result:=0;
  uid:=nil;
  if not is_chat then
  begin
    uid:=Proto_GetUniqueId(Proto);
    if uid=nil then exit;
  end;

  hContact:=db_find_first();
  while hContact<>0 do
  begin
    if is_chat then
    begin
      if IsChat(hContact) then
      begin
        pw:=DBReadUnicode(hContact,Proto,'ChatRoomID');
        if StrCmpW(pw,dbv.szVal.W)=0 then result:=hContact;
        mFreeMem(pw);
      end
    end
    else
    begin
      if DBReadSetting(hContact,Proto,uid,@ldbv)=0 then
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
        db_free(@ldbv);
      end;
    end;
    // added 2011.04.20
    if result<>0 then break;
    hContact:=db_find_next(hContact);
  end;
end;

function LoadContact(group,setting:PAnsiChar):TMCONTACT;
var
  p,Proto:PAnsiChar;
  section:array [0..63] of AnsiChar;
  dbv:TDBVARIANT;
  is_chat:boolean;
begin
  p:=StrCopyE(section,setting);
  StrCopy(p,opt_cproto); Proto  :=DBReadString(0,group,section);
  StrCopy(p,opt_ischat); is_chat:=DBReadByte  (0,group,section,0)<>0;
  StrCopy(p,opt_cuid  );
  if is_chat then
    dbv.szVal.W:=DBReadUnicode(0,group,section,@dbv)
  else
    DBReadSetting(0,group,section,@dbv);

  result:=FindContactHandle(Proto,dbv,is_chat);

  mFreeMem(Proto);
  if not is_chat then
    db_free(@dbv)
  else
    mFreeMem(dbv.szVal.W);
end;

function SaveContact(hContact:TMCONTACT;group,setting:PAnsiChar):integer;
var
  p,Proto,uid:PAnsiChar;
  cws:TDBVARIANT;
  section:array [0..63] of AnsiChar;
  pw:PWideChar;
  is_chat:boolean;
begin
  result:=0;
  Proto:=Proto_GetBaseAccountName(hContact);
  if Proto<>nil then
  begin
    p:=StrCopyE(section,setting);
    is_chat:=IsChat(hContact);
    if is_chat then
    begin
      pw:=DBReadUnicode(hContact,Proto,'ChatRoomID');
      StrCopy(p,opt_cuid); DBWriteUnicode(0,group,section,pw);
      mFreeMem(pw);
      result:=1;
    end
    else
    begin
      uid:=Proto_GetUniqueId(Proto);
      if uid<>nil then
      begin
        if DBReadSetting(hContact,Proto,uid,@cws)=0 then
        begin
          StrCopy(p,opt_cuid); DBWriteSetting(0,group,section,@cws);
          db_free(@cws);
          result:=1;
        end;
      end;
    end;
    if result<>0 then
    begin
      StrCopy(p,opt_cproto); DBWriteString(0,group,section,Proto);
      StrCopy(p,opt_ischat); DBWriteByte  (0,group,section,ord(is_chat));
    end;
  end;
end;

//----- Another functions -----

function GetCListSelContact:TMCONTACT;
begin
  result:=SendMessageW(cli^.hwndContactTree,CLM_GETSELECTION,0,0);
end;

function WndToContact(wnd:HWND):TMCONTACT;
var
  hContact:TMCONTACT;
  mwod:TMessageWindowData;
begin
  wnd:=GetParent(wnd);
  hContact:=db_find_first();

  while hContact<>0 do
  begin
    if Srmm_GetWindowData(hContact,@mwod)=0 then
    begin
      if mwod.hwndWindow=wnd then
      begin
        result:=hContact;
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

procedure ShowContactDialog(hContact:TMCONTACT;DblClk:boolean=true;anystatus:boolean=true);
var
  pc:array [0..127] of AnsiChar;
begin
  if (hContact<>0) and (db_is_contact(hContact)<>0) then
  begin
    if StrCopy(pc,Proto_GetBaseAccountName(hContact))<>nil then
      if DblClk or (DBReadByte(hContact,pc,'ChatRoom',0)=1) then // chat room
      begin
        if not anystatus then
        begin
          anystatus:=(Proto_GetStatus(pc)<>ID_STATUS_OFFLINE);
        end;
        if anystatus then
        begin
          Clist_ContactDoubleClicked(hContact);
        // if chat exist, open chat
        // else create new session
        end;
      end
      else
        CallService(MS_MSG_SENDMESSAGE,hContact,0);
  end;
end;

//----- List of contacts -----

const
  defformat = '%name% - %uid% (%account%:%group%)';

procedure FillContactList(list:HWND; filter:boolean=true;format:PWideChar=nil);
var
  hContact:TMCONTACT;
  buf:array [0..511] of WideChar;
  buf1:array [0..63] of WideChar;
  p:PWideChar;
  uid:PAnsiChar;
  ldbv:TDBVARIANT;
  acc:PAnsiChar;
  lName,
  lGroup,
  lAccount,
  lUID:boolean;
begin
  if format=nil then format:=defformat;

  SendMessage(list,CB_RESETCONTENT,0,0);
  hContact:=db_find_first();

  lName   :=StrPosW(format,'%name%')<>nil;
  lGroup  :=StrPosW(format,'%group%')<>nil;
  lAccount:=StrPosW(format,'%account%')<>nil;
  lUID    :=StrPosW(format,'%uid%')<>nil;

  while hContact<>0 do
  begin
    if ((not filter) and ((IsContactActive(hContact)+1)>=0)) or // + disabled (not deleted)
            (filter  and  (IsContactActive(hContact)   >=0)) then
    begin
      StrCopyW(buf,format);
      if lName then
        StrReplaceW(buf,'%name%', Clist_GetContactDisplayName(hContact,0));

      if lGroup then
      begin
        p:=DBReadUnicode(hContact,strCList,'Group',nil);
        StrReplaceW(buf,'%group%',p);
        mFreeMem(p);
      end;

      if lAccount then
      begin
        acc:=Proto_GetBaseAccountName(hContact);
        StrReplaceW(buf,'%account%',FastAnsiToWideBuf(acc,buf1));
      end
      else
        acc:=nil;

      if lUID then
      begin
        if acc=nil then
          acc:=Proto_GetBaseAccountName(hContact);
        if IsChat(hContact) then
        begin
          p:=DBReadUnicode(hContact,acc,'ChatRoomID');
          StrReplaceW(buf,'%uid%',p);
          mFreeMem(p);
        end
        else
        begin
          uid:=Proto_GetUniqueId(acc);
          if uid<>nil then
          begin
            if DBReadSetting(hContact,acc,uid,@ldbv)=0 then
            begin
              case ldbv._type of
                DBVT_DELETED: p:='[deleted]';
                DBVT_BYTE   : p:=IntToStr(buf1,ldbv.bVal);
                DBVT_WORD   : p:=IntToStr(buf1,ldbv.wVal);
                DBVT_DWORD  : p:=IntToStr(buf1,ldbv.dVal);
                DBVT_UTF8   : UTF8ToWide(ldbv.szVal.A,p);
                DBVT_ASCIIZ : AnsiToWide(ldbv.szVal.A,p,Langpack_GetDefaultCodePage);
                DBVT_WCHAR  : p:=ldbv.szVal.W;
                DBVT_BLOB   : p:='blob';
              end;
              StrReplaceW(buf,'%uid%',p);
              if ldbv._type in [DBVT_UTF8,DBVT_ASCIIZ] then
                mFreeMem(p);
              db_free(@ldbv);
            end;
          end;
          StrReplaceW(buf,'%uid%',nil);
        end;
      end;

      SendMessage(list,CB_SETITEMDATA,
          SendMessageW(list,CB_ADDSTRING,0,tlparam(@buf)),
          hContact);
    end;
    hContact:=db_find_next(hContact);
  end;
end;

function FindContact(list:HWND;contact:TMCONTACT):integer;
var
  j:integer;
begin
  result:=0;
  j:=SendMessage(list,CB_GETCOUNT,0,0);
  while j>0 do
  begin
    dec(j);
    if TMCONTACT(SendMessage(list,CB_GETITEMDATA,j,0))=contact then
    begin
      result:=j;
      break;
    end;
  end;
end;


end.

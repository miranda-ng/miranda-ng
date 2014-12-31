{$Include compilers.inc}
unit mirevents;

interface

uses
  Windows,
  m_api;

type
  TBaseEventType = (
    mtUnknown,
    mtMessage, mtUrl, mtFile, mtSystem, mtContacts, mtStatus,
    mtOther);

  PMessageTypes = ^TMessageTypes;
  TMessageTypes = set of TBaseEventType;

const
  BaseEventNames: array[TBaseEventType] of PAnsiChar = (
    'Unknown',
    'Message',               // SKINICON_EVENT_MESSAGE
    'Link',                  // SKINICON_EVENT_URL
    'File transfer',         // SKINICON_EVENT_FILE
    'System message',        // SKINICON_OTHER_MIRANDA, SKINICON_OTHER_MIRANDAWEB, 
    'Contacts',              // SKINICON_OTHER_ADDCONTACT, SKINICON_OTHER_USERDETAILS
    'Status changes',        // SKINICON_OTHER_STATUS, SKINICON_STATUS_* (MS_SKIN_LOADPROTOICON)
    'Other events (unknown)'
  );

//----- Event info -----

procedure GetEventInfo     (hDBEvent: THANDLE; var EventInfo: TDBEventInfo);
function  GetEventTimestamp(hDBEvent: THANDLE): DWord;
function  GetEventDateTime (hDBEvent: THANDLE): TDateTime;
function  GetEventCoreText (const EventInfo: TDBEventInfo; cp: integer = CP_ACP): PWideChar;

//----- Event check -----

function IsIncomingEvent(const EventInfo: TDBEventInfo):boolean; overload;
function IsIncomingEvent(hDBEvent: THANDLE):boolean; overload;
function IsOutgoingEvent(const EventInfo: TDBEventInfo):boolean; overload;
function IsOutgoingEvent(hDBEvent: THANDLE):boolean; overload;
function IsReadedEvent  (const EventInfo: TDBEventInfo):boolean; overload;
function IsReadedEvent  (hDBEvent: THANDLE):boolean; overload;

function GetEventBaseType(EventInfo: TDBEventInfo): TBaseEventType; overload;
function GetEventBaseType(hDBEvent : THANDLE     ): TBaseEventType; overload;

//----- Custom events processing -----

//procedure ReadEvent          (hDBEvent: THANDLE; var EventInfo: TDBEventInfo; UseCP: Cardinal = CP_ACP);
//function  GetEventName(const Hi: THistoryItem):PAnsiChar;

function GetEventText(hDBEvent: THANDLE            ; custom:boolean; cp:integer=CP_ACP):PWideChar; overload;
function GetEventText(const EventInfo: TDBEventInfo; custom:boolean; cp:integer=CP_ACP):PWideChar; overload;


implementation

uses
  common,
  datetime;

//----- Event info -----

procedure GetEventInfo(hDBEvent: THANDLE; var EventInfo: TDBEventInfo);
var
  BlobSize: integer;
begin
  ZeroMemory(@EventInfo, SizeOf(EventInfo));
  EventInfo.cbSize := SizeOf(EventInfo);
  BlobSize := db_event_getBlobSize(hDBEvent);
  if BlobSize > 0 then
  begin
    mGetMem(EventInfo.pBlob,BlobSize+2); // cheat, for possible crash avoid
  end
  else
    BlobSize := 0;
  EventInfo.cbBlob := BlobSize;
  if db_event_get(hDBEvent, @EventInfo) = 0 then
  begin
    EventInfo.cbBlob := BlobSize;
    if BlobSize > 0 then
    begin
      PAnsiChar(EventInfo.pBlob)[BlobSize  ]:=#0;
      PAnsiChar(EventInfo.pBlob)[BlobSize+1]:=#0;
    end;
  end
  else
    EventInfo.cbBlob := 0;
end;

function GetEventCoreText(const EventInfo: TDBEventInfo; cp: integer = CP_ACP): PWideChar;
var
  dbegt: TDBEVENTGETTEXT;
  msg: PWideChar;
begin
  dbegt.dbei     := @EventInfo;
  dbegt.datatype := DBVT_WCHAR;
  dbegt.codepage := cp;

  msg := PWideChar(CallService(MS_DB_EVENT_GETTEXT,0,LPARAM(@dbegt)));

  result := AdjustLineBreaks(msg);
  result := rtrimw(result);

  mir_free(msg);
end;

//----- Info functions (no blob required) -----
var
  RecentEvent: THANDLE = 0;
  RecentEventInfo: TDBEventInfo;

procedure CheckRecent(hDBEvent: THANDLE);
begin
  if RecentEvent <> hDBEvent then
  begin
    ZeroMemory(@RecentEventInfo, SizeOf(RecentEventInfo));
    RecentEventInfo.cbSize := SizeOf(RecentEventInfo);
    RecentEventInfo.cbBlob := 0;
    db_event_get(hDBEvent, @RecentEventInfo);
    RecentEvent := hDBEvent;
  end;
end;

function GetEventTimestamp(hDBEvent: THANDLE): DWord;
begin
  CheckRecent(hDBEvent);
  Result := RecentEventInfo.timestamp;
end;

function GetEventDateTime(hDBEvent: THANDLE): TDateTime;
begin
  Result := TimestampToDateTime(GetEventTimestamp(hDBEvent));
end;

//----- Event check -----

function IsIncomingEvent(const EventInfo: TDBEventInfo):boolean;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  Result:=(EventInfo.flags and DBEF_SENT) = 0
end;

function IsIncomingEvent(hDBEvent: THANDLE):boolean;
begin
  CheckRecent(hDBEvent);
  Result:=(RecentEventInfo.flags and DBEF_SENT) = 0
end;

function IsOutgoingEvent(const EventInfo: TDBEventInfo):boolean;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=(EventInfo.flags and DBEF_SENT) <> 0;
end;

function IsOutgoingEvent(hDBEvent: THANDLE):boolean;
begin
  CheckRecent(hDBEvent);
  result:=(RecentEventInfo.flags and DBEF_SENT) <> 0;
end;

function IsReadedEvent(const EventInfo: TDBEventInfo):boolean;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=(EventInfo.flags and DBEF_READ) <> 0;
end;

function IsReadedEvent(hDBEvent: THANDLE):boolean;
begin
  CheckRecent(hDBEvent);
  result:=(RecentEventInfo.flags and DBEF_READ) <> 0;
end;

//----- Not pure miranda functions -----

type
  TEventTableItem = record
    EventType   : Word;
    MessageType : TBaseEventType;
  end;

var
  BuiltinEventTable: array[0..6] of TEventTableItem = (
    // must be the first item in array for unknown events
    (EventType: MaxWord;               MessageType: mtOther),
    // events definitions
    (EventType: EVENTTYPE_MESSAGE;     MessageType: mtMessage),
    (EventType: EVENTTYPE_FILE;        MessageType: mtFile),
    (EventType: EVENTTYPE_URL;         MessageType: mtUrl),
    (EventType: EVENTTYPE_AUTHREQUEST; MessageType: mtSystem),
    (EventType: EVENTTYPE_ADDED;       MessageType: mtSystem),
    (EventType: EVENTTYPE_CONTACTS;    MessageType: mtContacts)
  );

function GetEventBaseType(EventInfo: TDBEventInfo): TBaseEventType;
var
  i: Integer;
  EventIndex: Integer;
begin
  EventIndex := 0;

  if EventInfo.szModule = nil then
  begin
    for i := 1 to High(BuiltinEventTable) do
      if BuiltinEventTable[i].EventType = EventInfo.EventType then
      begin
        EventIndex := i;
        break;
      end;
  end;

  Result := BuiltinEventTable[EventIndex].MessageType;
end;

function GetEventBaseType(hDBEvent: THANDLE): TBaseEventType;
begin
  CheckRecent(hDBEvent);
  Result := GetEventBaseType(RecentEventInfo);
end;

//----- Custom events processing -----


function _WideCharType(WC: WideChar; dwInfoType: Cardinal): Word;
begin
  GetStringTypeExW(GetThreadLocale, dwInfoType, PWideChar(@WC), 1, Result)
end;

function IsWideCharAlphaNumeric(WC: WideChar): Boolean;
begin
  Result := (_WideCharType(WC, CT_CTYPE1) and (C1_ALPHA + C1_DIGIT)) <> 0;
end;

const
  UrlPrefix: array[0..1] of PWideChar = (
    'www.',
    'ftp.');

const
  UrlProto: array[0..12] of record
      Proto: PWideChar;
      Idn  : Boolean;
    end = (
    (Proto: 'http:/';     Idn: True;),
    (Proto: 'ftp:/';      Idn: True;),
    (Proto: 'file:/';     Idn: False;),
    (Proto: 'mailto:/';   Idn: False;),
    (Proto: 'https:/';    Idn: True;),
    (Proto: 'gopher:/';   Idn: False;),
    (Proto: 'nntp:/';     Idn: False;),
    (Proto: 'prospero:/'; Idn: False;),
    (Proto: 'telnet:/';   Idn: False;),
    (Proto: 'news:/';     Idn: False;),
    (Proto: 'wais:/';     Idn: False;),
    (Proto: 'outlook:/';  Idn: False;),
    (Proto: 'callto:/';   Idn: False;));

function TextHasUrls(text: PWideChar): Boolean;
var
  i,len: Integer;
  buf,pPos: PWideChar;
begin
  Result := False;
  len := StrLenW(text);
  if len=0 then exit;

  // search in URL Prefixes like "www"
  // make Case-insensitive??

  for i := 0 to High(UrlPrefix) do
  begin
    pPos := StrPosW(text, UrlPrefix[i]);
    if not Assigned(pPos) then
      continue;
    Result := ((uint_ptr(pPos) = uint_ptr(text)) or not IsWideCharAlphaNumeric((pPos - 1)^)) and
      IsWideCharAlphaNumeric((pPos + StrLenW(UrlPrefix[i]))^);
    if Result then
      exit;
  end;

  // search in url protos like "http:/"

  if StrPosW(text,':/') = nil then exit;

  StrDupW(buf,text);

  CharLowerBuffW(buf,len);
  for i := 0 to High(UrlProto) do
  begin
    pPos := StrPosW(buf, UrlProto[i].proto);
    if not Assigned(pPos) then
      continue;
    Result := ((uint_ptr(pPos) = uint_ptr(buf)) or
      not IsWideCharAlphaNumeric((pPos - 1)^));
    if Result then
      break;
  end;
  mFreeMem(buf);
end;


// reads event from hDbEvent handle
// reads all THistoryItem fields
// *EXCEPT* Proto field. Fill it manually, plz
(*
procedure ReadEvent(hDBEvent: THANDLE; var hi: THistoryItem; UseCP: Cardinal = CP_ACP);
var
  EventInfo: TDBEventInfo;
  EventIndex: integer;
  Handled: Boolean;
begin
  ZeroMemory(@hi,SizeOf(hi));
  hi.Height := -1;
  GetEventInfo(hDBEvent, EventInfo);

  hi.Module      := EventInfo.szModule;                     {*}
  hi.proto       := nil;
  hi.Time        := EventInfo.Timestamp;                    {*}
  hi.IsRead      := Boolean(EventInfo.flags and DBEF_READ); {*}
  hi.MessageType := GetMessageType(EventInfo, EventIndex);  {!}
  hi.CodePage    := UseCP;                                  {?}
  // enable autoRTL feature
  if Boolean(EventInfo.flags and DBEF_RTL) then
    hi.RTLMode := hppRTLEnable;                             {*}

  hi.Text := GetEventCoreText(EventInfo, UseCP);
{!!
  if hi.Text = nil then
    EventTable[EventIndex].TextFunction(EventInfo, hi);
}
  hi.Text := AdjustLineBreaks(hi.Text);
  hi.Text := rtrimw(hi.Text);

  if hi.MessageType.code=mtMessage then
    if TextHasUrls(hi.Text) then
    begin
      hi.MessageType.code:=mtUrl;
    end;

  mFreeMem(EventInfo.pBlob);
end;
*)
(*
function GetEventName(const Hi: THistoryItem):PAnsiChar;
var
  MesType: THppMessageType;
  mt: TBuiltinMessageType;
  etd: PDBEVENTTYPEDESCR;
begin
  MesType := Hi.MessageType;
  for mt := Low(BuiltinEventNames) to High(BuiltinEventNames) do
  begin
    if MesType.code = mt then
    begin
      Result := BuiltinEventNames[mt];
      exit;
    end;
  end;

  etd := Pointer(CallService(MS_DB_EVENT_GETTYPE, WPARAM(Hi.Module), LPARAM(Hi.MessageType.event)));
  if etd = nil then
  begin
    Result := BuiltinEventNames[mtOther];
  end
  else
    Result := etd.descr;

end;
*)

type
  TCustomEvent = record
    Module      : PAnsiChar;
    EventType   : Word;
    MessageType : TBaseEventType;
  end;
const
  CustomEventTable: array [0..4] of TCustomEvent = (
    (Module:'WATrack'        ; EventType:EVENTTYPE_WAT_REQUEST  {; MessageType:}),
    (Module:'WATrack'        ; EventType:EVENTTYPE_WAT_ANSWER   {; MessageType:}),
    (Module:'WATrack'        ; EventType:EVENTTYPE_WAT_MESSAGE  {; MessageType:}),
    (Module:'NewStatusNotify'; EventType:EVENTTYPE_STATUSCHANGE {; MessageType:}),
    (Module:'Nudge'          ; EventType:1                      {; MessageType:})
//    (Module:nil; EventType:EVENTTYPE_AVATAR_CHANGE     {; MessageType:}),
//    (Module:nil; EventType:ICQEVENTTYPE_MISSEDMESSAGE  {; MessageType:}), ICQ_DB_GETEVENTTEXT_MISSEDMESSAGE
//    (Module:nil; EventType:ICQEVENTTYPE_EMAILEXPRESS   {; MessageType:}),
//    (Module:nil; EventType:ICQEVENTTYPE_WEBPAGER       {; MessageType:}),
//    (Module:nil; EventType:EVENTTYPE_SMS               {; MessageType:}),
//    (Module:nil; EventType:EVENTTYPE_SMSCONFIRMATION   {; MessageType:}),
//    (Module:nil; EventType:TWITTER_DB_EVENT_TYPE_TWEET {; MessageType:}),
  );

//----- Support functions -----

//----- Custom standard event text -----

function GetEventTextForUrl(const EventInfo: TDBEventInfo):PWideChar;
var
  pc:PAnsiChar;
  len,lend,lenf:integer;

  url,desc: PAnsiChar;
  urlw: PWideChar;
//  cp: Cardinal;
begin
  //blob is: URL(ASCII) or URL(ASCIIZ),description(ASCIIZ)
  len := StrLen(PAnsiChar(EventInfo.pBlob));
  if (integer(EventInfo.cbBlob)-len)>2 then // possible have description
  begin
    desc := PAnsiChar(EventInfo.pBlob) + len + 1;
    lend := StrLen(desc);
  end
  else
  begin
    desc := nil;
    lend := 0;
  end;

  lenf := len;
  if lend > 0 then
    inc(lenf, lend + 2 + 1); // #13#10 + #0
  mGetMem(url, lenf);
  pc := StrCopyE(url, PAnsiChar(EventInfo.pBlob));
  if lend > 0 then
  begin
    pc^ := #13; inc(pc);
    pc^ := #10; inc(pc);
    StrCopy(pc,desc);
  end;

  if (EventInfo.flags and DBEF_UTF)<>0 then
    UTF8ToWide(url,urlw)
  else
    AnsiToWide(url,urlw); // with proper codepage must be
  mFreeMem(url);
  result := FormatStrW('URL: %s', [urlw]);
  mFreeMem(urlw);
//  hi.Extended := PAnsiChar(EventInfo.pBlob);
end;

function GetEventTextForFile(const EventInfo: TDBEventInfo):PWideChar;
var
  pc,filea,fname,desc:PAnsiChar;
  format, filew:PWideChar;
  len:integer;
//  cp: Cardinal;
begin
  //blob is: sequenceid(DWORD),filename(ASCIIZ),description(ASCIIZ)
  fname:=PAnsiChar(EventInfo.pBlob) + SizeOf(DWORD);
  len  :=StrLen(fname);
  desc :=fname + len + 1;
  if desc^ <> #0 then
    inc(len, 2 + StrLen(desc)); // +#13#10
  mGetMem(filea, len + 1);      // +#0
  pc := StrCopyE(filea, fname);
  if desc^ <> #0 then
  begin
    pc^ := #13; inc(pc);
    pc^ := #10; inc(pc);
    StrCopy(pc,desc);
  end;

  if (EventInfo.flags and DBEF_SENT) <> 0 then
    format := 'Outgoing file transfer: %s'
  else
    format := 'Incoming file transfer: %s';

  if (EventInfo.flags and DBEF_UTF)<>0 then
    UTF8ToWide(filea,filew)
  else
    AnsiToWide(filea,filew); // with proper codepage must be
  mFreeMem(filea);
  result := FormatStrW(format, [filew]);
  mFreeMem(filew);
//  Hi.Extended := PAnsiChar(EventInfo.pBlob) + SizeOf(DWORD);
end;


function GetEventText(const EventInfo: TDBEventInfo; custom:boolean; cp:integer=CP_ACP):PWideChar;
begin
  result:=nil;
  if not custom then
  begin
    result:=GetEventCoreText(EventInfo);
    // ok if registered with text service
    // ok if have text in blob
  end;
  if (result = nil) or custom then
  begin
  end;
end;

function GetEventText(hDBEvent: THANDLE; custom:boolean; cp:integer=CP_ACP):PWideChar;
var
  EventInfo: TDBEventInfo;
begin
   GetEventInfo(hDBEvent, EventInfo);
   result:=GetEventText(EventInfo, custom);
end;

function GetStandardEventIcon(const EventInfo: TDBEventInfo):HICON;
var
  idx:integer;
begin
  case GetEventBaseType(EventInfo) of
    mtMessage : idx:=SKINICON_EVENT_MESSAGE;
    mtUrl     : idx:=SKINICON_EVENT_URL;
    mtFile    : idx:=SKINICON_EVENT_FILE;
//    mtSystem  : idx:=;
//    mtContacts: idx:=;
    mtStatus  : begin
      result:=0;
      exit;
    end;
  else
    idx:=0;
  end;
  result:=CallService(MS_SKIN_LOADICON,idx,0);
{
  case EventInfo.eventType of
    EVENTTYPE_MESSAGE:     idx:=SKINICON_EVENT_MESSAGE;
    EVENTTYPE_FILE:        idx:=SKINICON_EVENT_FILE;
    EVENTTYPE_URL:         idx:=SKINICON_EVENT_URL;
    EVENTTYPE_AUTHREQUEST: idx:=SKINICON_AUTH_REQUEST;
    EVENTTYPE_ADDED:       idx:=SKINICON_AUTH_ADD;
    EVENTTYPE_CONTACTS:    idx:=
    EVENTTYPE_SMS:         idx:=SKINICON_OTHER_SMS;
  else
  end;
}
end;

end.

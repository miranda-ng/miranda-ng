(*
    History++ plugin for Miranda IM: the free IM client for Microsoft* Windows*

    Copyright (C) 2006-2009 theMIROn, 2003-2006 Art Fedorov.
    History+ parts (C) 2001 Christian Kastner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*)

{-----------------------------------------------------------------------------
 hpp_bookmarks.pas (historypp project)

 Version:   1.5
 Created:   02.04.2006
 Author:    Oxygen

 [ Description ]

 Hello, this is dummy text


 [ History ]

 1.5 (02.04.2006)
   First version

 [ Modifications ]
 none

 [ Known Issues ]
 none

 Contributors: theMIROn, Art Fedorov
-----------------------------------------------------------------------------}

unit hpp_bookmarks;

interface

uses windows, m_api;

type
  TEventData = record
    hDBEvent: DWord;
    CRC32: DWord;
    Timestamp: Cardinal;
  end;
  PEventData = ^TEventData;

  TBadEventData = record
    hDBEvent: THandle;
    CRC32: DWord;
    Timestamp: Cardinal;
  end;
  PBadEventData = ^TBadEventData;

  TBookmarksHash = class;

  TContactBookmarks = class(TObject)
  private
    Bookmarks: TBookmarksHash;
    FContact: TMCONTACT;
    FContactCP: Cardinal;
    function GetBookmarked(Index: THandle): Boolean;
    procedure SetBookmarked(Index: THandle; const Value: Boolean);
    function GetBookmarkName(Index: THandle): String;
    procedure SetBookmarkName(Index: THandle; const Value: String);
    procedure LoadBookmarks(hContact:TMCONTACT);
    procedure SaveBookmarks;
    procedure DeleteBookmarks;
    function GetCount: Integer;
    function GetItems(Index: Integer): THandle;
    function GetNames(Index: Integer): String;
  public
    constructor Create(AContact: THandle);
    destructor Destroy; override;
    procedure Clear;
    property Bookmarked[Index: THandle]: Boolean read GetBookmarked write SetBookmarked;
    property BookmarkName[Index: THandle]: String read GetBookmarkName write SetBookmarkName;
    property Items[Index: Integer]: THandle read GetItems;
    property Names[Index: Integer]: String read GetNames;
    property Count: Integer read GetCount;
    property Contact: TMCONTACT read FContact;
    property ContactCP: Cardinal read FContactCP;
  end;
  PContactBookmarks = ^TContactBookmarks;

  TPseudoHashEntry = record
    Key: uint_ptr;
    Value: pointer;
  end;
  PPseudoHashEntry = ^TPseudoHashEntry;

  TPseudoHash = class(TObject)
  private
    Table: array of TPseudoHashEntry;
    procedure RemoveByIndex(Index: Integer);
//    procedure InsertByIndex(Index: Integer; Key:Cardinal;Value: pointer);
  protected
    function AddKey(Key:uint_ptr; Value: pointer): Boolean;
    function GetKey(Key:uint_ptr; var Value: pointer): Boolean;
    function RemoveKey(Key: uint_ptr): Boolean;
  public
    destructor Destroy; override;
  end;

  TContactsHash = class(TPseudoHash)
  private
    function GetContactBookmarks(Index: THandle): TContactBookmarks;
  public
    property Items[Index: THandle]: TContactBookmarks read GetContactBookmarks; default;
    function RemoveItem(Index: THandle): Boolean;
    destructor Destroy; override;
  end;

  TBookmarksHash = class(TPseudoHash)
  private
    Contact: TContactBookmarks;
    function GetHasItem(Index: THandle): Boolean;
//    function GetBookmark(hDBEvent: THandle; var EventData: TEventData): Boolean;
    function AddItem(hDBEvent: THandle): Boolean;
    function RemoveItem(hDBEvent: THandle): Boolean;
    function AddItemName(hDBEvent: THandle; const Value: String): Boolean;
    function GetItemName(hDBEvent: THandle): String;
    function RemoveItemName(hDBEvent: THandle): Boolean;
    function FindEventByTimestampAndCrc(ped: PEventData): Boolean;
  public
    constructor Create(AContact: TContactBookmarks);
    destructor Destroy; override;
    function Clear: Integer;
    function AddEventData(var EventData: TEventData): Boolean;
    property HasItem[Index: THandle]: Boolean read GetHasItem; default;
  end;

  TBookmarkServer = class(TObject)
  private
    hookContactDeleted,
    hookEventDeleted,
    hookEventAdded: THandle;
    CachedContacts: TContactsHash;
    function GetContacts(Index: THandle): TContactBookmarks;
  protected
    procedure ContactDeleted(hContact: THandle);
    procedure EventDeleted(hContact,hDBEvent: THandle);
    procedure EventAdded(hContact,hDBEvent: THandle);
  public
    constructor Create;
    destructor Destroy; override;
    property Contacts[Index: THandle]: TContactBookmarks read GetContacts; default;
  end;

var
  BookmarkServer: TBookmarkServer;

procedure hppInitBookmarkServer;
procedure hppDeinitBookmarkServer;

implementation

uses
  SysUtils,
  hpp_jclSysUtils, hpp_events, hpp_contacts, hpp_global, Checksum, hpp_database, hpp_forms;

procedure hppInitBookmarkServer;
begin
  BookmarkServer := TBookmarkServer.Create;
end;

procedure hppDeinitBookmarkServer;
begin
  BookmarkServer.Free;
end;

function ContactDeletedHelper(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
// wParam: hContact, lParam: 0
begin
  if Assigned(BookmarkServer) then
    BookmarkServer.ContactDeleted(wParam);
  Result := 0;
end;

function EventDeletedHelper(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
// wParam: hContact, lParam: hDBEvent
begin
  if Assigned(BookmarkServer) then
    BookmarkServer.EventDeleted(wParam,lParam);
  Result := 0;
end;

function EventAddedHelper(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
// wParam: hContact, lParam: hDBEvent
begin
  if Assigned(BookmarkServer) then
    BookmarkServer.EventAdded(wParam,lParam);
  Result := 0;
end;

function DynArrayComparePseudoHash(Item1, Item2: Pointer): Integer;
begin
  {$OVERFLOWCHECKS ON}
  Result := PInteger(@PPseudoHashEntry(Item1)^.Key)^ - PInteger(@PPseudoHashEntry(Item2)^.Key)^;
  {$OVERFLOWCHECKS OFF}
end;

{ TBookmarkServer }

function TBookmarkServer.GetContacts(Index: THandle): TContactBookmarks;
begin
  Result := CachedContacts[Index];
end;

constructor TBookmarkServer.Create;
begin
  inherited;
  CachedContacts := TContactsHash.Create;
  hookContactDeleted := HookEvent(ME_DB_CONTACT_DELETED,ContactDeletedHelper);
  hookEventDeleted   := HookEvent(ME_DB_EVENT_DELETED,EventDeletedHelper);
  hookEventAdded     := HookEvent(ME_DB_EVENT_ADDED,EventAddedHelper);
end;

destructor TBookmarkServer.Destroy;
begin
  UnhookEvent(hookContactDeleted);
  UnhookEvent(hookEventDeleted);
  UnhookEvent(hookEventAdded);
  CachedContacts.Free;
  BookmarkServer := nil;
  inherited;
end;

procedure TBookmarkServer.ContactDeleted(hContact: THandle);
begin
  // do we really need to delete bookmarks from contact,
  // if he is about to be deleted? I think don't
  //Contacts[hContact].DeleteBookmarks;
  CachedContacts.RemoveItem(hContact);
end;

procedure TBookmarkServer.EventDeleted(hContact, hDBEvent: THandle);
begin
  Contacts[hContact].Bookmarked[hDBEvent] := false;
end;

procedure TBookmarkServer.EventAdded(hContact, hDBEvent: THandle);
begin
  ;
end;

{ TContactBookmarks }

constructor TContactBookmarks.Create(AContact: THandle);
var
  i, nSub : integer;
begin
  FContact := AContact;
  FContactCP := GetContactCodepage(FContact);
  Bookmarks := TBookmarksHash.Create(Self);
  // read bookmarks from DB here
  LoadBookmarks(FContact);
  if db_mc_isMeta(FContact) then
  begin
    nSub := db_mc_getSubCount(FContact);
    for i := 0 to nSub-1 do
      LoadBookmarks(db_mc_getSub(FContact, i));
  end;
end;

procedure TContactBookmarks.DeleteBookmarks;
begin
  DBDelete(FContact,hppDBName,'Bookmarks');
end;

destructor TContactBookmarks.Destroy;
begin
  Bookmarks.Free;
  inherited;
end;

function TContactBookmarks.GetBookmarked(Index: THandle): Boolean;
begin
  Result := Bookmarks[Index];
end;

function TContactBookmarks.GetCount: Integer;
begin
  Result := Length(Bookmarks.Table);
end;

function TContactBookmarks.GetItems(Index: Integer): THandle;
begin
  Result := PEventData(Bookmarks.Table[Index].Value)^.hDBEvent;
end;

function TContactBookmarks.GetNames(Index: Integer): String;
var
  hDBEvent: THandle;
begin
  hDBEvent := PEventData(Bookmarks.Table[Index].Value)^.hDBEvent;
  Result := Bookmarks.GetItemName(hDBEvent)
end;

procedure TContactBookmarks.LoadBookmarks(hContact:TMCONTACT);
var
  i: Integer;
  mem: PEventData;
  mem_org: Pointer;
  mem_len: Integer;
  rec_size: Word;
  rec_count: Integer;
  ed: PEventData;
  tmp_ed: TEventData;
  pbed: PBadEventData;
  AllOk: Boolean;
begin
  if not GetDBBlob(hContact, hppDBName, 'Bookmarks', mem_org, mem_len) then
    exit;
  try
    AllOk := True;
    if mem_len < SizeOf(Word) then
      raise EAbort.Create('Too small bookmarks rec');
    rec_size := PWord(mem_org)^;
    if rec_size < SizeOf(TEventData) then
      raise EAbort.Create('Bookmark size is too small');
    rec_count := (mem_len - SizeOf(Word)) div rec_size;
    mem := pointer(uint_ptr(mem_org) + SizeOf(Word));
    for i := 0 to rec_count - 1 do
    begin
      ed := PEventData(int_ptr(mem) + i * rec_size);
      {$IFDEF WIN64}
      if rec_size = SizeOf(TBadEventData) then
      begin
        pbed := PBadEventData(int_ptr(mem) + i * rec_size);
        tmp_ed.hDBEvent := pbed^.hDBEvent;
        tmp_ed.CRC32 := pbed^.CRC32;
        tmp_ed.Timestamp := pbed^.Timestamp;
        ed := @tmp_ed;
        AllOk := false;
      end;
      {$ENDIF}
      if not Bookmarks.AddEventData(ed^) then
        AllOk := false;
    end;
    FreeMem(mem_org, mem_len);
    // if we found that some items are missing or different, save
    // correct copy:
    if not AllOk then
      SaveBookmarks;
  except
    DeleteBookmarks;
  end;
end;

procedure TContactBookmarks.SaveBookmarks;
var
  mem: Pointer;
  mem_len: Integer;
  i: Integer;
  src,dst: PEventData;
begin
  if Length(Bookmarks.Table) > 0 then
  begin
    mem_len := Length(Bookmarks.Table) * SizeOf(TEventData) + SizeOf(Word);
    GetMem(mem, mem_len);
    PWord(mem)^ := Word(SizeOf(TEventData));
    for i := 0 to High(Bookmarks.Table) do
    begin
      src := PEventData(Bookmarks.Table[i].Value);
      dst := PEventData(int_ptr(mem) + SizeOf(Word) + i * SizeOf(TEventData));
      Move(src^, dst^, SizeOf(src^));
    end;
    WriteDBBlob(FContact, hppDBName, 'Bookmarks', mem, mem_len);
    FreeMem(mem, mem_len);
  end
  else
  begin
    DeleteBookmarks;
  end;
end;

procedure TContactBookmarks.SetBookmarked(Index: THandle; const Value: Boolean);
var
  res: Boolean;
begin
  if Value then
    res := Bookmarks.AddItem(Index)
  else
    res := Bookmarks.RemoveItem(Index);
  if res then
  begin
    SaveBookmarks;
    NotifyAllForms(HM_NOTF_BOOKMARKCHANGED, FContact, Index);
  end;
end;

procedure TContactBookmarks.SetBookmarkName(Index: THandle; const Value: String);
begin
  Bookmarks.AddItemName(Index,Value);
end;

function TContactBookmarks.GetBookmarkName(Index: THandle): String;
begin
  Result := Bookmarks.GetItemName(Index);
end;

procedure TContactBookmarks.Clear;
begin
  Bookmarks.Clear;
  DeleteBookmarks;
  //NotifyAllForms(HM_NOTF_BOOKMARKCHANGED,hContact,0);
end;

{ TPseudoHash }

function TPseudoHash.AddKey(Key:uint_ptr; Value: pointer): Boolean;
var
  Nearest: Integer;
  ph: TPseudoHashEntry;
  i: Integer;
begin
  Result := False;
  ph.Key := Key;
  Nearest := SearchDynArray(Table,SizeOf(TPseudoHashEntry),DynArrayComparePseudoHash,@ph,True);
  if Nearest <> -1 then begin // we have nearest or match
    if Table[Nearest].Key = Key then
      exit;
    if Table[Nearest].Key < Key then
      Inc(Nearest);
  end
  else
    Nearest := 0; // table is empty

  SetLength(Table,Length(Table)+1);
  for i := Length(Table)-1 downto Nearest do
    Table[i] := Table[i-1];

  Table[Nearest].Key := Key;
  Table[Nearest].Value := Value;

  Result := True;
end;

destructor TPseudoHash.Destroy;
begin
  SetLength(Table,0);
  inherited;
end;

function TPseudoHash.GetKey(Key: uint_ptr; var Value: pointer): Boolean;
var
  ph: TPseudoHashEntry;
  res: Integer;
begin
  Result := false;
  ph.Key := Key;
  res := SearchDynArray(Table, SizeOf(TPseudoHashEntry), DynArrayComparePseudoHash, @ph, false);
  if res <> -1 then
  begin
    Result := True;
    Value := Table[res].Value;
  end;
end;
(*
procedure TPseudoHash.InsertByIndex(Index: Integer; Key:cardinal; Value: pointer);
begin
  //
end;
*)
procedure TPseudoHash.RemoveByIndex(Index: Integer);
var
  i: Integer;
begin
  for i := Index to Length(Table) - 2 do
    Table[i] := Table[i+1];
  SetLength(Table,Length(Table)-1);
end;

function TPseudoHash.RemoveKey(Key: uint_ptr): Boolean;
var
  idx: Integer;
  ph: TPseudoHashEntry;
begin
  Result := False;
  ph.Key := Key;
  idx := SearchDynArray(Table,SizeOf(TPseudoHashEntry),DynArrayComparePseudoHash,@ph,False);
  if idx = -1 then exit;
  RemoveByIndex(idx);
  Result := True;
end;

{ TContactsHash }

destructor TContactsHash.Destroy;
var
  i: Integer;
begin
  for i := 0 to Length(Table) - 1 do
    TContactBookmarks(Pointer(Table[i].Value)).Free;
  inherited;
end;

function TContactsHash.GetContactBookmarks(Index: THandle): TContactBookmarks;
var
  val: Pointer;
begin
  // Result := nil;
  if GetKey(Cardinal(Index), val) then
    Result := TContactBookmarks(val)
  else
  begin
    Result := TContactBookmarks.Create(Index);
    AddKey(Cardinal(Index), pointer(Result));
  end;
end;

function TContactsHash.RemoveItem(Index: THandle): Boolean;
var
  val: Pointer;
begin
  Result := false;
  if GetKey(Cardinal(Index), val) then
  begin
    RemoveKey(Cardinal(Index));
    TContactBookmarks(val).Free;
    Result := True;
  end;
end;

{ TBookmarksHash }

function TBookmarksHash.AddEventData(var EventData: TEventData): Boolean;
var
  ped: PEventData;
  ts: Cardinal;
  ItemExists, ItemCorrect, NewItemFound: Boolean;
begin
  GetMem(ped, SizeOf(TEventData));
  ped^.hDBEvent := EventData.hDBEvent;
  ped^.CRC32 := EventData.CRC32;
  ped^.Timestamp := EventData.Timestamp;
  ItemExists := (db_event_getBlobSize(EventData.hDBEvent) >= 0);
  ItemCorrect := false; // added by Awkward, default value
  if ItemExists then
  begin
    ts := GetEventTimestamp(EventData.hDBEvent);
    ItemCorrect := (ts = ped^.Timestamp);
    // we might check for CRC32 here also?
  end;
  if (not ItemExists) or (not ItemCorrect) then
  begin
    Result := false;
    NewItemFound := FindEventByTimestampAndCrc(ped); // try to find the item
    if not NewItemFound then
    begin // can not find
      FreeMem(ped, SizeOf(TEventData));
      exit;
    end
    else
      AddKey(ped^.hDBEvent, ped);
    // exit, but leave Result = False as we want to resave after this load
  end
  else
    Result := AddKey(ped^.hDBEvent, ped); // item exists, add as normal
end;

function TBookmarksHash.AddItem(hDBEvent: THandle): Boolean;
var
  ped: PEventData;
  hi: THistoryItem;
begin
  GetMem(ped,SizeOf(TEventData));
  ped^.hDBEvent := hDBEvent;
  hi := ReadEvent(hDBEvent,Contact.ContactCP);
  ped^.Timestamp := hi.Time;
  CalcCRC32(PWideChar(hi.Text),Length(hi.Text)*SizeOf(WideChar),Cardinal(ped^.CRC32));
  Result := AddKey(hDBEvent,ped);
end;

function TBookmarksHash.AddItemName(hDBEvent: THandle; const Value: String): Boolean;
begin
  Result := (WriteDBWideStr(Contact.FContact,hppDBName,AnsiString('bm'+intToStr(hDBEvent)),Value) = 0);
end;

function TBookmarksHash.GetItemName(hDBEvent: THandle): String;
begin
  Result := GetDBWideStr(Contact.FContact,hppDBName,AnsiString('bm'+intToStr(hDBEvent)),'');
end;

function TBookmarksHash.RemoveItemName(hDBEvent: THandle): Boolean;
begin
  if DBExists(Contact.FContact,hppDBName,AnsiString('bm'+intToStr(hDBEvent))) then
    Result := DBDelete(Contact.FContact,hppDBName,AnsiString('bm'+intToStr(hDBEvent)))
  else
    Result := True;
end;

constructor TBookmarksHash.Create(AContact: TContactBookmarks);
begin
  Contact := AContact;
end;

destructor TBookmarksHash.Destroy;
begin
  Clear;
  inherited;
end;

function TBookmarksHash.Clear: Integer;
var
  i: Integer;
begin
  for i := 0 to Length(Table) - 1 do
    FreeMem(PEventData(Table[i].Value),SizeOf(TEventData));
  Result := Length(Table);
  SetLength(Table,0);
end;

// currently finds events with similar timestamp ONLY
function TBookmarksHash.FindEventByTimestampAndCrc(ped: PEventData): Boolean;
var
  hDBEvent: THandle;
  first_ts,last_ts,ts,cur_ts: Integer;
  StartFromFirst: Boolean;
begin
  Result := false;

  hDBEvent := db_event_first(Contact.FContact);
  if hDBEvent = 0 then
    exit;
  first_ts := GetEventTimestamp(hDBEvent);
  hDBEvent := db_event_last(Contact.FContact);
  if hDBEvent = 0 then
    exit;
  last_ts := GetEventTimestamp(hDBEvent);
  ts := ped^.Timestamp;
  if (ts < first_ts) or (ts > last_ts) then
    exit;
  StartFromFirst := ((ts - first_ts) < (last_ts - ts));

  if StartFromFirst then
  begin
    hDBEvent := db_event_first(Contact.FContact);
    while hDBEvent <> 0 do
    begin
      cur_ts := GetEventTimestamp(hDBEvent);
      if cur_ts > ts then
        break;
      if cur_ts = ts then
      begin
        ped^.hDBEvent := hDBEvent;
        Result := True;
        break;
      end;
      hDBEvent := db_event_next(Contact.FContact,hDBEvent);
    end;
  end
  else
  begin
    hDBEvent := db_event_last(Contact.FContact);
    while hDBEvent <> 0 do
    begin
      cur_ts := GetEventTimestamp(hDBEvent);
      if ts > cur_ts then
        break;
      if cur_ts = ts then
      begin
        ped^.hDBEvent := hDBEvent;
        Result := True;
        break;
      end;
      hDBEvent := db_event_prev(Contact.FContact,hDBEvent);
    end;
  end;
end;
(*
function TBookmarksHash.GetBookmark(hDBEvent: THandle;
  var EventData: TEventData): Boolean;
var
  val: Pointer;
begin
  Result := False;
  if GetKey(Cardinal(hDBEvent),val) then begin
    EventData := PEventData(val)^;
    Result := True;
  end;
end;
*)
function TBookmarksHash.GetHasItem(Index: THandle): Boolean;
var
  val: Pointer;
begin
  Result := False;
  if GetKey(uint_ptr(Index),val) then
    Result := True;
end;

function TBookmarksHash.RemoveItem(hDBEvent: THandle): Boolean;
var
  ped: PEventData;
begin
  Result := false;
  if GetKey(uint_ptr(hDBEvent), pointer(ped)) then
  begin
    RemoveKey(uint_ptr(hDBEvent));
    FreeMem(ped, SizeOf(ped^));
    RemoveItemName(hDBEvent);
    Result := True;
  end;
end;

end.

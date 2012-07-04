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

unit hpp_eventfilters;

interface

uses Types, SysUtils, Classes, m_api, hpp_global;

const
  // filter modes
  FM_INCLUDE = 0; // show all events from filEvents (default)
  FM_EXCLUDE = 1; // show all events except from filEvents

const
  MAX_FILTER_NAME_LENGTH = 33; // make it uneven, so our db record would align in 4 bytes

type
  ThppEventFilter = record
    Name     : String;
    Events   : TMessageTypes; // resulting events mask generated from filMode and filEvents, filled in runtime
    filMode  : Byte;          // FM_* consts
    filEvents: TMessageTypes; // filter events which are combined with filMode
    filCustom: Word;          // filter events which are combined with filMode
  end;

  ThppEventFilterArray = array of ThppEventFilter;

var
  hppEventFilters: ThppEventFilterArray;
  hppDefEventFilters: ThppEventFilterArray;

  procedure InitEventFilters;
  procedure ReadEventFilters;
  procedure WriteEventFilters;
  procedure ResetEventFiltersToDefault;
  procedure CopyEventFilters(var Src,Dest: ThppEventFilterArray);
  function GetShowAllEventsIndex(Arr: ThppEventFilterArray = nil): Integer;

  function MessageTypesToDWord(mt: TMessageTypes): DWord;

  // compile filMode & filEvents into Events:
  function GenerateEvents(filMode: Byte; filEvents: TMessageTypes): TMessageTypes;
  // compile filMode & filEvents into Events for all filters
  procedure GenerateEventFilters(var Filters: array of ThppEventFilter);

const
  EventsInclude:   TMessageTypes = [];
  EventsDirection: TMessageTypes = [mtIncoming,mtOutgoing];
  EventsExclude:   TMessageTypes = [mtUnknown,mtCustom];
  EventsCustom:    TMessageTypes = [mtCustom];

implementation

uses
  hpp_database, hpp_forms;

var
  filterAll: TMessageTypes;

const
  hppIntDefEventFilters: array[0..12] of ThppEventFilter = (
    (Name: 'Show all events'; Events: []; filMode: FM_EXCLUDE; filEvents: []),
    (Name: 'Messages'; Events: []; filMode: FM_INCLUDE; filEvents: [mtMessage,mtIncoming,mtOutgoing]),
    (Name: 'Link URLs'; Events: []; filMode: FM_INCLUDE; filEvents: [mtUrl,mtIncoming,mtOutgoing]),
    (Name: 'Files'; Events: []; filMode: FM_INCLUDE; filEvents: [mtFile,mtIncoming,mtOutgoing]),
    (Name: 'Contacts'; Events: []; filMode: FM_INCLUDE; filEvents: [mtContacts,mtIncoming,mtOutgoing]),
    (Name: 'Status changes'; Events: [];  filMode: FM_INCLUDE; filEvents: [mtStatus,mtIncoming,mtOutgoing]),
    (Name: 'Status message changes'; Events: [];  filMode: FM_INCLUDE; filEvents: [mtStatusMessage,mtIncoming,mtOutgoing]),
    (Name: 'Nick changes'; Events: [];  filMode: FM_INCLUDE; filEvents: [mtNickChange,mtIncoming,mtOutgoing]),
    (Name: 'Avatar changes'; Events: [];  filMode: FM_INCLUDE; filEvents: [mtAvatarChange,mtIncoming,mtOutgoing]),
    (Name: 'WATrack notify'; Events: [];  filMode: FM_INCLUDE; filEvents: [mtWATrack,mtIncoming,mtOutgoing]),
    (Name: 'Voice calls'; Events: [];  filMode: FM_INCLUDE; filEvents: [mtVoiceCall,mtIncoming,mtOutgoing]),
    (Name: 'All except changes'; Events: []; filMode: FM_EXCLUDE; filEvents: [mtStatus,mtStatusMessage,mtNickChange,mtAvatarChange]),
    (Name: 'All except system'; Events: []; filMode: FM_EXCLUDE; filEvents: [mtSystem])
    );


function IsSameAsDefault: Boolean;
var
  i: Integer;
begin
  Result := False;
  if Length(hppDefEventFilters) <> Length(hppEventFilters) then
    exit;
  for i := 0 to Length(hppEventFilters) - 1 do
  begin
    if hppEventFilters[i].Name <> hppDefEventFilters[i].Name then
      exit;
    if hppEventFilters[i].Events <> hppDefEventFilters[i].Events then
      exit;
  end;
  Result := True;
end;

function DWordToMessageTypes(dwmt: DWord): TMessageTypes;
begin
  Result := [];
  Move(dwmt,Result,SizeOf(Result));
end;

function MessageTypesToDWord(mt: TMessageTypes): DWord;
begin
  Result := 0;
  Move(mt,Result,SizeOf(mt));
end;

procedure UpdateEventFiltersOnForms;
begin
  NotifyAllForms(HM_NOTF_FILTERSCHANGED,0,0);
end;

function GenerateEvents(filMode: Byte; filEvents: TMessageTypes): TMessageTypes;
begin
  if filMode = FM_INCLUDE then
    Result := filEvents
  else
    Result := filterAll - filEvents;
  Result := Result - EventsExclude + EventsInclude;
end;

procedure GenerateEventFilters(var Filters: array of ThppEventFilter);
var
  i: Integer;
begin
  for i := 0 to Length(Filters) - 1 do
  begin
    Filters[i].Events := GenerateEvents(Filters[i].filMode, Filters[i].filEvents);
  end;
end;

procedure CopyEventFilters(var Src,Dest: ThppEventFilterArray);
var
  i: Integer;
begin
  SetLength(Dest,Length(Src));
  for i := 0 to Length(Src) - 1 do
  begin
    Dest[i].Name      := Src[i].Name;
    Dest[i].Events    := Src[i].Events;
    Dest[i].filMode   := Src[i].filMode;
    Dest[i].filEvents := Src[i].filEvents;
    Dest[i].filCustom := Src[i].filCustom;
  end;
end;

function GetShowAllEventsIndex(Arr: ThppEventFilterArray = nil): Integer;
var
  i: Integer;
begin
  if Arr = nil then
    Arr := hppEventFilters;
  Result := 0;
  for i := 0 to Length(Arr) - 1 do
    if (Arr[i].filMode = FM_EXCLUDE) and (Arr[i].filEvents = []) then
    begin
      Result := i;
      break;
    end;
end;

procedure DeleteEventFilterSettings;
var
  i: Integer;
begin
  i := 1;
  while True do
  begin
    if not DBDelete(hppDBName, AnsiString('EventFilter' + IntToStr(i))) then
      break;
    Inc(i);
  end;
end;

procedure ResetEventFiltersToDefault;
begin
  CopyEventFilters(hppDefEventFilters,hppEventFilters);
  DeleteEventFilterSettings;
  UpdateEventFiltersOnForms;
end;

//----- Stealed fro Tnt -----
function ExtractStringFromStringArray(var P: PChar; Separator: Char = #0): String;
var
  Start: PChar;
begin
  Start := P;
  P := StrScan(Start, Separator);
  if P = nil then
  begin
    Result := Start;
    P := StrEnd(Start);
  end
  else
  begin
    SetString(Result, Start, P - Start);
    Inc(P);
  end;
end;

function ExtractStringsFromStringArray(P: PChar; Separator: Char = #0): TWideStringDynArray;
const
  GROW_COUNT = 256;
var
  Count: Integer;
  Item: WideString;
begin
  Count := 0;
  SetLength(Result, GROW_COUNT);
  Item := ExtractStringFromStringArray(P, Separator);
  While Item <> '' do
  begin
    if Count > High(Result) then
      SetLength(Result, Length(Result) + GROW_COUNT);
    Result[Count] := Item;
    Inc(Count);
    Item := ExtractStringFromStringArray(P, Separator);
  end;
  SetLength(Result, Count);
end;
//----- end of Tnt -----
procedure ReadEventFilters;
var
  i: Integer;
  FilterStr: String;
  hexs: TWideStringDynArray;
  filEvents: DWord;
  filMode: Byte;
  filCustom: Word;
begin
  SetLength(hppEventFilters, 0);
  try
    i := 1;
    while True do
    begin
      if not DBExists(hppDBName, AnsiString('EventFilter' + IntToStr(i))) then
      begin
        if Length(hppEventFilters) = 0 then
          raise EAbort.Create('No filters');
        break;
      end;
      FilterStr := GetDBWideStr(hppDBName, AnsiString('EventFilter' + IntToStr(i)), '');
      if FilterStr = '' then
        break;
      SetLength(hppEventFilters, Length(hppEventFilters) + 1);
      // parse String
      hexs := ExtractStringsFromStringArray(PWideChar(FilterStr),',');
      if Length(hexs) < 4 then
        raise EAbort.Create('Wrong filter (' + IntToStr(i) + ') format');
      filMode := 0;
      filEvents := 0;
      filCustom := 0;
      hppEventFilters[i - 1].Name := hexs[0];
      // read filMode
      HexToBin(PWideChar(hexs[1]), @filMode, SizeOf(filMode));
      hppEventFilters[i - 1].filMode := filMode;
      // read filEvents
      HexToBin(PWideChar(hexs[2]), @filEvents, SizeOf(filEvents));
      hppEventFilters[i - 1].filEvents := DWordToMessageTypes(filEvents);
      // read filCustom
      HexToBin(PWideChar(hexs[3]), @filEvents, SizeOf(filCustom));
      hppEventFilters[i - 1].filCustom := filCustom;
      Inc(i);
    end;
    GenerateEventFilters(hppEventFilters);
  except
    ResetEventFiltersToDefault;
  end;
end;

procedure WriteEventFilters;
var
  i: Integer;
  FilterStr: String;
  hex: String;
begin
  if Length(hppEventFilters) = 0 then
  begin
    ResetEventFiltersToDefault;
    exit;
  end;
  if IsSameAsDefault then
  begin
    // revert to default state
    DeleteEventFilterSettings;
    UpdateEventFiltersOnForms;
    exit;
  end;

  for i := 0 to Length(hppEventFilters) - 1 do
  begin
    FilterStr := Copy(hppEventFilters[i].Name, 1, MAX_FILTER_NAME_LENGTH);
    // add filMode
    SetLength(hex, SizeOf(hppEventFilters[i].filMode) * 2);
    BinToHex(@hppEventFilters[i].filMode, PChar(hex),SizeOf(hppEventFilters[i].filMode));
    FilterStr := FilterStr + ',' + hex;
    // add filEvents
    SetLength(hex, SizeOf(hppEventFilters[i].filEvents) * 2);
    BinToHex(@hppEventFilters[i].filEvents, PChar(hex),SizeOf(hppEventFilters[i].filEvents));
    FilterStr := FilterStr + ',' + hex;
    // add filCustom
    SetLength(hex, SizeOf(hppEventFilters[i].filCustom) * 2);
    BinToHex(@hppEventFilters[i].filCustom, PChar(hex),SizeOf(hppEventFilters[i].filCustom));
    FilterStr := FilterStr + ',' + hex;

    WriteDBWideStr(hppDBName, AnsiString('EventFilter' + IntToStr(i + 1)), FilterStr);
  end;
  // delete left filters if we have more than Length(hppEventFilters)
  i := Length(hppEventFilters) + 1;
  while True do
  begin
    if not DBDelete(hppDBName, AnsiString('EventFilter' + IntToStr(i))) then
      break;
    Inc(i);
  end;
  UpdateEventFiltersOnForms;
end;

procedure InitEventFilters;
var
  i: Integer;
  mt: TMessageType;
begin
  // translate and copy internal default static array to dynamic array
  SetLength(hppDefEventFilters, Length(hppIntDefEventFilters));
  for i := 0 to High(hppIntDefEventFilters) do
  begin
    hppDefEventFilters[i].Name := Copy(TranslateUnicodeString(hppIntDefEventFilters[i].Name), 1,
      MAX_FILTER_NAME_LENGTH { TRANSLATE-IGNORE } );
    hppDefEventFilters[i].filMode := hppIntDefEventFilters[i].filMode;
    hppDefEventFilters[i].filEvents := hppIntDefEventFilters[i].filEvents;
    hppDefEventFilters[i].filCustom := hppIntDefEventFilters[i].filCustom;
  end;

  filterAll := [];
  for mt := Low(TMessageType) to High(TMessageType) do
    Include(filterAll, mt);

  GenerateEventFilters(hppDefEventFilters);
end;

end.

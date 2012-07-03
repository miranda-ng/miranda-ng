Unit ImportTU;

interface

Uses windows, // StrUtils,
  m_api;

type
  PDayTable = ^TDayTable;
  TDayTable = array [1 .. 12] of Word;

const
  MonthDays: array [Boolean] of TDayTable = (
    (31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31),
    (31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31));

function Timestamp(Year, Month, Day, Hour, Min, Sec: Word; toGMT: Boolean = true): LongWord;
{ function TimestampICQ(Year,Month,Day,Hour,Min,Sec:Word):LongWord; }
function DateTimeToTimeStamp(const DateTime: TDateTime; toGMT: Boolean = true): DWord;

{ ***** Authtor of this procedures Alexey Kulakov aka Awkward***** }
function ChangeUnicode(str: PWideChar): PWideChar;
function UTF8Len(src: PAnsiChar): integer;
function WideToANSI(src: PWideChar; var dst: PAnsiChar; cp: DWord = CP_ACP): PAnsiChar;
function ANSIToWide(src: PAnsiChar; var dst: PWideChar; cp: DWord = CP_ACP): PWideChar;
function ANSIToUTF8(src: PAnsiChar; var dst: PAnsiChar; cp: DWord = CP_ACP): PAnsiChar;
function UTF8toANSI(src: PAnsiChar; var dst: PAnsiChar; cp: DWord = CP_ACP): PAnsiChar;
function UTF8toWide(src: PAnsiChar; var dst: PWideChar; len: cardinal = DWord(-1)): PWideChar;
function WidetoUTF8(src: PWideChar; var dst: PAnsiChar): PAnsiChar;
{ ***** }

implementation

uses SysUtils;

function IsLeapYear(Year: Word): Boolean;
begin
  Result := (Year mod 4 = 0) and ((Year mod 100 <> 0) or (Year mod 400 = 0));
end;

function Timestamp(Year, Month, Day, Hour, Min, Sec: Word; toGMT: Boolean = true): LongWord;
var
  i: integer;
  DayTable: PDayTable;
  DT, D: LongWord;
begin
  // fix for 2 digit year
  if Year > 0 then
    if Year < 90 then
      inc(Year, 2000)
    else if Year < 100 then
      inc(Year, 1900);
  //
  DayTable := @MonthDays[IsLeapYear(Year)];
  for i := 1 to Month - 1 do
    inc(Day, DayTable^[i]);
  i := Year - 1;
  D := i * 365 + i div 4 - i div 100 + i div 400 + Day -
    (1969 * 365 + 492 - 19 + 4 + 1);
  DT := (D * 24 * 60 * 60) + (Hour * 3600 + Min * 60 + Sec);
  // Приводим к GMT...судя по всему миранда хранит таймштампы в GMT
  if toGMT then
    Result := DT - (Longword(CallService(MS_DB_TIME_TIMESTAMPTOLOCAL, DT, 0)) - DT)
  else
    Result := DT;
end;

function DateTimeToTimeStamp(const DateTime: TDateTime; toGMT: Boolean = true): DWord;
begin
  Result := Round((DateTime - UnixDateDelta) * SecsPerDay);
  if toGMT then
    Result := Result - (Dword(CallService(MS_DB_TIME_TIMESTAMPTOLOCAL, Result, 0)) - Result);
end;

function ChangeUnicode(str: PWideChar): PWideChar;
var
  i, len: integer;
begin
  Result := str;
  if str = nil then
    exit;
  if (Word(str^) = $FFFE) or (Word(str^) = $FEFF) then
  begin
    len := lstrlenw(str);
    if Word(str^) = $FFFE then
    begin
      i := len - 1;
      while i > 0 do // str^<>#0
      begin
        pword(str)^ := swap(pword(str)^);
        inc(str);
        dec(i);
      end;
      // str:=result;
    end;
    move((Result + 1)^, Result^, len * SizeOf(WideChar));
  end;
end;

function WideToANSI(src: PWideChar; var dst: PAnsiChar; cp: DWord = CP_ACP): PAnsiChar;
var
  len, l: integer;
begin
  if (src = nil) or (src^ = #0) then
  begin
    GetMem(dst, 1);
    dst^ := #0;
  end
  else
  begin
    l := lstrlenw(src);
    len := WideCharToMultiByte(cp, 0, src, l, NIL, 0, NIL, NIL) + 1;
    GetMem(dst, len);
    FillChar(dst^, len, 0);
    WideCharToMultiByte(cp, 0, src, l, dst, len, NIL, NIL);
  end;
  Result := dst;
end;

function ANSIToWide(src: PAnsiChar; var dst: PWideChar; cp: DWord = CP_ACP): PWideChar;
var
  len, l: integer;
begin
  if (src = nil) or (src^ = #0) then
  begin
    GetMem(dst, SizeOf(WideChar));
    dst^ := #0;
  end
  else
  begin
    l := lstrlena(src);
    len := MultiByteToWideChar(cp, 0, src, l, NIL, 0) + 1;
    GetMem(dst, len * SizeOf(WideChar));
    FillChar(dst^, len * SizeOf(WideChar), 0);
    MultiByteToWideChar(cp, 0, src, l, dst, len);
  end;
  Result := dst;
end;

function ANSIToUTF8(src: PAnsiChar; var dst: PAnsiChar; cp: DWord = CP_ACP): PAnsiChar;
var
  tmp: PWideChar;
begin
  ANSIToWide(src, tmp, cp);
  Result := WidetoUTF8(tmp, dst);
  FreeMem(tmp);
end;

function UTF8Len(src: PAnsiChar): integer; // w/o zero
begin
  Result := 0;
  if src <> nil then
  begin
    while src^ <> #0 do
    begin
      if (ord(src^) and $80) = 0 then
      else if (ord(src^) and $E0) = $E0 then
        inc(src, 2)
      else
        inc(src);
      inc(Result);
      inc(src);
    end;
  end;
end;

function CalcUTF8Len(src: PWideChar): integer;
begin
  Result := 0;
  if src <> nil then
  begin
    while src^ <> #0 do
    begin
      if src^ < #$0080 then
      else if src^ < #$0800 then
        inc(Result)
      else
        inc(Result, 2);
      inc(src);
      inc(Result);
    end;
  end;
end;

function UTF8toWide(src: PAnsiChar; var dst: PWideChar; len: cardinal = DWord(-1)): PWideChar;
var
  w: Word;
  p: PWideChar;
begin
  GetMem(dst, (UTF8Len(src) + 1) * SizeOf(WideChar));
  p := dst;
  if src <> nil then
  begin
    while (src^ <> #0) and (len > 0) do
    begin
      if ord(src^) < $80 then
        w := ord(src^)
      else if (ord(src^) and $E0) = $E0 then
      begin
        w := (ord(src^) and $1F) shl 12;
        inc(src);
        dec(len);
        w := w or (((ord(src^)) and $3F) shl 6);
        inc(src);
        dec(len);
        w := w or (ord(src^) and $3F);
      end
      else
      begin
        w := (ord(src^) and $3F) shl 6;
        inc(src);
        dec(len);
        w := w or (ord(src^) and $3F);
      end;
      p^ := WideChar(w);
      inc(p);
      inc(src);
      dec(len);
    end;
  end;
  p^ := #0;
  Result := dst;
end;

function UTF8toANSI(src: PAnsiChar; var dst: PAnsiChar; cp: DWord = CP_ACP): PAnsiChar;
var
  tmp: PWideChar;
begin
  UTF8toWide(src, tmp);
  Result := WideToANSI(tmp, dst, cp);
  FreeMem(tmp);
end;

function WidetoUTF8(src: PWideChar; var dst: PAnsiChar): PAnsiChar;
var
  p: PAnsiChar;
begin
  GetMem(dst, CalcUTF8Len(src) + 1);
  p := dst;
  if src <> nil then
  begin
    while src^ <> #0 do
    begin
      if src^ < #$0080 then
        p^ := AnsiChar(src^)
      else if src^ < #$0800 then
      begin
        p^ := AnsiChar($C0 or (ord(src^) shr 6));
        inc(p);
        p^ := AnsiChar($80 or (ord(src^) and $3F));
      end
      else
      begin
        p^ := AnsiChar($E0 or (ord(src^) shr 12));
        inc(p);
        p^ := AnsiChar($80 or ((ord(src^) shr 6) and $3F));
        inc(p);
        p^ := AnsiChar($80 or (ord(src^) and $3F));
      end;
      inc(p);
      inc(src);
    end;
  end;
  p^ := #0;
  Result := dst;
end;

end.

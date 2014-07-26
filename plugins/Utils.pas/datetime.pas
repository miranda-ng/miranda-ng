unit DateTime;

interface

uses
  Windows;

const
  SecondsPerDay = 24*60*60;
  // Days between 1/1/0001 and 12/31/1899
  DateDelta = 693594;
  // Days between TDateTime basis (12/31/1899) and Unix time_t basis (1/1/1970)
  UnixDateDelta = 25569;
  // Days between Unix time_t basis (1/1/1970) and Windows timestamp (1/1/1601)
  WinDateDelta = 134774; // 

function IsLeapYear(Year:word):Boolean;
function EncodeTime(Hour, Minute, Sec: cardinal):TDateTime;
function EncodeDate(Year, Month , Day: cardinal):TDateTime;

function Timestamp(Year,Month,Day:cardinal;Hour:cardinal=0;Minute:cardinal=0;Sec:cardinal=0):dword;
function GetCurrentTimestamp:DWord;

procedure UnixTimeToFileTime(ts:int_ptr; var pft:TFILETIME);
function FileTimeToUnixTime(const pft: TFILETIME):int_ptr;
function TimeStampToLocalTimeStamp(ts:int_ptr):int_ptr;
function TimestampToDateTime(ts:int_ptr):TDateTime;
function TimestampToSystemTime(Time:DWord; var ST:TSystemTime):PSystemTime;

function DateTimeToStr(Time:Dword; Format:pWideChar=nil):pWideChar;
function DateToStr    (Time:Dword; Format:pWideChar=nil):pWideChar;
function TimeToStr    (Time:Dword; Format:pWideChar=nil):pWideChar;

implementation

uses
  common;

type
  PDayTable = ^TDayTable;
  TDayTable = array [0..11] of byte;

const
  MonthDays: array [Boolean] of TDayTable =
    ((31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31),
     (31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31));

function IsLeapYear(Year:word):Boolean;
begin
  Result:=(Year mod 4=0) and ((Year mod 100<>0) or (Year mod 400=0));
end;

function EncodeTime(Hour, Minute, Sec: cardinal): TDateTime;
begin
  result := (Hour*3600 + Minute*60 + Sec) / SecondsPerDay;
end;

function EncodeDate(Year, Month, Day: cardinal):TDateTime;
var
  DayTable: PDayTable;
begin
  DayTable := @MonthDays[IsLeapYear(Year)];
  dec(Month);
  while Month>0 do
  begin
    dec(Month);
    inc(Day,DayTable^[Month]);
  end;

  dec(Year);
  result := Year * 365 + Year div 4 - Year div 100 + Year div 400 + Day - DateDelta;
end;

function Timestamp(Year,Month,Day:cardinal;Hour:cardinal=0;Minute:cardinal=0;Sec:cardinal=0):dword;
var
  t:tDateTime;
begin
  t := EncodeDate(Year, Month, Day);
  if t >= 0 then
    t := t + EncodeTime(Hour, Minute, Sec)
  else
    t := t - EncodeTime(Hour, Minute, Sec);
  result:=Round((t - UnixDateDelta) * SecondsPerDay);
end;

function GetCurrentTimestamp:dword;
var
  st:tSystemTime;
begin
  GetSystemTime(st);
  result:=Timestamp(st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
end;

procedure UnixTimeToFileTime(ts:int_ptr; var pft:TFILETIME);
var
  ll:uint64;
begin
	ll := (int64(WinDateDelta)*SecondsPerDay + ts) * 10000000;
	pft.dwLowDateTime  := dword(ll);
	pft.dwHighDateTime := ll shr 32;
end;

function FileTimeToUnixTime(const pft: TFILETIME):int_ptr;
var
  ll:uint64;
begin
	ll := (uint64(pft.dwHighDateTime) shl 32) or pft.dwLowDateTime;
	ll := (ll div 10000000) - int64(WinDateDelta)*SecondsPerDay;
	result := int_ptr(ll);
end;

function TimeStampToLocalTimeStamp(ts:int_ptr):int_ptr;
var
  ft,lft:TFileTime;
begin
  UnixTimeToFileTime(ts,ft);
  FileTimeToLocalFileTime(ft, lft);
  result:=FileTimeToUnixTime(lft);
end;

function TimestampToDateTime(ts:int_ptr):TDateTime;
begin
  Result := UnixDateDelta + TimeStampToLocalTimeStamp(ts) / SecondsPerDay;
end;

function TimestampToSystemTime(Time:DWord; var ST:TSystemTime):PSystemTime;
var
  aft,lft:TFILETIME;
begin
  UnixTimeToFileTime(Time,aft);
  FileTimeToLocalFileTime(aft, lft);
  FileTimeToSystemTime(lft,ST);
  result:=@ST;
end;

function DateTimeToStr(Time:Dword; Format:pWideChar=nil):pWideChar;
var
  buf:array [0..300] of WideChar;
  ST: TSystemTime;
  pc:pWideChar;
begin
  TimestampToSystemTime(Time,ST);
  GetDateFormatW(LOCALE_USER_DEFAULT,0,@ST,Format,@buf,300);
  if Format<>nil then
    GetTimeFormatW(LOCALE_USER_DEFAULT,0,@ST,@buf,@buf,300)
  else
  begin
    pc:=StrEndW(@buf); pc^:=' '; inc(pc);
    GetTimeFormatW(LOCALE_USER_DEFAULT,0,@ST,nil,pc,300-(pc-pWideChar(@buf)))
  end;
  StrDupW(result,buf);
end;

function DateToStr(Time:Dword; Format:pWideChar=nil):pWideChar;
var
  buf:array [0..300] of WideChar;
  ST: TSystemTime;
begin
  TimestampToSystemTime(Time,ST);
  GetDateFormatW(LOCALE_USER_DEFAULT,0,@ST,Format,@buf,300);
  StrDupW(result,buf);
end;

function TimeToStr(Time:Dword; Format:pWideChar=nil):pWideChar;
var
  buf:array [0..300] of WideChar;
  ST: TSystemTime;
begin
  TimestampToSystemTime(Time,ST);
  GetTimeFormatW(LOCALE_USER_DEFAULT,0,@ST,Format,@buf,300);
  StrDupW(result,buf);
end;

end.

unit ChessClockUnit;

interface

type
  TChessClock = class
  public
    class function IsZeitnot(const time: TDateTime): boolean;
    class function ConvertToStr(const time: TDateTime): string;
    class function ConvertToFullStr(const time: TDateTime;
      bIncludeMSec: boolean = TRUE): string;
    class function ConvertFromFullStr(const strTime: string): TDateTime;
  end;

implementation

uses
  SysUtils;

const
  FULL_TIME_FORMAT = 'h":"n":"s"."z';
  HOUR_TIME_FORMAT = 'h":"nn":"ss';
  MIN_TIME_FORMAT = 'n":"ss';
  ZEITNOT_FORMAT = 's"."zzz';
  ZEITNOT_BOARDER = 10; // sec. - zeitnot border

////////////////////////////////////////////////////////////////////////////////
//  TChessClock

class function TChessClock.IsZeitnot(const time: TDateTime): boolean;
begin
  Result := ((time > 0) and (time < EncodeTime(0, 0, ZEITNOT_BOARDER, 0)));
end;


class function TChessClock.ConvertToStr(const time: TDateTime): string;
begin
  LongTimeFormat := MIN_TIME_FORMAT;
  if (time >= EncodeTime(1, 0, 0, 0)) then
    LongTimeFormat := HOUR_TIME_FORMAT
  else if (IsZeitnot(time)) then
    LongTimeFormat := ZEITNOT_FORMAT;

  Result := TimeToStr(time);
end;


class function TChessClock.ConvertToFullStr(const time: TDateTime;
  bIncludeMSec: boolean = TRUE): string;
begin
  if (bIncludeMSec) then
    LongTimeFormat := FULL_TIME_FORMAT
  else
    LongTimeFormat := HOUR_TIME_FORMAT;

  Result := TimeToStr(time);
end;


class function TChessClock.ConvertFromFullStr(const strTime: string): TDateTime;

  procedure NParse(strTime: string; out Hour, Min, Sec, MSec: Word);
  const
    TIME_DELIM = ':';
    MSEC_DELIM = '.';
  var
    iPos: integer;
    str: string;
  begin
    Hour := 0;
    Min := 0;
    Sec := 0;
    MSec := 0;

    iPos := LastDelimiter(MSEC_DELIM, strTime);
    if (iPos > 0) then
    begin
      str := Copy(strTime, iPos + 1, MaxInt);
      strTime := Copy(strTime, 1, iPos - 1);
      MSec := StrToInt(str);
    end;

    strTime := TIME_DELIM + strTime;

    iPos := LastDelimiter(TIME_DELIM, strTime);
    if (iPos = 0) then
      exit;
    str := Copy(strTime, iPos + 1, MaxInt);
    strTime := Copy(strTime, 1, iPos - 1);
    Sec := StrToInt(str);

    iPos := LastDelimiter(TIME_DELIM, strTime);
    if (iPos = 0) then
      exit;
    str := Copy(strTime, iPos + 1, MaxInt);
    strTime := Copy(strTime, 1, iPos - 1);
    Min := StrToInt(str);

    iPos := LastDelimiter(TIME_DELIM, strTime);
    if (iPos = 0) then
      exit;
    str := Copy(strTime, iPos + 1, MaxInt);
    Hour := StrToInt(str);
  end;

var
  Hour, Min, Sec, MSec: Word;
begin // .ConvertFromFullStr
  NParse(strTime, Hour, Min, Sec, MSec);
  Result := EncodeTime(Hour, Min, Sec, MSec);
end;

end.

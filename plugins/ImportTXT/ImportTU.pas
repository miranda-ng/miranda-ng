Unit ImportTU;

interface

Uses windows, StrUtils,
     m_api;

type
  PDayTable = ^TDayTable;
  TDayTable = array[1..12] of Word;

const
  MonthDays: array [Boolean] of TDayTable =
    ((31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31),
     (31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31));

function Timestamp(Year,Month,Day,Hour,Min,Sec:Word; toGMT:boolean = true):LongWord;
{function TimestampICQ(Year,Month,Day,Hour,Min,Sec:Word):LongWord;}
function DateTimeToTimeStamp(const DateTime: TDateTime; toGMT: boolean = true): DWord;



{***** Authtor of this procedures Alexey Kulakov aka Awkward*****}
function ChangeUnicode(str:PWideChar):PWideChar;
function UTF8Len(src:PChar):integer;
function WideToANSI(src:PWideChar;var dst:PChar    ;cp:dword=CP_ACP):PChar;
function ANSIToWide(src:PChar    ;var dst:PWideChar;cp:dword=CP_ACP):PWideChar;
function ANSIToUTF8(src:PChar    ;var dst:pChar    ;cp:dword=CP_ACP):PChar;
function UTF8toANSI(src:PChar    ;var dst:PChar    ;cp:dword=CP_ACP):PChar;
function UTF8toWide(src:PChar    ;var dst:PWideChar;len:cardinal=dword(-1)):PWideChar;
function WidetoUTF8(src:PWideChar;var dst:PChar    ):PChar;
{*****}


implementation

uses SysUtils;

function IsLeapYear(Year:Word):Boolean;
begin
  Result:=(Year mod 4=0) and ((Year mod 100<>0) or (Year mod 400=0));
end;

function Timestamp(Year,Month,Day,Hour,Min,Sec:Word; toGMT: boolean = true):LongWord;
var i:Integer;
    DayTable:PDayTable;
    DT,D:Longword;
begin
  //fix for 2 digit year
  if year>0 then
     if year<90 then inc(year,2000)
                else
        if year <100 then inc(year,1900);
  //
  DayTable:=@MonthDays[IsLeapYear(Year)];
  for i:=1 to Month-1 do Inc(Day,DayTable^[i]);
  I := Year - 1;
  D := I * 365 + I div 4 - I div 100 + I div 400 + Day - (1969*365 + 492 - 19 + 4 +1);
  DT:= (D*24*60*60) + (Hour*3600+Min*60+Sec);
  //Приводим к GMT...судя по всему миранда хранит таймштампы в GMT
  if toGMT then Result := DT -(PluginLink.CallService(MS_DB_TIME_TIMESTAMPTOLOCAL,DT,0)-DT)
           else Result := DT;
end;

function DateTimeToTimeStamp(const DateTime: TDateTime; toGMT: boolean = true): DWord;
begin
 Result := Round((DateTime-UnixDateDelta) * SecsPerDay);
 if toGMT then Result := Result -(PluginLink.CallService(MS_DB_TIME_TIMESTAMPTOLOCAL,Result,0)-Result);
end;

function ChangeUnicode(str:PWideChar):PWideChar;
var
  i,len:integer;
begin
  result:=str;
  if str=nil then
    exit;
  if (word(str^)=$FFFE) or (word(str^)=$FEFF) then
  begin
    len:=lstrlenw(str);
    if word(str^)=$FFFE then
    begin
      i:=len-1;
      while i>0 do // str^<>#0
      begin
        pword(str)^:=swap(pword(str)^);
        inc(str);
        dec(i);
      end;
//      str:=result;
    end;
    move((result+1)^,result^,len*SizeOf(WideChar));
  end;
end;

function WideToANSI(src:PWideChar;var dst:PChar; cp:dword=CP_ACP):PChar;
var
  len,l:integer;
begin
  if (src=nil) or (src^=#0) then
  begin
    GetMem(dst,1);
    dst^:=#0;
  end
  else
  begin
    l:=lstrlenw(src);
    len:=WideCharToMultiByte(cp,0,src,l,NIL,0,NIL,NIL)+1;
    GetMem(dst,len);
    FillChar(dst^,len,0);
    WideCharToMultiByte(cp,0,src,l,dst,len,NIL,NIL);
  end;
  result:=dst;
end;

function ANSIToWide(src:PChar;var dst:PWideChar; cp:dword=CP_ACP):PWideChar;
var
  len,l:integer;
begin
  if (src=nil) or (src^=#0) then
  begin
    GetMem(dst,SizeOf(WideChar));
    dst^:=#0;
  end
  else
  begin
    l:=lstrlen(src);
    len:=MultiByteToWideChar(cp,0,src,l,NIL,0)+1;
    GetMem(dst,len*SizeOf(WideChar));
    FillChar(dst^,len*SizeOf(WideChar),0);
    MultiByteToWideChar(cp,0,src,l,dst,len);
  end;
  result:=dst;
end;

function ANSIToUTF8(src:PChar;var dst:pChar;cp:dword=CP_ACP):PChar;
var
  tmp:PWideChar;
begin
  AnsiToWide(src,tmp,cp);
  result:=WideToUTF8(tmp,dst);
  FreeMem(tmp);
end;

function UTF8Len(src:PChar):integer; // w/o zero
begin
  result:=0;
  if src<>nil then
  begin
    while src^<>#0 do
    begin
      if (ord(src^) and $80)=0 then
      else if (ord(src^) and $E0)=$E0 then
        inc(src,2)
      else
        inc(src);
      inc(result);
      inc(src);
    end;
  end;
end;

function CalcUTF8Len(src:pWideChar):integer;
begin
  result:=0;
  if src<>nil then
  begin
    while src^<>#0 do
    begin
      if src^<#$0080 then
      else if src^<#$0800 then
        inc(result)
      else
        inc(result,2);
      inc(src);
      inc(result);
    end;
  end;
end;

function UTF8toWide(src:PChar; var dst:PWideChar; len:cardinal=dword(-1)):PWideChar;
var
  w:word;
  p:PWideChar;
begin
  GetMem(dst,(UTF8Len(src)+1)*SizeOf(WideChar));
  p:=dst;
  if src<>nil then
  begin
    while (src^<>#0) and (len>0) do
    begin
      if ord(src^)<$80 then
        w:=ord(src^)
      else if (ord(src^) and $E0)=$E0 then
      begin
        w:=(ord(src^) and $1F) shl 12;
        inc(src); dec(len);
        w:=w or (((ord(src^))and $3F) shl 6);
        inc(src); dec(len);
        w:=w or (ord(src^) and $3F);
      end
      else
      begin
        w:=(ord(src^) and $3F) shl 6;
        inc(src); dec(len);
        w:=w or (ord(src^) and $3F);
      end;
      p^:=WideChar(w);
      inc(p);
      inc(src); dec(len);
    end;
  end;
  p^:=#0;
  result:=dst;
end;

function UTF8toANSI(src:PChar;var dst:PChar;cp:dword=CP_ACP):PChar;
var
  tmp:pWideChar;
begin
  UTF8ToWide(src,tmp);
  result:=WideToAnsi(tmp,dst,cp);
  FreeMem(tmp);
end;

function WidetoUTF8(src:PWideChar; var dst:PChar):PChar;
var
  p:PChar;
begin
  GetMem(dst,CalcUTF8Len(src)+1);
  p:=dst;
  if src<>nil then
  begin
    while src^<>#0 do
    begin
      if src^<#$0080 then
        p^:=Char(src^)
      else if src^<#$0800 then
      begin
        p^:=chr($C0 or (ord(src^) shr 6));
        inc(p);
        p^:=chr($80 or (ord(src^) and $3F));
      end
      else
      begin
        p^:=chr($E0 or (ord(src^) shr 12));
        inc(p);
        p^:=chr($80 or ((ord(src^) shr 6) and $3F));
        inc(p);
        p^:=chr($80 or (ord(src^) and $3F));
      end;
      inc(p);
      inc(src);
    end;
  end;
  p^:=#0;
  result:=dst;
end;

end.

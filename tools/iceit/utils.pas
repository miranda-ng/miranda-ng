unit utils;

interface

uses Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms;

  function GetValue(S: String; H: Integer): String;
  function SetValue(S: String; H: Integer; Value: String): String;
  function SetWidth(Canvas: TCanvas; S: String; n: Integer): String;

implementation

//--------------------------------------------------------------------------------------
//  Функция возвращает текст из строки S содержащийся между символами '|' на позиции H
//--------------------------------------------------------------------------------------
function GetValue(S: String; H: Integer): String;
var F, n: Integer;
begin
  F := 0;
  Result := '';
  for n := 1 to Length(S) do
  begin
    if S[n] = '|' then inc(F);
    if (S[n] <> '|') and (F = H) then Result := Result + S[n]
  end;
end;

//--------------------------------------------------------------------------------------
//  Функция возвращает строку аналогичную S, но с замененным между
//  символами '|' на позиции H текстом на значение Value
//--------------------------------------------------------------------------------------
function SetValue(S: String; H: Integer; Value: String): String;
var F, n: Integer;
begin
  n := 1;
  F := 0;
  Result := '';
  while (n < Length(S)) and (F < H) do
  begin
    if S[n] = '|' then inc(F);
    Result := Result + S[n];
    inc(n);
  end;
  Result := Result + Value;
  while S[n] <> '|' do inc(n);
  while (n <= Length(S)) do
  begin
    Result := Result + S[n];
    inc(n);
  end;
end;

//--------------------------------------------------------------------------------------
//  Функция возвращает S если ширина строки < n,
//  или S с многоточием '...' урезанную до ширины n
//--------------------------------------------------------------------------------------
function SetWidth(Canvas: TCanvas; S: String; n: Integer): String;
begin
  if Canvas.TextWidth(S) > n then
  begin
    while Canvas.TextWidth(S + '...') > n do SetLength(S, Length(S)-1);
    Result := S + '...';
  end else Result := S;
end;

end.

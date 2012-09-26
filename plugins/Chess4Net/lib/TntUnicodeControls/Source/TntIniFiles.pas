{*****************************************************************************}
{                                                                             }
{    Tnt Delphi Unicode Controls                                              }
{                                                                             }
{    Portions created by Wild Hunter are                                      }
{    Copyright (c) 2003 Wild Hunter (raguotis@freemail.lt)                    }
{                                                                             }
{    Portions created by Stanley Xu are                                       }
{    Copyright (c) 1999-2006 Stanley Xu                                       }
{      (http://gosurfbrowser.com/?go=supportFeedback&ln=en)                   }
{                                                                             }
{    Portions created by Borland Software Corporation are                     }
{    Copyright (c) 1995-2001 Borland Software Corporation                     }
{                                                                             }
{*****************************************************************************}

unit TntIniFiles;

{$R-,T-,H+,X+}
{$INCLUDE TntCompilers.inc}

interface

uses
  Classes, IniFiles,
  TntClasses;

type

  TTntCustomIniFile = class({TCustomIniFile}TObject{TNT-ALLOW TObject})
  private
    FFileName: WideString;
  public
    constructor Create(const FileName: WideString);
    function SectionExists(const Section: WideString): Boolean;
    function ReadString(const Section, Ident, Default: WideString): WideString; virtual; abstract;
    procedure WriteString(const Section, Ident, Value: WideString); virtual; abstract;
    function ReadInteger(const Section, Ident: WideString; Default: Longint): Longint; virtual;
    procedure WriteInteger(const Section, Ident: WideString; Value: Longint); virtual;
    function ReadBool(const Section, Ident: WideString; Default: Boolean): Boolean; virtual;
    procedure WriteBool(const Section, Ident: WideString; Value: Boolean); virtual;
    function ReadBinaryStream(const Section, Name: WideString; Value: TStream): Integer; virtual;
    function ReadDate(const Section, Name: WideString; Default: TDateTime): TDateTime; virtual;
    function ReadDateTime(const Section, Name: WideString; Default: TDateTime): TDateTime; virtual;
    function ReadFloat(const Section, Name: WideString; Default: Double): Double; virtual;
    function ReadTime(const Section, Name: WideString; Default: TDateTime): TDateTime; virtual;
    procedure WriteBinaryStream(const Section, Name: WideString; Value: TStream); virtual;
    procedure WriteDate(const Section, Name: WideString; Value: TDateTime); virtual;
    procedure WriteDateTime(const Section, Name: WideString; Value: TDateTime); virtual;
    procedure WriteFloat(const Section, Name: WideString; Value: Double); virtual;
    procedure WriteTime(const Section, Name: WideString; Value: TDateTime); virtual;
    procedure ReadSection(const Section: WideString; Strings: TTntStrings); virtual; abstract;
    procedure ReadSections(Strings: TTntStrings); virtual; abstract;
    procedure ReadSectionValues(const Section: WideString; Strings: TTntStrings); virtual; abstract;
    procedure EraseSection(const Section: WideString); virtual; abstract;
    procedure DeleteKey(const Section, Ident: WideString); virtual; abstract;
    procedure UpdateFile; virtual; abstract;
    function ValueExists(const Section, Ident: WideString): Boolean;
    property FileName: WideString read FFileName;
  end;

  { TTntStringHash - used internally by TTntMemIniFile to optimize searches. }

  PPTntHashItem = ^PTntHashItem;
  PTntHashItem = ^TTntHashItem;
  TTntHashItem = record
    Next: PTntHashItem;
    Key: WideString;
    Value: Integer;
  end;

  TTntStringHash = class
  private
    Buckets: array of PTntHashItem;
  protected
    function Find(const Key: WideString): PPTntHashItem;
    function HashOf(const Key: WideString): Cardinal; virtual;
  public
    constructor Create(Size: Integer = 256);
    destructor Destroy; override;
    procedure Add(const Key: WideString; Value: Integer);
    procedure Clear;
    procedure Remove(const Key: WideString);
    function Modify(const Key: WideString; Value: Integer): Boolean;
    function ValueOf(const Key: WideString): Integer;
  end;

  { TTntHashedStringList - A TTntStringList that uses TTntStringHash to improve the
    speed of Find }

  TTntHashedStringList = class(TTntStringList)
  private
    FValueHash: TTntStringHash;
    FNameHash: TTntStringHash;
    FValueHashValid: Boolean;
    FNameHashValid: Boolean;
    procedure UpdateValueHash;
    procedure UpdateNameHash;
  protected
    procedure Changed; override;
  public
    destructor Destroy; override;
    function IndexOf(const S: WideString): Integer; override;
    function IndexOfName(const Name: WideString): Integer; override;
  end;

  { TTntMemIniFile - loads and entire ini file into memory and allows all
    operations to be performed on the memory image.  The image can then
    be written out to the disk file }

  TTntMemIniFile = class(TTntCustomIniFile)
  private
    FSections: TTntStringList;
    function AddSection(const Section: WideString): TTntStrings;
    function GetCaseSensitive: Boolean;
    procedure SetCaseSensitive(Value: Boolean);
    procedure LoadValues;
  public
    constructor Create(const FileName: WideString); virtual;
    destructor Destroy; override;
    procedure Clear;
    procedure DeleteKey(const Section, Ident: WideString); override;
    procedure EraseSection(const Section: WideString); override;
    procedure GetStrings(List: TTntStrings);
    procedure ReadSection(const Section: WideString; Strings: TTntStrings); override;
    procedure ReadSections(Strings: TTntStrings); override;
    procedure ReadSectionValues(const Section: WideString; Strings: TTntStrings); override;
    function ReadString(const Section, Ident, Default: WideString): WideString; override;
    procedure Rename(const FileName: WideString; Reload: Boolean);
    procedure SetStrings(List: TTntStrings);
    procedure UpdateFile; override;
    procedure WriteString(const Section, Ident, Value: WideString); override;
    property CaseSensitive: Boolean read GetCaseSensitive write SetCaseSensitive;
  end;

{$IFDEF MSWINDOWS}
  { TTntIniFile - Encapsulates the Windows INI file interface
    (Get/SetPrivateProfileXXX functions) }

  TTntIniFile = class(TTntCustomIniFile)
  private
    FAnsiIniFile: TIniFile; // For compatibility with Windows 95/98/Me
  public
    constructor Create(const FileName: WideString); virtual;
    destructor Destroy; override;
    function ReadString(const Section, Ident, Default: WideString): WideString; override;
    procedure WriteString(const Section, Ident, Value: WideString); override;
    procedure ReadSection(const Section: WideString; Strings: TTntStrings); override;
    procedure ReadSections(Strings: TTntStrings); override;
    procedure ReadSectionValues(const Section: WideString; Strings: TTntStrings); override;
    procedure EraseSection(const Section: WideString); override;
    procedure DeleteKey(const Section, Ident: WideString); override;
    procedure UpdateFile; override;
  end;
{$ELSE}
  TTntIniFile = class(TTntMemIniFile)
  public
    destructor Destroy; override;
  end;
{$ENDIF}


implementation

uses                                                    
  RTLConsts, SysUtils, TntSysUtils
{$IFDEF COMPILER_9_UP} , WideStrUtils {$ELSE} , TntWideStrUtils {$ENDIF}
{$IFDEF MSWINDOWS}     , Windows      {$ENDIF};

{ TTntCustomIniFile }

constructor TTntCustomIniFile.Create(const FileName: WideString);
begin
  FFileName := FileName;
end;

function TTntCustomIniFile.SectionExists(const Section: WideString): Boolean;
var
  S: TTntStrings;
begin
  S := TTntStringList.Create;
  try
    ReadSection(Section, S);
    Result := S.Count > 0;
  finally
    S.Free;
  end;
end;

function TTntCustomIniFile.ReadInteger(const Section, Ident: WideString;
  Default: Longint): Longint;
var
  IntStr: WideString;
begin
  IntStr := ReadString(Section, Ident, '');
  if (Length(IntStr) > 2) and (IntStr[1] = WideChar('0')) and
    ((IntStr[2] = WideChar('X')) or (IntStr[2] = WideChar('x'))) then
    IntStr := WideString('$') + Copy(IntStr, 3, Maxint);
  Result := StrToIntDef(IntStr, Default);
end;

procedure TTntCustomIniFile.WriteInteger(const Section, Ident: WideString; Value: Longint);
begin
  WriteString(Section, Ident, IntToStr(Value));
end;

function TTntCustomIniFile.ReadBool(const Section, Ident: WideString;
  Default: Boolean): Boolean;
begin
  Result := ReadInteger(Section, Ident, Ord(Default)) <> 0;
end;

function TTntCustomIniFile.ReadDate(const Section, Name: WideString; Default: TDateTime): TDateTime;
var
  DateStr: WideString;
begin
  DateStr := ReadString(Section, Name, '');
  Result := Default;
  if DateStr <> '' then
  try
    Result := StrToDate(DateStr);
  except
    on EConvertError do
    else raise;
  end;
end;

function TTntCustomIniFile.ReadDateTime(const Section, Name: WideString; Default: TDateTime): TDateTime;
var
  DateStr: WideString;
begin
  DateStr := ReadString(Section, Name, '');
  Result := Default;
  if DateStr <> '' then
  try
    Result := StrToDateTime(DateStr);
  except
    on EConvertError do
    else raise;
  end;
end;

function TTntCustomIniFile.ReadFloat(const Section, Name: WideString; Default: Double): Double;
var
  FloatStr: WideString;
begin
  FloatStr := ReadString(Section, Name, '');
  Result := Default;
  if FloatStr <> '' then
  try
    Result := StrToFloat(FloatStr);
  except
    on EConvertError do
    else raise;
  end;
end;

function TTntCustomIniFile.ReadTime(const Section, Name: WideString; Default: TDateTime): TDateTime;
var
  TimeStr: WideString;
begin
  TimeStr := ReadString(Section, Name, '');
  Result := Default;
  if TimeStr <> '' then
  try
    Result := StrToTime(TimeStr);
  except
    on EConvertError do
    else raise;
  end;
end;

procedure TTntCustomIniFile.WriteDate(const Section, Name: WideString; Value: TDateTime);
begin
  WriteString(Section, Name, DateToStr(Value));
end;

procedure TTntCustomIniFile.WriteDateTime(const Section, Name: WideString; Value: TDateTime);
begin
  WriteString(Section, Name, DateTimeToStr(Value));
end;

procedure TTntCustomIniFile.WriteFloat(const Section, Name: WideString; Value: Double);
begin
  WriteString(Section, Name, FloatToStr(Value));
end;

procedure TTntCustomIniFile.WriteTime(const Section, Name: WideString; Value: TDateTime);
begin
  WriteString(Section, Name, TimeToStr(Value));
end;

procedure TTntCustomIniFile.WriteBool(const Section, Ident: WideString; Value: Boolean);
const
  Values: array[Boolean] of WideString = ('0', '1');
begin
  WriteString(Section, Ident, Values[Value]);
end;

function TTntCustomIniFile.ValueExists(const Section, Ident: WideString): Boolean;
var
  S: TTntStrings;
begin
  S := TTntStringList.Create;
  try
    ReadSection(Section, S);
    Result := S.IndexOf(Ident) > -1;
  finally
    S.Free;
  end;
end;

function TTntCustomIniFile.ReadBinaryStream(const Section, Name: WideString;
  Value: TStream): Integer;
var
  Text: String; // Not Unicode: Due to HexToBin is not Unicode
  Stream: TMemoryStream;
  Pos: Integer;
begin
  Text := ReadString(Section, Name, '');
  if Text <> '' then
  begin
    if Value is TMemoryStream then
      Stream := TMemoryStream(Value)
    else Stream := TMemoryStream.Create;
    try
      Pos := Stream.Position;
      Stream.SetSize(Stream.Size + Length(Text) div 2);
      HexToBin(PChar(Text), PChar(Integer(Stream.Memory) + Stream.Position), Length(Text) div 2);
      Stream.Position := Pos;
      if Value <> Stream then Value.CopyFrom(Stream, Length(Text) div 2);
      Result := Stream.Size - Pos;
    finally
      if Value <> Stream then Stream.Free;
    end;
  end else Result := 0;
end;

procedure TTntCustomIniFile.WriteBinaryStream(const Section, Name: WideString;
  Value: TStream);
var
  Text: string; // Not Unicode: Due to BinToHex is not Unicode
  Stream: TMemoryStream;
begin
  SetLength(Text, (Value.Size - Value.Position) * 2);
  if Length(Text) > 0 then
  begin
    if Value is TMemoryStream then
      Stream := TMemoryStream(Value)
    else Stream := TMemoryStream.Create;
    try
      if Stream <> Value then
      begin
        Stream.CopyFrom(Value, Value.Size - Value.Position);
        Stream.Position := 0;
      end;
      BinToHex(PChar(Integer(Stream.Memory) + Stream.Position), PChar(Text),
        Stream.Size - Stream.Position);
    finally
      if Value <> Stream then Stream.Free;
    end;
  end;
  WriteString(Section, Name, Text);
end;

{ TTntStringHash }

procedure TTntStringHash.Add(const Key: WideString; Value: Integer);
var
  Hash: Integer;
  Bucket: PTntHashItem;
begin
  Hash := HashOf(Key) mod Cardinal(Length(Buckets));
  New(Bucket);
  Bucket^.Key := Key;
  Bucket^.Value := Value;
  Bucket^.Next := Buckets[Hash];
  Buckets[Hash] := Bucket;
end;

procedure TTntStringHash.Clear;
var
  I: Integer;
  P, N: PTntHashItem;
begin
  for I := 0 to Length(Buckets) - 1 do
  begin
    P := Buckets[I];
    while P <> nil do
    begin
      N := P^.Next;
      Dispose(P);
      P := N;
    end;
    Buckets[I] := nil;
  end;
end;

constructor TTntStringHash.Create(Size: Integer);
begin
  inherited Create;
  SetLength(Buckets, Size);
end;

destructor TTntStringHash.Destroy;
begin
  Clear;
  inherited;
end;

function TTntStringHash.Find(const Key: WideString): PPTntHashItem;
var
  Hash: Integer;
begin
  Hash := HashOf(Key) mod Cardinal(Length(Buckets));
  Result := @Buckets[Hash];
  while Result^ <> nil do
  begin
    if Result^.Key = Key then
      Exit
    else
      Result := @Result^.Next;
  end;
end;

function TTntStringHash.HashOf(const Key: WideString): Cardinal;
var
  I: Integer;
begin
  Result := 0;
  for I := 1 to Length(Key) do
    Result := ((Result shl 2) or (Result shr (SizeOf(Result) * 8 - 2))) xor
      Ord(Key[I]); // Is it OK for WideChar?
end;

function TTntStringHash.Modify(const Key: WideString; Value: Integer): Boolean;
var
  P: PTntHashItem;
begin
  P := Find(Key)^;
  if P <> nil then
  begin
    Result := True;
    P^.Value := Value;
  end
  else
    Result := False;
end;

procedure TTntStringHash.Remove(const Key: WideString);
var
  P: PTntHashItem;
  Prev: PPTntHashItem;
begin
  Prev := Find(Key);
  P := Prev^;
  if P <> nil then
  begin
    Prev^ := P^.Next;
    Dispose(P);
  end;
end;

function TTntStringHash.ValueOf(const Key: WideString): Integer;
var
  P: PTntHashItem;
begin
  P := Find(Key)^;
  if P <> nil then
    Result := P^.Value else
    Result := -1;
end;

{ TTntHashedStringList }

procedure TTntHashedStringList.Changed;
begin
  inherited;
  FValueHashValid := False;
  FNameHashValid := False;
end;

destructor TTntHashedStringList.Destroy;
begin
  FValueHash.Free;
  FNameHash.Free;
  inherited;
end;

function TTntHashedStringList.IndexOf(const S: WideString): Integer;
begin
  UpdateValueHash;
  if not CaseSensitive then
    Result :=  FValueHash.ValueOf(WideUpperCase(S))
  else
    Result :=  FValueHash.ValueOf(S);
end;

function TTntHashedStringList.IndexOfName(const Name: WideString): Integer;
begin
  UpdateNameHash;
  if not CaseSensitive then
    Result := FNameHash.ValueOf(WideUpperCase(Name))
  else
    Result := FNameHash.ValueOf(Name);
end;

procedure TTntHashedStringList.UpdateNameHash;
var
  I: Integer;
  P: Integer;
  Key: WideString;
begin
  if FNameHashValid then Exit;
  if FNameHash = nil then
    FNameHash := TTntStringHash.Create
  else
    FNameHash.Clear;
  for I := 0 to Count - 1 do
  begin
    Key := Get(I);
    P := Pos(NameValueSeparator, Key);
    if P <> 0 then
    begin
      if not CaseSensitive then
        Key := WideUpperCase(Copy(Key, 1, P - 1))
      else
        Key := Copy(Key, 1, P - 1);
      FNameHash.Add(Key, I);
    end;
  end;
  FNameHashValid := True;
end;

procedure TTntHashedStringList.UpdateValueHash;
var
  I: Integer;
begin
  if FValueHashValid then Exit;
  if FValueHash = nil then
    FValueHash := TTntStringHash.Create
  else
    FValueHash.Clear;
  for I := 0 to Count - 1 do
    if not CaseSensitive then
      FValueHash.Add(WideUpperCase(Self[I]), I)
    else
      FValueHash.Add(Self[I], I);
  FValueHashValid := True;
end;

{ TTntMemIniFile }

constructor TTntMemIniFile.Create(const FileName: WideString);
begin
  inherited Create(FileName);
  FSections := TTntHashedStringList.Create;
  FSections.NameValueSeparator := '=';
{$IFDEF LINUX}
  FSections.CaseSensitive := True;
{$ELSE}
  FSections.CaseSensitive := False;
{$ENDIF}
  LoadValues;
end;

destructor TTntMemIniFile.Destroy;
begin
  if FSections <> nil then Clear;
  FSections.Free;
  inherited;
end;

function TTntMemIniFile.AddSection(const Section: WideString): TTntStrings;
begin
  Result := TTntHashedStringList.Create;
  try
    TTntHashedStringList(Result).CaseSensitive := CaseSensitive;
    FSections.AddObject(Section, Result);
  except
    Result.Free;
    raise;
  end;
end;

procedure TTntMemIniFile.Clear;
var
  I: Integer;
begin
  for I := 0 to FSections.Count - 1 do
    TObject(FSections.Objects[I]).Free;
  FSections.Clear;
end;

procedure TTntMemIniFile.DeleteKey(const Section, Ident: WideString);
var
  I, J: Integer;
  Strings: TTntStrings;
begin
  I := FSections.IndexOf(Section);
  if I >= 0 then
  begin
    Strings := TTntStrings(FSections.Objects[I]);
    J := Strings.IndexOfName(Ident);
    if J >= 0 then Strings.Delete(J);
  end;
end;

procedure TTntMemIniFile.EraseSection(const Section: WideString);
var
  I: Integer;
begin
  I := FSections.IndexOf(Section);
  if I >= 0 then
  begin
    TStrings(FSections.Objects[I]).Free;
    FSections.Delete(I);
  end;
end;

function TTntMemIniFile.GetCaseSensitive: Boolean;
begin
  Result := FSections.CaseSensitive;
end;

procedure TTntMemIniFile.GetStrings(List: TTntStrings);
var
  I, J: Integer;
  Strings: TTntStrings;
begin
  List.BeginUpdate;
  try
    for I := 0 to FSections.Count - 1 do
    begin
      List.Add('[' + FSections[I] + ']');
      Strings := TTntStrings(FSections.Objects[I]);
      for J := 0 to Strings.Count - 1 do
        List.Add(Strings[J]);
      List.Add('');
    end;
  finally
    List.EndUpdate;
  end;
end;

procedure TTntMemIniFile.LoadValues;
var
  List: TTntStringList;
begin
  if (FileName <> '') and WideFileExists(FileName) then
  begin
    List := TTntStringList.Create;
    try
      List.LoadFromFile(FileName);
      SetStrings(List);
    finally
      List.Free;
    end;
  end else
    Clear;
end;

procedure TTntMemIniFile.ReadSection(const Section: WideString;
  Strings: TTntStrings);
var
  I, J: Integer;
  SectionStrings: TTntStrings;
begin
  Strings.BeginUpdate;
  try
    Strings.Clear;
    I := FSections.IndexOf(Section);
    if I >= 0 then
    begin
      SectionStrings := TTntStrings(FSections.Objects[I]);
      for J := 0 to SectionStrings.Count - 1 do
        Strings.Add(SectionStrings.Names[J]);
    end;
  finally
    Strings.EndUpdate;
  end;
end;

procedure TTntMemIniFile.ReadSections(Strings: TTntStrings);
begin
  Strings.Assign(FSections);
end;

procedure TTntMemIniFile.ReadSectionValues(const Section: WideString;
  Strings: TTntStrings);
var
  I: Integer;
begin
  Strings.BeginUpdate;
  try
    Strings.Clear;
    I := FSections.IndexOf(Section);
    if I >= 0 then Strings.Assign(TTntStrings(FSections.Objects[I]));
  finally
    Strings.EndUpdate;
  end;
end;

function TTntMemIniFile.ReadString(const Section, Ident,
  Default: WideString): WideString;
var
  I: Integer;
  Strings: TTntStrings;
begin
  I := FSections.IndexOf(Section);
  if I >= 0 then
  begin
    Strings := TTntStrings(FSections.Objects[I]);
    I := Strings.IndexOfName(Ident);
    if I >= 0 then
    begin
      Result := Copy(Strings[I], Length(Ident) + 2, Maxint);
      Exit;
    end;
  end;
  Result := Default;
end;

procedure TTntMemIniFile.Rename(const FileName: WideString; Reload: Boolean);
begin
  FFileName := FileName;
  if Reload then LoadValues;
end;

procedure TTntMemIniFile.SetCaseSensitive(Value: Boolean);
var
  I: Integer;
begin
  if Value <> FSections.CaseSensitive then
  begin
    FSections.CaseSensitive := Value;
    for I := 0 to FSections.Count - 1 do
      with TTntHashedStringList(FSections.Objects[I]) do
      begin
        CaseSensitive := Value;
        Changed;
      end;
    TTntHashedStringList(FSections).Changed;
  end;
end;

procedure TTntMemIniFile.SetStrings(List: TTntStrings);
var
  I, J: Integer;
  S: WideString;
  Strings: TTntStrings;
begin
  Clear;
  Strings := nil;
  for I := 0 to List.Count - 1 do
  begin
    S := Trim(List[I]);
    if (S <> '') and (S[1] <> ';') then
      if (S[1] = '[') and (S[Length(S)] = ']') then
      begin
        Delete(S, 1, 1);
        SetLength(S, Length(S)-1);
        Strings := AddSection(Trim(S));
      end
      else
        if Strings <> nil then
        begin
          J := Pos(FSections.NameValueSeparator, S);
          if J > 0 then // remove spaces before and after NameValueSeparator
            Strings.Add(Trim(Copy(S, 1, J-1)) + FSections.NameValueSeparator + TrimRight(Copy(S, J+1, MaxInt)) )
          else
            Strings.Add(S);
        end;
  end;
end;

procedure TTntMemIniFile.UpdateFile;
var
  List: TTntStringList;
begin
  List := TTntStringList.Create;
  try
    GetStrings(List);
    List.SaveToFile(FFileName);
  finally
    List.Free;
  end;
end;

procedure TTntMemIniFile.WriteString(const Section, Ident, Value: WideString);
var
  I: Integer;
  S: WideString;
  Strings: TTntStrings;
begin
  I := FSections.IndexOf(Section);
  if I >= 0 then
    Strings := TTntStrings(FSections.Objects[I]) else
    Strings := AddSection(Section);
  S := Ident + FSections.NameValueSeparator + Value;
  I := Strings.IndexOfName(Ident);
  if I >= 0 then Strings[I] := S else Strings.Add(S);
end;



{$IFDEF MSWINDOWS}
{ TTntIniFile }

constructor TTntIniFile.Create(const FileName: WideString);
begin
  inherited Create(FileName);
  if (not Win32PlatformIsUnicode) then
    FAnsiIniFile := TIniFile.Create(FileName);
end;

destructor TTntIniFile.Destroy;
begin
  UpdateFile; // flush changes to disk
  if (not Win32PlatformIsUnicode) then
    FAnsiIniFile.Free;
  inherited Destroy;
end;

function TTntIniFile.ReadString(const Section, Ident, Default: WideString): WideString;
var
  Buffer: array[0..2047] of WideChar;
begin
  if (not Win32PlatformIsUnicode) then
    { Windows 95/98/Me }
    Result := FAnsiIniFile.ReadString(Section, Ident, Default)
  else begin
    { Windows NT/2000/XP and later }
    GetPrivateProfileStringW(PWideChar(Section),
      PWideChar(Ident), PWideChar(Default), Buffer, Length(Buffer), PWideChar(FFileName));
    Result := WideString(Buffer);
  end;
end;

procedure TTntIniFile.WriteString(const Section, Ident, Value: WideString);
begin
  if (not Win32PlatformIsUnicode) then
    { Windows 95/98/Me }
    FAnsiIniFile.WriteString(Section, Ident, Value)
  else begin
    { Windows NT/2000/XP and later }
    if not WritePrivateProfileStringW(PWideChar(Section), PWideChar(Ident),
      PWideChar(Value), PWideChar(FFileName)) then
      raise EIniFileException.CreateResFmt(@SIniFileWriteError, [FileName]);
  end;
end;

procedure TTntIniFile.ReadSections(Strings: TTntStrings);
const
  BufSize = 16384 * SizeOf(WideChar);
var
  Buffer, P: PWideChar;
begin
  if (not Win32PlatformIsUnicode) then
  begin
    { Windows 95/98/Me }
    FAnsiIniFile.ReadSections(Strings.AnsiStrings);
  end else
  begin
    { Windows NT/2000/XP and later }
    GetMem(Buffer, BufSize);
    try
      Strings.BeginUpdate;
      try
        Strings.Clear;
        if GetPrivateProfileStringW(nil, nil, nil, Buffer, BufSize,
          PWideChar(FFileName)) <> 0 then
        begin
          P := Buffer;
          while P^ <> WideChar(#0) do
          begin
            Strings.Add(P);
            Inc(P, WStrLen(P) + 1);
          end;
        end;
      finally
        Strings.EndUpdate;
      end;
    finally
      FreeMem(Buffer, BufSize);
    end;
  end; {else}
end;

procedure TTntIniFile.ReadSection(const Section: WideString; Strings: TTntStrings);
const
  BufSize = 16384 * SizeOf(WideChar);
var
  Buffer, P: PWideChar;
begin
  if (not Win32PlatformIsUnicode) then
  begin
    { Windows 95/98/Me }
    FAnsiIniFile.ReadSection(Section, Strings.AnsiStrings);
  end else
  begin
    { Windows NT/2000/XP and later }
    GetMem(Buffer, BufSize);
    try
      Strings.BeginUpdate;
      try
        Strings.Clear;
        if GetPrivateProfileStringW(PWideChar(Section), nil, nil, Buffer, BufSize,
          PWideChar(FFileName)) <> 0 then
        begin
          P := Buffer;
          while P^ <> #0 do
          begin
            Strings.Add(P);
            Inc(P, WStrLen(P) + 1);
          end;
        end;
      finally
        Strings.EndUpdate;
      end;
    finally
      FreeMem(Buffer, BufSize);
    end;
  end;
end;

procedure TTntIniFile.ReadSectionValues(const Section: WideString; Strings: TTntStrings);
var
  KeyList: TTntStringList;
  I: Integer;
begin
  if (not Win32PlatformIsUnicode) then
  begin
    { Windows 95/98/Me }
    FAnsiIniFile.ReadSectionValues(Section, Strings.AnsiStrings);
  end else
  begin
    { Windows NT/2000/XP and later }
    KeyList := TTntStringList.Create;
    try
      ReadSection(Section, KeyList);
      Strings.BeginUpdate;
      try
        Strings.Clear;
        for I := 0 to KeyList.Count - 1 do
          Strings.Add(KeyList[I] + '=' + ReadString(Section, KeyList[I], ''))
      finally
        Strings.EndUpdate;
      end;
    finally
      KeyList.Free;
    end;
  end; {if}
end;

procedure TTntIniFile.EraseSection(const Section: WideString);
begin
  if (not Win32PlatformIsUnicode) then
  begin
    { Windows 95/98/Me }
    FAnsiIniFile.EraseSection(Section);
  end
  else begin
    { Windows NT/2000/XP and later }
    if not WritePrivateProfileStringW(PWideChar(Section), nil, nil,
      PWideChar(FFileName)) then
      raise EIniFileException.CreateResFmt(@SIniFileWriteError, [FileName]);
  end; {if}
end;

procedure TTntIniFile.DeleteKey(const Section, Ident: WideString);
begin
  if (not Win32PlatformIsUnicode) then
  begin
    { Windows 95/98/Me }
    FAnsiIniFile.DeleteKey(Section, Ident);
  end
  else begin
    { Windows NT/2000/XP and later }
    WritePrivateProfileStringW(PWideChar(Section), PWideChar(Ident), nil,
      PWideChar(FFileName));
  end; {if}
end;

procedure TTntIniFile.UpdateFile;
begin
  if (not Win32PlatformIsUnicode) then
  begin
    { Windows 95/98/Me }
    FAnsiIniFile.UpdateFile
  end
  else begin
    { Windows NT/2000/XP and later }
    WritePrivateProfileStringW(nil, nil, nil, PWideChar(FFileName));
  end; {if}
end;

{$ELSE}

destructor TTntIniFile.Destroy;
begin
  UpdateFile;
  inherited Destroy;
end;

{$ENDIF}




end.

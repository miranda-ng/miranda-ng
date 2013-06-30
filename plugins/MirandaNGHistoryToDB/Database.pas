{ ################################################################################ }
{ #                                                                              # }
{ #  MirandaNG HistoryToDB Plugin v2.5                                           # }
{ #                                                                              # }
{ #  License: GPLv3                                                              # }
{ #                                                                              # }
{ #  Author: Grigorev Michael (icq: 161867489, email: sleuthhound@gmail.com)     # }
{ #                                                                              # }
{ ################################################################################ }

{ ################################################################################ }
{ #                                                                              # }
{ # History++ plugin for Miranda IM: the free IM client for Microsoft* Windows*  # }
{ #                                                                              # }
{ # Copyright (C) 2006-2009 theMIROn, 2003-2006 Art Fedorov.                     # }
{ # History+ parts (C) 2001 Christian Kastner                                    # }
{ #                                                                              # }
{ # This program is free software; you can redistribute it and/or modify         # }
{ # it under the terms of the GNU General Public License as published by         # }
{ # the Free Software Foundation; either version 2 of the License, or            # }
{ # (at your option) any later version.                                          # }
{ #                                                                              # }
{ # This program is distributed in the hope that it will be useful,              # }
{ # but WITHOUT ANY WARRANTY; without even the implied warranty of               # }
{ # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                # }
{ # GNU General Public License for more details.                                 # }
{ #                                                                              # }
{ # You should have received a copy of the GNU General Public License            # }
{ # along with this program; if not, write to the Free Software                  # }
{ # Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    # }
{ #                                                                              # }
{ ################################################################################ }

unit Database;

interface

uses m_api, windows, global;

procedure SetSafetyMode(Safe: Boolean);

function DBGetContactSettingString(hContact: THandle; const szModule: PAnsiChar; const szSetting: PAnsiChar; ErrorValue: PAnsiChar): AnsiString;
function DBGetContactSettingWideString(hContact: THandle; const szModule: PAnsiChar; const szSetting: PAnsiChar; ErrorValue: PWideChar): WideString;
function DBWriteContactSettingWideString(hContact: THandle; const szModule: PAnsiChar; const szSetting: PAnsiChar; const val: PWideChar): Integer;

function DBDelete(const Module, Param: AnsiString): Boolean; overload;
function DBDelete(const hContact: THandle; const Module, Param: AnsiString): Boolean; overload;
function DBExists(const Module, Param: AnsiString): Boolean; overload;
function DBExists(const hContact: THandle; const Module, Param: AnsiString): Boolean; overload;

function GetDBBlob(const Module,Param: AnsiString; var Value: Pointer; var Size: Integer): Boolean; overload;
function GetDBBlob(const hContact: THandle; const Module,Param: AnsiString; var Value: Pointer; var Size: Integer): Boolean; overload;
function GetDBStr(const Module,Param: AnsiString; Default: AnsiString): AnsiString; overload;
function GetDBStr(const hContact: THandle; const Module,Param: AnsiString; Default: AnsiString): AnsiString; overload;
function GetDBWideStr(const Module,Param: AnsiString; Default: WideString): WideString; overload;
function GetDBWideStr(const hContact: THandle; const Module,Param: AnsiString; Default: WideString): WideString; overload;
function GetDBInt(const Module,Param: AnsiString; Default: Integer): Integer; overload;
function GetDBInt(const hContact: THandle; const Module,Param: AnsiString; Default: Integer): Integer; overload;
function GetDBWord(const Module,Param: AnsiString; Default: Word): Word; overload;
function GetDBWord(const hContact: THandle; const Module,Param: AnsiString; Default: Word): Word; overload;
function GetDBDWord(const Module,Param: AnsiString; Default: DWord): DWord; overload;
function GetDBDWord(const hContact: THandle; const Module,Param: AnsiString; Default: DWord): DWord; overload;
function GetDBByte(const Module,Param: AnsiString; Default: Byte): Byte; overload;
function GetDBByte(const hContact: THandle; const Module,Param: AnsiString; Default: Byte): Byte; overload;
function GetDBBool(const Module,Param: AnsiString; Default: Boolean): Boolean; overload;
function GetDBBool(const hContact: THandle; const Module,Param: AnsiString; Default: Boolean): Boolean; overload;
function GetDBDateTime(const hContact: THandle; const Module,Param: AnsiString; Default: TDateTime): TDateTime; overload;
function GetDBDateTime(const Module,Param: AnsiString; Default: TDateTime): TDateTime; overload;

function WriteDBBlob(const Module,Param: AnsiString; Value: Pointer; Size: Integer): Integer; overload;
function WriteDBBlob(const hContact: THandle; const Module,Param: AnsiString; Value: Pointer; Size: Integer): Integer; overload;
function WriteDBByte(const Module,Param: AnsiString; Value: Byte): Integer; overload;
function WriteDBByte(const hContact: THandle; const Module,Param: AnsiString; Value: Byte): Integer; overload;
function WriteDBWord(const Module,Param: AnsiString; Value: Word): Integer; overload;
function WriteDBWord(const hContact: THandle; const Module,Param: AnsiString; Value: Word): Integer; overload;
function WriteDBDWord(const Module,Param: AnsiString; Value: DWord): Integer; overload;
function WriteDBDWord(const hContact: THandle; const Module,Param: AnsiString; Value: DWord): Integer; overload;
function WriteDBInt(const Module,Param: AnsiString; Value: Integer): Integer; overload;
function WriteDBInt(const hContact: THandle; const Module,Param: AnsiString; Value: Integer): Integer; overload;
function WriteDBStr(const Module,Param: AnsiString; Value: AnsiString): Integer; overload;
function WriteDBStr(const hContact: THandle; const Module,Param: AnsiString; Value: AnsiString): Integer; overload;
function WriteDBWideStr(const Module,Param: AnsiString; Value: WideString): Integer; overload;
function WriteDBWideStr(const hContact: THandle; const Module,Param: AnsiString; Value: WideString): Integer; overload;
function WriteDBBool(const Module,Param: AnsiString; Value: Boolean): Integer; overload;
function WriteDBBool(const hContact: THandle; const Module,Param: AnsiString; Value: Boolean): Integer; overload;
function WriteDBDateTime(const hContact: THandle; const Module,Param: AnsiString; Value: TDateTime): Integer; overload;
function WriteDBDateTime(const Module,Param: AnsiString; Value: TDateTime): Integer; overload;

implementation

procedure SetSafetyMode(Safe: Boolean);
begin
  CallService(MS_DB_SETSAFETYMODE,WPARAM(Safe),0);
end;

function DBExists(const Module, Param: AnsiString): Boolean;
begin
  Result := DBExists(0,Module,Param);
end;

function DBExists(const hContact: THandle; const Module, Param: AnsiString): Boolean;
var
  dbv: TDBVARIANT;
begin
  Result := (db_get(hContact, PAnsiChar(Module), PAnsiChar(Param), @dbv) = 0);
  if Result then
    DBFreeVariant(@dbv);
end;

function GetDBBlob(const Module,Param: AnsiString; var Value: Pointer; var Size: Integer): Boolean;
begin
  Result := GetDBBlob(0,Module,Param,Value,Size);
end;

function GetDBBlob(const hContact: THandle; const Module,Param: AnsiString; var Value: Pointer; var Size: Integer): Boolean;
var
  dbv: TDBVARIANT;
begin
  Result := False;
  if db_get(hContact, PAnsiChar(Module), PAnsiChar(Param), @dbv) <> 0 then exit;
  Size := dbv.cpbVal;
  Value := nil;
  if dbv.cpbVal = 0 then exit;
  GetMem(Value,dbv.cpbVal);
  Move(dbv.pbVal^,PByte(Value)^,dbv.cpbVal);
  DBFreeVariant(@dbv);
  Result := True;
end;

function GetDBBool(const Module,Param: AnsiString; Default: Boolean): Boolean;
begin
  Result := GetDBBool(0,Module,Param,Default);
end;

function GetDBBool(const hContact: THandle; const Module,Param: AnsiString; Default: Boolean): Boolean;
begin
  Result := Boolean(GetDBByte(hContact,Module,Param,Byte(Default)));
end;

function GetDBByte(const Module,Param: AnsiString; Default: Byte): Byte;
begin
  Result := GetDBByte(0,Module,Param,Default);
end;

function GetDBByte(const hContact: THandle; const Module,Param: AnsiString; Default: Byte): Byte;
begin
  Result := DBGetContactSettingByte(hContact,PAnsiChar(Module),PAnsiChar(Param),Default);
end;

function GetDBWord(const Module,Param: AnsiString; Default: Word): Word;
begin
  Result := GetDBWord(0,Module,Param,Default);
end;

function GetDBWord(const hContact: THandle; const Module,Param: AnsiString; Default: Word): Word;
begin
  Result := DBGetContactSettingWord(hContact,PAnsiChar(Module),PAnsiChar(Param),Default);
end;

function GetDBDWord(const Module,Param: AnsiString; Default: DWord): DWord;
begin
  Result := GetDBDWord(0,Module,Param,Default);
end;

function GetDBDWord(const hContact: THandle; const Module,Param: AnsiString; Default: DWord): DWord;
begin
  Result := DBGetContactSettingDWord(hContact,PAnsiChar(Module),PAnsiChar(Param),Default);
end;

function GetDBInt(const Module,Param: AnsiString; Default: Integer): Integer;
begin
  Result := GetDBInt(0,Module,Param,Default);
end;

function GetDBInt(const hContact: THandle; const Module,Param: AnsiString; Default: Integer): Integer;
var
  dbv:TDBVariant;
begin
  dbv._type := DBVT_DWORD;
  dbv.dVal:=Default;
  if db_get(hContact, PAnsiChar(Module), PAnsiChar(Param), @dbv)<>0 then
    Result:=default
  else
    Result:=dbv.dval;
end;

function GetDBStr(const Module,Param: AnsiString; Default: AnsiString): AnsiString;
begin
  Result := GetDBStr(0,Module,Param,Default);
end;

function GetDBStr(const hContact: THandle; const Module,Param: AnsiString; Default: AnsiString): AnsiString;
begin
  Result := DBGetContactSettingString(hContact,PAnsiChar(Module),PAnsiChar(Param),PAnsiChar(Default));
end;

function DBGetContactSettingString(hContact: THandle; const szModule: PAnsiChar; const szSetting: PAnsiChar; ErrorValue: PAnsiChar): AnsiString;
var
  dbv: TDBVARIANT;
  tmp: WideString;
begin
  if db_get(hContact, szModule, szSetting, @dbv) <> 0 then
    Result := ErrorValue
  else begin
    case dbv._type of
      DBVT_ASCIIZ:
        Result := AnsiString(dbv.szVal.a);
      DBVT_UTF8: begin
        tmp := AnsiToWideString(dbv.szVal.a,CP_UTF8);
	      Result := WideToAnsiString(tmp,hppCodepage);
        end;
      DBVT_WCHAR:
        Result := WideToAnsiString(dbv.szVal.w,hppCodepage);
    end;
    // free variant
    DBFreeVariant(@dbv);
  end;
end;

function GetDBWideStr(const Module,Param: AnsiString; Default: WideString): WideString;
begin
  Result := GetDBWideStr(0,Module,Param,Default);
end;

function GetDBWideStr(const hContact: THandle; const Module,Param: AnsiString; Default: WideString): WideString;
begin
  Result := DBGetContactSettingWideString(hContact,PAnsiChar(Module),PAnsiChar(Param),PWideChar(Default));
end;

function DBGetContactSettingWideString(hContact: THandle; const szModule: PAnsiChar; const szSetting: PAnsiChar; ErrorValue: PWideChar): WideString;
var
  dbv: TDBVARIANT;
begin
  if db_get(hContact, szModule, szSetting, @dbv) <> 0 then
    Result := ErrorValue
  else begin
    case dbv._type of
      DBVT_ASCIIZ:
        Result := AnsiToWideString(dbv.szVal.a,hppCodepage);
      DBVT_UTF8:
        Result := AnsiToWideString(dbv.szVal.a,CP_UTF8);
      DBVT_WCHAR:
        Result := WideString(dbv.szVal.w);
    end;
    // free variant
    DBFreeVariant(@dbv);
  end;
end;

function GetDBDateTime(const hContact: THandle; const Module,Param: AnsiString; Default: TDateTime): TDateTime; overload;
var
  p: Pointer;
  s: Integer;
begin
  Result := Default;
  if not GetDBBlob(hContact,Module,Param,p,s) then exit;
  if s <> SizeOf(TDateTime) then begin
    FreeMem(p,s);
    exit;
  end;
  Result := PDateTime(p)^;
  FreeMem(p,s);
end;

function GetDBDateTime(const Module,Param: AnsiString; Default: TDateTime): TDateTime; overload;
begin
  Result := GetDBDateTime(0,Module,Param,Default);
end;

function DBDelete(const Module, Param: AnsiString): Boolean;
begin
  Result := DBDelete(0,Module,Param);
end;

function DBDelete(const hContact: THandle; const Module, Param: AnsiString): Boolean;
begin
  Result := (DBDeleteContactSetting(hContact,PAnsiChar(Module),PAnsiChar(Param)) = 0);
end;

function WriteDBBool(const Module,Param: AnsiString; Value: Boolean): Integer;
begin
  Result := WriteDBBool(0,Module,Param,Value);
end;

function WriteDBBool(const hContact: THandle; const Module,Param: AnsiString; Value: Boolean): Integer;
begin
  Result := WriteDBByte(hContact,Module,Param,Byte(Value));
end;

function WriteDBByte(const Module,Param: AnsiString; Value: Byte): Integer;
begin
  Result := WriteDBByte(0,Module,Param,Value);
end;

function WriteDBByte(const hContact: THandle; const Module,Param: AnsiString; Value: Byte): Integer;
begin
  Result := DBWriteContactSettingByte(hContact,PAnsiChar(Module), PAnsiChar(Param), Value);
end;

function WriteDBWord(const Module,Param: AnsiString; Value: Word): Integer;
begin
  Result := WriteDBWord(0,Module,Param,Value);
end;

function WriteDBWord(const hContact: THandle; const Module,Param: AnsiString; Value: Word): Integer;
begin
  Result := DBWriteContactSettingWord(hContact,PAnsiChar(Module),PAnsiChar(Param),Value);
end;

function WriteDBDWord(const Module,Param: AnsiString; Value: DWord): Integer;
begin
  Result := WriteDBWord(0,Module,Param,Value);
end;

function WriteDBDWord(const hContact: THandle; const Module,Param: AnsiString; Value: DWord): Integer;
begin
  Result := DBWriteContactSettingDWord(hContact,PAnsiChar(Module),PAnsiChar(Param),Value);
end;

function WriteDBInt(const Module,Param: AnsiString; Value: Integer): Integer;
begin
  Result := WriteDBInt(0,Module,Param,Value);
end;

function WriteDBInt(const hContact: THandle; const Module,Param: AnsiString; Value: Integer): Integer;
begin
  Result := db_set_dw(hContact, PAnsiChar(Module), PAnsiChar(Param), Value);
end;

function WriteDBStr(const Module,Param: AnsiString; Value: AnsiString): Integer;
begin
  Result := WriteDBStr(0,Module,Param,Value);
end;

function WriteDBStr(const hContact: THandle; const Module,Param: AnsiString; Value: AnsiString): Integer;
begin
  Result := DBWriteContactSettingString(hContact,PAnsiChar(Module),PAnsiChar(Param),PAnsiChar(Value));
end;

function WriteDBWideStr(const Module,Param: AnsiString; Value: WideString): Integer;
begin
  Result := WriteDBWideStr(0,Module,Param,Value);
end;

function WriteDBWideStr(const hContact: THandle; const Module,Param: AnsiString; Value: WideString): Integer;
begin
  Result := DBWriteContactSettingWideString(hContact,PAnsiChar(Module),PAnsiChar(Param),PWideChar(Value));
end;

function DBWriteContactSettingWideString(hContact: THandle; const szModule: PAnsiChar; const szSetting: PAnsiChar; const val: PWideChar): Integer;
var
  cws: TDBCONTACTWRITESETTING;
begin
  cws.szModule := szModule;
  cws.szSetting := szSetting;
  cws.value._type := DBVT_WCHAR;
  cws.value.szVal.w := val;
  Result := CallService(MS_DB_CONTACT_WRITESETTING, hContact, lParam(@cws));
end;

function WriteDBBlob(const Module,Param: AnsiString; Value: Pointer; Size: Integer): Integer;
begin
  Result := WriteDBBlob(0,Module,Param,Value,Size);
end;

function WriteDBBlob(const hContact: THandle; const Module,Param: AnsiString; Value: Pointer; Size: Integer): Integer;
var
  cws: TDBContactWriteSetting;
begin
  ZeroMemory(@cws,SizeOf(cws));
  cws.szModule := PAnsiChar(Module);
  cws.szSetting := PAnsiChar(Param);
  cws.value._type := DBVT_BLOB;
  cws.value.pbVal := Value;
  cws.value.cpbVal := Word(Size);
  Result := CallService(MS_DB_CONTACT_WRITESETTING,hContact,lParam(@cws));
end;

function WriteDBDateTime(const hContact: THandle; const Module,Param: AnsiString; Value: TDateTime): Integer; overload;
var
  p: PDateTime;
begin
  GetMem(p,SizeOf(TDateTime));
  p^ := Value;
  Result := WriteDBBlob(hContact,Module,Param,p,SizeOf(TDateTime));
  FreeMem(p,SizeOf(TDateTime));
end;

function WriteDBDateTime(const Module,Param: AnsiString; Value: TDateTime): Integer; overload;
begin
  Result := WriteDBDateTime(0,Module,Param,Value);
end;

end.

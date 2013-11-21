library athena;

{
  Athena: cryptor module for Miranda SecuredMMAP Database driver
  Copyright 2007-2008 Klyde

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}

uses
  UAthena,
  WIndows;

{$R *.res}

type
  TGenerateKey = function(key: PAnsiChar): PArr; stdcall;
  TFreeKey = procedure(key: PArr); stdcall;
  TEncryptMem = procedure(data: PByte; size: LongWord; key: PArr); stdcall;

  PCryptorInfo = ^TCryptorInfo;
  TCryptorInfo = record
    GenerateKey: TGenerateKey;
    FreeKey: TFreeKey;

    EncryptMem: TEncryptMem;
    DecryptMem: TEncryptMem;

    Name: PAnsiChar;
    Info: PAnsiChar;
    Author: PAnsiChar;
    Site: PAnsiChar;
    Email: PAnsiChar;

    version: dword;

    uid: word;
  end;

var
  Info: TCryptorInfo;


function PLUGIN_MAKE_VERSION(a,b,c,d: Cardinal): integer;
begin
  Result := (a shl 24) or (b shl 16) or (c shl 8) or d;
end;

function GenerateKey(pwd: PAnsiChar): PArr; stdcall;
var
  a: PArr;
begin

  new(a);
  MakeKey(a, 512, pwd);
  result := a;

end;

procedure FreeKey(key: PArr); stdcall;
begin
  FreeMem(key);
end;

procedure EncryptMem(data: PByte; size: LongWord; key: PArr); stdcall;
begin
  if size <= 0 then exit;
  EncryptData(key, data, size);
end;

procedure DecryptMem(data: PByte; size: LongWord; key: PArr); stdcall;
begin
  if size <= 0 then exit;
  DecryptData(key, data, size);
end;

function GetCryptor: PCryptorInfo; stdcall;
begin
  Info.Name := 'Athena';
  Info.Author := 'Klyde';
  Info.Site := 'http://cityopen.ru/forum/journal.php?user=151';
  Info.Email := 'xxxmara@mail.ru';
  Info.Info := 'Secure alghoritm developed in russian universities';

  Info.version := PLUGIN_MAKE_VERSION(0,0,3,0);

  Info.UID := $FEA8;

  Info.GenerateKey := GenerateKey;
  Info.FreeKey := FreeKey;
  Info.EncryptMem := EncryptMem;
  Info.DecryptMem := DecryptMem;

  result := @Info;
end;

exports GetCryptor;

begin
end.

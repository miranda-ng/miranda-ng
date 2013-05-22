{******************************************************************}
{ Вычисление хеш-суммы MD5 }
{ }
{ by delphibase.endimus.com }
{ Зависимости: Windows, SysUtils, Classes }
{ Автор:       Dimka Maslov, mainbox@endimus.ru, ICQ:148442121, Санкт-Петербург }
{ Copyright:   http://www.faqs.org/rfcs/rfc1321.html }
{ Дата:        19 июня 2002 г. }
{******************************************************************}

{******************************************************************}
{ MD5 Hashsum Evaluation Unit For Borland Delphi }
{ }
{ Copyright © 2002 by Dimka Maslov }
{ E-mail: mainbox@endimus.com, }
{ Web-site: http://www.endimus.com }
{ }
{ Derived from the RSA Data Security, Inc. }
{ MD5 Message-Digest Algorithm described in RFC 1321 }
{ http://www.faqs.org/rfcs/rfc1321.html }
{******************************************************************}

unit uMD5;

Interface

Uses Windows, SysUtils, Classes;

Type
{ The TMD5Digest record is the type of results of
 the MD5 hashsum evaluation functions. The contents
 of a record may be used as four 32-bit integer values
 or as an array of 16 bytes }
{ Тип TMD5Digest используется для получения
  результата функций вычисления хеш-суммы.
  Содержимое записи можно использовать
  как набор из 4 целых чисел, или как
  массив из 16 байт }
  PMD5Digest = ^TMD5Digest;

  TMD5Digest = Record
    Case Integer Of
      0: (A, B, C, D: LongInt);
      1: (v: Array[0..15] Of Byte);
  End;

{ The MD5String function evaluates the MD5 hashsum for
 a string. The S parameter specifies a string to
 evaluate hashsum }
// вычисление хеш-суммы для строки
Function MD5String(Const S: String): TMD5Digest;

{ The MD5File function evaluates the MD5 hashsum for
 a file. The FileName parameter specifies the name
 of a file to evaluate hashsum }
// вычисление хеш-суммы для файла
Function MD5File(Const FileName: String): TMD5Digest;

{ The MD5Stream function evaluates the MD5 hashsum for
 a stream. The Stream parameters specifies the
 TStream descendant class object to evaluate hashsum }
// вычисление хеш-суммы для содержиого потока Stream
Function MD5Stream(Const Stream: TStream): TMD5Digest;

{ The MD5Buffer function evaluates the MD5 hashsum for
 any memory buffer. The Buffer parameters specifies a
 buffer to evaluate hashsum. The Size parameter specifies
 the size (in bytes) of a buffer }
// вычисление хеш-суммы для произвольного буфера
Function MD5Buffer(Const Buffer; Size: Integer): TMD5Digest;

{ The MD5DigestToStr function converts the result of
 a hashsum evaluation function into a string of
 hexadecimal digits }
// преобразование хеш-суммы в строку из шестнадцатеричных цифр
Function MD5DigestToStr(Const Digest: TMD5Digest): String;


{ The MD5DigestCompare function compares two
 TMD5Digest record variables. This function returns
 TRUE if parameters are equal or FALSE otherwise }
// сравнение двух хеш-сумм
Function MD5DigestCompare(Const Digest1, Digest2: TMD5Digest): Boolean;

{
Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
}

Implementation

Type
  UINT4 = LongWord;

  PArray4UINT4 = ^TArray4UINT4;
  TArray4UINT4 = Array[0..3] Of UINT4;
  PArray2UINT4 = ^TArray2UINT4;
  TArray2UINT4 = Array[0..1] Of UINT4;
  PArray16Byte = ^TArray16Byte;
  TArray16Byte = Array[0..15] Of Byte;
  PArray64Byte = ^TArray64Byte;
  TArray64Byte = Array[0..63] Of Byte;

  PByteArray = ^TByteArray;
  TByteArray = Array[0..0] Of Byte;

  PUINT4Array = ^TUINT4Array;
  TUINT4Array = Array[0..0] Of UINT4;

  PMD5Context = ^TMD5Context;
  TMD5Context = Record
    state: TArray4UINT4;
    count: TArray2UINT4;
    buffer: TArray64Byte;
  End;

Const
  S11 = 7;
  S12 = 12;
  S13 = 17;
  S14 = 22;
  S21 = 5;
  S22 = 9;
  S23 = 14;
  S24 = 20;
  S31 = 4;
  S32 = 11;
  S33 = 16;
  S34 = 23;
  S41 = 6;
  S42 = 10;
  S43 = 15;
  S44 = 21;

Var
  Padding: TArray64Byte =
  ($80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);


Function _F(x, y, z: UINT4): UINT4;
Begin
  Result := (((x) And (y)) Or ((Not x) And (z)));
End;

Function _G(x, y, z: UINT4): UINT4;
Begin
  Result := (((x) And (z)) Or ((y) And (Not z)));
End;

Function _H(x, y, z: UINT4): UINT4;
Begin
  Result := ((x) Xor (y) Xor (z));
End;

Function _I(x, y, z: UINT4): UINT4;
Begin
  Result := ((y) Xor ((x) Or (Not z)));
End;

Function ROTATE_LEFT(x, n: UINT4): UINT4;
Begin
  Result := (((x) Shl (n)) Or ((x) Shr (32 - (n))));
End;

Procedure FF(Var a: UINT4; b, c, d, x, s, ac: UINT4);
Begin
  a := a + _F(b, c, d) + x + ac;
  a := ROTATE_LEFT(a, s);
  a := a + b;
End;

Procedure GG(Var a: UINT4; b, c, d, x, s, ac: UINT4);
Begin
  a := a + _G(b, c, d) + x + ac;
  a := ROTATE_LEFT(a, s);
  a := a + b;
End;

Procedure HH(Var a: UINT4; b, c, d, x, s, ac: UINT4);
Begin
  a := a + _H(b, c, d) + x + ac;
  a := ROTATE_LEFT(a, s);
  a := a + b;
End;

Procedure II(Var a: UINT4; b, c, d, x, s, ac: UINT4);
Begin
  a := a + _I(b, c, d) + x + ac;
  a := ROTATE_LEFT(a, s);
  a := a + b;
End;

Procedure MD5Encode(Output: PByteArray; Input: PUINT4Array; Len: LongWord);
Var
  i, j: LongWord;
Begin
  j := 0;
  i := 0;
  While j < Len Do Begin
    output[j] := Byte(input[i] And $FF);
    output[j + 1] := Byte((input[i] Shr 8) And $FF);
    output[j + 2] := Byte((input[i] Shr 16) And $FF);
    output[j + 3] := Byte((input[i] Shr 24) And $FF);
    Inc(j, 4);
    Inc(i);
  End;
End;

Procedure MD5Decode(Output: PUINT4Array; Input: PByteArray; Len: LongWord);
Var
  i, j: LongWord;
Begin
  j := 0;
  i := 0;
  While j < Len Do Begin
    Output[i] := UINT4(input[j]) Or (UINT4(input[j + 1]) Shl 8) Or
      (UINT4(input[j + 2]) Shl 16) Or (UINT4(input[j + 3]) Shl 24);
    Inc(j, 4);
    Inc(i);
  End;
End;

Procedure MD5_memcpy(Output: PByteArray; Input: PByteArray; Len: LongWord);
Begin
  Move(Input^, Output^, Len);
End;

Procedure MD5_memset(Output: PByteArray; Value: Integer; Len: LongWord);
Begin
  FillChar(Output^, Len, Byte(Value));
End;

Procedure MD5Transform(State: PArray4UINT4; Buffer: PArray64Byte);
Var
  a, b, c, d: UINT4;
  x: Array[0..15] Of UINT4;
Begin
  a := State[0]; b := State[1]; c := State[2]; d := State[3];
  MD5Decode(PUINT4Array(@x), PByteArray(Buffer), 64);

  FF(a, b, c, d, x[0], S11, $D76AA478);
  FF(d, a, b, c, x[1], S12, $E8C7B756);
  FF(c, d, a, b, x[2], S13, $242070DB);
  FF(b, c, d, a, x[3], S14, $C1BDCEEE);
  FF(a, b, c, d, x[4], S11, $F57C0FAF);
  FF(d, a, b, c, x[5], S12, $4787C62A);
  FF(c, d, a, b, x[6], S13, $A8304613);
  FF(b, c, d, a, x[7], S14, $FD469501);
  FF(a, b, c, d, x[8], S11, $698098D8);
  FF(d, a, b, c, x[9], S12, $8B44F7AF);
  FF(c, d, a, b, x[10], S13, $FFFF5BB1);
  FF(b, c, d, a, x[11], S14, $895CD7BE);
  FF(a, b, c, d, x[12], S11, $6B901122);
  FF(d, a, b, c, x[13], S12, $FD987193);
  FF(c, d, a, b, x[14], S13, $A679438E);
  FF(b, c, d, a, x[15], S14, $49B40821);

  GG(a, b, c, d, x[1], S21, $F61E2562);
  GG(d, a, b, c, x[6], S22, $C040B340);
  GG(c, d, a, b, x[11], S23, $265E5A51);
  GG(b, c, d, a, x[0], S24, $E9B6C7AA);
  GG(a, b, c, d, x[5], S21, $D62F105D);
  GG(d, a, b, c, x[10], S22, $2441453);
  GG(c, d, a, b, x[15], S23, $D8A1E681);
  GG(b, c, d, a, x[4], S24, $E7D3FBC8);
  GG(a, b, c, d, x[9], S21, $21E1CDE6);
  GG(d, a, b, c, x[14], S22, $C33707D6);
  GG(c, d, a, b, x[3], S23, $F4D50D87);

  GG(b, c, d, a, x[8], S24, $455A14ED);
  GG(a, b, c, d, x[13], S21, $A9E3E905);
  GG(d, a, b, c, x[2], S22, $FCEFA3F8);
  GG(c, d, a, b, x[7], S23, $676F02D9);
  GG(b, c, d, a, x[12], S24, $8D2A4C8A);

  HH(a, b, c, d, x[5], S31, $FFFA3942);
  HH(d, a, b, c, x[8], S32, $8771F681);
  HH(c, d, a, b, x[11], S33, $6D9D6122);
  HH(b, c, d, a, x[14], S34, $FDE5380C);
  HH(a, b, c, d, x[1], S31, $A4BEEA44);
  HH(d, a, b, c, x[4], S32, $4BDECFA9);
  HH(c, d, a, b, x[7], S33, $F6BB4B60);
  HH(b, c, d, a, x[10], S34, $BEBFBC70);
  HH(a, b, c, d, x[13], S31, $289B7EC6);
  HH(d, a, b, c, x[0], S32, $EAA127FA);
  HH(c, d, a, b, x[3], S33, $D4EF3085);
  HH(b, c, d, a, x[6], S34, $4881D05);
  HH(a, b, c, d, x[9], S31, $D9D4D039);
  HH(d, a, b, c, x[12], S32, $E6DB99E5);
  HH(c, d, a, b, x[15], S33, $1FA27CF8);
  HH(b, c, d, a, x[2], S34, $C4AC5665);

  II(a, b, c, d, x[0], S41, $F4292244);
  II(d, a, b, c, x[7], S42, $432AFF97);
  II(c, d, a, b, x[14], S43, $AB9423A7);
  II(b, c, d, a, x[5], S44, $FC93A039);
  II(a, b, c, d, x[12], S41, $655B59C3);
  II(d, a, b, c, x[3], S42, $8F0CCC92);
  II(c, d, a, b, x[10], S43, $FFEFF47D);
  II(b, c, d, a, x[1], S44, $85845DD1);
  II(a, b, c, d, x[8], S41, $6FA87E4F);
  II(d, a, b, c, x[15], S42, $FE2CE6E0);
  II(c, d, a, b, x[6], S43, $A3014314);
  II(b, c, d, a, x[13], S44, $4E0811A1);
  II(a, b, c, d, x[4], S41, $F7537E82);
  II(d, a, b, c, x[11], S42, $BD3AF235);
  II(c, d, a, b, x[2], S43, $2AD7D2BB);
  II(b, c, d, a, x[9], S44, $EB86D391);

  Inc(State[0], a);
  Inc(State[1], b);
  Inc(State[2], c);
  Inc(State[3], d);

  MD5_memset(PByteArray(@x), 0, SizeOf(x));
End;


Procedure MD5Init(Var Context: TMD5Context);
Begin
  FillChar(Context, SizeOf(Context), 0);
  Context.state[0] := $67452301;
  Context.state[1] := $EFCDAB89;
  Context.state[2] := $98BADCFE;
  Context.state[3] := $10325476;
End;

Procedure MD5Update(Var Context: TMD5Context; Input: PByteArray; InputLen: LongWord);
Var
  i, index, partLen: LongWord;

Begin
  index := LongWord((context.count[0] Shr 3) And $3F);
  Inc(Context.count[0], UINT4(InputLen) Shl 3);
  If Context.count[0] < UINT4(InputLen) Shl 3 Then Inc(Context.count[1]);
  Inc(Context.count[1], UINT4(InputLen) Shr 29);
  partLen := 64 - index;
  If inputLen >= partLen Then Begin
    MD5_memcpy(PByteArray(@Context.buffer[index]), Input, PartLen);
    MD5Transform(@Context.state, @Context.buffer);
    i := partLen;
    While i + 63 < inputLen Do Begin
      MD5Transform(@Context.state, PArray64Byte(@Input[i]));
      Inc(i, 64);
    End;
    index := 0;
  End Else i := 0;
  MD5_memcpy(PByteArray(@Context.buffer[index]), PByteArray(@Input[i]), inputLen - i);
End;


Procedure MD5Final(Var Digest: TMD5Digest; Var Context: TMD5Context);
Var
  bits: Array[0..7] Of Byte;
  index, padLen: LongWord;
Begin
  MD5Encode(PByteArray(@bits), PUINT4Array(@Context.count), 8);
  index := LongWord((Context.count[0] Shr 3) And $3F);
  If index < 56 Then padLen := 56 - index Else padLen := 120 - index;
  MD5Update(Context, PByteArray(@PADDING), padLen);
  MD5Update(Context, PByteArray(@Bits), 8);
  MD5Encode(PByteArray(@Digest), PUINT4Array(@Context.state), 16);
  MD5_memset(PByteArray(@Context), 0, SizeOf(Context));
End;

Function MD5DigestToStr(Const Digest: TMD5Digest): String;
Var
  i: Integer;
Begin
  Result := '';
  For i := 0 To 15 Do Result := Result + IntToHex(Digest.v[i], 2);
End;

Function MD5String(Const S: String): TMD5Digest;
Begin
  Result := MD5Buffer(PChar(S)^, Length(S));
End;

Function MD5File(Const FileName: String): TMD5Digest;
Var
  F: TFileStream;
Begin
  F := TFileStream.Create(FileName, fmOpenRead);
  Try
    Result := MD5Stream(F);
  Finally
    F.Free;
  End;
End;

Function MD5Stream(Const Stream: TStream): TMD5Digest;
Var
  Context: TMD5Context;
  Buffer: Array[0..4095] Of Byte;
  Size: Integer;
  ReadBytes: Integer;
  TotalBytes: Integer;
  SavePos: Integer;
Begin
  MD5Init(Context);
  Size := Stream.Size;
  SavePos := Stream.Position;
  TotalBytes := 0;
  Try
    Stream.Seek(0, soFromBeginning);
    Repeat
      ReadBytes := Stream.Read(Buffer, SizeOf(Buffer));
      Inc(TotalBytes, ReadBytes);
      MD5Update(Context, @Buffer, ReadBytes);
    Until (ReadBytes = 0) Or (TotalBytes = Size);
  Finally
    Stream.Seek(SavePos, soFromBeginning);
  End;
  MD5Final(Result, Context);
End;

Function MD5Buffer(Const Buffer; Size: Integer): TMD5Digest;
Var
  Context: TMD5Context;
Begin
  MD5Init(Context);
  MD5Update(Context, PByteArray(@Buffer), Size);
  MD5Final(Result, Context);
End;

Function MD5DigestCompare(Const Digest1, Digest2: TMD5Digest): Boolean;
Begin
  Result := False;
  If Digest1.A <> Digest2.A Then Exit;
  If Digest1.B <> Digest2.B Then Exit;
  If Digest1.C <> Digest2.C Then Exit;
  If Digest1.D <> Digest2.D Then Exit;
  Result := True;
End;

End.

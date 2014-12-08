unit rtfutils;

interface

uses
  richedit,
  windows;


function IsRTF(const Value: pWideChar): Boolean;

//used for Export only
function GetRichRTFW(RichEditHandle: THANDLE; var RTFStream: PWideChar;
                    SelectionOnly, PlainText, NoObjects, PlainRTF: Boolean): Integer;
function GetRichRTFA(RichEditHandle: THANDLE; var RTFStream: PAnsiChar;
                    SelectionOnly, PlainText, NoObjects, PlainRTF: Boolean): Integer;

function GetRichString(RichEditHandle: THANDLE; SelectionOnly: Boolean = false): PWideChar;

function SetRichRTFW(RichEditHandle: THANDLE; const RTFStream: PWideChar;
                    SelectionOnly, PlainText, PlainRTF: Boolean): Integer;
function SetRichRTFA(RichEditHandle: THANDLE; const RTFStream: PAnsiChar;
                    SelectionOnly, PlainText, PlainRTF: Boolean): Integer;

function FormatString2RTFW(Source: PWideChar; Suffix: PAnsiChar = nil): PAnsiChar;
function FormatString2RTFA(Source: PAnsiChar; Suffix: PAnsiChar = nil): PAnsiChar;

procedure ReplaceCharFormatRange(RichEditHandle: THANDLE;
     const fromCF, toCF: CHARFORMAT2; idx, len: Integer);
procedure ReplaceCharFormat(RichEditHandle: THANDLE; const fromCF, toCF: CHARFORMAT2);

function GetTextLength(RichEditHandle:THANDLE): Integer;
function GetTextRange (RichEditHandle:THANDLE; cpMin,cpMax: Integer): PWideChar;

function BitmapToRTF(pict: HBITMAP): pAnsiChar;

implementation

uses
  common;

function IsRTF(const Value: pWideChar): Boolean;
const
  RTF_BEGIN_1  = '{\RTF';
  RTF_BEGIN_2  = '{URTF';
begin
  Result := (StrPosW(Value,RTF_BEGIN_1) = Value)
         or (StrPosW(Value,RTF_BEGIN_2) = Value);
end;

type
  PTextStream = ^TTextStream;
  TTextStream = record
    Size: Integer;
    case Boolean of
      false: (Data:  PAnsiChar);
      true:  (DataW: PWideChar);
  end;

function RichEditStreamLoad(dwCookie: DWORD_PTR; pbBuff: PByte; cb: Longint; var pcb: Longint): dword; stdcall;
var
  pBuff: PAnsiChar;
begin
  with PTextStream(dwCookie)^ do
  begin
    pBuff := Data;
    pcb := Size;
    if pcb > cb then
      pcb := cb;
    Move(pBuff^, pbBuff^, pcb);
    Inc(Data, pcb);
    Dec(Size, pcb);
  end;
  Result := 0;
end;

function RichEditStreamSave(dwCookie: DWORD_PTR; pbBuff: PByte; cb: Longint; var pcb: Longint): dword; stdcall;
var
  prevSize: Integer;
begin
  with PTextStream(dwCookie)^ do
  begin
    prevSize := Size;
    Inc(Size,cb);
    ReallocMem(Data,Size);
    Move(pbBuff^,(Data+prevSize)^,cb);
    pcb := cb;
  end;
  Result := 0;
end;

function _GetRichRTF(RichEditHandle: THANDLE; TextStream: PTextStream;
                    SelectionOnly, PlainText, NoObjects, PlainRTF, Unicode: Boolean): Integer;
var
  es: TEditStream;
  Format: Longint;
begin
  format := 0;
  if SelectionOnly then
    Format := Format or SFF_SELECTION;
  if PlainText then
  begin
    if NoObjects then
      Format := Format or SF_TEXT
    else
      Format := Format or SF_TEXTIZED;
    if Unicode then
      Format := Format or SF_UNICODE;
  end
  else
  begin
    if NoObjects then
      Format := Format or SF_RTFNOOBJS
    else
      Format := Format or SF_RTF;
    if PlainRTF then
      Format := Format or SFF_PLAINRTF;
    // if Unicode then   format := format or SF_USECODEPAGE or (CP_UTF16 shl 16);
  end;
  TextStream^.Size := 0;
  TextStream^.Data := nil;
  es.dwCookie := DWORD_PTR(TextStream);
  es.dwError := 0;
  es.pfnCallback := @RichEditStreamSave;
  SendMessage(RichEditHandle, EM_STREAMOUT, format, LPARAM(@es));
  Result := es.dwError;
end;

function GetRichRTFW(RichEditHandle: THANDLE; var RTFStream: PWideChar;
                    SelectionOnly, PlainText, NoObjects, PlainRTF: Boolean): Integer;
var
  Stream: TTextStream;
begin
  Result := _GetRichRTF(RichEditHandle, @Stream,
                        SelectionOnly, PlainText, NoObjects, PlainRTF, PlainText);
  if Assigned(Stream.DataW) then
  begin
    if PlainText then
      StrDupW(RTFStream, Stream.DataW, Stream.Size div SizeOf(WideChar))
    else
      AnsiToWide(Stream.Data, RTFStream, CP_ACP);
    FreeMem(Stream.Data, Stream.Size);
  end
  else
    RTFStream := nil;
end;

function GetRichRTFA(RichEditHandle: THANDLE; var RTFStream: PAnsiChar;
                    SelectionOnly, PlainText, NoObjects, PlainRTF: Boolean): Integer;
var
  Stream: TTextStream;
begin
  Result := _GetRichRTF(RichEditHandle, @Stream,
                        SelectionOnly, PlainText, NoObjects, PlainRTF, False);
  if Assigned(Stream.Data) then
  begin
    StrDup(RTFStream, Stream.Data, Stream.Size - 1);
    FreeMem(Stream.Data, Stream.Size);
  end
  else
    RTFStream := nil;
end;

function GetRichString(RichEditHandle: THANDLE; SelectionOnly: Boolean = false): PWideChar;
begin
  GetRichRTFW(RichEditHandle,Result,SelectionOnly,True,True,False);
end;


function _SetRichRTF(RichEditHandle: THANDLE; TextStream: PTextStream;
                    SelectionOnly, PlainText, PlainRTF, Unicode: Boolean): Integer;
var
  es: TEditStream;
  Format: Longint;
begin
  Format := 0;
  if SelectionOnly then
    Format := Format or SFF_SELECTION;
  if PlainText then
  begin
    Format := Format or SF_TEXT;
    if Unicode then
      Format := Format or SF_UNICODE;
  end
  else
  begin
    Format := Format or SF_RTF;
    if PlainRTF then
      Format := Format or SFF_PLAINRTF;
    // if Unicode then  format := format or SF_USECODEPAGE or (CP_UTF16 shl 16);
  end;
  es.dwCookie := LPARAM(TextStream);
  es.dwError := 0;
  es.pfnCallback := @RichEditStreamLoad;
  SendMessage(RichEditHandle, EM_STREAMIN, format, LPARAM(@es));
  Result := es.dwError;
end;

function SetRichRTFW(RichEditHandle: THANDLE; const RTFStream: PWideChar;
                    SelectionOnly, PlainText, PlainRTF: Boolean): Integer;
var
  Stream: TTextStream;
  Buffer: PAnsiChar;
begin
  if PlainText then
  begin
    Stream.DataW := RTFStream;
    Stream.Size  := StrLenW(RTFStream) * SizeOf(WideChar);
    Buffer := nil;
  end
  else
  begin
    WideToAnsi(RTFStream, Buffer, CP_ACP);
    Stream.Data := Buffer;
    Stream.Size := StrLen(Buffer);
  end;
  Result := _SetRichRTF(RichEditHandle, @Stream,
                        SelectionOnly, PlainText, PlainRTF, PlainText);
  mFreeMem(Buffer);
end;

function SetRichRTFA(RichEditHandle: THANDLE; const RTFStream: PAnsiChar;
                    SelectionOnly, PlainText, PlainRTF: Boolean): Integer;
var
  Stream: TTextStream;
begin
  Stream.Data := RTFStream;
  Stream.Size := StrLen(RTFStream);
  Result := _SetRichRTF(RichEditHandle, @Stream,
                        SelectionOnly, PlainText, PlainRTF, False);
end;

function FormatString2RTFW(Source: PWideChar; Suffix: PAnsiChar = nil): PAnsiChar;
var
  Text: PWideChar;
  res: PAnsiChar;
  buf: array [0..15] of AnsiChar;
  len: integer;
begin
  // calculate len
  len:=Length('{\uc1 ');
  Text := PWideChar(Source);
  while Text[0] <> #0 do
  begin
    if (Text[0] = #13) and (Text[1] = #10) then
    begin
      inc(len,Length('\par '));
      Inc(Text);
    end
    else
      case Text[0] of
        #10: inc(len,Length('\par '));
        #09: inc(len,Length('\tab '));
        '\', '{', '}':
          inc(len,2);
      else
        if Word(Text[0]) < 128 then
          inc(len)
        else
          inc(len,3+IntStrLen(Word(Text[0]),10));
      end;
    Inc(Text);
  end;
  inc(len,StrLen(Suffix)+2);

  // replace
  Text := PWideChar(Source);
  GetMem(Result,len);
  res:=StrCopyE(Result,'{\uc1 ');
  while Text[0] <> #0 do
  begin
    if (Text[0] = #13) and (Text[1] = #10) then
    begin
      res:=StrCopyE(res,'\par ');
      Inc(Text);
    end
    else
      case Text[0] of
        #10: res:=StrCopyE(res,'\par ');
        #09: res:=StrCopyE(res,'\tab ');
        '\', '{', '}': begin
          res^:='\'; inc(res);
          res^:=AnsiChar(Text[0]); inc(res);
        end;
      else
        if Word(Text[0]) < 128 then
        begin
          res^:=AnsiChar(Word(Text[0])); inc(res);
        end
        else
        begin
          res:=StrCopyE(
            StrCopyE(res,'\u'),
            IntToStr(buf,Word(Text[0])));
          res^:='?'; inc(res);
        end;
      end;
    Inc(Text);
  end;

  res:=StrCopyE(res, Suffix);
  res^:='}'; inc(res); res^:=#0;
end;

function FormatString2RTFA(Source: PAnsiChar; Suffix: PAnsiChar = nil): PAnsiChar;
var
  Text,res: PAnsiChar;
  len: integer;
begin
  // calculate len
  len:=1;
  Text := PAnsiChar(Source);
  while Text[0] <> #0 do
  begin
    if (Text[0] = #13) and (Text[1] = #10) then
    begin
      inc(len,Length('\line '));
      Inc(Text);
    end
    else
      case Text[0] of
        #10: inc(len,Length('\line '));
        #09: inc(len,Length('\tab '));
        '\', '{', '}':
          inc(len,2);
      else
        inc(len);
      end;
    Inc(Text);
  end;
  inc(len,StrLen(Suffix)+2);

  // replace
  Text := PAnsiChar(Source);
  GetMem(Result,len);
  res:=Result;
  res^ := '{'; inc(res);
  while Text[0] <> #0 do
  begin
    if (Text[0] = #13) and (Text[1] = #10) then
    begin
      res:=StrCopyE(res,'\line ');
      Inc(Text);
    end
    else
      case Text[0] of
        #10: res:=StrCopyE(res,'\line ');
        #09: res:=StrCopyE(res,'\tab ');
        '\', '{', '}': begin
          res^:='\'; inc(res);
          res^:=Text[0]; inc(res);
        end;
      else
        res^:=Text[0]; inc(res);
      end;
    Inc(Text);
  end;

  res:=StrCopyE(res, Suffix);
  res^:='}'; inc(res); res^:=#0;
end;

function GetTextLength(RichEditHandle: THANDLE): Integer;
var
  gtxl: GETTEXTLENGTHEX;
begin
  gtxl.flags    := GTL_DEFAULT or GTL_PRECISE;
  gtxl.codepage := 1200; // Unicode
  gtxl.flags    := gtxl.flags or GTL_NUMCHARS;
  Result := SendMessage(RichEditHandle, EM_GETTEXTLENGTHEX, WPARAM(@gtxl), 0);
end;

procedure ReplaceCharFormatRange(RichEditHandle: THANDLE;
     const fromCF, toCF: CHARFORMAT2; idx, len: Integer);
var
  cr: CHARRANGE;
  cf: CHARFORMAT2;
  loglen: Integer;
  res: DWord;
begin
  if len = 0 then
    exit;
  cr.cpMin := idx;
  cr.cpMax := idx + len;
  SendMessage(RichEditHandle, EM_EXSETSEL, 0, LPARAM(@cr));
  ZeroMemory(@cf, SizeOf(cf));
  cf.cbSize := SizeOf(cf);
  cf.dwMask := fromCF.dwMask;
  res := SendMessage(RichEditHandle, EM_GETCHARFORMAT, SCF_SELECTION, LPARAM(@cf));
  if (res and fromCF.dwMask) = 0 then
  begin
    if len = 2 then
    begin
      // wtf, msdn tells that cf will get the format of the first AnsiChar,
      // and then we have to select it, if format match or second, if not
      // instead we got format of the last AnsiChar... weired
      if (cf.dwEffects and fromCF.dwEffects) = fromCF.dwEffects then
        Inc(cr.cpMin)
      else
        Dec(cr.cpMax);
      SendMessage(RichEditHandle, EM_EXSETSEL, 0, LPARAM(@cr));
      SendMessage(RichEditHandle, EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(@toCF));
    end
    else
    begin
      loglen := len div 2;
      ReplaceCharFormatRange(RichEditHandle, fromCF, toCF, idx, loglen);
      ReplaceCharFormatRange(RichEditHandle, fromCF, toCF, idx + loglen, len - loglen);
    end;
  end
  else if (cf.dwEffects and fromCF.dwEffects) = fromCF.dwEffects then
    SendMessage(RichEditHandle, EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(@toCF));
end;

procedure ReplaceCharFormat(RichEditHandle: THANDLE; const fromCF, toCF: CHARFORMAT2);
begin
  ReplaceCharFormatRange(RichEditHandle,fromCF,toCF,0,GetTextLength(RichEditHandle));
end;


function GetTextRange(RichEditHandle: THANDLE; cpMin,cpMax: Integer): PWideChar;
var
  tr: TextRangeW;
begin
  tr.chrg.cpMin := cpMin;
  tr.chrg.cpMax := cpMax;
  GetMem(Result,(cpMax-cpMin+1)*SizeOf(WideChar));
  tr.lpstrText := Result;

  SendMessageW(RichEditHandle,EM_GETTEXTRANGE,0,LPARAM(@tr));
end;

{ Direct Bitmap to RTF insertion }

function BytesPerScanline(PixelsPerScanline, BitsPerPixel, Alignment: Longint): Longint;
begin
  Dec(Alignment);
  Result := ((PixelsPerScanline * BitsPerPixel) + Alignment) and not Alignment;
  Result := Result div 8;
end;

procedure InitializeBitmapInfoHeader(Bitmap: HBITMAP; var BI: TBitmapInfoHeader; Colors: Integer);
var
  DS: TDIBSection;
  Bytes: Integer;
begin
  DS.dsbmih.biSize := 0;
  Bytes := GetObject(Bitmap, SizeOf(DS), @DS);
  if Bytes = 0 then {InvalidBitmap}
  else if (Bytes >= (sizeof(DS.dsbm) + sizeof(DS.dsbmih))) and
    (DS.dsbmih.biSize >= DWORD(sizeof(DS.dsbmih))) then
    BI := DS.dsbmih
  else
  begin
    FillChar(BI, sizeof(BI), 0);
    with BI, DS.dsbm do
    begin
      biSize := SizeOf(BI);
      biWidth := bmWidth;
      biHeight := bmHeight;
    end;
  end;
  case Colors of
    2: BI.biBitCount := 1;
    3..16:
      begin
        BI.biBitCount := 4;
        BI.biClrUsed := Colors;
      end;
    17..256:
      begin
        BI.biBitCount := 8;
        BI.biClrUsed := Colors;
      end;
  else
    BI.biBitCount := DS.dsbm.bmBitsPixel * DS.dsbm.bmPlanes;
  end;
  BI.biPlanes := 1;
  if BI.biClrImportant > BI.biClrUsed then
    BI.biClrImportant := BI.biClrUsed;
  if BI.biSizeImage = 0 then
    BI.biSizeImage := BytesPerScanLine(BI.biWidth, BI.biBitCount, 32) * Abs(BI.biHeight);
end;

procedure InternalGetDIBSizes(Bitmap: HBITMAP; var InfoHeaderSize: DWORD;
  var ImageSize: DWORD; Colors: Integer);
var
  BI: TBitmapInfoHeader;
begin
  InitializeBitmapInfoHeader(Bitmap, BI, Colors);
  if BI.biBitCount > 8 then
  begin
    InfoHeaderSize := SizeOf(TBitmapInfoHeader);
    if (BI.biCompression and BI_BITFIELDS) <> 0 then
      Inc(InfoHeaderSize, 12);
  end
  else
    if BI.biClrUsed = 0 then
      InfoHeaderSize := SizeOf(TBitmapInfoHeader) +
        SizeOf(TRGBQuad) * (1 shl BI.biBitCount)
    else
      InfoHeaderSize := SizeOf(TBitmapInfoHeader) +
        SizeOf(TRGBQuad) * BI.biClrUsed;
  ImageSize := BI.biSizeImage;
end;

procedure GetDIBSizes(Bitmap: HBITMAP; var InfoHeaderSize: DWORD; var ImageSize: DWORD);
begin
  InternalGetDIBSizes(Bitmap, InfoHeaderSize, ImageSize, 0);
end;

function InternalGetDIB(Bitmap: HBITMAP; Palette: HPALETTE;
  var BitmapInfo; var Bits; Colors: Integer): Boolean;
var
  OldPal: HPALETTE;
  DC: HDC;
begin
  InitializeBitmapInfoHeader(Bitmap, TBitmapInfoHeader(BitmapInfo), Colors);
  OldPal := 0;
  DC := CreateCompatibleDC(0);
  try
    if Palette <> 0 then
    begin
      OldPal := SelectPalette(DC, Palette, False);
      RealizePalette(DC);
    end;
    Result := GetDIBits(DC, Bitmap, 0, TBitmapInfoHeader(BitmapInfo).biHeight, @Bits,
      TBitmapInfo(BitmapInfo), DIB_RGB_COLORS) <> 0;
  finally
    if OldPal <> 0 then SelectPalette(DC, OldPal, False);
    DeleteDC(DC);
  end;
end;

function GetDIB(Bitmap: HBITMAP; Palette: HPALETTE; var BitmapInfo; var Bits): Boolean;
begin
  Result := InternalGetDIB(Bitmap, Palette, BitmapInfo, Bits, 0);
end;

const
  HexDigitChr: array [0..15] of AnsiChar = ('0','1','2','3','4','5','6','7',
                                            '8','9','A','B','C','D','E','F');

function BitmapToRTF(pict: HBITMAP): pAnsiChar;
const
  prefix  = '{\rtf1 {\pict\dibitmap ';
  postfix = ' }}';
var
  tmp, bi, bb, rtf: pAnsiChar;
  bis, bbs: cardinal;
  len,cnt: integer;
begin
  GetDIBSizes(pict, bis, bbs);
  GetMem(bi, bis);
  GetMem(bb, bbs);
  GetDIB(pict, {pict.Palette}0, bi^, bb^);

  len:=(bis+bbs)*2+cardinal(Length(prefix)+Length(postfix))+1;
  GetMem(result,len);

  rtf:=StrCopyE(result,prefix);
  tmp:=bi;
  for cnt := 0 to bis-1 do
  begin
    rtf^ := HexDigitChr[ord(tmp^) shr  4]; inc(rtf);
    rtf^ := HexDigitChr[ord(tmp^) and $F]; inc(rtf);
    inc(tmp);
  end;
  tmp:=bb;
  for cnt := 0 to bbs-1 do
  begin
    rtf^ := HexDigitChr[ord(tmp^) shr  4]; inc(rtf);
    rtf^ := HexDigitChr[ord(tmp^) and $F]; inc(rtf);
    inc(tmp);
  end;
  StrCopy(rtf,postfix);

  FreeMem(bi);
  FreeMem(bb);
end;


initialization
finalization

end.

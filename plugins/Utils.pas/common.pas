{$INCLUDE compilers.inc}
{$IFDEF Miranda}
  {.$DEFINE Use_MMI}
{$ENDIF}
unit common;

interface

uses
windows
{$IFDEF Miranda}
,m_api
{$ENDIF}
;

Const {- Character sets -}
  sBinNum   = ['0'..'1'];
  sOctNum   = ['0'..'7'];
  sNum      = ['0'..'9'];
  sHexNum   = ['0'..'9','A'..'F','a'..'f'];
  sWord     = ['0'..'9','A'..'Z','a'..'z','_',#128..#255];
  sIdFirst  = ['A'..'Z','a'..'z','_'];
  sLatWord  = ['0'..'9','A'..'Z','a'..'z','_'];
  sWordOnly = ['A'..'Z','a'..'z'];
  sSpace    = [#9,' '];
  sEmpty    = [#9,#10,#13,' '];

const
  HexDigitChrLo: array [0..15] of AnsiChar = ('0','1','2','3','4','5','6','7',
                                              '8','9','a','b','c','d','e','f');

  HexDigitChr  : array [0..15] of AnsiChar = ('0','1','2','3','4','5','6','7',
                                              '8','9','A','B','C','D','E','F');

var
  IsW2K,
  IsVista,
  IsAnsi:boolean;

function IIF(cond:bool;ret1,ret2:integer  ):integer;   overload;
function IIF(cond:bool;ret1,ret2:PAnsiChar):PAnsiChar; overload;
function IIF(cond:bool;ret1,ret2:PWideChar):PWideChar; overload;
function IIF(cond:bool;ret1,ret2:Extended ):Extended;  overload;
function IIF(cond:bool;ret1,ret2:tDateTime):tDateTime; overload;
function IIF(cond:bool;ret1,ret2:pointer  ):pointer;   overload;
function IIF(cond:bool;const ret1,ret2:string):string; overload;
{$IFNDEF DELPHI_7_UP}
function IIF(cond:bool;ret1,ret2:variant  ):variant;   overload;
{$ENDIF}
function Min(a,b:integer):integer;
function Max(a,b:integer):integer;

function GetImageType (buf:PByte;mime:PAnsiChar=nil):dword;
function GetImageTypeW(buf:PByte;mime:PWideChar=nil):int64;

//----- Clipboard -----

procedure CopyToClipboard(txt:pointer; Ansi:bool);
function  PasteFromClipboard(Ansi:boolean;cp:dword=CP_ACP):pointer;

//----- Memory -----

function  mGetMem (var dst;size:integer):pointer;
procedure mFreeMem(var ptr);
function  mReallocMem(var dst; size:integer):pointer;
procedure FillWord(var buf;count:cardinal;value:word); register;
function CompareMem(P1, P2: pointer; Length: integer): Boolean;
procedure ShowDump(ptr:pbyte;len:integer);
function BSwap(value:dword):dword;
function Hash(s:pointer; len:integer{const Seed: longword=$9747b28c}): LongWord;

type
  tSortProc = function (First,Second:integer):integer;
  {0=equ; 1=1st>2nd; -1=1st<2nd }
procedure ShellSort(size:integer;Compare,Swap:tSortProc);

//----- String processing -----

function FormatStrW   (fmt:PWideChar; const arr:array of PWideChar):PWideChar;
function FormatStr    (fmt:PAnsiChar; const arr:array of PAnsiChar):PAnsiChar;
function FormatSimpleW(fmt:PWideChar; const arr:array of const):PWideChar;
function FormatSimple (fmt:PAnsiChar; const arr:array of const):PAnsiChar;

const
  SIGN_UNICODE    = $FEFF;
  SIGN_REVERSEBOM = $FFFE;
  SIGN_UTF8       = $BFBBEF;
const
  CP_ACP        = 0;
  CP_UTF8       = 65001;
  CP_UNICODE    = 1200;
  CP_REVERSEBOM = 65534;
// trying to recognize text encoding. Returns CP_
function GetTextFormat(Buffer:PByte;sz:cardinal):integer;

function AdjustLineBreaks(s:PWideChar):PWideChar;

//----- Encoding conversion -----

function WideToCombo(src:PWideChar;var dst;cp:integer=CP_ACP):integer;

function ChangeUnicode(Str:PWideChar):PWideChar;
function UTF8Len(src:PAnsiChar):integer;
function WideToAnsi(src:PWideChar;var dst:PAnsiChar;cp:dword=CP_ACP):PAnsiChar;
function AnsiToWide(src:PAnsiChar;var dst:PWideChar;cp:dword=CP_ACP):PWideChar;
function AnsiToUTF8(src:PAnsiChar;var dst:PAnsiChar;cp:dword=CP_ACP):PAnsiChar;
function UTF8ToAnsi(src:PAnsiChar;var dst:PAnsiChar;cp:dword=CP_ACP):PAnsiChar;
function UTF8ToWide(src:PAnsiChar;var dst:PWideChar;len:cardinal=cardinal(-1)):PWideChar;
function WideToUTF8(src:PWideChar;var dst:PAnsiChar):PAnsiChar;

function CharWideToUTF8(src:WideChar;var dst:PAnsiChar):integer;
function CharUTF8ToWide(src:PAnsiChar;pin:pinteger=nil):WideChar;
function CharUTF8Len(src:PAnsiChar):integer;

function FastWideToAnsiBuf(src:PWideChar;dst:PAnsiChar;len:cardinal=cardinal(-1)):PAnsiChar;
function FastAnsiToWideBuf(src:PAnsiChar;dst:PWideChar;len:cardinal=cardinal(-1)):PWideChar;
function FastWideToAnsi   (src:PWideChar;var dst:PAnsiChar):PAnsiChar;
function FastAnsiToWide   (src:PAnsiChar;var dst:PWideChar):PWideChar;

// encode/decode text (URL coding)
function Encode(dst,src:PAnsiChar):PAnsiChar;
function Decode(dst,src:PAnsiChar):PAnsiChar;
// '\n'(#13#10) and '\t' (#9) (un)escaping
function UnEscape(buf:PAnsiChar):PAnsiChar;
function Escape  (buf:PAnsiChar):PAnsiChar;
procedure UpperCase(src:PWideChar);
procedure LowerCase(src:PWideChar);

//----- base strings functions -----

function StrDup (var dst:PAnsiChar;src:PAnsiChar;len:cardinal=0):PAnsiChar;
function StrDupW(var dst:PWideChar;src:PWideChar;len:cardinal=0):PWideChar;
function StrEmpty:pointer;
function StrDelete (aStr:PAnsiChar;pos,len:cardinal):PAnsiChar;
function StrDeleteW(aStr:PWideChar;pos,len:cardinal):PWideChar;
function StrInsert (SubStr,src:PAnsiChar;pos:cardinal):PAnsiChar;
function StrInsertW(SubStr,src:PWideChar;pos:cardinal):PWideChar;
function StrReplace (src,SubStr,NewStr:PAnsiChar):PAnsiChar;
function StrReplaceW(src,SubStr,NewStr:PWideChar):PWideChar;
function CharReplace (dst:PAnsiChar;old,new:AnsiChar):PAnsiChar;
function CharReplaceW(dst:PWideChar;old,new:WideChar):PWideChar;
function StrCmp (a,b:PAnsiChar;n:integer=0):integer;
function StrCmpW(a,b:PWideChar;n:integer=0):integer;
function StrEnd (const a:PAnsiChar):PAnsiChar;
function StrEndW(const a:PWideChar):PWideChar;
function StrScan (src:PAnsiChar;c:AnsiChar):PAnsiChar;
function StrScanW(src:PWideChar;c:WideChar):PWideChar;
function StrRScan (src:PAnsiChar;c:AnsiChar):PAnsiChar;
function StrRScanW(src:PWideChar;c:WideChar):PWideChar;
function StrLen (Str: PAnsiChar): Cardinal;
function StrLenW(Str: PWideChar): Cardinal;
function StrCat (Dest: PAnsiChar; const Source: PAnsiChar): PAnsiChar;
function StrCatW(Dest: PWideChar; const Source: PWideChar): PWideChar;
function StrCatE (Dest: PAnsiChar; const Source: PAnsiChar): PAnsiChar;
function StrCatEW(Dest: PWideChar; const Source: PWideChar): PWideChar;
function StrCopyE (dst:PAnsiChar;src:PAnsiChar;len:cardinal=0):PAnsiChar;
function StrCopyEW(dst:PWideChar;src:PWideChar;len:cardinal=0):PWideChar;
function StrCopy (dst:PAnsiChar;src:PAnsiChar;len:cardinal=0):PAnsiChar;
function StrCopyW(dst:PWideChar;src:PWideChar;len:cardinal=0):PWideChar;
function StrPos (const aStr, aSubStr: PAnsiChar): PAnsiChar;
function StrPosW(const aStr, aSubStr: PWideChar): PWideChar;
function StrIndex (const aStr, aSubStr: PAnsiChar):integer;
function StrIndexW(const aStr, aSubStr: PWideChar):integer;

function GetPairChar(ch:AnsiChar):AnsiChar; overload;
function GetPairChar(ch:WideChar):WideChar; overload;

//----- String/number conversion -----

function IntStrLen(value:int64; base:integer=10):integer;

function IntToHex(dst:PWideChar;value:int64;digits:integer=0):PWideChar; overload;
function IntToHex(dst:PAnsiChar;value:int64;digits:integer=0):PAnsiChar; overload;
function IntToStr(dst:PWideChar;value:int64;digits:integer=0):PWideChar; overload;
function IntToStr(dst:PAnsiChar;value:int64;digits:integer=0):PAnsiChar; overload;
function StrToInt(src:PWideChar):int64; overload;
function StrToInt(src:PAnsiChar):int64; overload;
function HexToInt(src:PWideChar;len:cardinal=$FFFF):int64; overload;
function HexToInt(src:PAnsiChar;len:cardinal=$FFFF):int64; overload;
function NumToInt(src:PWideChar):int64; overload;
function NumToInt(src:PAnsiChar):int64; overload;

//----- Date and Time -----

function TimeToInt(stime:PAnsiChar):integer; overload;
function TimeToInt(stime:PWideChar):integer; overload;
function IntToTime(dst:PWideChar;Time:integer):PWideChar; overload;
function IntToTime(dst:PAnsiChar;Time:integer):PAnsiChar; overload;

{
  filesize to string conversion
  value   - filelength
  divider - 1=byte; 1024=kbyte; 1024*1024 - Mbyte
  prec    - numbers after point (1-3)
  post    - 0=none
            1=(small)' bytes','kb','mb'
            2=(mix)  ' Bytes','Kb','Mb'
            3=(caps) ''      ,'KB','MB'
  postfix calculated from 'divider' value
}
function IntToK(dst:PWideChar;value,divider,prec,post:integer):PWideChar;

// filename work
function ChangeExt (src,ext:PAnsiChar):PAnsiChar;
function ChangeExtW(src,ext:PWideChar):PWideChar;
function Extract (s:PAnsiChar;name:Boolean=true):PAnsiChar;
function ExtractW(s:PWideChar;name:Boolean=true):PWideChar;
function GetExt(fname,dst:PWideChar;maxlen:dword=100):PWideChar; overload;
function GetExt(fname,dst:PAnsiChar;maxlen:dword=100):PAnsiChar; overload;

function isPathAbsolute(path:PWideChar):boolean; overload;
function isPathAbsolute(path:PAnsiChar):boolean; overload;

//-----------------------------------------------------------------------------

implementation

const
  IS_TEXT_UNICODE_ASCII16            = $1;
  IS_TEXT_UNICODE_REVERSE_ASCII16    = $10;
  IS_TEXT_UNICODE_STATISTICS         = $2;
  IS_TEXT_UNICODE_REVERSE_STATISTICS = $20;
  IS_TEXT_UNICODE_CONTROLS           = $4;
  IS_TEXT_UNICODE_REVERSE_CONTROLS   = $40;
  IS_TEXT_UNICODE_SIGNATURE          = $8;
  IS_TEXT_UNICODE_REVERSE_SIGNATURE  = $80;
  IS_TEXT_UNICODE_ILLEGAL_CHARS      = $100;
  IS_TEXT_UNICODE_ODD_LENGTH         = $200;
  IS_TEXT_UNICODE_DBCS_LEADBYTE      = $400;
  IS_TEXT_UNICODE_NULL_BYTES         = $1000;
  IS_TEXT_UNICODE_UNICODE_MASK       = $F;
  IS_TEXT_UNICODE_REVERSE_MASK       = $F0;
  IS_TEXT_UNICODE_NOT_UNICODE_MASK   = $F00;
  IS_TEXT_UNICODE_NOT_ASCII_MASK     = $F000;

function IsTextUTF8(Buffer:pbyte;Length:integer):boolean;
var
  Ascii:boolean;
  Octets:cardinal;
  c:byte;
begin
	Ascii:=true;
	Octets:=0;

	if Length=0 then
	  Length:=-1;
	repeat
	  if (Length=0) or (Buffer^=0) then
	    break;
	  dec(Length);
		c:=Buffer^;
		if (c and $80)<>0 then
			Ascii:=false;
		if Octets<>0 then
		begin
			if (c and $C0)<>$80 then
			begin
				result:=false;
				exit;
			end;
			dec(Octets);
		end
		else
		begin
			if (c and $80)<>0 then
			begin
				while (c and $80)<>0 do
				begin
					c:=c shl 1;
					inc(Octets);
				end;
				dec(Octets);
				if Octets=0 then
				begin
					result:=false;
					exit;
				end;
			end
		end;
		inc(Buffer);
	until false;
	result:= not ((Octets>0) or Ascii);
end;

function GetTextFormat(Buffer:PByte;sz:cardinal):integer;
var
  test:integer;
begin
	result:=-1;

	if sz>=2 then
	begin
  	if       pword (Buffer)^               =SIGN_UNICODE    then result := CP_UNICODE
	  else if  pword (Buffer)^               =SIGN_REVERSEBOM then result := CP_REVERSEBOM
  	else if  (sz>=4) and 
           ((pdword(Buffer)^ and $00FFFFFF)=SIGN_UTF8)      then result := CP_UTF8;
  end;

	if result<0 then
	begin
		test:=
			IS_TEXT_UNICODE_STATISTICS         or
			IS_TEXT_UNICODE_REVERSE_STATISTICS or
			IS_TEXT_UNICODE_CONTROLS           or
			IS_TEXT_UNICODE_REVERSE_CONTROLS   or
			IS_TEXT_UNICODE_ILLEGAL_CHARS      or
			IS_TEXT_UNICODE_ODD_LENGTH         or
			IS_TEXT_UNICODE_NULL_BYTES;

		if not odd(sz) and IsTextUnicode(Buffer,sz,@test) then
		begin
			if (test and (IS_TEXT_UNICODE_ODD_LENGTH or IS_TEXT_UNICODE_ILLEGAL_CHARS))=0 then
			begin
				if (test and (IS_TEXT_UNICODE_NULL_BYTES or
				              IS_TEXT_UNICODE_CONTROLS   or
				              IS_TEXT_UNICODE_REVERSE_CONTROLS))<>0 then
				begin
					if (test and (IS_TEXT_UNICODE_CONTROLS or
					              IS_TEXT_UNICODE_STATISTICS))<>0 then
						result:=CP_UNICODE
					else if (test and (IS_TEXT_UNICODE_REVERSE_CONTROLS or
					                   IS_TEXT_UNICODE_REVERSE_STATISTICS))<>0 then
						result:=CP_REVERSEBOM;
				end
			end
		end
		else if IsTextUTF8(Buffer,sz) then
			result:=CP_UTF8
		else
		  result:=CP_ACP;
	end;
end;

function IIF(cond:bool;ret1,ret2:integer):integer; overload;{$IFDEF AllowInline}inline;{$ENDIF}
begin
  if cond then result:=ret1 else result:=ret2;
end;
function IIF(cond:bool;ret1,ret2:PAnsiChar):PAnsiChar; overload;{$IFDEF AllowInline}inline;{$ENDIF}
begin
  if cond then result:=ret1 else result:=ret2;
end;
function IIF(cond:bool;ret1,ret2:PWideChar):PWideChar; overload;{$IFDEF AllowInline}inline;{$ENDIF}
begin
  if cond then result:=ret1 else result:=ret2;
end;
function IIF(cond:bool;ret1,ret2:Extended):Extended; overload;{$IFDEF AllowInline}inline;{$ENDIF}
begin
  if cond then result:=ret1 else result:=ret2;
end;
function IIF(cond:bool;ret1,ret2:tDateTime):tDateTime; overload;{$IFDEF AllowInline}inline;{$ENDIF}
begin
  if cond then result:=ret1 else result:=ret2;
end;
function IIF(cond:bool;ret1,ret2:pointer):pointer; overload;{$IFDEF AllowInline}inline;{$ENDIF}
begin
  if cond then result:=ret1 else result:=ret2;
end;
function IIF(cond:bool;const ret1,ret2:string):string; overload;{$IFDEF AllowInline}inline;{$ENDIF}
begin
  if cond then result:=ret1 else result:=ret2;
end;
{$IFNDEF DELPHI_7_UP}
function IIF(cond:bool;ret1,ret2:variant):variant; overload;{$IFDEF AllowInline}inline;{$ENDIF}
begin
  if cond then result:=ret1 else result:=ret2;
end;
{$ENDIF}

function Min(a,b:integer):integer;{$IFDEF AllowInline}inline;{$ENDIF}
begin
  if a>b then
    result:=b
  else
    result:=a;
end;

function Max(a,b:integer):integer;{$IFDEF AllowInline}inline;{$ENDIF}
begin
  if a<b then
    result:=b
  else
    result:=a;
end;

const
  mimecnt = 5;
  mimes:array [0..mimecnt-1] of record
     mime:PAnsiChar;
     ext:array [0..3] of AnsiChar
  end = (
  (mime:'image/gif' ; ext:'GIF'),
  (mime:'image/jpg' ; ext:'JPG'),
  (mime:'image/jpeg'; ext:'JPG'),
  (mime:'image/png' ; ext:'PNG'),
  (mime:'image/bmp' ; ext:'BMP')
);

function GetImageType(buf:PByte;mime:PAnsiChar=nil):dword;
var
  i:integer;
begin
  result:=0;
  if (mime<>nil) and (mime^<>#0) then
  begin
    for i:=0 to mimecnt-1 do
    begin
      if {lstrcmpia}StrCmp(mime,mimes[i].mime)=0 then
      begin
        result:=dword(mimes[i].ext);
        exit;
      end;
    end;
  end
  else if buf<>nil then
  begin
    if (pdword(buf)^ and $F0FFFFFF)=$E0FFD8FF then result:=$0047504A // 'JPG'
    else if pdword(buf)^=$38464947 then result:=$00464947 // 'GIF'
    else if pdword(buf)^=$474E5089 then result:=$00474E50 // 'PNG'
    else if pword (buf)^=$4D42     then result:=$00504D42 // 'BMP'
  end;
end;

function GetImageTypeW(buf:PByte;mime:PWideChar=nil):int64;
var
  i:integer;
  lmime:array [0..63] of AnsiChar;
begin
  result:=0;
  if (mime<>nil) and (mime^<>#0) then
  begin
    FastWideToAnsiBuf(mime,lmime);
    for i:=0 to mimecnt-1 do
    begin
      if {lstrcmpia}StrCmp(lmime,mimes[i].mime)=0 then
      begin
//        result:=dword(mimes[i].ext);
        FastAnsiToWideBuf(mimes[i].ext,PWideChar(@result));
        exit;
      end;
    end;
  end
  else if buf<>nil then
  begin
    if (pdword(buf)^ and $F0FFFFFF)=$E0FFD8FF then result:=$000000470050004A // 'JPG'
    else if pdword(buf)^=$38464947 then result:=$0000004600490047 // 'GIF'
    else if pdword(buf)^=$474E5089 then result:=$00000047004E0050 // 'PNG'
    else if pword (buf)^=$4D42     then result:=$00000050004D0042 // 'BMP'
  end;
end;

//----- Clipboard -----

procedure CopyToClipboard(txt:pointer; Ansi:bool);
var
  s:pointer;
  fh:THANDLE;
begin
  if pointer(txt)=nil then
    exit;
  if Ansi then 
  begin
    if PAnsiChar(txt)^=#0 then exit
  end
  else
    if PWideChar(txt)^=#0 then exit;

  if OpenClipboard(0) then
  begin
    if Ansi then
    begin
      fh:=GlobalAlloc(GMEM_MOVEABLE+GMEM_DDESHARE,(StrLen(PAnsiChar(txt))+1));
      s:=GlobalLock(fh);
      StrCopy(s,PAnsiChar(txt));
    end
    else
    begin
      fh:=GlobalAlloc(GMEM_MOVEABLE+GMEM_DDESHARE,
          (StrLenW(PWideChar(txt))+1)*SizeOf(WideChar));
      s:=GlobalLock(fh);
      StrCopyW(s,PWideChar(txt));
    end;
    GlobalUnlock(fh);
    EmptyClipboard;
    if Ansi then
      SetClipboardData(CF_TEXT,fh)
    else
      SetClipboardData(CF_UNICODETEXT,fh);
    CloseClipboard;
  end;
end;

function PasteFromClipboard(Ansi:boolean;cp:dword=CP_ACP):pointer;
var
  p:PWideChar;
  fh:THANDLE;
begin
  result:=nil;
  if OpenClipboard(0) then
  begin
    if not Ansi then
    begin
      fh:=GetClipboardData(CF_UNICODETEXT);
      if fh<>0 then
      begin
        p:=GlobalLock(fh);
        StrDupW(PWideChar(result),p);
      end
      else
      begin
        fh:=GetClipboardData(CF_TEXT);
        if fh<>0 then
        begin
          p:=GlobalLock(fh);
          AnsiToWide(PAnsiChar(p),PWideChar(result),cp);
        end;
      end;
    end
    else
    begin
      fh:=GetClipboardData(CF_TEXT);
      if fh<>0 then
      begin
        p:=GlobalLock(fh);
        StrDup(PAnsiChar(result),PAnsiChar(p));
      end;
    end;
    if fh<>0 then
      GlobalUnlock(fh);
    CloseClipboard;
  end
end;

procedure CheckSystem;
var
  ovi:TOSVersionInfo;
begin
  ovi.dwOSVersionInfoSize:=SizeOf(TOSVersionInfo);
  GetVersionEx(ovi);
//VER_PLATFORM_WIN32_NT for 2KXP
  with ovi do
  begin
    IsAnsi :=dwPlatformId=VER_PLATFORM_WIN32_WINDOWS;
    IsW2K  :=(dwMajorVersion=5) and (dwMinorVersion=0);
    IsVista:=(dwMajorVersion=6) and (dwMinorVersion=0);
  end;
end;

// --------- string conversion ----------

function WideToCombo(src:PWideChar;var dst;cp:integer=CP_ACP):integer;
var
  pc:PAnsiChar;
  i,j:Cardinal;
begin
  WideToAnsi(src,pc,cp);
  j:=StrLen(pc)+1;
  i:=j+(StrLenW(src)+1)*SizeOf(WideChar);
  mGetMem(PAnsiChar(dst),i);
  StrCopy(PAnsiChar(dst),pc);
  mFreeMem(pc);
  StrCopyW(PWideChar(PAnsiChar(dst)+j),src);
  result:=i;
end;

function ChangeUnicode(Str:PWideChar):PWideChar;
var
  i,len:integer;
begin
  result:=Str;
  if (Str=nil) or (Str^=#0) then
    exit;
  if (word(Str^)=$FFFE) or (word(Str^)=$FEFF) then
  begin
    len:=StrLenW(Str);
    if word(Str^)=$FFFE then
    begin
      i:=len-1;
      while i>0 do // Str^<>#0
      begin
        pword(Str)^:=swap(pword(Str)^);
        inc(Str);
        dec(i);
      end;
    end;
    move((result+1)^,result^,len*SizeOf(WideChar));
  end;
end;

function WideToAnsi(src:PWideChar;var dst:PAnsiChar; cp:dword=CP_ACP):PAnsiChar;
var
  len,l:integer;
begin
  if (src=nil) or (src^=#0) then
  begin
    mGetMem(result,SizeOf(AnsiChar));
    result^:=#0;
  end
  else
  begin
    l:=StrLenW(src);
    len:=WideCharToMultiByte(cp,0,src,l,NIL,0,NIL,NIL)+1;
    mGetMem(result,len);
    FillChar(result^,len,0);
    WideCharToMultiByte(cp,0,src,l,result,len,NIL,NIL);
  end;
  dst:=result;
end;

function AnsiToWide(src:PAnsiChar;var dst:PWideChar; cp:dword=CP_ACP):PWideChar;
var
  len,l:integer;
begin
  if (src=nil) or (src^=#0) then
  begin
    mGetMem(result,SizeOf(WideChar));
    result^:=#0;
  end
  else
  begin
    l:=StrLen(src);
    len:=MultiByteToWideChar(cp,0,src,l,NIL,0)+1;
    mGetMem(result,len*SizeOf(WideChar));
    FillChar(result^,len*SizeOf(WideChar),0);
    MultiByteToWideChar(cp,0,src,l,result,len);
  end;
  dst:=result;
end;

function AnsiToUTF8(src:PAnsiChar;var dst:PAnsiChar;cp:dword=CP_ACP):PAnsiChar;
var
  tmp:PWideChar;
begin
  AnsiToWide(src,tmp,cp);
  result:=WideToUTF8(tmp,dst);
  mFreeMem(tmp);
end;

function UTF8Len(src:PAnsiChar):integer; // w/o zero
begin
  result:=0;
  if src<>nil then
  begin
    if (pdword(src)^ and $00FFFFFF)=SIGN_UTF8 then
      inc(src,3);
    while src^<>#0 do
    begin
      if      (ord(src^) and $80)=0 then
      else if (ord(src^) and $E0)=$E0 then
        inc(src,2)
      else
        inc(src);
      inc(result);
      inc(src);
    end;
  end;
end;

function CalcUTF8Len(src:PWideChar):integer;
begin
  result:=0;
  if src<>nil then
  begin
    while src^<>#0 do
    begin
      if      src^<#$0080 then
      else if src^<#$0800 then
        inc(result)
      else
        inc(result,2);
      inc(src);
      inc(result);
    end;
  end;
end;

function CharWideToUTF8(src:WideChar;var dst:PAnsiChar):integer;
begin
  if src<#$0080 then
  begin
    dst^:=AnsiChar(src);
    result:=1;
  end
  else if src<#$0800 then
  begin
    dst^:=AnsiChar($C0 or (ord(src) shr 6));
    inc(dst);
    dst^:=AnsiChar($80 or (ord(src) and $3F));
    result:=2;
  end
  else
  begin
    dst^:=AnsiChar($E0 or (ord(src) shr 12));
    inc(dst);
    dst^:=AnsiChar($80 or ((ord(src) shr 6) and $3F));
    inc(dst);
    dst^:=AnsiChar($80 or (ord(src) and $3F));
    result:=3;
  end;
  inc(dst); dst^:=#0;
end;

function CharUTF8ToWide(src:PAnsiChar;pin:pinteger=nil):WideChar;
var
  cnt:integer;
  w:word;
begin
  if ord(src^)<$80 then
  begin
    w:=ord(src^);
    cnt:=1;
  end
  else if (ord(src^) and $E0)=$E0 then
  begin
    w:=(ord(src^) and $1F) shl 12;
    inc(src);
    w:=w or (((ord(src^))and $3F) shl 6);
    inc(src);
    w:=w or (ord(src^) and $3F);
    cnt:=3;
  end
  else
  begin
    w:=(ord(src^) and $3F) shl 6;
    inc(src);
    w:=w or (ord(src^) and $3F);
    cnt:=2;
  end;
  if pin<>nil then
    pin^:=cnt;
  result:=WideChar(w);
end;

function CharUTF8Len(src:PAnsiChar):integer;
begin
{!!}
  if (ord(src^) and $80)=0 then
    result:=1
  else if (ord(src^) and $E0)=$E0 then
    result:=3
  else
    result:=2;
{}
end;

function UTF8ToWide(src:PAnsiChar; var dst:PWideChar; len:cardinal=cardinal(-1)):PWideChar;
var
  w:word;
  p:PWideChar;
begin
  mGetMem(dst,(UTF8Len(src)+1)*SizeOf(WideChar));
  p:=dst;
  if src<>nil then
  begin
    if (pdword(src)^ and $00FFFFFF)=SIGN_UTF8 then
      inc(src,3);
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

function UTF8ToAnsi(src:PAnsiChar;var dst:PAnsiChar;cp:dword=CP_ACP):PAnsiChar;
var
  tmp:PWideChar;
begin
  UTF8ToWide(src,tmp);
  result:=WideToAnsi(tmp,dst,cp);
  mFreeMem(tmp);
end;

function WidetoUTF8(src:PWideChar; var dst:PAnsiChar):PAnsiChar;
var
  p:PAnsiChar;
begin
  mGetMem(dst,CalcUTF8Len(src)+1);
  p:=dst;
  if src<>nil then
  begin
    while src^<>#0 do
    begin
      if src^<#$0080 then
        p^:=AnsiChar(src^)
      else if src^<#$0800 then
      begin
        p^:=AnsiChar($C0 or (ord(src^) shr 6));
        inc(p);
        p^:=AnsiChar($80 or (ord(src^) and $3F));
      end
      else
      begin
        p^:=AnsiChar($E0 or (ord(src^) shr 12));
        inc(p);
        p^:=AnsiChar($80 or ((ord(src^) shr 6) and $3F));
        inc(p);
        p^:=AnsiChar($80 or (ord(src^) and $3F));
      end;
      inc(p);
      inc(src);
    end;
  end;
  p^:=#0;
  result:=dst;
end;

//----- Memory -----

procedure FillWord(var buf;count:cardinal;value:word); register;
{$IFNDEF WIN64}assembler;
{
  PUSH EDI 
  MOV EDI, ECX // Move Value To Write 
  MOV ECX, EDX // Move Number to ECX for countdown 
  MOV EDX, EAX // Move over buffer 
  MOV EAX, EDI // Value to Write needs to be here 
  MOV EDI, EDX // Pointer to Buffer[0] 
  REP STOSW 
  POP EDI
}
asm
  push edi
  mov  edi,buf // destination
  mov  ax,value   // value
  mov  ecx,count // count
  rep  stosw
  pop  edi
{
  push edi
  mov  edi,eax // destination
  mov  ax,cx   // value
  mov  ecx,edx // count
  rep  stosw
  pop  edi
}
end;
{$ELSE}
var
  ptr:pword;
  i:integer;
begin
  ptr:=pword(@buf);
  for i:=0 to count-1 do
  begin
    ptr^:=value;
    inc(ptr);
  end;
end;
{$ENDIF}
// from SysUtils
{ Delphi 7.0
function CompareMem(P1, P2: pointer; Length: integer): Boolean; assembler;
asm
     PUSH    ESI
     PUSH    EDI
     MOV     ESI,P1
     MOV     EDI,P2
     MOV     EDX,ECX
     XOR     EAX,EAX
     AND     EDX,3
     SAR     ECX,2
     JS      @@1     // Negative Length implies identity.
     REPE    CMPSD
     JNE     @@2
     MOV     ECX,EDX
     REPE    CMPSB
     JNE     @@2
@@1: INC     EAX
@@2: POP     EDI
     POP     ESI
end;
}

{$IFNDEF WIN64}
// Delphi 2009 realization
function CompareMem(P1, P2: pointer; Length: integer): Boolean; assembler;
asm
   add   eax, ecx
   add   edx, ecx
   xor   ecx, -1
   add   eax, -8
   add   edx, -8
   add   ecx, 9
   push  ebx
   jg    @Dword
   mov   ebx, [eax+ecx]
   cmp   ebx, [edx+ecx]
   jne   @Ret0
   lea   ebx, [eax+ecx]
   add   ecx, 4
   and   ebx, 3
   sub   ecx, ebx
   jg    @Dword
@DwordLoop:
   mov   ebx, [eax+ecx]
   cmp   ebx, [edx+ecx]
   jne   @Ret0
   mov   ebx, [eax+ecx+4]
   cmp   ebx, [edx+ecx+4]
   jne   @Ret0
   add   ecx, 8
   jg    @Dword
   mov   ebx, [eax+ecx]
   cmp   ebx, [edx+ecx]
   jne   @Ret0
   mov   ebx, [eax+ecx+4]
   cmp   ebx, [edx+ecx+4]
   jne   @Ret0
   add   ecx, 8
   jle   @DwordLoop
@Dword:
   cmp   ecx, 4
   jg    @Word
   mov   ebx, [eax+ecx]
   cmp   ebx, [edx+ecx]
   jne   @Ret0
   add   ecx, 4
@Word:
   cmp   ecx, 6
   jg    @Byte
   movzx ebx, word ptr [eax+ecx]
   cmp   bx, [edx+ecx]
   jne   @Ret0
   add   ecx, 2
@Byte:
   cmp   ecx, 7
   jg    @Ret1
   movzx ebx, byte ptr [eax+7]
   cmp   bl, [edx+7]
   jne   @Ret0
@Ret1:
   mov   eax, 1
   pop   ebx
   ret
@Ret0:
   xor   eax, eax
   pop   ebx
end;
{$ELSE}
function CompareMem(P1, P2: pointer; Length: integer): Boolean;
var
  i:integer;
begin
  for i:=0 to Length-1 do
  begin
    if PByte(P1)^<>pbyte(P2)^ then
    begin
      result:=false;
      exit;
    end;
    inc(pbyte(P1));
    inc(pbyte(P2));
  end;
  result:=true;
end;
{$ENDIF}

function mGetMem(var dst;size:integer):pointer;
begin
{$IFDEF Use_MMI}
  pointer(dst):=mir_alloc(size)
{$ELSE}
  GetMem(pointer(dst),size);
{$ENDIF}
  result:=pointer(dst);
end;

procedure mFreeMem(var ptr);
begin
  if pointer(ptr)<>nil then
  begin
{$IFDEF UseMMI}
    mir_free(pointer(ptr))
{$ELSE}
    FreeMem(pointer(ptr));
{$ENDIF}
    pointer(ptr):=nil;
  end;
end;

function mReallocMem(var dst; size:integer):pointer;
begin
{$IFDEF Use_MMI}
  pointer(dst):=mir_realloc(pointer(dst),size)
{$ELSE}
  ReallocMem(pointer(dst),size);
{$ENDIF}
  result:=pointer(dst);
end;

procedure ShowDump(ptr:pbyte;len:integer);
var
  buf: array of Ansichar;
  i:integer;
  p:PAnsiChar;
  p1:PByte;
  cnt:integer;
begin
  SetLength(buf,len*4+1);
  p:=@buf[0];
  p1:=ptr;
  cnt:=0;
  for i:=0 to len-1 do
  begin
    IntToHex(p,p1^,2);
    inc(p,2);
    inc(p1);
    inc(cnt);
    if cnt=4 then
    begin
      cnt:=0;
      p^:='.';
      inc(p);
    end;
  end;
  p^:=#0;
  messageboxa(0,@buf[0],'',0);
end;

// Murmur 2.0
function Hash(s:pointer; len:integer{const Seed: longword=$9747b28c}): longword;
var
  lhash: longword;
  k: longword;
  tmp,data: PByte;
const
  // 'm' and 'r' are mixing constants generated offline.
  // They're not really 'magic', they just happen to work well.
  m = $5bd1e995;
  r = 24;
begin
  //The default seed, $9747b28c, is from the original C library

  // Initialize the hash to a 'random' value
  lhash := {seed xor }len;

  // Mix 4 bytes at a time into the hash
  data := s;

  while(len >= 4) do
  begin
    k := PLongWord(data)^;

    k := k*m;
    k := k xor (k shr r);
    k := k*m;

    lhash := lhash*m;
    lhash := lhash xor k;

    inc(data,4);
    dec(len,4);
  end;

  //   Handle the last few bytes of the input array
  if len = 3 then
  begin
    tmp:=data;
    inc(tmp,2);
    lhash := lhash xor (longword(tmp^) shl 16);
  end;
  if len >= 2 then
  begin
    tmp:=data;
    inc(tmp);
    lhash := lhash xor (longword(tmp^) shl 8);
  end;
  if len >= 1 then
  begin
    lhash := lhash xor (longword(data^));
    lhash := lhash * m;
  end;

  // Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.
  lhash := lhash xor (lhash shr 13);
  lhash := lhash * m;
  lhash := lhash xor (lhash shr 15);

  Result := lhash;
end;

function BSwap(value:dword):dword;
  {$IFNDEF WIN64}
begin
  asm
    mov   eax,value
    bswap eax
    mov   result,eax
  end;
  {$ELSE}
begin
  result:=((value and $000000FF) shl 24) +
          ((value and $0000FF00) shl  8) +
          ((value and $00FF0000) shr  8) +
          ((value and $FF000000) shr 24);
  {$ENDIF}
end;

procedure ShellSort(size:integer;Compare,Swap:tSortProc);
var
  i,j,gap:longint;
begin
  gap:=size shr 1;
  while gap>0 do
  begin
    for i:=gap to size-1 do
    begin
      j:=i-gap;
      while (j>=0) and (Compare(j,UInt(j+gap))>0) do
      begin
        Swap(j,UInt(j+gap));
        dec(j,gap);
      end;
    end;
    gap:=gap shr 1;
  end;
end;

function Encode(dst,src:PAnsiChar):PAnsiChar;
begin
  while src^<>#0 do
  begin
    if not (src^ in [' ','%','+','&','?',#128..#255]) then
      dst^:=src^
    else
    begin
      dst^:='%'; inc(dst);
      dst^:=HexDigitChr[ord(src^) shr 4]; inc(dst);
      dst^:=HexDigitChr[ord(src^) and $0F];
    end;
    inc(src);
    inc(dst);
  end;
  dst^:=#0;
  result:=dst;
end;

function Decode(dst,src:PAnsiChar):PAnsiChar;
begin
  while (src^<>#0) and (src^<>'&') do
  begin
    if (src^='%') and ((src+1)^ in sHexNum) and ((src+2)^ in sHexNum) then
    begin
      inc(src);
      dst^:=AnsiChar(HexToInt(src,2));
      inc(src);
    end
    else
      dst^:=src^;
    inc(dst);
    inc(src);
  end;
  dst^:=#0;
  result:=dst;
end;

function UnEscape(buf:PAnsiChar):PAnsiChar;
begin
  if (buf<>nil) and (buf^<>#0) then
  begin
    StrReplace(buf,PAnsiChar(#$7F'n'),PAnsiChar(#$0D#$0A));
    StrReplace(buf,PAnsiChar(#$7F't'),PAnsiChar(#$09));
  end;
  result:=buf;
end;

function Escape(buf:PAnsiChar):PAnsiChar;
var
  i:integer;
begin
  i:=StrLen(buf);
  if i<>0 then
  begin
    Move(buf^,(buf+1)^,i+1);
    buf^:=#39;
    (buf+i+1)^:=#39;
    (buf+i+2)^:=#0;
    StrReplace(buf,#$0D#$0A,#$7F'n');
    StrReplace(buf,#$09,#$7F't');
  end;
  result:=buf;
end;

procedure UpperCase(src:PWideChar);
var
  c:WideChar;
begin
  if src<>nil then
  begin
    while src^<>#0 do
    begin
      c:=src^;
      if (c>='a') and (c<='z') then
        src^:=WideChar(ord(c)-$20);
      inc(src);
    end;
  end;
end;

procedure LowerCase(src:PWideChar);
var
  c:WideChar;
begin
  if src<>nil then
  begin
    while src^<>#0 do
    begin
      c:=src^;
      if (c>='A') and (c<='Z') then
        src^:=WideChar(ord(c)+$20);
      inc(src);
    end;
  end;
end;

const
  Posts:array [0..8] of PWideChar =
    (' bytes',' Bytes','','kb','Kb','KB','mb','Mb','MB');

function IntToK(dst:pWidechar;value,divider,prec,post:integer):pWidechar;
var
  tmp:integer;
  p:PWideChar;
  ls:array [0..4] of WideChar;
begin
  result:=dst;
  IntToStr(dst,value div divider);
  if divider=1 then prec:=0;
  while dst^<>#0 do inc(dst);
  if prec<>0 then
  begin
    if      prec=1 then prec:=10
    else if prec=2 then prec:=100
    else {if prec=3 then} prec:=1000;
    tmp:=round(frac(value*1.0/divider)*prec);
    dst^:='.'; inc(dst);
    IntToStr(ls,tmp);
    p:=ls;
    while p^<>#0 do
    begin
      dst^:=p^; inc(dst); inc(p);
    end;
    dst^:=#0;
  end;
  if post<>0 then
  begin
    if divider=1 then
      StrCatW(dst,Posts[post-1])
    else
    begin
      if divider=1024 then tmp:=1
      else {if divider=1024*1024 then} tmp:=2;
      p:=Posts[tmp*3+post-1];
      dst^:=p[0]; inc(dst);
      dst^:=p[1]; inc(dst);
      dst^:=#0;
    end;
  end;
end;

//----- String processing -----

function FormatStrW(fmt:PWideChar; const arr:array of PWideChar):PWideChar;
var
  i,len:integer;
  pc:PWideChar;
  number:integer;
begin
  result:=nil;
  if (fmt=nil) or (fmt^=#0) then
    exit;

  // calculate length
  len:=StrLenW(fmt); // -2*Length(arr)
  for i:=0 to HIGH(arr) do
    inc(len,StrLenW(arr[i]));

  // format
  mGetMem(result,(len+1)*SizeOf(WideChar));
  pc:=result;
  number:=0;
  while fmt^<>#0 do
  begin
    if (fmt^='%') and ((fmt+1)^='s') then
    begin
      if number<=HIGH(arr) then
      begin
        pc:=StrCopyEW(pc,arr[number]);
        inc(number);
      end;
      inc(fmt,2);
    end
    else
    begin
      pc^:=fmt^;
      inc(pc);
      inc(fmt);
    end;
  end;
  pc^:=#0;
end;

function FormatStr(fmt:PAnsiChar; const arr:array of PAnsiChar):PAnsiChar;
var
  i,len:integer;
  pc:PAnsiChar;
  number:integer;
begin
  result:=nil;
  if (fmt=nil) or (fmt^=#0) then
    exit;

  // calculate length
  len:=StrLen(fmt); // -2*Length(arr)
  for i:=0 to HIGH(arr) do
    inc(len,StrLen(arr[i]));

  // format
  mGetMem(result,len+1);
  pc:=result;
  number:=0;
  while fmt^<>#0 do
  begin
    if (fmt^='%') and ((fmt+1)^='s') then
    begin
      if number<=HIGH(arr) then
      begin
        pc:=StrCopyE(pc,arr[number]);
        inc(number);
      end;
      inc(fmt,2);
    end
    else
    begin
      pc^:=fmt^;
      inc(pc);
      inc(fmt);
    end;
  end;
  pc^:=#0;
end;

function FormatSimpleW(fmt:PWideChar; const arr:array of const):PWideChar;
var
  i,len:integer;
  pc:PWideChar;
  number:integer;
begin
  result:=nil;
  if (fmt=nil) or (fmt^=#0) then
    exit;

  // calculate length
  len:=StrLenW(fmt); // -2*Length(arr)
  for i:=0 to HIGH(arr) do
  begin
    case arr[i].VType of
      vtInteger  : inc(len,10); // max len of VInteger text
      vtPWideChar: inc(len,StrLenW(arr[i].VPWideChar));
    end;
  end;

  // format
  mGetMem(result,(len+1)*SizeOf(WideChar));
  pc:=result;
  number:=0;
  while fmt^<>#0 do
  begin
    if (fmt^='%') then
    begin
      case (fmt+1)^ of
        's': begin
          if number<=HIGH(arr) then
          begin
            pc:=StrCopyEW(pc,arr[number].VPWideChar);
            inc(number);
          end;
          inc(fmt,2);
        end;
        'd','u': begin
          if number<=HIGH(arr) then
          begin
            pc:=StrEndW(IntToStr(pc,arr[number].VInteger));
            inc(number);
          end;
          inc(fmt,2);
        end;
        '%': begin
          pc^:='%';
          inc(pc);
          inc(fmt,2);
        end;
      else
        pc^:=fmt^;
        inc(pc);
        inc(fmt);
      end;
    end;
  end;
  pc^:=#0;
end;

function FormatSimple(fmt:PAnsiChar; const arr:array of const):PAnsiChar;
var
  i,len:integer;
  pc:PAnsiChar;
  number:integer;
begin
  result:=nil;
  if (fmt=nil) or (fmt^=#0) then
    exit;

  // calculate length
  len:=StrLen(fmt); // -2*Length(arr)
  for i:=0 to HIGH(arr) do
  begin
    case arr[i].VType of
      vtInteger: inc(len,10); // max len of VInteger text
      vtPChar  : inc(len,StrLen(arr[i].VPChar));
    end;
  end;

  // format
  mGetMem(result,len+1);
  pc:=result;
  number:=0;
  while fmt^<>#0 do
  begin
    if (fmt^='%') then
    begin
      case (fmt+1)^ of
        's': begin
          if number<=HIGH(arr) then
          begin
            pc:=StrCopyE(pc,arr[number].VPChar);
            inc(number);
          end;
          inc(fmt,2);
        end;
        'd','u': begin
          if number<=HIGH(arr) then
          begin
            pc:=StrEnd(IntToStr(pc,arr[number].VInteger));
            inc(number);
          end;
          inc(fmt,2);
        end;
        '%': begin
          pc^:='%';
          inc(pc);
          inc(fmt,2);
        end;
      else
        pc^:=fmt^;
        inc(pc);
        inc(fmt);
      end;
    end;
  end;
  pc^:=#0;
end;

function AdjustLineBreaks(s:PWideChar):PWideChar;
var
  Source, Dest: PWideChar;
  Extra, len: Integer;
begin
  Result := nil;
  len := StrLenW(s);
  if len=0 then
    exit;

  Source := s;
  Extra := 0;
  while Source^ <> #0 do
  begin
    case Source^ of
      #10:
        Inc(Extra);
      #13:
        if Source[1] = #10 then
          Inc(Source)
        else
          Inc(Extra);
    end;
    Inc(Source);
  end;

  if Extra = 0 then
  begin
    StrDupW(Result, s);
  end
  else
  begin
    Source := s;
    mGetMem(Result, (len + Extra + 1) * SizeOf(WideChar));
    Dest := Result;
    while Source^ <> #0 do
    begin
      case Source^ of
        #10: begin
          Dest^ := #13;
          Inc(Dest);
          Dest^ := #10;
        end;
        #13: begin
          Dest^ := #13;
          Inc(Dest);
          Dest^ := #10;
          if Source[1] = #10 then
            Inc(Source);
        end;
      else
        Dest^ := Source^;
      end;
      Inc(Dest);
      Inc(Source);
    end;
    Dest^ := #0;
  end;
end;

// ----- base string functions -----

function StrDup(var dst:PAnsiChar;src:PAnsiChar;len:cardinal=0):PAnsiChar;
var
  l:cardinal;
  p:PAnsiChar;
begin
  if (src=nil) or (src^=#0) then
    dst:=nil
  else
  begin
    if len=0 then
      len:=high(cardinal);
    p:=src;
    l:=len;
    while (p^<>#0) and (l>0) do
    begin
      inc(p); dec(l);
    end;
    l:=p-src;

    mGetMem(dst,l+1);
    move(src^, dst^,l);
    dst[l]:=#0;
  end;
  result:=dst;
end;

function StrDupW(var dst:PWideChar;src:PWideChar;len:cardinal=0):PWideChar;
var
  l:cardinal;
  p:PWideChar;
begin
  if (src=nil) or (src^=#0) then
    dst:=nil
  else
  begin
    if len=0 then
      len:=high(cardinal);
    p:=src;
    l:=len;
    while (p^<>#0) and (l>0) do
    begin
      inc(p); dec(l);
    end;
    l:=p-src;
    mGetMem(dst,(l+1)*SizeOf(WideChar));
    move(src^, dst^,l*SizeOf(WideChar));
    dst[l]:=#0;
  end;
  result:=dst;
end;

function StrEmpty:pointer;
begin
  mGetMem(result,SizeOf(WideChar));
  pWord(result)^:=0;
end;

function StrCopyE(dst:PAnsiChar;src:PAnsiChar;len:cardinal=0):PAnsiChar;
var
  l:cardinal;
  p:PAnsiChar;
begin
  if dst<>nil then
  begin
    if (src=nil) or (src^=#0) then
      dst^:=#0
    else
    begin
      if len=0 then
        len:=high(cardinal);
      p:=src;
      l:=len;
      while (p^<>#0) and (l>0) do
      begin
        inc(p); dec(l);
      end;
      l:=p-src;
      move(src^, dst^,l);
      inc(dst,l);
      dst^:=#0;
    end;
  end;
  result:=dst;
end;

function StrCopyEW(dst:PWideChar;src:PWideChar;len:cardinal=0):PWideChar;
var
  l:cardinal;
  p:PWideChar;
begin
  if dst<>nil then
  begin
    if (src=nil) or (src^=#0) then
      dst^:=#0
    else
    begin
      if len=0 then
        len:=high(cardinal);
      p:=src;
      l:=len;
      while (p^<>#0) and (l>0) do
      begin
        inc(p); dec(l);
      end;
      l:=p-src;
      move(src^, dst^,l*SizeOf(WideChar));
      inc(dst,l);
      dst^:=#0;
    end;
  end;
  result:=dst;
end;

function StrCopy(dst:PAnsiChar;src:PAnsiChar;len:cardinal=0):PAnsiChar;
var
  l:cardinal;
  p:PAnsiChar;
begin
  if dst<>nil then
  begin
    if (src=nil) or (src^=#0) then
      dst^:=#0
    else
    begin
      if len=0 then
        len:=high(cardinal);
      p:=src;
      l:=len;
      while (p^<>#0) and (l>0) do
      begin
        inc(p); dec(l);
      end;
      l:=p-src;
      move(src^, dst^,l);
      dst[l]:=#0;
    end;
  end;
  result:=dst;
end;

function StrCopyW(dst:PWideChar;src:PWideChar;len:cardinal=0):PWideChar;
var
  l:cardinal;
  p:PWideChar;
begin
  if dst<>nil then
  begin
    if (src=nil) or (src^=#0) then
      dst^:=#0
    else
    begin
      if len=0 then
        len:=high(cardinal);
      p:=src;
      l:=len;
      while (p^<>#0) and (l>0) do
      begin
        inc(p); dec(l);
      end;
      l:=p-src;
      move(src^, dst^,l*SizeOf(WideChar));
      dst[l]:=#0;
    end;
  end;
  result:=dst;
end;

function StrDelete(aStr:PAnsiChar;pos,len:cardinal):PAnsiChar;
var
  i:cardinal;
begin
  if len>0 then
  begin
    i:=StrLen(aStr);
    if pos<i then
    begin
      if (pos+len)>i then
        len:=i-pos;
      StrCopy(aStr+pos,aStr+pos+len);
    end;
  end;
  result:=aStr;
end;

function StrDeleteW(aStr:PWideChar;pos,len:cardinal):PWideChar;
var
  i:cardinal;
begin
  if len>0 then
  begin
    i:=StrLenW(aStr);
    if pos<i then
    begin
      if (pos+len)>i then
        len:=i-pos;
      StrCopyW(aStr+pos,aStr+pos+len);
    end;
  end;
  result:=aStr;
end;

function StrInsert(SubStr,src:PAnsiChar;pos:cardinal):PAnsiChar;
var
  i:cardinal;
  p:PAnsiChar;
begin
  i:=StrLen(SubStr);
  if i<>0 then
  begin
    p:=src+pos;
    move(p^,(p+i)^,StrLen(src)-pos+1);
    move(SubStr^,p^,i);
  end;
  result:=src;
end;

function StrInsertW(SubStr,src:PWideChar;pos:cardinal):PWideChar;
var
  i:cardinal;
  p:PWideChar;
begin
  i:=StrLenW(SubStr);
  if i<>0 then
  begin
    p:=src+pos;
    move(p^,(p+i)^,(StrLenW(src)-pos+1)*SizeOf(PWideChar));
    move(SubStr^,p^,i*SizeOf(WideChar));
  end;
  result:=src;
end;

function StrReplace(src,SubStr,NewStr:PAnsiChar):PAnsiChar;
var
  i,j,l:integer;
  k:integer;
  p:PAnsiChar;
begin
  result:=src;
  p:=StrPos(src,SubStr);
  if p=nil then exit;
  i:=StrLen(SubStr);
  j:=StrLen(NewStr);
  l:=i-j;
  repeat
    if j=0 then
      StrCopy(p,p+i)
    else
    begin
      k:=StrLen(p)+1;
      if l>0 then
        move((p+l)^,p^,k-l)
      else if l<>0 then
        move(p^,(p-l)^,k);
      move(NewStr^,p^,j); {new characters}
      inc(p,j);
    end;
    p:=StrPos(p,SubStr);
    if p=nil then break;
  until false;
end;

function StrReplaceW(src,SubStr,NewStr:PWideChar):PWideChar;
var
  i,j,l:integer;
  k:integer;
  p:PWideChar;
begin
  result:=src;
  p:=StrPosW(src,SubStr);
  if p=nil then exit;
  i:=StrLenW(SubStr);
  j:=StrLenW(NewStr);
  l:=i-j;
  repeat
    if j=0 then
      StrCopyW(p,p+i)
    else
    begin
      k:=(StrLenW(p)+1)*SizeOf(WideChar);
      if l>0 then
        move((p+l)^,p^,k-l*SizeOf(WideChar))
      else if l<>0 then
        move(p^,(p-l)^,k);
      move(NewStr^,p^,j*SizeOf(WideChar)); {new characters}
      inc(p,j);
    end;
    p:=StrPosW(p,SubStr);
    if p=nil then break;
  until false;
end;

function CharReplace(dst:PAnsiChar;old,new:AnsiChar):PAnsiChar;
begin
  result:=dst;
  if dst<>nil then
  begin
    while dst^<>#0 do
    begin
      if dst^=old then dst^:=new;
      inc(dst);
    end;
  end;
end;

function CharReplaceW(dst:PWideChar;old,new:WideChar):PWideChar;
begin
  result:=dst;
  if dst<>nil then
  begin
    while dst^<>#0 do
    begin
      if dst^=old then dst^:=new;
      inc(dst);
    end;
  end;
end;

function StrCmp(a,b:PAnsiChar;n:integer=0):integer; // CompareString
begin
  result:=0;
  if (a=nil) and (b=nil) then
    exit;
  if (a=nil) or (b=nil) then
  begin
    result:=-1;
    exit;
  end;
  repeat
    result:=ord(a^)-ord(b^);
    if (result<>0) or (a^=#0) then
      break;
    inc(a);
    inc(b);
    dec(n);
  until n=0;
end;

function StrCmpW(a,b:PWideChar;n:integer=0):integer;
begin
  result:=0;
  if (a=nil) and (b=nil) then
    exit;
  if (a=nil) or (b=nil) then
  begin
    result:=-1;
    exit;
  end;
  repeat
    result:=ord(a^)-ord(b^);
    if (result<>0) or (a^=#0) then
      break;
    inc(a);
    inc(b);
    dec(n);
  until n=0;
end;

function StrEnd(const a:PAnsiChar):PAnsiChar;
begin
  result:=a;
  if result<>nil then
    while result^<>#0 do inc(result);
end;

function StrEndW(const a:PWideChar):PWideChar;
begin
  result:=a;
  if result<>nil then
    while result^<>#0 do inc(result);
end;

function StrScan(src:PAnsiChar;c:AnsiChar):PAnsiChar;
begin
  if src<>nil then
  begin
    while (src^<>#0) and (src^<>c) do inc(src);
    if src^<>#0 then
    begin
      result:=src;
      exit;
    end;
  end;
  result:=nil;
end;

function StrRScan(src:PAnsiChar;c:AnsiChar):PAnsiChar;
begin
  if src<>nil then
  begin
    result:=StrEnd(src);
    while (result>=src) and (result^<>c) do dec(result);
    if result<src then
      result:=nil;
  end
  else
    result:=nil;
end;

function StrScanW(src:PWideChar;c:WideChar):PWideChar;
begin
  if src<>nil then
  begin
    while (src^<>#0) and (src^<>c) do inc(src);
    if src^<>#0 then
    begin
      result:=src;
      exit;
    end;
  end;
  result:=nil;
end;

function StrRScanW(src:PWideChar;c:WideChar):PWideChar;
begin
  if src<>nil then
  begin
    result:=StrEndW(src);
    while (result>=src) and (result^<>c) do dec(result);
    if result<src then
      result:=nil;
  end
  else
    result:=nil;
end;

function StrLen(Str: PAnsiChar): Cardinal;
var
  P : PAnsiChar;
begin
  P := Str;
  if P<>nil then
    while (P^ <> #0) do Inc(P);
  Result := (P - Str);
end;

function StrLenW(Str: PWideChar): Cardinal;
var
  P : PWideChar;
begin
  P := Str;
  if P<>nil then
    while (P^ <> #0) do Inc(P);
  Result := (P - Str);
end;

function StrCat(Dest: PAnsiChar; const Source: PAnsiChar): PAnsiChar;
begin
  if Dest<>nil then
    StrCopy(StrEnd(Dest), Source);
  Result := Dest;
end;

function StrCatW(Dest: PWideChar; const Source: PWideChar): PWideChar;
begin
  if Dest<>nil then
    StrCopyW(StrEndW(Dest), Source);
  Result := Dest;
end;

function StrCatE(Dest: PAnsiChar; const Source: PAnsiChar): PAnsiChar;
begin
  if Dest<>nil then
    result:=StrCopyE(StrEnd(Dest), Source)
  else
    result:=nil;
end;

function StrCatEW(Dest: PWideChar; const Source: PWideChar): PWideChar;
begin
  if Dest<>nil then
    result:=StrCopyEW(StrEndW(Dest), Source)
  else
    result:=nil;
end;

function StrPos(const aStr, aSubStr: PAnsiChar): PAnsiChar;
var
  Str, SubStr: PAnsiChar;
  ch: AnsiChar;
begin
  if (aStr = nil) or (aStr^ = #0) or (aSubStr = nil) or (aSubStr^ = #0) then
  begin
    Result := nil;
    Exit;
  end;
  Result := aStr;
  ch := aSubStr^;
  repeat
    if Result^ = ch then
    begin
      Str := Result;
      SubStr := aSubStr;
      repeat
        Inc(Str);
        Inc(SubStr);
        if SubStr^ = #0 then exit;
        if Str^ = #0 then
        begin
          Result := nil;
          exit;
        end;
        if Str^ <> SubStr^ then break;
      until (FALSE);
    end;
    Inc(Result);
  until (Result^ = #0);
  Result := nil;
end;

function StrIndex(const aStr, aSubStr: PAnsiChar):integer;
var
  p:PAnsiChar;
begin
  p:=StrPos(aStr,aSubStr);
  if p=nil then
    result:=0
  else
    result:=p-aStr+1;
end;

function StrPosW(const aStr, aSubStr: PWideChar): PWideChar;
var
  Str, SubStr: PWideChar;
  ch: WideChar;
begin
  if (aStr = nil) or (aStr^ = #0) or (aSubStr = nil) or (aSubStr^ = #0) then
  begin
    Result := nil;
    Exit;
  end;
  Result := aStr;
  ch := aSubStr^;
  repeat
    if Result^ = ch then
    begin
      Str := Result;
       SubStr := aSubStr;
      repeat
        Inc(Str);
        Inc(SubStr);
        if SubStr^ = #0 then exit;
        if Str^ = #0 then
        begin
          Result := nil;
          exit;
        end;
        if Str^ <> SubStr^ then break;
      until (FALSE);
    end;
    Inc(Result);
  until (Result^ = #0);
  Result := nil;
end;

function StrIndexW(const aStr, aSubStr: PWideChar):integer;
var
  p:PWideChar;
begin
  p:=StrPosW(aStr,aSubStr);
  if p=nil then
    result:=0
  else
    result:=(p-aStr)+1; //!!!!
end;

//----- filename work -----

function ChangeExt(src,ext:PAnsiChar):PAnsiChar;
var
  i,j:integer;
begin
  i:=StrLen(src);
  j:=i;
  while (i>0) and (src[i]<>'\') and (src[i]<>':') and (src[i]<>'.') do dec(i);
  if src[i]<>'.' then
  begin
    i:=j;
    src[i]:='.';
  end;
  if ext=nil then
    ext:='';
  StrCopy(src+i+1,ext);
  result:=src;
end;

function ChangeExtW(src,ext:PWideChar):PWideChar;
var
  i,j:integer;
begin
  i:=StrLenW(src);
  j:=i;
  while (i>0) and (src[i]<>'\') and (src[i]<>':') and (src[i]<>'.') do dec(i);
  if src[i]<>'.' then
  begin
    i:=j;
    src[i]:='.';
  end;
  if ext=nil then
    ext:='';
  StrCopyW(src+i+1,ext);
  result:=src;
end;

function Extract(s:PAnsiChar;name:Boolean=true):PAnsiChar;
var
  i:integer;
begin
  i:=StrLen(s)-1;
//  j:=i;
  while (i>=0) and ((s[i]<>'\') and (s[i]<>'/')) do dec(i);
  if name then
  begin
    StrDup(result,s+i+1);
//    mGetMem(result,(j-i+1));
//    StrCopy(result,s+i+1);
  end
  else
  begin
    StrDup(result,s,i+1);
  end;
end;

function ExtractW(s:PWideChar;name:Boolean=true):PWideChar;
var
  i:integer;
begin
  i:=StrLenW(s)-1;
//  j:=i;
  while (i>=0) and ((s[i]<>'\') and (s[i]<>'/')) do dec(i);
  if name then
  begin
    StrDupW(result,s+i+1);
//    mGetMem(result,(j-i+1)*SizeOf(WideChar));
//    StrCopyW(result,s+i+1);
  end
  else
  begin
    StrDupW(result,s,i+1);
  end;
end;

function GetExt(fname,dst:PWideChar;maxlen:dword=100):PWideChar;
var
  ppc,pc:PWideChar;
begin
  result:=dst;
  dst^:=#0;
  if (fname<>nil) and (fname^<>#0) then
  begin
    pc:=StrEndW(fname)-1;
    while (pc>fname) and ((pc^='"') or (pc^=' ')) do dec(pc);
    ppc:=pc+1;
    while (pc>fname) and (pc^<>'.') do
    begin
      if maxlen=0 then exit;
      if not (AnsiChar(pc^) in ['0'..'9','A'..'Z','_','a'..'z']) then exit;
      dec(maxlen);
      dec(pc); //empty name not allowed!
    end;
    if pc>fname then
    begin
      repeat
        inc(pc);
        if pc=ppc then
        begin
          dst^:=#0;
          break;
        end;
        if (pc^>='a') and (pc^<='z') then
          dst^:=WideChar(ord(pc^)-$20)
        else
          dst^:=pc^;
        inc(dst);
      until false;
    end;
  end;
end;

function GetExt(fname,dst:PAnsiChar;maxlen:dword=100):PAnsiChar;
var
  ppc,pc:PAnsiChar;
begin
  result:=dst;
  dst^:=#0;
  if (fname<>nil) and (fname^<>#0) then
  begin
    pc:=StrEnd(fname)-1;
    while (pc>fname) and ((pc^='"') or (pc^=' ')) do dec(pc);
    ppc:=pc+1;
    while (pc>fname) and (pc^<>'.') do
    begin
      if maxlen=0 then exit;
      if not (AnsiChar(pc^) in ['0'..'9','A'..'Z','_','a'..'z']) then exit;
      dec(maxlen);
      dec(pc); //empty name not allowed!
    end;
    if pc>fname then
    begin
      repeat
        inc(pc);
        if pc=ppc then
        begin
          dst^:=#0;
          break;
        end;
        if (pc^>='a') and (pc^<='z') then
          dst^:=AnsiChar(ord(pc^)-$20)
        else
          dst^:=pc^;
        inc(dst);
      until false;
    end;
  end;
end;

function isPathAbsolute(path:PWideChar):boolean;
begin
  result:=((path[1]=':') and (path[2]='\')) or ((path[0]='\') {and (path[1]='\')}) or
          (StrPosW(path,'://')<>nil);
end;

function isPathAbsolute(path:PAnsiChar):boolean;
begin
  result:=((path[1]=':') and (path[2]='\')) or ((path[0]='\') {and (path[1]='\')}) or
          (StrPos(path,'://')<>nil);
end;

//----- Date and Time -----

function TimeToInt(stime:PAnsiChar):integer;
var
  hour,minute,sec,len,i:integer;
begin
  len:=StrLen(stime);
  i:=0;
  sec   :=0;
  minute:=0;
  hour  :=0;
  while i<len do
  begin
    if (stime[i]<'0') or (stime[i]>'9') then
    begin
      if minute>0 then
        hour:=minute;
      minute:=sec;
      sec:=0;
    end
    else
      sec:=sec*10+ord(stime[i])-ord('0');
    inc(i);
  end;
  result:=hour*3600+minute*60+sec;
end;

function TimeToInt(stime:PWideChar):integer;
var
  buf:array [0..63] of AnsiChar;
begin
  result:=TimeToInt(FastWideToAnsiBuf(stime,buf));
end;

function IntToTime(dst:PAnsiChar;Time:integer):PAnsiChar;
var
  day,hour,minute,sec:array [0..7] of AnsiChar;
  d,h:integer;
begin
  result:=dst;
  h:=Time div 3600;
  dec(Time,h*3600);
  IntToStr(sec,(Time mod 60),2);
  d:=h div 24;
  if d>0 then
  begin
    h:=h mod 24;
    IntToStr(day,d);
    dst^:=day[0]; inc(dst);
    if day[1]<>#0 then        // now only 99 days max
    begin
      dst^:=day[1]; inc(dst);
    end;
    dst^:=' '; inc(dst);
  end;
  if h>0 then
  begin
    IntToStr(hour,h);
    IntToStr(minute,(Time div 60),2);
    dst^:=hour[0]; inc(dst);
    if hour[1]<>#0 then
    begin
      dst^:=hour[1]; inc(dst);
    end;
    dst^:=':';    inc(dst);
    dst^:=minute[0]; inc(dst);
    dst^:=minute[1]; inc(dst);
  end
  else
  begin
    IntToStr(minute,Time div 60);
    dst^:=minute[0]; inc(dst);
    if minute[1]<>#0 then
    begin
      dst^:=minute[1]; inc(dst);
    end;
  end;
  dst^:=':';    inc(dst);
  dst^:=sec[0]; inc(dst);
  dst^:=sec[1]; inc(dst);
  dst^:=#0;
end;

function IntToTime(dst:PWideChar;Time:integer):PWideChar;
var
  buf:array [0..63] of AnsiChar;
begin
  result:=FastAnsiToWideBuf(IntToTime(buf,Time),dst);
end;

//----- String/number conversion -----

function NumToInt(src:PWideChar):int64;
begin
  result:=0;
  if src=nil then exit;

  if (src[0]='$') and
     (AnsiChar(src[1]) in sHexNum) then
    result:=HexToInt(src+1)
  else
  if (src[0]='0') and
     (src[1]='x') and
     (AnsiChar(src[2]) in sHexNum) then
    result:=HexToInt(src+2)
  else
    result:=StrToInt(src);
end;

function NumToInt(src:PAnsiChar):int64;
begin
  result:=0;
  if src=nil then exit;

  if (src[0]='$') and
     (src[1] in sHexNum) then
    result:=HexToInt(src+1)
  else
  if (src[0]='0') and
     (src[1]='x') and
     (src[2] in sHexNum) then
    result:=HexToInt(src+2)
  else
    result:=StrToInt(src);
end;

function StrToInt(src:PWideChar):int64;
var
  sign:boolean;
begin
  result:=0;
  if src<>nil then
  begin
    sign:=src^='-';
    if sign then inc(src);
    while src^<>#0 do
    begin
      if (src^>='0') and (src^<='9') then
        result:=result*10+ord(src^)-ord('0')
      else
        break;
      inc(src);
    end;
    if sign then result:=-result;
  end;
end;

function StrToInt(src:PAnsiChar):int64;
var
  sign:boolean;
begin
  result:=0;
  if src<>nil then
  begin
    sign:=src^='-';
    if sign then inc(src);
    while src^<>#0 do
    begin
      if (src^>='0') and (src^<='9') then
        result:=result*10+ord(src^)-ord('0')
      else
        break;
      inc(src);
    end;
    if sign then result:=-result;
  end;
end;

function IntStrLen(value:int64; base:integer=10):integer;
var
  i:uint64;
begin
  result:=0;
  if (base=10) and (value<0) then
    inc(result);
  i:=ABS(value);
  repeat
    i:=i div base;
    inc(result);
  until i=0;
end;

function IntToStr(dst:PAnsiChar;value:int64;digits:integer=0):PAnsiChar;
var
  i:uint64;
begin
  if digits<=0 then
  begin
    if value<0 then
      digits:=1
    else
      digits:=0;
    i:=ABS(value);
    repeat
      i:=i div 10;
      inc(digits);
    until i=0;
  end;
  dst[digits]:=#0;
  i:=ABS(value);
  repeat
    dec(digits);
    dst[digits]:=AnsiChar(ord('0')+(i mod 10));
    i:=i div 10;
    if (value<0) and (digits=1) then
    begin
      dst[0]:='-';
      break;
    end;
  until digits=0;
  result:=dst;
end;

function IntToStr(dst:PWideChar;value:int64;digits:integer=0):PWideChar;
var
  i:uint64;
begin
  if digits<=0 then
  begin
    if value<0 then
      digits:=1
    else
      digits:=0;
    i:=ABS(value);
    repeat
      i:=i div 10;
      inc(digits);
    until i=0;
  end;
  dst[digits]:=#0;
  i:=ABS(value);
  repeat
    dec(digits);
    dst[digits]:=WideChar(ord('0')+(i mod 10));
    i:=i div 10;
    if (value<0) and (digits=1) then
    begin
      dst[0]:='-';
      break;
    end;
  until digits=0;
  result:=dst;
end;

function HexToInt(src:PWideChar;len:cardinal=$FFFF):int64;
begin
  result:=0;
  while (src^<>#0) and (len>0) do
  begin
    if (src^>='0') and (src^<='9') then
      result:=result*16+ord(src^)-ord('0')
    else if ((src^>='A') and (src^<='F')) then
      result:=result*16+ord(src^)-ord('A')+10
    else if ((src^>='a') and (src^<='f')) then
      result:=result*16+ord(src^)-ord('a')+10
    else
      break;
    inc(src);
    dec(len);
  end;
end;

function HexToInt(src:PAnsiChar;len:cardinal=$FFFF):int64;
begin
  result:=0;
  while (src^<>#0) and (len>0) do
  begin
    if (src^>='0') and (src^<='9') then
      result:=result*16+ord(src^)-ord('0')
    else if ((src^>='A') and (src^<='F')) then
      result:=result*16+ord(src^)-ord('A')+10
    else if ((src^>='a') and (src^<='f')) then
      result:=result*16+ord(src^)-ord('a')+10
    else
      break;
    inc(src);
    dec(len);
  end;
end;

function IntToHex(dst:pWidechar;value:int64;digits:integer=0):PWideChar;
var
  i:dword;
begin
  if digits<=0 then
  begin
    digits:=0;
    i:=value;
    repeat
      i:=i shr 4;
      inc(digits);
    until i=0;
  end;
  dst[digits]:=#0;
  repeat
    Dec(digits);
    dst[digits]:=WideChar(HexDigitChr[value and $F]);
    value:=value shr 4;
  until digits=0;
  result:=dst;
end;

function IntToHex(dst:PAnsiChar;value:int64;digits:integer=0):PAnsiChar;
var
  i:dword;
begin
  if digits<=0 then
  begin
    digits:=0;
    i:=value;
    repeat
      i:=i shr 4;
      inc(digits);
    until i=0;
  end;
  dst[digits]:=#0;
  repeat
    Dec(digits);
    dst[digits]:=HexDigitChr[value and $F];
    value:=value shr 4;
  until digits=0;
  result:=dst;
end;

function GetPairChar(ch:AnsiChar):AnsiChar;
begin
  case ch of
    '[': result:=']';
    '<': result:='>';
    '(': result:=')';
    '{': result:='}';
  else // ' and " too
    result:=ch;
  end;
end;

function GetPairChar(ch:WideChar):WideChar;
begin
  case ch of
    '[': result:=']';
    '<': result:='>';
    '(': result:=')';
    '{': result:='}';
  else // ' and " too
    result:=ch;
  end;
end;

function FastWideToAnsiBuf(src:PWideChar;dst:PAnsiChar;len:cardinal=cardinal(-1)):PAnsiChar;
begin
  result:=dst;
  if src<>nil then
  begin
    repeat
      dst^:=AnsiChar(src^);
      if src^=#0 then
        break;
      dec(len);
      if len=0 then
      begin
        (dst+1)^:=#0;
        break;
      end;
      inc(src);
      inc(dst);
    until false;
  end
  else
    dst^:=#0;
end;

function FastWideToAnsi(src:PWideChar;var dst:PAnsiChar):PAnsiChar;
begin
  if src=nil then
    dst:=nil
  else
  begin
    mGetMem(dst,StrLenW(src)+1);
    FastWideToAnsiBuf(src,dst);
  end;
  result:=dst;
end;

function FastAnsiToWideBuf(src:PAnsiChar;dst:PWideChar;len:cardinal=cardinal(-1)):PWideChar;
begin
  result:=dst;
  if src<>nil then
  begin
    repeat
      dst^:=WideChar(src^);
      if src^=#0 then
        break;
      dec(len);
      if len=0 then
      begin
        (dst+1)^:=#0;
        break;
      end;
      inc(src);
      inc(dst);
    until false;
  end
  else
    dst^:=#0;
end;

function FastAnsiToWide(src:PAnsiChar;var dst:PWideChar):PWideChar;
begin
  if src=nil then
    dst:=nil
  else
  begin
    mGetMem(dst,(StrLen(src)+1)*SizeOf(WideChar));
    FastAnsiToWideBuf(src,dst);
  end;
  result:=dst;
end;

begin
  CheckSystem;
end.

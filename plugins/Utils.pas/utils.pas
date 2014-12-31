unit utils;

interface

uses windows;

function SaveTemporaryW(ptr:pointer;size:dword;ext:PWideChar=nil):PWideChar;
function SaveTemporary (ptr:pointer;size:dword;ext:PAnsiChar=nil):PAnsiChar;

implementation

uses common,io;

function SaveTemporaryW(ptr:pointer;size:dword;ext:PWideChar=nil):PWideChar;
var
  buf,buf1:array [0..MAX_PATH-1] of WideChar;
  f:THANDLE;
begin
  GetTempPathW(MAX_PATH,buf);
  GetTempFileNameW(buf,'wat',GetCurrentTime,buf1);
  ChangeExtW(buf1,ext);

  f:=ReWrite(buf1);
  BlockWrite(f,PByte(ptr)^,size);
  CloseHandle(f);
  StrDupW(result,buf1);
end;

function SaveTemporary(ptr:pointer;size:dword;ext:PAnsiChar=nil):PAnsiChar;
var
  buf,buf1:array [0..MAX_PATH-1] of AnsiChar;
  f:THANDLE;
begin
  GetTempPathA(SizeOf(buf),buf);
  GetTempFileNameA(buf,'wat',GetCurrentTime,buf1);
  ChangeExt(buf1,ext);

  f:=ReWrite(buf1);
  BlockWrite(f,PByte(ptr)^,size);
  CloseHandle(f);
  StrDup(result,buf1);
end;

end.
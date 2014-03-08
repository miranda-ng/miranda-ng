unit io;

interface
uses windows;

function Reset  (fname:PWideChar):THANDLE; overload;
function Reset  (fname:PAnsiChar):THANDLE; overload;
function ReWrite(fname:PWideChar):THANDLE; overload;
function ReWrite(fname:PAnsiChar):THANDLE; overload;
function Append (fname:PWideChar):THANDLE; overload;
function Append (fname:PAnsiChar):THANDLE; overload;

function GetFSize(name:PWideChar):dword; overload;
function GetFSize(name:PAnsiChar):dword; overload;
function FileExists(fname:PAnsiChar):Boolean; overload;
function FileExists(fname:PWideChar):Boolean; overload;

function Skip(f:THANDLE;count:integer):integer;
function Seek(f:THANDLE;pos:integer):integer;
function FilePos(f:THANDLE):dword;
function FileSize(f:THANDLE):dword;
function Eof(f:THANDLE):boolean;

function BlockRead (f:THANDLE;var buf;size:integer):dword;
function BlockWrite(f:THANDLE;var buf;size:integer):dword;

function ForceDirectories(path:PAnsiChar):boolean; overload;
function ForceDirectories(path:PWideChar):boolean; overload;
function DirectoryExists(Directory:PAnsiChar):Boolean; overload;
function DirectoryExists(Directory:PWideChar):Boolean; overload;

implementation

function Reset(fname:PWideChar):THANDLE;
begin
  result:=CreateFileW(fname,GENERIC_READ,FILE_SHARE_READ+FILE_SHARE_WRITE,nil,OPEN_EXISTING,0,0);
end;

function Reset(fname:PAnsiChar):THANDLE;
begin
  result:=CreateFileA(fname,GENERIC_READ,FILE_SHARE_READ+FILE_SHARE_WRITE,nil,OPEN_EXISTING,0,0);
end;

function Append(fname:PWideChar):THANDLE;
begin
  result:=CreateFileW(fname,GENERIC_WRITE,FILE_SHARE_READ,nil,OPEN_ALWAYS,0,0);
  SetFilePointer(result,0,nil,FILE_END);
end;

function Append(fname:PAnsiChar):THANDLE;
begin
  result:=CreateFileA(fname,GENERIC_WRITE,FILE_SHARE_READ,nil,OPEN_ALWAYS,0,0);
  SetFilePointer(result,0,nil,FILE_END);
end;

function ReWrite(fname:PWideChar):THANDLE; overload;
begin
  result:=CreateFileW(fname,GENERIC_WRITE,FILE_SHARE_READ,nil,CREATE_ALWAYS,0,0);
end;

function ReWrite(fname:PAnsiChar):THANDLE; overload;
begin
  result:=CreateFileA(fname,GENERIC_WRITE,FILE_SHARE_READ,nil,CREATE_ALWAYS,0,0);
end;

function Skip(f:THANDLE;count:integer):integer;
begin
  result:=SetFilePointer(f,count,nil,FILE_CURRENT);
end;

function Eof(f:THANDLE):boolean;
begin
  result:=FilePos(f)>=FileSize(f);
end;

function Seek(f:THANDLE;pos:integer):integer;
begin
  result:=SetFilePointer(f,pos,nil,FILE_BEGIN);
end;

function FilePos(f:THANDLE):dword;
begin
  result:=SetFilePointer(f,0,nil,FILE_CURRENT);
end;

function FileSize(f:THANDLE):dword;
begin
  result:=GetFileSize(f,nil);
end;

function BlockRead(f:THANDLE;var buf;size:integer):dword;
begin
  ReadFile(f,buf,size,result,nil);
end;

function BlockWrite(f:THANDLE;var buf;size:integer):dword;
begin
  WriteFile(f,buf,size,result,nil);
end;

function GetFSize(name:PWideChar):dword;
var
  lRec:WIN32_FIND_DATAW;
  h:THANDLE;
begin
  h:=FindFirstFileW(name,lRec);
  if h=THANDLE(INVALID_HANDLE_VALUE) then
    result:=0
  else
  begin
    result:=lRec.nFileSizeLow;
    FindClose(h);
  end;
end;

function GetFSize(name:PAnsiChar):dword;
var
  lRec:TWin32FindDataA;
  h:THANDLE;
begin
  h:=FindFirstFileA(name,lRec);
  if h=THANDLE(INVALID_HANDLE_VALUE) then
    result:=0
  else
  begin
    result:=lRec.nFileSizeLow;
    FindClose(h);
  end;
end;

function ForceDirectories(path:PAnsiChar):boolean;
var
  p,pc:PAnsiChar;
  i:cardinal;
  c:AnsiChar;
begin
  result:=true;
  if DirectoryExists(path) then exit;
  if (path<>nil) and (path^<>#0) then
  begin
    i:=lstrlena(path)+1;
    GetMem(pc,i);
    move(path^,pc^,i);
    p:=pc;
    if (p^ in ['A'..'Z','a'..'z']) and (p[1]=':') then inc(p,2);
    if p^ in ['/','\'] then inc(p);
    c:=#0;
    while p^<>#0 do
    begin
      c:=' ';
      if (p^ in ['/','\']) and (p[1]<>#0) then
      begin
        c:=p^;
        p^:=#0;
        if not CreateDirectoryA(pc,nil) then
        begin
          if GetLastError<>ERROR_ALREADY_EXISTS then
          begin
            result:=false;
            FreeMem(pc);
            exit;
          end;
        end;
        p^:=c;
      end;
      inc(p);
    end;
    if (c<>#0) and (c=' ') then
      if not CreateDirectoryA(pc,nil) then
        result:=false;
    FreeMem(pc);
  end;
end;

function ForceDirectories(path:PWideChar):boolean;
var
  p,pc:PWideChar;
  i:cardinal;
  c:WideChar;
begin
  result:=true;
  if DirectoryExists(path) then exit;
  if (path<>nil) and (path^<>#0) then
  begin
    i:=(lstrlenw(path)+1)*SizeOf(WideChar);
    GetMem(pc,i);
    move(path^,pc^,i);
    p:=pc;
    if (((p^>='A') and (p^<='Z')) or ((p^>='a') and (p^<='z'))) and (p[1]=':') then inc(p,2);
    if (p^='/') or (p^='\') then inc(p);
    c:=#0;
    while p^<>#0 do
    begin
      c:=' ';
      if ((p^='/') or (p^='\')) and (p[1]<>#0) then
      begin
        c:=p^;
        p^:=#0;
        if not CreateDirectoryW(pc,nil) then
          if GetLastError<>ERROR_ALREADY_EXISTS then
          begin
            result:=false;
            FreeMem(pc);
            exit;
          end;
        p^:=c;
      end;
      inc(p);
    end;
    if (c<>#0) and (c=' ') then
      if not CreateDirectoryW(pc,nil) then
        result:=false;
    FreeMem(pc);
  end;
end;

function DirectoryExists(Directory:PAnsiChar):Boolean;
var
  Code: integer;
begin
  Code := GetFileAttributesA(Directory);
  Result := (Code<>-1) and ((Code and FILE_ATTRIBUTE_DIRECTORY)<>0);
end;

function DirectoryExists(Directory:PWideChar):Boolean;
var
  Code: integer;
begin
  Code := GetFileAttributesW(Directory);
  Result := (Code<>-1) and ((Code and FILE_ATTRIBUTE_DIRECTORY)<>0);
end;

function FileExists(fname:PAnsiChar):Boolean;
var
  Code: integer;
begin
  Code := GetFileAttributesA(fname);
  Result := (Code<>-1) and ((Code and FILE_ATTRIBUTE_DIRECTORY)=0);
end;

function FileExists(fname:PWideChar):Boolean;
var
  Code: integer;
begin
  Code := GetFileAttributesW(fname);
  Result := (Code<>-1) and ((Code and FILE_ATTRIBUTE_DIRECTORY)=0);
end;

end.

{any utils}
function GetCover(var dst:pWideChar;MFile:pWideChar):boolean;
var
  line,line1:array [0..511] of WideChar;
  p,p1:PWideChar;
  i:integer;
  fdata:WIN32_FIND_DATAW;
  hTmp:THANDLE;
  wr,wr1:pWideChar;
begin
  result:=false;
  dst:=nil;
  if (CoverPaths=nil) or (CoverPaths^=#0) then exit;
  p:=CoverPaths;
  repeat
    p1:=p;
    while p^>=' ' do inc(p);
    i:=p-p1;
    if i>0 then
    begin
      move(p1^,line,i*SizeOf(WideChar));
      line[i]:=#0;
      if ServiceExists(MS_WAT_REPLACETEXT)<>0 then
        wr:=pWideChar(CallService(MS_WAT_REPLACETEXT,0,lparam(@line)))
      else
        wr:=@line;

      if isPathAbsolute(wr) then
      begin
        hTmp:=FindFirstFileW(wr,fdata);
      end
      else
      begin
        wr1:=ExtractW(MFile,false);
        StrCopyW(line,wr1);
        mFreeMem(wr1);
        StrCatW(line,wr);
        hTmp:=FindFirstFileW(line,fdata);
      end;
      if THANDLE(hTmp)<>INVALID_HANDLE_VALUE then
      begin
        wr1:=ExtractW(line,false);
        StrCopyW(line1,wr1);
        mFreeMem(wr1);
        StrCatW(line1,pWideChar(@fdata.cFileName));

        GetFullPathNameW(line1,SizeOf(line) div SizeOf(WideChar),line,wr1);
        StrDupW(dst,line);
        result:=true;
        FindClose(hTmp); //!!
      end;
      if wr<>@line then mFreeMem(wr);
      if result then break;
    end;
    while p^<' ' do
    begin
      if p^=#0 then break;
      inc(p);
    end;
  until p^=#0;
end;

function GetLyric(var dst:pWideChar;MFile:pWideChar):boolean;
var
  buf:array [0..511] of WideChar;
  f:THANDLE;
  size:integer;
  tmp:PAnsiChar;
begin
  StrCopyW(buf,MFile);
  ChangeExtW(buf,'txt');
  f:=Reset(buf);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
  begin
    dst:=nil;
    result:=false;
    exit;
  end;
  size:=FileSize(f);
  if size>0 then
  begin
    mGetMem(tmp,size+1);
    BlockRead(f,tmp^,size);
    tmp[size]:=#0;
    AnsiToWide(tmp,dst);
    mFreeMem(tmp);
  end;
  CloseHandle(f);
  result:=true;
end;

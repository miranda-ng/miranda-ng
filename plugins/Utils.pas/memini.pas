unit memini;

interface

function OpenStorage(fname:pWideChar):pointer; overload;
function OpenStorage(fname:pAnsiChar):pointer; overload;
function OpenStorageBuf(buf:pAnsiChar):pointer;
procedure CloseStorage(storage:pointer);

//function GetNamespaceList(storage:pointer):pAnsiChar;
function GetSectionList(storage:pointer;namespace:pAnsiChar=nil):pAnsiChar;
procedure FreeSectionList(ptr:pAnsiChar);

function GetParamStr(storage:pointer;section,param:pAnsiChar;default:pAnsiChar=nil;
         namespace:pAnsiChar=nil):pAnsiChar;
function GetParamInt(storage:pointer;section,param:pAnsiChar;default:integer=0;
         namespace:pAnsiChar=nil):integer;

function SearchSection(storage:pointer;section:pAnsiChar;namespace:pAnsiChar=nil):pointer;
function GetSectionName(section:pointer):pAnsiChar;

function GetParamSectionStr(section:pointer;param:pAnsiChar;default:pAnsiChar=nil):pAnsiChar;
function GetParamSectionInt(section:pointer;param:pAnsiChar;default:integer=0):integer;

implementation

uses windows,io,common;

const
  arstep = 8;
const
  ns_separator   = ':';
  line_separator = '\';
type
  pParam = ^tParam;
  tParam = record
    hash  :integer;   // param name hash
    name  :pAnsiChar; // points to source (for write only)
    value :pAnsiChar; // points to source? or modified
    assign:boolean;   // newly assigned value or in INI buffer
  end;
  pSection = ^tSection;
  tSection = record
    ns      :integer;    // namespace hash
    code    :integer;    // section name hash
    full    :integer;    // namespace+section name hash
    fullname:pAnsiChar;  // pointer to namespace:name
    name    :pAnsiChar;  // pointer to name only

    numparam:integer;
    arParams:array of tParam;
  end;
  pStorage = ^tStorage;
  tStorage = record
    name     :pAnsiChar; // filename
    buffer   :pAnsiChar; // source (INI) text

    numsect  :integer;
    arSection: array of tSection;
  end;


function HashOf(txt:pAnsiChar):integer;
begin
  result:=Hash(txt,StrLen(txt));
{
  result:=0;
  while txt^<>#0 do
  begin
    result:=((result shl 2) or (result shr (SizeOf(result)*8-2))) xor Ord(UpCase(txt^));
    inc(txt);
  end;
}
end;

// sections adds 1 by 1, without duplicate check
procedure AddSection(data:pStorage;anamespace,aname:pAnsiChar);
var
  i:integer;
  fnhash:integer;
  c:AnsiChar;
begin
  // search section with same name
  fnhash:=HashOf(anamespace);
  for i:=0 to data.numsect-1 do
  begin
    if data.arSection[i].full=fnhash then
      exit; // Doing nothing!
  end;

  // add section
  if data.numsect>High(data.arSection) then
    SetLength(data.arSection,Length(data.arSection)+arstep);

  FillChar(data.arSection[data.numsect],SizeOf(tSection),0);
  with data.arSection[data.numsect] do
  begin
    fullname:=anamespace;
    name    :=aname;
    full    :=fnhash;
    if anamespace<>aname then
    begin
      c:=(aname-1)^;
      (aname-1)^:=#0;
      code:=HashOf(aname);
      ns  :=HashOf(anamespace);
      (aname-1)^:=c;
    end;
  end;
  inc(data.numsect);
end;

procedure AddParam(data:pStorage;aname,avalue:pAnsiChar;assignvalue:boolean);
begin
  // search param with same name?
  
  with data.arSection[data.numsect-1] do
  begin
    // add param
    if numparam>High(arParams) then
      SetLength(arParams,Length(arParams)+arstep);

    FillChar(arParams[numparam],SizeOf(tParam),0);
    with arParams[numparam] do
    begin
      hash  :=HashOf(aname);
      name  :=aname;
      value :=avalue;
      assign:=assignvalue;
    end;
    inc(numparam);
  end;
end;

// quotes, multiline etc
// result = pointer to non-parameter line
// pointers: start of value, start of current line, end of value in line, end of current line
function ProcessParamValue(var start:pAnsiChar):pAnsiChar;
var
  lineend,eol,dst,bov:pAnsiChar;
  multiline,crlf:boolean;
begin

  dst:=start;
  bov:=start;
  result:=nil;
  repeat
    multiline:=false;
    crlf     :=false;
    // skip starting spaces
    while start^ in [#9,' '] do inc(start);

    if start^ in [#0,#10,#13] then // empty value or end
    begin
      while start^ in [#10,#13] do inc(start);
      exit;
    end;

    lineend:=start;
    while not (lineend^ in [#0,#10,#13]) do inc(lineend);
    eol:=lineend;
    dec(lineend);
    while lineend^ in [#9,' '] do dec(lineend);

    if lineend^=line_separator then // multiline or part of value
    begin
      if (lineend-1)^ in [#9,' '] then // multiline
      begin
        dec(lineend);
        multiline:=true;
        while lineend^ in [#9,' '] do dec(lineend);
      end
      // double separator = multiline + crlf saving
      else if ((lineend-1)^=line_separator) and ((lineend-2)^ in [#9,' ']) then
      begin
        dec(lineend,2);
        multiline:=true;
        crlf     :=true;
        while lineend^ in [#9,' '] do dec(lineend);
      end;
    end;
    // lineend points to last char
    // start points to first char
    // eol points to end of line

    //!! now just starting/ending quotes
    if (start^ in ['''','"']) and (lineend^ in ['''','"']) then
    begin
      inc(start);
      dec(lineend);
    end;

    while start<=lineend do
    begin
      dst^:=start^;
      inc(dst);
      inc(start);
    end;
    if crlf then
    begin
      dst^:=#13;
      inc(dst);
      dst^:=#10;
      inc(dst);
    end;
    start:=eol;
    while start^ in [#10,#13] do inc(start);

  until not multiline;
  dst^:=#0;
  result:=bov;
end;

procedure TranslateData(data:pStorage);
var
  pc2,pc1,pc:pAnsiChar;
  len:integer;
begin
  pc:=data^.buffer;
  data.numsect:=0;
  while pc^<>#0 do
  begin
    while pc^ in [#9,#10,#13,' '] do inc(pc);

    // comment
    if pc^=';' then
    begin
      // skip to next line (or end)
      while not (pc^ in [#0,#10,#13]) do inc(pc);
      // skip empty
      while pc^ in [#9,#10,#13,' '] do inc(pc);
    end
    // section
    else if pc^='[' then
    begin

      inc(pc);
      //!! without #0 check
      pc1:=pc;
      pc2:=pc;
//??      while pc^ in sLatWord do inc(pc);
      {}
      repeat
        len:=CharUTF8Len(pc);
        if (len>1) or (pc^ in sLatWord) then
          inc(pc,len)
        else
          break;
      until false;
      {}
      // namespace
      if pc^=ns_separator then
      begin
        inc(pc);
        pc2:=pc;
      end;
//      while pc^ <> ']' do inc(pc);
      {}
      repeat
        len:=CharUTF8Len(pc);
        if (len=1) and (pc^ = ']') then
          break;
        inc(pc,len);
      until false;
      {}
      pc^:=#0; //!!

      AddSection(data,pc1,pc2);
      inc(pc);
    end
    // parameter
    else if pc^ in sIdFirst then
    begin
      pc1:=pc;
      // skip param name
      while pc^ in sLatWord do inc(pc);
      pc^:=#0; //!!
      // skip spaces
      while pc^ in [#9,' '] do inc(pc);
      inc(pc); // must be "="
      // skip spaces
      while pc^ in [#9,' '] do inc(pc);
//      pc2:=pc;
      // parameter can be quoted
      // here need to cut spaces, comments but join next lines
      pc2:=ProcessParamValue(pc);

      AddParam(data,pc1,pc2,false);
    end
    else // wrong thing, skip line
      while not (pc^ in [#10,#13]) do inc(pc);
  end;

end;

function OpenStorageBuf(buf:pAnsiChar):pointer;
begin
  result:=nil;
  if (buf<>nil) and (buf^<>#0) then
  begin
    GetMem(result,SizeOf(tStorage));
    FillChar(result^,SizeOf(tStorage),0);

    StrDup(pStorage(result)^.buffer,buf);

    TranslateData(pStorage(result));
  end;
end;

function OpenFileStorage(h:THANDLE):pointer;
var
  size:integer;
begin
  result:=nil;
  if h<>THANDLE(INVALID_HANDLE_VALUE) then
  begin
    size:=FileSize(h);
    if size>0 then
    begin
      GetMem(result,SizeOf(tStorage));
      FillChar(result^,SizeOf(tStorage),0);

      // save name too?
      GetMem(pStorage(result)^.buffer,size+1);
      BlockRead(h,pStorage(result)^.buffer^,size);
      pStorage(result)^.buffer[size]:=#0;
    end;
    CloseHandle(h);
    TranslateData(pStorage(result));
  end;
end;

function OpenStorage(fname:pWideChar):pointer;
begin
  if FileExists(fname) then
    result:=OpenFileStorage(Reset(fname))
  else
  result:=nil;
end;

function OpenStorage(fname:pAnsiChar):pointer;
begin
  if FileExists(fname) then
    result:=OpenFileStorage(Reset(fname))
  else
  result:=nil;
end;

procedure CloseStorage(storage:pointer);
var
  i:integer;
begin
  if storage=nil then exit;

  with pStorage(storage)^ do
  begin
    if name<>nil then FreeMem(name);
    // cycle by sections
    for i:=0 to HIGH(arSection) do
      SetLength(arSection[i].arParams,0);

    SetLength(arSection,0);
    FreeMem(buffer);
  end;
  FreeMem(storage);
end;
{
function GetNamespaceList(storage:pointer):pAnsiChar;
begin
  if storage=nil then
  begin
    result:=nil;
    exit;
  end;

end;
}
function GetSectionList(storage:pointer;namespace:pAnsiChar=nil):pAnsiChar;
var
  i,lsize,lns:integer;
  pc:pAnsiChar;
begin
  if storage=nil then
  begin
    result:=nil;
    exit;
  end;

  // calculate size
  lsize:=0;
  if (namespace<>nil) and (namespace^<>#0) then
    lns:=HashOf(namespace)
  else
    lns:=0;

  with pStorage(storage)^ do
  begin
    for i:=0 to HIGH(arSection) do
    begin
      with arSection[i] do
      begin
        if (namespace=nil) or
           ((namespace^=#0) and (name=fullname)) then
          inc(lsize,StrLen(fullname)+1)
        else
        begin
          if lns<>ns then
            continue;
          inc(lsize,StrLen(name)+1);
        end;
      end;
{
      if (namespace<>nil) and (namespace^<>#0) then //?? ns=0
      begin
        if ns<>arSection[i].ns then
          continue;
        inc(size,StrLen(arSection[i].name)+1);
      end
      else
        inc(size,StrLen(arSection[i].fullname)+1);
}
    end;
    inc(lsize);
  // get memory
    GetMem(pc,lsize);
    result:=pc;
  // fill
    for i:=0 to HIGH(arSection) do
    begin
      with arSection[i] do
      begin
        if (namespace=nil) or
           ((namespace^=#0) and (name=fullname)) then
          pc:=StrCopyE(pc,fullname)
        else
        begin
          if lns<>ns then
            continue;
          pc:=StrCopyE(pc,name);
        end;
      end;
{     
      if (namespace<>nil) and (namespace^<>#0) then //?? ns=0
      begin
        if ns<>arSection[i].ns then
          continue;
        pc:=StrCopyE(pc,arSection[i].name);
      end
      else
        pc:=StrCopyE(pc,arSection[i].fullname);
}
      inc(pc);
    end;
    pc^:=#0;
  end;
end;

procedure FreeSectionList(ptr:pAnsiChar);
begin
  FreeMem(ptr);
end;

function SearchSection(storage:pointer;section:pAnsiChar;namespace:pAnsiChar=nil):pointer;
var
  i:integer;
  nsn,nss:integer;
begin
  result:=nil;
  if storage=nil then exit;

  nss:=HashOf(section);
  if namespace=nil then
  begin
    with pStorage(storage)^ do
      for i:=0 to numsect-1 do
      begin
        if arSection[i].full=nss then
        begin
          result:=@arSection[i];
          break;
        end;
      end;
  end
  else
  begin
    nsn:=HashOf(namespace);
    with pStorage(storage)^ do
    begin
      for i:=0 to numsect-1 do
      begin
        if (arSection[i].ns=nsn) and (arSection[i].code=nss) then
        begin
          result:=@arSection[i];
          break;
        end;
      end;
    end;
  end;
end;

function GetSectionName(section:pointer):pAnsiChar;
begin
  if section=nil then
    result:=nil
  else
    result:=pSection(section).name;
end;

function SearchParameter(section:pointer;param:pAnsiChar):pointer;
var
  i:integer;
  nsp:integer;
begin
  result:=nil;
  if section<>nil then
  begin
    nsp:=HashOf(param);
    with pSection(section)^ do
    begin
      for i:=0 to numparam-1 do
      begin
        if arParams[i].hash=nsp then
        begin
          result:=@arParams[i];
          break;
        end;
      end;
    end;
  end;
end;

function GetParamSectionStr(section:pointer;param:pAnsiChar;default:pAnsiChar=nil):pAnsiChar;
var
  pn:pParam;
begin
  result:=default;

  if section<>nil then
  begin
    pn:=SearchParameter(section,param);
    if pn<>nil then
      result:=pn.value //StrCopy(buffer,value,len-1);
  end;
end;

function GetParamSectionInt(section:pointer;param:pAnsiChar;default:integer=0):integer;
var
  pn:pParam;
begin
  result:=default;

  if section<>nil then
  begin
    pn:=SearchParameter(section,param);
    if pn<>nil then
    begin
      if pn.value[0]='$' then
        result:=HexToInt(pAnsiChar(@pn.value[1]))
      else
        result:=StrToInt(pn.value);
    end;
  end;
end;


function GetParamStr(storage:pointer;section,param:pAnsiChar;default:pAnsiChar=nil;
         namespace:pAnsiChar=nil):pAnsiChar;
var
  sn:pSection;
begin
  if storage=nil then
  begin
    result:=default;
    exit;
  end;

  sn:=SearchSection(storage,section,namespace);
  result:=GetParamSectionStr(sn,param,default);
end;

function GetParamInt(storage:pointer;section,param:pAnsiChar;default:integer=0;
         namespace:pAnsiChar=nil):integer;
var
  sn:pSection;
begin
  if storage=nil then
  begin
    result:=default;
    exit;
  end;

  sn:=SearchSection(storage,section,namespace);
  result:=GetParamSectionInt(sn,param,default);
end;

end.

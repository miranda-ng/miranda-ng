{Playlist process}
unit playlist;

interface

type
  tPlaylist = class
  private
    fShuffle  :boolean;
    plSize    :cardinal;  // playlist entries
    plCapacity:cardinal;
    base      :PWideChar;
    name      :PWideChar;
    descr     :PWideChar;
    plStrings :array of PWideChar;
    CurElement:cardinal;
    PlOrder   :array of cardinal;
    CurOrder  :cardinal;

    procedure SetShuffle(value:boolean);
    function  GetShuffle:boolean;
    procedure DoShuffle;

    function GetTrackNumber:integer;
    procedure SetTrackNumber(value:integer);

    procedure AddLine(name,descr:PWideChar;new:boolean=true);
    function ProcessElement(num:integer=-1):PWideChar; //virtual;

  public
    constructor Create(fname:PWideChar);
    destructor Free;

    procedure SetBasePath(path:PWideChar);

    function GetSong(number:integer=-1):PWideChar;
    function GetCount:integer;

    function Next    :PWideChar;
    function Previous:PWideChar;

    property Track  :integer read GetTrackNumber write SetTrackNumber;
    property Shuffle:boolean read GetShuffle     write SetShuffle;
  end;

function isPlaylist(fname:PWideChar):integer;
function CreatePlaylist(fname:PWideChar):tPlaylist;
function CreatePlaylistBuf(buf:pointer;format:integer):tPlaylist;

implementation

uses windows, common, io, memini;

const
  plSizeStart = 2048;
  plSizeStep  = 256;
const
  pltM3OLD   = $100;
  pltM3UTF   = $200;

type
  tM3UPlaylist = class(tPlaylist)
  private
  public
    constructor Create(fname:PWideChar);
    constructor CreateBuf(buf:pointer);
  end;

  tPLSPlaylist = class(tPlaylist)
  private
  public
    constructor Create(fname:PWideChar);
    constructor CreateBuf(buf:pointer);
  end;

function isPlaylist(fname:PWideChar):integer;
var
  ext:array [0..7] of WideChar;
begin
  GetExt(fname,ext,7);
  if      StrCmpW(ext,'M3U',3)=0 then result:=1
  else if StrCmpW(ext,'PLS'  )=0 then result:=2
  else result:=0;
end;

function CreatePlaylist(fname:PWideChar):tPlaylist;
begin
  case isPlaylist(fname) of
    1: result:=tM3UPlaylist.Create(fname);
    2: result:=tPLSPlaylist.Create(fname);
  else result:=nil;
  end;
end;

function CreatePlaylistBuf(buf:pointer;format:integer):tPlaylist;
begin
  case format of
    1: result:=tM3UPlaylist.CreateBuf(buf);
    2: result:=tPLSPlaylist.CreateBuf(buf);
  else result:=nil;
  end;
end;

//-----  -----

function SkipLine(var p:PWideChar):bool;
begin
  while p^>=' ' do inc(p);
  while p^<=' ' do // Skip spaces too
  begin
    if p^=#0 then
    begin
      result:=false;
      exit;
    end;
    p^:=#0;
    inc(p);
  end;
  result:=true;
end;

constructor tM3UPlaylist.CreateBuf(buf:pointer);
var
  p:PAnsiChar;
  pp,pd:PWideChar;
  plBufW:PWideChar;
  lname,ldescr:PWideChar;
  finish:boolean;
  pltNew:boolean;
begin
  inherited;

  p:=buf;
  if (pdword(p)^ and $00FFFFFF)=$00BFBBEF then
  begin
    inc(p,3);
    UTF8ToWide(p,plBufW)
  end
  else
    AnsiToWide(p,plBufW);

  pp:=plBufW;
  pltNew:=StrCmpW(pp,'#EXTM3U',7)=0;
  if pltNew then SkipLine(pp);

  ldescr:=nil;
  finish:=false;
  repeat
    if pltNew then
    begin
      pd:=StrScanW(pp,',');
      if pd<>nil then
      begin
        ldescr:=pd+1;
        if not SkipLine(pp) then break;
      end;
    end;
    lname:=pp;
    finish:=SkipLine(pp);
    AddLine(lname,ldescr);
  until not finish;

  mFreeMem(plBufW);
end;

constructor tM3UPlaylist.Create(fname:PWideChar);
var
  f:THANDLE;
  i:integer;
  plBuf:PAnsiChar;
begin
  inherited;

  f:=Reset(fname);

  if f<>THANDLE(INVALID_HANDLE_VALUE) then
  begin
    i:=integer(FileSize(f));
    if i=-1 then
      i:=integer(GetFSize(fname));
    if i<>-1 then
    begin
      mGetMem(plBuf,i+1);
      BlockRead(f,plBuf^,i);
      CloseHandle(f);
      plBuf[i]:=#0;
      CreateBuf(plBuf);
      mFreeMem(plBuf);
    end;
  end;

end;

//-----  -----

constructor tPLSPlaylist.CreateBuf(buf:pointer);
var
  lname,ldescr:PWideChar;
  section,storage,sectionlist:pointer;
  ffile,ftitle:array [0..31] of AnsiChar;
  f,t:PAnsiChar;
  i,size:integer;
begin
  inherited;

  storage:=OpenStorageBuf(buf);
  if storage=nil then
    exit;
  sectionlist:=GetSectionList(storage);
  section:=SearchSection(storage,sectionlist);
  FreeSectionList(sectionlist);

  size:=GetParamSectionInt(section,'NumberOfEntries');
  f:=StrCopyE(ffile ,'File');
  t:=StrCopyE(ftitle,'Title');
  for i:=1 to size do
  begin
    IntToStr(f,i);
    AnsiToWide(GetParamSectionStr(section,ffile),lname);

    IntToStr(t,i);
    AnsiToWide(GetParamSectionStr(section,ftitle),ldescr);

    AddLine(lname,ldescr,false);
  end;

  CloseStorage(storage);
end;

constructor tPLSPlaylist.Create(fname:PWideChar);
var
  buf:PAnsiChar;
  h:THANDLE;
  size:integer;
begin
  if FileExists(fname) then
  begin
    h:=Reset(fname);
    if h<>THANDLE(INVALID_HANDLE_VALUE) then
    begin
      size:=FileSize(h);
      if size>0 then
      begin
        GetMem(buf,size+1);
        BlockRead(h,buf^,size);
        buf[size]:=#0;
        CreateBuf(buf);
        FreeMem(buf);
      end;
      CloseHandle(h);
    end;
  end;
end;

//-----  -----

constructor tPlaylist.Create(fName:PWideChar);
begin
//  inherited;

  CurElement:=0;
  base:=nil;
  name:=nil;
  descr:=nil;
  Shuffle:=false;
  plSize:=0;

  SetBasePath(fname);
end;

destructor tPlaylist.Free;
var
  i:integer;
begin
  PlOrder:=nil;

  mFreeMem(base);
  mFreeMem(name);
  mFreeMem(descr);

  for i:=0 to plSize-1 do
  begin
    mFreeMem(plStrings[i*2]);
    mFreeMem(plStrings[i*2+1]);
  end;
  plStrings:=nil;

  inherited Free;
end;

procedure tPlaylist.AddLine(name,descr:PWideChar;new:boolean=true);
begin
  if plCapacity=0 then
  begin
    plCapacity:=plSizeStart;
    SetLength(plStrings,plSizeStart*2);
    fillChar(plStrings[0],plSizeStart*2*SizeOf(PWideChar),0);
  end
  else if plSize=plCapacity then
  begin
    inc(plCapacity,plSizeStep);
    SetLength(plStrings,plCapacity*2);
    fillChar(plStrings[plSize],plSizeStep*2*SizeOf(PWideChar),0);
  end;
  if new then
  begin
    StrDupW(plStrings[plSize*2  ],name);
    StrDupW(plStrings[plSize*2+1],descr);
  end
  else
  begin
    plStrings[plSize*2  ]:=name;
    plStrings[plSize*2+1]:=descr;
  end;
  inc(plSize);
end;

procedure tPlaylist.SetBasePath(path:PWideChar);
var
  buf:array [0..MAX_PATH-1] of WideChar;
  p,pp:PWideChar;
begin
  mFreeMem(base);

  pp:=ExtractW(path,false);
  p:=StrCopyEW(buf,pp);
  mFreeMem(pp);
  
  if ((p-1)^<>'\') and ((p-1)^<>'/') then
  begin
    if StrScanW(buf,'/')<>nil then
      p^:='/'
    else
      p^:='\';
    inc(p);
  end;
  p^:=#0;
  StrDupW(base,buf);
end;

function tPlaylist.GetCount:integer;
begin
  result:=plSize;
end;

function tPlaylist.GetTrackNumber:integer;
begin
  if fShuffle then
    result:=CurOrder
  else
    result:=CurElement;
end;

procedure tPlaylist.SetTrackNumber(value:integer);
begin
  if value<0 then
    value:=0
  else if value>=integer(plSize) then
    value:=plSize-1;

  if fShuffle then
    CurOrder:=value
  else
    CurElement:=value;
end;

function tPlaylist.ProcessElement(num:integer=-1):PWideChar;
begin
  if num<0 then
    num:=Track
  else if num>=integer(plSize) then
    num:=plSize-1;
  if fShuffle then
    num:=PlOrder[num];

  result:=plStrings[num*2];
end;

function tPlaylist.GetSong(number:integer=-1):PWideChar;
var
  buf:array [0..MAX_PATH-1] of WideChar;
begin
  result:=ProcessElement(number);

  if (result<>nil) and not isPathAbsolute(result) and (base<>nil) then
  begin
    StrCopyW(StrCopyEW(buf,base),result);
    StrDupW(result,buf);
  end
  else
    StrDupW(result,result);
end;

procedure tPlaylist.SetShuffle(value:boolean);
begin
  if value then
  begin
//    if not fShuffle then // need to set Shuffle
      DoShuffle;
  end;

  fShuffle:=value;
end;

function tPlaylist.GetShuffle:boolean;
begin
  result:=fShuffle;
end;

procedure tPlaylist.DoShuffle;
var
  i,RandInx: cardinal;
  SwapItem: cardinal;
begin
  SetLength(PlOrder,plSize);
  Randomize;
  for i:=0 to plSize-1 do
    PlOrder[i]:=i;
  if plSize>1 then
  begin
    for i:=0 to plSize-2 do
    begin
      RandInx:=cardinal(Random(plSize-i));
      SwapItem:=PlOrder[i];
      PlOrder[i      ]:=PlOrder[RandInx];
      PlOrder[RandInx]:=SwapItem;
    end;
  end;
  CurOrder:=0;
end;

function tPlaylist.Next:PWideChar;
begin
  if plSize<>0 then
  begin
    if not Shuffle then
    begin
      inc(CurElement);
      if CurElement=plSize then
        CurElement:=0;
    end
    else // if mode=plShuffle then
    begin
      inc(CurOrder);
      if CurOrder=plSize then
      begin
        DoShuffle;
        CurOrder:=0;
      end;
    end;
    result:=GetSong;
  end
  else
    result:=nil;
end;

function tPlaylist.Previous:PWideChar;
begin
  if plSize<>0 then
  begin
    if not Shuffle then
    begin
      if CurElement=0 then
        CurElement:=plSize;
      Dec(CurElement);
    end
    else // if mode=plShuffle then
    begin
      if CurOrder=0 then
      begin
        DoShuffle;
        CurOrder:=plSize;
      end;
      dec(CurOrder);
    end;
    result:=GetSong;
  end
  else
    result:=nil;
end;

end.

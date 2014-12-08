unit lowlevelc;

interface

uses
  windows,
  iac_global;

// Macro flags
const
  ACF_ASSIGNED = $80000000; // macro assigned
  ACF_FIRSTRUN = $40000000; // FirstRun flag
  ACF_USEDNOW  = $20000000; // macro in use (reserved)
  ACF_VOLATILE = $10000000; // don't save in DB

  ACF_TOSAVE  = ACF_ASSIGNED or ACF_FIRSTRUN;

type
  pActionList = ^tActionList;
  tActionList = array [0..1023] of tBaseAction;

const
  MacroNameLen = 64;
type
  pMacroRecord = ^tMacroRecord;
  tMacroRecord = record
    id         :dword;
    flags      :dword;     // ACF_* flags
    descr      :array [0..MacroNameLen-1] of WideChar;
    ActionList :pActionList;
    ActionCount:integer;
  end;

type // array dimension - just for indexing
  pMacroList  = ^taMacroList;
  taMacroList = array [0..1023] of tMacroRecord;

type
  tMacroList = class
  private
    fMacroList:pMacroList;
    fMacroCount:cardinal;

    procedure ReallocMacroList;
    function GetMacroElement(i:integer):pMacroRecord;
  public

    constructor Create(isize:cardinal);
    destructor Destroy; override;

    procedure Clear(filter:dword=0);
    function Clone:tMacroList;
    function NewMacro:cardinal;

    function GetMacro(id:uint_ptr   ):pMacroRecord; overload;
    function GetMacro(name:pWideChar):pMacroRecord; overload;
    function GetMacroNameById(id:dword):PWideChar;

    property List[i:integer]:pMacroRecord read GetMacroElement; default;
    property Count: cardinal read fMacroCount;
  end;

procedure FreeMacro(Macro:pMacroRecord;mask:dword=0);

var
  MacroList:tMacroList;


implementation

uses Common;

const
  MacroListPage = 8;


function tMacroList.GetMacroElement(i:integer):pMacroRecord;
begin
  result:=@fMacroList[i];
end;

function tMacroList.GetMacro(id:uint_ptr):pMacroRecord;
var
  i:integer;
begin
  for i:=0 to fMacroCount-1 do
  begin
    if ((fMacroList^[i].flags and ACF_ASSIGNED)<>0) and
       (id=fMacroList^[i].id) then
    begin
      result:=@(fMacroList^[i]);
      exit;
    end;
  end;
  result:=nil;
end;

function tMacroList.GetMacro(name:pWideChar):pMacroRecord;
var
  i:integer;
begin
  for i:=0 to fMacroCount-1 do
  begin
    if ((fMacroList^[i].flags and ACF_ASSIGNED)<>0) and
       (StrCmpW(name,fMacroList^[i].descr)=0) then
    begin
      result:=@(fMacroList^[i]);
      exit;
    end;
  end;
  result:=nil;
end;

function tMacroList.GetMacroNameById(id:dword):PWideChar;
var
  p:pMacroRecord;
begin
  p:=GetMacro(id);
  if p<>nil then
    result:=@(p^.descr)
  else
    result:=nil;
end;

procedure FreeActionList(var src:pActionList; count:integer; mask:dword);
var
  i:integer;
begin
  for i:=0 to count-1 do
  begin
    if (mask=0) or ((src^[i].flags and mask)<>0) then
      src^[i].Free;
  end;
  FreeMem(src);
  src:=nil;
end;

procedure FreeMacro(Macro:pMacroRecord;mask:dword=0);
begin
  with Macro^ do
  begin
    if (flags and ACF_ASSIGNED)<>0 then
    begin
      flags:=0; // make Unassigned
      FreeActionList(ActionList,ActionCount,mask);
      ActionCount:=0;
    end;
  end;
end;

procedure tMacroList.Clear(filter:dword=0);
var
  i:integer;
begin
  for i:=0 to fMacroCount-1 do
  begin
    FreeMacro(@(fMacroList[i]),filter);
  end;
  fMacroCount:=0;
  FreeMem(fMacroList);
  fMacroList:=nil;
end;

destructor tMacroList.Destroy;
begin
  fMacroCount:=0;
  FreeMem(fMacroList);
  fMacroList:=nil;

  inherited Destroy;
end;

function CloneActionList(src:pActionList;count:integer):pActionList;
begin
  if src=nil then
  begin
    result:=nil;
    exit;
  end;
  GetMem(result    ,count*SizeOf(tBaseAction));
  move(src^,result^,count*SizeOf(tBaseAction))
end;

procedure CloneMacro(var dst:pMacroRecord; src:pMacroRecord);
begin
  if (src^.flags and ACF_ASSIGNED)<>0 then
  begin
    move(src^,dst^,SizeOf(tMacroRecord));
    dst^.ActionList:=CloneActionList(src^.ActionList,src^.ActionCount);
  end;
end;

function tMacroList.Clone:tMacroList;
var
  src,dst:pMacroRecord;
  i:integer;
  cnt:integer;
begin
  result:=nil;

  if fMacroList<>nil then
  begin
    cnt:=0;
    for i:=0 to fMacroCount-1 do
      if (fMacroList^[i].flags and ACF_ASSIGNED)<>0 then
        inc(cnt);
    if cnt>0 then
    begin
      result:=tMacroList.Create(cnt);

      src:=pMacroRecord(self.fMacroList);
      dst:=pMacroRecord(result.fMacroList);

      while cnt>0 do
      begin
        if (src^.flags and ACF_ASSIGNED)<>0 then
        begin
          CloneMacro(dst,src);
          inc(dst);
          dec(cnt);
        end;
        inc(src);
      end;
    end;
  end;
  if result=nil then
    result:=tMacroList.Create(0);
end;

procedure tMacroList.ReallocMacroList;
var
  i:cardinal;
  tmp:pMacroList;
begin
  i:=(fMacroCount+MacroListPage)*SizeOf(tMacroRecord);
  GetMem(tmp,i);
  FillChar(tmp^,i,0);
  if fMacroCount>0 then
  begin
    move(fMacroList^,tmp^,fMacroCount*SizeOf(tMacroRecord));
    FreeMem(fMacroList);
  end;
  fMacroList:=tmp;
  inc(fMacroCount,MacroListPage);
end;

constructor tMacroList.Create(isize:cardinal);
begin
  inherited Create;

  if isize<MacroListPage then
    fMacroCount:=MacroListPage
  else
    fMacroCount:=isize;
  GetMem  (fMacroList ,fMacroCount*SizeOf(tMacroRecord));
  FillChar(fMacroList^,fMacroCount*SizeOf(tMacroRecord),0);
end;

procedure InitMacroValue(pMacro:pMacroRecord);
var
  tmp:int64;
begin
  with pMacro^ do
  begin
    StrCopyW(descr,NoDescription,MacroNameLen-1);
    QueryPerformanceCounter(tmp);
    id         :=tmp and $FFFFFFFF;
    flags      :=ACF_ASSIGNED;
  end;
end;

function tMacroList.NewMacro:cardinal;
var
  i:cardinal;
  pMacro:pMacroRecord;
begin
  i:=0;
  pMacro:=pMacroRecord(fMacroList);
  while i<fMacroCount do
  begin
    if (pMacro^.flags and ACF_ASSIGNED)=0 then
    begin
      result:=i;
      InitMacroValue(pMacro);
      exit;
    end;
    inc(i);
    inc(pMacro);
  end;
  // realloc
  result:=fMacroCount;
  ReallocMacroList;
  InitMacroValue(@(fMacroList^[result]));
end;

end.

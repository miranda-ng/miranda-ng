unit iac_global;

interface

uses
  windows, messages,
  m_api;


const
  rtUnkn = 0;
  rtInt  = 1;
  rtWide = 2;
  rtAnsi = 3;
  rtUTF8 = 4;

var
  xmlparser:TXML_API_W;

const
  IcoLibPrefix = 'action_type_';
const
  NoDescription:PWideChar='No description';
const
  protostr = '<proto>';
const
  WM_ACT_SETVALUE   = WM_USER + 13;
  WM_ACT_RESET      = WM_USER + 14;
  WM_ACT_SAVE       = WM_USER + 15;
  WM_ACT_LISTCHANGE = WM_USER + 16; // group, action

// Action flags
const
  ACF_DISABLED   = $10000000; // action disabled
  // options editor only
  ACF_REPLACED   = $20000000; // action replaced by new in options
  ACF_INTRODUCED = $40000000; // action is newly created (not saved) in options
const
  ACF_MASK       = $00FFFFFF; // mask for private action flags

type
  tLRType = record
    value:uint_ptr;
    rtype:byte; // rt* const
  end;
type
  pWorkData = ^tWorkData;
  tWorkData = record
    Parameter  :LPARAM;
    ActionList :pointer;
    LastResult :uint_ptr;
    ActionCount:integer;
    ResultType :integer;   // rt* const
    Storage    :array [0..9] of tLRType;
  end;

type
  pBaseAction = ^tBaseAction;
  tBaseAction = class
    ActionDescr:pWideChar; // description (user name)
    UID        :dword;     // hash of action type name
    flags      :dword;

    procedure Duplicate(var dst:tBaseAction);

    constructor Create(uid:dword);
    destructor Destroy; override;
//    function  Clone:tBaseAction; virtual;
    function  DoAction(var WorkData:tWorkData):LRESULT; virtual; // process action
    procedure Load(node:pointer;fmt:integer); virtual;           // load/import action
    procedure Save(node:pointer;fmt:integer); virtual;           // save/export action
  end;

type
  tCreateActionFunc = function:tBaseAction;
  tCreateDialogFunc = function(parent:HWND):HWND;
//  tCheckImportFunc  = function(node:pointer;fmt:integer):boolean;

type
  pActModule = ^tActModule;
  tActModule = record
    Next     :pActModule;
    Name     :pAnsiChar;         // action type name
    Dialog   :tCreateDialogFunc; // action dialog creating
    Create   :tCreateActionFunc; // action object creation
//    CheckImp :tCheckImportFunc;  // check for action type
    Icon     :pAnsiChar;         // icon resource name
    // runtime data
    DlgHandle:HWND;
    Hash     :dword;             // will be calculated at registration cycle
  end;

const
  ModuleLink:pActModule=nil;

function ClearResult    (var   WorkData:tWorkData;num:integer=-1):uint_ptr;
function GetResultNumber(const WorkData:tWorkData;num:integer=-1):uint_ptr;

procedure InsertString(wnd:HWND;num:dword;str:PAnsiChar);

function GetLinkName  (hash:dword    ):PAnsiChar;
function GetLink      (hash:dword    ):pActModule;
function GetLinkByName(name:pAnsiChar):pActModule;

function ImportContact   (node:HXML   ):TMCONTACT;
function ImportContactINI(node:pointer):TMCONTACT;


implementation

uses Common, global, dbsettings, inouttext,
  mirutils, mircontacts;


//----- tBaseAction code -----
const
  ioDisabled = 'disabled';
  ioName     = 'name';
const
  opt_uid   = 'uid';
  opt_descr = 'descr';
  opt_flags = 'flags';

constructor tBaseAction.Create(uid:dword);
begin
  inherited Create;

  if uid<>0 then
  begin
    StrDupW(ActionDescr,NoDescription);
    Self.UID:=uid;
    flags:=0;
  end;
end;

destructor tBaseAction.Destroy;
begin
  mFreeMem(ActionDescr);

  inherited Destroy;
end;

procedure tBaseAction.Duplicate(var dst:tBaseAction);
begin
  StrDupW(dst.ActionDescr,ActionDescr);
  dst.UID  :=UID;
  dst.flags:=flags;
end;
{
function tBaseAction.Clone:tBaseAction;
begin
  //dummy
  result:=nil;
end;
}
function tBaseAction.DoAction(var WorkData:tWorkData):LRESULT;
begin
  result:=0;
  // nothing
end;

procedure tBaseAction.Load(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  case fmt of
    100..199, // for V2
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      mFreeMem(ActionDescr); // created by constructor
      StrCopy(pc,opt_descr); ActionDescr:=DBReadUnicode(0,DBBranch,section,NoDescription);
      StrCopy(pc,opt_flags); flags      :=DBReadDword  (0,DBBranch,section);
      // UID reading in main program, set by constructor
    end;

    1: begin
      with xmlparser do
      begin
        if StrToInt(getAttrValue(HXML(node),ioDisabled))=1 then
          flags:=flags or ACF_DISABLED;

        StrDupW(ActionDescr,getAttrValue(HXML(node),ioName));
      end;
    end;
{
    2: begin
      if GetParamSectionInt(node,ioDisabled))=1 then
        flags:=flags or ACF_DISABLED;

      UF8ToWide(GetParamSectionStr(node,ioName),ActionDescr);
    end;
}
  end;
end;

procedure tBaseAction.Save(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:PAnsiChar;
begin
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_uid  ); DBWriteDWord  (0,DBBranch,section,UID);
      StrCopy(pc,opt_flags); DBWriteDWord  (0,DBBranch,section,flags);
      StrCopy(pc,opt_descr); DBWriteUnicode(0,DBBranch,section,ActionDescr);
    end;
{
    1: begin
    end;
}
    13: begin
      tTextExport(node).AddText ('type'    ,GetLinkName(UID));
      tTextExport(node).AddTextW('name'    ,ActionDescr);
      tTextExport(node).AddFlag ('disabled',(flags and ACF_DISABLED)<>0);
      tTextExport(node).AddNewLine();
      tTextExport(node).ShiftRight();
    end;
  end;
end;

//----- LastResult processing -----

function ClearResult(var WorkData:tWorkData;num:integer=-1):uint_ptr;
var
  rt:pbyte;
  lr:^uint_ptr;
begin
  result:=0;

  if num<0 then
  begin
    rt:=@WorkData.ResultType;
    lr:=@WorkData.LastResult;
  end
  else if num<10 then
  begin
    rt:=@WorkData.Storage[num].rtype;
    lr:=@WorkData.Storage[num].value;
  end
  else
    exit;

  if rt^=rtInt then
    result:=lr^
  else if rt^<>rtUnkn then
  begin
    mFreeMem(pWideChar(lr^));
    result:=0;
  end;
end;

function GetResultNumber(const WorkData:tWorkData;num:integer=-1):uint_ptr;
var
  rt:pbyte;
  lr:^uint_ptr;
begin
  result:=0;

  if num<0 then
  begin
    rt:=@WorkData.ResultType;
    lr:=@WorkData.LastResult;
  end
  else if num<10 then
  begin
    rt:=@WorkData.Storage[num].rtype;
    lr:=@WorkData.Storage[num].value;
  end
  else
    exit;

  if rt^=rtInt then
    result:=lr^
  else if rt^<>rtUnkn then
  begin
    result:=NumToInt(pWideChar(lr^));
  end;
end;

procedure InsertString(wnd:HWND;num:dword;str:PAnsiChar);
var
  buf:array [0..127] of WideChar;
begin
  SendMessageW(wnd,CB_SETITEMDATA,
      SendMessageW(wnd,CB_ADDSTRING,0,
          lparam(TranslateW(FastAnsiToWideBuf(str,buf)))),
      num);
{
  SendMessageW(wnd,CB_INSERTSTRING,num,
      dword(TranslateW(FastAnsiToWideBuf(str,buf))));
}
end;

function GetLinkName(hash:dword):PAnsiChar;
var
  link:pActModule;
begin
  link:=ModuleLink;
  while (link<>nil) and (link.Hash<>hash) do
    link:=link^.Next;
  if link<>nil then
    result:=link^.Name
  else
    result:=nil;
end;

function GetLink(hash:dword):pActModule;
begin
  result:=ModuleLink;
  while (result<>nil) and (result.Hash<>hash) do
    result:=result^.Next;
end;

function GetLinkByName(name:pAnsiChar):pActModule;
begin
  result:=ModuleLink;
  while (result<>nil) and (StrCmp(result.Name,name)<>0) do
    result:=result^.Next;
end;

const
  ioCProto   = 'cproto';
  ioIsChat   = 'ischat';
  ioCUID     = 'cuid';
  ioCUIDType = 'cuidtype';

function ImportContact(node:HXML):TMCONTACT;
var
  proto:pAnsiChar;
  tmpbuf:array [0..63] of AnsiChar;
  dbv:TDBVARIANT;
  tmp:pWideChar;
  is_chat:boolean;
  bufLen:int; 
begin
  with xmlparser do
  begin
    proto:=FastWideToAnsiBuf(getAttrValue(node,ioCProto),tmpbuf);
    if (proto=nil) or (proto^=#0) then
    begin
      result:=0;
      exit;
    end;
    is_chat:=StrToInt(getAttrValue(node,ioIsChat))<>0;

    tmp:=getAttrValue(node,ioCUID);
    if is_chat then
    begin
      dbv.szVal.W:=tmp;
    end
    else
    begin
      FillChar(dbv,SizeOf(TDBVARIANT),0);
      dbv._type:=StrToInt(getAttrValue(node,ioCUIDType));
      case dbv._type of
        DBVT_BYTE  : dbv.bVal:=StrToInt(tmp);
        DBVT_WORD  : dbv.wVal:=StrToInt(tmp);
        DBVT_DWORD : dbv.dVal:=StrToInt(tmp);
        DBVT_ASCIIZ: FastWideToAnsi(tmp,dbv.szVal.A);
        DBVT_UTF8  : WideToUTF8(tmp,dbv.szVal.A);
        DBVT_WCHAR : dbv.szVal.W:=tmp;
        DBVT_BLOB  : begin
          dbv.pbVal := mir_base64_decode(FastWideToAnsi(tmp,pAnsiChar(dbv.pbVal)),bufLen);
          dbv.cpbVal := bufLen;
        end;
      end;
    end;
  end;
  result:=FindContactHandle(proto,dbv,is_chat);
  if not is_chat then
    case dbv._type of
      DBVT_ASCIIZ,
      DBVT_UTF8  : mFreeMem(dbv.szVal.A);
      DBVT_BLOB  : mFreeMem(dbv.pbVal);
    end;
end;

function ImportContactINI(node:pointer):TMCONTACT;
{
var
  proto:pAnsiChar;
  dbv:TDBVARIANT;
  tmp:pAnsiChar;
  is_chat:boolean;
}
begin
  result:=0;
{
  proto:=GetParamSectionStr(node,ioCProto); // LATIN chars must be
  if (proto=nil) or (proto^=#0) then
  begin
    result:=0;
    exit;
  end;
  is_chat:=GetParamSectionInt(node,ioIsChat)<>0;

  tmp:=GetParamSectionStr(node,ioCUID);
  if is_chat then
  begin
    dbv.szVal.W:=UTF8ToWide(tmp);
  end
  else
  begin
    FillChar(dbv,SizeOf(TDBVARIANT),0);
    dbv._type:=GetParamSectionInt(node,ioCUIDType);
    case dbv._type of
      DBVT_BYTE  : dbv.bVal:=StrToInt(tmp);
      DBVT_WORD  : dbv.wVal:=StrToInt(tmp);
      DBVT_DWORD : dbv.dVal:=StrToInt(tmp);
      DBVT_ASCIIZ: dbv.szVal.A:=tmp; // must be LATIN
      DBVT_UTF8  : dbv.szVal.A:=tmp;
      DBVT_WCHAR : UTF8ToWide(tmp);
      DBVT_BLOB  : begin // must be LATIN (base64)
        Base64Decode(tmp,dbv.pbVal);
      end;
    end;
  end;

  result:=FindContactHandle(proto,dbv,is_chat);

  if is_chat or (dbv._type=DBVT_WCHAR) then
    mFreeMem(dbv.szVal.W)
  else if dbv._type=DBVT_BLOB then
    mFreeMem(dbv.pbVal);
}
end;
{
function CreateImportClass(node:pointer;fmt:integer):tBaseAction;
var
  module:pActModule;
  uid:dword;
  section:array [0..127] of AnsiChar;
begin
  result:=nil;
  module:=ModuleLink;
  case fmt of
    0: begin
      StrCopy(StrCopyE(section,pAnsiChar(node)),opt_uid);
      uid:=DBReadDWord(0,DBBranch,section,0);

      while module<>nil do
      begin
        module:=module^.Next;
      end;
    end;
    1: begin
    end;
    2: begin
    end;
  end;
end;
}

//----- DLL Handle Cache -----
type
  tDLLCacheElement = record
    DllName  :PAnsiChar;
    DllHandle:THANDLE;
    count    :word; // count for end-of-macro flag
    flags    :byte; // handle free mode
  end;
  tDLLCache = array of tDLLCacheElement;

const
  actDLLCache: tDLLCache = nil;

function GetDllHandle(adllname:pAnsiChar;mode:dword=0):THANDLE;
var
  i,zero:integer;
begin
  // 1 - search that name in cache
  i:=0;
  zero:=-1;
  while i<=HIGH(actDLLCache) do
  begin
    with actDLLCache[i] do
    begin
      // remember first empty slot
      if DllHandle=0 then
      begin
        if zero<0 then
          zero:=i;
      end
      else if StrCmp(DllName,adllname)=0 then
      begin
        result:=DllHandle;
        inc(count);
        if mode=3 then // per-session
          flags:=3;
        exit;
      end;
    end;
    inc(i);
  end;
  // 2 - not found, load library
  result:=LoadLibraryA(adllname);
  // 3 - add to cache if not per-action
  if mode<>0 then
  begin
    if zero>=0 then
      i:=zero
    else
    begin
      SetLength(actDLLCache,i);
      dec(i);
    end;

    with actDLLCache[i] do
    begin
      StrDup(DllName,adllname);
      DllHandle:=result;
      count    :=0;
      flags    :=mode;
    end;
  end;
end;

procedure CloseDllHandle(handle:THANDLE);
var
  i:integer;
begin
  i:=HIGH(actDLLCache);
  while i>=0 do
  begin
    with actDLLCache[i] do
    begin
      if DllHandle=handle then
      begin
        dec(count);
        if count=0 then
        begin
          if flags=2 then // per-macro+not needed -> free
          begin
            FreeLibrary(DllHandle);
            DllHandle:=0;
            mFreeMem(DllName);
          end;
        end;
        exit;
      end;
    end;
    dec(i);
  end;
  // if not found in cache
  FreeLibrary(handle);
end;

procedure FreeDllHandleCache;
var
  i:integer;
begin
  i:=HIGH(actDLLCache);
  while i>=0 do
  begin
    if actDLLCache[i].DllHandle<>0 then
    begin
      FreeLibrary(actDLLCache[i].DllHandle);
      mFreeMem(actDLLCache[i].DllName);
    end;
    dec(i);
  end;
  SetLength(actDLLCache,0);
end;

end.

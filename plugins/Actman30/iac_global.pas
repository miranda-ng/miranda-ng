unit iac_global;

interface

uses
  windows, messages,
  m_api;

var
  xmlparser:XML_API_W;

const
  IcoLibPrefix = 'action_type_';
const
  NoDescription:PWideChar='No Description';
const
  protostr = '<proto>';
const
  WM_ACT_SETVALUE   = WM_USER + 13;
  WM_ACT_RESET      = WM_USER + 14;
  WM_ACT_SAVE       = WM_USER + 15;
  WM_ACT_LISTCHANGE = WM_USER + 16; // group, action

const
  ACF_DISABLED   = $10000000;  // action disabled
  ACF_REPLACED   = $20000000;  // action replaced by new in options
  ACF_INTRODUCED = $40000000;  // action is newly created (not saved) in options

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

function ClearResult(var WorkData:tWorkData;num:integer=-1):uint_ptr;
function GetResultNumber(var WorkData:tWorkData;num:integer=-1):uint_ptr;

procedure InsertString(wnd:HWND;num:dword;str:PAnsiChar);

function GetLink(hash:dword):pActModule;
function GetLinkByName(name:pAnsiChar):pActModule;

function ImportContact   (node:HXML   ):THANDLE;
function ImportContactINI(node:pointer):THANDLE;

implementation

uses Common, global, dbsettings, base64, mirutils;

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
  pc:pAnsiChar;
begin
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_uid  ); DBWriteDWord  (0,DBBranch,section,uid);
      StrCopy(pc,opt_flags); DBWriteDWord  (0,DBBranch,section,flags);
      StrCopy(pc,opt_descr); DBWriteUnicode(0,DBBranch,section,ActionDescr);
    end;
{
    1: begin
    end;
}
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

function GetResultNumber(var WorkData:tWorkData;num:integer=-1):uint_ptr;
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
{
    if (pWideChar(WorkData.LastResult)[0]='$') and
       (AnsiChar(pWideChar(WorkData.LastResult)[1]) in sHexNum) then
      result:=HexToInt(pWideChar(WorkData.LastResult)+1)
    else
    if (pWideChar(WorkData.LastResult)[0]='0') and
       (pWideChar(WorkData.LastResult)[1]='x') and
       (AnsiChar(pWideChar(WorkData.LastResult)[2]) in sHexNum) then
      result:=HexToInt(pWideChar(WorkData.LastResult)+2)
    else
      result:=StrToInt(pWideChar(WorkData.LastResult));
}
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

function ImportContact(node:HXML):THANDLE;
var
  proto:pAnsiChar;
  tmpbuf:array [0..63] of AnsiChar;
  dbv:TDBVARIANT;
  tmp:pWideChar;
  is_chat:boolean;
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
          Base64Decode(FastWideToAnsi(tmp,pAnsiChar(dbv.pbVal)),dbv.pbVal);
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

function ImportContactINI(node:pointer):THANDLE;
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
end.

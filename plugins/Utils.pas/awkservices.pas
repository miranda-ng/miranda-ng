unit awkservices;

interface

uses windows;

//===== Basic parameter and result type flags =====

const
  ACF_TYPE_NUMBER  = $00; // Parameter is number
  ACF_TYPE_STRING  = $01; // Parameter is ANSI String
  ACF_TYPE_UNICODE = $02; // Parameter is Unicode string
  ACF_TYPE_STRUCT  = $03; // Parameter is (result is in) structure
//----- Additional parameter types -----
  ACF_TYPE_PARAM   = $08; // Parameter is Call parameter
  ACF_TYPE_CURRENT = $09; // Parameter is ignored, used current user handle from current message window
  ACF_TYPE_RESULT  = $0A; // Parameter is previous action result

  ACF_TYPE_MASK    = $0F; // parameter/result type mask


//===== Flags =====

const
  ACF_FLAG_SCRIPT = $00000200; // Parameter or service name is script

//----- Parameter flags -----
const
  ACF_FLAG_TEMPLATE = $00000100; // Parameter is text from template (with possible comments)

//----- Service result flags -----
const
  ACF_FLAG_FREEMEM = $00000100; // Need to free memory
  // visual view (controlled by ACF_NOVISUAL flag in dialogs)
  ACF_FLAG_HEXNUM  = $00001000; // Show number as hex
  ACF_FLAG_SIGNED  = $00002000; // Show number as signed


//===== Block creation flags =====

const
  // Common block flags
  ACF_BLOCK_NOSTATIC  = $01000000; // No label text in block
  ACF_BLOCK_NOBORDER  = $02000000; // No group border around block
  ACF_BLOCK_NOSTRUCT  = $04000000; // don't add structure as param type
  // Param only
  ACF_BLOCK_NOCURRENT = $08000000; // don't show "Current contact" type in list (slow processing)
  // Result only
  ACF_BLOCK_NOVISUAL  = $08000000; // don't show number view styles
  // Service only
  ACF_BLOCK_EXPAND    = $10000000; // all service subblocks are visible


//===== Types =====

// can be placed to srvblock module if no function exports here
type
  pServiceValue = ^tServiceValue;
  tServiceValue = record
    service:PAnsiChar;
    wparam,
    lparam: pointer;
    w_flags,
    l_flags,
    flags:dword; // result etc
  end;

type
  pSubstData = ^tSubstData;
  tSubstData = record
    Parameter :LPARAM;
    LastResult:uint_ptr;
    ResultType:integer;   // ACF_TYPE_* const (ACF_TYPE_NUMBER or ACF_TYPE_UNICODE right now)
  end;


procedure ClearSubstData(var data:tSubstData);

{}
//----- Parameter dialog -----

function CreateParamBlock(parent:HWND;x,y,width:integer;flags:dword=0):THANDLE;
function ClearParamFields(Dialog:HWND):HWND;
function FillParam       (Dialog:HWND;txt:PAnsiChar):integer;
function SetParamValue   (Dialog:HWND;    flags:dword;    value:pointer):boolean;
function GetParamValue   (Dialog:HWND;var flags:dword;var value:pointer):boolean;
function SetParamLabel   (Dialog:HWND; lbl:PWideChar):HWND;

//----- Parameter value -----

procedure SaveParamValue(    flags:dword;     param:pointer; module,setting:PAnsiChar);
procedure LoadParamValue(var flags:dword; var param:pointer; module,setting:PAnsiChar);
procedure ClearParam    (    flags:dword; var param);
function  DuplicateParam(    flags:dword; var asparam,adparam):dword;
{??
function  TranslateParam(param:uint_ptr;flags:dword;hContact:TMCONTACT):uint_ptr;
}

//----- Execution -----

function  PrepareParameter(flags:dword;const aparam:LPARAM; const data:tSubstData):LPARAM;
procedure ReleaseParameter(flags:dword;var   aparam:LPARAM);

//----- result dialog -----

function CreateResultBlock(parent:HWND;x,y,width:integer;flags:dword=0):THANDLE;
function ClearResultFields(Dialog:HWND):HWND;
function SetResultValue   (Dialog:HWND;flags:dword):integer;
function GetResultValue   (Dialog:HWND):dword;

//----- Old flags converting -----

function ConvertParamFlags (flags:dword):dword;
function ConvertResultFlags(flags:dword):dword;


//----- Service dialog -----

function  CreateServiceBlock(parent:HWND;x,y,width,height:integer;flags:dword=0):HWND;
procedure ClearServiceBlock (Dialog:HWND);
procedure SetServiceListMode(Dialog:HWND;mode:integer);

function SetSrvBlockValue(Dialog:HWND;const value:tServiceValue):boolean;
function GetSrvBlockValue(Dialog:HWND;var   value:tServiceValue):boolean;
// service setting for templates
procedure SetSrvBlockService(Dialog:HWND; service:PAnsiChar);
function  GetSrvBlockService(Dialog:HWND):PAnsiChar;

//----- ServiceValue functions -----

procedure CopyServiceValue (var   dst :tServiceValue; const src:tServiceValue);
procedure ClearServiceValue(var   data:tServiceValue);
procedure SaveServiceValue (const data:tServiceValue; module,setting:PAnsiChar);
procedure LoadServiceValue (var   data:tServiceValue; module,setting:PAnsiChar);

//----- Service execute -----

function ExecuteService(const service:tServiceValue; var data:tSubstData):boolean;
{}

implementation

uses
  common
  ,sparam
  ,srvblock;

procedure ClearSubstData(var data:tSubstData);
begin
  case data.ResultType of
    ACF_TYPE_STRING,  // this condition must be never existing rigth now
    ACF_TYPE_UNICODE: mFreeMem(data.LastResult);
  end;
end;

//==================================================

//----- Parameter dialog -----

function CreateParamBlock(parent:HWND;x,y,width:integer;flags:dword=0):THANDLE;
begin
  result:=sparam.CreateParamBlock(parent,x,y,width,flags);
end;

function ClearParamFields(Dialog:HWND):HWND;
begin
  result:=sparam.ClearParamFields(Dialog);
end;

function FillParam(Dialog:HWND;txt:PAnsiChar):integer;
begin
  result:=sparam.FillParam(Dialog,txt);
end;

function SetParamValue(Dialog:HWND; flags:dword; value:pointer):boolean;
begin
  result:=sparam.SetParamValue(Dialog,flags,value);
end;

function GetParamValue(Dialog:HWND;var flags:dword;var value:pointer):boolean;
begin
  result:=sparam.GetParamValue(Dialog,flags,value);
end;

function SetParamLabel(Dialog:HWND; lbl:PWideChar):HWND;
begin
  result:=sparam.SetParamLabel(Dialog,lbl);
end;

//----- Parameter value -----

procedure SaveParamValue(flags:dword; param:pointer; module,setting:PAnsiChar);
begin
  sparam.SaveParamValue(flags,param,module,setting);
end;

procedure LoadParamValue(var flags:dword; var param:pointer; module,setting:PAnsiChar);
begin
  sparam.LoadParamValue(flags,param,module,setting);
end;

procedure ClearParam(flags:dword; var param);
begin
  sparam.ClearParam(flags,param);
end;

function DuplicateParam(flags:dword; var asparam,adparam):dword;
begin
  result:=sparam.DuplicateParam(flags,asparam,adparam);
end;

//----- Execution -----

function PrepareParameter(flags:dword;const aparam:LPARAM; const data:tSubstData):LPARAM;
begin
  result:=sparam.PrepareParameter(flags,aparam,data);
end;

procedure ReleaseParameter(flags:dword;var aparam:LPARAM);
begin
  sparam.ReleaseParameter(flags,aparam);
end;

//----- result dialog -----

function CreateResultBlock(parent:HWND;x,y,width:integer;flags:dword=0):THANDLE;
begin
  result:=sparam.CreateResultBlock(parent,x,y,width,flags);
end;

function ClearResultFields(Dialog:HWND):HWND;
begin
  result:=sparam.ClearResultFields(Dialog);
end;

function SetResultValue(Dialog:HWND;flags:dword):integer;
begin
  result:=sparam.SetResultValue(Dialog,flags);
end;

function GetResultValue(Dialog:HWND):dword;
begin
  result:=sparam.GetResultValue(Dialog);
end;

//----- Old flags converting -----

function ConvertParamFlags(flags:dword):dword;
begin
  result:=sparam.ConvertParamFlags(flags);
end;

function ConvertResultFlags(flags:dword):dword;
begin
  result:=sparam.ConvertResultFlags(flags);
end;


//----- Service dialog -----

function CreateServiceBlock(parent:HWND;x,y,width,height:integer;flags:dword=0):HWND;
begin
  result:=srvblock.CreateServiceBlock(parent,x,y,width,height,flags)
end;

procedure ClearServiceBlock(Dialog:HWND);
begin
  srvblock.ClearServiceBlock(Dialog)
end;

procedure SetServiceListMode(Dialog:HWND;mode:integer);
begin
  srvblock.SetServiceListMode(Dialog,mode);
end;

function SetSrvBlockValue(Dialog:HWND;const value:tServiceValue):boolean;
begin
  result:=srvblock.SetSrvBlockValue(Dialog,value);
end;

function GetSrvBlockValue(Dialog:HWND;var value:tServiceValue):boolean;
begin
  result:=srvblock.GetSrvBlockValue(Dialog,value);
end;

// service setting for templates
procedure SetSrvBlockService(Dialog:HWND; service:PAnsiChar);
begin
  srvblock.SetSrvBlockService(Dialog,service);
end;

function GetSrvBlockService(Dialog:HWND):PAnsiChar;
begin
  result:=srvblock.GetSrvBlockService(Dialog);
end;

//----- ServiceValue functions -----

procedure CopyServiceValue(var dst:tServiceValue; const src:tServiceValue);
begin
  srvblock.CopyServiceValue(dst,src);
end;

procedure ClearServiceValue(var data:tServiceValue);
begin
  srvblock.ClearServiceValue(data);
end;

procedure SaveServiceValue(const data:tServiceValue; module,setting:PAnsiChar);
begin
  srvblock.SaveServiceValue(data,module,setting);
end;

procedure LoadServiceValue(var data:tServiceValue; module,setting:PAnsiChar);
begin
  srvblock.LoadServiceValue(data,module,setting);
end;

//----- Service execute -----

function ExecuteService(const service:tServiceValue; var data:tSubstData):boolean;
begin
  result:=srvblock.ExecuteService(service,data);
end;


end.

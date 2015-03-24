{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}

{$IFNDEF M_CORE}
{$DEFINE M_CORE}


const
  CoreDLL = 'mir_core.dll';

///////////////////////////////////////////////////////////////////////////////
// command line support

// Parse ptszCmdLine. it must NOT be a constant (content will be patched)
procedure CmdLine_Parse(ptszCmdLine:PWideChar); stdcall;
                 external CoreDLL name 'CmdLine_Parse';

function CmdLine_GetOption(ptszParameter:PWideChar):PWideChar; stdcall;
                 external CoreDLL name 'CmdLine_GetOption';

///////////////////////////////////////////////////////////////////////////////
// database functions

const
  DBVT_ASIS            = 0;   // in, for db_get_s, to not translate output value
  DBVT_DELETED         = 0;   // setting got deleted, no values are valid
  DBVT_BYTE            = 1;   // bVal, cVal are valid
  DBVT_WORD            = 2;   // wVal, sVal are valid
  DBVT_DWORD           = 4;   // dVal, lVal are valid
  DBVT_ASCIIZ          = 255; // pszVal is valid
  DBVT_BLOB            = 254; // cpbVal and pbVal are valid
  DBVT_UTF8            = 253; // pszVal is valid
  DBVT_WCHAR           = 252; // pszVal is valid
  DBVTF_VARIABLELENGTH = $80; // ?

type
  PMCONTACT = ^TMCONTACT;
  TMCONTACT  = dword;
const
  INVALID_CONTACT_ID = TMCONTACT(-1);

type
  TMEVENT = dword;

///////////////////////////////////////////////////////////////////////////////
// events, hooks & services

const
  MAXMODULELABELLENGTH = 64;

type
  TMIRANDAHOOK            = function(wParam: WPARAM; lParam: LPARAM): int; cdecl;
  TMIRANDAHOOKPARAM       = function(wParam: WPARAM; lParam,lParam1: LPARAM): int; cdecl;
  TMIRANDAHOOKOBJ         = function(ptr:pointer;wParam:WPARAM;lParam:LPARAM): int; cdecl;
  TMIRANDAHOOKOBJPARAM    = function(ptr:pointer;wParam:WPARAM;lParam,lParam1: LPARAM): int; cdecl;

  TMIRANDASERVICE         = function(wParam: WPARAM; lParam: LPARAM): int_ptr; cdecl;
  TMIRANDASERVICEPARAM    = function(wParam:WPARAM;lParam,lParam1:LPARAM):int_ptr; cdecl;
  TMIRANDASERVICEOBJ      = function(ptr:pointer;wParam:WPARAM;lParam:LPARAM):int_ptr; cdecl;
  TMIRANDASERVICEOBJPARAM = function(ptr:pointer;wParam:WPARAM;lParam,lParam1:LPARAM):int_ptr; cdecl;

const
  {$IFDEF WIN64}
  CALLSERVICE_NOTFOUND = int_ptr($8000000000000000);
  {$ELSE}
  CALLSERVICE_NOTFOUND = int_ptr($80000000);
  {$ENDIF}

function CreateHookableEvent(const name: PAnsiChar): THANDLE; stdcall;
                 external CoreDLL name 'CreateHookableEvent';
function DestroyHookableEvent(hEvent:THANDLE): int; stdcall;
                 external CoreDLL name 'DestroyHookableEvent';
function SetHookDefaultForHookableEvent(hEvent:THANDLE; pfnHook:TMIRANDAHOOK):int; stdcall;
                 external CoreDLL name 'SetHookDefaultForHookableEvent';
function CallPluginEventHook(hInst:HINST; hEvent:THANDLE; wParam:WPARAM; lParam:LPARAM):int; stdcall;
                 external CoreDLL name 'CallPluginEventHook';
function NotifyEventHooks(hEvent:THANDLE; wParam:WPARAM; lParam:LPARAM): int; stdcall;
                 external CoreDLL name 'NotifyEventHooks';
function NotifyFastHook(hEvent:THANDLE; wParam:WPARAM; lParam:LPARAM): int; stdcall;
                 external CoreDLL name 'NotifyFastHook';

function HookEvent(const name:PAnsiChar; hookProc: TMIRANDAHOOK): THANDLE; stdcall;
                 external CoreDLL name 'HookEvent';
function HookEventParam(const name:PAnsiChar; hookProc:TMIRANDAHOOKPARAM;lParam:LPARAM):THANDLE; stdcall;
                 external CoreDLL name 'HookEventParam';
function HookEventObj(const name:PAnsiChar; hookProc:TMIRANDAHOOKOBJ;ptr:pointer):THANDLE; stdcall;
                 external CoreDLL name 'HookEventObj';
function HookEventObjParam(const name:PAnsiChar; hookProc:TMIRANDAHOOKOBJPARAM;ptr:pointer;lParam:LPARAM):THANDLE; stdcall;
                 external CoreDLL name 'HookEventObjParam';
function HookEventMessage(const name:PAnsiChar; Wnd:HWND; wMsg:uint):THANDLE; stdcall;
                 external CoreDLL name 'HookEventMessage';
function UnhookEvent(hHook:THANDLE): int; stdcall;
                 external CoreDLL name 'UnhookEvent';
procedure KillObjectEventHooks(var ptr); stdcall;
                 external CoreDLL name 'KillObjectEventHooks';
procedure KillModuleEventHooks(pModule:HINST); stdcall;
                 external CoreDLL name 'KillModuleEventHooks';

function CreateServiceFunction(const name:PAnsiChar; serviceProc:TMIRANDASERVICE): THANDLE; stdcall;
                 external CoreDLL name 'CreateServiceFunction';
function CreateServiceFunctionParam(const name:PAnsiChar; serviceProc:TMIRANDASERVICEPARAM;lParam:LPARAM):THANDLE; stdcall;
                 external CoreDLL name 'CreateServiceFunctionParam';
function CreateServiceFunctionObj(const name:PAnsiChar; serviceProc:TMIRANDASERVICEOBJ;ptr:pointer):THANDLE; stdcall;
                 external CoreDLL name 'CreateServiceFunctionObj';
function CreateServiceFunctionObjParam(const name:PAnsiChar; serviceProc:TMIRANDASERVICEOBJPARAM;ptr:pointer;lParam:LPARAM):THANDLE; stdcall;
                 external CoreDLL name 'CreateServiceFunctionObjParam';
function DestroyServiceFunction(hService:THANDLE):int; stdcall;
                 external CoreDLL name 'DestroyServiceFunction';
function ServiceExists(const name:PAnsiChar):int; stdcall;
                 external CoreDLL name 'ServiceExists';

function CallService(const name:PAnsiChar; wParam:WPARAM; lParam:LPARAM):int_ptr; stdcall;
                 external CoreDLL name 'CallService';
function CallServiceSync(const name:PAnsiChar; wParam:WPARAM; lParam:LPARAM):int_ptr; stdcall;
                 external CoreDLL name 'CallServiceSync';

function CallFunctionAsync(ptr1,ptr2:pointer):int; stdcall;
                 external CoreDLL name 'CallFunctionAsync';
procedure KillModuleServices(hInst:HINST); stdcall;
                 external CoreDLL name 'KillModuleServices';
procedure KillObjectServices(var ptr); stdcall;
                 external CoreDLL name 'KillObjectServices';

function CallContactService(hContact:TMCONTACT;const name:PAnsiChar;wParam:WPARAM;lParam:LPARAM):int_ptr; cdecl;
                 external CoreDLL name 'CallContactService';
function CallProtoService(const szModule:PAnsiChar;const szService:PAnsiChar;wParam:WPARAM;lParam:LPARAM):int_ptr; cdecl;
                 external CoreDLL name 'CallProtoService';


///////////////////////////////////////////////////////////////////////////////
// http

// returned result must be freed using mir_free()
function mir_urlEncode(url:PAnsiChar): PAnsiChar; stdcall;
                 external CoreDLL name 'mir_urlEncode';

///////////////////////////////////////////////////////////////////////////////
// exceptions

type
  PEXCEPTION_POINTERS = ^EXCEPTION_POINTERS;
  pfnExceptionFilter = function(code:dword; info:PEXCEPTION_POINTERS):dword; cdecl;

function GetExceptionFilter():pfnExceptionFilter; stdcall;
                 external CoreDLL name 'GetExceptionFilter';
function SetExceptionFilter(pMirandaExceptFilter:pfnExceptionFilter):pfnExceptionFilter; stdcall;
                 external CoreDLL name 'SetExceptionFilter';


///////////////////////////////////////////////////////////////////////////////
// icons support

type
  pIconItem = ^tIconItem;
  tIconItem = record
    szDescr:PAnsiChar;
    szName :PAnsiChar;
    defIconID,
    size   :int;
    hIcolib:THANDLE;
  end;
  tagIconItem = tIconItem;

  pIconItemW = ^tIconItemW;
  tIconItemW = record
    szDescr:PWideChar;
    szName :PAnsiChar;
    defIconID,
    size   :int;
    hIcolib:THANDLE;
  end;
  tagIconItemW = tIconItemW;

procedure mir_Icon_Register(hInst:HINST; const szSection:PAnsiChar; pIcons:pIconItem;
    iCount:size_t; prefix:PAnsiChar; hLangpack:int=0); stdcall;
                 external CoreDLL name 'Icon_Register';
procedure mir_Icon_RegisterW(hInst:HINST; const szSection:PWideChar; pIcons:pIconItemW;
    iCount:size_t; prefix:PAnsiChar; hLangpack:int=0); stdcall;
                 external CoreDLL name 'Icon_RegisterT';

///////////////////////////////////////////////////////////////////////////////
// language packs support

const
  LANG_UNICODE = $1000;

procedure Langpack_SortDuplicates(); stdcall;
                 external CoreDLL name 'Langpack_SortDuplicates';
function Langpack_GetDefaultCodePage():int; stdcall;
                 external CoreDLL name 'Langpack_GetDefaultCodePage';
function Langpack_GetDefaultLocale():int; stdcall;
                 external CoreDLL name 'Langpack_GetDefaultLocale';
function Langpack_PcharToTchar(const pszStr:PAnsiChar):PWideChar; stdcall;
                 external CoreDLL name 'Langpack_PcharToTchar';

function LoadLangPackModule():int; stdcall;
                 external CoreDLL name 'LoadLangPackModule';
function LoadLangPack(const szLangPack:PWideChar):int; stdcall;
                 external CoreDLL name 'LoadLangPack';
procedure ReloadLangpack(pszStr:PWideChar); stdcall;
                 external CoreDLL name 'ReloadLangpack';

function TranslateA_LP(const str:PAnsiChar; hLang:int):PAnsiChar; stdcall;
                 external CoreDLL name 'TranslateA_LP';
function TranslateW_LP(const str:PWideChar; hLang:int):PWideChar; stdcall;
                 external CoreDLL name 'TranslateW_LP';
procedure TranslateMenu_LP(hMenu:HMENU; hLang:int); stdcall;
                 external CoreDLL name 'TranslateMenu_LP';
procedure TranslateDialog_LP(hDlg:HWND; hLang:int); stdcall;
                 external CoreDLL name 'TranslateDialog_LP';

function mir_hash(const key:pointer; len:uint):uint; stdcall;
                 external CoreDLL name 'mir_hash';


///////////////////////////////////////////////////////////////////////////////
// lists

type
  TFSortFunc = function (para1:pointer; para2:pointer):int;cdecl;
{
// Assumes first 32 bit value of the data is the numeric key
// and uses it to perform sort/search operations, this results
// in much better performance as no compare function calls needed
// Incredibly useful for Hash Tables
}
const
  NumericKeySort = {TFSortFunc}(pointer(-1));
  HandleKeySort  = {TFSortFunc}(pointer(-2));
  PtrKeySort     = {TFSortFunc}(pointer(-3));

type
  PSortedList = ^TSortedList;
  TSortedList = record
    items    : array of pointer;
    realCount: int;
    limit    : int;
    increment: int;
    sortFunc : TFSortFunc;
  end;

// BUGGED with limit <> 0. list pointer must befreed by mir_free
function List_Create(limit:int; increment:int):PSortedList;stdcall;
                 external CoreDLL name 'List_Create';

procedure List_Destroy(list:PSortedList);stdcall;
                 external CoreDLL name 'List_Destroy';
function List_Find(list:PSortedList; value:pointer):pointer;stdcall;
                 external CoreDLL name 'List_Find';
function List_GetIndex(list:PSortedList; value:pointer; index:Pint):int;stdcall;
                 external CoreDLL name 'List_GetIndex';
function List_IndexOf(list:PSortedList; value:pointer):int;stdcall;
                 external CoreDLL name 'List_IndexOf';
function List_Insert(list:PSortedList; value:pointer; index:int):int;stdcall;
                 external CoreDLL name 'List_Insert';
function List_InsertPtr(list:PSortedList; p:pointer):int;stdcall;
                 external CoreDLL name 'List_InsertPtr';
function List_Remove(list:PSortedList; index:int):int;stdcall;
                 external CoreDLL name 'List_Remove';
function List_RemovePtr(list:PSortedList; p:pointer):int;stdcall;
                 external CoreDLL name 'List_RemovePtr';
procedure List_Copy(src:PSortedList; dst:PSortedList; size:size_t); stdcall;
                 external CoreDLL name 'List_Copy';
procedure List_ObjCopy(src:PSortedList; dst:PSortedList; size:size_t); stdcall;
                 external CoreDLL name 'List_ObjCopy';


///////////////////////////////////////////////////////////////////////////////
// log functions

function mir_createLog(pszName:PAnsiChar; ptszDescr, ptszFile:PWideChar; options:Cardinal):THANDLE; stdcall;
                 external CoreDLL name 'mir_createLog';
procedure mir_closeLog(hLogger:THANDLE); stdcall;
                 external CoreDLL name 'mir_closeLog';

function mir_writeLogA(hLogger:THANDLE; format:PAnsiChar):int; cdecl;
                 external CoreDLL name 'mir_writeLogA';
function mir_writeLogW(hLogger:THANDLE; format:PWideChar):int; cdecl;
                 external CoreDLL name 'mir_writeLogW';

///////////////////////////////////////////////////////////////////////////////
// md5 functions

// Define the state of the MD5 Algorithm.
type
  pmir_md5_state_t = ^mir_md5_state_t;
  mir_md5_state_t = record
    count:array [0.. 1] of longword; // message length in bits, lsw first
    abcd :array [0.. 3] of longword; // digest buffer
    buf  :array [0..63] of byte;     // accumulate block
  end;

  TMD5Hash = array [0..15] of byte;

procedure mir_md5_init(pms:pmir_md5_state_t); stdcall;
                       external CoreDLL name 'mir_md5_init';
procedure mir_md5_append(pms:pmir_md5_state_t; const data:pbyte; nbytes:int); stdcall;
                         external CoreDLL name 'mir_md5_append';
procedure mir_md5_finish(pms:pmir_md5_state_t; digest:TMD5Hash); stdcall;
                         external CoreDLL name 'mir_md5_finish';
procedure mir_md5_hash(const data:pbyte; len:int; digest:TMD5Hash); stdcall;
                       external CoreDLL name 'mir_md5_hash';


///////////////////////////////////////////////////////////////////////////////
// memory functions

function mir_alloc(para1:size_t):pointer; cdecl;
                 external CoreDLL name 'mir_alloc';
function mir_calloc(para1:size_t):pointer; cdecl;
                 external CoreDLL name 'mir_calloc';
function mir_realloc(para1:pointer; para2:size_t):pointer; cdecl;
                 external CoreDLL name 'mir_realloc';
procedure mir_free(para1:pointer); cdecl;
                 external CoreDLL name 'mir_free';

function mir_strdup(const src:PAnsiChar):PAnsiChar; stdcall;
                 external CoreDLL name 'mir_strdup';
function mir_wstrdup(const src:PWideChar):PWideChar; stdcall;
                 external CoreDLL name 'mir_wstrdup';
function mir_strndup(const src:PAnsiChar; len:size_t):PAnsiChar; stdcall;
                 external CoreDLL name 'mir_strndup';
function mir_wstrndup(const src:PWideChar; len:size_t):PWideChar; stdcall;
                 external CoreDLL name 'mir_wstrndup';

function mir_strlen(const src:PAnsiChar):size_t; stdcall;
                 external CoreDLL name 'mir_strlen';
function mir_wstrlen(const src:PWideChar):size_t; stdcall;
                 external CoreDLL name 'mir_wstrlen';

function mir_strcpy(dst:PAnsiChar; const src:PAnsiChar):PAnsiChar; stdcall;
                 external CoreDLL name 'mir_strcpy';
function mir_wstrcpy(dst:PWideChar; const src:PWideChar):PWideChar; stdcall;
                 external CoreDLL name 'mir_wstrcpy';
function mir_strncpy(dst:PAnsiChar; const src:PAnsiChar; len:size_t):PAnsiChar; stdcall;
                 external CoreDLL name 'mir_strncpy';
function mir_wstrncpy(dst:PWideChar; const src:PWideChar; len:size_t):PWideChar; stdcall;
                 external CoreDLL name 'mir_wstrncpy';

function mir_strcat(dst:PAnsiChar; const src:PAnsiChar):PAnsiChar; stdcall;
                 external CoreDLL name 'mir_strcat';
function mir_wstrcat(dst:PWideChar; const src:PWideChar):PWideChar; stdcall;
                 external CoreDLL name 'mir_wstrcat';
function mir_strncat(dst:PAnsiChar; const src:PAnsiChar; len:size_t):PAnsiChar; stdcall;
                 external CoreDLL name 'mir_strncat';
function mir_wstrncat(dst:PWideChar; const src:PWideChar; len:size_t):PWideChar; stdcall;
                 external CoreDLL name 'mir_wstrncat';

function mir_strcmp(const p1,p2:PAnsiChar):int; stdcall;
                 external CoreDLL name 'mir_strcmp';
function mir_wstrcmp(const p1,p2:PWideChar):int; stdcall;
                 external CoreDLL name 'mir_wstrcmp';
function mir_strcmpi(const p1,p2:PAnsiChar):int; stdcall;
                 external CoreDLL name 'mir_strcmpi';
function mir_wstrcmpi(const p1,p2:PWideChar):int; stdcall;
                 external CoreDLL name 'mir_wstrcmpi';

///////////////////////////////////////////////////////////////////////////////
// modules

procedure RegisterModule(hInst:HINST); stdcall;
                 external CoreDLL name 'RegisterModule';
procedure UnregisterModule(hInst:HINST); stdcall;
                 external CoreDLL name 'UnregisterModule';

function GetInstByAddress(codePtr:pointer):HINST; stdcall;
                 external CoreDLL name 'GetInstByAddress';


///////////////////////////////////////////////////////////////////////////////
// path utils

procedure CreatePathToFile(wszFilePath:PAnsiChar); stdcall;
                 external CoreDLL name 'CreatePathToFile';
function CreateDirectoryTree(const szDir:PAnsiChar):int; stdcall;
                 external CoreDLL name 'CreateDirectoryTree';
function PathIsAbsolute(const pSrc:PAnsiChar):int; stdcall;
                 external CoreDLL name 'PathIsAbsolute';
function PathToAbsolute(const pSrc:PAnsiChar; pOut:PAnsiChar; base:PAnsiChar=nil):int; stdcall;
                 external CoreDLL name 'PathToAbsolute';
function PathToRelative(const pSrc:PAnsiChar; pOut:PAnsiChar; base:PAnsiChar=nil):int; stdcall;
                 external CoreDLL name 'PathToRelative';

procedure CreatePathToFileW(wszFilePath:PWideChar); stdcall;
                 external CoreDLL name 'CreatePathToFileW';
function CreateDirectoryTreeW(const szDir:PWideChar):int; stdcall;
                 external CoreDLL name 'CreateDirectoryTreeW';
function PathIsAbsoluteW(const pSrc:PWideChar):int; stdcall;
                 external CoreDLL name 'PathIsAbsoluteW';
function PathToAbsoluteW(const pSrc:PWideChar; pOut:PWideChar; base:PWideChar=nil):int; stdcall;
                 external CoreDLL name 'PathToAbsoluteW';
function PathToRelativeW(const pSrc:PWideChar; pOut:PWideChar; base:PWideChar=nil):int; stdcall;
                 external CoreDLL name 'PathToRelativeW';


///////////////////////////////////////////////////////////////////////////////
// print functions

function mir_snprintf(buffer:PAnsiChar;count:size_t;fmt:PAnsiChar{, ...}):int; stdcall;
                 external CoreDLL name 'mir_snprintf';
function mir_snwprintf(buffer:PWideChar;count:size_t;fmt:PWideChar{, ...}):int; stdcall;
                 external CoreDLL name 'mir_snwprintf';
function mir_vsnprintf(buffer:PAnsiChar;count:size_t;fmt:PAnsiChar;va:va_list):int; stdcall;
                 external CoreDLL name 'mir_vsnprintf';
function mir_vsnwprintf(buffer:PWideChar;count:size_t;fmt:PWideChar;va:va_list):int; stdcall;
                 external CoreDLL name 'mir_vsnwprintf';


///////////////////////////////////////////////////////////////////////////////
// protocol functions

function ProtoBroadcastAck(const szModule:PAnsiChar; hContact:TMCONTACT; type_:int; result_:int; hProcess:THANDLE; lParam:LPARAM): int_ptr; stdcall;
                 external CoreDLL name 'ProtoBroadcastAck';

function ProtoServiceExists(const szModule, szName:PAnsiChar):int; stdcall;
                 external CoreDLL name 'ProtoServiceExists';

function ProtoCallService(const szModule, szName:PAnsiChar; wParam:WPARAM; lParam:LPARAM):int_ptr; stdcall;
                 external CoreDLL name 'ProtoServiceExists';

procedure ProtoWindowAdd(pThis:pointer; wnd:HWND); stdcall;
                 external CoreDLL name 'ProtoWindowAdd';

procedure ProtoWindowRemove(pThis:pointer; wnd:HWND); stdcall;
                 external CoreDLL name 'ProtoWindowRemove';

// Call it in the very beginning of your proto's constructor
procedure ProtoConstructor(pThis:pointer{PPROTO_INTERFACE}; const pszModuleName:PAnsiChar;
                           ptszUserName:PWideChar); stdcall;
                 external CoreDLL name 'ProtoConstructor';

// Call it in the very end of your proto's destructor
procedure ProtoDestructor(pThis:pointer{PPROTO_INTERFACE}); stdcall;
                 external CoreDLL name 'ProtoDestructor';

{!!
typedef void (__cdecl PROTO_INTERFACE::*ProtoThreadFunc)(void*);
procedure   ProtoForkThread(PROTO_INTERFACE *pThis, ProtoThreadFunc, void *param);
function ProtoForkThreadEx(PROTO_INTERFACE *pThis, ProtoThreadFunc, void *param, uint* threadID):THANDLE;

typedef int (__cdecl PROTO_INTERFACE::*ProtoEventFunc)(WPARAM, LPARAM);
procedure   ProtoHookEvent(PROTO_INTERFACE *pThis, const char* szName, ProtoEventFunc pFunc);
function ProtoCreateHookableEvent(PROTO_INTERFACE *pThis, const char* szService):THANDLE;

typedef INT_PTR (__cdecl PROTO_INTERFACE::*ProtoServiceFunc)(WPARAM, LPARAM);
procedure ProtoCreateService(PROTO_INTERFACE *pThis, const char* szService, ProtoServiceFunc);

typedef INT_PTR (__cdecl PROTO_INTERFACE::*ProtoServiceFuncParam)(WPARAM, LPARAM, LPARAM);
procedure ProtoCreateServiceParam(PROTO_INTERFACE *pThis, const char* szService, ProtoServiceFuncParam, LPARAM);
}

procedure ProtoLogA(pThis:pointer{PPROTO_INTERFACE}; szFormat :PAnsiChar; args:va_list); stdcall;
                 external CoreDLL name 'ProtoLogA';
procedure ProtoLogW(pThis:pointer{PPROTO_INTERFACE}; wszFormat:PWideChar; args:va_list); stdcall;
                 external CoreDLL name 'ProtoLogW';

// returns image extension by a PA_* constant or empty string for PA_FORMAT_UNKNOWN
function ProtoGetAvatarExtension(format:int):PWideChar; stdcall;
                 external CoreDLL name 'ProtoGetAvatarExtension';

// detects image format by extension
function ProtoGetAvatarFormat(const szFileName:PWideChar):int; stdcall;
                 external CoreDLL name 'ProtoGetAvatarFormat';

// detects image format by its contents
function ProtoGetAvatarFileFormat(const szFileName:PWideChar):int; stdcall;
                 external CoreDLL name 'ProtoGetAvatarFileFormat';

// returns the image format and extension by the first bytes of picture
// ptszExtension might be NULL
function ProtoGetBufferFormat(const buf:PByte; var ext:PWideChar):int; stdcall;
                 external CoreDLL name 'ProtoGetBufferFormat';


///////////////////////////////////////////////////////////////////////////////
// sha1 functions

const
  MIR_SHA1_HASH_SIZE = 20;

type
  pmir_sha1_ctx = ^mir_sha1_ctx;
  mir_sha1_ctx = record
    H:array [0.. 4] of longword;
    W:array [0..79] of longword;
    lenW:int;
    sizeHi,sizeLo:longword;
  end;

  SHA1Hash = array [0..MIR_SHA1_HASH_SIZE-1] of byte;

procedure mir_sha1_init(ctx:pmir_sha1_ctx); stdcall;
                 external CoreDLL name 'mir_sha1_init';
procedure mir_sha1_append(ctx:pmir_sha1_ctx; dataIn:pbyte; len:int); stdcall;
                 external CoreDLL name 'mir_sha1_append';
procedure mir_sha1_finish(ctx:pmir_sha1_ctx; hashout:SHA1Hash); stdcall;
                 external CoreDLL name 'mir_sha1_finish';
procedure mir_sha1_hash(dataIn:pbyte; len:int;hashout:SHA1Hash); stdcall;
                 external CoreDLL name 'mir_sha1_hash';

procedure mir_hmac_sha1(hashout:SHA1Hash; const key:pbyte; keyLen:size_t;
                        const dataIn:pbyte; dataLen:size_t); stdcall;
                 external CoreDLL name 'mir_hmac_sha1';


///////////////////////////////////////////////////////////////////////////////
// strings

function mir_base64_decode(str:PAnsiChar; var resultSize:int):PByte; stdcall;
               external CoreDLL name 'mir_base64_decode';
function mir_base64_encode(str:PByte; dataSize:int):PAnsiChar; stdcall;
               external CoreDLL name 'mir_base64_encode';
function mir_base64_encodebuf(data:PByte; dataSize:int; output:PAnsiChar; outputLen:int):PAnsiChar; stdcall;
               external CoreDLL name 'mir_base64_encodebuf';

function rtrim(str:PAnsiChar):PAnsiChar; stdcall;
               external CoreDLL name 'rtrim';
function rtrimw(str:PWideChar):PWideChar; stdcall;
                external CoreDLL name 'rtrimw';

// returns pointer to the beginning of string
function ltrim(str:PAnsiChar):PAnsiChar; stdcall;
               external CoreDLL name 'ltrim';
function ltrimw(str:PWideChar):PWideChar; stdcall;
               external CoreDLL name 'ltrimw';

// returns pointer to the trimmed portion of string
function ltrimp(str:PAnsiChar):PAnsiChar; stdcall;
                external CoreDLL name 'ltrimp';
function ltrimpw(str:PWideChar):PWideChar; stdcall;
                external CoreDLL name 'ltrimpw';

function wildcmp(name:PAnsiChar; mask:PAnsiChar):int; stdcall;
                 external CoreDLL name 'wildcmp';
function wildcmpw(name:PWideChar; mask:PWideChar):int; stdcall;
                 external CoreDLL name 'wildcmpw';

function wildcmpi(name:PAnsiChar; mask:PAnsiChar):int; stdcall;
                 external CoreDLL name 'wildcmpi';
function wildcmpiw(name:PWideChar; mask:PWideChar):int; stdcall;
                 external CoreDLL name 'wildcmpiw';

// mir_free dest, mir_strdup src to dest
function replaceStr(var dest:PAnsiChar; const src:PAnsiChar):PAnsiChar; stdcall;
                 external CoreDLL name 'replaceStr';
function replaceStrW(var dest:PWideChar; const src:PWideChar):PWideChar; stdcall;
                 external CoreDLL name 'replaceStrW';

function bin2hex(data:pointer; dataLen:size_t; pDest:PAnsiChar):PAnsiChar; stdcall;
                 external CoreDLL name 'bin2hex';
function bin2hexW(data:pointer; dataLen:size_t; pDest:PWideChar):PWideChar; stdcall;
                 external CoreDLL name 'bin2hexW';


///////////////////////////////////////////////////////////////////////////////
// text conversion functions

function mir_a2u_cp(src:PAnsiChar;codepage:int):PWideChar; stdcall;
                 external CoreDLL name 'mir_a2u_cp';
function mir_a2u(src:PAnsiChar):PWideChar; stdcall;
                 external CoreDLL name 'mir_a2u';
function mir_u2a_cp(src:PWideChar;codepage:int):PAnsiChar; stdcall;
                 external CoreDLL name 'mir_u2a_cp';
function mir_u2a(src:PWideChar):PAnsiChar; stdcall;
                 external CoreDLL name 'mir_u2a';


///////////////////////////////////////////////////////////////////////////////
// threads
type
  pThreadFunc      = procedure(ptr:pointer); cdecl;
  pThreadFuncEx    = function (ptr:pointer):uint_ptr; stdcall;
  pThreadFuncOwner = function (owner:pointer;param:pointer):uint_ptr; cdecl;

function Thread_Push(hInst:HINST; Owner:pointer):int_ptr; stdcall;
                 external CoreDLL name 'Thread_Push';
function Thread_Pop():int_ptr; stdcall;
                 external CoreDLL name 'Thread_Pop';
procedure Thread_Wait(); stdcall;
                 external CoreDLL name 'Thread_Wait';

function forkthread(func:pThreadFunc; stacksize:uint; arg:pointer):uint_ptr; stdcall;
                 external CoreDLL name 'forkthread';
function forkthreadex(sec:pointer; stacksize:uint; func:pThreadFuncEx; owner:pointer; arg:pointer; thraddr:puint_ptr):uint_ptr; stdcall;
                 external CoreDLL name 'forkthreadex';

procedure Thread_SetName(const szThreadName:PAnsiChar); stdcall;
                 external CoreDLL name 'Thread_SetName';

procedure KillObjectThreads(pObject:pointer); stdcall;
                 external CoreDLL name 'KillObjectThreads';


///////////////////////////////////////////////////////////////////////////////
// utf8 interface
{ commented to avoid newer Delphi version conflicts
function Utf8Decode(str:PAnsiChar; var ucs2:PWideChar):PAnsiChar;stdcall;
                 external CoreDLL name 'Utf8Decode';
function Utf8DecodeCP(str:PAnsiChar; codepage:int; var ucs2:PWideChar):PAnsiChar;stdcall;
                 external CoreDLL name 'Utf8DecodeCP';

function Utf8DecodeW(const str:PAnsiChar):PWideChar;stdcall;
                 external CoreDLL name 'Utf8DecodeW';

function Utf8Encode(const src:PAnsiChar):PAnsiChar;stdcall;
                 external CoreDLL name 'Utf8Encode';
function Utf8EncodeCP(const src:PAnsiChar;codepage:int):PAnsiChar;stdcall;
                 external CoreDLL name 'Utf8EncodeCP';

function Utf8EncodeW(const src:PWideChar):PAnsiChar;stdcall;
                 external CoreDLL name 'Utf8EncodeW';

function Ucs2toUtf8Len(const src:PWideChar):int; stdcall;
                 external CoreDLL name 'Ucs2toUtf8Len';

function Utf8CheckString(const astr:PAnsiChar):bool;stdcall;
                 external CoreDLL name 'Utf8CheckString';
}
// aliases
function mir_utf8decode(str:PAnsiChar; var ucs2:PWideChar):PAnsiChar;stdcall;
                 external CoreDLL name 'Utf8Decode';
function mir_utf8decodecp(str:PAnsiChar; codepage:int; var ucs2:PWideChar):PAnsiChar;stdcall;
                 external CoreDLL name 'Utf8DecodeCP';
function mir_utf8decodew(const str:PAnsiChar):PWideChar;stdcall;
                 external CoreDLL name 'Utf8DecodeW';

function mir_utf8encode(const src:PAnsiChar):PAnsiChar;stdcall;
                 external CoreDLL name 'Utf8Encode';
function mir_utf8encodecp(const src:PAnsiChar;codepage:int):PAnsiChar;stdcall;
                 external CoreDLL name 'Utf8EncodeCP';

function mir_utf8encodew(const src:PWideChar):PAnsiChar;stdcall;
                 external CoreDLL name 'Utf8EncodeW';

function mir_utf8lenW(const src:PWideChar):int; stdcall;
                 external CoreDLL name 'Ucs2toUtf8Len';

function mir_utf8checkstring(const astr:PAnsiChar):bool;stdcall;
                 external CoreDLL name 'Utf8CheckString';

///////////////////////////////////////////////////////////////////////////////
// Window subclassing

procedure mir_subclassWindow(Wnd:HWND; WndProc:TWNDPROC); stdcall;
                 external CoreDLL name 'mir_subclassWindow';
procedure mir_subclassWindowFull(Wnd:HWND; WndProc, OldWndProc:TWNDPROC); stdcall;
                 external CoreDLL name 'mir_subclassWindowFull';
function  mir_callNextSubclass(Wnd:HWND; WndProc:TWNDPROC; uMsg:uint; wParam:WPARAM; lParam:LPARAM ):LRESULT; stdcall;
                 external CoreDLL name 'mir_callNextSubclass';
procedure mir_unsubclassWindow(Wnd:HWND; WndProc:TWNDPROC); stdcall;
                 external CoreDLL name 'mir_unsubclassWindow';

procedure KillModuleSubclassing(hInst: HMODULE);
                       external CoreDLL name 'KillModuleSubclassing';

///////////////////////////////////////////////////////////////////////////////
// Windows utilities

function IsWinVerVistaPlus():bool; stdcall;
                 external CoreDLL name 'IsWinVerVistaPlus';
function IsWinVer7Plus():bool; stdcall;
                 external CoreDLL name 'IsWinVer7Plus';
function IsFullScreen():int; stdcall;
                 external CoreDLL name 'IsFullScreen';
function IsWorkstationLocked():int; stdcall;
                 external CoreDLL name 'IsWorkstationLocked';
function IsScreenSaverRunning():int; stdcall;
                 external CoreDLL name 'IsScreenSaverRunning';

procedure UnloadCoreModule(); stdcall;
          external CoreDLL name 'UnloadCoreModule';

{$ENDIF}

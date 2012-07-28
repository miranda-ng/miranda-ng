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
  DBVT_DELETED         = 0;   // setting got deleted, no values are valid
  DBVT_BYTE            = 1;   // bVal, cVal are valid
  DBVT_WORD            = 2;   // wVal, sVal are valid
  DBVT_DWORD           = 4;   // dVal, lVal are valid
  DBVT_ASCIIZ          = 255; // pszVal is valid
  DBVT_BLOB            = 254; // cpbVal and pbVal are valid
  DBVT_UTF8            = 253; // pszVal is valid
  DBVT_WCHAR           = 252; // pszVal is valid
  DBVTF_VARIABLELENGTH = $80; // ?
  DBVTF_DENYUNICODE    = $10000;

type
  HDBEVENT = Integer;
  PDBVARIANT = ^TDBVARIANT;
  TDBVARIANT = record
    _type: Byte;
    case LongInt of
    0: (bVal: Byte);
    1: (cVal: AnsiChar);
    2: (wVal: Word);
    3: (sVal: ShortInt);
    4: (dVal: dword);
    5: (lVal: long);
    6: (
      szVal : TChar;
      cchVal: Word;
    );
    7: (
      cpbVal: Word;
      pbVal : PByte;
    );
  end;


function db_free(dbv:PDBVARIANT):int_ptr; stdcall;
                 external CoreDLL name 'db_free';
function db_unset(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar):int_ptr; stdcall;
                  external CoreDLL name 'db_unset';

function db_get_b(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; errorValue:int):int; stdcall;
                  external CoreDLL name 'db_get_b';
function db_get_w(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; errorValue:int):int; stdcall;
                  external CoreDLL name 'db_get_w';
function db_get_dw(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; errorValue:dword):dword; stdcall;
                   external CoreDLL name 'db_get_dw';
function db_get(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; dbv:PDBVARIANT):int_ptr; stdcall;
                external CoreDLL name 'db_get';
function db_get_s(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; dbv:PDBVARIANT; const nType:int):int_ptr; stdcall;
                  external CoreDLL name 'db_get_s';
function db_get_sa(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar):pAnsiChar; stdcall;
                   external CoreDLL name 'db_get_sa';
function db_get_wsa(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar):pWideChar; stdcall;
                    external CoreDLL name 'db_get_wsa';

function db_set_b(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; val:byte):int_ptr; stdcall;
                  external CoreDLL name 'db_set_b';
function db_set_w(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; val:word):int_ptr; stdcall;
                  external CoreDLL name 'db_set_w';
function db_set_dw(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; val:dword):int_ptr; stdcall;
                   external CoreDLL name 'db_set_dw';
function db_set_s(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; const val:pAnsiChar):int_ptr; stdcall;
                  external CoreDLL name 'db_set_s';
function db_set_ws(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; const val:pWideChar):int_ptr; stdcall;
                   external CoreDLL name 'db_set_ws';
function db_set_utf(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; const val:pAnsiChar):int_ptr; stdcall;
                    external CoreDLL name 'db_set_utf';
function db_set_blob(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; val:pointer; len:uint):int_ptr; stdcall;
                     external CoreDLL name 'db_set_blob';

// Aliases
function DBFreeVariant(dbv:PDBVARIANT):int_ptr; stdcall;
                 external CoreDLL name 'db_free';
function DBDeleteContactSetting(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar):int_ptr; stdcall;
                  external CoreDLL name 'db_unset';

function DBGetContactSettingByte(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; errorValue:int):int; stdcall;
                                 external CoreDLL name 'db_get_b';
function DBGetContactSettingWord(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; errorValue:int):int; stdcall;
                                 external CoreDLL name 'db_get_w';
function DBGetContactSettingDWord(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; errorValue:dword):dword; stdcall;
                                  external CoreDLL name 'db_get_dw';
function DBGetContactSetting(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; dbv:PDBVARIANT):int_ptr; stdcall;
                             external CoreDLL name 'db_get';
function DBGetContactSettingStr(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; dbv:PDBVARIANT; const nType:int):int_ptr; stdcall;
                                external CoreDLL name 'db_get_s';
function DBGetContactSettingString(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar):pAnsiChar; stdcall;
                                   external CoreDLL name 'db_get_sa';
function DBGetContactSettingWString(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar):pWideChar; stdcall;
                                    external CoreDLL name 'db_get_wsa';

function DBWriteContactSettingByte(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; val:byte):int_ptr; stdcall;
                                   external CoreDLL name 'db_set_b';
function DBWriteContactSettingWord(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; val:word):int_ptr; stdcall;
                                   external CoreDLL name 'db_set_w';
function DBWriteContactSettingDWord(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; val:dword):int_ptr; stdcall;
                                    external CoreDLL name 'db_set_dw';
function DBWriteContactSettingString(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; const val:pAnsiChar):int_ptr; stdcall;
                                     external CoreDLL name 'db_set_s';
function DBWriteContactSettingWString(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; const val:pWideChar):int_ptr; stdcall;
                                      external CoreDLL name 'db_set_ws';
function DBWriteContactSettingUTF8String(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; const val:pAnsiChar):int_ptr; stdcall;
                                         external CoreDLL name 'db_set_utf';
function DBWriteContactSettingBlob(hContact:THANDLE; const szModule:pAnsiChar; const szSetting:pAnsiChar; val:pointer; len:uint):int_ptr; stdcall;
                                   external CoreDLL name 'db_set_blob';


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
  TMIRANDASERVICEOBJ      = function(ptr:pointer;wParam,lParam:LPARAM):int_ptr; cdecl;
  TMIRANDASERVICEOBJPARAM = function(ptr:pointer;wParam:WPARAM;lParam,lParam1:LPARAM):int_ptr; cdecl;

const
  {$IFDEF WIN64}
  CALLSERVICE_NOTFOUND = $8000000000000000;
  {$ELSE}
  CALLSERVICE_NOTFOUND = $80000000;
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

function CallContactService(hContact:THANDLE;const name:PAnsiChar;wParam:WPARAM;lParam:LPARAM):int_ptr; cdecl;
                            external CoreDLL name 'CallContactService';
function CallProtoService(const szModule:PAnsiChar;const szService:PAnsiChar;wParam:WPARAM;lParam:LPARAM):int_ptr; cdecl;
                          external CoreDLL name 'CallProtoService';


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
// language packs support

const
  LANG_UNICODE = $1000;

procedure Langpack_SortDuplicates(); stdcall;
                                  external CoreDLL name 'Langpack_SortDuplicates';
function Langpack_GetDefaultCodePage():int; stdcall;
                                  external CoreDLL name 'Langpack_GetDefaultCodePage';
function Langpack_GetDefaultLocale():int; stdcall;
                                  external CoreDLL name 'Langpack_GetDefaultLocale';
function Langpack_PcharToTchar(const pszStr:pAnsiChar):pWideChar; stdcall;
                              external CoreDLL name 'Langpack_PcharToTchar';

function LoadLangPackModule():int; stdcall;
                            external CoreDLL name 'LoadLangPackModule';
function LoadLangPack(const szLangPack:pWideChar):int; stdcall;
                      external CoreDLL name 'LoadLangPack';
procedure ReloadLangpack(pszStr:pWideChar); stdcall;
                         external CoreDLL name 'ReloadLangpack';

function TranslateA_LP(const str:pAnsiChar; hLang:int):pAnsiChar; stdcall;
                       external CoreDLL name 'TranslateA_LP';
function TranslateW_LP(const str:pWideChar; hLang:int):pWideChar; stdcall;
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
// md5 functions

// Define the state of the MD5 Algorithm.
type
  pmir_md5_byte_t = ^mir_md5_byte_t;
  pmir_md5_word_t = ^mir_md5_word_t;
  mir_md5_byte_t = byte;     // 8-bit byte
  mir_md5_word_t = cardinal; // 32-bit word

  pmir_md5_state_t = ^mir_md5_state_t;
  mir_md5_state_t = record
    count:array [0.. 1] of mir_md5_word_t; // message length in bits, lsw first
    abcd :array [0.. 3] of mir_md5_word_t; // digest buffer
    buf  :array [0..63] of mir_md5_byte_t; // accumulate block
  end;

  TMD5Hash = array [0..15] of mir_md5_byte_t;

procedure mir_md5_init(pms:pmir_md5_state_t); stdcall;
                       external CoreDLL name 'mir_md5_init';
procedure mir_md5_append(pms:pmir_md5_state_t; const data:pmir_md5_byte_t; nbytes:int); stdcall;
                         external CoreDLL name 'mir_md5_append';
procedure mir_md5_finish(pms:pmir_md5_state_t; digest:TMD5Hash); stdcall;
                         external CoreDLL name 'mir_md5_finish';
procedure mir_md5_hash(const data:pmir_md5_byte_t; len:int; digest:TMD5Hash); stdcall;
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

procedure CreatePathToFile(wszFilePath:pAnsiChar); stdcall;
                           external CoreDLL name 'CreatePathToFile';
function CreateDirectoryTree(const szDir:pAnsiChar):int; stdcall;
                             external CoreDLL name 'CreateDirectoryTree';
function PathToAbsolute(const pSrc:pAnsiChar; pOut:pAnsiChar; base:pAnsiChar):int; stdcall;
                        external CoreDLL name 'PathToAbsolute';
function PathToRelative(const pSrc:pAnsiChar; pOut:pAnsiChar):int; stdcall;
                        external CoreDLL name 'PathToRelative';

procedure CreatePathToFileW(wszFilePath:pWideChar); stdcall;
                            external CoreDLL name 'CreatePathToFileW';
function CreateDirectoryTreeW(const szDir:pWideChar):int; stdcall;
                              external CoreDLL name 'CreateDirectoryTreeW';
function PathToAbsoluteW(const pSrc:pWideChar; pOut:pWideChar; base:pWideChar):int; stdcall;
                         external CoreDLL name 'PathToAbsoluteW';
function PathToRelativeW(const pSrc:pWideChar; pOut:pWideChar):int; stdcall;
                         external CoreDLL name 'PathToRelativeW';


///////////////////////////////////////////////////////////////////////////////
// print functions

function mir_snprintf(buffer:pAnsiChar;count:size_t;fmt:pAnsiChar{, ...}):int; stdcall;
                      external CoreDLL name 'mir_snprintf';
function mir_sntprintf(buffer:pWideChar;count:size_t;fmt:pWideChar{, ...}):int; stdcall;
                       external CoreDLL name 'mir_sntprintf';
function mir_vsnprintf(buffer:pAnsiChar;count:size_t;fmt:pAnsiChar;va:va_list):int; stdcall;
                       external CoreDLL name 'mir_vsnprintf';
function mir_vsntprintf(buffer:pWideChar;count:size_t;fmt:pWideChar;va:va_list):int; stdcall;
                        external CoreDLL name 'mir_vsntprintf';


///////////////////////////////////////////////////////////////////////////////
// sha1 functions
type
  pmir_sha1_byte_t = ^mir_sha1_byte_t;
  pmir_sha1_long_t = ^mir_sha1_long_t;
  mir_sha1_byte_t = byte;
  mir_sha1_long_t = longword;

const
  MIR_SHA1_HASH_SIZE = 20;

type
  pmir_sha1_ctx = ^mir_sha1_ctx;
  mir_sha1_ctx = record
    H:array [0.. 4] of mir_sha1_long_t;
    W:array [0..79] of mir_sha1_long_t;
    lenW:int;
    sizeHi,sizeLo:mir_sha1_long_t;
  end;

  SHA1Hash = array [0..MIR_SHA1_HASH_SIZE-1] of mir_sha1_byte_t;

procedure mir_sha1_init(ctx:pmir_sha1_ctx); stdcall;
                        external CoreDLL name 'mir_sha1_init';
procedure mir_sha1_append(ctx:pmir_sha1_ctx; dataIn:pmir_sha1_byte_t; len:int); stdcall;
                          external CoreDLL name 'mir_sha1_append';
procedure mir_sha1_finish(ctx:pmir_sha1_ctx; hashout:SHA1Hash); stdcall;
                          external CoreDLL name 'mir_sha1_finish';
procedure mir_sha1_hash(dataIn:pmir_sha1_byte_t; len:int;hashout:SHA1Hash); stdcall;
                        external CoreDLL name 'mir_sha1_hash';


///////////////////////////////////////////////////////////////////////////////
// strings

function rtrim(str:pAnsiChar):pAnsiChar; stdcall;
               external CoreDLL name 'rtrim';
function wrtrim(str:pWideChar):pWideChar; stdcall;
                external CoreDLL name 'wrtrim';

// returns pointer to the beginning of string
function ltrim(str:pAnsiChar):pAnsiChar; stdcall;
               external CoreDLL name 'ltrim';
// returns pointer to the trimmed portion of string
function ltrimp(str:pAnsiChar):pAnsiChar; stdcall;
                external CoreDLL name 'ltrimp';

function wildcmp(name:pAnsiChar; mask:pAnsiChar):int; stdcall;
                 external CoreDLL name 'wildcmp';

// mir_free dest, mir_strdup src to dest
function replaceStr(var dest:pAnsiChar; const src:pAnsiChar):pAnsiChar; stdcall;
                 external CoreDLL name 'replaceStr';
function replaceStrW(var dest:pWideChar; const src:pWideChar):pWideChar; stdcall;
                 external CoreDLL name 'replaceStrW';
///////////////////////////////////////////////////////////////////////////////
// text conversion functions

function mir_a2u_cp(src:PAnsiChar;codepage:int):pWideChar; stdcall;
                    external CoreDLL name 'mir_a2u_cp';
function mir_a2u(src:PAnsiChar):pWideChar; stdcall;
                 external CoreDLL name 'mir_a2u';
function mir_u2a_cp(src:pWideChar;codepage:int):PAnsiChar; stdcall;
                    external CoreDLL name 'mir_u2a_cp';
function mir_u2a(src:pWideChar):PAnsiChar; stdcall;
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

procedure Thread_SetName(const szThreadName:pAnsiChar); stdcall;
                         external CoreDLL name 'Thread_SetName';

procedure KillObjectThreads(pObject:pointer); stdcall;
                            external CoreDLL name 'KillObjectThreads';


///////////////////////////////////////////////////////////////////////////////
// utf8 interface
{ commented to avoid newer Delphi version conflicts
function Utf8Decode(str:PAnsiChar; var ucs2:pWideChar):PAnsiChar;stdcall;
                    external CoreDLL name 'Utf8Decode';
function Utf8DecodeCP(str:PAnsiChar; codepage:int; var ucs2:pWideChar):PAnsiChar;stdcall;
                      external CoreDLL name 'Utf8DecodeCP';

function Utf8DecodeW(const str:PAnsiChar):pWideChar;stdcall;
                     external CoreDLL name 'Utf8DecodeW';

function Utf8Encode(const src:PAnsiChar):PAnsiChar;stdcall;
                    external CoreDLL name 'Utf8Encode';
function Utf8EncodeCP(const src:PAnsiChar;codepage:int):PAnsiChar;stdcall;
                      external CoreDLL name 'Utf8EncodeCP';

function Utf8EncodeW(const src:PWideChar):PAnsiChar;stdcall;
                     external CoreDLL name 'Utf8EncodeW';

function Ucs2toUtf8Len(const src:pWideChar):int; stdcall;
                       external CoreDLL name 'Ucs2toUtf8Len';

function Utf8CheckString(const astr:PAnsiChar):bool;stdcall;
                         external CoreDLL name 'Utf8CheckString';
}
// aliases
function mir_utf8decode(str:PAnsiChar; var ucs2:pWideChar):PAnsiChar;stdcall;
                       external CoreDLL name 'Utf8Decode';
function mir_utf8decodecp(str:PAnsiChar; codepage:int; var ucs2:pWideChar):PAnsiChar;stdcall;
                          external CoreDLL name 'Utf8DecodeCP';
function mir_utf8decodew(const str:PAnsiChar):pWideChar;stdcall;
                         external CoreDLL name 'Utf8DecodeW';

function mir_utf8encode(const src:PAnsiChar):PAnsiChar;stdcall;
                        external CoreDLL name 'Utf8Encode';
function mir_utf8encodecp(const src:PAnsiChar;codepage:int):PAnsiChar;stdcall;
                          external CoreDLL name 'Utf8EncodeCP';

function mir_utf8encodew(const src:PWideChar):PAnsiChar;stdcall;
                         external CoreDLL name 'Utf8EncodeW';

function mir_utf8lenW(const src:pWideChar):int; stdcall;
                       external CoreDLL name 'Ucs2toUtf8Len';

function mir_utf8checkstring(const astr:PAnsiChar):bool;stdcall;
                         external CoreDLL name 'Utf8CheckString';

///////////////////////////////////////////////////////////////////////////////

procedure UnloadCoreModule(); stdcall;
                           external CoreDLL name 'UnloadCoreModule';

{$ENDIF}

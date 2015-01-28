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
  PDBVARIANT = ^TDBVARIANT;
  TDBVARIANT = record
    _type: byte;
    case LongInt of
    0: (bVal: byte);
    1: (cVal: AnsiChar);
    2: (wVal: word);
    3: (sVal: ShortInt);
    4: (dVal: dword);
    5: (lVal: long);
    6: (
      szVal : TChar;
      cchVal: word;
    );
    7: (
      cpbVal: word;
      pbVal : PByte;
    );
  end;

const
  DBEF_SENT      =  2; // if set, the event was sent by the user, otherwise it was received
  DBEF_READ      =  4; // event has been read by the user -- only needed for history
  DBEF_RTL       =  8; // event contains the right-to-left aligned text
  DBEF_UTF       = 16; // event contains a text in utf-8
  DBEF_ENCRYPTED = 32; // event is encrypted (never reported outside a driver)

const
  EVENTTYPE_MESSAGE     = 0;
  EVENTTYPE_URL         = 1;
  EVENTTYPE_CONTACTS    = 2;     // v0.1.2.2+
  EVENTTYPE_ADDED       = 1000;  // v0.1.1.0+: these used to be module-
  EVENTTYPE_AUTHREQUEST = 1001;  // specific codes, hence the module-
  EVENTTYPE_FILE        = 1002;  // specific limit has been raised to 2000

type
  PDBEVENTINFO = ^TDBEVENTINFO;
  TDBEVENTINFO = record
    cbSize   : int;  // size of the structure
    szModule : PAnsiChar; // module that 'owns' this event and controls the data format
    timestamp: dword; // timestamp in UNIX time
    flags    : dword; // the DBEF_* flags above
    eventType: word;  // event type, such as message, can be module defined
    cbBlob   : dword; // size in bytes of pBlob^
    pBlob    : PByte; // pointer to buffer containing the module defined event data
  end;

(******************************************************************************
 * DATABASE EVENTS
 *)

{
Adds a new event to a contact's event list
Returns a handle to the newly added event, or NULL on failure
Triggers a db/event/added event just before it returns.
Events are sorted chronologically as they are entered, so you cannot guarantee
that the new hEvent is the last event in the chain, however if a new event is
added that has a timestamp less than 90 seconds *before* the event that should
be after it, it will be added afterwards, to allow for protocols that only
store times to the nearest minute, and slight delays in transports.
There are a few predefined eventTypes below for easier compatibility, but
modules are free to define their own, beginning at 2000
DBEVENTINFO.timestamp is in GMT, as returned by time(). There are services
db/time/x below with useful stuff for dealing with it.
}
function db_event_add(hContact:TMCONTACT; dbei:PDBEVENTINFO):TMEVENT; stdcall;
                 external CoreDLL name 'db_event_add';

{
Gets the number of events in the chain belonging to a contact in the database.
Returns the number of events in the chain owned by hContact or -1 if hContact
is invalid. They can be retrieved using the db_event_first/last() services.
Returns 0 for Subcontacts (use db_event_last to recognize empty history)
}
function db_event_count(hContact:TMCONTACT):int; stdcall;
                 external CoreDLL name 'db_event_count';

{
Removes a single event from the database
hDbEvent should have been returned by db_event_add/first/last/next/prev()
Returns 0 on success, or nonzero if hDbEvent was invalid
Triggers a db/event/deleted event just *before* the event is deleted
}
function db_event_delete(hContact:TMCONTACT; hDbEvent:TMEVENT):int; stdcall;
                 external CoreDLL name 'db_event_delete';

{
Retrieves a handle to the first event in the chain for hContact
Returns the handle, or NULL if hContact is invalid or has no events
Events in a chain are sorted chronologically automatically
}
function db_event_first(hContact:TMCONTACT):TMEVENT; stdcall;
                 external CoreDLL name 'db_event_first';

{
Retrieves a handle to the first unread event in the chain for hContact
Returns the handle, or NULL if hContact is invalid or all its events have been
read

Events in a chain are sorted chronologically automatically, but this does not
necessarily mean that all events after the first unread are unread too. They
should be checked individually with db_event_next() and db_event_get()
This service is designed for startup, reloading all the events that remained
unread from last time
}
function db_event_firstUnread(hContact:TMCONTACT):TMEVENT; stdcall;
                 external CoreDLL name 'db_event_firstUnread';

{
Retrieves all the information stored in hDbEvent
hDbEvent should have been returned by db_event_add/first/last/next/prev()
Returns 0 on success or nonzero if hDbEvent is invalid
Don't forget to set dbe.cbSize, dbe.pBlob and dbe.cbBlob before calling this
service
The correct value dbe.cbBlob can be got using db/event/getblobsize
If successful, all the fields of dbe are filled. dbe.cbBlob is set to the
actual number of bytes retrieved and put in dbe.pBlob
If dbe.cbBlob is too small, dbe.pBlob is filled up to the size of dbe.cbBlob
and then dbe.cbBlob is set to the required size of data to go in dbe.pBlob
On return, dbe.szModule is a pointer to the database module's own internal list
of modules. Look but don't touch.
}
function db_event_get(hDbEvent:TMEVENT; dbei:PDBEVENTINFO):int; stdcall;
                 external CoreDLL name 'db_event_get';

{
Retrieves the space in bytes required to store the blob in hDbEvent
hDbEvent should have been returned by db_event_add/first/last/next/prev()
Returns the space required in bytes, or -1 if hDbEvent is invalid
}
function db_event_getBlobSize(hDbEvent:TMEVENT):int; stdcall;
                 external CoreDLL name 'db_event_getBlobSize';

{
Retrieves a handle to the contact that owns hDbEvent.
hDbEvent should have been returned by db_event_add/first/last/next/prev()
NULL is a valid return value, meaning, as usual, the user.
Returns (HANDLE)(-1) if hDbEvent is invalid, or the handle to the contact on
success
This service is exceptionally slow. Use only when you have no other choice at
all.
}
function db_event_getContact(hDbEvent:TMEVENT):TMCONTACT; stdcall;
                 external CoreDLL name 'db_event_getContact';

{
Retrieves a handle to the last event in the chain for hContact
Returns the handle, or NULL if hContact is invalid or has no events
Events in a chain are sorted chronologically automatically
}
function db_event_last(hContact:TMCONTACT):TMEVENT; stdcall;
                 external CoreDLL name 'db_event_last';

{
Changes the flags for an event to mark it as read.
hDbEvent should have been returned by db_event_add/first/last/next/prev()
Returns the entire flag dword for the event after the change, or -1 if hDbEvent
is invalid.
This is the one database write operation that does not trigger an event.
Modules should not save flags states for any length of time.
}
function db_event_markRead(hContact:TMCONTACT; hDbEvent:TMEVENT):int; stdcall;
                 external CoreDLL name 'db_event_markRead';

{
Retrieves a handle to the next event in a chain after hDbEvent
Returns the handle, or NULL if hDbEvent is invalid or is the last event
Events in a chain are sorted chronologically automatically
}
function db_event_next(hContact:TMCONTACT; hDbEvent:TMEVENT):THANDLE; stdcall;
                 external CoreDLL name 'db_event_next';

{
Retrieves a handle to the previous event in a chain before hDbEvent
Returns the handle, or NULL if hDbEvent is invalid or is the first event
Events in a chain are sorted chronologically automatically
}
function db_event_prev(hContact:TMCONTACT; hDbEvent:TMEVENT):THANDLE; stdcall;
                 external CoreDLL name 'db_event_prev';

function db_free(dbv:PDBVARIANT):int_ptr; stdcall;
                 external CoreDLL name 'db_free';

(******************************************************************************
 * DATABASE CONTACTS
 *)

{
Gets the handle of the first contact in the database. This handle can be used
with loads of functions. It does not need to be closed.
You can specify szProto to find only its contacts
Returns a handle to the first contact in the db on success, or NULL if there
are no contacts in the db.
}
function db_find_first(const szModule:PAnsiChar=nil):TMCONTACT; stdcall;
                 external CoreDLL name 'db_find_first';

{
Gets the handle of the next contact after hContact in the database. This handle
can be used with loads of functions. It does not need to be closed.
You can specify szProto to find only its contacts
Returns a handle to the contact after hContact in the db on success or NULL if
hContact was the last contact in the db or hContact was invalid.
}
function db_find_next(hContact:TMCONTACT; const szModule:PAnsiChar=nil):TMCONTACT; stdcall;
                 external CoreDLL name 'db_find_next';

(******************************************************************************
 * DATABASE SETTINGS
 *)

function db_get(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; dbv:PDBVARIANT):int_ptr; stdcall;
                 external CoreDLL name 'db_get';
function db_get_b(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; errorValue:int):int; stdcall;
                 external CoreDLL name 'db_get_b';
function db_get_w(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; errorValue:int):int; stdcall;
                 external CoreDLL name 'db_get_w';
function db_get_dw(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; errorValue:dword):dword; stdcall;
                 external CoreDLL name 'db_get_dw';
function db_get_s(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; dbv:PDBVARIANT; const nType:int=DBVT_ASCIIZ):int_ptr; stdcall;
                 external CoreDLL name 'db_get_s';
function db_get_sa(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar):PAnsiChar; stdcall;
                 external CoreDLL name 'db_get_sa';
function db_get_wsa(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar):PWideChar; stdcall;
                 external CoreDLL name 'db_get_wsa';

function db_get_static(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; szDest:PAnsiChar; destLen:int):int; stdcall;
                 external CoreDLL name 'db_get_static';
function db_get_static_utf(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; szDest:PAnsiChar; destLen:int):int; stdcall;
                 external CoreDLL name 'db_get_static_utf';
function db_get_wstatic(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; szDest:PWideChar; destLen:int):int; stdcall;
                 external CoreDLL name 'db_get_wstatic';

function db_set(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; dbv:PDBVARIANT):int_ptr; stdcall;
                 external CoreDLL name 'db_set';
function db_set_b(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; val:byte):int_ptr; stdcall;
                 external CoreDLL name 'db_set_b';
function db_set_w(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; val:word):int_ptr; stdcall;
                 external CoreDLL name 'db_set_w';
function db_set_dw(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; val:dword):int_ptr; stdcall;
                 external CoreDLL name 'db_set_dw';
function db_set_s(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; const val:PAnsiChar):int_ptr; stdcall;
                 external CoreDLL name 'db_set_s';
function db_set_ws(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; const val:PWideChar):int_ptr; stdcall;
                 external CoreDLL name 'db_set_ws';
function db_set_utf(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; const val:PAnsiChar):int_ptr; stdcall;
                 external CoreDLL name 'db_set_utf';
function db_set_blob(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; val:pointer; len:uint):int_ptr; stdcall;
                 external CoreDLL name 'db_set_blob';

function db_unset(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar):int_ptr; stdcall;
                  external CoreDLL name 'db_unset';

function db_set_resident(const szModule:PAnsiChar; const szSetting:PAnsiChar; bEnable:int):int; stdcall;
                 external CoreDLL name 'db_set_resident';

// deprecated Aliases
function DBFreeVariant(dbv:PDBVARIANT):int_ptr; stdcall;
                 external CoreDLL name 'db_free';
function DBDeleteContactSetting(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar):int_ptr; stdcall;
                 external CoreDLL name 'db_unset';
function DBGetContactSettingByte(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; errorValue:int):int; stdcall;
                 external CoreDLL name 'db_get_b';
function DBGetContactSettingWord(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; errorValue:int):int; stdcall;
                 external CoreDLL name 'db_get_w';
function DBGetContactSettingDWord(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; errorValue:dword):dword; stdcall;
                 external CoreDLL name 'db_get_dw';
function DBGetContactSetting(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; dbv:PDBVARIANT):int_ptr; stdcall;
                 external CoreDLL name 'db_get';
function DBGetContactSettingStr(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; dbv:PDBVARIANT; const nType:int=DBVT_ASCIIZ):int_ptr; stdcall;
                 external CoreDLL name 'db_get_s';
function DBGetContactSettingString(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar):PAnsiChar; stdcall;
                 external CoreDLL name 'db_get_sa';
function DBGetContactSettingWString(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar):PWideChar; stdcall;
                 external CoreDLL name 'db_get_wsa';

function DBWriteContactSettingByte(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; val:byte):int_ptr; stdcall;
                 external CoreDLL name 'db_set_b';
function DBWriteContactSettingWord(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; val:word):int_ptr; stdcall;
                 external CoreDLL name 'db_set_w';
function DBWriteContactSettingDWord(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; val:dword):int_ptr; stdcall;
                 external CoreDLL name 'db_set_dw';
function DBWriteContactSettingString(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; const val:PAnsiChar):int_ptr; stdcall;
                 external CoreDLL name 'db_set_s';
function DBWriteContactSettingWString(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; const val:PWideChar):int_ptr; stdcall;
                 external CoreDLL name 'db_set_ws';
function DBWriteContactSettingUTF8String(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; const val:PAnsiChar):int_ptr; stdcall;
                 external CoreDLL name 'db_set_utf';
function DBWriteContactSettingBlob(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; val:pointer; len:uint):int_ptr; stdcall;
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

{$IFNDEF M_DATABASE}
{$DEFINE M_DATABASE}

type
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
  EVENTTYPE_CONTACTS    = 2;     // v0.1.2.2+
  EVENTTYPE_ADDED       = 1000;  // v0.1.1.0+: these used to be module-
  EVENTTYPE_AUTHREQUEST = 1001;  // specific codes, hence the module-
  EVENTTYPE_FILE        = 1002;  // specific limit has been raised to 2000

type
  PDBEVENTINFO = ^TDBEVENTINFO;
  TDBEVENTINFO = record
    szModule : PAnsiChar; // module that 'owns' this event and controls the data format
    timestamp: dword;     // timestamp in UNIX time
    flags    : dword;     // the DBEF_* flags above
    eventType: word;      // event type, such as message, can be module defined
    cbBlob   : dword;     // size in bytes of pBlob^
    pBlob    : PByte;     // pointer to buffer containing the module defined event data
    szId     : PAnsiChar; // server id of event
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
function db_event_add(hContact:TMCONTACT; dbei:PDBEVENTINFO):TMEVENT; stdcall; external CoreDLL;

{
Gets the number of events in the chain belonging to a contact in the database.
Returns the number of events in the chain owned by hContact or -1 if hContact
is invalid. They can be retrieved using the db_event_first/last() services.
Returns 0 for Subcontacts (use db_event_last to recognize empty history)
}
function db_event_count(hContact:TMCONTACT):int; stdcall; external CoreDLL;

{
Removes a single event from the database
hDbEvent should have been returned by db_event_add/first/last/next/prev()
Returns 0 on success, or nonzero if hDbEvent was invalid
Triggers a db/event/deleted event just *before* the event is deleted
}
function db_event_delete(hDbEvent:TMEVENT):int; stdcall; external CoreDLL;

{
Retrieves a handle to the first event in the chain for hContact
Returns the handle, or NULL if hContact is invalid or has no events
Events in a chain are sorted chronologically automatically
}
function db_event_first(hContact:TMCONTACT):TMEVENT; stdcall; external CoreDLL;

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
function db_event_get(hDbEvent:TMEVENT; dbei:PDBEVENTINFO):int; stdcall; external CoreDLL;

{
Retrieves the space in bytes required to store the blob in hDbEvent
hDbEvent should have been returned by db_event_add/first/last/next/prev()
Returns the space required in bytes, or -1 if hDbEvent is invalid
}
function db_event_getBlobSize(hDbEvent:TMEVENT):int; stdcall; external CoreDLL;

{
Retrieves a handle to the contact that owns hDbEvent.
hDbEvent should have been returned by db_event_add/first/last/next/prev()
NULL is a valid return value, meaning, as usual, the user.
Returns (HANDLE)(-1) if hDbEvent is invalid, or the handle to the contact on
success
This service is exceptionally slow. Use only when you have no other choice at
all.
}
function db_event_getContact(hDbEvent:TMEVENT):TMCONTACT; stdcall; external CoreDLL;

{
Retrieves a handle to the last event in the chain for hContact
Returns the handle, or NULL if hContact is invalid or has no events
Events in a chain are sorted chronologically automatically
}
function db_event_last(hContact:TMCONTACT):TMEVENT; stdcall; external CoreDLL;

{
Changes the flags for an event to mark it as read.
hDbEvent should have been returned by db_event_add/first/last/next/prev()
Returns the entire flag dword for the event after the change, or -1 if hDbEvent
is invalid.
This is the one database write operation that does not trigger an event.
Modules should not save flags states for any length of time.
}
function db_event_markRead(hContact:TMCONTACT; hDbEvent:TMEVENT):int; stdcall; external CoreDLL;

{
Retrieves a handle to the next event in a chain after hDbEvent
Returns the handle, or NULL if hDbEvent is invalid or is the last event
Events in a chain are sorted chronologically automatically
}
function db_event_next(hContact:TMCONTACT; hDbEvent:TMEVENT):THANDLE; stdcall; external CoreDLL;

{
Retrieves a handle to the previous event in a chain before hDbEvent
Returns the handle, or NULL if hDbEvent is invalid or is the first event
Events in a chain are sorted chronologically automatically
}
function db_event_prev(hContact:TMCONTACT; hDbEvent:TMEVENT):THANDLE; stdcall; external CoreDLL;

function db_free(dbv:PDBVARIANT):int_ptr; stdcall; external CoreDLL;

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
function db_find_first(const szModule:PAnsiChar=nil):TMCONTACT; stdcall; external CoreDLL;

{
Gets the handle of the next contact after hContact in the database. This handle
can be used with loads of functions. It does not need to be closed.
You can specify szProto to find only its contacts
Returns a handle to the contact after hContact in the db on success or NULL if
hContact was the last contact in the db or hContact was invalid.
}
function db_find_next(hContact:TMCONTACT; const szModule:PAnsiChar=nil):TMCONTACT; stdcall; external CoreDLL;

(******************************************************************************
 * DATABASE SETTINGS
 *)

function db_get(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; dbv:PDBVARIANT):int_ptr; stdcall; external CoreDLL;
function db_get_b(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; errorValue:int):int; stdcall; external CoreDLL;
function db_get_w(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; errorValue:int):int; stdcall; external CoreDLL;
function db_get_dw(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; errorValue:dword):dword; stdcall; external CoreDLL;
function db_get_s(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; dbv:PDBVARIANT; const nType:int=DBVT_ASCIIZ):int_ptr; stdcall; external CoreDLL;

function db_get_static(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; szDest:PAnsiChar; destLen:int):int; stdcall; external CoreDLL;
function db_get_static_utf(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; szDest:PAnsiChar; destLen:int):int; stdcall; external CoreDLL;
function db_get_wstatic(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; szDest:PWideChar; destLen:int):int; stdcall; external CoreDLL;

function db_set(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; dbv:PDBVARIANT):int_ptr; stdcall; external CoreDLL;
function db_set_b(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; val:byte):int_ptr; stdcall; external CoreDLL;
function db_set_w(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; val:word):int_ptr; stdcall; external CoreDLL;
function db_set_dw(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; val:dword):int_ptr; stdcall; external CoreDLL;
function db_set_s(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; const val:PAnsiChar):int_ptr; stdcall; external CoreDLL;
function db_set_ws(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; const val:PWideChar):int_ptr; stdcall; external CoreDLL;
function db_set_utf(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; const val:PAnsiChar):int_ptr; stdcall; external CoreDLL;
function db_set_blob(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar; val:pointer; len:uint):int_ptr; stdcall; external CoreDLL;

function db_unset(hContact:TMCONTACT; const szModule:PAnsiChar; const szSetting:PAnsiChar):int_ptr; stdcall; external CoreDLL;

function db_set_resident(const szModule:PAnsiChar; const szSetting:PAnsiChar; bEnable:int):int; stdcall; external CoreDLL;

///////////////////////////////////////////////////////////////////////////////

function db_add_contact() : TMCONTACT; stdcall; external CoreDLL;
function db_delete_contact(hContact:TMCONTACT) : int; stdcall; external CoreDLL;

function db_is_contact(hContact:TMCONTACT) : int; stdcall; external CoreDLL;

function db_get_contact_count() : int; stdcall; external CoreDLL;

///////////////////////////////////////////////////////////////////////////////

function Profile_GetNameA(cbLen:cardinal; pDest:PAnsiChar) : int; stdcall; external AppDll;
function Profile_GetNameW(cbLen:cardinal; pDest:PWideChar) : int; stdcall; external AppDll;

function Profile_GetPathA(cbLen:cardinal; pDest:PAnsiChar) : int; stdcall; external AppDll;
function Profile_GetPathW(cbLen:cardinal; pDest:PWideChar) : int; stdcall; external AppDll;

///////////////////////////////////////////////////////////////////////////////

type
  PDBCONTACTWRITESETTING = ^TDBCONTACTWRITESETTING;
  TDBCONTACTWRITESETTING = record
    szModule : PAnsiChar;  // module sig to write this setting under
    szSetting: PAnsiChar;  // setting name to write
    value    : TDBVARIANT; // variant containing value to set
  end;

{
    wParam : Handle of a contact to enum settings for
    lParam : Pointer to a TDBCONTACTENUMSETTINGS structure, must be initalised
    affect : Enumerates all settings for a given contact under a module,
             TDBCONTACTENUMSETTINGS must be filled with the function pointer to call
             the TDBCONTACTENUMSETTINGS.lParam value to pass to it each time,
             as well as the .szModule under which the contact is valid
    returns: returns the value of the last call to the enum function, or -1
             if no settings could be enumerated
    notes  : the szSetting argument passed to the enumeration function is only
             valid for the duration of that enumeration call,
             it must be allocated dynamically if it is required after that call frame
             has returned.
             Also, deleting settings as they are enumerated has unpredictable results!
             but writing a new value for a setting is okay.
             it is unclear how you stop the enumeration once it is started, maybe
             possible to return -1 to stop it.
    vesion : only valid for 0.1.0.1+
}

type
  TDBSETTINGENUMPROC = function(const szSetting: PAnsiChar; lParam: LPARAM): int; cdecl;

function db_enum_settings(hContact:TMCONTACT; pFunc:TDBSETTINGENUMPROC; szModule:PAnsiChar; param:pointer) : int; stdcall; external CoreDll;

//************************** Event *********************************

{ DB/EventType/Register service (0.7+)
Registers the specified database event type, with module, id & description.
When someone needs to retrieve an event's text, a service named Module/GetEventText<id>
will be called. For example, for module named 'foo' and event id 2000 a service
foo/GetEventText2000 should be defined to process this request. That handler should
decode a blob and return the event text in the required format, its prototype is identical
to a call of MS_DB_EVENT_GETTEXT (see below)
  wParam=0
  lParam=(LPARAM)(DBEVENTTYPEDESCR*)
Returns -1 on error (e.g., event type already registred), 0 on success
}

type
  PDBEVENTTYPEDESCR = ^TDBEVENTTYPEDESCR;
  TDBEVENTTYPEDESCR = record
    module     :PAnsiChar; // event module name
    flags      :dword;     // flags, combination of the DETF_*
    eventType  :int;       // event id, unique for this module (actually, word size)
    descr      :PAnsiChar; // event type description (i.e. "File Transfer")
    textService:PAnsiChar; // service name for MS_DB_EVENT_GETTEXT (0.8+, default Module+'/GetEventText'+EvtID)
    iconService:PAnsiChar; // service name for MS_DB_EVENT_GETICON (0.8+, default Module+'/GetEventIcon'+EvtID)
    eventIcon  :THANDLE;   // icolib handle to eventicon           (0.8+, default 'eventicon_'+Module+EvtID)
  end;

const
// constants for default event behaviour
  DETF_HISTORY   = 1; // show event in history
  DETF_MSGWINDOW = 2; // show event in message window
  DETF_NONOTIFY  = 4; // block event notify (e.g. Popups)

function DbEvent_RegisterType(et:PDBEVENTTYPEDESCR) : int; stdcall; external AppDll;

function DbEvent_GetType(szModule:PAnsiChar; eventType:int) : PDBEVENTTYPEDESCR; stdcall; external AppDll;

function DbEvent_GetTextA(dbei:PDBEVENTINFO; codepage:int) : PAnsiChar; stdcall; external AppDll;
function DbEvent_GetTextW(dbei:PDBEVENTINFO; codepage:int) : PWideChar; stdcall; external AppDll;

function DbEvent_GetIcon(dbei:PDBEVENTINFO; flags:int) : HICON; stdcall; external AppDll;

//*************************** Modules ******************************

function db_delete_module(hContact:TMCONTACT; szModuleName:PAnsiChar) : int; stdcall; external CoreDll;

//************************** RANDOM ********************************

function db_set_safety_mode(bMode:int) : int; stdcall; external CoreDll;

//************************** EVENTS ********************************

  {
    wParam : TMCONTACT
    lParam : TMEVENT
    Affect : Called when a new event has been added to the event chain
             for a contact, TMCONTACT contains the contact who added the event,
             TMEVENT a handle to what was added.
             see notes
    notes  : since events are sorted chronologically, you can not guarantee
             that TMEVENT is in any particular position in the chain.

  }
const
  ME_DB_EVENT_ADDED:PAnsiChar = 'DB/Event/Added';

  {
    wParam :  TMCONTACT
    lParam :  @DBEVENTINFO
    Affects:  Hook is fired before any DBEVENTS are created within the database for
          a contact (or a user, if hContact is NULL(0)) - It allows a module to
          query/change DBEVENTINFO before it is created, see notes.
    Returns:  Hook should return 1 to stop event being added (will stop other hooks seeing the event too)
          Or 0 to continue processing (passing the data on as well)
    Notes  :  This hook is fired for all event types, and the BLOBS that the eventypes mark
          Maybe changed, therefore be careful about using BLOB formats.
          Because the memory pointing within the DBEVENTINFO CAN NOT BE OWNED or free()'d
          it is recommended that the hook only be used to stop events.
    Version: 0.3.3a+ (2003/12/03)
  }
  ME_DB_EVENT_FILTER_ADD:PAnsiChar = 'DB/Event/FilterAdd';

  {
    wParam : TMCONTACT
    lParam : TMEVENT
    Affect : Called when an event is marked read
  }
  ME_DB_EVENT_MARKED_READ:PAnsiChar = 'DB/Event/Marked/Read';

  {
    wParam : TMCONTACT
    lParam : TMEVENT
    Affect : Called when an event is about to be deleted from the event chain
             for a contact, see notes
    notes  : Returning non zero from your hook will NOT stop the deletion,
             but it will as usual stop other hooks being called
  }
  ME_DB_EVENT_DELETED:PAnsiChar = 'DB/Event/Deleted';

  {
    wParam : TMCONTACT
    lParam : 0
    Affect : Called when a new contact has been added to the database,
             TMCONTACT contains a handle to the new contact.
  }
  ME_DB_CONTACT_ADDED:PAnsiChar = 'DB/Contact/Added';

  {
    wParam : TMCONTACT
    lParam : 0
    Affect : Called when a contact is about to be deleted
    Returns: Returning nonzero from your hook will not stop the deletion
             but it will stop the other hooks from being called
  }
  ME_DB_CONTACT_DELETED:PAnsiChar = 'DB/Contact/Deleted';

  {
    wParam : TMCONTACT
    lParam : Pointer to a TDBCONTACTWRITESETTING
    Affect : Calleed when a contact has one of it's settings changed
             hContact is a valid handle to the contact that has changed,
             see notes.
    notes  : this event will be triggered many times rapidly when alot of values
             are set.
             Modules that hook this should be aware of this fact and quickly
             return if they are not interested in the value that has changed.
             Careful not to get into infinite loops with this event,

             The TDBCONTACTWRITESETTING pointer is the same one as the
             original service all, so don't change any of it's fields
  }
  ME_DB_CONTACT_SETTINGCHANGED:PAnsiChar = 'DB/Contact/SettingChanged';

{$ENDIF}

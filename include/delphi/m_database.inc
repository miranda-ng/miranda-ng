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


const
  {
    wParam : size of the buffer to be filled
    lParam : pointer to the buffer to be filled
    affect : Get's the name of the current profile being used by the database
             module -- this is the same as the filename of the profile without
             the .ext
    return : 0 on success, non zero on failure
  }
  MS_DB_GETPROFILENAME :PAnsiChar = 'DB/GetProfileName';
  MS_DB_GETPROFILENAMEW:PAnsiChar = 'DB/GetProfileNameW';

  {
    wParam : size of buffer pointed to by lParam
    lParam : pointer to a buffer to be filled
    affect : Get the path of the base folder where Miranda will store all individual profiles
             The returned path does NOT include a trailing backslash.
             Essentially this is what has been set in mirandaboot.ini as ProfileDir.
             For more options to retrieve profile paths check MS_UTILS_REPLACEVARS
    return : 0 on success, non zero on failure
    version: 0.3a only
  }
  MS_DB_GETPROFILEPATH :PAnsiChar = 'DB/GetProfilePath';
  MS_DB_GETPROFILEPATHW:PAnsiChar = 'DB/GetProfilePathW';

  {
  Sets the default profile name programmatically
  Analog of Database/DefaultProfile in mirandaboot.ini
    wParam = (WPARAM)(TCHAR*)ptszProfileName
    lParam = 0 (unused)
  }
  MS_DB_SETDEFAULTPROFILE:PAnsiChar = 'DB/SetDefaultProfile';

  {
  Checks the specified profile like dbtool did.
  Implemented in the dbchecker plugins, thus it might not exist
    wParam = (WPARAM)(TCHAR*)ptszProfileName
    lParam = (BOOL)bConversionMode
  }

  MS_DB_CHECKPROFILE:PAnsiChar = 'DB/CheckProfile';

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

  PDBCONTACTENUMSETTINGS = ^TDBCONTACTENUMSETTINGS;
  TDBCONTACTENUMSETTINGS = record
    pfnEnumProc: TDBSETTINGENUMPROC; // function pointer to call to start the
                                     // enum via MS_DB_CONTACT_ENUMSETTINGS
    lParam     : LPARAM;             // passed to the above function
    szModule   : PAnsiChar;          // name of the module to get settings for
    ofsSettings: dword;              // not used by us
  end;

const
  MS_DB_CONTACT_ENUMSETTINGS:PAnsiChar = 'DB/Contact/EnumSettings';

  {
    wParam : 0
    lParam : 0
    affect : none
    returns: Returns the number of contacts in the database for the loaded profile
             not including the profile user, see notes.
    notes  : the contacts in the database can be read with FindFirst/FindNext
  }
  MS_DB_CONTACT_GETCOUNT:PAnsiChar = 'DB/Contact/GetCount';

  {
    wParam : Handle of a contact to delete
    lParam : 0
    affect : the user by the given handle is deleted from the database, see notes
    returns: Returns 0 on success or nonzero if the handle was invalid
    notes  : this triggers DB/Contact/Deleted BEFORE it actually deletes the contact
             all events are also deleted -- other modules may end up with invalid
             handles because of this, which they should be prepared for.
  }
  MS_DB_CONTACT_DELETE:PAnsiChar = 'DB/Contact/Delete';

  {
    wParam : 0
    lParam : 0
    affects: creates a new contact in the database, they have no settings,
             settings must be added with MS_DB_CONTACT_WRITESETTING or
             database helper functions for writing, see notes
    returns: A handle to a new contact or NULL(0) on failure.
    notes  : triggers the ME_DB_CONTACT_ADDED event just before the service returns
  }
  MS_DB_CONTACT_ADD:PAnsiChar = 'DB/Contact/Add';


  {
    wParam : (HANDLE) hContact
    lParam : 0
    affects: Checks the given handle within the database for valid information, for
           a proper internal header.
    returns: Returns 1 if the contact handle is valid, 0 if it is not
    notes  : Due to the nature of multiple threading a contact handle can be deleted
         soon after this service has returned a handle as valid, however it will never point
         to another contact.
  }
  MS_DB_CONTACT_IS:PAnsiChar = 'DB/Contact/Is';

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
    cbSize     :int;       // structure size in bytes
    module     :PAnsiChar; // event module name
    eventType  :int;       // event id, unique for this module (actually, word size)
    descr      :PAnsiChar; // event type description (i.e. "File Transfer")
    textService:PAnsiChar; // service name for MS_DB_EVENT_GETTEXT (0.8+, default Module+'/GetEventText'+EvtID)
    iconService:PAnsiChar; // service name for MS_DB_EVENT_GETICON (0.8+, default Module+'/GetEventIcon'+EvtID)
    eventIcon  :THANDLE;   // icolib handle to eventicon           (0.8+, default 'eventicon_'+Module+EvtID)
    flags      :dword;     // flags, combination of the DETF_*
  end;

const
// constants for default event behaviour
  DETF_HISTORY   = 1; // show event in history
  DETF_MSGWINDOW = 2; // show event in message window
  DETF_NONOTIFY  = 4; // block event notify (e.g. Popups)

const
  MS_DB_EVENT_REGISTERTYPE:PAnsiChar = 'DB/EventType/Register';

  { DB/EventType/Get service (0.7+)
  Retrieves the previously registered database event type, by module & id.
    wParam=(WPARAM)(AnsiChar*)szModule
    lParam=(LPARAM)(int)eventType
  Returns DBEVENTTYPEDESCR* or NULL, if an event isn't found.
  }
  MS_DB_EVENT_GETTYPE:PAnsiChar = 'DB/EventType/Get';

  { DB/Event/GetText (0.7.0+)
    Retrieves the event's text
      wParam=0
      lParam=pointer to TDBEVENTGETTEXT
    dbe should be the valid database event read via MS_DB_EVENT_GET
    Only events of type EVENTTYPE_MESSAGE are supported.

  egt->dbei should be the valid database event read via db_event_get()
  egt->datatype = DBVT_WCHAR or DBVT_ASCIIZ or DBVT_TCHAR.
  egt->codepage is any valid codepage, CP_ACP by default.

    Function returns a pointer to a string in the required format.
    This string should be freed by a call of mir_free
  }
type
  TDBEVENTGETTEXT = record
    dbei:PDBEVENTINFO;
    datatype:int; // DBVT_ASCIIZ, DBVT_WCHAR (DBVT_TCHAR)
    codepage:int;
  end;

const
  MS_DB_EVENT_GETTEXT:PAnsiChar = 'DB/Event/GetText';

  { DB/Event/GetIcon (0.7.0.1+)
    wParam : flags - use LR_SHARED for shared HICON
    lParam : dbei - pointer to DBEVENTINFO
    affect : Retrieves the event's icon
    Returns: HICON (use DestroyIcon to release resources if not LR_SHARED)
    notes  : dbei should be a valid database event read via MS_DB_EVENT_GET
             A plugin can register the standard event icon in IcoLib named
             'eventicon_'+Module+EvtID,like eventicon_ICQ2001. Otherwise, to declare an icon
             with the non-standard name, you can declare the special service,
             Module/GetEventIcon<id>, which will retrieve the custom icon handle (HICON). This
             service function has the same parameters MS_DB_EVENT_GETICON does.
  }
  MS_DB_EVENT_GETICON:PAnsiChar = 'DB/Event/GetIcon';

{ DB/Event/GetString (0.9.0+)
  Converts the event's string to TCHAR* depending on the event's format
  wParam=(LPARAM)(DBEVENTINFO*)dbei
  lParam=(WPARAM)(char*)str - string to be converted
  returns TCHAR* - the converted string
  Caller must free the result using mir_free
}

  MS_DB_EVENT_GETSTRINGT:PAnsiChar = 'DB/Event/GetStringT';

//**************************** Time ********************************

  {
    wParam : timestamp (dword)
    lParam : 0
    Affect : Converts a GMT timestap into local time
    Returns: Returns the converted value, see notes
    Notes  : Timestamps have a zereo at midnight 1/1/1970 GMT, this service
             converts such a value to be based at midnight 1/1/1970 local time.
             This service does not use a simple conversion based on the current offset
             between GMT and local. Rather, it figures out whether daylight savings time
             would have been in place at the time of the stamp and gives the local time as
             it would have been at the time and date the stamp contains.
  }
  MS_DB_TIME_TIMESTAMPTOLOCAL:PAnsiChar = 'DB/Time/TimestampToLocal';

  {
    wParam : timestamp (dword)
    lParam : pointer to initalised DBTIMETOSTRING structure
    Affect : Converts a GMT timestamp to a customisable local time string
             see notes
    Returns: Always returns 0
    notes  : The string is formatted according to thhe current user's locale
             language and preference.

   szFormat can have the following special chars :
      t  time without seconds, e.g. hh:mm
      s  time with seconds, e.g. hh:mm:ss
      m  time without minutes e.g. hh
      d  short date, e.g. dd/mm/yyyy
      D  long date, e.g. d mmmm yyyy
      I  ISO 8061 Time yyyy-mm-ddThh:mm:ssZ

   all other characters are copied as is.
  }

type
  PDBTIMETOSTRING = ^TDBTIMETOSTRING;
  TDBTIMETOSTRING = record
    szFormat: TChar; // format string, see above
    szDest  : TChar; // pointer to dest buffer to store the result
    cbDest  : int;   // size of the buffer
  end;

const
  MS_DB_TIME_TIMESTAMPTOSTRING :PAnsiChar = 'DB/Time/TimestampToString';
  MS_DB_TIME_TIMESTAMPTOSTRINGT:PAnsiChar = 'DB/Time/TimestampToStringT';

//*************************** Random *******************************

  {
    wParam : newSetting (BOOLEAN)
    lParam : 0
    Affect : Miranda's database is normally protected against corruption by
             aggressively flushing data to the disk on writes, if you're doing
             alot of writes e.g. an import plugin, it can sometimes be desirable
             to switch this feature off to speed up the process, if you do switch
             it off, you must remember that crashes are far more likely to be
             catastrophic, so switch it back on at the earliest possible opportunity.
             if you're doing a lot of setting writes, the flush is already delayed
             so you need not use this service for that purpose, see notes.
    Returns: Always returns 0 (successful)
    notes  : This is set to true initally
  }
  MS_DB_SETSAFETYMODE:PAnsiChar = 'DB/SetSafetyMode';

//*************************** Modules ******************************

  {
    wParam : (caller defined data) will be passed to lParam of the call back
    lParam : function pointer to TDBMODULEENUMPROC
    Affects: Enumerates the names of all modules that have stored or
             requested information from the database,
             the modules are returned in no real order --
             Writing to the database while module names are being enumerated will cause
             unpredictable results in the enumeration, but the write will work.

             the enumeration will stop if the callback returns a non zero value.

    Returns: the last return value from the enumeration call back.
    Notes  : This service is only useful for debugging or EnumSettings
    version: The service registered to enumerate all modules that have touched
             the database module uses wParam as the lParam cookie value and the lParam
             value given here is the function pointer -- this is not safe
             to use before v0.1.2.1 because I don't know if this was done in v0.1.2.1-
  }
type
  TDBMODULEENUMPROC = function(const szModule: PAnsiChar; ofsModuleName: dword; lParam: LPARAM): int; cdecl;
const
  MS_DB_MODULES_ENUM:PAnsiChar = 'DB/Modules/Enum';

{ DB/Module/Delete  0.8.0+

  Removes all settings for the specified module.
  wParam=Contact's handle or 0 for global settings
  lParam=(LPARAM)(AnsiChar*)szModuleName - the module name to be deleted
}
  MS_DB_MODULE_DELETE:PAnsiChar = 'DB/Module/Delete';

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

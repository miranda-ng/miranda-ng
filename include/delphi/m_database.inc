{$IFNDEF M_DATABASE}
{$DEFINE M_DATABASE}

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
    lParam = 0 (unused)
  }

  MS_DB_CHECKPROFILE:PAnsiChar = 'DB/CheckProfile';

type
  PDBCONTACTGETSETTING = ^TDBCONTACTGETSETTING;
  TDBCONTACTGETSETTING = record
    szModule : PAnsiChar;  // name of the module that wrote the setting to get
    szSetting: PAnsiChar;  // the name of the setting to get
    pValue   : PDBVARIANT; // pointer to DBVARIANT to receive the value
  end;

  PDBCONTACTWRITESETTING = ^TDBCONTACTWRITESETTING;
  TDBCONTACTWRITESETTING = record
    szModule : PAnsiChar;  // module sig to write this setting under
    szSetting: PAnsiChar;  // setting name to write
    value    : TDBVARIANT; // variant containing value to set
  end;

const
  {
    wParam : Handle of a contact to get the setting for (see notes)
    lParam : pointer to a TDBCONTACTGETSETTING structure to be filled with setting
             this structure also has to be initalised (see notes)
    affect : Queries the database module for a setting from a contact.
    returns: 0 on success, non zero on failure (contact not found, setting doesn't exist)
    notes  : TDBCONTACTGETSETTING must be filled with the module name that created
             /wrote the setting you want to get (e.g. your module name)
             and the actual setting to read with TDBCONTACTGETSETTING.szModule and
             TDBCONTACTGETSETTING.szSetting -- TDBCONTACTGETSETTING.pValue is
             a pointer to a TDBVARIANT with the returned setting, this maybe nil
             and MUST be freed after you're done with it with FreeVariant()

             There are helper functions for reading/writing/deleting common types to and
             from the database -- see DBGetContactSetting<type>

             the contact handle (hContact) can be returned by FindContact/AddContact
  }
  MS_DB_CONTACT_GETSETTING:PAnsiChar = 'DB/Contact/GetSetting';

  {
    DB/Contact/GetSettingString service  0.4.3+
    Same as DB/Contact/GetSetting, but also gets the required string type
    inside the dbcgs->type parameter
  }
  MS_DB_CONTACT_GETSETTING_STR:PAnsiChar = 'DB/Contact/GetSettingStr';

  {
    wParam : Handle for a contact to query a setting for
    lParam : Pointer to a TDBCONTACTGETSETTING structure
    affects: This service is almost the same as the one above, but it does
             not return a dynamic copy (with malloc()) -- the caller
             must do this for datatypes which require it, e.g. a string.

             This means the TDBCONTACTGETSETTING.pValue *has* to exist and be
             allocated by the caller (doesn't have to be allocated from the heap)
             the DBVARIANT structure has to be initalised with the type wanted
             and enough buffer space around to return the info, do not
             expect this service to be as fast as the one above.

    returns: 0 on success, non zero on failure.
  }
  MS_DB_CONTACT_GETSETTINGSTATIC:PAnsiChar = 'DB/Contact/GetSettingStatic';

  {
    wParam : 0
    lParam : Pointer to a TDBVARIANT structure
    affect : Free's the passed DBVARIANT's dynamic memory (if any) see notes
    returns: 0 on success, non zero on failure
    notes  : use the helper function FreeVariant()
  }
  MS_DB_CONTACT_FREEVARIANT:PAnsiChar = 'DB/Contact/FreeVariant';

  {
    wParam : Handle to contact to write setting for
    lParam : Pointer to TDBCONTACTWRITESETTING which must be initalised
    affects: writes a setting under a contact -- TDBCONTACTWRITESETTING structure
             must contain the module name writing -- the setting name, and the value
             to write (which is NOT a pointer) .szModule, .szSetting, .Value, see notes
    returns: 0 on success, non zero on failure
    notes  : this service triggers 'DB/Contact/SettingChanged' before it returns
             as always, there is a helper function to use this service.
  }
  MS_DB_CONTACT_WRITESETTING:PAnsiChar = 'DB/Contact/WriteSetting';

  {
    wParam : hContact under which the setting should be deleted
    lParam : Pointer to a TDBCONTACTGETSETTING structure
    affects: Deletes the given setting for a contact, the TDBCONTACTGETSETTING.pValue
             field is ignored -- only .szModule and .szSetting are needed, see notes
    returns: 0 on success, non zero on failure
    notes  : triggers 'DB/Contact/SettingChanged' BEFORE it deletes the given
             setting, when the service returns the TDBVARIANT structure .type_ is set
             to 0 and no fields are valid, there is a helper function for this
             service, see below.
  }
  MS_DB_CONTACT_DELETESETTING:PAnsiChar = 'DB/Contact/DeleteSetting';

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
    ofsSettings: DWORD;              // not used by us
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
    wParam : 0
    lParam : 0
    returns: Returns a handle to the first contact in the database,
             this handle does not need to be closed, if there are no users
             NULL(0) is returned.
  }
  MS_DB_CONTACT_FINDFIRST:PAnsiChar = 'DB/Contact/FindFirst';

  {
    wParam : Contact handle
    lParam : 0
    returns: Returns a handle to the next contact after the given contact in
             wParam, this handle does not neeed to be closed -- may return NULL(0)
             if the given contact in wParam was the last in the database, or the
             given contact was invalid
  }
  MS_DB_CONTACT_FINDNEXT:PAnsiChar = 'DB/Contact/FindNext';

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
Always returns 0.
}

type
  PDBEVENTTYPEDESCR = ^TDBEVENTTYPEDESCR;
  TDBEVENTTYPEDESCR = record
    cbSize     :int;      // structure size in bytes
    module     :PAnsiChar;    // event module name
    eventType  :int;      // event id, unique for this module
    descr      :PAnsiChar;    // event type description (i.e. "File Transfer")
    textService:PAnsiChar;    // service name for MS_DB_EVENT_GETTEXT (0.8+, default Module+'/GetEventText'+EvtID)
    iconService:PAnsiChar;    // service name for MS_DB_EVENT_GETICON (0.8+, default Module+'/GetEventIcon'+EvtID)
    eventIcon  :THANDLE;  // icolib handle to eventicon (0.8+, default 'eventicon_'+Module+EvtID)
    flags      :DWORD;    // flags, combination of the DETF_*
  end;

const
// constants for default event behaviour
  DETF_HISTORY   = 1; // show event in history
  DETF_MSGWINDOW = 2; // show event in message window
  DETF_NONOTIFY  = 4; // block event notify (e.g. Popups)

const
  DBEVENTTYPEDESCR_SIZE    = sizeof(TDBEVENTTYPEDESCR);
  {$IFNDEF WIN64}
  DBEVENTTYPEDESCR_SIZE_V1 = $10;
  {$ELSE}
  DBEVENTTYPEDESCR_SIZE_V1 = DBEVENTTYPEDESCR_SIZE;
  {$ENDIF}

const
  MS_DB_EVENT_REGISTERTYPE:PAnsiChar = 'DB/EventType/Register';

  { DB/EventType/Get service (0.7+)
  Retrieves the previously registered database event type, by module & id.
    wParam=(WPARAM)(AnsiChar*)szModule
    lParam=(LPARAM)(int)eventType
  Returns DBEVENTTYPEDESCR* or NULL, if an event isn't found.
  }
  MS_DB_EVENT_GETTYPE:PAnsiChar = 'DB/EventType/Get';

  {
    wParam : contact handle for events count is needed
    lParam : 0
    service: Gets the number of events in the chain belonging to a contact
             in the databasee.
    returns: the numbef of events owned by hContact or -1 if hContact
             is invalid, they can be found with the event/find* servicees
  }
  MS_DB_EVENT_GETCOUNT:PAnsiChar = 'DB/Event/GetCount';

  {
    wParam : contact handle to add an event for
    lParam : Pointer to TDBEVENTINFO initialised with data
    affect : Add's an event to the contact's event list, the TDBEVENTINFO
             structure should be filled with the event of message -- see notes
    returns: a handle to a DB event (HDBEVENT), or NULL on error
    notes  : Triggers DB/Event/Added event just before it returns,
             Events are sorted chronologically as they are entered,
             so you cannot guarantee that the new hEvent is the last event in the chain,
             however if a new event is added that has a timestamp less than
             90 seconds *before* the event that should be after it,
             it will be added afterwards, to allow for protocols that only
             store times to the nearest minute, and slight delays in transports.
             There are a few predefined eventTypes below for easier compatibility, but
             modules are free to define their own, beginning at 2000
             DBEVENTINFO.timestamp is in GMT, as returned by time()
  }

  DBEF_FIRST =  1; // internally only, do not use
  DBEF_SENT  =  2; // if set, the event was sent by the user, otherwise it was received
  DBEF_READ  =  4; // event has been read by the user -- only needed for history
  DBEF_RTL   =  8; // event contains the right-to-left aligned text
  DBEF_UTF   = 16; // event contains a text in utf-8

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
    timestamp: DWORD; // timestamp in UNIX time
    flags    : DWORD; // the DBEF_* flags above
    eventType: WORD;  // event type, such as message, can be module defined
    cbBlob   : DWORD; // size in bytes of pBlob^
    pBlob    : PByte; // pointer to buffer containing the module defined event data
  end;

const
  MS_DB_EVENT_ADD:PAnsiChar = 'DB/Event/Add';

  {
    wParam : Handle to the contact
    lParam : HDBEVENT handle to delete
    affects: Removes a single event from the database for the given contact
    returns: 0 on success, nonzero on failure
    notes  : Triggers DB/Event/Deleted just before the event *is* deleted
  }
  MS_DB_EVENT_DELETE:PAnsiChar = 'DB/Event/Delete';

  {
    wParam : Handle to DB event
    lParam : 0
    returns: Returns the space in bytes requried to store the blob in HDBEVENT
             given by HDBEVENT(wParam) -- or -1 on error
  }
  MS_DB_EVENT_GETBLOBSIZE:PAnsiChar = 'DB/Event/GetBlobSize';

  {
    wParam : Handle to a DB event
    lParam : Pointer to a TDBEVENTINFO structure which must be initialised
    affects: Returns all the information about an DB event handle to a TDBEVENTINFO
             structure which must be initalised, DBEI.cbSize, DBEI.pBlob and DBEI.cbSize
             before calling this service, the size can be assertained with
             GetBlobSize() service, see notes
    returns: Returns 0 on success, non zero on failure
    notes  : The correct value dbe.cbBlob can be got using db/event/getblobsize
             If successful, all the fields of dbe are filled. dbe.cbBlob is set to the
             actual number of bytes retrieved and put in dbe.pBlob
             If dbe.cbBlob is too small, dbe.pBlob is filled up to the size of dbe.cbBlob
             and then dbe.cbBlob is set to the required size of data to go in dbe.pBlob
             On return, dbe.szModule is a pointer to the database module's
             own internal list of modules. Look but don't touch.
  }
  MS_DB_EVENT_GET:PAnsiChar = 'DB/Event/Get';

  { DB/Event/GetText (0.7.0+)
    Retrieves the event's text
      wParam=0
      lParam=pointer to TDBEVENTGETTEXT
    dbe should be the valid database event read via MS_DB_EVENT_GET
    Only events of type EVENTTYPE_MESSAGE are supported.
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

  {
    wParam : HCONTACT
    lParam : HDBEVENT
    affect : Changes the flag for an event to mark it as read
    Returns: Returns the entire flag DWORD for the event after the change, or -1
             if HDBEVENT is invalid, see notes
    notes  : This iss one of the database write operations that does not trigger
             an event, modules should not save flagss states for any length of time.
  }
  MS_DB_EVENT_MARKREAD:PAnsiChar = 'DB/Event/MarkRead';

  {
    wParam : HDBEVENT
    lParam : 0
    Affect : Returns a handle to a contact that owns the HDBEVENT,
             see notes
    Returns: Returns a handle if successful or HDBEEVENT(-1) on failure
    notes  : This service is very slow, only use wheen you have no other choice
             at all.
  }
  MS_DB_EVENT_GETCONTACT:PAnsiChar = 'DB/Event/GetContact';

  {
    wParam : HCONTACT
    lParam : 0
    Affect : Retrieves a handlee to the first event in the chain
             for a HCONTACT
    returns: Returns a handle, or NULL(0) if HCONTACT is invalid or has
             no events, events in a chain are sorted chronologically automatically
  }
  MS_DB_EVENT_FINDFIRST:PAnsiChar = 'DB/Event/FindFirst';

  {
    wParam : HCONTACT
    lParam : 0
    Affect : Retrieves a handle to the first unreead event in a chain for a HCONTACT
             see notes
    Returns: Returns a HDBEVENT handle or NULL(0) if the HCONTACT is invalid
             or all it's events have beeen read.
    Notes  : Events in a chain are sorted chronologically automatically,
             but this does not necessarily mean that all events after
             the first unread are unread too.
             They should be checked individually with event/findnext and event/get
             This service is designed for startup, reloading all the events that remained
             unread from last time
  }
  MS_DB_EVENT_FINDFIRSTUNREAD:PAnsiChar = 'DB/Event/FindFirstUnread';

  {
    wParam : HCONTACT
    lParam : 0;
    Affects: Retrieves a handle to the lasts event in the chain for a HCONTACT
    Returns: Returns a handle or NULL(0) if HCONTACT is invalid or has no events
  }
  MS_DB_EVENT_FINDLAST:PAnsiChar = 'DB/Event/FindLast';

  {
    wParam : HDBEVENT
    lParam : 0
    Affects: Retrieves a handle to the next event in a chain after HDBEVENT
    Returns: A handle to the next DB event or NULL(0) if HDBEVENT is invalid
             or the last event in the chain.
  }
  MS_DB_EVENT_FINDNEXT:PAnsiChar = 'DB/Event/FindNext';

  {
    wParam : HDBEVENT
    lParam : 0
    Affects: Retrieves a handle to the previous event in a chain before HDBEVENT
    Returns: A handle to the previous HDBEVENT or NULL(0) if HDBEVENT is invalid
             or is the first event in the chain
  }
  MS_DB_EVENT_FINDPREV:PAnsiChar = 'DB/Event/FindPrev';

//************************** Encryption ****************************

  {
    wParam : size in bytes of string buffer (including null term)
    lParam : pointer to string buffer
    Affect : Scrambles the string buffer in place using a strange encryption algorithm,
             see notes
    Returns: Always returns 0
    notes  : this service may be changed at a later date such that it increasess
             the length of the string
  }
  MS_DB_CRYPT_ENCODESTRING:PAnsiChar = 'DB/Crypt/EncodeString';

  {
    wParam : size in bytes of string buffer, including null term
    lParam : pointer to string buffer
    Affect : Descrambles pszString in-place using the strange encryption algorithm,
             see notes.
    Return : Always returns 0
    notes  : Reverses the operation done by MS_DB_CRYPT_ENCODINGSTRING
  }
  MS_DB_CRYPT_DECODESTRING:PAnsiChar = 'DB/Crypt/DecodeString';

//**************************** Time ********************************

  {
    wParam : timestamp (DWORD)
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
    wParam : timestamp (DWORD)
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

             prior to v0.1.2.1 you can not pass a value to the enumeration because
             of a bug -- which is fixed, but hey :) -- [sam]
  }
type
  TDBMODULEENUMPROC = function(const szModule: PAnsiChar; ofsModuleName: DWORD; lParam: LPARAM): int; cdecl;
const
  MS_DB_MODULES_ENUM:PAnsiChar = 'DB/Modules/Enum';

{ DB/Module/Delete  0.8.0+

  Removes all settings for the specified module.
  wParam=0 (unused)
  lParam=(LPARAM)(AnsiChar*)szModuleName - the module name to be deleted
}
  MS_DB_MODULE_DELETE:PAnsiChar = 'DB/Module/Delete';

//************************** EVENTS ********************************

  {
    wParam : HCONTACT
    lParam : HDBCONTACT
    Affect : Called when a new event has been added to the event chain
             for a contact, HCONTACT contains the contact who added the event,
             HDBCONTACT a handle to what was added.
             see notes
    notes  : since events are sorted chronologically, you can not guarantee
             that HDBEVEnT is in any particular position in the chain.

  }
  ME_DB_EVENT_ADDED:PAnsiChar = 'DB/Event/Added';

  {
    wParam :  HANDLE (hContact)
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
    wParam : HCONTACT
    lParam : HDBEVENT
    Affect : Called when an event is about to be deleted from the event chain
             for a contact, see notes
    notes  : Returning non zero from your hook will NOT stop the deletion,
             but it will as usual stop other hooks being called
  }
  ME_DB_EVENT_DELETED:PAnsiChar = 'DB/Event/Deleted';

  {
    wParam : HCONTACT
    lParam : 0
    Affect : Called when a new contact has been added to the database,
             HCONTACT contains a handle to the new contact.
  }
  ME_DB_CONTACT_ADDED:PAnsiChar = 'DB/Contact/Added';

  {
    wParam : HCONTACT
    lParam : 0
    Affect : Called when a contact is about to be deleted
    Returns: Returning nonzero from your hook will not stop the deletion
             but it will stop the other hooks from being called
  }
  ME_DB_CONTACT_DELETED:PAnsiChar = 'DB/Contact/Deleted';

  {
    wParam : HCONTACT
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

  { DB/Contact/SetSettingResident service (0.6+)
    Disables a setting saving to the database.
    wParam=(WPARAM)(BOOL)bIsResident
    lParam=(LPARAM)(AnsiChar*)pszSettingName
  }
  MS_DB_SETSETTINGRESIDENT:PAnsiChar = 'DB/SetSettingResident';

{$ENDIF}

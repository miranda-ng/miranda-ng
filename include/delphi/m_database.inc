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
    lParam : HDBEVENT
    Affect : Called when a new event has been added to the event chain
             for a contact, TMCONTACT contains the contact who added the event,
             HDBEVENT a handle to what was added.
             see notes
    notes  : since events are sorted chronologically, you can not guarantee
             that HDBEVENT is in any particular position in the chain.

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
    lParam : HDBEVENT
    Affect : Called when an event is marked read
  }
  ME_DB_EVENT_MARKED_READ:PAnsiChar = 'DB/Event/Marked/Read';

  {
    wParam : TMCONTACT
    lParam : HDBEVENT
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

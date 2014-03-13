(*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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

*)

{$IFNDEF M_DATABASE}
{$DEFINE M_DATABASE}

const

    DBVT_DELETED            = 0;        // setting got deleted, no values are valid
    DBVT_BYTE               = 1;        // bVal, cVal are valid
    DBVT_WORD               = 2;        // wVal, sVal are valid
    DBVT_DWORD              = 4;        // dVal, lVal are valid
    DBVT_ASCIIZ             = 255;      // pszVal is valid
    DBVT_BLOB               = 254;      // cpbVal and pbVal are valid
    DBVTF_VARIABLELENGTH    = $80;      // ?

type

    MCONTACT                = Integer;
    HDBEVENT                = Integer;

    PDBVARIANT = ^TDBVARIANT;
    TDBVARIANT = record
        type_: Byte;
        case LongInt of
        0: (bVal: Byte);
        1: (cVal: Char);
        2: (wVal: Word);
        3: (sVal: SmallInt);
        4: (dVal: LongInt);
        5: (lVal: Integer);
        6: (
            pszVal: PChar;
            cchVal: Word;
        );
        7: (
            cpbVal: Word;
            pbVal: PByte;
        );
    end;

const

    {
        wParam : size of the buffer to be filled
        lParam : pointer to the buffer to be filled
        affect : Get's the name of the current profile being used by the database
                 module -- this is the same as the filename of the profile without
                 the .ext
        return : 0 on success, non zero on failure
    }
    MS_DB_GETPROFILENAME            = 'DB/GetProfileName';

    {
        wParam : size of buffer pointed to by lParam
        lParam : pointer to a buffer to be filled
        affect : Fill a buffer with the current profile path being used, this does not include the trailing backslash.
        return : 0 on success, non zero on failure
        version: 0.3a only
    }
    MS_DB_GETPROFILEPATH            = 'DB/GetProfilePath';

type

    PDBCONTACTGETSETTING = ^TDBCONTACTGETSETTING;
    TDBCONTACTGETSETTING = record
        { name of the module that wrote the setting to get }
        szModule: PChar;
        { the name of the setting to get }
        szSetting: PChar;
        { pointer to DBVARIANT to receive the value -- must be allocated for GETSETTINGSTATIC
        calls thou }
        pValue: PDBVARIANT;
    end;

    PDBCONTACTWRITESETTING = ^TDBCONTACTWRITESETTING;
    TDBCONTACTWRITESETTING = record
        { module sig to write this setting under }
        szModule: PChar;
        { setting name to write }
        szSetting: PChar;
        { variant containing value to set }
        value: TDBVARIANT;
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
    MS_DB_CONTACT_GETSETTING        = 'DB/Contact/GetSetting';

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
    MS_DB_CONTACT_GETSETTINGSTATIC  = 'DB/Contact/GetSettingStatic';

    {
        wParam : 0
        lParam : Pointer to a TDBVARIANT structure
        affect : Free's the passed DBVARIANT's dynamic memory (if any) see notes
        returns: 0 on success, non zero on failure
        notes  : use the helper function FreeVariant()
    }
    MS_DB_CONTACT_FREEVARIANT       = 'DB/Contact/FreeVariant';

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
    MS_DB_CONTACT_WRITESETTING      = 'DB/Contact/WriteSetting';

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
    MS_DB_CONTACT_DELETESETTING     = 'DB/Contact/DeleteSetting';

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

    TDBSETTINGENUMPROC = function(const szSetting: PChar; lParam: LPARAM): int; cdecl;

    PDBCONTACTENUMSETTINGS = ^TDBCONTACTENUMSETTINGS;
    TDBCONTACTENUMSETTINGS = record
        { function pointer to call to start the enum via MS_DB_CONTACT_ENUMSETTINGS }
        pfnEnumProc: TDBSETTINGENUMPROC;
        { passed to the above function }
        lParam: LPARAM;
        { name of the module to get settings for }
        szModule: PChar;
        { not used by us }
        ofsSettings: DWORD;
    end;

const

    MS_DB_CONTACT_ENUMSETTINGS      = 'DB/Contact/EnumSettings';

    {
        wParam : 0
        lParam : 0
        affect : none
        returns: Returns the number of contacts in the database for the loaded profile
                 not including the profile user, see notes.
        notes  : the contacts in the database can be read with FindFirst/FindNext
    }
    MS_DB_CONTACT_GETCOUNT          = 'DB/Contact/GetCount';

    {
        wParam : 0
        lParam : 0
        returns: Returns a handle to the first contact in the database,
                 this handle does not need to be closed, if there are no users
                 NULL(0) is returned.
    }
    MS_DB_CONTACT_FINDFIRST         = 'DB/Contact/FindFirst';

    {
        wParam : Contact handle
        lParam : 0
        returns: Returns a handle to the next contact after the given contact in
                 wParam, this handle does not neeed to be closed -- may return NULL(0)
                 if the given contact in wParam was the last in the database, or the
                 given contact was invalid
    }
    MS_DB_CONTACT_FINDNEXT          = 'DB/Contact/FindNext';

    {
        wParam : Handle of a contact to delete
        lParam : 0
        affect : the user by the given handle is deleted from the database, see notes
        returns: Returns 0 on success or nonzero if the handle was invalid
        notes  : this triggers DB/Contact/Deleted BEFORE it actually deletes the contact
                 all events are also deleted -- other modules may end up with invalid
                 handles because of this, which they should be prepared for.
    }
    MS_DB_CONTACT_DELETE            = 'DB/Contact/Delete';

    {
        wParam : 0
        lParam : 0
        affects: creates a new contact in the database, they have no settings,
                 settings must be added with MS_DB_CONTACT_WRITESETTING or
                 database helper functions for writing, see notes
        returns: A handle to a new contact or NULL(0) on failure.
        notes  : triggers the ME_DB_CONTACT_ADDED event just before the service returns
    }
    MS_DB_CONTACT_ADD               = 'DB/Contact/Add';


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
	MS_DB_CONTACT_IS 				= 'DB/Contact/Is';


    {
        wParam : contact handle for events count is needed
        lParam : 0
        service: Gets the number of events in the chain belonging to a contact
                 in the databasee.
        returns: the numbef of events owned by hContact or -1 if hContact
                 is invalid, they can be found with the event/find* servicees
    }
    MS_DB_EVENT_GETCOUNT            = 'DB/Event/GetCount';

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

    DBEF_SENT  = 2;     // if set, the event was sent by the user, otherwise it was received
    DBEF_READ  = 4;     // event has been read by the user -- only needed for history

    EVENTTYPE_MESSAGE       = 0;
    EVENTTYPE_URL           = 1;
    EVENTTYPE_CONTACTS      = 2;     // v0.1.2.2+
    EVENTTYPE_ADDED         = 1000;  // v0.1.1.0+: these used to be module-
    EVENTTYPE_AUTHREQUEST   = 1001;  // specific codes, hence the module-
    EVENTTYPE_FILE          = 1002;  // specific limit has been raised to 2000

type

    PDBEVENTINFO = ^TDBEVENTINFO;
    TDBEVENTINFO = record
        { size of the structure }
        cbSize: int;
        { module that 'owns' this event and controls the data format }
        szModule: PChar;
        { timestamp in UNIX time }
        timestamp: DWORD;
        { the DBEF_* flags above }
        flags: DWORD;
        { event type, such as message, can be module defined }
        eventType: WORD;
        { size in bytes of pBlob^ }
        cbBlob: DWORD;
        { pointer to buffer containing the module defined event data }
        pBlob: PByte;
    end;

const

    MS_DB_EVENT_ADD                 = 'DB/Event/Add';



    {
        wParam : Handle to the contact
        lParam : HDBEVENT handle to delete
        affects: Removes a single event from the database for the given contact
        returns: 0 on success, nonzero on failure
        notes  : Triggers DB/Event/Deleted just before the event *is* deleted
    }
    MS_DB_EVENT_DELETE              = 'DB/Event/Delete';

    {
        wParam : Handle to DB event
        lParam : 0
        returns: Returns the space in bytes requried to store the blob in HDBEVENT
                 given by HDBEVENT(wParam) -- or -1 on error
    }
    MS_DB_EVENT_GETBLOBSIZE         = 'DB/Event/GetBlobSize';

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
    MS_DB_EVENT_GET                 = 'DB/Event/Get';

    {
        wParam : MCONTACT
        lParam : HDBEVENT
        affect : Changes the flag for an event to mark it as read
        Returns: Returns the entire flag DWORD for the event after the change, or -1
                 if HDBEVENT is invalid, see notes
        notes  : This iss one of the database write operations that does not trigger
                 an event, modules should not save flagss states for any length of time.
    }
    MS_DB_EVENT_MARKREAD            = 'DB/Event/MarkRead';

    {
        wParam : HDBEVENT
        lParam : 0
        Affect : Returns a handle to a contact that owns the HDBEVENT,
                 see notes
        Returns: Returns a handle if successful or HDBEEVENT(-1) on failure
        notes  : This service is very slow, only use wheen you have no other choice
                 at all.
    }
    MS_DB_EVENT_GETCONTACT          = 'DB/Event/GetContact';

    {
        wParam : MCONTACT
        lParam : 0
        Affect : Retrieves a handlee to the first event in the chain
                 for a MCONTACT
        returns: Returns a handle, or NULL(0) if MCONTACT is invalid or has
                 no events, events in a chain are sorted chronologically automatically
    }
    MS_DB_EVENT_FINDFIRST           = 'DB/Event/FindFirst';

    {
        wParam : MCONTACT
        lParam : 0
        Affect : Retrieves a handle to the first unreead event in a chain for a MCONTACT
                 see notes
        Returns: Returns a HDBEVENT handle or NULL(0) if the MCONTACT is invalid
                 or all it's events have beeen read.
        Notes  : Events in a chain are sorted chronologically automatically,
                 but this does not necessarily mean that all events after
                 the first unread are unread too.
                 They should be checked individually with event/findnext and event/get
                 This service is designed for startup, reloading all the events that remained
                 unread from last time
    }
    MS_DB_EVENT_FINDFIRSTUNREAD     = 'DB/Event/FindFirstUnread';

    {
        wParam : MCONTACT
        lParam : 0;
        Affects: Retrieves a handle to the lasts event in the chain for a MCONTACT
        Returns: Returns a handle or NULL(0) if MCONTACT is invalid or has no events
    }
    MS_DB_EVENT_FINDLAST            = 'DB/Event/FindLast';

    {
        wParam : HDBEVENT
        lParam : 0
        Affects: Retrieves a handle to the next event in a chain after HDBEVENT
        Returns: A handle to the next DB event or NULL(0) if HDBEVENT is invalid
                 or the last event in the chain.
    }
    MS_DB_EVENT_FINDNEXT            = 'DB/Event/FindNext';

    {
        wParam : HDBEVENT
        lParam : 0
        Affects: Retrieves a handle to the previous event in a chain before HDBEVENT
        Returns: A handle to the previous HDBEVENT or NULL(0) if HDBEVENT is invalid
                 or is the first event in the chain
    }
    MS_DB_EVENT_FINDPREV            = 'DB/Event/FindPrev';



    {
        wParam : size in bytes of string buffer (including null term)
        lParam : pointer to string buffer
        Affect : Scrambles the string buffer in place using a strange encryption algorithm,
                 see notes
        Returns: Always returns 0
        notes  : this service may be changed at a later date such that it increasess
                 the length of the string
    }
    MS_DB_CRYPT_ENCODESTRING        = 'DB/Crypt/EncodeString';

    {
        wParam : size in bytes of string buffer, including null term
        lParam : pointer to string buffer
        Affect : Descrambles pszString in-place using the strange encryption algorithm,
                 see notes.
        Return : Always returns 0
        notes  : Reverses the operation done by MS_DB_CRYPT_ENCODINGSTRING
    }
    MS_DB_CRYPT_DECODESTRING        = 'DB/Crypt/DecodeString';



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
    MS_DB_TIME_TIMESTAMPTOLOCAL     = 'DB/Time/TimestampToLocal';

    {
        wParam : timestamp (DWORD)
        lParam : pointer to initalised DBTIMETOSTRING structure
        Affect : Converts a GMT timestamp to a customisable local time string
                 see notes
        Returns: Always returns 0
        notes  : The string is formatted according to thhe current user's locale
                 language and preference --

                 .szFormat can have the following special chars :
                    t       time without seconds, e.g. hh:mm
                    s       time with seconds, e.g. hh:mm:ss
                    m       time without minutes e.g. hh
                    d       short date, e.g. dd/mm/yyyy
                    D       long date, e.g. d mmmm yyyy

                 all other characters are copied as is.
    }

type

    PDBTIMETOSTRING = ^TDBTIMETOSTRING;
    TDBTIMETOSTRING = record
        { format string, see above }
        szFormat: PChar;
        { pointer to dest buffer to store the result }
        szDest: PChar;
        { size of the buffer }
        cbDest: int;
    end;

const

    MS_DB_TIME_TIMESTAMPTOSTRING    = 'DB/Time/TimestampToString';



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
    MS_DB_SETSAFETYMODE             = 'DB/SetSafetyMode';

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
    TDBMODULEENUMPROC = function(const szModule: PChar; ofsModuleName: DWORD; lParam: LPARAM): int; cdecl;
const
    MS_DB_MODULES_ENUM              = 'DB/Modules/Enum';



    {
        wParam : MCONTACT
        lParam : HDBCONTACT
        Affect : Called when a new event has been added to the event chain
                 for a contact, MCONTACT contains the contact who added the event,
                 HDBCONTACT a handle to what was added.
                 see notes
        notes  : since events are sorted chronologically, you can not guarantee
                 that HDBEVEnT is in any particular position in the chain.

    }
    ME_DB_EVENT_ADDED               = 'DB/Event/Added';
                 
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
	ME_DB_EVENT_FILTER_ADD 			= 'DB/Event/FilterAdd';

    {
        wParam : MCONTACT
        lParam : HDBEVENT
        Affect : Called when an event is about to be deleted from the event chain
                 for a contact, see notes
        notes  : Returning non zero from your hook will NOT stop the deletion,
                 but it will as usual stop other hooks being called
    }
    ME_DB_EVENT_DELETED             = 'DB/Event/Deleted';



    {
        wParam : MCONTACT
        lParam : 0
        Affect : Called when a new contact has been added to the database,
                 MCONTACT contains a handle to the new contact.
    }
    ME_DB_CONTACT_ADDED             = 'DB/Contact/Added';

    {
        wParam : MCONTACT
        lParam : 0
        Affect : Called when a contact is about to be deleted
        Returns: Returning nonzero from your hook will not stop the deletion
                 but it will stop the other hooks from being called
    }
    ME_DB_CONTACT_DELETED           = 'DB/Contact/Deleted';

    {
        wParam : MCONTACT
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
    ME_DB_CONTACT_SETTINGCHANGED    = 'DB/Contact/SettingChanged';

{$ENDIF}

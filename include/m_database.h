/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
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
*/

#ifndef M_DATABASE_H__
#define M_DATABASE_H__ 1

/******************* DATABASE MODULE ***************************/

/*	Notes (as I think of them):
- The module is 100% thread-safe
- The database is the main routing point for the vast majority of Miranda.
  Events are sent from the protocol module to here, and the send/recv message
  module (for example) hooks the db/event/added event. Events like 'contact
  online status changed' do not come through here - icqlib will send that one.
- contacts work much the same. the find/add users module calls db/contact/add
  and db/contact/writesetting and the contact list will get db/contact/added
  and db/contact/settingchanged events
- The user is just a special contact. A hcontact of NULL in most functions
  means the user. Functions in which it cannot be used will be stated
- events attached to the user are things like system messages
- also in this module are crypt/decrypt functions for stuff that should be
  obfuscated on the disk, and some time functions for dealing with timestamps
  in events.
- the contactsettings system is designed for being read by many different
  modules. eg lots of people will be interested in "ICQ"/"UIN", but the module
  name passed to contact/writesetting should always be your own. The Mirabilis
  ICQ database importer clearly has to be an exception to this rule, along with
  a few other bits.
- the current database format means that geteventcontact is exceptionally slow.
  It should be avoidable in most cases so I'm not too concerned, but if people
  really need to use it a lot, I'll sort it out.
- handles do not need to be closed unless stated
- the database is loaded as a memory mapped file. This has various
  disadvantages but a massive advantage in speed for random access.
- The database is optimised for reading. Write performance is fairly bad,
  except for adding events which is the most common activity and pretty good.
- I'll work on caching to improve this later
- Deleted items are left as empty space and never reused. All new items are
  put at the end. A count is kept of this slack space and at some point a
  separate programme will need to be written to repack the database when the
  slack gets too high. It's going to be a good few months of usage before this
  can happen to anyone though, so no rush.
*/

/******************** GENERALLY USEFUL STUFF***********************/

#if !defined(M_SYSTEM_H__)
	#include "m_system.h"
#endif

#if !defined(M_UTILS_H__)
	#include "m_utils.h"
#endif

#ifdef _MSC_VER
	#pragma warning(disable:4201 4204)
#endif

/******************************************************************/
/************************* SERVICES *******************************/
/******************************************************************/

/* DB/GetProfileName service
Gets the name of the profile currently being used by the database module. This
is the same as the filename of the database
  wParam = (WPARAM)(UINT)cbSize
  lParam = (LPARAM)(char*)pszName
pszName is a pointer to the buffer that receives the name of the profile
cbSize is the size in characters of the pszName buffer
Returns 0 on success or nonzero otherwise
*/
#define MS_DB_GETPROFILENAME  "DB/GetProfileName"
#define MS_DB_GETPROFILENAMEW "DB/GetProfileNameW"

/* DB/GetProfilePath service
Get the path of the base folder where Miranda will store all individual profiles
The returned path does NOT include a trailing backslash.
Essentially this is what has been set in mirandaboot.ini as ProfileDir.
For more options to retrieve profile paths check MS_UTILS_REPLACEVARS
  wParam = (WPARAM)(UINT)cbSize
  lParam = (LPARAM)(char*)pszName
pszName is a pointer to the buffer that receives the path of the profile
cbSize is the size in characters of the pszName buffer
Returns 0 on success or nonzero otherwise
*/
#define MS_DB_GETPROFILEPATH  "DB/GetProfilePath"
#define MS_DB_GETPROFILEPATHW "DB/GetProfilePathW"

#if defined(_UNICODE)
	#define MS_DB_GETPROFILEPATHT MS_DB_GETPROFILEPATHW
	#define MS_DB_GETPROFILENAMET MS_DB_GETPROFILENAMEW
#else
	#define MS_DB_GETPROFILEPATHT MS_DB_GETPROFILEPATH
	#define MS_DB_GETPROFILENAMET MS_DB_GETPROFILENAME
#endif

/* DB/SetDefaultProfile service
Sets the default profile name programmatically
Analog of Database/DefaultProfile in mirandaboot.ini
  wParam = (WPARAM)(TCHAR*)ptszProfileName
  lParam = 0 (unused)
*/

#define MS_DB_SETDEFAULTPROFILE "DB/SetDefaultProfile"

/* DB/CheckProfile service
Checks the specified profile like dbtool did.
Implemented in the dbchecker plugins, thus it might not exist
  wParam = (WPARAM)(TCHAR*)ptszProfileName
  lParam = (BOOL)bConversionMode
*/

#define MS_DB_CHECKPROFILE "DB/CheckProfile"

/************************* Contact ********************************/

typedef struct {
	const char *szModule;   // pointer to name of the module that wrote the
	                        // setting to get
	const char *szSetting;  // pointer to name of the setting to get
	DBVARIANT *pValue;      // pointer to variant to receive the value
} DBCONTACTGETSETTING;

typedef struct {
	const char *szModule;   // pointer to name of the module that wrote the
	                        // setting to get
	const char *szSetting;  // pointer to name of the setting to get
	DBVARIANT value;        // variant containing the value to set
} DBCONTACTWRITESETTING;

/* db/contact/enumsettings    v0.1.0.1+
Lists all the settings a specific modules has stored in the database for a
specific contact.
wParam = (MCONTACT)hContact
lParam = (LPARAM)(DBCONTACTENUMSETTINGS*)&dbces
Returns the return value of the last call to pfnEnumProc, or -1 if there are
no settings for that module/contact pair
Writing to or deleting from the database while enumerating will have
unpredictable results for the enumeration, but the write will succeed.
Use db/modules/enum to get a complete list of module names
szSetting is only guaranteed to be valid for the duration of the callback. If
you want to keep it for longer you must allocation your own storage.
*/
typedef int (*DBSETTINGENUMPROC)(const char *szSetting, LPARAM lParam);
typedef struct {
	DBSETTINGENUMPROC pfnEnumProc;
	LPARAM lParam;        // passed direct to pfnEnumProc
	const char *szModule; // name of the module to get settings for
	DWORD ofsSettings;    // filled by the function to contain the offset from
	                      // the start of the database of the requested settings group.
} DBCONTACTENUMSETTINGS;
#define MS_DB_CONTACT_ENUMSETTINGS   "DB/Contact/EnumSettings"

/* DB/Contact/GetCount service
Gets the number of contacts in the database, which does not count the user
  wParam = lParam = 0
Returns the number of contacts. They can be retrieved using contact/findfirst
and contact/findnext
*/
#define MS_DB_CONTACT_GETCOUNT  "DB/Contact/GetCount"

/* DB/Contact/Delete
Deletes the contact hContact from the database and all events and settings
associated with it.
  wParam = (MCONTACT)hContact
  lParam = 0
Returns 0 on success or nonzero if hContact was invalid
Please don't try to delete the user contact (hContact = NULL)
Triggers a db/contact/deleted event just *before* it removes anything
Because all events are deleted, lots of people may end up with invalid event
handles from this operation, which they should be prepared for.
*/
#define MS_DB_CONTACT_DELETE  "DB/Contact/Delete"

/* DB/Contact/Add
Adds a new contact to the database. New contacts initially have no settings
whatsoever, they must all be added with db/contacts/writesetting.
  wParam = lParam = 0
Returns a handle to the newly created contact on success, or NULL otherwise.
Triggers a db/contact/added event just before it returns.
*/
#define MS_DB_CONTACT_ADD  "DB/Contact/Add"

/* DB/Contact/Is
Checks if a given value is a valid contact handle, note that due
to the nature of multiple threading, a valid contact can still become
invalid after a call to this service.
	wParam = (WPARAM)hContact
	lParam = 0
Returns 1 if the contact is a contact, or 0 if the contact is not valid.
*/
#define MS_DB_CONTACT_IS "DB/Contact/Is"

/************************** Event *********************************/

/* DB/EventType/Register service (0.7+)
Registers the specified database event type, with module, id & description.
When someone needs to retrieve an event's text, a service named Module/GetEventText<id>
will be called. For example, for module named 'foo' and event id 2000 a service
foo/GetEventText2000 should be defined to process this request. That handler should
decode a blob and return the event text in the required format, its prototype is identical
to a call of MS_DB_EVENT_GETTEXT (see below)
  wParam = 0
  lParam = (LPARAM)(DBEVENTTYPEDESCR*)
Returns -1 on error (e.g., event type already registred), 0 on success
*/

typedef struct
{
	int    cbSize;      // structure size in bytes
	LPSTR  module;      // event module name
	int    eventType;   // event id, unique for this module
	LPSTR  descr;       // event type description (i.e. "File Transfer")
	LPSTR  textService; // service name for MS_DB_EVENT_GETTEXT (0.8+, default Module+'/GetEventText'+EvtID)
	LPSTR  iconService; // service name for MS_DB_EVENT_GETICON (0.8+, default Module+'/GetEventIcon'+EvtID)
	HANDLE eventIcon;  // icolib handle to eventicon (0.8+, default 'eventicon_'+Module+EvtID)
	DWORD  flags;       // flags, combination of the DETF_*
}
	DBEVENTTYPEDESCR;

// constants for default event behaviour
#define DETF_HISTORY    1   // show event in history
#define DETF_MSGWINDOW  2   // show event in message window
#define DETF_NONOTIFY   4   // block event notify (e.g. Popups)

#define MS_DB_EVENT_REGISTERTYPE  "DB/EventType/Register"

/* DB/EventType/Get service (0.7+)
Retrieves the previously registered database event type, by module & id.
  wParam = (WPARAM)(char*)szModule
  lParam = (LPARAM)(int)eventType
Returns DBEVENTTYPEDESCR* or NULL, if an event isn't found.
*/

#define MS_DB_EVENT_GETTYPE "DB/EventType/Get"

__forceinline MCONTACT DbGetAuthEventContact(DBEVENTINFO* dbei)
{
	return (MCONTACT)(*(DWORD*)&dbei->pBlob[sizeof(DWORD)]);
}

/* DB/Event/GetText (0.7.0+)
Retrieves the event's text
  wParam = 0 (unused)
  lParam = (LPARAM)(DBEVENTGETTEXT*)egt - pointer to structure with parameters
  egt->dbei should be the valid database event read via db_event_get()
  egt->datatype = DBVT_WCHAR or DBVT_ASCIIZ or DBVT_TCHAR.
  egt->codepage is any valid codepage, CP_ACP by default.

Function returns a pointer to a string in the required format.
This string should be freed by a call of mir_free
*/

typedef struct {
	DBEVENTINFO* dbei;
	int datatype;
	int codepage;
} DBEVENTGETTEXT;

#define MS_DB_EVENT_GETTEXT "DB/Event/GetText"

__forceinline char* DbGetEventTextA(DBEVENTINFO* dbei, int codepage)
{
	DBEVENTGETTEXT temp = { dbei, DBVT_ASCIIZ, codepage };
	return (char*)CallService(MS_DB_EVENT_GETTEXT, 0, (LPARAM)&temp);
}

__forceinline WCHAR* DbGetEventTextW(DBEVENTINFO* dbei, int codepage)
{
	DBEVENTGETTEXT temp = { dbei, DBVT_WCHAR, codepage };
	return (WCHAR*)CallService(MS_DB_EVENT_GETTEXT, 0, (LPARAM)&temp);
}

__forceinline TCHAR* DbGetEventTextT(DBEVENTINFO* dbei, int codepage)
{
	DBEVENTGETTEXT temp = { dbei, DBVT_TCHAR, codepage };
	return (TCHAR*)CallService(MS_DB_EVENT_GETTEXT, 0, (LPARAM)&temp);
}

/* DB/Event/GetIcon (0.7.0.1+)
Retrieves the event's icon
  wParam = (WPARAM)(int)flags - use LR_SHARED for shared HICON
  lParam = (LPARAM)(DBEVENTINFO*)dbei
dbei should be a valid database event read via db_event_get()

Function returns HICON (use DestroyIcon to release resources if not LR_SHARED)

A plugin can register the standard event icon in IcoLib named
'eventicon_'+Module+EvtID, like eventicon_ICQ2001. Otherwise, to declare an icon
with the non-standard name, you can declare the special service, Module/GetEventIcon<id>,
which will retrieve the custom icon handle (HICON). This service function has the
same parameters MS_DB_EVENT_GETICON does.
*/
#define MS_DB_EVENT_GETICON "DB/Event/GetIcon"

/* DB/Event/GetString (0.9.0+)
Converts the event's string to TCHAR* depending on the event's format
  wParam = (LPARAM)(DBEVENTINFO*)dbei
  lParam = (WPARAM)(char*)str - string to be converted
  returns TCHAR* - the converted string
Caller must free the result using mir_free
*/

#define MS_DB_EVENT_GETSTRINGT "DB/Event/GetStringT"

__forceinline TCHAR* DbGetEventStringT(DBEVENTINFO* dbei, const char* str)
{
	return (TCHAR*)CallService(MS_DB_EVENT_GETSTRINGT, (WPARAM)dbei, (LPARAM)str);
}

/**************************** Time ********************************/

/* DB/Time/TimestampToLocal
Converts a GMT timestamp into local time
  wParam = (WPARAM)(DWORD)timestamp
  lParam = 0
Returns the converted value
Timestamps have zero at midnight 1/1/1970 GMT, this service converts such a
value to be based at midnight 1/1/1970 local time.
This service does not use a simple conversion based on the current offset
between GMT and local. Rather, it figures out whether daylight savings time
would have been in place at the time of the stamp and gives the local time as
it would have been at the time and date the stamp contains.
This service isn't nearly as useful as db/time/TimestampToString below and I
recommend avoiding its use when possible so that you don't get your timezones
mixed up (like I did. Living at GMT makes things easier for me, but has certain
disadvantages :-)).
*/
#define MS_DB_TIME_TIMESTAMPTOLOCAL   "DB/Time/TimestampToLocal"

/* DB/Time/TimestampToString
Converts a GMT timestamp into a customisable local time string
  wParam = (WPARAM)(DWORD)timestamp
  lParam = (LPARAM)(DBTIMETOSTRING*)&tts
Returns 0 always
Uses db/time/timestamptolocal for the conversion so read that description to
see what's going on.
The string is formatted according to the current user's locale, language and
preferences.
szFormat can have the following special characters:
  t  Time without seconds, eg hh:mm
  s  Time with seconds, eg hh:mm:ss
  m  Time without minutes, eg hh
  d  Short date, eg dd/mm/yyyy
  D  Long date, eg d mmmm yyyy
  I  ISO 8061 Time yyyy-mm-ddThh:mm:ssZ
All other characters are copied across to szDest as-is
*/
typedef struct {
	char *szFormat;  // format string, as above
	char *szDest;    // place to put the output string
	int cbDest;      // maximum number of bytes to put in szDest
} DBTIMETOSTRING;
#define MS_DB_TIME_TIMESTAMPTOSTRING  "DB/Time/TimestampToString"

typedef struct {
	TCHAR *szFormat; // format string, as above
	TCHAR *szDest;	  // place to put the output string
	int cbDest;      // maximum number of bytes to put in szDest
} DBTIMETOSTRINGT;
#define MS_DB_TIME_TIMESTAMPTOSTRINGT "DB/Time/TimestampToStringT"

/*************************** Random *******************************/

/*
Switches safety settings on or off
wParam = (WPARAM)(BOOL)newSetting
lParam = 0
returns 0 always
newSetting is TRUE initially.
Miranda's database is normally protected against corruption by agressively
flushing data to the disk on writes. If you're doing a lot of writes (eg in
an import plugin) it can sometimes be desirable to switch this feature off to
speed up the process. If you do switch it off, you must remember that crashes
are far more likely to be catastrophic, so switch it back on at the earliest
possible opportunity.
Note that if you're doing a lot of setting writes, the flush is already delayed
so you need not use this service for that purpose.
*/
#define MS_DB_SETSAFETYMODE     "DB/SetSafetyMode"

/*************************** Modules ******************************/

/* db/modules/enum   v0.1.0.1+
Enumerates the names of all modules that have stored or requested information
from the database.
wParam = lParam
lParam = (WPARAM)(DBMODULEENUMPROC)dbmep
Returns the value returned by the last call to dbmep
This service is only really useful for debugging, in conjunction with
db/contact/enumsettings
lParam is passed directly to dbmep
dbmep should return 0 to continue enumeration, or nonzero to stop.
ofsModuleName is the offset of the module name from the start of the profile
database, and is only useful for really heavyweight debugging
Modules names will be enumerated in no particular order
Writing to the database while module names are being enumerated will cause
unpredictable results in the enumeration, but the write will work.
szModuleName is only guaranteed to be valid for the duration of the callback.
If you want to keep it for longer you must allocation your own storage.
*/
typedef int (*DBMODULEENUMPROC)(const char *szModuleName, DWORD ofsModuleName, LPARAM lParam);
#define MS_DB_MODULES_ENUM    "DB/Modules/Enum"

/* DB/Module/Delete  0.8.0+

Removes all settings for the specified module.
wParam = (WPARAM)(MCONTACT)hContact or 0 for global settings
lParam = (LPARAM)(char*)szModuleName - the module name to be deleted
*/

#define MS_DB_MODULE_DELETE "DB/Module/Delete"

/******************************************************************/
/************************** EVENTS ********************************/
/******************************************************************/

/* DB/Event/Added event
Called when a new event has been added to the event chain for a contact
  wParam = (MCONTACT)hContact
  lParam = (LPARAM)(HANDLE)hDbEvent
hDbEvent is a valid handle to the event. hContact is a valid handle to the
contact to which hDbEvent refers.
Since events are sorted chronologically, you cannot guarantee that hDbEvent is
at any particular position in the chain.
*/
#define ME_DB_EVENT_ADDED  "DB/Event/Added"

/* DB/Event/FilterAdd (NOTE: Added during 0.3.3+ development!)
Called **before** a new event is made of a DBEVENTINFO structure, this
hook is not SAFE unless you know what you're doing with it, the arguments
are passed as-is (with errors, pointer problems, if any) from any arguments
passed to db_event_add.

The point of this hook is to stop any unwanted database events, to stop
an event being added, return 1, to allow the event to pass through return
0.
  wParam = (MCONTACT)hContact
  lParam = (LPARAM)&DBEVENTINFO

Any changed made to the said DBEVENTINFO are also passed along to the database,
therefore it is possible to shape the data, however DO NOT DO THIS.
*/
#define ME_DB_EVENT_FILTER_ADD "DB/Event/FilterAdd"

/* DB/Event/Marked/Read event
Called when an event is marked read
wParam = (MCONTACT)hContact
lParam = (LPARAM)(HANDLE)hDbEvent
hDbEvent is a valid handle to the event.
hContact is a valid handle to the contact to which hDbEvent refers, and will
remain valid.
*/
#define ME_DB_EVENT_MARKED_READ "DB/Event/Marked/Read"

/* DB/Event/Deleted event
Called when an event is about to be deleted from the event chain for a contact
  wParam = (MCONTACT)hContact
  lParam = (LPARAM)(HANDLE)hDbEvent
hDbEvent is a valid handle to the event which is about to be deleted, but it
won't be once your hook has returned.
hContact is a valid handle to the contact to which hDbEvent refers, and will
remain valid.
Returning nonzero from your hook will not stop the deletion, but it will, as
usual, stop other hooks from being called.
*/
#define ME_DB_EVENT_DELETED  "DB/Event/Deleted"

/* DB/Contact/Added event
Called when a new contact has been added to the database
  wParam = (MCONTACT)hContact
  lParam = 0
hContact is a valid handle to the new contact.
Contacts are initially created without any settings, so if you hook this event
you will almost certainly also want to hook db/contact/settingchanged as well.
*/
#define ME_DB_CONTACT_ADDED  "DB/Contact/Added"

/* DB/Contact/Deleted event
Called when an contact is about to be deleted
  wParam = (MCONTACT)hContact
  lParam = 0
hContact is a valid handle to the contact which is about to be deleted, but it
won't be once your hook has returned.
Returning nonzero from your hook will not stop the deletion, but it will, as
usual, stop other hooks from being called.
Deleting a contact invalidates all events in its chain.
*/
#define ME_DB_CONTACT_DELETED  "DB/Contact/Deleted"

/* DB/Contact/SettingChanged event
Called when a contact has had one of its settings changed
  wParam = (MCONTACT)hContact
  lParam = (LPARAM)(DBCONTACTWRITESETTING*)&dbcws
hContact is a valid handle to the contact that has changed.
This event will be triggered many times rapidly when a whole bunch of values
are set.
Modules which hook this should be aware of this fact and quickly return if they
are not interested in the value that has been changed.
Careful not to get into infinite loops with this event.
The structure dbcws is the same one as is passed to the original service, so
don't change any of the members.
*/
#define ME_DB_CONTACT_SETTINGCHANGED  "DB/Contact/SettingChanged"

/******************************************************************/
/********************* SETTINGS HELPER FUNCTIONS ******************/
/******************************************************************/

#ifndef DB_NOHELPERFUNCTIONS

/* inlined range tolerate versions */

__inline BYTE DBGetContactSettingRangedByte(MCONTACT hContact, const char *szModule, const char *szSetting, BYTE errorValue, BYTE minValue, BYTE maxValue)
{
	BYTE bVal = db_get_b(hContact, szModule, szSetting, errorValue);
	return (bVal < minValue || bVal > maxValue) ? errorValue : bVal;
}

__inline WORD DBGetContactSettingRangedWord(MCONTACT hContact, const char *szModule, const char *szSetting, WORD errorValue, WORD minValue, WORD maxValue)
{
	WORD wVal = db_get_w(hContact, szModule, szSetting, errorValue);
	return (wVal < minValue || wVal > maxValue) ? errorValue : wVal;
}

__inline DWORD DBGetContactSettingRangedDword(MCONTACT hContact, const char *szModule, const char *szSetting, DWORD errorValue, DWORD minValue, DWORD maxValue)
{
	DWORD dwVal = db_get_dw(hContact, szModule, szSetting, errorValue);
	return (dwVal < minValue || dwVal > maxValue) ? errorValue : dwVal;
}

#endif

#endif // M_DATABASE_H__

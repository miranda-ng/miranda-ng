/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

/////////////////////////////////////////////////////////////////////////////////////////
// GENERALLY USEFUL STUFF

#if !defined(M_SYSTEM_H__)
	#include "m_system.h"
#endif

#if !defined(M_UTILS_H__)
	#include "m_utils.h"
#endif

#ifdef _MSC_VER
	#pragma warning(disable:4201 4204)
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// database functions

#if defined(__cplusplus)
extern "C"
{
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// DBVARIANT: used by db/contact/getsetting and db/contact/writesetting

#define DBVT_DELETED  0   // this setting just got deleted, no other values are valid
#define DBVT_BYTE     1	  // bVal and cVal are valid
#define DBVT_WORD     2	  // wVal and sVal are valid
#define DBVT_DWORD    4	  // dVal and lVal are valid
#define DBVT_ASCIIZ 255	  // pszVal is valid
#define DBVT_BLOB   254	  // cpbVal and pbVal are valid
#define DBVT_UTF8   253   // pszVal is valid
#define DBVT_WCHAR  252   // pszVal is valid
#if defined(_UNICODE)
#define DBVT_TCHAR DBVT_WCHAR
#else
#define DBVT_TCHAR DBVT_ASCIIZ
#endif
#define DBVTF_VARIABLELENGTH  0x80

typedef struct
{
	BYTE type;
	union {
		BYTE bVal; char cVal;
		WORD wVal; short sVal;
		DWORD dVal; long lVal;
		struct {
			union {
				char *pszVal;
				TCHAR *ptszVal;
				wchar_t *pwszVal;
			};
			WORD cchVal;   //only used for db/contact/getsettingstatic
		};
		struct {
			WORD cpbVal;
			BYTE *pbVal;
		};
	};
} DBVARIANT;

#define DBEF_SENT       2  // this event was sent by the user. If not set this event was received.
#define DBEF_READ       4  // event has been read by the user. It does not need to be processed any more except for history.
#define DBEF_RTL        8  // event contains the right-to-left aligned text
#define DBEF_UTF       16  // event contains a text in utf-8
#define DBEF_ENCRYPTED 32  // event is encrypted (never reported outside a driver)

typedef struct
{
	int   cbSize;           // size of the structure in bytes
	char *szModule;         // pointer to name of the module that 'owns' this
	// event, ie the one that is in control of the data format
	DWORD timestamp;        // seconds since 00:00, 01/01/1970. Gives us times until
	// 2106 unless you use the standard C library which is
	// signed and can only do until 2038. In GMT.
	DWORD flags;            // the omnipresent flags
	WORD  eventType;        // module-defined event type field
	DWORD cbBlob;           // size of pBlob in bytes
	PBYTE pBlob;            // pointer to buffer containing module-defined event data

#if defined(__cplusplus)
	bool __forceinline markedRead() const
	{
		return (flags & (DBEF_SENT | DBEF_READ)) != 0;
	}
#endif
} DBEVENTINFO;

MIR_CORE_DLL(INT_PTR) db_free(DBVARIANT *dbv);

/////////////////////////////////////////////////////////////////////////////////////////
// Database contacts

/*
Gets the handle of the first contact in the database. This handle can be used
with loads of functions. It does not need to be closed.
You can specify szProto to find only its contacts
Returns a handle to the first contact in the db on success, or NULL if there
are no contacts in the db.
*/

#if defined(__cplusplus)
MIR_CORE_DLL(MCONTACT) db_find_first(const char *szProto = NULL);
#else
MIR_CORE_DLL(MCONTACT) db_find_first(const char *szProto);
#endif

/*
Gets the handle of the next contact after hContact in the database. This handle
can be used with loads of functions. It does not need to be closed.
You can specify szProto to find only its contacts
Returns a handle to the contact after hContact in the db on success or NULL if
hContact was the last contact in the db or hContact was invalid.
*/

#if defined(__cplusplus)
MIR_CORE_DLL(MCONTACT) db_find_next(MCONTACT hContact, const char *szProto = NULL);
#else
MIR_CORE_DLL(MCONTACT) db_find_next(MCONTACT hContact, const char *szProto);
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// Database events

/*
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
*/

#define EVENTTYPE_MESSAGE         0
#define EVENTTYPE_URL             1
#define EVENTTYPE_CONTACTS        2   //v0.1.2.2+
#define EVENTTYPE_ADDED         1000  //v0.1.1.0+: these used to be module-
#define EVENTTYPE_AUTHREQUEST   1001  //specific codes, hence the module-
#define EVENTTYPE_FILE          1002  //specific limit has been raised to 2000

MIR_CORE_DLL(MEVENT) db_event_add(MCONTACT hContact, DBEVENTINFO *dbei);

/*
Gets the number of events in the chain belonging to a contact in the database.
Returns the number of events in the chain owned by hContact or -1 if hContact
is invalid. They can be retrieved using the db_event_first/last() services.
*/

MIR_CORE_DLL(int) db_event_count(MCONTACT hContact);

/*
Removes a single event from the database
hDbEvent should have been returned by db_event_add/first/last/next/prev()
Returns 0 on success, or nonzero if hDbEvent was invalid
Triggers a db/event/deleted event just *before* the event is deleted
*/

MIR_CORE_DLL(int) db_event_delete(MCONTACT hContact, MEVENT hDbEvent);

/*
Retrieves a handle to the first event in the chain for hContact
Returns the handle, or NULL if hContact is invalid or has no events
Events in a chain are sorted chronologically automatically
*/

MIR_CORE_DLL(MEVENT) db_event_first(MCONTACT hContact);

/*
Retrieves a handle to the first unread event in the chain for hContact
Returns the handle, or NULL if hContact is invalid or all its events have been
read

Events in a chain are sorted chronologically automatically, but this does not
necessarily mean that all events after the first unread are unread too. They
should be checked individually with db_event_next() and db_event_get()
This service is designed for startup, reloading all the events that remained
unread from last time
*/

MIR_CORE_DLL(MEVENT) db_event_firstUnread(MCONTACT hContact);

/*
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
*/

MIR_CORE_DLL(int) db_event_get(MEVENT hDbEvent, DBEVENTINFO *dbei);

/*
Retrieves the space in bytes required to store the blob in hDbEvent
hDbEvent should have been returned by db_event_add/first/last/next/prev()
Returns the space required in bytes, or -1 if hDbEvent is invalid
*/

MIR_CORE_DLL(int) db_event_getBlobSize(MEVENT hDbEvent);

/*
Retrieves a handle to the contact that owns hDbEvent.
hDbEvent should have been returned by db_event_add/first/last/next/prev()
NULL is a valid return value, meaning, as usual, the user.
Returns INVALID_CONTACT_ID if hDbEvent is invalid, or the handle to the contact on success
*/

MIR_CORE_DLL(MCONTACT) db_event_getContact(MEVENT hDbEvent);

/*
Retrieves a handle to the last event in the chain for hContact
Returns the handle, or NULL if hContact is invalid or has no events
Events in a chain are sorted chronologically automatically
*/

MIR_CORE_DLL(MEVENT) db_event_last(MCONTACT hContact);

/*
Changes the flags for an event to mark it as read.
hDbEvent should have been returned by db_event_add/first/last/next/prev()
Returns the entire flag DWORD for the event after the change, or -1 if hDbEvent
is invalid.
This is the one database write operation that does not trigger an event.
Modules should not save flags states for any length of time.
*/

MIR_CORE_DLL(int) db_event_markRead(MCONTACT hContact, MEVENT hDbEvent);

/*
Retrieves a handle to the next event in a chain after hDbEvent
Returns the handle, or NULL if hDbEvent is invalid or is the last event
Events in a chain are sorted chronologically automatically
*/

MIR_CORE_DLL(MEVENT) db_event_next(MCONTACT hContact, MEVENT hDbEvent);

/*
Retrieves a handle to the previous event in a chain before hDbEvent
Returns the handle, or NULL if hDbEvent is invalid or is the first event
Events in a chain are sorted chronologically automatically
*/

MIR_CORE_DLL(MEVENT) db_event_prev(MCONTACT hContact, MEVENT hDbEvent);

/////////////////////////////////////////////////////////////////////////////////////////
// Database settings

MIR_CORE_DLL(INT_PTR)  db_get(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv);
MIR_CORE_DLL(int)      db_get_b(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, int errorValue);
MIR_CORE_DLL(int)      db_get_w(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, int errorValue);
MIR_CORE_DLL(DWORD)    db_get_dw(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, DWORD errorValue);
MIR_CORE_DLL(char*)    db_get_sa(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting);
MIR_CORE_DLL(wchar_t*) db_get_wsa(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting);

MIR_CORE_DLL(int)      db_get_static(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, char *pDest, int cbDest);
MIR_CORE_DLL(int)      db_get_static_utf(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, char *pDest, int cbDest);
MIR_CORE_DLL(int)      db_get_wstatic(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, wchar_t *pDest, int cbDest);

#if defined(__cplusplus)
MIR_CORE_DLL(INT_PTR) db_get_s(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, const int nType = DBVT_ASCIIZ);
#else
MIR_CORE_DLL(INT_PTR) db_get_s(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, const int nType);
#endif

MIR_CORE_DLL(INT_PTR)  db_set(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv);
MIR_CORE_DLL(INT_PTR)  db_set_b(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, BYTE val);
MIR_CORE_DLL(INT_PTR)  db_set_w(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, WORD val);
MIR_CORE_DLL(INT_PTR)  db_set_dw(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, DWORD val);
MIR_CORE_DLL(INT_PTR)  db_set_s(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, LPCSTR val);
MIR_CORE_DLL(INT_PTR)  db_set_ws(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, LPCWSTR val);
MIR_CORE_DLL(INT_PTR)  db_set_utf(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, LPCSTR val);
MIR_CORE_DLL(INT_PTR)  db_set_blob(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, void *val, unsigned len);

MIR_CORE_DLL(INT_PTR) db_unset(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting);

#if defined(__cplusplus)
MIR_CORE_DLL(BOOL) db_set_resident(LPCSTR szModule, const char *szService, BOOL bEnable = TRUE);
#else
MIR_CORE_DLL(BOOL) db_set_resident(LPCSTR szModule, const char *szService, BOOL bEnable);
#endif

#define db_get_ws(a,b,c,d)    db_get_s(a,b,c,d,DBVT_WCHAR)
#define db_get_utf(a,b,c,d)   db_get_s(a,b,c,d,DBVT_UTF8)

#ifdef _UNICODE
#define db_get_ts(a,b,c,d) db_get_s(a,b,c,d,DBVT_WCHAR)
#define db_get_tsa         db_get_wsa
#define db_set_ts          db_set_ws
#define db_get_tstatic     db_get_wstatic
#else
#define db_get_ts(a,b,c,d) db_get_s(a,b,c,d,DBVT_ASCIIZ)
#define db_get_tsa         db_get_sa
#define db_set_ts          db_set_s
#define db_get_tstatic     db_get_static
#endif

#if defined(__cplusplus)
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// Profile services

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

/////////////////////////////////////////////////////////////////////////////////////////
// Contact services

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

/////////////////////////////////////////////////////////////////////////////////////////
// Event services

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

/////////////////////////////////////////////////////////////////////////////////////////
// Time services

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

/////////////////////////////////////////////////////////////////////////////////////////
// Random services

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

/////////////////////////////////////////////////////////////////////////////////////////
// Module services

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

/////////////////////////////////////////////////////////////////////////////////////////
// Database events

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

/////////////////////////////////////////////////////////////////////////////////////////
// Settings helper functions

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

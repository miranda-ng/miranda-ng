/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org)
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

// Switches safety settings on or off
// newSetting is TRUE initially.
// Miranda's database is normally protected against corruption by agressively
// flushing data to the disk on writes. If you're doing a lot of writes (eg in
// an import plugin) it can sometimes be desirable to switch this feature off to
// speed up the process. If you do switch it off, you must remember that crashes
// are far more likely to be catastrophic, so switch it back on at the earliest
// possible opportunity.
// Note that if you're doing a lot of setting writes, the flush is already delayed
// so you need not use this service for that purpose.

EXTERN_C MIR_CORE_DLL(void) db_set_safety_mode(BOOL bNewMode);

// Gets the number of contacts in the database, which does not count the user
// Returns the number of contacts. They can be retrieved using contact/findfirst and contact/findnext

EXTERN_C MIR_CORE_DLL(int) db_get_contact_count(void);

// Removes all settings for the specified module.
// hContact is 0 for global settings or matches the concrete contact

EXTERN_C MIR_CORE_DLL(int) db_delete_module(MCONTACT hContact, const char *szModuleName);

/////////////////////////////////////////////////////////////////////////////////////////
// contact functions

// Adds a new contact to the database. New contacts initially have no settings
// whatsoever, they must all be added with db/contacts/writesetting.
// Returns a handle to the newly created contact on success, or NULL otherwise.
// Triggers a db/contact/added event just before it returns.

EXTERN_C MIR_CORE_DLL(MCONTACT) db_add_contact(void);

// Deletes the contact hContact from the database and all events and settings associated with it.
// Returns 0 on success or nonzero if hContact was invalid
// Please don't try to delete the user contact (hContact = NULL)
// Triggers a db/contact/deleted event just *before* it removes anything
// Because all events are deleted, lots of people may end up with invalid event
// handles from this operation, which they should be prepared for.

EXTERN_C MIR_CORE_DLL(int) db_delete_contact(MCONTACT hContact);

// Checks if a given value is a valid contact handle, note that due
// to the nature of multiple threading, a valid contact can still become
// invalid after a call to this service.
// Returns 1 if the contact is a contact, or 0 if the contact is not valid.

EXTERN_C MIR_CORE_DLL(int) db_is_contact(MCONTACT hContact);

/////////////////////////////////////////////////////////////////////////////////////////
// enumerators

// Enumerates the names of all modules that have stored or requested information from the database.
// Returns the value returned by the last call to dbmep
// This service is only really useful for debugging, in conjunction with db/contact/enumsettings
// dbmep should return 0 to continue enumeration, or nonzero to stop.
// ofsModuleName is the offset of the module name from the start of the profile
// database, and is only useful for really heavyweight debugging
// Modules names will be enumerated in no particular order
// Writing to the database while module names are being enumerated will cause
// unpredictable results in the enumeration, but the write will work.
// szModuleName is only guaranteed to be valid for the duration of the callback.
// If you want to keep it for longer you must allocation your own storage.

typedef int(*DBMODULEENUMPROC)(const char *szModuleName, DWORD ofsModuleName, LPARAM lParam);

EXTERN_C MIR_CORE_DLL(int) db_enum_modules(DBMODULEENUMPROC dbmep, const void *param = NULL);

// Lists all resident settings

EXTERN_C MIR_CORE_DLL(int) db_enum_residents(DBMODULEENUMPROC pFunc, const void *param = NULL);

// Lists all the settings a specific modules has stored in the database for a specific contact.
// Returns the return value of the last call to pfnEnumProc, or -1 if there are
// no settings for that module/contact pair
// Writing to or deleting from the database while enumerating will have
// unpredictable results for the enumeration, but the write will succeed.
// Use db/modules/enum to get a complete list of module names
// szSetting is only guaranteed to be valid for the duration of the callback. If
// you want to keep it for longer you must allocation your own storage.

typedef int (*DBSETTINGENUMPROC)(const char *szSetting, LPARAM lParam);

EXTERN_C MIR_CORE_DLL(int) db_enum_settings(MCONTACT hContact, DBSETTINGENUMPROC pfnEnumProc, const char *szModule, const void *param = NULL);

/////////////////////////////////////////////////////////////////////////////////////////
// DBVARIANT: used by db/contact/getsetting and db/contact/writesetting

#define DBVT_DELETED  0   // this setting just got deleted, no other values are valid
#define DBVT_BYTE     1	  // bVal and cVal are valid
#define DBVT_WORD     2	  // wVal and sVal are valid
#define DBVT_DWORD    4	  // dVal and lVal are valid
#define DBVT_ASCIIZ 255	  // pszVal is valid
#define DBVT_BLOB   254	  // cpbVal and pbVal are valid
#define DBVT_UTF8   253   // pszVal is valid
#define DBVT_WCHAR  252   // pwszVal is valid

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
				wchar_t *ptszVal;
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
	char *szModule;         // pointer to name of the module that 'owns' this event
	DWORD timestamp;        // seconds since 00:00, 01/01/1970. Gives us times until 2106 
	                        // unless you use the standard C library which is
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

	wchar_t* getString(const char *str) const
	{
		return (flags & DBEF_UTF) ? Utf8DecodeW(str) : mir_a2u(str);
	}

#endif
} DBEVENTINFO;

EXTERN_C MIR_CORE_DLL(INT_PTR) db_free(DBVARIANT *dbv);

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
EXTERN_C MIR_CORE_DLL(MCONTACT) db_find_first(const char *szProto = NULL);
#else
EXTERN_C MIR_CORE_DLL(MCONTACT) db_find_first(const char *szProto);
#endif

/*
Gets the handle of the next contact after hContact in the database. This handle
can be used with loads of functions. It does not need to be closed.
You can specify szProto to find only its contacts
Returns a handle to the contact after hContact in the db on success or NULL if
hContact was the last contact in the db or hContact was invalid.
*/

#if defined(__cplusplus)
EXTERN_C MIR_CORE_DLL(MCONTACT) db_find_next(MCONTACT hContact, const char *szProto = NULL);
#else
EXTERN_C MIR_CORE_DLL(MCONTACT) db_find_next(MCONTACT hContact, const char *szProto);
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

EXTERN_C MIR_CORE_DLL(MEVENT) db_event_add(MCONTACT hContact, DBEVENTINFO *dbei);

/*
Gets the number of events in the chain belonging to a contact in the database.
Returns the number of events in the chain owned by hContact or -1 if hContact
is invalid. They can be retrieved using the db_event_first/last() services.
*/

EXTERN_C MIR_CORE_DLL(int) db_event_count(MCONTACT hContact);

/*
Removes a single event from the database
hDbEvent should have been returned by db_event_add/first/last/next/prev()
Returns 0 on success, or nonzero if hDbEvent was invalid
Triggers a db/event/deleted event just *before* the event is deleted
*/

EXTERN_C MIR_CORE_DLL(int) db_event_delete(MCONTACT hContact, MEVENT hDbEvent);

/*
Retrieves a handle to the first event in the chain for hContact
Returns the handle, or NULL if hContact is invalid or has no events
Events in a chain are sorted chronologically automatically
*/

EXTERN_C MIR_CORE_DLL(MEVENT) db_event_first(MCONTACT hContact);

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

EXTERN_C MIR_CORE_DLL(MEVENT) db_event_firstUnread(MCONTACT hContact);

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

EXTERN_C MIR_CORE_DLL(int) db_event_get(MEVENT hDbEvent, DBEVENTINFO *dbei);

/*
Retrieves the space in bytes required to store the blob in hDbEvent
hDbEvent should have been returned by db_event_add/first/last/next/prev()
Returns the space required in bytes, or -1 if hDbEvent is invalid
*/

EXTERN_C MIR_CORE_DLL(int) db_event_getBlobSize(MEVENT hDbEvent);

/*
Retrieves a handle to the contact that owns hDbEvent.
hDbEvent should have been returned by db_event_add/first/last/next/prev()
NULL is a valid return value, meaning, as usual, the user.
Returns INVALID_CONTACT_ID if hDbEvent is invalid, or the handle to the contact on success
*/

EXTERN_C MIR_CORE_DLL(MCONTACT) db_event_getContact(MEVENT hDbEvent);

/*
Retrieves a handle to the last event in the chain for hContact
Returns the handle, or NULL if hContact is invalid or has no events
Events in a chain are sorted chronologically automatically
*/

EXTERN_C MIR_CORE_DLL(MEVENT) db_event_last(MCONTACT hContact);

/*
Changes the flags for an event to mark it as read.
hDbEvent should have been returned by db_event_add/first/last/next/prev()
Returns the entire flag DWORD for the event after the change, or -1 if hDbEvent
is invalid.
This is the one database write operation that does not trigger an event.
Modules should not save flags states for any length of time.
*/

EXTERN_C MIR_CORE_DLL(int) db_event_markRead(MCONTACT hContact, MEVENT hDbEvent);

/*
Retrieves a handle to the next event in a chain after hDbEvent
Returns the handle, or NULL if hDbEvent is invalid or is the last event
Events in a chain are sorted chronologically automatically
*/

EXTERN_C MIR_CORE_DLL(MEVENT) db_event_next(MCONTACT hContact, MEVENT hDbEvent);

/*
Retrieves a handle to the previous event in a chain before hDbEvent
Returns the handle, or NULL if hDbEvent is invalid or is the first event
Events in a chain are sorted chronologically automatically
*/

EXTERN_C MIR_CORE_DLL(MEVENT) db_event_prev(MCONTACT hContact, MEVENT hDbEvent);

/////////////////////////////////////////////////////////////////////////////////////////
// Database settings

EXTERN_C MIR_CORE_DLL(INT_PTR)  db_get(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv);
EXTERN_C MIR_CORE_DLL(int)      db_get_b(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, int errorValue = 0);
EXTERN_C MIR_CORE_DLL(int)      db_get_w(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, int errorValue = 0);
EXTERN_C MIR_CORE_DLL(DWORD)    db_get_dw(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, DWORD errorValue = 0);
EXTERN_C MIR_CORE_DLL(char*)    db_get_sa(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting);
EXTERN_C MIR_CORE_DLL(wchar_t*) db_get_wsa(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting);

EXTERN_C MIR_CORE_DLL(int)      db_get_static(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, char *pDest, int cbDest);
EXTERN_C MIR_CORE_DLL(int)      db_get_static_utf(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, char *pDest, int cbDest);
EXTERN_C MIR_CORE_DLL(int)      db_get_wstatic(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, wchar_t *pDest, int cbDest);

#if defined(__cplusplus)
EXTERN_C MIR_CORE_DLL(INT_PTR) db_get_s(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, const int nType = DBVT_ASCIIZ);
#else
EXTERN_C MIR_CORE_DLL(INT_PTR) db_get_s(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, const int nType);
#endif

EXTERN_C MIR_CORE_DLL(INT_PTR)  db_set(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv);
EXTERN_C MIR_CORE_DLL(INT_PTR)  db_set_b(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, BYTE val);
EXTERN_C MIR_CORE_DLL(INT_PTR)  db_set_w(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, WORD val);
EXTERN_C MIR_CORE_DLL(INT_PTR)  db_set_dw(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, DWORD val);
EXTERN_C MIR_CORE_DLL(INT_PTR)  db_set_s(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, LPCSTR val);
EXTERN_C MIR_CORE_DLL(INT_PTR)  db_set_ws(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, LPCWSTR val);
EXTERN_C MIR_CORE_DLL(INT_PTR)  db_set_utf(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, LPCSTR val);
EXTERN_C MIR_CORE_DLL(INT_PTR)  db_set_blob(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, void *val, unsigned len);

EXTERN_C MIR_CORE_DLL(INT_PTR) db_unset(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting);

#if defined(__cplusplus)
EXTERN_C MIR_CORE_DLL(BOOL) db_set_resident(LPCSTR szModule, const char *szService, BOOL bEnable = TRUE);
#else
EXTERN_C MIR_CORE_DLL(BOOL) db_set_resident(LPCSTR szModule, const char *szService, BOOL bEnable);
#endif

#define db_get_ws(a,b,c,d)    db_get_s(a,b,c,d,DBVT_WCHAR)
#define db_get_utf(a,b,c,d)   db_get_s(a,b,c,d,DBVT_UTF8)

/////////////////////////////////////////////////////////////////////////////////////////
// Profile services

// Gets the name of the profile currently being used by the database module. 
// This is the same as the filename of the database
// Returns 0 on success or nonzero otherwise

EXTERN_C MIR_APP_DLL(int) Profile_GetNameA(size_t cbLen, char *pszDest);
EXTERN_C MIR_APP_DLL(int) Profile_GetNameW(size_t cbLen, wchar_t *pwszDest);

// Get the path of the base folder where Miranda will store all individual profiles
// The returned path does NOT include a trailing backslash.
// Essentially this is what has been set in mirandaboot.ini as ProfileDir.
// For more options to retrieve profile paths check MS_UTILS_REPLACEVARS
// Returns 0 on success or nonzero otherwise

EXTERN_C MIR_APP_DLL(int) Profile_GetPathA(size_t cbLen, char *pszDest);
EXTERN_C MIR_APP_DLL(int) Profile_GetPathW(size_t cbLen, wchar_t *pwszDest);

// Sets the default profile name programmatically
// Analog of Database/DefaultProfile in mirandaboot.ini

EXTERN_C MIR_APP_DLL(void) Profile_SetDefault(const wchar_t *pwszPath);

// Checks the specified profile like dbtool did.
// Implemented in the dbchecker plugins, thus it might not exist
//  wParam = (WPARAM)(wchar_t*)ptszProfileName
//  lParam = (BOOL)bConversionMode

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

/////////////////////////////////////////////////////////////////////////////////////////
// Event services

// Registers the specified database event type, with module, id & description.
// When someone needs to retrieve an event's text, a service named Module/GetEventText<id>
// will be called. For example, for module named 'foo' and event id 2000 a service
// foo/GetEventText2000 should be defined to process this request. That handler should
// decode a blob and return the event text in the required format, its prototype is identical
// to a call of DbEvent_GetText (see below)
//
// Returns -1 on error (e.g., event type already registred), 0 on success

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

EXTERN_C MIR_APP_DLL(int) DbEvent_RegisterType(DBEVENTTYPEDESCR*);

/////////////////////////////////////////////////////////////////////////////////////////
// Retrieves the previously registered database event type, by module & id.
// Returns DBEVENTTYPEDESCR* or NULL, if an event isn't found.

EXTERN_C MIR_APP_DLL(DBEVENTTYPEDESCR*) DbEvent_GetType(const char *szModule, int eventType);

/////////////////////////////////////////////////////////////////////////////////////////
// macro to extract MCONTACT from the auth blob

__forceinline MCONTACT DbGetAuthEventContact(DBEVENTINFO *dbei)
{
	return (MCONTACT)(*(DWORD*)&dbei->pBlob[sizeof(DWORD)]);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Retrieves the event's text
// * dbei should be the valid database event read via db_event_get()
// * codepage is any valid codepage, CP_ACP by default.
//
// Function returns a pointer to a string in the required format.
// This string should be freed by a call of mir_free

EXTERN_C MIR_APP_DLL(char*) DbEvent_GetTextA(DBEVENTINFO *dbei, int codepage);
EXTERN_C MIR_APP_DLL(wchar_t*) DbEvent_GetTextW(DBEVENTINFO *dbei, int codepage);

/////////////////////////////////////////////////////////////////////////////////////////
// Retrieves the event's icon
// * use LR_SHARED in flags for shared HICON
// dbei should be a valid database event read via db_event_get()
//
// Function returns HICON (use DestroyIcon to release resources if not LR_SHARED)
// 
// A plugin can register the standard event icon in IcoLib named
// 'eventicon_'+Module+EvtID, like eventicon_ICQ2001. Otherwise, to declare an icon
// with the non-standard name, you can declare the special service, Module/GetEventIcon<id>,
// which will retrieve the custom icon handle (HICON). This service function has the
// same parameters MS_DB_EVENT_GETICON does.

EXTERN_C MIR_APP_DLL(HICON) DbEvent_GetIcon(DBEVENTINFO *dbei, int flags);

/////////////////////////////////////////////////////////////////////////////////////////
// Converts the event's string to wchar_t* depending on the event's format
// returns wchar_t* - the converted string
// Caller must free the result using mir_free

EXTERN_C MIR_APP_DLL(wchar_t*) DbEvent_GetString(DBEVENTINFO *dbei, const char *str);

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

/////////////////////////////////////////////////////////////////////////////////////////
// Helper to process the auth req body

/* blob is: 0(DWORD), hContact(DWORD), nick(UTF8), firstName(UTF8), lastName(UTF8), email(UTF8), reason(UTF8) */

#pragma warning(disable : 4251)

class MIR_APP_EXPORT DB_AUTH_BLOB
{
	MCONTACT m_hContact;
	DWORD m_dwUin;
	ptrA m_szNick, m_szFirstName, m_szLastName, m_szEmail, m_szReason;
	DWORD m_size;

	PBYTE makeBlob();

public:
	explicit DB_AUTH_BLOB(MCONTACT hContact, LPCSTR nick, LPCSTR fname, LPCSTR lname, LPCSTR id, LPCSTR reason);
	explicit DB_AUTH_BLOB(PBYTE blob);
	~DB_AUTH_BLOB();

	__forceinline operator char*() { return (char*)makeBlob(); }
	__forceinline operator BYTE*() { return makeBlob(); }

	__forceinline DWORD size() const  { return m_size; }

	__forceinline MCONTACT    get_contact()   const { return m_hContact;    }
	__forceinline const char* get_nick()      const { return m_szNick;      }
	__forceinline const char* get_firstName() const { return m_szFirstName; }
	__forceinline const char* get_lastName()  const { return m_szLastName;  }
	__forceinline const char* get_email()     const { return m_szEmail;     }
	__forceinline const char* get_reason()    const { return m_szReason;    }
	
	__forceinline DWORD get_uin() const { return m_dwUin; }
	__forceinline void set_uin(DWORD dwValue) { m_dwUin = dwValue; }
};

#endif // M_DATABASE_H__

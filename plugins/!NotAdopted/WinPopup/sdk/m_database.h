/*

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

#include <tchar.h>

#if !defined(M_SYSTEM_H__)
	#include "m_system.h"
#endif

#if !defined(M_UTILS_H__)
	#include "m_utils.h"
#endif

#ifdef _MSC_VER
	#pragma warning(disable:4201 4204)
#endif

//DBVARIANT: used by db/contact/getsetting and db/contact/writesetting
#define DBVT_DELETED 0    //this setting just got deleted, no other values are valid
#define DBVT_BYTE   1	  //bVal and cVal are valid
#define DBVT_WORD   2	  //wVal and sVal are valid
#define DBVT_DWORD  4	  //dVal and lVal are valid
#define DBVT_ASCIIZ 255	  //pszVal is valid
#define DBVT_BLOB   254	  //cpbVal and pbVal are valid
#define DBVT_UTF8   253   //pszVal is valid
#define DBVT_WCHAR  252   //pszVal is valid
#if defined( _UNICODE )
  #define DBVT_TCHAR DBVT_WCHAR
#else
  #define DBVT_TCHAR DBVT_ASCIIZ
#endif
#define DBVTF_VARIABLELENGTH  0x80
#define DBVTF_DENYUNICODE     0x10000
typedef struct {
	BYTE type;
	union {
		BYTE bVal; char cVal;
		WORD wVal; short sVal;
		DWORD dVal; long lVal;
		struct {
			union {
				char *pszVal;
				TCHAR *ptszVal;
				WCHAR *pwszVal;
			};
			WORD cchVal;   //only used for db/contact/getsettingstatic
		};
		struct {
			WORD cpbVal;
			BYTE *pbVal;
		};
	};
} DBVARIANT;

/******************************************************************/
/************************* SERVICES *******************************/
/******************************************************************/

/* DB/Contact/GetProfileName service
Gets the name of the profile currently being used by the database module. This
is the same as the filename of the database
  wParam=(WPARAM)(UINT)cbName
  lParam=(LPARAM)(char*)pszName
pszName is a pointer to the buffer that receives the name of the profile
cbName is the size in bytes of the pszName buffer
Returns 0 on success or nonzero otherwise
*/
#define MS_DB_GETPROFILENAME  "DB/GetProfileName"

/* DB/Contact/GetProfilePath service
Gets the path of the profile currently being used by the database module. This
path does not include the last '\'.
  wParam=(WPARAM)(UINT)cbName
  lParam=(LPARAM)(char*)pszName
pszName is a pointer to the buffer that receives the path of the profile
cbName is the size in bytes of the pszName buffer
Returns 0 on success or nonzero otherwise
*/
#define MS_DB_GETPROFILEPATH  "DB/GetProfilePath"

/************************* Contact ********************************/

/* DB/Contact/GetSetting service
Look up the value of a named setting for a specific contact in the database
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(DBCONTACTGETSETTING*)&dbcgs
hContact should have been returned by find*contact or addcontact
Caller is responsible for free()ing dbcgs.pValue->pszVal and pbVal if they are
returned. This should be done with db/contact/freevariant if you have your own
heap (like DLLs do).
Note that DBCONTACTGETSETTING takes a pointer to a DBVARIANT, whereas
DBCONTACTWRITESETTING contains a DBVARIANT.
Returns 0 on success or nonzero if the setting name was not found or hContact
was invalid
Because this is such a common function there are some short helper function at
the bottom of this header that use it.

(Added during 0.3.3+ development!!)

If a setting is queried under for contact and it is deleted it will
not be returned as a successful attempt, prior to 0.3.3 a *deleted*
setting would be successfully read (which was a bug because the pValue
was often garbage and maybe not even NULL terminated)

To test for existing but 'deleted' settings, the return value will
be 2, and pValue->type==DBVT_DELETED, at this point pValue is undefined.
*/
typedef struct {
	const char *szModule;	// pointer to name of the module that wrote the
	                        // setting to get
	const char *szSetting;	// pointer to name of the setting to get
	DBVARIANT *pValue;		// pointer to variant to receive the value
} DBCONTACTGETSETTING;
#define MS_DB_CONTACT_GETSETTING  "DB/Contact/GetSetting"

/* DB/Contact/GetSettingString service  0.4.3+
Same as DB/Contact/GetSetting, but also gets the required string type inside
the dbcgs->type parameter
*/
#define MS_DB_CONTACT_GETSETTING_STR  "DB/Contact/GetSettingStr"

/* DB/Contact/GetSettingStatic service
Look up the value of a named setting for a specific contact in the database
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(DBCONTACTGETSETTING*)&dbcgs
hContact should have been returned by find*contact or addcontact
This service differs from db/contact/getsetting in that it won't malloc()
memory for the return value if it needs to do so. This introduces some extra
constraints:
Upon calling dbcgs.pValue->type should be initialised to the expected type of
the setting. If the setting is of an integral type it won't matter if it's
wrong and the service will correct it before returning, however if the setting
is a string or a blob the service needs to know where to put the data and will
fail if type is set wrongly.
If dbcgs.pValue->type is DBVT_ASCIIZ or DBVT_BLOB upon calling, the
corresponding data field (pszVal or pbVal) must point to a buffer allocated by
the caller and the length field (cchVal or cpbVal) must contain the size of
that buffer in bytes.
If the setting type is variable length (DBVT_ASCIIZ or DBVT_BLOB), on exit the
length field (cchVal or cpbVal) will be filled with the full length of the
setting's value (excluding the terminating nul if it's DBVT_ASCIIZ).
This service exists as well as db/contact/getsetting because malloc()/free()
can be too slow for frequently queried settings.
Returns 0 on success or nonzero if the setting name was not found or hContact
was invalid.
*/
#define MS_DB_CONTACT_GETSETTINGSTATIC  "DB/Contact/GetSettingStatic"

/* DB/Contact/FreeVariant service
Free the memory in a DBVARIANT that is allocated by a call to
db/contact/getsetting
  wParam=0
  lParam=(LPARAM)(DBVARIANT*)&dbv
Returns 0 on success, nonzero otherwise
This service is actually just a wrapper around a call to free() and a test to
check that it is a string or a blob in the variant. It exists because DLLs have
their own heap and cannot free the memory allocated in db/contact/getsetting.
Thus it need not be called if you know the variant contains some form of int,
and you will often see free() used instead in code written before I noticed
this problem.
Good style, of course, dictates that it should be present to match all calls to
db/contact/getsetting, but that's not going to happen of course.
There's a helper function for this at the bottom of this header too.
*/
#define MS_DB_CONTACT_FREEVARIANT  "DB/Contact/FreeVariant"

/* DB/Contact/WriteSetting service
Change the value of, or create a new value with, a named setting for a specific
contact in the database to the given value
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(DBCONTACTWRITESETTING*)&dbcws
hContact should have been returned by find*contact or addcontact
Returns 0 on success or nonzero if hContact was invalid
Note that DBCONTACTGETSETTING takes a pointer to a DBVARIANT, whereas
DBCONTACTWRITESETTING contains a DBVARIANT.
Because this is such a common function there are some short helper function at
the bottom of this header that use it.
Triggers a db/contact/settingchanged event just before it returns.
*/
typedef struct {
	const char *szModule;	// pointer to name of the module that wrote the
	                        // setting to get
	const char *szSetting;	// pointer to name of the setting to get
	DBVARIANT value;		// variant containing the value to set
} DBCONTACTWRITESETTING;
#define MS_DB_CONTACT_WRITESETTING  "DB/Contact/WriteSetting"

/* DB/Contact/DeleteSetting service
Removes a named setting for a specific contact from the database
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(DBCONTACTGETSETTING*)&dbcgs
hContact should have been returned by find*contact or addcontact
pValue from dbcgs is not used.
Returns 0 on success or nonzero if the setting was not present or hContact was
invalid
Triggers a db/contact/settingchanged event before it deletes the setting. The
'new value' of the setting is set to type=0 and all the other fields are
undefined.
*/
#define MS_DB_CONTACT_DELETESETTING  "DB/Contact/DeleteSetting"

/* db/contact/enumsettings    v0.1.0.1+
Lists all the settings a specific modules has stored in the database for a
specific contact.
wParam=(WPARAM)(HANDLE)hContact
lParam=(LPARAM)(DBCONTACTENUMSETTINGS*)&dbces
Returns the return value of the last call to pfnEnumProc, or -1 if there are
no settings for that module/contact pair
Writing to or deleting from the database while enumerating will have
unpredictable results for the enumeration, but the write will succeed.
Use db/modules/enum to get a complete list of module names
szSetting is only guaranteed to be valid for the duration of the callback. If
you want to keep it for longer you must allocation your own storage.
*/
typedef int (*DBSETTINGENUMPROC)(const char *szSetting,LPARAM lParam);
typedef struct {
	DBSETTINGENUMPROC pfnEnumProc;
	LPARAM lParam;    //passed direct to pfnEnumProc
	const char *szModule;    //name of the module to get settings for
	DWORD ofsSettings;   //filled by the function to contain the offset from
	       //the start of the database of the requested settings group.
} DBCONTACTENUMSETTINGS;
#define MS_DB_CONTACT_ENUMSETTINGS   "DB/Contact/EnumSettings"

/* DB/Contact/GetCount service
Gets the number of contacts in the database, which does not count the user
  wParam=lParam=0
Returns the number of contacts. They can be retrieved using contact/findfirst
and contact/findnext
*/
#define MS_DB_CONTACT_GETCOUNT  "DB/Contact/GetCount"

/* DB/Contact/FindFirst service
Gets the handle of the first contact in the database. This handle can be used
with loads of functions. It does not need to be closed.
  wParam=lParam=0
Returns a handle to the first contact in the db on success, or NULL if there
are no contacts in the db.
*/
#define MS_DB_CONTACT_FINDFIRST  "DB/Contact/FindFirst"

/* DB/Contact/FindNext service
Gets the handle of the next contact after hContact in the database. This handle
can be used with loads of functions. It does not need to be closed.
  wParam=(WPARAM)(HANDLE)hContact
  lParam=0
Returns a handle to the contact after hContact in the db on success or NULL if
hContact was the last contact in the db or hContact was invalid.
*/
#define MS_DB_CONTACT_FINDNEXT  "DB/Contact/FindNext"

/* DB/Contact/Delete
Deletes the contact hContact from the database and all events and settings
associated with it.
  wParam=(WPARAM)(HANDLE)hContact
  lParam=0
Returns 0 on success or nonzero if hContact was invalid
Please don't try to delete the user contact (hContact=NULL)
Triggers a db/contact/deleted event just *before* it removes anything
Because all events are deleted, lots of people may end up with invalid event
handles from this operation, which they should be prepared for.
*/
#define MS_DB_CONTACT_DELETE  "DB/Contact/Delete"

/* DB/Contact/Add
Adds a new contact to the database. New contacts initially have no settings
whatsoever, they must all be added with db/contacts/writesetting.
  wParam=lParam=0
Returns a handle to the newly created contact on success, or NULL otherwise.
Triggers a db/contact/added event just before it returns.
*/
#define MS_DB_CONTACT_ADD  "DB/Contact/Add"

/* DB/Contact/Is
Checks if a given value is a valid contact handle, note that due
to the nature of multiple threading, a valid contact can still become
invalid after a call to this service.
	wParam=(WPARAM)hContact
	lParam=0
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
  wParam=0
  lParam=(LPARAM)(DBEVENTTYPEDESCR*)
Always returns 0.
*/

#define DBEVENTTYPEDESCR_SIZE    sizeof(DBEVENTTYPEDESCR)
#define DBEVENTTYPEDESCR_SIZE_V1 0x10

typedef struct
{
	int   cbSize;      // structure size in bytes
	char* module;      // event module name
	int   eventType;   // event id, unique for this module
	char* descr;       // event type description (i.e. "File Transfer")
	char* textService; // service name for MS_DB_EVENT_GETTEXT (0.8+, default Module+'/GetEventText'+EvtID)
	char* iconService; // service name for MS_DB_EVENT_GETICON (0.8+, default Module+'/GetEventIcon'+EvtID)
	HANDLE eventIcon;  // icolib handle to eventicon (0.8+, default 'eventicon_'+Module+EvtID)
	DWORD flags;       // flags, combination of the DETF_*
}
	DBEVENTTYPEDESCR;

// constants for default event behaviour
#define DETF_HISTORY    1   // show event in history
#define DETF_MSGWINDOW  2   // show event in message window
#define DETF_NONOTIFY   4   // block event notify (e.g. Popups)


#define MS_DB_EVENT_REGISTERTYPE  "DB/EventType/Register"

/* DB/EventType/Get service (0.7+)
Retrieves the previously registered database event type, by module & id.
  wParam=(WPARAM)(char*)szModule
  lParam=(LPARAM)(int)eventType
Returns DBEVENTTYPEDESCR* or NULL, if an event isn't found.
*/

#define MS_DB_EVENT_GETTYPE "DB/EventType/Get"

/* DB/Event/GetCount service
Gets the number of events in the chain belonging to a contact in the database.
  wParam=(WPARAM)(HANDLE)hContact
  lParam=0
Returns the number of events in the chain owned by hContact or -1 if hContact
is invalid. They can be retrieved using the event/find* services.
*/
#define MS_DB_EVENT_GETCOUNT  "DB/Event/GetCount"

/* DB/Event/Add
Adds a new event to a contact's event list
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(DBEVENTINFO*)&dbe
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
#define DBEF_FIRST    1    //this is the first event in the chain;
                           //internal only: *do not* use this flag
#define DBEF_SENT     2    //this event was sent by the user. If not set this
                           //event was received.
#define DBEF_READ     4    //event has been read by the user. It does not need
                           //to be processed any more except for history.
#define DBEF_RTL      8    //event contains the right-to-left aligned text
#define DBEF_UTF     16    //event contains a text in utf-8

typedef struct {
	int cbSize;       //size of the structure in bytes
	char *szModule;	  //pointer to name of the module that 'owns' this
                      //event, ie the one that is in control of the data format
	DWORD timestamp;  //seconds since 00:00, 01/01/1970. Gives us times until
	                  //2106 unless you use the standard C library which is
					  //signed and can only do until 2038. In GMT.
	DWORD flags;	  //the omnipresent flags
	WORD eventType;	  //module-defined event type field
	DWORD cbBlob;	  //size of pBlob in bytes
	PBYTE pBlob;	  //pointer to buffer containing module-defined event data
} DBEVENTINFO;
#define EVENTTYPE_MESSAGE   0
#define EVENTTYPE_URL       1
#define EVENTTYPE_CONTACTS  2	//v0.1.2.2+
#define EVENTTYPE_ADDED         1000  //v0.1.1.0+: these used to be module-
#define EVENTTYPE_AUTHREQUEST   1001  //specific codes, hence the module-
#define EVENTTYPE_FILE          1002  //specific limit has been raised to 2000
#define MS_DB_EVENT_ADD  "DB/Event/Add"

/* DB/Event/Delete
Removes a single event from the database
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(HANDLE)hDbEvent
hDbEvent should have been returned by db/event/add or db/event/find*event
Returns 0 on success, or nonzero if hDbEvent was invalid
Triggers a db/event/deleted event just *before* the event is deleted
*/
#define MS_DB_EVENT_DELETE  "DB/Event/Delete"

/* DB/Event/GetBlobSize
Retrieves the space in bytes required to store the blob in hDbEvent
  wParam=(WPARAM)(HANDLE)hDbEvent
  lParam=0
hDbEvent should have been returned by db/event/add or db/event/find*event
Returns the space required in bytes, or -1 if hDbEvent is invalid
*/
#define MS_DB_EVENT_GETBLOBSIZE  "DB/Event/GetBlobSize"

/* DB/Event/Get
Retrieves all the information stored in hDbEvent
  wParam=(WPARAM)(HANDLE)hDbEvent
  lParam=(LPARAM)(DBEVENTINFO*)&dbe
hDbEvent should have been returned by db/event/add or db/event/find*event
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
#define MS_DB_EVENT_GET  "DB/Event/Get"

/* DB/Event/GetText (0.7.0+)
Retrieves the event's text
  wParam=(WPARAM)0 (unused)
  lParam=(LPARAM)(DBEVENTGETTEXT*)egt - pointer to structure with parameters
  egt->dbei should be the valid database event read via MS_DB_EVENT_GET
  egt->datatype = DBVT_WCHAR or DBVT_ASCIIZ or DBVT_TCHAR. If a caller wants to
suppress Unicode part of event in answer, add DBVTF_DENYUNICODE to this field.
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

__inline static char* DbGetEventTextA( DBEVENTINFO* dbei, int codepage )
{  DBEVENTGETTEXT temp = { dbei, DBVT_ASCIIZ, codepage };
   return (char*)CallService(MS_DB_EVENT_GETTEXT,0,(LPARAM)&temp);
}

__inline static WCHAR* DbGetEventTextW( DBEVENTINFO* dbei, int codepage )
{  DBEVENTGETTEXT temp = { dbei, DBVT_WCHAR, codepage };
   return (WCHAR*)CallService(MS_DB_EVENT_GETTEXT,0,(LPARAM)&temp);
}

__inline static TCHAR* DbGetEventTextT( DBEVENTINFO* dbei, int codepage )
{  DBEVENTGETTEXT temp = { dbei, DBVT_TCHAR, codepage };
   return (TCHAR*)CallService(MS_DB_EVENT_GETTEXT,0,(LPARAM)&temp);
}

/* DB/Event/GetIcon (0.7.0.1+)
Retrieves the event's icon
  wParam=(WPARAM)(int)flags - use LR_SHARED for shared HICON
  lParam=(LPARAM)(DBEVENTINFO*)dbei
dbei should be a valid database event read via MS_DB_EVENT_GET

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
  wParam=(LPARAM)(DBEVENTINFO*)dbei
  lParam=(WPARAM)(char*)str - string to be converted
  returns TCHAR* - the converted string
Caller must free the result using mir_free
*/

#define MS_DB_EVENT_GETSTRINGT "DB/Event/GetStringT"

__inline static TCHAR* DbGetEventStringT( DBEVENTINFO* dbei, const char* str )
{  
   return (TCHAR*)CallService( MS_DB_EVENT_GETSTRINGT, (WPARAM)dbei, (LPARAM)str );
}

/* DB/Event/MarkRead
Changes the flags for an event to mark it as read.
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(HANDLE)hDbEvent
hDbEvent should have been returned by db/event/add or db/event/find*event
Returns the entire flag DWORD for the event after the change, or -1 if hDbEvent
is invalid.
This is the one database write operation that does not trigger an event.
Modules should not save flags states for any length of time.
*/
#define MS_DB_EVENT_MARKREAD  "DB/Event/MarkRead"

/* DB/Event/GetContact
Retrieves a handle to the contact that owns hDbEvent.
  wParam=(WPARAM)(HANDLE)hDbEvent
  lParam=0
hDbEvent should have been returned by db/event/add or db/event/find*event
NULL is a valid return value, meaning, as usual, the user.
Returns (HANDLE)(-1) if hDbEvent is invalid, or the handle to the contact on
success
This service is exceptionally slow. Use only when you have no other choice at
all.
*/
#define MS_DB_EVENT_GETCONTACT  "DB/Event/GetContact"

/* DB/Event/FindFirst
Retrieves a handle to the first event in the chain for hContact
  wParam=(WPARAM)(HANDLE)hContact
  lParam=0
Returns the handle, or NULL if hContact is invalid or has no events
Events in a chain are sorted chronologically automatically
*/
#define MS_DB_EVENT_FINDFIRST  "DB/Event/FindFirst"

/* DB/Event/FindFirstUnread
Retrieves a handle to the first unread event in the chain for hContact
  wParam=(WPARAM)(HANDLE)hContact
  lParam=0
Returns the handle, or NULL if hContact is invalid or all its events have been
read
Events in a chain are sorted chronologically automatically, but this does not
necessarily mean that all events after the first unread are unread too. They
should be checked individually with event/findnext and event/get
This service is designed for startup, reloading all the events that remained
unread from last time
*/
#define MS_DB_EVENT_FINDFIRSTUNREAD  "DB/Event/FindFirstUnread"

/* DB/Event/FindLast
Retrieves a handle to the last event in the chain for hContact
  wParam=(WPARAM)(HANDLE)hContact
  lParam=0
Returns the handle, or NULL if hContact is invalid or has no events
Events in a chain are sorted chronologically automatically
*/
#define MS_DB_EVENT_FINDLAST  "DB/Event/FindLast"

/* DB/Event/FindNext
Retrieves a handle to the next event in a chain after hDbEvent
  wParam=(WPARAM)(HANDLE)hDbEvent
  lParam=0
Returns the handle, or NULL if hDbEvent is invalid or is the last event
Events in a chain are sorted chronologically automatically
*/
#define MS_DB_EVENT_FINDNEXT  "DB/Event/FindNext"

/* DB/Event/FindPrev
Retrieves a handle to the previous event in a chain before hDbEvent
  wParam=(WPARAM)(HANDLE)hDbEvent
  lParam=0
Returns the handle, or NULL if hDbEvent is invalid or is the first event
Events in a chain are sorted chronologically automatically
*/
#define MS_DB_EVENT_FINDPREV  "DB/Event/FindPrev"

/************************** Encryption ****************************/

/* DB/Crypt/EncodeString
Scrambles pszString in-place using a strange encryption algorithm
  wParam=(WPARAM)(int)cbString
  lParam=(LPARAM)(char*)pszString
cbString is the size of the buffer pointed to by pszString, *not* the length
of pszString. This service may be changed at a later date such that it
increases the length of pszString
Returns 0 always
*/
#define MS_DB_CRYPT_ENCODESTRING  "DB/Crypt/EncodeString"

/* DB/Crypt/DecodeString
Descrambles pszString in-place using the strange encryption algorithm
  wParam=(WPARAM)(int)cbString
  lParam=(LPARAM)(char*)pszString
Reverses the operation done by crypt/encodestring
cbString is the size of the buffer pointed to by pszString, *not* the length
of pszString.
Returns 0 always
*/
#define MS_DB_CRYPT_DECODESTRING  "DB/Crypt/DecodeString"

/**************************** Time ********************************/

/* DB/Time/TimestampToLocal
Converts a GMT timestamp into local time
  wParam=(WPARAM)(DWORD)timestamp
  lParam=0
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
disadvantages :-) ).
*/
#define MS_DB_TIME_TIMESTAMPTOLOCAL   "DB/Time/TimestampToLocal"

/* DB/Time/TimestampToString
Converts a GMT timestamp into a customisable local time string
  wParam=(WPARAM)(DWORD)timestamp
  lParam=(LPARAM)(DBTIMETOSTRING*)&tts
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
wParam=(WPARAM)(BOOL)newSetting
lParam=0
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
wParam=lParam
lParam=(WPARAM)(DBMODULEENUMPROC)dbmep
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
**BUG**: Prior to 0.1.2.0 dbmep was called as (lParam)(szMod,ofsMod,lParam).
  This means that the lParam parameter to dbmep was useless, and explains the
  slightly odd 'wParam=lParam' in the definition.
*/
typedef int (*DBMODULEENUMPROC)(const char *szModuleName,DWORD ofsModuleName,LPARAM lParam);
#define MS_DB_MODULES_ENUM    "DB/Modules/Enum"

/* DB/Module/Delete  0.8.0+

Removes all settings for the specified module.
wParam=0 (unused)
lParam=(LPARAM)(char*)szModuleName - the module name to be deleted
*/

#define MS_DB_MODULE_DELETE "DB/Module/Delete"

/******************************************************************/
/************************** EVENTS ********************************/
/******************************************************************/

/* DB/Event/Added event
Called when a new event has been added to the event chain for a contact
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(HANDLE)hDbEvent
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
passed to MS_DB_EVENT_ADD.

The point of this hook is to stop any unwanted database events, to stop
an event being added, return 1, to allow the event to pass through return
0.
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)&DBEVENTINFO

Any changed made to the said DBEVENTINFO are also passed along to the database,
therefore it is possible to shape the data, however DO NOT DO THIS.
*/
#define ME_DB_EVENT_FILTER_ADD "DB/Event/FilterAdd"

/* DB/Event/Deleted event
Called when an event is about to be deleted from the event chain for a contact
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(HANDLE)hDbEvent
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
  wParam=(WPARAM)(HANDLE)hContact
  lParam=0
hContact is a valid handle to the new contact.
Contacts are initially created without any settings, so if you hook this event
you will almost certainly also want to hook db/contact/settingchanged as well.
*/
#define ME_DB_CONTACT_ADDED  "DB/Contact/Added"

/* DB/Contact/Deleted event
Called when an contact is about to be deleted
  wParam=(WPARAM)(HANDLE)hContact
  lParam=0
hContact is a valid handle to the contact which is about to be deleted, but it
won't be once your hook has returned.
Returning nonzero from your hook will not stop the deletion, but it will, as
usual, stop other hooks from being called.
Deleting a contact invalidates all events in its chain.
*/
#define ME_DB_CONTACT_DELETED  "DB/Contact/Deleted"

/* DB/Contact/SettingChanged event
Called when a contact has had one of its settings changed
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(DBCONTACTWRITESETTING*)&dbcws
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

/* DB/Contact/SetSettingResident service (0.6+)
Disables a setting saving to the database.
  wParam=(WPARAM)(BOOL)bIsResident
  lParam=(LPARAM)(char*)pszSettingName
*/
#define MS_DB_SETSETTINGRESIDENT "DB/SetSettingResident"

/******************************************************************/
/********************* SETTINGS HELPER FUNCTIONS ******************/
/******************************************************************/

#ifndef DB_NOHELPERFUNCTIONS

#define db_byte_get(a,b,c,d)                   DBGetContactSettingByte(a,b,c,d)
#define db_word_get(a,b,c,d)                   DBGetContactSettingWord(a,b,c,d)
#define db_dword_get(a,b,c,d)                  DBGetContactSettingDword(a,b,c,d)
#define db_get(a,b,c,d)                        DBGetContactSetting(a,b,c,d)

#define db_byte_set(a,b,c,d)                   DBWriteContactSettingByte(a,b,c,d)
#define db_word_set(a,b,c,d)                   DBWriteContactSettingWord(a,b,c,d)
#define db_dword_set(a,b,c,d)                  DBWriteContactSettingDword(a,b,c,d)
#define db_string_set(a,b,c,d)                 DBWriteContactSettingString(a,b,c,d)

#define db_unset(a,b,c)                        DBDeleteContactSetting(a,b,c);

#ifdef _DEBUG
	#define DBGetContactSettingByte(a,b,c,d)       DBGetContactSettingByte_Helper(a,b,c,d,__FILE__,__LINE__)
	#define DBGetContactSettingWord(a,b,c,d)       DBGetContactSettingWord_Helper(a,b,c,d,__FILE__,__LINE__)
	#define DBGetContactSettingDword(a,b,c,d)      DBGetContactSettingDword_Helper(a,b,c,d,__FILE__,__LINE__)
	#define DBGetContactSetting(a,b,c,d)           DBGetContactSetting_Helper(a,b,c,d,__FILE__,__LINE__)
	#define DBGetContactSettingString(a,b,c,d)     DBGetContactSettingString_Helper(a,b,c,d,__FILE__,__LINE__,DBVT_ASCIIZ)
	#define DBGetContactSettingWString(a,b,c,d)    DBGetContactSettingString_Helper(a,b,c,d,__FILE__,__LINE__,DBVT_WCHAR)
	#define DBGetContactSettingUTF8String(a,b,c,d) DBGetContactSettingString_Helper(a,b,c,d,__FILE__,__LINE__,DBVT_UTF8)
#else
	#define DBGetContactSettingByte(a,b,c,d)       DBGetContactSettingByte_Helper(a,b,c,d)
	#define DBGetContactSettingWord(a,b,c,d)       DBGetContactSettingWord_Helper(a,b,c,d)
	#define DBGetContactSettingDword(a,b,c,d)      DBGetContactSettingDword_Helper(a,b,c,d)
	#define DBGetContactSetting(a,b,c,d)           DBGetContactSetting_Helper(a,b,c,d)
	#define DBGetContactSettingString(a,b,c,d)     DBGetContactSettingString_Helper(a,b,c,d,DBVT_ASCIIZ)
	#define DBGetContactSettingWString(a,b,c,d)    DBGetContactSettingString_Helper(a,b,c,d,DBVT_WCHAR)
	#define DBGetContactSettingUTF8String(a,b,c,d) DBGetContactSettingString_Helper(a,b,c,d,DBVT_UTF8)
#endif

#ifdef _UNICODE
#define DBGetContactSettingTString DBGetContactSettingWString
#else
#define DBGetContactSettingTString DBGetContactSettingString
#endif

#define db_msg_dbg(s) MessageBoxA(0,(s),"",0);

/* Deprecated & bizarre aliases */
#define DBGetContactSettingStringUtf           DBGetContactSettingUTF8String
#define DBWriteContactSettingStringUtf         DBWriteContactSettingUTF8String
#ifdef _DEBUG
	#define DBGetContactSettingW(a,b,c,d)          DBGetContactSettingString_Helper(a,b,c,d,__FILE__,__LINE__,0)
#else
	#define DBGetContactSettingW(a,b,c,d)          DBGetContactSettingString_Helper(a,b,c,d,0)
#endif

#ifdef _DEBUG
#include <stdio.h>
#endif

__inline static int DBGetContactSettingByte_Helper(HANDLE hContact, const char *szModule, const char *szSetting, int errorValue
#ifdef _DEBUG
	,const char *szFile, const int nLine
#endif
)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING cgs;

	cgs.szModule=szModule;
	cgs.szSetting=szSetting;
	cgs.pValue=&dbv;
	if(CallService(MS_DB_CONTACT_GETSETTING,(WPARAM)hContact,(LPARAM)&cgs))
		return errorValue;
#ifdef _DEBUG
	if(dbv.type!=DBVT_BYTE) {
		char buf[128];
		_snprintf(buf,sizeof(buf),"%s:%d for %s/%s not a byte, return: %d",szFile,nLine,szModule,szSetting,dbv.type);
		buf[sizeof(buf)-1]=0;
		db_msg_dbg(buf);
	}
#endif
	return dbv.bVal;
}

__inline static int DBGetContactSettingWord_Helper(HANDLE hContact,const char *szModule,const char *szSetting,int errorValue
#ifdef _DEBUG
	,const char *szFile, const int nLine
#endif
)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING cgs;

	cgs.szModule=szModule;
	cgs.szSetting=szSetting;
	cgs.pValue=&dbv;
	if(CallService(MS_DB_CONTACT_GETSETTING,(WPARAM)hContact,(LPARAM)&cgs))
		return errorValue;
#ifdef _DEBUG
	if(dbv.type!=DBVT_WORD) {
		char buf[128];
		_snprintf(buf,sizeof(buf),"%s:%d for %s/%s not a word, return: %d",szFile,nLine,szModule,szSetting,dbv.type);
		buf[sizeof(buf)-1]=0;
		db_msg_dbg(buf);
	}
#endif
	return dbv.wVal;
}

__inline static DWORD DBGetContactSettingDword_Helper(HANDLE hContact,const char *szModule, const char *szSetting, DWORD errorValue
#ifdef _DEBUG
	,const char *szFile, const int nLine
#endif
)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING cgs;

	cgs.szModule=szModule;
	cgs.szSetting=szSetting;
	cgs.pValue=&dbv;
	if(CallService(MS_DB_CONTACT_GETSETTING,(WPARAM)hContact,(LPARAM)&cgs))
		return errorValue;
#ifdef _DEBUG
	if(dbv.type!=DBVT_DWORD) {
		char buf[128];
		_snprintf(buf,sizeof(buf),"%s:%d for %s/%s not a dword, return: %d",szFile,nLine,szModule,szSetting,dbv.type);
		buf[sizeof(buf)-1]=0;
		db_msg_dbg(buf);
	}
#endif
	return dbv.dVal;
}

__inline static INT_PTR DBGetContactSetting_Helper(HANDLE hContact,const char *szModule,const char *szSetting,DBVARIANT *dbv
#if defined(_DEBUG)
	,const char *szFile, const int nLine
#endif
)
{
	INT_PTR rc;
	DBCONTACTGETSETTING cgs;
	cgs.szModule=szModule;
	cgs.szSetting=szSetting;
	cgs.pValue=dbv;

	rc=CallService(MS_DB_CONTACT_GETSETTING,(WPARAM)hContact,(LPARAM)&cgs);
#if defined(_DEBUG) && defined(DBCHECKSETTINGS)
	if (rc != 0) {
		char buf[128];
		_snprintf(buf,sizeof(buf),"%s:%d failed to fetch %s/%s",szFile,nLine,szModule,szSetting);
		db_msg_dbg(buf);
	}
#endif
	return rc;
}

__inline static INT_PTR DBGetContactSettingString_Helper(HANDLE hContact,const char *szModule,const char *szSetting,DBVARIANT *dbv,
#if defined(_DEBUG)
	const char *szFile, const int nLine,
#endif
	const int nType)
{
	INT_PTR rc;
	DBCONTACTGETSETTING cgs;
	cgs.szModule=szModule;
	cgs.szSetting=szSetting;
	cgs.pValue=dbv;
	dbv->type=(BYTE)nType;

	rc=CallService(MS_DB_CONTACT_GETSETTING_STR,(WPARAM)hContact,(LPARAM)&cgs);
#if defined(_DEBUG) && defined(DBCHECKSETTINGS)
	if (rc != 0) {
		char buf[128];
		_snprintf(buf,sizeof(buf),"%s:%d failed to fetch %s/%s",szFile,nLine,szModule,szSetting);
		db_msg_dbg(buf);
	}
#endif
	return rc;
}

__inline static INT_PTR DBFreeVariant(DBVARIANT *dbv)
{
	return CallService(MS_DB_CONTACT_FREEVARIANT,0,(LPARAM)dbv);
}

__inline static char *DBGetString(HANDLE hContact,const char *szModule,const char *szSetting)
{
	char *str=NULL;
	DBVARIANT dbv={0};
	DBGetContactSettingString(hContact,szModule,szSetting,&dbv);
	if(dbv.type==DBVT_ASCIIZ)
		str=mir_strdup(dbv.pszVal);
	DBFreeVariant(&dbv);
	return str;
}

#define DBGetStringA DBGetString

__inline static wchar_t *DBGetStringW(HANDLE hContact,const char *szModule,const char *szSetting)
{
	wchar_t *str=NULL;
	DBVARIANT dbv={0};
	DBGetContactSettingWString(hContact,szModule,szSetting,&dbv);
	if(dbv.type==DBVT_WCHAR)
		str=mir_wstrdup(dbv.pwszVal);
	DBFreeVariant(&dbv);
	return str;
}

#ifdef _UNICODE
#define DBGetStringT DBGetStringW
#else
#define DBGetStringT DBGetString
#endif

__inline static INT_PTR DBDeleteContactSetting(HANDLE hContact,const char *szModule,const char *szSetting)
{
	DBCONTACTGETSETTING cgs;
	cgs.szModule=szModule;
	cgs.szSetting=szSetting;
	return CallService(MS_DB_CONTACT_DELETESETTING,(WPARAM)hContact,(LPARAM)&cgs);
}

__inline static INT_PTR DBWriteContactSettingByte(HANDLE hContact,const char *szModule,const char *szSetting,BYTE val)
{
	DBCONTACTWRITESETTING cws;

	cws.szModule=szModule;
	cws.szSetting=szSetting;
	cws.value.type=DBVT_BYTE;
	cws.value.bVal=val;
	return CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws);
}

__inline static INT_PTR DBWriteContactSettingWord(HANDLE hContact,const char *szModule,const char *szSetting,WORD val)
{
	DBCONTACTWRITESETTING cws;

	cws.szModule=szModule;
	cws.szSetting=szSetting;
	cws.value.type=DBVT_WORD;
	cws.value.wVal=val;
	return CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws);
}

__inline static INT_PTR DBWriteContactSettingDword(HANDLE hContact,const char *szModule,const char *szSetting,DWORD val)
{
	DBCONTACTWRITESETTING cws;

	cws.szModule=szModule;
	cws.szSetting=szSetting;
	cws.value.type=DBVT_DWORD;
	cws.value.dVal=val;
	return CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws);
}

__inline static INT_PTR DBWriteContactSettingString(HANDLE hContact,const char *szModule,const char *szSetting,const char *val)
{
	DBCONTACTWRITESETTING cws;

	cws.szModule=szModule;
	cws.szSetting=szSetting;
	cws.value.type=DBVT_ASCIIZ;
	cws.value.pszVal=(char*)val;
	return CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws);
}

__inline static INT_PTR DBWriteContactSettingWString(HANDLE hContact,const char *szModule,const char *szSetting,const WCHAR *val)
{
	DBCONTACTWRITESETTING cws;

	cws.szModule=szModule;
	cws.szSetting=szSetting;
	cws.value.type=DBVT_WCHAR;
	cws.value.pwszVal=(WCHAR*)val;
	return CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws);
}

#ifdef _UNICODE
#define DBWriteContactSettingTString DBWriteContactSettingWString
#else
#define DBWriteContactSettingTString DBWriteContactSettingString
#endif

__inline static INT_PTR DBWriteContactSettingUTF8String(HANDLE hContact,const char *szModule,const char *szSetting,const char *val)
{
	DBCONTACTWRITESETTING cws;

	cws.szModule=szModule;
	cws.szSetting=szSetting;
	cws.value.type=DBVT_UTF8;
	cws.value.pszVal=(char*)val;
	return CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws);
}

__inline static INT_PTR DBWriteContactSettingBlob(HANDLE hContact,const char *szModule,const char *szSetting,void *val,unsigned len)
{
	DBCONTACTWRITESETTING cws;

	cws.szModule=szModule;
	cws.szSetting=szSetting;
	cws.value.type=DBVT_BLOB;
    cws.value.cpbVal = (WORD)len;
	cws.value.pbVal=(unsigned char*)val;
	return CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws);
}

/* inlined range tolerate versions */

__inline static BYTE DBGetContactSettingRangedByte(HANDLE hContact, const char *szModule, const char *szSetting, BYTE errorValue, BYTE minValue, BYTE maxValue) {
	BYTE bVal = (BYTE)DBGetContactSettingByte(hContact, szModule, szSetting, errorValue);

	if (bVal < minValue || bVal > maxValue) {
#ifdef _DEBUG
		char szBuf[MAX_PATH];
		_snprintf(szBuf,sizeof(szBuf),"(%s:%s) not in range of %d..%d",szModule,szSetting,minValue,maxValue);
		szBuf[sizeof(szBuf)-1]=0;
		MessageBoxA(0,szBuf,"DBGetContactSettingRangedByte failed",MB_ICONERROR);
#endif
		return errorValue;
	}
	return bVal;
}

__inline static WORD DBGetContactSettingRangedWord(HANDLE hContact, const char *szModule, const char *szSetting, WORD errorValue, WORD minValue, WORD maxValue) {
	WORD wVal = (WORD)DBGetContactSettingWord(hContact, szModule, szSetting, errorValue);

	if (wVal < minValue || wVal > maxValue) {
#ifdef _DEBUG
		char szBuf[MAX_PATH];
		_snprintf(szBuf,sizeof(szBuf),"(%s:%s) not in range of %d..%d",szModule,szSetting,minValue,maxValue);
		szBuf[sizeof(szBuf)-1]=0;
		MessageBoxA(0,szBuf,"DBGetContactSettingRangedWord failed",MB_ICONERROR);
#endif
		return errorValue;
	}
	return wVal;
}

__inline static DWORD DBGetContactSettingRangedDword(HANDLE hContact, const char *szModule, const char *szSetting, DWORD errorValue, DWORD minValue, DWORD maxValue) {
	DWORD dVal = DBGetContactSettingDword(hContact, szModule, szSetting, errorValue);

	if (dVal < minValue || dVal > maxValue) {
#ifdef _DEBUG
		char szBuf[MAX_PATH];
		_snprintf(szBuf,sizeof(szBuf),"(%s:%s) not in range of %d..%d",szModule,szSetting,minValue,maxValue);
		szBuf[sizeof(szBuf)-1]=0;
		MessageBoxA(0,szBuf,"DBGetContactSettingRangedDword failed",MB_ICONERROR);
#endif
		return errorValue;
	}
	return dVal;
}

#endif

#endif // M_DATABASE_H__

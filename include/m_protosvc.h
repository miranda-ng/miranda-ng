/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-09 Miranda ICQ/IM project,
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

//this module was created in v0.1.1.0

//none of these services should be used on their own (ie using CallService,
//CreateServiceFunction(), etc), hence the PS_ prefix. Instead use the services
//exposed in m_protocols.h

#ifndef M_PROTOSVC_H__
#define M_PROTOSVC_H__ 1

#include "m_protocols.h"

/* *****  Unicode Services note   ********
******************************************

Only new style protocols (Miranda 0.9+) with m_iVersion set to 2 or higher
support Unicode services documented below, all other support only ANSI.

For all other that do not support Unicode services, Miranda core will
convert Unicode to ANSI and call the appropriate service.
*/

/*************************** NON-CONTACT SERVICES ************************/
//these should be called with CallProtoServiceInt(NULL,)

//Get the capability flags of the module.
//wParam = flagNum
//lParam = 0
//Returns a bitfield corresponding to wParam. See the #defines below
//Should return 0 for unknown values of flagNum
//Non-network-access modules should return flags to represent the things they
//actually actively use, not the values that it is known to pass through
//correctly
#define PFLAGNUM_1   1
#define PF1_IMSEND        0x00000001       //supports IM sending
#define PF1_IMRECV        0x00000002       //supports IM receiving
#define PF1_IM            (PF1_IMSEND|PF1_IMRECV)
#define PF1_URLSEND       0x00000004       //supports separate URL sending
#define PF1_URLRECV       0x00000008       //supports separate URL receiving
#define PF1_URL           (PF1_URLSEND|PF1_URLRECV)
#define PF1_FILESEND      0x00000010       //supports file sending
#define PF1_FILERECV      0x00000020       //supports file receiving
#define PF1_FILE          (PF1_FILESEND|PF1_FILERECV)
#define PF1_MODEMSGSEND   0x00000040       //supports broadcasting away messages
#define PF1_MODEMSGRECV   0x00000080       //supports reading others' away messages
#define PF1_MODEMSG       (PF1_MODEMSGSEND|PF1_MODEMSGRECV)
#define PF1_SERVERCLIST   0x00000100       //contact lists are stored on the server, not locally. See notes below
#define PF1_AUTHREQ       0x00000200       //will get authorisation requests for some or all contacts
#define PF1_ADDED         0x00000400       //will get 'you were added' notifications
#define PF1_VISLIST       0x00000800       //has an invisible list
#define PF1_INVISLIST     0x00001000       //has a visible list for when in invisible mode
#define PF1_INDIVSTATUS   0x00002000       //supports setting different status modes to each contact
#define PF1_EXTENSIBLE    0x00004000       //the protocol is extensible and supports plugin-defined messages
#define PF1_PEER2PEER     0x00008000       //supports direct (not server mediated) communication between clients
#define PF1_NEWUSER       0x00010000       //supports creation of new user IDs
#define PF1_CHAT          0x00020000       //has a realtime chat capability
#define PF1_INDIVMODEMSG  0x00040000       //supports replying to a mode message request with different text depending on the contact requesting
#define PF1_BASICSEARCH   0x00080000       //supports a basic user searching facility
#define PF1_EXTSEARCH     0x00100000	   //supports one or more protocol-specific extended search schemes
#define PF1_CANRENAMEFILE 0x00200000       //supports renaming of incoming files as they are transferred
#define PF1_FILERESUME    0x00400000       //can resume broken file transfers, see PS_FILERESUME below
#define PF1_ADDSEARCHRES  0x00800000       //can add search results to the contact list
#define PF1_CONTACTSEND   0x01000000	   //can send contacts to other users
#define PF1_CONTACTRECV   0x02000000	   //can receive contacts from other users
#define PF1_CONTACT       (PF1_CONTACTSEND|PF1_CONTACTRECV)
#define PF1_CHANGEINFO    0x04000000       //can change our user information stored on server
#define PF1_SEARCHBYEMAIL 0x08000000       //supports a search by e-mail feature
#define PF1_USERIDISEMAIL 0x10000000       //set if the uniquely identifying field of the network is the e-mail address
#define PF1_SEARCHBYNAME  0x20000000       //supports searching by nick/first/last names
#define PF1_EXTSEARCHUI   0x40000000       //has a dialog box to allow searching all the possible fields
#define PF1_NUMERICUSERID 0x80000000       //the unique user IDs for this protocol are numeric

#define PFLAGNUM_2   2			 //the status modes that the protocol supports
#define PF2_ONLINE        0x00000001   //an unadorned online mode
#define PF2_INVISIBLE     0x00000002
#define PF2_SHORTAWAY     0x00000004   //Away on ICQ, BRB on MSN
#define PF2_LONGAWAY      0x00000008   //NA on ICQ, Away on MSN
#define PF2_LIGHTDND      0x00000010   //Occupied on ICQ, Busy on MSN
#define PF2_HEAVYDND      0x00000020   //DND on ICQ
#define PF2_FREECHAT      0x00000040
#define PF2_OUTTOLUNCH    0x00000080
#define PF2_ONTHEPHONE    0x00000100
#define PF2_IDLE		  0x00000200   //added during 0.3.4 (2004/09/13)

//the status modes that the protocol supports
//away-style messages for. Uses the PF2_ flags.
// PFLAGNUM_3 is implemented by protocol services that support away messages
// there may be no support and 0 will be returned, if there is
// support it will consist of a set of PF2_* bits
#define PFLAGNUM_3   3

// given a status will return what bit flags to test for
static __inline unsigned long Proto_Status2Flag(int status)
{
	switch(status) {
		case ID_STATUS_ONLINE: return PF2_ONLINE;
		case ID_STATUS_OFFLINE: return 0;
		case ID_STATUS_INVISIBLE: return PF2_INVISIBLE;
		case ID_STATUS_OUTTOLUNCH: return PF2_OUTTOLUNCH;
		case ID_STATUS_ONTHEPHONE: return PF2_ONTHEPHONE;
		case ID_STATUS_AWAY: return PF2_SHORTAWAY;
		case ID_STATUS_NA: return PF2_LONGAWAY;
		case ID_STATUS_OCCUPIED: return PF2_LIGHTDND;
		case ID_STATUS_DND: return PF2_HEAVYDND;
		case ID_STATUS_FREECHAT: return PF2_FREECHAT;
		case ID_STATUS_IDLE: return PF2_IDLE;
	}
	return 0;
}

#define PFLAGNUM_4           4          // another bunch of flags

#define PF4_FORCEAUTH	     0x00000001 // forces auth requests to be sent when adding users
#define PF4_FORCEADDED	     0x00000002 // forces "you were added" requests to be sent
#define PF4_NOCUSTOMAUTH     0x00000004 // protocol doesn't support custom auth text (doesn't show auth text box)
#define PF4_SUPPORTTYPING    0x00000008 // protocol supports user is typing messages
#define PF4_SUPPORTIDLE      0x00000010 // protocol understands idle
#define PF4_AVATARS		     0x00000020 // protocol has avatar support
#define PF4_OFFLINEFILES     0x00000040 // protocols supports sending files to offline users
#define PF4_IMSENDOFFLINE    0x00000100 // protocol supports sending offline messages
#define PF4_INFOSETTINGSVC   0x00000200 // protocol supports user info translation services
#define PF4_NOAUTHDENYREASON 0x00000400 // protocol doesn't support authorization deny reason
#define PF4_GROUPCHATFILES   0x00000800 // protocol supports sending files to group chats
#define PF4_SINGLEFILEONLY   0x00001000 // protocol supports sending files one by one only

#define PFLAG_UNIQUEIDTEXT  100    //returns a static buffer of text describing the unique field by which this protocol identifies users (already translated), or NULL

#define PFLAG_MAXCONTACTSPERPACKET  200   //v0.1.2.2+: returns the maximum number of contacts which can be sent in a single PSS_CONTACTS, lParam = (LPARAM)hContact.

#define PFLAG_UNIQUEIDSETTING 300 // returns the setting name of where the unique id is stored

#define PFLAG_MAXLENOFMESSAGE 400		// v0.3.2+: return the maximum length of an instant message, lParam = (LPARAM)hContact

/*

  A protocol might not support this cap, it allows a protocol to say that PFLAGNUM_2 is for
  statuses contacts supports, and that PFLAGNUM_5 is for statuses a protocol can SET TO ITSELF,
  if this is not replied to, then PFLAGNUM_2 is alone in telling you which statuses a protocol
  can set to and what statuses a contact can set to as well.

  E.g. A protocol might report 'wireless' users but a login of the protocol from Miranda can
  not set itself to 'wireless' so PFLAGNUM_2 would return PF2_ONTHEPHONE and PFLAGNUM_5 would
  return PF2_ONTHEPHONE as well, this means "I will get contacts who are on the phone but you can
  not set on the phone" and so on.

  Do note that the reply here is a NEGATION of bitflags reported for PFLAGNUM_2, e.g. returning
  PF2_ONTHEPHONE for PFLAGNUM_2 and returning the same for PFLAGNUM_5 says that you DO NOT SUPPORT
  PF2_ONTHEPHONE for the user to PS_SETSTATUS to, but you will expect other contacts to have
  that status, e.g. you can get onthephone for users but can't go online with onthephone.

  The same PF2_* status flags are used in the reply.

Added during 0.3.4 (2004/09/14)
*/
#define PFLAGNUM_5 5

/* Deleting contacts from protocols that store the contact list on the server:
If a contact is deleted while the protocol is online, it is expected that the
protocol will have hooked me_db_contact_deleted and take the appropriate
action by itself.
If a contact is deleted while the protocol is offline, the contact list will
display a message to the user about the problem, and set the byte setting
"CList"/"Delete" to 1. Each time such a protocol changes status from offline
or connecting to online the contact list will check for contacts with this
flag set and delete them at that time. Your hook for me_db_contact_deleted
will pick this up and everything will be good.
*/
#define PS_GETCAPS     "/GetCaps"

//Get a human-readable name for the protocol
//wParam = cchName
//lParam = (LPARAM)(char*)szName
//Returns 0 on success, nonzero on failure
//cchName is the number of characters in the buffer szName
//This should be translated before being returned
//Some example strings are:
//"ICQ", "AIM", "RSA-1024 Encryption"
#define PS_GETNAME     "/GetName"

//Loads one of the protocol-specific icons
//wParam = whichIcon
//lParam = 0
//Returns the HICON, or NULL on failure
//The returned HICON must be DestroyIcon()ed.
//The UI should overlay the online icon with a further UI-specified icon to
//represent the exact status mode.
#define PLI_PROTOCOL   1    //An icon representing the protocol (eg the multicoloured flower for ICQ)
#define PLI_ONLINE     2    //Online state icon for that protocol (eg green flower for ICQ)
#define PLI_OFFLINE    3    //Offline state icon for that protocol (eg red flower for ICQ)
#define PLIF_LARGE     0		//OR with one of the above to get the large (32x32 by default) icon
#define PLIF_SMALL     0x10000  //OR with one of the above to get the small (16x16 by default) icon
#define PLIF_ICOLIB    0x20000  //the returned HICON is managed by IcoLib, DO NOT DestroyIcon() it
#define PLIF_ICOLIBHANDLE 0x40000 //the function will return IcoLib handle not HICON
#define PS_LOADICON    "/LoadIcon"

//Change the protocol's status mode
//wParam = newMode, from ui/contactlist/statusmodes.h
//lParam = 0
//returns 0 on success, nonzero on failure
//Will send an ack with:
//type = ACKTYPE_STATUS, result = ACKRESULT_SUCCESS, hProcess = (HANDLE)previousMode, lParam = newMode
//when the change completes. This ack is sent for all changes, not just ones
//caused by calling this function.
//Note that newMode can be ID_STATUS_CONNECTING <= newMode<ID_STATUS_CONNECTING+
//MAX_CONNECT_RETRIES to signify that it's connecting and it's the nth retry.
//Protocols are initially always in offline mode.
//Non-network-level protocol modules do not have the concept of a status and
//should leave this service unimplemented
//If a protocol doesn't support the specific status mode, it should pick the
//closest one that it does support, and change to that.
//If the new mode requires that the protocol switch from offline to online then
//it will do so, but errors will be reported in the form of an additional ack:
//type = ACKTYPE_LOGIN, result = ACKRESULT_FAILURE, hProcess = NULL, lParam = LOGINERR_
// (added during 0.3.4.3) the protocol will send LOGINERR_OTHERLOCATION if the login
// was disconnected because of a login at another location
#define LOGINERR_WRONGPASSWORD  1
#define LOGINERR_NONETWORK      2
#define LOGINERR_PROXYFAILURE   3
#define LOGINERR_BADUSERID      4
#define LOGINERR_NOSERVER       5
#define LOGINERR_TIMEOUT        6
#define LOGINERR_WRONGPROTOCOL  7
#define LOGINERR_OTHERLOCATION  8
//protocols may define more error codes starting at 1000
#define PS_SETSTATUS   "/SetStatus"

//Sets the status-mode specific message for yourself
//wParam = status mode
//lParam = (LPARAM)(const char*)szMessage
//Returns 0 on success, nonzero on failure
//Note that this service will not be available unless PF1_MODEMSGSEND is set
//and PF1_INDIVMODEMSG is *not* set.
//Protocol modules must support szMessage = NULL. It may either mean to use an
//empty message, or (preferably) to not reply at all to any requests
#define PS_SETAWAYMSG   "/SetAwayMsg"
#define PS_SETAWAYMSGW  "/SetAwayMsgW"

//Get the status mode that a protocol is currently in
//wParam = lParam = 0
//Returns the status mode
//Non-network-level protocol modules do not have the concept of a status and
//should leave this service unimplemented
#define PS_GETSTATUS	"/GetStatus"

//Allow somebody to add us to their contact list
//wParam = (WPARAM)(HANDLE)hDbEvent
//lParam = 0
//Returns 0 on success, nonzero on failure
//Auth requests come in the form of an event added to the database for the NULL
//user. The form is:
//DWORD protocolSpecific
//ASCIIZ nick, firstName, lastName, e-mail, requestReason
//hDbEvent must be the handle of such an event
//One or more fields may be empty if the protocol doesn't support them
#define PS_AUTHALLOW   "/Authorize"

//Deny an authorisation request
//wParam = (WPARAM)(HANDLE)hDbEvent
//lParam = (LPARAM)(const TCHAR*)szReason
//Returns 0 on success, nonzero on failure
//Protocol modules must be able to cope with szReason = NULL
#define PS_AUTHDENY    "/AuthDeny"
#define PS_AUTHDENYW   "/AuthDenyW"

// Send a "You were added" event
// wParam = lParam = 0
// Returns 0 on success, nonzero on failure
#define PSS_ADDED	"/YouWereAdded"

//Create account manager UI form
//wParam = 0
//lParam = (LPARAM)(HWND)hwndAccMgr
//Returns handle on newly created form.
//Size for best fit is 186x134 DLUs, please avoid groupboxes
//paddind and advanced options. This should provide minimal setup
//for initial connect.
#define PS_CREATEACCMGRUI "/CreateAccMgrUI"

//Send a basic search request
//wParam = 0
//lParam = (LPARAM)(const TCHAR*)szId
//Returns a handle for the search, or zero on failure
//All protocols identify users uniquely by a single field. This service will
//search by that field.
//Note that if users are identified by an integer (eg ICQ) szId should be a
//string containing that integer, not the integer itself.
//All search replies (even protocol-specific extended searches) are replied by
//means of a series of acks:
//result acks, one of more of:
//type = ACKTYPE_SEARCH, result = ACKRESULT_DATA, lParam = (LPARAM)(PROTOSEARCHRESULT*)&psr
//ending ack:
//type = ACKTYPE_SEARCH, result = ACKRESULT_SUCCESS, lParam = 0
//Note that the pointers in the structure are not guaranteed to be valid after
//the ack is complete.

#define PSR_UNICODE 1        // return Unicode status
#if defined(_UNICODE)
	#define PSR_TCHAR PSR_UNICODE
#else
	#define PSR_TCHAR 0
#endif

typedef struct {
	int cbSize;
	MAllStrings nick;
	MAllStrings firstName;
	MAllStrings lastName;
	MAllStrings email;
	MAllStrings id;
	int flags;
} PROTOSEARCHRESULT;
#define PS_BASICSEARCH  "/BasicSearch"
#define PS_BASICSEARCHW "/BasicSearchW"

//Search for users by e-mail address           v0.1.2.1+
//wParam = 0
//lParam = (LPARAM)(TCHAR*)szEmail
//Returns a HANDLE to the search, or NULL on failure
//Results are returned as for PS_BASICSEARCH.
//This function is only available if the PF1_SEARCHBYEMAIL capability is set
//If the PF1_USERIDISEMAIL capability is set then this function and
//PS_BASICSEARCH should have the same result (and it's best if they are the
//same function).
#define PS_SEARCHBYEMAIL    "/SearchByEmail"
#define PS_SEARCHBYEMAILW   "/SearchByEmailW"

//Search for users by name           v0.1.2.1+
//wParam = 0
//lParam = (LPARAM)(PROTOSEARCHBYNAME*)&psbn
//Returns a HANDLE to the search, or NULL on failure
//Results are returned as for PS_BASICSEARCH.
//This function is only available if the PF1_SEARCHBYNAME capability is set
typedef struct {
	TCHAR *pszNick;
	TCHAR *pszFirstName;
	TCHAR *pszLastName;
} PROTOSEARCHBYNAME;
#define PS_SEARCHBYNAME    "/SearchByName"
#define PS_SEARCHBYNAMEW   "/SearchByNameW"

//Create the advanced search dialog box     v0.1.2.1+
//wParam = 0
//lParam = (HWND)hwndOwner
//Returns a HWND, or NULL on failure
//This function is only available if the PF1_EXTSEARCHUI capability is set
//Advanced search is very protocol-specific so it is left to the protocol
//itself to supply a dialog containing the options. This dialog should not
//have a title bar and contain only search fields. The rest of the UI is
//supplied by Miranda.
//The dialog should be created with CreateDialog() or its kin and still be
//hidden when this function returns.
//The dialog will be destroyed when the find/add dialog is closed.
#define PS_CREATEADVSEARCHUI        "/CreateAdvSearchUI"

//Search using the advanced search dialog     v0.1.2.1+
//wParam = 0
//lParam = (LPARAM)(HWND)hwndAdvancedSearchDlg
//Returns a HANDLE to the search, or NULL on failure
//Results are returned as for PS_BASICSEARCH.
//This function is only available if the PF1_EXTSEARCHUI capability is set
#define PS_SEARCHBYADVANCED          "/SearchByAdvanced"

typedef struct {
	size_t nSize;
	int nFieldCount;
	TCHAR **pszFields;
	PROTOSEARCHRESULT psr;
} CUSTOMSEARCHRESULTS;

//Adds a search result to the contact list
//wParam = flags
//lParam = (LPARAM)(PROTOSEARCHRESULT*)&psr
//Returns a HANDLE to the new contact, or NULL on failure
//psr must be a result returned by a search function, since the extended
//information past the end of the official structure may contain important
//data required by the protocol.
//The protocol library should not allow duplicate contacts to be added, but if
//such a request is received it should return the original hContact, and do the
//appropriate thing with the temporary flag (ie newflag = (oldflag&thisflag))
#define PALF_TEMPORARY    1    //add the contact temporarily and invisibly, just to get user info or something
#define PS_ADDTOLIST   "/AddToList"

//Adds a contact to the contact list given an auth, added or contacts event
//wParam = MAKEWPARAM(flags, iContact)
//lParam = (LPARAM)(HANDLE)hDbEvent
//Returns a HANDLE to the new contact, or NULL on failure
//hDbEvent must be either EVENTTYPE_AUTHREQ or EVENTTYPE_ADDED
//flags are the same as for PS_ADDTOLIST.
//iContact is only used for contacts events. It is the 0-based index of the
//contact in the event to add. There is no way to add two or more contacts at
//once, you should just do lots of calls.
#define PS_ADDTOLISTBYEVENT  "/AddToListByEvent"

//Informs the protocol of the users chosen resume behaviour   v0.1.2.2+
//wParam = (WPARAM)(HANDLE)hFileTransfer
//lParam = (LPARAM)(PROTOFILERESUME*)&pfr
//Returns 0 on success, nonzero on failure
//If the protocol supports file resume (PF1_FILERESUME) then before each
//individual file receive begins (note: not just each file that already exists)
//it will broadcast an ack with type = ACKTYPE_FILE, result = ACKRESULT_RESUME,
//hProcess = hFileTransfer, lParam = (LPARAM)(PROTOFILETRANSFERSTATUS*)&fts. If the
//UI processes this ack it must return nonzero from its hook. If all the hooks
//complete without returning nonzero then the protocol will assume that no
//resume UI was available and will continue the file receive with a default
//behaviour (overwrite for ICQ). If a hook does return nonzero then that UI
//must call this function, PS_FILERESUME, at some point. When the protocol
//module receives this call it will proceed with the file receive using the
//given information.
//Having said that PS_FILERESUME must be called, it is also acceptable to call
//PSS_FILECANCEL to completely abort the transfer instead.
#define FILERESUME_OVERWRITE  1
#define FILERESUME_RESUME     2
#define FILERESUME_RENAME     3
#define FILERESUME_SKIP       4
typedef struct {
	int action;    //a FILERESUME_ flag
	const TCHAR *szFilename;  // full path. Only valid if action == FILERESUME_RENAME
} PROTOFILERESUME;
#define PS_FILERESUME     "/FileResume"
#define PS_FILERESUMEW    "/FileResumeW"

//Asks a protocol to join the chatroom from contact  v0.8.0+
//wParam = (MCONTACT)hContact
//lParam = 0
//Returns 0 on success, nonzero on failure
#define PS_JOINCHAT "/JoinChat"

//Asks a protocol to leave the chatroom from contact  v0.8.0+
//wParam = (MCONTACT)hContact
//lParam = 0
//Returns 0 on success, nonzero on failure
#define PS_LEAVECHAT "/LeaveChat"

//Asks a protocol to read contact information and translate them (for a lookup fields)  v0.8.0+
//wParam = (MCONTACT)hContact
//lParam = (LPARAM)(DBCONTACTGETSETTING*)&dbcgs
//The flag PF4_INFOSETTINGSVC indicates that a protocol supports this. Basically it should
//do the same as MS_DB_CONTACT_GETSETTING_STR, except that for a lookup settings (e.g. Language)
//it returns string instead of an ID stored in the database.
//Caller is responsible for free()ing dbcgs.pValue->pszVal and pbVal if they are
//returned. You must **NOT** do this from your version of free() you have to use Miranda's free()
//you can get a function pointer to Miranda's free() via MS_SYSTEM_GET_MMI, see m_system.h
//Returns 0 on success or nonzero if the setting name was not found or hContact
//was invalid
#define PS_GETINFOSETTING "/GetInfoSetting"

// Asks protocol for the status message for a status
// wParam = (WORD) 0 for current status or a status id
// lParam = SGMA_xxx
// Returns status msg or NULL if there is none.  The protocol have to handle only the current
// status. Handling messages for other statuses is optional.
// Remember to mir_free the return value

#define SGMA_UNICODE 1        // return Unicode status
#if defined(_UNICODE)
	#define SGMA_TCHAR SGMA_UNICODE
#else
	#define SGMA_TCHAR 0
#endif

#define PS_GETMYAWAYMSG  "/GetMyAwayMsg"

// Set nickname for the user
// wParam = (WPARAM)SMNN_xxx
// lParam = (LPARAM)(char *) The new nickname for the user
// return = 0 for success

#define SMNN_UNICODE 1        // return Unicode status
#if defined(_UNICODE)
	#define SMNN_TCHAR SMNN_UNICODE
#else
	#define SMNN_TCHAR 0
#endif

#define PS_SETMYNICKNAME "/SetNickname"

// Get the max allowed length for the user nickname
// Optional, default value is 1024
// wParam = 0
// lParam = 0
// return = <= 0 for error, >0 the max length of the nick
#define PS_GETMYNICKNAMEMAXLENGTH "/GetMyNicknameMaxLength"

// WAYD = What are you doing
#define WAYD_UNICODE 1        // return Unicode texts
#if defined(_UNICODE)
	#define WAYD_TCHAR WAYD_UNICODE
#else
	#define WAYD_TCHAR 0
#endif

// Get the WAYD message for the user
// wParam = (WPARAM)WAYD_xxx
// lParam = 0
// Returns the text or NULL if there is none. Remember to mir_free the return value.
#define PS_GETMYWAYD "/GetMyWAYD"

// Sets the WAYD message for the user
// wParam = (WPARAM)WAYD_xxx
// lParam = (LPARAM)(WCHAR * or char *)The message
// Returns 0 on success, nonzero on failure
#define PS_SETMYWAYD "/SetMyWAYD"

// Get the max allowed length that a WAYD message can have
// Optional, default value is 1024
// wParam = 0
// lParam = 0
// Returns the max length
#define PS_GETMYWAYDMAXLENGTH "/GetMyWAYDMaxLength"

// Get the unread email message count, optional
// wParam = 0
// lParam = 0
// Returns the number of unread emails
#define PS_GETUNREADEMAILCOUNT "/GetUnreadEmailCount"

/****************************** SENDING SERVICES *************************/
//these should be called with CallContactService()

//Updates a contact's details from the server
//wParam = flags
//lParam = 0
//returns 0 on success, nonzero on failure
//Will update all the information in the database, and then send acks with
//type = ACKTYPE_GETINFO, result = ACKRESULT_SUCCESS, hProcess = (HANDLE)(int)nReplies, lParam = thisReply
//Since some protocols do not allow the library to tell when it has got all
//the information so it can send a final ack, one ack will be sent after each
//chunk of data has been received. nReplies contains the number of distinct
//acks that will be sent to get all the information, thisReply is the zero-
//based index of this ack. When thisReply = 0 the 'minimal' information has just
//been received. All other numbering is arbitrary.
#define SGIF_MINIMAL   1     //get only the most basic information. This should
                             //contain at least a Nick and e-mail.
#define SGIF_ONOPEN    2     //set when the User Info form is being opened
#define PSS_GETINFO      "/GetInfo"

//Send an instant message
//wParam = flags
//lParam = (LPARAM)(const char*)szMessage
//returns a hProcess corresponding to the one in the ack event.
//Will send an ack when the message actually gets sent
//type = ACKTYPE_MESSAGE, result = success/failure, (char*)lParam = error message or NULL.
//Protocols modules are free to define flags starting at 0x10000
//The event will *not* be added to the database automatically.
#define PSS_MESSAGE      "/SendMsg"

//Send an URL message
//wParam = flags
//lParam = (LPARAM)(const char*)szMessage
//returns a hProcess corresponding to the one in the ack event.
//szMessage should be encoded as the URL followed by the description, the
//separator being a single nul (\0). If there is no description, do not forget
//to end the URL with two nuls.
//Will send an ack when the message actually gets sent
//type = ACKTYPE_URL, result = success/failure, (char*)lParam = error message or NULL.
//Protocols modules are free to define flags starting at 0x10000
//The event will *not* be added to the database automatically.
#define PSS_URL          "/SendUrl"

//Send a set of contacts
//wParam = MAKEWPARAM(flags, nContacts)
//lParam = (LPARAM)(HANDLE*)hContactsList
//returns a hProcess corresponding to the one in the ack event, NULL on
//failure.
//hContactsList is an array of nContacts handles to contacts. If this array
//includes one or more contacts that cannot be transferred using this protocol
//the function will fail.
//Will send an ack when the contacts actually get sent
//type = ACKTYPE_CONTACTS, result = success/failure, (char*)lParam = error message or NULL.
//No flags have yet been defined.
//The event will *not* be added to the database automatically.
#define PSS_CONTACTS          "/SendContacts"

//Send a request to retrieve somebody's mode message.
//wParam = lParam = 0
//returns an hProcess identifying the request, or 0 on failure
//This function will fail if the contact's current status mode doesn't have an
//associated message
//The reply will be in the form of an ack:
//type = ACKTYPE_AWAYMSG, result = success/failure, lParam = (const char*)szMessage
#define PSS_GETAWAYMSG    "/GetAwayMsg"

//Allows a file transfer to begin
//wParam = (WPARAM)(HANDLE)hTransfer
//lParam = (LPARAM)(const TCHAR*)szPath
//Returns a new handle to the transfer, to be used from now on
//If szPath does not point to a directory then:
//  if a single file is being transferred and the protocol supports file
//    renaming (PF1_CANRENAMEFILE) then the file is given this name
//  otherwise the filename is removed and the file(s) are placed in the
//    resulting directory
//File transfers are marked by an EVENTTYPE_FILE added to the database. The
//format is:
//DWORD hTransfer
//ASCIIZ filename(s), description
#define PSS_FILEALLOW   "/FileAllow"
#define PSS_FILEALLOWW  "/FileAllowW"

//Refuses a file transfer request
//wParam = (WPARAM)(HANDLE)hTransfer
//lParam = (LPARAM)(const TCHAR*)szReason
//Returns 0 on success, nonzero on failure
#define PSS_FILEDENY    "/FileDeny"
#define PSS_FILEDENYW   "/FileDenyW"

//Cancel an in-progress file transfer
//wParam = (WPARAM)(HANDLE)hTransfer
//lParam = 0
//Returns 0 on success, nonzero on failure
#define PSS_FILECANCEL  "/FileCancel"

//Initiate a file send
//wParam = (WPARAM)(const TCHAR*)szDescription
//lParam = (LPARAM)(TCHAR **)ppszFiles
//Returns a transfer handle on success, NULL on failure
//All notification is done through acks, with type = ACKTYPE_FILE
//If result = ACKRESULT_FAILED then lParam = (LPARAM)(const char*)szReason
#define PSS_FILE    "/SendFile"
#define PSS_FILEW   "/SendFileW"

//Set the status mode you will appear in to a user
//wParam = statusMode
//lParam = 0
//Returns 0 on success, nonzero on failure
//Set statusMode = 0 to revert to normal behaviour for the contact
//ID_STATUS_ONLINE is possible iff PF1_VISLIST
//ID_STATUS_OFFLINE is possible iff PF1_INVISLIST
//Other modes are possible iff PF1_INDIVSTATUS
#define PSS_SETAPPARENTMODE  "/SetApparentMode"

// Send an auth request
// wParam = 0
// lParam = (const TCHAR *)szMessage
// Returns 0 on success, nonzero on failure
#define PSS_AUTHREQUEST    "/AuthRequest"
#define PSS_AUTHREQUESTW   "/AuthRequestW"

// Send "User is Typing" (user is typing a message to the user) v0.3.3+
// wParam = (MCONTACT)hContact
// lParam = (LPARAM)(int)typing type - see PROTOTYPE_SELFTYPING_X defines in m_protocols.h
#define PSS_USERISTYPING   "/UserIsTyping"

/**************************** RECEIVING SERVICES *************************/
//These services are not for calling by general modules. They serve a specific
//role in communicating through protocol module chains before the whole app is
//notified that an event has occurred.
//When the respective event is received over the network, the network-level
//protocol module initiates the chain by calling MS_PROTO_CHAINRECV with wParam
//set to zero and lParam pointing to the CCSDATA structure.
//Protocol modules should continue the message up the chain by calling
//MS_PROTO_CHAINRECV with the same wParam they received and a modified (or not)
//lParam (CCSDATA). If they do not do this and return nonzero then all further
//processing for the event will cease and the event will be ignored.
//Once all non-network protocol modules have been called (in reverse order),
//the network protocol module will be called so that it can finish its
//processing using the modified information.
//This final processing should consist of the protocol module adding the
//event to the database, and it is the ME_DB_EVENT_ADDED event that people who
//just want to know the final result should hook.
//In all cases, the database should store what the user read/wrote.

//An instant message has been received
//wParam = 0
//lParam = (LPARAM)(PROTORECVEVENT*)&pre
//DB event: EVENTTYPE_MESSAGE, blob contains szMessage without 0 terminator
//Return 0 - success, other failure
typedef struct {
	DWORD  flags;
	DWORD  timestamp;   // unix time
	char  *szMessage;   // message body in utf8
	LPARAM lParam;      // extra space for the network level protocol module
	void  *pCustomData;
	DWORD  cbCustomDataSize;
} PROTORECVEVENT;

#define PREF_CREATEREAD   1     //create the database event with the 'read' flag set
#define PREF_RTL          4     // 0.5+ addition: support for right-to-left messages
#define PREF_SENT        16     // message will be created with the DBEF_SENT flag

/* Proto/RecvMessage
Copies a message from a PROTORECVEVENT event into the database
  wParam = 0 (unused)
  lParam = CCSDATA*
Returns the result of db_event_add()
*/
#define PSR_MESSAGE   "/RecvMessage"

#define MS_PROTO_RECVMSG "Proto/RecvMessage"

__forceinline INT_PTR Proto_RecvMessage(MCONTACT hContact, PROTORECVEVENT *pcre)
{
	CCSDATA ccs = { hContact, PSR_MESSAGE, 0, (LPARAM)pcre };
	return CallService(MS_PROTO_RECVMSG, 0, (LPARAM)&ccs );
}

/* Proto/AuthRecv
Copies the EVENTTYPE_AUTHREQUEST event from PROTORECVEVENT into DBEVENTINFO and adds it
  wParam = char* : protocol name
  lParam = PROTORECVEVENT*
Returns the result of db_event_add()
*/
#define PSR_AUTH		"/RecvAuth"

#define MS_PROTO_AUTHRECV  "Proto/AuthRecv"

__forceinline INT_PTR Proto_AuthRecv(const char *szProtoName, PROTORECVEVENT *pcre)
{	return CallService(MS_PROTO_AUTHRECV, (WPARAM)szProtoName, (LPARAM)pcre);
}

// File(s) have been received
// wParam = 0
// lParam = (LPARAM)(PROTORECVFILET*)&prf

#define PRFF_UNICODE 1
#if defined(_UNICODE)
	#define PRFF_TCHAR PRFF_UNICODE
#else
	#define PRFF_TCHAR 0
#endif

typedef struct {
	DWORD dwFlags;       // PRFF_*
	DWORD timestamp;     // unix time
	union {
		char  *szDescription;   // utf8
		TCHAR *tszDescription;  // TCHAR*, if PRFF_TCHAR is specified
	};
	int fileCount;
	union {
		char  **pszFiles;  // utf8
		TCHAR **ptszFiles; // TCHAR*, if PRFF_TCHAR is specified
	};
	LPARAM lParam;       // extra space for the network level protocol module
} PROTORECVFILET;

#define PSR_FILE       "/RecvFile"

#define MS_PROTO_RECVFILET "Proto/RecvFileT"

__forceinline INT_PTR Proto_RecvFile(MCONTACT hContact, PROTORECVFILET *pcre)
{
	CCSDATA ccs = { hContact, PSR_FILE, 0, (LPARAM)pcre };
	return CallService(MS_PROTO_RECVFILET, 0, ( LPARAM )&ccs);
}

// An URL has been received
// wParam = 0
// lParam = (LPARAM)(PROTORECVEVENT*)&pre
// szMessage is encoded the same as for PSS_URL
// DB event: EVENTTYPE_URL, blob contains szMessage without 0 terminator
#define PSR_URL       "/RecvUrl"

// Contacts have been received
// wParam = 0
// lParam = (LPARAM)(PROTORECVEVENT*)&pre
// pre.szMessage is actually a (PROTOSEARCHRESULT**) list.
// pre.lParam is the number of contacts in that list.
// pre.flags can contain PREF_UTF defining the strings as utf-8 encoded (0.7.0+)
// PS_ADDTOLIST can be used to add the contacts to the contact list.
#define PSR_CONTACTS       "/RecvContacts"

/* contacts database event format (EVENTTYPE_CONTACTS)
repeat {
  ASCIIZ userNick
  ASCIIZ userId
}
userNick should be a human-readable description of the user. It need not
be the nick, or even confined to displaying just one type of
information. The dbe.flags can contain DBEF_UTF defining userNick as utf-8
encoded.
userId should be a machine-readable representation of the unique
protocol identifying field of the user. Because of the need to be
zero-terminated, binary data should be converted to text.
Use PS_ADDTOLISTBYEVENT to add the contacts from one of these to the list.
*/

// An away message reply has been received
// wParam = statusMode
// lParam = (LPARAM)(PROTORECVEVENT*)&pre
#define PSR_AWAYMSG    "/RecvAwayMsg"

#ifdef _UNICODE

#define PS_SETAWAYMSGT    PS_SETAWAYMSGW
#define PSS_FILET         PSS_FILEW
#define PSS_FILEALLOWT    PSS_FILEALLOWW
#define PSS_FILEDENYT     PSS_FILEDENYW
#define PSS_AUTHREQUESTT  PSS_AUTHREQUESTW
#define PS_AUTHDENYT      PS_AUTHDENYW
#define PS_FILERESUMET    PS_FILERESUMEW
#define PS_BASICSEARCHT   PS_BASICSEARCHW
#define PS_SEARCHBYNAMET  PS_SEARCHBYNAMEW
#define PS_SEARCHBYEMAILT PS_SEARCHBYEMAILW

#else

#define PS_SETAWAYMSGT    PS_SETAWAYMSG
#define PSS_FILET         PSS_FILE
#define PSS_FILEALLOWT    PSS_FILEALLOW
#define PSS_FILEDENYT     PSS_FILEDENY
#define PSS_AUTHREQUESTT  PSS_AUTHREQUEST
#define PS_AUTHDENYT      PS_AUTHDENY
#define PS_FILERESUMET    PS_FILERESUME
#define PS_BASICSEARCHT   PS_BASICSEARCH
#define PS_SEARCHBYNAMET  PS_SEARCHBYNAME
#define PS_SEARCHBYEMAILT PS_SEARCHBYEMAIL

#endif

#endif  // M_PROTOSVC_H__

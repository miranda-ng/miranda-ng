/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)
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

// this module was created in v0.1.1.0

#ifndef M_PROTOCOLS_H__
#define M_PROTOCOLS_H__ 1

struct PROTO_INTERFACE;

#include "statusmodes.h"

#include <m_core.h>
#include <m_system.h>
#include <m_genmenu.h>

struct CCSDATA
{
	MCONTACT hContact;
	const char *szProtoService;   // a PS_ constant
	WPARAM wParam;
	LPARAM lParam;
};

/////////////////////////////////////////////////////////////////////////////////////////
// a general network 'ack'
// wParam = 0
// lParam = (LPARAM)(ACKDATA*)&ack
// Note that just because definitions are here doesn't mean they will be sent.
// Read the documentation for the function you are calling to see what replies
// you will receive.

#define ACKTYPE_MESSAGE    0
#define ACKTYPE_FILE       2
#define ACKTYPE_CHAT       3
#define ACKTYPE_AWAYMSG    4
#define ACKTYPE_AUTHREQ    5
#define ACKTYPE_ADDED      6
#define ACKTYPE_GETINFO    7
#define ACKTYPE_SETINFO    8
#define ACKTYPE_LOGIN      9
#define ACKTYPE_SEARCH     10
#define ACKTYPE_NEWUSER    11
#define ACKTYPE_STATUS     12
#define ACKTYPE_CONTACTS   13	// send/recv of contacts
#define ACKTYPE_AVATAR	   14 // send/recv of avatars from a protocol
#define ACKTYPE_EMAIL      15	// notify if the unread emails changed

#define ACKRESULT_SUCCESS      0
#define ACKRESULT_FAILED       1

// 'in progress' result codes:
#define ACKRESULT_CONNECTING   100
#define ACKRESULT_CONNECTED    101
#define ACKRESULT_INITIALISING 102
#define ACKRESULT_SENTREQUEST  103  // waiting for reply...
#define ACKRESULT_DATA         104  // blob of file data sent/recved, or search result
#define ACKRESULT_NEXTFILE     105  // file transfer went to next file
#define ACKRESULT_FILERESUME   106  // a file is about to be received, see PS_FILERESUME
#define ACKRESULT_DENIED       107  // a file send has been denied
#define ACKRESULT_STATUS       108  // an ack or a series of acks to do with a task have a status change
#define ACKRESULT_LISTENING    109  // waiting for connection
#define ACKRESULT_CONNECTPROXY 110  // connecting to file proxy
#define ACKRESULT_SEARCHRESULT 111  // result of extended search

struct ACKDATA
{
	const char *szModule;  // the name of the protocol module which initiated this ack
	MCONTACT hContact;
	int type;              // an ACKTYPE_ constant
	int result;            // an ACKRESULT_ constant
	HANDLE hProcess;       // a caller-defined process code
	LPARAM lParam;	        // caller-defined extra info
};

#define ME_PROTO_ACK "Proto/Ack"

// When result is ACKRESULT_FAILED or ACKRESULT_DENIED, lParam can point to
// a human readable string with an explanation. For example: "The message was too
// long to be delivered". If no error message is specified, lParam must be NULL.
// Right now only explanations from ACKTYPE_MESSAGE is shown.

// when type == ACKTYPE_FILE && (result == ACKRESULT_DATA || result == ACKRESULT_FILERESUME),
// lParam points to this

#define PFTS_RECEIVING 0
#define PFTS_SENDING   1
#define PFTS_UNICODE   2
#define PFTS_UTF       4

struct PROTOFILETRANSFERSTATUS
{
	MCONTACT        hContact;
	DWORD           flags;      // one of PFTS_* constants
	MAllStringArray pszFiles;
	int             totalFiles;
	int             currentFileNumber;
	uint64_t        totalBytes;
	uint64_t        totalProgress;
	MAllStrings     szWorkingDir;
	MAllStrings     szCurrentFile;
	uint64_t        currentFileSize;
	uint64_t        currentFileProgress;
	uint64_t        currentFileTime;  // as seconds since 1970
};

/////////////////////////////////////////////////////////////////////////////////////////
// For recv, it will go from lower to higher, so in this case:
// check ignore, decrypt (encryption), translate
// 
// For send, it will go translate, encrypt, ignore(??), send
// 
// The DB will store higher numbers here, LOWER in the protocol chain, and lower numbers
// here HIGHER in the protocol chain

#define PROTOTYPE_IGNORE          50    // added during v0.3.3
#define PROTOTYPE_PROTOCOL      1000	 // old style protocol
#define PROTOTYPE_VIRTUAL       1001    // virtual protocol (has no accounts)
#define PROTOTYPE_PROTOWITHACCS 1002	 // new style protocol
#define PROTOTYPE_ENCRYPTION    2000
#define PROTOTYPE_FILTER        3000
#define PROTOTYPE_TRANSLATION   4000
#define PROTOTYPE_OTHER        10000    // avoid using this if at all possible

struct PROTOCOLDESCRIPTOR
{
	char *szName;        // unique name of the module
	int   type;          // module type, see PROTOTYPE_ constants
};

/////////////////////////////////////////////////////////////////////////////////////////
// Enumerate the currently running protocols
// Returns 0 on success, nonzero on failure
// Neither wParam nor lParam may be NULL
// The list returned by this service is the protocol modules currently installed
// and running. It is not the complete list of all protocols that have ever been
// installed.
// IMPORTANT NOTE #1: the list returned is not static, it may be changed in the
// program's lifetime. Do not use this list in the global context, copy protocols
// names otherwise.
// IMPORTANT NOTE #2: in version 0.8 this service is mapped to the MS_PROTO_ENUMACCOUNTS
// service to provide the compatibility with old plugins (first three members of
// PROTOACCOUNT are equal to the old PROTOCOLDESCRIPTOR format). If you declare the
// MIRANDA_VER macro with value greater or equal to 0x800, use MS_PROTO_ENUMPROTOS
// service instead to obtain the list of running protocols instead of accounts.
// Note that a protocol module need not be an interface to an Internet server,
// they can be encryption and loads of other things, too.
// And yes, before you ask, that is triple indirection. Deal with it.
// Access members using ppProtocolDescriptors[index]->element

EXTERN_C MIR_APP_DLL(void) Proto_EnumProtocols(int *nProtos, PROTOCOLDESCRIPTOR ***pProtos);

/////////////////////////////////////////////////////////////////////////////////////////
// determines if a protocol module is loaded or not
// Returns a pointer to the PROTOCOLDESCRIPTOR if the protocol is loaded, or
// NULL if it isn't.

EXTERN_C MIR_APP_DLL(PROTOCOLDESCRIPTOR*) Proto_IsProtocolLoaded(const char *szProtoName);

/////////////////////////////////////////////////////////////////////////////////////////
// gets a name of a variable inside a protocol's module that identifies a contact

EXTERN_C MIR_APP_DLL(const char*) Proto_GetUniqueId(const char *szModuleName);

/////////////////////////////////////////////////////////////////////////////////////////
// sets a name of a variable inside a protocol's module that identifies a contact

EXTERN_C MIR_APP_DLL(void) Proto_SetUniqueId(const char *szProtoName, const char *szUniqueId);

/////////////////////////////////////////////////////////////////////////////////////////
// determines whether the specified contact has the given protocol in its chain
// Returns -1 if it is base protocol, positive number if it is filter and 0 if it doesn't

EXTERN_C MIR_APP_DLL(int) Proto_IsProtoOnContact(MCONTACT hContact, const char *szProto);

/////////////////////////////////////////////////////////////////////////////////////////
// This service is for notifying protocols that the user is typing a message v0.3.3+
// in a message dialog.
// This is typically sent by a message dialog when a user in the clist is typing.
// wParam = (MCONTACT)hContact
// lParam = (LPARAM)(int)typing state
// NOTE: Only protocols should generally call this service

#define PROTOTYPE_SELFTYPING_OFF      0
#define PROTOTYPE_SELFTYPING_ON       1

#define MS_PROTO_SELFISTYPING "Proto/SelfIsTyping"

/////////////////////////////////////////////////////////////////////////////////////////
// This service is for notifying message dialogs/other plugins of a user typing. v0.3.3+
// This is typically sent by a protocol when a user in the clist is typing.
// wParam = (MCONTACT)hContact
// lParam = (LPARAM)(int)time (secs)
// NOTE: The time in seconds is used to tell a message dialog (or other plugin)
// how long to display its notification.  If time is 0, then notification
// of typing ends.
// NOTE: Only protocols should generally call this service

#define PROTOTYPE_CONTACTTYPING_OFF      0
#define PROTOTYPE_CONTACTTYPING_INFINITE 2147483647

#define MS_PROTO_CONTACTISTYPING "Proto/ContactIsTyping"

/////////////////////////////////////////////////////////////////////////////////////////
// This hook notifies when a user is typing.  If a message dialog supports sending v0.3.3+
// typing notifications it should hook this event and fire the
// ProtoService PSS_USERISTYPING to the contacts protocol *after* verifying
// that the hContact is not NULL and the the user wishes to send notifications
// to this user (checked visibility, individual typing blocking, etc).
// wParam = (MCONTACT)hContact
// lParam = (LPARAM)(int)typing state

#define ME_PROTO_CONTACTISTYPING "Proto/ContactIsTypingEvent"

/////////////////////////////////////////////////////////////////////////////////////////
// Accounts support

struct MIR_APP_EXPORT PROTOACCOUNT : public MZeroedObject
{
	char*    szModuleName;    // unique physical account name (matches database module name)
	wchar_t* tszAccountName;  // user-defined account name
	char*    szProtoName;     // physical protocol name
	bool     bIsEnabled;      // is account enabled?
	bool     bIsVisible;      // is account visible?
	bool     bIsVirtual;      // is account virtual?
	bool     bIsLocked;		  // is locked in the contacts list?
	bool     bOldProto;       // old-styled account (one instance per dll)
	bool     bDynDisabled;    // dynamic disable flag, is never written to db

	bool     bAccMgrUIChanged;
	HWND     hwndAccMgrUI;

	int      iOrder;          // account order in various menus & lists
	PROTO_INTERFACE *ppro;    // pointer to the underlying object

	char*    szUniqueId;      // setting's unique id for any contact in the account
	int      iIconBase;       // index of the first icon in ClistImages
	int      iRealStatus;     // last status reported by protocol

	int      protoindex;
	HGENMENU menuhandle[MAX_STATUS_COUNT];

	//---- methods ------------
	PROTOACCOUNT(const char *szProto);
	~PROTOACCOUNT();

	bool     IsEnabled(void) const;
	bool     IsLocked(void) const;
	bool     IsVisible(void) const;
};

/////////////////////////////////////////////////////////////////////////////////////////
// account enumeration service
// wParam = (WPARAM)(int*)piNumAccounts
// lParam = (LPARAM)(PROTOACCOUNT**)paAccounts

EXTERN_C MIR_APP_DLL(void) Proto_EnumAccounts(int *nAccs, PROTOACCOUNT ***pAccs);

MIR_APP_DLL(LIST<PROTOACCOUNT>&) Accounts(void);

/////////////////////////////////////////////////////////////////////////////////////////
// creates new account
// return value = PROTOACCOUNT* or NULL

EXTERN_C MIR_APP_DLL(PROTOACCOUNT*) Proto_CreateAccount(const char *pszInternal, const char *pszBaseProto, const wchar_t *ptszAccountName);

/////////////////////////////////////////////////////////////////////////////////////////
// calculates the average protocol status among all visible & active accounts
// returns the common status or -1 if statuses differ

EXTERN_C MIR_APP_DLL(int) Proto_GetAverageStatus(int *pAccountNumber = nullptr);

/////////////////////////////////////////////////////////////////////////////////////////
// retrieves an account's interface by its physical name (database module)
// returns PROTOACCOUNT* or NULL on error

EXTERN_C MIR_APP_DLL(PROTOACCOUNT*) Proto_GetAccount(const char *pszModuleName);

/////////////////////////////////////////////////////////////////////////////////////////
// retrieves an account's interface associated with a contact's id
// returns PROTOACCOUNT* or NULL on error

EXTERN_C MIR_APP_DLL(PROTOACCOUNT*) Proto_GetContactAccount(MCONTACT hContact);

/////////////////////////////////////////////////////////////////////////////////////////
// returns last status reported by a protocol

EXTERN_C MIR_APP_DLL(int) Proto_GetStatus(const char *pszModuleName);

/////////////////////////////////////////////////////////////////////////////////////////
// this event is fired when the accounts list gets changed
// wParam = event type (1 - added, 2 - changed, 3 - deleted, 4 - upgraded, 5 - enabled/disabled)
// lParam = (LPARAM)(PROTOACCOUNT*) - account being changed

#define PRAC_ADDED    1
#define PRAC_CHANGED  2
#define PRAC_REMOVED  3
#define PRAC_UPGRADED 4
#define PRAC_CHECKED  5

#define ME_PROTO_ACCLISTCHANGED "Proto/AccListChanged"

/////////////////////////////////////////////////////////////////////////////////////////
// displays the Account Manager
// wParam = 0
// lParam = 0

#define MS_PROTO_SHOWACCMGR "Protos/ShowAccountManager"

/////////////////////////////////////////////////////////////////////////////////////////
// gets the account associated with a contact
// Returns a char* pointing to the asciiz name of the protocol or NULL if the
// contact has no protocol. There is no need to mir_free() it or anything.
// This is the name of the module that actually accesses the network for that
// contact.

EXTERN_C MIR_APP_DLL(char*) Proto_GetBaseAccountName(MCONTACT);

/* -------------- avatar support ---------------------

First a protocol must report it supports PF4_AVATARS via PS_GETCAPS, secondly
it will return the following acks for certain events, the protocol must use
ProtoBroadcastAck(), listeners must hook ME_PROTO_ACK, note that lParam = ACKDATA*

*/

/*

  The following ACKs MUST be sent in order of .result via ProtoBroadcastAck()

	.szModule = protocol module
	.hContact = contact the avatar is for, or 0 if its for the user
	.type = ACKTYPE_AVATAR
	.result = ACKRESULT_CONNECTING, ACKRESULT_CONNECTED, ACKRESULT_SENTREQUEST, ACKRESULT_DATA
	.hProcess = protocol specific
	.lParam = 0

  Once the transfer is complete the following ACKs MUST be sent by the protocol

  .result = ACKRESULT_SUCCESS or ACKRESULT_FAILED
  .hProcess = (HANDLE) &PROTO_AVATAR_INFORMATION
  .lParam = 0;

  Anytime before or during the ack or series of acks ACKRESULT_STATUS maybe sent, this might
  be sent a long while before anything else happens (or after anything happening) For avatars
  it allows callers to identify status information to do with the avatar, including the time
  before background transport has been created, so that you know an avatar will be exchanged
  sometime in the future.

  When ACKRESULT_STATUS, hProcess is the result code - the value of this is protocol dependent,
  See protocol documentation to do with avatars for what codes are defined and what they mean.

*/

#define PA_FORMAT_UNKNOWN  0	// the protocol can not determine much about the "bitmap"
#define PA_FORMAT_PNG      1	// the image is PNG
#define PA_FORMAT_JPEG     2
#define PA_FORMAT_ICON     3
#define PA_FORMAT_BMP      4
#define PA_FORMAT_GIF      5
#define PA_FORMAT_SWF      6
#define PA_FORMAT_XML      7
#define PA_FORMAT_WEBP     8

typedef struct {
	MCONTACT hContact;         // this might have to be set by the caller too
	int format;                // PA_FORMAT_*
	wchar_t filename[MAX_PATH];  // full path to filename which contains the avatar
} PROTO_AVATAR_INFORMATION;

#define GAIF_FORCE 1          // force an update of the avatar if there is none

#define GAIR_SUCCESS  0       // information about the avatar has been returned
#define GAIR_WAITFOR  1       // you must hook ME_PROTO_ACK and wait for replies about avatar status
#define GAIR_NOAVATAR 2       // sorry, this contact has no avatars

/*
	wParam : GAIF_*
	lParam : (LPARAM) &PROTO_AVATAR_INFORMATION
	Affect : Make a request to the protocol to return information about a hContact's avatar (or main user)
			 it can return information, tell you there is none, or if forced start requesting an avatar.
*/

#define PS_GETAVATARINFO "/GetAvatarInformation"

/////////////////////////////////////////////////////////////////////////////////////////
// notify the protocol manager that you're around
// wParam = 0
// lParam = (PROTOCOLDESCRIPTOR*)&descriptor
// returns 0 on success, nonzero on failure
// This service must be called in your module's Load(void) routine.
// descriptor.type can be a value other than the PROTOTYPE_ constants specified
// above to provide more precise positioning information for the contact
// protocol lists. It is strongly recommended that you give values relative to
// the constants, however, by adding or subtracting small integers ( <= 100).
// PROTOTYPE_PROTOCOL modules must not do this. The value must be exact.
// See MS_PROTO_ENUMPROTOCOLS for more notes.

EXTERN_C MIR_APP_DLL(PROTOCOLDESCRIPTOR*) Proto_RegisterModule(int type, const char *szName);

/////////////////////////////////////////////////////////////////////////////////////////
// adds the specified protocol module to the chain for a contact
// returns 0 on success, nonzero on failure
// The module is added in the correct position according to the type given when
// it was registered.

EXTERN_C MIR_APP_DLL(int) Proto_AddToContact(MCONTACT, const char *szProto);

/////////////////////////////////////////////////////////////////////////////////////////
// removes the specified protocol module from the chain for a contact
// wParam = (MCONTACT)hContact
// lParam = (LPARAM)(const char*)szName
// returns 0 on success, nonzero on failure

EXTERN_C MIR_APP_DLL(int) Proto_RemoveFromContact(MCONTACT, const char *szProto);

/////////////////////////////////////////////////////////////////////////////////////////
// Call the next service in the chain for this send operation
// The return value should be returned immediately
// iOrder and ccs should be passed as the parameters that your service was
// called with. iOrder must remain untouched but ccs is a CCSDATA structure
// that can be copied and modified if needed.
// Typically, the last line of any chaining protocol function is
// return Proto_ChainSend(iOrder, ccs);

EXTERN_C MIR_APP_DLL(INT_PTR) Proto_ChainSend(int iOrder, CCSDATA *ccs);

__forceinline INT_PTR ProtoChainSend(MCONTACT hContact, const char *szProtoService, WPARAM wParam, LPARAM lParam)
{
	CCSDATA ccs = { hContact, szProtoService, wParam, lParam };
	return Proto_ChainSend(0, &ccs);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Call the next service in the chain for this receive operation
// The return value should be returned immediately
// iOrder and ccs should be passed as the parameters that your service was
// called with. iOrder must remain untouched but ccs is a CCSDATA structure
// that can be copied and modified if needed.
// When being initiated by the network-access protocol module, wParam should be zero.
// Thread safety: ms_proto_chainrecv is completely thread safe since 0.1.2.0
// Calls to it are translated to the main thread and passed on from there. The
// function will not return until all callees have returned, irrepective of
// differences between threads the functions are in.

EXTERN_C MIR_APP_DLL(INT_PTR) Proto_ChainRecv(int iOrder, CCSDATA *ccs);

__forceinline INT_PTR ProtoChainRecv(MCONTACT hContact, const char *szService, WPARAM wParam, LPARAM lParam)
{
	CCSDATA ccs = { hContact, szService, wParam, lParam };
	return Proto_ChainRecv(0, &ccs);
}

#endif // M_PROTOCOLS_H

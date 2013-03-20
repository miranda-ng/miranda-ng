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

//this module was created in v0.1.1.0

#ifndef M_PROTOCOLS_H__
#define M_PROTOCOLS_H__ 1

#include "statusmodes.h"

//call a specific protocol service. See the PS_ constants in m_protosvc.h
#if MIRANDA_VER < 0x800
__inline static INT_PTR CallProtoService(const char *szModule,const char *szService,WPARAM wParam,LPARAM lParam)
{
	char str[MAXMODULELABELLENGTH];
	_snprintf(str, sizeof(str), "%s%s", szModule, szService);
    str[MAXMODULELABELLENGTH-1] = 0;
	return CallService(str,wParam,lParam);
}
#endif

//send a general request through the protocol chain for a contact
//wParam=0
//lParam=(LPARAM)(CCSDATA*)&ccs
//returns the value as documented in the PS_ definition (m_protosvc.h)

typedef struct {
	HANDLE hContact;
	const char *szProtoService;   //a PS_ constant
	WPARAM wParam;
	LPARAM lParam;
} CCSDATA;

#define MS_PROTO_CALLCONTACTSERVICE    "Proto/CallContactService"

#if MIRANDA_VER < 0x800
__inline static INT_PTR CallContactService(HANDLE hContact,const char *szProtoService,WPARAM wParam,LPARAM lParam)
{
	CCSDATA ccs;
	ccs.hContact=hContact;
	ccs.szProtoService=szProtoService;
	ccs.wParam=wParam;
	ccs.lParam=lParam;
	return CallService(MS_PROTO_CALLCONTACTSERVICE,0,(LPARAM)&ccs);
}
#endif

//a general network 'ack'
//wParam=0
//lParam=(LPARAM)(ACKDATA*)&ack
//Note that just because definitions are here doesn't mean they will be sent.
//Read the documentation for the function you are calling to see what replies
//you will receive.
typedef struct {
	int cbSize;
	const char *szModule;  //the name of the protocol module which initiated this ack
	HANDLE hContact;
	int type;     //an ACKTYPE_ constant
	int result; 	//an ACKRESULT_ constant
	HANDLE hProcess;   //a caller-defined process code
	LPARAM lParam;	   //caller-defined extra info
} ACKDATA;
#define ACKTYPE_MESSAGE    0
#define ACKTYPE_URL        1
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
#define ACKTYPE_CONTACTS   13	//send/recv of contacts
#define ACKTYPE_AVATAR	   14 //send/recv of avatars from a protocol
#define ACKTYPE_EMAIL      15	//notify if the unread emails changed

#define ACKRESULT_SUCCESS      0
#define ACKRESULT_FAILED       1

//'in progress' result codes:
#define ACKRESULT_CONNECTING   100
#define ACKRESULT_CONNECTED    101
#define ACKRESULT_INITIALISING 102
#define ACKRESULT_SENTREQUEST  103  //waiting for reply...
#define ACKRESULT_DATA         104  //blob of file data sent/recved, or search result
#define ACKRESULT_NEXTFILE     105  //file transfer went to next file
#define ACKRESULT_FILERESUME   106  //a file is about to be received, see PS_FILERESUME
#define ACKRESULT_DENIED       107  //a file send has been denied
#define ACKRESULT_STATUS	    108  //an ack or a series of acks to do with a task have a status change
#define ACKRESULT_LISTENING    109  //waiting for connection
#define ACKRESULT_CONNECTPROXY 110 //connecting to file proxy
#define ACKRESULT_SEARCHRESULT 111 //result of extended search

#define ME_PROTO_ACK       "Proto/Ack"

// v0.3.2+: When result is ACKRESULT_FAILED or ACKRESULT_DENIED, lParam can point to
// a human readable string with an explanation. For example: "The message was too
// long to be delivered". If no error message is specified, lParam must be NULL.
// Right now only explanations from ACKTYPE_MESSAGE is shown.

//when type==ACKTYPE_FILE && (result==ACKRESULT_DATA || result==ACKRESULT_FILERESUME),
//lParam points to this

#if MIRANDA_VER >= 0x0900
	#define FNAMECHAR TCHAR
#else
	#define FNAMECHAR char
#endif

#define PFTS_RECEIVING 0
#define PFTS_SENDING   1
#define PFTS_UNICODE   2
#define PFTS_UTF       4

#if defined( _UNICODE )
	#define PFTS_TCHAR  PFTS_UNICODE
#else
	#define PFTS_TCHAR  0
#endif

typedef struct tagPROTOFILETRANSFERSTATUS_V1 
{
	size_t cbSize;
	HANDLE hContact;
	int    sending;
    char **files;
	int totalFiles;
	int currentFileNumber;
	unsigned long totalBytes;
	unsigned long totalProgress;
    char *workingDir;
    char *currentFile;
	unsigned long currentFileSize;
	unsigned long currentFileProgress;
	unsigned long currentFileTime;  //as seconds since 1970
} 
PROTOFILETRANSFERSTATUS_V1;

#if MIRANDA_VER < 0x0900

typedef PROTOFILETRANSFERSTATUS_V1 PROTOFILETRANSFERSTATUS;

#else

typedef struct tagPROTOFILETRANSFERSTATUS 
{
	size_t cbSize;
	HANDLE hContact;
	DWORD  flags;      // one of PFTS_* constants

    union {
  	  char **pszFiles;
      TCHAR **ptszFiles;
      WCHAR **pwszFiles;
    };

    int totalFiles;
	int currentFileNumber;
	unsigned __int64 totalBytes;
	unsigned __int64 totalProgress;

    union {
	   char *szWorkingDir;
      TCHAR *tszWorkingDir;
      WCHAR *wszWorkingDir;
    };

    union {
  	  char *szCurrentFile;
      TCHAR *tszCurrentFile;
      WCHAR *wszCurrentFile;
    };

	unsigned __int64 currentFileSize;
	unsigned __int64 currentFileProgress;
	unsigned __int64 currentFileTime;  //as seconds since 1970
} 
PROTOFILETRANSFERSTATUS;

#endif

//Enumerate the currently running protocols
//wParam=(WPARAM)(int*)&numberOfProtocols
//lParam=(LPARAM)(PROTOCOLDESCRIPTOR***)&ppProtocolDescriptors
//Returns 0 on success, nonzero on failure
//Neither wParam nor lParam may be NULL
//The list returned by this service is the protocol modules currently installed
//and running. It is not the complete list of all protocols that have ever been
//installed.
//IMPORTANT NOTE #1: the list returned is not static, it may be changed in the
//program's lifetime. Do not use this list in the global context, copy protocols
//names otherwise.
//IMPORTANT NOTE #2: in version 0.8 this service is mapped to the MS_PROTO_ENUMACCOUNTS
//service to provide the compatibility with old plugins (first three members of 
//PROTOACCOUNT are equal to the old PROTOCOLDESCRIPTOR format). If you declare the
//MIRANDA_VER macro with value greater or equal to 0x800, use MS_PROTO_ENUMPROTOS
//service instead to obtain the list of running protocols instead of accounts.
//Note that a protocol module need not be an interface to an Internet server,
//they can be encryption and loads of other things, too.
//And yes, before you ask, that is triple indirection. Deal with it.
//Access members using ppProtocolDescriptors[index]->element

#define PROTOCOLDESCRIPTOR_V3_SIZE (sizeof(size_t)+sizeof(INT_PTR)+sizeof(char*))

 // initializes an empty account
typedef struct tagPROTO_INTERFACE* ( *pfnInitProto )( const char* szModuleName, const TCHAR* szUserName );

// deallocates an account instance
typedef int ( *pfnUninitProto )( struct tagPROTO_INTERFACE* );

// removes an account from the database
typedef int ( *pfnDestroyProto )( struct tagPROTO_INTERFACE* );

typedef struct {
	size_t cbSize;
	char *szName;        // unique name of the module
	int   type;          // module type, see PROTOTYPE_ constants

	// 0.8.0+ additions
	#if MIRANDA_VER >= 0x800
		pfnInitProto fnInit; // initializes an empty account
		pfnUninitProto fnUninit; // deallocates an account instance
		pfnDestroyProto fnDestroy; // removes an account
	#endif
}
	PROTOCOLDESCRIPTOR;

// v0.3.3+:
//
// For recv, it will go from lower to higher, so in this case:
// check ignore, decrypt (encryption), translate
//
// For send, it will go translate, encrypt, ignore(??), send
//
// The DB will store higher numbers here, LOWER in the protocol chain, and lower numbers
// here HIGHER in the protocol chain
//
#define PROTOTYPE_IGNORE	  50    // added during v0.3.3
#define PROTOTYPE_PROTOCOL    1000
#define PROTOTYPE_ENCRYPTION  2000
#define PROTOTYPE_FILTER      3000
#define PROTOTYPE_TRANSLATION 4000
#define PROTOTYPE_OTHER       10000   //avoid using this if at all possible
#define PROTOTYPE_DISPROTO    20000

#if MIRANDA_VER >= 0x800
	#define MS_PROTO_ENUMPROTOS        "Proto/EnumProtos"
	#define MS_PROTO_ENUMPROTOCOLS     "Proto/EnumAccounts"
#else
	#define MS_PROTO_ENUMPROTOCOLS     "Proto/EnumProtocols"
#endif

//determines if a protocol module is loaded or not
//wParam=0
//lParam=(LPARAM)(const char*)szName
//Returns a pointer to the PROTOCOLDESCRIPTOR if the protocol is loaded, or
//NULL if it isn't.
#define MS_PROTO_ISPROTOCOLLOADED  "Proto/IsProtocolLoaded"

//gets the network-level protocol associated with a contact
//wParam=(WPARAM)(HANDLE)hContact
//lParam=0
//Returns a char* pointing to the asciiz name of the protocol or NULL if the
//contact has no protocol. There is no need to free() it or anything.
//This is the name of the module that actually accesses the network for that
//contact.
#define MS_PROTO_GETCONTACTBASEPROTO  "Proto/GetContactBaseProto"

//determines whether the specified contact has the given protocol in its chain
//wParam=(WPARAM)(HANDLE)hContact
//lParam=(LPARAM)(const char*)szName
//Returns -1 if it is base protocol, positive number if it is filter and 0 if it doesn't
#define MS_PROTO_ISPROTOONCONTACT  "Proto/IsProtoOnContact"

#define PROTOTYPE_SELFTYPING_OFF      0
#define PROTOTYPE_SELFTYPING_ON       1
//This service is for notifying protocols that the user is typing a message v0.3.3+
//in a message dialog.
//This is typically sent by a message dialog when a user in the clist is typing.
//wParam=(WPARAM)(HANDLE)hContact
//lParam=(LPARAM)(int)typing state
//NOTE: Only protocols should generally call this service
#define MS_PROTO_SELFISTYPING "Proto/SelfIsTyping"

#define PROTOTYPE_CONTACTTYPING_OFF      0
#define PROTOTYPE_CONTACTTYPING_INFINITE 2147483647
//This service is for notifying message dialogs/other plugins of a user typing. v0.3.3+
//This is typically sent by a protocol when a user in the clist is typing.
//wParam=(WPARAM)(HANDLE)hContact
//lParam=(LPARAM)(int)time (secs)
//NOTE: The time in seconds is used to tell a message dialog (or other plugin)
//how long to display its notification.  If time is 0, then notification
//of typing ends.
//NOTE: Only protocols should generally call this service
#define MS_PROTO_CONTACTISTYPING "Proto/ContactIsTyping"

//This hook notifies when a user is typing.  If a message dialog supports sending v0.3.3+
//typing notifications it should hook this event and fire the
//ProtoService PSS_USERISTYPING to the contacts protocol *after* verifying
//that the hContact is not NULL and the the user wishes to send notifications
//to this user (checked visibility, individual typing blocking, etc).
//wParam=(WPARAM)(HANDLE)hContact
//lParam=(LPARAM)(int)typing state
#define ME_PROTO_CONTACTISTYPING "Proto/ContactIsTypingEvent"

// -------------- accounts support --------------------- 0.8.0+ 

typedef struct tagACCOUNT
{
	int    cbSize;          // sizeof this structure
	char*  szModuleName;    // unique physical account name (matches database module name)
	int    type;            // always equal to PROTOTYPE_PROTOCOL
	TCHAR* tszAccountName;  // user-defined account name
	char*  szProtoName;     // physical protocol name
	int    bIsEnabled;      // is account enabled?
	int    bIsVisible;      // is account visible?
	int    iOrder;          // account order in various menus & lists
	BOOL   bOldProto;       // old-styled account (one instance per dll)
	struct tagPROTO_INTERFACE* ppro;  // pointer to the underlying object

	HWND   hwndAccMgrUI;
	BOOL   bAccMgrUIChanged;
	BOOL   bDynDisabled;    // dynamic disable flag, is never written to db
}
	PROTOACCOUNT;

//account enumeration service
//wParam=(WPARAM)(int*)piNumAccounts
//lParam=(LPARAM)(PROTOACCOUNT**)paAccounts
#define MS_PROTO_ENUMACCOUNTS "Proto/EnumAccounts"

__inline static INT_PTR ProtoEnumAccounts( int* accNumber, PROTOACCOUNT*** accArray )
{	return CallService( MS_PROTO_ENUMACCOUNTS, ( WPARAM )accNumber, (LPARAM)accArray );
}

//retrieves an account's interface by its physical name (database module)
//wParam=0
//lParam=(LPARAM)(char*)szAccountName
//return value = PROTOACCOUNT* or NULL
#define MS_PROTO_GETACCOUNT "Proto/GetAccount"

__inline static PROTOACCOUNT* ProtoGetAccount( const char* accName )
{	return (PROTOACCOUNT*)CallService( MS_PROTO_GETACCOUNT, 0, (LPARAM)accName );
}

//this event is fired when the accounts list gets changed
//wParam = event type (1 - added, 2 - changed, 3 - deleted, 4 - upgraded, 5 - enabled/disabled)
//lParam = (LPARAM)(PROTOACCOUNT*) - account being changed

#define PRAC_ADDED    1
#define PRAC_CHANGED  2
#define PRAC_REMOVED  3
#define PRAC_UPGRADED 4
#define PRAC_CHECKED  5


#define ME_PROTO_ACCLISTCHANGED "Proto/AccListChanged"

//displays the Account Manager
//wParam=0
//lParam=0
#define MS_PROTO_SHOWACCMGR "Protos/ShowAccountManager" 

//determines if an account is enabled or not
//wParam = 0
//lParam = (LPARAM)(PROTOACCOUNT*)
//Returns 1 if an account is valid and enabled, 0 otherwise
#define MS_PROTO_ISACCOUNTENABLED "Proto/IsAccountEnabled"

__inline static int IsAccountEnabled( const PROTOACCOUNT* pa )
{
#if MIRANDA_VER < 0x0900
	return pa && (( pa->bIsEnabled && !pa->bDynDisabled ) || pa->bOldProto );
#else
  return (int)CallService( MS_PROTO_ISACCOUNTENABLED, 0, (LPARAM)pa );
#endif
}

//determines if an account is locked or not
//wParam = 0
//lParam = (LPARAM)(char*)szAccountName
//Returns 1 if an account is locked and not supposed to change status, 0 otherwise
#define MS_PROTO_ISACCOUNTLOCKED "Proto/IsAccountLocked"


//gets the account associated with a contact
//wParam=(WPARAM)(HANDLE)hContact
//lParam=0
//Returns a char* pointing to the asciiz name of the protocol or NULL if the
//contact has no protocol. There is no need to mir_free() it or anything.
//This is the name of the module that actually accesses the network for that
//contact.
#define MS_PROTO_GETCONTACTBASEACCOUNT  "Proto/GetContactBaseAccount"

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

#define PA_FORMAT_UNKNOWN	0	// the protocol can not determine much about the "bitmap"
#define PA_FORMAT_PNG		1	// the image is PNG
#define PA_FORMAT_JPEG		2
#define PA_FORMAT_ICON		3
#define PA_FORMAT_BMP		4
#define PA_FORMAT_GIF       5
#define PA_FORMAT_SWF       6
#define PA_FORMAT_XML       7

typedef struct {
	int cbSize;					// sizeof()
	HANDLE hContact;			// this might have to be set by the caller too
	int format;					// PA_FORMAT_*
	char filename[MAX_PATH];	// full path to filename which contains the avatar
} PROTO_AVATAR_INFORMATION;

#define GAIF_FORCE 1			// force an update of the avatar if there is none

#define GAIR_SUCCESS  0			// information about the avatar has been returned
#define GAIR_WAITFOR  1			// you must hook ME_PROTO_ACK and wait for replies about avatar status
#define GAIR_NOAVATAR 2			// sorry, this contact has no avatars

/*
	wParam : GAIF_*
	lParam : (LPARAM) &PROTO_AVATAR_INFORMATION
	Affect : Make a request to the protocol to return information about a hContact's avatar (or main user)
			 it can return information, tell you there is none, or if forced start requesting an avatar.
	Note:
	Version: 0.3.4+ (2004/09/13)
*/
#define PS_GETAVATARINFO "/GetAvatarInformation"


#endif // M_PROTOCOLS_H

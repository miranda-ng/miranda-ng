{
  Miranda IM: the free IM client for Microsoft  Windows

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
}
// this module was created in v0.1.1.0
{$IFNDEF M_PROTOCOLS}
{$DEFINE M_PROTOCOLS}

{$include statusmodes.inc}

//send a general request through the protocol chain for a contact
//wParam=0
//lParam=(LPARAM)(CCSDATA*)&ccs
//returns the value as documented in the PS_ definition (m_protosvc.h)
type
  PCCSDATA = ^TCCSDATA;
  TCCSDATA = record
    hContact      : TMCONTACT;
    szProtoService: PAnsiChar;      // a PS_* constant
    wParam        : WPARAM;
    lParam        : LPARAM;
  end;

const
  MS_PROTO_CALLCONTACTSERVICE:PAnsiChar = 'Proto/CallContactService';

//a general network 'ack'
//wParam=0
//lParam=(LPARAM)(ACKDATA*)&ack
//Note that just because definitions are here doesn't mean they will be sent.
//Read the documentation for the function you are calling to see what replies
//you will receive.
type
  PACKDATA = ^TACKDATA;
  TACKDATA = record
    cbSize  : int;
    szModule: PAnsiChar; // the name of the protocol module which initiated this ack
    hContact: TMCONTACT;
    _type   : int;       // an ACKTYPE_* constant
    _result : int;       // an ACKRESULT_* constant
    hProcess: THANDLE;   // caller defined seq, I mean process code
    lParam  : LPARAM;    // caller defined data
  end;

const
  ACKTYPE_MESSAGE   = 0;
  ACKTYPE_URL       = 1;
  ACKTYPE_FILE      = 2;
  ACKTYPE_CHAT      = 3;
  ACKTYPE_AWAYMSG   = 4;
  ACKTYPE_AUTHREQ   = 5;
  ACKTYPE_ADDED     = 6;
  ACKTYPE_GETINFO   = 7;
  ACKTYPE_SETINFO   = 8;
  ACKTYPE_LOGIN     = 9;
  ACKTYPE_SEARCH    = 10;
  ACKTYPE_NEWUSER   = 11;
  ACKTYPE_STATUS    = 12;
  ACKTYPE_CONTACTS  = 13; // send/recv of contacts
  ACKTYPE_AVATAR    = 14; // send/recv of avatars from a protocol
  ACKTYPE_EMAIL     = 15; // notify if the unread emails changed

  ACKRESULT_SUCCESS = 0;
  ACKRESULT_FAILED  = 1;

// 'in progress' result codes:
  ACKRESULT_CONNECTING   = 100;
  ACKRESULT_CONNECTED    = 101;
  ACKRESULT_INITIALISING = 102;
  ACKRESULT_SENTREQUEST  = 103; // waiting for reply...
  ACKRESULT_DATA         = 104; // blob of file data sent/recved, or search result
  ACKRESULT_NEXTFILE     = 105; // file transfer went to next file
  ACKRESULT_FILERESUME   = 106; // a file is about to be received, see PS_FILERESUME
  ACKRESULT_DENIED       = 107; // a file send has been denied
  ACKRESULT_STATUS       = 108; // an ack or a series of acks to do with a task
                                // have a status change
  ACKRESULT_LISTENING    = 109; // waiting for connection
  ACKRESULT_CONNECTPROXY = 110; // connecting to file proxy
  ACKRESULT_SEARCHRESULT = 111; // result of extended search
const
  ME_PROTO_ACK:PAnsiChar = 'Proto/Ack';

{ v0.3.2+: When result is ACKRESULT_FAILED or ACKRESULT_DENIED, lParam can point to }
{ a human readable string with an explanation. For example: "The message was too }
{ long to be delivered". If no error message is specified, lParam must be NULL. }
{ Right now only explanations from ACKTYPE_MESSAGE is shown. }
{when type==ACKTYPE_FILE && (result==ACKRESULT_DATA || result==ACKRESULT_FILERESUME), }
{lParam points to this }
const
  PFTS_RECEIVING = 0;
  PFTS_SENDING   = 1;
  PFTS_UNICODE   = 2;
  PFTS_UTF       = 4;
{
type
  PFNAMECHAR = ^FNAMECHAR;
//#if MIRANDA_VER >= 0x0900
  FNAMECHAR = TCHAR;
//#else
//  FNAMECHAR = AnsiChar;
//#endif
  TFNAMECHAR = FNAMECHAR;
}
type
  // especially for Miranda 9.0+
  PPROTOFILETRANSFERSTATUS = ^TPROTOFILETRANSFERSTATUS;
  TPROTOFILETRANSFERSTATUS = record
    cbSize             : size_t;
    hContact           : TMCONTACT;
    flags              : dword;     // one of PFTS_* constants
    files              : ^TCHAR;
    totalFiles         : int;
    currentFileNumber  : int;
    totalBytes         : uint64;
    totalProgress      : uint64;
    workingDir         : TCHAR;
    currentFile        : TCHAR;
    currentFileSize    : uint64;
    currentFileProgress: uint64;
    currentFileTime    : uint64;   // as seconds since 1970
  end;

{ Enumerate the currently running protocols
  wParam=(WPARAM)(int*)&numberOfProtocols
  lParam=(LPARAM)(PROTOCOLDESCRIPTOR***)&ppProtocolDescriptors
  Returns 0 on success, nonzero on failure
  Neither wParam nor lParam may be NULL
  The list returned by this service is the protocol modules currently installed
  and running. It is not the complete list of all protocols that have ever been
  installed.
  IMPORTANT NOTE #1: the list returned is not static, it may be changed in the
  program's lifetime. Do not use this list in the global context, copy protocols
  names otherwise.
  IMPORTANT NOTE #2: in version 0.8 this service is mapped to the MS_PROTO_ENUMACCOUNTS
  service to provide the compatibility with old plugins (first three members of
  PROTOACCOUNT are equal to the old PROTOCOLDESCRIPTOR format). If you declare the
  MIRANDA_VER macro with value greater or equal to 0x800, use MS_PROTO_ENUMPROTOS
  service instead to obtain the list of running protocols instead of accounts.
  Note that a protocol module need not be an interface to an Internet server,
  they can be encryption and loads of other things, too.
  And yes, before you ask, that is triple indirection. Deal with it.
  Access members using ppProtocolDescriptors[index]->element }
type
  pfnInitProto = function(const szModuleName:PAnsiChar;const szUserName:TCHAR):PPROTO_INTERFACE; cdecl;

// deallocates an account instance
  pfnUninitProto = function(pin:PPROTO_INTERFACE):int; cdecl;

// removes an account from the database
  pfnDestroyProto = function(pin:PPROTO_INTERFACE):int; cdecl;

type
  PPROTOCOLDESCRIPTOR = ^TPROTOCOLDESCRIPTOR;
  TPROTOCOLDESCRIPTOR = record
    cbSize : size_t;
    szName : PAnsiChar; // unique name of the module
    _type  : int;   // module type, see PROTOTYPE_ constants
  // 0.8.0+ additions
    fnInit   :pfnInitProto;    // initializes an empty account
    fnUninit :pfnUninitProto;  // deallocates an account instance
    fnDestroy:pfnDestroyProto; // removes an account
  end;

const
  PROTOCOLDESCRIPTOR_V3_SIZE = SizeOf(int_ptr)+SizeOf(PAnsiChar)+SizeOf(size_t);

{ v0.3.3+:
  For recv, it will go from lower to higher, so in this case:
  check ignore, decrypt (encryption), translate
  For send, it will go translate, encrypt, ignore(??), send
  The DB will store higher numbers here, LOWER in the protocol chain, and lower numbers
  here HIGHER in the protocol chain }
const
  PROTOTYPE_IGNORE      = 50;    // added during v0.3.3
  PROTOTYPE_PROTOCOL    = 1000;
  PROTOTYPE_VIRTUAL     = 1001;  // virtual protocol (has no accounts)
  PROTOTYPE_ENCRYPTION  = 2000;
  PROTOTYPE_FILTER      = 3000;
  PROTOTYPE_TRANSLATION = 4000;
  PROTOTYPE_OTHER       = 10000; // avoid using this if at all possible

{ determines if a protocol module is loaded or not
  wParam=0
  lParam=(LPARAM)(const AnsiChar*)szName
  Returns a pointer to the PROTOACCOUNT if the protocol is loaded, or
  NULL if it isn't.
}
  MS_PROTO_ISPROTOCOLLOADED:PAnsiChar = 'Proto/IsProtocolLoaded';

{ gets the network-level protocol associated with a contact
  wParam=(WPARAM)(HANDLE)hContact
  lParam=0
  Returns a AnsiChar* pointing to the asciiz name of the protocol or NULL if the
  contact has no protocol. There is no need to free() it or anything.
  This is the name of the module that actually accesses the network for that
  contact. }
  MS_PROTO_GETCONTACTBASEPROTO = 'Proto/GetContactBaseProto';

{ determines whether the specified contact has the given protocol in its chain
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(const AnsiChar*)szName
  Returns -1 if it is base protocol, positive number if it is filter and 0 if it doesn't }
  MS_PROTO_ISPROTOONCONTACT:PAnsiChar = 'Proto/IsProtoOnContact';

const
  PROTOTYPE_SELFTYPING_OFF = 0;
  PROTOTYPE_SELFTYPING_ON  = 1;
{ This service is for notifying protocols that the user is typing a message v0.3.3+
  in a message dialog.
  This is typically sent by a message dialog when a user in the clist is typing.
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(int)typing state
  NOTE: Only protocols should generally call this service }
  MS_PROTO_SELFISTYPING:PAnsiChar = 'Proto/SelfIsTyping';

const
  PROTOTYPE_CONTACTTYPING_OFF      = 0;
  PROTOTYPE_CONTACTTYPING_INFINITE = $7FFFFFFF;
{ This service is for notifying message dialogs/other plugins of a user typing. v0.3.3+
  This is typically sent by a protocol when a user in the clist is typing.
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(int)time (secs)
  NOTE: The time in seconds is used to tell a message dialog (or other plugin)
  how long to display its notification.  If time is 0, then notification
  of typing ends.
  NOTE: Only protocols should generally call this service }
  MS_PROTO_CONTACTISTYPING:PAnsiChar = 'Proto/ContactIsTyping';

{ This hook notifies when a user is typing.  If a message dialog supports sending v0.3.3+
  typing notifications it should hook this event and fire the
  ProtoService PSS_USERISTYPING to the contacts protocol *after* verifying
  that the hContact is not NULL and the the user wishes to send notifications
  to this user (checked visibility, individual typing blocking, etc).
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(int)typing state }
  ME_PROTO_CONTACTISTYPING:PAnsiChar = 'Proto/ContactIsTypingEvent';

// -------------- accounts support --------------------- 0.8.0+

type
  PPROTOACCOUNT = ^TPROTOACCOUNT;
  TPROTOACCOUNT = record
    cbSize          :int;              // sizeof this structure
    szModuleName    :PAnsiChar;        // unique physical account name (matches database module name)
    tszAccountName  :TChar;            // user-defined account name
    szProtoName     :PAnsiChar;        // physical protocol name
    bIsEnabled      :ByteBool;         // is account enabled?
    bIsVisible      :ByteBool;         // is account visible?
    bIsVirtual      :ByteBool;         // is account virtual?
    bOldProto       :ByteBool;         // old-styled account (one instance per dll)
    bDynDisabled    :ByteBool;         // dynamic disable flag, is never written to db
    bAccMgrUIChanged:ByteBool;
    hwndAccMgrUI    :HWND;
    iOrder          :int;              // account order in various menus & lists
    ppro            :PPROTO_INTERFACE; // pointer to the underlying object
  end;
  tagACCOUNT = TPROTOACCOUNT;

//account enumeration service
//wParam=(WPARAM)(int)piNumAccounts
//lParam=(LPARAM)(PROTOACCOUNT**)paAccounts
const
  MS_PROTO_ENUMACCOUNTS:PAnsiChar = 'Proto/EnumAccounts';

  MS_PROTO_ENUMPROTOS:PAnsiChar = 'Proto/EnumProtos';   // MIRANDA_VER >= $800

type
  PACC_CREATE = ^ACC_CREATE;
  ACC_CREATE = record
	 pszBaseProto   :PAnsiChar;
    pszInternal    :PAnsiChar;
    ptszAccountName:PWideChar;
  end;

const
// creates new account
// wParam = 0
// lParam = (LPARAM)(ACC_CREATE*) account definition
// return value = PROTOACCOUNT* or NULL
  MS_PROTO_CREATEACCOUNT:PAnsiChar = 'Proto/CreateAccount';

//retrieves an account's interface by its physical name (database module)
//wParam=0
//lParam=(LPARAM)(AnsiChar*)szAccountName
//return value = PROTOACCOUNT* or NULL
  MS_PROTO_GETACCOUNT:PAnsiChar = 'Proto/GetAccount';
(*
__inline PROTOACCOUNT* ProtoGetAccount( const AnsiChar* accName )
{  return (PROTOACCOUNT* )CallService( MS_PROTO_GETACCOUNT, 0, (LPARAM)accName );
}
*)

//this event is fired when the accounts list gets changed
//wParam = event type (1 - added, 2 - changed, 3 - deleted, 5 - enabled/disabled)
//lParam = (LPARAM)(PROTOACCOUNT*) - account being changed

  PRAC_ADDED    = 1;
  PRAC_CHANGED  = 2;
  PRAC_REMOVED  = 3;
  PRAC_UPGRADED = 4;
  PRAC_CHECKED  = 5;
//  PRAC_CONVERT  = 6;

  ME_PROTO_ACCLISTCHANGED:PAnsiChar = 'Proto/AccListChanged';
{

  displays the Account Manager
  wParam=0
  lParam=0
}
  MS_PROTO_SHOWACCMGR:PAnsiChar = 'Protos/ShowAccountManager';

{
  determines if an account is enabled or not
  wParam = 0
  lParam = PPROTOACCOUNT
  Returns 1 if an account is valid and enabled, 0 otherwise
}
  MS_PROTO_ISACCOUNTENABLED:PAnsiChar = 'Proto/IsAccountEnabled';

{
  determines if an account is locked or not
  wParam = 0
  lParam = PAnsiChar szAccountName
  Returns 1 if an account is locked and not supposed to change status, 0 otherwise
}
  MS_PROTO_ISACCOUNTLOCKED:PAnsiChar = 'Proto/IsAccountLocked';

{
  gets the account associated with a contact
  wParam=(WPARAM)(HANDLE)hContact
  lParam=0
  Returns a PAnsiChar pointing to the asciiz name of the protocol or NULL if the
  contact has no protocol. There is no need to mir_free() it or anything.
  This is the name of the module that actually accesses the network for that contact.
}
  MS_PROTO_GETCONTACTBASEACCOUNT:PAnsiChar = 'Proto/GetContactBaseAccount';

// -------------- avatar support ---------------------
{
  First a protocol must report it supports PF4_AVATARS via PS_GETCAPS, secondly
  it will return the following acks for certain events, the protocol must use
  ProtoBroadcastAck(), listeners must hook ME_PROTO_ACK, note that lParam = ACKDATA
}
{
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
}
const
  PA_FORMAT_UNKNOWN = 0; // the protocol can not determine much about the "bitmap"
  PA_FORMAT_PNG     = 1; // the image is PNG
  PA_FORMAT_JPEG    = 2;
  PA_FORMAT_ICON    = 3;
  PA_FORMAT_BMP     = 4;
  PA_FORMAT_GIF     = 5;
  PA_FORMAT_SWF     = 6;
  PA_FORMAT_XML     = 7;

type
  PPROTO_AVATAR_INFORMATIONW = ^TPROTO_AVATAR_INFORMATIONW;
  TPROTO_AVATAR_INFORMATIONW = record
    cbSize  : int;       // sizeof()
    hContact: TMCONTACT; // this might have to be set by the caller too
    format  : int;       // PA_FORMAT_*
    // full path to filename which contains the avatar
    filename: array[0..(MAX_PATH)-1] of WideChar;
  end;

  PPROTO_AVATAR_INFORMATION = ^TPROTO_AVATAR_INFORMATION;
  TPROTO_AVATAR_INFORMATION = record
    cbSize  : int;       // sizeof()
    hContact: TMCONTACT; // this might have to be set by the caller too
    format  : int;       // PA_FORMAT_*
    // full path to filename which contains the avatar
    filename: array[0..(MAX_PATH)-1] of AnsiChar;
  end;

const
  GAIF_FORCE    = 1; // force an update of the avatar if there is none

  GAIR_SUCCESS  = 0; // information about the avatar has been returned
  GAIR_WAITFOR  = 1; // you must hook ME_PROTO_ACK and wait for replies about avatar status
  GAIR_NOAVATAR = 2; // sorry, this contact has no avatars

{
 wParam : GAIF_
 lParam : (LPARAM) &PROTO_AVATAR_INFORMATION
 Affect : Make a request to the protocol to return information about a hContact's avatar (or main user)
      it can return information, tell you there is none, or if forced start requesting an avatar.
 Note:
 Version: 0.3.4+ (2004/09/13)
}
  PS_GETAVATARINFO  = '/GetAvatarInformation';
  PS_GETAVATARINFOW = '/GetAvatarInformationW';

{$ENDIF}

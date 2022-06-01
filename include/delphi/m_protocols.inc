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

//a general network 'ack'
//wParam=0
//lParam=(LPARAM)(ACKDATA*)&ack
//Note that just because definitions are here doesn't mean they will be sent.
//Read the documentation for the function you are calling to see what replies
//you will receive.
type
  PACKDATA = ^TACKDATA;
  TACKDATA = record
    szModule: PAnsiChar; // the name of the protocol module which initiated this ack
    hContact: TMCONTACT;
    _type   : int;       // an ACKTYPE_* constant
    _result : int;       // an ACKRESULT_* constant
    hProcess: THANDLE;   // caller defined seq, I mean process code
    lParam  : LPARAM;    // caller defined data
  end;

const
  ACKTYPE_MESSAGE   = 0;
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
  PPROTOCOLDESCRIPTOR = ^TPROTOCOLDESCRIPTOR;
  PPPROTOCOLDESCRIPTOR = ^PPROTOCOLDESCRIPTOR;
  TPROTOCOLDESCRIPTOR = record
    szName : PAnsiChar; // unique name of the module
    _type  : int;   // module type, see PROTOTYPE_ constants
  end;

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
  Returns a pointer to the PROTOCOLDESCRIPTOR if the protocol is loaded, or
  NULL if it isn't.
}

function Proto_IsProtocolLoaded(protoName:PAnsiCHar) : PPROTOCOLDESCRIPTOR; stdcall; external AppDLL;

// -------------- accounts support --------------------- 0.8.0+

type
  PPROTOACCOUNT = ^TPROTOACCOUNT;
  PPPROTOACCOUNT = ^PPROTOACCOUNT;
  TPROTOACCOUNT = record
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
    ppro            :pointer;          // pointer to the underlying object
  end;
  tagACCOUNT = TPROTOACCOUNT;

//account enumeration service
procedure Proto_EnumAccounts(var nAccs:int; var pAccs:PPPROTOACCOUNT); stdcall; external AppDLL;

procedure Proto_EnumProtocols(var nProtos:int; var pProtos:PPPROTOCOLDESCRIPTOR); stdcall; external AppDLL;

//retrieves an account's interface by its physical name (database module)
//return value = PROTOACCOUNT* or NULL

function Proto_GetAccount(proto:PAnsiChar) : PPROTOACCOUNT; stdcall; external AppDLL;

// retrieves protocol status
function Proto_GetStatus(proto:PAnsiChar) : int; stdcall; external AppDLL;

//this event is fired when the accounts list gets changed
//wParam = event type (1 - added, 2 - changed, 3 - deleted, 5 - enabled/disabled)
//lParam = (LPARAM)(PROTOACCOUNT*) - account being changed

const
  PRAC_ADDED    = 1;
  PRAC_CHANGED  = 2;
  PRAC_REMOVED  = 3;
  PRAC_UPGRADED = 4;
  PRAC_CHECKED  = 5;
//  PRAC_CONVERT  = 6;

  ME_PROTO_ACCLISTCHANGED:PAnsiChar = 'Proto/AccListChanged';

{
  gets the account associated with a contact
  Returns a PAnsiChar pointing to the asciiz name of the protocol or NULL if the
  contact has no protocol. There is no need to mir_free() it or anything.
  This is the name of the module that actually accesses the network for that contact.
}

function Proto_GetBaseAccountName(hContact:TMCONTACT) : PAnsiChar; stdcall; external AppDLL;

{
  gets and sets unique id name of any contact
}

function Proto_GetUniqueId(proto:PAnsiChar) : PAnsiChar; stdcall; external AppDLL;
procedure Proto_SetUniqueId(proto, id:PAnsiChar); stdcall; external AppDLL;

{$ENDIF}

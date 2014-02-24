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

{$IFNDEF M_PROTOCOLS}
{$DEFINE M_PROTOCOLS}

const

    ACKTYPE_MESSAGE             = 0;
    ACKTYPE_URL                 = 1;
    ACKTYPE_FILE                = 2;
    ACKTYPE_CHAT                = 3;
    ACKTYPE_AWAYMSG             = 4;
    ACKTYPE_AUTHREQ             = 5;
    ACKTYPE_ADDED               = 6;
    ACKTYPE_GETINFO             = 7;
    ACKTYPE_SETINFO             = 8;
    ACKTYPE_LOGIN               = 9;
    ACKTYPE_SEARCH              = 10;
    ACKTYPE_NEWUSER             = 11;
    ACKTYPE_STATUS              = 12;
    ACKTYPE_CONTACTS            = 13;   //send/recv of contacts

    ACKRESULT_SUCCESS           = 0;
    ACKRESULT_FAILED            = 1;
    //'in progress' result codes:
    ACKRESULT_CONNECTING        = 100;
    ACKRESULT_CONNECTED         = 101;
    ACKRESULT_INITIALISING      = 102;
    ACKRESULT_SENTREQUEST       = 103;  // waiting for reply...
    ACKRESULT_DATA              = 104;  // blob of file data sent/recved, or search result
    ACKRESULT_NEXTFILE          = 105;  // file transfer went to next file
    ACKRESULT_FILERESUME        = 106;  // a file is about to be received, see PS_FILERESUME
    ACKRESULT_DENIED            = 107;  // a file send has been denied (0.3a + only)

    // for PROTOCOLDESCRIPTOR.type

    PROTOTYPE_PROTOCOL          = 1000;
    PROTOTYPE_ENCRYPTION        = 2000;
    PROTOTYPE_FILTER            = 3000;
    PROTOTYPE_TRANSLATION       = 4000;
    PROTOTYPE_OTHER             = 10000;//avoid using this if at all possible

type

    PCCSDATA = ^TCCSDATA;
    TCCSDATA = record
        hContact: THandle;
        szProtoService: PChar;      // a PS_* constant
        wParam: WPARAM;
        lParam: LPARAM;
    end;

    PACKDATA = ^TACKDATA;
    TACKDATA = record
        cbSize: int;
        szModule: PChar;        // the name of the protocol module which initiated this ack
        hContact: THandle;
        type_: int;             // an ACKTYPE_* constant
        result_: int;           // an ACKRESULT_* constant
        hProcess: THandle;      // caller defined seq, I mean process code
        lParam: LPARAM;         // caller defined data
    end;

    // when type=ACKTYPE_FILE and (result=ACKRESULT_DATA or result=ACKRESULT_FILERESUME)

    PPROTOFILETRANSFERSTATUS = ^TPROTOFILETRANSFERSTATUS;
    TPROTOFILETRANSFERSTATUS = record
        cbSize: int;
        hContact: THandle;
        sending: int;               // true if sending, false if receiving
        files: PChar;               // pointer to an array of pchar's
        totalFiles: int;
        currentFileNumber: int;
        totalBytes: LongInt;
        totalProgress: LongInt;
        workingDir: PChar;
        currentFile: PChar;
        currentFileSize: LongInt;
        currentFileProgress: LongInt;
        currentFileTime: LongInt;   // UNIX time
    end;

    // for registering a protocol, enumeration

    PPROTOCOLDESCRIPTOR = ^TPROTOCOLDESCRIPTOR;
    TPROTOCOLDESCRIPTOR = record
        cbSize: int;
        szName: PChar;  // unique name of module
        type_: int;     // a PROTOTYPE_* constant
    end;

const

    {
        wParam : 0
        lParam : Pointer to an initalised CSSDATA structure
        Affect : Send a general request thru the protocol chain for a contact
        Return : the return value documented in the PS_* def (m_protosvc.inc)
    }
    MS_PROTO_CALLCONTACTSERVICE         = 'Proto/CallContactService';

    {
        wParam : 0
        lParam : Pointer to an initalised TACKDATA structure
        Affect : a general 'ack', see notes
        Notes  : Just because defs are here doesn't mean they will be sent
                 read the docs for the function you are calling to see what
                 replies you will get.
    }
    ME_PROTO_ACK                        = 'Proto/Ack';

    {
        wParam : pointer to an int to store number of protocols
        lParam : Pointer to an an array of PPROTOCOLDESCRIPTOR pointers
        Affect : Enumerate the currently running protocols, see notes
        Returns: 0 on success, [non zero] on failure
        Notes  : Neither wParam/lParam maybe NULL(0), the list returned by
                 this service is the protocol modules currently installed
                 and running, it is not a complete list of protocols that have
                 ever been installed.
                 -
                 A protocol module does not have to be a protocol running thru
                 the internet, it can be a vast number of things
    }
    MS_PROTO_ENUMPROTOCOLS              = 'Proto/EnumProtocols';

    {
        wParam : 0
        lParam : Pointer to null terminated string containing protocol name
        Affect : Determines if a protocol is running or not.
        Returns: A pointer to the PPROTOCOLDESCRIPTOR if the protocol is loaded
                 or NULL(0) if it isn't
    }
    MS_PROTO_ISPROTOCOLLOADED           = 'Proto/IsProtocolLoaded';

    {
        wParam : MCONTACT
        lParam : Pointer to a null terminated string containing a name
        Affect : Determine whether the given contact has the given protocol
                 in it's chain.
        Returns : 0 if the protocol isn't in the chain, [non zero] if it is
    }
    MS_PROTO_ISPROTOONCONTACT           = 'Proto/IsProtoOnContact';

    {
        wParam : MCONTACT
        lParam : 0
        Affect : Gets the network-level protocol associated with a contact
        Returns: a PChar pointing to the ASCIIZ name of the protocol or NULL(0)
                 if the contact has no protocol, There's no need to dispsose
                 the returned string.
                 -
                 This is the name of the module that actually accesses the network
                 for that contact.
    }
    MS_PROTO_GETCONTACTBASEPROTO        = 'Proto/GetContactBaseProto';

{$ENDIF}
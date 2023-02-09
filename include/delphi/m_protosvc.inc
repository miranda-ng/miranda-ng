{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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

{$IFNDEF M_PROTOSVC}
{$DEFINE M_PROTOSVC}

{ *****  Unicode Services note   ********
******************************************

Only new style protocols (Miranda 0.9+) with m_iVersion set to 2 or higher
support Unicode services documented below, all other support only ANSI.

For all other that do not support Unicode services, Miranda core will
convert Unicode to ANSI and call the appropriate service.
}

type
//  PFNAMECHAR = ^FNAMECHAR;
//#if MIRANDA_VER >= 0x0900
  FNAMECHAR = TCHAR;
//#else
//  FNAMECHAR = AnsiChar;
//#endif
  TFNAMECHAR = FNAMECHAR;

{<</
    none of these services should be used on there own (i.e. using CallService(), etc)
    hence the PS_ prefix, instead use the services exposed in m_protocols.inc

    these should be called with CallProtoService which prefixes the protocol module
    name before calling.
    -
    Deleting contacts from protocols that store the contact list on the server:
    If a contact is deleted while the protocol is online, it is expected that the
    protocol will have hooked me_db_contact_deleted and take the appropriate
    action by itself.
    If a contact is deleted while the protocol is offline, the contact list will
    display a message to the user about the problem, and set the byte setting
    "CList"/"Delete" to 1. Each time such a protocol changes status from offline
    or connecting to online the contact list will check for contacts with this
    flag set and delete them at that time. Your hook for me_db_contact_deleted
    will pick this up and everything will be good.
/>>}

const
  PFLAGNUM_1        = $1;
  PF1_IMSEND        = $00000001; // supports IM sending
  PF1_IMRECV        = $00000002; // supports IM receiving
  PF1_IM            = (PF1_IMSEND or PF1_IMRECV);
  PF1_FILESEND      = $00000010; // supports file sending
  PF1_FILERECV      = $00000020; // supports file receiving
  PF1_FILE          = (PF1_FILESEND or PF1_FILERECV);
  PF1_MODEMSGSEND   = $00000040; // supports broadcasting away messages
  PF1_MODEMSGRECV   = $00000080; // supports reading others' away messages
  PF1_MODEMSG       = (PF1_MODEMSGSEND or PF1_MODEMSGRECV);
  PF1_SERVERCLIST   = $00000100; // contact lists are stored on the server, not locally. See notes below
  PF1_AUTHREQ       = $00000200; // will get authorisation requests for some or all contacts
  PF1_VISLIST       = $00000800; // has an invisible list
  PF1_INVISLIST     = $00001000; // has a visible list for when in invisible mode
  PF1_INDIVSTATUS   = $00002000; // supports setting different status modes to each contact
  PF1_EXTENSIBLE    = $00004000; // the protocol is extensible and supports plugin-defined messages
  PF1_PEER2PEER     = $00008000; // supports direct (not server mediated) communication between clients
  PF1_NEWUSER       = $00010000; // supports creation of new user IDs
  PF1_CHAT          = $00020000; // has a realtime chat capability
  PF1_INDIVMODEMSG  = $00040000; // supports replying to a mode message request with different text depending on the contact requesting
  PF1_BASICSEARCH   = $00080000; // supports a basic user searching facility
  PF1_EXTSEARCH     = $00100000; // supports one or more protocol-specific extended search schemes
  PF1_CANRENAMEFILE = $00200000; // supports renaming of incoming files as they are transferred
  PF1_FILERESUME    = $00400000; // can resume broken file transfers, see PS_FILERESUME below
  PF1_ADDSEARCHRES  = $00800000; // can add search results to the contact list
  PF1_CONTACTSEND   = $01000000; // can send contacts to other users
  PF1_CONTACTRECV   = $02000000; // can receive contacts from other users
  PF1_CONTACT       = (PF1_CONTACTSEND or PF1_CONTACTRECV);
  PF1_CHANGEINFO    = $04000000; // can change our user information stored on server
  PF1_SEARCHBYEMAIL = $08000000; // supports a search by e-mail feature
  PF1_USERIDISEMAIL = $10000000; // set if the uniquely identifying field of the network is the e-mail address
  PF1_SEARCHBYNAME  = $20000000; // supports searching by nick/first/last names
  PF1_EXTSEARCHUI   = $40000000; // has a dialog box to allow searching all the possible fields
  PF1_NUMERICUSERID = $80000000; // the unique user IDs for this protocol are numeric

  PFLAGNUM_2     = 2;          // the status modes that the protocol supports
  PF2_ONLINE     = $00000001; // an unadorned online mode
  PF2_INVISIBLE  = $00000002;
  PF2_SHORTAWAY  = $00000004; // Away on ICQ
  PF2_LONGAWAY   = $00000008; // NA on ICQ
  PF2_LIGHTDND   = $00000010; // Occupied on ICQ
  PF2_HEAVYDND   = $00000020; // DND on ICQ
  PF2_FREECHAT   = $00000040;
  PF2_IDLE       = $00000200; //added during 0.3.4 (2004/09/13)

  PFLAGNUM_3 = 3; //the status modes that the protocol supports
                  //away-style messages for. Uses the PF2_ flags.

  PFLAGNUM_4           = 4;         // v0.3+: flag asking a protocol plugin how auths are handled
  PF4_FORCEAUTH        = $00000001; // protocol has to send auth's for things to work
  PF4_NOCUSTOMAUTH     = $00000004; // protocol can't send a custom message while asking others for auth
  PF4_SUPPORTTYPING    = $00000008; // protocol supports user is typing messages v0.3.3+
  PF4_SUPPORTIDLE      = $00000010; // protocol understands idle, added during v0.3.4+ (2004/09/13)
  PF4_AVATARS          = $00000020; // protocol has avatar support, added during v0.3.4 (2004/09/13)
  PF4_OFFLINEFILES     = $00000040; // protocols supports sending files to offline users (v0.5.2)
  PF4_IMSENDUTF        = $00000080; // protocol is able to process messages in utf-8 (v.0.7.0+)
  PF4_IMSENDOFFLINE    = $00000100; // protocol supports sending offline messages (v0.8.0+)
  PF4_INFOSETTINGSVC   = $00000200; // protocol supports user info translation services (v0.8.0+)
  PF4_NOAUTHDENYREASON = $00000400; // protocol doesn't support authorization deny reason (v0.9.0+)
  PF4_GROUPCHATFILES   = $00000800; // protocol supports sending files to group chats (v0.95.2+)
  PF4_SINGLEFILEONLY   = $00001000; // protocol supports sending files one by one only

  PFLAG_UNIQUEIDTEXT         = 100; // returns a static buffer of text describing the unique field by which this protocol identifies users (already translated), or NULL
  PFLAG_MAXCONTACTSPERPACKET = 200; // v0.1.2.2+: returns the maximum number of contacts which can be sent in a single PSS_CONTACTS. lParam=(LPARAM)hContact.
  // for PS_SETSTATUS

  LOGINERR_WRONGPASSWORD = 1;
  LOGINERR_NONETWORK     = 2;
  LOGINERR_PROXYFAILURE  = 3;
  LOGINERR_BADUSERID     = 4;
  LOGINERR_NOSERVER      = 5;
  LOGINERR_TIMEOUT       = 6;
  LOGINERR_WRONGPROTOCOL = 7;
  LOGINERR_OTHERLOCATION = 8;

  // flag for PS_ADDTOLIST

  PALF_TEMPORARY = 1; // add the contact temporarily and invisibly, just to get user info or something

  // flags for PS_GETINFO

  SGIF_MINIMAL = 1; // get only the most basic information. This should
                    // contain at least a Nick and e-mail.
  SGIF_ONOPEN  = 2; // set when the User Info form is being opened

  // for PSR_MESSAGE

  PREF_CREATEREAD =  1; // create the database event with the 'read' flag set
  PREF_RTL        =  4; // 0.5+ addition: support for right-to-left messages
  PREF_SENT       = 16; // message will be created with the DBEF_SENT flag

const
  PSR_UNICODE = 1;

type
  PPROTOSEARCHRESULT = ^TPROTOSEARCHRESULT;
  TPROTOSEARCHRESULT = record
    cbSize   : int;
    flags    : int;
    nick     : TFNAMECHAR;
    firstName: TFNAMECHAR;
    lastName : TFNAMECHAR;
    email    : TFNAMECHAR;
    id       : TFNAMECHAR;
    reserved : array [0..(8*SizeOf(THANDLE) div SizeOf(dword))-1] of byte;
    // Protocols may extend this structure with extra members at will and supply
    // a larger cbSize to reflect the new information, but they must not change
    // any elements above this comment
    // The 'reserved' field is part of the basic structure, not space to
    // overwrite with protocol-specific information.
    // If modules do this, they should take steps to ensure that information
    // they put there will be retained by anyone trying to save this structure.
  end;

  PPROTORECVEVENT = ^TPROTORECVEVENT;
  TPROTORECVEVENT = record
    flags           : dword;
    timestamp       : dword;
    szMessage       : TChar;
    lParam          : LPARAM;
    szUserId        : PAnsiChar; // user id of event
  end;

const
  {
    wParam : PFLAGNUM_* (see above)
    lParam : 0
    Affects: Returns a bitfield for settings corresponding to flag number, see notes
    Returns: a bitfield of supported features -- or 0 if flag_num is not supported
    Notes  : this checks what sort of things are actively supported by a protocol
             module
  }
  PS_GETCAPS = '/GetCaps';

  {
    wParam : cchName
    lParam : Pointer to a buffer to fill with human-readable name
    Affect : Get a human-readable name for the protocol, see notes
    Result : 0 on success, [non zero] on failure
    Notes  : Should be translated before being returned, cchName
             has the size of the buffer, example strings: "ICQ", "AIM"
  }
  PS_GETNAME = '/GetName';

  PLI_PROTOCOL = $1;     // An icon representing the protocol (eg the multicoloured flower for ICQ)

  PLIF_LARGE        = $0;     // Or with one of the above to get the large (32x32 by default) icon
  PLIF_SMALL        = $10000; // Or with one of the above to get the small (16x16 by default) icon
  PLIF_ICOLIB       = $20000; // the returned HICON is managed by IcoLib, DO NOT DestroyIcon() it
  PLIF_ICOLIBHANDLE = $40000; // the function will return IcoLib handle not HICON

  PS_LOADICON = '/LoadIcon';

  {
    wParam : status_mode
    lParam : Pointer to a null terminated string containing message
    Affect : Sets the status mode specific message for the user, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : This service is not available unless PF1_MODEMSGSEND is set,
             and PF1_INDIVMODEMSG is *not* set.
             -
             Protocol modules smust support lParam=NULL, it may eithere mean
             to use an empty message or (preferably) not to reply at all to
             any requests.
  }
  PS_SETAWAYMSG  = '/SetAwayMsg';

  {
    wParam : newMode from statusmodes.inc
    lParam : 0
    Affect : Change the protocol's status mode, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : Will send an ack with :
             type=ACKTYPE_SUCCESS, result=ACKRESULT_SUCCESS, hProcess=previousMode, lParam=newMode
             -
             when the change completes. This ack is sent for all changes, not
             just ones caused by calling this function.
             -
             NewMode can be ID_STATUS_CONNECTING<=newMode<ID_STATUS_CONNECTING+
             MAX_CONNECT_RETRIES to signify that it's connecting and it's the nth retry.
             -
             Protocols are initially always in offline mode, if a protocol
             doesn't support a specific status mode, it should pick the closest
             ones that it does support, and change to that.

             If a protocol has to switch from offline mode to online (or a substate
             of online, like away) then it should report any errors in the
             form of an additional ack :

             type=ACKTYPE_LOGIN, result=ACKRESULT_FAILURE, hProcess=NULL, lParam=LOGINERR_*

             SetStatus() is called when a protocol module is first loaded
             with newMode=ID_STATUS_ONLINE.
             -
             Protocols can define their own LOGINERR_* starting at $1000, see
             LOGINERR_* above
  }
  PS_SETSTATUS = '/SetStatus';

  {
    wParam : 0
    lParam : 0
    Affect : Get the status mode that a protocol is currently in, see notes
    Returns: The current status mode
    Notes  : Only protocol modules need to implement this, non network level
             protocol modules do not need to (but if you register as a protocol
             you need to, Miranda will GPF otherwise)
  }
  PS_GETSTATUS = '/GetStatus';

  {
    wParam : 0
    lParam : Pointer to a null terminated string containing an ID to search for
    Affect : Send a basic search request, see notes
    Returns: A handle to the search request or NULL(0) on failure
    Notes  : All protocols identify users uniquely by a single field
             this service will search by that field.
             -
             All search replies (even protocol-spec extended searches)
             are replied by a series of ack's,-
             -
             Result acks are a series of:
             type=ACKTYPE_SEARCH, result=ACKRESULT_DATA, lParam=Pointer to a TPROTOSEARCHRESULT structure
             -
             ending ack:
             type=ACKTYPE_SEARCH, result=ACKRESULT_SUCCESS, lParam=0
             -
             The pointers in the structure are not guaranteed to be
             valid after the ack is complete.
             -
             The structure to reply with search results can be extended
             per protocol basis (see below)

  }
  PS_BASICSEARCH  = '/BasicSearch';

  {
    wParam : 0
    lParam : Handle to window owner
    Affect : Create the advanced search dialog box, see notes
    Returns: A window handle, or NULL(0) on failure
    Notes  : this service is only available if PF1_EXTSEARCHUI capability is
             set, advanced search is very protocol-spec'd so it is left to
             the protocol itself to supply a dialog containing the options,
             this dialog should not have a titlebar and contain only search
             fields. the rest of the UI is supplied by Miranda.
             -
             The dialog should be created with CreateDialog() or it's kin
             and still be hidden when this function returns,
             -
             The dialog will be destroyed when the find/add dialog is closed
    Version: v0.1.2.1+
  }
  PS_CREATEADVSEARCHUI = '/CreateAdvSearchUI';

  {
    wParam : 0
    lParam : Handle to advanced search window handle
    Affect : Search using the advanced search dialog, see notes
    Returns: A handle or NULL(0) on failure
    Notes  : Results are returned in the same manner as PS_BASICSEARCH,
             this service is only available if PF1_EXTSEARCHUI capability is set
    Version: v0.1.2.1+
  }
  PS_SEARCHBYADVANCED = '/SearchByAdvanced';

type
  TCUSTOMSEARCHRESULTS = record
    nSize      :size_t;
    nFieldCount:int;
    szFields   :^TCHAR;
    psr        :TPROTOSEARCHRESULT;
  end;

  {
    wParam : flags
    lParam : Pointer to a TPROTOSEARCHRESULT structure
    Affect : Adds a search result to the contact list, see notes
    Returns: A handle to the new contact (TMCONTACT) or NULL(0) on failure
    Notes  : The pointer MUST be a result returned by a search function
             since there maybe extra protocol-spec data required by the protocol.
             -
             the protocol module should not allow duplicate contains to be added,
             but if such a request *is* received it should return a TMCONTACT
             to the original user,
             -
             If flags is PALF_TEMPORARY set, the contact should be added
             temorarily and invisiblely, just to get the user info (??)
             -
  }
const
  PS_ADDTOLIST = '/AddToList';

  {
    CCSDATA: Yes
    wParam : flags
    lParam : Pointer to a null terminated string
    Affect : Send an instant message
    Returns: an hProcess corresponding to an ACK which will be sent after
             the hProcess.
    Notes:  type=ACKTYPE_MESSAGE, result=ACKRESULT_SUCCESS/FAILURE,
             lParam=ansi error message or NIL
             -
             here's the deal, you must return a 'seq' from this service
             which you have to ack when the message actually get's sent,
             or send a fake ack sometime soon if you can't find out if the message
             was successfully received with the protocol that you're using.
             -
             this event is NOT added to the database automatically.
  }
  PSS_MESSAGE  = '/SendMsg';

  {
    CCSDATA: Yes
    wParam : 0
    lParam : 0
    Affect : Send a request to retrieve TMCONTACT's mode message, see notes
    Returns: a hProcess which will be ack'd later, NULL(0) on failure
    Notes  : the reply will come in a form of an ack :

             type=ACKTYPE_AWAYMSG, result=ACKRESULT_SUCCESS/FAILURE,
             lParam=pointer to a null terminated string the containing message
  }
  PSS_GETAWAYMSG = '/GetAwayMsg';

{ Proto/RecvMessage
  Copies a message from a PROTORECVEVENT event into the database
  wParam = 0 (unused)
  lParam = CCSDATA*
  Returns the result of MS_DB_EVENT_ADD
}
  PSR_MESSAGE  = '/RecvMessage';

{$ENDIF}

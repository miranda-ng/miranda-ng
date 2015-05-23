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
  PF1_URLSEND       = $00000004; // supports separate URL sending
  PF1_URLRECV       = $00000008; // supports separate URL receiving
  PF1_URL           = (PF1_URLSEND or PF1_URLRECV);
  PF1_FILESEND      = $00000010; // supports file sending
  PF1_FILERECV      = $00000020; // supports file receiving
  PF1_FILE          = (PF1_FILESEND or PF1_FILERECV);
  PF1_MODEMSGSEND   = $00000040; // supports broadcasting away messages
  PF1_MODEMSGRECV   = $00000080; // supports reading others' away messages
  PF1_MODEMSG       = (PF1_MODEMSGSEND or PF1_MODEMSGRECV);
  PF1_SERVERCLIST   = $00000100; // contact lists are stored on the server, not locally. See notes below
  PF1_AUTHREQ       = $00000200; // will get authorisation requests for some or all contacts
  PF1_ADDED         = $00000400; // will get 'you were added' notifications
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
  PF2_SHORTAWAY  = $00000004; // Away on ICQ, BRB on MSN
  PF2_LONGAWAY   = $00000008; // NA on ICQ, Away on MSN
  PF2_LIGHTDND   = $00000010; // Occupied on ICQ, Busy on MSN
  PF2_HEAVYDND   = $00000020; // DND on ICQ
  PF2_FREECHAT   = $00000040;
  PF2_OUTTOLUNCH = $00000080;
  PF2_ONTHEPHONE = $00000100;
  PF2_IDLE       = $00000200; //added during 0.3.4 (2004/09/13)

  PFLAGNUM_3 = 3; //the status modes that the protocol supports
                  //away-style messages for. Uses the PF2_ flags.

  PFLAGNUM_4           = 4;         // v0.3+: flag asking a protocol plugin how auths are handled
  PF4_FORCEAUTH        = $00000001; // protocol has to send auth's for things to work
  PF4_FORCEADDED       = $00000002; // protocol has to tell people that they were added (otherwise things don't work)
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
  PFLAG_UNIQUEIDSETTING      = 300; // v0.3+: returns the DB setting name (e.g. szProto=ICQ, szSetting=UIN) that has the ID which makes this user unique on that system (0.3a ONLY), the string is statically allocated so no need to free()
  PFLAG_MAXLENOFMESSAGE      = 400; // v0.3.2+: return the maximum length of an instant message, lParam=(LPARAM)hContact
  {
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

    The same PF2_  status flags are used in the reply.

    Added during 0.3.4 (2004/09/14)
  }

  PFLAGNUM_5 = 5;

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

  // for PS_FILERESUME

  FILERESUME_OVERWRITE = 1;
  FILERESUME_RESUME    = 2;
  FILERESUME_RENAME    = 3;
  FILERESUME_SKIP      = 4;

const
  PSR_UNICODE = 1;

type
  PPROTOSEARCHRESULT = ^TPROTOSEARCHRESULT;
  TPROTOSEARCHRESULT = record
    cbSize   : int;
    nick     : TFNAMECHAR;
    firstName: TFNAMECHAR;
    lastName : TFNAMECHAR;
    email    : TFNAMECHAR;
    id       : TFNAMECHAR;
    flags    : int;
    reserved : array [0..(8*SizeOf(THANDLE) div SizeOf(dword))-1] of byte;
    // Protocols may extend this structure with extra members at will and supply
    // a larger cbSize to reflect the new information, but they must not change
    // any elements above this comment
    // The 'reserved' field is part of the basic structure, not space to
    // overwrite with protocol-specific information.
    // If modules do this, they should take steps to ensure that information
    // they put there will be retained by anyone trying to save this structure.
  end;

  PPROTOSEARCHBYNAME = ^TPROTOSEARCHBYNAME;
  TPROTOSEARCHBYNAME = record
    pszNick     : TChar;
    pszFirstName: TChar;
    pszLastName : TChar;
  end;

  PPROTORECVEVENT = ^TPROTORECVEVENT;
  TPROTORECVEVENT = record
    flags           : dword;
    timestamp       : dword;
    szMessage       : TChar;
    lParam          : LPARAM;
    pCustomData     : pointer;
    cbCustomDataSize: dword;
  end;

  PPROTORECVFILE = ^TPROTORECVFILE;
  TPROTORECVFILE = record
    flags        : dword;
    timestamp    : dword;   // unix time
    szDescription: PAnsiChar;
    pFiles       : ^PAnsiChar;     // pointer to an array of PAnsiChar's
    lParam       : LPARAM;
  end;

  PPROTOFILERESUME = ^TPROTOFILERESUME;
  TPROTOFILERESUME = record
    action    : int;        // FILERESUME_* flag
    szFilename: TFNAMECHAR; // full path, only valid if action=FILERESUME_RENAME
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

  {
    wParam : whichIcon
    lParam : 0
    Affect : Loads one of the protocol-sspecific icons
    Returns: the HICON or NULL on failure, the returned icon
             must be DestroyIcon()ed, the UI should overlay
             the online icon with further UI-specified icon to
             repressent the exact status mode.
  }
  PLI_PROTOCOL = $1;     // An icon representing the protocol (eg the multicoloured flower for ICQ)
  PLI_ONLINE   = $2;     // Online state icon for that protocol (eg green flower for ICQ)
  PLI_OFFLINE  = $3;     // Offline state icon for that protocol (eg red flower for ICQ)

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
  PS_SETAWAYMSGW = '/SetAwayMsgW';

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
    wParam : TMEVENT
    lParam : 0
    Affect : allow 'somebody' to add the user to their contact list, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : Auth request come in the form of an event added to the database
             for the NULL(0) user, the form is:
             -
             protocolSpecific: dword;
             nick, firstname, lastName, e-mail, requestReason: ASCIIZ;
             -
             TMEVENT musts be the handle of such an event, one or more
             fields may be empty if the protocol doesn't support them
  }
  PS_AUTHALLOW = '/Authorize';

  {
    wParam : TMEVENT
    lParam : TChar - Reason
    Affect : Deny an authorisation request
    Returns: 0 on success, [non zero] on failure
    Notes  : Protocol modules must be able to cope with lParam=NULL(0)
  }
  PS_AUTHDENY  = '/AuthDeny';
  PS_AUTHDENYW = '/AuthDenyW';

  {
    Send a "You were added" event
    wParam=lParam=0
    Returns 0 on success, nonzero on failure
  }
  PSS_ADDED = '/YouWereAdded';

{ Create account manager UI form
  wParam=0
  lParam=(LPARAM)(HWND)hwndAccMgr
  Returns handle on newly created form.
  Size for best fit is 186x134 DLUs, please avoid groupboxes
  paddind and advanced options. This should provide minimal setup
  for initial connect.
}
  PS_CREATEACCMGRUI = '/CreateAccMgrUI';
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
  PS_BASICSEARCHW = '/BasicSearchW';

  {
    wParam : 0
    lParam : Pointer to a NULL terminated string containing the e-mail to search for
    Affect : Search for user(s) by e-mail address, see notes
    Returns: A HANDLE to the search, or NULL(0) on failure
    Notes  : Results are returned as for PS_BASICSEARCH, this service
             is only available if the PF1_USERIDISEMAIL flag is set for caps --
             -
             This service with the above service should be mapped to the same
             function if the aforementioned flag is set.
    Version: v0.1.2.1+
  }
  PS_SEARCHBYEMAIL  = '/SearchByEmail';
  PS_SEARCHBYEMAILW = '/SearchByEmailW';

  {
    wParam : 0
    lParam : Pointer to a TPROTOSEARCHBYNAME structure
    Affect : Search for users by name, see notes
    Returns: Handle to the search, NULL(0) on failure
    Notes  : this service is only available, if PF1_SEARCHBYNAME capability is set.
             Results are returned in the same manner as PS_BASICSEEARCH
    Version: v0.1.2.1+
  }
  PS_SEARCHBYNAME  = '/SearchByName';
  PS_SEARCHBYNAMEW = '/SearchByNameW';

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
    wParam : MAKEWPARAM(flags, iContact)
    lParam : TMEVENT
    Affects: Add a contact to the contact list given an auth/added/contacts events, see notes
    Returns: A TMCONTACT or NULL(0) on failure
    Notes  : TMEVENT must be either EVENTTYPE_AUTHREQ or EVENTTYPE_ADDED
             flags are the same as PS_ADDTOLIST,
             -
             iContacts is only used for contacts vents, it is 0-based index
             of the contacts in the event to add, there's no way to add two or more
             contacts at once, you should just call this as many times as needed.
  }
  PS_ADDTOLISTBYEVENT = '/AddToListByEvent';

  {
    wParam : HFILETRANSFER
    lParam : Pointer to a initalised TPROTOFILERESUME
    Affect : Informs the protocol of the user's chosen resume behaviour, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : If the protocol supports file resume (caps: PF1_FILERESUME) then before
             each file receive begins it will broadcast an ack with :

             type=ACKTYPE_FILE, result=ACKRESULT_RESUME, hProcess=hFileTransfer,
             lParam = TPROTOFILETRANSFERSTATUS.

             If the UI processes this ack it must return a [non zero] valuee from it's
             hook, it all the hooks complete without returning [non zero] then the
             protocol will assume that no resume UI was available and will continue
             to receive the file with a default behaviour (default: overwrite)
             -
             If a hook does return [non zero] then that UI MUST call this service,
             PS_FILERESUME at some point.
             When the protocol module receives this call it will proceed wit the
             file recieve usingg the given information.
             -
             Having sasid that, PS_FILERESUME MUST be called, it is also
             acceptable to completely abort the transfer instead, i.e. the file
             exists locally and the user doesn't want to overwrite or resume or
             reget.
    Version: v0.1.2.2+
  }
  PS_FILERESUME  = '/FileResume';
  PS_FILERESUMEW = '/FileResumeW';

{
  Asks a protocol to join the chatroom from contact  v0.8.0+
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)0
  Returns 0 on success, nonzero on failure
}
  PS_JOINCHAT = '/JoinChat';

{
  Asks a protocol to leave the chatroom from contact  v0.8.0+
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)0
  Returns 0 on success, nonzero on failure
}
  PS_LEAVECHAT = '/LeaveChat';

{
  Asks a protocol to read contact information and translate them (for a lookup fields)  v0.8.0+
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(DBCONTACTGETSETTING*)&dbcgs
  The flag PF4_INFOSETTINGSVC indicates that a protocol supports this. Basically it should
  do the same as MS_DB_CONTACT_GETSETTING_STR, except that for a lookup settings (e.g. Language)
  it returns string instead of an ID stored in the database.
  Caller is responsible for free()ing dbcgs.pValue->pszVal and pbVal if they are
  returned. You must **NOT** do this from your version of free() you have to use Miranda's free()
  you can get a function pointer to Miranda's free() via MS_SYSTEM_GET_MMI, see m_system.h
  Returns 0 on success or nonzero if the setting name was not found or hContact
  was invalid
}
  PS_GETINFOSETTING = '/GetInfoSetting';

{
 Asks protocol for the status message for a status
 wParam=(word) 0 for current status or a status id
 lParam=SGMA_xxx
 Returns status msg or NULL if there is none.  The protocol have to handle only the current
 status. Handling messages for other statuses is optional.
 Remember to mir_free the return value
}
  SGMA_UNICODE = 1;        // return Unicode status

  PS_GETMYAWAYMSG = '/GetMyAwayMsg';

{
  Set nickname for the user
  wParam=(WPARAM)SMNN_xxx
  lParam=(LPARAM)(char *) The new nickname for the user
  return=0 for success
}
  SMNN_UNICODE = 1;        // return Unicode status

  PS_SETMYNICKNAME = '/SetNickname';

{
  Get the max allowed length for the user nickname
  Optional, default value is 1024
  wParam=(WPARAM)0
  lParam=(LPARAM)0
  return= <=0 for error, >0 the max length of the nick
}
  PS_GETMYNICKNAMEMAXLENGTH = '/GetMyNicknameMaxLength';

// WAYD = What are you doing
  WAYD_UNICODE = 1;        // return Unicode texts

{
  Get the WAYD message for the user
  wParam=(WPARAM)WAYD_xxx
  lParam=(LPARAM)0
  Returns the text or NULL if there is none. Remember to mir_free the return value.
}
  PS_GETMYWAYD = '/GetMyWAYD';

{
  Sets the WAYD message for the user
  wParam=(WPARAM)WAYD_xxx
  lParam=(LPARAM)(WCHAR * or char *)The message
  Returns 0 on success, nonzero on failure
}
  PS_SETMYWAYD = '/SetMyWAYD';

{
  Get the max allowed length that a WAYD message can have
  Optional, default value is 1024
  wParam=(WPARAM)0
  lParam=(LPARAM)0
  Returns the max length
}
  PS_GETMYWAYDMAXLENGTH = '/GetMyWAYDMaxLength';

{
  Get the unread email message count, optional
  wParam = (WPARAM)0
  lParam = (LPARAM)0
  Returns the number of unread emails
}
  PS_GETUNREADEMAILCOUNT = '/GetUnreadEmailCount';

// these should be called with CallContactService()

{<</
    !IMPORTANT!
    wParam, lParam data expected declarations should be treated with
    one level of indirection, where it says (CCSDATA: Yes)
    should be :

    What you *actually* get in the service:

    wParam = 0
    lParam = pCCSDATA

    CCSDATA contains the ..wParam, ..lParam, hContact data declared with each service,
    so the wParam, lParam passed does not contain the data itself, but lParam
    contains a pointer to a structure which contains the data.

/>>}

  {
    CCSDATA: Yes
    wParam : flags
    Param : 0

    Affect : Updates a contact's details from the server, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  :

             flags which may have SGIF_MINIMAL set to only get
             "basic" information, such as nickname, email address.

             PCCSDATA(lParam)^.hContact has the TMCONTACT handle to get user
             information for.

             Will update all the information in the database and then
             send acks with :

             type=ACKTYPE_GETINFO, result=ACKRESULT_SUCCESS, hProcess=nReplies, lParam=thisReply
             -
             Since some protocol do not allow the module to tell when it has
             got all the information so it can send a final ack, one
             ack will be sent after each chunk of data has been received,
             -
             nReplies contains the number of distinct acks
             that will be sent to get all the information, 'thisReply'
             is the zero based index of this ack.
             When thisReply=0 the minimal information has just been received,
             all other numbering is arbitrary.

  }
  PSS_GETINFO = '/GetInfo';

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
//  PSS_MESSAGEW = '/SendMsgW';

  {
    CCSDATA: Yes
    wParam : flags
    lParam : null terminated string to the URL, see notes
    Affect : Send a URL message, see notes
    Returns: A hProcess which will be ack'd later
    Notes  : lParam may contain TWO strings, the first for URL, the second for
             description, in the format :
             <url>#0<desc>#0 or <url>#0#0
             Will send an ack for hProcess when the URL actually gets sent
             type=ACKTYPE_URL, result=ACKRESULT_SUCCESS/FAILURE,
             lParam=ansi error message or NIL
             -
             protocol modules are free to define flags starting at $10000
             -
             The event will *not* be added to the database automatically
  }
  PSS_URL = '/SendUrl';

  {
    CCSDATA: Yes
    wParam : MAKEWPARAM(flags)
    lParam : Pointer to hContactsList
    Affect : Send a set of contacts, see notes
    Returns: A hProcess which will be ack, NULL(0) on failure
    Notes  : hContactsList is an array of nContacts handles to contacts,
             if this array includes one or more contains that can not be transferred
             using this protocol the function will fail.
             -
             Will send an ack when the contacts actually get sent:

             type=ACKTYPE_CONTACTS, result=ACKRESULT_SUCCESS/FAILURE,
             lParam=ansi error message or NIL
             -
             No flags have ben defined yet,
             -
             The event will *not* be added to the database automatically
  }
  PSS_CONTACTS = '/SendContacts';

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

  {
    CCSDATA: Yes
    wParam : status_mode
    lParam : 0
    Affect : Set the status mode the user will appear in to a user, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : If status_mode = 0 then revert to normal state for the user,
             ID_STATUS_ONLINE is possible if PF1_VISLIST
             ID_STATUS_ONLINE is possible if PF1_INDIVSTATUS
  }
  PSS_SETAPPARENTMODE = '/SetApparentMode';

  // only valid if caps support IM xfers
  {
    CCSDATA: Yes
    wParam : HTRANSFER
    lParam : null terminated string containing the path
    Affect : Allow a file transfer to begin, see notes
    Returns: A handle to the transfer to be used from now on.
    Notes  : If the path does not point to a directory then:
             if a single file is being transfered and the protocol supports
             file renaming (PF1_CANRENAMEFILE) then the file is given
             this name, othewise the file is removed and file(s) are placed
             into the resulting directory.
             -
             File transfers are marked by a EVENTTYPE_FILE added to the database.
             The format is :
             hTransfer: dword
             filename(s), description: ASCIIZ
  }
  PSS_FILEALLOW  = '/FileAllow';
  PSS_FILEALLOWW = '/FileAllowW';

  {
    CCSDATA: Yes
    wParam : HTRANSFER
    lparam : Pointer to a buffer to be filled with reason
    Affect : Refuses a file transfer request
    Returns: 0 on success, [non zero] on failure
  }
  PSS_FILEDENY  = '/FileDeny';
  PSS_FILEDENYW = '/FileDenyW';

  {
    CCSDATA: Yes
    wParam : HTRANSFER
    lParam : 0
    Affect : Cancel an in-progress file transfer
    Returns: 0 on success, [non zero] on failure
  }
  PSS_FILECANCEL = '/FileCancel';

  {
    CCSDATA: Yes
    wParam : null terminated string containing description
    lParam : pointer to an array of PAnsiChar's containing file paths/directories
    Affect : Start a file(s) send, see notes
    Returns: A HTRANSFER handle on success, NULL(0) on failur
    Notes  : All notifications are done thru acks :
             -
             type=ACKTYPE_FILE, if result=ACKRESULT_FAILED then
             lParam=null terminated string containing reason
  }
  PSS_FILE  = '/SendFile';
  PSS_FILEW = '/SendFileW';

  {
    Send an auth request
    wParam=0
    lParam=TChar szMessage
    Returns 0 on success, nonzero on failure
  }
  PSS_AUTHREQUEST  = '/AuthRequest';
  PSS_AUTHREQUESTW = '/AuthRequestW';

  {
    Send "User is Typing" (user is typing a message to the user) v0.3.3+
    wParam=(WPARAM)(HANDLE)hContact
    lParam=(LPARAM)(int)typing type - see PROTOTYPE_SELFTYPING_X defines in m_protocols.h
  }
  PSS_USERISTYPING = '/UserIsTyping';

// Receiving Services
{>>/
  Receiving Services:
  Before a message is sent to /RecvMessage it goes through a MS_PROTO_CHAINRECV
  which allows any other module to change data (for decryption, etc),
  this then reaches /RecvMessage.

  This does not have to be the same structure/memory contained within that
  structure that started the chain call.

  /RecvMessage adds the event to the database, any other modules who
  are interested in what message the user will see should hook at this point.
/>>}

  {
    CCSDATA: Yes
    wParam : 0
    lParam : Pointer to a TPROTORECVEVENT
    Affect : An instant message has beeen received, see notes
    Returns: 0 - success, other failure
    // handle to the newly added event, or NULL on failure
    Notes  : lParam^.lParam^.szMessage has the message, see structure above
             stored as DB event EVENTTYPE_MESSAGE, blob contains message
             string without null termination.
  }

{ Proto/RecvMessage
Copies a message from a PROTORECVEVENT event into the database
  wParam = 0 (unused)
  lParam = CCSDATA*
Returns the result of MS_DB_EVENT_ADD
}
  PSR_MESSAGE  = '/RecvMessage';
//  PSR_MESSAGEW = '/RecvMessageW';

  MS_PROTO_RECVMSG = 'Proto/RecvMessage';


{ Proto/AuthRecv
Copies the EVENTTYPE_AUTHREQUEST event from PROTORECVEVENT into DBEVENTINFO and adds it
  wParam = char* : protocol name
  lParam = PROTORECVEVENT*
Returns the result of MS_DB_EVENT_ADD
}
  PSR_AUTH = '/RecvAuth';
  MS_PROTO_AUTHRECV = 'Proto/AuthRecv';

//File(s) have been received
//wParam = 0
//lParam = (LPARAM)(PROTORECVFILET*)&prf

type
  TPROTORECVFILET = record
    flags:dword;
    timestamp:dword;    // unix time
    szDescription:TChar;
    fileCount:int;
    ptszFiles:^TChar;
    lParam:LPARAM;     // extra space for the network level protocol module
  end;

const
  PSR_FILE = '/RecvFile';

  MS_PROTO_RECVFILET = 'Proto/RecvFileT';

  {
    CCSDATA: Yes
    wParam : 0
    lParam : Pointer to a TPROTORECVEVENT, see notes
    Affect : A URL has been received
    Notes  : szMessage is encoded the same as PSS_URL
             -
             Stored in the database : EVENTTYPE_URL, blob contains message
             without null termination
  }
  PSR_URL = '/RecvUrl';

  {
    CCSDATA: Yes
    wParam : 0
    lParam : Pointer to a TPROTORECVEVENT
    Affect : Contacts have been received, see notes
    Notes  : pre.szMessage is actually a PROTOSEARCHRESULT list
             pre.lParam is the number of contains in that list.
             pre.flags can contain PREF_UTF defining the strings as utf-8 encoded (0.7.0+)
             -
             PS_ADDTOLIST can be used to add contacts to the list
             -
             repeat [
                ASCIIZ userNick
                ASCIIZ userId
             ]
             userNick should be a human-readable description of the user. It need not
             be the nick, or even confined to displaying just one type of
             information. The dbe.flags can contain DBEF_UTF defining userNick as
             utf-8 encoded.
             userId should be a machine-readable representation of the unique
             protocol identifying field of the user. Because of the need to be
             zero-terminated, binary data should be converted to text.
             Use PS_ADDTOLISTBYEVENT to add the contacts from one of these to the list.
  }
  PSR_CONTACTS = '/RecvContacts';

  {
    CCSDATA: Yes
    wParam : status_mode
    lParam : Pointer to a TPROTORECVEVENT structure
    Affect : An away message reply has been received
  }
  PSR_AWAYMSG = '/RecvAwayMsg';


{$ENDIF}

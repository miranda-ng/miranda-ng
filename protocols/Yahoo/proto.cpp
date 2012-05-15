/*
 * $Id: proto.cpp 13497 2011-03-25 04:55:36Z borkra $
 *
 * myYahoo Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */

#include "yahoo.h"

#include <shlwapi.h>
#include <win2k.h>

#include <m_idle.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_userinfo.h>

#include "resource.h"
#include "file_transfer.h"

#ifdef _MSC_VER
	#pragma warning(disable:4355)
#endif

CYahooProto::CYahooProto( const char* aProtoName, const TCHAR* aUserName ) :
	m_bLoggedIn( FALSE ), poll_loop( 0 ) 
{
	m_iVersion = 2;
	m_tszUserName = mir_tstrdup( aUserName );
	m_szModuleName = mir_strdup( aProtoName );

	m_startStatus = ID_STATUS_ONLINE;
	m_connections = NULL;
	m_connection_tags = 0;
	
	logoff_buddies();

	SkinAddNewSoundExT("mail", m_tszUserName, LPGENT("New E-mail available in Inbox"));
	
	LoadYahooServices();
	IconsInit();
	InitCustomFolders();
}

CYahooProto::~CYahooProto()
{
	if (m_bLoggedIn)
		logout();
	
	DebugLog("Logged out");

	DestroyHookableEvent(hYahooNudge);

	MenuUninit();
	
	mir_free( m_szModuleName );
	mir_free( m_tszUserName );

	FREE(m_startMsg);
	FREE(m_pw_token); 
	
	Netlib_CloseHandle( m_hNetlibUser );
}

////////////////////////////////////////////////////////////////////////////////////////
// OnModulesLoadedEx - performs hook registration

//static COLORREF crCols[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

INT_PTR CYahooProto::OnModulesLoadedEx( WPARAM, LPARAM )
{
	YHookEvent( ME_USERINFO_INITIALISE, 		&CYahooProto::OnUserInfoInit );
	YHookEvent( ME_IDLE_CHANGED, 				&CYahooProto::OnIdleEvent);
	YHookEvent( ME_CLIST_PREBUILDCONTACTMENU, 	&CYahooProto::OnPrebuildContactMenu );

	TCHAR tModuleDescr[ 100 ];
	mir_sntprintf(tModuleDescr, SIZEOF(tModuleDescr), TranslateT( "%s plugin connections" ), m_tszUserName);
	
	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
#ifdef HTTP_GATEWAY
	nlu.flags = NUF_OUTGOING | NUF_HTTPGATEWAY| NUF_HTTPCONNS | NUF_TCHAR;
#else
  	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
#endif
	nlu.szSettingsModule = m_szModuleName;
	nlu.ptszDescriptiveName = tModuleDescr;
	
#ifdef HTTP_GATEWAY
	// Here comes the Gateway Code! 
	nlu.szHttpGatewayHello = NULL;
	nlu.szHttpGatewayUserAgent = "User-Agent: Mozilla/4.01 [en] (Win95; I)";
 	nlu.pfnHttpGatewayInit = YAHOO_httpGatewayInit;
	nlu.pfnHttpGatewayBegin = NULL;
	nlu.pfnHttpGatewayWrapSend = YAHOO_httpGatewayWrapSend;
	nlu.pfnHttpGatewayUnwrapRecv = YAHOO_httpGatewayUnwrapRecv;
#endif	
	
	m_hNetlibUser = ( HANDLE )YAHOO_CallService( MS_NETLIB_REGISTERUSER, 0, ( LPARAM )&nlu );
	MenuContactInit();
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// AddToList - adds a contact to the contact list

HANDLE CYahooProto::AddToList( int flags, PROTOSEARCHRESULT* psr )
{
	DebugLog("[YahooAddToList] Flags: %d", flags);

	if (!m_bLoggedIn){
		DebugLog("[YahooAddToList] WARNING: WE ARE OFFLINE!");
		return 0;
	}

	if (psr == NULL || psr->cbSize != sizeof( PROTOSEARCHRESULT )) {
		DebugLog("[YahooAddToList] Empty data passed?");
		return 0;
	}

	char *id = psr->flags & PSR_UNICODE ? mir_utf8encodeW((wchar_t*)psr->id) : mir_utf8encode((char*)psr->id);
	HANDLE hContact = getbuddyH(id);
	if (hContact != NULL) {
		if (DBGetContactSettingByte(hContact, "CList", "NotOnList", 0)) {
			DebugLog("[YahooAddToList] Temporary Buddy:%s already on our buddy list", id);
			//return 0;
		} else {
			DebugLog("[YahooAddToList] Buddy:%s already on our buddy list", id);
			mir_free(id);
			return 0;
		}
	} else if (flags & PALF_TEMPORARY ) { /* not on our list */
		DebugLog("[YahooAddToList] Adding Temporary Buddy:%s ", id);
	}

	int protocol = psr->reserved[0];
	DebugLog("Adding buddy:%s", id);
	hContact = add_buddy(id, id, protocol, flags);
	mir_free(id);
	return hContact;
}

HANDLE __cdecl CYahooProto::AddToListByEvent( int flags, int /*iContact*/, HANDLE hDbEvent )
{
	DBEVENTINFO dbei;
	HANDLE hContact;

	DebugLog("[YahooAddToListByEvent]");
	if ( !m_bLoggedIn )
		return 0;

	memset( &dbei, 0, sizeof( dbei ));
	dbei.cbSize = sizeof( dbei );

	if (( dbei.cbBlob = YAHOO_CallService( MS_DB_EVENT_GETBLOBSIZE, ( LPARAM )hDbEvent, 0 )) == -1 ) {
		DebugLog("[YahooAddToListByEvent] ERROR: Can't get blob size.");
		return 0;
	}

	DebugLog("[YahooAddToListByEvent] Got blob size: %lu", dbei.cbBlob);
	dbei.pBlob = ( PBYTE )_alloca( dbei.cbBlob );
	if ( YAHOO_CallService( MS_DB_EVENT_GET, ( WPARAM )hDbEvent, ( LPARAM )&dbei )) {
		DebugLog("[YahooAddToListByEvent] ERROR: Can't get event.");
		return 0;
	}

	if ( dbei.eventType != EVENTTYPE_AUTHREQUEST ) {
		DebugLog("[YahooAddToListByEvent] ERROR: Not an authorization request.");
		return 0;
	}

	if ( strcmp( dbei.szModule, m_szModuleName )) {
		DebugLog("[YahooAddToListByEvent] ERROR: Not Yahoo protocol.");
		return 0;
	}

	//Adds a contact to the contact list given an auth, added or contacts event
//wParam=MAKEWPARAM(flags,iContact)
//lParam=(LPARAM)(HANDLE)hDbEvent
//Returns a HANDLE to the new contact, or NULL on failure
//hDbEvent must be either EVENTTYPE_AUTHREQ or EVENTTYPE_ADDED
//flags are the same as for PS_ADDTOLIST.
//iContact is only used for contacts events. It is the 0-based index of the
//contact in the event to add. There is no way to add two or more contacts at
//once, you should just do lots of calls.

	/* TYPE ADDED
		blob is: uin(DWORD), hcontact(HANDLE), nick(ASCIIZ), first(ASCIIZ), 
		last(ASCIIZ), email(ASCIIZ) 
	
	   TYPE AUTH REQ
		blob is: uin(DWORD), hcontact(HANDLE), nick(ASCIIZ), first(ASCIIZ), 
		last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
	*/
	memcpy(&hContact,( char* )( dbei.pBlob + sizeof( DWORD ) ), sizeof(HANDLE)); 

	if (hContact != NULL) {
		DebugLog("Temp Buddy found at: %p ", hContact);
	} else
		DebugLog("hContact NULL???");

	return hContact;
}

////////////////////////////////////////////////////////////////////////////////////////
// AuthAllow - processes the successful authorization

int CYahooProto::Authorize( HANDLE hdbe )
{
	DebugLog("[YahooAuthAllow]");
	if ( !m_bLoggedIn ) {
		DebugLog("[YahooAuthAllow] Not Logged In!");
		return 1;
	}

	DBEVENTINFO dbei;
	memset( &dbei, 0, sizeof( dbei ));
	dbei.cbSize = sizeof( dbei );
	if (( dbei.cbBlob = YAHOO_CallService( MS_DB_EVENT_GETBLOBSIZE, ( WPARAM )hdbe, 0 )) == -1 )
		return 1;

	dbei.pBlob = ( PBYTE )_alloca( dbei.cbBlob );
	if ( YAHOO_CallService( MS_DB_EVENT_GET, ( WPARAM )hdbe, ( LPARAM )&dbei ))
		return 1;

	if ( dbei.eventType != EVENTTYPE_AUTHREQUEST )
		return 1;

	if ( strcmp( dbei.szModule, m_szModuleName ))
		return 1;

	HANDLE hContact;
	memcpy(&hContact,( char* )( dbei.pBlob + sizeof( DWORD ) ), sizeof(HANDLE)); 

	/* Need to remove the buddy from our Miranda Lists */
	DBVARIANT dbv;
	if (hContact != NULL && !DBGetContactSettingString( hContact, m_szModuleName, YAHOO_LOGINID, &dbv )){
		DebugLog("Accepting buddy:%s", dbv.pszVal);    
		accept(dbv.pszVal, GetWord(hContact, "yprotoid", 0));
		DBFreeVariant(&dbv);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// AuthDeny - handles the unsuccessful authorization

int CYahooProto::AuthDeny( HANDLE hdbe, const TCHAR* reason )
{
	DebugLog("[YahooAuthDeny]");
	if ( !m_bLoggedIn )
		return 1;

	DBEVENTINFO dbei;
	memset( &dbei, 0, sizeof( dbei ));
	dbei.cbSize = sizeof( dbei );

	if (( dbei.cbBlob = YAHOO_CallService( MS_DB_EVENT_GETBLOBSIZE, ( WPARAM )hdbe, 0 )) == -1 ){
		DebugLog("[YahooAuthDeny] ERROR: Can't get blob size");
		return 1;
	}

	dbei.pBlob = ( PBYTE )alloca( dbei.cbBlob );
	if ( YAHOO_CallService( MS_DB_EVENT_GET, ( WPARAM )hdbe, ( LPARAM )&dbei )){
		DebugLog("YahooAuthDeny - Can't get db event!");
		return 1;
	}

	if ( dbei.eventType != EVENTTYPE_AUTHREQUEST ){
		DebugLog("YahooAuthDeny - not Authorization event");
		return 1;
	}

	if ( strcmp( dbei.szModule, m_szModuleName )){
		DebugLog("YahooAuthDeny - wrong module?");
		return 1;
	}

	HANDLE hContact;
	memcpy(&hContact,( char* )( dbei.pBlob + sizeof( DWORD ) ), sizeof(HANDLE)); 

	/* Need to remove the buddy from our Miranda Lists */
	DBVARIANT dbv;
	if (hContact != NULL && !DBGetContactSettingString( hContact, m_szModuleName, YAHOO_LOGINID, &dbv )){
		char *u_reason;
		
		u_reason = mir_utf8encodeT(reason);
		
		DebugLog("Rejecting buddy:%s msg: %s", dbv.pszVal, u_reason);    
		reject(dbv.pszVal, GetWord(hContact, "yprotoid", 0), u_reason);
		DBFreeVariant(&dbv);
		YAHOO_CallService( MS_DB_CONTACT_DELETE, (WPARAM) hContact, 0);
		
		mir_free(u_reason);
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSR_AUTH

int __cdecl CYahooProto::AuthRecv( HANDLE hContact, PROTORECVEVENT* pre )
{
	DebugLog("[YahooRecvAuth] ");
	DBDeleteContactSetting(hContact,"CList","Hidden");

	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = m_szModuleName;
	dbei.timestamp = pre->timestamp;
	dbei.flags = pre->flags & (PREF_CREATEREAD?DBEF_READ:0);
	dbei.flags |= (pre->flags & PREF_UTF) ? DBEF_UTF : 0;
	dbei.eventType = EVENTTYPE_AUTHREQUEST;

	/* Just copy the Blob from PSR_AUTH event. */
	dbei.cbBlob = pre->lParam;
	dbei.pBlob = (PBYTE)pre->szMessage;

	CallService(MS_DB_EVENT_ADD,(WPARAM)NULL,(LPARAM)&dbei);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSS_AUTHREQUEST

int __cdecl CYahooProto::AuthRequest( HANDLE hContact, const TCHAR* msg )
{	
	DebugLog("[YahooSendAuthRequest]");
	
	if (hContact && m_bLoggedIn) {
		if (hContact) {
			DBVARIANT dbv;
			if (!DBGetContactSettingString(hContact, m_szModuleName, YAHOO_LOGINID, &dbv )) {
				char *u_msg;
				
				u_msg = mir_utf8encodeT(msg);
				DebugLog("Adding buddy:%s Auth:%s", dbv.pszVal, u_msg);
				AddBuddy( dbv.pszVal, GetWord(hContact, "yprotoid", 0), "miranda", u_msg );
				SetString(hContact, "YGroup", "miranda");
				DBFreeVariant( &dbv );
				
				mir_free(u_msg);
				return 0; // Success
			}
		}
	}
	
	return 1; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// ChangeInfo 

HANDLE __cdecl CYahooProto::ChangeInfo( int /*iInfoType*/, void* )
{
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetCaps - return protocol capabilities bits

DWORD_PTR __cdecl CYahooProto::GetCaps( int type, HANDLE /*hContact*/ )
{
	int ret = 0;
	switch ( type ) {        
	case PFLAGNUM_1:
		ret = PF1_IM  | PF1_ADDED | PF1_AUTHREQ | PF1_MODEMSGRECV | PF1_MODEMSGSEND |  PF1_BASICSEARCH |
			PF1_EXTSEARCH | PF1_FILESEND  | PF1_FILERECV| PF1_VISLIST | PF1_SERVERCLIST ;
		break;

	case PFLAGNUM_2:
		ret = PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_ONTHEPHONE | 
			PF2_OUTTOLUNCH | PF2_INVISIBLE | PF2_LIGHTDND /*| PF2_HEAVYDND*/;
		break;

	case PFLAGNUM_3:
		ret = PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_ONTHEPHONE | 
			PF2_OUTTOLUNCH | PF2_LIGHTDND ;
		break;

	case PFLAGNUM_4:
		ret = PF4_FORCEAUTH | PF4_FORCEADDED | PF4_SUPPORTTYPING | PF4_SUPPORTIDLE
			|PF4_AVATARS | PF4_OFFLINEFILES | PF4_IMSENDUTF | PF4_IMSENDOFFLINE /* for Meta plugin*/;
		break;
	case PFLAG_UNIQUEIDTEXT:
		ret = (DWORD_PTR) Translate("ID");
		break;
	case PFLAG_UNIQUEIDSETTING:
		ret = (DWORD_PTR) YAHOO_LOGINID;
		break;
	case PFLAG_MAXLENOFMESSAGE:
		ret = 800; /* STUPID YAHOO!!! */
		break;
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetIcon - loads an icon for the contact list

HICON __cdecl CYahooProto::GetIcon( int iconIndex )
{
	if (LOWORD(iconIndex) == PLI_PROTOCOL)
	{
		if (iconIndex & PLIF_ICOLIBHANDLE)
			return (HICON)GetIconHandle(IDI_YAHOO);
		
		bool big = (iconIndex & PLIF_SMALL) == 0;
		HICON hIcon = LoadIconEx("yahoo", big);

		if (iconIndex & PLIF_ICOLIB)
			return hIcon;

		hIcon = CopyIcon(hIcon);
		ReleaseIconEx("yahoo", big);
		return hIcon;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetInfo - retrieves a contact info

void __cdecl CYahooProto::get_info_thread(HANDLE hContact) 
{
	SleepEx(500, TRUE);
	ProtoBroadcastAck(m_szModuleName, hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
}

int __cdecl CYahooProto::GetInfo( HANDLE hContact, int /*infoType*/ )
{
	YForkThread(&CYahooProto::get_info_thread, hContact);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchByEmail - searches the contact by its e-mail

HANDLE __cdecl CYahooProto::SearchByEmail( const PROTOCHAR* email )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchByName - searches the contact by its first or last name, or by a nickname

HANDLE __cdecl CYahooProto::SearchByName( const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvContacts

int __cdecl CYahooProto::RecvContacts( HANDLE /*hContact*/, PROTORECVEVENT* )
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvFile

int __cdecl CYahooProto::RecvFile( HANDLE hContact, PROTORECVFILET* evt )
{
	DBDeleteContactSetting(hContact, "CList", "Hidden");

	CCSDATA ccs = { hContact, PSR_FILE, 0, ( LPARAM )evt };
	return CallService( MS_PROTO_RECVFILE, 0, ( LPARAM )&ccs );
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvUrl

int __cdecl CYahooProto::RecvUrl( HANDLE /*hContact*/, PROTORECVEVENT* )
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendContacts

int __cdecl CYahooProto::SendContacts( HANDLE /*hContact*/, int /*flags*/, int /*nContacts*/, HANDLE* /*hContactsList*/ )
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendUrl

int __cdecl CYahooProto::SendUrl( HANDLE /*hContact*/, int /*flags*/, const char* /*url*/ )
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SetApparentMode - sets the visibility status

int __cdecl CYahooProto::SetApparentMode( HANDLE hContact, int mode )
{
	if (mode && mode != ID_STATUS_OFFLINE)
		return 1;

	int oldMode = DBGetContactSettingWord(hContact, m_szModuleName, "ApparentMode", 0);
	if (mode != oldMode)
		DBWriteContactSettingWord(hContact, m_szModuleName, "ApparentMode", (WORD)mode);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SetStatus - sets the protocol status

int __cdecl CYahooProto::SetStatus( int iNewStatus )
{
	LOG(("[SetStatus] New status %s", (char *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, iNewStatus, 0)));
	
	if (iNewStatus == ID_STATUS_OFFLINE) {
		
		logout();
		
	} else if (!m_bLoggedIn) {
		DBVARIANT dbv;
		int err = 0;
		char errmsg[80];

		if (m_iStatus == ID_STATUS_CONNECTING)
			return 0;

		YAHOO_utils_logversion();

		/*
		* Load Yahoo ID from the database.
		*/
		if (!GetString(YAHOO_LOGINID, &dbv)) {
			if (lstrlenA(dbv.pszVal) > 0) {
				lstrcpynA(m_yahoo_id, dbv.pszVal, 255);
			} else
				err++;
			DBFreeVariant(&dbv);
		} else {
			ProtoBroadcastAck(m_szModuleName, NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
			err++;
		}

		if (err) {
			lstrcpynA(errmsg, Translate("Please enter your yahoo id in Options/Network/Yahoo"), 80);
		} else {
			if (!GetString(YAHOO_PASSWORD, &dbv)) {
				CallService(MS_DB_CRYPT_DECODESTRING, lstrlenA(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
				if (lstrlenA(dbv.pszVal) > 0) {
					lstrcpynA(m_password, dbv.pszVal, 255);
				} else
					err++;

				DBFreeVariant(&dbv);
			}  else  {
				ProtoBroadcastAck(m_szModuleName, NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);
				err++;
			}

			if (err)
				lstrcpynA(errmsg, Translate("Please enter your yahoo password in Options/Network/Yahoo"), 80);
		}

		if (err != 0){
			BroadcastStatus(ID_STATUS_OFFLINE);

			ShowError(Translate("Yahoo Login Error"), errmsg);
			return 0;
		}

		if (iNewStatus == ID_STATUS_OFFLINE)
			iNewStatus = ID_STATUS_ONLINE;

		FREE(m_pw_token); // No Token yet.
		
		if (!GetString(YAHOO_PWTOKEN, &dbv)) {
			if (lstrlenA(dbv.pszVal) > 0) {
				m_pw_token = strdup(dbv.pszVal);
			} 
			
			DBFreeVariant(&dbv);
		}
		
		//DBWriteContactSettingWord(NULL, m_szModuleName, "StartupStatus", status);
		m_startStatus = iNewStatus;

		//reset the unread email count. We'll get a new packet since we are connecting.
		m_unreadMessages = 0;

		BroadcastStatus(ID_STATUS_CONNECTING);

		iNewStatus = (iNewStatus == ID_STATUS_INVISIBLE) ? YAHOO_STATUS_INVISIBLE: YAHOO_STATUS_AVAILABLE;
		YForkThread(&CYahooProto::server_main, (void *)iNewStatus);
	}
	else if (iNewStatus == ID_STATUS_INVISIBLE){ /* other normal away statuses are set via setaway */
		BroadcastStatus(iNewStatus);
		set_status(m_iStatus,NULL,(m_iStatus != ID_STATUS_ONLINE) ? 1 : 0);
	}
	else {
		/* clear out our message just in case, STUPID AA! */
		FREE(m_startMsg);

		/* now tell miranda that we are Online, don't tell Yahoo server yet though! */
		BroadcastStatus(iNewStatus);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetAwayMsg - returns a contact's away message

void __cdecl CYahooProto::get_status_thread(HANDLE hContact)
{
	int l;
	DBVARIANT dbv;
	char *gm = NULL, *sm = NULL, *fm;

	Sleep( 150 );

	/* Check Yahoo Games Message */
	if (! DBGetContactSettingString(( HANDLE )hContact, m_szModuleName, "YGMsg", &dbv )) {
		gm = strdup(dbv.pszVal);

		DBFreeVariant( &dbv );
	}

	if (! DBGetContactSettingString(hContact, "CList", "StatusMsg", &dbv )) {
		if (lstrlenA(dbv.pszVal) >= 1)
			sm = strdup(dbv.pszVal);

		DBFreeVariant( &dbv );
	} else {
		WORD status = DBGetContactSettingWord(hContact, m_szModuleName, "YStatus", YAHOO_STATUS_OFFLINE);
		sm = yahoo_status_code( yahoo_status( status ));
		if (sm) sm = strdup(sm); /* we need this to go global FREE later */
	}

	l = 0;
	if (gm)
		l += lstrlenA(gm) + 3;

	l += lstrlenA(sm) + 1;
	fm = (char *) malloc(l);

	fm[0] ='\0';
	if (gm && lstrlenA(gm) > 0) {
		/* BAH YAHOO SUCKS! WHAT A PAIN!
		find first carriage return add status message then add the rest */
		char *c = strchr(gm, '\r');

		if (c != NULL) {
			lstrcpynA(fm,gm, c - gm + 1);
			fm[c - gm + 1] = '\0';
		} else
			lstrcpyA(fm, gm);

		if (sm) {
			lstrcatA(fm, ": ");
			lstrcatA(fm, sm);
		}

		if (c != NULL)
			lstrcatA(fm, c);
	} else if (sm) {
		lstrcatA(fm, sm);
	}

	FREE(sm);

	SendBroadcast( hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, ( HANDLE )1, ( LPARAM ) fm );
}

HANDLE __cdecl CYahooProto::GetAwayMsg( HANDLE hContact )
{
	DebugLog("[YahooGetAwayMessage] ");

	if (hContact && m_bLoggedIn) {
		if (DBGetContactSettingWord(hContact, m_szModuleName, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
			return 0; /* user offline, what Status message? */

		YForkThread(&CYahooProto::get_status_thread, hContact);
		return (HANDLE)1; //Success		
	}

	return 0; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PSR_AWAYMSG

int __cdecl CYahooProto::RecvAwayMsg( HANDLE /*hContact*/, int /*statusMode*/, PROTORECVEVENT* )
{	
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSS_AWAYMSG

int __cdecl CYahooProto::SendAwayMsg( HANDLE /*hContact*/, HANDLE /*hProcess*/, const char* )
{	
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SetAwayMsg - sets the away status message

int __cdecl CYahooProto::SetAwayMsg( int status, const PROTOCHAR* msg )
{
	char *c = msg && msg[0] ? mir_utf8encodeT(msg) : NULL;
		
	DebugLog("[YahooSetAwayMessage] Status: %s, Msg: %s",(char *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, status, 0), (char*) c);
	
    if(!m_bLoggedIn){
		if (m_iStatus == ID_STATUS_OFFLINE) {
			DebugLog("[YahooSetAwayMessage] WARNING: WE ARE OFFLINE!"); 
			mir_free(c);
			return 1;
		} else {
			if (m_startMsg) free(m_startMsg);

			m_startMsg = c ? strdup(c) : NULL;

			mir_free(c);
			return 0;
		}
	}              
	
	/* need to tell ALL plugins that we are changing status */
	BroadcastStatus(status);
	
	if (m_startMsg) free(m_startMsg);
	
	/* now decide what we tell the server */
	if (c != 0) {
		m_startMsg = strdup(c);
		if(status == ID_STATUS_ONLINE) {
			set_status(YAHOO_CUSTOM_STATUS, c, 0);
		} else if(status != ID_STATUS_INVISIBLE){ 
			set_status(YAHOO_CUSTOM_STATUS, c, 1);
		}
    } else {
		set_status(status, NULL, 0);
		m_startMsg = NULL;
	}
	
	mir_free(c);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_GETMYAWAYMSG

INT_PTR __cdecl CYahooProto::GetMyAwayMsg(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn || ! m_startMsg)
		return 0;
	
	if (lParam & SGMA_UNICODE)  {
		return (INT_PTR) mir_utf8decodeW(m_startMsg);
	} else {
		return (INT_PTR) mir_utf8decodeA(m_startMsg);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// UserIsTyping - sends a UTN notification

int __cdecl CYahooProto::UserIsTyping( HANDLE hContact, int type )
{
	if (!m_bLoggedIn)
		return 0;

	char *szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if (szProto==NULL || strcmp(szProto, m_szModuleName))
		return 0;

	DBVARIANT dbv;
	if (!DBGetContactSettingString(hContact, m_szModuleName, YAHOO_LOGINID, &dbv)) {
		if (type == PROTOTYPE_SELFTYPING_OFF || type == PROTOTYPE_SELFTYPING_ON) {
			sendtyping(dbv.pszVal, GetWord(hContact, "yprotoid", 0), type == PROTOTYPE_SELFTYPING_ON?1:0);
		}
		DBFreeVariant(&dbv);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnEvent - maintain protocol events

int __cdecl CYahooProto::OnEvent( PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam )
{
	switch( eventType ) {
		case EV_PROTO_ONLOAD:    return OnModulesLoadedEx( 0, 0 );
		//case EV_PROTO_ONEXIT:    return OnPreShutdown( 0, 0 );
		case EV_PROTO_ONOPTIONS: return OnOptionsInit( wParam, lParam );

		case EV_PROTO_ONMENU:
			MenuMainInit();
			break;

		case EV_PROTO_ONRENAME:
			if ( mainMenuRoot ) {	
				CLISTMENUITEM clmi = { 0 };
				clmi.cbSize = sizeof( CLISTMENUITEM );
				clmi.flags = CMIM_NAME | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
				clmi.ptszName = m_tszUserName;
				YAHOO_CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )mainMenuRoot, ( LPARAM )&clmi );
			}
			break;

		case EV_PROTO_ONCONTACTDELETED:
			return OnContactDeleted(wParam, lParam);

		case EV_PROTO_DBSETTINGSCHANGED:
			return OnSettingChanged(wParam, lParam);
	}	
	return 1;
}

INT_PTR CALLBACK first_run_dialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			CYahooProto* ppro = (CYahooProto*)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			DBVARIANT dbv;
			if ( !ppro->GetString(YAHOO_LOGINID, &dbv))
			{
				SetDlgItemTextA(hwndDlg, IDC_HANDLE, dbv.pszVal);
				DBFreeVariant(&dbv);
			}

			if ( !ppro->GetString(YAHOO_PASSWORD, &dbv))
			{
				CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
				SetDlgItemTextA(hwndDlg, IDC_PASSWORD, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			
			SetButtonCheck( hwndDlg, IDC_YAHOO_JAPAN, ppro->GetByte( "YahooJapan", 0 ) );
			return TRUE;
		}

	case WM_COMMAND:
		if ( LOWORD( wParam ) == IDC_NEWYAHOOACCOUNTLINK ) {
			CallService( MS_UTILS_OPENURL, 
							1, 
							(( BYTE )IsDlgButtonChecked( hwndDlg, IDC_YAHOO_JAPAN ) == 1) ?
							( LPARAM ) "http://edit.yahoo.co.jp/config/eval_register" :
							( LPARAM ) "http://edit.yahoo.com/config/eval_register" 
						);
			return TRUE;
		}

		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) 
		{
			switch( LOWORD( wParam )) {
			case IDC_HANDLE:			
			case IDC_PASSWORD:
			case IDC_YAHOO_JAPAN:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (UINT)PSN_APPLY ) 
		{
			CYahooProto* ppro = (CYahooProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			char str[128];
			DBVARIANT dbv;
			BOOL reconnectRequired = FALSE;
			
			GetDlgItemTextA(hwndDlg, IDC_HANDLE, str, sizeof(str));
			
			dbv.pszVal = NULL;
			
			if ( ppro->GetString( YAHOO_LOGINID, &dbv ) || lstrcmpA( str, dbv.pszVal ))
				reconnectRequired = TRUE;
			
			if ( dbv.pszVal != NULL )
				DBFreeVariant( &dbv );
			
			ppro->SetString(YAHOO_LOGINID, str);
			GetDlgItemTextA(hwndDlg, IDC_PASSWORD, str, sizeof(str));
			
			dbv.pszVal = NULL;
			if ( ppro->GetString( YAHOO_PASSWORD, &dbv ) || lstrcmpA( str, dbv.pszVal ))
				reconnectRequired = TRUE;
			if ( dbv.pszVal != NULL )
				DBFreeVariant( &dbv );
			
			if (reconnectRequired ) {
				DBDeleteContactSetting(NULL, ppro->m_szModuleName, YAHOO_PWTOKEN);
			}
			
			CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(str), (LPARAM) str);
			ppro->SetString(YAHOO_PASSWORD, str);
			ppro->SetByte("YahooJapan", ( BYTE )IsDlgButtonChecked( hwndDlg, IDC_YAHOO_JAPAN ));
			
			if ( reconnectRequired && ppro->m_bLoggedIn )
				MessageBoxA( hwndDlg, Translate( "The changes you have made require you to reconnect to the Yahoo network before they take effect"), Translate("YAHOO Options"), MB_OK );

			return TRUE;
		}
		break;
	}

	return FALSE;
}


INT_PTR CYahooProto::SvcCreateAccMgrUI(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam (hInstance, MAKEINTRESOURCE( IDD_YAHOOACCOUNT ), 
		 (HWND)lParam, first_run_dialog, (LPARAM)this );
}


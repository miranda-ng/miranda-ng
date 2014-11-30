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

#include <win2k.h>

#include <m_options.h>
#include <m_skin.h>
#include <m_userinfo.h>

#include "resource.h"
#include "file_transfer.h"

#pragma warning(disable:4355)

CYahooProto::CYahooProto( const char* aProtoName, const TCHAR* aUserName ) :
	PROTO<CYahooProto>(aProtoName, aUserName),
	m_bLoggedIn( FALSE ),
	poll_loop( 0),
	m_chatrooms(3, ChatRoom::compare)
{
	m_connections = NULL;
	m_connection_tags = 0;

	logoff_buddies();

	SkinAddNewSoundExT("mail", m_tszUserName, LPGENT("New E-mail available in Inbox"));

	LoadYahooServices();
	IconsInit();
}

CYahooProto::~CYahooProto()
{
	if (m_bLoggedIn)
		logout();

	debugLogA("Logged out");

	DestroyHookableEvent(hYahooNudge);

	MenuUninit();

	FREE(m_startMsg);
	FREE(m_pw_token);

	Netlib_CloseHandle(m_hNetlibUser);
}

////////////////////////////////////////////////////////////////////////////////////////
// OnModulesLoadedEx - performs hook registration

int CYahooProto::OnModulesLoadedEx(WPARAM, LPARAM)
{
	HookProtoEvent( ME_USERINFO_INITIALISE, 		&CYahooProto::OnUserInfoInit );
	HookProtoEvent( ME_IDLE_CHANGED, 				&CYahooProto::OnIdleEvent);
	HookProtoEvent( ME_CLIST_PREBUILDCONTACTMENU, 	&CYahooProto::OnPrebuildContactMenu );

	TCHAR tModuleDescr[ 100 ];
	mir_sntprintf(tModuleDescr, SIZEOF(tModuleDescr), TranslateT("%s plugin connections"), m_tszUserName);

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

	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
	MenuContactInit();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// AddToList - adds a contact to the contact list

MCONTACT CYahooProto::AddToList( int flags, PROTOSEARCHRESULT* psr )
{
	debugLogA("[YahooAddToList] Flags: %d", flags);

	if (!m_bLoggedIn) {
		debugLogA("[YahooAddToList] WARNING: WE ARE OFFLINE!");
		return 0;
	}

	if (psr == NULL || psr->cbSize != sizeof( PROTOSEARCHRESULT )) {
		debugLogA("[YahooAddToList] Empty data passed?");
		return 0;
	}

	char *id = psr->flags & PSR_UNICODE ? mir_utf8encodeW((wchar_t*)psr->id) : mir_utf8encode((char*)psr->id);
	MCONTACT hContact = getbuddyH(id);
	if (hContact != NULL) {
		if (db_get_b(hContact, "CList", "NotOnList", 0)) {
			debugLogA("[YahooAddToList] Temporary Buddy:%s already on our buddy list", id);
			//return 0;
		}
		else {
			debugLogA("[YahooAddToList] Buddy:%s already on our buddy list", id);
			mir_free(id);
			return 0;
		}
	}
	else if (flags & PALF_TEMPORARY) { /* not on our list */
		debugLogA("[YahooAddToList] Adding Temporary Buddy:%s ", id);
	}

	int protocol = psr->reserved[0];
	debugLogA("Adding buddy:%s", id);
	hContact = add_buddy(id, id, protocol, flags);
	mir_free(id);
	return hContact;
}

MCONTACT __cdecl CYahooProto::AddToListByEvent( int flags, int /*iContact*/, HANDLE hDbEvent )
{
	debugLogA("[YahooAddToListByEvent]");
	if (!m_bLoggedIn)
		return 0;

	DBEVENTINFO dbei = { sizeof( dbei ) };
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == -1 ) {
		debugLogA("[YahooAddToListByEvent] ERROR: Can't get blob size.");
		return 0;
	}

	debugLogA("[YahooAddToListByEvent] Got blob size: %lu", dbei.cbBlob);
	dbei.pBlob = ( PBYTE )_alloca( dbei.cbBlob );
	if (db_event_get(hDbEvent, &dbei)) {
		debugLogA("[YahooAddToListByEvent] ERROR: Can't get event.");
		return 0;
	}

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) {
		debugLogA("[YahooAddToListByEvent] ERROR: Not an authorization request.");
		return 0;
	}

	if ( strcmp(dbei.szModule, m_szModuleName)) {
		debugLogA("[YahooAddToListByEvent] ERROR: Not Yahoo protocol.");
		return 0;
	}

	MCONTACT hContact = DbGetAuthEventContact(&dbei);
	if (hContact != NULL)
		debugLogA("Temp Buddy found at: %p ", hContact);
	else
		debugLogA("hContact NULL???");

	return hContact;
}

////////////////////////////////////////////////////////////////////////////////////////
// AuthAllow - processes the successful authorization

int CYahooProto::Authorize( HANDLE hdbe )
{
	debugLogA("[YahooAuthAllow]");
	if ( !m_bLoggedIn ) {
		debugLogA("[YahooAuthAllow] Not Logged In!");
		return 1;
	}

	DBEVENTINFO dbei = { sizeof(dbei) };
	if (( dbei.cbBlob = db_event_getBlobSize(hdbe)) == -1 )
		return 1;

	dbei.pBlob = ( PBYTE )_alloca( dbei.cbBlob );
	if (db_event_get(hdbe, &dbei))
		return 1;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return 1;

	if ( strcmp(dbei.szModule, m_szModuleName))
		return 1;

	/* Need to remove the buddy from our Miranda Lists */
	MCONTACT hContact = DbGetAuthEventContact(&dbei);
	if (hContact != NULL) {
		ptrA who( getStringA(hContact, YAHOO_LOGINID));
		if (who) {
			ptrA myid( getStringA(hContact, "MyIdentity"));
			debugLogA("Accepting buddy:%s", who);
			accept(myid, who, getWord(hContact, "yprotoid", 0));
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// AuthDeny - handles the unsuccessful authorization

int CYahooProto::AuthDeny( HANDLE hdbe, const TCHAR* reason )
{
	debugLogA("[YahooAuthDeny]");
	if ( !m_bLoggedIn )
		return 1;

	DBEVENTINFO dbei = { sizeof( dbei ) };
	if (( dbei.cbBlob = db_event_getBlobSize(hdbe)) == -1 ) {
		debugLogA("[YahooAuthDeny] ERROR: Can't get blob size");
		return 1;
	}

	dbei.pBlob = ( PBYTE )alloca( dbei.cbBlob );
	if (db_event_get(hdbe, &dbei)) {
		debugLogA("YahooAuthDeny - Can't get db event!");
		return 1;
	}

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) {
		debugLogA("YahooAuthDeny - not Authorization event");
		return 1;
	}

	if (strcmp( dbei.szModule, m_szModuleName)) {
		debugLogA("YahooAuthDeny - wrong module?");
		return 1;
	}

	/* Need to remove the buddy from our Miranda Lists */
	MCONTACT hContact = DbGetAuthEventContact(&dbei);
	if (hContact != NULL) {
		ptrA who( getStringA(hContact, YAHOO_LOGINID));
		if (who) {
			ptrA myid( getStringA(hContact, "MyIdentity"));
			ptrA u_reason( mir_utf8encodeT(reason));

			debugLogA("Rejecting buddy:%s msg: %s", who, u_reason);
			reject(myid, who, getWord(hContact, "yprotoid", 0), u_reason);
			CallService(MS_DB_CONTACT_DELETE, hContact, 0);
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSR_AUTH

int __cdecl CYahooProto::AuthRecv(MCONTACT hContact, PROTORECVEVENT* pre)
{
	debugLogA("[YahooRecvAuth] ");
	db_unset(hContact,"CList","Hidden");

	Proto_AuthRecv(m_szModuleName, pre);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSS_AUTHREQUEST

int __cdecl CYahooProto::AuthRequest(MCONTACT hContact, const TCHAR* msg )
{
	debugLogA("[YahooSendAuthRequest]");

	if (hContact && m_bLoggedIn) {
		AddBuddy(hContact, "miranda", msg);
		return 0; // Success
	}

	return 1; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// GetCaps - return protocol capabilities bits

DWORD_PTR __cdecl CYahooProto::GetCaps(int type, MCONTACT hContact)
{
	switch ( type ) {
	case PFLAGNUM_1:
		return PF1_IM  | PF1_ADDED | PF1_AUTHREQ | PF1_MODEMSGRECV | PF1_MODEMSGSEND |  PF1_BASICSEARCH |
			PF1_EXTSEARCH | PF1_FILESEND  | PF1_FILERECV| PF1_VISLIST | PF1_SERVERCLIST;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_ONTHEPHONE |
			PF2_OUTTOLUNCH | PF2_INVISIBLE | PF2_LIGHTDND /*| PF2_HEAVYDND*/;

	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_ONTHEPHONE |
			PF2_OUTTOLUNCH | PF2_LIGHTDND ;

	case PFLAGNUM_4:
		return PF4_FORCEAUTH | PF4_FORCEADDED | PF4_SUPPORTTYPING | PF4_SUPPORTIDLE
			|PF4_AVATARS | PF4_OFFLINEFILES | PF4_IMSENDUTF | PF4_IMSENDOFFLINE /* for Meta plugin*/;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR) Translate("ID");
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR) YAHOO_LOGINID;
	case PFLAG_MAXLENOFMESSAGE:
		return 800; /* STUPID YAHOO!!! */
	default:
		return 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// GetInfo - retrieves a contact info

void __cdecl CYahooProto::get_info_thread(void *hContact)
{
	SleepEx(500, TRUE);
	ProtoBroadcastAck((MCONTACT)hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}

int __cdecl CYahooProto::GetInfo(MCONTACT hContact, int /*infoType*/ )
{
	ForkThread(&CYahooProto::get_info_thread, (void*)hContact);
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

int __cdecl CYahooProto::RecvContacts(MCONTACT/*hContact*/, PROTORECVEVENT*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvFile

int __cdecl CYahooProto::RecvFile(MCONTACT hContact, PROTORECVFILET* evt)
{
	db_unset(hContact, "CList", "Hidden");

	return Proto_RecvFile(hContact, evt);
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvUrl

int __cdecl CYahooProto::RecvUrl(MCONTACT/*hContact*/, PROTORECVEVENT*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendContacts

int __cdecl CYahooProto::SendContacts(MCONTACT/*hContact*/, int /*flags*/, int /*nContacts*/, MCONTACT* /*hContactsList*/)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendUrl

int __cdecl CYahooProto::SendUrl(MCONTACT/*hContact*/, int /*flags*/, const char* /*url*/)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SetApparentMode - sets the visibility status

int __cdecl CYahooProto::SetApparentMode(MCONTACT hContact, int mode)
{
	if (mode && mode != ID_STATUS_OFFLINE)
		return 1;

	int oldMode = getWord(hContact, "ApparentMode", 0);
	if (mode != oldMode)
		setWord(hContact, "ApparentMode", mode);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SetStatus - sets the protocol status

int __cdecl CYahooProto::SetStatus(int iNewStatus)
{
	LOG(("[SetStatus] New status %S", pcli->pfnGetStatusModeDescription(iNewStatus, 0)));

	if (iNewStatus == ID_STATUS_OFFLINE) {

		logout();

	}
	else if (!m_bLoggedIn) {
		DBVARIANT dbv;
		int err = 0;
		char errmsg[80];

		if (m_iStatus == ID_STATUS_CONNECTING)
			return 0;

		YAHOO_utils_logversion();

		/*
		* Load Yahoo ID from the database.
		*/
		if (!getString(YAHOO_LOGINID, &dbv)) {
			if (mir_strlen(dbv.pszVal) > 0)
				mir_strncpy(m_yahoo_id, dbv.pszVal, 255);
			else
				err++;
			db_free(&dbv);
		}
		else {
			ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
			err++;
		}

		if (err)
			mir_strncpy(errmsg, Translate("Please enter your Yahoo ID in Options/Network/Yahoo"), 80);
		else {
			if (!getString(YAHOO_PASSWORD, &dbv)) {
				if (mir_strlen(dbv.pszVal) > 0)
					mir_strncpy(m_password, dbv.pszVal, 255);
				else
					err++;

				db_free(&dbv);
			}
			else  {
				ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);
				err++;
			}

			if (err)
				mir_strncpy(errmsg, Translate("Please enter your Yahoo password in Options/Network/Yahoo"), 80);
		}

		if (err != 0) {
			BroadcastStatus(ID_STATUS_OFFLINE);

			ShowError(TranslateT("Yahoo Login Error"), _A2T(errmsg));
			return 0;
		}

		if (iNewStatus == ID_STATUS_OFFLINE)
			iNewStatus = ID_STATUS_ONLINE;

		FREE(m_pw_token); // No Token yet.

		if (!getString(YAHOO_PWTOKEN, &dbv)) {
			if (mir_strlen(dbv.pszVal) > 0)
				m_pw_token = strdup(dbv.pszVal);

			db_free(&dbv);
		}

		m_startStatus = iNewStatus;

		//reset the unread email count. We'll get a new packet since we are connecting.
		m_unreadMessages = 0;

		BroadcastStatus(ID_STATUS_CONNECTING);

		iNewStatus = (iNewStatus == ID_STATUS_INVISIBLE) ? YAHOO_STATUS_INVISIBLE : YAHOO_STATUS_AVAILABLE;
		ForkThread(&CYahooProto::server_main, (void *)iNewStatus);
	}
	else if (iNewStatus == ID_STATUS_INVISIBLE) { /* other normal away statuses are set via setaway */
		BroadcastStatus(iNewStatus);
		set_status(m_iStatus, NULL, (m_iStatus != ID_STATUS_ONLINE) ? 1 : 0);
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

void __cdecl CYahooProto::get_status_thread(void *param)
{
	int l;
	DBVARIANT dbv;
	char *gm = NULL, *sm = NULL, *fm;
	MCONTACT hContact = (MCONTACT)param;

	Sleep(150);

	/* Check Yahoo Games Message */
	if (!getString(hContact, "YGMsg", &dbv)) {
		gm = strdup(dbv.pszVal);
		db_free(&dbv);
	}

	if (!db_get_s(hContact, "CList", "StatusMsg", &dbv)) {
		if (mir_strlen(dbv.pszVal) >= 1)
			sm = strdup(dbv.pszVal);

		db_free(&dbv);
	}
	else {
		int status = getWord(hContact, "YStatus", (WORD)YAHOO_STATUS_OFFLINE);
		sm = yahoo_status_code(yahoo_status(status));
		if (sm) sm = strdup(sm); /* we need this to go global FREE later */
	}

	l = 0;
	if (gm)
		l += mir_strlen(gm) + 3;

	l += mir_strlen(sm) + 1;
	fm = (char *)malloc(l);

	fm[0] = '\0';
	if (gm && mir_strlen(gm) > 0) {
		/* BAH YAHOO SUCKS! WHAT A PAIN!
		find first carriage return add status message then add the rest */
		char *c = strchr(gm, '\r');

		if (c != NULL) {
			mir_strncpy(fm, gm, c - gm + 1);
			fm[c - gm + 1] = '\0';
		}
		else
			mir_strcpy(fm, gm);

		if (sm) {
			mir_strcat(fm, ": ");
			mir_strcat(fm, sm);
		}

		if (c != NULL)
			mir_strcat(fm, c);
	}
	else if (sm) {
		mir_strcat(fm, sm);
	}

	FREE(sm);

	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)(TCHAR*)_A2T(fm));
}

HANDLE __cdecl CYahooProto::GetAwayMsg(MCONTACT hContact)
{
	debugLogA("[YahooGetAwayMessage] ");

	if (hContact && m_bLoggedIn) {
		if (getWord(hContact, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
			return 0; /* user offline, what Status message? */

		ForkThread(&CYahooProto::get_status_thread, (void*)hContact);
		return (HANDLE)1; //Success
	}

	return 0; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PSR_AWAYMSG

int __cdecl CYahooProto::RecvAwayMsg(MCONTACT/*hContact*/, int /*statusMode*/, PROTORECVEVENT*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SetAwayMsg - sets the away status message

int __cdecl CYahooProto::SetAwayMsg(int status, const PROTOCHAR* msg)
{
	char *c = msg && msg[0] ? mir_utf8encodeT(msg) : NULL;

	debugLogA("[YahooSetAwayMessage] Status: %S, Msg: %s", pcli->pfnGetStatusModeDescription(status, 0), (char*)c);

	if (!m_bLoggedIn) {
		if (m_iStatus == ID_STATUS_OFFLINE) {
			debugLogA("[YahooSetAwayMessage] WARNING: WE ARE OFFLINE!");
			mir_free(c);
			return 1;
		}
		else {
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
		if (status == ID_STATUS_ONLINE) {
			set_status(YAHOO_CUSTOM_STATUS, c, 0);
		}
		else if (status != ID_STATUS_INVISIBLE) {
			set_status(YAHOO_CUSTOM_STATUS, c, 1);
		}
	}
	else {
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
	if (!m_bLoggedIn || !m_startMsg)
		return 0;

	if (lParam & SGMA_UNICODE)  {
		return (INT_PTR)mir_utf8decodeW(m_startMsg);
	}
	else {
		return (INT_PTR)mir_utf8decodeA(m_startMsg);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// UserIsTyping - sends a UTN notification

int __cdecl CYahooProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (!m_bLoggedIn)
		return 0;

	char *szProto = GetContactProto(hContact);
	if (szProto == NULL || strcmp(szProto, m_szModuleName))
		return 0;

	DBVARIANT dbv;
	if (!getString(hContact, YAHOO_LOGINID, &dbv)) {
		if (type == PROTOTYPE_SELFTYPING_OFF || type == PROTOTYPE_SELFTYPING_ON) {
			sendtyping(dbv.pszVal, getWord(hContact, "yprotoid", 0), type == PROTOTYPE_SELFTYPING_ON ? 1 : 0);
		}
		db_free(&dbv);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnEvent - maintain protocol events

int __cdecl CYahooProto::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	switch (eventType) {
	case EV_PROTO_ONLOAD:    return OnModulesLoadedEx(0, 0);
		//case EV_PROTO_ONEXIT:    return OnPreShutdown( 0, 0);
	case EV_PROTO_ONOPTIONS: return OnOptionsInit(wParam, lParam);

	case EV_PROTO_ONMENU:
		MenuMainInit();
		break;

	case EV_PROTO_ONRENAME:
		if (mainMenuRoot) {
			CLISTMENUITEM mi = { sizeof(mi) };
			mi.flags = CMIM_NAME | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
			mi.ptszName = m_tszUserName;
			Menu_ModifyItem(mainMenuRoot, &mi);
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
	DBVARIANT dbv;
	CYahooProto* ppro = (CYahooProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		ppro = (CYahooProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		if (!ppro->getString(YAHOO_LOGINID, &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_HANDLE, dbv.pszVal);
			db_free(&dbv);
		}

		if (!ppro->getString(YAHOO_PASSWORD, &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_PASSWORD, dbv.pszVal);
			db_free(&dbv);
		}

		SetButtonCheck(hwndDlg, IDC_YAHOO_JAPAN, ppro->getByte("YahooJapan", 0));
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_NEWYAHOOACCOUNTLINK) {
			CallService(MS_UTILS_OPENURL,
				1,
				((BYTE)IsDlgButtonChecked(hwndDlg, IDC_YAHOO_JAPAN) == 1) ?
				(LPARAM) "http://edit.yahoo.co.jp/config/eval_register" :
				(LPARAM) "http://edit.yahoo.com/config/eval_register"
				);
			return TRUE;
		}

		if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus())
		{
			switch (LOWORD(wParam)) {
			case IDC_HANDLE:
			case IDC_PASSWORD:
			case IDC_YAHOO_JAPAN:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (UINT)PSN_APPLY) {
			char str[128];
			bool reconnectRequired = false;

			GetDlgItemTextA(hwndDlg, IDC_HANDLE, str, SIZEOF(str));
			
			if (ppro->getString(YAHOO_LOGINID, &dbv)) {
				reconnectRequired = true;
			}
			else {
				if(mir_strcmp(str, dbv.pszVal))
					reconnectRequired = true;
				db_free(&dbv);
			}

			ppro->setString(YAHOO_LOGINID, str);
			GetDlgItemTextA(hwndDlg, IDC_PASSWORD, str, SIZEOF(str));

			if (ppro->getString(YAHOO_PASSWORD, &dbv)) {
				reconnectRequired = true;
			}
			else {
				if(mir_strcmp(str, dbv.pszVal))
					reconnectRequired = true;
				db_free(&dbv);
			}

			ppro->setString(YAHOO_PASSWORD, str);
			ppro->setByte("YahooJapan", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_YAHOO_JAPAN));

			if (reconnectRequired) {
				ppro->delSetting(YAHOO_PWTOKEN);
				if (ppro->m_bLoggedIn)
					MessageBox(hwndDlg, TranslateT("The changes you have made require you to reconnect to the Yahoo network before they take effect"), TranslateT("YAHOO Options"), MB_OK);
			}
			return TRUE;
		}
		break;
	}

	return FALSE;
}

INT_PTR CYahooProto::SvcCreateAccMgrUI(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_YAHOOACCOUNT),
		(HWND)lParam, first_run_dialog, (LPARAM)this);
}

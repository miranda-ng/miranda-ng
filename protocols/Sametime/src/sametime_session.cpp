#include "StdAfx.h"
#include "sametime.h"

/// not in CSametimeProto (used at NETLIBOPENCONNECTION_tag.waitcallback)
bool continue_connect;

#define MS_SAMETIME_MENUANNOUNCESESSION		"/SessionAnnounce"

// utf8 encoded
struct {
	char* szOnline;
	char* szAway;
	char* szDND;
	char* szOccupied;	// away msg for 'in meeting' status
} AwayMessages;

void __cdecl SessionClear(mwSession* session)
{
	CSametimeProto* proto = (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
	proto->debugLog(_T("SessionClear()"));
}

int __cdecl SessionWrite(mwSession* session, const unsigned char* buf, gsize len)
{
	CSametimeProto* proto = (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
	proto->debugLog(_T("SessionWrite()  server_connection=[%d], len=[%d]"), proto->server_connection, len);
	if (!proto->server_connection) return 1;
	if (Netlib_Send(proto->server_connection, (const char*)buf, len, 0) == SOCKET_ERROR)
		return 1;
	return 0;
}

void __cdecl SessionClose(mwSession* session)
{
	CSametimeProto* proto = (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
	proto->debugLog(_T("SessionClose()  server_connection=[%d]"), proto->server_connection);
	Netlib_CloseHandle(proto->server_connection);
	proto->server_connection = 0;
}

void CSametimeProto::SessionStarted()
{
	UserListCreate();
	if (options.get_server_contacts)
		UserListAddStored();
}

void CSametimeProto::SessionStopping()
{
	UserListDestroy();
}

void CSametimeProto::InitMeanwhileServices()
{
	debugLog(_T("InitMeanwhileServices() start"));

	if (options.encrypt_session) {
		mwSession_addCipher(session, mwCipher_new_RC2_128(session));
		mwSession_addCipher(session, mwCipher_new_RC2_40(session));
	}

	InitUserList();
	InitMessaging();
	InitFiles();
	InitConference();

	mwSession_setProperty(session, "PROTO_STRUCT_PTR", this, NULL);

	mwSession_setProperty(session, mwSession_AUTH_USER_ID, options.id, NULL);
	mwSession_setProperty(session, mwSession_AUTH_PASSWORD, options.pword, NULL);
	mwSession_setProperty(session, mwSession_CLIENT_TYPE_ID, (void*)options.client_id, NULL);

	mwSession_setProperty(session, mwSession_CLIENT_VER_MAJOR, (void*)options.client_versionMajor, NULL);
	mwSession_setProperty(session, mwSession_CLIENT_VER_MINOR, (void*)options.client_versionMinor, NULL);

}

void CSametimeProto::DeinitMeanwhileServices()
{
	debugLog(_T("DeinitMeanwhileServices() start"));
	DeinitConference();
	DeinitFiles();
	DeinitMessaging();
	DeinitUserList();
	mwCipher_free(mwSession_getCipher(session, mwCipher_RC2_40));
	mwCipher_free(mwSession_getCipher(session, mwCipher_RC2_128));
}

void __cdecl SessionStateChange(mwSession* session, mwSessionState state, gpointer info)
{
	CSametimeProto* proto = (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
	proto->debugLog(_T("SessionStateChange()  state=[%d]"), state);

	switch (state) {
	case mwSession_STARTING:
		break;

	case mwSession_HANDSHAKE:
		break;

	case mwSession_HANDSHAKE_ACK:
		break;

	case mwSession_STARTED:
		proto->SessionStarted();
		break;

	case mwSession_STOPPING:
		if ((int)info) {// & ERR_FAILURE) {
			proto->showPopup((int)info);
		}
		proto->SessionStopping();
		break;

	case mwSession_STOPPED:
		break;

	case mwSession_LOGIN_REDIR:
		proto->debugLog(_T("SessionStateChange()  mwSession_LOGIN_REDIR  info=[%s]"), _A2T((char*)info));
		//options.server_name = str((char*)info);
		strcpy(proto->options.server_name, (char*)info);
		proto->LogOut();
		proto->LogIn(proto->login_status, proto->m_hNetlibUser);
		break;

	case mwSession_LOGIN_CONT:
		break;

	case mwSession_LOGIN:
		break;

	case mwSession_LOGIN_ACK:
		break;

	case mwSession_UNKNOWN:
		break;
	}
}

void __cdecl SessionAdmin(struct mwSession* session, const char* text)
{
	CSametimeProto* proto = (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
	proto->debugLog(_T("SessionAdmin()"));
	TCHAR* tt = mir_utf8decodeT(text);
	MessageBox(0, tt, TranslateT("Sametime administrator message"), MB_OK);
	mir_free(tt);
}

void __cdecl SessionAnnounce(struct mwSession* session, struct mwLoginInfo* from, gboolean may_reply, const char* text)
{
	CSametimeProto* proto = (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
	proto->debugLog(_T("SessionAnnounce()"));
	TCHAR* stzFrom;
	TCHAR* stzText;
	TCHAR stzFromBuff[256];
	stzFrom = mir_utf8decodeT(from->user_name);
	stzText = mir_utf8decodeT(text);
	mir_sntprintf(stzFromBuff, SIZEOF(stzFromBuff), TranslateT("Session announcement - from '%s'"), stzFrom);
	MessageBox(0, TranslateTS(stzText), stzFromBuff, MB_OK);
	mir_free(stzText);
	mir_free(stzFrom);
}

void __cdecl SessionSetPrivacyInfo(struct mwSession* session)
{
	CSametimeProto* proto = (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
	proto->debugLog(_T("SessionSetPrivacyInfo()"));
}

void __cdecl SessionSetUserStatus(struct mwSession* session)
{
	CSametimeProto* proto = (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");

	int new_status;
	struct mwUserStatus us;
	mwUserStatus_clone(&us, mwSession_getUserStatus(session));

	proto->debugLog(_T("SessionSetUserStatus()  us.status=[%d]"), us.status);

	switch (us.status) {
	case mwStatus_ACTIVE:
		new_status = ID_STATUS_ONLINE;
		break;

	case mwStatus_AWAY:
		new_status = ID_STATUS_AWAY;
		if (proto->idle_status) {
			// ignore setting to away by idle module, after we've set ourselves idle
			// most standard clients represent idle and away the same way anyway,
			// but this allows miranda users to make use of the idle timestamp
			// but show our status in clist as away
			proto->BroadcastNewStatus(new_status);
			mwUserStatus_clear(&us);
			return;
		}
		break;

	case mwStatus_BUSY:
		new_status = ID_STATUS_DND;
		break;

	case mwStatus_IDLE:
		new_status = ID_STATUS_AWAY;
		if (!proto->first_online && !proto->options.idle_as_away) { // show our status in clist as away if idle when going online or treating idle as away
			mwUserStatus_clear(&us);
			return;
		}
		break;

	case mwStatus_IN_MEETING: // new 'in meeting' status
		new_status = ID_STATUS_OCCUPIED;
		break;

	default:
		TCHAR buff[512];
		mir_sntprintf(buff, SIZEOF(buff), TranslateT("Unknown user status: %d"), us.status);
		proto->showPopup(buff, SAMETIME_POPUP_ERROR);
		proto->debugLog(buff);

		mwUserStatus_clear(&us);
		// just go online...to prevent us getting stuck 'connecting'
		new_status = ID_STATUS_ONLINE;
		break;
	}

	proto->m_iDesiredStatus = new_status;

	if (proto->first_online) {
		proto->first_online = false;
		//proto->showPopup(TranslateT("Setting login status"), SAMETIME_POPUP_INFO);
		proto->debugLog(_T("Setting login status"));
		proto->SetSessionStatus(proto->login_status);
	}
	else proto->BroadcastNewStatus(new_status);

	mwUserStatus_clear(&us);
}

void CSametimeProto::UpdateSelfStatus()
{
	mir_cslock lck(session_cs);
	if (session) SessionSetUserStatus(session);
}

int CSametimeProto::SetSessionStatus(int status)
{
	struct mwUserStatus us;
	debugLog(_T("SetSessionStatus() start  status=[%d]"), status);

	if (idle_timerid) KillTimer(0, idle_timerid);

	us.time = (DWORD)time(0);
	//us.time = 0;

	switch (status) {
	case ID_STATUS_FREECHAT:
	case ID_STATUS_ONLINE:
		us.desc = AwayMessages.szOnline; us.status = mwStatus_ACTIVE;
		break;
	case ID_STATUS_NA:
	case ID_STATUS_INVISIBLE:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
	case ID_STATUS_AWAY:
		us.desc = AwayMessages.szAway; us.status = mwStatus_AWAY;
		break;
	case ID_STATUS_OCCUPIED:	// link 'Occupied' MIR_NG status with 'in meeting' Sametime status
		us.desc = AwayMessages.szOccupied; us.status = mwStatus_IN_MEETING;
		break;
	case ID_STATUS_DND:
		us.desc = AwayMessages.szDND; us.status = mwStatus_BUSY;
		break;
	default:
		// act as online for unsupported status
		us.desc = AwayMessages.szOnline; us.status = mwStatus_ACTIVE; break;
	}

	debugLog(_T("SetSessionStatus() mwSession_setUserStatus  us.status=[%d], us.desc:len=[%d]"), us.status, us.desc == NULL ? -1 : mir_strlen(us.desc));
	mwSession_setUserStatus(session, &us);

	return 0;
}

VOID CALLBACK IdleTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	CSametimeProto* proto = (CSametimeProto*)idEvent;

	KillTimer(0, proto->idle_timerid);
	proto->idle_timerid = 0;

	if (proto->idle_status) {
		struct mwUserStatus us;
		us.time = (DWORD)time(0);
		us.status = mwStatus_IDLE;
		us.desc = 0;
		mwSession_setUserStatus(proto->session, &us);
	}
	else proto->SetSessionStatus(proto->m_iStatus);
}

int CSametimeProto::SetIdle(bool idle)
{
	// set a timer, to wait for any autoaway module which might set our status
	debugLog(_T("CSametimeProto::SetIdle()  idle=[%d], idle_status=[%d], idle_timerid=[%d]"), idle, idle_status, idle_timerid);
	if (idle && !idle_status) {
		idle_status = true;
		if (!idle_timerid)
			idle_timerid = SetTimer(0, (UINT_PTR)this, 200, IdleTimerProc);
	}
	else if (idle_status) {
		idle_status = false;
		if (!idle_timerid)
			idle_timerid = SetTimer(0, (UINT_PTR)this, 200, IdleTimerProc);
	}
	return 0;
}

void CSametimeProto::SetSessionAwayMessage(int status, const PROTOCHAR* msgT)
{
	debugLog(_T("SetSessionAwayMessage() status=[%d], msgT:len=[%d]"), status, msgT == NULL ? -1 : mir_tstrlen(msgT));

	T2Utf msg(msgT);
	if (status == ID_STATUS_ONLINE)
		replaceStr(AwayMessages.szOnline, msg);
	else if (status == ID_STATUS_AWAY)
		replaceStr(AwayMessages.szAway, msg);
	else if (status == ID_STATUS_DND)
		replaceStr(AwayMessages.szDND, msg);
	else if (status == ID_STATUS_OCCUPIED)
		replaceStr(AwayMessages.szOccupied, msg);	// manage Occupied as away message
	else
		return; // unsupported status

	if (session)
		SetSessionStatus(status); // update current away message
}

static VOID CALLBACK NullAPC(DWORD_PTR)
{
	// This function intentionally left blank
}

void WakeThread(HANDLE hThread)
{
	QueueUserAPC(NullAPC, hThread, 0);
}

void __cdecl KeepAliveThread(LPVOID param)
{
	CSametimeProto* proto = (CSametimeProto*)param;
	int i = 120;
	proto->debugLog(_T("KeepAliveThread() start"));

	while (1) {

		if (i <= 0) {
			i = 120;
			// send keepalive every 120 * 250 = 30000[ms]
			if (mwSession_isStarted(proto->session) && proto->session) {
				mwSession_sendKeepalive(proto->session);
			}
		}

		i--;

		SleepEx(250, TRUE);

		mir_cslock lck(proto->session_cs);
		if (Miranda_Terminated() || !proto->session) {
			proto->debugLog(_T("KeepAliveThread() end"));
			break;
		}
	}

	return;
}

int waitcallback(unsigned int* timeout)
{
	return continue_connect ? 1 : 0;
}

void __cdecl SessionThread(LPVOID param)
{
	CSametimeProto* proto = (CSametimeProto*)param;
	HANDLE hNetlibUser = proto->m_hNetlibUser;
	proto->debugLog(_T("SessionThread() start"));

	continue_connect = true;

	//setup
	NETLIBOPENCONNECTION conn_data = { 0 };
	conn_data.cbSize = sizeof(NETLIBOPENCONNECTION);
	conn_data.flags = NLOCF_V2;
	conn_data.szHost = proto->options.server_name;
	conn_data.wPort = proto->options.port;
	conn_data.timeout = 20;
	conn_data.waitcallback = waitcallback;

	proto->BroadcastNewStatus(ID_STATUS_CONNECTING);

	proto->server_connection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hNetlibUser, (LPARAM)&conn_data);

	if (!proto->server_connection) {

		proto->BroadcastNewStatus(ID_STATUS_OFFLINE);

		if (continue_connect) {
			// real timeout - not user cancelled
			proto->showPopup(TranslateT("No server connection!"), SAMETIME_POPUP_ERROR);
		}

		proto->debugLog(_T("SessionThread() end, no server_connection, continue_connect=[%d]"), continue_connect);
		return;
	}

	mwSessionHandler handler = { 0 };
	handler.clear = SessionClear;
	handler.io_write = SessionWrite;
	handler.io_close = SessionClose;
	handler.on_stateChange = SessionStateChange;
	handler.on_admin = SessionAdmin;
	handler.on_announce = SessionAnnounce;
	handler.on_setPrivacyInfo = SessionSetPrivacyInfo;
	handler.on_setUserStatus = SessionSetUserStatus;

	{
		mir_cslock lck(proto->session_cs);
		proto->session = mwSession_new(&handler);

		proto->InitMeanwhileServices();

		mwSession_start(proto->session);
	}

	mir_forkthread(KeepAliveThread, (void*)proto);

	unsigned char* recv_buffer = (unsigned char*)mir_alloc(1024 * 32);
	int bytes;
	//while(session && server_connection && mwSession_getState(session) != mwSession_STOPPED) {
	while (proto->server_connection) {// && session) {// && !mwSession_isStopped(session)) { // break on error
		bytes = Netlib_Recv(proto->server_connection, (char *)recv_buffer, 1024 * 32, 0);
		proto->debugLog(_T("SessionThread() Netlib_Recv'ed bytes=[%d]"), bytes);

		if (bytes == 0) {
			break;
		}
		else if (bytes == SOCKET_ERROR) {
			// this is normal - e.g. socket closed due to log off, during blocking read above
			break;
		}
		else {
			mir_cslock lck(proto->session_cs);
			mwSession_recv(proto->session, recv_buffer, bytes);
		}
	}
	mir_free(recv_buffer);

	mir_cslock lck2(proto->session_cs);
	proto->DeinitMeanwhileServices();
	mwSession* old_session = proto->session;
	proto->session = 0; // kills keepalive thread, if awake
	mwSession_free(old_session);

	proto->BroadcastNewStatus(ID_STATUS_OFFLINE);
	proto->SetAllOffline();
	proto->first_online = true;

	proto->debugLog(_T("SessionThread() end"));
	return;
}

WORD CSametimeProto::GetClientVersion()
{
	if (!session) return 0;

	WORD retval = 0;
	retval = (int)mwSession_getProperty(session, mwSession_CLIENT_VER_MAJOR) << 8;
	retval |= (int)mwSession_getProperty(session, mwSession_CLIENT_VER_MINOR);
	return retval;
}

WORD CSametimeProto::GetServerVersion()
{
	if (!session) return 0;

	WORD retval = 0;
	retval = (int)mwSession_getProperty(session, mwSession_SERVER_VER_MAJOR) << 8;
	retval |= (int)mwSession_getProperty(session, mwSession_SERVER_VER_MINOR);
	return retval;
}

int CSametimeProto::LogIn(int ls, HANDLE hNetlibUser)
{
	debugLog(_T("LogIn() start"));

	mir_cslock lck(session_cs);
	if (session) {
		debugLog(_T("LogIn() end, currently in session"));
		return 0;
	}

	login_status = ls;

	mir_forkthread(SessionThread, (void*)this);

	return 0;
}

int CSametimeProto::LogOut()
{
	debugLog(_T("LogOut() start"));
	continue_connect = false;

	mir_cslock lck(session_cs);
	if (session && server_connection && m_iStatus != ID_STATUS_OFFLINE && !mwSession_isStopped(session) && !mwSession_isStopping(session)) {
		debugLog(_T("LogOut() mwSession_stop"));
		mwSession_stop(session, 0);
	}

	return 0;
}

void CSametimeProto::InitAwayMsg()
{
	AwayMessages.szOnline = 0;
	AwayMessages.szAway = 0;
	AwayMessages.szDND = 0;
	AwayMessages.szOccupied = 0;
}

void CSametimeProto::DeinitAwayMsg()
{
	mir_free(AwayMessages.szOnline);
	mir_free(AwayMessages.szAway);
	mir_free(AwayMessages.szDND);
	mir_free(AwayMessages.szOccupied);
}

void SendAnnouncement(SendAnnouncementFunc_arg* arg)
{
	CSametimeProto* proto = arg->proto;
	if (proto->session && arg->recipients)
		mwSession_sendAnnounce(proto->session, false, T2Utf(arg->msg), arg->recipients);
}

INT_PTR CSametimeProto::SessionAnnounce(WPARAM wParam, LPARAM lParam)
{
	debugLog(_T("CSametimeProto::SessionAnnounce() start"));
	SessionAnnounceDialogProc_arg* sadpArg = (SessionAnnounceDialogProc_arg*)mir_calloc(sizeof(SessionAnnounceDialogProc_arg));
	sadpArg->proto = this;
	sadpArg->sendAnnouncementFunc = SendAnnouncement;
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SESSIONANNOUNCE), GetDesktopWindow(), SessionAnnounceDialogProc, (LPARAM)sadpArg);
	return 0;
}

void CSametimeProto::InitSessionMenu()
{
	debugLog(_T("CSametimeProto::InitSessionMenu()"));

	CreateProtoService(MS_SAMETIME_MENUANNOUNCESESSION, &CSametimeProto::SessionAnnounce);

	char service[128];

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_TCHAR;
	mi.popupPosition = 500085001;
	mi.position = 2000060000;
	mi.ptszName = LPGENT("Send announcement...");
	mir_snprintf(service, SIZEOF(service), "%s%s", m_szModuleName, MS_SAMETIME_MENUANNOUNCESESSION);
	mi.pszService = service;
	mi.icolibItem = GetIconHandle(IDI_ICON_ANNOUNCE);
	mi.pszContactOwner = m_szModuleName;
	hSessionAnnounceMenuItem = Menu_AddContactMenuItem(&mi);
}

void CSametimeProto::DeinitSessionMenu()
{
	debugLog(_T("CSametimeProto::DeinitSessionMenu()"));
	CallService(MO_REMOVEMENUITEM, (WPARAM)hSessionAnnounceMenuItem, 0);
}

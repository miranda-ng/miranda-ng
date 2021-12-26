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
	proto->debugLogW(L"SessionClear()");
}

int __cdecl SessionWrite(mwSession* session, const unsigned char* buf, gsize len)
{
	CSametimeProto* proto = (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
	proto->debugLogW(L"SessionWrite()  server_connection=[%d], len=[%d]", proto->server_connection, len);
	if (!proto->server_connection) return 1;
	if (Netlib_Send(proto->server_connection, (const char*)buf, len, 0) == SOCKET_ERROR)
		return 1;
	return 0;
}

void __cdecl SessionClose(mwSession* session)
{
	CSametimeProto* proto = (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
	proto->debugLogW(L"SessionClose()  server_connection=[%d]", proto->server_connection);
	Netlib_CloseHandle(proto->server_connection);
	proto->server_connection = nullptr;
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
	debugLogW(L"InitMeanwhileServices() start");

	if (options.encrypt_session) {
		mwSession_addCipher(session, mwCipher_new_RC2_128(session));
		mwSession_addCipher(session, mwCipher_new_RC2_40(session));
	}

	InitUserList();
	InitMessaging();
	InitFiles();
	InitConference();

	mwSession_setProperty(session, "PROTO_STRUCT_PTR", this, nullptr);

	mwSession_setProperty(session, mwSession_AUTH_USER_ID, options.id, nullptr);
	mwSession_setProperty(session, mwSession_AUTH_PASSWORD, options.pword, nullptr);
	mwSession_setProperty(session, mwSession_CLIENT_TYPE_ID, (void*)options.client_id, nullptr);

	mwSession_setProperty(session, mwSession_CLIENT_VER_MAJOR, (void*)options.client_versionMajor, nullptr);
	mwSession_setProperty(session, mwSession_CLIENT_VER_MINOR, (void*)options.client_versionMinor, nullptr);

}

void CSametimeProto::DeinitMeanwhileServices()
{
	debugLogW(L"DeinitMeanwhileServices() start");
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
	proto->debugLogW(L"SessionStateChange()  state=[%d]", state);

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
		if ((INT_PTR)info)
			proto->showPopup((INT_PTR)info);
		
		proto->SessionStopping();
		break;

	case mwSession_STOPPED:
		break;

	case mwSession_LOGIN_REDIR:
		proto->OnLogInRedirect((char*)info);
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
	proto->debugLogW(L"SessionAdmin()");
	wchar_t* tt = mir_utf8decodeW(text);
	MessageBox(nullptr, tt, TranslateT("Sametime administrator message"), MB_OK);
	mir_free(tt);
}

void __cdecl SessionAnnounce(struct mwSession* session, struct mwLoginInfo* from, gboolean, const char* text)
{
	CSametimeProto* proto = (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
	proto->debugLogW(L"SessionAnnounce()");
	wchar_t* stzFrom;
	wchar_t* stzText;
	wchar_t stzFromBuff[256];
	stzFrom = mir_utf8decodeW(from->user_name);
	stzText = mir_utf8decodeW(text);
	mir_snwprintf(stzFromBuff, TranslateT("Session announcement - from '%s'"), stzFrom);
	MessageBox(nullptr, TranslateW(stzText), stzFromBuff, MB_OK);
	mir_free(stzText);
	mir_free(stzFrom);
}

void __cdecl SessionSetPrivacyInfo(struct mwSession* session)
{
	CSametimeProto* proto = (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
	proto->debugLogW(L"SessionSetPrivacyInfo()");
}

void __cdecl SessionSetUserStatus(struct mwSession* session)
{
	CSametimeProto* proto = (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");

	int new_status;
	struct mwUserStatus us;
	mwUserStatus_clone(&us, mwSession_getUserStatus(session));

	proto->debugLogW(L"SessionSetUserStatus()  us.status=[%d]", us.status);

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
		wchar_t buff[512];
		mir_snwprintf(buff, TranslateT("Unknown user status: %d"), us.status);
		proto->showPopup(buff, SAMETIME_POPUP_ERROR);
		proto->debugLogW(buff);

		mwUserStatus_clear(&us);
		// just go online...to prevent us getting stuck 'connecting'
		new_status = ID_STATUS_ONLINE;
		break;
	}

	proto->m_iDesiredStatus = new_status;

	if (proto->first_online) {
		proto->first_online = false;
		//proto->showPopup(TranslateT("Setting login status"), SAMETIME_POPUP_INFO);
		proto->debugLogW(L"Setting login status");
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
	debugLogW(L"SetSessionStatus() start  status=[%d]", status);

	if (idle_timerid) KillTimer(nullptr, idle_timerid);

	us.time = (uint32_t)time(0);
	//us.time = 0;

	switch (status) {
	case ID_STATUS_FREECHAT:
	case ID_STATUS_ONLINE:
		us.desc = AwayMessages.szOnline; us.status = mwStatus_ACTIVE;
		break;
	case ID_STATUS_NA:
	case ID_STATUS_INVISIBLE:
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

	debugLogW(L"SetSessionStatus() mwSession_setUserStatus  us.status=[%d], us.desc:len=[%d]", us.status, us.desc == nullptr ? -1 : mir_strlen(us.desc));
	mwSession_setUserStatus(session, &us);

	return 0;
}

VOID CALLBACK IdleTimerProc(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	CSametimeProto* proto = (CSametimeProto*)idEvent;

	KillTimer(nullptr, proto->idle_timerid);
	proto->idle_timerid = 0;

	if (proto->idle_status) {
		struct mwUserStatus us;
		us.time = (uint32_t)time(0);
		us.status = mwStatus_IDLE;
		us.desc = nullptr;
		mwSession_setUserStatus(proto->session, &us);
	}
	else proto->SetSessionStatus(proto->m_iStatus);
}

int CSametimeProto::SetIdle(bool idle)
{
	// set a timer, to wait for any autoaway module which might set our status
	debugLogW(L"CSametimeProto::SetIdle()  idle=[%d], idle_status=[%d], idle_timerid=[%d]", idle, idle_status, idle_timerid);
	if (idle && !idle_status) {
		idle_status = true;
		if (!idle_timerid)
			idle_timerid = SetTimer(nullptr, (UINT_PTR)this, 200, IdleTimerProc);
	}
	else if (idle_status) {
		idle_status = false;
		if (!idle_timerid)
			idle_timerid = SetTimer(nullptr, (UINT_PTR)this, 200, IdleTimerProc);
	}
	return 0;
}

void CSametimeProto::SetSessionAwayMessage(int status, const wchar_t* msgT)
{
	debugLogW(L"SetSessionAwayMessage() status=[%d], msgT:len=[%d]", status, msgT == nullptr ? -1 : mir_wstrlen(msgT));

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

void __cdecl CSametimeProto::KeepAliveThread(void*)
{
	int i = 120;
	debugLogW(L"KeepAliveThread() start");

	while (1) {

		if (i <= 0) {
			i = 120;
			// send keepalive every 120 * 250 = 30000[ms]
			if (mwSession_isStarted(session) && session) {
				mwSession_sendKeepalive(session);
			}
		}

		i--;

		SleepEx(250, TRUE);

		mir_cslock lck(session_cs);
		if (Miranda_IsTerminated() || !session) {
			debugLogW(L"KeepAliveThread() end");
			break;
		}
	}

	return;
}

int waitcallback(unsigned int*)
{
	return continue_connect ? 1 : 0;
}

void __cdecl CSametimeProto::SessionThread(LPVOID)
{
	debugLogW(L"SessionThread() start");

	continue_connect = true;

	BroadcastNewStatus(ID_STATUS_CONNECTING);

	// setup
	NETLIBOPENCONNECTION conn_data = { 0 };
	conn_data.flags = NLOCF_V2;
	conn_data.szHost = options.server_name;
	conn_data.wPort = options.port;
	conn_data.timeout = 20;
	conn_data.waitcallback = waitcallback;
	server_connection = Netlib_OpenConnection(m_hNetlibUser, &conn_data);

	if (!server_connection) {

		BroadcastNewStatus(ID_STATUS_OFFLINE);

		if (continue_connect) {
			// real timeout - not user cancelled
			showPopup(TranslateT("No server connection!"), SAMETIME_POPUP_ERROR);
		}

		debugLogW(L"SessionThread() end, no server_connection, continue_connect=[%d]", continue_connect);
		return;
	}

	mwSessionHandler handler = {};
	handler.clear = ::SessionClear;
	handler.io_write = ::SessionWrite;
	handler.io_close = ::SessionClose;
	handler.on_stateChange = ::SessionStateChange;
	handler.on_admin = ::SessionAdmin;
	handler.on_announce = ::SessionAnnounce;
	handler.on_setPrivacyInfo = ::SessionSetPrivacyInfo;
	handler.on_setUserStatus = ::SessionSetUserStatus;

	{
		mir_cslock lck(session_cs);
		session = mwSession_new(&handler);

		InitMeanwhileServices();

		mwSession_start(session);
	}

	ForkThread(&CSametimeProto::KeepAliveThread);

	unsigned char* recv_buffer = (unsigned char*)mir_alloc(1024 * 32);
	int bytes;
	//while(session && server_connection && mwSession_getState(session) != mwSession_STOPPED) {
	while (server_connection) {// && session) {// && !mwSession_isStopped(session)) { // break on error
		bytes = Netlib_Recv(server_connection, (char *)recv_buffer, 1024 * 32, 0);
		debugLogW(L"SessionThread() Netlib_Recv'ed bytes=[%d]", bytes);

		if (bytes == 0) {
			break;
		}
		else if (bytes == SOCKET_ERROR) {
			// this is normal - e.g. socket closed due to log off, during blocking read above
			break;
		}
		else {
			mir_cslock lck(session_cs);
			mwSession_recv(session, recv_buffer, bytes);
		}
	}
	mir_free(recv_buffer);

	mir_cslock lck2(session_cs);
	DeinitMeanwhileServices();
	mwSession* old_session = session;
	session = nullptr; // kills keepalive thread, if awake
	mwSession_free(old_session);

	BroadcastNewStatus(ID_STATUS_OFFLINE);
	SetAllOffline();
	first_online = true;

	debugLogW(L"SessionThread() end");
	return;
}

uint16_t CSametimeProto::GetClientVersion()
{
	if (!session) return 0;

	uint16_t retval = 0;
	retval = (UINT_PTR)mwSession_getProperty(session, mwSession_CLIENT_VER_MAJOR) << 8;
	retval |= (UINT_PTR)mwSession_getProperty(session, mwSession_CLIENT_VER_MINOR);
	return retval;
}

uint16_t CSametimeProto::GetServerVersion()
{
	if (!session) return 0;

	uint16_t retval = 0;
	retval = (UINT_PTR)mwSession_getProperty(session, mwSession_SERVER_VER_MAJOR) << 8;
	retval |= (UINT_PTR)mwSession_getProperty(session, mwSession_SERVER_VER_MINOR);
	return retval;
}

int CSametimeProto::LogIn(int ls)
{
	debugLogW(L"LogIn() start");

	mir_cslock lck(session_cs);
	if (session) {
		debugLogW(L"LogIn() end, currently in session");
		return 0;
	}

	login_status = ls;

	ForkThread(&CSametimeProto::SessionThread);
	return 0;
}

int CSametimeProto::LogOut()
{
	debugLogW(L"LogOut() start");
	continue_connect = false;

	mir_cslock lck(session_cs);
	if (session && server_connection && m_iStatus != ID_STATUS_OFFLINE && !mwSession_isStopped(session) && !mwSession_isStopping(session)) {
		debugLogW(L"LogOut() mwSession_stop");
		mwSession_stop(session, 0);
	}

	return 0;
}

int CSametimeProto::OnLogInRedirect(char* newHost)
{
	debugLogA("OnLogInRedirect() mwSession_LOGIN_REDIR  newHost=[%s]", newHost ? newHost : "(null)");

	if (!newHost || !mir_strcmp(newHost, options.server_name) || db_get_b(0, m_szModuleName, "ForceLogin", 0) == 1) {
		debugLogW(L"OnLogInRedirect() forceLogin");
		mwSession_forceLogin(session);
		return 0;
	}

	debugLogW(L"OnLogInRedirect() redirect");
	mir_strcpy(options.server_name, newHost);
	LogOut();
	Sleep(50);  //wait for SessionThread end
	LogIn(login_status);
	return 0;

}

void CSametimeProto::InitAwayMsg()
{
	AwayMessages.szOnline = nullptr;
	AwayMessages.szAway = nullptr;
	AwayMessages.szDND = nullptr;
	AwayMessages.szOccupied = nullptr;
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

INT_PTR CSametimeProto::SessionAnnounce(WPARAM, LPARAM)
{
	debugLogW(L"CSametimeProto::SessionAnnounce() start");
	SessionAnnounceDialogProc_arg* sadpArg = (SessionAnnounceDialogProc_arg*)mir_calloc(sizeof(SessionAnnounceDialogProc_arg));
	sadpArg->proto = this;
	sadpArg->sendAnnouncementFunc = SendAnnouncement;
	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SESSIONANNOUNCE), GetDesktopWindow(), SessionAnnounceDialogProc, (LPARAM)sadpArg);
	return 0;
}

void CSametimeProto::InitSessionMenu()
{
	debugLogW(L"CSametimeProto::InitSessionMenu()");

	CreateProtoService(MS_SAMETIME_MENUANNOUNCESESSION, &CSametimeProto::SessionAnnounce);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xc58b08ac, 0x5a44, 0x4f19, 0xbc, 0x9e, 0xe3, 0x5f, 0xf6, 0x9, 0x4d, 0xf);
	mi.flags = CMIF_UNICODE;
	mi.position = 2000060000;
	mi.name.w = LPGENW("Send announcement...");
	mi.pszService = MS_SAMETIME_MENUANNOUNCESESSION;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ICON_ANNOUNCE);
	Menu_AddContactMenuItem(&mi, m_szModuleName);
}

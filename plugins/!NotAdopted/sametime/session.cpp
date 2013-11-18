#include "session.h"
#include <glib.h>

HANDLE sessionThread = 0;
DWORD session_thread_id = 0;

CRITICAL_SECTION session_cs;

HANDLE server_connection = 0;
mwSession *session = 0;

HANDLE hSessionSyncEvent = 0;

int idle_timerid = 0;
bool idle_status = false;
int login_status;
bool first_online = true;	// set our status after the first online status comes from the server


#define MS_SAMETIME_MENUANNOUNCESESSION		"/SessionAnnounce"
HICON hIconProto, hIconAnnounce;
HANDLE hSessionAnnounceMenuItem;

struct {
	char *szOnline;
	char *szAway;
	char *szDND;
} AwayMessages;

void CALLBACK sttMainThreadStatusCallback( ULONG dwParam ) {
	if(current_status != dwParam) {
		previous_status = current_status;
		current_status = dwParam;
		ProtoBroadcastAck(PROTO,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS, (HANDLE)previous_status, current_status);
	}
}

void __cdecl SessionClear(struct mwSession *) {
}

int __cdecl SessionWrite(mwSession *, const unsigned char *buf, gsize len) {
	if(!server_connection) return 1;
	if(Netlib_Send(server_connection, (const char *)buf, len, 0) == SOCKET_ERROR)
		return 1;
	return 0;
}

void __cdecl SessionClose(mwSession *) {
	Netlib_CloseHandle(server_connection);
	server_connection = 0;
}

void SessionStarted() {
	//PUShowMessage("Session started", SM_NOTIFY);
	UserListCreate();
	if(options.get_server_contacts) UserListAddStored();
}

void SessionStopping() {
	UserListDestroy();
}

void InitMeanwhileServices() {
	if(options.encrypt_session) {
		mwSession_addCipher(session, mwCipher_new_RC2_128(session));
		mwSession_addCipher(session, mwCipher_new_RC2_40(session));
	}

	InitUserList(session);
	InitMessaging(session);
	InitFiles(session);
	InitConference(session);
}

void DeinitMeanwhileServices() {
	DeinitConference(session);
	DeinitFiles(session);
	DeinitMessaging(session);
	DeinitUserList(session);

	mwCipher_free(mwSession_getCipher(session, mwCipher_RC2_40));
	mwCipher_free(mwSession_getCipher(session, mwCipher_RC2_128));
}

void __cdecl SessionStateChange(mwSession *s, mwSessionState state, gpointer info) {

	switch(state) {
	case mwSession_STARTING:      
		break;
	case mwSession_HANDSHAKE:  
		break;
	case mwSession_HANDSHAKE_ACK:
		break;
	case mwSession_STARTED: 
		SessionStarted();
		break;
	case mwSession_STOPPING:     
		if((int)info) {// & ERR_FAILURE) {
			char *msg = mwError((int)info);
			//MessageBoxA(0, Translate(msg), Translate("Sametime Error"), MB_OK | MB_ICONWARNING);
			TCHAR *ts = u2t(msg);
			ShowError(TranslateTS(ts));
			g_free(msg);
			free(ts);
		}

		SessionStopping();
		break;
	case mwSession_STOPPED:     
		break;
		
	case mwSession_LOGIN_REDIR:  
		//options.server_name = str((char *)info);
		strcpy(options.server_name, (char *)info);
		LogOut();
		LogIn(login_status, hNetlibUser);
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

void __cdecl SessionAdmin(struct mwSession *, const char *text) {
	TCHAR *tt = u2t(text);
	MessageBox(0, tt, TranslateT("Sametime Administrator Message"), MB_OK);
	free(tt);
}

void __cdecl SessionAnnounce(struct mwSession *, struct mwLoginInfo *from, gboolean may_reply, const char *text) {
	TCHAR buff[256], *tt1, *tt2;
#ifdef _UNICODE
	_snwprintf(buff, 256, TranslateT("Session Announcement - from '%s'"), tt1 = u2t(from->user_name));
#else
	_snprintf(buff, 256, TranslateT("Session Announcement - from '%s'"), tt1 = u2t(from->user_name));
#endif
	MessageBox(0, TranslateTS(tt2 = u2t(text)), buff, MB_OK);
	free(tt1); free(tt2);
}

void __cdecl SessionSetPrivacyInfo(struct mwSession *) {
}

void __cdecl SessionSetUserStatus(struct mwSession *session) {
	int new_status;
	struct mwUserStatus us;
	mwUserStatus_clone(&us, mwSession_getUserStatus(session));

	switch(us.status) {
	case mwStatus_ACTIVE:	
		new_status = ID_STATUS_ONLINE; 
		break;
	case mwStatus_AWAY:		
		new_status = ID_STATUS_AWAY; 
		if(idle_status) {
			// ignore setting to away by idle module, after we've set ourselves idle
			// most standard clients represent idle and away the same way anyway,
			// but this allows miranda users to make use of the idle timestamp

			// but show our status in clist as away
			QueueUserAPC(sttMainThreadStatusCallback, mainThread, new_status);
			
			mwUserStatus_clear(&us);
			return;
		}
		break;
	case mwStatus_BUSY:		
		new_status = ID_STATUS_DND; 
		break;
	case mwStatus_IDLE:		
		new_status = ID_STATUS_AWAY; 
		if(!first_online && !options.idle_as_away) { // show our status in clist as away if idle when going online or treating idle as away
			mwUserStatus_clear(&us);
			return;
		}
		break;
	case 8: // new 'in a meeting' status, not handled by meanwhile lib
		new_status = ID_STATUS_OCCUPIED;
		break;
	default:
		{
			char buff[512];
			sprintf(buff, "Unknown user status: %d", us.status);
			PUShowMessage(buff, SM_WARNING);
		}
		mwUserStatus_clear(&us);
		// just go online...to prevent us getting stuck 'connecting'
		new_status = ID_STATUS_ONLINE;
		break;
	}

	if(first_online) {
		first_online = false;
		//PUShowMessage("Setting login status", SM_NOTIFY);
		SetSessionStatus(login_status);
	} else
		QueueUserAPC(sttMainThreadStatusCallback, mainThread, new_status);

	mwUserStatus_clear(&us);
}

void UpdateSelfStatus() {
	EnterCriticalSection(&session_cs);
	if(session) SessionSetUserStatus(session);
	LeaveCriticalSection(&session_cs);
}

int SetSessionStatus(int status) {
	struct mwUserStatus us;

	if(idle_timerid) KillTimer(0, idle_timerid);

	us.time = (DWORD)time(0);
	//us.time = 0;

	switch(status) {
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
		case ID_STATUS_OCCUPIED:
		case ID_STATUS_DND: 
			us.desc = AwayMessages.szDND; us.status = mwStatus_BUSY; 
			break;
		default:
			// act as online for unsupported status
			us.desc = AwayMessages.szOnline; us.status = mwStatus_ACTIVE; break;
	}

	mwSession_setUserStatus(session, &us);

	return 0;
}

VOID CALLBACK IdleTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	KillTimer(0, idle_timerid);
	idle_timerid = 0;

	if(idle_status) {
		struct mwUserStatus us;
	
		us.time = (DWORD)time(0);
		us.status = mwStatus_IDLE;
		us.desc = 0;
		mwSession_setUserStatus(session, &us);
	} else
		SetSessionStatus(current_status);
}

int SetIdle(bool idle) {

	// set a timer, to wait for any autoaway module which might set our status 
	if(idle && !idle_status) {
		idle_status = true;
		if(!idle_timerid) 
			idle_timerid = SetTimer(0, 0, 200, IdleTimerProc);
	} else if(idle_status) {
		idle_status = false;
		if(!idle_timerid) 
			idle_timerid = SetTimer(0, 0, 200, IdleTimerProc);
	}

	return 0;
}

void SetSessionAwayMessage(int status, char *msg) {
	if(status == ID_STATUS_ONLINE) {
		if(AwayMessages.szOnline) free(AwayMessages.szOnline);
		if(msg) {
			AwayMessages.szOnline = _strdup(msg);
		} else AwayMessages.szOnline = 0;
	} else if(status == ID_STATUS_AWAY) {
		if(AwayMessages.szAway) free(AwayMessages.szAway);
		if(msg) {
			AwayMessages.szAway = _strdup(msg);
		} else AwayMessages.szAway = 0;
	} else if(status == ID_STATUS_DND) {
		if(AwayMessages.szDND) free(AwayMessages.szDND);
		if(msg) {
			AwayMessages.szDND = _strdup(msg);
		} else AwayMessages.szDND = 0;
	} else 
		return; // unsupported status

	SetSessionStatus(status); // update current away message
}

void WINAPI NullAPC (DWORD dwData) {
     // This function intentionally left blank
}

void WakeThread(HANDLE hThread) {
	QueueUserAPC(NullAPC, hThread, 0);
}

unsigned long __stdcall KeepAliveThread(LPVOID param) {
	CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);
	//PUShowMessage("KA Thread start", SM_NOTIFY);

	while(1) {// && !mwSession_isStopped(session)) {
		SleepEx(30000, TRUE);
		
		EnterCriticalSection(&session_cs);
		if(!session) {
			LeaveCriticalSection(&session_cs);
			break;
		}
		if(mwSession_isStarted(session) && !Miranda_Terminated() && session) {
			mwSession_sendKeepalive(session);
			//PUShowMessage("KA", SM_NOTIFY);
		}
		LeaveCriticalSection(&session_cs);
	}

	//PUShowMessage("KA Thread end", SM_NOTIFY);
	CallService(MS_SYSTEM_THREAD_POP, 0, 0);
	return 0;
}

bool continue_connect;

int waitcallback(unsigned int *timeout) {
	return continue_connect ? 1 : 0;
}

unsigned long __stdcall SessionThread(LPVOID param) {
	HANDLE hNetlibUser = (HANDLE)param;

	CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);

	continue_connect = true;

	//setup
	NETLIBOPENCONNECTION conn_data = {0};
	conn_data.cbSize = sizeof(NETLIBOPENCONNECTION);
	conn_data.flags = NLOCF_V2;
	conn_data.szHost = options.server_name;
	conn_data.wPort = options.port;
	conn_data.timeout = 20;
	conn_data.waitcallback = waitcallback;

	QueueUserAPC(sttMainThreadStatusCallback, mainThread, ID_STATUS_CONNECTING);	

	server_connection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hNetlibUser, (LPARAM)&conn_data);

	if(!server_connection) {
		QueueUserAPC(sttMainThreadStatusCallback, mainThread, ID_STATUS_OFFLINE);	
		if(continue_connect) { // real timeout - not user cancelled
			//MessageBox(0, "No server connection!", "Error", MB_OK);
			ShowError(TranslateT("No server connection!"));
		}
		CallService(MS_SYSTEM_THREAD_POP, 0, 0);
		return 1;
	}

	mwSessionHandler handler = {0};
	handler.clear = SessionClear;
	handler.io_write = SessionWrite;
	handler.io_close = SessionClose;
	handler.on_stateChange = SessionStateChange;
	handler.on_admin = SessionAdmin;
	handler.on_announce = SessionAnnounce;
	handler.on_setPrivacyInfo = SessionSetPrivacyInfo;
	handler.on_setUserStatus = SessionSetUserStatus;
	
	EnterCriticalSection(&session_cs);
	session = mwSession_new(&handler);

	InitMeanwhileServices();

	mwSession_setProperty(session, mwSession_AUTH_USER_ID, options.id, NULL);
	mwSession_setProperty(session, mwSession_AUTH_PASSWORD, options.pword, NULL);
	mwSession_setProperty(session, mwSession_CLIENT_TYPE_ID, (void *)options.client_id, NULL);

	if(options.use_old_default_client_ver) {
		mwSession_setProperty(session, mwSession_CLIENT_VER_MAJOR, GUINT_TO_POINTER(DBGetContactSettingWord(0, PROTO, "ClientVersionMajor", MW_PROTOCOL_VERSION_MAJOR)), 0);
		mwSession_setProperty(session, mwSession_CLIENT_VER_MINOR, GUINT_TO_POINTER(DBGetContactSettingWord(0, PROTO, "ClientVersionMinor", MW_PROTOCOL_VERSION_MINOR)), 0);
	} else {
		mwSession_setProperty(session, mwSession_CLIENT_VER_MAJOR, GUINT_TO_POINTER(DBGetContactSettingWord(0, PROTO, "ClientVersionMajor", 0x001e)), 0);
		mwSession_setProperty(session, mwSession_CLIENT_VER_MINOR, GUINT_TO_POINTER(DBGetContactSettingWord(0, PROTO, "ClientVersionMinor", 0x196f)), 0);
	}

	mwSession_start(session);
	LeaveCriticalSection(&session_cs);

	DWORD tid;
	HANDLE hKAThread = CreateThread(0, 0, KeepAliveThread, 0, 0, &tid);

	//SetEvent(hSessionSyncEvent);

	unsigned char *recv_buffer = new unsigned char[1024 * 32];
	int bytes;
	//while(session && server_connection && mwSession_getState(session) != mwSession_STOPPED) {
	while(server_connection) {// && session) {// && !mwSession_isStopped(session)) { // break on error
		bytes = Netlib_Recv(server_connection, (char *)recv_buffer, 1024 * 32, 0);

		if(bytes == 0) {
			break;
		} else if(bytes == SOCKET_ERROR) {
			// this is normal - e.g. socket closed due to log off, during blocking read above
			break;
		} else {
			EnterCriticalSection(&session_cs);
			mwSession_recv(session, recv_buffer, bytes);
			LeaveCriticalSection(&session_cs);
		}
	}
	delete recv_buffer;

	EnterCriticalSection(&session_cs);
	DeinitMeanwhileServices();

	mwSession *old_session = session;
	session = 0; // kills keepalive thread, if awake

	mwSession_free(old_session);
	LeaveCriticalSection(&session_cs);

	// wake alive thread

	WakeThread(hKAThread);
	WaitForSingleObject(hKAThread, INFINITE);
	CloseHandle(hKAThread);

	QueueUserAPC(sttMainThreadStatusCallback, mainThread, ID_STATUS_OFFLINE);	
	SetAllOffline();

	first_online = true;

	CallService(MS_SYSTEM_THREAD_POP, 0, 0);
	return 0;
}

WORD GetClientVersion() {
	if(!session) return 0;

	WORD retval = 0;

	retval = (int)mwSession_getProperty(session, mwSession_CLIENT_VER_MAJOR) << 8;
	retval |= (int)mwSession_getProperty(session, mwSession_CLIENT_VER_MINOR);

	return retval;
}

WORD GetServerVersion() {
	if(!session) return 0;

	WORD retval = 0;

	retval = (int)mwSession_getProperty(session, mwSession_SERVER_VER_MAJOR) << 8;
	retval |= (int)mwSession_getProperty(session, mwSession_SERVER_VER_MINOR);

	return retval;
}

int LogIn(int ls, HANDLE hNetlibUser) {
	EnterCriticalSection(&session_cs);
	if(session) {
		LeaveCriticalSection(&session_cs);
		return 0;
	}
	LeaveCriticalSection(&session_cs);

	login_status = ls;

	//hSessionSyncEvent = CreateEvent(NULL, true, false, NULL);
	sessionThread = CreateThread(0, 0, SessionThread, (void *)hNetlibUser, 0, &session_thread_id);


	// we can't wait for the thread - critical section (csHooks) from core is locked during call to this function,
	// and hooks are created in the individual init sections...

	CloseHandle(sessionThread);
	//WaitForSingleObject(hSessionSyncEvent, INFINITE);
	//CloseHandle(hSessionSyncEvent);
	//hSessionSyncEvent = 0;

	return 0;
}

int LogOut() {
	continue_connect = false;

	EnterCriticalSection(&session_cs);
	if(session && server_connection && current_status != ID_STATUS_OFFLINE && !mwSession_isStopped(session) && !mwSession_isStopping(session)) 
		mwSession_stop(session, 0);
	LeaveCriticalSection(&session_cs);
	
	return 0;
}

void InitAwayMsg() {
	AwayMessages.szOnline = 0;
	AwayMessages.szAway = 0;
	AwayMessages.szDND = 0;
}

void DeinitAwayMsg() {
	if(AwayMessages.szOnline) free(AwayMessages.szOnline);
	if(AwayMessages.szAway) free(AwayMessages.szAway);
	if(AwayMessages.szDND) free(AwayMessages.szDND);
}

void SendAnnouncement(AnnouncementData *ad) {
	char *utfs = t2u(ad->msg);
	if(session && ad && ad->recipients) mwSession_sendAnnounce(session, false , utfs, ad->recipients);
	free(utfs);
}

int SessionAnnounce(WPARAM wParam, LPARAM lParam) {
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SESSIONANNOUNCE), GetDesktopWindow(), SessionAnnounceDialogProc, (LPARAM)SendAnnouncement);
	return 0;
}

void InitSessionMenu() {
	CreateProtoServiceFunction(PROTO, MS_SAMETIME_MENUANNOUNCESESSION, SessionAnnounce);

	hIconAnnounce = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ANNOUNCE));
	hIconProto = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_PROTO));

	char service_function[128];
	strcpy(service_function, PROTO);
	char *d = service_function + strlen(service_function);

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.popupPosition = 500085001;
	mi.pszPopupName = PROTO;
	mi.position = 2000060000;

	mi.pszName = Translate("Send Announcement...");
	strcpy(d, MS_SAMETIME_MENUANNOUNCESESSION);
	mi.pszService = service_function;

	mi.hIcon = hIconProto;
	//mi.hIcon = hIconAnnounce; // first submenu icon is used as icon for main menu popup...

	hSessionAnnounceMenuItem = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);
}

void DeinitSessionMenu() {
	DestroyIcon(hIconProto);
	DestroyIcon(hIconAnnounce);
}

void InitCritSection() {
	InitializeCriticalSection(&session_cs);

}
void DeinitCritSection() {
	DeleteCriticalSection(&session_cs);
}

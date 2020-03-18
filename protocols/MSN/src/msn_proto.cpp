/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2020 Miranda NG team
Copyright (c) 2008-2012 Boris Krasnovskiy.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "msn_proto.h"

static int CompareLists(const MsnContact *p1, const MsnContact *p2)
{
	return _stricmp(p1->email, p2->email);
}

CMsnProto::CMsnProto(const char* aProtoName, const wchar_t* aUserName) :
	PROTO<CMsnProto>(aProtoName, aUserName),
	m_arContacts(10, CompareLists),
	m_arGroups(10, CompareId),
	m_arThreads(10, PtrKeySortT),
	m_arGCThreads(10, PtrKeySortT),
	lsMessageQueue(1),
	lsAvatarQueue(1),
	msgCache(5, CompareId),
	authContactToken("authContact", "service::contacts.msn.com::MBI_SSL"),
	authStorageToken("authStorage", "service::storage.msn.com::MBI_SSL"),
	authSSLToken("authSSL", "service::ssl.live.com::MBI_SSL"),
	authSkypeComToken("authSkypeCom", "service::skype.com::MBI_SSL"),
	authStrToken("authStr", "service::chatservice.live.com::MBI_SSL", true),
	authSkypeToken("authSkype")
{
	db_set_resident(m_szModuleName, "IdleTS");
	db_set_resident(m_szModuleName, "p2pMsgId");
	db_set_resident(m_szModuleName, "MobileEnabled");
	db_set_resident(m_szModuleName, "MobileAllowed");

	// Initialize tokens
	authContactToken.Init(this);
	authStorageToken.Init(this);
	authSSLToken.Init(this);
	authSkypeComToken.Init(this);
	authStrToken.Init(this);
	authSkypeToken.Init(this);
	LoadAuthTokensDB();

	// Protocol services and events...

	CreateProtoService(PS_CREATEACCMGRUI, &CMsnProto::SvcCreateAccMgrUI);

	CreateProtoService(PS_GETAVATARINFO, &CMsnProto::GetAvatarInfo);
	CreateProtoService(PS_GETMYAWAYMSG, &CMsnProto::GetMyAwayMsg);

	CreateProtoService(PS_LEAVECHAT, &CMsnProto::OnLeaveChat);

	CreateProtoService(PS_GETMYAVATAR, &CMsnProto::GetAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CMsnProto::SetAvatar);
	CreateProtoService(PS_GETAVATARCAPS, &CMsnProto::GetAvatarCaps);

	CreateProtoService(PS_SETMYNICKNAME, &CMsnProto::SetNickName);

	hMSNNudge = CreateProtoEvent(PE_NUDGE);
	CreateProtoService(PS_SEND_NUDGE, &CMsnProto::SendNudge);

	CreateProtoService(PS_GETUNREADEMAILCOUNT, &CMsnProto::GetUnreadEmailCount);

	// event hooks
	HookProtoEvent(ME_MSG_WINDOWPOPUP, &CMsnProto::OnWindowPopup);
	HookProtoEvent(ME_CLIST_GROUPCHANGE, &CMsnProto::OnGroupChange);
	HookProtoEvent(ME_OPT_INITIALISE, &CMsnProto::OnOptionsInit);
	HookProtoEvent(ME_CLIST_DOUBLECLICKED, &CMsnProto::OnContactDoubleClicked);
	HookProtoEvent(ME_DB_CONTACT_SETTINGCHANGED, &CMsnProto::OnDbSettingChanged);

	LoadOptions();

	for (auto &hContact : AccContacts()) {
		delSetting(hContact, "Status");
		delSetting(hContact, "IdleTS");
		delSetting(hContact, "p2pMsgId");
		delSetting(hContact, "AccList");
	}
	delSetting("MobileEnabled");
	delSetting("MobileAllowed");

	char path[MAX_PATH];
	if (db_get_static(NULL, m_szModuleName, "LoginServer", path, sizeof(path)) == 0 &&
		(mir_strcmp(path, MSN_DEFAULT_LOGIN_SERVER) == 0 ||
			mir_strcmp(path, MSN_DEFAULT_GATEWAY) == 0))
		delSetting("LoginServer");

	if (MyOptions.SlowSend) {
		if (db_get_dw(0, "SRMsg", "MessageTimeout", 10000) < 60000)
			db_set_dw(0, "SRMsg", "MessageTimeout", 60000);
		if (db_get_dw(0, "SRMM", "MessageTimeout", 10000) < 60000)
			db_set_dw(0, "SRMM", "MessageTimeout", 60000);
	}

	mailsoundname = (char*)mir_alloc(64);
	mir_snprintf(mailsoundname, 64, "%s:Hotmail", m_szModuleName);
	g_plugin.addSound(mailsoundname, m_tszUserName, LPGENW("Live Mail"));

	alertsoundname = (char*)mir_alloc(64);
	mir_snprintf(alertsoundname, 64, "%s:Alerts", m_szModuleName);
	g_plugin.addSound(alertsoundname, m_tszUserName, LPGENW("Live Alert"));

	AvatarQueue_Init();
	InitCustomFolders();

	wchar_t szBuffer[MAX_PATH];
	mir_snwprintf(szBuffer, TranslateT("%s plugin connections"), m_tszUserName);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = szBuffer;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	m_DisplayNameCache = nullptr;
}

CMsnProto::~CMsnProto()
{
	MSN_FreeGroups();
	Threads_Uninit();
	AvatarQueue_Uninit();
	Lists_Uninit();
	CachedMsg_Uninit();

	mir_free(mailsoundname);
	mir_free(alertsoundname);

	for (int i = 0; i < MSN_NUM_MODES; i++)
		mir_free(msnModeMsgs[i]);

	mir_free(msnLastStatusMsg);
	mir_free(msnPreviousUUX);
	mir_free(msnExternalIP);
	mir_free(msnRegistration);

	mir_free(abCacheKey);
	mir_free(sharingCacheKey);
	mir_free(storageCacheKey);
	mir_free(m_DisplayNameCache);

	FreeAuthTokens();
}

void CMsnProto::OnModulesLoaded()
{
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;
	gcr.iMaxText = 0;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	HookProtoEvent(ME_GC_EVENT, &CMsnProto::MSN_GCEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CMsnProto::MSN_GCMenuHook);

	HookProtoEvent(ME_IDLE_CHANGED, &CMsnProto::OnIdleChanged);
	InitPopups();
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnPreShutdown - prepare a global Miranda shutdown

void CMsnProto::OnShutdown()
{
	g_bTerminated = true;
	ReleaseSemaphore(hevAvatarQueue, 1, nullptr);

	Popup_UnregisterClass(hPopupError);
	Popup_UnregisterClass(hPopupHotmail);
	Popup_UnregisterClass(hPopupNotify);
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnAddToList - adds contact to the server list

MCONTACT CMsnProto::AddToListByEmail(const char *email, const char *nick, DWORD flags)
{
	MCONTACT hContact = MSN_HContactFromEmail(email, nick, true, flags & PALF_TEMPORARY);

	if (flags & PALF_TEMPORARY) {
		if (!Contact_OnList(hContact))
			Contact_Hide(hContact);
	}
	else {
		Contact_Hide(hContact, false);
		if (msnLoggedIn) {
			int netId = strncmp(email, "tel:", 4) ? NETID_MSN : NETID_MOB;
			if (MSN_AddUser(hContact, email, netId, LIST_FL)) {
				MSN_AddUser(hContact, email, netId, LIST_PL + LIST_REMOVE);
				MSN_AddUser(hContact, email, netId, LIST_BL + LIST_REMOVE);
				MSN_AddUser(hContact, email, netId, LIST_AL);
				Contact_Hide(hContact, false);
			}
			MSN_SetContactDb(hContact, email);

			if (MSN_IsMeByContact(hContact)) displayEmailCount(hContact);
		}
		else hContact = NULL;
	}
	return hContact;
}

MCONTACT CMsnProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	wchar_t *id = psr->id.w ? psr->id.w : psr->email.w;
	return AddToListByEmail(
		psr->flags & PSR_UNICODE ? UTF8((wchar_t*)id) : UTF8((char*)id),
		psr->flags & PSR_UNICODE ? UTF8((wchar_t*)psr->nick.w) : UTF8((char*)psr->nick.w),
		flags);
}

MCONTACT CMsnProto::AddToListByEvent(int flags, int, MEVENT hDbEvent)
{
	DBEVENTINFO dbei = {};
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)(-1))
		return NULL;

	dbei.pBlob = (PBYTE)alloca(dbei.cbBlob);
	if (db_event_get(hDbEvent, &dbei)) return NULL;
	if (mir_strcmp(dbei.szModule, m_szModuleName)) return NULL;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) return NULL;

	DB_AUTH_BLOB blob(dbei.pBlob);
	return AddToListByEmail(blob.get_email(), blob.get_nick(), flags);
}

int CMsnProto::AuthRecv(MCONTACT, PROTORECVEVENT* pre)
{
	return Proto_AuthRecv(m_szModuleName, pre);
}

/////////////////////////////////////////////////////////////////////////////////////////
// PSS_AUTHREQUEST

int CMsnProto::AuthRequest(MCONTACT hContact, const wchar_t* szMessage)
{
	if (msnLoggedIn) {
		char email[MSN_MAX_EMAIL_LEN];
		if (db_get_static(hContact, m_szModuleName, "wlid", email, sizeof(email)) &&
			db_get_static(hContact, m_szModuleName, "e-mail", email, sizeof(email)))
			return 1;

		int netId = strncmp(email, "tel:", 4) == 0 ? NETID_MOB : (strncmp(email, "live:", 5) == 0 ? NETID_SKYPE : NETID_MSN);
		if (MSN_AddUser(hContact, email, netId, LIST_FL, T2Utf(szMessage))) {
			MSN_AddUser(hContact, email, netId, LIST_PL + LIST_REMOVE);
			MSN_AddUser(hContact, email, netId, LIST_BL + LIST_REMOVE);
			MSN_AddUser(hContact, email, netId, LIST_AL);
		}
		MSN_SetContactDb(hContact, email);

		if (MSN_IsMeByContact(hContact)) displayEmailCount(hContact);
		return 0;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnAuthAllow - called after successful authorization

int CMsnProto::Authorize(MEVENT hDbEvent)
{
	if (!msnLoggedIn)
		return 1;

	DBEVENTINFO dbei = {};
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == -1)
		return 1;

	dbei.pBlob = (PBYTE)alloca(dbei.cbBlob);
	if (db_event_get(hDbEvent, &dbei))
		return 1;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return 1;

	if (mir_strcmp(dbei.szModule, m_szModuleName))
		return 1;

	DB_AUTH_BLOB blob(dbei.pBlob);

	MCONTACT hContact = MSN_HContactFromEmail(blob.get_email(), blob.get_nick(), true, 0);
	int netId = Lists_GetNetId(blob.get_email());

	MSN_AddUser(hContact, blob.get_email(), netId, LIST_AL);
	MSN_AddUser(hContact, blob.get_email(), netId, LIST_BL + LIST_REMOVE);
	MSN_AddUser(hContact, blob.get_email(), netId, LIST_PL + LIST_REMOVE);

	MSN_SetContactDb(hContact, blob.get_email());
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnAuthDeny - called after unsuccessful authorization

int CMsnProto::AuthDeny(MEVENT hDbEvent, const wchar_t*)
{
	if (!msnLoggedIn)
		return 1;

	DBEVENTINFO dbei = {};
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == -1)
		return 1;

	dbei.pBlob = (PBYTE)alloca(dbei.cbBlob);
	if (db_event_get(hDbEvent, &dbei))
		return 1;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return 1;

	if (mir_strcmp(dbei.szModule, m_szModuleName))
		return 1;

	DB_AUTH_BLOB blob(dbei.pBlob);

	MsnContact* msc = Lists_Get(blob.get_email());
	if (msc == nullptr)
		return 0;

	MSN_AddUser(NULL, blob.get_email(), msc->netId, LIST_PL + LIST_REMOVE);
	MSN_AddUser(NULL, blob.get_email(), msc->netId, LIST_BL);
	MSN_AddUser(NULL, blob.get_email(), msc->netId, LIST_RL);

	if (!(msc->list & (LIST_FL | LIST_LL))) {
		if (msc->hContact) db_delete_contact(msc->hContact);
		msc->hContact = NULL;
		MCONTACT hContact = MSN_HContactFromEmail(blob.get_email());
		if (hContact) db_delete_contact(hContact);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnBasicSearch - search contacts by e-mail

void __cdecl CMsnProto::MsnSearchAckThread(void* arg)
{
	const wchar_t* emailT = (wchar_t*)arg;
	T2Utf email(emailT);

	if (Lists_IsInList(LIST_FL, email)) {
		MSN_ShowPopup(emailT, TranslateT("Contact already in your contact list"), MSN_ALLOW_MSGBOX, nullptr);
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg);
		mir_free(arg);
		return;
	}

	if (MyOptions.netId == NETID_SKYPE) MSN_SKYABSearch(email, arg);
	else {
		unsigned res = MSN_ABContactAdd(email, nullptr, NETID_MSN, nullptr, 1, true);
		switch (res) {
		case 0:
		case 2:
		case 3:
			{
				PROTOSEARCHRESULT psr = {};
				psr.cbSize = sizeof(psr);
				psr.flags = PSR_UNICODE;
				psr.id.w = (wchar_t*)emailT;
				psr.nick.w = (wchar_t*)emailT;
				psr.email.w = (wchar_t*)emailT;

				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, arg, (LPARAM)&psr);
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg);
			}
			break;

		case 1:
			if (strstr(email, "@yahoo.com") == nullptr)
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg);
			break;

		default:
			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg);
			break;
		}
	}
	mir_free(arg);
}

HANDLE CMsnProto::SearchBasic(const wchar_t* id)
{
	if (!msnLoggedIn) return nullptr;

	wchar_t* email = mir_wstrdup(id);
	ForkThread(&CMsnProto::MsnSearchAckThread, email);

	return email;
}

HANDLE CMsnProto::SearchByEmail(const wchar_t* email)
{
	return SearchBasic(email);
}

/////////////////////////////////////////////////////////////////////////////////////////
// stolen from netlibhttp.cpp

static void MyNetlibConnFromUrl(const char* szUrl, NETLIBOPENCONNECTION &nloc)
{
	bool secur = _strnicmp(szUrl, "https", 5) == 0;
	const char* phost = strstr(szUrl, "://");

	char* szHost = mir_strdup(phost ? phost + 3 : szUrl);

	char* ppath = strchr(szHost, '/');
	if (ppath) *ppath = '\0';

	memset(&nloc, 0, sizeof(nloc));
	nloc.szHost = szHost;

	char* pcolon = strrchr(szHost, ':');
	if (pcolon) {
		*pcolon = '\0';
		nloc.wPort = (WORD)strtol(pcolon + 1, nullptr, 10);
	}
	else nloc.wPort = secur ? 443 : 80;
	nloc.flags = (secur ? NLOCF_SSL : 0);
}


void __cdecl CMsnProto::MsnFileAckThread(void* arg)
{
	filetransfer* ft = (filetransfer*)arg;

	wchar_t filefull[MAX_PATH];
	mir_snwprintf(filefull, L"%s\\%s", ft->std.szWorkingDir.w, ft->std.szCurrentFile.w);
	replaceStrW(ft->std.szCurrentFile.w, filefull);

	ResetEvent(ft->hResumeEvt);
	if (ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, ft, (LPARAM)&ft->std))
		WaitForSingleObject(ft->hResumeEvt, INFINITE);

	ft->create();

	ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, ft);

	if (ft->tType == SERVER_HTTP) {
		const char *pszSkypeToken;

		if (ft->fileId != -1 && (pszSkypeToken = authSkypeToken.Token())) {
			NETLIBHTTPHEADER nlbhHeaders[3] = {};
			NETLIBHTTPREQUEST nlhr = { 0 }, *nlhrReply;
			char szRange[32];

			nlbhHeaders[0].szName = "User-Agent";		nlbhHeaders[0].szValue = (LPSTR)MSN_USER_AGENT;
			nlbhHeaders[1].szName = "Authorization";	nlbhHeaders[1].szValue = (char*)pszSkypeToken;
			nlhr.headersCount = 2;
			if (ft->std.currentFileProgress) {
				mir_snprintf(szRange, sizeof(szRange), "bytes=%I64d-", ft->std.currentFileProgress);
				nlbhHeaders[2].szName = "Range";
				nlbhHeaders[2].szValue = szRange;
				nlhr.headersCount++;
			}

			nlhr.cbSize = sizeof(nlhr);
			nlhr.requestType = REQUEST_GET;
			nlhr.flags = NLHRF_HTTP11;
			nlhr.szUrl = ft->szInvcookie;
			nlhr.headers = (NETLIBHTTPHEADER*)&nlbhHeaders;

			NETLIBOPENCONNECTION nloc = {};
			MyNetlibConnFromUrl(nlhr.szUrl, nloc);
			nloc.flags |= NLOCF_HTTP;
			if (nloc.flags & NLOCF_SSL)
				nlhr.flags |= NLHRF_SSL;

			HNETLIBCONN nlc = Netlib_OpenConnection(m_hNetlibUser, &nloc);
			if (nlc && Netlib_SendHttpRequest(nlc, &nlhr) != SOCKET_ERROR && (nlhrReply = Netlib_RecvHttpHeaders(nlc))) {
				if (nlhrReply->resultCode == 200 || nlhrReply->resultCode == 206) {
					ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, ft);

					INT_PTR dw;
					char buf[1024];
					while (!ft->bCanceled && ft->std.currentFileProgress < ft->std.currentFileSize &&
						(dw = Netlib_Recv(nlc, buf, sizeof(buf), MSG_NODUMP)) > 0 && dw != SOCKET_ERROR)
					{
						_write(ft->fileId, buf, dw);
						ft->std.totalProgress += dw;
						ft->std.currentFileProgress += dw;
						ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
					}

					if (ft->std.currentFileProgress == ft->std.currentFileSize)
						ft->std.currentFileNumber++;
				}
				Netlib_FreeHttpRequest(nlhrReply);
			}
			Netlib_CloseHandle(nlc);
			mir_free((char*)nloc.szHost);
			if (ft->std.currentFileNumber >= ft->std.totalFiles)
				ft->complete();
		}
		delete ft;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnFileAllow - starts the file transfer

HANDLE CMsnProto::FileAllow(MCONTACT, HANDLE hTransfer, const wchar_t* szPath)
{
	filetransfer* ft = (filetransfer*)hTransfer;

	if ((ft->std.szWorkingDir.w = mir_wstrdup(szPath)) == nullptr) {
		wchar_t szCurrDir[MAX_PATH];
		GetCurrentDirectory(_countof(szCurrDir), szCurrDir);
		ft->std.szWorkingDir.w = mir_wstrdup(szCurrDir);
	}
	else {
		size_t len = mir_wstrlen(ft->std.szWorkingDir.w) - 1;
		if (ft->std.szWorkingDir.w[len] == '\\')
			ft->std.szWorkingDir.w[len] = 0;
	}

	ForkThread(&CMsnProto::MsnFileAckThread, ft);

	return ft;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnFileCancel - cancels the active file transfer

int CMsnProto::FileCancel(MCONTACT, HANDLE hTransfer)
{
	filetransfer* ft = (filetransfer*)hTransfer;
	if (ft->tType == SERVER_HTTP)
		ft->bCanceled = true;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnFileDeny - rejects the file transfer request

int CMsnProto::FileDeny(MCONTACT, HANDLE hTransfer, const wchar_t* /*szReason*/)
{
	filetransfer* ft = (filetransfer*)hTransfer;
	if (ft->tType == SERVER_HTTP)
		delete ft;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnFileResume - renames a file

int CMsnProto::FileResume(HANDLE hTransfer, int* action, const wchar_t** szFilename)
{
	filetransfer* ft = (filetransfer*)hTransfer;

	if (ft->tType == SERVER_HTTP) {
		switch (*action) {
		case FILERESUME_SKIP:
			ft->close();
			ft->bCanceled = true;
			break;
		case FILERESUME_RENAME:
			replaceStrW(ft->std.szCurrentFile.w, *szFilename);
			break;
		case FILERESUME_OVERWRITE:
			ft->std.currentFileProgress = 0;
			break;
		case FILERESUME_RESUME:
			struct _stati64 statbuf;
			_wstat64(ft->std.szCurrentFile.w, &statbuf);
			ft->std.currentFileProgress = statbuf.st_size;
			break;
		}
		SetEvent(ft->hResumeEvt);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnGetAwayMsg - reads the current status message for a user

struct AwayMsgInfo
{
	INT_PTR id;
	MCONTACT hContact;
};

void __cdecl CMsnProto::MsnGetAwayMsgThread(void* arg)
{
	Sleep(150);

	AwayMsgInfo *inf = (AwayMsgInfo*)arg;

	ptrW wszStatus(db_get_wsa(inf->hContact, "CList", "StatusMsg"));
	ProtoBroadcastAck(inf->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)inf->id, wszStatus);

	mir_free(inf);
}

HANDLE CMsnProto::GetAwayMsg(MCONTACT hContact)
{
	AwayMsgInfo *inf = (AwayMsgInfo*)mir_alloc(sizeof(AwayMsgInfo));
	inf->hContact = hContact;
	inf->id = MSN_GenRandom();

	ForkThread(&CMsnProto::MsnGetAwayMsgThread, inf);
	return (HANDLE)inf->id;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnGetCaps - obtain the protocol capabilities

INT_PTR CMsnProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_SERVERCLIST | PF1_AUTHREQ | PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_CHAT | PF1_CONTACT | 
			PF1_FILERECV | PF1_VISLIST | PF1_MODEMSG;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LIGHTDND | PF2_INVISIBLE | PF2_IDLE;

	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LIGHTDND;

	case PFLAGNUM_4:
		return PF4_FORCEAUTH | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_SUPPORTIDLE | PF4_IMSENDOFFLINE | PF4_NOAUTHDENYREASON;

	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)Translate("Live ID");

	case PFLAG_MAXLENOFMESSAGE:
		return 1202;

	case PFLAG_MAXCONTACTSPERPACKET:
		return 1024; // Only an assumption...
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnRecvMessage - creates a database event from the message been received

MEVENT CMsnProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT* pre)
{
	char tEmail[MSN_MAX_EMAIL_LEN];
	if (!db_get_static(hContact, m_szModuleName, "wlid", tEmail, sizeof(tEmail)) || !db_get_static(hContact, m_szModuleName, "e-mail", tEmail, sizeof(tEmail)))
		if (Lists_IsInList(LIST_FL, tEmail) && db_get_b(hContact, "MetaContacts", "IsSubcontact", 0) == 0)
			Contact_Hide(hContact, false);

	return CSuper::RecvMsg(hContact, pre);
}

int CMsnProto::GetInfo(MCONTACT hContact, int)
{
	if (MyOptions.netId == NETID_SKYPE) {
		char tEmail[MSN_MAX_EMAIL_LEN];
		if (db_get_static(hContact, m_szModuleName, "wlid", tEmail, sizeof(tEmail)) && db_get_static(hContact, m_szModuleName, "e-mail", tEmail, sizeof(tEmail)))
			return 0;

		MSN_SKYABGetProfile(tEmail);
		return 1;
	}
	return 0;
}

// MsnRecvContacts - creates a database event from the contacts received
int CMsnProto::RecvContacts(MCONTACT hContact, PROTORECVEVENT* pre)
{
	PROTOSEARCHRESULT **isrList = (PROTOSEARCHRESULT**)pre->szMessage;
	DBEVENTINFO dbei = {};
	BYTE *pCurBlob;
	int i;

	for (i = 0; i < pre->lParam; i++)
		dbei.cbBlob += int(mir_wstrlen(isrList[i]->nick.w) + 2 + mir_wstrlen(isrList[i]->id.w));
	dbei.pBlob = (PBYTE)_alloca(dbei.cbBlob);
	for (i = 0, pCurBlob = dbei.pBlob; i < pre->lParam; i++) {
		mir_strcpy((char*)pCurBlob, _T2A(isrList[i]->nick.w));
		pCurBlob += mir_strlen((char*)pCurBlob) + 1;
		mir_strcpy((char*)pCurBlob, _T2A(isrList[i]->id.w));
		pCurBlob += mir_strlen((char*)pCurBlob) + 1;
	}

	dbei.szModule = m_szModuleName;
	dbei.timestamp = pre->timestamp;
	dbei.flags = (pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0;
	dbei.eventType = EVENTTYPE_CONTACTS;
	db_event_add(hContact, &dbei);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnSendMessage - sends the message to a server

int CMsnProto::SendMsg(MCONTACT hContact, int flags, const char* pszSrc)
{
	if (!msnLoggedIn) {
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)999999, (LPARAM)Translate("Protocol is offline"));
		return 999999;
	}

	char tEmail[MSN_MAX_EMAIL_LEN];
	if (MSN_IsMeByContact(hContact, tEmail)) {
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)999999, (LPARAM)Translate("You cannot send message to yourself"));
		return 999999;
	}

	char *msg = (char*)pszSrc;
	if (msg == nullptr)
		return 0;

	int rtlFlag = (flags & PREF_RTL) ? MSG_RTL : 0;

	int seq = 0;
	int netId = Lists_GetNetId(tEmail);

	switch (netId) {
	case NETID_MOB:
		if (mir_strlen(msg) > 133) {
			seq = 999997;
			ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)seq, (LPARAM)Translate("Message is too long: SMS page limited to 133 UTF-8 chars"));
		}
		else {
			seq = msnNsThread->sendMessage('1', tEmail, netId, msg, rtlFlag);
			ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)seq);
		}
		break;

	case NETID_YAHOO:
		if (mir_strlen(msg) > 1202) {
			seq = 999996;
			ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)seq, (LPARAM)Translate("Message is too long: MSN messages are limited by 1202 UTF-8 chars"));
		}
		else {
			seq = msnNsThread->sendMessage('1', tEmail, netId, msg, rtlFlag);
			ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)seq);
		}
		break;

	default:
		if (mir_strlen(msg) > 1202) {
			seq = 999996;
			ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)seq, (LPARAM)Translate("Message is too long: MSN messages are limited by 1202 UTF-8 chars"));
		}
		else {
			if (netId != NETID_LCS) {
				seq = msnNsThread->sendMessage('1', tEmail, netId, msg, rtlFlag | MSG_OFFLINE);
				ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)seq);
			}
			else {
				seq = 999993;
				ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)seq, (LPARAM)Translate("Offline messaging is not allowed for LCS contacts"));
			}
		}
		break;
	}

	return seq;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnSendContacts - sends contacts to a certain user

int CMsnProto::SendContacts(MCONTACT hContact, int, int nContacts, MCONTACT *hContactsList)
{
	if (!msnLoggedIn)
		return 0;

	char tEmail[MSN_MAX_EMAIL_LEN];
	if (MSN_IsMeByContact(hContact, tEmail)) return 0;

	int seq = 0;
	int netId = Lists_GetNetId(tEmail);
	CMStringA msg;

	msg.Append("<contacts alt=\"[Contacts enclosed. Please upgrade to latest Skype version to receive contacts.]\">");
	for (int i = 0; i < nContacts; i++) {
		ptrA wlid(getStringA(hContactsList[i], "wlid"));
		if (wlid != NULL)
			msg.AppendFormat("<c t=\"s\" s=\"%s\"/>", wlid.get());
	}
	msg.Append("</contacts>");
	seq = msnNsThread->sendMessage('1', tEmail, netId, msg, MSG_CONTACT);
	ProtoBroadcastAsync(hContact, ACKTYPE_CONTACTS, ACKRESULT_SUCCESS, (HANDLE)seq);
	return seq;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnSetAwayMsg - sets the current status message for a user

int CMsnProto::SetAwayMsg(int status, const wchar_t* msg)
{
	char** msgptr = GetStatusMsgLoc(status);

	if (msgptr == nullptr)
		return 1;

	mir_free(*msgptr);
	char* buf = *msgptr = mir_utf8encodeW(msg);
	if (buf && mir_strlen(buf) > 1859) {
		buf[1859] = 0;
		const int i = 1858;
		if (buf[i] & 128) {
			if (buf[i] & 64)
				buf[i] = '\0';
			else if ((buf[i - 1] & 224) == 224)
				buf[i - 1] = '\0';
			else if ((buf[i - 2] & 240) == 240)
				buf[i - 2] = '\0';
		}
	}

	if (status == m_iDesiredStatus)
		MSN_SendStatusMessage(*msgptr);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnSetStatus - set the plugin's connection status

int CMsnProto::SetStatus(int iNewStatus)
{
	if (m_iDesiredStatus == iNewStatus) return 0;

	m_iDesiredStatus = iNewStatus;
	debugLogA("PS_SETSTATUS(%d,0)", iNewStatus);

	if (m_iDesiredStatus == ID_STATUS_OFFLINE) {
		if (msnNsThread)
			msnNsThread->sendTerminate();
	}
	else if (!msnLoggedIn && m_iStatus == ID_STATUS_OFFLINE) {
		char szPassword[100];
		int ps = db_get_static(NULL, m_szModuleName, "Password", szPassword, sizeof(szPassword));
		if (ps != 0 || *szPassword == 0) {
			ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_WRONGPASSWORD);
			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			return 0;
		}

		if (*MyOptions.szEmail == 0) {
			ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_BADUSERID);
			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			return 0;
		}

		int oldMode = m_iStatus;
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldMode, m_iStatus);

		ThreadData* newThread = new ThreadData;

		newThread->mType = SERVER_NOTIFICATION;
		newThread->mIsMainThread = true;

		newThread->startThread(&CMsnProto::MSNServerThread, this);
	}
	else
		if (m_iStatus > ID_STATUS_OFFLINE) MSN_SetServerStatus(m_iDesiredStatus);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnUserIsTyping - notify another contact that we're typing a message

int CMsnProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (!msnLoggedIn) return 0;

	char tEmail[MSN_MAX_EMAIL_LEN];
	if (MSN_IsMeByContact(hContact, tEmail)) return 0;

	bool typing = type == PROTOTYPE_SELFTYPING_ON;
	int netId = Lists_GetNetId(tEmail);

	if (getWord(hContact, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
		MSN_SendTyping(msnNsThread, tEmail, netId, typing);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//	MsnSetApparentMode - controls contact visibility

int CMsnProto::SetApparentMode(MCONTACT hContact, int mode)
{
	if (mode && mode != ID_STATUS_OFFLINE)
		return 1;

	WORD oldMode = getWord(hContact, "ApparentMode", 0);
	if (mode != oldMode)
		setWord(hContact, "ApparentMode", (WORD)mode);

	return 1;
}

void CMsnProto::OnErase()
{
	char szDbsettings[64];
	mir_snprintf(szDbsettings, "%s_HTTPS", m_szModuleName);
	db_delete_module(0, szDbsettings);
}

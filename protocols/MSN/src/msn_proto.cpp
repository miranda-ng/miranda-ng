/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2017 Miranda NG Team
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

int msn_httpGatewayInit(HNETLIBCONN hConn, NETLIBOPENCONNECTION *nloc, NETLIBHTTPREQUEST *nlhr);
int msn_httpGatewayWrapSend(HNETLIBCONN hConn, PBYTE buf, int len, int flags);
PBYTE msn_httpGatewayUnwrapRecv(NETLIBHTTPREQUEST *nlhr, PBYTE buf, int len, int *outBufLen, void *(*)(void*, size_t));

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

	hMSNNudge = CreateProtoEvent("/Nudge");
	CreateProtoService(PS_SEND_NUDGE, &CMsnProto::SendNudge);

	CreateProtoService(PS_GETUNREADEMAILCOUNT, &CMsnProto::GetUnreadEmailCount);

	// event hooks
	HookProtoEvent(ME_MSG_WINDOWPOPUP, &CMsnProto::OnWindowPopup);
	HookProtoEvent(ME_CLIST_GROUPCHANGE, &CMsnProto::OnGroupChange);
	HookProtoEvent(ME_OPT_INITIALISE, &CMsnProto::OnOptionsInit);
	HookProtoEvent(ME_CLIST_DOUBLECLICKED, &CMsnProto::OnContactDoubleClicked);

	LoadOptions();

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
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
		if (db_get_dw(NULL, "SRMsg", "MessageTimeout", 10000) < 60000)
			db_set_dw(NULL, "SRMsg", "MessageTimeout", 60000);
		if (db_get_dw(NULL, "SRMM", "MessageTimeout", 10000) < 60000)
			db_set_dw(NULL, "SRMM", "MessageTimeout", 60000);
	}

	mailsoundname = (char*)mir_alloc(64);
	mir_snprintf(mailsoundname, 64, "%s:Hotmail", m_szModuleName);
	Skin_AddSound(mailsoundname, m_tszUserName, LPGENW("Live Mail"));

	alertsoundname = (char*)mir_alloc(64);
	mir_snprintf(alertsoundname, 64, "%s:Alerts", m_szModuleName);
	Skin_AddSound(alertsoundname, m_tszUserName, LPGENW("Live Alert"));

	AvatarQueue_Init();
	InitCustomFolders();

	wchar_t szBuffer[MAX_PATH];
	char  szDbsettings[64];

	NETLIBUSER nlu1 = {};
	nlu1.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu1.szSettingsModule = szDbsettings;
	nlu1.szDescriptiveName.w = szBuffer;

	mir_snprintf(szDbsettings, "%s_HTTPS", m_szModuleName);
	mir_snwprintf(szBuffer, TranslateT("%s plugin HTTPS connections"), m_tszUserName);
	hNetlibUserHttps = Netlib_RegisterUser(&nlu1);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = szBuffer;

	nlu.szHttpGatewayUserAgent = (char*)MSN_USER_AGENT;
	nlu.pfnHttpGatewayInit = msn_httpGatewayInit;
	nlu.pfnHttpGatewayWrapSend = msn_httpGatewayWrapSend;
	nlu.pfnHttpGatewayUnwrapRecv = msn_httpGatewayUnwrapRecv;

	mir_snwprintf(szBuffer, TranslateT("%s plugin connections"), m_tszUserName);
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	m_DisplayNameCache = NULL;
}

CMsnProto::~CMsnProto()
{
	MSN_FreeGroups();
	Threads_Uninit();
	AvatarQueue_Uninit();
	Lists_Uninit();
	CachedMsg_Uninit();

	Netlib_CloseHandle(m_hNetlibUser);
	Netlib_CloseHandle(hNetlibUserHttps);

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

int CMsnProto::OnModulesLoaded(WPARAM, LPARAM)
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
	return 0;
}

// OnPreShutdown - prepare a global Miranda shutdown
int CMsnProto::OnPreShutdown(WPARAM, LPARAM)
{
	g_bTerminated = true;
	ReleaseSemaphore(hevAvatarQueue, 1, NULL);

	Popup_UnregisterClass(hPopupError);
	Popup_UnregisterClass(hPopupHotmail);
	Popup_UnregisterClass(hPopupNotify);
	return 0;
}

// MsnAddToList - adds contact to the server list
MCONTACT CMsnProto::AddToListByEmail(const char *email, const char *nick, DWORD flags)
{
	MCONTACT hContact = MSN_HContactFromEmail(email, nick, true, flags & PALF_TEMPORARY);

	if (flags & PALF_TEMPORARY) {
		if (db_get_b(hContact, "CList", "NotOnList", 0) == 1)
			db_set_b(hContact, "CList", "Hidden", 1);
	}
	else {
		db_unset(hContact, "CList", "Hidden");
		if (msnLoggedIn) {
			int netId = strncmp(email, "tel:", 4) ? NETID_MSN : NETID_MOB;
			if (MSN_AddUser(hContact, email, netId, LIST_FL)) {
				MSN_AddUser(hContact, email, netId, LIST_PL + LIST_REMOVE);
				MSN_AddUser(hContact, email, netId, LIST_BL + LIST_REMOVE);
				MSN_AddUser(hContact, email, netId, LIST_AL);
				db_unset(hContact, "CList", "Hidden");
			}
			MSN_SetContactDb(hContact, email);

			if (MSN_IsMeByContact(hContact)) displayEmailCount(hContact);
		}
		else hContact = NULL;
	}
	return hContact;
}

MCONTACT __cdecl CMsnProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	wchar_t *id = psr->id.w ? psr->id.w : psr->email.w;
	return AddToListByEmail(
		psr->flags & PSR_UNICODE ? UTF8((wchar_t*)id) : UTF8((char*)id),
		psr->flags & PSR_UNICODE ? UTF8((wchar_t*)psr->nick.w) : UTF8((char*)psr->nick.w),
		flags);
}

MCONTACT __cdecl CMsnProto::AddToListByEvent(int flags, int, MEVENT hDbEvent)
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

// PSS_AUTHREQUEST
int __cdecl CMsnProto::AuthRequest(MCONTACT hContact, const wchar_t* szMessage)
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
	if (msc == NULL) return 0;

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

// MsnBasicSearch - search contacts by e-mail
void __cdecl CMsnProto::MsnSearchAckThread(void* arg)
{
	const wchar_t* emailT = (wchar_t*)arg;
	T2Utf email(emailT);

	if (Lists_IsInList(LIST_FL, email)) {
		MSN_ShowPopup(emailT, TranslateT("Contact already in your contact list"), MSN_ALLOW_MSGBOX, NULL);
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg, 0);
		mir_free(arg);
		return;
	}

	if (MyOptions.netId == NETID_SKYPE) MSN_SKYABSearch(email, arg);
	else {
		unsigned res = MSN_ABContactAdd(email, NULL, NETID_MSN, NULL, 1, true);
		switch (res) {
		case 0:
		case 2:
		case 3:
		{
			PROTOSEARCHRESULT psr = { 0 };
			psr.cbSize = sizeof(psr);
			psr.flags = PSR_UNICODE;
			psr.id.w = (wchar_t*)emailT;
			psr.nick.w = (wchar_t*)emailT;
			psr.email.w = (wchar_t*)emailT;

			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, arg, (LPARAM)&psr);
			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg, 0);
		}
		break;

		case 1:
			if (strstr(email, "@yahoo.com") == NULL)
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg, 0);
			break;

		default:
			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg, 0);
			break;
		}
	}
	mir_free(arg);
}


HANDLE __cdecl CMsnProto::SearchBasic(const wchar_t* id)
{
	if (!msnLoggedIn) return 0;

	wchar_t* email = mir_wstrdup(id);
	ForkThread(&CMsnProto::MsnSearchAckThread, email);

	return email;
}

HANDLE __cdecl CMsnProto::SearchByEmail(const wchar_t* email)
{
	return SearchBasic(email);
}

// stolen from netlibhttp.cpp
static void MyNetlibConnFromUrl(const char* szUrl, NETLIBOPENCONNECTION &nloc)
{
	bool secur =_strnicmp(szUrl, "https", 5) == 0;
	const char* phost = strstr(szUrl, "://");

	char* szHost = mir_strdup(phost ? phost + 3 : szUrl);

	char* ppath = strchr(szHost, '/');
	if (ppath) *ppath = '\0';

	memset(&nloc, 0, sizeof(nloc));
	nloc.cbSize = sizeof(nloc);
	nloc.szHost = szHost;

	char* pcolon = strrchr(szHost, ':');
	if (pcolon) {
		*pcolon = '\0';
		nloc.wPort = (WORD)strtol(pcolon+1, NULL, 10);
	}
	else nloc.wPort = secur ? 443 : 80;
	nloc.flags = (secur ? NLOCF_SSL : 0);
}


void __cdecl CMsnProto::MsnFileAckThread(void* arg)
{
	filetransfer* ft = (filetransfer*)arg;

	wchar_t filefull[MAX_PATH];
	mir_snwprintf(filefull, L"%s\\%s", ft->std.tszWorkingDir, ft->std.tszCurrentFile);
	replaceStrW(ft->std.tszCurrentFile, filefull);

	ResetEvent(ft->hResumeEvt);
	if (ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, ft, (LPARAM)&ft->std))
		WaitForSingleObject(ft->hResumeEvt, INFINITE);

	ft->create();

	ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, ft, 0);

	if (ft->tType == SERVER_HTTP) {
		const char *pszSkypeToken;

		if (ft->fileId != -1 && (pszSkypeToken=authSkypeToken.Token())) {
			NETLIBHTTPHEADER nlbhHeaders[3] = { 0 };
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

			NETLIBOPENCONNECTION nloc = { 0 };
			MyNetlibConnFromUrl(nlhr.szUrl, nloc);
			nloc.flags |= NLOCF_HTTP;
			if (nloc.flags & NLOCF_SSL)
				nlhr.flags |= NLHRF_SSL;
			
			HNETLIBCONN nlc = Netlib_OpenConnection(m_hNetlibUser, &nloc);
			if (nlc && Netlib_SendHttpRequest(nlc, &nlhr) != SOCKET_ERROR && (nlhrReply = Netlib_RecvHttpHeaders(nlc))) {
				if (nlhrReply->resultCode == 200 || nlhrReply->resultCode == 206) {
					INT_PTR dw;
					char buf[1024];

					ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, ft, 0);
					while (!ft->bCanceled && ft->std.currentFileProgress < ft->std.currentFileSize &&
						(dw = Netlib_Recv(nlc, buf, sizeof(buf), MSG_NODUMP))>0 && dw!=SOCKET_ERROR) 
					{
						_write(ft->fileId, buf, dw);
						ft->std.totalProgress += dw;
						ft->std.currentFileProgress += dw;
						ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
					}
					if (ft->std.currentFileProgress == ft->std.currentFileSize) ft->std.currentFileNumber++;

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

// MsnFileAllow - starts the file transfer
HANDLE __cdecl CMsnProto::FileAllow(MCONTACT, HANDLE hTransfer, const wchar_t* szPath)
{
	filetransfer* ft = (filetransfer*)hTransfer;

	if ((ft->std.tszWorkingDir = mir_wstrdup(szPath)) == NULL) {
		wchar_t szCurrDir[MAX_PATH];
		GetCurrentDirectory(_countof(szCurrDir), szCurrDir);
		ft->std.tszWorkingDir = mir_wstrdup(szCurrDir);
	}
	else {
		size_t len = mir_wstrlen(ft->std.tszWorkingDir) - 1;
		if (ft->std.tszWorkingDir[len] == '\\')
			ft->std.tszWorkingDir[len] = 0;
	}

	ForkThread(&CMsnProto::MsnFileAckThread, ft);

	return ft;
}

// MsnFileCancel - cancels the active file transfer
int __cdecl CMsnProto::FileCancel(MCONTACT, HANDLE hTransfer)
{
	filetransfer* ft = (filetransfer*)hTransfer;

	if (ft->tType == SERVER_HTTP)
		ft->bCanceled = true;
	return 0;
}

// MsnFileDeny - rejects the file transfer request
int __cdecl CMsnProto::FileDeny(MCONTACT, HANDLE hTransfer, const wchar_t* /*szReason*/)
{
	filetransfer* ft = (filetransfer*)hTransfer;
	if (ft->tType == SERVER_HTTP)
		delete ft;

	return 0;
}

// MsnFileResume - renames a file
int __cdecl CMsnProto::FileResume(HANDLE hTransfer, int* action, const wchar_t** szFilename)
{
	filetransfer* ft = (filetransfer*)hTransfer;

	if (ft->tType == SERVER_HTTP) {
		switch (*action) {
			case FILERESUME_SKIP:
				ft->close();
				ft->bCanceled = true;
				break;
			case FILERESUME_RENAME:
				replaceStrW(ft->std.tszCurrentFile, *szFilename);
				break;
			case FILERESUME_OVERWRITE:
				ft->std.currentFileProgress = 0;
				break;
			case FILERESUME_RESUME:
				{
					struct _stati64 statbuf;
					_wstat64(ft->std.tszCurrentFile, &statbuf);
					ft->std.currentFileProgress = statbuf.st_size;
				}
				break;
		}
		SetEvent(ft->hResumeEvt);
	}
	return 0;
}

typedef struct AwayMsgInfo_tag
{
	INT_PTR id;
	MCONTACT hContact;
} AwayMsgInfo;

// MsnGetAwayMsg - reads the current status message for a user
void __cdecl CMsnProto::MsnGetAwayMsgThread(void* arg)
{
	Sleep(150);

	AwayMsgInfo *inf = (AwayMsgInfo*)arg;
	DBVARIANT dbv;
	if (!db_get_ws(inf->hContact, "CList", "StatusMsg", &dbv)) {
		ProtoBroadcastAck(inf->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)inf->id, (LPARAM)dbv.ptszVal);
		db_free(&dbv);
	}
	else ProtoBroadcastAck(inf->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)inf->id, 0);

	mir_free(inf);
}

HANDLE __cdecl CMsnProto::GetAwayMsg(MCONTACT hContact)
{
	AwayMsgInfo* inf = (AwayMsgInfo*)mir_alloc(sizeof(AwayMsgInfo));
	inf->hContact = hContact;
	inf->id = MSN_GenRandom();

	ForkThread(&CMsnProto::MsnGetAwayMsgThread, inf);
	return (HANDLE)inf->id;
}

// MsnGetCaps - obtain the protocol capabilities
DWORD_PTR __cdecl CMsnProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_SERVERCLIST | PF1_AUTHREQ | PF1_BASICSEARCH |
			PF1_ADDSEARCHRES | PF1_CHAT | PF1_CONTACT | 
			/*PF1_FILESEND |*/ PF1_FILERECV | PF1_URLRECV | PF1_VISLIST | PF1_MODEMSG;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LIGHTDND | PF2_INVISIBLE | PF2_ONTHEPHONE | PF2_IDLE;

	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LIGHTDND;

	case PFLAGNUM_4:
		return PF4_FORCEAUTH | PF4_FORCEADDED | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_SUPPORTIDLE | PF4_IMSENDOFFLINE | PF4_NOAUTHDENYREASON;

	case PFLAGNUM_5:
		return PF2_ONTHEPHONE;

	case PFLAG_UNIQUEIDTEXT:
		return (UINT_PTR)Translate("Live ID");

	case PFLAG_UNIQUEIDSETTING:
		return (UINT_PTR)"wlid";

	case PFLAG_MAXLENOFMESSAGE:
		return 1202;

	case PFLAG_MAXCONTACTSPERPACKET:
		return 1024; // Only an assumption...
	}

	return 0;
}

// MsnRecvMessage - creates a database event from the message been received
int __cdecl CMsnProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT* pre)
{
	char tEmail[MSN_MAX_EMAIL_LEN];
	if (!db_get_static(hContact, m_szModuleName, "wlid", tEmail, sizeof(tEmail))
			|| !db_get_static(hContact, m_szModuleName, "e-mail", tEmail, sizeof(tEmail))) {
		if (Lists_IsInList(LIST_FL, tEmail) && db_get_b(hContact, "MetaContacts", "IsSubcontact", 0) == 0)
			db_unset(hContact, "CList", "Hidden");
	}


	return Proto_RecvMessage(hContact, pre);
}

int CMsnProto::GetInfo(MCONTACT hContact, int)
{
	if (MyOptions.netId == NETID_SKYPE) {
		char tEmail[MSN_MAX_EMAIL_LEN];
		if (db_get_static(hContact, m_szModuleName, "wlid", tEmail, sizeof(tEmail))
				&& db_get_static(hContact, m_szModuleName, "e-mail", tEmail, sizeof(tEmail)))
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

struct TFakeAckParams
{
	inline TFakeAckParams(MCONTACT p2, long p3, const char* p4, CMsnProto *p5, int p6=ACKTYPE_MESSAGE) :
		hContact(p2),
		id(p3),
		msg(p4),
		proto(p5),
		type(p6)
	{}

	MCONTACT hContact;
	int     type;
	long	id;
	const char*	msg;
	CMsnProto *proto;
};

void CMsnProto::MsnFakeAck(void* arg)
{
	TFakeAckParams* tParam = (TFakeAckParams*)arg;

	Sleep(150);
	tParam->proto->ProtoBroadcastAck(tParam->hContact, tParam->type,
		tParam->msg ? ACKRESULT_FAILED : ACKRESULT_SUCCESS,
		(HANDLE)tParam->id, LPARAM(tParam->msg));

	delete tParam;
}

// MsnSendMessage - sends the message to a server
int __cdecl CMsnProto::SendMsg(MCONTACT hContact, int flags, const char* pszSrc)
{
	const char *errMsg = NULL;

	if (!msnLoggedIn) {
		errMsg = Translate("Protocol is offline");
		ForkThread(&CMsnProto::MsnFakeAck, new TFakeAckParams(hContact, 999999, errMsg, this));
		return 999999;
	}

	char tEmail[MSN_MAX_EMAIL_LEN];
	if (MSN_IsMeByContact(hContact, tEmail)) {
		errMsg = Translate("You cannot send message to yourself");
		ForkThread(&CMsnProto::MsnFakeAck, new TFakeAckParams(hContact, 999999, errMsg, this));
		return 999999;
	}

	char *msg = (char*)pszSrc;
	if (msg == NULL)
		return 0;

	int rtlFlag = (flags & PREF_RTL) ? MSG_RTL : 0;

	int seq = 0;
	int netId = Lists_GetNetId(tEmail);

	switch (netId) {
	case NETID_MOB:
		if (mir_strlen(msg) > 133) {
			errMsg = Translate("Message is too long: SMS page limited to 133 UTF8 chars");
			seq = 999997;
		}
		else {
			errMsg = NULL;
			seq = msnNsThread->sendMessage('1', tEmail, netId, msg, rtlFlag);
		}
		ForkThread(&CMsnProto::MsnFakeAck, new TFakeAckParams(hContact, seq, errMsg, this));
		break;

	case NETID_YAHOO:
		if (mir_strlen(msg) > 1202) {
			seq = 999996;
			errMsg = Translate("Message is too long: MSN messages are limited by 1202 UTF8 chars");
			ForkThread(&CMsnProto::MsnFakeAck, new TFakeAckParams(hContact, seq, errMsg, this));
		}
		else {
			seq = msnNsThread->sendMessage('1', tEmail, netId, msg, rtlFlag);
			ForkThread(&CMsnProto::MsnFakeAck, new TFakeAckParams(hContact, seq, NULL, this));
		}
		break;

	default:
		if (mir_strlen(msg) > 1202) {
			seq = 999996;
			errMsg = Translate("Message is too long: MSN messages are limited by 1202 UTF8 chars");
			ForkThread(&CMsnProto::MsnFakeAck, new TFakeAckParams(hContact, seq, errMsg, this));
		}
		else {
			// MSNP24 doesn't have a switchboard anymore
			bool isOffline = true;
			ThreadData *thread = NULL;

			if (thread == NULL) {
				if (isOffline) {
					if (netId != NETID_LCS) {
						seq = msnNsThread->sendMessage('1', tEmail, netId, msg, rtlFlag | MSG_OFFLINE);
						ForkThread(&CMsnProto::MsnFakeAck, new TFakeAckParams(hContact, seq, NULL, this));
					}
					else {
						seq = 999993;
						errMsg = Translate("Offline messaging is not allowed for LCS contacts");
						ForkThread(&CMsnProto::MsnFakeAck, new TFakeAckParams(hContact, seq, errMsg, this));
					}
				}
			}
		}
		break;
	}

	return seq;
}

// MsnSendContacts - sends contacts to a certain user
int __cdecl CMsnProto::SendContacts(MCONTACT hContact, int, int nContacts, MCONTACT *hContactsList)
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
		if (wlid != NULL) msg.AppendFormat("<c t=\"s\" s=\"%s\"/>", wlid);
	}
	msg.Append("</contacts>");
	seq = msnNsThread->sendMessage('1', tEmail, netId, msg, MSG_CONTACT);
	ForkThread(&CMsnProto::MsnFakeAck, new TFakeAckParams(hContact, seq, NULL, this, ACKTYPE_CONTACTS));
	return seq;
}

// MsnSetAwayMsg - sets the current status message for a user
int __cdecl CMsnProto::SetAwayMsg(int status, const wchar_t* msg)
{
	char** msgptr = GetStatusMsgLoc(status);

	if (msgptr == NULL)
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

// MsnSetStatus - set the plugin's connection status
int __cdecl CMsnProto::SetStatus(int iNewStatus)
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
			ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);
			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			return 0;
		}

		if (*MyOptions.szEmail == 0) {
			ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			return 0;
		}

		usingGateway = false;

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

// MsnUserIsTyping - notify another contact that we're typing a message
int __cdecl CMsnProto::UserIsTyping(MCONTACT hContact, int type)
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

//	MsnSetApparentMode - controls contact visibility
int __cdecl CMsnProto::SetApparentMode(MCONTACT hContact, int mode)
{
	if (mode && mode != ID_STATUS_OFFLINE)
		return 1;

	WORD oldMode = getWord(hContact, "ApparentMode", 0);
	if (mode != oldMode)
		setWord(hContact, "ApparentMode", (WORD)mode);

	return 1;
}

int __cdecl CMsnProto::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	switch (eventType) {
	case EV_PROTO_ONLOAD:
		return OnModulesLoaded(0, 0);

	case EV_PROTO_ONEXIT:
		return OnPreShutdown(0, 0);

	case EV_PROTO_ONOPTIONS:
		return OnOptionsInit(wParam, lParam);

	case EV_PROTO_ONMENU:
		MsnInitMainMenu();
		break;

	case EV_PROTO_ONERASE:
		char szDbsettings[64];
		mir_snprintf(szDbsettings, "%s_HTTPS", m_szModuleName);
		db_delete_module(0, szDbsettings);
		break;

	case EV_PROTO_ONCONTACTDELETED:
		return OnContactDeleted(wParam, lParam);

	case EV_PROTO_DBSETTINGSCHANGED:
		return OnDbSettingChanged(wParam, lParam);
	}
	return 1;
}

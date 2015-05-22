/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
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

#include "msn_global.h"
#include "msn_proto.h"

static const COLORREF crCols[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

int msn_httpGatewayInit(HANDLE hConn,NETLIBOPENCONNECTION *nloc,NETLIBHTTPREQUEST *nlhr);
int msn_httpGatewayBegin(HANDLE hConn,NETLIBOPENCONNECTION *nloc);
int msn_httpGatewayWrapSend(HANDLE hConn,PBYTE buf,int len,int flags,MIRANDASERVICE pfnNetlibSend);
PBYTE msn_httpGatewayUnwrapRecv(NETLIBHTTPREQUEST *nlhr,PBYTE buf,int len,int *outBufLen,void *(*NetlibRealloc)(void*,size_t));

static int CompareLists(const MsnContact *p1, const MsnContact *p2)
{
	return _stricmp(p1->email, p2->email);
}

CMsnProto::CMsnProto(const char* aProtoName, const TCHAR* aUserName) :
	PROTO<CMsnProto>(aProtoName, aUserName),
	m_arContacts(10, CompareLists),
	m_arGroups(10, CompareId),
	m_arThreads(10, PtrKeySortT),
	m_arGCThreads(10, PtrKeySortT),
#ifdef OBSOLETE
	m_arSessions(10, PtrKeySortT),
	m_arDirect(10, PtrKeySortT),
#endif
	lsMessageQueue(1),
	lsAvatarQueue(1),
	msgCache(5, CompareId)
{
	db_set_resident(m_szModuleName, "Status");
	db_set_resident(m_szModuleName, "IdleTS");
	db_set_resident(m_szModuleName, "p2pMsgId");
	db_set_resident(m_szModuleName, "MobileEnabled");
	db_set_resident(m_szModuleName, "MobileAllowed");

	// Protocol services and events...

	CreateProtoService(PS_CREATEACCMGRUI, &CMsnProto::SvcCreateAccMgrUI);

	CreateProtoService(PS_GETAVATARINFOT, &CMsnProto::GetAvatarInfo);
	CreateProtoService(PS_GETMYAWAYMSG, &CMsnProto::GetMyAwayMsg);

	CreateProtoService(PS_LEAVECHAT, &CMsnProto::OnLeaveChat);

	CreateProtoService(PS_GETMYAVATART, &CMsnProto::GetAvatar);
	CreateProtoService(PS_SETMYAVATART, &CMsnProto::SetAvatar);
	CreateProtoService(PS_GETAVATARCAPS, &CMsnProto::GetAvatarCaps);

	CreateProtoService(PS_SETMYNICKNAME, &CMsnProto::SetNickName);
#ifdef OBSOLETE
	CreateProtoService(PS_GET_LISTENINGTO, &CMsnProto::GetCurrentMedia);
	CreateProtoService(PS_SET_LISTENINGTO, &CMsnProto::SetCurrentMedia);

	MsgQueue_Init();
#endif

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
		(strcmp(path, MSN_DEFAULT_LOGIN_SERVER) == 0 ||
		strcmp(path, MSN_DEFAULT_GATEWAY) == 0))
		delSetting("LoginServer");

	if (MyOptions.SlowSend) {
		if (db_get_dw(NULL, "SRMsg", "MessageTimeout", 10000) < 60000)
			db_set_dw(NULL, "SRMsg", "MessageTimeout", 60000);
		if (db_get_dw(NULL, "SRMM", "MessageTimeout", 10000) < 60000)
			db_set_dw(NULL, "SRMM", "MessageTimeout", 60000);
	}

	mailsoundname = (char*)mir_alloc(64);
	mir_snprintf(mailsoundname, 64, "%s:Hotmail", m_szModuleName);
	SkinAddNewSoundExT(mailsoundname, m_tszUserName, LPGENT("Live Mail"));

	alertsoundname = (char*)mir_alloc(64);
	mir_snprintf(alertsoundname, 64, "%s:Alerts", m_szModuleName);
	SkinAddNewSoundExT(alertsoundname, m_tszUserName, LPGENT("Live Alert"));

	AvatarQueue_Init();
	InitCustomFolders();

	TCHAR szBuffer[MAX_PATH];
	char  szDbsettings[64];

	NETLIBUSER nlu1 = { 0 };
	nlu1.cbSize = sizeof(nlu1);
	nlu1.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu1.szSettingsModule = szDbsettings;
	nlu1.ptszDescriptiveName = szBuffer;

	mir_snprintf(szDbsettings, SIZEOF(szDbsettings), "%s_HTTPS", m_szModuleName);
	mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s plugin HTTPS connections"), m_tszUserName);
	hNetlibUserHttps = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu1);

	NETLIBUSER nlu = { 0 };
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = m_szModuleName;
	nlu.ptszDescriptiveName = szBuffer;

	nlu.szHttpGatewayUserAgent = (char*)MSN_USER_AGENT;
	nlu.pfnHttpGatewayInit = msn_httpGatewayInit;
	nlu.pfnHttpGatewayWrapSend = msn_httpGatewayWrapSend;
	nlu.pfnHttpGatewayUnwrapRecv = msn_httpGatewayUnwrapRecv;

	mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s plugin connections"), m_tszUserName);
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	m_DisplayNameCache = NULL;
}

CMsnProto::~CMsnProto()
{
	MsnRemoveMainMenus();

	MSN_FreeGroups();
	Threads_Uninit();
	AvatarQueue_Uninit();
	Lists_Uninit();
#ifdef OBSOLETE
	DestroyHookableEvent(hMSNNudge);
	P2pSessions_Uninit();
	MsgQueue_Uninit();
#endif
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
	GCREGISTER gcr = { 0 };
	gcr.cbSize = sizeof(GCREGISTER);
	gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;
	gcr.iMaxText = 0;
	gcr.nColors = 16;
	gcr.pColors = (COLORREF*)crCols;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	CallServiceSync(MS_GC_REGISTER, 0, (LPARAM)&gcr);

	HookProtoEvent(ME_GC_EVENT, &CMsnProto::MSN_GCEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CMsnProto::MSN_GCMenuHook);

	HookProtoEvent(ME_IDLE_CHANGED, &CMsnProto::OnIdleChanged);
	InitPopups();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnPreShutdown - prepare a global Miranda shutdown

int CMsnProto::OnPreShutdown(WPARAM, LPARAM)
{
	SetEvent(hevAvatarQueue);

	Popup_UnregisterClass(hPopupError);
	Popup_UnregisterClass(hPopupHotmail);
	Popup_UnregisterClass(hPopupNotify);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
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
	TCHAR *id = psr->id ? psr->id : psr->email;
	return AddToListByEmail(
		psr->flags & PSR_UNICODE ? UTF8((wchar_t*)id) : UTF8((char*)id),
		psr->flags & PSR_UNICODE ? UTF8((wchar_t*)psr->nick) : UTF8((char*)psr->nick),
		flags);
}

MCONTACT __cdecl CMsnProto::AddToListByEvent(int flags, int, MEVENT hDbEvent)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)(-1))
		return NULL;

	dbei.pBlob = (PBYTE)alloca(dbei.cbBlob);
	if (db_event_get(hDbEvent, &dbei)) return NULL;
	if (strcmp(dbei.szModule, m_szModuleName)) return NULL;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) return NULL;

	char* nick = (char *)(dbei.pBlob + sizeof(DWORD) * 2);
	char* firstName = nick + mir_strlen(nick) + 1;
	char* lastName = firstName + mir_strlen(firstName) + 1;
	char* email = lastName + mir_strlen(lastName) + 1;

	return AddToListByEmail(email, nick, flags);
}

int CMsnProto::AuthRecv(MCONTACT, PROTORECVEVENT* pre)
{
	Proto_AuthRecv(m_szModuleName, pre);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSS_AUTHREQUEST

int __cdecl CMsnProto::AuthRequest(MCONTACT hContact, const TCHAR* szMessage)
{
	if (msnLoggedIn) {
		char email[MSN_MAX_EMAIL_LEN];
		if (db_get_static(hContact, m_szModuleName, "wlid", email, sizeof(email)) &&
			db_get_static(hContact, m_szModuleName, "e-mail", email, sizeof(email)))
			return 1;

		char* szMsg = mir_utf8encodeT(szMessage);

		int netId = strncmp(email, "tel:", 4) == 0 ? NETID_MOB : (strncmp(email, "live:", 5) == 0 ? NETID_SKYPE : NETID_MSN);
		if (MSN_AddUser(hContact, email, netId, LIST_FL, szMsg)) {
			MSN_AddUser(hContact, email, netId, LIST_PL + LIST_REMOVE);
			MSN_AddUser(hContact, email, netId, LIST_BL + LIST_REMOVE);
			MSN_AddUser(hContact, email, netId, LIST_AL);
		}
		MSN_SetContactDb(hContact, email);
		mir_free(szMsg);

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

	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == -1)
		return 1;

	dbei.pBlob = (PBYTE)alloca(dbei.cbBlob);
	if (db_event_get(hDbEvent, &dbei))
		return 1;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return 1;

	if (strcmp(dbei.szModule, m_szModuleName))
		return 1;

	char *nick = (char*)(dbei.pBlob + sizeof(DWORD) * 2);
	char *firstName = nick + mir_strlen(nick) + 1;
	char *lastName = firstName + mir_strlen(firstName) + 1;
	char *email = lastName + mir_strlen(lastName) + 1;

	MCONTACT hContact = MSN_HContactFromEmail(email, nick, true, 0);
	int netId = Lists_GetNetId(email);

	MSN_AddUser(hContact, email, netId, LIST_AL);
	MSN_AddUser(hContact, email, netId, LIST_BL + LIST_REMOVE);
	MSN_AddUser(hContact, email, netId, LIST_PL + LIST_REMOVE);

	MSN_SetContactDb(hContact, email);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnAuthDeny - called after unsuccessful authorization

int CMsnProto::AuthDeny(MEVENT hDbEvent, const TCHAR*)
{
	if (!msnLoggedIn)
		return 1;

	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == -1)
		return 1;

	dbei.pBlob = (PBYTE)alloca(dbei.cbBlob);
	if (db_event_get(hDbEvent, &dbei))
		return 1;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return 1;

	if (strcmp(dbei.szModule, m_szModuleName))
		return 1;

	char* nick = (char*)(dbei.pBlob + sizeof(DWORD) * 2);
	char* firstName = nick + mir_strlen(nick) + 1;
	char* lastName = firstName + mir_strlen(firstName) + 1;
	char* email = lastName + mir_strlen(lastName) + 1;

	MsnContact* msc = Lists_Get(email);
	if (msc == NULL) return 0;

	MSN_AddUser(NULL, email, msc->netId, LIST_PL + LIST_REMOVE);
	MSN_AddUser(NULL, email, msc->netId, LIST_BL);
	MSN_AddUser(NULL, email, msc->netId, LIST_RL);

	if (!(msc->list & (LIST_FL | LIST_LL))) {
		if (msc->hContact) CallService(MS_DB_CONTACT_DELETE, (WPARAM)msc->hContact, 0);
		msc->hContact = NULL;
		MCONTACT hContact = MSN_HContactFromEmail(email);
		if (hContact) CallService(MS_DB_CONTACT_DELETE, hContact, 0);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnBasicSearch - search contacts by e-mail

void __cdecl CMsnProto::MsnSearchAckThread(void* arg)
{
	const TCHAR* emailT = (TCHAR*)arg;
	char *email = mir_utf8encodeT(emailT);

	if (Lists_IsInList(LIST_FL, email)) {
		MSN_ShowPopup(emailT, TranslateT("Contact already in your contact list"), MSN_ALLOW_MSGBOX, NULL);
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg, 0);
		mir_free(arg);
		return;
	}

	unsigned res = MSN_ABContactAdd(email, NULL, NETID_MSN, NULL, 1, true);
	switch (res) {
	case 0:
	case 2:
	case 3:
	{
		PROTOSEARCHRESULT isr = { 0 };
		isr.cbSize = sizeof(isr);
		isr.flags = PSR_TCHAR;
		isr.id = (TCHAR*)emailT;
		isr.nick = (TCHAR*)emailT;
		isr.email = (TCHAR*)emailT;

		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, arg, (LPARAM)&isr);
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg, 0);
	}
	break;

	case 1:
		if (strstr(email, "@yahoo.com") == NULL)
			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg, 0);
#ifdef OBSOLETE
		else {
			msnSearchId = arg;
			MSN_FindYahooUser(email);
		}
#endif
		break;

	default:
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, arg, 0);
		break;
	}
	mir_free(email);
	mir_free(arg);
}


HANDLE __cdecl CMsnProto::SearchBasic(const PROTOCHAR* id)
{
	if (!msnLoggedIn) return 0;

	TCHAR* email = mir_tstrdup(id);
	ForkThread(&CMsnProto::MsnSearchAckThread, email);

	return email;
}

HANDLE __cdecl CMsnProto::SearchByEmail(const PROTOCHAR* email)
{
	return SearchBasic(email);
}

#ifdef OBSOLETE
/////////////////////////////////////////////////////////////////////////////////////////
// MsnFileAllow - starts the file transfer

void __cdecl CMsnProto::MsnFileAckThread(void* arg)
{
	filetransfer* ft = (filetransfer*)arg;

	TCHAR filefull[MAX_PATH];
	mir_sntprintf(filefull, SIZEOF(filefull), _T("%s\\%s"), ft->std.tszWorkingDir, ft->std.tszCurrentFile);
	replaceStrT(ft->std.tszCurrentFile, filefull);

	if (ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, ft, (LPARAM)&ft->std))
		return;

	bool fcrt = ft->create() != -1;

	if (ft->p2p_appID != 0) {
		if (fcrt)
			p2p_sendFeedStart(ft);
		p2p_sendStatus(ft, fcrt ? 200 : 603);
	}
	else msnftp_sendAcceptReject(ft, fcrt);

	ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, ft, 0);
}

HANDLE __cdecl CMsnProto::FileAllow(MCONTACT, HANDLE hTransfer, const PROTOCHAR* szPath)
{
	filetransfer* ft = (filetransfer*)hTransfer;

	if (!msnLoggedIn || !p2p_sessionRegistered(ft))
		return 0;

	if ((ft->std.tszWorkingDir = mir_tstrdup(szPath)) == NULL) {
		TCHAR szCurrDir[MAX_PATH];
		GetCurrentDirectory(SIZEOF(szCurrDir), szCurrDir);
		ft->std.tszWorkingDir = mir_tstrdup(szCurrDir);
	}
	else {
		size_t len = _tcslen(ft->std.tszWorkingDir) - 1;
		if (ft->std.tszWorkingDir[len] == '\\')
			ft->std.tszWorkingDir[len] = 0;
	}

	ForkThread(&CMsnProto::MsnFileAckThread, ft);

	return ft;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnFileCancel - cancels the active file transfer

int __cdecl CMsnProto::FileCancel(MCONTACT, HANDLE hTransfer)
{
	filetransfer* ft = (filetransfer*)hTransfer;

	if (!msnLoggedIn || !p2p_sessionRegistered(ft))
		return 0;

	if (!(ft->std.flags & PFTS_SENDING) && ft->fileId == -1) {
		if (ft->p2p_appID != 0)
			p2p_sendStatus(ft, 603);
		else
			msnftp_sendAcceptReject(ft, false);
	}
	else {
		ft->bCanceled = true;
		if (ft->p2p_appID != 0) {
			p2p_sendCancel(ft);
			if (!(ft->std.flags & PFTS_SENDING) && ft->p2p_isV2)
				p2p_sessionComplete(ft);
		}
	}

	ft->std.ptszFiles = NULL;
	ft->std.totalFiles = 0;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnFileDeny - rejects the file transfer request

int __cdecl CMsnProto::FileDeny(MCONTACT, HANDLE hTransfer, const PROTOCHAR* /*szReason*/)
{
	filetransfer* ft = (filetransfer*)hTransfer;

	if (!msnLoggedIn || !p2p_sessionRegistered(ft))
		return 1;

	if (!(ft->std.flags & PFTS_SENDING) && ft->fileId == -1) {
		if (ft->p2p_appID != 0)
			p2p_sendStatus(ft, 603);
		else
			msnftp_sendAcceptReject(ft, false);
	}
	else {
		ft->bCanceled = true;
		if (ft->p2p_appID != 0)
			p2p_sendCancel(ft);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnFileResume - renames a file

int __cdecl CMsnProto::FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename)
{
	filetransfer* ft = (filetransfer*)hTransfer;

	if (!msnLoggedIn || !p2p_sessionRegistered(ft))
		return 1;

	switch (*action) {
	case FILERESUME_SKIP:
		if (ft->p2p_appID != 0)
			p2p_sendStatus(ft, 603);
		else
			msnftp_sendAcceptReject(ft, false);
		break;

	case FILERESUME_RENAME:
		replaceStrT(ft->std.tszCurrentFile, *szFilename);

	default:
		bool fcrt = ft->create() != -1;
		if (ft->p2p_appID != 0) {
			if (fcrt)
				p2p_sendFeedStart(ft);

			p2p_sendStatus(ft, fcrt ? 200 : 603);
		}
		else
			msnftp_sendAcceptReject(ft, fcrt);

		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, ft, 0);
		break;
	}

	return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// MsnGetAwayMsg - reads the current status message for a user

typedef struct AwayMsgInfo_tag
{
	INT_PTR id;
	MCONTACT hContact;
} AwayMsgInfo;

void __cdecl CMsnProto::MsnGetAwayMsgThread(void* arg)
{
	Sleep(150);

	AwayMsgInfo *inf = (AwayMsgInfo*)arg;
	DBVARIANT dbv;
	if (!db_get_ts(inf->hContact, "CList", "StatusMsg", &dbv)) {
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

/////////////////////////////////////////////////////////////////////////////////////////
// MsnGetCaps - obtain the protocol capabilities

DWORD_PTR __cdecl CMsnProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_SERVERCLIST | PF1_AUTHREQ | PF1_BASICSEARCH |
			PF1_ADDSEARCHRES | PF1_CHAT | PF1_CONTACT | 
			/*PF1_FILESEND | PF1_FILERECV | */PF1_URLRECV | PF1_VISLIST | PF1_MODEMSG;

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

/////////////////////////////////////////////////////////////////////////////////////////
// MsnRecvMessage - creates a database event from the message been received

int __cdecl CMsnProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT* pre)
{
	char tEmail[MSN_MAX_EMAIL_LEN];
	if (db_get_static(hContact, m_szModuleName, "wlid", tEmail, sizeof(tEmail)))
	    db_get_static(hContact, m_szModuleName, "e-mail", tEmail, sizeof(tEmail));

	if (Lists_IsInList(LIST_FL, tEmail))
		db_unset(hContact, "CList", "Hidden");

	return Proto_RecvMessage(hContact, pre);
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnRecvContacts - creates a database event from the contacts received

int CMsnProto::RecvContacts(MCONTACT hContact, PROTORECVEVENT* pre)
{
	PROTOSEARCHRESULT **isrList = (PROTOSEARCHRESULT**)pre->szMessage;
	DBEVENTINFO dbei = { sizeof(dbei) };
	BYTE *pCurBlob;
	int i;

	for (i = 0; i < pre->lParam; i++)
		dbei.cbBlob += int(mir_tstrlen(isrList[i]->nick) + 2 + mir_tstrlen(isrList[i]->id));
	dbei.pBlob = (PBYTE)_alloca(dbei.cbBlob);
	for (i = 0, pCurBlob = dbei.pBlob; i < pre->lParam; i++) {
		mir_strcpy((char*)pCurBlob, _T2A(isrList[i]->nick));
		pCurBlob += mir_strlen((char*)pCurBlob) + 1;
		mir_strcpy((char*)pCurBlob, _T2A(isrList[i]->id));
		pCurBlob += mir_strlen((char*)pCurBlob) + 1;
	}

	dbei.szModule = m_szModuleName;
	dbei.timestamp = pre->timestamp;
	dbei.flags = (pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0;
	dbei.eventType = EVENTTYPE_CONTACTS;
	db_event_add(hContact, &dbei);
	return 0;
}


#ifdef OBSOLETE
/////////////////////////////////////////////////////////////////////////////////////////
// MsnSendFile - initiates a file transfer

HANDLE __cdecl CMsnProto::SendFile(MCONTACT hContact, const PROTOCHAR*, PROTOCHAR** ppszFiles)
{
	if (!msnLoggedIn)
		return 0;

	if (getWord(hContact, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
		return 0;

	MsnContact *cont = Lists_Get(hContact);

	if (!cont || _stricmp(cont->email, MyOptions.szEmail) == 0) return 0;

	if ((cont->cap1 & 0xf0000000) == 0 && cont->netId != NETID_MSN) return 0;

	filetransfer* sft = new filetransfer(this);
	sft->std.ptszFiles = ppszFiles;
	sft->std.hContact = hContact;
	sft->std.flags |= PFTS_SENDING;

	int count = 0;
	while (ppszFiles[count] != NULL) {
		struct _stati64 statbuf;
		if (_tstati64(ppszFiles[count++], &statbuf) == 0 && (statbuf.st_mode & _S_IFDIR) == 0) {
			sft->std.totalBytes += statbuf.st_size;
			++sft->std.totalFiles;
		}
	}

	if (sft->openNext() == -1) {
		delete sft;
		return 0;
	}

	if (cont->cap1 & 0xf0000000)
		p2p_invite(MSN_APPID_FILE, sft, NULL);
	else {
		sft->p2p_dest = mir_strdup(cont->email);
		msnftp_invite(sft);
	}

	ProtoBroadcastAck(hContact, ACKTYPE_FILE, ACKRESULT_SENTREQUEST, sft, 0);
	return sft;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// MsnSendMessage - sends the message to a server

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
			const char msgType = MyOptions.SlowSend ? 'A' : 'N';
			bool isOffline;
			ThreadData* thread;
#ifdef OBSOLETE
			thread = MSN_StartSB(tEmail, isOffline);
#else
			/* MSNP24 doesn't have a switchboard anymore */
			thread = NULL; isOffline = true;
#endif

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
#ifdef OBSOLETE
				else
					seq = MsgQueue_Add(tEmail, msgType, msg, 0, 0, rtlFlag);
			}
			else {
				seq = thread->sendMessage(msgType, tEmail, netId, msg, rtlFlag);
				if (!MyOptions.SlowSend)
					ForkThread(&CMsnProto::MsnFakeAck, new TFakeAckParams(hContact, seq, NULL, this));
#endif
			}
		}
		break;
	}

	return seq;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnSendContacts - sends contacts to a certain user
int __cdecl CMsnProto::SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList)
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


/////////////////////////////////////////////////////////////////////////////////////////
// MsnSetAwayMsg - sets the current status message for a user

int __cdecl CMsnProto::SetAwayMsg(int status, const TCHAR* msg)
{
	char** msgptr = GetStatusMsgLoc(status);

	if (msgptr == NULL)
		return 1;

	mir_free(*msgptr);
	char* buf = *msgptr = mir_utf8encodeT(msg);
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

#ifdef OBSOLETE
		m_arSessions.destroy();
		m_arDirect.destroy();
#endif

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

/////////////////////////////////////////////////////////////////////////////////////////
// MsnUserIsTyping - notify another contact that we're typing a message

int __cdecl CMsnProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (!msnLoggedIn) return 0;

	char tEmail[MSN_MAX_EMAIL_LEN];
	if (MSN_IsMeByContact(hContact, tEmail)) return 0;

	bool typing = type == PROTOTYPE_SELFTYPING_ON;

	int netId = Lists_GetNetId(tEmail);
#ifdef OBSOLETE
	switch (netId) {
	case NETID_UNKNOWN:
	case NETID_MSN:
	case NETID_LCS:
		bool isOffline;
		{
			ThreadData* thread = MSN_StartSB(tEmail, isOffline);
			if (thread == NULL) {
				if (isOffline)
					return 0;
				MsgQueue_Add(tEmail, 2571, NULL, 0, NULL, typing);
			}
			else MSN_StartStopTyping(thread, typing);
		}
		break;

	case NETID_YAHOO:
		if (typing) MSN_SendTyping(msnNsThread, tEmail, netId);
		break;

	default:
		break;
	}
#endif
	if (getWord(hContact, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
		MSN_SendTyping(msnNsThread, tEmail, netId, typing);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
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
		mir_snprintf(szDbsettings, SIZEOF(szDbsettings), "%s_HTTPS", m_szModuleName);
		CallService(MS_DB_MODULE_DELETE, 0, (LPARAM)szDbsettings);
		break;

	case EV_PROTO_ONRENAME:
		if (mainMenuRoot) {
			CLISTMENUITEM clmi = { sizeof(clmi) };
			clmi.flags = CMIM_NAME | CMIF_TCHAR;
			clmi.ptszName = m_tszUserName;
			Menu_ModifyItem(mainMenuRoot, &clmi);
		}
		break;

	case EV_PROTO_ONCONTACTDELETED:
		return OnContactDeleted(wParam, lParam);

	case EV_PROTO_DBSETTINGSCHANGED:
		return OnDbSettingChanged(wParam, lParam);
	}
	return 1;
}

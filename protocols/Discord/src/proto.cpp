/*
Copyright © 2016-22 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

static int compareMessages(const COwnMessage *p1, const COwnMessage *p2)
{
	return compareInt64(p1->nonce, p2->nonce);
}

static int compareRequests(const AsyncHttpRequest *p1, const AsyncHttpRequest *p2)
{
	return p1->m_iReqNum - p2->m_iReqNum;
}

int compareUsers(const CDiscordUser *p1, const CDiscordUser *p2)
{
	return compareInt64(p1->id, p2->id);
}

static int compareGuilds(const CDiscordGuild *p1, const CDiscordGuild *p2)
{
	return compareInt64(p1->m_id, p2->m_id);
}

static int compareCalls(const CDiscordVoiceCall *p1, const CDiscordVoiceCall *p2)
{
	return compareInt64(p1->channelId, p2->channelId);
}

CDiscordProto::CDiscordProto(const char *proto_name, const wchar_t *username) :
	PROTO<CDiscordProto>(proto_name, username),
	m_impl(*this),
	m_arHttpQueue(10, compareRequests),
	m_evRequestsQueue(CreateEvent(nullptr, FALSE, FALSE, nullptr)),
	arUsers(10, compareUsers),
	arGuilds(1, compareGuilds),
	arMarkReadQueue(1, compareUsers),
	arOwnMessages(1, compareMessages),
	arVoiceCalls(1, compareCalls),

	m_wszEmail(this, "Email", L""),
	m_wszDefaultGroup(this, "GroupName", DB_KEYVAL_GROUP),
	m_bSyncMarkRead(this, "SendMarkRead", true),
	m_bUseGroupchats(this, "UseGroupChats", true),
	m_bHideGroupchats(this, "HideChats", true),
	m_bUseGuildGroups(this, "UseGuildGroups", false),
	m_bSyncDeleteMsgs(this, "DeleteServerMsgs", true),
	m_bSyncDeleteUsers(this, "DeleteServerUsers", true)
{
	// Hidden setting!
	m_szApiUrl = getMStringA("ApiUrl", "https://discord.com/api/v9");

	// Services
	CreateProtoService(PS_GETAVATARINFO, &CDiscordProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CDiscordProto::GetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CDiscordProto::GetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CDiscordProto::SetMyAvatar);

	CreateProtoService(PS_VOICE_CALL, &CDiscordProto::VoiceCallCreate);
	CreateProtoService(PS_VOICE_DROPCALL, &CDiscordProto::VoiceCallCancel);
	CreateProtoService(PS_VOICE_ANSWERCALL, &CDiscordProto::VoiceCallAnswer);
	CreateProtoService(PS_VOICE_CALL_CONTACT_VALID, &CDiscordProto::VoiceCanCall);

	CreateProtoService(PS_MENU_REQAUTH, &CDiscordProto::RequestFriendship);

	CreateProtoService(PS_MENU_LOADHISTORY, &CDiscordProto::OnMenuLoadHistory);
	CreateProtoService(PS_EMPTY_SRV_HISTORY, &CDiscordProto::SvcEmptyServerHistory);

	CreateProtoService(PS_LEAVECHAT, &CDiscordProto::SvcLeaveChat);

	CreateProtoService(PS_OFFLINEFILE, &CDiscordProto::SvcOfflineFile);

	CreateProtoService(PS_VOICE_CAPS, &CDiscordProto::VoiceCaps);

	// Events
	HookProtoEvent(ME_OPT_INITIALISE, &CDiscordProto::OnOptionsInit);
	HookProtoEvent(ME_PROTO_ACCLISTCHANGED, &CDiscordProto::OnAccountChanged);
	
	HookProtoEvent(PE_VOICE_CALL_STATE, &CDiscordProto::OnVoiceState);

	// avatars
	CreateDirectoryTreeW(GetAvatarPath());

	// database
	db_set_resident(m_szModuleName, "XStatusMsg");

	// Groupchat initialization
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR | GC_DATABASE;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	// Network initialization
	CMStringW descr;
	NETLIBUSER nlu = {};

	nlu.szSettingsModule = m_szModuleName;
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	descr.Format(TranslateT("%s server connection"), m_tszUserName);
	nlu.szDescriptiveName.w = descr.GetBuffer();
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	CMStringA module(FORMAT, "%s.Gateway", m_szModuleName);
	nlu.szSettingsModule = module.GetBuffer();
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_UNICODE;
	descr.Format(TranslateT("%s gateway connection"), m_tszUserName);
	nlu.szDescriptiveName.w = descr.GetBuffer();
	m_hGatewayNetlibUser = Netlib_RegisterUser(&nlu);
}

CDiscordProto::~CDiscordProto()
{
	debugLogA("CDiscordProto::~CDiscordProto");

	for (auto &msg : m_wszStatusMsg)
		mir_free(msg);

	arUsers.destroy();

	m_arHttpQueue.destroy();
	::CloseHandle(m_evRequestsQueue);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::OnModulesLoaded()
{
	// Clist
	Clist_GroupCreate(0, m_wszDefaultGroup);

	HookProtoEvent(ME_GC_EVENT, &CDiscordProto::GroupchatEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CDiscordProto::GroupchatMenuHook);

	InitMenus();
	InitVoip(true);

	// Voice support
	if (g_plugin.bVoiceService) {
		VOICE_MODULE voice = {};
		voice.cbSize = sizeof(voice);
		voice.name = m_szModuleName;
		voice.description = TranslateT("Discord voice call");
		voice.icon = m_hProtoIcon;
		voice.flags = VOICE_CAPS_CALL_CONTACT | VOICE_CAPS_VOICE;
		CallService(MS_VOICESERVICE_REGISTER, (WPARAM)&voice, 0);
	}
}

void CDiscordProto::OnShutdown()
{
	debugLogA("CDiscordProto::OnPreShutdown");

	m_bTerminated = true;
	SetEvent(m_evRequestsQueue);

	InitVoip(false);

	for (auto &it : arGuilds)
		it->SaveToFile();

	if (m_ws)
		m_ws->terminate();

	if (g_plugin.bVoiceService)
		CallService(MS_VOICESERVICE_UNREGISTER, (WPARAM)m_szModuleName, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_MODEMSG | PF1_MODEMSGRECV | PF1_SERVERCLIST | PF1_BASICSEARCH | PF1_EXTSEARCH | PF1_ADDSEARCHRES | PF1_FILESEND;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND | PF2_INVISIBLE;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND | PF2_INVISIBLE;
	case PFLAGNUM_4:
		return PF4_FORCEAUTH | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_SUPPORTIDLE | PF4_AVATARS | 
			PF4_IMSENDOFFLINE | PF4_SERVERMSGID | PF4_OFFLINEFILES | PF4_GROUPCHATFILES | PF4_REPLY;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("User ID");
	}
	return 0;
}

int CDiscordProto::SetStatus(int iNewStatus)
{
	debugLogA("CDiscordProto::SetStatus iNewStatus = %d, m_iStatus = %d, m_iDesiredStatus = %d m_hWorkerThread = %p", iNewStatus, m_iStatus, m_iDesiredStatus, m_hWorkerThread);

	if (iNewStatus == m_iStatus)
		return 0;

	m_iDesiredStatus = iNewStatus;
	int iOldStatus = m_iStatus;

	// go offline
	if (iNewStatus == ID_STATUS_OFFLINE) {
		if (m_bOnline)
			SetServerStatus(ID_STATUS_OFFLINE);
		
		ShutdownSession();
		
		m_iStatus = m_iDesiredStatus;
		setAllContactStatuses(ID_STATUS_OFFLINE, false);

		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	}
	// not logged in? come on
	else if (m_hWorkerThread == nullptr && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		ForkThread(&CDiscordProto::ServerThread);
	}
	else if (m_bOnline) {
		debugLogA("setting server online status to %d", iNewStatus);
		SetServerStatus(iNewStatus);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// Authorization

int CDiscordProto::AuthRequest(MCONTACT hContact, const wchar_t *)
{
	ptrW wszUsername(getWStringA(hContact, DB_KEY_NICK));
	int iDiscriminator(getDword(hContact, DB_KEY_DISCR, -1));
	if (wszUsername == nullptr || iDiscriminator == -1)
		return 1; // error

	JSONNode root; root << WCHAR_PARAM("username", wszUsername);
	if (iDiscriminator )
		root << INT_PARAM("discriminator", iDiscriminator);
	Push(new AsyncHttpRequest(this, REQUEST_POST, "/users/@me/relationships", nullptr, &root));
	return 0;
}

int CDiscordProto::AuthRecv(MCONTACT, DB::EventInfo &dbei)
{
	return Proto_AuthRecv(m_szModuleName, dbei);
}

int CDiscordProto::Authorize(MEVENT hDbEvent)
{
	DB::EventInfo dbei;
	dbei.cbBlob = -1;
	if (db_event_get(hDbEvent, &dbei)) return 1;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) return 1;
	if (mir_strcmp(dbei.szModule, m_szModuleName)) return 1;

	MCONTACT hContact = DbGetAuthEventContact(&dbei);
	AddFriend(getId(hContact, DB_KEY_ID));
	return 0;
}

int CDiscordProto::AuthDeny(MEVENT hDbEvent, const wchar_t *)
{
	DB::EventInfo dbei;
	dbei.cbBlob = -1;
	if (db_event_get(hDbEvent, &dbei)) return 1;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) return 1;
	if (mir_strcmp(dbei.szModule, m_szModuleName)) return 1;

	MCONTACT hContact = DbGetAuthEventContact(&dbei);
	RemoveFriend(getId(hContact, DB_KEY_ID));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK AdvancedSearchDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetFocus(GetDlgItem(hwndDlg, IDC_NICK));
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == EN_SETFOCUS)
			PostMessage(GetParent(hwndDlg), WM_COMMAND, MAKEWPARAM(0, EN_SETFOCUS), (LPARAM)hwndDlg);
	}
	return FALSE;
}

HWND CDiscordProto::CreateExtendedSearchUI(HWND hwndParent)
{
	if (hwndParent)
		return CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_EXTSEARCH), hwndParent, AdvancedSearchDlgProc, 0);

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////

MCONTACT CDiscordProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	if (mir_wstrlen(psr->nick.w) == 0)
		return 0;

	wchar_t *p = wcschr(psr->nick.w, '#');
	if (p == nullptr)
		return 0;

	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);
	if (flags & PALF_TEMPORARY)
		Contact::RemoveFromList(hContact);
	
	*p = 0;
	CDiscordUser *pUser = new CDiscordUser(0);
	pUser->hContact = hContact;
	pUser->wszUsername = psr->nick.w;
	pUser->iDiscriminator = _wtoi(p + 1);
	*p = '#';

	if (mir_wstrlen(psr->id.w)) {
		pUser->id = _wtoi64(psr->id.w);
		setId(hContact, DB_KEY_ID, pUser->id);
	}

	Clist_SetGroup(hContact, m_wszDefaultGroup);
	setWString(hContact, DB_KEY_NICK, pUser->wszUsername);
	setDword(hContact, DB_KEY_DISCR, pUser->iDiscriminator);
	arUsers.insert(pUser);

	return hContact;
}

MCONTACT CDiscordProto::AddToListByEvent(int flags, int, MEVENT hDbEvent)
{
	DB::EventInfo dbei;
	dbei.cbBlob = -1;
	if (db_event_get(hDbEvent, &dbei))
		return 0;
	if (mir_strcmp(dbei.szModule, m_szModuleName))
		return 0;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return 0;

	DB::AUTH_BLOB blob(dbei.pBlob);
	if (flags & PALF_TEMPORARY)
		Contact::RemoveFromList(blob.get_contact());
	else
		Contact::PutOnList(blob.get_contact());
	return blob.get_contact();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::SearchThread(void *param)
{
	Sleep(100);

	PROTOSEARCHRESULT psr = { 0 };
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_UNICODE;
	psr.nick.w = (wchar_t *)param;
	psr.firstName.w = L"";
	psr.lastName.w = L"";
	psr.id.w = L"";
	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, this, (LPARAM)&psr);

	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, this, 0);
	mir_free(param);
}

HANDLE CDiscordProto::SearchAdvanced(HWND hwndDlg)
{
	if (!m_bOnline || !IsWindow(hwndDlg))
		return nullptr;

	wchar_t wszNick[200];
	GetDlgItemTextW(hwndDlg, IDC_NICK, wszNick, _countof(wszNick));
	if (wszNick[0] == 0) // empty string? reject
		return nullptr;

	wchar_t *p = wcschr(wszNick, '#');
	if (p == nullptr) // wrong user id
		return nullptr;

	ForkThread(&CDiscordProto::SearchThread, mir_wstrdup(wszNick));
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Basic search - by SnowFlake

void CDiscordProto::OnReceiveUserinfo(MHttpResponse *pReply, AsyncHttpRequest *)
{
	JsonReply root(pReply);
	if (!root) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_FAILED, this);
		return;
	}

	ForkThread(&CDiscordProto::SearchThread, getNick(root.data()).Detach());
}

HANDLE CDiscordProto::SearchBasic(const wchar_t *wszId)
{
	if (!m_bOnline)
		return nullptr;

	CMStringA szUrl = "/users/";
	szUrl.AppendFormat(ptrA(mir_utf8encodeW(wszId)));
	Push(new AsyncHttpRequest(this, REQUEST_GET, szUrl, &CDiscordProto::OnReceiveUserinfo));
	return (HANDLE)1; // Success
}

////////////////////////////////////////////////////////////////////////////////////////
// SendMsg

void CDiscordProto::OnSendMsg(MHttpResponse *pReply, AsyncHttpRequest *pReq)
{
	JsonReply root(pReply);
	if (!root) {
		int iReqNum = -1;
		for (auto &it : arOwnMessages)
			if (it->reqId == pReq->m_iReqNum) {
				iReqNum = it->reqId;
				arOwnMessages.removeItem(&it);
				break;
			}

		if (iReqNum != -1) {
			CMStringW wszErrorMsg(root.data()["message"].as_mstring());
			if (wszErrorMsg.IsEmpty())
				wszErrorMsg = TranslateT("Message send failed");
			ProtoBroadcastAck(pReq->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)iReqNum, (LPARAM)wszErrorMsg.c_str());
		}
	}
}

int CDiscordProto::SendMsg(MCONTACT hContact, MEVENT hReplyEvent, const char *pszSrc)
{
	if (!m_bOnline) 
		return -1;

	ptrW wszText(mir_utf8decodeW(pszSrc));
	if (wszText == nullptr)
		return 0;

	CDiscordUser *pUser = FindUser(getId(hContact, DB_KEY_ID));
	if (pUser == nullptr || pUser->id == 0)
		return 0;

	// no channel - we need to create one
	if (pUser->channelId == 0) {
		JSONNode list(JSON_ARRAY); list.set_name("recipients"); list << SINT64_PARAM("", pUser->id);
		JSONNode body; body << list;
		CMStringA szUrl(FORMAT, "/users/%lld/channels", m_ownId);

		// theoretically we get the same data from the gateway thread, but there could be a delay
		// so we bind data analysis to the http packet reply
		mir_cslock lck(m_csHttpQueue);
		ExecuteRequest(new AsyncHttpRequest(this, REQUEST_POST, szUrl, &CDiscordProto::OnReceiveCreateChannel, &body));
		if (pUser->channelId == 0)
			return 0;
	}

	// we generate a random 64-bit integer and pass it to the server
	// to distinguish our own messages from these generated by another clients
	SnowFlake nonce; Utils_GetRandom(&nonce, sizeof(nonce)); nonce = abs(nonce);
	JSONNode body; body << WCHAR_PARAM("content", wszText) << SINT64_PARAM("nonce", nonce);

	if (hReplyEvent) {
		SnowFlake iReplyId = 0;
		DB::EventInfo dbei(hReplyEvent, false);
		if (dbei && dbei.szId) {
			iReplyId = _atoi64(dbei.szId);

			JSONNode reply; reply.set_name("message_reference");
			reply << INT64_PARAM("channel_id", pUser->channelId) << INT64_PARAM("message_id", iReplyId);
			body << reply;
		}
	}

	CMStringA szUrl(FORMAT, "/channels/%lld/messages", pUser->channelId);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, szUrl, &CDiscordProto::OnSendMsg, &body);
	pReq->hContact = hContact;
	arOwnMessages.insert(new COwnMessage(nonce, pReq->m_iReqNum));
	Push(pReq);
	return pReq->m_iReqNum;
}

/////////////////////////////////////////////////////////////////////////////////////////

void __cdecl CDiscordProto::GetAwayMsgThread(void *param)
{
	Thread_SetName("Jabber: GetAwayMsgThread");

	auto *pUser = (CDiscordUser *)param;
	if (pUser == nullptr)
		return;

	if (pUser->wszTopic.IsEmpty())
		ProtoBroadcastAck(pUser->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, 0);
	else
		ProtoBroadcastAck(pUser->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)pUser->wszTopic.c_str());
}

HANDLE CDiscordProto::GetAwayMsg(MCONTACT hContact)
{
	ForkThread(&CDiscordProto::GetAwayMsgThread, FindUser(getId(hContact, DB_KEY_ID)));
	return (HANDLE)1;
}

int CDiscordProto::SetAwayMsg(int iStatus, const wchar_t *msg)
{
	if (iStatus < ID_STATUS_MIN || iStatus > ID_STATUS_MAX)
		return 0;

	wchar_t *&pwszMessage = m_wszStatusMsg[iStatus - ID_STATUS_MIN];
	if (!mir_wstrcmp(msg, pwszMessage))
		return 0;

	replaceStrW(pwszMessage, msg);

	if (m_bOnline) {
		JSONNode status; status.set_name("custom_status"); status << WCHAR_PARAM("text", (msg) ? msg : L"");
		JSONNode root; root << status;
		Push(new AsyncHttpRequest(this, REQUEST_PATCH, "/users/@me/settings", nullptr, &root));
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDiscordProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (type == PROTOTYPE_SELFTYPING_ON) {
		CMStringA szUrl(FORMAT, "/channels/%lld/typing", getId(hContact, DB_KEY_CHANNELID));
		Push(new AsyncHttpRequest(this, REQUEST_POST, szUrl, nullptr));
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::OnReceiveMarkRead(MHttpResponse *pReply, AsyncHttpRequest *)
{
	JsonReply root(pReply);
	if (root)
		SaveToken(root.data());
}

void CDiscordProto::SendMarkRead()
{
	mir_cslock lck(csMarkReadQueue);
	while (arMarkReadQueue.getCount()) {
		CDiscordUser *pUser = arMarkReadQueue[0];
		JSONNode payload; payload << CHAR_PARAM("token", m_szTempToken);
		CMStringA szUrl(FORMAT, "/channels/%lld/messages/%lld/ack", pUser->channelId, pUser->lastMsgId);
		auto *pReq = new AsyncHttpRequest(this, REQUEST_POST, szUrl, &CDiscordProto::OnReceiveMarkRead, &payload);
		Push(pReq);
		arMarkReadQueue.remove(0);
	}
}

void CDiscordProto::OnMarkRead(MCONTACT hContact, MEVENT)
{
	if (m_bOnline && m_bSyncMarkRead) {
		m_impl.m_markRead.Start(200);

		CDiscordUser *pUser = FindUser(getId(hContact, DB_KEY_ID));
		if (pUser != nullptr) {
			mir_cslock lck(csMarkReadQueue);
			if (arMarkReadQueue.indexOf(pUser) == -1)
				arMarkReadQueue.insert(pUser);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDiscordProto::OnAccountChanged(WPARAM iAction, LPARAM lParam)
{
	if (iAction == PRAC_ADDED) {
		PROTOACCOUNT *pa = (PROTOACCOUNT*)lParam;
		if (pa && pa->ppro == this) {
			m_bUseGroupchats = false;
			m_bUseGuildGroups = true;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::OnCacheInit()
{
	std::vector<MCONTACT> lostIds;

	// Fill users list
	for (auto &hContact : AccContacts()) {
		m_bCacheInited = true;

		CDiscordUser *pNew = new CDiscordUser(getId(hContact, DB_KEY_ID));
		pNew->hContact = hContact;
		pNew->lastMsgId = getId(hContact, DB_KEY_LASTMSGID);
		pNew->wszUsername = ptrW(getWStringA(hContact, DB_KEY_NICK));
		pNew->iDiscriminator = getDword(hContact, DB_KEY_DISCR);

		// set EnableSync = 1 by default for all existing guilds
		switch (getByte(hContact, "ChatRoom")) {
		case 2: // guild
			delSetting(hContact, DB_KEY_CHANNELID);
			surelyGetBool(hContact, DB_KEY_ENABLE_HIST);
			surelyGetBool(hContact, DB_KEY_ENABLE_SYNC);
			break;

		case 1: // group chat
			pNew->channelId = getId(hContact, DB_KEY_CHANNELID);
			if (!pNew->channelId) {
				lostIds.push_back(hContact);
				delete pNew;
				continue;
			}
			break;

		default:
			pNew->channelId = getId(hContact, DB_KEY_CHANNELID);
			break;
		}
		arUsers.insert(pNew);
	}

	for (auto &hContact : lostIds)
		db_delete_contact(hContact);
}

bool CDiscordProto::OnContactDeleted(MCONTACT hContact, uint32_t flags)
{
	if (flags & CDF_DEL_CONTACT) {
		CDiscordUser *pUser = FindUser(getId(hContact, DB_KEY_ID));
		if (pUser == nullptr || !m_bOnline)
			return false;

		if (pUser->channelId)
			Push(new AsyncHttpRequest(this, REQUEST_DELETE, CMStringA(FORMAT, "/channels/%lld", pUser->channelId), nullptr));

		if (pUser->id)
			RemoveFriend(pUser->id);
	}
	return true;
}

void CDiscordProto::OnEventDeleted(MCONTACT hContact, MEVENT hEvent, int flags)
{
	// the command arrived from the server, don't send it back then
	if (!(flags & CDF_DEL_HISTORY))
		return;

	DB::EventInfo dbei(hEvent, false);
	if (dbei && dbei.szId) {
		CMStringA szUrl(FORMAT, "/channels/%lld/messages/%s", getId(hContact, DB_KEY_ID), dbei.szId);
		Push(new AsyncHttpRequest(this, REQUEST_DELETE, szUrl, 0));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::CDiscordProto::SvcEmptyServerHistory(WPARAM, LPARAM)
{
	return 0;
}

INT_PTR CDiscordProto::RequestFriendship(WPARAM hContact, LPARAM)
{
	AuthRequest(hContact, 0);
	return 0;
}

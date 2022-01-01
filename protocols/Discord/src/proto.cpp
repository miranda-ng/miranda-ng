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
	return compareInt64(p1->id, p2->id);
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
	arVoiceCalls(1),

	m_wszEmail(this, "Email", L""),
	m_wszDefaultGroup(this, "GroupName", DB_KEYVAL_GROUP),
	m_bUseGroupchats(this, "UseGroupChats", true),
	m_bHideGroupchats(this, "HideChats", true),
	m_bUseGuildGroups(this, "UseGuildGroups", false),
	m_bSyncDeleteMsgs(this, "DeleteServerMsgs", true)
{
	// Services
	CreateProtoService(PS_CREATEACCMGRUI, &CDiscordProto::SvcCreateAccMgrUI);

	CreateProtoService(PS_GETAVATARINFO, &CDiscordProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CDiscordProto::GetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CDiscordProto::GetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CDiscordProto::SetMyAvatar);

	CreateProtoService(PS_MENU_REQAUTH, &CDiscordProto::RequestFriendship);
	CreateProtoService(PS_MENU_LOADHISTORY, &CDiscordProto::OnMenuLoadHistory);

	CreateProtoService(PS_VOICE_CAPS, &CDiscordProto::VoiceCaps);

	// Events
	HookProtoEvent(ME_OPT_INITIALISE, &CDiscordProto::OnOptionsInit);
	HookProtoEvent(ME_DB_EVENT_MARKED_READ, &CDiscordProto::OnDbEventRead);
	HookProtoEvent(ME_PROTO_ACCLISTCHANGED, &CDiscordProto::OnAccountChanged);
	
	HookProtoEvent(PE_VOICE_CALL_STATE, &CDiscordProto::OnVoiceState);

	// database
	db_set_resident(m_szModuleName, "XStatusMsg");

	// custom events
	DBEVENTTYPEDESCR dbEventType = {};
	dbEventType.module = m_szModuleName;
	dbEventType.flags = DETF_HISTORY | DETF_MSGWINDOW;

	dbEventType.eventType = EVENT_INCOMING_CALL;
	dbEventType.descr = Translate("Incoming call");
	dbEventType.eventIcon = g_plugin.getIconHandle(IDI_VOICE_CALL);
	DbEvent_RegisterType(&dbEventType);

	dbEventType.eventType = EVENT_CALL_FINISHED;
	dbEventType.descr = Translate("Call ended");
	dbEventType.eventIcon = g_plugin.getIconHandle(IDI_VOICE_ENDED);
	DbEvent_RegisterType(&dbEventType);

	// Groupchat initialization
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;
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
	nlu.flags = NUF_OUTGOING | NUF_UNICODE;
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
	std::vector<MCONTACT> lostIds;

	// Fill users list
	for (auto &hContact : AccContacts()) {
		CDiscordUser *pNew = new CDiscordUser(getId(hContact, DB_KEY_ID));
		pNew->hContact = hContact;
		pNew->lastMsgId = getId(hContact, DB_KEY_LASTMSGID);
		pNew->wszUsername = ptrW(getWStringA(hContact, DB_KEY_NICK));
		pNew->iDiscriminator = getDword(hContact, DB_KEY_DISCR);

		// set EnableSync = 1 by default for all existing guilds
		switch (getByte(hContact, "ChatRoom")) {
		case 2: // guild
			delSetting(hContact, DB_KEY_CHANNELID);
			if (getDword(hContact, "EnableSync", -1) == -1)
				setDword(hContact, "EnableSync", 1);
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

	for (auto &hContact: lostIds)
		db_delete_contact(hContact);

	// Clist
	Clist_GroupCreate(0, m_wszDefaultGroup);

	HookProtoEvent(ME_GC_EVENT, &CDiscordProto::GroupchatEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CDiscordProto::GroupchatMenuHook);

	InitMenus();

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

	for (auto &it : arGuilds)
		it->SaveToFile();

	if (m_hGatewayConnection)
		Netlib_Shutdown(m_hGatewayConnection);

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
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_HEAVYDND | PF2_INVISIBLE;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_LONGAWAY | PF2_HEAVYDND | PF2_INVISIBLE;
	case PFLAGNUM_4:
		return PF4_FORCEAUTH | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_SUPPORTIDLE | PF4_AVATARS | PF4_IMSENDOFFLINE | PF4_SERVERMSGID | PF4_OFFLINEFILES;
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
		if (m_bOnline) {
			SetServerStatus(ID_STATUS_OFFLINE);
			ShutdownSession();
		}
		m_iStatus = m_iDesiredStatus;
		setAllContactStatuses(ID_STATUS_OFFLINE, false);

		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	}
	// not logged in? come on
	else if (m_hWorkerThread == nullptr && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		m_hWorkerThread = ForkThreadEx(&CDiscordProto::ServerThread, nullptr, nullptr);
	}
	else if (m_bOnline) {
		debugLogA("setting server online status to %d", iNewStatus);
		SetServerStatus(iNewStatus);
	}

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

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::SearchThread(void *param)
{
	Sleep(100);

	PROTOSEARCHRESULT psr = { 0 };
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_UNICODE;
	psr.nick.w = (wchar_t*)param;
	psr.firstName.w = L"";
	psr.lastName.w = L"";
	psr.id.w = L"";
	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);

	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
	mir_free(param);
}

HWND CDiscordProto::SearchAdvanced(HWND hwndDlg)
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
	return (HWND)1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Basic search - by SnowFlake

void CDiscordProto::OnReceiveUserinfo(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	JsonReply root(pReply);
	if (!root) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)1);
		return;
	}

	auto &data = root.data();
	CMStringW wszUserId(data["username"].as_mstring() + L"#" + data["discriminator"].as_mstring());
	ForkThread(&CDiscordProto::SearchThread, wszUserId.Detach());
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
// Authorization

int CDiscordProto::AuthRequest(MCONTACT hContact, const wchar_t*)
{
	ptrW wszUsername(getWStringA(hContact, DB_KEY_NICK));
	int iDiscriminator(getDword(hContact, DB_KEY_DISCR, -1));
	if (wszUsername == nullptr || iDiscriminator == -1)
		return 1; // error

	JSONNode root; root << WCHAR_PARAM("username", wszUsername) << INT_PARAM("discriminator", iDiscriminator);
	Push(new AsyncHttpRequest(this, REQUEST_POST, "/users/@me/relationships", nullptr, &root));
	return 0;
}

int CDiscordProto::AuthRecv(MCONTACT, PROTORECVEVENT *pre)
{
	return Proto_AuthRecv(m_szModuleName, pre);
}

int CDiscordProto::Authorize(MEVENT hDbEvent)
{
	DB::EventInfo dbei;
	dbei.cbBlob = -1;
	if (db_event_get(hDbEvent, &dbei)) return 1;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) return 1;
	if (mir_strcmp(dbei.szModule, m_szModuleName)) return 1;

	JSONNode root;
	MCONTACT hContact = DbGetAuthEventContact(&dbei);
	CMStringA szUrl(FORMAT, "/users/@me/relationships/%lld", getId(hContact, DB_KEY_ID));
	Push(new AsyncHttpRequest(this, REQUEST_PUT, szUrl, nullptr, &root));
	return 0;
}

int CDiscordProto::AuthDeny(MEVENT hDbEvent, const wchar_t*)
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
		Contact_RemoveFromList(hContact);
	
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
		Contact_RemoveFromList(blob.get_contact());
	else
		Contact_PutOnList(blob.get_contact());
	return blob.get_contact();
}

////////////////////////////////////////////////////////////////////////////////////////
// SendMsg

void CDiscordProto::OnSendMsg(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
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

int CDiscordProto::SendMsg(MCONTACT hContact, int /*flags*/, const char *pszSrc)
{
	if (!m_bOnline) {
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)1, (LPARAM)TranslateT("Protocol is offline or user isn't authorized yet"));
		return 1;
	}

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

void CDiscordProto::OnReceiveMarkRead(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *)
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

int CDiscordProto::OnDbEventRead(WPARAM, LPARAM hDbEvent)
{
	MCONTACT hContact = db_event_getContact(hDbEvent);
	if (!hContact)
		return 0;

	// filter out only events of my protocol
	const char *szProto = Proto_GetBaseAccountName(hContact);
	if (mir_strcmp(szProto, m_szModuleName))
		return 0;

	if (m_bOnline) {
		m_impl.m_markRead.Start(200);

		CDiscordUser *pUser = FindUser(getId(hContact, DB_KEY_ID));
		if (pUser != nullptr) {
			mir_cslock lck(csMarkReadQueue);
			if (arMarkReadQueue.indexOf(pUser) == -1)
				arMarkReadQueue.insert(pUser);
		}
	}
	return 0;
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

void CDiscordProto::OnContactDeleted(MCONTACT hContact)
{
	CDiscordUser *pUser = FindUser(getId(hContact, DB_KEY_ID));
	if (pUser == nullptr || !m_bOnline)
		return;

	if (pUser->channelId)
		Push(new AsyncHttpRequest(this, REQUEST_DELETE, CMStringA(FORMAT, "/channels/%lld", pUser->channelId), nullptr));

	if (pUser->id)
		RemoveFriend(pUser->id);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::RequestFriendship(WPARAM hContact, LPARAM)
{
	AuthRequest(hContact, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct SendFileThreadParam
{
	MCONTACT hContact;
	CMStringW wszDescr, wszFileName;

	SendFileThreadParam(MCONTACT _p1, LPCWSTR _p2, LPCWSTR _p3) :
		hContact(_p1),
		wszFileName(_p2),
		wszDescr(_p3)
	{}
};

void CDiscordProto::SendFileThread(void *param)
{
	SendFileThreadParam *p = (SendFileThreadParam*)param;

	FILE *in = _wfopen(p->wszFileName, L"rb");
	if (in == nullptr) {
		debugLogA("cannot open file %S for reading", p->wszFileName.c_str());
	LBL_Error:
		ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, param);
		delete p;
		return;
	}

	ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, param);

	char szRandom[16], szRandomText[33];
	Utils_GetRandom(szRandom, _countof(szRandom));
	bin2hex(szRandom, _countof(szRandom), szRandomText);
	CMStringA szBoundary(FORMAT, "----Boundary%s", szRandomText);

	CMStringA szUrl(FORMAT, "/channels/%lld/messages", getId(p->hContact, DB_KEY_CHANNELID));
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, szUrl, &CDiscordProto::OnReceiveFile);
	pReq->AddHeader("Content-Type", CMStringA("multipart/form-data; boundary=" + szBoundary));
	pReq->AddHeader("Accept", "*/*");

	szBoundary.Insert(0, "--");

	CMStringA szBody;
	szBody.Append(szBoundary + "\r\n");
	szBody.Append("Content-Disposition: form-data; name=\"content\"\r\n\r\n");
	szBody.Append(ptrA(mir_utf8encodeW(p->wszDescr)));
	szBody.Append("\r\n");

	szBody.Append(szBoundary + "\r\n");
	szBody.Append("Content-Disposition: form-data; name=\"tts\"\r\n\r\nfalse\r\n");

	wchar_t *pFileName = wcsrchr(p->wszFileName.GetBuffer(), '\\');
	if (pFileName != nullptr)
		pFileName++;
	else
		pFileName = p->wszFileName.GetBuffer();

	szBody.Append(szBoundary + "\r\n");
	szBody.AppendFormat("Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n", ptrA(mir_utf8encodeW(pFileName)).get());
	szBody.AppendFormat("Content-Type: %S\r\n", ProtoGetAvatarMimeType(ProtoGetAvatarFileFormat(p->wszFileName)));
	szBody.Append("\r\n");

	size_t cbBytes = filelength(fileno(in));

	szBoundary.Insert(0, "\r\n");
	szBoundary.Append("--\r\n");
	pReq->dataLength = int(szBody.GetLength() + szBoundary.GetLength() + cbBytes);
	pReq->pData = (char*)mir_alloc(pReq->dataLength+1);
	memcpy(pReq->pData, szBody.c_str(), szBody.GetLength());
	size_t cbRead = fread(pReq->pData + szBody.GetLength(), 1, cbBytes, in);
	fclose(in);
	if (cbBytes != cbRead) {
		debugLogA("cannot read file %S: %d bytes read instead of %d", p->wszFileName.c_str(), cbRead, cbBytes);
		delete pReq;
		goto LBL_Error;
	}
	
	memcpy(pReq->pData + szBody.GetLength() + cbBytes, szBoundary, szBoundary.GetLength());
	pReq->pUserInfo = p;
	Push(pReq);

	ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, param);
}

void CDiscordProto::OnReceiveFile(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	SendFileThreadParam *p = (SendFileThreadParam*)pReq->pUserInfo;
	if (pReply->resultCode != 200) {
		ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, p);
		debugLogA("CDiscordProto::SendFile failed: %d", pReply->resultCode);
	}
	else {
		ProtoBroadcastAck(p->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, p);
		debugLogA("CDiscordProto::SendFile succeeded");
	}

	delete p;
}

HANDLE CDiscordProto::SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles)
{
	SnowFlake id = getId(hContact, DB_KEY_CHANNELID);
	if (id == 0)
		return nullptr;

	// we don't wanna block the main thread, right?
	SendFileThreadParam *param = new SendFileThreadParam(hContact, ppszFiles[0], szDescription);
	ForkThread(&CDiscordProto::SendFileThread, param);
	return param;
}

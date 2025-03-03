#include "stdafx.h"

CSteamProto::CSteamProto(const char *protoName, const wchar_t *userName) :
	PROTO<CSteamProto>(protoName, userName),
	m_impl(*this),
	m_arOwnMessages(1, NumericKeySortT),
	m_wszGroupName(this, "DefaultGroup", L"Steam"),
	m_wszDeviceName(this, "DeviceName", L"Miranda NG")
{
	// temporary DB settings
	db_set_resident(m_szModuleName, "XStatusId");
	db_set_resident(m_szModuleName, "XStatusName");
	db_set_resident(m_szModuleName, "XStatusMsg");
	db_set_resident(m_szModuleName, "IdleTS");
	db_set_resident(m_szModuleName, "GameID");
	db_set_resident(m_szModuleName, "ServerIP");

	SetAllContactStatuses(ID_STATUS_OFFLINE);

	// avatar API
	CreateDirectoryTreeW(GetAvatarPath());

	CreateProtoService(PS_GETAVATARINFO, &CSteamProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CSteamProto::GetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CSteamProto::GetMyAvatar);

	// history
	CreateProtoService(PS_MENU_LOADHISTORY, &CSteamProto::SvcLoadServerHistory);
	CreateProtoService(PS_CAN_EMPTY_HISTORY, &CSteamProto::SvcCanEmptyHistory);
	CreateProtoService(PS_EMPTY_SRV_HISTORY, &CSteamProto::SvcEmptyHistory);

	// custom status API
	CreateProtoService(PS_GETCUSTOMSTATUSEX, &CSteamProto::OnGetXStatusEx);
	CreateProtoService(PS_GETCUSTOMSTATUSICON, &CSteamProto::OnGetXStatusIcon);
	CreateProtoService(PS_GETADVANCEDSTATUSICON, &CSteamProto::OnRequestAdvStatusIconIdx);

	// menus
	CreateProtoService(PS_MENU_REQAUTH, &CSteamProto::AuthRequestCommand);
	CreateProtoService(PS_MENU_REVOKEAUTH, &CSteamProto::AuthRevokeCommand);

	// custom db events API
	CreateProtoService(STEAM_DB_GETEVENTTEXT_CHATSTATES, &CSteamProto::OnGetEventTextChatStates);

	// hooks
	HookProtoEvent(ME_OPT_INITIALISE, &CSteamProto::OnOptionsInit);

	// netlib support
	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = m_tszUserName;
	nlu.szSettingsModule = m_szModuleName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	// groupchat initialization
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF | GC_DATABASE | GC_PERSISTENT;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	CreateProtoService(PS_LEAVECHAT, &CSteamProto::SvcLeaveChat);

	HookProtoEvent(ME_GC_EVENT, &CSteamProto::GcEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CSteamProto::GcMenuHook);
}

CSteamProto::~CSteamProto()
{
}

MCONTACT CSteamProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	uint64_t id = _wtoi64(psr->id.w);
	MCONTACT hContact = AddContact(id, psr->nick.w, true);

	SendUserInfoRequest(id);

	return hContact;
}

MCONTACT CSteamProto::AddToListByEvent(int, int, MEVENT hDbEvent)
{
	DB::EventInfo dbei(hDbEvent);
	if (!dbei)
		return 0;
	if (mir_strcmp(dbei.szModule, m_szModuleName))
		return 0;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return 0;

	DB::AUTH_BLOB blob(dbei.pBlob);
	return AddContact(_atoi64(blob.get_email()), Utf2T(blob.get_nick()));
}

int CSteamProto::Authorize(MEVENT hDbEvent)
{
	if (IsOnline() && hDbEvent) {
		MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
		if (hContact == INVALID_CONTACT_ID)
			return 1;

		ptrA sessionId(getStringA("SessionID"));
		char *who = getStringA(hContact, DBKEY_STEAM_ID);

		SendRequest(new ApprovePendingRequest(m_szAccessToken, sessionId, m_iSteamId, who), &CSteamProto::OnPendingApproved, who);
		return 0;
	}

	return 1;
}

int CSteamProto::AuthRecv(MCONTACT hContact, DB::EventInfo &dbei)
{
	// remember to not create this event again, unless authorization status changes again
	setByte(hContact, "AuthAsked", 1);
	return Proto_AuthRecv(m_szModuleName, dbei);
}

int CSteamProto::AuthDeny(MEVENT hDbEvent, const wchar_t*)
{
	if (IsOnline() && hDbEvent) {
		MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
		if (hContact == INVALID_CONTACT_ID)
			return 1;

		ptrA sessionId(getStringA("SessionID"));
		char *who = getStringA(hContact, DBKEY_STEAM_ID);
		SendRequest(new IgnorePendingRequest(m_szAccessToken, sessionId, m_iSteamId, who), &CSteamProto::OnPendingIgnoreded, who);
		return 0;
	}

	return 1;
}

int CSteamProto::AuthRequest(MCONTACT hContact, const wchar_t*)
{
	if (IsOnline() && hContact) {
		UINT hAuth = InterlockedIncrement(&hAuthProcess);
		SendUserAddRequest(GetId(hContact, DBKEY_STEAM_ID));
		return hAuth;
	}

	return 1;
}

INT_PTR CSteamProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_BASICSEARCH | PF1_SEARCHBYNAME | PF1_AUTHREQ | PF1_SERVERCLIST | PF1_ADDSEARCHRES | PF1_MODEMSGRECV;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_HEAVYDND | PF2_FREECHAT | PF2_INVISIBLE;
	case PFLAGNUM_4:
		return PF4_AVATARS | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_FORCEAUTH | PF4_SUPPORTIDLE | PF4_SUPPORTTYPING | PF4_SERVERMSGID;
	case PFLAGNUM_5:
		return PF2_HEAVYDND | PF2_FREECHAT;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT(DBKEY_STEAM_ID);
	default:
		return 0;
	}
}

HANDLE CSteamProto::SearchBasic(const wchar_t* id)
{
	if (!this->IsOnline())
		return nullptr;

	ptrA steamId(mir_u2a(id));
	SendRequest(new GetUserSummariesRequest(m_szAccessToken, steamId), &CSteamProto::OnSearchResults, (HANDLE)STEAM_SEARCH_BYID);

	return (HANDLE)STEAM_SEARCH_BYID;
}

HANDLE CSteamProto::SearchByName(const wchar_t *nick, const wchar_t *firstName, const wchar_t *lastName)
{
	if (!IsOnline())
		return nullptr;

	// Combine all fields to single text
	wchar_t keywordsT[200];
	mir_snwprintf(keywordsT, L"%s %s %s", nick, firstName, lastName);

	ptrA keywords(mir_utf8encodeW(rtrimw(keywordsT)));

	SendRequest(
		new SearchRequest(m_szAccessToken, keywords),
		&CSteamProto::OnSearchByNameStarted,
		(HANDLE)STEAM_SEARCH_BYNAME);

	return (HANDLE)STEAM_SEARCH_BYNAME;
}

int CSteamProto::SendMsg(MCONTACT hContact, MEVENT, const char *message)
{
	if (!IsOnline())
		return -1;

	UINT hMessage = InterlockedIncrement(&hMessageProcess);
	auto *pOwn = new COwnMessage(hContact, hMessage);
	{
		mir_cslock lck(m_csOwnMessages);
		m_arOwnMessages.insert(pOwn);
	}

	SendFriendMessage(EChatEntryType::ChatMsg, GetId(hContact, DBKEY_STEAM_ID), message, pOwn);
	return hMessage;
}

int CSteamProto::SetStatus(int new_status)
{
	// Routing statuses not supported by Steam
	switch (new_status) {
	case ID_STATUS_OFFLINE:
	case ID_STATUS_AWAY:
	case ID_STATUS_NA:
	case ID_STATUS_INVISIBLE:
		break;

	case ID_STATUS_DND:
	case ID_STATUS_OCCUPIED:
		new_status = ID_STATUS_NA;
		break;

	default:
		new_status = ID_STATUS_ONLINE;
		break;
	}

	{
		mir_cslock lock(m_setStatusLock);
		if (new_status == m_iDesiredStatus)
			return 0;
	}

	debugLogA(__FUNCTION__ ": changing status from %i to %i", m_iStatus, new_status);

	int old_status = m_iStatus;
	m_iDesiredStatus = new_status;

	if (new_status == ID_STATUS_OFFLINE) {
		if (!Miranda_IsTerminated())
			SetAllContactStatuses(ID_STATUS_OFFLINE);

		if (IsOnline())
			SendLogout();

		Logout();
	}
	else if (m_ws == nullptr && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		ForkThread(&CSteamProto::ServerThread);
	}
	else if (IsOnline()) {
		m_iStatus = new_status;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		SendPersonaStatus(m_iStatus);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::GetAwayMsgThread(void *arg)
{
	// Maybe not needed, but better to be sure that this won't happen faster than core handling return value of GetAwayMsg()
	Sleep(50);

	MCONTACT hContact = (UINT_PTR)arg;
	CMStringW message(db_get_wsm(hContact, "CList", "StatusMsg"));

	// if contact has no status message, get xstatus message
	if (message.IsEmpty()) {
		ptrW xStatusName(getWStringA(hContact, "XStatusName"));
		ptrW xStatusMsg(getWStringA(hContact, "XStatusMsg"));

		if (xStatusName)
			message.AppendFormat(L"%s: %s", xStatusName.get(), xStatusMsg.get());
		else
			message.Append(xStatusMsg);
	}

	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)message.c_str());
}

HANDLE CSteamProto::GetAwayMsg(MCONTACT hContact)
{
	ForkThread(&CSteamProto::GetAwayMsgThread, (void*)hContact);
	return (HANDLE)1;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CSteamProto::OnContactDeleted(MCONTACT hContact, uint32_t)
{
	// remove only authorized contacts
	if (Contact::IsGroupChat(hContact))
		SvcLeaveChat(hContact, 0);
	else if (!getByte(hContact, "Auth"))
		SendUserRemoveRequest(hContact);

	return true;
}

void CSteamProto::OnEventDeleted(MCONTACT hContact, MEVENT hDbEvent, int flags)
{
	if (!hContact || !Contact::IsGroupChat(hContact) || !(flags & CDF_DEL_HISTORY))
		return;

	DB::EventInfo dbei(hDbEvent, false);

	mir_cslock lck(m_csChats);
	if (hContact != m_deletedContact && m_deletedContact != INVALID_CONTACT_ID)
		SendDeleteMessageRequest();

	m_deletedContact = hContact;
	m_deletedMessages.push_back(dbei.iTimestamp);
	m_impl.m_deleteMsg.Start(500);
}

void CSteamProto::OnMarkRead(MCONTACT hContact, MEVENT hDbEvent)
{
	if (IsOnline()) {
		DB::EventInfo dbei(hDbEvent, false);

		if (Contact::IsGroupChat(hContact)) {
			CChatRoomAckChatMessageNotification request;
			request.chat_group_id = GetId(hContact, DBKEY_STEAM_ID); request.has_chat_group_id = true;
			request.chat_id = getDword(hContact, "ChatId"); request.has_chat_id = true;
			request.timestamp = dbei.iTimestamp; request.has_timestamp = true;
			WSSendService(AckChatMessage, request);
		}
		else {
			CFriendMessagesAckMessageNotification request;
			request.steamid_partner = GetId(hContact, DBKEY_STEAM_ID); request.has_steamid_partner = true;
			request.timestamp = dbei.iTimestamp; request.has_timestamp = true;
			WSSendService(FriendAckMessage, request);
		}
	}
}

int CSteamProto::OnPreCreateMessage(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent *)lParam;
	if (!mir_strcmp(Proto_GetBaseAccountName(evt->hContact), m_szModuleName)) {
		mir_cslock lck(m_csOwnMessages);
		if (auto *pOwn = m_arOwnMessages.find((COwnMessage *)&evt->seq)) {
			evt->dbei->iTimestamp = pOwn->timestamp;
			m_arOwnMessages.remove(pOwn);
		}
	}

	return 0;
}

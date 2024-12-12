#include "stdafx.h"

static int CompareRequests(const ProtoRequest *p1, const ProtoRequest *p2)
{
	if (p1->id == p2->id)
		return 0;
	return (p1->id < p2->id) ? -1 : 1;
}

CSteamProto::CSteamProto(const char *protoName, const wchar_t *userName) :
	PROTO<CSteamProto>(protoName, userName),
	m_impl(*this),
	m_arRequests(10, CompareRequests),
	m_wszGroupName(this, "DefaultGroup", L"Steam"),
	m_wszDeviceName(this, "DeviceName", L"Miranda NG")
{
	// icons
	wchar_t filePath[MAX_PATH];
	GetModuleFileName(g_plugin.getInst(), filePath, MAX_PATH);

	wchar_t sectionName[100];
	mir_snwprintf(sectionName, L"%s/%s", LPGENW("Protocols"), _A2W(MODULE));

	char settingName[100];
	mir_snprintf(settingName, "%s_%s", MODULE, "main");

	SKINICONDESC sid = {};
	sid.flags = SIDF_ALL_UNICODE;
	sid.defaultFile.w = filePath;
	sid.pszName = settingName;
	sid.section.w = sectionName;
	sid.description.w = LPGENW("Protocol icon");
	sid.iDefaultIndex = -IDI_STEAM;
	g_plugin.addIcon(&sid);

	mir_snprintf(settingName, "%s_%s", MODULE, "gaming");
	sid.description.w = LPGENW("Gaming icon");
	sid.iDefaultIndex = -IDI_GAMING;
	g_plugin.addIcon(&sid);

	// temporary DB settings
	db_set_resident(m_szModuleName, "XStatusId");
	db_set_resident(m_szModuleName, "XStatusName");
	db_set_resident(m_szModuleName, "XStatusMsg");
	db_set_resident(m_szModuleName, "IdleTS");
	db_set_resident(m_szModuleName, "GameID");
	db_set_resident(m_szModuleName, "ServerIP");
	db_set_resident(m_szModuleName, "ServerID");

	SetAllContactStatuses(ID_STATUS_OFFLINE);

	// avatar API
	CreateDirectoryTreeW(GetAvatarPath());

	CreateProtoService(PS_GETAVATARINFO, &CSteamProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CSteamProto::GetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CSteamProto::GetMyAvatar);

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

	debugLogA(__FUNCTION__":Setting protocol / module name to '%s'", m_szModuleName);

	if (uint32_t iGlobalValue = getDword(DB_KEY_LASTMSGTS)) {
		for (auto &cc : AccContacts())
			setDword(cc, DB_KEY_LASTMSGTS, iGlobalValue);
		delSetting(DB_KEY_LASTMSGTS);
	}
}

CSteamProto::~CSteamProto()
{
}

MCONTACT CSteamProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	MCONTACT hContact = AddContact(T2Utf(psr->id.w), psr->nick.w, true);

	if (psr->cbSize == sizeof(STEAM_SEARCH_RESULT)) {
		STEAM_SEARCH_RESULT *ssr = (STEAM_SEARCH_RESULT *)psr;
		UpdateContactDetails(hContact, *ssr->data);
	}

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
	return AddContact(blob.get_email(), Utf2T(blob.get_nick()));
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

		SendAuthParam *param = (SendAuthParam*)mir_calloc(sizeof(SendAuthParam));
		param->hContact = hContact;
		param->hAuth = (HANDLE)hAuth;

		ptrA sessionId(getStringA("SessionID"));
		ptrA who(getStringA(hContact, DBKEY_STEAM_ID));
		SendRequest(new AddFriendRequest(m_szAccessToken, sessionId, m_iSteamId, who), &CSteamProto::OnFriendAdded, param);
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
		return PF4_AVATARS | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_FORCEAUTH | PF4_SUPPORTIDLE | PF4_SUPPORTTYPING;// | PF4_IMSENDOFFLINE;
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

	return OnSendMessage(hContact, message);
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
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

		if (!Miranda_IsTerminated())
			SetAllContactStatuses(ID_STATUS_OFFLINE);

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
	}
	return 0;
}

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

bool CSteamProto::OnContactDeleted(MCONTACT hContact, uint32_t)
{
	// remove only authorized contacts
	if (!getByte(hContact, "Auth", 0)) {
		ptrA sessionId(getStringA("SessionID"));
		char *who = getStringA(hContact, DBKEY_STEAM_ID);
		SendRequest(new RemoveFriendRequest(m_szAccessToken, sessionId, m_iSteamId, who), &CSteamProto::OnFriendRemoved, (void*)who);
	}
	return true;
}

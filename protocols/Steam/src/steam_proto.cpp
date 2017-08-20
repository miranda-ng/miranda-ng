#include "stdafx.h"

CSteamProto::CSteamProto(const char* protoName, const wchar_t* userName)
	: PROTO<CSteamProto>(protoName, userName),
	hAuthProcess(1), hMessageProcess(1)
{
	CreateProtoService(PS_CREATEACCMGRUI, &CSteamProto::OnAccountManagerInit);

	m_idleTS = 0;
	m_lastMessageTS = 0;
	isLoginAgain = false;
	m_hQueueThread = NULL;
	m_pollingConnection = NULL;
	m_hPollingThread = NULL;

	// icons
	wchar_t filePath[MAX_PATH];
	GetModuleFileName(g_hInstance, filePath, MAX_PATH);

	wchar_t sectionName[100];
	mir_snwprintf(sectionName, L"%s/%s", LPGENW("Protocols"), _A2W(MODULE));

	char settingName[100];
	mir_snprintf(settingName, "%s_%s", MODULE, "main");

	SKINICONDESC sid = { 0 };
	sid.flags = SIDF_ALL_UNICODE;
	sid.defaultFile.w = filePath;
	sid.pszName = settingName;
	sid.section.w = sectionName;
	sid.description.w = LPGENW("Protocol icon");
	sid.iDefaultIndex = -IDI_STEAM;
	IcoLib_AddIcon(&sid);

	mir_snprintf(settingName, "%s_%s", MODULE, "gaming");
	sid.description.w = LPGENW("Gaming icon");
	sid.iDefaultIndex = -IDI_GAMING;
	IcoLib_AddIcon(&sid);

	// temporary DB settings
	db_set_resident(m_szModuleName, "XStatusId");
	db_set_resident(m_szModuleName, "XStatusName");
	db_set_resident(m_szModuleName, "XStatusMsg");
	db_set_resident(m_szModuleName, "IdleTS");
	db_set_resident(m_szModuleName, "GameID");
	db_set_resident(m_szModuleName, "ServerIP");
	db_set_resident(m_szModuleName, "ServerID");

	SetAllContactsStatus(ID_STATUS_OFFLINE);

	// services
	CreateServiceFunction(MODULE"/MenuChoose", CSteamProto::MenuChooseService);

	// avatar API
	CreateProtoService(PS_GETAVATARINFO, &CSteamProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CSteamProto::GetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CSteamProto::GetMyAvatar);

	// custom status API
	CreateProtoService(PS_GETCUSTOMSTATUSEX, &CSteamProto::OnGetXStatusEx);
	CreateProtoService(PS_GETCUSTOMSTATUSICON, &CSteamProto::OnGetXStatusIcon);
	CreateProtoService(PS_GETADVANCEDSTATUSICON, &CSteamProto::OnRequestAdvStatusIconIdx);

	// custom db events API
	CreateProtoService(STEAM_DB_GETEVENTTEXT_CHATSTATES, &CSteamProto::OnGetEventTextChatStates);

	// netlib support
	wchar_t name[128];
	mir_snwprintf(name, TranslateT("%s connection"), m_tszUserName);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = name;
	nlu.szSettingsModule = m_szModuleName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	requestQueue = new RequestQueue(m_hNetlibUser);
}

CSteamProto::~CSteamProto()
{
	delete requestQueue;
}

MCONTACT CSteamProto::AddToList(int, PROTOSEARCHRESULT* psr)
{
	MCONTACT hContact = NULL;
	ptrA steamId(mir_u2a(psr->id.w));
	if (psr->cbSize == sizeof(PROTOSEARCHRESULT))
	{
		if (!FindContact(steamId))
		{
			//hContact = AddContact(steamId, true);
			//ForkThread(&CSteamProto::UpdateContactsThread, (void*)mir_strdup(steamId));

			ptrA token(getStringA("TokenSecret"));

			PushRequest(
				new GetUserSummariesRequest(token, steamId),
				&CSteamProto::OnGotUserSummaries);
		}
	}
	else if (psr->cbSize == sizeof(STEAM_SEARCH_RESULT))
	{
		STEAM_SEARCH_RESULT *ssr = (STEAM_SEARCH_RESULT*)psr;
		hContact = AddContact(steamId, true);
		UpdateContactDetails(hContact, ssr->data);
	}

	return hContact;
}

int CSteamProto::Authorize(MEVENT hDbEvent)
{
	if (IsOnline() && hDbEvent)
	{
		MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
		if (hContact == INVALID_CONTACT_ID)
			return 1;

		//ForkThread(&CSteamProto::AuthAllowThread, (void*)hContact);

		ptrA token(getStringA("TokenSecret"));
		ptrA sessionId(getStringA("SessionID"));
		ptrA steamId(getStringA("SteamID"));
		char *who = getStringA(hContact, "SteamID");

		PushRequest(
			new ApprovePendingRequest(token, sessionId, steamId, who),
			&CSteamProto::OnPendingApproved,
			who, MirFreeArg);

		return 0;
	}

	return 1;
}

int CSteamProto::AuthDeny(MEVENT hDbEvent, const wchar_t*)
{
	if (IsOnline() && hDbEvent)
	{
		MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
		if (hContact == INVALID_CONTACT_ID)
			return 1;

		//ForkThread(&CSteamProto::AuthDenyThread, (void*)hContact);

		ptrA token(getStringA("TokenSecret"));
		ptrA sessionId(getStringA("SessionID"));
		ptrA steamId(getStringA("SteamID"));
		char *who = getStringA(hContact, "SteamID");

		PushRequest(
			new IgnorePendingRequest(token, sessionId, steamId, who),
			&CSteamProto::OnPendingIgnoreded,
			who, MirFreeArg);

		return 0;
	}

	return 1;
}

int CSteamProto::AuthRequest(MCONTACT hContact, const wchar_t*)
{
	if (IsOnline() && hContact)
	{
		UINT hAuth = InterlockedIncrement(&hAuthProcess);

		SendAuthParam *param = (SendAuthParam*)mir_calloc(sizeof(SendAuthParam));
		param->hContact = hContact;
		param->hAuth = (HANDLE)hAuth;

		//ForkThread(&CSteamProto::AddContactThread, param);

		ptrA token(getStringA("TokenSecret"));
		ptrA sessionId(getStringA("SessionID"));
		ptrA steamId(getStringA("SteamID"));
		ptrA who(getStringA(hContact, "SteamID"));

		/*
		posilame: (kdyz my zadame)
		sessionID	MjYzNDM4NDgw
		steamid	76561198166125402
		accept_invite	0

		pri uspesnem pozadavku vrati: {"invited":["76561198166125402"],"success":1}
		kdyz nas ignoruje: {"failed_invites":["76561198166125402"],"failed_invites_result":[41],"success":1}

		*/

		PushRequest(
			new AddFriendRequest(token, sessionId, steamId, who),
			&CSteamProto::OnFriendAdded,
			param);

		return hAuth;
	}

	return 1;
}

DWORD_PTR CSteamProto:: GetCaps(int type, MCONTACT)
{
	switch(type)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_BASICSEARCH | PF1_SEARCHBYNAME | PF1_AUTHREQ | PF1_SERVERCLIST | PF1_ADDSEARCHRES | PF1_MODEMSGRECV;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_HEAVYDND | PF2_OUTTOLUNCH | PF2_FREECHAT;
	case PFLAGNUM_4:
		return PF4_AVATARS | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_FORCEAUTH | PF4_FORCEADDED | PF4_SUPPORTIDLE | PF4_SUPPORTTYPING;// | PF4_IMSENDOFFLINE;
	case PFLAGNUM_5:
		return PF2_HEAVYDND | PF2_OUTTOLUNCH | PF2_FREECHAT;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)Translate("SteamID");
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"SteamID";
	default:
		return 0;
	}
}

HANDLE CSteamProto::SearchBasic(const wchar_t* id)
{
	if (!this->IsOnline())
		return 0;

	ptrA token(getStringA("TokenSecret"));
	ptrA steamId(mir_u2a(id));

	PushRequest(
		new GetUserSummariesRequest(token, steamId),
		&CSteamProto::OnSearchResults,
		(HANDLE)STEAM_SEARCH_BYID);

	return (HANDLE)STEAM_SEARCH_BYID;
}

HANDLE CSteamProto::SearchByName(const wchar_t* nick, const wchar_t* firstName, const wchar_t* lastName)
{
	if (!this->IsOnline())
		return 0;

	// Combine all fields to single text
	wchar_t keywordsT[200];
	mir_snwprintf(keywordsT, L"%s %s %s", nick, firstName, lastName);

	ptrA token(getStringA("TokenSecret"));
	ptrA keywords(mir_utf8encodeW(keywordsT));

	PushRequest(
		new SearchRequest(token, keywords),
		&CSteamProto::OnSearchByNameStarted,
		(HANDLE)STEAM_SEARCH_BYNAME);

	return (HANDLE)STEAM_SEARCH_BYNAME;
}

int CSteamProto::SendMsg(MCONTACT hContact, int, const char *message)
{
	if (!IsOnline())
	{
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, NULL, (LPARAM)Translate("You cannot send messages when you are offline."));
		return 0;
	}

	return OnSendMessage(hContact, message);
}

int CSteamProto::SetStatus(int new_status)
{
	mir_cslock lock(set_status_lock);

	// Routing statuses not supported by Steam
	switch (new_status)
	{
	case ID_STATUS_OFFLINE:
	case ID_STATUS_AWAY:
	case ID_STATUS_NA:
		break;

	case ID_STATUS_DND:
	case ID_STATUS_OCCUPIED:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		new_status = ID_STATUS_NA;
		break;

	default:
		new_status = ID_STATUS_ONLINE;
		break;
	}

	if (new_status == m_iDesiredStatus)
		return 0;

	debugLogW(L"CSteamProto::SetStatus: changing status from %i to %i", m_iStatus, new_status);

	int old_status = m_iStatus;
	m_iDesiredStatus = new_status;

	if (new_status == ID_STATUS_OFFLINE)
	{
		// Reset relogin flag
		isLoginAgain = false;

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		ptrA token(getStringA("TokenSecret"));
		ptrA umqid(getStringA("UMQID"));
		SendRequest(new LogoffRequest(token, umqid));

		requestQueue->Stop();

		if (!Miranda_IsTerminated())
			SetAllContactsStatus(ID_STATUS_OFFLINE);
	}
	else if (old_status == ID_STATUS_OFFLINE)
	{
		// Load last message timestamp for correct loading of messages history
		m_lastMessageTS = getDword("LastMessageTS", 0);

		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		requestQueue->Start();

		ptrA token(getStringA("TokenSecret"));
		ptrA sessionId(getStringA("SessionID"));
		if (mir_strlen(token) > 0 && mir_strlen(sessionId) > 0)
		{
			PushRequest(
				new LogonRequest(token),
				&CSteamProto::OnLoggedOn);
		}
		else
		{
			ptrA username(mir_urlEncode(ptrA(mir_utf8encodeW(getWStringA("Username")))));
			if (username == NULL || username[0] == '\0')
			{
				m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
				ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
				return 0;
			}

			PushRequest(
				new GetRsaKeyRequest(username),
				&CSteamProto::OnGotRsaKey);
		}
	}
	else
	{
		m_iStatus = new_status;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	}

	return 0;
}

void __cdecl CSteamProto::GetAwayMsgThread(void *arg)
{
	// Maybe not needed, but better to be sure that this won't happen faster than core handling return value of GetAwayMsg()
	Sleep(50);

	MCONTACT hContact = (UINT_PTR)arg;
	CMStringW message(db_get_wsa(hContact, "CList", "StatusMsg"));
	
	// if contact has no status message, get xstatus message
	if (message.IsEmpty())
	{
		ptrW xStatusName(getWStringA(hContact, "XStatusName"));
		ptrW xStatusMsg(getWStringA(hContact, "XStatusMsg"));

		if (xStatusName)
			message.AppendFormat(L"%s: %s", xStatusName, xStatusMsg);
		else
			message.Append(xStatusMsg);
	}

	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)message.c_str());
}

HANDLE __cdecl CSteamProto::GetAwayMsg(MCONTACT hContact)
{
	ForkThread(&CSteamProto::GetAwayMsgThread, (void*)hContact);
	return (HANDLE)1;
}

int __cdecl CSteamProto::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	switch (eventType)
	{
	case EV_PROTO_ONLOAD:
		return this->OnModulesLoaded(wParam, lParam);

	/*case EV_PROTO_ONOPTIONS:
		return this->OnOptionsInit(wParam, lParam);*/

	case EV_PROTO_ONCONTACTDELETED:
		if (IsOnline())
		{
			MCONTACT hContact = (MCONTACT)wParam;

			ptrA token(getStringA("TokenSecret"));
			ptrA sessionId(getStringA("SessionID"));
			ptrA steamId(getStringA("SteamID"));
			ptrA who(getStringA(hContact, "SteamID"));

			// Don't request delete contact from server when we're not friends anyway
			if (getByte(hContact, "Auth", 0) != 0)
				return 0;

			PushRequest(
				new RemoveFriendRequest(token, sessionId, steamId, who),
				&CSteamProto::OnFriendRemoved,
				(void*)hContact);
		}
		return 0;

	case EV_PROTO_ONMENU:
		this->OnInitStatusMenu();
		break;
	}

	return 1;
}
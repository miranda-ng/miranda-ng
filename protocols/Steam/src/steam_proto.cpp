#include "stdafx.h"

CSteamProto::CSteamProto(const char* protoName, const TCHAR* userName) :
	PROTO<CSteamProto>(protoName, userName),
	hAuthProcess(1),
	hMessageProcess(1)
{
	CreateProtoService(PS_CREATEACCMGRUI, &CSteamProto::OnAccountManagerInit);

	m_idleTS = 0;
	isTerminated = false;
	m_hQueueThread = NULL;
	m_pollingConnection = NULL;
	m_hPollingThread = NULL;
	m_hMenuRoot = NULL;

	// icons
	wchar_t filePath[MAX_PATH];
	GetModuleFileName(g_hInstance, filePath, MAX_PATH);

	wchar_t sectionName[100];
	mir_sntprintf(sectionName, SIZEOF(sectionName), _T("%s/%s"), LPGENT("Protocols"), LPGENT(MODULE));

	char settingName[100];
	mir_snprintf(settingName, SIZEOF(settingName), "%s_%s", MODULE, "main");

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(SKINICONDESC);
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszDefaultFile = filePath;
	sid.pszName = settingName;
	sid.ptszSection = sectionName;
	sid.ptszDescription = LPGENT("Protocol icon");
	sid.iDefaultIndex = -IDI_STEAM;
	Skin_AddIcon(&sid);

	mir_snprintf(settingName, SIZEOF(settingName), "%s_%s", MODULE, "gaming");
	sid.ptszDescription = LPGENT("Gaming icon");
	sid.iDefaultIndex = -IDI_GAMING;
	Skin_AddIcon(&sid);

	// temporary DB settings
	db_set_resident(m_szModuleName, "Status");
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
	CreateProtoService(PS_GETAVATARINFOT, &CSteamProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CSteamProto::GetAvatarCaps);
	CreateProtoService(PS_GETMYAVATART, &CSteamProto::GetMyAvatar);
	// custom status API
	CreateProtoService(PS_GETCUSTOMSTATUSEX, &CSteamProto::OnGetXStatusEx);
	CreateProtoService(PS_GETCUSTOMSTATUSICON, &CSteamProto::OnGetXStatusIcon);
	CreateProtoService(PS_GETADVANCEDSTATUSICON, &CSteamProto::OnRequestAdvStatusIconIdx);
}

CSteamProto::~CSteamProto()
{
}

MCONTACT CSteamProto::AddToList(int, PROTOSEARCHRESULT* psr)
{
	MCONTACT hContact = NULL;
	ptrA steamId(mir_u2a(psr->id));
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
		UpdateContact(hContact, ssr->data);
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

int CSteamProto::AuthDeny(MEVENT hDbEvent, const TCHAR*)
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

int CSteamProto::AuthRequest(MCONTACT hContact, const TCHAR*)
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
		return PF1_IM | PF1_BASICSEARCH | PF1_SEARCHBYNAME | PF1_AUTHREQ | PF1_SERVERCLIST | PF1_ADDSEARCHRES;
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
	case PFLAG_MAXLENOFMESSAGE:
		return 200000; // this is guessed limit, in reality it is probably bigger
	default:
		return 0;
	}
}

HANDLE CSteamProto::SearchBasic(const TCHAR* id)
{
	if (!this->IsOnline())
		return 0;

	//ForkThread(&CSteamProto::SearchByIdThread, mir_wstrdup(id));

	ptrA token(getStringA("TokenSecret"));
	ptrA steamId(mir_t2a(id));

	PushRequest(
		new GetUserSummariesRequest(token, steamId),
		&CSteamProto::OnSearchByIdEnded,
		mir_tstrdup(id),
		MirFreeArg);

	return (HANDLE)STEAM_SEARCH_BYID;
}

int CSteamProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT* pre)
{
	return (INT_PTR)AddDBEvent(hContact, EVENTTYPE_MESSAGE, pre->timestamp, DBEF_UTF, mir_strlen(pre->szMessage), (BYTE*)pre->szMessage);
}

int CSteamProto::SendMsg(MCONTACT hContact, int flags, const char *message)
{
	if (!IsOnline())
	{
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, NULL, (LPARAM)Translate("You cannot send messages when you are offline."));
		return 0;
	}

	UINT hMessage = InterlockedIncrement(&hMessageProcess);

	SendMessageParam *param = (SendMessageParam*)mir_calloc(sizeof(SendMessageParam));
	param->hContact = hContact;
	param->hMessage = (HANDLE)hMessage;
	param->message = mir_strdup(message);

	ptrA token(getStringA("TokenSecret"));
	ptrA umqid(getStringA("UMQID"));
	ptrA steamId(getStringA(hContact, "SteamID"));
	PushRequest(
		new SendMessageRequest(token, umqid, steamId, message),
		&CSteamProto::OnMessageSent,
		param, MessageParamFree);

	return hMessage;
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

	debugLog(_T("CSteamProto::SetStatus: changing status from %i to %i"), m_iStatus, new_status);

	int old_status = m_iStatus;
	m_iDesiredStatus = new_status;

	if (new_status == ID_STATUS_OFFLINE)
	{
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		ptrA token(getStringA("TokenSecret"));
		ptrA umqid(getStringA("UMQID"));
		SendRequest(new LogoffRequest(token, umqid));

		requestQueue->Stop();

		if (!Miranda_Terminated())
			SetAllContactsStatus(ID_STATUS_OFFLINE);
	}
	else if (old_status == ID_STATUS_OFFLINE)
	{
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		requestQueue->Start();

		ptrA token(getStringA("TokenSecret"));
		if (mir_strlen(token) > 0)
		{
			PushRequest(
				new LogonRequest(token),
				&CSteamProto::OnLoggedOn);
		}
		else
		{
			ptrA username(mir_urlEncode(ptrA(mir_utf8encodeT(getTStringA("Username")))));
			if (username == NULL || username[0] == '\0')
			{
				m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
				ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
				return 0;
			}

			PushRequest(
				new RsaKeyRequest(username),
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

int __cdecl CSteamProto::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	switch (eventType)
	{
	case EV_PROTO_ONLOAD:
		return this->OnModulesLoaded(wParam, lParam);

	case EV_PROTO_ONEXIT:
		return this->OnPreShutdown(wParam, lParam);

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
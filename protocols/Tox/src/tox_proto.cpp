#include "common.h"

CToxProto::CToxProto(const char* protoName, const TCHAR* userName) :
PROTO<CToxProto>(protoName, userName)
{
	InitToxCore();

	CreateProtoService(PS_CREATEACCMGRUI, &CToxProto::OnAccountManagerInit);

	SetAllContactsStatus(ID_STATUS_OFFLINE);

	// icons
	wchar_t filePath[MAX_PATH];
	GetModuleFileName(g_hInstance, filePath, MAX_PATH);

	wchar_t sectionName[100];
	mir_sntprintf(sectionName, SIZEOF(sectionName), _T("%s/%s"), LPGENT("Protocols"), MODULE);

	char settingName[100];
	mir_snprintf(settingName, SIZEOF(settingName), "%s_%s", MODULE, "main");

	SKINICONDESC sid = { 0 };
	sid.cbSize = sizeof(SKINICONDESC);
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszDefaultFile = filePath;
	sid.pszName = settingName;
	sid.ptszSection = sectionName;
	sid.ptszDescription = LPGENT("Protocol icon");
	sid.iDefaultIndex = -IDI_TOX;
	Skin_AddIcon(&sid);
}

CToxProto::~CToxProto()
{
	UninitToxCore();
}

DWORD_PTR __cdecl CToxProto::GetCaps(int type, MCONTACT hContact)
{
	switch (type)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_AUTHREQ | PF1_EXTSEARCH;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LIGHTDND;
	case PFLAGNUM_4:
		return PF4_IMSENDUTF | PF4_NOAUTHDENYREASON | PF4_FORCEAUTH | PF4_FORCEADDED | PF4_SUPPORTTYPING;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)"Tox ID";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)TOX_SETTINGS_ID;
	case PFLAG_MAXLENOFMESSAGE:
		return TOX_MAX_MESSAGE_LENGTH;
	}

	return 0;
}

MCONTACT __cdecl CToxProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	return AddContact(_T2A(psr->id), true);
}

MCONTACT __cdecl CToxProto::AddToListByEvent(int flags, int iContact, HANDLE hDbEvent) { return 0; }

int __cdecl CToxProto::Authorize(HANDLE hDbEvent)
{
	if (this->IsOnline() && hDbEvent)
	{
		MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
		if (hContact == INVALID_CONTACT_ID)
			return 1;

		std::string toxId = getStringA(hContact, TOX_SETTINGS_ID);
		std::vector<uint8_t> clientId = HexStringToData(toxId);

		if (tox_add_friend_norequest(tox, &clientId[0]) >= 0)
		{
			SaveToxData();

			return 0;
		}
	}

	return 1;
}

int __cdecl CToxProto::AuthDeny(HANDLE hDbEvent, const PROTOCHAR* szReason) { return 0; }
int __cdecl CToxProto::AuthRecv(MCONTACT hContact, PROTORECVEVENT*) { return 0; }

int __cdecl CToxProto::AuthRequest(MCONTACT hContact, const PROTOCHAR* szMessage)
{
	std::string toxId = getStringA(hContact, TOX_SETTINGS_ID);
	std::vector<uint8_t> clientId = HexStringToData(toxId);

	ptrA reason(mir_utf8encodeW(szMessage));

	int32_t friendnumber = tox_add_friend(tox, &clientId[0], (uint8_t*)(char*)reason, (uint16_t)strlen(reason));
	if (friendnumber >= 0)
	{
		SaveToxData();

		clientId.resize(TOX_CLIENT_ID_SIZE);
		tox_get_client_id(tox, friendnumber, &clientId[0]);
		std::string toxId = DataToHexString(clientId);

		setString(hContact, TOX_SETTINGS_ID, toxId.c_str());

		db_unset(hContact, "CList", "NotOnList");
		db_unset(hContact, "CList", "Auth");

		std::vector<uint8_t> username(TOX_MAX_NAME_LENGTH);
		tox_get_name(tox, friendnumber, &username[0]);
		std::string nick(username.begin(), username.end());
		setString(hContact, "Nick", nick.c_str());

		return 0;
	}

	return 1;
}

HANDLE __cdecl CToxProto::ChangeInfo(int iInfoType, void* pInfoData) { return 0; }

HANDLE __cdecl CToxProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szPath) { return 0; }
int __cdecl CToxProto::FileCancel(MCONTACT hContact, HANDLE hTransfer) { return 0; }
int __cdecl CToxProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szReason) { return 0; }
int __cdecl CToxProto::FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename) { return 0; }

int __cdecl CToxProto::GetInfo(MCONTACT hContact, int infoType) { return 0; }

HANDLE __cdecl CToxProto::SearchBasic(const PROTOCHAR* id) { return 0; }

HANDLE __cdecl CToxProto::SearchByEmail(const PROTOCHAR* email) { return 0; }

HANDLE __cdecl CToxProto::SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName) { return 0; }

HWND __cdecl CToxProto::SearchAdvanced(HWND owner)
{
	std::smatch match;
	std::regex regex("^\\s*([A-Fa-f0-9]{76})\\s*$");

	char text[TOX_FRIEND_ADDRESS_SIZE * 2 + 1];
	GetDlgItemTextA(owner, IDC_SEARCH, text, SIZEOF(text));

	const std::string query = text;
	if (std::regex_search(query, match, regex))
	{
		std::string clientId = match[1];

		ADDCONTACTSTRUCT acs = { 0 };

		PROTOSEARCHRESULT psr = { 0 };
		psr.cbSize = sizeof(psr);
		psr.flags = PSR_TCHAR;
		psr.id = mir_a2t(query.c_str());

		acs.psr = &psr;
		acs.szProto = m_szModuleName;

		acs.handleType = HANDLE_SEARCHRESULT;
		CallService(MS_ADDCONTACT_SHOW, (WPARAM)owner, (LPARAM)&acs);

		ForkThread(&CToxProto::SearchByIdAsync, mir_strdup(query.c_str()));
	}
	else
	{
		regex = "^\\s*([A-Za-z]+)(@toxme.se)?\\s*$";
		if (std::regex_search(query, match, regex))
		{
			std::string query = match[1];
			ForkThread(&CToxProto::SearchByNameAsync, mir_strdup(query.c_str()));
		}
	}

	return (HWND)1;
}

HWND __cdecl CToxProto::CreateExtendedSearchUI(HWND owner)
{
	return CreateDialogParam(
		g_hInstance,
		MAKEINTRESOURCE(IDD_SEARCH),
		owner,
		SearchDlgProc,
		(LPARAM)this);
}

int __cdecl CToxProto::RecvContacts(MCONTACT hContact, PROTORECVEVENT*) { return 0; }
int __cdecl CToxProto::RecvFile(MCONTACT hContact, PROTOFILEEVENT*) { return 0; }

int __cdecl CToxProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	return Proto_RecvMessage(hContact, pre);
}

int __cdecl CToxProto::RecvUrl(MCONTACT hContact, PROTORECVEVENT*) { return 0; }

int __cdecl CToxProto::SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT* hContactsList) { return 0; }
HANDLE __cdecl CToxProto::SendFile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles) { return 0; }

int __cdecl CToxProto::SendMsg(MCONTACT hContact, int flags, const char* msg)
{
	std::string toxId(getStringA(hContact, TOX_SETTINGS_ID));
	std::vector<uint8_t> clientId = HexStringToData(toxId);

	uint32_t number = tox_get_friend_number(tox, clientId.data());

	int result = tox_send_message(tox, number, (uint8_t*)msg, (uint16_t)strlen(msg));
	if (result == 0)
	{
		debugLogA("CToxProto::SendMsg: could not to send message");
	}

	return result;
}

int __cdecl CToxProto::SendUrl(MCONTACT hContact, int flags, const char* url) { return 0; }

int __cdecl CToxProto::SetApparentMode(MCONTACT hContact, int mode) { return 0; }

int __cdecl CToxProto::SetStatus(int iNewStatus)
{
	if (iNewStatus == m_iDesiredStatus)
		return 0;

	int old_status = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE)
	{
		// logout
		isTerminated = true;
		isConnected = false;

		if (!Miranda_Terminated())
		{
			SetAllContactsStatus(ID_STATUS_OFFLINE);
		}

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	}
	else
	{
		if (old_status == ID_STATUS_OFFLINE && !IsOnline())
		{
			m_iStatus = ID_STATUS_CONNECTING;

			isTerminated = isConnected = false;
			hPollingThread = ForkThreadEx(&CToxProto::PollingThread, 0, NULL);
		}
		else
		{
			// set tox status
			m_iStatus = iNewStatus;
			if (tox_set_user_status(tox, MirandaToToxStatus(iNewStatus)) == 0)
			{
				SaveToxData();
			}
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	return 0;
}

HANDLE __cdecl CToxProto::GetAwayMsg(MCONTACT hContact) { return 0; }
int __cdecl CToxProto::RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt) { return 0; }
int __cdecl CToxProto::SetAwayMsg(int iStatus, const PROTOCHAR* msg) { return 0; }

int __cdecl CToxProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (hContact && IsOnline())
	{
		std::string toxId(getStringA(hContact, TOX_SETTINGS_ID));
		std::vector<uint8_t> clientId = HexStringToData(toxId);

		uint32_t number = tox_get_friend_number(tox, clientId.data());

		if (number >= 0)
		{
			tox_set_user_is_typing(tox, number, type);
			return 0;
		}
	}

	return 1;
}

int __cdecl CToxProto::OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam)
{
	switch (iEventType)
	{
	case EV_PROTO_ONLOAD:
		return OnAccountLoaded(wParam, lParam);

	case EV_PROTO_ONCONTACTDELETED:
		return OnContactDeleted(wParam, lParam);

	case EV_PROTO_ONEXIT:
		return OnPreShutdown(wParam, lParam);
	}

	return 1;
}
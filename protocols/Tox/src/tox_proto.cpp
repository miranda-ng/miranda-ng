#include "common.h"

CToxProto::CToxProto(const char* protoName, const TCHAR* userName) :
PROTO<CToxProto>(protoName, userName)
{
	accountName = mir_tstrdup(userName);

	InitToxCore();

	CreateProtoService(PS_CREATEACCMGRUI, &CToxProto::OnAccountManagerInit);

	SetAllContactsStatus(ID_STATUS_OFFLINE);

	// icons
	wchar_t filePath[MAX_PATH];
	GetModuleFileName(g_hInstance, filePath, MAX_PATH);

	wchar_t sectionName[100];
	mir_sntprintf(sectionName, SIZEOF(sectionName), _T("%s/%s"), LPGENT("Protocols"), _A2T(MODULE));

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

	// custom event
	DBEVENTTYPEDESCR dbEventType = { sizeof(dbEventType) };
	dbEventType.module = this->m_szModuleName;
	dbEventType.flags = DETF_HISTORY | DETF_MSGWINDOW;

	dbEventType.eventType = TOX_DB_EVENT_TYPE_ACTION;
	dbEventType.descr = "Tox action";
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	// avatars
	/*CreateProtoService(PS_GETAVATARCAPS, &CToxProto::GetAvatarCaps);
	CreateProtoService(PS_GETAVATARINFOT, &CToxProto::GetAvatarInfo);
	CreateProtoService(PS_GETMYAVATART, &CToxProto::GetMyAvatar);
	CreateProtoService(PS_SETMYAVATART, &CToxProto::SetMyAvatar);*/
}

CToxProto::~CToxProto()
{
	UninitToxCore();

	mir_free(accountName);
}

DWORD_PTR __cdecl CToxProto::GetCaps(int type, MCONTACT hContact)
{
	switch (type)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILE | PF1_AUTHREQ | PF1_EXTSEARCH;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LIGHTDND;
	case PFLAGNUM_4:
		return PF4_IMSENDUTF | PF4_SINGLEFILEONLY | PF4_NOAUTHDENYREASON |PF4_FORCEAUTH
			| PF4_FORCEADDED | PF4_SUPPORTTYPING | PF4_AVATARS;
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
	std::string address(mir_t2a(psr->id));
	if (IsMe(address))
	{
		debugLogA("CToxProto::AddToList: you cannot add yourself to friend list");
		return NULL;
	}
	// set tox address as contact id
	return AddContact(address, flags & PALF_TEMPORARY);
}

MCONTACT __cdecl CToxProto::AddToListByEvent(int flags, int iContact, HANDLE hDbEvent)
{
	return 0;
}

int __cdecl CToxProto::Authorize(HANDLE hDbEvent)
{
	MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
	{
		return 1;
	}

	std::string id = getStringA(hContact, TOX_SETTINGS_ID);
	std::vector<uint8_t> clientId = HexStringToData(id);
	if (tox_add_friend_norequest(tox, clientId.data()) != TOX_ERROR)
	{
		return 1;
	}

	db_unset(hContact, "CList", "NotOnList");
	delSetting(hContact, "Auth");
	SaveToxData();

	return 0;
}

int __cdecl CToxProto::AuthDeny(HANDLE hDbEvent, const PROTOCHAR* szReason) { return 0; }

int __cdecl CToxProto::AuthRecv(MCONTACT, PROTORECVEVENT* pre)
{
	return Proto_AuthRecv(m_szModuleName, pre);
}

int __cdecl CToxProto::AuthRequest(MCONTACT hContact, const PROTOCHAR* szMessage)
{
	ptrA reason(mir_utf8encodeW(szMessage));

	std::string address = getStringA(hContact, TOX_SETTINGS_ID);
	std::vector<uint8_t> pubKey = HexStringToData(address);
	int32_t number = tox_add_friend(tox, pubKey.data(), (uint8_t*)(char*)reason, (uint16_t)strlen(reason));
	if (number > TOX_ERROR)
	{
		SaveToxData();

		// change tox address in contact id by tox id
		std::string id = ToxAddressToId(address);
		setString(hContact, TOX_SETTINGS_ID, id.c_str());

		db_unset(hContact, "CList", "NotOnList");
		delSetting(hContact, "Auth");

		std::string nick("", TOX_MAX_NAME_LENGTH);
		tox_get_name(tox, number, (uint8_t*)&nick[0]);
		setString(hContact, "Nick", nick.c_str());

		return 0;
	}

	return 1;
}

HANDLE __cdecl CToxProto::ChangeInfo(int iInfoType, void* pInfoData) { return 0; }

int __cdecl CToxProto::GetInfo(MCONTACT hContact, int infoType) { return 0; }

int __cdecl CToxProto::RecvContacts(MCONTACT hContact, PROTORECVEVENT*) { return 0; }

int __cdecl CToxProto::RecvFile(MCONTACT hContact, PROTOFILEEVENT *pre)
{
	return Proto_RecvFile(hContact, pre);
}

int __cdecl CToxProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	return Proto_RecvMessage(hContact, pre);
}

int __cdecl CToxProto::RecvUrl(MCONTACT hContact, PROTORECVEVENT*) { return 0; }

int __cdecl CToxProto::SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT* hContactsList) { return 0; }

int __cdecl CToxProto::SendUrl(MCONTACT hContact, int flags, const char* url) { return 0; }

int __cdecl CToxProto::SetApparentMode(MCONTACT hContact, int mode) { return 0; }

int __cdecl CToxProto::SetStatus(int iNewStatus)
{
	if (iNewStatus == m_iDesiredStatus)
		return 0;

	debugLogA("CToxProto::SetStatus: changing status from %i to %i", m_iStatus, iNewStatus);

	int old_status = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE)
	{
		// logout
		isTerminated = true;

		if (!Miranda_Terminated())
		{
			SetAllContactsStatus(ID_STATUS_OFFLINE);
		}

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	}
	else
	{
		if (old_status == ID_STATUS_CONNECTING)
		{
			return 0;
		}

		if (old_status == ID_STATUS_OFFLINE && !IsOnline())
		{
			m_iStatus = ID_STATUS_CONNECTING;

			isTerminated = false;
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
#include "common.h"

CToxProto::CToxProto(const char* protoName, const TCHAR* userName) :
	PROTO<CToxProto>(protoName, userName), password(NULL)
{
	InitNetlib();

	accountName = mir_tstrdup(userName);

	CreateProtoService(PS_CREATEACCMGRUI, &CToxProto::OnAccountManagerInit);

	SetAllContactsStatus(ID_STATUS_OFFLINE);

	// custom event
	DBEVENTTYPEDESCR dbEventType = { sizeof(dbEventType) };
	dbEventType.module = m_szModuleName;
	dbEventType.flags = DETF_HISTORY | DETF_MSGWINDOW;
	dbEventType.eventType = TOX_DB_EVENT_TYPE_ACTION;
	dbEventType.descr = Translate("Action");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	// avatars
	CreateProtoService(PS_GETAVATARCAPS, &CToxProto::GetAvatarCaps);
	CreateProtoService(PS_GETAVATARINFOT, &CToxProto::GetAvatarInfo);
	CreateProtoService(PS_GETMYAVATART, &CToxProto::GetMyAvatar);
	CreateProtoService(PS_SETMYAVATART, &CToxProto::SetMyAvatar);

	// nick
	CreateProtoService(PS_SETMYNICKNAME, &CToxProto::SetMyNickname);
}

CToxProto::~CToxProto()
{
	mir_free(accountName);
	UninitNetlib();
}

DWORD_PTR __cdecl CToxProto::GetCaps(int type, MCONTACT)
{
	switch (type)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILE | PF1_AUTHREQ | PF1_MODEMSG | PF1_EXTSEARCH | PF1_SERVERCLIST;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LIGHTDND;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LIGHTDND;
	case PFLAGNUM_4:
		return PF4_IMSENDUTF | PF4_SINGLEFILEONLY | PF4_SUPPORTTYPING | PF4_AVATARS
			| PF4_FORCEADDED | PF4_NOAUTHDENYREASON | PF4_FORCEAUTH;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)"Tox ID";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)TOX_SETTINGS_ID;
	case PFLAG_MAXLENOFMESSAGE:
		return TOX_MAX_MESSAGE_LENGTH;
	}

	return 0;
}

MCONTACT __cdecl CToxProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	std::string address(ptrA(mir_t2a(psr->id)));
	ptrA myAddress(getStringA(NULL, TOX_SETTINGS_ID));
	if (strnicmp(address.c_str(), myAddress, TOX_PUBLIC_KEY_SIZE) == 0)
	{
		ShowNotification(TranslateT("You cannot add yourself to your contact list"), 0);
		return NULL;
	}
	MCONTACT hContact = GetContact(address.c_str());
	if (hContact)
	{
		ShowNotification(TranslateT("Contact already in your contact list"), 0, hContact);
		return NULL;
	}
	// set tox address as contact public key
	return AddContact(address.c_str(), _T(""), flags & PALF_TEMPORARY);
}

int __cdecl CToxProto::Authorize(MEVENT hDbEvent)
{
	MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	return OnGrantAuth(hContact, 0);
}

int __cdecl CToxProto::AuthRecv(MCONTACT, PROTORECVEVENT* pre)
{
	return Proto_AuthRecv(m_szModuleName, pre);
}

int __cdecl CToxProto::AuthRequest(MCONTACT hContact, const PROTOCHAR *szMessage)
{
	ptrA reason(mir_utf8encodeW(szMessage));
	return OnRequestAuth(hContact, (LPARAM)reason);
}

HWND __cdecl CToxProto::SearchAdvanced(HWND owner)
{
	return OnSearchAdvanced(owner);
}

HWND __cdecl CToxProto::CreateExtendedSearchUI(HWND owner)
{
	return OnCreateExtendedSearchUI(owner);
}

int __cdecl CToxProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	return OnReceiveMessage(hContact, pre);
}

int CToxProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{
	return OnSendMessage(hContact, flags, msg);
}

int __cdecl CToxProto::SetStatus(int iNewStatus)
{
	if (iNewStatus == m_iDesiredStatus)
		return 0;

	switch (iNewStatus)
	{
	case ID_STATUS_FREECHAT:
	case ID_STATUS_ONTHEPHONE:
		iNewStatus = ID_STATUS_ONLINE;
		break;

	case ID_STATUS_NA:
	case ID_STATUS_OUTTOLUNCH:
		iNewStatus = ID_STATUS_AWAY;
		break;

	case ID_STATUS_DND:
	case ID_STATUS_INVISIBLE:
		iNewStatus = ID_STATUS_OCCUPIED;
		break;
	}

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
			CloseAllChatChatSessions();
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
			// login
			m_iStatus = ID_STATUS_CONNECTING;

			isTerminated = false;
			hPollingThread = ForkThreadEx(&CToxProto::PollingThread, 0, NULL);
		}
		else
		{
			// set tox status
			m_iStatus = iNewStatus;
			if (tox_set_user_status(tox, MirandaToToxStatus(iNewStatus)) == TOX_ERROR)
			{
				debugLogA("CToxProto::SetStatus: failed to change status from %i", m_iStatus, iNewStatus);
			}
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	return 0;
}

HANDLE __cdecl CToxProto::GetAwayMsg(MCONTACT) { return 0; }

int __cdecl CToxProto::SetAwayMsg(int, const PROTOCHAR *msg)
{
	if (IsOnline())
	{
		ptrA statusMessage(msg == NULL ? mir_strdup("") : mir_utf8encodeT(msg));
		if (tox_set_status_message(tox, (uint8_t*)(char*)statusMessage, min(TOX_MAX_STATUSMESSAGE_LENGTH, mir_strlen(statusMessage))) == TOX_ERROR)
		{
			debugLogA("CToxProto::SetAwayMsg: failed to set status status message %s", msg);
		}
	}

	return 0;
}

int __cdecl CToxProto::OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam)
{
	switch (iEventType)
	{
	case EV_PROTO_ONLOAD:
		return OnAccountLoaded(wParam, lParam);

	case EV_PROTO_ONRENAME:
		return OnAccountRenamed(wParam, lParam);

	case EV_PROTO_ONCONTACTDELETED:
		return OnContactDeleted(wParam, lParam);

	case EV_PROTO_ONMENU:
		return OnInitStatusMenu();
	}

	return 1;
}

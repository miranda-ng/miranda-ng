#include "common.h"

WORD CToxProto::GetContactStatus(MCONTACT hContact)
{
	return getWord(hContact, "Status", ID_STATUS_OFFLINE);
}

void CToxProto::SetContactStatus(MCONTACT hContact, WORD status)
{
	WORD oldStatus = GetContactStatus(hContact);
	if (oldStatus != status)
	{
		setWord(hContact, "Status", status);
	}
}

void CToxProto::SetAllContactsStatus(WORD status)
{
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		SetContactStatus(hContact, status);
	}
}

MCONTACT CToxProto::GetContactFromAuthEvent(MEVENT hEvent)
{
	DWORD body[3];
	DBEVENTINFO dbei = { sizeof(DBEVENTINFO) };
	dbei.cbBlob = sizeof(DWORD)* 2;
	dbei.pBlob = (PBYTE)&body;

	if (db_event_get(hEvent, &dbei))
		return INVALID_CONTACT_ID;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return INVALID_CONTACT_ID;

	if (strcmp(dbei.szModule, m_szModuleName) != 0)
		return INVALID_CONTACT_ID;

	return DbGetAuthEventContact(&dbei);
}

MCONTACT CToxProto::GetContact(const int friendNumber)
{
	uint8_t data[TOX_PUBLIC_KEY_SIZE];
	tox_get_client_id(tox, friendNumber, data);
	ToxHexAddress pubKey(data, TOX_PUBLIC_KEY_SIZE);
	return GetContact(pubKey);
}

MCONTACT CToxProto::GetContact(const char *pubKey)
{
	MCONTACT hContact = NULL;
	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		ptrA contactPubKey(getStringA(hContact, TOX_SETTINGS_ID));
		// check only public key part of address
		if (strnicmp(pubKey, contactPubKey, TOX_PUBLIC_KEY_SIZE) == 0)
		{
			break;
		}
	}
	return hContact;
}

MCONTACT CToxProto::AddContact(const char *address, const std::tstring &dnsId, bool isTemporary)
{
	MCONTACT hContact = GetContact(address);
	if (!hContact)
	{
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName);

		setString(hContact, TOX_SETTINGS_ID, address);

		if (!dnsId.empty())
		{
			setTString(hContact, TOX_SETTINGS_DNS, dnsId.c_str());
		}

		DBVARIANT dbv;
		if (!getTString(TOX_SETTINGS_GROUP, &dbv))
		{
			db_set_ts(hContact, "CList", "Group", dbv.ptszVal);
			db_free(&dbv);
		}

		setByte(hContact, "Auth", 1);
		setByte(hContact, "Grant", 1);

		if (isTemporary)
		{
			db_set_b(hContact, "CList", "NotOnList", 1);
		}
	}
	return hContact;
}

int32_t CToxProto::GetToxFriendNumber(MCONTACT hContact)
{
	ToxBinAddress pubKey = ptrA(getStringA(hContact, TOX_SETTINGS_ID));
	int32_t friendNumber = tox_get_friend_number(tox, pubKey);
	if (friendNumber == TOX_ERROR)
	{
		debugLogA("CToxProto::SendMsg: failed to get friend number");
	}
	return friendNumber;
}

void CToxProto::LoadFriendList(void*)
{
	uint32_t count = tox_count_friendlist(tox);
	if (count > 0)
	{
		int32_t *friends = (int32_t*)mir_alloc(count * sizeof(int32_t));
		tox_get_friendlist(tox, friends, count);

		uint8_t data[TOX_PUBLIC_KEY_SIZE];
		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t friendNumber = friends[i];
			tox_get_client_id(tox, friendNumber, data);
			ToxHexAddress pubKey(data, TOX_PUBLIC_KEY_SIZE);
			MCONTACT hContact = AddContact(pubKey, _T(""));
			if (hContact)
			{
				delSetting(hContact, "Auth");
				delSetting(hContact, "Grant");

				int size = tox_get_name_size(tox, friendNumber);
				if (size != TOX_ERROR)
				{
					std::string nick(size, 0);
					tox_get_name(tox, friendNumber, (uint8_t*)nick.data());
					setWString(hContact, "Nick", ptrT(Utf8DecodeT(nick.c_str())));
				}

				uint64_t timestamp = tox_get_last_online(tox, friendNumber);
				if (timestamp)
				{
					setDword(hContact, "LastEventDateTS", timestamp);
				}
			}
		}

		mir_free(friends);
	}
	else
	{
		debugLogA("CToxProto::LoadContactList: your friend list is empty");
	}
}

void CToxProto::OnFriendRequest(Tox *, const uint8_t *data, const uint8_t *message, const uint16_t messageSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	ToxHexAddress address(data, TOX_FRIEND_ADDRESS_SIZE);
	MCONTACT hContact = proto->AddContact(address, _T(""));
	if (!hContact)
	{
		proto->debugLogA("CToxProto::OnFriendRequest: failed to create contact");
	}

	proto->delSetting(hContact, "Auth");

	PROTORECVEVENT pre = { 0 };
	pre.flags = PREF_UTF;
	pre.timestamp = time(NULL);
	pre.lParam = (DWORD)(sizeof(DWORD) * 2 + address.GetLength() + messageSize + 5);

	/*blob is: 0(DWORD), hContact(DWORD), nick(ASCIIZ), firstName(ASCIIZ), lastName(ASCIIZ), id(ASCIIZ), reason(ASCIIZ)*/
	PBYTE pBlob, pCurBlob;
	pCurBlob = pBlob = (PBYTE)mir_calloc(pre.lParam);

	*((PDWORD)pCurBlob) = 0;
	pCurBlob += sizeof(DWORD);
	*((PDWORD)pCurBlob) = (DWORD)hContact;
	pCurBlob += sizeof(DWORD);
	pCurBlob += 3;
	mir_strcpy((char *)pCurBlob, address);
	pCurBlob += address.GetLength() + 1;
	mir_strcpy((char *)pCurBlob, (char*)message);
	pre.szMessage = (char*)pBlob;

	ProtoChainRecv(hContact, PSR_AUTH, 0, (LPARAM)&pre);
}

void CToxProto::OnFriendNameChange(Tox *, const int friendNumber, const uint8_t *name, const uint16_t, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact)
	{
		proto->setString(hContact, "Nick", (char*)name);
	}
}

void CToxProto::OnStatusMessageChanged(Tox *, const int friendNumber, const uint8_t* message, const uint16_t, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact)
	{
		ptrW statusMessage(mir_utf8decodeW((char*)message));
		db_set_ws(hContact, "CList", "StatusMsg", statusMessage);
	}
}

void CToxProto::OnUserStatusChanged(Tox *, int32_t friendNumber, uint8_t usertatus, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact)
	{
		TOX_USERSTATUS userstatus = (TOX_USERSTATUS)usertatus;
		int status = proto->ToxToMirandaStatus(userstatus);
		proto->SetContactStatus(hContact, status);
	}
}

void CToxProto::OnConnectionStatusChanged(Tox *tox, const int friendNumber, const uint8_t status, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact)
	{
		int newStatus = status ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
		proto->SetContactStatus(hContact, newStatus);
		if (status)
		{
			tox_send_avatar_info(proto->tox, friendNumber);
			proto->delSetting(hContact, "Auth");

			for (size_t i = 0; i < proto->transfers->Count(); i++)
			{
				// only for receiving
				FileTransferParam *transfer = proto->transfers->GetAt(i);
				if (transfer->friendNumber == friendNumber && transfer->GetDirection() == 1)
				{
					proto->debugLogA("CToxProto::OnConnectionStatusChanged: sending ask to resume the transfer of file (%d)", transfer->fileNumber);
					transfer->status = STARTED;
					if (tox_file_send_control(tox, friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_RESUME_BROKEN, (uint8_t*)&transfer->pfts.currentFileProgress, sizeof(transfer->pfts.currentFileProgress)) == TOX_ERROR)
					{
						tox_file_send_control(tox, friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
					}
				}
			}
		}
		else
		{
			for (size_t i = 0; i < proto->transfers->Count(); i++)
			{
				FileTransferParam *transfer = proto->transfers->GetAt(i);
				if (transfer->friendNumber == friendNumber)
				{
					transfer->status = BROKEN;
				}
			}
		}
	}
}

int CToxProto::OnUserInfoInit(WPARAM wParam, LPARAM lParam)
{
	if (!CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)m_szModuleName))
	{
		return 0;
	}

	MCONTACT hContact = lParam;
	char *szProto = GetContactProto(hContact);
	if (szProto != NULL && !strcmp(szProto, m_szModuleName))
	{
		OPTIONSDIALOGPAGE odp = { sizeof(odp) };
		odp.flags = ODPF_TCHAR | ODPF_DONTTRANSLATE;
		odp.hInstance = g_hInstance;
		odp.dwInitParam = (LPARAM)this;
		odp.ptszTitle = m_tszUserName;

		odp.pfnDlgProc = UserInfoProc;
		odp.position = -2000000000;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_USER_INFO);
		UserInfo_AddPage(wParam, &odp);
	}

	return 0;
}

INT_PTR CToxProto::UserInfoProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CToxProto *proto = (CToxProto*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			proto = (CToxProto*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom)
		{
		case 0:
			switch (((LPNMHDR)lParam)->code)
			{
			case PSN_INFOCHANGED:
			{
				MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
				char *szProto = (hContact == NULL) ? szProto = proto->m_szModuleName : (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, hContact, 0);
				if (szProto == NULL)
				{
					break;
				}

				SetDlgItemText(hwnd, IDC_DNS_ID, ptrT(proto->getTStringA(hContact, TOX_SETTINGS_DNS)));
			}
				break;

			case PSN_PARAMCHANGED:
				SetWindowLongPtr(hwnd, GWLP_USERDATA, ((PSHNOTIFY*)lParam)->lParam);
				break;

			case PSN_APPLY:
				MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
				char *szProto = (hContact == NULL) ? szProto = proto->m_szModuleName : (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, hContact, 0);
				if (szProto == NULL)
				{
					break;
				}
				TCHAR dnsId[MAX_PATH];
				GetDlgItemText(hwnd, IDC_DNS_ID, dnsId, MAX_PATH);
				proto->setTString(hContact, TOX_SETTINGS_DNS, dnsId);
				break;
			}
			break;
		}
		break;

	case WM_COMMAND:
		if ((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE)
		{
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		}
		break;

		break;
	}

	return FALSE;
}
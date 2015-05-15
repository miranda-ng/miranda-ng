#include "stdafx.h"

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
	dbei.cbBlob = sizeof(DWORD) * 2;
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
	TOX_ERR_FRIEND_GET_PUBLIC_KEY error;
	if (!tox_friend_get_public_key(tox, friendNumber, data, &error))
	{
		debugLogA(__FUNCTION__": failed to get friend public key (%d)", error);
		return NULL;
	}
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

MCONTACT CToxProto::AddContact(const char *address, const TCHAR *dnsId, bool isTemporary)
{
	MCONTACT hContact = GetContact(address);
	if (!hContact)
	{
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName);

		setString(hContact, TOX_SETTINGS_ID, address);

		if (dnsId && mir_tstrlen(dnsId))
		{
			setTString(hContact, TOX_SETTINGS_DNS, dnsId);
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

uint32_t CToxProto::GetToxFriendNumber(MCONTACT hContact)
{
	ToxBinAddress pubKey = ptrA(getStringA(hContact, TOX_SETTINGS_ID));
	TOX_ERR_FRIEND_BY_PUBLIC_KEY error;
	uint32_t friendNumber = tox_friend_by_public_key(tox, pubKey, &error);
	if (error != TOX_ERR_FRIEND_BY_PUBLIC_KEY_OK)
	{
		debugLogA(__FUNCTION__": failed to get friend number (%d)", error);
	}
	return friendNumber;
}

void CToxProto::LoadFriendList(void*)
{
	size_t count = tox_self_get_friend_list_size(tox);
	if (count > 0)
	{
		uint32_t *friends = (uint32_t*)mir_alloc(count * sizeof(uint32_t));
		tox_self_get_friend_list(tox, friends);

		uint8_t data[TOX_PUBLIC_KEY_SIZE];
		for (size_t i = 0; i < count; i++)
		{
			uint32_t friendNumber = friends[i];
			TOX_ERR_FRIEND_GET_PUBLIC_KEY getPublicKeyResult;
			if (!tox_friend_get_public_key(tox, friendNumber, data, &getPublicKeyResult))
			{
				debugLogA(__FUNCTION__": failed to get friend public key (%d)", getPublicKeyResult);
				continue;
			}
			ToxHexAddress pubKey(data, TOX_PUBLIC_KEY_SIZE);
			MCONTACT hContact = AddContact(pubKey, _T(""));
			if (hContact)
			{
				delSetting(hContact, "Auth");
				delSetting(hContact, "Grant");

				TOX_ERR_FRIEND_QUERY getNameResult;
				uint8_t nick[TOX_MAX_NAME_LENGTH] = { 0 };
				if (tox_friend_get_name(tox, friendNumber, nick, &getNameResult))
					setWString(hContact, "Nick", ptrT(mir_utf8decodeW((char*)nick)));
				else
					debugLogA(__FUNCTION__": failed to get friend name (%d)", getNameResult);

				TOX_ERR_FRIEND_GET_LAST_ONLINE getLastOnlineResult;
				uint64_t timestamp = tox_friend_get_last_online(tox, friendNumber, &getLastOnlineResult);
				if (getLastOnlineResult == TOX_ERR_FRIEND_GET_LAST_ONLINE_OK)
					setDword(hContact, "LastEventDateTS", timestamp);
				else
					debugLogA(__FUNCTION__": failed to get friend last online (%d)", getLastOnlineResult);
			}
		}
		mir_free(friends);
	}
}

INT_PTR CToxProto::OnRequestAuth(WPARAM hContact, LPARAM lParam)
{
	if (!IsOnline())
	{
		return -1; // ???
	}

	char *reason = lParam ? (char*)lParam : " ";
	size_t length = mir_strlen(reason);
	ToxBinAddress address(ptrA(getStringA(hContact, TOX_SETTINGS_ID)));

	TOX_ERR_FRIEND_ADD addFriendResult;
	int32_t friendNumber = tox_friend_add(tox, address, (uint8_t*)reason, length, &addFriendResult);
	if (addFriendResult != TOX_ERR_FRIEND_ADD_OK)
	{
		debugLogA(__FUNCTION__": failed to request auth (%d)", addFriendResult);
		return addFriendResult;
	}

	// trim address to public key
	setString(hContact, TOX_SETTINGS_ID, address.ToHex().GetPubKey());
	db_unset(hContact, "CList", "NotOnList");
	delSetting(hContact, "Grant");

	uint8_t nick[TOX_MAX_NAME_LENGTH] = { 0 };
	TOX_ERR_FRIEND_QUERY errorFriendQuery;
	if (tox_friend_get_name(tox, friendNumber, nick, &errorFriendQuery))
		setWString(hContact, "Nick", ptrT(mir_utf8decodeW((char*)nick)));
	else
		debugLogA(__FUNCTION__": failed to get friend name (%d)", errorFriendQuery);

	return 0;
}

INT_PTR CToxProto::OnGrantAuth(WPARAM hContact, LPARAM)
{
	if (!IsOnline())
	{
		// TODO: warn
		return 0;
	}

	ToxBinAddress pubKey(ptrA(getStringA(hContact, TOX_SETTINGS_ID)));
	TOX_ERR_FRIEND_ADD error;
	tox_friend_add_norequest(tox, pubKey, &error);
	if (error != TOX_ERR_FRIEND_ADD_OK)
	{
		debugLogA(__FUNCTION__": failed to grant auth (%d)", error);
		return error;
	}

	// trim address to public key
	// setString(hContact, TOX_SETTINGS_ID, pubKey.ToHex());
	db_unset(hContact, "CList", "NotOnList");
	delSetting(hContact, "Grant");

	SaveToxProfile();

	return 0;
}

int CToxProto::OnContactDeleted(MCONTACT hContact, LPARAM)
{
	if (!IsOnline())
	{
		// TODO: warn
		return 0;
	}

	if (!isChatRoom(hContact))
	{
		int32_t friendNumber = GetToxFriendNumber(hContact);
		TOX_ERR_FRIEND_DELETE error;
		if (!tox_friend_delete(tox, friendNumber, &error))
		{
			debugLogA(__FUNCTION__": failed to delete friend (%d)", error);
			return error;
		}
	}
	/*else
	{
	OnLeaveChatRoom(hContact, 0);
	int groupNumber = 0; // ???
	if (groupNumber == TOX_ERROR || tox_del_groupchat(tox, groupNumber) == TOX_ERROR)
	{
	return 1;
	}
	}*/

	return 0;
}

void CToxProto::OnFriendRequest(Tox*, const uint8_t *pubKey, const uint8_t *message, size_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	ToxHexAddress address(pubKey, TOX_ADDRESS_SIZE);
	MCONTACT hContact = proto->AddContact(address, _T(""));
	if (!hContact)
	{
		proto->debugLogA(__FUNCTION__": failed to create contact");
		return;
	}

	proto->delSetting(hContact, "Auth");

	PROTORECVEVENT pre = { 0 };
	pre.flags = PREF_UTF;
	pre.timestamp = time(NULL);
	pre.lParam = (DWORD)(sizeof(DWORD) * 2 + address.GetLength() + length + 5);

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

void CToxProto::OnFriendNameChange(Tox*, uint32_t friendNumber, const uint8_t *name, size_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	if (MCONTACT hContact = proto->GetContact(friendNumber))
	{
		ptrA rawName((char*)mir_alloc(length + 1));
		memcpy(rawName, name, length);
		rawName[length] = 0;

		ptrT nickname(mir_utf8decodeW(rawName));
		proto->setTString(hContact, "Nick", nickname);
	}
}

void CToxProto::OnStatusMessageChanged(Tox*, uint32_t friendNumber, const uint8_t *message, size_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	if (MCONTACT hContact = proto->GetContact(friendNumber))
	{
		ptrA rawMessage((char*)mir_alloc(length + 1));
		memcpy(rawMessage, message, length);
		rawMessage[length] = 0;

		ptrT statusMessage(mir_utf8decodeT(rawMessage));
		db_set_ts(hContact, "CList", "StatusMsg", statusMessage);
	}
}

void CToxProto::OnUserStatusChanged(Tox*, uint32_t friendNumber, TOX_USER_STATUS userstatus, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact)
	{
		int status = proto->ToxToMirandaStatus(userstatus);
		proto->SetContactStatus(hContact, status);
	}
}

void CToxProto::OnConnectionStatusChanged(Tox*, uint32_t friendNumber, TOX_CONNECTION status, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact)
	{
		if (status != TOX_CONNECTION_NONE)
		{
			proto->delSetting(hContact, "Auth");
			proto->delSetting(hContact, "Grant");

			// resume transfers
			for (size_t i = 0; i < proto->transfers.Count(); i++)
			{
				// only for receiving
				FileTransferParam *transfer = proto->transfers.GetAt(i);
				if (transfer->friendNumber == friendNumber && transfer->GetDirection() == 1)
				{
					proto->debugLogA(__FUNCTION__": sending ask to resume the transfer of file (%d)", transfer->fileNumber);
					TOX_ERR_FILE_CONTROL error;
					if (!tox_file_control(proto->tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_RESUME, &error))
					{
						proto->debugLogA(__FUNCTION__": failed to resume the transfer (%d)", error);
						tox_file_control(proto->tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_RESUME, NULL);
					}
				}
			}

			// update avatar
			std::tstring avatarPath = proto->GetAvatarFilePath();
			if (IsFileExists(avatarPath))
			{
				FILE *hFile = _tfopen(avatarPath.c_str(), L"rb");
				if (!hFile)
				{
					proto->debugLogA(__FUNCTION__": failed to open avatar file");
					return;
				}

				fseek(hFile, 0, SEEK_END);
				size_t length = ftell(hFile);
				rewind(hFile);

				uint8_t hash[TOX_HASH_LENGTH];
				DBVARIANT dbv;
				if (!db_get(NULL, proto->m_szModuleName, TOX_SETTINGS_AVATAR_HASH, &dbv))
				{
					memcpy(hash, dbv.pbVal, TOX_HASH_LENGTH);
					db_free(&dbv);
				}

				TOX_ERR_FILE_SEND error;
				uint32_t fileNumber = tox_file_send(proto->tox, friendNumber, TOX_FILE_KIND_AVATAR, length, hash, NULL, 0, &error);
				if (error != TOX_ERR_FILE_SEND_OK)
				{
					proto->debugLogA(__FUNCTION__": failed to set new avatar");
					return;
				}

				AvatarTransferParam *transfer = new AvatarTransferParam(friendNumber, fileNumber, NULL, length);
				memcpy(transfer->hash, hash, TOX_HASH_LENGTH);
				transfer->pfts.hContact = hContact;
				transfer->hFile = hFile;
				proto->transfers.Add(transfer);
			}
			else
				tox_file_send(proto->tox, friendNumber, TOX_FILE_KIND_AVATAR, 0, NULL, NULL, 0, NULL);
		}
		else
		{
			proto->SetContactStatus(hContact, ID_STATUS_OFFLINE);
			proto->setDword(hContact, "LastEventDateTS", time(NULL));

			/*for (size_t i = 0; i < proto->transfers.Count(); i++)
			{
				FileTransferParam *transfer = proto->transfers.GetAt(i);
				if (transfer->friendNumber == friendNumber)
					transfer->status = BROKEN;
			}*/
		}
	}
}

int CToxProto::OnUserInfoInit(WPARAM wParam, LPARAM lParam)
{
	if (!CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)m_szModuleName))
		return 0;

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
				char *szProto = (hContact == NULL) ? proto->m_szModuleName : GetContactProto(hContact);
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
				char *szProto = (hContact == NULL) ? proto->m_szModuleName : GetContactProto(hContact);
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
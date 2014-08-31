#include "common.h"

WORD CToxProto::GetContactStatus(MCONTACT hContact)
{
	return getWord(hContact, "Status", ID_STATUS_OFFLINE);
}

bool CToxProto::IsContactOnline(MCONTACT hContact)
{
	return GetContactStatus(hContact) == ID_STATUS_ONLINE;
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

MCONTACT CToxProto::GetContactFromAuthEvent(HANDLE hEvent)
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

MCONTACT CToxProto::FindContact(const std::string &id)
{
	MCONTACT hContact = NULL;
	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		std::string contactId = ToxAddressToId(getStringA(hContact, TOX_SETTINGS_ID));
		if (id == contactId)
		{
			break;
		}
	}
	return hContact;
}

MCONTACT CToxProto::FindContact(const int friendNumber)
{
	std::vector<uint8_t> clientId(TOX_CLIENT_ID_SIZE);
	tox_get_client_id(tox, friendNumber, &clientId[0]);
	std::string id = DataToHexString(clientId);

	return FindContact(id);
}

MCONTACT CToxProto::AddContact(const std::string &id, bool isTemporary)
{
	MCONTACT hContact = FindContact(id);
	if (!hContact)
	{
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName);

		setString(hContact, TOX_SETTINGS_ID, id.c_str());
		setByte(hContact, "Auth", 1);

		DBVARIANT dbv;
		if (!getTString(TOX_SETTINGS_GROUP, &dbv))
		{
			db_set_ts(hContact, "CList", "Group", dbv.ptszVal);
			db_free(&dbv);
		}

		if (isTemporary)
		{
			db_set_b(hContact, "CList", "NotOnList", 1);
		}
	}
	return hContact;
}

void CToxProto::LoadFriendList()
{
	uint32_t count = tox_count_friendlist(tox);
	if (count > 0)
	{
		int32_t *friends = (int32_t*)mir_alloc(count * sizeof(int32_t));
		tox_get_friendlist(tox, friends, count);

		std::vector<uint8_t> clientId(TOX_CLIENT_ID_SIZE);
		for (uint32_t i = 0; i < count; ++i)
		{
			tox_get_client_id(tox, friends[i], &clientId[0]);
			std::string id = DataToHexString(clientId);

			MCONTACT hContact = AddContact(id);
			if (hContact)
			{
				int size = tox_get_name_size(tox, friends[i]);
				std::vector<uint8_t> username(size);
				tox_get_name(tox, friends[i], &username[0]);
				std::string nick(username.begin(), username.end());
				setWString(hContact, "Nick", ptrW(Utf8DecodeW(nick.c_str())));

				uint64_t timestamp = tox_get_last_online(tox, friends[i]);
				if (timestamp)
				{
					setDword(hContact, "LastEventDateTS", timestamp);
				}
			}
		}
	}
	else
	{
		debugLogA("CToxProto::LoadContactList: your friend list is empty");
	}
}

int CToxProto::OnContactDeleted(MCONTACT hContact, LPARAM lParam)
{
	if (hContact)
	{
		std::string toxId(getStringA(hContact, TOX_SETTINGS_ID));
		std::vector<uint8_t> clientId = HexStringToData(toxId);

		uint32_t number = tox_get_friend_number(tox, clientId.data());
		if (tox_del_friend(tox, number) == 0)
		{
			SaveToxData();

			return 0;
		}
	}

	return 1;
}

void CToxProto::OnFriendRequest(Tox *tox, const uint8_t *address, const uint8_t *message, const uint16_t messageSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	// trim tox address to tox id
	std::vector<uint8_t> clientId(address, address + TOX_CLIENT_ID_SIZE);
	std::string id = proto->DataToHexString(clientId);

	MCONTACT hContact = proto->AddContact(id, true);

	PROTORECVEVENT pre = { 0 };
	pre.flags = PREF_UTF;
	pre.timestamp = time(NULL);
	pre.lParam = (DWORD)(sizeof(DWORD)* 2 + id.length() + messageSize + 5);

	/*blob is: 0(DWORD), hContact(DWORD), nick(ASCIIZ), firstName(ASCIIZ), lastName(ASCIIZ), id(ASCIIZ), reason(ASCIIZ)*/
	PBYTE pBlob, pCurBlob;
	pCurBlob = pBlob = (PBYTE)mir_calloc(pre.lParam);

	*((PDWORD)pCurBlob) = 0;
	pCurBlob += sizeof(DWORD);
	*((PDWORD)pCurBlob) = (DWORD)hContact;
	pCurBlob += sizeof(DWORD);
	pCurBlob += 3;
	strcpy((char *)pCurBlob, id.c_str());
	pCurBlob += id.length() + 1;
	strcpy((char *)pCurBlob, (char*)message);
	pre.szMessage = (char*)pBlob;

	ProtoChainRecv(hContact, PSR_AUTH, 0, (LPARAM)&pre);
}

void CToxProto::OnFriendNameChange(Tox *tox, const int number, const uint8_t *name, const uint16_t nameSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		proto->setString(hContact, "Nick", (char*)name);
	}
}

void CToxProto::OnStatusMessageChanged(Tox *tox, const int number, const uint8_t* message, const uint16_t messageSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		ptrW statusMessage(mir_utf8decodeW((char*)message));
		db_set_ws(hContact, "CList", "StatusMsg", statusMessage);
	}
}

void CToxProto::OnUserStatusChanged(Tox *tox, int32_t number, uint8_t usertatus, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		TOX_USERSTATUS userstatus = (TOX_USERSTATUS)usertatus;
		int status = proto->ToxToMirandaStatus(userstatus);
		proto->SetContactStatus(hContact, status);
	}
}

void CToxProto::OnConnectionStatusChanged(Tox *tox, const int number, const uint8_t status, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		int newStatus = status ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
		proto->SetContactStatus(hContact, newStatus);
	}
}
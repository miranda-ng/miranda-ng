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


bool CToxProto::IsProtoContact(MCONTACT hContact)
{
	return lstrcmpiA(GetContactProto(hContact), m_szModuleName) == 0;
}

MCONTACT CToxProto::FindContact(const std::string &id)
{
	MCONTACT hContact = NULL;

	//mir_cs(contact_search_lock);

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

void CToxProto::LoadContactList()
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

void CToxProto::SearchByIdAsync(void*)
{
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HWND)1, 0);
}

void CToxProto::SearchByNameAsync(void* arg)
{
	NETLIBHTTPREQUEST request = { sizeof(NETLIBHTTPREQUEST) };
	request.requestType = REQUEST_POST;
	request.szUrl = "https://toxme.se/api";
	request.flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

	request.headers = (NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER)* 2);
	request.headers[0].szName = "Content-Type";
	request.headers[0].szValue = "text/plain; charset=utf-8";
	request.headersCount = 1;

	std::string query = "{\"action\":3,\"name\":\"";
	query += (char*)arg;
	query += "\"}";

	request.dataLength = query.length();
	request.pData = (char*)query.c_str();

	NETLIBHTTPREQUEST* response = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser, (LPARAM)&request);

	if (response)
	{
		std::smatch match;
		std::regex regex("\"public_key\": \"(.+?)\"");

		const std::string content = response->pData;

		if (std::regex_search(content, match, regex))
		{
			std::string toxId = match[1];

			PROTOSEARCHRESULT psr = { sizeof(PROTOSEARCHRESULT) };
			psr.flags = PSR_TCHAR;
			psr.id = mir_a2t(toxId.c_str());

			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);

			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
			return;
		}
	}

	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
	mir_free(request.headers);

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)1, 0);
	mir_free(arg);
}
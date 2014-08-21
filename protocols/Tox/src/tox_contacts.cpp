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

	if (::db_event_get(hEvent, &dbei))
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

MCONTACT CToxProto::FindContact(const char *clientId)
{
	MCONTACT hContact = NULL;

	//EnterCriticalSection(&contact_search_lock);

	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		ptrA contactId(getStringA(hContact, TOX_SETTINGS_ID));
		if (lstrcmpiA(contactId, clientId) == 0)
			break;
	}

	//LeaveCriticalSection(&contact_search_lock);

	return hContact;
}

MCONTACT CToxProto::AddContact(const char *clientId, bool isTemporary)
{
	MCONTACT hContact = FindContact(clientId);
	if (!hContact)
	{
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName);

		if (isTemporary)
		{
			db_set_b(hContact, "CList", "NotOnList", 1);
			db_set_b(hContact, "CList", "Auth", 1);
		}

		setString(hContact, TOX_SETTINGS_ID, clientId);

		DBVARIANT dbv;
		if (!getTString(TOX_SETTINGS_GROUP, &dbv))
		{
			db_set_ts(hContact, "CList", "Group", dbv.ptszVal);
			db_free(&dbv);
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
			std::string toxId = DataToHexString(clientId);

			MCONTACT hContact = AddContact(toxId.c_str());
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
}

void CToxProto::SearchByIdAsync(void* arg)
{
	std::string clientId = mir_utf8encodeT((TCHAR*)arg);
	clientId.erase(clientId.begin() + TOX_CLIENT_ID_SIZE * 2, clientId.end());

	std::string toxId = clientId;
	MCONTACT hContact = FindContact(clientId.c_str());
	if (hContact)
	{
		ShowNotification(TranslateT("Contact already in your contact list"), 0, hContact);
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)1, 0);
}

void CToxProto::SearchByNameAsync(void* arg)
{
	NETLIBHTTPREQUEST request = { sizeof(NETLIBHTTPREQUEST) };
	request.requestType = REQUEST_POST;
	request.szUrl = "https://toxme.se/api";
	request.flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;
	
	request.headers = (NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER)*2);
	request.headers[0].szName = mir_strdup("Content-Type");
	request.headers[0].szValue = mir_strdup("text/plain; charset=utf-8");
	request.headersCount = 1;

	char data[128];
	ptrA search(mir_utf8encodeT((TCHAR*)arg));
	ptrA searchEncoded(mir_urlEncode(search));
	mir_snprintf(data, SIZEOF(data), "{\"action\":3,\"name\":\"%s\"}", searchEncoded);

	request.dataLength = strlen(data);
	request.pData = (char*)mir_alloc(request.dataLength + 1);
	memcpy(request.pData, data, request.dataLength);
	request.pData[request.dataLength] = 0;

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
	mir_free(request.pData);
	mir_free(request.headers[0].szName);
	mir_free(request.headers[0].szValue);
	mir_free(request.headers);

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)1, 0);
}
#include "common.h"

void CToxProto::SetContactStatus(MCONTACT hContact, WORD status)
{
	WORD oldStatus = getWord(hContact, "Status", ID_STATUS_OFFLINE);
	if (oldStatus != status)
	{
		setWord(hContact, "Status", status);
	}
}

void CToxProto::SetAllContactsStatus(WORD status)
{
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		setWord(hContact, "Status", status);
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
		ptrA contactId(getStringA(hContact, TOX_SETTING_ID));
		if (lstrcmpiA(contactId, clientId) == 0)
			break;
	}

	//LeaveCriticalSection(&contact_search_lock);

	return hContact;
}

MCONTACT CToxProto::AddContact(const char *clientId, bool isHidden)
{
	MCONTACT hContact = FindContact(clientId);
	if (!hContact)
	{
		hContact = (MCONTACT)::CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName);

		/*db_set_b(hContact, "CList", "NotOnList", 1);

		if (isHidden)
		{
			db_set_b(hContact, "CList", "Hidden", 1);
		}*/

		setString(hContact, TOX_SETTING_ID, clientId);
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
		std::vector<uint8_t> username(TOX_MAX_NAME_LENGTH);
		for (uint32_t i = 0; i < count; ++i)
		{
			tox_get_client_id(tox, friends[i], &clientId[0]);
			std::string toxId = DataToHexString(clientId);

			MCONTACT hContact = AddContact(toxId.c_str());
		}
	}
}

void __cdecl CToxProto::SearchByUidAsync(void* arg)
{
	ptrW userId((wchar_t*)arg);

	//MCONTACT hContact = GetContactByUserId(userId);
	//if (hContact)
	//{
	//	ShowNotification(TranslateT("Contact already in your contact list"), 0, hContact);
	//	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)TOX_SEARCH_BYUID, 0);
	//	return;
	//}

	//// there will be placed code 
	//// that will search for contact by userId
	// ...
	// ...
	//// and call
	PROTOSEARCHRESULT psr = {0};
	psr.cbSize = sizeof(psr);
	//// get user id [and nick]
	//psr.id = userId;
	//psr.nick  = nick;

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)TOX_SEARCH_BYUID, (LPARAM)&psr);
}
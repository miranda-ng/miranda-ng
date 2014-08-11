#include "common.h"

bool CToxProto::IsProtoContact(MCONTACT hContact)
{
	return ::lstrcmpiA(::GetContactProto(hContact), m_szModuleName) == 0;
}

MCONTACT CToxProto::GetContactByUserId(const char *clientId)
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

MCONTACT CToxProto::AddContact(const char *clientId, const char *nick, bool isHidden)
{
	MCONTACT hContact = GetContactByUserId(clientId);
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
		setString(hContact, "Nick", nick);
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
			tox_get_name(tox, friends[i], &username[0]);
			std::string nick(username.begin(), username.end());

			AddContact(toxId.c_str(), nick.c_str());
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
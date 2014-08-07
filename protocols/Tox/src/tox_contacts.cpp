#include "common.h"

bool CToxProto::IsProtoContact(MCONTACT hContact)
{
	return ::lstrcmpiA(::GetContactProto(hContact), m_szModuleName) == 0;
}

MCONTACT CToxProto::GetContactByUserId(const wchar_t *userId)
{
	MCONTACT hContact = NULL;

	//EnterCriticalSection(&contact_search_lock);

	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		ptrW cUserId(::db_get_wsa(hContact, m_szModuleName, "UserID"));
		if (lstrcmpi(cUserId, userId) == 0)
			break;
	}

	//LeaveCriticalSection(&contact_search_lock);

	return hContact;
}

MCONTACT CToxProto::AddContact(const wchar_t *userId, const wchar_t *nick, bool isHidden)
{
	MCONTACT hContact = GetContactByUserId(userId);
	if ( !hContact)
	{
		hContact = (MCONTACT)::CallService(MS_DB_CONTACT_ADD, 0, 0);
		::CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName);

		db_set_b(hContact, "CList", "NotOnList", 1);

		if (isHidden)
			db_set_b(hContact, "CList", "Hidden", 1);

		setWString(hContact, "UserId", userId);
		setWString(hContact, "Nick", nick);
	}

	return hContact;
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
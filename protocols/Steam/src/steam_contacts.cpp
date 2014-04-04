#include "common.h"

MCONTACT CSteamProto::FindContact(const char *steamId)
{
	MCONTACT hContact = NULL;

	EnterCriticalSection(&this->contact_search_lock);

	for (hContact = db_find_first(this->m_szModuleName); hContact; hContact = db_find_next(hContact, this->m_szModuleName))
	{
		ptrA cSteamId(db_get_sa(hContact, this->m_szModuleName, "SteamID"));
		if (!lstrcmpA(cSteamId, steamId))
			break;
	}

	LeaveCriticalSection(&this->contact_search_lock);

	return hContact;
}

MCONTACT CSteamProto::AddContact(const SteamWebApi::FriendApi::Friend &contact)
{
	MCONTACT hContact = this->FindContact(contact.GetSteamId());
	if (!hContact)
	{
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)this->m_szModuleName);

		this->setString(hContact, "SteamID", contact.GetSteamId());
		this->setWString(hContact, "Nick", contact.GetNickname());
		this->setString(hContact, "Homepage", contact.GetHomepage());
		this->setDword(hContact, "LastEventDateTS", contact.GetLastEvent());

		DBVARIANT dbv;
		if (!getWString("DefaultGroup", &dbv))
		{
			db_set_ts(hContact, "CList", "Group", dbv.ptszVal);
			db_free(&dbv);
		}
	}

	return hContact;
}
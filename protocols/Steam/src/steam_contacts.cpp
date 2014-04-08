#include "common.h"

void CSteamProto::SetContactStatus(MCONTACT hContact, WORD status)
{
	WORD oldStatus = getWord(hContact, "Status", ID_STATUS_OFFLINE);
	if (oldStatus != status)
		setWord(hContact, "Status", status);
}

void CSteamProto::SetAllContactsStatus(WORD status)
{
	for (MCONTACT hContact = db_find_first(this->m_szModuleName); hContact; hContact = db_find_next(hContact, this->m_szModuleName))
	{
		if (this->isChatRoom(hContact))
			continue;
		//if (this->IsContactOnline(hContact))
		setWord(hContact, "Status", status);
	}
}

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

		setString(hContact, "SteamID", contact.GetSteamId());
		setWString(hContact, "Nick", contact.GetNickname());
		setString(hContact, "Homepage", contact.GetHomepage());
		setDword(hContact, "LastEventDateTS", contact.GetLastEvent());
		db_set_ws(hContact, "CList", "MyHandle", contact.GetNickname());

		DBVARIANT dbv;
		if (!getWString("DefaultGroup", &dbv))
		{
			db_set_ts(hContact, "CList", "Group", dbv.ptszVal);
			db_free(&dbv);
		}
	}

	return hContact;
}

void CSteamProto::SearchByIdThread(void* arg)
{
	ptrW steamIdW((wchar_t*)arg);
	ptrA steamId(mir_u2a(steamIdW));

	ptrA token(getStringA("TokenSecret"));

	SteamWebApi::FriendApi::Friend rFriend;
	SteamWebApi::FriendApi::LoadSummaries(m_hNetlibUser, token, steamId, &rFriend);

	if (!rFriend.IsSuccess())
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)STEAM_SEARCH_BYID, 0);
		return;
	}

	PROTOSEARCHRESULT psr = { 0 };
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_TCHAR;

	psr.id = mir_wstrdup(steamIdW);
	psr.nick  = mir_wstrdup(rFriend.GetNickname());

	const wchar_t *realname = rFriend.GetRealname();
	const wchar_t *p = wcschr(realname, ' ');
	if (p == NULL)
		psr.firstName = mir_wstrdup(realname);
	else
	{
		int length = p - realname;
		psr.firstName = (wchar_t*)mir_alloc(sizeof(wchar_t) * (length + 1));
		wmemcpy(psr.firstName, realname, length);
		psr.firstName[length] = '\0';
		psr.lastName = mir_wstrdup(p + 1);
	}

	//psr.reserved[0] = &psr;

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)STEAM_SEARCH_BYID, (LPARAM)&psr);
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)STEAM_SEARCH_BYID, 0);
}
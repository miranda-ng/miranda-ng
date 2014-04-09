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

void CSteamProto::UpdateContact(MCONTACT hContact, const SteamWebApi::FriendApi::Summary *contact)
{
	if (hContact && !FindContact(contact->GetSteamId()))
		return;

	// set common data
	setWString(hContact, "Nick", contact->GetNickname());
	setWord(hContact, "Status", SteamToMirandaStatus(contact->GetState()));
	setString(hContact, "Homepage", contact->GetHomepage());
	setDword(hContact, "LastEventDateTS", contact->GetLastEvent());

	// set name
	ptrW realname(mir_wstrdup(contact->GetRealname()));
	const wchar_t *p = wcschr(realname, ' ');
	if (p == NULL)
		setWString(hContact, "FirstName", realname);
	else
	{
		int length = p - (wchar_t*)realname;
		realname[length] = '\0';
		setWString(hContact, "LastName", realname);
		setWString(hContact, "LastName", p + 1);
	}

	// set country
	const char *isoCode = contact->GetCountryCode();
	if (!lstrlenA(isoCode))
		this->delSetting(hContact, "Country");
	else
	{
		// todo: is should be free()?
		char *country = (char *)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)isoCode, 0);
		setString(hContact, "Country", country);
	}
}

void CSteamProto::UpdateContactsThread(void *arg)
{
	ptrA steamIds((char*)arg);

	ptrA token(getStringA("TokenSecret"));

	SteamWebApi::FriendApi::Summaries summarues;
	SteamWebApi::FriendApi::LoadSummaries(m_hNetlibUser, token, steamIds, &summarues);

	if (!summarues.IsSuccess())
		return;

	for (int i = 0; i < summarues.GetItemCount(); i++)
	{
		const SteamWebApi::FriendApi::Summary *contact = summarues.GetAt(i);
		
		MCONTACT hContact = this->FindContact(contact->GetSteamId());
		if (!hContact)
		{
			UpdateContact(hContact, contact);
		}
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

MCONTACT CSteamProto::AddContact(const SteamWebApi::FriendApi::Summary *contact)
{
	MCONTACT hContact = this->FindContact(contact->GetSteamId());
	if (!hContact)
	{
		// create contact
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)this->m_szModuleName);

		setString(hContact, "SteamID", contact->GetSteamId());

		// update info
		UpdateContact(hContact, contact);

		// move to default group
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

	SteamWebApi::FriendApi::Summaries summarues;
	SteamWebApi::FriendApi::LoadSummaries(m_hNetlibUser, token, steamId, &summarues);

	if (!summarues.IsSuccess())
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)STEAM_SEARCH_BYID, 0);
		return;
	}

	if (summarues.GetItemCount() == 0)
		return;

	const SteamWebApi::FriendApi::Summary *contact = summarues.GetAt(0);

	STEAM_SEARCH_RESULT ssr = { 0 };
	ssr.hdr.cbSize = sizeof(STEAM_SEARCH_RESULT);
	ssr.hdr.flags = PSR_TCHAR;
	
	ssr.hdr.id = mir_wstrdup(steamIdW);
	ssr.hdr.nick  = mir_wstrdup(contact->GetNickname());

	const wchar_t *realname = contact->GetRealname();
	const wchar_t *p = wcschr(realname, ' ');
	if (p == NULL)
		ssr.hdr.firstName = mir_wstrdup(realname);
	else
	{
		int length = p - realname;
		ssr.hdr.firstName = (wchar_t*)mir_alloc(sizeof(wchar_t) * (length + 1));
		wmemcpy(ssr.hdr.firstName, realname, length);
		ssr.hdr.firstName[length] = '\0';
		ssr.hdr.lastName = mir_wstrdup(p + 1);
	}
	
	ssr.contact = contact;

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)STEAM_SEARCH_BYID, (LPARAM)&ssr);
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)STEAM_SEARCH_BYID, 0);
}

void CSteamProto::SearchByNameThread(void* arg)
{
	ptrW keywordsW((wchar_t*)arg);
	ptrA keywords(mir_utf8encodeW(keywordsW));

	ptrA token(getStringA("TokenSecret"));

	SteamWebApi::SearchApi::SearchResult searchResult;
	SteamWebApi::SearchApi::Search(m_hNetlibUser, token, keywords, &searchResult);

	if (!searchResult.IsSuccess())
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)STEAM_SEARCH_BYNAME, 0);
		return;
	}
	
	CMStringA steamIds;
	for (int i = 0; i < searchResult.GetItemCount(); i++)
	{
		const SteamWebApi::SearchApi::SearchItem *item = searchResult.GetAt(i);
		if (steamIds.IsEmpty())
			steamIds.Append(item->GetSteamId());
		else
			steamIds.AppendFormat(",%s", item->GetSteamId());
	}

	SteamWebApi::FriendApi::Summaries summarues;
	SteamWebApi::FriendApi::LoadSummaries(m_hNetlibUser, token, steamIds, &summarues);

	if (!summarues.IsSuccess())
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)STEAM_SEARCH_BYNAME, 0);
		return;
	}

	for (int i = 0; i < summarues.GetItemCount(); i++)
	{
		const SteamWebApi::FriendApi::Summary *contact = summarues.GetAt(i);

		STEAM_SEARCH_RESULT ssr = { 0 };
		ssr.hdr.cbSize = sizeof(STEAM_SEARCH_RESULT);
		ssr.hdr.flags = PSR_TCHAR;

		ssr.hdr.id = mir_a2u(contact->GetSteamId());
		ssr.hdr.nick  = mir_wstrdup(contact->GetNickname());

		const wchar_t *realname = contact->GetRealname();
		const wchar_t *p = wcschr(realname, ' ');
		if (p == NULL)
			ssr.hdr.firstName = mir_wstrdup(realname);
		else
		{
			int length = p - realname;
			ssr.hdr.firstName = (wchar_t*)mir_alloc(sizeof(wchar_t) * (length + 1));
			wmemcpy(ssr.hdr.firstName, realname, length);
			ssr.hdr.firstName[length] = '\0';
			ssr.hdr.lastName = mir_wstrdup(p + 1);
		}

		ssr.contact = contact;

		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)STEAM_SEARCH_BYNAME, (LPARAM)&ssr);
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)STEAM_SEARCH_BYNAME, 0);
}
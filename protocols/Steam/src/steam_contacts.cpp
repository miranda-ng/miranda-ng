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

void CSteamProto::UpdateContact(MCONTACT hContact, const SteamWebApi::FriendApi::Summary *contact)
{
	// only if contact is in contact list
	if (hContact && !FindContact(contact->GetSteamId()))
		return;

	// set common data
	setWString(hContact, "Nick", contact->GetNickname());
	setString(hContact, "Homepage", contact->GetHomepage());
	// only for contacts
	if (hContact)
	{
		WORD status = SteamToMirandaStatus(contact->GetState());
		setWord(hContact, "Status", status);
		setDword(hContact, "LastEventDateTS", contact->GetLastEvent());

		if (status == ID_STATUS_OUTTOLUNCH)
		{
			db_set_ws(hContact, "CList", "StatusMsg", contact->GetGameInfo());
			setDword(hContact, "GameID", contact->GetGameId());
		}
		else
		{
			db_unset(hContact, "CList", "StatusMsg");
			delSetting(hContact, "GameID");
		}
	}

	// set name
	ptrW realname(mir_wstrdup(contact->GetRealname()));
	const wchar_t *p = wcschr(realname, ' ');
	if (p == NULL)
	{
		setWString(hContact, "FirstName", realname);
		delSetting(hContact, "LastName");
	}
	else
	{
		int length = p - (wchar_t*)realname;
		realname[length] = '\0';
		setWString(hContact, "FirstName", realname);
		setWString(hContact, "LastName", p + 1);
	}

	// avatar
	ptrA oldAvatar(getStringA("AvatarUrl"));
	if (lstrcmpiA(oldAvatar, contact->GetAvatarUrl()))
	{
		// todo: need to place in thread
		SteamWebApi::AvatarApi::Avatar avatar;
		debugLogA("CSteamProto::UpdateContact: SteamWebApi::AvatarApi::GetAvatar");
		SteamWebApi::AvatarApi::GetAvatar(m_hNetlibUser, contact->GetAvatarUrl(), &avatar);

		if (avatar.IsSuccess() && avatar.GetDataSize() > 0)
		{
			ptrW avatarPath(GetAvatarFilePath(hContact));
			FILE *fp = _wfopen(avatarPath, L"wb");
			if (fp)
			{
				fwrite(avatar.GetData(), sizeof(char), avatar.GetDataSize(), fp);
				fclose(fp);

				PROTO_AVATAR_INFORMATIONW pai = { sizeof(pai) };
				pai.format = PA_FORMAT_JPEG;
				pai.hContact = hContact;
				wcscpy(pai.filename, avatarPath);

				ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, 0);

				setString("AvatarUrl", contact->GetAvatarUrl());
			}
		}
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
	debugLogA("CSteamProto::UpdateContactsThread: call SteamWebApi::FriendApi::LoadSummaries");
	SteamWebApi::FriendApi::LoadSummaries(m_hNetlibUser, token, steamIds, &summarues);

	if (!summarues.IsSuccess())
		return;

	for (size_t i = 0; i < summarues.GetItemCount(); i++)
	{
		const SteamWebApi::FriendApi::Summary *contact = summarues.GetAt(i);
		
		MCONTACT hContact = NULL;
		if (!IsMe(contact->GetSteamId()))
		{
			hContact = this->FindContact(contact->GetSteamId());
			if (hContact == NULL)
				return;
		}

		UpdateContact(hContact, contact);
	}
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

void CSteamProto::LoadContactListThread(void*)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA steamId(getStringA("SteamID"));

	SteamWebApi::FriendListApi::FriendList friendList;
	debugLogA("CSteamProto::LoadContactListThread: call SteamWebApi::FriendListApi::Load");
	SteamWebApi::FriendListApi::Load(m_hNetlibUser, token, steamId, &friendList);
	
	if (friendList.IsSuccess())
	{
		CMStringA newContacts;
		for (size_t i = 0; i < friendList.GetItemCount(); i++)
		{
			const char * steamId = friendList.GetAt(i);
			if (!FindContact(steamId))
			{
				if (newContacts.IsEmpty())
					newContacts.Append(steamId);
				else
					newContacts.AppendFormat(",%s", steamId);
			}
		}

		if (!newContacts.IsEmpty())
		{
			SteamWebApi::FriendApi::Summaries summaries;
			debugLogA("CSteamProto::LoadContactListThread: call SteamWebApi::FriendApi::LoadSummaries");
			SteamWebApi::FriendApi::LoadSummaries(m_hNetlibUser, token, newContacts, &summaries);

			if (summaries.IsSuccess())
			{
				for (size_t i = 0; i < summaries.GetItemCount(); i++)
				{
					const SteamWebApi::FriendApi::Summary *summary = summaries.GetAt(i);
					AddContact(summary);
				}
			}
		}
	}
}

void CSteamProto::SearchByIdThread(void* arg)
{
	ptrW steamIdW((wchar_t*)arg);
	ptrA steamId(mir_u2a(steamIdW));

	ptrA token(getStringA("TokenSecret"));

	SteamWebApi::FriendApi::Summaries summarues;
	debugLogA("CSteamProto::SearchByIdThread: call SteamWebApi::FriendApi::LoadSummaries");
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
	debugLogA("CSteamProto::SearchByNameThread: call SteamWebApi::SearchApi::Search");
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
	debugLogA("CSteamProto::SearchByNameThread: call SteamWebApi::FriendApi::LoadSummaries");
	SteamWebApi::FriendApi::LoadSummaries(m_hNetlibUser, token, steamIds, &summarues);

	if (!summarues.IsSuccess())
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)STEAM_SEARCH_BYNAME, 0);
		return;
	}

	for (size_t i = 0; i < summarues.GetItemCount(); i++)
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
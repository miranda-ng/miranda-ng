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

MCONTACT CSteamProto::GetContactFromAuthEvent(HANDLE hEvent)
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

void CSteamProto::UpdateContact(MCONTACT hContact, const SteamWebApi::FriendApi::Summary *summary)
{
	// only if contact is in contact list
	if (hContact && !FindContact(summary->GetSteamId()))
		return;

	// set common data
	setWString(hContact, "Nick", summary->GetNickName());
	setString(hContact, "Homepage", summary->GetHomepage());
	// only for contacts
	if (hContact)
	{
		setDword(hContact, "LastEventDateTS", summary->GetLastEvent());

		DWORD gameId = summary->GetGameId();
		if (gameId >0)
		{
			setWord(hContact, "Status", ID_STATUS_OUTTOLUNCH);
			db_set_ws(hContact, "CList", "StatusMsg", summary->GetGameInfo());

			setWString(hContact, "GameInfo", summary->GetGameInfo());
			setDword(hContact, "GameID", summary->GetGameId());
		}
		else
		{
			WORD status = SteamToMirandaStatus(summary->GetState());
			setWord(hContact, "Status", status);

			db_unset(hContact, "CList", "StatusMsg");
			delSetting(hContact, "GameID");
		}
	}

	// set name
	const wchar_t *firstName = summary->GetFirstName();
	const wchar_t *lastName = summary->GetLastName();
	if (lstrlen(firstName) == 0)
	{
		delSetting(hContact, "FirstName");
		delSetting(hContact, "LastName");
	}
	else if (lstrlen(lastName) == 0)
	{
		setWString(hContact, "FirstName", firstName);
		delSetting(hContact, "LastName");
	}
	else
	{
		setWString(hContact, "FirstName", firstName);
		setWString(hContact, "LastName", lastName);
	}

	// avatar
	ptrA oldAvatar(getStringA("AvatarUrl"));
	if (lstrcmpiA(oldAvatar, summary->GetAvatarUrl()))
	{
		// todo: need to place in thread
		SteamWebApi::AvatarApi::Avatar avatar;
		debugLogA("CSteamProto::UpdateContact: SteamWebApi::AvatarApi::GetAvatar");
		SteamWebApi::AvatarApi::GetAvatar(m_hNetlibUser, summary->GetAvatarUrl(), &avatar);

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

				setString("AvatarUrl", summary->GetAvatarUrl());
			}
		}
	}

	// set country
	const char *isoCode = summary->GetCountryCode();
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
			const char *steamId = contact->GetSteamId();
			hContact = this->FindContact(steamId);
			if (hContact == NULL)
				hContact = AddContact(steamId);
			if (hContact == NULL)
				return;
		}

		UpdateContact(hContact, contact);
	}
}

MCONTACT CSteamProto::AddContact(const char *steamId)
{
	MCONTACT hContact = this->FindContact(steamId);
	if (!hContact)
	{
		// create contact
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)this->m_szModuleName);

		setString(hContact, "SteamID", steamId);

		// update info
		//UpdateContact(hContact, contact);

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

void CSteamProto::RaiseAuthRequestThread(void *arg)
{
	MCONTACT hContact = (MCONTACT)arg;
	if (!hContact)
		debugLogA("CSteamProto::RaiseAuthRequestThread: error (contact is NULL)");

	ptrA token(getStringA("TokenSecret"));
	ptrA steamId(getStringA(hContact, "SteamID"));

	SteamWebApi::FriendApi::Summaries summaries;
	debugLogA("CSteamProto::RaiseAuthRequestThread: call SteamWebApi::FriendApi::LoadSummaries");
	SteamWebApi::FriendApi::LoadSummaries(m_hNetlibUser, token, steamId, &summaries);

	if (summaries.IsSuccess())
	{
		const SteamWebApi::FriendApi::Summary *summary = summaries.GetAt(0);

		UpdateContact(hContact, summary);

		char *nickName = mir_utf8encodeW(summary->GetNickName());
		char *firstName = mir_utf8encodeW(summary->GetFirstName());
		char *lastName = mir_utf8encodeW(summary->GetLastName());
		char reason[MAX_PATH];
		mir_snprintf(reason, SIZEOF(reason), Translate("%s has added you to his or her Friend List"), nickName);

		// blob is: 0(DWORD), hContact(DWORD), nick(ASCIIZ), firstName(ASCIIZ), lastName(ASCIIZ), sid(ASCIIZ), reason(ASCIIZ)
		DWORD cbBlob = (DWORD)(sizeof(DWORD)* 2 + strlen(nickName) + strlen(firstName) + strlen(lastName) + strlen(steamId) + strlen(reason) + 5);

		PBYTE pBlob, pCurBlob;
		pCurBlob = pBlob = (PBYTE)mir_alloc(cbBlob);

		*((PDWORD)pCurBlob) = 0;
		pCurBlob += sizeof(DWORD);
		*((PDWORD)pCurBlob) = (DWORD)hContact;
		pCurBlob += sizeof(DWORD);
		strcpy((char*)pCurBlob, nickName);
		pCurBlob += strlen(nickName) + 1;
		strcpy((char*)pCurBlob, firstName);
		pCurBlob += strlen(firstName) + 1;
		strcpy((char*)pCurBlob, lastName);
		pCurBlob += strlen(lastName) + 1;
		strcpy((char*)pCurBlob, steamId);
		pCurBlob += strlen(steamId) + 1;
		strcpy((char*)pCurBlob, mir_strdup(reason));

		AddDBEvent(hContact, EVENTTYPE_AUTHREQUEST, time(NULL), DBEF_UTF, cbBlob, pBlob);
	}
}

void CSteamProto::AuthAllowThread(void *arg)
{
	MCONTACT hContact = (MCONTACT)arg;
	if (!hContact)
		return;

	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));
	ptrA steamId(getStringA("SteamID"));
	ptrA who(getStringA(hContact, "SteamID"));

	SteamWebApi::InvitationApi::Result result;
	debugLogA("CSteamProto::AuthAllowThread: call SteamWebApi::InvitationApi::Accept");
	SteamWebApi::InvitationApi::Accept(m_hNetlibUser, token, sessionId, steamId, who, &result);

	if (result.IsSuccess())
	{
		delSetting(hContact, "Auth");
		delSetting(hContact, "Grant");
	}
}

void CSteamProto::AuthDenyThread(void *arg)
{
	MCONTACT hContact = (MCONTACT)arg;
	if (!hContact)
		return;

	ptrA sessionId(getStringA("SessionID"));
	ptrA steamId(getStringA("SteamID"));
	ptrA who(getStringA(hContact, "SteamID"));

	SteamWebApi::InvitationApi::Result result;
	debugLogA("CSteamProto::AuthDenyThread: call SteamWebApi::InvitationApi::Ignore");
	SteamWebApi::InvitationApi::Ignore(m_hNetlibUser, sessionId, steamId, who, &result);
}

void CSteamProto::AddContactThread(void *arg)
{
}

void CSteamProto::RemoveContactThread(void *arg)
{
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
			const SteamWebApi::FriendListApi::FriendListItem *item = friendList.GetAt(i);

			const char *steamId = item->GetSteamId();
			SteamWebApi::FriendListApi::FRIEND_TYPE type = item->GetType();

			if (type == SteamWebApi::FriendListApi::FRIEND_TYPE_FRIEND)
			{
				if (!FindContact(steamId))
				{
					if (newContacts.IsEmpty())
						newContacts.Append(steamId);
					else
						newContacts.AppendFormat(",%s", steamId);
				}
			}
			else if (type == SteamWebApi::FriendListApi::FRIEND_TYPE_NONE)
			{
				MCONTACT hContact = FindContact(steamId);
				if (!hContact)
					hContact = AddContact(steamId);

				RaiseAuthRequestThread((void*)hContact);
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
					MCONTACT hContact = AddContact(summary->GetSteamId());
					if (hContact)
						UpdateContact(hContact, summary);
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
	ssr.hdr.nick  = mir_wstrdup(contact->GetNickName());
	ssr.hdr.firstName = mir_wstrdup(contact->GetFirstName());
	ssr.hdr.lastName = mir_wstrdup(contact->GetLastName());
	
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
		ssr.hdr.nick  = mir_wstrdup(contact->GetNickName());
		ssr.hdr.firstName = mir_wstrdup(contact->GetFirstName());
		ssr.hdr.lastName = mir_wstrdup(contact->GetLastName());

		ssr.contact = contact;

		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)STEAM_SEARCH_BYNAME, (LPARAM)&ssr);
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)STEAM_SEARCH_BYNAME, 0);
}
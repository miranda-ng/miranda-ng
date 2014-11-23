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

void CSteamProto::UpdateContact(MCONTACT hContact, JSONNODE *data)
{
	JSONNODE *node = NULL;

	// set common data
	node = json_get(data, "personaname");
	setWString(hContact, "Nick", json_as_string(node));

	node = json_get(data, "profileurl");
	setString(hContact, "Homepage", ptrA(mir_u2a(json_as_string(node))));

	// set name
	node = json_get(data, "realname");
	if (node != NULL)
	{
		std::wstring realname = json_as_string(node);
		if (!realname.empty())
		{
			size_t pos = realname.find(' ', 1);
			if (pos != std::string::npos)
			{
				const wchar_t *firstName = realname.substr(0, pos).c_str();
				const wchar_t *lastName = realname.substr(pos + 1).c_str();

				setWString(hContact, "FirstName", firstName);
				setWString(hContact, "LastName", lastName);
			}
			else
			{
				setWString(hContact, "FirstName", realname.c_str());
				delSetting(hContact, "LastName");
			}
		}
	}
	else
	{
		delSetting(hContact, "FirstName");
		delSetting(hContact, "LastName");
	}

	// avatar
	node = json_get(data, "avatarfull");
	ptrA avatarUrl(mir_u2a(json_as_string(node)));
	setString("AvatarUrl", avatarUrl);

	PushRequest(
		new SteamWebApi::GetAvatarRequest(avatarUrl),
		&CSteamProto::OnGotAvatar,
		(void*)hContact);

	// set country
	node = json_get(data, "loccountrycode");
	if (node != NULL)
	{
		const char *iso = ptrA(mir_u2a(json_as_string(node)));
		char *country = (char *)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)iso, 0);
		setString(hContact, "Country", country);
	}
	else
		this->delSetting(hContact, "Country");

	// only for contacts
	if (hContact)
	{
		node = json_get(data, "lastlogoff");
		setDword(hContact, "LastEventDateTS", json_as_int(node));

		node = json_get(data, "gameid");
		DWORD gameId = atol(ptrA(mir_u2a(json_as_string(node))));
		if (gameId > 0)
		{
			node = json_get(data, "gameextrainfo");
			const wchar_t *gameInfo = json_as_string(node);

			db_set_ws(hContact, "CList", "StatusMsg", gameInfo);
			setWord(hContact, "Status", ID_STATUS_OUTTOLUNCH);

			setWString(hContact, "GameInfo", gameInfo);
			setDword(hContact, "GameID", gameId);
		}
		else
		{
			node = json_get(data, "personastate");
			WORD status = SteamToMirandaStatus(json_as_int(node));
			setWord(hContact, "Status", status);

			db_unset(hContact, "CList", "StatusMsg");
			delSetting(hContact, "GameID");
		}
	}
}

MCONTACT CSteamProto::AddContact(const char *steamId, bool isTemporary)
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

		if (isTemporary)
		{
			setByte(hContact, "Auth", 1);
			//setByte(hContact, "Grant", 1);
			db_set_b(hContact, "CList", "NotOnList", 1);
		}

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

void CSteamProto::OnGotFriendList(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL)
		return;

	JSONNODE *root = json_parse(response->pData), *node, *child;

	if (root == NULL)
		return;

	std::string steamIds;

	node = json_get(root, "friends");
	root = json_as_array(node);
	if (root != NULL)
	{
		for (size_t i = 0; i < json_size(root); i++)
		{
			child = json_at(root, i);
			if (child == NULL)
				break;

			node = json_get(child, "steamid");
			ptrA steamId(mir_u2a(json_as_string(node)));

			MCONTACT hContact = FindContact(steamId);
			if (!hContact)
			{
				hContact = AddContact(steamId);
				steamIds.append(steamId).append(",");
			}

			node = json_get(child, "relationship");
			ptrA relationship(mir_u2a(json_as_string(node)));
			/*if (!lstrcmpiA(relationship, "friend"))
			{
				if (!FindContact(steamId))
				{
					AddContact(steamId);
					steamIds.append(steamId).append(",");
				}
			}
			else */if (!lstrcmpiA(relationship, "ignoredfriend"))
			{
				// todo
				setByte(hContact, "Block", 1);
			}
			else if (!lstrcmpiA(relationship, "requestrecipient"))
			{
				// todo
				//RaiseAuthRequestThread((void*)hContact);
			}
			else continue;
		}
	}

	json_delete(root);

	if (!steamIds.empty())
	{
		steamIds.pop_back();
		ptrA token(getStringA("TokenSecret"));

		PushRequest(
			new SteamWebApi::GetUserSummariesRequest(token, steamIds.c_str()),
			&CSteamProto::OnGotUserSummaries);
	}
}

void CSteamProto::OnGotBlockList(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL)
		return;

	JSONNODE *root = json_parse(response->pData), *node, *child;

	if (root == NULL)
		return;

	//std::string steamIds;

	node = json_get(root, "friends");
	JSONNODE *nroot = json_as_array(node);
	if (nroot != NULL)
	{
		for (size_t i = 0; i < json_size(nroot); i++)
		{
			child = json_at(nroot, i);
			if (child == NULL)
				break;

			node = json_get(child, "steamid");
			ptrA steamId(mir_u2a(json_as_string(node)));

			/*MCONTACT hContact = FindContact(steamId);
			if (!hContact)
			{
				hContact = AddContact(steamId);
				steamIds.append(steamId).append(",");
			}*/

			node = json_get(child, "relationship");
			ptrA relationship(mir_u2a(json_as_string(node)));

			if (!lstrcmpiA(relationship, "ignoredfriend"))
			{
				// todo: open block list
			}
			else continue;
		}
	}

	json_delete(root);
}

void CSteamProto::OnGotUserSummaries(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL)
		return;

	JSONNODE *root = json_parse(response->pData), *node, *item;

	node = json_get(root, "players");
	JSONNODE *nroot = json_as_array(node);
	if (nroot != NULL)
	{
		for (size_t i = 0; i < json_size(nroot); i++)
		{
			item = json_at(nroot, i);
			if (item == NULL)
				break;

			node = json_get(item, "steamid");
			ptrA steamId(mir_u2a(json_as_string(node)));

			MCONTACT hContact = FindContact(steamId);
			if (!hContact)
				hContact = AddContact(steamId);

			UpdateContact(hContact, item);
		}
	}

	json_delete(root);
}

void CSteamProto::OnGotAvatar(const NETLIBHTTPREQUEST *response, void *arg)
{
	MCONTACT hContact = (MCONTACT)arg;

	if (response == NULL || response->resultCode != HTTP_STATUS_OK)
	{
		ptrA steamId(getStringA(hContact, "SteamID"));
		debugLogA("CSteamProto::OnGotAvatar: failed to get avatar %s", steamId);
		return;
	}

	ptrW avatarPath(GetAvatarFilePath(hContact));
	FILE *fp = _wfopen(avatarPath, L"wb");
	if (fp)
	{
		fwrite(response->pData, sizeof(char), response->dataLength, fp);
		fclose(fp);

		PROTO_AVATAR_INFORMATIONW pai = { sizeof(pai) };
		pai.format = PA_FORMAT_JPEG;
		pai.hContact = hContact;
		wcscpy(pai.filename, avatarPath);

		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, 0);
	}
}

void CSteamProto::OnFriendAdded(const NETLIBHTTPREQUEST *response, void *arg)
{
	SendAuthParam *param = (SendAuthParam*)arg;

	if (response == NULL || response->resultCode != HTTP_STATUS_OK || lstrcmpiA(response->pData, "true"))
	{
		ptrA steamId(getStringA(param->hContact, "SteamID"));
		debugLogA("CSteamProto::OnFriendAdded: failed to add friend %s", steamId);

		ProtoBroadcastAck(param->hContact, ACKTYPE_AUTHREQ, ACKRESULT_FAILED, param->hAuth, 0);

		return;
	}

	delSetting(param->hContact, "Auth");
	delSetting(param->hContact, "Grant");
	db_unset(param->hContact, "CList", "NotOnList");

	ProtoBroadcastAck(param->hContact, ACKTYPE_AUTHREQ, ACKRESULT_SUCCESS, param->hAuth, 0);
}

void CSteamProto::OnFriendBlocked(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL || response->resultCode != HTTP_STATUS_OK || lstrcmpiA(response->pData, "true"))
	{
		debugLogA("CSteamProto::OnFriendIgnored: failed to ignore friend %s", ptrA((char*)arg));
		return;
	}
}

void CSteamProto::OnFriendRemoved(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL || response->resultCode != HTTP_STATUS_OK || lstrcmpiA(response->pData, "true"))
	{
		debugLogA("CSteamProto::OnFriendRemoved: failed to remove friend %s", ptrA((char*)arg));
		return;
	}
}

void CSteamProto::OnAuthRequested(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL || response->resultCode != HTTP_STATUS_OK)
	{
		debugLogA("CSteamProto::OnAuthRequested: failed to request info for %s", ptrA((char*)arg));
		return;
	}

	JSONNODE *root = json_parse(response->pData), *node;

	node = json_get(root, "players");
	JSONNODE *nroot = json_at(json_as_array(node), 0);
	if (nroot != NULL)
	{
		node = json_get(nroot, "steamid");
		ptrA steamId(mir_u2a(json_as_string(node)));

		MCONTACT hContact = FindContact(steamId);
		if (!hContact)
			hContact = AddContact(steamId);

		UpdateContact(hContact, nroot);

		char *nickName = getStringA(hContact, "Nick");
		char *firstName = getStringA(hContact, "FirstName");
		if (firstName == NULL)
			firstName = mir_strdup("");
		char *lastName = getStringA(hContact, "LastName");
		if (lastName == NULL)
			lastName = mir_strdup("");

		char reason[MAX_PATH];
		mir_snprintf(reason, SIZEOF(reason), Translate("%s has added you to his or her Friend List"), nickName);

		// blob is: 0(DWORD), hContact(DWORD), nick(ASCIIZ), firstName(ASCIIZ), lastName(ASCIIZ), sid(ASCIIZ), reason(ASCIIZ)
		DWORD cbBlob = (DWORD)(sizeof(DWORD)* 2 + lstrlenA(nickName) + lstrlenA(firstName) + lstrlenA(lastName) + lstrlenA(steamId) + lstrlenA(reason) + 5);

		PBYTE pBlob, pCurBlob;
		pCurBlob = pBlob = (PBYTE)mir_alloc(cbBlob);

		*((PDWORD)pCurBlob) = 0;
		pCurBlob += sizeof(DWORD);
		*((PDWORD)pCurBlob) = (DWORD)hContact;
		pCurBlob += sizeof(DWORD);
		strcpy((char*)pCurBlob, nickName);
		pCurBlob += lstrlenA(nickName) + 1;
		strcpy((char*)pCurBlob, firstName);
		pCurBlob += lstrlenA(firstName) + 1;
		strcpy((char*)pCurBlob, lastName);
		pCurBlob += lstrlenA(lastName) + 1;
		strcpy((char*)pCurBlob, steamId);
		pCurBlob += lstrlenA(steamId) + 1;
		strcpy((char*)pCurBlob, mir_strdup(reason));

		AddDBEvent(hContact, EVENTTYPE_AUTHREQUEST, time(NULL), DBEF_UTF, cbBlob, pBlob);
	}

	json_delete(root);
}

void CSteamProto::OnPendingApproved(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL || response->resultCode != HTTP_STATUS_OK)
	{
		debugLogA("CSteamProto::OnPendingApproved: failed to approve pending from %s", ptrA((char*)arg));
		return;
	}

	JSONNODE *root = json_parse(response->pData), *node;

	node = json_get(root, "success");
	if (json_as_int(node) == 0)
	{
		node = json_get(root, "error_text");
		debugLogA("CSteamProto::OnPendingApproved: failed to approve pending from %s (%s)", ptrA((char*)arg), ptrA(mir_utf8encodeW(json_as_string(node))));
	}

	json_delete(root);
}

void CSteamProto::OnPendingIgnoreded(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL || response->resultCode != HTTP_STATUS_OK)
	{
		debugLogA("CSteamProto::OnPendingApproved: failed to ignore pending from %s", ptrA((char*)arg));
		return;
	}

	JSONNODE *root = json_parse(response->pData), *node;

	node = json_get(root, "success");
	if (json_as_int(node) == 0)
	{
		node = json_get(root, "error_text");
		debugLogA("CSteamProto::OnPendingApproved: failed to ignore pending from %s (%s)", ptrA((char*)arg), ptrA(mir_utf8encodeW(json_as_string(node))));
	}

	json_delete(root);
}

void CSteamProto::OnSearchByIdEnded(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL || response->resultCode != HTTP_STATUS_OK)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)STEAM_SEARCH_BYID, 0);
		debugLogA("CSteamProto::OnSearchByIdEnded: failed to get summaries for %s", ptrA((char*)arg));
		return;
	}

	JSONNODE *root = json_parse(response->pData), *node;

	node = json_get(root, "players");
	root = json_at(json_as_array(node), 0);
	if (root != NULL)
	{
		STEAM_SEARCH_RESULT ssr = { 0 };
		ssr.hdr.cbSize = sizeof(STEAM_SEARCH_RESULT);
		ssr.hdr.flags = PSR_TCHAR;
	
		ssr.hdr.id = (wchar_t*)arg;

		node = json_get(root, "personaname");
		ssr.hdr.nick  = mir_wstrdup(json_as_string(node));

		node = json_get(root, "realname");
		if (node != NULL)
		{
			std::wstring realname = json_as_string(node);
			if (!realname.empty())
			{
				size_t pos = realname.find(' ', 1);
				if (pos != std::string::npos)
				{
					ssr.hdr.firstName = mir_wstrdup(realname.substr(0, pos).c_str());
					ssr.hdr.lastName = mir_wstrdup(realname.substr(pos + 1).c_str());
				}
				else
					ssr.hdr.firstName = mir_wstrdup(realname.c_str());
			}
		}
	
		//ssr.contact = contact;
		ssr.data = json_copy(root);

		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)STEAM_SEARCH_BYID, (LPARAM)&ssr);
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)STEAM_SEARCH_BYID, 0);
	}
}

void CSteamProto::OnSearchByNameStarted(const NETLIBHTTPREQUEST *response, void *arg)
{
}

//void CSteamProto::SearchByNameThread(void* arg)
//{
//	ptrW keywordsW((wchar_t*)arg);
//	ptrA keywords(mir_utf8encodeW(keywordsW));
//
//	ptrA token(getStringA("TokenSecret"));
//
//	SteamWebApi::SearchApi::SearchResult searchResult;
//	debugLogA("CSteamProto::SearchByNameThread: call SteamWebApi::SearchApi::Search");
//	SteamWebApi::SearchApi::Search(m_hNetlibUser, token, keywords, &searchResult);
//
//	if (!searchResult.IsSuccess())
//	{
//		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)STEAM_SEARCH_BYNAME, 0);
//		return;
//	}
//	
//	CMStringA steamIds;
//	for (int i = 0; i < searchResult.GetItemCount(); i++)
//	{
//		const SteamWebApi::SearchApi::SearchItem *item = searchResult.GetAt(i);
//		if (steamIds.IsEmpty())
//			steamIds.Append(item->GetSteamId());
//		else
//			steamIds.AppendFormat(",%s", item->GetSteamId());
//	}
//
//	SteamWebApi::FriendApi::Summaries summarues;
//	debugLogA("CSteamProto::SearchByNameThread: call SteamWebApi::FriendApi::LoadSummaries");
//	SteamWebApi::FriendApi::LoadSummaries(m_hNetlibUser, token, steamIds, &summarues);
//
//	if (!summarues.IsSuccess())
//	{
//		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)STEAM_SEARCH_BYNAME, 0);
//		return;
//	}
//
//	for (size_t i = 0; i < summarues.GetItemCount(); i++)
//	{
//		const SteamWebApi::FriendApi::Summary *contact = summarues.GetAt(i);
//
//		STEAM_SEARCH_RESULT ssr = { 0 };
//		ssr.hdr.cbSize = sizeof(STEAM_SEARCH_RESULT);
//		ssr.hdr.flags = PSR_TCHAR;
//
//		ssr.hdr.id = mir_a2u(contact->GetSteamId());
//		ssr.hdr.nick  = mir_wstrdup(contact->GetNickName());
//		ssr.hdr.firstName = mir_wstrdup(contact->GetFirstName());
//		ssr.hdr.lastName = mir_wstrdup(contact->GetLastName());
//
//		ssr.contact = contact;
//
//		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)STEAM_SEARCH_BYNAME, (LPARAM)&ssr);
//	}
//
//	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)STEAM_SEARCH_BYNAME, 0);
//}
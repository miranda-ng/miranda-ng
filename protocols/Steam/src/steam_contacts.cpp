#include "common.h"

void CSteamProto::SetContactStatus(MCONTACT hContact, WORD status)
{
	WORD oldStatus = getWord(hContact, "Status", ID_STATUS_OFFLINE);
	if (oldStatus != status)
	{
		setWord(hContact, "Status", status);

		// Special handling of some statuses
		switch (status)
		{
		case ID_STATUS_FREECHAT:
			{
				// Contact is looking to play, save it to as status message
				if (hContact)
					db_set_ts(hContact, "CList", "StatusMsg", TranslateT("Looking to play"));
			}
			break;

		case ID_STATUS_OUTTOLUNCH:
			{
				// Contact is looking to trade, save it to as status message
				if (hContact)
					db_set_ts(hContact, "CList", "StatusMsg", TranslateT("Looking to trade"));
			}
			break;

		case ID_STATUS_OFFLINE:
			{
				// If contact is offline, clear also xstatus
				delSetting(hContact, "XStatusId");
				delSetting(hContact, "XStatusName");
				delSetting(hContact, "XStatusMsg");

				if (hContact)
					SetContactExtraIcon(hContact, NULL);
			}
			// no break intentionally

		default:
			{
				if (hContact)
					db_unset(hContact, "CList", "StatusMsg");
			}
			break;
		}
	}
}

void CSteamProto::SetAllContactsStatus(WORD status)
{
	for (MCONTACT hContact = db_find_first(this->m_szModuleName); hContact; hContact = db_find_next(hContact, this->m_szModuleName))
	{
		if (this->isChatRoom(hContact))
			continue;

		SetContactStatus(hContact, status);
	}
}

MCONTACT CSteamProto::GetContactFromAuthEvent(MEVENT hEvent)
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
	setTString(hContact, "Nick", ptrT(json_as_string(node)));

	node = json_get(data, "profileurl");
	setString(hContact, "Homepage", _T2A(ptrT(json_as_string(node))));

	node = json_get(data, "primaryclanid");
	setString(hContact, "PrimaryClanID", _T2A(ptrT(json_as_string(node))));

	// set name
	node = json_get(data, "realname");
	if (node != NULL)
	{
		std::wstring realname = ptrT(json_as_string(node));
		if (!realname.empty())
		{
			size_t pos = realname.find(L' ', 1);
			if (pos != std::wstring::npos)
			{
				setWString(hContact, "FirstName", realname.substr(0, pos).c_str());
				setWString(hContact, "LastName", realname.substr(pos + 1).c_str());
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
	bool biggerAvatars = getBool("UseBigAvatars", false);
	node = json_get(data, biggerAvatars ? "avatarfull" : "avatarmedium");
	std::string avatarUrl = _T2A(ptrT(json_as_string(node)));
	CheckAvatarChange(hContact, avatarUrl);

	// set country
	node = json_get(data, "loccountrycode");
	if (node != NULL)
	{
		const char *iso = ptrA(mir_u2a(ptrT(json_as_string(node))));
		char *country = (char *)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)iso, 0);
		setString(hContact, "Country", country);
	}
	else
		delSetting(hContact, "Country");

	// state code
	node = json_get(data, "locstatecode");
	if (node)
		setDword(hContact, "StateCode", json_as_int(node));
	else
		delSetting(hContact, "StateCode");

	// city id
	node = json_get(data, "loccityid");
	if (node)
		setDword(hContact, "CityID", json_as_int(node));
	else
		delSetting(hContact, "CityID");

	// account created
	node = json_get(data, "timecreated");
	setDword(hContact, "MemberTS", json_as_int(node));

	// last logout time
	node = json_get(data, "lastlogoff");
	setDword(hContact, "LogoffTS", json_as_int(node));

	// status
	// NOTE: this here is wrong info, probably depending on publicity of steam profile, but we don't need this at all, we get status updates by polling
	/*node = json_get(data, "personastate");
	status = SteamToMirandaStatus(json_as_int(node));
	SetContactStatus(hContact, status);
	*/

	// client
	node = json_get(data, "personastateflags");
	int stateflags = node ? json_as_int(node) : -1;
	switch (stateflags)
	{
	case 0:
		{
			// nothing special, either standard client or in different status (only online, I want to play, I want to trade statuses support this flags)
			WORD status = getWord(hContact, "Status", ID_STATUS_OFFLINE);
			if (status == ID_STATUS_ONLINE || status == ID_STATUS_OUTTOLUNCH || status == ID_STATUS_FREECHAT)
				setTString(hContact, "MirVer", _T("Steam"));
		}
		break;
	case 256:
		// on website
		setTString(hContact, "MirVer", _T("Steam (website)"));
		break;
	case 512:
		// on mobile
		setTString(hContact, "MirVer", _T("Steam (mobile)"));
		break;
	case 1024:
		// big picture mode
		setTString(hContact, "MirVer", _T("Steam (Big Picture)"));
		break;
	default:
		// none/unknown (e.g. when contact is offline)
		delSetting(hContact, "MirVer");
		break;
	}

	// playing game
	node = json_get(data, "gameid");
	DWORD gameId = node ? atol(_T2A(ptrT(json_as_string(node)))) : 0;

	node = json_get(data, "gameextrainfo");
	ptrT gameInfo(json_as_string(node));

	if (gameId > 0 || gameInfo[0] != '\0')
	{
		node = json_get(data, "gameserverip");
		ptrT serverIP(json_as_string(node));

		node = json_get(data, "gameserversteamid");
		ptrT serverID (json_as_string(node));

		setDword(hContact, "GameID", gameId);
		setString(hContact, "ServerIP", _T2A(serverIP));
		setString(hContact, "ServerID", _T2A(serverID));

		CMString message(gameInfo);
		if (!gameId)
			message.Append(TranslateT(" (Non-Steam)"));
		if (serverIP[0] != '\0')
			message.AppendFormat(TranslateT(" on server %s"), serverIP);
		
		setDword(hContact, "XStatusId", gameId);
		setTString(hContact, "XStatusName", TranslateT("Playing"));
		setTString(hContact, "XStatusMsg", message);

		SetContactExtraIcon(hContact, gameId);
	}
	else
	{
		delSetting(hContact, "GameID");
		delSetting(hContact, "ServerIP");
		delSetting(hContact, "ServerID");

		delSetting(hContact, "XStatusId");
		delSetting(hContact, "XStatusName");
		delSetting(hContact, "XStatusMsg");

		SetContactExtraIcon(hContact, NULL);
	}
}

void CSteamProto::ContactIsRemoved(MCONTACT hContact)
{
	if (!getDword(hContact, "DeletedTS", 0) && getByte(hContact, "Auth", 0) == 0)
	{
		setByte(hContact, "Auth", 1);
		setDword(hContact, "DeletedTS", ::time(NULL));
		SetContactStatus(hContact, ID_STATUS_OFFLINE);

		ptrT nick(getTStringA(hContact, "Nick"));
		TCHAR message[MAX_PATH];
		mir_sntprintf(message, MAX_PATH, TranslateT("%s has been removed from your contact list"), nick);

		ShowNotification(_T("Steam"), message);
	}
}

void CSteamProto::ContactIsFriend(MCONTACT hContact)
{
	if (getDword(hContact, "DeletedTS", 0) || getByte(hContact, "Auth", 0) != 0)
	{
		delSetting(hContact, "Auth");
		delSetting(hContact, "DeletedTS");		
		delSetting(hContact, "Grant");

		ptrT nick(getTStringA(hContact, "Nick"));
		TCHAR message[MAX_PATH];
		mir_sntprintf(message, MAX_PATH, TranslateT("%s is back in your contact list"), nick);

		ShowNotification(_T("Steam"), message);
	}
}

void CSteamProto::ContactIsIgnored(MCONTACT hContact)
{
	// todo
	setByte(hContact, "Block", 1);
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

void CSteamProto::ProcessContact(std::map<std::string, JSONNODE*>::iterator *it, MCONTACT hContact)
{
	std::string steamId = (*it)->first;
	JSONNODE *child = (*it)->second;

	if (!hContact)
		hContact = AddContact(steamId.c_str());

	JSONNODE *node = json_get(child, "friend_since");
	if (node)
		db_set_dw(hContact, "UserInfo", "ContactAddTime", json_as_int(node));

	node = json_get(child, "relationship");
	if (node == NULL)
		return;

	ptrA relationship(mir_u2a(ptrT(json_as_string(node))));
	if (!lstrcmpiA(relationship, "friend"))
	{
		ContactIsFriend(hContact);
	}
	else if (!lstrcmpiA(relationship, "ignoredfriend"))
	{
		ContactIsIgnored(hContact);
	}
	else if (!lstrcmpiA(relationship, "requestrecipient"))
	{
		// todo
		//RaiseAuthRequestThread((void*)hContact);
	}
}

void CSteamProto::OnGotFriendList(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL)
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	std::string steamIds = ptrA(getStringA("SteamID"));

	std::map<std::string, JSONNODE*> friends;

	// Remember contacts on server
	JSONNODE *node = json_get(root, "friends");
	JSONNODE *nroot = json_as_array(node);
	if (nroot != NULL)
	{
		for (size_t i = 0; i < json_size(nroot); i++)
		{
			JSONNODE *child = json_at(nroot, i);
			if (child == NULL)
				break;

			node = json_get(child, "steamid");
			if (node == NULL)
				continue;

			std::string steamId = _T2A(ptrT(json_as_string(node)));
			friends.insert(std::make_pair(steamId, child));
		}		
	}

	// Check and update contacts in database
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		if (isChatRoom(hContact))
			continue;

		ptrA id(getStringA(hContact, "SteamID"));
		if (id == NULL)
			continue;

		std::map<std::string, JSONNODE*>::iterator it = friends.find(std::string(id));

		if (it != friends.end())
		{
			// Contact is on server-list, update (and eventually notify) it
			ProcessContact(&it, hContact);

			steamIds.append(",").append(it->first);
			friends.erase(it);
		}
		else
		{
			// Contact was removed from server-list, notify it
			ContactIsRemoved(hContact);
		}
	}

	// Check remaining contacts in map and add them to contact list
	for (std::map<std::string, JSONNODE*>::iterator it = friends.begin(); it != friends.end();)
	{
		// Contact is on server-list, but not in database, add (but not notify) it
		ProcessContact(&it, NULL);
		
		steamIds.append(",").append(it->first);
		it = friends.erase(it);
	}
	friends.clear();

	// We need to delete nroot here at the end, because we had references to JSONNODE objects stored in friends map
	json_delete(nroot);

	if (!steamIds.empty())
	{
		//steamIds.pop_back();
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

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	//std::string steamIds;

	JSONNODE *node = json_get(root, "friends");
	JSONNODE *nroot = json_as_array(node);
	if (nroot != NULL)
	{
		for (size_t i = 0; i < json_size(nroot); i++)
		{
			JSONNODE *child = json_at(nroot, i);
			if (child == NULL)
				break;

			node = json_get(child, "steamid");
			ptrA steamId(mir_u2a(ptrT(json_as_string(node))));

			/*MCONTACT hContact = FindContact(steamId);
			if (!hContact)
			{
				hContact = AddContact(steamId);
				steamIds.append(steamId).append(",");
			}*/

			node = json_get(child, "relationship");
			ptrA relationship(mir_u2a(ptrT(json_as_string(node))));

			if (!lstrcmpiA(relationship, "ignoredfriend"))
			{
				// todo: open block list
			}
			else continue;
		}
		json_delete(nroot);
	}
}

void CSteamProto::OnGotUserSummaries(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL)
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;
		
	JSONNODE *node = json_get(root, "players");
	JSONNODE *nroot = json_as_array(node);
	if (nroot != NULL)
	{
		for (size_t i = 0; i < json_size(nroot); i++)
		{
			JSONNODE *item = json_at(nroot, i);
			if (item == NULL)
				break;

			node = json_get(item, "steamid");
			ptrA steamId(mir_u2a(ptrT(json_as_string(node))));

			MCONTACT hContact = NULL;
			if (!IsMe(steamId)) {
				hContact = FindContact(steamId);
				if (!hContact)
					hContact = AddContact(steamId);
			}

			UpdateContact(hContact, item);
		}
		json_delete(nroot);
	}
}

void CSteamProto::OnGotAvatar(const NETLIBHTTPREQUEST *response, void *arg)
{
	PROTO_AVATAR_INFORMATIONW pai = { sizeof(pai) };
	pai.hContact = (MCONTACT)arg;
	GetDbAvatarInfo(pai);

	if (response == NULL || response->resultCode != HTTP_STATUS_OK)
	{
		ptrA steamId(getStringA(pai.hContact, "SteamID"));
		debugLogA("CSteamProto::OnGotAvatar: failed to get avatar %s", steamId);

		if (pai.hContact)
			ProtoBroadcastAck(pai.hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&pai, 0);
		return;
	}

	FILE *fp = _tfopen(pai.filename, _T("wb"));
	if (fp)
	{
		fwrite(response->pData, sizeof(char), response->dataLength, fp);
		fclose(fp);

		if (pai.hContact)
			ProtoBroadcastAck(pai.hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, 0);
		else
			CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);
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
		debugLogA("CSteamProto::OnFriendIgnored: failed to ignore friend %s", (char*)arg);
		return;
	}
}

void CSteamProto::OnFriendRemoved(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL || response->resultCode != HTTP_STATUS_OK || lstrcmpiA(response->pData, "true"))
	{
		MCONTACT hContact = (MCONTACT)arg;
		ptrA who(getStringA(hContact, "SteamID"));

		debugLogA("CSteamProto::OnFriendRemoved: failed to remove friend %s", who);
		return;
	}
}

void CSteamProto::OnAuthRequested(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL || response->resultCode != HTTP_STATUS_OK)
	{
		debugLogA("CSteamProto::OnAuthRequested: failed to request info for %s", (char*)arg);
		return;
	}

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNODE *node = json_get(root, "players");
	JSONNODE *nodes = json_as_array(node);
	JSONNODE *nroot = json_at(nodes, 0);	

	if (nroot != NULL)
	{
		node = json_get(nroot, "steamid");
		ptrA steamId(mir_u2a(ptrT(json_as_string(node))));

		MCONTACT hContact = FindContact(steamId);
		if (!hContact)
			hContact = AddContact(steamId);

		UpdateContact(hContact, nroot);

		ptrA nickName(getStringA(hContact, "Nick"));
		ptrA firstName(getStringA(hContact, "FirstName"));
		if (firstName == NULL)
			firstName = mir_strdup("");
		ptrA lastName(getStringA(hContact, "LastName"));
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
		strcpy((char*)pCurBlob, reason);

		AddDBEvent(hContact, EVENTTYPE_AUTHREQUEST, time(NULL), DBEF_UTF, cbBlob, pBlob);
	}

	json_delete(nodes);
}

void CSteamProto::OnPendingApproved(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL || response->resultCode != HTTP_STATUS_OK)
	{
		debugLogA("CSteamProto::OnPendingApproved: failed to approve pending from %s", (char*)arg);
		return;
	}

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNODE *node = json_get(root, "success");
	if (json_as_int(node) == 0)
	{
		node = json_get(root, "error_text");
		debugLogA("CSteamProto::OnPendingApproved: failed to approve pending from %s (%s)", (char*)arg, ptrA(mir_utf8encodeW(ptrT(json_as_string(node)))));
	}
}

void CSteamProto::OnPendingIgnoreded(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL || response->resultCode != HTTP_STATUS_OK)
	{
		debugLogA("CSteamProto::OnPendingApproved: failed to ignore pending from %s", (char*)arg);
		return;
	}

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNODE *node = json_get(root, "success");
	if (json_as_int(node) == 0)
	{
		node = json_get(root, "error_text");
		debugLogA("CSteamProto::OnPendingApproved: failed to ignore pending from %s (%s)", (char*)arg, ptrA(mir_utf8encodeW(ptrT(json_as_string(node)))));
	}
}

void CSteamProto::OnSearchByIdEnded(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL || response->resultCode != HTTP_STATUS_OK)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)STEAM_SEARCH_BYID, 0);
		debugLogA("CSteamProto::OnSearchByIdEnded: failed to get summaries for %s", (char*)arg);
		return;
	}

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNODE *node = json_get(root, "players");
	JSONNODE *nodes = json_as_array(node);
	JSONNODE *nroot = json_at(nodes, 0);

	if (nroot != NULL)
	{
		STEAM_SEARCH_RESULT ssr = { 0 };
		ssr.hdr.cbSize = sizeof(STEAM_SEARCH_RESULT);
		ssr.hdr.flags = PSR_TCHAR;
	
		ssr.hdr.id = (TCHAR*)arg;

		node = json_get(nroot, "personaname");
		ssr.hdr.nick  = mir_wstrdup(ptrT(json_as_string(node)));

		node = json_get(nroot, "realname");
		if (node != NULL)
		{
			std::wstring realname = ptrT(json_as_string(node));
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
		ssr.data = json_copy(nroot);

		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)STEAM_SEARCH_BYID, (LPARAM)&ssr);
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)STEAM_SEARCH_BYID, 0);
	}

	json_delete(nodes);
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
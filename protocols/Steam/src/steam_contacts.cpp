#include "stdafx.h"

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
					db_set_ws(hContact, "CList", "StatusMsg", TranslateT("Looking to play"));
			}
			break;

		case ID_STATUS_OUTTOLUNCH:
			{
				// Contact is looking to trade, save it to as status message
				if (hContact)
					db_set_ws(hContact, "CList", "StatusMsg", TranslateT("Looking to trade"));
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
	DBEVENTINFO dbei = {};
	dbei.cbBlob = sizeof(DWORD)* 2;
	dbei.pBlob = (PBYTE)&body;

	if (db_event_get(hEvent, &dbei))
		return INVALID_CONTACT_ID;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return INVALID_CONTACT_ID;

	if (mir_strcmp(dbei.szModule, m_szModuleName) != 0)
		return INVALID_CONTACT_ID;

	return DbGetAuthEventContact(&dbei);
}

MCONTACT CSteamProto::FindContact(const char *steamId)
{
	MCONTACT hContact = NULL;

	mir_cslock lck(this->contact_search_lock);

	for (hContact = db_find_first(this->m_szModuleName); hContact; hContact = db_find_next(hContact, this->m_szModuleName))
	{
		ptrA cSteamId(db_get_sa(hContact, this->m_szModuleName, "SteamID"));
		if (!lstrcmpA(cSteamId, steamId))
			break;
	}

	return hContact;
}

void CSteamProto::UpdateContactDetails(MCONTACT hContact, JSONNode *data)
{
	// set common data
	JSONNode *node = json_get(data, "personaname");
	setWString(hContact, "Nick", ptrW(json_as_string(node)));

	node = json_get(data, "profileurl");
	setString(hContact, "Homepage", _T2A(ptrW(json_as_string(node))));

	node = json_get(data, "primaryclanid");
	setString(hContact, "PrimaryClanID", _T2A(ptrW(json_as_string(node))));

	// set name
	node = json_get(data, "realname");
	if (node != NULL)
	{
		std::wstring realname = (wchar_t*)ptrW(json_as_string(node));
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
	std::string avatarUrl = (char*)_T2A(ptrW(json_as_string(node)));
	CheckAvatarChange(hContact, avatarUrl);

	// set country
	node = json_get(data, "loccountrycode");
	if (node != NULL)
	{
		const char *iso = ptrA(mir_u2a(ptrW(json_as_string(node))));
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
	
	if (stateflags == 0) {
		// nothing special, either standard client or in different status (only online, I want to play, I want to trade statuses support this flags)
		WORD status = getWord(hContact, "Status", ID_STATUS_OFFLINE);
		if (status == ID_STATUS_ONLINE || status == ID_STATUS_OUTTOLUNCH || status == ID_STATUS_FREECHAT)
			setWString(hContact, "MirVer", L"Steam");
	}
	else if (stateflags & 2) {
		// game
		setWString(hContact, "MirVer", L"Steam (in game)");
	}
	else if (stateflags & 256) {
		// on website
		setWString(hContact, "MirVer", L"Steam (website)");
	}
	else if (stateflags & 512) {
		// on mobile
		setWString(hContact, "MirVer", L"Steam (mobile)");
	}
	else if (stateflags & 1024) {
		// big picture mode
		setWString(hContact, "MirVer", L"Steam (Big Picture)");
	}
	else {
		// none/unknown (e.g. when contact is offline)
		delSetting(hContact, "MirVer");
	}
	
	// playing game
	node = json_get(data, "gameid");
	DWORD gameId = node ? atol(_T2A(ptrW(json_as_string(node)))) : 0;

	node = json_get(data, "gameextrainfo");
	ptrW gameInfo(json_as_string(node));

	if (gameId > 0 || gameInfo[0] != '\0')
	{
		node = json_get(data, "gameserverip");
		ptrW serverIP(json_as_string(node));

		node = json_get(data, "gameserversteamid");
		ptrW serverID (json_as_string(node));

		setDword(hContact, "GameID", gameId);
		setString(hContact, "ServerIP", _T2A(serverIP));
		setString(hContact, "ServerID", _T2A(serverID));

		CMStringW message(gameInfo);
		if (!gameId)
			message.Append(TranslateT(" (Non-Steam)"));
		if (serverIP[0] != '\0')
			message.AppendFormat(TranslateT(" on server %s"), serverIP);
		
		setDword(hContact, "XStatusId", gameId);
		setWString(hContact, "XStatusName", TranslateT("Playing"));
		setWString(hContact, "XStatusMsg", message);

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
	delSetting(hContact, "AuthAsked");

	// If this contact was authorized and now is not (and isn't filled time of deletion), notify it 
	if (!getDword(hContact, "DeletedTS", 0) && getByte(hContact, "Auth", 0) == 0)
	{
		setDword(hContact, "DeletedTS", ::time(NULL));

		ptrW nick(getWStringA(hContact, "Nick"));
		wchar_t message[MAX_PATH];
		mir_snwprintf(message, MAX_PATH, TranslateT("%s has been removed from your contact list"), nick);

		ShowNotification(L"Steam", message);
	}

	if (getByte(hContact, "Auth", 0) != 1)
	{
		setByte(hContact, "Auth", 1);
	}

	SetContactStatus(hContact, ID_STATUS_OFFLINE);
}

void CSteamProto::ContactIsFriend(MCONTACT hContact)
{
	delSetting(hContact, "AuthAsked");

	// Check if this contact was removed someday and if so, notify he's back
	if (getDword(hContact, "DeletedTS", 0) && getByte(hContact, "Auth", 0) != 0)
	{
		delSetting(hContact, "DeletedTS");

		ptrW nick(getWStringA(hContact, "Nick"));
		wchar_t message[MAX_PATH];
		mir_snwprintf(message, MAX_PATH, TranslateT("%s is back in your contact list"), nick);

		ShowNotification(L"Steam", message);
	}

	if (getByte(hContact, "Auth", 0) != 0 || getByte(hContact, "Grant", 0) != 0)
	{
		delSetting(hContact, "Auth");
		delSetting(hContact, "Grant");
	}
}

void CSteamProto::ContactIsIgnored(MCONTACT hContact)
{
	// todo
	setByte(hContact, "Block", 1);
}

void CSteamProto::ContactIsAskingAuth(MCONTACT hContact)
{
	if (getByte(hContact, "AuthAsked", 0) != 0) {
		// auth request was already showed, do nothing here
		return;
	}

	if (getByte(hContact, "Auth", 0) == 0) {
		// user was just added or he already has authorization, but because we've just got auth request, he was probably deleted and requested again
		ContactIsRemoved(hContact);
	}

	// create auth request event
	ptrA steamId(getStringA(hContact, "SteamID"));
	ptrA nickName(getStringA(hContact, "Nick"));
	ptrA firstName(getStringA(hContact, "FirstName"));
	if (firstName == NULL)
		firstName = mir_strdup("");
	ptrA lastName(getStringA(hContact, "LastName"));
	if (lastName == NULL)
		lastName = mir_strdup("");

	char reason[MAX_PATH];
	mir_snprintf(reason, Translate("%s has added you to his or her Friend List"), nickName);

	DB_AUTH_BLOB blob(hContact, nickName, firstName, lastName, steamId, reason);

	PROTORECVEVENT recv = { 0 };
	recv.timestamp = time(NULL);
	recv.szMessage = blob;
	recv.lParam = blob.size();
	ProtoChainRecv(hContact, PSR_AUTH, 0, (LPARAM)&recv);

	// remember to not create this event again, unless authorization status changes again
	setByte(hContact, "AuthAsked", 1);
}

MCONTACT CSteamProto::AddContact(const char *steamId, bool isTemporary)
{
	MCONTACT hContact = NULL;

	mir_cslock lck(this->contact_search_lock);

	for (hContact = db_find_first(this->m_szModuleName); hContact; hContact = db_find_next(hContact, this->m_szModuleName))
	{
		ptrA cSteamId(db_get_sa(hContact, this->m_szModuleName, "SteamID"));
		if (!lstrcmpA(cSteamId, steamId))
			break;
	}

	if (!hContact)
	{
		// create contact
		hContact = db_add_contact();
		Proto_AddToContact(hContact, this->m_szModuleName);

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
			if(Clist_GroupExists(dbv.ptszVal))
				db_set_ws(hContact, "CList", "Group", dbv.ptszVal);
			db_free(&dbv);
		}
	}

	return hContact;
}

void CSteamProto::UpdateContactRelationship(MCONTACT hContact, JSONNode *data)
{
	JSONNode *node = json_get(data, "friend_since");
	if (node)
		db_set_dw(hContact, "UserInfo", "ContactAddTime", json_as_int(node));

	node = json_get(data, "relationship");
	if (node == NULL)
		return;

	ptrA relationship(mir_u2a(ptrW(json_as_string(node))));
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
		ContactIsAskingAuth(hContact);
	}
}

void CSteamProto::OnGotFriendList(const HttpResponse *response)
{
	if (!CheckResponse(response))
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	std::string steamIds = (char*)ptrA(getStringA("SteamID"));

	std::map<std::string, JSONNode*> friends;

	// Remember contacts on server
	JSONNode *node = json_get(root, "friends");
	JSONNode *nroot = json_as_array(node);
	if (nroot != NULL)
	{
		for (size_t i = 0; i < json_size(nroot); i++)
		{
			JSONNode *child = json_at(nroot, i);
			if (child == NULL)
				break;

			node = json_get(child, "steamid");
			if (node == NULL)
				continue;

			std::string steamId = (char*)_T2A(ptrW(json_as_string(node)));
			friends.insert(std::make_pair(steamId, child));
		}
	}

	{
		// Check and update contacts in database
		mir_cslock lck(this->contact_search_lock);

		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
		{
			if (isChatRoom(hContact))
				continue;

			ptrA id(getStringA(hContact, "SteamID"));
			if (id == NULL)
				continue;

			std::map<std::string, JSONNode*>::iterator it = friends.find(std::string(id));

			if (it != friends.end())
			{
				// Contact is on server-list, update (and eventually notify) it
				UpdateContactRelationship(hContact, it->second);

				steamIds.append(",").append(it->first);
				friends.erase(it);
			}
			else
			{
				// Contact was removed from server-list, notify it
				ContactIsRemoved(hContact);
			}
		}
	}

	// Check remaining contacts in map and add them to contact list
	for (std::map<std::string, JSONNode*>::iterator it = friends.begin(); it != friends.end();)
	{
		// Contact is on server-list, but not in database, add (but not notify) it
		MCONTACT hContact = AddContact(it->first.c_str());
		UpdateContactRelationship(hContact, it->second);
		
		steamIds.append(",").append(it->first);
		it = friends.erase(it);
	}
	friends.clear();

	// We need to delete nroot here at the end, because we had references to JSONNode objects stored in friends map
	json_delete(nroot);

	ptrA token(getStringA("TokenSecret"));

	if (!steamIds.empty())
	{
		//steamIds.pop_back();

		PushRequest(
			new GetUserSummariesRequest(token, steamIds.c_str()),
			&CSteamProto::OnGotUserSummaries);
	}

	// Download last messages
	PushRequest(
		new GetConversationsRequest(token),
		&CSteamProto::OnGotConversations);
}

void CSteamProto::OnGotBlockList(const HttpResponse *response)
{
	if (!CheckResponse(response))
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	//std::string steamIds;

	JSONNode *node = json_get(root, "friends");
	JSONNode *nroot = json_as_array(node);
	if (nroot != NULL)
	{
		for (size_t i = 0; i < json_size(nroot); i++)
		{
			JSONNode *child = json_at(nroot, i);
			if (child == NULL)
				break;

			node = json_get(child, "steamid");
			ptrA steamId(mir_u2a(ptrW(json_as_string(node))));

			/*MCONTACT hContact = FindContact(steamId);
			if (!hContact)
			{
				hContact = AddContact(steamId);
				steamIds.append(steamId).append(",");
			}*/

			node = json_get(child, "relationship");
			ptrA relationship(mir_u2a(ptrW(json_as_string(node))));

			if (!lstrcmpiA(relationship, "ignoredfriend"))
			{
				// todo: open block list
			}
			else continue;
		}
		json_delete(nroot);
	}
}

void CSteamProto::OnGotUserSummaries(const HttpResponse *response)
{
	if (!CheckResponse(response))
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;
		
	JSONNode *node = json_get(root, "players");
	JSONNode *nroot = json_as_array(node);
	if (nroot != NULL)
	{
		for (size_t i = 0; i < json_size(nroot); i++)
		{
			JSONNode *item = json_at(nroot, i);
			if (item == NULL)
				break;

			node = json_get(item, "steamid");
			ptrA steamId(mir_u2a(ptrW(json_as_string(node))));

			MCONTACT hContact = NULL;
			if (!IsMe(steamId)) {
				hContact = AddContact(steamId);
			}

			UpdateContactDetails(hContact, item);
		}
		json_delete(nroot);
	}
}

void CSteamProto::OnGotAvatar(const HttpResponse *response, void *arg)
{
	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.hContact = (UINT_PTR)arg;
	GetDbAvatarInfo(ai);

	if (!ResponseHttpOk(response))
	{
		ptrA steamId(getStringA(ai.hContact, "SteamID"));
		debugLogA("CSteamProto::OnGotAvatar: failed to get avatar %s", steamId);

		if (ai.hContact)
			ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&ai, 0);
		return;
	}

	FILE *fp = _wfopen(ai.filename, L"wb");
	if (fp)
	{
		fwrite(response->pData, sizeof(char), response->dataLength, fp);
		fclose(fp);

		if (ai.hContact)
			ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&ai, 0);
		else
			CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);
	}
}

void CSteamProto::OnFriendAdded(const HttpResponse *response, void *arg)
{
	SendAuthParam *param = (SendAuthParam*)arg;

	if (!response || response->resultCode != HTTP_CODE_OK || lstrcmpiA(response->pData, "true"))
	{
		ptrA steamId(getStringA(param->hContact, "SteamID"));
		debugLogA("CSteamProto::OnFriendAdded: failed to add friend %s", steamId);

		ProtoBroadcastAck(param->hContact, ACKTYPE_AUTHREQ, ACKRESULT_FAILED, param->hAuth, 0);

		return;
	}

	delSetting(param->hContact, "Auth");
	delSetting(param->hContact, "Grant");
	delSetting(param->hContact, "DeletedTS");
	db_unset(param->hContact, "CList", "NotOnList");

	ProtoBroadcastAck(param->hContact, ACKTYPE_AUTHREQ, ACKRESULT_SUCCESS, param->hAuth, 0);
}

void CSteamProto::OnFriendBlocked(const HttpResponse *response, void *arg)
{
	if (!response || response->resultCode != HTTP_CODE_OK || lstrcmpiA(response->pData, "true"))
	{
		debugLogA("CSteamProto::OnFriendIgnored: failed to ignore friend %s", (char*)arg);
		return;
	}
}

void CSteamProto::OnFriendRemoved(const HttpResponse *response, void *arg)
{
	MCONTACT hContact = (UINT_PTR)arg;

	if (!response || response->resultCode != HTTP_CODE_OK || lstrcmpiA(response->pData, "true"))
	{
		ptrA who(getStringA(hContact, "SteamID"));

		debugLogA("CSteamProto::OnFriendRemoved: failed to remove friend %s", who);
		return;
	}

	setByte(hContact, "Auth", 1);
	setDword(hContact, "DeletedTS", ::time(NULL));
}

void CSteamProto::OnAuthRequested(const HttpResponse *response, void *arg)
{
	if (!ResponseHttpOk(response))
	{
		debugLogA("CSteamProto::OnAuthRequested: failed to request info for %s", (char*)arg);
		return;
	}

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNode *node = json_get(root, "players");
	JSONNode *nodes = json_as_array(node);
	JSONNode *nroot = json_at(nodes, 0);

	if (nroot != NULL)
	{
		node = json_get(nroot, "steamid");
		ptrA steamId(mir_u2a(ptrW(json_as_string(node))));

		MCONTACT hContact = AddContact(steamId);

		UpdateContactDetails(hContact, nroot);

		ContactIsAskingAuth(hContact);
	}

	json_delete(nodes);
}

void CSteamProto::OnPendingApproved(const HttpResponse *response, void *arg)
{
	if (!ResponseHttpOk(response))
	{
		debugLogA("CSteamProto::OnPendingApproved: failed to approve pending from %s", (char*)arg);
		return;
	}

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNode *node = json_get(root, "success");
	if (json_as_int(node) == 0)
	{
		node = json_get(root, "error_text");
		debugLogA("CSteamProto::OnPendingApproved: failed to approve pending from %s (%s)", (char*)arg, ptrA(mir_utf8encodeW(ptrW(json_as_string(node)))));
	}
}

void CSteamProto::OnPendingIgnoreded(const HttpResponse *response, void *arg)
{
	if (!ResponseHttpOk(response))
	{
		debugLogA("CSteamProto::OnPendingIgnored: failed to ignore pending from %s", (char*)arg);
		return;
	}

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNode *node = json_get(root, "success");
	if (json_as_int(node) == 0)
	{
		node = json_get(root, "error_text");
		debugLogA("CSteamProto::OnPendingIgnored: failed to ignore pending from %s (%s)", (char*)arg, ptrA(mir_utf8encodeW(ptrW(json_as_string(node)))));
	}
}

void CSteamProto::OnSearchResults(const HttpResponse *response, void *arg)
{
	HANDLE searchType = (HANDLE)arg;

	if (!ResponseHttpOk(response))
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, searchType, 0);
		debugLogA("CSteamProto::AddSearchResults: failed to get summaries");
		return;
	}

	JSONROOT root(response->pData);
	if (root == NULL)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, searchType, 0);
		return;
	}

	JSONNode *node = json_get(root, "players");
	JSONNode *nroot = json_as_array(node);
	if (nroot != NULL)
	{
		for (size_t i = 0; i < json_size(nroot); i++)
		{
			JSONNode *child = json_at(nroot, i);
			if (child == NULL)
				break;

			STEAM_SEARCH_RESULT ssr = { 0 };
			ssr.hdr.cbSize = sizeof(STEAM_SEARCH_RESULT);
			ssr.hdr.flags = PSR_UNICODE;

			node = json_get(child, "steamid");
			ssr.hdr.id.w = mir_wstrdup(ptrW(json_as_string(node)));

			node = json_get(child, "personaname");
			ssr.hdr.nick.w = mir_wstrdup(ptrW(json_as_string(node)));

			node = json_get(child, "realname");
			if (node != NULL)
			{
				std::wstring realname = (wchar_t*)ptrW(json_as_string(node));
				if (!realname.empty())
				{
					size_t pos = realname.find(' ', 1);
					if (pos != std::wstring::npos)
					{
						ssr.hdr.firstName.w = mir_wstrdup(realname.substr(0, pos).c_str());
						ssr.hdr.lastName.w = mir_wstrdup(realname.substr(pos + 1).c_str());
					}
					else
						ssr.hdr.firstName.w = mir_wstrdup(realname.c_str());
				}
			}

			//ssr.contact = contact;
			ssr.data = json_copy(child); // FIXME: is this needed and safe (no memleak) to be here?

			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, searchType, (LPARAM)&ssr);
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, searchType, 0);

	json_delete(nroot);
}

void CSteamProto::OnSearchByNameStarted(const HttpResponse *response, void *arg)
{
	if (!ResponseHttpOk(response))
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)arg, 0);
		debugLogA("CSteamProto::OnSearchByNameEnded: failed to get results");
		return;
	}

	JSONROOT root(response->pData);
	if (root == NULL)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)arg, 0);
		return;
	}

	// JSONNode *count = json_get(root, "count"); // number of results given in this request
	// JSONNode *total = json_get(root, "total"); // number of all search results
	// TODO: may need to load all remaining results, but we need to remember our previous offset and then increment it and cycle with results

	std::string steamIds;

	JSONNode *node = json_get(root, "results");
	JSONNode *nroot = json_as_array(node);
	if (nroot != NULL)
	{
		for (size_t i = 0; i < json_size(nroot); i++)
		{
			JSONNode *child = json_at(nroot, i);
			if (child == NULL)
				break;

			node = json_get(child, "steamid");
			if (node == NULL)
				continue;

			std::string steamId = (char*)_T2A(ptrW(json_as_string(node)));
			steamIds.append(steamId).append(",");
		}
		json_delete(nroot);
	}

	if (!steamIds.empty())
	{
		// remove trailing ","
		steamIds.pop_back();

		ptrA token(getStringA("TokenSecret"));

		PushRequest(
			new GetUserSummariesRequest(token, steamIds.c_str()),
			&CSteamProto::OnSearchResults,
			(HANDLE)arg);
	}
	else
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)arg, 0);
	}
}

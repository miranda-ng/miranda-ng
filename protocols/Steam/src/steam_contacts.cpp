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
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
		SetContactStatus(hContact, status);
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

	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		ptrA cSteamId(db_get_sa(hContact, this->m_szModuleName, "SteamID"));
		if (!lstrcmpA(cSteamId, steamId))
			break;
	}

	return hContact;
}

void CSteamProto::UpdateContactDetails(MCONTACT hContact, const JSONNode &data)
{
	// set common data
	CMStringW nick = data["personaname"].as_mstring();
	setWString(hContact, "Nick", nick);

	json_string homepage = data["profileurl"].as_string();
	setString(hContact, "Homepage", homepage.c_str());

	json_string primaryClanId = data["primaryclanid"].as_string();
	setString(hContact, "PrimaryClanID", primaryClanId.c_str());

	// set name
	JSONNode node = data["realname"];
	if (!node.isnull())
	{
		CMStringW realName = node.as_mstring();
		if (!realName.IsEmpty())
		{
			int pos = realName.Find(L' ', 1);
			if (pos != -1)
			{
				setWString(hContact, "FirstName", realName.Mid(0, pos));
				setWString(hContact, "LastName", realName.Mid(pos + 1));
			}
			else
			{
				setWString(hContact, "FirstName", realName);
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
	json_string avatarUrl = data[biggerAvatars ? "avatarfull" : "avatarmedium"].as_string();
	CheckAvatarChange(hContact, avatarUrl);

	// set country
	node = data["loccountrycode"];
	if (!node.isnull())
	{
		json_string countryCode = node.as_string();
		char *country = (char *)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)countryCode.c_str(), 0);
		setString(hContact, "Country", country);
	}
	else
		delSetting(hContact, "Country");

	// state code
	node = data["locstatecode"];
	if (!node.isnull())
		setDword(hContact, "StateCode", node.as_int());
	else
		delSetting(hContact, "StateCode");

	// city id
	node = data["loccityid"];
	if (!node.isnull())
		setDword(hContact, "CityID", node.as_int());
	else
		delSetting(hContact, "CityID");

	// account created
	node = data["timecreated"];
	setDword(hContact, "MemberTS", node.as_int());

	// last logout time
	node = data["lastlogoff"];
	setDword(hContact, "LogoffTS", node.as_int());

	// status
	// NOTE: this here is wrong info, probably depending on publicity of steam profile, but we don't need this at all, we get status updates by polling
	/*node = json_get(data, "personastate");
	status = SteamToMirandaStatus(json_as_int(node));
	SetContactStatus(hContact, status);
	*/

	// client
	node = data["personastateflags"];
	int stateflags = !node.isnull() ? node.as_int() : -1;
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
	json_string appId = data["gameid"].as_string();
	CMStringW gameInfo = data["gameextrainfo"].as_mstring();
	if (!appId.empty() || !gameInfo.IsEmpty())
	{
		DWORD gameId = atol(appId.c_str());
		json_string serverIP = data["gameserverip"].as_string();
		json_string serverID = data["gameserversteamid"].as_string();

		setDword(hContact, "GameID", gameId);
		setString(hContact, "ServerIP", serverIP.c_str());
		setString(hContact, "ServerID", serverID.c_str());

		CMStringW message(gameInfo);
		if (gameId && message.IsEmpty())
		{
			ptrA token(getStringA("TokenSecret"));
			PushRequest(
				new GetAppInfoRequest(token, appId.c_str()),
				&CSteamProto::OnGotAppInfo,
				(void*)hContact);
		}
		else
		{
			if (!gameId)
				message.Append(TranslateT(" (Non-Steam)"));
			if (!serverIP.empty())
				message.AppendFormat(TranslateT(" on server %s"), serverIP.c_str());
		}
		
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
		setDword(hContact, "DeletedTS", ::time(nullptr));

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
	recv.timestamp = time(nullptr);
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

void CSteamProto::UpdateContactRelationship(MCONTACT hContact, const JSONNode &data)
{
	JSONNode node = data["friend_since"];
	if (!node.isnull())
		db_set_dw(hContact, "UserInfo", "ContactAddTime", node.as_int());

	node = data["relationship"];
	if (node.isnull())
		return;

	json_string relationship = node.as_string();
	if (!lstrcmpiA(relationship.c_str(), "friend"))
		ContactIsFriend(hContact);
	else if (!lstrcmpiA(relationship.c_str(), "ignoredfriend"))
		ContactIsIgnored(hContact);
	else if (!lstrcmpiA(relationship.c_str(), "requestrecipient"))
		ContactIsAskingAuth(hContact);
}

void CSteamProto::OnGotAppInfo(const JSONNode &root, void *arg)
{
	MCONTACT hContact = (UINT_PTR)arg;

	JSONNode apps = root["apps"].as_array();
	for (const JSONNode &app : apps)
	{
		DWORD gameId = app["appid"].as_int();
		CMStringW message = app["name"].as_mstring();

		setDword(hContact, "XStatusId", gameId);
		setWString(hContact, "XStatusName", TranslateT("Playing"));
		setWString(hContact, "XStatusMsg", message);
	}
}

void CSteamProto::OnGotFriendList(const JSONNode &root, void*)
{
	if (root.isnull())
		return;

	// Comma-separated list of steam ids to update summaries
	std::string steamIds = (char*)ptrA(getStringA("SteamID"));

	// Remember contacts on server
		std::map<json_string, JSONNode*> friendsMap;
	JSONNode friends = root["friends"].as_array();
	for (const JSONNode &_friend : friends)
	{
		json_string steamId = _friend["steamid"].as_string();
		friendsMap.insert(std::make_pair(steamId, json_copy(&_friend)));
	}

	{ // Check and update contacts in database
		mir_cslock lck(this->contact_search_lock);

		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
		{
			ptrA steamId(getStringA(hContact, "SteamID"));
			if (steamId == nullptr)
				continue;

			auto it = friendsMap.find((char*)steamId);
			if (it != friendsMap.end())
			{
				// Contact is on server-list, update (and eventually notify) it
				UpdateContactRelationship(hContact, *it->second);
				steamIds.append(",").append(it->first);
				json_delete(it->second);
				friendsMap.erase(it);
			}
			else
			{
				// Contact was removed from server-list, notify it
				ContactIsRemoved(hContact);
			}
		}
	}

	// Check remaining contacts in map and add them to contact list
	for (auto it : friendsMap)
	{
		// Contact is on server-list, but not in database, add (but not notify) it
		MCONTACT hContact = AddContact(it.first.c_str());
		UpdateContactRelationship(hContact, *it.second);
		
		steamIds.append(",").append(it.first);
		json_delete(it.second);
	}
	friendsMap.clear();

	// We need to delete nroot here at the end, because we had references to JSONNode objects stored in friends map
	// json_delete(nroot);

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

void CSteamProto::OnGotBlockList(const JSONNode &root, void*)
{
	if (root.isnull())
		return;

	//std::string steamIds;

	JSONNode friends = root["friends"].as_array();
	if (friends.isnull())
		return;

	for (size_t i = 0; i < friends.size(); i++)
	{
		JSONNode node = friends[i].as_node();

		json_string steamId = node["steamid"].as_string();

		/*MCONTACT hContact = FindContact(steamId);
		if (!hContact)
		{
			hContact = AddContact(steamId);
			steamIds.append(steamId).append(",");
		}*/

		json_string relationship = node["relationship"].as_string();
		if (!lstrcmpiA(relationship.c_str(), "ignoredfriend"))
		{
			// todo: open block list
		}
		else continue;
	}
}

void CSteamProto::OnGotUserSummaries(const JSONNode &root, void*)
{
	JSONNode players = root["players"].as_array();
	if (players.isnull())
		return;

	for (size_t i = 0; i < players.size(); i++)
	{
		JSONNode player = players[i];
		json_string steamId = player["steamid"].as_string();

		MCONTACT hContact = NULL;
		if (!IsMe(steamId.c_str()))
			hContact = AddContact(steamId.c_str());

		UpdateContactDetails(hContact, player);
	}
}

void CSteamProto::OnGotAvatar(const HttpResponse &response, void *arg)
{
	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.hContact = (UINT_PTR)arg;
	GetDbAvatarInfo(ai);

	if (!response.IsSuccess())
	{
		ptrA steamId(getStringA(ai.hContact, "SteamID"));
		debugLogA(__FUNCTION__ ": failed to get avatar %s", steamId);

		if (ai.hContact)
			ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&ai, 0);
		return;
	}

	FILE *fp = _wfopen(ai.filename, L"wb");
	if (fp)
	{
		fwrite(response.Content, sizeof(char), response.Content.GetSize(), fp);
		fclose(fp);

		if (ai.hContact)
			ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&ai, 0);
		else
			CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);
	}
}

void CSteamProto::OnFriendAdded(const HttpResponse &response, void *arg)
{
	SendAuthParam *param = (SendAuthParam*)arg;

	if (!response.IsSuccess() || lstrcmpiA(response.Content, "true"))
	{
		ptrA steamId(getStringA(param->hContact, "SteamID"));
		debugLogA(__FUNCTION__ ": failed to add friend %s", steamId);

		ProtoBroadcastAck(param->hContact, ACKTYPE_AUTHREQ, ACKRESULT_FAILED, param->hAuth, 0);

		return;
	}

	delSetting(param->hContact, "Auth");
	delSetting(param->hContact, "Grant");
	delSetting(param->hContact, "DeletedTS");
	db_unset(param->hContact, "CList", "NotOnList");

	ProtoBroadcastAck(param->hContact, ACKTYPE_AUTHREQ, ACKRESULT_SUCCESS, param->hAuth, 0);
}

void CSteamProto::OnFriendBlocked(const HttpResponse &response, void *arg)
{
	ptrA steamId((char*)arg);

	if (!response.IsSuccess() || lstrcmpiA(response.Content, "true"))
	{
		debugLogA(__FUNCTION__ ": failed to ignore friend %s", (char*)arg);
		return;
	}
}

void CSteamProto::OnFriendRemoved(const HttpResponse &response, void *arg)
{
	MCONTACT hContact = (UINT_PTR)arg;

	if (!response.IsSuccess() || lstrcmpiA(response.Content, "true"))
	{
		ptrA who(getStringA(hContact, "SteamID"));

		debugLogA(__FUNCTION__ ": failed to remove friend %s", who);
		return;
	}

	setByte(hContact, "Auth", 1);
	setDword(hContact, "DeletedTS", ::time(nullptr));
}

void CSteamProto::OnAuthRequested(const JSONNode &root, void*)
{
	if (root.isnull())
		return;

	int first = 0;
	JSONNode players = root["players"].as_array();
	JSONNode player = players[first];
	if (!player.isnull())
	{
		json_string steamId = player["steamid"].as_string();
		MCONTACT hContact = AddContact(steamId.c_str());
		UpdateContactDetails(hContact, player);
		ContactIsAskingAuth(hContact);
	}
}

void CSteamProto::OnPendingApproved(const JSONNode &root, void *arg)
{
	ptrA steamId((char*)arg);

	if (root.isnull())
		return;

	int success = root["success"].as_int();
	if (success == 0)
	{
		json_string error = root["error_text"].as_string();
		debugLogA(__FUNCTION__ ": failed to approve pending from %s (%s)", steamId, ptrA(mir_utf8decodeA(error.c_str())));
	}
}

void CSteamProto::OnPendingIgnoreded(const JSONNode &root, void *arg)
{
	ptrA steamId((char*)arg);

	if (root.isnull())
		return;

	int success = root["success"].as_int();
	if (success == 0)
	{
		json_string error = root["error_text"].as_string();
		debugLogA(__FUNCTION__ ": failed to ignore pending from %s (%s)", steamId, ptrA(mir_utf8decodeA(error.c_str())));
	}
}

void CSteamProto::OnSearchResults(const HttpResponse &response, void *arg)
{
	HANDLE searchType = (HANDLE)arg;

	if (!response.IsSuccess())
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, searchType, 0);
		debugLogA(__FUNCTION__ ": failed to get summaries");
		return;
	}

	JSONNode root = JSONNode::parse(response.Content);
	if (root.isnull())
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, searchType, 0);
		return;
	}

	JSONNode players = root["players"].as_array();
	for (size_t i = 0; i < players.size(); i++)
	{
		JSONNode player = players[i];

		STEAM_SEARCH_RESULT ssr = { 0 };
		ssr.hdr.cbSize = sizeof(STEAM_SEARCH_RESULT);
		ssr.hdr.flags = PSR_UNICODE;

		CMStringW steamId = player["steamid"].as_mstring();
		ssr.hdr.id.w = steamId.Detach();

		CMStringW nick = player["personaname"].as_mstring();
		ssr.hdr.nick.w = nick.Detach();

		JSONNode node = player["realname"];
		if (!node.isnull())
		{
			CMStringW realName = node.as_mstring();
			if (!realName.IsEmpty())
			{
				int pos = realName.Find(' ', 1);
				if (pos != -1)
				{
					ssr.hdr.firstName.w = realName.Mid(0, pos).Detach();
					ssr.hdr.lastName.w = realName.Mid(pos + 1).Detach();
				}
				else
					ssr.hdr.firstName.w = realName.Detach();
			}
		}

		//ssr.contact = contact;
		ssr.data = json_copy(&player); // FIXME: is this needed and safe (no memleak) to be here?

		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, searchType, (LPARAM)&ssr);
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, searchType, 0);
}

void CSteamProto::OnSearchByNameStarted(const HttpResponse &response, void *arg)
{
	if (!response.IsSuccess())
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)arg, 0);
		debugLogA(__FUNCTION__ ": failed to get results");
		return;
	}

	JSONNode root = JSONNode::parse(response.Content);
	if (root.isnull())
	{
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)arg, 0);
		return;
	}

	// JSONNode *count = json_get(root, "count"); // number of results given in this request
	// JSONNode *total = json_get(root, "total"); // number of all search results
	// TODO: may need to load all remaining results, but we need to remember our previous offset and then increment it and cycle with results

	std::string steamIds;

	JSONNode results = root["results"].as_array();
	for (size_t i = 0; i < results.size(); i++)
	{
		JSONNode result = results[i];
		json_string steamId = result["steamid"].as_string();
		steamIds.append(steamId).append(",");
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

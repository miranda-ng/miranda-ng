#include "stdafx.h"

void CSteamProto::SetAllContactStatuses(int status)
{
	for (auto &hContact : AccContacts())
		SetContactStatus(hContact, status);
}

void CSteamProto::SetContactStatus(MCONTACT hContact, uint16_t status)
{
	if (!hContact)
		return;

	uint16_t oldStatus = getWord(hContact, "Status", ID_STATUS_OFFLINE);
	if (oldStatus == status)
		return;

	setWord(hContact, "Status", status);

	// Special handling of some statuses
	switch (status) {
	case ID_STATUS_FREECHAT:
		// Contact is looking to play, save it to as status message
		db_set_ws(hContact, "CList", "StatusMsg", TranslateT("Looking to play"));
		break;

	case ID_STATUS_OFFLINE:
		// if contact is offline, remove played game info
		delSetting(hContact, "GameID");
		delSetting(hContact, "ServerIP");
		delSetting(hContact, "ServerID");
		// clear also xstatus
		delSetting(hContact, "XStatusId");
		delSetting(hContact, "XStatusName");
		delSetting(hContact, "XStatusMsg");
		// and extra icon
		SetContactExtraIcon(hContact, NULL);
		__fallthrough;

	default:
		db_unset(hContact, "CList", "StatusMsg");
		break;
	}
}

MCONTACT CSteamProto::GetContactFromAuthEvent(MEVENT hEvent)
{
	uint32_t body[3];
	DBEVENTINFO dbei = {};
	dbei.cbBlob = sizeof(uint32_t) * 2;
	dbei.pBlob = (uint8_t*)& body;

	if (db_event_get(hEvent, &dbei))
		return INVALID_CONTACT_ID;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return INVALID_CONTACT_ID;

	if (mir_strcmp(dbei.szModule, m_szModuleName) != 0)
		return INVALID_CONTACT_ID;

	return DbGetAuthEventContact(&dbei);
}

MCONTACT CSteamProto::GetContact(const char *steamId)
{
	for (auto &hContact : AccContacts()) {
		ptrA cSteamId(getStringA(hContact, "SteamID"));
		if (!mir_strcmp(cSteamId, steamId))
			return hContact;
	}
	return NULL;
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
	const JSONNode &node = data["realname"];
	if (node) {
		CMStringW realName = node.as_mstring();
		if (!realName.IsEmpty()) {
			int pos = realName.Find(L' ', 1);
			if (pos != -1) {
				setWString(hContact, "FirstName", realName.Mid(0, pos));
				setWString(hContact, "LastName", realName.Mid(pos + 1));
			}
			else {
				setWString(hContact, "FirstName", realName);
				delSetting(hContact, "LastName");
			}
		}
	}
	else {
		delSetting(hContact, "FirstName");
		delSetting(hContact, "LastName");
	}

	// avatar
	bool biggerAvatars = getBool("UseBigAvatars", false);
	json_string avatarUrl = data[biggerAvatars ? "avatarfull" : "avatarmedium"].as_string();
	CheckAvatarChange(hContact, avatarUrl);

	// set country
	json_string countryCode = data["loccountrycode"].as_string();
	if (!countryCode.empty()) {
		char *country = (char *)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)countryCode.c_str(), 0);
		setString(hContact, "Country", country);
	}
	else delSetting(hContact, "Country");

	// state code
	// note: it seems that steam sends "incorrect" state code
	//node = data["locstatecode"];
	//if (!node.isnull())
	//{
	//	json_string stateCode = node.as_string();
	//	setString(hContact, "State", stateCode.c_str());
	//}
	//else
	//{
	//	delSetting(hContact, "State");
	delSetting(hContact, "StateCode");
	//}

	// city id
	// note: steam no longer sends state city id
	//node = data["loccityid"];
	//if (!node.isnull())
	//	setDword(hContact, "CityID", node.as_int());
	//else
	delSetting(hContact, "CityID");

	// account created
	setDword(hContact, "MemberTS", data["timecreated"].as_int());

	// last logout time
	setDword(hContact, "LogoffTS", data["lastlogoff"].as_int());

	if (!IsOnline())
		return;

	// status
	// note: this here is often wrong info, probably depending on publicity of steam profile
	// but sometimes polling does not get status at all
	uint16_t oldStatus = getWord(hContact, "Status", ID_STATUS_OFFLINE);
	// so, set status only if contact is offline
	if (oldStatus == ID_STATUS_OFFLINE) {
		uint16_t status = SteamToMirandaStatus((PersonaState)data["personastate"].as_int());
		SetContactStatus(hContact, status);
	}

	// client
	const JSONNode &nFlags = data["personastateflags"];
	PersonaStateFlag stateflags = (nFlags) ? (PersonaStateFlag)nFlags.as_int() : (PersonaStateFlag)(-1);

	if (stateflags == PersonaStateFlag::None) {
		// nothing special, either standard client or in different status (only online, I want to play, I want to trade statuses support this flags)
		uint16_t status = getWord(hContact, "Status", ID_STATUS_OFFLINE);
		if (status == ID_STATUS_ONLINE || status == ID_STATUS_FREECHAT)
			setWString(hContact, "MirVer", L"Steam");
	}
	else if (contains_flag(stateflags, PersonaStateFlag::InJoinableGame)) {
		// game
		setWString(hContact, "MirVer", L"Steam (in game)");
	}
	else if (contains_flag(stateflags, PersonaStateFlag::ClientTypeWeb)) {
		// on website
		setWString(hContact, "MirVer", L"Steam (website)");
	}
	else if (contains_flag(stateflags, PersonaStateFlag::ClientTypeMobile)) {
		// on mobile
		setWString(hContact, "MirVer", L"Steam (mobile)");
	}
	else if (contains_flag(stateflags, PersonaStateFlag::ClientTypeBigPicture)) {
		// on big picture
		setWString(hContact, "MirVer", L"Steam (Big Picture)");
	}
	else if (contains_flag(stateflags, PersonaStateFlag::ClientTypeVR)) {
		// on VR
		setWString(hContact, "MirVer", L"Steam (VR)");
	}
	else {
		// none/unknown (e.g. when contact is offline)
		delSetting(hContact, "MirVer");
	}

	// playing game
	json_string appId = data["gameid"].as_string();
	CMStringW gameInfo = data["gameextrainfo"].as_mstring();
	if (!appId.empty() || !gameInfo.IsEmpty()) {
		uint32_t gameId = atol(appId.c_str());
		json_string serverIP = data["gameserverip"].as_string();
		json_string serverID = data["gameserversteamid"].as_string();

		setDword(hContact, "GameID", gameId);
		setString(hContact, "ServerIP", serverIP.c_str());
		setString(hContact, "ServerID", serverID.c_str());

		CMStringW message(gameInfo);
		if (gameId && message.IsEmpty()) {
			ptrA token(getStringA("TokenSecret"));
			PushRequest(new GetAppInfoRequest(token, appId.c_str()), &CSteamProto::OnGotAppInfo, (void*)hContact);
		}
		else {
			if (!gameId)
				message.Append(TranslateT(" (Non-Steam)"));
			if (!serverIP.empty())
				message.AppendFormat(TranslateT(" on server %S"), serverIP.c_str());
		}

		setDword(hContact, "XStatusId", gameId);
		setWString(hContact, "XStatusName", TranslateT("Playing"));
		setWString(hContact, "XStatusMsg", message);

		SetContactExtraIcon(hContact, gameId);
	}
	else {
		delSetting(hContact, "GameID");
		delSetting(hContact, "ServerIP");
		delSetting(hContact, "ServerID");

		delSetting(hContact, "XStatusId");
		delSetting(hContact, "XStatusName");
		delSetting(hContact, "XStatusMsg");

		SetContactExtraIcon(hContact, NULL);
	}
}

void CSteamProto::ContactIsFriend(MCONTACT hContact)
{
	debugLogA("Contact %d added to a friends list", hContact);

	delSetting(hContact, "AuthAsked");
	delSetting(hContact, "Auth");
	delSetting(hContact, "Grant");
	Contact::PutOnList(hContact);

	// Check if this contact was removed someday and if so, notify he's back
	if (getDword(hContact, "DeletedTS", 0) && !getByte(hContact, "Auth", 0)) {
		delSetting(hContact, "DeletedTS");

		wchar_t message[MAX_PATH];
		mir_snwprintf(message, MAX_PATH, TranslateT("%s is back in your contact list"), Clist_GetContactDisplayName(hContact));
		ShowNotification(message);
	}
}

void CSteamProto::ContactIsBlocked(MCONTACT hContact)
{
	// todo
	setByte(hContact, "Block", 1);
}

void CSteamProto::ContactIsUnblocked(MCONTACT hContact)
{
	delSetting(hContact, "Block");
}

void CSteamProto::ContactIsRemoved(MCONTACT hContact)
{
	delSetting(hContact, "AuthAsked");

	// If this contact was authorized and now is not (and isn't filled time of deletion), notify it
	if (!getDword(hContact, "DeletedTS", 0) && !getByte(hContact, "Auth", 0)) {
		setDword(hContact, "DeletedTS", now());

		ptrW nick(getWStringA(hContact, "Nick"));
		wchar_t message[MAX_PATH];
		mir_snwprintf(message, MAX_PATH, TranslateT("%s has been removed from your contact list"), nick.get());

		ShowNotification(message);
	}

	setByte(hContact, "Auth", 1);
	SetContactStatus(hContact, ID_STATUS_OFFLINE);
}

void CSteamProto::ContactIsAskingAuth(MCONTACT hContact)
{
	// auth request was already showed, do nothing here
	if (getByte(hContact, "AuthAsked", 0))
		return;

	// create auth request event
	ptrA steamId(getUStringA(hContact, "SteamID"));
	SendRequest(new GetUserSummariesRequest(this, steamId), &CSteamProto::OnGotUserSummaries);

	ptrA nickName(getUStringA(hContact, "Nick"));
	if (nickName == nullptr)
		nickName = mir_strdup(steamId);

	ptrA firstName(getUStringA(hContact, "FirstName"));
	ptrA lastName(getUStringA(hContact, "LastName"));

	char reason[MAX_PATH];
	mir_snprintf(reason, Translate("%s has added you to contact list"), nickName.get());

	DB::AUTH_BLOB blob(hContact, nickName, firstName, lastName, steamId, reason);

	PROTORECVEVENT recv = { 0 };
	recv.timestamp = now();
	recv.szMessage = blob;
	recv.lParam = blob.size();
	ProtoChainRecv(hContact, PSR_AUTH, 0, (LPARAM)&recv);
}

MCONTACT CSteamProto::AddContact(const char *steamId, const wchar_t *nick, bool isTemporary)
{
	mir_cslock lock(m_addContactLock);

	if (!steamId || !mir_strlen(steamId)) {
		debugLogA(__FUNCTION__ ": empty steam id");
		return NULL;
	}

	MCONTACT hContact = GetContact(steamId);
	if (hContact)
		return hContact;

	// create contact
	hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);

	setString(hContact, "SteamID", steamId);
	if (mir_wstrlen(nick)) {
		setWString(hContact, "Nick", nick);
		db_set_ws(hContact, "CList", "MyHandle", nick);
	}

	if (isTemporary) {
		debugLogA("Contact %d added as a temporary one");
		Contact::RemoveFromList(hContact);
	}

	setByte(hContact, "Auth", 1);

	// move to default group
	Clist_SetGroup(hContact, m_defaultGroup);

	return hContact;
}

void CSteamProto::UpdateContactRelationship(MCONTACT hContact, const JSONNode &data)
{
	const JSONNode &node = data["friend_since"];
	if (node)
		db_set_dw(hContact, "UserInfo", "ContactAddTime", node.as_int());

	json_string relationship = data["relationship"].as_string();
	if (relationship == "friend")
		ContactIsFriend(hContact);
	else if (relationship == "ignoredfriend")
		ContactIsBlocked(hContact);
	else if (relationship == "requestrecipient")
		ContactIsAskingAuth(hContact);
}

void CSteamProto::OnGotAppInfo(const JSONNode &root, void *arg)
{
	MCONTACT hContact = (UINT_PTR)arg;

	for (auto &app : root["apps"]) {
		uint32_t gameId = app["appid"].as_int();
		CMStringW message = app["name"].as_mstring();

		setDword(hContact, "XStatusId", gameId);
		setWString(hContact, "XStatusName", TranslateT("Playing"));
		setWString(hContact, "XStatusMsg", message);
	}
}

void CSteamProto::OnGotFriendList(const JSONNode &root, void *)
{
	if (root.isnull())
		return;

	// Comma-separated list of steam ids to update summaries
	std::string steamIds = (char *)ptrA(getStringA("SteamID"));

	// Remember contacts on server
	std::map<json_string, const JSONNode*> friendsMap;
	for (auto &_friend : root["friends"]) {
		json_string steamId = _friend["steamid"].as_string();
		friendsMap.insert(std::make_pair(steamId, &_friend));
	}
	
	if (friendsMap.empty()) {
		debugLogA("Empty friends list, exiting");
		return;
	}

	// Check and update contacts in database
	for (auto &hContact : AccContacts()) {
		ptrA steamId(getStringA(hContact, "SteamID"));
		if (steamId == nullptr)
			continue;

		auto it = friendsMap.find((char *)steamId);
		if (it == friendsMap.end()) {
			// Contact was removed from server-list, notify it
			ContactIsRemoved(hContact);
			continue;
		}

		const JSONNode &_friend = *it->second;

		// Contact is on server-list, update (and eventually notify) it
		UpdateContactRelationship(hContact, _friend);

		// Do not update summary for non friends
		json_string relationship = _friend["relationship"].as_string();
		if (relationship == "friend")
			steamIds.append(",").append(it->first);

		friendsMap.erase(it);
	}

	// Check remaining contacts in map and add them to contact list
	for (auto it : friendsMap) {
		// Contact is on server-list, but not in database, add (but not notify) it
		const JSONNode &_friend = *it.second;

		json_string relationship = _friend["relationship"].as_string();

		MCONTACT hContact = AddContact(it.first.c_str(), nullptr, relationship != "friend");
		UpdateContactRelationship(hContact, _friend);

		if (relationship == "friend")
			steamIds.append(",").append(it.first);
	}
	friendsMap.clear();

	if (!steamIds.empty())
		PushRequest(new GetUserSummariesRequest(this, steamIds.c_str()), &CSteamProto::OnGotUserSummaries);

	// Load last conversations
	PushRequest(new GetConversationsRequest(this), &CSteamProto::OnGotConversations);
}

void CSteamProto::OnGotBlockList(const JSONNode &root, void *)
{
	if (root.isnull())
		return;

	for (auto &_friend : root["friends"]) {
		// json_string steamId = _friend["steamid"].as_string();

		json_string relationship = _friend["relationship"].as_string();
		if (!mir_strcmp(relationship.c_str(), "ignoredfriend")) {
			// todo: open block list
		}
		else continue;
	}
}

void CSteamProto::OnGotUserSummaries(const JSONNode &root, void *)
{
	for (auto &player : root["players"]) {
		json_string steamId = player["steamid"].as_string();
		CMStringW nick = player["personaname"].as_mstring();
		MCONTACT hContact = !IsMe(steamId.c_str()) ? AddContact(steamId.c_str(), nick) : 0;
		UpdateContactDetails(hContact, player);
	}
}

void CSteamProto::OnGotAvatar(const HttpResponse &response, void *arg)
{
	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.hContact = (UINT_PTR)arg;
	GetDbAvatarInfo(ai);

	if (!response.IsSuccess()) {
		ptrA steamId(getStringA(ai.hContact, "SteamID"));
		debugLogA(__FUNCTION__ ": failed to get avatar %s", steamId.get());

		if (ai.hContact)
			ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)& ai, 0);
		return;
	}

	FILE *file = _wfopen(ai.filename, L"wb");
	if (file) {
		fwrite((const char *)response.Content, sizeof(char), response.Content.size(), file);
		fclose(file);

		if (ai.hContact)
			ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)& ai, 0);
		else
			ReportSelfAvatarChanged();
	}
}

void CSteamProto::OnFriendAdded(const HttpResponse &response, void *arg)
{
	SendAuthParam *param = (SendAuthParam *)arg;

	if (!response.IsSuccess() || mir_strcmp(response.Content, "true")) {

		ptrW steamId(getWStringA(param->hContact, "SteamID"));
		ptrW who(getWStringA(param->hContact, "Nick"));
		if (!who)
			who = mir_wstrdup(steamId);

		wchar_t message[MAX_PATH];
		mir_snwprintf(message, L"Error adding friend %s", who.get());

		JSONNode root = JSONNode::parse(response.Content);
		if (root) {
			int success = root["success"].as_int();
			if (success == 1) {
				const JSONNode &invited = root["invited"];
				if (invited) {
					const JSONNode &errors = root["failed_invites_result"];
					if (!errors.empty()) {
						int first = 0;
						int errorCode = errors[first].as_int();
						switch (errorCode) {
						case 11:
							mir_snwprintf(message, L"All communication with %s is blocked. Communication is only possible if you have lifted the blocking. To do this, visit the user's Steam Community page.", who.get());
							break;
						case 15:
							mir_snwprintf(message, L"Request to %s can not be sent. His/her friends list is full.", who.get());
							break;
						case 24:
							mir_wstrcpy(message, L"Your account does not meet the requirements to use this feature. Visit Steam Support to get more information.");
							break;
						case 25:
							mir_snwprintf(message, L"Request to %s can not be sent. Your friends list is full.", who.get());
							break;
						case 40:
							mir_snwprintf(message, L"Error adding friend %s. The communication between you and the other Steam member is blocked.", who.get());
							break;
						case 84:
							mir_wstrcpy(message, L"You've been sending too many invitations lately. Please try again in a day or two.");
							break;
						}
					}
				}
				else {
					ContactIsFriend(param->hContact);
					ProtoBroadcastAck(param->hContact, ACKTYPE_AUTHREQ, ACKRESULT_SUCCESS, param->hAuth, 0);
					return;
				}
			}
		}

		ShowNotification(message, MB_ICONERROR);
		ProtoBroadcastAck(param->hContact, ACKTYPE_AUTHREQ, ACKRESULT_FAILED, param->hAuth, 0);
		return;
	}

	ContactIsFriend(param->hContact);
	ProtoBroadcastAck(param->hContact, ACKTYPE_AUTHREQ, ACKRESULT_SUCCESS, param->hAuth, 0);
}

void CSteamProto::OnFriendBlocked(const HttpResponse &response, void *arg)
{
	ptrA steamId((char *)arg);

	if (!response.IsSuccess() || mir_strcmp(response.Content, "true")) {
		debugLogA(__FUNCTION__ ": failed to ignore friend %s", (char *)steamId);
		return;
	}

	MCONTACT hContact = GetContact(steamId);
	if (hContact)
		ContactIsBlocked(hContact);
}

void CSteamProto::OnFriendUnblocked(const HttpResponse &response, void *arg)
{
	ptrA steamId((char *)arg);

	if (!response.IsSuccess() || mir_strcmp(response.Content, "true")) {
		debugLogA(__FUNCTION__ ": failed to unignore friend %s", (char *)steamId);
		return;
	}

	MCONTACT hContact = GetContact(steamId);
	if (hContact)
		ContactIsUnblocked(hContact);
}

void CSteamProto::OnFriendRemoved(const HttpResponse &response, void *arg)
{
	ptrA steamId((char *)arg);

	if (!response.IsSuccess() || mir_strcmp(response.Content, "true")) {
		debugLogA(__FUNCTION__ ": failed to remove friend %s", (char *)steamId);
		return;
	}

	MCONTACT hContact = GetContact(steamId);
	if (hContact)
		ContactIsRemoved(hContact);
}

void CSteamProto::OnAuthRequested(const JSONNode &root, void *)
{
	if (root.isnull())
		return;

	for (auto &player : root["players"]) {
		json_string steamId = player["steamid"].as_string();
		CMStringW nick = player["personaname"].as_mstring();
		MCONTACT hContact = AddContact(steamId.c_str(), nick);
		UpdateContactDetails(hContact, player);
		ContactIsAskingAuth(hContact);
	}
}

void CSteamProto::OnPendingApproved(const JSONNode &root, void *arg)
{
	ptrA steamId((char *)arg);

	if (root.isnull())
		return;

	int success = root["success"].as_int();
	if (success == 0) {
		json_string error = root["error_text"].as_string();
		debugLogA(__FUNCTION__ ": failed to approve pending from %s (%s)", steamId.get(), ptrA(mir_utf8decodeA(error.c_str())).get());
	}
}

void CSteamProto::OnPendingIgnoreded(const JSONNode &root, void *arg)
{
	ptrA steamId((char *)arg);

	if (root.isnull())
		return;

	int success = root["success"].as_int();
	if (success == 0) {
		json_string error = root["error_text"].as_string();
		debugLogA(__FUNCTION__ ": failed to ignore pending from %s (%s)", steamId.get(), ptrA(mir_utf8decodeA(error.c_str())).get());
	}
}

void CSteamProto::OnSearchResults(const HttpResponse &response, void *arg)
{
	HANDLE searchType = (HANDLE)arg;

	if (!response.IsSuccess()) {
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, searchType, 0);
		debugLogA(__FUNCTION__ ": failed to get summaries");
		return;
	}

	JSONNode root = JSONNode::parse(response.Content);
	if (root.isnull()) {
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, searchType, 0);
		debugLogA(__FUNCTION__ ": no data");
		return;
	}

	for (auto &player : root["players"]) {
		STEAM_SEARCH_RESULT ssr = { 0 };
		ssr.psr.cbSize = sizeof(STEAM_SEARCH_RESULT);
		ssr.psr.flags = PSR_UNICODE;

		CMStringW steamId = player["steamid"].as_mstring();
		ssr.psr.id.w = steamId.Detach();

		CMStringW nick = player["personaname"].as_mstring();
		ssr.psr.nick.w = nick.Detach();

		const JSONNode &node = player["realname"];
		if (node) {
			CMStringW realName = node.as_mstring();
			if (!realName.IsEmpty()) {
				int pos = realName.Find(' ', 1);
				if (pos != -1) {
					ssr.psr.firstName.w = realName.Mid(0, pos).Detach();
					ssr.psr.lastName.w = realName.Mid(pos + 1).Detach();
				}
				else ssr.psr.firstName.w = realName.Detach();
			}
		}

		// todo: is this needed and safe (no memleak) to be here?
		ssr.data = json_copy(&player);

		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, searchType, (LPARAM)& ssr);
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, searchType, 0);
}

void CSteamProto::OnSearchByNameStarted(const HttpResponse &response, void *arg)
{
	if (!response.IsSuccess()) {
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)arg, 0);
		debugLogA(__FUNCTION__ ": failed to get results");
		return;
	}

	JSONNode root = JSONNode::parse(response.Content);
	if (root.isnull()) {
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)arg, 0);
		debugLogA(__FUNCTION__ ": no data");
		return;
	}

	// JSONNode *count = json_get(root, "count"); // number of results given in this request
	// JSONNode *total = json_get(root, "total"); // number of all search results
	// TODO: may need to load all remaining results, but we need to remember our previous offset and then increment it and cycle with results

	std::string steamIds;

	for (auto &result : root["results"]) {
		json_string steamId = result["steamid"].as_string();
		steamIds.append(steamId).append(",");
	}

	if (steamIds.empty()) {
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)arg, 0);
		return;
	}

	// remove trailing ","
	steamIds.pop_back();

	PushRequest(new GetUserSummariesRequest(this, steamIds.c_str()), &CSteamProto::OnSearchResults, (HANDLE)arg);
}

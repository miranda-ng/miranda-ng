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
	dbei.pBlob = (char *)&body;

	if (db_event_get(hEvent, &dbei))
		return INVALID_CONTACT_ID;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return INVALID_CONTACT_ID;

	if (mir_strcmp(dbei.szModule, m_szModuleName) != 0)
		return INVALID_CONTACT_ID;

	return DbGetAuthEventContact(&dbei);
}

MCONTACT CSteamProto::GetContact(int64_t steamId)
{
	for (auto &hContact : AccContacts())
		if (GetId(hContact, DBKEY_STEAM_ID) == steamId)
			return hContact;

	return NULL;
}

void CSteamProto::OnGotFriendInfo(const CMsgClientPersonaState &reply, const CMsgProtoBufHeader &)
{
	for (int i = 0; i < reply.n_friends; i++) {
		auto *F = reply.friends[i];

		auto hContact = GetContact(F->friendid);
		if (!hContact && F->friendid != m_iSteamId)
			hContact = AddContact(F->friendid);		

		// set name
		if (F->player_name) {
			CMStringW realName(Utf2T(F->player_name));
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
		if (F->avatar_hash.len != 0) {
			if (!IsNull(F->avatar_hash)) {
				CMStringA szHash;
				szHash.Truncate(int(F->avatar_hash.len) * 2 + 1);
				bin2hex(F->avatar_hash.data, F->avatar_hash.len, szHash.GetBuffer());
				CheckAvatarChange(hContact, szHash);
			}
			else CheckAvatarChange(hContact, 0);
		}
		else CheckAvatarChange(hContact, 0);

		// last logout time
		if (F->has_last_logoff)
			setDword(hContact, "LogoffTS", F->last_logoff);
		if (F->has_last_logon)
			setDword(hContact, "LogonTS", F->last_logon);

		// status
		// note: this here is often wrong info, probably depending on publicity of steam profile
		// but sometimes polling does not get status at all
		int oldStatus = Contact::GetStatus(hContact);
		// so, set status only if contact is offline
		if (oldStatus == ID_STATUS_OFFLINE) {
			uint16_t status = SteamToMirandaStatus(PersonaState(F->persona_state));
			SetContactStatus(hContact, status);
		}

		// client
		PersonaStateFlag stateflags = (F->has_persona_state_flags) ? (PersonaStateFlag)(F->persona_state_flags) : (PersonaStateFlag)(-1);

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
		auto gameId = F->has_game_played_app_id ? F->game_played_app_id : 0;
		char *gameInfo = F->game_name;
		if (gameId && gameInfo) {
			in_addr in; in.S_un.S_addr = F->game_server_ip;
			auto *serverIP = inet_ntoa(in);

			setDword(hContact, "GameID", gameId);
			setString(hContact, "ServerIP", serverIP);

			CMStringW message(gameInfo);
			if (gameId && message.IsEmpty()) {
				CMStringA szSetting(FORMAT, "AppInfo_%d", gameId);
				ptrW szName(g_plugin.getWStringA(szSetting));
				if (szName)
					message = szName;
				else
					SendAppInfoRequest(gameId);
			}
			else {
				if (!gameId)
					message.Append(TranslateT(" (Non-Steam)"));
				if (serverIP)
					message.AppendFormat(TranslateT(" on server %S"), serverIP);
			}

			setDword(hContact, "XStatusId", gameId);
			setWString(hContact, "XStatusName", TranslateT("Playing"));
			setWString(hContact, "XStatusMsg", message);

			SetContactExtraIcon(hContact, gameId);
		}
		else {
			delSetting(hContact, "GameID");
			delSetting(hContact, "ServerIP");

			delSetting(hContact, "XStatusId");
			delSetting(hContact, "XStatusName");
			delSetting(hContact, "XStatusMsg");

			SetContactExtraIcon(hContact, NULL);
		}
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
	uint64_t id(GetId(hContact, DBKEY_STEAM_ID));
	SendUserInfoRequest(id);

	char steamId[100];
	_i64toa(id, steamId, 10);

	ptrA nickName(getUStringA(hContact, "Nick"));
	if (nickName == nullptr)
		nickName = mir_strdup(steamId);

	ptrA firstName(getUStringA(hContact, "FirstName"));
	ptrA lastName(getUStringA(hContact, "LastName"));

	char reason[MAX_PATH];
	mir_snprintf(reason, Translate("%s has added you to contact list"), nickName.get());

	DB::AUTH_BLOB blob(hContact, nickName, firstName, lastName, steamId, reason);

	DB::EventInfo dbei;
	dbei.timestamp = now();
	dbei.pBlob = blob;
	dbei.cbBlob = blob.size();
	ProtoChainRecv(hContact, PSR_AUTH, 0, (LPARAM)&dbei);
}

MCONTACT CSteamProto::AddContact(int64_t steamId, const wchar_t *nick, bool isTemporary)
{
	mir_cslock lock(m_addContactLock);

	if (!steamId) {
		debugLogA(__FUNCTION__ ": empty steam id");
		return NULL;
	}

	MCONTACT hContact = GetContact(steamId);
	if (hContact)
		return hContact;

	// create contact
	hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);

	SetId(hContact, DBKEY_STEAM_ID, steamId);
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
	if (!Clist_GroupExists(m_wszGroupName))
		Clist_GroupCreate(0, m_wszGroupName);
	Clist_SetGroup(hContact, m_wszGroupName);

	return hContact;
}

void CSteamProto::UpdateContactRelationship(MCONTACT hContact, FriendRelationship iRelationType)
{
	switch (iRelationType) {
	case FriendRelationship::Friend:
		ContactIsFriend(hContact);
		break;
	case FriendRelationship::IgnoredFriend:
		ContactIsBlocked(hContact);
		break;
	case FriendRelationship::RequestRecipient:
		ContactIsAskingAuth(hContact);
		break;
	}
}

void CSteamProto::OnGotFriendList(const CMsgClientFriendsList &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed())
		return;

	if (reply.n_friends == 0) {
		debugLogA("Empty friends list, exiting");
		return;
	}

	std::map<uint64_t, FriendRelationship> friendsMap;
	for (int i = 0; i < reply.n_friends; i++) {
		auto *F = reply.friends[i];
		friendsMap[F->ulfriendid] = FriendRelationship(F->efriendrelationship);
	}

	// Comma-separated list of steam ids to update summaries
	std::vector<uint64_t> ids;
	ids.push_back(GetId(DBKEY_STEAM_ID));

	// Check and update contacts in database
	for (auto &hContact : AccContacts()) {
		int64_t steamId(GetId(hContact, DBKEY_STEAM_ID));
		if (!steamId)
			continue;

		// Contact was removed from server-list, notify it
		auto it = friendsMap.find(steamId);
		if (it == friendsMap.end()) {
			if (!reply.bincremental)
				ContactIsRemoved(hContact);
			continue;
		}

		// Contact is on server-list, update (and eventually notify) it
		UpdateContactRelationship(hContact, it->second);

		// Do not update summary for non friends
		if (it->second == FriendRelationship::Friend)
			ids.push_back(it->first);

		friendsMap.erase(it);
	}

	// Check remaining contacts in map and add them to contact list
	for (auto it : friendsMap) {
		// Contact is on server-list, but not in database, add (but not notify) it
		MCONTACT hContact = AddContact(it.first, nullptr, it.second != FriendRelationship::Friend);
		UpdateContactRelationship(hContact, it.second);

		if (it.second == FriendRelationship::Friend)
			ids.push_back(it.first);
	}
	friendsMap.clear();

	if (!ids.empty())
		SendUserInfoRequest(ids);

	// Load last conversations
	SendFriendActiveSessions();
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

void CSteamProto::OnGotAvatar(const MHttpResponse &response, void *arg)
{
	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.hContact = (UINT_PTR)arg;
	GetDbAvatarInfo(ai);

	if (response.resultCode != 200) {
		ptrA steamId(getStringA(ai.hContact, DBKEY_STEAM_ID));
		debugLogA(__FUNCTION__ ": failed to get avatar %s", steamId.get());

		if (ai.hContact)
			ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)& ai, 0);
		return;
	}

	FILE *file = _wfopen(ai.filename, L"wb");
	if (file) {
		fwrite(response.body, sizeof(char), response.body.GetLength(), file);
		fclose(file);

		if (ai.hContact)
			ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)& ai, 0);
		else
			ReportSelfAvatarChanged();
	}
}

/*
void CSteamProto::OnFriendAdded(const MHttpResponse &response, void *arg)
{
	SendAuthParam *param = (SendAuthParam *)arg;

	if (response.resultCode != 200 || mir_strcmp(response.body, "true")) {
		ptrW steamId(getWStringA(param->hContact, DBKEY_STEAM_ID));
		ptrW who(getWStringA(param->hContact, "Nick"));
		if (!who)
			who = mir_wstrdup(steamId);

		wchar_t message[MAX_PATH];
		mir_snwprintf(message, L"Error adding friend %s", who.get());

		JSONNode root = JSONNode::parse(response.body);
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
*/

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

void CSteamProto::OnSearchResults(const MHttpResponse &response, void *arg)
{
	HANDLE searchType = (HANDLE)arg;

	if (response.resultCode != 200) {
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, searchType, 0);
		debugLogA(__FUNCTION__ ": failed to get summaries");
		return;
	}

	JSONNode root = JSONNode::parse(response.body);
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

void CSteamProto::OnSearchByNameStarted(const MHttpResponse &response, void *arg)
{
	if (response.resultCode != 200) {
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)arg, 0);
		debugLogA(__FUNCTION__ ": failed to get results");
		return;
	}

	JSONNode root = JSONNode::parse(response.body);
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

	SendRequest(new GetUserSummariesRequest(m_szAccessToken, steamIds.c_str()), &CSteamProto::OnSearchResults, (HANDLE)arg);
}

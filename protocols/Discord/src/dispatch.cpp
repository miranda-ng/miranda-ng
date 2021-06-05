/*
Copyright © 2016-21 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

#pragma pack(4)

/////////////////////////////////////////////////////////////////////////////////////////

struct CDiscordCommand
{
	const wchar_t *szCommandId;
	GatewayHandlerFunc pFunc;
}
static handlers[] = // these structures must me sorted alphabetically
{
	{ L"CALL_CREATE", &CDiscordProto::OnCommandCallCreated },
	{ L"CALL_DELETE", &CDiscordProto::OnCommandCallDeleted },
	{ L"CALL_UPDATE", &CDiscordProto::OnCommandCallUpdated },

	{ L"CHANNEL_CREATE", &CDiscordProto::OnCommandChannelCreated },
	{ L"CHANNEL_DELETE", &CDiscordProto::OnCommandChannelDeleted },
	{ L"CHANNEL_UPDATE", &CDiscordProto::OnCommandChannelUpdated },

	{ L"GUILD_CREATE", &CDiscordProto::OnCommandGuildCreated },
	{ L"GUILD_DELETE", &CDiscordProto::OnCommandGuildDeleted },
	{ L"GUILD_MEMBER_ADD", &CDiscordProto::OnCommandGuildMemberAdded },
	{ L"GUILD_MEMBER_LIST_UPDATE", &CDiscordProto::OnCommandGuildMemberListUpdate },
	{ L"GUILD_MEMBER_REMOVE", &CDiscordProto::OnCommandGuildMemberRemoved },
	{ L"GUILD_MEMBER_UPDATE", &CDiscordProto::OnCommandGuildMemberUpdated },
	{ L"GUILD_ROLE_CREATE", &CDiscordProto::OnCommandRoleCreated },
	{ L"GUILD_ROLE_DELETE", &CDiscordProto::OnCommandRoleDeleted },
	{ L"GUILD_ROLE_UPDATE", &CDiscordProto::OnCommandRoleCreated },
	
	{ L"MESSAGE_ACK", &CDiscordProto::OnCommandMessageAck },
	{ L"MESSAGE_CREATE", &CDiscordProto::OnCommandMessageCreate },
	{ L"MESSAGE_DELETE", &CDiscordProto::OnCommandMessageDelete },
	{ L"MESSAGE_UPDATE", &CDiscordProto::OnCommandMessageUpdate },

	{ L"PRESENCE_UPDATE", &CDiscordProto::OnCommandPresence },

	{ L"READY", &CDiscordProto::OnCommandReady },

	{ L"RELATIONSHIP_ADD", &CDiscordProto::OnCommandFriendAdded },
	{ L"RELATIONSHIP_REMOVE", &CDiscordProto::OnCommandFriendRemoved },

	{ L"TYPING_START", &CDiscordProto::OnCommandTyping },

	{ L"USER_SETTINGS_UPDATE", &CDiscordProto::OnCommandUserSettingsUpdate },
	{ L"USER_UPDATE", &CDiscordProto::OnCommandUserUpdate },
};

static int __cdecl pSearchFunc(const void *p1, const void *p2)
{
	return wcscmp(((CDiscordCommand*)p1)->szCommandId, ((CDiscordCommand*)p2)->szCommandId);
}

GatewayHandlerFunc CDiscordProto::GetHandler(const wchar_t *pwszCommand)
{
	CDiscordCommand tmp = { pwszCommand, nullptr };
	CDiscordCommand *p = (CDiscordCommand*)bsearch(&tmp, handlers, _countof(handlers), sizeof(handlers[0]), pSearchFunc);
	return (p != nullptr) ? p->pFunc : nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// channel operations

void CDiscordProto::OnCommandChannelCreated(const JSONNode &pRoot)
{
	SnowFlake guildId = ::getId(pRoot["guild_id"]);
	if (guildId == 0)
		PreparePrivateChannel(pRoot);
	else {
		// group channel for a guild
		CDiscordGuild *pGuild = FindGuild(guildId);
		if (pGuild && m_bUseGroupchats) {
			CDiscordUser *pUser = ProcessGuildChannel(pGuild, pRoot);
			if (pUser)
				CreateChat(pGuild, pUser);
		}
	}
}

void CDiscordProto::OnCommandChannelDeleted(const JSONNode &pRoot)
{
	CDiscordUser *pUser = FindUserByChannel(::getId(pRoot["id"]));
	if (pUser == nullptr)
		return;

	SnowFlake guildId = ::getId(pRoot["guild_id"]);
	if (guildId == 0) {
		pUser->channelId = pUser->lastMsgId = 0;
		delSetting(pUser->hContact, DB_KEY_CHANNELID);
	}
	else {
		CDiscordGuild *pGuild = FindGuild(guildId);
		if (pGuild != nullptr)
			Chat_Terminate(m_szModuleName, pUser->wszUsername, true);
	}
}

void CDiscordProto::OnCommandChannelUpdated(const JSONNode &pRoot)
{
	CDiscordUser *pUser = FindUserByChannel(::getId(pRoot["id"]));
	if (pUser == nullptr)
		return;

	pUser->lastMsgId = ::getId(pRoot["last_message_id"]);

	SnowFlake guildId = ::getId(pRoot["guild_id"]);
	if (guildId != 0) {
		CDiscordGuild *pGuild = FindGuild(guildId);
		if (pGuild == nullptr)
			return;

		CMStringW wszName = pRoot["name"].as_mstring();
		if (!wszName.IsEmpty()) {
			CMStringW wszNewName = pGuild->wszName + L"#" + wszName;
			Chat_ChangeSessionName(m_szModuleName, pUser->wszUsername, wszNewName);
		}

		CMStringW wszTopic = pRoot["topic"].as_mstring();
		Chat_SetStatusbarText(m_szModuleName, pUser->wszUsername, wszTopic);

		GCEVENT gce = { m_szModuleName, 0, GC_EVENT_TOPIC };
		gce.pszID.w	 = pUser->wszUsername;
		gce.pszText.w = wszTopic;
		gce.time = time(0);
		Chat_Event(&gce);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// reading a new message

void CDiscordProto::OnCommandFriendAdded(const JSONNode &pRoot)
{
	CDiscordUser *pUser = PrepareUser(pRoot["user"]);
	pUser->bIsPrivate = true;
	ProcessType(pUser, pRoot);
}

void CDiscordProto::OnCommandFriendRemoved(const JSONNode &pRoot)
{
	SnowFlake id = ::getId(pRoot["id"]);
	CDiscordUser *pUser = FindUser(id);
	if (pUser != nullptr) {
		if (pUser->hContact)
			if (pUser->bIsPrivate)
				db_delete_contact(pUser->hContact);

		arUsers.remove(pUser);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// guild synchronization

void CDiscordProto::OnCommandGuildCreated(const JSONNode &pRoot)
{
	if (m_bUseGroupchats)
		ProcessGuild(pRoot);
}

void CDiscordProto::OnCommandGuildDeleted(const JSONNode &pRoot)
{
	CDiscordGuild *pGuild = FindGuild(::getId(pRoot["id"]));
	if (pGuild == nullptr)
		return;

	for (auto &it : arUsers.rev_iter())
		if (it->pGuild == pGuild) {
			Chat_Terminate(m_szModuleName, it->wszUsername, true);
			arUsers.removeItem(&it);
		}

	Chat_Terminate(m_szModuleName, pRoot["name"].as_mstring(), true);

	arGuilds.remove(pGuild);
}

/////////////////////////////////////////////////////////////////////////////////////////
// guild members

void CDiscordProto::OnCommandGuildMemberAdded(const JSONNode&)
{
}

void CDiscordProto::OnCommandGuildMemberListUpdate(const JSONNode &pRoot)
{
	auto *pGuild = FindGuild(::getId(pRoot["guild_id"]));
	if (pGuild == nullptr)
		return;

	int iStatus = 0;

	for (auto &ops: pRoot["ops"]) {
		for (auto &it : ops["items"]) {
			auto &item = it.at((size_t)0);
			if (!mir_strcmp(item .name(), "group")) {
				iStatus = item ["id"].as_string() == "online" ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
				continue;
			}

			if (!mir_strcmp(item .name(), "member")) {
				bool bNew = false;
				auto *pm = ProcessGuildUser(pGuild, item, &bNew);
				pm->iStatus = iStatus;

				if (bNew)
					AddGuildUser(pGuild, *pm);
				else if (iStatus) {
					CMStringW wszUserId(FORMAT, L"%lld", pm->userId);

					GCEVENT gce = { m_szModuleName, 0, GC_EVENT_SETCONTACTSTATUS };
					gce.time = time(0);
					gce.pszUID.w = wszUserId;

					for (auto &cc : pGuild->arChannels) {
						if (!cc->bIsGroup)
							continue;
						
						gce.pszID.w = cc->wszChannelName;
						gce.dwItemData = iStatus;
						Chat_Event(&gce);
					}
				}
			}
		}
	}

	pGuild->bSynced = true;
}

void CDiscordProto::OnCommandGuildMemberRemoved(const JSONNode &pRoot)
{
	CDiscordGuild *pGuild = FindGuild(::getId(pRoot["guild_id"]));
	if (pGuild == nullptr)
		return;
	
	CMStringW wszUserId = pRoot["user"]["id"].as_mstring();

	for (auto &pUser : arUsers) {
		if (pUser->pGuild != pGuild)
			continue;

		GCEVENT gce = { m_szModuleName, 0, GC_EVENT_PART };
		gce.pszUID.w = pUser->wszUsername;
		gce.time = time(0);
		gce.pszUID.w = wszUserId;
		Chat_Event(&gce);
	}
}

void CDiscordProto::OnCommandGuildMemberUpdated(const JSONNode &pRoot)
{
	CDiscordGuild *pGuild = FindGuild(::getId(pRoot["guild_id"]));
	if (pGuild == nullptr)
		return;

	CMStringW wszUserId = pRoot["user"]["id"].as_mstring();
	CDiscordGuildMember *gm = pGuild->FindUser(_wtoi64(wszUserId));
	if (gm == nullptr)
		return;

	gm->wszNick = pRoot["nick"].as_mstring();
	if (gm->wszNick.IsEmpty())
		gm->wszNick = pRoot["user"]["username"].as_mstring() + L"#" + pRoot["user"]["discriminator"].as_mstring();

	for (auto &it : arUsers) {
		if (it->pGuild != pGuild)
			continue;

		CMStringW wszOldNick;
		SESSION_INFO *si = g_chatApi.SM_FindSession(it->wszUsername, m_szModuleName);
		if (si != nullptr) {
			USERINFO *ui = g_chatApi.UM_FindUser(si, wszUserId);
			if (ui != nullptr)
				wszOldNick = ui->pszNick;
		}

		GCEVENT gce = { m_szModuleName, 0, GC_EVENT_NICK };
		gce.pszID.w = it->wszUsername;
		gce.time = time(0);
		gce.pszUID.w = wszUserId;
		gce.pszNick.w = wszOldNick;
		gce.pszText.w = gm->wszNick;
		Chat_Event(&gce);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// roles

void CDiscordProto::OnCommandRoleCreated(const JSONNode &pRoot)
{
	CDiscordGuild *pGuild = FindGuild(::getId(pRoot["guild_id"]));
	if (pGuild != nullptr)
		ProcessRole(pGuild, pRoot["role"]);
}

void CDiscordProto::OnCommandRoleDeleted(const JSONNode &pRoot)
{
	CDiscordGuild *pGuild = FindGuild(::getId(pRoot["guild_id"]));
	if (pGuild == nullptr)
		return;

	SnowFlake id = ::getId(pRoot["role_id"]);
	CDiscordRole *pRole = pGuild->arRoles.find((CDiscordRole*)&id);
	if (pRole == nullptr)
		return;

	int iOldPosition = pRole->position;
	pGuild->arRoles.remove(pRole);

	for (auto &it : pGuild->arRoles)
		if (it->position > iOldPosition)
			it->position--;

	for (auto &it : arUsers) {
		if (it->pGuild != pGuild)
			continue;

		SESSION_INFO *si = g_chatApi.SM_FindSession(it->wszUsername, m_szModuleName);
		if (si != nullptr) {
			g_chatApi.TM_RemoveAll(&si->pStatuses);
			BuildStatusList(pGuild, si);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// reading a new message

void CDiscordProto::OnCommandMessageCreate(const JSONNode &pRoot)
{
	OnCommandMessage(pRoot, true);
}

void CDiscordProto::OnCommandMessageUpdate(const JSONNode &pRoot)
{
	OnCommandMessage(pRoot, false);
}

void CDiscordProto::OnCommandMessage(const JSONNode &pRoot, bool bIsNew)
{
	CMStringW wszMessageId = pRoot["id"].as_mstring();
	CMStringW wszUserId = pRoot["author"]["id"].as_mstring();
	SnowFlake userId = _wtoi64(wszUserId);
	SnowFlake msgId = _wtoi64(wszMessageId);

	// try to find a sender by his channel
	SnowFlake channelId = ::getId(pRoot["channel_id"]);
	CDiscordUser *pUser = FindUserByChannel(channelId);
	if (pUser == nullptr) {
		debugLogA("skipping message with unknown channel id=%lld", channelId);
		return;
	}

	char szMsgId[100];
	_i64toa_s(msgId, szMsgId, _countof(szMsgId), 10);

	COwnMessage ownMsg(::getId(pRoot["nonce"]), 0);
	COwnMessage *p = arOwnMessages.find(&ownMsg);
	if (p != nullptr) { // own message? skip it
		ProtoBroadcastAck(pUser->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)p->reqId, (LPARAM)szMsgId);
		debugLogA("skipping own message with nonce=%lld, id=%lld", ownMsg.nonce, msgId);
	}
	else {
		CMStringW wszText = PrepareMessageText(pRoot);
		if (wszText.IsEmpty())
			return;

		// old message? try to restore it from database
		bool bOurMessage = userId == m_ownId;
		if (!bIsNew) {
			MEVENT hOldEvent = db_event_getById(m_szModuleName, szMsgId);
			if (hOldEvent) {
				DB::EventInfo dbei;
				dbei.cbBlob = -1;
				if (!db_event_get(hOldEvent, &dbei)) {
					ptrW wszOldText(DbEvent_GetTextW(&dbei, CP_UTF8));
					if (wszOldText)
						wszText.Insert(0, wszOldText);
					if (dbei.flags & DBEF_SENT)
						bOurMessage = true;
				}
			}
		}

		const JSONNode &edited = pRoot["edited_timestamp"];
		if (!edited.isnull())
			wszText.AppendFormat(L" (%s %s)", TranslateT("edited at"), edited.as_mstring().c_str());

		if (pUser->bIsPrivate && !pUser->bIsGroup) {
			// if a message has myself as an author, add some flags
			PROTORECVEVENT recv = {};
			if (bOurMessage)
				recv.flags = PREF_CREATEREAD | PREF_SENT;

			debugLogA("store a message from private user %lld, channel id %lld", pUser->id, pUser->channelId);
			ptrA buf(mir_utf8encodeW(wszText));

			recv.timestamp = (DWORD)StringToDate(pRoot["timestamp"].as_mstring());
			recv.szMessage = buf;
			recv.szMsgId = szMsgId;
			ProtoChainRecvMsg(pUser->hContact, &recv);
		}
		else {
			debugLogA("store a message into the group channel id %lld", channelId);

			SESSION_INFO *si = g_chatApi.SM_FindSession(pUser->wszUsername, m_szModuleName);
			if (si == nullptr) {
				debugLogA("message to unknown channel %lld ignored", channelId);
				return;
			}

			ProcessChatUser(pUser, wszUserId, pRoot);

			ParseSpecialChars(si, wszText);
			wszText.Replace(L"%", L"%%");

			GCEVENT gce = { m_szModuleName, 0, GC_EVENT_MESSAGE };
			gce.pszID.w = pUser->wszUsername;
			gce.dwFlags = GCEF_ADDTOLOG;
			gce.pszUID.w = wszUserId;
			gce.pszText.w = wszText;
			gce.time = (DWORD)StringToDate(pRoot["timestamp"].as_mstring());
			gce.bIsMe = bOurMessage;
			Chat_Event(&gce);

			debugLogW(L"New channel %s message from %s: %s", si->ptszID, gce.pszUID.w, gce.pszText.w);
		}
	}

	pUser->lastMsgId = msgId;

	SnowFlake lastId = getId(pUser->hContact, DB_KEY_LASTMSGID); // as stored in a database
	if (lastId < msgId)
		setId(pUser->hContact, DB_KEY_LASTMSGID, msgId);
}

/////////////////////////////////////////////////////////////////////////////////////////
// someone changed its status

void CDiscordProto::OnCommandMessageAck(const JSONNode &pRoot)
{
	CDiscordUser *pUser = FindUserByChannel(pRoot["channel_id"]);
	if (pUser != nullptr)
		pUser->lastMsgId = ::getId(pRoot["message_id"]);
}

/////////////////////////////////////////////////////////////////////////////////////////
// message deleted

void CDiscordProto::OnCommandMessageDelete(const JSONNode &pRoot)
{
	if (!m_bSyncDeleteMsgs)
		return;

	CMStringA msgid(pRoot["id"].as_mstring());
	if (!msgid.IsEmpty()) {
		MEVENT hEvent = db_event_getById(m_szModuleName, msgid);
		if (hEvent)
			db_event_delete(hEvent);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// someone changed its status

void CDiscordProto::OnCommandPresence(const JSONNode &pRoot)
{
	auto *pGuild = FindGuild(::getId(pRoot["user"]["guild_id"]));
	if (pGuild == nullptr)
		ProcessPresence(pRoot);
	// else
		// pGuild->ProcessPresence(pRoot);
}

/////////////////////////////////////////////////////////////////////////////////////////
// gateway session start

void CDiscordProto::OnCommandReady(const JSONNode &pRoot)
{
	OnLoggedIn();

	GatewaySendHeartbeat();
	m_impl.m_heartBeat.StartSafe(m_iHartbeatInterval);

	m_szGatewaySessionId = pRoot["session_id"].as_mstring();

	if (m_bUseGroupchats)
		for (auto &it : pRoot["guilds"])
			ProcessGuild(it);

	for (auto &it : pRoot["relationships"]) {
		CDiscordUser *pUser = PrepareUser(it["user"]);
		ProcessType(pUser, it);
	}

	for (auto &it : pRoot["presences"])
		ProcessPresence(it);

	for (auto &it : pRoot["private_channels"])
		PreparePrivateChannel(it);

	for (auto &it : pRoot["read_state"]) {
		CDiscordUser *pUser = FindUserByChannel(::getId(it["id"]));
		if (pUser != nullptr)
			pUser->lastReadId = ::getId(it["last_message_id"]);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// UTN support

void CDiscordProto::OnCommandTyping(const JSONNode &pRoot)
{
	SnowFlake userId = ::getId(pRoot["user_id"]);
	SnowFlake channelId = ::getId(pRoot["channel_id"]);
	debugLogA("user typing notification: userid=%lld, channelid=%lld", userId, channelId);

	CDiscordUser *pUser = FindUser(userId);
	if (pUser == nullptr) {
		debugLogA("user with id=%lld is not found", userId);
		return;
	}

	if (pUser->channelId == channelId) {
		debugLogA("user is typing in his private channel");
		CallService(MS_PROTO_CONTACTISTYPING, pUser->hContact, 20);
	}
	else {
		debugLogA("user is typing in a group channel, skipped");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// User info update

void CDiscordProto::OnCommandUserUpdate(const JSONNode &pRoot)
{
	SnowFlake id = ::getId(pRoot["id"]);

	MCONTACT hContact;
	if (id != m_ownId) {
		CDiscordUser *pUser = FindUser(id);
		if (pUser == nullptr)
			return;

		hContact = pUser->hContact;
	}
	else hContact = 0;

	// force rereading avatar
	CheckAvatarChange(hContact, pRoot["avatar"].as_mstring());
}

void CDiscordProto::OnCommandUserSettingsUpdate(const JSONNode &pRoot)
{
	int iStatus = StrToStatus(pRoot["status"].as_mstring());
	if (iStatus != 0) {
		int iOldStatus = m_iStatus; m_iStatus = iStatus;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	}
}

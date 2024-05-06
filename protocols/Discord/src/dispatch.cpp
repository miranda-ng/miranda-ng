/*
Copyright © 2016-22 Miranda NG team

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
	{ L"CHANNEL_RECIPIENT_ADD", &CDiscordProto::OnCommandChannelUserAdded },
	{ L"CHANNEL_RECIPIENT_REMOVE", &CDiscordProto::OnCommandChannelUserLeft },
	{ L"CHANNEL_UNREAD_UPDATE", &CDiscordProto::OnCommandChannelUnreadUpdate },
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
		if (auto *si = pUser->si) {
			Chat_Control(si, SESSION_OFFLINE);
			Chat_Terminate(si);
		}
		db_delete_contact(pUser->hContact);
	}
	else {
		CDiscordGuild *pGuild = FindGuild(guildId);
		if (pGuild != nullptr) {
			db_delete_contact(pUser->si->hContact);
			pUser->si = nullptr;
		}
	}
	arUsers.remove(pUser);
}

void CDiscordProto::OnCommandChannelUserAdded(const JSONNode &pRoot)
{
	CDiscordUser *pUser = FindUserByChannel(::getId(pRoot["channel_id"]));
	if (pUser == nullptr || pUser->si == nullptr)
		return;

	auto nUser = pRoot["user"];
	CMStringW wszUserId = nUser["id"].as_mstring();
	CMStringW wszNick = getNick(nUser);

	GCEVENT gce = { pUser->si, GC_EVENT_JOIN };
	gce.dwFlags = GCEF_SILENT;
	gce.pszUID.w = wszUserId;
	gce.pszNick.w = wszNick;
	gce.time = time(0);
	Chat_Event(&gce);
}

void CDiscordProto::OnCommandChannelUserLeft(const JSONNode &pRoot)
{
	CDiscordUser *pUser = FindUserByChannel(::getId(pRoot["channel_id"]));
	if (pUser == nullptr || pUser->si == nullptr)
		return;

	CMStringW wszUserId = pRoot["user"]["id"].as_mstring();

	GCEVENT gce = { pUser->si, GC_EVENT_PART };
	gce.dwFlags = GCEF_SILENT;
	gce.pszUID.w = wszUserId;
	gce.time = time(0);
	Chat_Event(&gce);
}

void CDiscordProto::OnCommandChannelUnreadUpdate(const JSONNode &pRoot)
{
	auto *pGuild = FindGuild(::getId(pRoot["guild_id"]));
	if (pGuild == nullptr)
		return;

	for (auto &it : pRoot["channel_unread_updates"])
		if (auto *pChannel = FindUserByChannel(::getId(it["id"])))
			if (pChannel->lastMsgId < ::getId(it["last_message_id"]))
				RetrieveHistory(pChannel, MSG_AFTER, pChannel->lastMsgId, 99);
}

void CDiscordProto::OnCommandChannelUpdated(const JSONNode &pRoot)
{
	CDiscordUser *pUser = FindUserByChannel(::getId(pRoot["id"]));
	if (pUser == nullptr)
		return;

	pUser->lastMsgId = ::getId(pRoot["last_message_id"]);

	SnowFlake ownerId = ::getId(pRoot["owner_id"]);
	setId(pUser->hContact, DB_KEY_OWNERID, ownerId);

	// if channel name was changed
	CMStringW wszName = pRoot["name"].as_mstring();
	if (!wszName.IsEmpty()) {
		SnowFlake guildId = ::getId(pRoot["guild_id"]);
		if (guildId != 0) {
			CDiscordGuild *pGuild = FindGuild(guildId);
			if (pGuild == nullptr)
				return;

			CMStringW wszNewName = pGuild->m_wszName + L"#" + wszName;
			Chat_ChangeSessionName(pUser->si, wszNewName);
		}
		else Chat_ChangeSessionName(pUser->si, wszName);
	}

	// if a topic was changed
	CMStringW wszTopic = pRoot["topic"].as_mstring();
	Chat_SetStatusbarText(pUser->si, wszTopic);
	{
		GCEVENT gce = { pUser->si, GC_EVENT_TOPIC };
		gce.pszText.w = wszTopic;
		gce.time = time(0);
		Chat_Event(&gce);
	}

	// reset members info for private channels
	if (pUser->pGuild == nullptr) {
		for (auto &it : pUser->si->arUsers) {
			SnowFlake userId = _wtoi64(it->pszUID);

			GCEVENT gce = { pUser->si, GC_EVENT_SETSTATUS };
			gce.pszUID.w = it->pszUID;
			gce.pszStatus.w = (userId == ownerId) ? L"Owners" : L"Participants";
			gce.bIsMe = userId == m_ownId;
			gce.time = time(0);
			Chat_Event(&gce);
		}
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
			db_delete_contact(it->si->hContact);
			arUsers.removeItem(&it);
		}

	db_delete_contact(pGuild->pParentSi->hContact);
	pGuild->pParentSi = nullptr;

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

					GCEVENT gce = { 0, GC_EVENT_SETCONTACTSTATUS };
					gce.time = time(0);
					gce.pszUID.w = wszUserId;

					for (auto &cc : pGuild->arChannels) {
						if (!cc->bIsGroup)
							continue;
						
						gce.si = cc->si;
						gce.dwItemData = iStatus;
						Chat_Event(&gce);
					}
				}
			}
		}
	}

	pGuild->m_bSynced = true;
}

void CDiscordProto::OnCommandGuildMemberRemoved(const JSONNode &pRoot)
{
	CDiscordGuild *pGuild = FindGuild(::getId(pRoot["guild_id"]));
	if (pGuild == nullptr)
		return;
	
	CMStringW wszUserId = pRoot["user"]["id"].as_mstring();
	auto *gm = pGuild->FindUser(_wtoi64(wszUserId));
	if (gm == nullptr)
		return;

	// remove a user once from the common list of users
	GCEVENT gce = { pGuild->pParentSi, GC_EVENT_PART };
	gce.pszNick.w = gm->wszNick;
	gce.time = time(0);
	gce.pszUID.w = wszUserId;
	Chat_Event(&gce);

	// then update every nicklist for any opened chat
	for (auto &cc: arUsers) {
		if (cc->pGuild != pGuild)
			continue;

		if (cc->si && cc->si->pDlg)
			cc->si->pDlg->UpdateNickList();
	}

	pGuild->arChatUsers.remove(gm);
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

	gm->wszDiscordId = getNick(pRoot["user"]);
	gm->wszNick = pRoot["nick"].as_mstring();
	if (gm->wszNick.IsEmpty())
		gm->wszNick = pRoot["user"]["username"].as_mstring();

	for (auto &it : arUsers) {
		if (it->pGuild != pGuild || !it->si)
			continue;

		CMStringW wszOldNick;
		SESSION_INFO *si = it->si;
		if (si != nullptr) {
			USERINFO *ui = g_chatApi.UM_FindUser(si, wszUserId);
			if (ui != nullptr)
				wszOldNick = ui->pszNick;
		}

		GCEVENT gce = { si, GC_EVENT_NICK };
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
	if (auto *pGuild = FindGuild(::getId(pRoot["guild_id"])))
		pGuild->ProcessRole(pRoot["role"]);
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

		SESSION_INFO *si = Chat_Find(it->wszUsername, m_szModuleName);
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
	CMStringA szUserId = pRoot["author"]["id"].as_mstring();
	SnowFlake userId = _atoi64(szUserId);
	SnowFlake msgId = _wtoi64(wszMessageId);

	// try to find a sender by his channel
	SnowFlake channelId = ::getId(pRoot["channel_id"]);
	CDiscordUser *pUser = FindUserByChannel(channelId);
	if (pUser == nullptr) {
		debugLogA("skipping message with unknown channel id=%lld", channelId);
		return;
	}

	// shift & store LastMsgId field
	pUser->lastMsgId = msgId;

	SnowFlake lastId = getId(pUser->hContact, DB_KEY_LASTMSGID); // as stored in a database
	if (lastId < msgId)
		setId(pUser->hContact, DB_KEY_LASTMSGID, msgId);

	char szMsgId[100];
	_i64toa_s(msgId, szMsgId, _countof(szMsgId), 10);

	COwnMessage ownMsg(::getId(pRoot["nonce"]), 0);
	COwnMessage *p = arOwnMessages.find(&ownMsg);
	if (p != nullptr) { // own message? skip it
		ProtoBroadcastAck(pUser->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)p->reqId, (LPARAM)szMsgId);
		debugLogA("skipping own message with nonce=%lld, id=%lld", ownMsg.nonce, msgId);
	}
	else {
		CMStringW wszText = PrepareMessageText(pRoot), wszMentioned;
		SnowFlake mentionId = 0;

		for (auto &it : pRoot["mentions"]) {
			wszMentioned = getName(it);
			mentionId = _wtoi64(it["id"].as_mstring());
			break;
		}

		switch (pRoot["type"].as_int()) {
		case 4: // chat was renamed
			if (pUser->si)
				setWString(pUser->si->hContact, "Nick", wszText);
			return;

		case 1: // user was added to chat
			if (mentionId != userId)
				wszText.Format(TranslateT("%s added %s to the group"), getName(pRoot["author"]).c_str(), wszMentioned.c_str());
			else
				wszText.Format(TranslateT("%s joined the group"), wszMentioned.c_str());
			break;

		case 2: // user was removed from chat
			if (mentionId != userId)
				wszText.Format(TranslateT("%s removed %s from the group"), getName(pRoot["author"]).c_str(), wszMentioned.c_str());
			else
				wszText.Format(TranslateT("%s left the group"), wszMentioned.c_str());
			break;
		}

		if (wszText.IsEmpty())
			return;

		else {
			// old message? try to restore it from database
			bool bOurMessage = userId == m_ownId;
			if (!bIsNew) {
				MEVENT hOldEvent = db_event_getById(m_szModuleName, szMsgId);
				if (hOldEvent) {
					DB::EventInfo dbei;
					dbei.cbBlob = -1;
					if (!db_event_get(hOldEvent, &dbei)) {
						ptrW wszOldText(DbEvent_GetTextW(&dbei));
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

			// if a message has myself as an author, add some flags
			DB::EventInfo dbei;
			if (bOurMessage)
				dbei.flags = DBEF_READ | DBEF_SENT;

			debugLogA("store a message from private user %lld, channel id %lld", pUser->id, pUser->channelId);
			ptrA buf(mir_utf8encodeW(wszText));

			dbei.timestamp = (uint32_t)StringToDate(pRoot["timestamp"].as_mstring());
			dbei.pBlob = buf;
			dbei.szId = szMsgId;

			if (!pUser->bIsPrivate || pUser->bIsGroup) {
				dbei.szUserId = szUserId;
				ProcessChatUser(pUser, userId, pRoot);
			}

			ProtoChainRecvMsg(pUser->hContact, dbei);
		}
	}

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
			db_event_delete(hEvent, CDF_FROM_SERVER);
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
	SnowFlake channelId = ::getId(pRoot["channel_id"]);
	debugLogA("user typing notification: channelid=%lld", channelId);

	CDiscordUser *pChannel = FindUserByChannel(channelId);
	if (pChannel == nullptr) {
		debugLogA("channel with id=%lld is not found", channelId);
		return;
	}

	// both private groupchats & guild channels are chat rooms for Miranda
	if (pChannel->pGuild) {
		debugLogA("user is typing in a group channel");

		CMStringW wszUerId = pRoot["user_id"].as_mstring();
		ProcessGuildUser(pChannel->pGuild, pRoot); // never returns null

		GCEVENT gce = { pChannel->si, GC_EVENT_TYPING };
		gce.pszUID.w = wszUerId;
		gce.dwItemData = 1;
		gce.time = time(0);
		Chat_Event(&gce);
	}
	else {
		debugLogA("user is typing in his private channel");
		CallService(MS_PROTO_CONTACTISTYPING, pChannel->hContact, 20);
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

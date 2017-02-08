/*
Copyright © 2016-17 Miranda NG team

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
	{ L"CHANNEL_CREATE", &CDiscordProto::OnCommandChannelCreated },
	{ L"CHANNEL_DELETE", &CDiscordProto::OnCommandChannelDeleted },

	{ L"GUILD_CREATE", &CDiscordProto::OnCommandGuildCreate },
	{ L"GUILD_DELETE", &CDiscordProto::OnCommandGuildDelete },
	{ L"GUILD_MEMBER_REMOVE", &CDiscordProto::OnCommandGuildRemoveMember },
	{ L"GUILD_MEMBER_UPDATE", &CDiscordProto::OnCommandGuildUpdateMember },
	{ L"GUILD_SYNC", &CDiscordProto::OnCommandGuildSync },

	{ L"MESSAGE_ACK", &CDiscordProto::OnCommandMessageAck },
	{ L"MESSAGE_CREATE", &CDiscordProto::OnCommandMessage },
	{ L"MESSAGE_UPDATE", &CDiscordProto::OnCommandMessage },

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
	CDiscordCommand tmp = { pwszCommand, NULL };
	CDiscordCommand *p = (CDiscordCommand*)bsearch(&tmp, handlers, _countof(handlers), sizeof(handlers[0]), pSearchFunc);
	return (p != NULL) ? p->pFunc : NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// channel operations

void CDiscordProto::OnCommandChannelCreated(const JSONNode &pRoot)
{
	const JSONNode &members = pRoot["recipients"];
	for (auto it = members.begin(); it != members.end(); ++it) {
		CDiscordUser *pUser = PrepareUser(*it);
		pUser->lastMessageId = ::getId(pRoot["last_message_id"]);
		pUser->channelId = ::getId(pRoot["id"]);
		setId(pUser->hContact, DB_KEY_CHANNELID, pUser->channelId);
	}
}

void CDiscordProto::OnCommandChannelDeleted(const JSONNode &pRoot)
{
	CDiscordUser *pUser = FindUserByChannel(::getId(pRoot["id"]));
	if (pUser != NULL) {
		pUser->channelId = pUser->lastMessageId = 0;
		delSetting(pUser->hContact, DB_KEY_CHANNELID);
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
	if (pUser != NULL) {
		if (pUser->hContact) {
			if (pUser->bIsPrivate)
				db_delete_contact(pUser->hContact);
		}
		arUsers.remove(pUser);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// guild synchronization

static int sttGetPresence(const JSONNode &pStatuses, const CMStringW &wszId)
{
	for (auto it = pStatuses.begin(); it != pStatuses.end(); ++it) {
		const JSONNode &s = *it;

		CMStringW wszUserId = s["user"]["id"].as_mstring();
		if (wszUserId == wszId)
			return StrToStatus(s["status"].as_mstring());
	}

	return 0;
}

static SnowFlake sttGetLastRead(const JSONNode &reads, const wchar_t *wszChannelId)
{
	for (auto it = reads.begin(); it != reads.end(); ++it) {
		const JSONNode &p = *it;

		if (p["id"].as_mstring() == wszChannelId)
			return ::getId(p["last_message_id"]);
	}
	return 0;
}

void CDiscordProto::ProcessGuild(const JSONNode &readState, const JSONNode &p)
{
	SnowFlake guildId = ::getId(p["id"]);
	GatewaySendGuildInfo(guildId);
	CMStringW wszGuildName = p["name"].as_mstring();

	GCSessionInfoBase *si = Chat_NewSession(GCW_SERVER, m_szModuleName, wszGuildName, wszGuildName);
	Chat_Control(m_szModuleName, wszGuildName, WINDOW_HIDDEN);
	Chat_Control(m_szModuleName, wszGuildName, SESSION_ONLINE);
	setId(si->hContact, DB_KEY_CHANNELID, guildId);

	const JSONNode &channels = p["channels"];
	for (auto itc = channels.begin(); itc != channels.end(); ++itc) {
		const JSONNode &pch = *itc;
		if (pch["type"].as_int() != 0)
			continue;

		CMStringW wszChannelName = pch["name"].as_mstring();
		CMStringW wszChannelId = pch["id"].as_mstring();
		SnowFlake channelId = _wtoi64(wszChannelId);

		si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszChannelId, wszGuildName + L"#" + wszChannelName);
		Chat_AddGroup(m_szModuleName, wszChannelId, TranslateT("User"));
		Chat_Control(m_szModuleName, wszChannelId, WINDOW_HIDDEN);
		Chat_Control(m_szModuleName, wszChannelId, SESSION_ONLINE);

		CDiscordUser *pUser = FindUserByChannel(channelId);
		if (pUser == NULL) {
			// missing channel - create it
			pUser = new CDiscordUser(channelId);
			pUser->bIsPrivate = false;
			pUser->hContact = si->hContact;
			pUser->channelId = channelId;
			arUsers.insert(pUser);
		}
		pUser->wszUsername = wszChannelId;
		pUser->guildId = guildId;
		pUser->lastMessageId = ::getId(pch["last_message_id"]);
		pUser->lastReadId = sttGetLastRead(readState, wszChannelId);

		setId(pUser->hContact, DB_KEY_CHANNELID, channelId);
	}
}

void CDiscordProto::OnCommandGuildCreate(const JSONNode &pRoot)
{
	ProcessGuild(JSONNode(), pRoot);
	OnCommandGuildSync(pRoot);
}

void CDiscordProto::OnCommandGuildSync(const JSONNode &pRoot)
{
	const JSONNode &pStatuses = pRoot["presences"];

	SnowFlake guildId = ::getId(pRoot["id"]);

	const JSONNode &pMembers = pRoot["members"];
	for (auto it = pMembers.begin(); it != pMembers.end(); ++it) {
		const JSONNode &m = *it;

		for (int i = 0; i < arUsers.getCount(); i++) {
			CDiscordUser &pUser = arUsers[i];
			if (pUser.guildId != guildId)
				continue;

			GCDEST gcd = { m_szModuleName, pUser.wszUsername, GC_EVENT_JOIN };
			GCEVENT gce = { &gcd };

			CMStringW wszNick = m["nick"].as_mstring();
			CMStringW wszUsername = m["user"]["username"].as_mstring() + L"#" + m["user"]["discriminator"].as_mstring();
			CMStringW wszUserId = m["user"]["id"].as_mstring();
			SnowFlake userid = _wtoi64(wszUserId);
			gce.bIsMe = (userid == m_ownId);
			gce.ptszUID = wszUserId;
			gce.ptszNick = wszNick.IsEmpty() ? wszUsername : wszNick;
			Chat_Event(&gce);

			int flags = GC_SSE_ONLYLISTED;
			switch (sttGetPresence(pStatuses, wszUserId)) {
			case ID_STATUS_ONLINE: case ID_STATUS_NA: case ID_STATUS_DND:
				flags += GC_SSE_ONLINE;
				break;
			default:
				flags += GC_SSE_OFFLINE;
			}
			Chat_SetStatusEx(m_szModuleName, pUser.wszUsername, flags, wszUserId);
		}
	}
}

void CDiscordProto::OnCommandGuildDelete(const JSONNode &pRoot)
{
	SnowFlake guildId = ::getId(pRoot["id"]);

	for (int i = arUsers.getCount()-1; i >= 0; i--) {
		CDiscordUser &pUser = arUsers[i];
		if (pUser.guildId == guildId) {
			Chat_Terminate(m_szModuleName, pUser.wszUsername, true);
			arUsers.remove(i);
		}
	}

	Chat_Terminate(m_szModuleName, pRoot["name"].as_mstring(), true);
}

void CDiscordProto::OnCommandGuildRemoveMember(const JSONNode &pRoot)
{
	SnowFlake guildId = ::getId(pRoot["guild_id"]);
	CMStringW wszUserId = pRoot["user"]["id"].as_mstring();

	for (int i = 0; i < arUsers.getCount(); i++) {
		CDiscordUser &pUser = arUsers[i];
		if (pUser.guildId != guildId)
			continue;

		GCDEST gcd = { m_szModuleName, pUser.wszUsername, GC_EVENT_PART };
		GCEVENT gce = { &gcd };
		gce.ptszUID = wszUserId;
		Chat_Event(&gce);
	}
}

void CDiscordProto::OnCommandGuildUpdateMember(const JSONNode &pRoot)
{
	SnowFlake guildId = ::getId(pRoot["guild_id"]);
	CMStringW wszUserId = pRoot["user"]["id"].as_mstring();
	CMStringW wszUserNick = pRoot["nick"].as_mstring(), wszOldNick;

	for (int i = 0; i < arUsers.getCount(); i++) {
		CDiscordUser &pUser = arUsers[i];
		if (pUser.guildId != guildId)
			continue;

		SESSION_INFO *si = pci->SM_FindSession(pUser.wszUsername, m_szModuleName);
		if (si != nullptr) {
			USERINFO *ui = pci->UM_FindUser(si->pUsers, wszUserId);
			if (ui != nullptr)
				wszOldNick = ui->pszNick;
		}

		GCDEST gcd = { m_szModuleName, pUser.wszUsername, GC_EVENT_NICK };
		GCEVENT gce = { &gcd };
		gce.ptszUID = wszUserId;
		gce.ptszNick = wszOldNick;
		gce.ptszText = wszUserNick;
		Chat_Event(&gce);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
// reading a new message

void CDiscordProto::OnCommandMessage(const JSONNode &pRoot)
{
	PROTORECVEVENT recv = {};
	CMStringW wszMessageId = pRoot["id"].as_mstring();
	SnowFlake messageId = _wtoi64(wszMessageId);
	SnowFlake nonce = ::getId(pRoot["nonce"]);

	SnowFlake *p = arOwnMessages.find(&nonce);
	if (p != NULL) { // own message? skip it
		debugLogA("skipping own message with nonce=%lld, id=%lld", nonce, messageId);
		return;
	}

	// try to find a sender by his channel
	CMStringW wszChannelId = pRoot["channel_id"].as_mstring();
	SnowFlake channelId = _wtoi64(wszChannelId);
	CDiscordUser *pUser = FindUserByChannel(channelId);
	if (pUser == NULL) {
		debugLogA("skipping message with unknown channel id=%lld", channelId);
		return;
	}

	// if a message has myself as an author, add some flags
	CMStringW wszUserId = pRoot["author"]["id"].as_mstring();
	if (_wtoi64(wszUserId) == m_ownId)
		recv.flags = PREF_CREATEREAD | PREF_SENT;

	CMStringW wszText = PrepareMessageText(pRoot);

	const JSONNode &edited = pRoot["edited_timestamp"];
	if (!edited.isnull())
		wszText.AppendFormat(L" (%s %s)", TranslateT("edited at"), edited.as_mstring().c_str());

	if (pUser->bIsPrivate) {
		debugLogA("store a message from private user %lld, channel id %lld", pUser->id, pUser->channelId);
		ptrA buf(mir_utf8encodeW(wszText));
		recv.timestamp = (DWORD)StringToDate(pRoot["timestamp"].as_mstring());
		recv.szMessage = buf;
		recv.lParam = (LPARAM)wszMessageId.c_str();
		ProtoChainRecvMsg(pUser->hContact, &recv);

		SnowFlake lastId = getId(pUser->hContact, DB_KEY_LASTMSGID); // as stored in a database
		if (lastId < messageId)
			setId(pUser->hContact, DB_KEY_LASTMSGID, messageId);
	}
	else {
		debugLogA("store a message into the group channel id %lld", channelId);

		GCDEST gcd = { m_szModuleName, wszChannelId, GC_EVENT_MESSAGE };
		GCEVENT gce = { &gcd };
		gce.dwFlags = GCEF_ADDTOLOG;
		gce.ptszUID = wszUserId;
		gce.ptszText = wszText;
		gce.time = (DWORD)StringToDate(pRoot["timestamp"].as_mstring());
		gce.bIsMe = _wtoi64(wszUserId) == m_ownId;
		Chat_Event(&gce);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// someone changed its status

void CDiscordProto::OnCommandMessageAck(const JSONNode &pRoot)
{
	CDiscordUser *pUser = FindUserByChannel(pRoot["channel_id"]);
	if (pUser != NULL)
		pUser->lastMessageId = ::getId(pRoot["message_id"]);
}

/////////////////////////////////////////////////////////////////////////////////////////
// someone changed its status

void CDiscordProto::OnCommandPresence(const JSONNode &pRoot)
{
	CDiscordUser *pUser = FindUser(::getId(pRoot["user"]["id"]));
	if (pUser == NULL)
		return;

	int iStatus = StrToStatus(pRoot["status"].as_mstring());
	if (iStatus != 0)
		setWord(pUser->hContact, "Status", iStatus);

	CMStringW wszGame = pRoot["game"]["name"].as_mstring();
	if (!wszGame.IsEmpty())
		setWString(pUser->hContact, "XStatusMsg", wszGame);
	else
		delSetting(pUser->hContact, "XStatusMsg");		
}

/////////////////////////////////////////////////////////////////////////////////////////
// gateway session start

void CALLBACK CDiscordProto::HeartbeatTimerProc(HWND, UINT, UINT_PTR id, DWORD)
{
	((CDiscordProto*)id)->GatewaySendHeartbeat();
}

static void __stdcall sttStartTimer(void *param)
{
	CDiscordProto *ppro = (CDiscordProto*)param;
	SetTimer(g_hwndHeartbeat, (UINT_PTR)param, ppro->getHeartbeatInterval(), &CDiscordProto::HeartbeatTimerProc);
}

void CDiscordProto::OnCommandReady(const JSONNode &pRoot)
{
	GatewaySendHeartbeat();
	CallFunctionAsync(sttStartTimer, this);

	m_szGatewaySessionId = pRoot["session_id"].as_mstring();

	const JSONNode &readState = pRoot["read_state"];
	const JSONNode &pStatuses = pRoot["presences"];

	const JSONNode &guilds = pRoot["guilds"];
	for (auto it = guilds.begin(); it != guilds.end(); ++it)
		ProcessGuild(readState, *it);

	const JSONNode &relations = pRoot["relationships"];
	for (auto it = relations.begin(); it != relations.end(); ++it) {
		const JSONNode &p = *it;

		CDiscordUser *pUser = PrepareUser(p["user"]);
		ProcessType(pUser, p);

		int iStatus = sttGetPresence(pStatuses, p["user"]["id"].as_mstring());
		if (iStatus)
			setWord(pUser->hContact, "Status", iStatus);
	}

	const JSONNode &channels = pRoot["private_channels"];
	for (auto it = channels.begin(); it != channels.end(); ++it) {
		const JSONNode &p = *it;

		CDiscordUser *pUser = NULL;
		const JSONNode &recipients = p["recipients"];
		for (auto it2 = recipients.begin(); it2 != recipients.end(); ++it2)
			pUser = PrepareUser(*it2);

		if (pUser == NULL)
			continue;
		
		CMStringW wszChannelId = p["id"].as_mstring();
		pUser->channelId = _wtoi64(wszChannelId);
		pUser->lastMessageId = ::getId(p["last_message_id"]);
		pUser->lastReadId = sttGetLastRead(readState, wszChannelId);
		pUser->bIsPrivate = true;

		setId(pUser->hContact, DB_KEY_CHANNELID, pUser->channelId);

		SnowFlake oldMsgId = getId(pUser->hContact, DB_KEY_LASTMSGID);
		if (pUser->lastMessageId > oldMsgId)
			RetrieveHistory(pUser->hContact, MSG_AFTER, oldMsgId, 99);
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
	if (pUser == NULL) {
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
		if (pUser == NULL)
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
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	}
}

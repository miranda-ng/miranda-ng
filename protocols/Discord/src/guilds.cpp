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

int compareUsers(const CDiscordUser *p1, const CDiscordUser *p2);

static int compareRoles(const CDiscordRole *p1, const CDiscordRole *p2)
{
	return p1->id - p2->id;
}

static int compareChatUsers(const CDiscordGuildMember *p1, const CDiscordGuildMember *p2)
{
	return p1->userId - p2->userId;
}

CDiscordGuild::CDiscordGuild(SnowFlake _id)
	: id(_id),
	arChatUsers(30, compareChatUsers),
	arRoles(10, compareRoles)
{
}

CDiscordGuild::~CDiscordGuild()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// reads a role from json

void CDiscordProto::ProcessRole(CDiscordGuild *guild, const JSONNode &role)
{
	SnowFlake id = ::getId(role["id"]);
	CDiscordRole *p = guild->arRoles.find((CDiscordRole*)&id);
	if (p == nullptr) {
		p = new CDiscordRole();
		p->id = id;
		guild->arRoles.insert(p);
	}

	p->color = role["color"].as_int();
	p->position = role["position"].as_int();
	p->permissions = role["permissions"].as_int();
	p->wszName = role["name"].as_mstring();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::ProcessGuild(const JSONNode &p)
{
	SnowFlake guildId = ::getId(p["id"]);
	GatewaySendGuildInfo(guildId);

	CDiscordGuild *pGuild = FindGuild(guildId);
	if (pGuild == nullptr) {
		pGuild = new CDiscordGuild(guildId);
		arGuilds.insert(pGuild);
	}
	pGuild->ownerId = ::getId(p["owner_id"]);
	pGuild->wszName = p["name"].as_mstring();

	GCSessionInfoBase *si = Chat_NewSession(GCW_SERVER, m_szModuleName, pGuild->wszName, pGuild->wszName, pGuild);
	Chat_Control(m_szModuleName, pGuild->wszName, WINDOW_HIDDEN);
	Chat_Control(m_szModuleName, pGuild->wszName, SESSION_ONLINE);
	
	pGuild->hContact = si->hContact;
	setId(si->hContact, DB_KEY_CHANNELID, guildId);

	const JSONNode &roles = p["roles"];
	for (auto itr = roles.begin(); itr != roles.end(); ++itr)
		ProcessRole(pGuild, *itr);

	const JSONNode &channels = p["channels"];
	for (auto itc = channels.begin(); itc != channels.end(); ++itc)
		ProcessGuildChannel(pGuild, *itc);
}

/////////////////////////////////////////////////////////////////////////////////////////

CDiscordUser* CDiscordProto::ProcessGuildChannel(CDiscordGuild *pGuild, const JSONNode &pch)
{
	// filter our all channels but the text ones
	if (pch["type"].as_int() != 0)
		return nullptr;

	CMStringW wszChannelName = pGuild->wszName + L"#" + pch["name"].as_mstring();
	CMStringW wszChannelId = pch["id"].as_mstring();
	CMStringW wszTopic = pch["topic"].as_mstring();
	SnowFlake channelId = _wtoi64(wszChannelId);

	GCSessionInfoBase *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszChannelId, wszChannelName);
	BuildStatusList(pGuild, wszChannelId);

	Chat_Control(m_szModuleName, wszChannelId, WINDOW_HIDDEN);
	Chat_Control(m_szModuleName, wszChannelId, SESSION_ONLINE);

	if (!wszTopic.IsEmpty()) {
		Chat_SetStatusbarText(m_szModuleName, wszChannelId, wszTopic);

		GCDEST gcd = { m_szModuleName, wszChannelId, GC_EVENT_TOPIC };
		GCEVENT gce = { &gcd };
		gce.time = time(0);
		gce.ptszText = wszTopic;
		Chat_Event(&gce);
	}

	CDiscordUser *pUser = FindUserByChannel(channelId);
	if (pUser == nullptr) {
		// missing channel - create it
		pUser = new CDiscordUser(channelId);
		pUser->bIsPrivate = false;
		pUser->hContact = si->hContact;
		pUser->id = channelId;
		pUser->channelId = channelId;
		arUsers.insert(pUser);
	}
	pUser->wszUsername = wszChannelId;
	pUser->guildId = pGuild->id;
	pUser->lastMsg = CDiscordMessage(::getId(pch["last_message_id"]));

	setId(pUser->hContact, DB_KEY_ID, channelId);
	setId(pUser->hContact, DB_KEY_CHANNELID, channelId);

	SnowFlake oldMsgId = getId(pUser->hContact, DB_KEY_LASTMSGID);
	if (oldMsgId != 0 && pUser->lastMsg.id > oldMsgId)
		RetrieveHistory(pUser->hContact, MSG_AFTER, oldMsgId, 99);

	return pUser;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::ApplyUsersToChannel(CDiscordGuild *pGuild, const CDiscordUser &pUser)
{
	GCDEST gcd = { m_szModuleName, pUser.wszUsername, GC_EVENT_JOIN };
	GCEVENT gce = { &gcd };
	gce.time = time(0);
	gce.dwFlags = GCEF_SILENT;

	for (int i = 0; i < pGuild->arChatUsers.getCount(); i++) {
		CDiscordGuildMember &m = pGuild->arChatUsers[i];

		wchar_t wszUserId[100];
		_i64tow_s(m.userId, wszUserId, _countof(wszUserId), 10);

		gce.ptszStatus = m.wszRole;
		gce.bIsMe = (m.userId == m_ownId);
		gce.ptszUID = wszUserId;
		gce.ptszNick = m.wszNick;
		Chat_Event(&gce);

		int flags = GC_SSE_ONLYLISTED;
		switch (m.iStatus) {
		case ID_STATUS_ONLINE: case ID_STATUS_NA: case ID_STATUS_DND:
			flags += GC_SSE_ONLINE;
			break;
		default:
			flags += GC_SSE_OFFLINE;
		}
		Chat_SetStatusEx(m_szModuleName, pUser.wszUsername, flags, wszUserId);
	}
}

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

static int compareRoles(const CDiscordRole *p1, const CDiscordRole *p2)
{
	return compareInt64(p1->id, p2->id);
}

static int compareChatUsers(const CDiscordGuildMember *p1, const CDiscordGuildMember *p2)
{
	return compareInt64(p1->userId, p2->userId);
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

	CDiscordGuild *pGuild = FindGuild(guildId);
	if (pGuild == nullptr) {
		pGuild = new CDiscordGuild(guildId);
		arGuilds.insert(pGuild);

		GatewaySendGuildInfo(guildId);
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
	CMStringW wszChannelId = pch["id"].as_mstring();
	SnowFlake channelId = _wtoi64(wszChannelId);
	CMStringW wszChannelName = pGuild->wszName + L"#" + pch["name"].as_mstring();

	// filter our all channels but the text ones
	if (pch["type"].as_int() != 0) {
		return nullptr;
	}

	CMStringW wszTopic = pch["topic"].as_mstring();

	GCSessionInfoBase *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszChannelId, wszChannelName);
	BuildStatusList(pGuild, wszChannelId);

	Chat_Control(m_szModuleName, wszChannelId, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(m_szModuleName, wszChannelId, SESSION_ONLINE);

	if (!wszTopic.IsEmpty()) {
		Chat_SetStatusbarText(m_szModuleName, wszChannelId, wszTopic);

		GCEVENT gce = { m_szModuleName, wszChannelId, GC_EVENT_TOPIC };
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

	for (int i = 0; i < pGuild->arChatUsers.getCount(); i++)
		AddUserToChannel(*pUser, pGuild->arChatUsers[i]);

	return pUser;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::AddUserToChannel(const CDiscordUser &pChannel, const CDiscordGuildMember &pUser)
{
	GCEVENT gce = { m_szModuleName, pChannel.wszUsername, GC_EVENT_JOIN };
	gce.time = time(0);
	gce.dwFlags = GCEF_SILENT;

	wchar_t wszUserId[100];
	_i64tow_s(pUser.userId, wszUserId, _countof(wszUserId), 10);

	gce.ptszStatus = pUser.wszRole;
	gce.bIsMe = (pUser.userId == m_ownId);
	gce.ptszUID = wszUserId;
	gce.ptszNick = pUser.wszNick;
	Chat_Event(&gce);

	int flags = GC_SSE_ONLYLISTED;
	switch (pUser.iStatus) {
	case ID_STATUS_ONLINE: case ID_STATUS_NA: case ID_STATUS_DND:
		flags += GC_SSE_ONLINE;
		break;
	default:
		flags += GC_SSE_OFFLINE;
	}
	Chat_SetStatusEx(m_szModuleName, pChannel.wszUsername, flags, wszUserId);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::ParseGuildContents(CDiscordGuild *pGuild, const JSONNode &pRoot)
{
	LIST<CDiscordGuildMember> newMembers(10);

	// store all guild members
	const JSONNode &pMembers = pRoot["members"];
	for (auto it = pMembers.begin(); it != pMembers.end(); ++it) {
		const JSONNode &m = *it;

		CMStringW wszUserId = m["user"]["id"].as_mstring();
		SnowFlake userId = _wtoi64(wszUserId);
		CDiscordGuildMember *pm = pGuild->FindUser(userId);
		if (pm == nullptr) {
			pm = new CDiscordGuildMember(userId);
			pGuild->arChatUsers.insert(pm);
			newMembers.insert(pm);
		}

		pm->wszNick = m["nick"].as_mstring();
		if (pm->wszNick.IsEmpty())
			pm->wszNick = m["user"]["username"].as_mstring() + L"#" + m["user"]["discriminator"].as_mstring();

		if (userId == pGuild->ownerId)
			pm->wszRole = L"@owner";
		else {
			CDiscordRole *pRole = nullptr;
			const JSONNode &pRoles = m["roles"];
			for (auto itr = pRoles.begin(); itr != pRoles.end(); ++itr) {
				SnowFlake roleId = ::getId(*itr);
				if (pRole = pGuild->arRoles.find((CDiscordRole*)&roleId))
					break;
			}
			pm->wszRole = (pRole == nullptr) ? L"@everyone" : pRole->wszName;
		}
		pm->iStatus = ID_STATUS_OFFLINE;
	}

	// parse online statuses
	const JSONNode &pStatuses = pRoot["presences"];
	for (auto it = pStatuses.begin(); it != pStatuses.end(); ++it) {
		const JSONNode &s = *it;
		CDiscordGuildMember *gm = pGuild->FindUser(::getId(s["user"]["id"]));
		if (gm != nullptr)
			gm->iStatus = StrToStatus(s["status"].as_mstring());
	}

	for (int k = 0; k < newMembers.getCount(); k++) {
		CDiscordGuildMember *pm = newMembers[k];
		for (int i = 0; i < arUsers.getCount(); i++) {
			CDiscordUser &pUser = arUsers[i];
			if (pUser.guildId == pGuild->id)
				AddUserToChannel(pUser, *pm);
		}
	}
}

/*
Copyright © 2016-19 Miranda NG team

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

class CChatRoomDlg : public CSrmmBaseDialog
{
	CChatRoomDlg(); // just to suppress compiler's warnings, never implemented
};

int compareUsers(const CDiscordUser *p1, const CDiscordUser *p2);

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
	arChannels(10, compareUsers),
	arChatUsers(30, compareChatUsers),
	arRoles(10, compareRoles)
{
}

CDiscordGuild::~CDiscordGuild()
{
}

CDiscordUser::~CDiscordUser()
{
	if (pGuild != nullptr)
		pGuild->arChannels.remove(this);
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

static void sttSetGroupName(MCONTACT hContact, const wchar_t *pwszGroupName)
{
	ptrW wszOldName(db_get_wsa(hContact, "CList", "Group"));
	if (wszOldName != nullptr) {
		ptrW wszChatGroup(Chat_GetGroup());
		if (mir_wstrcmpi(wszOldName, wszChatGroup))
			return; // custom group, don't touch it
	}

	db_set_ws(hContact, "CList", "Group", pwszGroupName);
}

void CDiscordProto::BatchChatCreate(void *param)
{
	CDiscordGuild *pGuild = (CDiscordGuild*)param;

	for (auto &it : pGuild->arChannels)
		if (!it->bIsPrivate && !it->bIsGroup)
			CreateChat(pGuild, it);
}

void CDiscordProto::CreateChat(CDiscordGuild *pGuild, CDiscordUser *pUser)
{
	SESSION_INFO *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, pUser->wszUsername, pUser->wszChannelName);
	si->pParent = pGuild->pParentSi;
	pUser->hContact = si->hContact;

	if (m_bUseGuildGroups) {
		if (pUser->parentId) {
			CDiscordUser *pParent = FindUserByChannel(pUser->parentId);
			if (pParent != nullptr)
				sttSetGroupName(pUser->hContact, pParent->wszChannelName);
		}
		else sttSetGroupName(pUser->hContact, Clist_GroupGetName(pGuild->groupId));
	}

	BuildStatusList(pGuild, si);

	Chat_Control(m_szModuleName, pUser->wszUsername, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(m_szModuleName, pUser->wszUsername, SESSION_ONLINE);

	if (!pUser->wszTopic.IsEmpty()) {
		Chat_SetStatusbarText(m_szModuleName, pUser->wszUsername, pUser->wszTopic);

		GCEVENT gce = { m_szModuleName, 0, GC_EVENT_TOPIC };
		gce.pszID.w = pUser->wszUsername;
		gce.time = time(0);
		gce.pszText.w = pUser->wszTopic;
		Chat_Event(&gce);
	}
}

void CDiscordProto::ProcessGuild(const JSONNode &p)
{
	SnowFlake guildId = ::getId(p["id"]);

	CDiscordGuild *pGuild = FindGuild(guildId);
	if (pGuild == nullptr) {
		pGuild = new CDiscordGuild(guildId);
		arGuilds.insert(pGuild);
	}

	pGuild->ownerId = ::getId(p["owner_id"]);
	pGuild->wszName = p["name"].as_mstring();
	if (m_bUseGuildGroups)
		pGuild->groupId = Clist_GroupCreate(Clist_GroupExists(m_wszDefaultGroup), pGuild->wszName);

	SESSION_INFO *si = Chat_NewSession(GCW_SERVER, m_szModuleName, pGuild->wszName, pGuild->wszName, pGuild);
	pGuild->pParentSi = (SESSION_INFO*)si;
	pGuild->hContact = si->hContact;

	if (!pGuild->bSynced && getByte(si->hContact, "EnableSync"))
		GatewaySendGuildInfo(guildId);

	Chat_Control(m_szModuleName, pGuild->wszName, WINDOW_HIDDEN);
	Chat_Control(m_szModuleName, pGuild->wszName, SESSION_ONLINE);

	const JSONNode &roles = p["roles"];
	for (auto itr = roles.begin(); itr != roles.end(); ++itr)
		ProcessRole(pGuild, *itr);
	BuildStatusList(pGuild, si);

	for (auto &it : pGuild->arChatUsers)
		AddGuildUser(pGuild, *it);

	const JSONNode &channels = p["channels"];
	for (auto itc = channels.begin(); itc != channels.end(); ++itc)
		ProcessGuildChannel(pGuild, *itc);

	if (m_bUseGroupchats)
		ForkThread(&CDiscordProto::BatchChatCreate, pGuild);
}

/////////////////////////////////////////////////////////////////////////////////////////

CDiscordUser* CDiscordProto::ProcessGuildChannel(CDiscordGuild *pGuild, const JSONNode &pch)
{
	CMStringW wszChannelId = pch["id"].as_mstring();
	SnowFlake channelId = _wtoi64(wszChannelId);
	CMStringW wszName = pch["name"].as_mstring();
	CDiscordUser *pUser;

	// filter our all channels but the text ones
	switch (pch["type"].as_int()) {
	case 4: // channel group
		if (!m_bUseGuildGroups) // ignore groups when they aren't enabled
			return nullptr;

		pUser = FindUserByChannel(channelId);
		if (pUser == nullptr) {
			// missing channel - create it
			pUser = new CDiscordUser(channelId);
			pUser->bIsPrivate = false;
			pUser->channelId = channelId;
			pUser->bIsGroup = true;
			arUsers.insert(pUser);

			pGuild->arChannels.insert(pUser);

			MGROUP grpId = Clist_GroupCreate(pGuild->groupId, wszName);
			pUser->wszChannelName = Clist_GroupGetName(grpId);
		}
		return pUser;

	case 0: // text channel
		pUser = FindUserByChannel(channelId);
		if (pUser == nullptr) {
			// missing channel - create it
			pUser = new CDiscordUser(channelId);
			pUser->bIsPrivate = false;
			pUser->channelId = channelId;
			arUsers.insert(pUser);
		}

		if (pGuild->arChannels.find(pUser) == nullptr)
			pGuild->arChannels.insert(pUser);

		pUser->wszUsername = wszChannelId;
		if (m_bUseGuildGroups)
			pUser->wszChannelName = L"#" + wszName;
		else
			pUser->wszChannelName = pGuild->wszName + L"#" + wszName;
		pUser->wszTopic = pch["topic"].as_mstring();
		pUser->pGuild = pGuild;
		pUser->lastMsgId = ::getId(pch["last_message_id"]);
		pUser->parentId = _wtoi64(pch["parent_id"].as_mstring());

		SnowFlake oldMsgId = getId(pUser->hContact, DB_KEY_LASTMSGID);
		if (oldMsgId == 0)
			RetrieveHistory(pUser, MSG_BEFORE, pUser->lastMsgId, 20);
		else if (!pUser->bSynced && pUser->lastMsgId > oldMsgId) {
			pUser->bSynced = true;
			RetrieveHistory(pUser, MSG_AFTER, oldMsgId, 99);
		}

		setId(pUser->hContact, DB_KEY_ID, channelId);
		setId(pUser->hContact, DB_KEY_CHANNELID, channelId);
		return pUser;
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::AddGuildUser(CDiscordGuild *pGuild, const CDiscordGuildMember &pUser)
{
	int flags = 0;
	switch (pUser.iStatus) {
	case ID_STATUS_ONLINE: case ID_STATUS_NA: case ID_STATUS_DND:
		flags = 1;
		break;
	}

	auto *pStatus = g_chatApi.TM_FindStatus(pGuild->pParentSi->pStatuses, pUser.wszRole);

	wchar_t wszUserId[100];
	_i64tow_s(pUser.userId, wszUserId, _countof(wszUserId), 10);
	
	auto *p = g_chatApi.UM_AddUser(pGuild->pParentSi, wszUserId, pUser.wszNick, (pStatus) ? pStatus->iStatus : 0);
	p->iStatusEx = flags;
	if (pUser.userId == m_ownId)
		pGuild->pParentSi->pMe = p;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::ParseGuildContents(CDiscordGuild *pGuild, const JSONNode &pRoot)
{
	LIST<CDiscordGuildMember> newMembers(100);

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

	for (auto &pm : newMembers)
		AddGuildUser(pGuild, *pm);

	// retrieve missing histories
	for (auto &it : pGuild->arChannels) {
		if (it->bIsPrivate)
			continue;

		if (newMembers.getCount()) {
			auto *si = g_chatApi.SM_FindSession(it->wszUsername, m_szModuleName);
			if (si && si->pDlg)
				si->pDlg->UpdateNickList();
		}			

		if (!it->bSynced) {
			it->bSynced = true;
			SnowFlake oldMsgId = getId(it->hContact, DB_KEY_LASTMSGID);
			if (oldMsgId != 0 && it->lastMsgId > oldMsgId)
				RetrieveHistory(it, MSG_AFTER, oldMsgId, 99);
		}
	}

	pGuild->bSynced = true;
}

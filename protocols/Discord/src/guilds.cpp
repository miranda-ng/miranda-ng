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

/////////////////////////////////////////////////////////////////////////////////////////
// reads a presence block from json

void CDiscordProto::ProcessPresence(const JSONNode &root)
{
	auto userId = ::getId(root["user"]["id"]);
	CDiscordUser *pUser = FindUser(userId);
	if (pUser == nullptr) {
		debugLogA("Presence from unknown user id %lld ignored", userId);
		return;
	}

	setWord(pUser->hContact, "Status", StrToStatus(root["status"].as_mstring()));

	CheckAvatarChange(pUser->hContact, root["user"]["avatar"].as_mstring());

	for (auto &act : root["activities"]) {
		CMStringW wszStatus(act["state"].as_mstring());
		if (!wszStatus.IsEmpty())
			db_set_ws(pUser->hContact, "CList", "StatusMsg", wszStatus);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static void sttSetGroupName(MCONTACT hContact, const wchar_t *pwszGroupName)
{
	ptrW wszOldName(Clist_GetGroup(hContact));
	if (wszOldName != nullptr) {
		ptrW wszChatGroup(Chat_GetGroup());
		if (mir_wstrcmpi(wszOldName, wszChatGroup))
			return; // custom group, don't touch it
	}

	Clist_SetGroup(hContact, pwszGroupName);
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
	auto *si = pUser->si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, pUser->wszUsername, pUser->wszChannelName);
	si->pParent = pGuild->pParentSi;
	pUser->hContact = si->hContact;
	setId(pUser->hContact, DB_KEY_ID, pUser->channelId);
	setId(pUser->hContact, DB_KEY_CHANNELID, pUser->channelId);

	SnowFlake oldMsgId = getId(pUser->hContact, DB_KEY_LASTMSGID);
	if (oldMsgId == 0)
		RetrieveHistory(pUser, MSG_BEFORE, pUser->lastMsgId + 1, 20);
	else if (!pUser->bSynced && pUser->lastMsgId > oldMsgId) {
		pUser->bSynced = true;
		RetrieveHistory(pUser, MSG_AFTER, oldMsgId, 99);
	}

	if (m_bUseGuildGroups) {
		if (pUser->parentId) {
			CDiscordUser *pParent = FindUserByChannel(pUser->parentId);
			if (pParent != nullptr)
				sttSetGroupName(pUser->hContact, pParent->wszChannelName);
		}
		else sttSetGroupName(pUser->hContact, Clist_GroupGetName(pGuild->m_groupId));
	}

	BuildStatusList(pGuild, si);

	Chat_Control(si, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(si, SESSION_ONLINE);

	if (!pUser->wszTopic.IsEmpty()) {
		Chat_SetStatusbarText(si, pUser->wszTopic);

		GCEVENT gce = { si, GC_EVENT_TOPIC };
		gce.time = time(0);
		gce.pszText.w = pUser->wszTopic;
		Chat_Event(&gce);
	}
}

void CDiscordProto::ProcessGuild(const JSONNode &pRoot)
{
	SnowFlake guildId = ::getId(pRoot["id"]);

	CDiscordGuild *pGuild = FindGuild(guildId);
	if (pGuild == nullptr) {
		pGuild = new CDiscordGuild(guildId);
		pGuild->LoadFromFile();
		arGuilds.insert(pGuild);
	}

	pGuild->m_ownerId = ::getId(pRoot["owner_id"]);
	pGuild->m_wszName = pRoot["name"].as_mstring();
	if (m_bUseGuildGroups)
		pGuild->m_groupId = Clist_GroupCreate(Clist_GroupExists(m_wszDefaultGroup), pGuild->m_wszName);

	SESSION_INFO *si = Chat_NewSession(GCW_SERVER, m_szModuleName, pGuild->m_wszName, pGuild->m_wszName, pGuild);
	if (si == nullptr)
		return;

	pGuild->pParentSi = (SESSION_INFO*)si;
	pGuild->m_hContact = si->hContact;
	setId(pGuild->m_hContact, DB_KEY_CHANNELID, guildId);

	Chat_Control(si, WINDOW_HIDDEN);
	Chat_Control(si, SESSION_ONLINE);

	for (auto &it : pRoot["roles"])
		pGuild->ProcessRole(it);

	BuildStatusList(pGuild, si);

	if (!pGuild->m_bSynced && getByte(si->hContact, "EnableSync"))
		GatewaySendGuildInfo(pGuild);

	// store all guild members
	for (auto &it : pRoot["members"]) {
		auto *pm = ProcessGuildUser(pGuild, it);

		CMStringW wszNick = it["nick"].as_mstring();
		if (!wszNick.IsEmpty())
			pm->wszNick = wszNick;

		pm->iStatus = ID_STATUS_OFFLINE;
	}

	// parse channels
	for (auto &it : pRoot["channels"])
		ProcessGuildChannel(pGuild, it);

	// parse online statuses
	for (auto &it : pRoot["presences"]) {
		CDiscordGuildMember *gm = pGuild->FindUser(::getId(it["user"]["id"]));
		if (gm != nullptr)
			gm->iStatus = StrToStatus(it["status"].as_mstring());
	}

	for (auto &it : pGuild->arChatUsers)
		AddGuildUser(pGuild, *it);

	if (!m_bTerminated)
		ForkThread(&CDiscordProto::BatchChatCreate, pGuild);

	pGuild->m_bSynced = true;
}

/////////////////////////////////////////////////////////////////////////////////////////

CDiscordUser* CDiscordProto::ProcessGuildChannel(CDiscordGuild *pGuild, const JSONNode &pch)
{
	CMStringW wszChannelId = pch["id"].as_mstring();
	SnowFlake channelId = _wtoi64(wszChannelId);
	CMStringW wszName = pch["name"].as_mstring();
	CDiscordUser *pUser;

	// check permissions to enter the channel
	auto permissions = pGuild->CalcPermissionOverride(m_ownId, pch["permission_overwrites"]);
	if (!(permissions & Permission::VIEW_CHANNEL))
		return nullptr;

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

			MGROUP grpId = Clist_GroupCreate(pGuild->m_groupId, wszName);
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
			pUser->wszChannelName = pGuild->m_wszName + L"#" + wszName;
		pUser->wszTopic = pch["topic"].as_mstring();
		pUser->pGuild = pGuild;
		pUser->lastMsgId = ::getId(pch["last_message_id"]);
		pUser->parentId = _wtoi64(pch["parent_id"].as_mstring());
		return pUser;
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

CDiscordGuildMember* CDiscordProto::ProcessGuildUser(CDiscordGuild *pGuild, const JSONNode &pRoot, bool *pbNew)
{
	auto& pUser = pRoot["user"];

	CMStringW wszUserId = pUser["id"].as_mstring();
	SnowFlake userId = _wtoi64(wszUserId);
	bool bNew = false, bIsMe = userId == m_ownId;

	CDiscordGuildMember *pm = pGuild->FindUser(userId);
	if (pm == nullptr) {
		pm = new CDiscordGuildMember(userId);
		pGuild->arChatUsers.insert(pm);
		bNew = true;
	}

	pm->wszDiscordId = pUser["username"].as_mstring() + L"#" + pUser["discriminator"].as_mstring();
	pm->wszNick = pRoot["nick"].as_mstring();
	if (pm->wszNick.IsEmpty())
		pm->wszNick = pUser["username"].as_mstring();
	else
		bNew = true;

	if (userId == pGuild->m_ownerId) {
		pm->wszRole = L"@owner";
		pm->permissions = Permission::ALL;
	}
	else {
		pm->permissions = pGuild->m_permissions;

		CDiscordRole *pRole = nullptr;
		for (auto &itr : pRoot["roles"]) {
			SnowFlake roleId = ::getId(itr);

			if (auto *p = pGuild->arRoles.find((CDiscordRole *)&roleId)) {
				pm->permissions |= p->permissions;
				if (pRole == nullptr)
					pRole = p;

				if (bIsMe)
					p->bIsMe = true;
			}
		}
		pm->wszRole = (pRole == nullptr) ? L"@everyone" : pRole->wszName;
	}

	if (pbNew)
		*pbNew = bNew;
	return pm;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::ProcessChatUser(CDiscordUser *pChat, SnowFlake userId, const JSONNode &pRoot)
{
	// input data control
	CDiscordGuild *pGuild = pChat->pGuild;
	if (pGuild == nullptr || userId == 0)
		return;

	// does user exist? if yes, there's nothing to do
	auto *pm = pGuild->FindUser(userId);
	if (pm != nullptr)
		return;

	// otherwise let's create a user and insert him into all guild's chats
	pm = new CDiscordGuildMember(userId);
	pm->wszDiscordId = pRoot["author"]["username"].as_mstring() + L"#" + pRoot["author"]["discriminator"].as_mstring();
	pm->wszNick = pRoot["nick"].as_mstring();
	if (pm->wszNick.IsEmpty())
		pm->wszNick = pRoot["author"]["username"].as_mstring();
	pGuild->arChatUsers.insert(pm);

	debugLogA("add missing user to chat: id=%lld, nick=%S", userId, pm->wszNick.c_str());
	AddGuildUser(pGuild, *pm);
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
	
	auto *pu = g_chatApi.UM_AddUser(pGuild->pParentSi, wszUserId, pUser.wszNick, (pStatus) ? pStatus->iStatus : 0);
	pu->iStatusEx = flags;
	if (pUser.userId == m_ownId)
		pGuild->pParentSi->pMe = pu;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CDiscordGuild members

int compareUsers(const CDiscordUser *p1, const CDiscordUser *p2);

static int compareRoles(const CDiscordRole *p1, const CDiscordRole *p2)
{
	return compareInt64(p1->id, p2->id);
}

static int compareChatUsers(const CDiscordGuildMember *p1, const CDiscordGuildMember *p2)
{
	return compareInt64(p1->userId, p2->userId);
}

CDiscordGuild::CDiscordGuild(SnowFlake _id) :
	m_id(_id),
	arChannels(10, compareUsers),
	arChatUsers(30, compareChatUsers),
	arRoles(10, compareRoles)
{}

CDiscordGuild::~CDiscordGuild()
{}

CDiscordUser::~CDiscordUser()
{
	if (pGuild != nullptr)
		pGuild->arChannels.remove(this);
}

/////////////////////////////////////////////////////////////////////////////////////////
// calculates effective rights

uint64_t CDiscordGuild::CalcPermissionOverride(SnowFlake myUserId, const JSONNode &json)
{
	if (myUserId == m_ownerId)
		return Permission::ALL;

	uint64_t permissions = m_permissions;

	if (auto *pUser = FindUser(myUserId)) {
		if (pUser->permissions & Permission::ADMIN)
			return Permission::ALL;
		
		permissions = pUser->permissions;
	}

	struct Item
	{
		Item() :
			allow(0),
			deny(0)
		{}

		Item(SnowFlake _1, SnowFlake _2) :
			allow(_1),
			deny(_2)
		{}

		SnowFlake allow, deny;
	};
	std::map<SnowFlake, Item> items;

	// verify permissions
	for (auto &it : json) {
		if (it["type"].as_int() != 0)
			continue;

		SnowFlake id = ::getId(it["id"]);
		items[id] = Item(::getId(it["allow"]), ::getId(it["deny"]));
	}

	auto everyone = items[m_id];
	permissions &= ~everyone.deny;
	permissions |= everyone.allow;

	uint64_t allow = 0, deny = 0;
	for (auto &it : arRoles) {
		if (it->bIsMe) {
			auto role = items[it->id];
			deny |= role.deny;
			allow |= role.allow;
		}
	}

	permissions &= ~deny;
	permissions |= allow;

	auto personal = items[myUserId];
	permissions &= ~personal.deny;
	permissions |= personal.allow;

	return permissions;
}

/////////////////////////////////////////////////////////////////////////////////////////
// reads a role from json

void CDiscordGuild::ProcessRole(const JSONNode &role)
{
	SnowFlake id = ::getId(role["id"]);
	CDiscordRole *p = arRoles.find((CDiscordRole *)&id);
	if (p == nullptr) {
		p = new CDiscordRole();
		p->id = id;
		arRoles.insert(p);
	}

	p->color = role["color"].as_int();
	p->position = role["position"].as_int();
	p->permissions = ::getId(role["permissions"]);
	p->wszName = role["name"].as_mstring();

	if (m_id == id)
		m_permissions = p->permissions;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Persistence manager

void CDiscordGuild::LoadFromFile()
{
	JSONNode cached;
	if (!file2json(GetCacheFile(), cached))
		return;

	for (auto &it : cached) {
		SnowFlake userId = getId(it["id"]);
		auto *pUser = FindUser(userId);
		if (pUser == nullptr) {
			pUser = new CDiscordGuildMember(userId);
			arChatUsers.insert(pUser);
		}

		pUser->wszNick = it["n"].as_mstring();
		pUser->wszRole = it["r"].as_mstring();
	}
}

void CDiscordGuild ::SaveToFile()
{
	JSONNode members(JSON_ARRAY);
	for (auto &it : arChatUsers) {
		JSONNode member; 
		member << INT64_PARAM("id", it->userId) << WCHAR_PARAM("n", it->wszNick) << WCHAR_PARAM("r", it->wszRole);
		members << member;
	}

	CMStringW wszFileName(GetCacheFile());
	CreatePathToFileW(wszFileName);
	json2file(members, wszFileName);
}

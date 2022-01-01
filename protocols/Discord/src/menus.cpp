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

INT_PTR CDiscordProto::OnMenuCopyId(WPARAM hContact, LPARAM)
{
	CopyId(CMStringW(FORMAT, L"%s#%d", getMStringW(hContact, DB_KEY_NICK).c_str(), getDword(hContact, DB_KEY_DISCR)));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::OnMenuCreateChannel(WPARAM hContact, LPARAM)
{
	ENTER_STRING es = { m_szModuleName, "channel_name", TranslateT("Enter channel name"), nullptr, ESF_COMBO, 5 };
	if (EnterString(&es)) {
		JSONNode roles(JSON_ARRAY); roles.set_name("permission_overwrites");
		JSONNode root; root << INT_PARAM("type", 0) << WCHAR_PARAM("name", es.ptszResult) << roles;
		CMStringA szUrl(FORMAT, "/guilds/%lld/channels", getId(hContact, DB_KEY_CHANNELID));
		Push(new AsyncHttpRequest(this, REQUEST_POST, szUrl, nullptr, &root));
		mir_free(es.ptszResult);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::OnMenuJoinGuild(WPARAM, LPARAM)
{
	ENTER_STRING es = { m_szModuleName, "guild_name", TranslateT("Enter invitation code you received"), nullptr, ESF_COMBO, 5 };
	if (EnterString(&es)) {
		CMStringA szUrl(FORMAT, "/invite/%S", es.ptszResult);
		Push(new AsyncHttpRequest(this, REQUEST_POST, szUrl, nullptr));
		mir_free(es.ptszResult);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::OnMenuLeaveGuild(WPARAM hContact, LPARAM)
{
	if (IDYES == MessageBox(nullptr, TranslateT("Do you really want to leave the guild?"), m_tszUserName, MB_ICONQUESTION | MB_YESNOCANCEL)) {
		CMStringA szUrl(FORMAT, "/users/@me/guilds/%lld", getId(hContact, DB_KEY_CHANNELID));
		Push(new AsyncHttpRequest(this, REQUEST_DELETE, szUrl, nullptr));
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::OnMenuLoadHistory(WPARAM hContact, LPARAM)
{
	auto *pUser = FindUser(getId(hContact, DB_KEY_ID));
	if (pUser) {
		RetrieveHistory(pUser, MSG_AFTER, 0, 100);
		delSetting(hContact, DB_KEY_LASTMSGID);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::OnMenuToggleSync(WPARAM hContact, LPARAM)
{
	bool bEnabled = !getBool(hContact, "EnableSync");
	setByte(hContact, "EnableSync", bEnabled);

	if (bEnabled)
		if (auto *pGuild = FindGuild(getId(hContact, DB_KEY_CHANNELID)))
			GatewaySendGuildInfo(pGuild);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDiscordProto::OnMenuPrebuild(WPARAM hContact, LPARAM)
{
	// "Leave guild" menu item should be visible only for the guild contacts
	bool bIsGuild = getByte(hContact, "ChatRoom") == 2;
	Menu_ShowItem(m_hMenuLeaveGuild, bIsGuild);
	Menu_ShowItem(m_hMenuCreateChannel, bIsGuild);
	Menu_ShowItem(m_hMenuToggleSync, bIsGuild);

	if (!bIsGuild && getWord(hContact, "ApparentMode") != 0)
		Menu_ShowItem(GetMenuItem(PROTO_MENU_REQ_AUTH), true);

	if (getByte(hContact, "EnableSync"))
		Menu_ModifyItem(m_hMenuToggleSync, LPGENW("Disable sync"), Skin_GetIconHandle(SKINICON_CHAT_LEAVE));
	else
		Menu_ModifyItem(m_hMenuToggleSync, LPGENW("Enable sync"), Skin_GetIconHandle(SKINICON_CHAT_JOIN));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Protocol menu items

void CDiscordProto::OnBuildProtoMenu()
{
	CMenuItem mi(&g_plugin);
	mi.root = Menu_GetProtocolRoot(this);
	mi.flags = CMIF_UNMOVABLE;

	mi.pszService = "/JoinGuild";
	CreateProtoService(mi.pszService, &CDiscordProto::OnMenuJoinGuild);
	mi.name.a = LPGEN("Join guild");
	mi.position = 200001;
	mi.hIcolibItem = g_iconList[1].hIcolib;
	Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/CopyId";
	CreateProtoService(mi.pszService, &CDiscordProto::OnMenuCopyId);
	mi.name.a = LPGEN("Copy my Discord ID");
	mi.position = 200002;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_USERONLINE);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Contact menu items

void CDiscordProto::InitMenus()
{
	CMenuItem mi(&g_plugin);
	mi.pszService = "/LeaveGuild";
	CreateProtoService(mi.pszService, &CDiscordProto::OnMenuLeaveGuild);
	SET_UID(mi, 0x6EF11AD6, 0x6111, 0x4E29, 0xBA, 0x8B, 0xA7, 0xB2, 0xE0, 0x22, 0xE1, 0x8C);
	mi.name.a = LPGEN("Leave guild");
	mi.position = -200001000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_CHAT_LEAVE);
	m_hMenuLeaveGuild = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = "/CreateChannel";
	CreateProtoService(mi.pszService, &CDiscordProto::OnMenuCreateChannel);
	SET_UID(mi, 0x6EF11AD6, 0x6111, 0x4E29, 0xBA, 0x8B, 0xA7, 0xB2, 0xE0, 0x22, 0xE1, 0x8D);
	mi.name.a = LPGEN("Create new channel");
	mi.position = -200001001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_ADDCONTACT);
	m_hMenuCreateChannel = Menu_AddContactMenuItem(&mi, m_szModuleName);

	SET_UID(mi, 0x6EF11AD6, 0x6111, 0x4E29, 0xBA, 0x8B, 0xA7, 0xB2, 0xE0, 0x22, 0xE1, 0x8E);
	mi.pszService = "/CopyId";
	mi.name.a = LPGEN("Copy ID");
	mi.position = -200001002;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_USERONLINE);
	Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = "/ToggleSync";
	CreateProtoService(mi.pszService, &CDiscordProto::OnMenuToggleSync);
	SET_UID(mi, 0x6EF11AD6, 0x6111, 0x4E29, 0xBA, 0x8B, 0xA7, 0xB2, 0xE0, 0x22, 0xE1, 0x8F);
	mi.name.a = LPGEN("Enable guild sync");
	mi.position = -200001003;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_CHAT_JOIN);
	m_hMenuToggleSync = Menu_AddContactMenuItem(&mi, m_szModuleName);

	HookProtoEvent(ME_CLIST_PREBUILDCONTACTMENU, &CDiscordProto::OnMenuPrebuild);
}

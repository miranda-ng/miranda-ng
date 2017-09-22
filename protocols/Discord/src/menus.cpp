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

INT_PTR CDiscordProto::OnMenuCopyId(WPARAM hContact, LPARAM)
{
	CMStringW mynick(ptrW(getWStringA(hContact, DB_KEY_NICK)));
	mynick.AppendFormat(L"#%d", getDword(hContact, DB_KEY_DISCR));

	if (OpenClipboard(nullptr)) {
		EmptyClipboard();
		int length = mynick.GetLength() + 1;
		HGLOBAL hMemory = GlobalAlloc(GMEM_FIXED, length * sizeof(wchar_t));
		mir_wstrncpy((wchar_t*)GlobalLock(hMemory), mynick, length);
		GlobalUnlock(hMemory);
		SetClipboardData(CF_UNICODETEXT, hMemory);
		CloseClipboard();
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::OnMenuCreateChannel(WPARAM hContact, LPARAM)
{
	ENTER_STRING es = { sizeof(es), ESF_COMBO, m_szModuleName, "channel_name", TranslateT("Enter channel name"), 0, 5 };
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
	ENTER_STRING es = { sizeof(es), ESF_COMBO, m_szModuleName, "guild_name", TranslateT("Enter invitation code you received"), 0, 5 };
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

int CDiscordProto::OnMenuPrebuild(WPARAM hContact, LPARAM)
{
	// "Leave guild" menu item should be visible only for the guild contacts
	bool bIsGuild = getByte(hContact, "ChatRoom") == 2;
	Menu_ShowItem(m_hMenuLeaveGuild, bIsGuild);
	Menu_ShowItem(m_hMenuCreateChannel, bIsGuild);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::InitMenus()
{
	CMenuItem mi;
	mi.root = Menu_GetProtocolRoot(this);
	mi.flags = CMIF_UNMOVABLE;

	// Protocol menu items
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

	// Contact menu items
	CMenuItem mi2;
	mi2.pszService = "/LeaveGuild";
	CreateProtoService(mi2.pszService, &CDiscordProto::OnMenuLeaveGuild);
	mi2.name.a = LPGEN("Leave guild");
	mi2.position = -200001000;
	mi2.hIcolibItem = Skin_GetIconHandle(SKINICON_CHAT_LEAVE);
	SET_UID(mi2, 0x6EF11AD6, 0x6111, 0x4E29, 0xBA, 0x8B, 0xA7, 0xB2, 0xE0, 0x22, 0xE1, 0x8C);
	m_hMenuLeaveGuild = Menu_AddContactMenuItem(&mi2, m_szModuleName);

	mi2.pszService = "/CreateChannel";
	CreateProtoService(mi2.pszService, &CDiscordProto::OnMenuCreateChannel);
	mi2.name.a = LPGEN("Create new channel");
	mi2.position = -200001001;
	mi2.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_ADDCONTACT);
	SET_UID(mi2, 0x6EF11AD6, 0x6111, 0x4E29, 0xBA, 0x8B, 0xA7, 0xB2, 0xE0, 0x22, 0xE1, 0x8D);
	m_hMenuCreateChannel = Menu_AddContactMenuItem(&mi2, m_szModuleName);

	mi2.pszService = "/CopyId";
	mi2.name.a = LPGEN("Copy Discord ID");
	mi2.position = -200001002;
	mi2.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_USERONLINE);
	SET_UID(mi2, 0x6EF11AD6, 0x6111, 0x4E29, 0xBA, 0x8B, 0xA7, 0xB2, 0xE0, 0x22, 0xE1, 0x8E);
	Menu_AddContactMenuItem(&mi2, m_szModuleName);

	HookProtoEvent(ME_CLIST_PREBUILDCONTACTMENU, &CDiscordProto::OnMenuPrebuild);
}

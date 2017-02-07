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

INT_PTR CDiscordProto::OnMenuJoinGuild(WPARAM, LPARAM)
{
	ENTER_STRING es = { sizeof(es) };
	es.szModuleName = m_szModuleName;
	es.szDataPrefix = "Discord";
	es.type = ESF_MULTILINE;
	es.caption = TranslateT("Enter invitation code you received");
	if (EnterString(&es)) {
		CMStringA szUrl(FORMAT, "/invite/%S", es.ptszResult);
		Push(new AsyncHttpRequest(this, REQUEST_POST, szUrl, NULL));
		mir_free(es.ptszResult);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::OnMenuLeaveGuild(WPARAM hContact, LPARAM)
{
	if (IDYES == MessageBox(NULL, TranslateT("Do you really want to leave the guild?"), m_tszUserName, MB_ICONQUESTION | MB_YESNOCANCEL)) {
		CMStringA szUrl(FORMAT, "/users/@me/guilds/%lld", getId(hContact, DB_KEY_CHANNELID));
		Push(new AsyncHttpRequest(this, REQUEST_DELETE, szUrl, NULL));
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDiscordProto::OnMenuPrebuild(WPARAM hContact, LPARAM)
{
	// "Leave guild" menu item should be visible only for the guild contacts
	Menu_ShowItem(m_hMenuLeaveGuild, getByte(hContact, "ChatRoom") == 2);
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

	CMenuItem mi2;
	mi2.pszService = "/LeaveGuild";
	CreateProtoService(mi2.pszService, &CDiscordProto::OnMenuLeaveGuild);
	mi2.name.a = LPGEN("Leave guild");
	mi2.position = -200001000;
	mi2.hIcolibItem = Skin_GetIconHandle(SKINICON_CHAT_LEAVE);
	SET_UID(mi2, 0x6EF11AD6, 0x6111, 0x4E29, 0xBA, 0x8B, 0xA7, 0xB2, 0xE0, 0x22, 0xE1, 0x8C);
	m_hMenuLeaveGuild = Menu_AddContactMenuItem(&mi2, m_szModuleName);

	HookProtoEvent(ME_CLIST_PREBUILDCONTACTMENU, &CDiscordProto::OnMenuPrebuild);
}

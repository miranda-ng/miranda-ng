/*
Copyright (c) 2025 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

HGENMENU CTeamsProto::ContactMenuItems[CMI_MAX];

int CTeamsProto::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (!hContact)
		return 0;

	if (m_iStatus < ID_STATUS_ONLINE)
		return 0;

	if (isChatRoom(hContact))
		return 0;

	bool isCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	bool isAuthNeed = getByte(hContact, "Auth", 0) > 0;
	bool isGrantNeed = getByte(hContact, "Grant", 0) > 0;
	bool isBlocked = getBool(hContact, "IsBlocked", false);

	Menu_ShowItem(GetMenuItem(PROTO_MENU_REQ_AUTH), isCtrlPressed || isAuthNeed);
	Menu_ShowItem(GetMenuItem(PROTO_MENU_GRANT_AUTH), isCtrlPressed || isGrantNeed);
	
	Menu_ShowItem(ContactMenuItems[CMI_BLOCK], true);
	Menu_ShowItem(ContactMenuItems[CMI_UNBLOCK], isCtrlPressed || isBlocked);
	return 0;
}

int CTeamsProto::PrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	for (auto &it : ContactMenuItems)
		Menu_ShowItem(it, false);
	CTeamsProto *proto = CMPlugin::getInstance(hContact);
	return proto ? proto->OnPrebuildContactMenu(hContact, lParam) : 0;
}

void CTeamsProto::InitMenus()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, &CTeamsProto::PrebuildContactMenu);

	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE;

	mi.pszService = MODULENAME "/BlockContact";
	mi.name.w = LPGENW("Block contact");
	mi.position = CMI_POSITION + CMI_BLOCK;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_BLOCKUSER);
	SET_UID(mi, 0xc6169b8f, 0x53ab, 0x4242, 0xbe, 0x90, 0xe2, 0x4a, 0xa5, 0x73, 0x88, 0x32);
	ContactMenuItems[CMI_BLOCK] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CTeamsProto::BlockContact>);

	mi.pszService = MODULENAME "/UnblockContact";
	mi.name.w = LPGENW("Unblock contact");
	mi.position = CMI_POSITION + CMI_UNBLOCK;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_UNBLOCKUSER);
	SET_UID(mi, 0x88542f43, 0x7448, 0x48d0, 0x81, 0xa3, 0x26, 0x0, 0x4f, 0x37, 0xee, 0xe0);
	ContactMenuItems[CMI_UNBLOCK] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CTeamsProto::UnblockContact>);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Protocol's menu in the status bar

void CTeamsProto::OnBuildProtoMenu()
{
	CMenuItem mi(&g_plugin);
	mi.root = Menu_GetProtocolRoot(this);

	mi.pszService = "/CreateNewChat";
	CreateProtoService(mi.pszService, &CTeamsProto::SvcCreateChat);
	mi.name.a = LPGEN("Create new chat");
	mi.position = 200000;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_CONFERENCE);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/SetMood";
	CreateProtoService(mi.pszService, &CTeamsProto::SvcSetMood);
	mi.name.a = LPGEN("Set own mood");
	mi.position++;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_USERONLINE);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);
}

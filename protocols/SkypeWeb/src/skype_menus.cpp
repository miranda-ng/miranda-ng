/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

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

HGENMENU CSkypeProto::ContactMenuItems[CMI_MAX];

int CSkypeProto::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (!hContact)
		return 0;

	if (m_iStatus < ID_STATUS_ONLINE)
		return 0;

	if (this->isChatRoom(hContact))
	{
		return 0;
	}

	bool isCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	bool isAuthNeed = getByte(hContact, "Auth", 0) > 0;
	bool isGrantNeed = getByte(hContact, "Grant", 0) > 0;
	bool isBlocked = getBool(hContact, "IsBlocked", false);

	Menu_ShowItem(ContactMenuItems[CMI_AUTH_REQUEST], isCtrlPressed || isAuthNeed);
	Menu_ShowItem(ContactMenuItems[CMI_AUTH_GRANT], isCtrlPressed || isGrantNeed);
	Menu_ShowItem(ContactMenuItems[CMI_BLOCK], true);
	Menu_ShowItem(ContactMenuItems[CMI_UNBLOCK], isCtrlPressed || isBlocked);
	Menu_ShowItem(ContactMenuItems[CMI_GETSERVERHISTORY], true);

	return 0;
}

int CSkypeProto::PrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	for (int i = 0; i < _countof(ContactMenuItems); i++)
		Menu_ShowItem(ContactMenuItems[i], false);
	CSkypeProto *proto = CSkypeProto::GetContactAccount(hContact);
	return proto ? proto->OnPrebuildContactMenu(hContact, lParam) : 0;
}

void CSkypeProto::InitMenus()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, &CSkypeProto::PrebuildContactMenu);

	//hChooserMenu = Menu_AddObject("SkypeAccountChooser", LPGEN("Skype menu chooser"), 0, "Skype/MenuChoose");

	TMO_MenuItem mi = { 0 };
	mi.flags = CMIF_TCHAR;

	// Request authorization
	mi.pszService = MODULE"/RequestAuth";
	mi.name.t = LPGENT("Request authorization");
	mi.position = CMI_POSITION + CMI_AUTH_REQUEST;
	mi.hIcolibItem = ::Skin_GetIconHandle(SKINICON_AUTH_REQUEST);
	ContactMenuItems[CMI_AUTH_REQUEST] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::OnRequestAuth>);

	// Grant authorization
	mi.pszService = MODULE"/GrantAuth";
	mi.name.t = LPGENT("Grant authorization");
	mi.position = CMI_POSITION + CMI_AUTH_GRANT;
	mi.hIcolibItem = ::Skin_GetIconHandle(SKINICON_AUTH_GRANT);
	ContactMenuItems[CMI_AUTH_GRANT] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::OnGrantAuth>);

	mi.pszService = MODULE"/GetHistory";
	mi.name.t = LPGENT("Get server history");
	mi.position = CMI_POSITION + CMI_GETSERVERHISTORY;
	mi.hIcolibItem = GetIconHandle("synchistory");
	ContactMenuItems[CMI_GETSERVERHISTORY] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::GetContactHistory>);

	mi.pszService = MODULE"/BlockContact";
	mi.name.t = LPGENT("Block contact");
	mi.position = CMI_POSITION + CMI_BLOCK;
	mi.hIcolibItem = GetIconHandle("user_block");
	ContactMenuItems[CMI_BLOCK] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::BlockContact>);

	mi.pszService = MODULE"/UnblockContact";
	mi.name.t = LPGENT("Unblock contact");
	mi.position = CMI_POSITION + CMI_UNBLOCK;
	mi.hIcolibItem = GetIconHandle("user_unblock");
	ContactMenuItems[CMI_UNBLOCK] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::UnblockContact>);
}

void CSkypeProto::UninitMenus()
{
}


int CSkypeProto::OnInitStatusMenu()
{
	HGENMENU hStatusMenuRoot = Menu_GetProtocolRoot(m_szModuleName);
	if (!hStatusMenuRoot)
	{
		TMO_MenuItem mi = { 0 };
		mi.name.t = m_tszUserName;
		mi.position = -1999901006;
		mi.flags = CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
		mi.hIcolibItem = Skin_GetIconHandle("main");
		hStatusMenuRoot = Menu_AddProtoMenuItem(&mi);
	}

	TMO_MenuItem mi = { 0 };
	mi.root = hStatusMenuRoot;

	mi.pszService = "/CreateNewChat";
	CreateProtoService(mi.pszService, &CSkypeProto::SvcCreateChat);
	mi.name.a = LPGEN("Create new chat");
	mi.position = SMI_POSITION + SMI_CREATECHAT;
	mi.hIcolibItem = GetIconHandle("conference");
	Menu_AddProtoMenuItem(&mi, m_szModuleName);
	return 0;
}
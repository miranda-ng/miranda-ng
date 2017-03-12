#include "stdafx.h"

HGENMENU CSlackProto::ContactMenuItems[CMI_MAX];

int CSlackProto::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (!hContact)
		return 0;

	if (!this->IsOnline())
		return 0;

	if (this->isChatRoom(hContact))
		return 0;

	bool isCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

	bool isAuthNeed = getByte(hContact, "Auth", 0) > 0;
	Menu_ShowItem(ContactMenuItems[CMI_AUTH_REQUEST], isCtrlPressed || isAuthNeed);

	bool isGrantNeed = getByte(hContact, "Grant", 0) > 0;
	Menu_ShowItem(ContactMenuItems[CMI_AUTH_GRANT], isCtrlPressed || isGrantNeed);

	return 0;
}

int CSlackProto::PrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	for (int i = 0; i < _countof(ContactMenuItems); i++)
		Menu_ShowItem(ContactMenuItems[i], FALSE);
	CSlackProto *proto = CSlackProto::GetContactAccount(hContact);
	return proto ? proto->OnPrebuildContactMenu(hContact, lParam) : 0;
}

void CSlackProto::InitMenus()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, &CSlackProto::PrebuildContactMenu);

	CMenuItem mi;
	mi.flags = CMIF_UNICODE;
	/*
	// Request authorization
	mi.pszService = MODULE"/RequestAuth";
	mi.ptszName = LPGENT("Request authorization");
	mi.position = CMI_POSITION + CMI_AUTH_REQUEST;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	ContactMenuItems[CMI_AUTH_REQUEST] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSlackProto::OnRequestAuth>);

	// Grant authorization
	mi.pszService = MODULE"/GrantAuth";
	mi.ptszName = LPGENT("Grant authorization");
	mi.position = CMI_POSITION + CMI_AUTH_GRANT;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_GRANT);
	ContactMenuItems[CMI_AUTH_GRANT] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSlackProto::OnGrantAuth>);*/
}

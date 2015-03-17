#include "common.h"

HGENMENU CSkypeProto::ContactMenuItems[CMI_MAX];

int CSkypeProto::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (!hContact)
		return 0;

	if (m_iStatus < ID_STATUS_ONLINE)
		return 0;

	if (this->isChatRoom(hContact))
		return 0;

	bool isCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	bool isAuthNeed = getByte(hContact, "Auth", 0) > 0;
	bool isGrantNeed = getByte(hContact, "Grant", 0) > 0;

	Menu_ShowItem(ContactMenuItems[CMI_AUTH_REQUEST], isCtrlPressed || isAuthNeed);
	Menu_ShowItem(ContactMenuItems[CMI_AUTH_GRANT], isCtrlPressed || isGrantNeed);

	return 0;
}

int CSkypeProto::PrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	for (int i = 0; i < SIZEOF(ContactMenuItems); i++)
	{
		Menu_ShowItem(ContactMenuItems[i], false);
	}
	CSkypeProto *proto = CSkypeProto::GetContactAccount(hContact);
	return proto ? proto->OnPrebuildContactMenu(hContact, lParam) : 0;
}

void CSkypeProto::InitMenus()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, &CSkypeProto::PrebuildContactMenu);

	//hChooserMenu = MO_CreateMenuObject("SkypeAccountChooser", LPGEN("Skype menu chooser"), 0, "Skype/MenuChoose");

	CLISTMENUITEM mi = { sizeof(CLISTMENUITEM) };
	mi.flags = CMIF_TCHAR;

	// Request authorization
	mi.pszService = MODULE"/RequestAuth";
	mi.ptszName = LPGENT("Request authorization");
	mi.position = CMI_POSITION + CMI_AUTH_REQUEST;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	ContactMenuItems[CMI_AUTH_REQUEST] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::OnRequestAuth>);

	// Grant authorization
	mi.pszService = MODULE"/GrantAuth";
	mi.ptszName = LPGENT("Grant authorization");
	mi.position = CMI_POSITION + CMI_AUTH_GRANT;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_GRANT);
	ContactMenuItems[CMI_AUTH_GRANT] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::OnGrantAuth>);
}

void CSkypeProto::UninitMenus()
{
}


int CSkypeProto::OnInitStatusMenu()
{
	char text[MAX_PATH];
	mir_strcpy(text, m_szModuleName);
	char *tDest = text + strlen(text);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszService = text;

	HGENMENU hStatusMunuRoot = MO_GetProtoRootMenu(m_szModuleName);
	if (!hStatusMunuRoot)
	{
		mi.ptszName = m_tszUserName;
		mi.position = -1999901006;
		mi.hParentMenu = HGENMENU_ROOT;
		mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
		mi.icolibItem = GetSkinIconHandle("main");
		hStatusMunuRoot = /*m_hMenuRoot = */Menu_AddProtoMenuItem(&mi);
	}
	/*else
	{
		if (m_hMenuRoot)
			CallService(MO_REMOVEMENUITEM, (WPARAM)m_hMenuRoot, 0);
		m_hMenuRoot = NULL;
	}*/

	mi.hParentMenu = hStatusMunuRoot;
	mi.flags = CMIF_CHILDPOPUP | CMIF_TCHAR;

	return 0;
}
#include "common.h"

HGENMENU CToxProto::ContactMenuItems[CMI_MAX];

int CToxProto::OnPrebuildContactMenu(MCONTACT hContact, LPARAM)
{
	if (!hContact)
		return 0;

	if (!this->IsOnline())
		return 0;

	if (this->isChatRoom(hContact))
		return 0;

	bool isCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	bool isAuthNeed = getByte(hContact, "Auth", 0) > 0;
	bool isGrantNeed = getByte(hContact, "Grant", 0) > 0;
	//bool isOffline = GetContactStatus(hContact) == ID_STATUS_OFFLINE;
	//bool isLongOffline = ((time(NULL) - getDword(hContact, "LastEventDateTS", 0)) / (1000 * 60 * 60 * 24 * 7)) > 0;
	//bool hasDnsID = mir_strlen(ptrA(getStringA(hContact, TOX_SETTINGS_DNS))) > 0;

	Menu_ShowItem(ContactMenuItems[CMI_AUTH_REQUEST], isCtrlPressed || isAuthNeed);
	Menu_ShowItem(ContactMenuItems[CMI_AUTH_GRANT], isCtrlPressed || isGrantNeed);

	return 0;
}

int CToxProto::PrebuildContactMenu(MCONTACT hContact, LPARAM lParam)
{
	for (int i = 0; i < SIZEOF(ContactMenuItems); i++)
	{
		Menu_ShowItem(ContactMenuItems[i], false);
	}
	CToxProto *proto = CToxProto::GetContactAccount(hContact);
	return proto ? proto->OnPrebuildContactMenu(hContact, lParam) : 0;
}

void CToxProto::InitMenus()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, &CToxProto::PrebuildContactMenu);

	//hChooserMenu = MO_CreateMenuObject("SkypeAccountChooser", LPGEN("Skype menu chooser"), 0, "Skype/MenuChoose");

	CLISTMENUITEM mi = { sizeof(CLISTMENUITEM) };
	mi.flags = CMIF_TCHAR;

	// Request authorization
	mi.pszService = MODULE"/RequestAuth";
	mi.ptszName = LPGENT("Request authorization");
	mi.position = CMI_POSITION + CMI_AUTH_REQUEST;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	ContactMenuItems[CMI_AUTH_REQUEST] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CToxProto::OnRequestAuth>);

	// Grant authorization
	mi.pszService = MODULE"/GrantAuth";
	mi.ptszName = LPGENT("Grant authorization");
	mi.position = CMI_POSITION + CMI_AUTH_GRANT;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_GRANT);
	ContactMenuItems[CMI_AUTH_GRANT] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CToxProto::OnGrantAuth>);
}

void CToxProto::UninitMenus()
{
}


int CToxProto::OnInitStatusMenu()
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

	// Create copy tox id command
	mir_strcpy(tDest, "/CopyToxID");
	CreateProtoService(tDest, &CToxProto::OnCopyToxID);
	mi.ptszName = LPGENT("Copy Tox ID");
	mi.position = SMI_POSITION + SMI_TOXID_COPY;
	Menu_AddProtoMenuItem(&mi);

	// Create group chat command
	/*mir_strcpy(tDest, "/CreateChatRoom");
	CreateProtoService(tDest, &CToxProto::OnCreateChatRoom);
	mi.ptszName = LPGENT("Create group chat");
	mi.position = SMI_POSITION + SMI_GROUPCHAT_CREATE;
	mi.icolibItem = GetSkinIconHandle("conference");
	Menu_AddProtoMenuItem(&mi);*/

	return 0;
}
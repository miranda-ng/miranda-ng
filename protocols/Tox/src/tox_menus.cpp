#include "stdafx.h"

HGENMENU CToxProto::ContactMenuItems[CMI_MAX];

int CToxProto::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
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

	bool isContactOnline = GetContactStatus(hContact) > ID_STATUS_OFFLINE;
	Menu_ShowItem(ContactMenuItems[CMI_AUDIO_CALL], toxAv && isContactOnline);

	return 0;
}

int CToxProto::PrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	for (int i = 0; i < _countof(ContactMenuItems); i++)
		Menu_ShowItem(ContactMenuItems[i], FALSE);
	CToxProto *proto = CToxProto::GetContactAccount(hContact);
	return proto ? proto->OnPrebuildContactMenu(hContact, lParam) : 0;
}

void CToxProto::InitMenus()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, &CToxProto::PrebuildContactMenu);

	//hChooserMenu = Menu_AddObject("SkypeAccountChooser", LPGEN("Skype menu chooser"), 0, "Skype/MenuChoose");

	CLISTMENUITEM mi = { 0 };
	mi.flags = CMIF_TCHAR;

	// Request authorization
	mi.pszService = MODULE"/RequestAuth";
	mi.ptszName = LPGENT("Request authorization");
	mi.position = CMI_POSITION + CMI_AUTH_REQUEST;
	mi.icolibItem = ::Skin_GetIconHandle(SKINICON_AUTH_REQUEST);
	ContactMenuItems[CMI_AUTH_REQUEST] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CToxProto::OnRequestAuth>);

	// Grant authorization
	mi.pszService = MODULE"/GrantAuth";
	mi.ptszName = LPGENT("Grant authorization");
	mi.position = CMI_POSITION + CMI_AUTH_GRANT;
	mi.icolibItem = ::Skin_GetIconHandle(SKINICON_AUTH_GRANT);
	ContactMenuItems[CMI_AUTH_GRANT] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CToxProto::OnGrantAuth>);

	// Start audio call
	mi.pszService = MODULE"/Audio/Call";
	mi.ptszName = LPGENT("Call");
	mi.position = -2000020000 + CMI_AUDIO_CALL;
	mi.icolibItem = GetIconHandle("audio_start");
	ContactMenuItems[CMI_AUDIO_CALL] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CToxProto::OnSendAudioCall>);
}

void CToxProto::UninitMenus()
{
}


int CToxProto::OnInitStatusMenu()
{
	char text[MAX_PATH];
	mir_strcpy(text, m_szModuleName);
	char *tDest = text + mir_strlen(text);

	CLISTMENUITEM mi = { 0 };
	mi.pszService = text;

	HGENMENU hStatusMunuRoot = Menu_GetProtocolRoot(m_szModuleName);
	if (!hStatusMunuRoot)
	{
		mi.ptszName = m_tszUserName;
		mi.position = -1999901006;
		mi.hParentMenu = HGENMENU_ROOT;
		mi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
		mi.icolibItem = Skin_GetIconHandle("main");
		hStatusMunuRoot = /*m_hMenuRoot = */Menu_AddProtoMenuItem(&mi);
	}
	/*else
	{
		if (m_hMenuRoot)
			Menu_RemoveItem(m_hMenuRoot, 0);
		m_hMenuRoot = NULL;
	}*/

	mi.hParentMenu = hStatusMunuRoot;
	mi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR;

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
	mi.icolibItem = Skin_GetIconHandle("conference");
	HGENMENU hCreateChatRoom = Menu_AddProtoMenuItem(&mi);*/

	return 0;
}
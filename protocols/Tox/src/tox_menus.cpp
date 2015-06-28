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

	TMO_MenuItem mi = { 0 };
	mi.flags = CMIF_TCHAR;

	// Request authorization
	mi.pszService = MODULE"/RequestAuth";
	mi.name.t = LPGENT("Request authorization");
	mi.position = CMI_POSITION + CMI_AUTH_REQUEST;
	mi.hIcolibItem = ::Skin_GetIconHandle(SKINICON_AUTH_REQUEST);
	ContactMenuItems[CMI_AUTH_REQUEST] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CToxProto::OnRequestAuth>);

	// Grant authorization
	mi.pszService = MODULE"/GrantAuth";
	mi.name.t = LPGENT("Grant authorization");
	mi.position = CMI_POSITION + CMI_AUTH_GRANT;
	mi.hIcolibItem = ::Skin_GetIconHandle(SKINICON_AUTH_GRANT);
	ContactMenuItems[CMI_AUTH_GRANT] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CToxProto::OnGrantAuth>);

	// Start audio call
	mi.pszService = MODULE"/Audio/Call";
	mi.name.t = LPGENT("Call");
	mi.position = -2000020000 + CMI_AUDIO_CALL;
	mi.hIcolibItem = GetIconHandle("audio_start");
	ContactMenuItems[CMI_AUDIO_CALL] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CToxProto::OnSendAudioCall>);
}

void CToxProto::UninitMenus()
{
}


int CToxProto::OnInitStatusMenu()
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

	// Create copy tox id command
	mi.pszService = "/CopyToxID";
	CreateProtoService(mi.pszService, &CToxProto::OnCopyToxID);
	mi.name.t = LPGENT("Copy Tox ID");
	mi.position = SMI_POSITION + SMI_TOXID_COPY;
	Menu_AddProtoMenuItem(&mi, m_szModuleName);
	
	// Create group chat command
	/*
	mi.pszService = "/CreateChatRoom";
	CreateProtoService(mi.pszService, &CToxProto::OnCreateChatRoom);
	mi.name.t = LPGENT("Create group chat");
	mi.position = SMI_POSITION + SMI_GROUPCHAT_CREATE;
	mi.hIcolibItem = Skin_GetIconHandle("conference");
	HGENMENU hCreateChatRoom = Menu_AddProtoMenuItem(&mi, m_szModuleName);*/

	return 0;
}
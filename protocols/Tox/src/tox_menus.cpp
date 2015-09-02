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

	CMenuItem mi;
	mi.flags = CMIF_TCHAR;

	// Request authorization
	SET_UID(mi, 0x36375a1f, 0xc142, 0x4d6e, 0xa6, 0x57, 0xe4, 0x76, 0x5d, 0xbc, 0x59, 0x8e);
	mi.pszService = MODULE"/RequestAuth";
	mi.name.t = LPGENT("Request authorization");
	mi.position = CMI_POSITION + CMI_AUTH_REQUEST;
	mi.hIcolibItem = ::Skin_GetIconHandle(SKINICON_AUTH_REQUEST);
	ContactMenuItems[CMI_AUTH_REQUEST] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CToxProto::OnRequestAuth>);

	// Grant authorization
	SET_UID(mi, 0x4c90452a, 0x869a, 0x4a81, 0xaf, 0xa8, 0x28, 0x34, 0xaf, 0x2b, 0x6b, 0x30);
	mi.pszService = MODULE"/GrantAuth";
	mi.name.t = LPGENT("Grant authorization");
	mi.position = CMI_POSITION + CMI_AUTH_GRANT;
	mi.hIcolibItem = ::Skin_GetIconHandle(SKINICON_AUTH_GRANT);
	ContactMenuItems[CMI_AUTH_GRANT] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CToxProto::OnGrantAuth>);

	// Start audio call
	SET_UID(mi, 0x116cb7fe, 0xce37, 0x429c, 0xb0, 0xa9, 0x7d, 0xe7, 0x70, 0x59, 0xc3, 0x95);
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
	CMenuItem mi;
	mi.flags = CMIF_TCHAR;
	mi.root = Menu_GetProtocolRoot(this);

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
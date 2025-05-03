#include "stdafx.h"

HGENMENU CSteamProto::contactMenuItems[CMI_MAX];

INT_PTR CSteamProto::AuthRequestCommand(WPARAM hContact, LPARAM)
{
	ProtoChainSend(hContact, PSS_AUTHREQUEST, 0, 0);
	return 0;
}

INT_PTR CSteamProto::AuthRevokeCommand(WPARAM hContact, LPARAM)
{
	SendUserRemoveRequest(hContact);
	return 0;
}

INT_PTR CSteamProto::BlockCommand(WPARAM hContact, LPARAM)
{
	SendUserIgnoreRequest(hContact, true);
	return 0;
}

INT_PTR CSteamProto::UnblockCommand(WPARAM hContact, LPARAM)
{
	SendUserIgnoreRequest(hContact, false);
	return 0;
}

INT_PTR CSteamProto::JoinGameCommand(WPARAM hContact, LPARAM)
{
	char url[MAX_PATH];
	uint32_t gameId = getDword(hContact, "GameID", 0);
	mir_snprintf(url, "steam://rungameid/%lu", gameId);
	Utils_OpenUrl(url);
	return 0;
}

INT_PTR CSteamProto::JoinChatCommand(WPARAM hContact, LPARAM)
{
	CChatRoomJoinChatRoomGroupRequest request;
	request.chat_group_id = GetId(hContact, DBKEY_STEAM_ID); request.has_chat_group_id = true;
	request.chat_id = getDword(hContact, DBKEY_CHAT_ID); request.has_chat_id = true;
	WSSendService(JoinChatRoomGroup, request);
	return 0;
}

INT_PTR CSteamProto::OpenBlockListCommand(WPARAM, LPARAM)
{
	// SendRequest(new GetFriendListRequest(m_szAccessToken, m_iSteamId, "ignoredfriend"), &CSteamProto::OnGotBlockList);
	return 0;
}

int CSteamProto::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (!hContact)
		return 0;

	if (!IsOnline() || mir_strcmp(Proto_GetBaseAccountName(hContact), m_szModuleName))
		return 0;

	bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

	if (!Contact::IsGroupChat(hContact)) {
		bool authNeeded = getBool(hContact, "Auth");
		Menu_ShowItem(GetMenuItem(PROTO_MENU_REQ_AUTH), authNeeded || ctrlPressed);
		Menu_ShowItem(GetMenuItem(PROTO_MENU_REVOKE_AUTH), !authNeeded || ctrlPressed);
	}

	bool isBlocked = getBool(hContact, "Block");
	Menu_ShowItem(contactMenuItems[CMI_BLOCK], !isBlocked || ctrlPressed);
	Menu_ShowItem(contactMenuItems[CMI_UNBLOCK], isBlocked || ctrlPressed);

	uint32_t gameId = getDword(hContact, "GameID");
	Menu_ShowItem(contactMenuItems[CMI_JOIN_GAME], gameId || ctrlPressed);

	int chatType = getByte(hContact, "ChatRoom");
	int64_t chatId = GetId(hContact, DBKEY_CHAT_ID);
	Menu_ShowItem(contactMenuItems[CMI_JOIN_CHAT], chatId && chatType == GCW_SERVER);
	return 0;
}

int CSteamProto::PrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	for (auto &it : contactMenuItems)
		Menu_ShowItem(it, false);

	CSteamProto *ppro = CMPlugin::getInstance((MCONTACT)hContact);
	return (ppro) ? ppro->OnPrebuildContactMenu(hContact, lParam) : 0;
}

void CSteamProto::OnInitStatusMenu()
{
	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE;
	mi.root = Menu_GetProtocolRoot(this);

	// Show block list
	//mi.pszService = "/BlockList";
	//CreateProtoService(mi.pszService, &CSteamProto::OpenBlockListCommand);
	//mi.name.w = LPGENW("Blocked contacts");
	//mi.position = 200000 + SMI_BLOCKED_LIST;
	//Menu_AddProtoMenuItem(&mi, m_szModuleName);
}

void CSteamProto::InitMenus()
{
	CMenuItem mi(&g_plugin);

	// Block
	SET_UID(mi, 0xc6169b8f, 0x53ab, 0x4242, 0xbe, 0x90, 0xe2, 0x4a, 0xa5, 0x73, 0x88, 0x32);
	mi.pszService = MODULENAME "/Block";
	mi.name.a = LPGEN("Block");
	mi.position = -201001001 + CMI_BLOCK;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_OFF);
	contactMenuItems[CMI_BLOCK] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::BlockCommand>);

	// Unblock
	SET_UID(mi, 0xc6169b8f, 0x53ab, 0x4242, 0xbe, 0x90, 0xe2, 0x4a, 0xa5, 0x73, 0x88, 0x32);
	mi.pszService = MODULENAME "/Unblock";
	mi.name.a = LPGEN("Unblock");
	mi.position = -201001001 + CMI_UNBLOCK;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_ON);
	contactMenuItems[CMI_UNBLOCK] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::UnblockCommand>);

	mi.flags |= CMIF_NOTOFFLINE;

	// Join to game
	SET_UID(mi, 0x1a6aaab7, 0xba31, 0x4b47, 0x8e, 0xce, 0xf8, 0x8e, 0xf4, 0x62, 0x4f, 0xd7);
	mi.pszService = MODULENAME "/JoinGame";
	mi.name.a = LPGEN("Join the game");
	mi.position = -200001000 + CMI_JOIN_GAME;
	mi.hIcolibItem = nullptr;
	contactMenuItems[CMI_JOIN_GAME] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::JoinGameCommand>);

	// Join clan's chat
	SET_UID(mi, 0x4da0c3d3, 0x6fca, 0x42ba, 0xac, 0x72, 0x5c, 0xc9, 0x42, 0x5a, 0x70, 0x4e);
	mi.pszService = MODULENAME "/JoinChat";
	mi.name.a = LPGEN("Join group chat");
	mi.position = -200001002 + CMI_JOIN_GAME;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_CHAT_JOIN);
	contactMenuItems[CMI_JOIN_CHAT] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::JoinChatCommand>);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, &CSteamProto::PrebuildContactMenu);
}

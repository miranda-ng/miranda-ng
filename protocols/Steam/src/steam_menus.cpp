#include "stdafx.h"

int CSteamProto::hChooserMenu;
HGENMENU CSteamProto::contactMenuItems[CMI_MAX];

template<int(__cdecl CSteamProto::*Service)(WPARAM, LPARAM)>
INT_PTR GlobalService(WPARAM hContact, LPARAM lParam)
{
	CSteamProto *ppro = CMPlugin::getInstance((MCONTACT)hContact);
	return ppro ? (ppro->*Service)(hContact, lParam) : 0;
}

INT_PTR CSteamProto::AuthRequestCommand(WPARAM hContact, LPARAM)
{
	ProtoChainSend(hContact, PSS_AUTHREQUEST, 0, 0);
	return 0;
}

INT_PTR CSteamProto::AuthRevokeCommand(WPARAM hContact, LPARAM)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));
	ptrA steamId(getStringA("SteamID"));
	char *who = getStringA(hContact, "SteamID");
	PushRequest(new RemoveFriendRequest(token, sessionId, steamId, who), &CSteamProto::OnFriendRemoved, who);
	return 0;
}

int CSteamProto::BlockCommand(WPARAM hContact, LPARAM)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));
	ptrA steamId(getStringA("SteamID"));
	char *who = getStringA(hContact, "SteamID");
	PushRequest(new BlockFriendRequest(token, sessionId, steamId, who), &CSteamProto::OnFriendBlocked, who);
	return 0;
}

int CSteamProto::UnblockCommand(WPARAM hContact, LPARAM)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));
	ptrA steamId(getStringA("SteamID"));
	char *who = getStringA(hContact, "SteamID");
	PushRequest(new UnblockFriendRequest(token, sessionId, steamId, who), &CSteamProto::OnFriendUnblocked, who);
	return 0;
}

int CSteamProto::JoinToGameCommand(WPARAM hContact, LPARAM)
{
	char url[MAX_PATH];
	uint32_t gameId = getDword(hContact, "GameID", 0);
	mir_snprintf(url, "steam://rungameid/%lu", gameId);
	Utils_OpenUrl(url);
	return 0;
}

INT_PTR CSteamProto::OpenBlockListCommand(WPARAM, LPARAM)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA steamId(getStringA("SteamID"));
	PushRequest(new GetFriendListRequest(token, steamId, "ignoredfriend"), &CSteamProto::OnGotBlockList);
	return 0;
}

int CSteamProto::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (!hContact)
		return 0;

	if (!IsOnline() || mir_strcmp(Proto_GetBaseAccountName(hContact), m_szModuleName))
		return 0;

	bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

	bool authNeeded = getBool(hContact, "Auth", 0);
	Menu_ShowItem(GetMenuItem(PROTO_MENU_REQ_AUTH), authNeeded || ctrlPressed);
	Menu_ShowItem(GetMenuItem(PROTO_MENU_REVOKE_AUTH), !authNeeded || ctrlPressed);

	bool isBlocked = getBool(hContact, "Block", 0);
	Menu_ShowItem(contactMenuItems[CMI_BLOCK], !isBlocked || ctrlPressed);
	Menu_ShowItem(contactMenuItems[CMI_UNBLOCK], isBlocked || ctrlPressed);

	uint32_t gameId = getDword(hContact, "GameID", 0);
	Menu_ShowItem(contactMenuItems[CMI_JOIN_GAME], gameId || ctrlPressed);
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
	hChooserMenu = Menu_AddObject("SteamAccountChooser", LPGEN("Steam menu chooser"), nullptr, "Steam/MenuChoose");

	//////////////////////////////////////////////////////////////////////////////////////
	// Contact menu initialization
	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE;

	// "Block"
	SET_UID(mi, 0xc6169b8f, 0x53ab, 0x4242, 0xbe, 0x90, 0xe2, 0x4a, 0xa5, 0x73, 0x88, 0x32);
	mi.pszService = MODULE "/Block";
	mi.name.w = LPGENW("Block");
	mi.position = -201001001 + CMI_BLOCK;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_OFF);
	contactMenuItems[CMI_BLOCK] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::BlockCommand>);

	// "Unblock"
	SET_UID(mi, 0xc6169b8f, 0x53ab, 0x4242, 0xbe, 0x90, 0xe2, 0x4a, 0xa5, 0x73, 0x88, 0x32);
	mi.pszService = MODULE "/Unblock";
	mi.name.w = LPGENW("Unblock");
	mi.position = -201001001 + CMI_UNBLOCK;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_ON);
	contactMenuItems[CMI_UNBLOCK] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::UnblockCommand>);

	mi.flags |= CMIF_NOTOFFLINE;

	// "Join to game"
	SET_UID(mi, 0x1a6aaab7, 0xba31, 0x4b47, 0x8e, 0xce, 0xf8, 0x8e, 0xf4, 0x62, 0x4f, 0xd7);
	mi.pszService = MODULE "/JoinToGame";
	mi.name.w = LPGENW("Join to game");
	mi.position = -200001000 + CMI_JOIN_GAME;
	mi.hIcolibItem = nullptr;
	contactMenuItems[CMI_JOIN_GAME] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::JoinToGameCommand>);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, &CSteamProto::PrebuildContactMenu);
}

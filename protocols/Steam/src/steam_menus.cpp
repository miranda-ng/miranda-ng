#include "stdafx.h"

int CSteamProto::hChooserMenu;
HGENMENU CSteamProto::contactMenuItems[CMI_MAX];

template<int(__cdecl CSteamProto::*Service)(WPARAM, LPARAM)>
INT_PTR GlobalService(WPARAM wParam, LPARAM lParam)
{
	CSteamProto *ppro = CSteamProto::GetContactAccount((MCONTACT)wParam);
	return ppro ? (ppro->*Service)(wParam, lParam) : 0;
}

int CSteamProto::AuthRequestCommand(WPARAM hContact, LPARAM)
{
	ProtoChainSend(hContact, PSS_AUTHREQUEST, 0, 0);

	return 0;
}

int CSteamProto::AuthRevokeCommand(WPARAM hContact, LPARAM)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));
	ptrA steamId(getStringA("SteamID"));
	char *who = getStringA(hContact, "SteamID");
	PushRequest(
		new RemoveFriendRequest(token, sessionId, steamId, who),
		&CSteamProto::OnFriendRemoved,
		(void*)who);

	return 0;
}

int CSteamProto::BlockCommand(WPARAM hContact, LPARAM)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));
	ptrA steamId(getStringA("SteamID"));
	char *who = getStringA(hContact, "SteamID");

	PushRequest(
		new BlockFriendRequest(token, sessionId, steamId, who),
		&CSteamProto::OnFriendBlocked,
		who);

	return 0;
}

int CSteamProto::UnblockCommand(WPARAM hContact, LPARAM)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));
	ptrA steamId(getStringA("SteamID"));
	char *who = getStringA(hContact, "SteamID");

	PushRequest(
		new UnblockFriendRequest(token, sessionId, steamId, who),
		&CSteamProto::OnFriendUnblocked,
		who);

	return 0;
}

int CSteamProto::JoinToGameCommand(WPARAM hContact, LPARAM)
{
	char url[MAX_PATH];
	DWORD gameId = getDword(hContact, "GameID", 0);
	mir_snprintf(url, "steam://rungameid/%lu", gameId);
	Utils_OpenUrl(url);

	return 0;
}

INT_PTR CSteamProto::OpenBlockListCommand(WPARAM, LPARAM)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA steamId(getStringA("SteamID"));
	PushRequest(
		new GetFriendListRequest(token, steamId, "ignoredfriend"),
		&CSteamProto::OnGotBlockList);

	return 0;
}

int CSteamProto::OnPrebuildContactMenu(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	if (!hContact)
		return 0;

	if (!IsOnline() || mir_strcmp(GetContactProto(hContact), m_szModuleName))
		return 0;

	bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

	bool authNeeded = getBool(hContact, "Auth", 0);
	Menu_ShowItem(contactMenuItems[CMI_AUTH_REQUEST], authNeeded || ctrlPressed);
	Menu_ShowItem(contactMenuItems[CMI_AUTH_REVOKE], !authNeeded || ctrlPressed);

	bool isBlocked = getBool(hContact, "Block", 0);
	Menu_ShowItem(contactMenuItems[CMI_BLOCK], !isBlocked || ctrlPressed);
	Menu_ShowItem(contactMenuItems[CMI_UNBLOCK], isBlocked || ctrlPressed);

	DWORD gameId = getDword(hContact, "GameID", 0);
	Menu_ShowItem(contactMenuItems[CMI_JOIN_GAME], gameId || ctrlPressed);

	return 0;
}

int CSteamProto::PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i < CMI_MAX; i++)
		Menu_ShowItem(CSteamProto::contactMenuItems[i], false);

	CSteamProto* ppro = CSteamProto::GetContactAccount((MCONTACT)wParam);
	return (ppro) ? ppro->OnPrebuildContactMenu(wParam, lParam) : 0;
}

void CSteamProto::OnInitStatusMenu()
{
	CMenuItem mi;
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
	CMenuItem mi;
	mi.flags = CMIF_UNICODE;

	// "Request authorization"
	SET_UID(mi, 0x36375a1f, 0xc142, 0x4d6e, 0xa6, 0x57, 0xe4, 0x76, 0x5d, 0xbc, 0x59, 0x8e);
	mi.pszService = MODULE "/AuthRequest";
	mi.name.w = LPGENW("Request authorization");
	mi.position = -201001000 + CMI_AUTH_REQUEST;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REQUEST);
	contactMenuItems[CMI_AUTH_REQUEST] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::AuthRequestCommand>);

	// "Revoke authorization"
	SET_UID(mi, 0x619efdcb, 0x99c0, 0x44a8, 0xbf, 0x28, 0xc3, 0xe0, 0x2f, 0xb3, 0x7e, 0x77);
	mi.pszService = MODULE "/RevokeAuth";
	mi.name.w = LPGENW("Revoke authorization");
	mi.position = -201001001 + CMI_AUTH_REVOKE;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REVOKE);
	contactMenuItems[CMI_AUTH_REVOKE] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::AuthRevokeCommand>);

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
}

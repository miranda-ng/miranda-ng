#include "stdafx.h"

int CSteamProto::hChooserMenu;
HGENMENU CSteamProto::contactMenuItems[CMI_MAX];

template<int(__cdecl CSteamProto::*Service)(WPARAM, LPARAM)>
INT_PTR GlobalService(WPARAM wParam, LPARAM lParam)
{
	CSteamProto *ppro = CSteamProto::GetContactProtoInstance((MCONTACT)wParam);
	return ppro ? (ppro->*Service)(wParam, lParam) : 0;
}

INT_PTR CSteamProto::MenuChooseService(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
		*(void**)lParam = (void*)wParam;

	return 0;
}

int CSteamProto::AuthRequestCommand(WPARAM hContact, LPARAM)
{
	CallContactService(hContact, PSS_AUTHREQUEST, 0, 0);

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
		who,
		MirFreeArg);

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

	if (!this->IsOnline() || lstrcmpA(GetContactProto(hContact), m_szModuleName))
		return 0;

	//bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

	bool authNeeded = getBool(hContact, "Auth", 0);
	Menu_ShowItem(contactMenuItems[CMI_AUTH_REQUEST], authNeeded);

	bool isBlocked = getBool(hContact, "Block", 0);
	Menu_ShowItem(contactMenuItems[CMI_BLOCK], !isBlocked);

	DWORD gameId = getDword(hContact, "GameID", 0);
	Menu_ShowItem(contactMenuItems[CMI_JOIN_GAME], gameId > 0);

	return 0;
}

int CSteamProto::PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i < _countof(CSteamProto::contactMenuItems); i++)
		Menu_ShowItem(CSteamProto::contactMenuItems[i], false);

	CSteamProto* ppro = CSteamProto::GetContactProtoInstance((MCONTACT)wParam);
	return (ppro) ? ppro->OnPrebuildContactMenu(wParam, lParam) : 0;
}

void CSteamProto::OnInitStatusMenu()
{
	CMenuItem mi;
	mi.flags = CMIF_TCHAR;
	mi.root = Menu_GetProtocolRoot(this);

	// Show block list
	mi.pszService = "/BlockList";
	CreateProtoService(mi.pszService, &CSteamProto::OpenBlockListCommand);
	mi.name.t = LPGENT("Blocked contacts");
	mi.position = 200000 + SMI_BLOCKED_LIST;
	Menu_AddProtoMenuItem(&mi, m_szModuleName);
}

void CSteamProto::InitMenus()
{
	hChooserMenu = Menu_AddObject("SteamAccountChooser", LPGEN("Steam menu chooser"), 0, "Steam/MenuChoose");

	//////////////////////////////////////////////////////////////////////////////////////
	// Contact menu initialization
	CMenuItem mi;
	mi.flags = CMIF_TCHAR;

	// "Request authorization"
	SET_UID(mi, 0x2a5a29c5, 0xed3f, 0x4c15, 0xa7, 0x9c, 0xc1, 0x80, 0x58, 0xb8, 0x6e, 0x27);
	mi.pszService = MODULE "/AuthRequest";
	mi.name.t = LPGENT("Request authorization");
	mi.position = -201001000 + CMI_AUTH_REQUEST;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REQUEST);
	contactMenuItems[CMI_AUTH_REQUEST] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::AuthRequestCommand>);

	// "Block"
	SET_UID(mi, 0xe22ccbb4, 0x7876, 0x42f0, 0x96, 0x85, 0x61, 0x89, 0xea, 0x42, 0xb, 0x85);
	mi.pszService = MODULE "/Block";
	mi.name.t = LPGENT("Block");
	mi.position = -201001001 + CMI_BLOCK;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REQUEST);
	contactMenuItems[CMI_BLOCK] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::BlockCommand>);

	mi.flags |= CMIF_NOTOFFLINE;

	// "Join to game"
	SET_UID(mi, 0x1a6aaab7, 0xba31, 0x4b47, 0x8e, 0xce, 0xf8, 0x8e, 0xf4, 0x62, 0x4f, 0xd7);
	mi.pszService = MODULE "/JoinToGame";
	mi.name.t = LPGENT("Join to game");
	mi.position = -200001000 + CMI_JOIN_GAME;
	mi.hIcolibItem = NULL;
	contactMenuItems[CMI_JOIN_GAME] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::JoinToGameCommand>);
}

void CSteamProto::UninitMenus()
{
	Menu_RemoveItem(contactMenuItems[CMI_AUTH_REQUEST]);
	Menu_RemoveItem(contactMenuItems[CMI_BLOCK]);
	Menu_RemoveItem(contactMenuItems[CMI_JOIN_GAME]);
}

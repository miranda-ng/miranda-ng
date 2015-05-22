#include "stdafx.h"

HANDLE CSteamProto::hChooserMenu;
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
		new SteamWebApi::BlockFriendRequest(token, sessionId, steamId, who),
		&CSteamProto::OnFriendBlocked,
		who,
		ARG_MIR_FREE);

	return 0;
}

int CSteamProto::JoinToGameCommand(WPARAM hContact, LPARAM)
{
	char url[MAX_PATH];
	DWORD gameId = getDword(hContact, "GameID", 0);
	mir_snprintf(url, SIZEOF(url), "steam://rungameid/%lu", gameId);
	CallService(MS_UTILS_OPENURL, 0, (LPARAM)url);

	return 0;
}

INT_PTR CSteamProto::OpenBlockListCommand(WPARAM, LPARAM)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA steamId(getStringA("SteamID"));

	PushRequest(
		new SteamWebApi::GetFriendListRequest(token, steamId, "ignoredfriend"),
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
	for (int i = 0; i < SIZEOF(CSteamProto::contactMenuItems); i++)
		Menu_ShowItem(CSteamProto::contactMenuItems[i], false);

	CSteamProto* ppro = CSteamProto::GetContactProtoInstance((MCONTACT)wParam);
	return (ppro) ? ppro->OnPrebuildContactMenu(wParam, lParam) : 0;
}

void CSteamProto::OnInitStatusMenu()
{
	char text[200];
	mir_strncpy(text, m_szModuleName, 100);
	char* tDest = text + mir_strlen(text);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszService = text;

	HGENMENU hSteamRoot = MO_GetProtoRootMenu(m_szModuleName);
	if (!hSteamRoot)
	{
		mi.ptszName = m_tszUserName;
		mi.position = -1999901006;
		mi.hParentMenu = HGENMENU_ROOT;
		mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
		//mi.icolibItem = NULL;
		hSteamRoot = m_hMenuRoot = Menu_AddProtoMenuItem(&mi);
	}
	else
	{
		if (m_hMenuRoot)
			CallService(MO_REMOVEMENUITEM, (WPARAM)m_hMenuRoot, 0);
		m_hMenuRoot = NULL;
	}

	mi.hParentMenu = hSteamRoot;
	mi.flags = CMIF_CHILDPOPUP | CMIF_TCHAR;

	// Show block list
	mir_strcpy(tDest, "/BlockList");
	CreateProtoService(tDest, &CSteamProto::OpenBlockListCommand);
	mi.ptszName = LPGENT("Blocked contacts");
	mi.position = 200000 + SMI_BLOCKED_LIST;
	//mi.icolibItem = NULL;
	Menu_AddProtoMenuItem(&mi);
}

void CSteamProto::InitMenus()
{
	hChooserMenu = MO_CreateMenuObject("SteamAccountChooser", LPGEN("Steam menu chooser"), 0, "Steam/MenuChoose");

	//////////////////////////////////////////////////////////////////////////////////////
	// Contact menu initialization
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.flags = CMIF_TCHAR;

	// "Request authorization"
	mi.pszService = MODULE "/AuthRequest";
	mi.ptszName = LPGENT("Request authorization");
	mi.position = -201001000 + CMI_AUTH_REQUEST;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	contactMenuItems[CMI_AUTH_REQUEST] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::AuthRequestCommand>);

	// "Block"
	mi.pszService = MODULE "/Block";
	mi.ptszName = LPGENT("Block");
	mi.position = -201001001 + CMI_BLOCK;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	contactMenuItems[CMI_BLOCK] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::BlockCommand>);

	mi.flags |= CMIF_NOTOFFLINE;

	// "Join to game"
	mi.pszService = MODULE "/JoinToGame";
	mi.ptszName = LPGENT("Join to game");
	mi.position = -200001000 + CMI_JOIN_GAME;
	mi.icolibItem = NULL;
	contactMenuItems[CMI_JOIN_GAME] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::JoinToGameCommand>);
}

void CSteamProto::UninitMenus()
{
	CallService(MO_REMOVEMENUITEM, (WPARAM)contactMenuItems[CMI_AUTH_REQUEST], 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)contactMenuItems[CMI_BLOCK], 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)contactMenuItems[CMI_JOIN_GAME], 0);
}


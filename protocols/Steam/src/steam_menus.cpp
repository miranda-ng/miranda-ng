#include "common.h"

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

int CSteamProto::JoinToGameCommand(WPARAM hContact, LPARAM)
{
	char url[MAX_PATH];
	DWORD gameId = getDword(hContact, "GameID", 0);
	mir_snprintf(url, SIZEOF(url), "steam://rungameid/%lu", gameId);
	CallService(MS_UTILS_OPENURL, 0, (LPARAM)url);

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

void CSteamProto::InitMenus()
{
	hChooserMenu = MO_CreateMenuObject("SkypeAccountChooser", LPGEN("Steam menu chooser"), 0, "Steam/MenuChoose");

	//////////////////////////////////////////////////////////////////////////////////////
	// Contact menu initialization
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.flags = CMIF_TCHAR | CMIF_NOTOFFLINE;

	// "Join to game"
	mi.pszService = MODULE"/JoinToGame";
	mi.ptszName = LPGENT("Join to game");
	mi.position = -200001000 + CMI_JOIN_GAME;
	//mi.icolibItem = CSkypeProto::GetSkinIconHandle("block");
	contactMenuItems[CMI_JOIN_GAME] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CSteamProto::JoinToGameCommand>);
}

void CSteamProto::UninitMenus()
{
	CallService(MS_CLIST_REMOVETRAYMENUITEM, (WPARAM)contactMenuItems[CMI_JOIN_GAME], 0);
}
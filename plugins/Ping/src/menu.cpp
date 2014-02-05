#include "common.h"

HANDLE hMenuDisable, hMenuGraph, hMenuEdit;
HANDLE hEventMenuBuild;

void InitMenus()
{
	// main menu
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_TCHAR;
	mi.popupPosition = 500099900;
	mi.ptszPopupName = LPGENT("Ping");
	mi.position = 2000060000;
	mi.hIcon = hIconResponding;
	mi.ptszName = LPGENT("Enable all pings");
	mi.pszService = PLUG "/EnableAll";
	Menu_AddMainMenuItem(&mi);

	mi.popupPosition = 500299901;
	mi.position = 2000060001;
	mi.popupPosition = 0;
	mi.hIcon = hIconDisabled;
	mi.ptszName = LPGENT("Disable all pings");
	mi.pszService = PLUG "/DisableAll";
	Menu_AddMainMenuItem(&mi);
}

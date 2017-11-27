#include "stdafx.h"

HANDLE hMenuDisable, hMenuGraph, hMenuEdit;
HANDLE hEventMenuBuild;

void InitMenus()
{
	// main menu
	CMenuItem mi;
	mi.flags = CMIF_UNICODE;
	mi.root = Menu_CreateRoot(MO_MAIN, LPGENW("Ping"), 500099900);
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "FF273885-6AEC-4C8A-B8BC-9247D920F223");

	SET_UID(mi, 0xc9a4d17a, 0xd5fa, 0x40d9, 0xbe, 0x7b, 0x9e, 0x2e, 0x5b, 0xbf, 0x8, 0x59);
	mi.position = 2000060000;
	mi.hIcolibItem = hIconResponding;
	mi.name.w = LPGENW("Enable all pings");
	mi.pszService = PLUG "/EnableAll";
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x1dac6ea6, 0x116e, 0x41a3, 0xa6, 0xe8, 0x42, 0xdf, 0xf7, 0x27, 0xc4, 0x7c);
	mi.position = 2000060001;
	mi.hIcolibItem = hIconDisabled;
	mi.name.w = LPGENW("Disable all pings");
	mi.pszService = PLUG "/DisableAll";
	Menu_AddMainMenuItem(&mi);
}

#include "stdafx.h"

HICON hIconRemove, hIconKeep, hIconClear;

static IconItem iconList[] = 
{
	{ LPGEN("Disable"), "remove", IDI_HREMOVE },
	{ LPGEN("Enable"),  "keep",   IDI_HKEEP   },
	{ LPGEN("Clear"),   "clear",  IDI_HCLEAR  },
};

int ReloadIcons(WPARAM, LPARAM)
{
	hIconRemove	= IcoLib_GetIconByHandle(iconList[0].hIcolib);
	hIconKeep	= IcoLib_GetIconByHandle(iconList[1].hIcolib);
	hIconClear	= IcoLib_GetIconByHandle(iconList[2].hIcolib);
	return 0;
}

void InitIcons()
{
	g_plugin.registerIcon(LPGEN("No History"), iconList, MODULENAME);

	ReloadIcons(0, 0);

	HookEvent(ME_SKIN_ICONSCHANGED, ReloadIcons);
}

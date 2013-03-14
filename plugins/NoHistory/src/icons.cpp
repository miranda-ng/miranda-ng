#include "common.h"

HICON hIconRemove, hIconKeep, hIconClear;

static IconItem iconList[] = 
{
	{ LPGEN("Disable"), "remove", IDI_HREMOVE },
	{ LPGEN("Enable"),  "keep",   IDI_HKEEP   },
	{ LPGEN("Clear"),   "clear",  IDI_HCLEAR  },
};

int ReloadIcons(WPARAM wParam, LPARAM lParam)
{
	hIconRemove	= Skin_GetIconByHandle(iconList[0].hIcolib);
	hIconKeep	= Skin_GetIconByHandle(iconList[1].hIcolib);
	hIconClear	= Skin_GetIconByHandle(iconList[2].hIcolib);
	return 0;
}

void InitIcons()
{
	Icon_Register(hInst, LPGEN("No History"), iconList, SIZEOF(iconList), MODULE);

	ReloadIcons(0, 0);

	HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);
}

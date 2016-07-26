#include "stdafx.h"

IconItem Icons[] =
{
	{ LPGEN("Script"), "script", IDI_SCRIPT },
	{ LPGEN("Loaded"), "loaded", IDI_LOADED },
	{ LPGEN("Failed"), "failed", IDI_FAILED },
	{ LPGEN("Open"),   "open",   IDI_OPEN   },
	{ LPGEN("Reload"), "reload", IDI_RELOAD },
};

void InitIcons()
{
	Icon_Register(g_hInstance, MODULE, Icons, _countof(Icons), MODULE);
}

HICON GetIcon(int iconId)
{
	for (size_t i = 0; i < _countof(Icons); i++)
		if (Icons[i].defIconID == iconId)
			return IcoLib_GetIconByHandle(Icons[i].hIcolib);

	return NULL;
}

HANDLE GetIconHandle(int iconId)
{
	for (size_t i = 0; i < _countof(Icons); i++)
		if (Icons[i].defIconID == iconId)
			return Icons[i].hIcolib;

	return NULL;
}
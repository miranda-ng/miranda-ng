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
	g_plugin.registerIcon(MODULE, Icons, MODULE);
}

HICON GetIcon(int iconId)
{
	for (auto &it : Icons)
		if (it.defIconID == iconId)
			return IcoLib_GetIconByHandle(it.hIcolib);

	return nullptr;
}

HANDLE GetIconHandle(int iconId)
{
	for (auto &it : Icons)
		if (it.defIconID == iconId)
			return it.hIcolib;

	return nullptr;
}
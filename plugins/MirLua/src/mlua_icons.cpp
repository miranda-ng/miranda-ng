#include "stdafx.h"

IconItemT Icons[] =
{
	{ LPGENT("Script"),			"script",			IDI_SCRIPT		},
	{ LPGENT("Open"),			"open",				IDI_OPEN		},
};

void InitIcons()
{
	Icon_RegisterT(g_hInstance, LPGENT(MODULE), Icons, _countof(Icons), MODULE);
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
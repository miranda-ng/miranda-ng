#include "stdafx.h"

static IconItem iconList[] =
{
	{ LPGEN("Protocol icon"), "main", IDI_DROPBOX }
};

void InitializeIcons()
{
	Icon_Register(g_hInstance, "Protocols/" MODULE, iconList, SIZEOF(iconList), MODULE);
}

HANDLE GetIconHandle(int iconId)
{
	for (unsigned i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return iconList[i].hIcolib;

	return NULL;
}

HICON LoadIconEx(int iconId, bool big)
{
	for (int i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return Skin_GetIconByHandle(iconList[i].hIcolib, big);

	return NULL;
}

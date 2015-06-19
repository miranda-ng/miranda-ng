#include "stdafx.h"

static IconItem iconList[] =
{
	{	LPGEN("Protocol icon"),		"main",		IDI_DROPBOX	},
	{	LPGEN("Upload file(s)"),	"upload",	IDI_UPLOAD	}
};

void InitializeIcons()
{
	Icon_Register(g_hInstance, "Protocols/" MODULE, iconList, _countof(iconList), MODULE);
}

HANDLE GetIconHandleByName(const char *name)
{
	for (size_t i = 0; i < _countof(iconList); i++)
		if (mir_strcmpi(iconList[i].szName, name) == 0)
			return iconList[i].hIcolib;

	return NULL;
}

HICON LoadIconEx(int iconId, bool big)
{
	for (int i = 0; i < _countof(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return IcoLib_GetIconByHandle(iconList[i].hIcolib, big);

	return NULL;
}

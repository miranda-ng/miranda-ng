#include "stdafx.h"

static IconItem iconList[] =
{
	{	LPGEN("Upload file(s)"),	"upload",	IDI_UPLOAD		},
	{	LPGEN("Dropbox"),			"dropbox",	IDI_DROPBOX		},
	{	LPGEN("Google Drive"),		"gdrive",	IDI_GDRIVE		},
	{	LPGEN("OneDrive"),			"onedrive",	IDI_ONEDRIVE	},
	{	LPGEN("Yandex.Disk"),		"yadisk",	IDI_YADISK		}
};

void InitializeIcons()
{
	Icon_Register(hInstance, "Protocols/" MODULE, iconList, _countof(iconList), MODULE);
}

HANDLE GetIconHandle(int iconId)
{
	for (auto &it : iconList)
		if (it.defIconID == iconId)
			return it.hIcolib;

	return nullptr;
}

HANDLE GetIconHandle(const char *name)
{
	for (auto &it : iconList)
		if (mir_strcmpi(it.szName, name) == 0)
			return it.hIcolib;

	return nullptr;
}

HICON LoadIconEx(int iconId, bool big)
{
	for (auto &it : iconList)
		if (it.defIconID == iconId)
			return IcoLib_GetIconByHandle(it.hIcolib, big);

	return nullptr;
}
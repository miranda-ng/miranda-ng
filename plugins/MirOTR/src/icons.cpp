#include "stdafx.h"

TCHAR g_dllpath[1024];

static IconItem iconList[] = {
	{ LPGEN(LANG_ICON_OTR), ICON_OTR, IDI_OTR },
	{ LPGEN(LANG_ICON_PRIVATE), ICON_PRIVATE, IDI_PRIVATE },
	{ LPGEN(LANG_ICON_UNVERIFIED), ICON_UNVERIFIED, IDI_UNVERIFIED },
	{ LPGEN(LANG_ICON_FINISHED), ICON_FINISHED, IDI_FINISHED },
	{ LPGEN(LANG_ICON_NOT_PRIVATE), ICON_NOT_PRIVATE, IDI_INSECURE },
	{ LPGEN(LANG_ICON_REFRESH), ICON_REFRESH, IDI_REFRESH }
};

void InitIcons()
{
	Icon_Register(hInst, "OTR", iconList, SIZEOF(iconList));
}

HICON LoadIcon(const char* name, int big) {
	return Skin_GetIcon(name, big);
}

HANDLE GetIconHandle(const char* name)
{
	return Skin_GetIconHandle(name);
}

void ReleaseIcon(const char* name, int big)
{
	Skin_ReleaseIcon(name, big);
}

void ReleaseIcon(HICON handle, int big)
{
	Skin_ReleaseIcon(handle);
}

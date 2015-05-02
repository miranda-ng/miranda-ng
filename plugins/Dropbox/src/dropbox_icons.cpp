#include "stdafx.h"

void CDropbox::InitializeIcons()
{
	wchar_t filePath[MAX_PATH];
	GetModuleFileName(g_hInstance, filePath, MAX_PATH);

	wchar_t sectionName[100];
	mir_sntprintf(
		sectionName,
		SIZEOF(sectionName),
		_T("%s/%s"),
		LPGENT("Protocols"),
		LPGENT(MODULE));

	char settingName[100];
	mir_snprintf(
		settingName,
		SIZEOF(settingName),
		"%s_%s",
		MODULE,
		"main");

	SKINICONDESC sid = { 0 };
	sid.cbSize = sizeof(SKINICONDESC);
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszDefaultFile = filePath;
	sid.pszName = settingName;
	sid.ptszSection = sectionName;
	sid.ptszDescription = LPGENT("Protocol icon");
	sid.iDefaultIndex = -IDI_DROPBOX;
	/*HANDLE hIcon = */Skin_AddIcon(&sid);
}

HICON CDropbox::LoadIconEx(const char *name, bool big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", MODULE, name);
	return Skin_GetIcon(szSettingName, big);
}

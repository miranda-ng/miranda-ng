#include "common.h"

IconInfo CSkypeProto::Icons[] =
{
	{ LPGENT("Protocol icon"),			"main",				IDI_SKYPE },
};

void CSkypeProto::InitIcons()
{
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(g_hInstance, szFile, MAX_PATH);

	char szSettingName[100];
	TCHAR szSectionName[100];

	SKINICONDESC sid = { sizeof(SKINICONDESC) };
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszDefaultFile = szFile;
	sid.pszName = szSettingName;
	sid.ptszSection = szSectionName;

	mir_sntprintf(szSectionName, SIZEOF(szSectionName), _T("%s/%s"), LPGENT("Protocols"), LPGENT(MODULE));
	for (int i = 0; i < SIZEOF(Icons); i++)
	{
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", MODULE, Icons[i].Name);

		sid.ptszDescription = Icons[i].Description;
		sid.iDefaultIndex = -Icons[i].IconId;
		Icons[i].Handle = Skin_AddIcon(&sid);
	}
}

HANDLE CSkypeProto::GetIconHandle(const char *name)
{
	for (size_t i = 0; i < SIZEOF(Icons); i++)
		if (mir_strcmpi(Icons[i].Name, name) == 0)
			return Icons[i].Handle;
	return 0;
}

HANDLE CSkypeProto::GetSkinIconHandle(const char *name)
{
	char iconName[100];
	mir_snprintf(iconName, SIZEOF(iconName), "%s_%s", MODULE, name);
	HANDLE hIcon = Skin_GetIconHandle(iconName);
	if (hIcon == NULL)
		hIcon = GetIconHandle(name);
	return hIcon;
}

void CSkypeProto::UninitIcons()
{
	for (size_t i = 0; i < SIZEOF(Icons); i++)
		Skin_RemoveIcon(Icons[i].Name);
}
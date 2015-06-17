#include "stdafx.h"

IconInfo CToxProto::Icons[] =
{
	{ LPGENT("Protocol icon"),			"main",				IDI_TOX			},
	{ LPGENT("Audio call"),				"audio_call",		IDI_AUDIO_CALL	},
	{ LPGENT("Audio ring"),				"audio_ring",		IDI_AUDIO_RING	},
	{ LPGENT("Audio start"),			"audio_start",		IDI_AUDIO_START	},
	{ LPGENT("Audio end"),				"audio_end",		IDI_AUDIO_END	},
};

void CToxProto::InitIcons()
{
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(g_hInstance, szFile, MAX_PATH);

	char szSettingName[100];
	TCHAR szSectionName[100];

	SKINICONDESC sid = { 0 };
	sid.flags = SIDF_ALL_TCHAR;
	sid.defaultFile.t = szFile;
	sid.pszName = szSettingName;
	sid.section.t = szSectionName;

	mir_sntprintf(szSectionName, SIZEOF(szSectionName), _T("%s/%s"), LPGENT("Protocols"), LPGENT(MODULE));
	for (int i = 0; i < SIZEOF(Icons); i++)
	{
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", MODULE, Icons[i].Name);

		sid.description.t = Icons[i].Description;
		sid.iDefaultIndex = -Icons[i].IconId;
		Icons[i].Handle = IcoLib_AddIcon(&sid);
	}
}

HICON CToxProto::GetIcon(const char *name, bool size)
{
	for (size_t i = 0; i < SIZEOF(Icons); i++)
		if (mir_strcmpi(Icons[i].Name, name) == 0)
			return IcoLib_GetIconByHandle(Icons[i].Handle, size);

	return NULL;
}

HANDLE CToxProto::GetIconHandle(const char *name)
{
	for (size_t i = 0; i < SIZEOF(Icons); i++)
		if (mir_strcmpi(Icons[i].Name, name) == 0)
			return Icons[i].Handle;

	return NULL;
}

HANDLE CToxProto::Skin_GetIconHandle(const char *name)
{
	char iconName[100];
	mir_snprintf(iconName, SIZEOF(iconName), "%s_%s", MODULE, name);
	HANDLE hIcon = IcoLib_GetIconHandle(iconName);
	if (hIcon == NULL)
		hIcon = GetIconHandle(name);

	return hIcon;
}

void CToxProto::UninitIcons()
{
	for (size_t i = 0; i < SIZEOF(Icons); i++)
		IcoLib_RemoveIcon(Icons[i].Name);
}
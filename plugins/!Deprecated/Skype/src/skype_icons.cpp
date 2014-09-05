#include "skype.h"

_tag_iconList CSkypeProto::IconList[] =
{
	{ LPGENT("Protocol icon"),			"main",				IDI_SKYPE },

	{ LPGENT("Call"),					"call",				IDI_CALL },
	{ LPGENT("Invite to conference"),	"addContacts",		IDI_ADD_CONTACTS },
	{ LPGENT("Conference"),				"conference",		IDI_CONFERENCE },
	{ LPGENT("Send contact"),			"sendContacts",		IDI_SEND_CONTACTS },
	{ LPGENT("Contact"),				"contact",			IDI_CONTACT },
	{ LPGENT("Delete"),					"delete",			IDI_DELETE },
	{ LPGENT("Block"),					"block",			IDI_BLOCK },
};

void CSkypeProto::InitIcons()
{
	wchar_t szFile[MAX_PATH];
	::GetModuleFileName(g_hInstance, szFile, MAX_PATH);

	char szSettingName[100];
	wchar_t szSectionName[100];

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(SKINICONDESC);
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszDefaultFile = szFile;
	sid.pszName = szSettingName;
	sid.ptszSection = szSectionName;

	::mir_sntprintf(szSectionName, SIZEOF(szSectionName), _T("%s/%s"), LPGENT("Protocols"), LPGENT(MODULE));
	for (int i = 0; i < SIZEOF(CSkypeProto::IconList); i++) 
	{
		::mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", MODULE, CSkypeProto::IconList[i].Name);

		sid.ptszDescription = CSkypeProto::IconList[i].Description;
		sid.iDefaultIndex = -CSkypeProto::IconList[i].IconId;
		CSkypeProto::IconList[i].Handle = ::Skin_AddIcon(&sid);
	}	
}

HANDLE CSkypeProto::GetIconHandle(const char* name)
{
	for (size_t i = 0; i < SIZEOF(CSkypeProto::IconList); i++)
	{
		if (::stricmp(CSkypeProto::IconList[i].Name, name) == 0)
			return CSkypeProto::IconList[i].Handle;
	}
	return 0;
}

HANDLE CSkypeProto::GetSkinIconHandle(const char* name)
{
	char iconName[100];
	::mir_snprintf(iconName, SIZEOF(iconName), "%s_%s", MODULE, name);
	HANDLE hIcon = ::Skin_GetIconHandle(iconName);
	if ( !hIcon)
		hIcon = CSkypeProto::GetIconHandle(name);
	return hIcon;
}

void CSkypeProto::UninitIcons()
{
	for (size_t i = 0; i < SIZEOF(CSkypeProto::IconList); i++)
		::Skin_RemoveIcon(CSkypeProto::IconList[i].Name);
}
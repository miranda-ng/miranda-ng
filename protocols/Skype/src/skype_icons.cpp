#include "skype_proto.h"

_tag_iconList CSkypeProto::iconList[] =
{
	{ LPGENT("Protocol icon"),		"main",			IDI_ICON },
	{ LPGENT("Revoke authorization"),	"authRevoke",		IDI_AUTH_REVOKE },
	{ LPGENT("Request authorization"),	"authRequest",		IDI_AUTH_REQUEST },
	{ LPGENT("Grant authorization"),	"authGrant",		IDI_AUTH_GRANT },
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
	for (int i = 0; i < SIZEOF(iconList); i++) 
	{
		::mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", MODULE, iconList[i].Name);

		sid.ptszDescription = iconList[i].Description;
		sid.iDefaultIndex = -iconList[i].IconId;
		iconList[i].Handle = ::Skin_AddIcon(&sid);
	}	
}

HANDLE GetIconHandle(const char* name)
{
	for(size_t i=0; i<SIZEOF(iconList); i++)
	{
		if(strcmp(iconList[i].Name, name) == 0)
			return iconList[i].Handle;
	}
	return 0;
}

void CSkypeProto::UninitIcons()
{
	for (int i = 0; i < SIZEOF(iconList); i++)
	{
		::Skin_RemoveIcon(iconList[i].Name);
	}
}
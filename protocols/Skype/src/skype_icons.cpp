#include "skype_proto.h"

struct _tag_iconList
{
	TCHAR*  szDescr;
	char*  szName;
	int    defIconID;
	HANDLE hIconLibItem;
}

static iconList[] =
{
	{LPGENT("Protocol icon"),	"main",			IDI_ICON},
};

void IconsLoad()
{
	TCHAR szFile[MAX_PATH];
	char szSettingName[100];
	SKINICONDESC sid = {0};
	unsigned i;

	GetModuleFileName(hInst, szFile, MAX_PATH);

	sid.cbSize = sizeof(SKINICONDESC);
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszDefaultFile = szFile;
	sid.pszName = szSettingName;
	sid.ptszSection = _T(MODULE);

	for (i = 0; i < SIZEOF(iconList); i++) 
	{
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", MODULE, iconList[i].szName);

		sid.ptszDescription = iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		iconList[i].hIconLibItem = Skin_AddIcon(&sid);
	}	
}
#include "metacontacts.h"

struct
{
	char*  szDescr;
	char*  szName;
	int    defIconID;
	HANDLE hIcolib;
}
static iconList[] = {
	{ LPGEN("Toggle Off"),             "mc_off",     IDI_MCMENUOFF     },
	{ LPGEN("Toggle On"),              "mc_on",      IDI_MCMENU        },
	{ LPGEN("Convert to MetaContact"), "mc_convert", IDI_MCCONVERT     },
	{ LPGEN("Add to Existing"),        "mc_add",     IDI_MCADD         },
	{ LPGEN("Edit"),                   "mc_edit",    IDI_MCEDIT        },
	{ LPGEN("Set to Default"),         "mc_default", IDI_MCSETDEFAULT  },
	{ LPGEN("Remove"),                 "mc_remove",  IDI_MCREMOVE      },
};

HANDLE GetIconHandle(IconIndex i)
{
	return iconList[i].hIcolib;
}

HICON LoadIconEx(IconIndex i)
{
	return Skin_GetIcon(iconList[i].szName);
}

void InitIcons(void) 
{
	TCHAR path[MAX_PATH];
	GetModuleFileName(hInstance, path, SIZEOF(path));

	SKINICONDESC sid = { sizeof(sid) };
	sid.flags = SIDF_PATH_TCHAR;
	sid.pszSection = META_PROTO;
	sid.ptszDefaultFile = path;

	for (int i=0; i < SIZEOF(iconList); ++i) {
		sid.pszDescription = iconList[i].szDescr;
		sid.pszName = iconList[i].szName;
		sid.iDefaultIndex = -iconList[i].defIconID;
		iconList[i].hIcolib = Skin_AddIcon(&sid);
	}
}

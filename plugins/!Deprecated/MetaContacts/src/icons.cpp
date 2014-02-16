#include "metacontacts.h"

static IconItem iconList[] = {
	{ LPGEN("Toggle Off"),             "off",     IDI_MCMENUOFF     },
	{ LPGEN("Toggle On"),              "on",      IDI_MCMENU        },
	{ LPGEN("Convert to MetaContact"), "convert", IDI_MCCONVERT     },
	{ LPGEN("Add to Existing"),        "add",     IDI_MCADD         },
	{ LPGEN("Edit"),                   "edit",    IDI_MCEDIT        },
	{ LPGEN("Set to Default"),         "default", IDI_MCSETDEFAULT  },
	{ LPGEN("Remove"),                 "remove",  IDI_MCREMOVE      },
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
	Icon_Register(hInstance, META_PROTO, iconList, SIZEOF(iconList), "mc");
}

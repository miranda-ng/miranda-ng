#include "metacontacts.h"

HANDLE hIcoLibIconsChanged = NULL;


typedef struct {
	char* szDescr;
	char* szName;
	int   defIconID;
} IconStruct;

static IconStruct iconList[] = {
	{ "Toggle Off",              "mc_off",      IDI_MCMENUOFF     },
	{ "Toggle On",               "mc_on",       IDI_MCMENU        },
	{ "Convert to MetaContact",  "mc_convert",  IDI_MCCONVERT     },
	{ "Add to Existing",         "mc_add",      IDI_MCADD         },
	{ "Edit",                    "mc_edit",     IDI_MCEDIT        },
	{ "Set to Default",          "mc_default",  IDI_MCSETDEFAULT  },
	{ "Remove",                  "mc_remove",   IDI_MCREMOVE      },
};


HICON LoadIconEx(IconIndex i) {
	HICON hIcon;

	if (hIcoLibIconsChanged)
		hIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)iconList[(int)i].szName);
	else
		hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(iconList[(int)i].defIconID), IMAGE_ICON, 0, 0, 0);

	return hIcon;
}


void ReleaseIconEx(HICON hIcon) {
	if (hIcoLibIconsChanged)
		CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
	else
		DestroyIcon(hIcon);
}

int ReloadIcons(WPARAM wParam, LPARAM lParam) {
	// fix menu icons
	CLISTMENUITEM menu = {0};

	menu.cbSize = sizeof(menu);
	menu.flags = CMIM_ICON;

	menu.hIcon = LoadIconEx(Meta_IsEnabled() ? I_MENUOFF : I_MENU);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuOnOff, (LPARAM)&menu);
	ReleaseIconEx(menu.hIcon);

	menu.hIcon = LoadIconEx(I_CONVERT);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuConvert, (LPARAM)&menu);
	ReleaseIconEx(menu.hIcon);
	
	menu.hIcon = LoadIconEx(I_ADD);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuAdd, (LPARAM)&menu);
	ReleaseIconEx(menu.hIcon);
	
	menu.hIcon = LoadIconEx(I_EDIT);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuEdit, (LPARAM)&menu);
	ReleaseIconEx(menu.hIcon);

	menu.hIcon = LoadIconEx(I_SETDEFAULT);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDefault, (LPARAM)&menu);
	ReleaseIconEx(menu.hIcon);

	menu.hIcon = LoadIconEx(I_REMOVE);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDelete, (LPARAM)&menu);
	ReleaseIconEx(menu.hIcon);

	return 0;
}

void InitIcons(void)  {
	SKINICONDESC sid = {0};
	char path[MAX_PATH];
	int i;
	
	sid.cbSize = sizeof(SKINICONDESC);
	sid.pszSection = META_PROTO;
	sid.pszDefaultFile = path;
	GetModuleFileName(hInstance, path, sizeof(path));

	for (i = 0; i < sizeof(iconList) / sizeof(IconStruct); ++i)
	{
		sid.pszDescription = Translate(iconList[i].szDescr);
		sid.pszName = iconList[i].szName;
		sid.iDefaultIndex = -iconList[i].defIconID;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}

	hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);

	ReloadIcons(0, 0); 
}

void DeinitIcons(void) {
	if (hIcoLibIconsChanged) UnhookEvent(hIcoLibIconsChanged);
}

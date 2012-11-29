#include "metacontacts.h"

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


HICON LoadIconEx(IconIndex i)
{
	return Skin_GetIcon(iconList[i].szName);
}

void ReleaseIconEx(HICON hIcon)
{
	Skin_ReleaseIcon(hIcon);
}

int ReloadIcons(WPARAM wParam, LPARAM lParam)
{
	// fix menu icons
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ICON;

	mi.hIcon = LoadIconEx(Meta_IsEnabled() ? I_MENUOFF : I_MENU);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuOnOff, (LPARAM)&mi);
	ReleaseIconEx(mi.hIcon);

	mi.hIcon = LoadIconEx(I_CONVERT);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuConvert, (LPARAM)&mi);
	ReleaseIconEx(mi.hIcon);
	
	mi.hIcon = LoadIconEx(I_ADD);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuAdd, (LPARAM)&mi);
	ReleaseIconEx(mi.hIcon);
	
	mi.hIcon = LoadIconEx(I_EDIT);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuEdit, (LPARAM)&mi);
	ReleaseIconEx(mi.hIcon);

	mi.hIcon = LoadIconEx(I_SETDEFAULT);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDefault, (LPARAM)&mi);
	ReleaseIconEx(mi.hIcon);

	mi.hIcon = LoadIconEx(I_REMOVE);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDelete, (LPARAM)&mi);
	ReleaseIconEx(mi.hIcon);

	return 0;
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
		Skin_AddIcon(&sid);
	}

	HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);

	ReloadIcons(0, 0); 
}

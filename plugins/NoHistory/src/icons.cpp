#include "common.h"
#include "icons.h"
#include "resource.h"

HICON hIconRemove, hIconKeep, hIconClear;
HANDLE hIcoLibIconsChanged = 0;

int ReloadIcons(WPARAM wParam, LPARAM lParam) {
	hIconRemove	= Skin_GetIcon(MODULE "_remove");
	hIconKeep	= Skin_GetIcon(MODULE "_keep");
	hIconClear	= Skin_GetIcon(MODULE "_clear");

	return 0;
}

void InitIcons()
{
	SKINICONDESC sid = { sizeof(sid) };

	sid.pszSection = MODULE;
	sid.flags = SIDF_PATH_TCHAR;

	TCHAR path[MAX_PATH];
	GetModuleFileName(hInst,path,MAX_PATH);
	sid.ptszDefaultFile = path;

	sid.pszDescription = LPGEN("Disable");
	sid.pszName = MODULE "_remove";
	sid.iDefaultIndex = -IDI_HREMOVE;
	Skin_AddIcon(&sid);

	sid.pszDescription = LPGEN("Enable");
	sid.pszName = MODULE "_keep";
	sid.iDefaultIndex = -IDI_HKEEP;
	Skin_AddIcon(&sid);

	sid.pszDescription = LPGEN("Clear");
	sid.pszName = MODULE "_clear";
	sid.iDefaultIndex = -IDI_HCLEAR;
	Skin_AddIcon(&sid);

	ReloadIcons(0, 0);

	hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);
}

void DeinitIcons() {
	if(hIcoLibIconsChanged)
		UnhookEvent(hIcoLibIconsChanged);
}

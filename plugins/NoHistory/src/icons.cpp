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

#define AddIcon(x,y,z) \
	sid.pszDescription = x; \
	sid.pszName = y; \
	sid.iDefaultIndex = -z; \
	Skin_AddIcon(&sid); \
	sid.iDefaultIndex++;																		

	AddIcon( LPGEN("Disable"), MODULE "_remove", IDI_HREMOVE);
	AddIcon( LPGEN("Enable"), MODULE "_keep", IDI_HKEEP);
	AddIcon( LPGEN("Clear"), MODULE "_clear", IDI_HCLEAR);

#undef	AddIcon

	ReloadIcons(0, 0);

	hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);
}

void DeinitIcons() {
	if(hIcoLibIconsChanged)
		UnhookEvent(hIcoLibIconsChanged);
}

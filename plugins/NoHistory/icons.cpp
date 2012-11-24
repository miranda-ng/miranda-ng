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

void InitIcons() {
		SKINICONDESC sid = {0};
		sid.cbSize = sizeof(SKINICONDESC);

		sid.ptszSection = _T(MODULE);
		sid.flags  = SIDF_ALL_TCHAR;

		TCHAR path[MAX_PATH]; 
		GetModuleFileName(hInst,path,MAX_PATH);

#define AddIcon(x,y,z)																				\
		sid.ptszDescription = x;															\
		sid.pszName = y;																			\
		sid.ptszDefaultFile = path;															\
		sid.hDefaultIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(z), IMAGE_ICON, 0, 0, 0);		\
		Skin_AddIcon(&sid);												\
		sid.iDefaultIndex++;																		

		AddIcon(LPGENT("Disable"), MODULE "_remove", IDI_HREMOVE);
		AddIcon(LPGENT("Enable"), MODULE "_keep", IDI_HKEEP);
		AddIcon(LPGENT("Clear"), MODULE "_clear", IDI_HCLEAR);

#undef	AddIcon

		ReloadIcons(0, 0);

		hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);

}

void DeinitIcons() {
	if(hIcoLibIconsChanged)
		UnhookEvent(hIcoLibIconsChanged);
}

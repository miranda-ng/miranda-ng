#include "common.h"
#include "icons.h"
#include "resource.h"

HICON hIconRemove, hIconKeep, hIconClear;
HANDLE hIcoLibIconsChanged = 0;

int ReloadIcons(WPARAM wParam, LPARAM lParam) {
	hIconRemove = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)MODULE "_remove");
	hIconKeep = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)MODULE "_keep");
	hIconClear = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)MODULE "_clear");

	return 0;
}

void InitIcons() {
	if(ServiceExists(MS_SKIN2_ADDICON)) {
		SKINICONDESC sid = {0};
		sid.cbSize = sizeof(SKINICONDESC);

		sid.pszSection = MODULE;

#define AddIcon(x,y,z)																				\
		sid.pszDescription = Translate(x);															\
		sid.pszName = y;																			\
		sid.pszDefaultFile = MODULE ".dll";															\
		sid.hDefaultIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(z), IMAGE_ICON, 0, 0, 0);		\
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);												\
		sid.iDefaultIndex++;																		

		AddIcon("Disable", MODULE "_remove", IDI_HREMOVE);
		AddIcon("Enable", MODULE "_keep", IDI_HKEEP);
		AddIcon("Clear", MODULE "_clear", IDI_HCLEAR);

#undef	AddIcon

		ReloadIcons(0, 0);

		hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);
	} else {
		hIconRemove = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_HREMOVE), IMAGE_ICON, 0, 0, 0);//LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS );
		hIconKeep = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_HKEEP), IMAGE_ICON, 0, 0, 0);//LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS );
		hIconClear = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_HCLEAR), IMAGE_ICON, 0, 0, 0);//LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS );
	}

}

void DeinitIcons() {
	if(hIcoLibIconsChanged) UnhookEvent(hIcoLibIconsChanged);
	else {
		DestroyIcon(hIconRemove);
		DestroyIcon(hIconKeep);
		DestroyIcon(hIconClear);
	}
}

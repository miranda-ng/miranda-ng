#include "common.h"
#include "icons.h"

HANDLE hIcoLibIconsChanged;

HICON hIconMenuSet, hIconList1, hIconList2, hIconMenuShowHide, hIconSystray;

int ReloadIcons(WPARAM wParam, LPARAM lParam) {
	hIconMenuSet = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"alarms_menu_set");
	hIconList1 = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"alarms_list1");
	hIconList2 = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"alarms_list2");
	if(!ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
		hIconMenuShowHide = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"alarms_menu_showhide");

	RefreshReminderFrame();
	return 0;
}

void InitIcons() {
	if(ServiceExists(MS_SKIN2_ADDICON)) {
		SKINICONDESC sid = {0};

		sid.cbSize = sizeof(SKINICONDESC);
		sid.pszSection = Translate("Alarms");

		sid.pszDescription = Translate("Menu: Set Alarm");
		sid.pszName = "alarms_menu_set";
		sid.pszDefaultFile = "alarms.dll";
		sid.iDefaultIndex = 0;
		sid.hDefaultIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MAINMENU), IMAGE_ICON, 16, 16, 0);
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Reminder: Soon");
		sid.pszName = "alarms_list1";
		sid.pszDefaultFile = "alarms.dll";
		sid.iDefaultIndex = 1;
		sid.hDefaultIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_LIST1), IMAGE_ICON, 16, 16, 0);//LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS );
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Reminder: Very Soon");
		sid.pszName = "alarms_list2";
		sid.pszDefaultFile = "alarms.dll";
		sid.iDefaultIndex = 2;
		sid.hDefaultIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_LIST2), IMAGE_ICON, 16, 16, 0);//LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS );
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Alarm: System Tray");
		sid.pszName = "alarms_systray";
		sid.pszDefaultFile = "alarms.dll";
		sid.iDefaultIndex = 3;
		sid.hDefaultIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MAINMENU), IMAGE_ICON, 16, 16, 0);//LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS );
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		if(!ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
			sid.pszDescription = Translate("Menu: Show/Hide Reminders");
			sid.pszName = "alarms_menu_showhide";
			sid.pszDefaultFile = "alarms.dll";
			sid.iDefaultIndex = 4;
			sid.hDefaultIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MAINMENU), IMAGE_ICON, 16, 16, 0);
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

			hIconMenuShowHide = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"alarms_menu_showhide");
		}

		hIconMenuSet = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"alarms_menu_set");
		hIconList1 = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"alarms_list1");
		hIconList2 = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"alarms_list2");
		hIconSystray = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"alarms_systray");

		hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);
	} else {
		hIconMenuSet = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MAINMENU), IMAGE_ICON, 16, 16, 0);//LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS );
		hIconList1 = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_LIST1), IMAGE_ICON, 16, 16, 0);//LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS );
		hIconList2 = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_LIST2), IMAGE_ICON, 16, 16, 0);//LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS );
		hIconSystray = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MAINMENU), IMAGE_ICON, 16, 16, 0);//LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS );
		if(!ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
			hIconMenuShowHide = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MAINMENU), IMAGE_ICON, 16, 16, 0);//LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS );
	}
}


void DeinitIcons() {
	if(!ServiceExists(MS_SKIN2_ADDICON)) {
		DestroyIcon(hIconMenuSet);
		DestroyIcon(hIconList1);
		DestroyIcon(hIconList2);
		DestroyIcon(hIconSystray);
		if(!ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
			DestroyIcon(hIconMenuShowHide);
	}
}

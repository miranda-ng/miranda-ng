#include "common.h"
#include "icons.h"

HICON hIconMenuSet, hIconList1, hIconList2, hIconMenuShowHide, hIconSystray;

int ReloadIcons(WPARAM wParam, LPARAM lParam)
{
	hIconMenuSet = Skin_GetIcon("alarms_menu_set");
	hIconList1 = Skin_GetIcon("alarms_list1");
	hIconList2 = Skin_GetIcon("alarms_list2");
	if ( !ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
		hIconMenuShowHide = Skin_GetIcon("alarms_menu_showhide");

	RefreshReminderFrame();
	return 0;
}

void InitIcons()
{
	TCHAR path[MAX_PATH];
	GetModuleFileName(hInst, path, MAX_PATH);

	SKINICONDESC sid = { sizeof(sid) };
	sid.pszSection = "Alarms";
	sid.ptszDefaultFile = path;
	sid.flags = SIDF_PATH_TCHAR;

	sid.pszDescription = LPGEN("Menu: Set Alarm");
	sid.pszName = "alarms_menu_set";
	sid.pszDefaultFile = "alarms.dll";
	sid.iDefaultIndex = -IDI_MAINMENU;
	Skin_AddIcon(&sid);

	sid.pszDescription = LPGEN("Reminder: Soon");
	sid.pszName = "alarms_list1";
	sid.pszDefaultFile = "alarms.dll";
	sid.iDefaultIndex = -IDI_LIST1;

	sid.pszDescription = LPGEN("Reminder: Very Soon");
	sid.pszName = "alarms_list2";
	sid.pszDefaultFile = "alarms.dll";
	sid.iDefaultIndex = -IDI_LIST2;
	Skin_AddIcon(&sid);

	sid.pszDescription = LPGEN("Alarm: System Tray");
	sid.pszName = "alarms_systray";
	sid.pszDefaultFile = "alarms.dll";
	sid.iDefaultIndex = -IDI_MAINMENU;
	Skin_AddIcon(&sid);

	if ( !ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		sid.pszDescription = LPGEN("Menu: Show/Hide Reminders");
		sid.pszName = "alarms_menu_showhide";
		sid.pszDefaultFile = "alarms.dll";
		sid.iDefaultIndex = -IDI_MAINMENU;
		Skin_AddIcon(&sid);

		hIconMenuShowHide = Skin_GetIcon("alarms_menu_showhide");
	}

	hIconMenuSet = Skin_GetIcon("alarms_menu_set");
	hIconList1 = Skin_GetIcon("alarms_list1");
	hIconList2 = Skin_GetIcon("alarms_list2");
	hIconSystray = Skin_GetIcon("alarms_systray");

	HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);
}

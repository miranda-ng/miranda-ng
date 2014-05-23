
#include "common.h"

#define TTBI_GROUPSHOWHIDE		"TTBInternal/GroupShowHide"
#define TTBI_SOUNDSONOFF		"TTBInternal/SoundsOnOFF"
#define TTBI_MAINMENUBUTT		"TTBInternal/MainMenuBUTT"

static HWND hwndContactTree;

struct {
	char *name, *pszService;
	int iconidUp, iconidDn;
	char *tooltipUp, *tooltipDn;
	HANDLE hButton;
}
static stdButtons[] = {
	{ "Show only online users", MS_CLIST_SETHIDEOFFLINE,  IDI_SHOWONLINEUP, IDI_SHOWONLINEDN, "Hide offline users", "Show all users" },
	{ "Groups on/off",          TTBI_GROUPSHOWHIDE,       IDI_GROUPSUP,     IDI_GROUPSDN,     "Hide groups",        "Show groups"    },
	{ "Sounds enable/disable",  TTBI_SOUNDSONOFF,         IDI_SOUNDUP,      IDI_SOUNDDN,      "Disable sounds",     "Enable sounds"  },

	{ "Show options page",      "Options/OptionsCommand", IDI_OPTIONSUP,    0,                "Show options"   },
	{ "Minimize button",        MS_CLIST_SHOWHIDE,        IDI_MINIMIZEUP,   0,                NULL             },
	{ "Find/Add contacts",      MS_FINDADD_FINDADD,       IDI_FINDADDUP,    0,                NULL             },
	{ "Show main menu",         TTBI_MAINMENUBUTT,        IDI_MIRANDAUP,    0,                "Show main menu" }
};

///////////////////////////////////////////////////////////////////////////////

int OnSettingChanging(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING *)lParam;
	if (hContact != 0 || dbcws == NULL)
		return 0;

	if (!strcmp(dbcws->szModule, "CList")) {
		if (!strcmp(dbcws->szSetting, "HideOffline"))
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)stdButtons[0].hButton, (dbcws->value.bVal) ? TTBST_PUSHED : TTBST_RELEASED);

		else if (!strcmp(dbcws->szSetting, "UseGroups"))
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)stdButtons[1].hButton, (dbcws->value.bVal) ? TTBST_PUSHED : TTBST_RELEASED);
	}

	else if (!strcmp(dbcws->szModule, "Skin")) {
		if (!strcmp(dbcws->szSetting, "UseSound"))
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)stdButtons[2].hButton, (dbcws->value.bVal) ? TTBST_RELEASED : TTBST_PUSHED);
	}

	return 0;
}

INT_PTR TTBInternalMainMenuButt(WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN, 0, 0);

	POINT pt;
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), NULL);
	return 0;
}

INT_PTR TTBInternalGroupShowHide(WPARAM wParam, LPARAM lParam)
{
	int newVal = !(GetWindowLongPtr(hwndContactTree, GWL_STYLE) & CLS_USEGROUPS);
	db_set_b(NULL, "CList", "UseGroups", (BYTE)newVal);
	SendMessage(hwndContactTree, CLM_SETUSEGROUPS, newVal, 0);
	return 0;
}

INT_PTR TTBInternalSoundsOnOff(WPARAM wParam, LPARAM lParam)
{
	int newVal = !(db_get_b(NULL, "Skin", "UseSound", 1));
	db_set_b(NULL, "Skin", "UseSound", (BYTE)newVal);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void InitInternalButtons()
{
	hwndContactTree = (HWND)CallService(MS_CLUI_GETHWNDTREE, 0, 0);

	CreateServiceFunction(TTBI_GROUPSHOWHIDE, TTBInternalGroupShowHide);
	CreateServiceFunction(TTBI_SOUNDSONOFF, TTBInternalSoundsOnOff);
	CreateServiceFunction(TTBI_MAINMENUBUTT, TTBInternalMainMenuButt);

	for (int i = 0; i < SIZEOF(stdButtons); i++) {
		TTBButton ttb = { 0 };
		ttb.cbSize = sizeof(ttb);
		ttb.name = stdButtons[i].name;
		ttb.pszService = stdButtons[i].pszService;
		ttb.dwFlags = TTBBF_VISIBLE | TTBBF_INTERNAL;
		if ((ttb.pszTooltipDn = stdButtons[i].tooltipDn) != NULL)
			ttb.dwFlags |= TTBBF_SHOWTOOLTIP;
		ttb.pszTooltipUp = stdButtons[i].tooltipUp;
		ttb.hIconUp = (HICON)LoadImage(hInst, MAKEINTRESOURCE(stdButtons[i].iconidUp), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		if (stdButtons[i].iconidDn) {
			ttb.dwFlags |= TTBBF_ASPUSHBUTTON;
			ttb.hIconDn = (HICON)LoadImage(hInst, MAKEINTRESOURCE(stdButtons[i].iconidDn), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		}
		else
			ttb.hIconDn = NULL;

		if (i == 0)
			ttb.wParamUp = 1;

		stdButtons[i].hButton = (HANDLE)TTBAddButton((WPARAM)&ttb, 0);
	}

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)stdButtons[0].hButton,
		db_get_b(NULL, "CList", "HideOffline", 0) ? TTBST_PUSHED : TTBST_RELEASED);

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)stdButtons[1].hButton,
		db_get_b(NULL, "CList", "UseGroups", 1) ? TTBST_PUSHED : TTBST_RELEASED);

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)stdButtons[2].hButton,
		db_get_b(NULL, "Skin", "UseSound", 1) ? TTBST_RELEASED : TTBST_PUSHED);

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnSettingChanging);
}

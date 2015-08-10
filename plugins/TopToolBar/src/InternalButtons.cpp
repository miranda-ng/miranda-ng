
#include "stdafx.h"

#define TTBI_GROUPSHOWHIDE		"TTBInternal/GroupShowHide"
#define TTBI_SOUNDSONOFF		"TTBInternal/SoundsOnOFF"
#define TTBI_MAINMENUBUTT		"TTBInternal/MainMenuBUTT"
#define TTBI_STATUSMENUBUTT		"TTBInternal/StatusMenuButt"
#define TTBI_SHOWHIDEOFFLINE	"TTBInternal/ShowHideOffline"

#define INDEX_OFFLINE			5
#define INDEX_META				6
#define INDEX_GROUPS			7
#define INDEX_SOUNDS			8

static HWND hwndContactTree;

struct {
	char *name, *pszService;
	int iconidUp, iconidDn;
	char *tooltipUp, *tooltipDn;
	bool bCustomIcon, bDefVisible;
	HANDLE hButton;
}
static stdButtons[] = {
	{ LPGEN("Show main menu"),              TTBI_MAINMENUBUTT,           SKINICON_OTHER_MAINMENU, 0,               LPGEN("Show main menu"),        NULL, 0, 1 },
	{ LPGEN("Show options page"),           "Options/OptionsCommand",    SKINICON_OTHER_OPTIONS,  0,               LPGEN("Show options page"),     NULL, 0, 1 },
	{ LPGEN("Show accounts manager"),       "Protos/ShowAccountManager", SKINICON_OTHER_ACCMGR,   0,               LPGEN("Show accounts manager"), NULL, 0, 0 },
	{ LPGEN("Find/add contacts"),           MS_FINDADD_FINDADD,          SKINICON_OTHER_FINDUSER, 0,               LPGEN("Find/add contacts"),     NULL, 0, 1 },
	{ LPGEN("Show status menu"),            TTBI_STATUSMENUBUTT,         SKINICON_OTHER_STATUS,   0,               LPGEN("Show status menu"),      NULL, 0, 0 },
	{ LPGEN("Show/hide offline contacts"),  TTBI_SHOWHIDEOFFLINE,        IDI_HIDEOFFLINE,         IDI_SHOWOFFLINE, LPGEN("Hide offline contacts"), LPGEN("Show offline contacts"), 1, 1 },
	{ LPGEN("Enable/disable metacontacts"), "MetaContacts/OnOff",        IDI_METAOFF,             IDI_METAON,      LPGEN("Disable metacontacts"),  LPGEN("Enable metacontacts"), 1, 1 },
	{ LPGEN("Enable/disable groups"),       TTBI_GROUPSHOWHIDE,          IDI_GROUPSOFF,           IDI_GROUPSON,    LPGEN("Enable groups"),         LPGEN("Disable groups"), 1, 1 },
	{ LPGEN("Enable/disable sounds"),       TTBI_SOUNDSONOFF,            IDI_SOUNDSOFF,           IDI_SOUNDSON,    LPGEN("Disable sounds"),        LPGEN("Enable sounds"), 1, 1 },
	{ LPGEN("Minimize contact list"),       MS_CLIST_SHOWHIDE,           SKINICON_OTHER_SHOWHIDE, 0,               LPGEN("Minimize contact list"), NULL, 0, 1 },
	{ LPGEN("Exit"),                        "CloseAction",               SKINICON_OTHER_EXIT,     0,               LPGEN("Exit"),                  NULL, 0, 0 }
};

///////////////////////////////////////////////////////////////////////////////

int OnSettingChanging(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING *)lParam;
	if (hContact != 0 || dbcws == NULL)
		return 0;

	if (!mir_strcmp(dbcws->szModule, "CList")) {
		if (!mir_strcmp(dbcws->szSetting, "HideOffline"))
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)stdButtons[INDEX_OFFLINE].hButton, dbcws->value.bVal ? 0 : TTBST_PUSHED);
		else if (!mir_strcmp(dbcws->szSetting, "UseGroups"))
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)stdButtons[INDEX_GROUPS].hButton, dbcws->value.bVal ? TTBST_PUSHED : 0);
	}
	else if (!mir_strcmp(dbcws->szModule, "Skin")) {
		if (!mir_strcmp(dbcws->szSetting, "UseSound"))
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)stdButtons[INDEX_SOUNDS].hButton, dbcws->value.bVal ? TTBST_PUSHED : 0);
	}
	else if (!mir_strcmp(dbcws->szModule, "MetaContacts")) {
		if (!mir_strcmp(dbcws->szSetting, "Enabled"))
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)stdButtons[INDEX_META].hButton, dbcws->value.bVal ? TTBST_PUSHED : 0);
	}

	return 0;
}

INT_PTR TTBInternalMainMenuButt(WPARAM, LPARAM)
{
	POINT pt;
	GetCursorPos(&pt);
	TrackPopupMenu(Menu_GetMainMenu(), TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, pcli->hwndContactList, NULL);
	return 0;
}

INT_PTR TTBInternalStatusMenuButt(WPARAM, LPARAM)
{
	POINT pt;
	GetCursorPos(&pt);
	TrackPopupMenu(Menu_GetStatusMenu(), TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, pcli->hwndContactList, NULL);
	return 0;
}

INT_PTR TTBInternalGroupShowHide(WPARAM, LPARAM)
{
	int newVal = !(GetWindowLongPtr(hwndContactTree, GWL_STYLE) & CLS_USEGROUPS);
	db_set_b(NULL, "CList", "UseGroups", (BYTE)newVal);
	SendMessage(hwndContactTree, CLM_SETUSEGROUPS, newVal, 0);
	return 0;
}

INT_PTR TTBInternalSoundsOnOff(WPARAM, LPARAM)
{
	int newVal = !(db_get_b(NULL, "Skin", "UseSound", 1));
	db_set_b(NULL, "Skin", "UseSound", (BYTE)newVal);
	return 0;
}

INT_PTR TTBInternalShowHideOffline(WPARAM, LPARAM)
{
	CallService(MS_CLIST_SETHIDEOFFLINE, -1, 0);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void InitInternalButtons()
{
	hwndContactTree = pcli->hwndContactTree;

	CreateServiceFunction(TTBI_GROUPSHOWHIDE, TTBInternalGroupShowHide);
	CreateServiceFunction(TTBI_SOUNDSONOFF, TTBInternalSoundsOnOff);
	CreateServiceFunction(TTBI_MAINMENUBUTT, TTBInternalMainMenuButt);
	CreateServiceFunction(TTBI_STATUSMENUBUTT, TTBInternalStatusMenuButt);
	CreateServiceFunction(TTBI_SHOWHIDEOFFLINE, TTBInternalShowHideOffline);

	for (int i = 0; i < _countof(stdButtons); i++) {
		TTBButton ttb = { 0 };
		ttb.name = stdButtons[i].name;
		ttb.pszService = stdButtons[i].pszService;
		ttb.dwFlags = TTBBF_INTERNAL;
		if (stdButtons[i].bDefVisible == TRUE)
			ttb.dwFlags |= TTBBF_VISIBLE;
		if ((ttb.pszTooltipDn = stdButtons[i].tooltipDn) != NULL)
			ttb.dwFlags |= TTBBF_SHOWTOOLTIP;
		ttb.pszTooltipUp = stdButtons[i].tooltipUp;
		if (stdButtons[i].bCustomIcon) {
			ttb.hIconUp = (HICON)LoadImage(hInst, MAKEINTRESOURCE(stdButtons[i].iconidUp), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			if (stdButtons[i].iconidDn) {
				ttb.dwFlags |= TTBBF_ASPUSHBUTTON;
				ttb.hIconDn = (HICON)LoadImage(hInst, MAKEINTRESOURCE(stdButtons[i].iconidDn), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			}
			else
				ttb.hIconDn = NULL;
		}
		else {
			ttb.hIconHandleUp = Skin_GetIconHandle(stdButtons[i].iconidUp);
			if (stdButtons[i].iconidDn) {
				ttb.dwFlags |= TTBBF_ASPUSHBUTTON;
				ttb.hIconHandleDn = Skin_GetIconHandle(stdButtons[i].iconidDn);
			}
			else
				ttb.hIconHandleDn = ttb.hIconDn = NULL;
		}

		if (i == 0)
			ttb.wParamUp = 1;

		stdButtons[i].hButton = (HANDLE)TTBAddButton((WPARAM)&ttb, 0);
	}

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)stdButtons[INDEX_OFFLINE].hButton,
		db_get_b(NULL, "CList", "HideOffline", 0) ? 0 : TTBST_PUSHED);

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)stdButtons[INDEX_GROUPS].hButton,
		db_get_b(NULL, "CList", "UseGroups", 1) ? TTBST_PUSHED : 0);

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)stdButtons[INDEX_SOUNDS].hButton,
		db_get_b(NULL, "Skin", "UseSound", 1) ? TTBST_PUSHED : 0);

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)stdButtons[INDEX_META].hButton,
		db_get_b(NULL, "MetaContacts", "Enabled", 1) ? TTBST_PUSHED : 0);

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnSettingChanging);
}

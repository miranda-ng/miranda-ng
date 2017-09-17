
#include "stdafx.h"

#define TTBI_SOUNDSONOFF      "TTBInternal/SoundsOnOFF"
#define TTBI_MAINMENUBUTT     "TTBInternal/MainMenuBUTT"
#define TTBI_STATUSMENUBUTT   "TTBInternal/StatusMenuButt"

static HWND hwndContactTree;
HANDLE hMainMenu, hOptions, hAccManager, hFindAdd, hStatusMenu, hShowHideOffline, hMetaContacts, hGroups, hSounds, hMinimize, hExit;

///////////////////////////////////////////////////////////////////////////////

int OnSettingChanging(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING *)lParam;
	if (hContact != 0 || dbcws == NULL)
		return 0;

	if (!strcmp(dbcws->szModule, "CList")) {
		if (!strcmp(dbcws->szSetting, "HideOffline"))
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hShowHideOffline, dbcws->value.bVal ? 0 : TTBST_PUSHED);
		else if (!strcmp(dbcws->szSetting, "UseGroups"))
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hGroups, dbcws->value.bVal ? TTBST_PUSHED : 0);
	}
	else if (!strcmp(dbcws->szModule, "Skin")) {
		if (!strcmp(dbcws->szSetting, "UseSound"))
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hSounds, dbcws->value.bVal ? TTBST_PUSHED : 0);
	}
	else if (!strcmp(dbcws->szModule, "MetaContacts")) {
		if (!strcmp(dbcws->szSetting, "Enabled"))
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hMetaContacts, dbcws->value.bVal ? TTBST_PUSHED : 0);
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

INT_PTR TTBInternalSoundsOnOff(WPARAM, LPARAM)
{
	int newVal = !(db_get_b(NULL, "Skin", "UseSound", 1));
	db_set_b(NULL, "Skin", "UseSound", (BYTE)newVal);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void InitInternalButtons()
{
	hwndContactTree = pcli->hwndContactTree;

	CreateServiceFunction(TTBI_SOUNDSONOFF, TTBInternalSoundsOnOff);
	CreateServiceFunction(TTBI_MAINMENUBUTT, TTBInternalMainMenuButt);
	CreateServiceFunction(TTBI_STATUSMENUBUTT, TTBInternalStatusMenuButt);

	TTBButton ttb = { 0 };
	ttb.name = LPGEN("Show main menu");
	ttb.pszService = TTBI_MAINMENUBUTT;
	ttb.dwFlags = TTBBF_INTERNAL | TTBBF_VISIBLE;
	ttb.pszTooltipUp = LPGEN("Show main menu");
	ttb.hIconHandleUp = Skin_GetIconHandle(SKINICON_OTHER_MAINMENU);
	ttb.wParamUp = 1;
	hMainMenu = TopToolbar_AddButton(&ttb);

	ttb.name = LPGEN("Show options page");
	ttb.pszService = "Options/OptionsCommand";
	ttb.dwFlags = TTBBF_INTERNAL | TTBBF_VISIBLE;
	ttb.pszTooltipUp = LPGEN("Show options page");
	ttb.hIconHandleUp = Skin_GetIconHandle(SKINICON_OTHER_OPTIONS);
	ttb.wParamUp = 0;
	hOptions = TopToolbar_AddButton(&ttb);

	ttb.name = LPGEN("Show accounts manager");
	ttb.pszService = "Protos/ShowAccountManager";
	ttb.dwFlags = TTBBF_INTERNAL;
	ttb.pszTooltipUp = LPGEN("Show accounts manager");
	ttb.hIconHandleUp = Skin_GetIconHandle(SKINICON_OTHER_ACCMGR);
	hAccManager = TopToolbar_AddButton(&ttb);

	ttb.name = LPGEN("Find/add contacts");
	ttb.pszService = MS_FINDADD_FINDADD;
	ttb.dwFlags = TTBBF_INTERNAL | TTBBF_VISIBLE;
	ttb.pszTooltipUp = LPGEN("Find/add contacts");
	ttb.hIconHandleUp = Skin_GetIconHandle(SKINICON_OTHER_FINDUSER);
	hFindAdd = TopToolbar_AddButton(&ttb);

	ttb.name = LPGEN("Show status menu");
	ttb.pszService = TTBI_STATUSMENUBUTT;
	ttb.dwFlags = TTBBF_INTERNAL;
	ttb.pszTooltipUp = LPGEN("Show status menu");
	ttb.hIconHandleUp = Skin_GetIconHandle(SKINICON_OTHER_STATUS);
	hStatusMenu = TopToolbar_AddButton(&ttb);

	ttb.name = LPGEN("Show/hide offline contacts");
	ttb.pszService = MS_CLIST_TOGGLEHIDEOFFLINE;
	ttb.dwFlags = TTBBF_INTERNAL | TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | TTBBF_ASPUSHBUTTON;
	ttb.pszTooltipDn = LPGEN("Show offline contacts");
	ttb.pszTooltipUp = LPGEN("Hide offline contacts");
	ttb.hIconHandleDn = ttb.hIconHandleUp = NULL;
	ttb.hIconUp = IcoLib_GetIconByHandle(iconList[1].hIcolib);
	ttb.hIconDn = IcoLib_GetIconByHandle(iconList[2].hIcolib);
	hShowHideOffline = TopToolbar_AddButton(&ttb);

	ttb.name = LPGEN("Enable/disable metacontacts");
	ttb.pszService = "MetaContacts/OnOff";
	ttb.dwFlags = TTBBF_INTERNAL | TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | TTBBF_ASPUSHBUTTON;
	ttb.pszTooltipDn = LPGEN("Enable metacontacts");
	ttb.pszTooltipUp = LPGEN("Disable metacontacts");
	ttb.hIconUp = IcoLib_GetIconByHandle(iconList[7].hIcolib);
	ttb.hIconDn = IcoLib_GetIconByHandle(iconList[8].hIcolib);
	hMetaContacts = TopToolbar_AddButton(&ttb);

	ttb.name = LPGEN("Enable/disable groups");
	ttb.pszService = MS_CLIST_TOGGLEGROUPS;
	ttb.dwFlags = TTBBF_INTERNAL | TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | TTBBF_ASPUSHBUTTON;
	ttb.pszTooltipDn = LPGEN("Disable groups");
	ttb.pszTooltipUp = LPGEN("Enable groups");
	ttb.hIconUp = IcoLib_GetIconByHandle(iconList[3].hIcolib);
	ttb.hIconDn = IcoLib_GetIconByHandle(iconList[4].hIcolib);
	hGroups = TopToolbar_AddButton(&ttb);

	ttb.name = LPGEN("Enable/disable sounds");
	ttb.pszService = TTBI_SOUNDSONOFF;
	ttb.dwFlags = TTBBF_INTERNAL | TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | TTBBF_ASPUSHBUTTON;
	ttb.pszTooltipDn = LPGEN("Enable sounds");
	ttb.pszTooltipUp = LPGEN("Disable sounds");
	ttb.hIconUp = IcoLib_GetIconByHandle(iconList[5].hIcolib);
	ttb.hIconDn = IcoLib_GetIconByHandle(iconList[6].hIcolib);
	hSounds = TopToolbar_AddButton(&ttb);

	ttb.name = LPGEN("Minimize contact list");
	ttb.pszService = "Clist/ShowHide";
	ttb.dwFlags = TTBBF_INTERNAL | TTBBF_VISIBLE;
	ttb.pszTooltipUp = LPGEN("Minimize contact list");
	ttb.hIconHandleUp = Skin_GetIconHandle(SKINICON_OTHER_SHOWHIDE);
	ttb.pszTooltipDn = NULL;
	ttb.hIconUp = ttb.hIconDn = NULL;
	hMinimize = TopToolbar_AddButton(&ttb);

	ttb.name = LPGEN("Exit");
	ttb.pszService = "CloseAction";
	ttb.dwFlags = TTBBF_INTERNAL;
	ttb.pszTooltipUp = LPGEN("Exit");
	ttb.hIconHandleUp = Skin_GetIconHandle(SKINICON_OTHER_EXIT);
	hExit = TopToolbar_AddButton(&ttb);

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hShowHideOffline,
		db_get_b(NULL, "CList", "HideOffline", 0) ? 0 : TTBST_PUSHED);

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hGroups,
		db_get_b(NULL, "CList", "UseGroups", 1) ? TTBST_PUSHED : 0);

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hSounds,
		db_get_b(NULL, "Skin", "UseSound", 1) ? TTBST_PUSHED : 0);

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hMetaContacts,
		db_get_b(NULL, "MetaContacts", "Enabled", 1) ? TTBST_PUSHED : 0);

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnSettingChanging);
}

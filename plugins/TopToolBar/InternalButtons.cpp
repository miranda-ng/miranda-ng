
#include "common.h"

#define TTBI_GROUPSHOWHIDE  "TTBInternal/GroupShowHide"
#define TTBI_SOUNDSONOFF    "TTBInternal/SoundsOnOFF"
#define TTBI_MAINMENUBUTT   "TTBInternal/MainMenuBUTT"

int LoadInternalButtons( HWND );
int UnLoadInternalButtons();

extern HINSTANCE hInst;

static HANDLE hOnlineBut, hGroupBut, hSoundsBut, hOptionsBut, hMainMenuBut;
static HANDLE hMinimizeBut;
static HANDLE hFindUsers;

static HWND hwndContactTree;

int OnSettingChanging(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0)
		return 0;
	
	DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING *)lParam;
	if (dbcws == NULL)
		return 0;

	if ( !strcmp(dbcws->szModule, "CList")) {
		if ( !strcmp(dbcws->szSetting, "HideOffline")) {
			int val = dbcws->value.bVal;

			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hOnlineBut, (LPARAM)(val)?TTBST_PUSHED:TTBST_RELEASED);		

			CallService(MS_TTB_SETBUTTONOPTIONS, MAKEWPARAM(TTBO_TIPNAME, hOnlineBut), 
				(LPARAM)((!val)?Translate("Hide Offline Users"):Translate("Show All Users")));

			return 0;
		}

		if (!strcmp(dbcws->szSetting, "UseGroups")) {
			int val = dbcws->value.bVal;
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hGroupBut, (LPARAM)((val)?TTBST_PUSHED:TTBST_RELEASED));
			return 0;
		}
	}

	if (!strcmp(dbcws->szModule, "Skin")) {
		if (!strcmp(dbcws->szSetting, "UseSound")) {
			int val = dbcws->value.bVal;
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hSoundsBut, (LPARAM)(val)?TTBST_RELEASED:TTBST_PUSHED);		
			return 0;
		}
	}

	return 0;
}

INT_PTR TTBInternalMainMenuButt(WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN, 0, 0);

	POINT pt;
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON, pt.x, pt.y, 0, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), NULL);	
	
	return 0;
}

INT_PTR TTBInternalGroupShowHide(WPARAM wParam, LPARAM lParam)
{
	int newVal = !(GetWindowLongPtr(hwndContactTree, GWL_STYLE)&CLS_USEGROUPS);
	DBWriteContactSettingByte(NULL, "CList", "UseGroups", (BYTE)newVal);
	SendMessage(hwndContactTree, CLM_SETUSEGROUPS, newVal, 0);
	return 0;
}

INT_PTR TTBInternalSoundsOnOff(WPARAM wParam, LPARAM lParam)
{
	int newVal = !(DBGetContactSettingByte(NULL, "Skin", "UseSound", 1));
	DBWriteContactSettingByte(NULL, "Skin", "UseSound", (BYTE)newVal);
	CallService(MS_TTB_SETBUTTONOPTIONS, MAKEWPARAM(TTBO_TIPNAME, hSoundsBut), 
		(LPARAM)((newVal)?Translate("Disable Sounds"):Translate("Enable Sounds")));
	return 0;
}

int UnLoadInternalButtons()
{
	return 0;
}

int LoadInternalButtons(HWND hwnd)
{
	hwndContactTree = hwnd;
	CreateServiceFunction(TTBI_GROUPSHOWHIDE, TTBInternalGroupShowHide);
	CreateServiceFunction(TTBI_SOUNDSONOFF, TTBInternalSoundsOnOff);

	CreateServiceFunction(TTBI_MAINMENUBUTT, TTBInternalMainMenuButt);

	int ShowOnline = DBGetContactSettingByte(NULL, "CList", "HideOffline", 0);
	int ShowGroups = DBGetContactSettingByte(NULL, "CList", "UseGroups", 2);
	int SoundsEnabled = DBGetContactSettingByte(NULL, "Skin", "UseSound", 1);

	TTBButton ttb = { 0 };
	ttb.cbSize = sizeof(ttb);
	ttb.hIconUp = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_SHOWONLINEUP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ttb.hIconDn = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_SHOWONLINEDN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ttb.dwFlags = (ShowOnline?TTBBF_PUSHED:0)|TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP|TTBBF_ASPUSHBUTTON|TTBBF_INTERNAL;
	ttb.pszService = MS_CLIST_SETHIDEOFFLINE;
	ttb.wParamUp = ttb.wParamDown = -1;
	ttb.name = "Show only Online Users";
	hOnlineBut = (HANDLE)TTBAddButton((WPARAM)&ttb, 0);

	ttb.wParamUp = ttb.wParamDown = 0;

	ttb.hIconUp = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_GROUPSUP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ttb.hIconDn = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_GROUPSDN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ttb.dwFlags = (ShowGroups?TTBBF_PUSHED:0)|TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP|TTBBF_ASPUSHBUTTON|TTBBF_INTERNAL;
	ttb.pszService = TTBI_GROUPSHOWHIDE;
	ttb.name = "Groups On/Off";
	hGroupBut = (HANDLE)TTBAddButton((WPARAM)&ttb, 0);

	ttb.hIconUp = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_SOUNDUP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ttb.hIconDn = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_SOUNDDN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ttb.dwFlags = (SoundsEnabled?0:TTBBF_PUSHED)|TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP|TTBBF_ASPUSHBUTTON|TTBBF_INTERNAL;
	ttb.pszService = TTBI_SOUNDSONOFF;
	ttb.name = "Sounds Enable/Disable";
	hSoundsBut = (HANDLE)TTBAddButton((WPARAM)&ttb, 0);

	ttb.hIconUp = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_OPTIONSUP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ttb.hIconDn = 0;	
	ttb.dwFlags = TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP|TTBBF_INTERNAL;
	ttb.pszService = "Options/OptionsCommand";
	ttb.name = "Show Options Page";
	hOptionsBut = (HANDLE)TTBAddButton((WPARAM)&ttb, 0);

	ttb.hIconUp = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MINIMIZEUP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ttb.hIconDn = 0;	
	ttb.dwFlags = TTBBF_VISIBLE|TTBBF_INTERNAL;
	ttb.pszService = MS_CLIST_SHOWHIDE;
	ttb.name = "Minimize Button";
	hMinimizeBut = (HANDLE)TTBAddButton((WPARAM)&ttb, 0);

	ttb.hIconUp = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_FINDADDUP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ttb.hIconDn = 0;	
	ttb.dwFlags = TTBBF_VISIBLE|TTBBF_INTERNAL;
	ttb.pszService = MS_FINDADD_FINDADD;
	ttb.name = "Find/Add Contacts";
	hFindUsers = (HANDLE)TTBAddButton((WPARAM)&ttb, 0);

	ttb.hIconUp = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MIRANDAUP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ttb.hIconDn = 0;	
	ttb.dwFlags = TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP|TTBBF_INTERNAL;
	ttb.pszService = TTBI_MAINMENUBUTT;
	ttb.name = "Show Main Menu";
	hMainMenuBut = (HANDLE)TTBAddButton((WPARAM)&ttb, 0);

	CallService(MS_TTB_SETBUTTONOPTIONS, MAKEWPARAM(TTBO_TIPNAME, hOnlineBut), 
		(LPARAM)((ShowOnline) ? "Hide Offline Users" : "Show All Users" ));

	CallService(MS_TTB_SETBUTTONOPTIONS, MAKEWPARAM(TTBO_TIPNAME, hGroupBut), 
		(LPARAM)((ShowGroups) ? "Hide Groups" : "Show Groups" ));

	CallService(MS_TTB_SETBUTTONOPTIONS, MAKEWPARAM(TTBO_TIPNAME, hSoundsBut), 
		(LPARAM)((SoundsEnabled) ? "Disable Sounds" : "Enable Sounds" ));

	CallService(MS_TTB_SETBUTTONOPTIONS, MAKEWPARAM(TTBO_TIPNAME, hOptionsBut), 
		(LPARAM)"Show Options");

	CallService(MS_TTB_SETBUTTONOPTIONS, MAKEWPARAM(TTBO_TIPNAME, hMainMenuBut), 
		(LPARAM)"Show Main Menu");

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED,OnSettingChanging);

 	return 0;
}

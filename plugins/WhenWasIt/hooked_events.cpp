/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006-2011 Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"
#include "hooked_events.h"

#define DATE_CHANGE_CHECK_INTERVAL 20

#define EXCLUDE_HIDDEN  1
#define EXCLUDE_IGNORED 2

#define HOST "http://eblis.tla.ro/projects"


#define WHENWASIT_DATA "WhenWasIt%20unicode"


#if defined(WIN64) || defined(_WIN64)
#define WHENWASIT_VERSION_URL HOST "/miranda/WhenWasIt/updater/x64/WhenWasIt.html"
#define WHENWASIT_UPDATE_URL HOST "/miranda/WhenWasIt/updater/x64/" WHENWASIT_DATA ".zip"
#else
#define WHENWASIT_VERSION_URL HOST "/miranda/WhenWasIt/updater/WhenWasIt.html"
#define WHENWASIT_UPDATE_URL HOST "/miranda/WhenWasIt/updater/" WHENWASIT_DATA ".zip"
#endif
#define WHENWASIT_VERSION_PREFIX "WhenWasIt version "

HANDLE hModulesLoaded;
HANDLE hOptionsInitialize;
HANDLE hIconsChanged;
HANDLE hExtraIconListRebuild;
HANDLE hExtraImageApply;
HANDLE hContactSettingChanged;
HANDLE hTopToolBarModuleLoaded;
//HANDLE hContactSendMessage;

HANDLE hmCheckBirthdays = NULL;
HANDLE hmBirthdayList = NULL;
HANDLE hmRefreshDetails = NULL;
HANDLE hmAddChangeBirthday = NULL;
HANDLE hmImportBirthdays = NULL;
HANDLE hmExportBirthdays = NULL;

UINT_PTR hCheckTimer = NULL;
UINT_PTR hDateChangeTimer = NULL;

int currentDay;

int HookEvents()
{
	Log("%s", "Entering function " __FUNCTION__);
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hOptionsInitialize = HookEvent(ME_OPT_INITIALISE, OnOptionsInitialise);
	Log("%s", "Leaving function " __FUNCTION__);
	
	return 0;
}

int UnhookEvents()
{
	Log("%s", "Entering function " __FUNCTION__);
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hOptionsInitialize);
	UnhookEvent(hIconsChanged);
	UnhookEvent(hExtraIconListRebuild);
	UnhookEvent(hExtraImageApply);
	UnhookEvent(hContactSettingChanged);
	UnhookEvent(hTopToolBarModuleLoaded);
	
	KillTimers();
	
	Log("%s", "Leaving function " __FUNCTION__);
	
	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	hIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, OnIconsChanged);
	hExtraIconListRebuild = HookEvent(ME_CLIST_EXTRA_LIST_REBUILD, OnExtraIconListRebuild);
	hExtraImageApply = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, OnExtraImageApply);
	hContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);
	hTopToolBarModuleLoaded = HookEvent(ME_TTB_MODULELOADED, OnTopToolBarModuleLoaded);
	
	SkinAddNewSoundEx(BIRTHDAY_NEAR_SOUND, LPGEN("WhenWasIt"), LPGEN("Birthday near"));
	SkinAddNewSoundEx(BIRTHDAY_TODAY_SOUND, LPGEN("WhenWasIt"), LPGEN("Birthday today"));
	
	RebuildAdvIconList();
	
	char buffer[1024];
	Update update = {0};
	update.cbSize = sizeof(Update);
	update.szComponentName = __PLUGIN_DISPLAY_NAME;
	update.pbVersion = (BYTE *) CreateVersionString(VERSION, buffer);
	update.cpbVersion = (int) strlen((char *) update.pbVersion);
	update.szUpdateURL = UPDATER_AUTOREGISTER;
	update.szBetaVersionURL = WHENWASIT_VERSION_URL;
	update.szBetaUpdateURL = WHENWASIT_UPDATE_URL;
	update.pbBetaVersionPrefix = (BYTE *) WHENWASIT_VERSION_PREFIX;
	update.cpbBetaVersionPrefix = (int) strlen(WHENWASIT_VERSION_PREFIX);
	CallService(MS_UPDATE_REGISTER, 0, (LPARAM) &update);

	UpdateTimers();
	CLISTMENUITEM cl = {0};
	cl.cbSize = sizeof(CLISTMENUITEM);
	cl.hIcon = hiCheckMenu;
	cl.position = 10000000;
	cl.pszService = MS_WWI_CHECK_BIRTHDAYS;
	cl.pszName = "Check for birthdays";
	cl.pszPopupName = "Birthdays (When Was It)";
	hmCheckBirthdays = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_LIST_SHOW;
	cl.pszName = "Birthday list";
	cl.hIcon = hiListMenu;
	hmBirthdayList = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_REFRESH_USERDETAILS;
	cl.position = 10100000;
	cl.pszName = "Refresh user details";
	cl.hIcon = hiRefreshUserDetails;
	hmRefreshDetails = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_IMPORT_BIRTHDAYS;
	cl.position = 10200000;
	cl.pszName = "Import birthdays";
	cl.hIcon = hiImportBirthdays;
	hmImportBirthdays = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_EXPORT_BIRTHDAYS;
	cl.pszName = "Export birthdays";
	cl.hIcon = hiExportBirthdays;
	hmExportBirthdays = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_ADD_BIRTHDAY;
	cl.position = 10000000;
	cl.hIcon = hiAddBirthdayContact;
	cl.pszName = "Add/change user &birthday";
	hmAddChangeBirthday = Menu_AddContactMenuItem(&cl);

	// Register hotkeys
	HOTKEYDESC hotkey = {0};
	hotkey.cbSize = sizeof(hotkey);
	hotkey.pszSection = LPGEN("Birthdays");

	hotkey.pszName = "wwi_birthday_list";
	hotkey.pszDescription = LPGEN("Birthday list");
	hotkey.pszService = MS_WWI_LIST_SHOW;
	Hotkey_Register(&hotkey);
		
	hotkey.pszName = "wwi_check_birthdays";
	hotkey.pszDescription = LPGEN("Check for birthdays");
	hotkey.pszService = MS_WWI_CHECK_BIRTHDAYS;
	Hotkey_Register(&hotkey);
	
	return 0;
}

int OnTopToolBarModuleLoaded(WPARAM wParam, LPARAM lParam)
{
	TTBButton ttb = {0};
	ttb.cbSize = sizeof(TTBButton);
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = MS_WWI_CHECK_BIRTHDAYS;
	ttb.hIconUp = ttb.hIconDn = hiCheckMenu;
	ttb.name = "Check for birthdays";
	CallService(MS_TTB_ADDBUTTON, (WPARAM) &ttb, 0);
	return 0;
}

int OnOptionsInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	
	odp.cbSize = sizeof(odp);
	odp.position = 100000000;
	odp.hInstance = hInstance;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_WWI);
	odp.pszTitle = LPGEN("When Was It");
	odp.pszGroup = LPGEN("Contact List");
	odp.groupPosition = 910000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcOptions;
	Options_AddPage(wParam, &odp);
	
	return 0;
}

void UpdateMenuItems()
{
	CLISTMENUITEM cl = {0};
	cl.cbSize = sizeof(CLISTMENUITEM);
	cl.flags = CMIM_ICON;
	cl.hIcon = hiCheckMenu;
	int res = CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hmCheckBirthdays, (LPARAM) &cl);
	
	cl.hIcon = hiListMenu;
	res = CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hmBirthdayList, (LPARAM) &cl);
	
	cl.hIcon = hiRefreshUserDetails;
	res = CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hmRefreshDetails, (LPARAM) &cl);
	
	cl.hIcon = hiAddBirthdayContact;
	res = CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hmAddChangeBirthday, (LPARAM) &cl);
	
	cl.hIcon = hiImportBirthdays;
	res = CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hmImportBirthdays, (LPARAM) &cl);
	
	cl.hIcon = hiExportBirthdays;
	res = CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hmExportBirthdays, (LPARAM) &cl);
}

int OnIconsChanged(WPARAM wParam, LPARAM lParam)
{
	GetIcons();
	UpdateMenuItems();
	
	return 0;
}

int OnContactSettingChanged(WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dw = (DBCONTACTWRITESETTING *) lParam;
	//static HANDLE oldContact = NULL;
	DBVARIANT dv = dw->value;
	if ((strcmp(dw->szModule, DUMMY_MODULE) == 0) && (strcmp(dw->szSetting, DUMMY_SETTING) == 0))
		{
			OnExtraImageApply(wParam, 0);
		}
	//oldContact = (HANDLE) wParam;
	
	return 0;
}

int OnExtraIconListRebuild(WPARAM wParam, LPARAM lParam)
{
	RebuildAdvIconList();
	
	return 0;
}

int OnExtraImageApply(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;
	if ((hContact))
		{
			int count = CallService(MS_DB_CONTACT_GETCOUNT, 0, 0);
			//int daysInAdvance = DBGetContactSettingWord(NULL, ModuleName, "DaysInAdvance", DAYS_TO_NOTIFY);
			//int popupTimeout = DBGetContactSettingWord(NULL, ModuleName, "PopupTimeout", POPUP_TIMEOUT);
			//DWORD foreground = DBGetContactSettingDword(NULL, ModuleName, "Foreground", FOREGROUND_COLOR);
			//DWORD background = DBGetContactSettingDword(NULL, ModuleName, "Background", BACKGROUND_COLOR);
			//int bUsePopups = DBGetContactSettingByte(NULL, ModuleName, "UsePopups", TRUE);
			//int bUseClistIcon = DBGetContactSettingByte(NULL, ModuleName, "UseClistIcon", TRUE);
			//int bUseDialog = DBGetContactSettingByte(NULL, ModuleName, "UseDialog", TRUE);
			//int clistIcon = DBGetContactSettingByte(NULL, ModuleName, "AdvancedIcon", CLIST_ICON);
			//int notifyFor = DBGetContactSettingByte(NULL, ModuleName, "NotifyFor", 0);
			int hidden = DBGetContactSettingByte(hContact, "CList", "Hidden", 0);
			int ignored = DBGetContactSettingDword(hContact, "Ignore", "Mask1", 0);
			ignored = ((ignored & 0x3f) != 0) ? 1 : 0;
			int ok = 1;
			if (commonData.notifyFor & EXCLUDE_HIDDEN)
			{
				ok &= (hidden == 0);
			}
			if (commonData.notifyFor & EXCLUDE_IGNORED)
			{
				ok &= (ignored == 0);
			}
		
			int dtb;
			int dab;
			int caps = ServiceExists(MS_CLIST_EXTRA_ADD_ICON); // CallService(MS_CLUI_GETCAPS, 0, 0);
			
			time_t today = Today();
			
			if ((ok) && (((dtb = NotifyContactBirthday(hContact, today, commonData.daysInAdvance)) >= 0) || ((dab = NotifyMissedContactBirthday(hContact, today, commonData.daysAfter)) > 0)))
			{
				int age = GetContactAge(hContact);
				DBWriteContactSettingByte(hContact, "UserInfo", "Age", age);
				
				if ((bShouldCheckBirthdays) && (commonData.bUsePopups))
				{
					if (dtb >= 0)
					{
						bBirthdayFound = 1; //only set it if we're called from our CheckBirthdays service
						PopupNotifyBirthday(hContact, dtb, age);
					}
					else if (dab > 0)
					{
						PopupNotifyMissedBirthday(hContact, dab, age);
					}
				}
					
				if (bShouldCheckBirthdays)
				{
					if (dtb >= 0)
					{
						SoundNotifyBirthday(dtb);
					}
				}
					
				if ((bShouldCheckBirthdays) && (commonData.bUseDialog))
				{
					if (dtb >= 0)
					{
						DialogNotifyBirthday(hContact, dtb, age);
					}
					else if (dab > 0)
					{
						DialogNotifyMissedBirthday(hContact, dab, age);
					}
				}
				
				if ((caps > 0) && (commonData.bUseClistIcon)) //TODO
				{
					if (dtb >= 0)
					{
						ClistIconNotifyBirthday(hContact, dtb, commonData.clistIcon);
					}
				}
			}
			else{
				if (caps > 0) //TODO
				{ //clear the icon
					ClearClistIcon(hContact, commonData.clistIcon);
				}
			}
		}
		
	return 0;
}

int UpdateTimers()
{
	if (hCheckTimer)
		{
			KillTimer(NULL, hCheckTimer);
			hCheckTimer = NULL;
		}
	long interval;
	interval = DBGetContactSettingDword(NULL, ModuleName, "Interval", CHECK_INTERVAL);
	interval *= 1000 * 60 * 60; //go from miliseconds to hours
	hCheckTimer = SetTimer(NULL, 0, interval, (TIMERPROC) OnCheckTimer);
	if (!hDateChangeTimer)
		{
			hDateChangeTimer = SetTimer(NULL, 0, 1000 * DATE_CHANGE_CHECK_INTERVAL, (TIMERPROC) OnDateChangeTimer);
		}
		
	return 0;
}

int KillTimers()
{
	Log("%s", "Entering function " __FUNCTION__);
	if (hCheckTimer)
		{
			KillTimer(NULL, hCheckTimer);
			hCheckTimer = NULL;
		}
	if (hDateChangeTimer)
		{
			KillTimer(NULL, hDateChangeTimer);
			hDateChangeTimer = NULL;
		}
	Log("%s", "Leaving function " __FUNCTION__);
			
	return 0;
}

VOID CALLBACK OnCheckTimer(HWND hWnd, UINT msg, UINT_PTR idEvent, DWORD dwTime)
{
	Log("%s", "Entering function " __FUNCTION__);
	CheckBirthdaysService(0, 1);
	Log("%s", "Leaving function " __FUNCTION__);
}

VOID CALLBACK OnDateChangeTimer(HWND hWnd, UINT msg, UINT_PTR idEvent, DWORD dwTime)
{
	SYSTEMTIME now;
	//Log("%s", "Entering function " __FUNCTION__);
	GetLocalTime(&now);
	if (currentDay != now.wDay)
		{
			CheckBirthdaysService(0, 1);
		}
		//else{
		//	RefreshAllContactListIcons();
		//}
	currentDay = now.wDay;
		
	//Log("%s", "Leaving function " __FUNCTION__);
}


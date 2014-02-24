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

#define DATE_CHANGE_CHECK_INTERVAL 20

#define EXCLUDE_HIDDEN  1
#define EXCLUDE_IGNORED 2

HANDLE hmCheckBirthdays = NULL;
HANDLE hmBirthdayList = NULL;
HANDLE hmRefreshDetails = NULL;
HANDLE hmAddChangeBirthday = NULL;
HANDLE hmImportBirthdays = NULL;
HANDLE hmExportBirthdays = NULL;

UINT_PTR hCheckTimer = NULL;
UINT_PTR hDateChangeTimer = NULL;

int currentDay;

static int OnTopToolBarModuleLoaded(WPARAM wParam, LPARAM lParam)
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = MS_WWI_CHECK_BIRTHDAYS;
	ttb.hIconHandleUp = hCheckMenu;
	ttb.name = ttb.pszTooltipUp = LPGEN("Check for birthdays");
	TopToolbar_AddButton(&ttb);
	return 0;
}

static int OnOptionsInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 100000000;
	odp.hInstance = hInstance;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_WWI);
	odp.ptszTitle = LPGENT("Birthdays");
	odp.ptszGroup = LPGENT("Contacts");
	odp.groupPosition = 910000000;
	odp.flags = ODPF_BOLDGROUPS|ODPF_TCHAR;
	odp.pfnDlgProc = DlgProcOptions;
	Options_AddPage(wParam, &odp);
	return 0;
}

static int OnContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dw = (DBCONTACTWRITESETTING *) lParam;
	DBVARIANT dv = dw->value;
	if ((strcmp(dw->szModule, DUMMY_MODULE) == 0) && (strcmp(dw->szSetting, DUMMY_SETTING) == 0))
		RefreshContactListIcons(hContact);
	
	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);
	HookEvent(ME_TTB_MODULELOADED, OnTopToolBarModuleLoaded);
	
	SkinAddNewSoundExT(BIRTHDAY_NEAR_SOUND, LPGENT("WhenWasIt"), LPGENT("Birthday near"));
	SkinAddNewSoundExT(BIRTHDAY_TODAY_SOUND, LPGENT("WhenWasIt"), LPGENT("Birthday today"));
	
	UpdateTimers();

	CLISTMENUITEM cl = { sizeof(cl) };
	cl.position = 10000000;
	cl.pszPopupName = LPGEN("Birthdays (When Was It)");

	cl.pszService = MS_WWI_CHECK_BIRTHDAYS;
	cl.icolibItem = hCheckMenu;
	cl.pszName = LPGEN("Check for birthdays");
	hmCheckBirthdays = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_LIST_SHOW;
	cl.pszName = LPGEN("Birthday list");
	cl.icolibItem = hListMenu;
	hmBirthdayList = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_REFRESH_USERDETAILS;
	cl.position = 10100000;
	cl.pszName = LPGEN("Refresh user details");
	cl.icolibItem = hRefreshUserDetails;
	hmRefreshDetails = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_IMPORT_BIRTHDAYS;
	cl.position = 10200000;
	cl.pszName = LPGEN("Import birthdays");
	cl.icolibItem = hImportBirthdays;
	hmImportBirthdays = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_EXPORT_BIRTHDAYS;
	cl.pszName = LPGEN("Export birthdays");
	cl.icolibItem = hExportBirthdays;
	hmExportBirthdays = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_ADD_BIRTHDAY;
	cl.position = 10000000;
	cl.icolibItem = hAddBirthdayContact;
	cl.pszName = LPGEN("Add/change user &birthday");
	hmAddChangeBirthday = Menu_AddContactMenuItem(&cl);

	// Register hotkeys
	HOTKEYDESC hotkey = { sizeof(hotkey) };
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

int HookEvents()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, OnOptionsInitialise);
	return 0;
}

int UnhookEvents()
{
	KillTimers();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int RefreshContactListIcons(MCONTACT hContact)
{
	if (hContact == 0)
		return 0;

	int count = CallService(MS_DB_CONTACT_GETCOUNT, 0, 0);
	int hidden = db_get_b(hContact, "CList", "Hidden", 0);
	int ignored = db_get_dw(hContact, "Ignore", "Mask1", 0);
	ignored = ((ignored & 0x3f) != 0) ? 1 : 0;
	int ok = 1;
	if (commonData.notifyFor & EXCLUDE_HIDDEN)
		ok &= (hidden == 0);

	if (commonData.notifyFor & EXCLUDE_IGNORED)
		ok &= (ignored == 0);

	time_t today = Today();

	int dtb = NotifyContactBirthday(hContact, today, commonData.daysInAdvance);
	int dab = NotifyMissedContactBirthday(hContact, today, commonData.daysAfter);

	if (ok && (dtb >= 0 || dab > 0)) {
		int age = GetContactAge(hContact);
		db_set_b(hContact, "UserInfo", "Age", age);

		if ((bShouldCheckBirthdays) && (commonData.bUsePopups))
		{
			if (dtb >= 0) {
				bBirthdayFound = 1; //only set it if we're called from our CheckBirthdays service
				PopupNotifyBirthday(hContact, dtb, age);
			}
			else if (dab > 0)
				PopupNotifyMissedBirthday(hContact, dab, age);
		}

		if (bShouldCheckBirthdays)
			if (dtb >= 0)
				SoundNotifyBirthday(dtb);

		if ((bShouldCheckBirthdays) && (commonData.bUseDialog)) {
			if (dtb >= 0)
				DialogNotifyBirthday(hContact, dtb, age);
			else if (dab > 0)
				DialogNotifyMissedBirthday(hContact, dab, age);
		}

		if (dtb >= 0)
			ExtraIcon_SetIcon(hWWIExtraIcons, hContact, GetDTBIconHandle(dtb));
	}
	else ExtraIcon_Clear(hWWIExtraIcons, hContact);

	return 0;
}

int UpdateTimers()
{
	if (hCheckTimer) {
		KillTimer(NULL, hCheckTimer);
		hCheckTimer = NULL;
	}

	long interval = db_get_dw(NULL, ModuleName, "Interval", CHECK_INTERVAL);
	interval *= 1000 * 60 * 60; //go from miliseconds to hours
	hCheckTimer = SetTimer(NULL, 0, interval, (TIMERPROC) OnCheckTimer);
	if ( !hDateChangeTimer)
		hDateChangeTimer = SetTimer(NULL, 0, 1000 * DATE_CHANGE_CHECK_INTERVAL, (TIMERPROC) OnDateChangeTimer);
		
	return 0;
}

int KillTimers()
{
	if (hCheckTimer) {
		KillTimer(NULL, hCheckTimer);
		hCheckTimer = NULL;
	}

	if (hDateChangeTimer) {
		KillTimer(NULL, hDateChangeTimer);
		hDateChangeTimer = NULL;
	}

	return 0;
}

VOID CALLBACK OnCheckTimer(HWND hWnd, UINT msg, UINT_PTR idEvent, DWORD dwTime)
{
	CheckBirthdaysService(0, 1);
}

VOID CALLBACK OnDateChangeTimer(HWND hWnd, UINT msg, UINT_PTR idEvent, DWORD dwTime)
{
	SYSTEMTIME now;
	GetLocalTime(&now);

	if (currentDay != now.wDay)
		CheckBirthdaysService(0, 1);

	currentDay = now.wDay;
}


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

#include "stdafx.h"

#define DATE_CHANGE_CHECK_INTERVAL 20

#define EXCLUDE_HIDDEN  1
#define EXCLUDE_IGNORED 2
UINT_PTR hCheckTimer = NULL;
UINT_PTR hDateChangeTimer = NULL;
static int currentDay = 0;

void CloseBirthdayList();
void CloseUpcoming();

static int OnTopToolBarModuleLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = MS_WWI_CHECK_BIRTHDAYS;
	ttb.hIconHandleUp = hCheckMenu;
	ttb.name = ttb.pszTooltipUp = LPGEN("Check for birthdays");
	g_plugin.addTTB(&ttb);
	return 0;
}

static int OnContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dw = (DBCONTACTWRITESETTING *)lParam;
	if ((strcmp(dw->szModule, DUMMY_MODULE) == 0) && (strcmp(dw->szSetting, DUMMY_SETTING) == 0))
		RefreshContactListIcons(hContact);

	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	CloseBirthdayList();
	CloseUpcoming();

	WindowList_Broadcast(hAddBirthdayWndsList, WM_CLOSE, 0, 0);
	WindowList_Destroy(hAddBirthdayWndsList);
	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);
	HookEvent(ME_TTB_MODULELOADED, OnTopToolBarModuleLoaded);

	UpdateTimers();
	return 0;
}

int HookEvents()
{
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
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

	bool hidden = Contact::IsHidden(hContact);
	int ignored = db_get_dw(hContact, "Ignore", "Mask1", 0);
	ignored = ((ignored & 0x3f) != 0) ? 1 : 0;
	int ok = 1;
	if (g_plugin.notifyFor & EXCLUDE_HIDDEN)
		ok &= (hidden == 0);

	if (g_plugin.notifyFor & EXCLUDE_IGNORED)
		ok &= (ignored == 0);

	time_t today = Today();

	int dtb = NotifyContactBirthday(hContact, today, g_plugin.daysInAdvance);
	int dab = NotifyMissedContactBirthday(hContact, today, g_plugin.daysAfter);

	if (ok && (dtb >= 0 || dab > 0)) {
		int age = GetContactAge(hContact);
		db_set_b(hContact, "UserInfo", "Age", age);

		if ((bShouldCheckBirthdays) && (g_plugin.bUsePopups))
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

		if ((bShouldCheckBirthdays) && (g_plugin.bUseDialog)) {
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
	UINT interval = g_plugin.getDword("Interval", CHECK_INTERVAL);
	interval *= (1000 * 60 * 60); //go from miliseconds to hours
	hCheckTimer = SetTimer(nullptr, 0, interval, OnCheckTimer);
	if (!hDateChangeTimer)
		hDateChangeTimer = SetTimer(nullptr, 0, (1000 * DATE_CHANGE_CHECK_INTERVAL), OnDateChangeTimer);

	return 0;
}

int KillTimers()
{
	KillTimer(nullptr, hCheckTimer);
	hCheckTimer = NULL;

	KillTimer(nullptr, hDateChangeTimer);
	hDateChangeTimer = NULL;

	return 0;
}

VOID CALLBACK OnCheckTimer(HWND, UINT, UINT_PTR, DWORD)
{
	CheckBirthdaysService(0, 1);
}

VOID CALLBACK OnDateChangeTimer(HWND, UINT, UINT_PTR, DWORD)
{
	SYSTEMTIME now;
	GetLocalTime(&now);

	if (currentDay != now.wDay)
		CheckBirthdaysService(0, 1);

	currentDay = now.wDay;
}


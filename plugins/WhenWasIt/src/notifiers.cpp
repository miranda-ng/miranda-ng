/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006 Cristian Libotean

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

void FillPopupData(POPUPDATAW &ppd, int dtb)
{
	int popupTimeout = (dtb == 0) ? g_plugin.popupTimeoutToday : g_plugin.popupTimeout;

	ppd.colorBack = g_plugin.background;
	ppd.colorText = g_plugin.foreground;
	ppd.iSeconds = popupTimeout;
}

void PopupNotifyNoBirthdays()
{
	POPUPDATAW ppd;
	FillPopupData(ppd, -1);
	ppd.lchIcon = GetDTBIcon(-1);

	wcsncpy(ppd.lpwzContactName, TranslateT("WhenWasIt"), MAX_CONTACTNAME - 1);
	wcsncpy(ppd.lpwzText, TranslateT("No upcoming birthdays."), MAX_SECONDLINE - 1);
	PUAddPopupW(&ppd);
}

wchar_t *BuildDTBText(int dtb, wchar_t *name, wchar_t *text, int size)
{
	if (dtb > 1)
		mir_snwprintf(text, size, TranslateT("%s has birthday in %d days."), name, dtb);
	else if (dtb == 1)
		mir_snwprintf(text, size, TranslateT("%s has birthday tomorrow."), name);
	else
		mir_snwprintf(text, size, TranslateT("%s has birthday today."), name);

	return text;
}

wchar_t *BuildDABText(int dab, wchar_t *name, wchar_t *text, int size)
{
	if (dab > 1)
		mir_snwprintf(text, size, TranslateT("%s had birthday %d days ago."), name, dab);
	else if (dab == 1)
		mir_snwprintf(text, size, TranslateT("%s had birthday yesterday."), name);
	else
		mir_snwprintf(text, size, TranslateT("%s has birthday today (Should not happen, please report)."), name);

	return text;
}

int PopupNotifyBirthday(MCONTACT hContact, int dtb, int age)
{
	if (g_plugin.bIgnoreSubcontacts && db_mc_isSub(hContact))
		return 0;

	wchar_t *name = Clist_GetContactDisplayName(hContact);

	wchar_t text[1024];
	BuildDTBText(dtb, name, text, _countof(text));
	int gender = GetContactGender(hContact);

	POPUPDATAW ppd;
	FillPopupData(ppd, dtb);
	ppd.lchContact = hContact;
	ppd.PluginWindowProc = DlgProcPopup;
	ppd.lchIcon = GetDTBIcon(dtb);

	mir_snwprintf(ppd.lpwzContactName, MAX_CONTACTNAME, TranslateT("Birthday - %s"), name);
	wchar_t *sex;
	switch (toupper(gender)) {
	case 'M':
		sex = TranslateT("He");
		break;
	case 'F':
		sex = TranslateT("She");
		break;
	default:
		sex = TranslateT("He/She");
		break;
	}
	if (age > 0) {
		if (dtb > 0)
			mir_snwprintf(ppd.lpwzText, MAX_SECONDLINE, TranslateT("%s\n%s will be %d years old."), text, sex, age);
		else
			mir_snwprintf(ppd.lpwzText, MAX_SECONDLINE, TranslateT("%s\n%s just turned %d."), text, sex, age);
	}
	else
		mir_wstrncpy(ppd.lpwzText, text, MAX_SECONDLINE - 1);

	PUAddPopupW(&ppd);

	return 0;
}

int PopupNotifyMissedBirthday(MCONTACT hContact, int dab, int age)
{
	if (g_plugin.bIgnoreSubcontacts && db_mc_isSub(hContact))
		return 0;

	wchar_t *name = Clist_GetContactDisplayName(hContact);

	wchar_t text[1024];
	BuildDABText(dab, name, text, _countof(text));
	int gender = GetContactGender(hContact);

	POPUPDATAW ppd;
	FillPopupData(ppd, dab);
	ppd.lchContact = hContact;
	ppd.PluginWindowProc = DlgProcPopup;
	ppd.lchIcon = GetDTBIcon(dab);

	mir_snwprintf(ppd.lpwzContactName, MAX_CONTACTNAME, TranslateT("Birthday - %s"), name);
	wchar_t *sex;
	switch (toupper(gender)) {
	case 'M':
		sex = TranslateT("He");
		break;
	case 'F':
		sex = TranslateT("She");
		break;
	default:
		sex = TranslateT("He/She");
		break;
	}
	if (age > 0)
		mir_snwprintf(ppd.lpwzText, MAX_SECONDLINE, TranslateT("%s\n%s just turned %d."), text, sex, age);
	else
		mir_wstrncpy(ppd.lpwzText, text, MAX_SECONDLINE - 1);

	PUAddPopupW(&ppd);
	return 0;
}

int DialogNotifyBirthday(MCONTACT hContact, int dtb, int age)
{
	wchar_t *name = Clist_GetContactDisplayName(hContact);

	wchar_t text[1024];
	BuildDTBText(dtb, name, text, _countof(text));
	if (!hUpcomingDlg) {
		hUpcomingDlg = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_UPCOMING), nullptr, DlgProcUpcoming);
		ShowWindow(hUpcomingDlg, g_plugin.bOpenInBackground ? SW_SHOWNOACTIVATE : SW_SHOW);
	}

	TUpcomingBirthday data = { 0 };
	data.name = name;
	data.message = text;
	data.dtb = dtb;
	data.hContact = hContact;
	data.age = age;

	SendMessage(hUpcomingDlg, WWIM_ADD_UPCOMING_BIRTHDAY, (WPARAM)&data, NULL);
	return 0;
}

int DialogNotifyMissedBirthday(MCONTACT hContact, int dab, int age)
{
	wchar_t *name = Clist_GetContactDisplayName(hContact);

	wchar_t text[1024];
	BuildDABText(dab, name, text, _countof(text));
	if (!hUpcomingDlg) {
		hUpcomingDlg = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_UPCOMING), nullptr, DlgProcUpcoming);
		ShowWindow(hUpcomingDlg, g_plugin.bOpenInBackground ? SW_SHOWNOACTIVATE : SW_SHOW);
	}

	TUpcomingBirthday data = { 0 };
	data.name = name;
	data.message = text;
	data.dtb = -dab;
	data.hContact = hContact;
	data.age = age;

	SendMessage(hUpcomingDlg, WWIM_ADD_UPCOMING_BIRTHDAY, (WPARAM)&data, NULL);
	return 0;
}

int SoundNotifyBirthday(int dtb)
{
	if (dtb == 0)
		Skin_PlaySound(BIRTHDAY_TODAY_SOUND);
	else if (dtb <= g_plugin.cSoundNearDays)
		Skin_PlaySound(BIRTHDAY_NEAR_SOUND);

	return 0;
}

//if oldClistIcon != -1 it will remove the old location of the clist extra icon
//called with oldClistIcon != -1 from dlg_handlers whtn the extra icon slot changes.
int RefreshAllContactListIcons(int oldClistIcon)
{
	for (auto &hContact : Contacts()) {
		if (oldClistIcon != -1)
			ExtraIcon_Clear(hWWIExtraIcons, hContact);

		RefreshContactListIcons(hContact); //will change bBirthdayFound if needed
	}
	return 0;
}

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

#include "notifiers.h"

void FillPopupData(POPUPDATAT &pd, int dtb)
{
	//DWORD foreground = DBGetContactSettingDword(NULL, ModuleName, "Foreground", FOREGROUND_COLOR);
	//DWORD background = DBGetContactSettingDword(NULL, ModuleName, "Background", BACKGROUND_COLOR);
	int popupTimeout = (dtb == 0) ? commonData.popupTimeoutToday : commonData.popupTimeout;

	pd.colorBack = commonData.background;
	pd.colorText = commonData.foreground;
	pd.iSeconds = popupTimeout;
}

int PopupNotifyNoBirthdays()
{
	POPUPDATAT pd = {0};
	FillPopupData(pd, -1);
	pd.lchIcon = GetDTBIcon(-1);

	_tcscpy(pd.lptzContactName, TranslateT("WhenWasIt"));
	_tcscpy(pd.lptzText, TranslateT("No upcoming birthdays."));

	
	return PUAddPopUpT(&pd);
}

TCHAR *BuildDTBText(int dtb, TCHAR *name, TCHAR *text, int size)
{
	if (dtb > 1)
	{
		mir_sntprintf(text, size, TranslateT("%s has birthday in %d days."), name, dtb);
	}
	else{
		if (dtb == 1)
		{
			mir_sntprintf(text, size, TranslateT("%s has birthday tomorrow."), name);
		}
		else{
			mir_sntprintf(text, size, TranslateT("%s has birthday today."), name);
		}
	}
		
	return text;
}

TCHAR *BuildDABText(int dab, TCHAR *name, TCHAR *text, int size)
{
	if (dab > 1)
	{
		mir_sntprintf(text, size, TranslateT("%s had birthday %d days ago."), name, dab);
	}
	else{
		if (dab == 1)
		{
			mir_sntprintf(text, size, TranslateT("%s had birthday yesterday."), name);
		}
		else{
			mir_sntprintf(text, size, TranslateT("%s has birthday today (Should not happen, please report)."), name);
		}
	}
	
	return text;
}

int PopupNotifyBirthday(HANDLE hContact, int dtb, int age)
{
	TCHAR *name = GetContactName(hContact, NULL);
	const int MAX_SIZE = 1024;
	TCHAR text[MAX_SIZE];
	//int bIgnoreSubcontacts = DBGetContactSettingByte(NULL, ModuleName, "IgnoreSubcontacts", FALSE);
	if (commonData.bIgnoreSubcontacts)
		{
			HANDLE hMetacontact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM) hContact, 0);
			if ((hMetacontact) && (hMetacontact != hContact)) //not main metacontact
				{
					return 0;
				}
		}
	BuildDTBText(dtb, name, text, MAX_SIZE);
	int gender = GetContactGender(hContact);
	
	POPUPDATAT pd = {0};
	FillPopupData(pd, dtb);
	pd.lchContact = hContact;
	pd.PluginWindowProc = (WNDPROC) DlgProcPopup;
	//pd.PluginData = (void *) hContact;
	//pd.colorBack = background;
	//pd.colorText = foreground;
	//pd.iSeconds = popupTimeout;
	pd.lchIcon = GetDTBIcon(dtb);
	
	//strcpy(pd.lpzContactName, text);
	_stprintf(pd.lptzContactName, TranslateT("Birthday - %s"), name);
	TCHAR *sex;
	switch (toupper(gender))
		{
			case _T('M'):
				{
					sex = TranslateT("He");
					break;
				}
			case _T('F'):
				{
					sex = TranslateT("She");
					break;
				}
			default:
				{
					sex = TranslateT("He/She");
					break;
				}
		}
	if (dtb > 0)
		{
			_stprintf(pd.lptzText, TranslateT("%s\n%s will be %d years old."), text, sex, age);
		}
		else{
			_stprintf(pd.lptzText, TranslateT("%s\n%s just turned %d."), text, sex, age);
		}
	
	PUAddPopUpT(&pd);
	free(name);
	
	return 0;
}

int PopupNotifyMissedBirthday(HANDLE hContact, int dab, int age)
{
	TCHAR *name = GetContactName(hContact, NULL);
	const int MAX_SIZE = 1024;
	TCHAR text[MAX_SIZE];
	//int bIgnoreSubcontacts = DBGetContactSettingByte(NULL, ModuleName, "IgnoreSubcontacts", FALSE);
	if (commonData.bIgnoreSubcontacts)
		{
			HANDLE hMetacontact = (HANDLE) CallService(MS_MC_GETMETACONTACT, (WPARAM) hContact, 0);
			if ((hMetacontact) && (hMetacontact != hContact)) //not main metacontact
				{
					return 0;
				}
		}
	BuildDABText(dab, name, text, MAX_SIZE);
	int gender = GetContactGender(hContact);
	
	POPUPDATAT pd = {0};
	FillPopupData(pd, dab);
	pd.lchContact = hContact;
	pd.PluginWindowProc = (WNDPROC) DlgProcPopup;
	//pd.PluginData = (void *) hContact;
	//pd.colorBack = background;
	//pd.colorText = foreground;
	//pd.iSeconds = popupTimeout;
	pd.lchIcon = GetDABIcon(dab);
	
	//strcpy(pd.lpzContactName, text);
	_stprintf(pd.lptzContactName, TranslateT("Birthday - %s"), name);
	TCHAR *sex;
	switch (toupper(gender))
		{
			case _T('M'):
				{
					sex = TranslateT("He");
					break;
				}
			case _T('F'):
				{
					sex = TranslateT("She");
					break;
				}
			default:
				{
					sex = TranslateT("He/She");
					break;
				}
		}
	if (dab > 0)
		{
			_stprintf(pd.lptzText, TranslateT("%s\n%s just turned %d."), text, sex, age);
		}
		else{
			_stprintf(pd.lptzText, TranslateT("%s\n%s just turned %d."), text, sex, age);
		}
	
	PUAddPopUpT(&pd);
	free(name);
	
	return 0;
}

int ClistIconNotifyBirthday(HANDLE hContact, int dtb, int advancedIcon)
{
	if (ServiceExists(MS_EXTRAICON_SET_ICON))
	{
		ExtraIcon_SetIcon(hWWIExtraIcons, hContact, GetClistIcon(dtb));
	}
	else {
		IconExtraColumn iec = {0};
		iec.cbSize = sizeof(IconExtraColumn);
		iec.ColumnType = advancedIcon;
		iec.hImage = GetClistIcon(dtb);
		CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) hContact, (LPARAM) &iec);
	}
	
	return 0;
}

int DialogNotifyBirthday(HANDLE hContact, int dtb, int age)
{
	TCHAR *name = GetContactName(hContact, NULL);
	const int MAX_SIZE = 1024;
	TCHAR text[MAX_SIZE];
	
	BuildDTBText(dtb, name, text, MAX_SIZE);
	if (!hUpcomingDlg)
	{
		hUpcomingDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_UPCOMING), NULL, DlgProcUpcoming);
		ShowWindow(hUpcomingDlg, commonData.bOpenInBackground ? SW_SHOWNOACTIVATE : SW_SHOW);
	}
	
	TUpcomingBirthday data = {0};
	data.name = name;
	data.message = text;
	data.dtb = dtb;
	data.hContact = hContact;
	data.age = age;
	
	SendMessage(hUpcomingDlg, WWIM_ADD_UPCOMING_BIRTHDAY, (WPARAM) &data, NULL);
	
	free(name);
	
	return 0;
}

int DialogNotifyMissedBirthday(HANDLE hContact, int dab, int age)
{
	TCHAR *name = GetContactName(hContact, NULL);
	const int MAX_SIZE = 1024;
	TCHAR text[MAX_SIZE];
	
	BuildDABText(dab, name, text, MAX_SIZE);
	if (!hUpcomingDlg)
	{
		hUpcomingDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_UPCOMING), NULL, DlgProcUpcoming);
		ShowWindow(hUpcomingDlg, commonData.bOpenInBackground ? SW_SHOWNOACTIVATE : SW_SHOW);
	}
	
	TUpcomingBirthday data = {0};
	data.name = name;
	data.message = text;
	data.dtb = -dab;
	data.hContact = hContact;
	data.age = age;
	
	SendMessage(hUpcomingDlg, WWIM_ADD_UPCOMING_BIRTHDAY, (WPARAM) &data, NULL);
	
	free(name);
	
	return 0;
}

int SoundNotifyBirthday(int dtb)
{
	//int daysToPlay = DBGetContactSettingByte(NULL, ModuleName, "SoundNearDays", BIRTHDAY_NEAR_DEFAULT_DAYS);
	if (dtb == 0)
		{
			SkinPlaySound(BIRTHDAY_TODAY_SOUND);
		}
		else{
			if (dtb <= commonData.cSoundNearDays)
				{
					SkinPlaySound(BIRTHDAY_NEAR_SOUND);
				}
		}
	
	return 0;
}

int ClearClistIcon(HANDLE hContact, int advancedIcon)
{
	if (ServiceExists(MS_EXTRAICON_SET_ICON))
	{
		return ExtraIcon_SetIcon(hWWIExtraIcons, hContact, (HANDLE) -1);
	}
	else {
		IconExtraColumn iec = {0};
		iec.cbSize = sizeof(IconExtraColumn);
		iec.ColumnType = advancedIcon;
		iec.hImage = (HANDLE) -1;
	
		return CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) hContact, (LPARAM) &iec);
	}
}

int RefreshContactListIcons(HANDLE hContact)
{
	//if (DBGetContactSettingWord(hContact, DUMMY_MODULE, DUMMY_SETTING, 0) != 0)
	//	{
	//		DBDeleteContactSetting(hContact, DUMMY_MODULE, DUMMY_SETTING);
	//	}
	//	else{
	//		DBWriteContactSettingWord(hContact, DUMMY_MODULE, DUMMY_SETTING, 101);
	//	}
	OnExtraImageApply((WPARAM) hContact, 0);
			
	return 0;
}

//if oldClistIcon != -1 it will remove the old location of the clist extra icon
//called with oldClistIcon != -1 from dlg_handlers whtn the extra icon slot changes.
int RefreshAllContactListIcons(int oldClistIcon)
{
	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact != NULL)
		{
			if (oldClistIcon != -1)
				{
					ClearClistIcon(hContact, oldClistIcon);
				}
			RefreshContactListIcons(hContact); //will change bBirthdayFound if needed
			hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
		}
	return 0;
}
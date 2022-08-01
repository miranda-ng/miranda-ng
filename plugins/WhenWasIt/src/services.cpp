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

#define COMMENT_CHAR '#'

int bShouldCheckBirthdays = 0;
int bBirthdayFound = 0;

int InitServices()
{
	Log("%s", "Entering function " __FUNCTION__);

	CreateServiceFunction(MS_WWI_CHECK_BIRTHDAYS, CheckBirthdaysService);
	CreateServiceFunction(MS_WWI_LIST_SHOW, ShowListService);
	CreateServiceFunction(MS_WWI_ADD_BIRTHDAY, AddBirthdayService);
	CreateServiceFunction(MS_WWI_REFRESH_USERDETAILS, RefreshUserDetailsService);
	CreateServiceFunction(MS_WWI_IMPORT_BIRTHDAYS, ImportBirthdaysService);
	CreateServiceFunction(MS_WWI_EXPORT_BIRTHDAYS, ExportBirthdaysService);

	Log("%s", "Leaving function " __FUNCTION__);
	return 0;
}

/*
returns -1 if notify is not necesarry
returns daysToBirthday if it should notify
*/
int NotifyContactBirthday(MCONTACT hContact, time_t now, int daysInAdvance)
{
	int year, month, day;
	GetContactDOB(hContact, year, month, day);
	int daysToBirthday = DaysToBirthday(now, year, month, day);
	if (daysInAdvance >= daysToBirthday)
		return daysToBirthday;

	return -1;
}

// returns -1 if notify is not necessary
// returns daysAfterBirthday if it should notify
int NotifyMissedContactBirthday(MCONTACT hContact, time_t now, int daysAfter)
{
	if (daysAfter > 0)
	{
		int year, month, day;
		GetContactDOB(hContact, year, month, day);
		int daysAfterBirthday = DaysAfterBirthday(now, year, month, day);
		if ((daysAfterBirthday >= 0) && (daysAfter >= daysAfterBirthday))
		{
			return daysAfterBirthday;
		}
	}

	return -1;
}

//the timer functions call this service with lParam = 1
//lParam = 0 - force check, lParam - 1 do not force it.

INT_PTR CheckBirthdaysService(WPARAM, LPARAM lParam)
{
	bBirthdayFound = 0; //no birthdays have been found in the given interval

	SYSTEMTIME today;
	GetLocalTime(&today);

	uint32_t lastChecked = g_plugin.getDword("LastChecked", 0); //get last checked date
	int lcDay = LOBYTE(LOWORD(lastChecked));
	int lcMonth = HIBYTE(LOWORD(lastChecked));
	int lcYear = HIWORD(lastChecked);

	int daysAfter = DaysAfterBirthday(Today(), lcYear, lcMonth, lcDay); //get difference between last checked date and today
	int savedDaysAfter = 0;

	savedDaysAfter = g_plugin.daysAfter; //save value

	if ((daysAfter > g_plugin.daysAfter) && (g_plugin.daysAfter > 0))//check for passed birthdays
		g_plugin.daysAfter = daysAfter; //bigger values of the two

	if ((lParam) && (g_plugin.bOncePerDay)) //if force check then we don't take OncePerDay into account
		if (lcDay == today.wDay && lcMonth == today.wMonth && lcYear == today.wYear)
			return 0; //already checked today

	bShouldCheckBirthdays = 1;
	RefreshAllContactListIcons();
	if ((!bBirthdayFound) && (g_plugin.bNoBirthdaysPopup))
		PopupNotifyNoBirthdays();

	bShouldCheckBirthdays = 0;

	g_plugin.daysAfter = savedDaysAfter; //restore previous value

	if (lParam) //if not forced - i.e. timer check
		g_plugin.setDword("LastChecked", MAKELONG(MAKEWORD(today.wDay, today.wMonth), today.wYear)); //write the value in DB so we don't check again today

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Refresh users' details thread

void ShowPopupMessage(const wchar_t *title, const wchar_t *message, HANDLE icon)
{
	POPUPDATAW ppd;
	ppd.lchIcon = IcoLib_GetIconByHandle(icon);
	wcsncpy_s(ppd.lpwzContactName, title, _TRUNCATE);
	wcsncpy_s(ppd.lpwzText, message, _TRUNCATE);
	ppd.colorText = g_plugin.foreground;
	ppd.colorBack = g_plugin.background;
	PUAddPopupW(&ppd);
}

void __cdecl RefreshUserDetailsWorkerThread(void*)
{
	Thread_SetName("WhenWasIt: RefreshUserDetailsWorkerThread");

	ShowPopupMessage(TranslateT("WhenWasIt"), TranslateT("Starting to refresh user details"), hRefreshUserDetails);
	int delay = g_plugin.getWord("UpdateDelay", REFRESH_DETAILS_DELAY);

	MCONTACT hContact = db_find_first();
	while (hContact != NULL) {
		ProtoChainSend(hContact, PSS_GETINFO, 0, 0);
		hContact = db_find_next(hContact);
		if (hContact)
			Sleep(delay); //sleep for a few seconds between requests
	}
	ShowPopupMessage(TranslateT("WhenWasIt"), TranslateT("Done refreshing user details"), hRefreshUserDetails);
}

INT_PTR RefreshUserDetailsService(WPARAM, LPARAM)
{
	mir_forkthread(RefreshUserDetailsWorkerThread);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Birthdays import

INT_PTR ImportBirthdaysService(WPARAM, LPARAM)
{
	wchar_t fileName[1024] = { 0 };
	OPENFILENAME of = { 0 };
	of.lStructSize = sizeof(OPENFILENAME);
	//of.g_plugin.getInst() = g_plugin.getInst();
	wchar_t filter[MAX_PATH];
	mir_snwprintf(filter, L"%s (*" BIRTHDAY_EXTENSION L")%c*" BIRTHDAY_EXTENSION L"%c", TranslateT("Birthdays files"), 0, 0);
	of.lpstrFilter = filter;
	of.lpstrFile = fileName;
	of.nMaxFile = _countof(fileName);
	of.lpstrTitle = TranslateT("Please select a file to import birthdays from...");
	of.Flags = OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&of)) {
		wchar_t buffer[2048];
		mir_snwprintf(buffer, TranslateT("Importing birthdays from file: %s"), fileName);
		ShowPopupMessage(TranslateT("WhenWasIt"), buffer, hImportBirthdays);
		DoImport(fileName);
		ShowPopupMessage(TranslateT("WhenWasIt"), TranslateT("Done importing birthdays"), hImportBirthdays);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Birthdays export

INT_PTR ExportBirthdaysService(WPARAM, LPARAM)
{
	wchar_t fileName[1024] = { 0 };
	OPENFILENAME of = { 0 };
	of.lStructSize = sizeof(OPENFILENAME);
	//of.g_plugin.getInst() = g_plugin.getInst();
	wchar_t filter[MAX_PATH];
	mir_snwprintf(filter, L"%s (*" BIRTHDAY_EXTENSION L")%c*" BIRTHDAY_EXTENSION L"%c%s (*.*)%c*.*%c", TranslateT("Birthdays files"), 0, 0, TranslateT("All Files"), 0, 0);
	of.lpstrFilter = filter;
	of.lpstrFile = fileName;
	of.nMaxFile = _countof(fileName);
	of.lpstrTitle = TranslateT("Please select a file to export birthdays to...");

	if (GetSaveFileName(&of)) {
		wchar_t buffer[2048];
		wchar_t *fn = wcsrchr(fileName, '\\') + 1;
		if (!wcschr(fn, '.'))
			mir_wstrcat(fileName, BIRTHDAY_EXTENSION);

		mir_snwprintf(buffer, TranslateT("Exporting birthdays to file: %s"), fileName);
		ShowPopupMessage(TranslateT("WhenWasIt"), buffer, hExportBirthdays);
		DoExport(fileName);
		ShowPopupMessage(TranslateT("WhenWasIt"), TranslateT("Done exporting birthdays"), hExportBirthdays);
	}

	return 0;
}

int DoImport(wchar_t *fileName)
{
	FILE *fin = _wfopen(fileName, L"rt");
	if (!fin) {
		MessageBox(nullptr, TranslateT("Could not open file to import birthdays"), TranslateT("Error"), MB_OK | MB_ICONERROR);
		return 1;
	}

	while (!feof(fin)) {
		wchar_t buffer[4096];
		fgetws(buffer, _countof(buffer), fin);
		if (buffer[0] == COMMENT_CHAR)
			continue;

		wchar_t *delAccount = wcsstr(buffer, L" : ");
		if (delAccount) {
			int tmp = delAccount[0];
			delAccount[0] = '\0';
			wchar_t *delProto = wcsrchr(buffer, '@');
			if (delProto) {
				delProto[0] = '\0';

				wchar_t *szHandle = buffer;
				wchar_t *szProto = delProto + 1;
				MCONTACT hContact = GetContactFromID(szHandle, szProto);
				if (hContact) {
					delProto[0] = tmp;
					delAccount[0] = tmp;

					int year, month, day;
					swscanf(delAccount, L" : %02d/%02d/%04d", &day, &month, &year);
					SaveBirthday(hContact, year, month, day, DOB_USERINFO);
				}
				else {
					CMStringW msg(FORMAT, TranslateT("Could not find UID '%s [%S]' in current database, skipping"), szHandle, szProto);
					ShowPopupMessage(TranslateT("Warning"), msg, hImportBirthdays);
				}
			}
		}
	}

	fclose(fin);
	return 0;
}

int DoExport(wchar_t *fileName)
{
	FILE *fout = _wfopen(fileName, L"wt");
	if (!fout) {
		MessageBox(nullptr, TranslateT("Could not open file to export birthdays"), TranslateT("Error"), MB_OK | MB_ICONERROR);
		return 1;
	}

	fwprintf(fout, L"%c%s", COMMENT_CHAR, TranslateT("Please do not edit this file by hand. Use the export function of WhenWasIt plugin.\n"));

	for (auto &hContact : Contacts()) {
		int year, month, day;
		GetContactDOB(hContact, year, month, day);
		if (IsDOBValid(year, month, day)) {
			char *szProto = Proto_GetBaseAccountName(hContact);
			wchar_t *szHandle = GetContactID(hContact, szProto);

			if ((szHandle) && (mir_strlen(szProto) > 0))
				fwprintf(fout, L"%s@%S : %02d/%02d/%04d\n", szHandle, szProto, day, month, year);

			if (szHandle)
				free(szHandle);
		}
	}

	fclose(fout);
	return 0;
}

/*
Miranda IM History Sweeper Light plugin
Copyright (C) 2002-2003  Sergey V. Gershovich
Copyright (C) 2006-2009  Boris Krasnovskiy
Copyright (C) 2010, 2011 tico-tico

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

#include "historysweeperlight.h"

// Time Stamps strings
char* time_stamp_strings[] =
{
	LPGEN("Delete older than 1 day"),
	LPGEN("Delete older than 3 days"),
	LPGEN("Delete older than 7 days"),
	LPGEN("Delete older than 2 weeks (14 days)"),
	LPGEN("Delete older than 1 month (30 days)"),
	LPGEN("Delete older than 3 months (90 days)"),
	LPGEN("Delete older than 6 months (180 days)"),
	LPGEN("Delete older than 1 year (365 days)")
};

char* keep_strings[] =
{
	LPGEN("Keep 1 last event"),
	LPGEN("Keep 2 last events"),
	LPGEN("Keep 5 last events"),
	LPGEN("Keep 10 last events"),
	LPGEN("Keep 20 last events"),
	LPGEN("Keep 50 last events")
};

static IconItem iconList[] =
{
	{ LPGEN("Default Action"), "actG", IDI_ACTG },
	{ LPGEN("Action 1"), "act1", IDI_ACT1 },
	{ LPGEN("Action 2"), "act2", IDI_ACT2 },
	{ LPGEN("Delete All"), "actDel", IDI_ACTDEL }
};

static HANDLE hIconLibItem[SIZEOF(iconList)];

void InitIcons(void)
{
	Icon_Register(hInst, ModuleName, iconList, SIZEOF(iconList), ModuleName);
}

HICON LoadIconEx(const char* name)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", ModuleName, name);
	return Skin_GetIcon(szSettingName);
}

HANDLE GetIconHandle(const char* name)
{
	for (int i = 0; i < SIZEOF(iconList); i++)
		if (mir_strcmp(iconList[i].szName, name) == 0)
			return hIconLibItem[i];

	return NULL;
}

void  ReleaseIconEx(const char* name)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", ModuleName, name);
	Skin_ReleaseIcon(szSettingName);
}

HANDLE hAllContacts, hSystemHistory;

static void ShowAllContactIcons(HWND hwndList)
{
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hAllContacts,
		MAKELPARAM(0, db_get_b(NULL, ModuleName, "SweepHistory", 0)));
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hSystemHistory,
		MAKELPARAM(0, db_get_b(NULL, ModuleName, "SweepSHistory", 0)));

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
		SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem,
			MAKELPARAM(0, db_get_b(hContact, ModuleName, "SweepHistory", 0)));
	}
}

void LoadSettings(HWND hwndDlg)
{
	int i;
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);

	CLCINFOITEM cii = { 0 };
	cii.cbSize = sizeof(cii);
	cii.flags = CLCIIF_GROUPFONT;

	cii.pszText = TranslateT("*** Default Action ***");
	hAllContacts = (HANDLE)SendMessage(hwndList, CLM_ADDINFOITEM, 0, (LPARAM)&cii);

	cii.pszText = TranslateT("*** System History ***");
	hSystemHistory = (HANDLE)SendMessage(hwndList, CLM_ADDINFOITEM, 0, (LPARAM)&cii);

	ShowAllContactIcons(hwndList);

	SendDlgItemMessage(hwndDlg, IDC_SSOLDER, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(hwndDlg, IDC_SSKEEP, CB_RESETCONTENT, 0, 0);

	for (i = 0; i < SIZEOF(time_stamp_strings); i++) {
		TCHAR* ptszTimeStr = (TCHAR*)CallService(MS_LANGPACK_PCHARTOTCHAR, 0, (LPARAM)time_stamp_strings[i]);
		SendDlgItemMessage(hwndDlg, IDC_SSOLDER, CB_ADDSTRING, 0, (LPARAM)ptszTimeStr);
		mir_free(ptszTimeStr);
	}

	for (i = 0; i < SIZEOF(keep_strings); i++) {
		TCHAR* ptszTimeStr = (TCHAR*)CallService(MS_LANGPACK_PCHARTOTCHAR, 0, (LPARAM)keep_strings[i]);
		SendDlgItemMessage(hwndDlg, IDC_SSKEEP, CB_ADDSTRING, 0, (LPARAM)ptszTimeStr);
		mir_free(ptszTimeStr);
	}

	SendDlgItemMessage(hwndDlg, IDC_SSOLDER, CB_SETCURSEL, db_get_b(NULL, ModuleName, "StartupShutdownOlder", 0), 0);
	SendDlgItemMessage(hwndDlg, IDC_SSKEEP, CB_SETCURSEL, db_get_b(NULL, ModuleName, "StartupShutdownKeep", 0), 0);

	CheckDlgButton(hwndDlg, IDC_UNSAFEMODE, db_get_b(NULL, ModuleName, "UnsafeMode", 0) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SWEEPONCLOSE, db_get_b(NULL, ModuleName, "SweepOnClose", 0) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_HISTMW, db_get_b(NULL, ModuleName, "ChangeInMW", 0) ? BST_CHECKED : BST_UNCHECKED);
}//LoadSettings


void SaveSettings(HWND hwndDlg)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);

	db_set_b(NULL, ModuleName, "StartupShutdownOlder", (BYTE)SendDlgItemMessage(hwndDlg, IDC_SSOLDER, CB_GETCURSEL, 0, 0));
	db_set_b(NULL, ModuleName, "StartupShutdownKeep", (BYTE)SendDlgItemMessage(hwndDlg, IDC_SSKEEP, CB_GETCURSEL, 0, 0));
	db_set_b(NULL, ModuleName, "UnsafeMode", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_UNSAFEMODE));
	db_set_b(NULL, ModuleName, "SweepOnClose", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SWEEPONCLOSE));
	db_set_b(NULL, ModuleName, "ChangeInMW", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_HISTMW));

	db_set_b(NULL, ModuleName, "SweepHistory",
		(BYTE)SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hAllContacts, 0));
	db_set_b(NULL, ModuleName, "SweepSHistory",
		(BYTE)SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hSystemHistory, 0));

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);

		int st = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, 0);
		if (st != 0)
			db_set_b(hContact, ModuleName, "SweepHistory", (BYTE)st);
		else
			db_unset(hContact, ModuleName, "SweepHistory");
	}

	for (int i = 0; i < g_hWindows.getCount(); i++)
		SetSrmmIcon(MCONTACT(g_hWindows[i]));

	// set tooltips
	int st = db_get_b(NULL, ModuleName, "SweepHistory", 0);

	StatusIconData sid = { 0 };
	sid.cbSize = sizeof(sid);
	sid.szModule = ModuleName;
	sid.dwId = 0;
	sid.hIcon = LoadIconEx("actG");
	if (st == 0)	sid.szTooltip = LPGEN("Keep all events");
	else if (st == 1)	sid.szTooltip = LPGEN(time_stamp_strings[db_get_b(NULL, ModuleName, "StartupShutdownOlder", 0)]);
	else if (st == 2)	sid.szTooltip = LPGEN(keep_strings[db_get_b(NULL, ModuleName, "StartupShutdownKeep", 0)]);
	else if (st == 3)	sid.szTooltip = LPGEN("Delete all events");
	Srmm_ModifyIcon(NULL, &sid);

	sid.dwId = 1;
	sid.hIcon = LoadIconEx("act1");
	sid.szTooltip = time_stamp_strings[db_get_b(NULL, ModuleName, "StartupShutdownOlder", 0)];
	Srmm_ModifyIcon(NULL, &sid);

	sid.dwId = 2;
	sid.hIcon = LoadIconEx("act2");
	sid.szTooltip = keep_strings[db_get_b(NULL, ModuleName, "StartupShutdownKeep", 0)];
	Srmm_ModifyIcon(NULL, &sid);
}

INT_PTR CALLBACK DlgProcHSOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			HIMAGELIST hIml = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK | ILC_COLOR32, 2, 2);

			HICON hIcon = LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT);
			ImageList_AddIcon(hIml, hIcon);
			Skin_ReleaseIcon(hIcon);

			hIcon = LoadIconEx("act1");
			ImageList_AddIcon(hIml, hIcon);
			SendDlgItemMessage(hwndDlg, IDC_ACT1, STM_SETICON, (WPARAM)hIcon, 0);

			hIcon = LoadIconEx("act2");
			ImageList_AddIcon(hIml, hIcon);
			SendDlgItemMessage(hwndDlg, IDC_ACT2, STM_SETICON, (WPARAM)hIcon, 0);

			hIcon = LoadIconEx("actDel");
			ImageList_AddIcon(hIml, hIcon);

			SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hIml);
			SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRACOLUMNS, 1, 0);
		}
		LoadSettings(hwndDlg);
		return TRUE;

	case WM_DESTROY:
		ImageList_Destroy((HIMAGELIST)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETEXTRAIMAGELIST, 0, 0));
		ReleaseIconEx("act1");
		ReleaseIconEx("act2");
		ReleaseIconEx("actDel");
		break;

	case WM_COMMAND:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0); // Send message to activate "Apply" button
		return TRUE;

	case WM_NOTIFY:
		NMCLISTCONTROL *nmc = (NMCLISTCONTROL*)lParam;
		if (nmc->hdr.idFrom == 0 && nmc->hdr.code == (unsigned)PSN_APPLY)
			SaveSettings(hwndDlg);
		else if (nmc->hdr.idFrom == IDC_LIST) {
			switch (nmc->hdr.code) {
			case CLN_NEWCONTACT:
			case CLN_LISTREBUILT:
				ShowAllContactIcons(nmc->hdr.hwndFrom);
				break;

			case NM_CLICK:
				HANDLE hItem; DWORD hitFlags; int iImage;

				if (nmc->iColumn == -1)
					break;

				// Find clicked item
				hItem = (HANDLE)SendMessage(nmc->hdr.hwndFrom, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nmc->pt.x, nmc->pt.y));
				// Nothing was clicked
				if (hItem == NULL || !(IsHContactContact(hItem) || IsHContactInfo(hItem)))
					break;

				// It was not our extended icon
				if (!(hitFlags & CLCHT_ONITEMEXTRA))
					break;

				iImage = SendMessage(nmc->hdr.hwndFrom, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nmc->iColumn, 0));
				SendMessage(nmc->hdr.hwndFrom, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nmc->iColumn, (iImage + 1) % 4));

				// Activate Apply button
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;
	}
	return FALSE;
}

int HSOptInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_HISTORYSWEEPER);
	odp.pszTitle = ModuleName;
	odp.pszGroup = LPGEN("History");
	odp.pfnDlgProc = DlgProcHSOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}

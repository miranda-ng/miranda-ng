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

#include "stdafx.h"

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

static HANDLE hIconLibItem[_countof(iconList)];

void InitIcons(void)
{
	g_plugin.registerIcon(MODULENAME, iconList, MODULENAME);
}

HICON LoadIconEx(const char* name)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, "%s_%s", MODULENAME, name);
	return IcoLib_GetIcon(szSettingName);
}

HANDLE GetIconHandle(const char* name)
{
	for (auto &it : iconList)
		if (mir_strcmp(it.szName, name) == 0)
			return &it;

	return nullptr;
}

void  ReleaseIconEx(const char* name)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, "%s_%s", MODULENAME, name);
	IcoLib_Release(szSettingName);
}

HANDLE hAllContacts, hSystemHistory;

static void ShowAllContactIcons(HWND hwndList)
{
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hAllContacts,
		MAKELPARAM(0, g_plugin.getByte("SweepHistory", 0)));
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hSystemHistory,
		MAKELPARAM(0, g_plugin.getByte("SweepSHistory", 0)));

	for (auto &hContact : Contacts()) {
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
		SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem,
			MAKELPARAM(0, g_plugin.getByte(hContact, "SweepHistory")));
	}
}

void LoadSettings(HWND hwndDlg)
{
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

	for (auto &it : time_stamp_strings) {
		ptrW ptszTimeStr(Langpack_PcharToTchar(it));
		SendDlgItemMessage(hwndDlg, IDC_SSOLDER, CB_ADDSTRING, 0, (LPARAM)ptszTimeStr);
	}

	for (auto &it : keep_strings) {
		ptrW ptszTimeStr(Langpack_PcharToTchar(it));
		SendDlgItemMessage(hwndDlg, IDC_SSKEEP, CB_ADDSTRING, 0, (LPARAM)ptszTimeStr);
	}

	SendDlgItemMessage(hwndDlg, IDC_SSOLDER, CB_SETCURSEL, g_plugin.getByte("StartupShutdownOlder", 0), 0);
	SendDlgItemMessage(hwndDlg, IDC_SSKEEP, CB_SETCURSEL, g_plugin.getByte("StartupShutdownKeep", 0), 0);

	CheckDlgButton(hwndDlg, IDC_UNSAFEMODE, g_plugin.getByte("UnsafeMode", 0) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SWEEPONCLOSE, g_plugin.getByte("SweepOnClose", 0) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_HISTMW, g_plugin.getByte("ChangeInMW", 0) ? BST_CHECKED : BST_UNCHECKED);
}

void SaveSettings(HWND hwndDlg)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);

	g_plugin.setByte("StartupShutdownOlder", (BYTE)SendDlgItemMessage(hwndDlg, IDC_SSOLDER, CB_GETCURSEL, 0, 0));
	g_plugin.setByte("StartupShutdownKeep", (BYTE)SendDlgItemMessage(hwndDlg, IDC_SSKEEP, CB_GETCURSEL, 0, 0));
	g_plugin.setByte("UnsafeMode", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_UNSAFEMODE));
	g_plugin.setByte("SweepOnClose", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SWEEPONCLOSE));
	g_plugin.setByte("ChangeInMW", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_HISTMW));

	g_plugin.setByte("SweepHistory", (BYTE)SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hAllContacts, 0));
	g_plugin.setByte("SweepSHistory", (BYTE)SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hSystemHistory, 0));

	for (auto &hContact : Contacts()) {
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);

		int st = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, 0);
		if (st != 0)
			g_plugin.setByte(hContact, "SweepHistory", (BYTE)st);
		else
			g_plugin.delSetting(hContact, "SweepHistory");
	}

	for (auto &it : g_hWindows)
		SetSrmmIcon(UINT_PTR(it));

	// set tooltips
	int st = g_plugin.getByte("SweepHistory", 0);

	StatusIconData sid = {};
	sid.szModule = MODULENAME;
	sid.dwId = 0;
	sid.hIcon = LoadIconEx("actG");
	if (st == 0)	sid.szTooltip.a = LPGEN("Keep all events");
	else if (st == 1)	sid.szTooltip.a = LPGEN(time_stamp_strings[g_plugin.getByte("StartupShutdownOlder", 0)]);
	else if (st == 2)	sid.szTooltip.a = LPGEN(keep_strings[g_plugin.getByte("StartupShutdownKeep", 0)]);
	else if (st == 3)	sid.szTooltip.a = LPGEN("Delete all events");
	Srmm_ModifyIcon(NULL, &sid);

	sid.dwId = 1;
	sid.hIcon = LoadIconEx("act1");
	sid.szTooltip.a = time_stamp_strings[g_plugin.getByte("StartupShutdownOlder", 0)];
	Srmm_ModifyIcon(NULL, &sid);

	sid.dwId = 2;
	sid.hIcon = LoadIconEx("act2");
	sid.szTooltip.a = keep_strings[g_plugin.getByte("StartupShutdownKeep", 0)];
	Srmm_ModifyIcon(NULL, &sid);
}

INT_PTR CALLBACK DlgProcHSOpts(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			HIMAGELIST hIml = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK | ILC_COLOR32, 2, 2);

			HICON hIcon = Skin_LoadIcon(SKINICON_OTHER_SMALLDOT);
			ImageList_AddIcon(hIml, hIcon);
			IcoLib_ReleaseIcon(hIcon);

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
		NMCLISTCONTROL * nmc = (NMCLISTCONTROL*)lParam;
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
				if (hItem == nullptr || !(IsHContactContact(hItem) || IsHContactInfo(hItem)))
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

int HSOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_HISTORYSWEEPER);
	odp.szTitle.a = MODULENAME;
	odp.szGroup.a = LPGEN("History");
	odp.pfnDlgProc = DlgProcHSOpts;
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

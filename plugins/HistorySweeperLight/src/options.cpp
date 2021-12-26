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

/////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////////

// Time Stamps strings
wchar_t* time_stamp_strings[] =
{
	LPGENW("Delete older than 1 day"),
	LPGENW("Delete older than 3 days"),
	LPGENW("Delete older than 7 days"),
	LPGENW("Delete older than 2 weeks (14 days)"),
	LPGENW("Delete older than 1 month (30 days)"),
	LPGENW("Delete older than 3 months (90 days)"),
	LPGENW("Delete older than 6 months (180 days)"),
	LPGENW("Delete older than 1 year (365 days)")
};

wchar_t* keep_strings[] =
{
	LPGENW("Keep 1 last event"),
	LPGENW("Keep 2 last events"),
	LPGENW("Keep 5 last events"),
	LPGENW("Keep 10 last events"),
	LPGENW("Keep 20 last events"),
	LPGENW("Keep 50 last events")
};

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

	for (auto &it : time_stamp_strings)
		SendDlgItemMessage(hwndDlg, IDC_SSOLDER, CB_ADDSTRING, 0, (LPARAM)TranslateW(it));

	for (auto &it : keep_strings)
		SendDlgItemMessage(hwndDlg, IDC_SSKEEP, CB_ADDSTRING, 0, (LPARAM)TranslateW(it));

	SendDlgItemMessage(hwndDlg, IDC_SSOLDER, CB_SETCURSEL, g_plugin.getByte("StartupShutdownOlder", 0), 0);
	SendDlgItemMessage(hwndDlg, IDC_SSKEEP, CB_SETCURSEL, g_plugin.getByte("StartupShutdownKeep", 0), 0);

	CheckDlgButton(hwndDlg, IDC_UNSAFEMODE, g_plugin.getByte("UnsafeMode", 0) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SWEEPONCLOSE, g_plugin.getByte("SweepOnClose", 0) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_HISTMW, g_plugin.getByte("ChangeInMW", 0) ? BST_CHECKED : BST_UNCHECKED);
}

void SaveSettings(HWND hwndDlg)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);

	g_plugin.setByte("StartupShutdownOlder", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_SSOLDER, CB_GETCURSEL, 0, 0));
	g_plugin.setByte("StartupShutdownKeep", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_SSKEEP, CB_GETCURSEL, 0, 0));
	g_plugin.setByte("UnsafeMode", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_UNSAFEMODE));
	g_plugin.setByte("SweepOnClose", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SWEEPONCLOSE));
	g_plugin.setByte("ChangeInMW", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_HISTMW));

	g_plugin.setByte("SweepHistory", (uint8_t)SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hAllContacts, 0));
	g_plugin.setByte("SweepSHistory", (uint8_t)SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hSystemHistory, 0));

	for (auto &hContact : Contacts()) {
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);

		int st = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, 0);
		if (st != 0)
			g_plugin.setByte(hContact, "SweepHistory", (uint8_t)st);
		else
			g_plugin.delSetting(hContact, "SweepHistory");
	}

	for (auto &it : g_hWindows)
		SetSrmmIcon(UINT_PTR(it));

	// set tooltips
	int st = g_plugin.getByte("SweepHistory", 0);

	const wchar_t *pwszToolTip = nullptr;
	if (st == 0)
		pwszToolTip = LPGENW("Keep all events");
	else if (st == 1)
		pwszToolTip = time_stamp_strings[g_plugin.getByte("StartupShutdownOlder", 0)];
	else if (st == 2)
		pwszToolTip = keep_strings[g_plugin.getByte("StartupShutdownKeep", 0)];
	else if (st == 3)
		pwszToolTip = LPGENW("Delete all events");
	Srmm_ModifyIcon(NULL, MODULENAME, 0, nullptr, pwszToolTip);

	Srmm_ModifyIcon(NULL, MODULENAME, 1, nullptr, time_stamp_strings[g_plugin.getByte("StartupShutdownOlder", 0)]);
	Srmm_ModifyIcon(NULL, MODULENAME, 2, nullptr, keep_strings[g_plugin.getByte("StartupShutdownKeep", 0)]);
}

INT_PTR CALLBACK DlgProcHSOpts(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			HIMAGELIST hIml = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK | ILC_COLOR32, 2, 2);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_SMALLDOT);

			HICON hIcon = g_plugin.getIcon(IDI_ACT1);
			ImageList_AddIcon(hIml, hIcon);
			SendDlgItemMessage(hwndDlg, IDC_ACT1, STM_SETICON, (WPARAM)hIcon, 0);

			hIcon = g_plugin.getIcon(IDI_ACT2);
			ImageList_AddIcon(hIml, hIcon);
			SendDlgItemMessage(hwndDlg, IDC_ACT2, STM_SETICON, (WPARAM)hIcon, 0);

			hIcon = g_plugin.getIcon(IDI_ACTDEL);
			ImageList_AddIcon(hIml, hIcon);

			SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hIml);
			SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRACOLUMNS, 1, 0);
		}
		LoadSettings(hwndDlg);
		return TRUE;

	case WM_DESTROY:
		ImageList_Destroy((HIMAGELIST)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETEXTRAIMAGELIST, 0, 0));
		g_plugin.releaseIcon(IDI_ACT1);
		g_plugin.releaseIcon(IDI_ACT2);
		g_plugin.releaseIcon(IDI_ACTDEL);
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
				HANDLE hItem; uint32_t hitFlags; int iImage;

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

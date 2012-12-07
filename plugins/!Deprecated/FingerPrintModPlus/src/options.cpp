/*
Fingerprint Mod+ (client version) icons module for Miranda IM

Copyright © 2006-2007 Artem Shpynov aka FYR, Bio, Faith Healer. 2009-2010 HierOS

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

//Start of header
#include "global.h"

struct _settingsInfo
{
	int idCtrl;
	LPCSTR szSetName;
}
static settings [] =
{
	{IDC_OVERLAY1, "Overlay1"},
	{IDC_OVERLAY2, "Overlay2"},
	{IDC_OVERLAY3, "Overlay3"},
	{IDC_VERSION, "ShowVersion"},

	{IDC_GROUPMIRANDA, "GroupMiranda"},
	{IDC_GROUPMULTI, "GroupMulti"},
	{IDC_GROUPPACKS, "GroupPacks"},
	{IDC_GROUPOTHERS, "GroupOtherProtos"},

	{IDC_GROUPAIM, "GroupAim"},
	{IDC_GROUPGADU, "GroupGadu"},
	{IDC_GROUPICQ, "GroupIcq"},
	{IDC_GROUPIRC, "GroupIrc"},
	{IDC_GROUPJABBER, "GroupJabber"},
	{IDC_GROUPMAIL, "GroupMail"},
	{IDC_GROUPMSN, "GroupMsn"},
	{IDC_GROUPQQ, "GroupQQ"},
	{IDC_GROUPRSS, "GroupRSS"},
	{IDC_GROUPSKYPE, "GroupSkype"},
	{IDC_GROUPTLEN, "GroupTlen"},
	{IDC_GROUPVOIP, "GroupVoIP"},
	{IDC_GROUPWEATHER, "GroupWeather"},
	{IDC_GROUPYAHOO, "GroupYahoo"}
};

static void OptDlgChanged(HWND hwndDlg, BOOL show)
{
	if (show)
		ShowWindow(GetDlgItem(hwndDlg, IDC_OPTCHANGENOTE), SW_SHOW);
	SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
}

static void LoadDBCheckState(HWND hwndDlg, int idCtrl, LPCSTR szSetting, BYTE bDef)
{
	CheckDlgButton(hwndDlg, idCtrl, db_get_b(NULL, "Finger", szSetting, bDef));
}

static void StoreDBCheckState(HWND hwndDlg, int idCtrl, LPCSTR szSetting)
{
	db_set_b(NULL, "Finger", szSetting, (BYTE)IsDlgButtonChecked(hwndDlg, idCtrl));
}

static INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			for (int i = 0; i < SIZEOF(settings); i++) {
				if (lstrcmpA(settings[i].szSetName, "ShowVersion") == 0)
					LoadDBCheckState(hwndDlg, settings[i].idCtrl, settings[i].szSetName, 0);
				else
					LoadDBCheckState(hwndDlg, settings[i].idCtrl, settings[i].szSetName, 1);
			}

			ShowWindow(GetDlgItem(hwndDlg, IDC_OPTCHANGENOTE), SW_HIDE);
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_OVERLAY1:
		case IDC_OVERLAY2:
		case IDC_OVERLAY3:
		case IDC_VERSION:
			OptDlgChanged(hwndDlg, true);
			break;

		case IDC_GROUPMIRANDA:
		case IDC_GROUPMULTI:
		case IDC_GROUPPACKS:
		case IDC_GROUPOTHERS:
		case IDC_GROUPAIM:
		case IDC_GROUPGADU:
		case IDC_GROUPICQ:
		case IDC_GROUPIRC:
		case IDC_GROUPJABBER:
		case IDC_GROUPMAIL:
		case IDC_GROUPMSN:
		case IDC_GROUPQQ:
		case IDC_GROUPRSS:
		case IDC_GROUPSKYPE:
		case IDC_GROUPTLEN:
		case IDC_GROUPVOIP:
		case IDC_GROUPWEATHER:
		case IDC_GROUPYAHOO:
			OptDlgChanged(hwndDlg, false);
			break;

		default:
			return 0;
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR *)lParam;
			if (hdr && hdr->code == PSN_APPLY) {
				for (int i = 0; i < SIZEOF(settings); i++)
					StoreDBCheckState(hwndDlg, settings[i].idCtrl, settings[i].szSetName);

				ClearFI();
				RegisterIcons();
			}
		}
		break;
	}
	return FALSE;
}

int OnOptInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.hInstance = g_hInst;
	odp.ptszGroup = LPGENT("Icons");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_DIALOG);
	odp.ptszTitle = LPGENT("Fingerprint");
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_BOLDGROUPS|ODPF_TCHAR;;
	Options_AddPage(wParam, &odp);
	return 0;
}

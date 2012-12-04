/*
Fingerprint NG (client version) icons module for Miranda NG

Copyright © 2006-12 ghazan, mataes, HierOS, FYR, Bio, nullbie, faith_healer and all respective contributors.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

//Start of header
#include "global.h"

struct
{
	int idCtrl;
	LPCSTR szSetName;
}
static settings[] =
{
	{IDC_GROUP_MIRANDA,           "GroupMiranda"},
	{IDC_GROUP_MIRANDA_VERSION,   "GroupMirandaVersion"},
	{IDC_GROUP_MIRANDA_PACKS,     "GroupMirandaPacks"},
	
	{IDC_GROUP_MULTI,             "GroupMulti"},
	{IDC_GROUP_AIM,               "GroupAIM"},
	{IDC_GROUP_GG,                "GroupGG"},
	{IDC_GROUP_ICQ,               "GroupICQ"},
	{IDC_GROUP_IRC,               "GroupIRC"},
	{IDC_GROUP_JABBER,            "GroupJabber"},
	{IDC_GROUP_MRA,               "GroupMRA"},
	{IDC_GROUP_MSN,               "GroupMSN"},
	{IDC_GROUP_QQ,                "GroupQQ"},
	{IDC_GROUP_RSS,               "GroupRSS"},
	{IDC_GROUP_TLEN,              "GroupTlen"},
	{IDC_GROUP_WEATHER,           "GroupWeather"},
	{IDC_GROUP_YAHOO,             "GroupYahoo"},
	
	{IDC_GROUP_OTHER_PROTOS,      "GroupOtherProtos"},
	{IDC_GROUP_OTHERS,            "GroupOthers"},

	{IDC_GROUP_OVERLAYS_RESOURCE, "GroupOverlaysResource"},
	{IDC_GROUP_OVERLAYS_PLATFORM, "GroupOverlaysPlatform"},
	{IDC_GROUP_OVERLAYS_PROTO,    "GroupOverlaysProtos"},
//	{IDC_GROUP_OVERLAYS_SECURITY, "GroupOtherProtos"}
};

static void OptDlgChanged(HWND hwndDlg, BOOL show)
{
	if (show)
		ShowWindow(GetDlgItem(hwndDlg, IDC_OPTCHANGENOTE), SW_SHOW);
	SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
}

static void LoadDBCheckState(HWND hwndDlg, int idCtrl, LPCSTR szSetting, BYTE bDef)
{
	CheckDlgButton(hwndDlg, idCtrl, db_get_b(NULL,	"Finger", szSetting, bDef));
}

static void StoreDBCheckState(HWND hwndDlg, int idCtrl, LPCSTR szSetting)
{
	db_set_b(NULL,	"Finger", szSetting, (BYTE)IsDlgButtonChecked(hwndDlg, idCtrl));
}

static INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			for (int i = 0; i < SIZEOF(settings); i++) {
				if (lstrcmpA(settings[i].szSetName,	"GroupMirandaVersion") == 0)
					LoadDBCheckState(hwndDlg, settings[i].idCtrl, settings[i].szSetName, 0);
				else
					LoadDBCheckState(hwndDlg, settings[i].idCtrl, settings[i].szSetName, 1);
			}

			ShowWindow(GetDlgItem(hwndDlg, IDC_OPTCHANGENOTE), SW_HIDE);
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		
		case IDC_GROUP_MIRANDA:
		case IDC_GROUP_MIRANDA_VERSION:
		case IDC_GROUP_MIRANDA_PACKS:
			OptDlgChanged(hwndDlg, true);
			break;

		case IDC_GROUP_MULTI:
		case IDC_GROUP_AIM:
		case IDC_GROUP_GG:
		case IDC_GROUP_ICQ:
		case IDC_GROUP_IRC:
		case IDC_GROUP_JABBER:
		case IDC_GROUP_MRA:
		case IDC_GROUP_MSN:
		case IDC_GROUP_QQ:
		case IDC_GROUP_RSS:
		case IDC_GROUP_TLEN:
		case IDC_GROUP_WEATHER:
		case IDC_GROUP_YAHOO:
		
		case IDC_GROUP_OTHER_PROTOS:
		case IDC_GROUP_OTHERS:
		
		case IDC_GROUP_OVERLAYS_RESOURCE:
		case IDC_GROUP_OVERLAYS_PLATFORM:
		case IDC_GROUP_OVERLAYS_PROTO:
//		case IDC_GROUP_OVERLAYS_SECURITY:
			OptDlgChanged(hwndDlg, false);
			break;

		default:
			return 0;
		}
		break;

	case WM_NOTIFY:
		NMHDR *hdr = (NMHDR *)lParam;
		if (hdr && hdr->code == PSN_APPLY) {
			for (int i = 0; i < SIZEOF(settings); i++)
				StoreDBCheckState(hwndDlg, settings[i].idCtrl, settings[i].szSetName);

			ClearFI();
			RegisterIcons();

			for (HANDLE hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
				OnExtraImageApply((WPARAM)hContact, 0);
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
	odp.ptszGroup = LPGENT("Customize");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_DIALOG);
	odp.ptszTitle = LPGENT("Fingerprint");
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_BOLDGROUPS|ODPF_TCHAR;;
	Options_AddPage(wParam, &odp);
	return 0;
}

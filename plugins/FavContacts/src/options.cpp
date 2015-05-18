/*
Favorite Contacts for Miranda IM

Copyright 2007 Victor Pavlychko

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

void LoadOptions()
{
	g_Options.bSecondLine = db_get_b(NULL, "FavContacts", "SecondLine", 1);
	g_Options.bAvatars = db_get_b(NULL, "FavContacts", "Avatars", 1);
	g_Options.bAvatarBorder = db_get_b(NULL, "FavContacts", "AvatarBorder", 0);
	g_Options.wAvatarRadius = db_get_w(NULL, "FavContacts", "AvatarRadius", 3);
	g_Options.bNoTransparentBorder = db_get_b(NULL, "FavContacts", "NoTransparentBorder",
															!db_get_b(NULL, "FavContacts", "AvatarBorderTransparent", 1));
	g_Options.bSysColors = db_get_b(NULL, "FavContacts", "SysColors", 0);
	g_Options.bCenterHotkey = db_get_b(NULL, "FavContacts", "CenterHotkey", 1);
	g_Options.bUseGroups = db_get_b(NULL, "FavContacts", "UseGroups", 0);
	g_Options.bUseColumns = db_get_b(NULL, "FavContacts", "UseColumns", 1);
	g_Options.bRightAvatars = db_get_b(NULL, "FavContacts", "RightAvatars", 0);
	g_Options.bDimIdle = db_get_b(NULL, "FavContacts", "DimIdle", 1);

	g_Options.wMaxRecent = db_get_b(NULL, "FavContacts", "MaxRecent", 10);
}

static void sttSaveOptions()
{
	db_set_b(NULL, "FavContacts", "SecondLine", g_Options.bSecondLine);
	db_set_b(NULL, "FavContacts", "Avatars", g_Options.bAvatars);
	db_set_b(NULL, "FavContacts", "AvatarBorder", g_Options.bAvatarBorder);
	db_set_w(NULL, "FavContacts", "AvatarRadius", g_Options.wAvatarRadius);
	db_set_b(NULL, "FavContacts", "NoTransparentBorder", g_Options.bNoTransparentBorder);
	db_set_b(NULL, "FavContacts", "SysColors", g_Options.bSysColors);
	db_set_b(NULL, "FavContacts", "CenterHotkey", g_Options.bCenterHotkey);
	db_set_b(NULL, "FavContacts", "UseGroups", g_Options.bUseGroups);
	db_set_b(NULL, "FavContacts", "UseColumns", g_Options.bUseColumns);
	db_set_b(NULL, "FavContacts", "RightAvatars", g_Options.bRightAvatars);
	db_set_b(NULL, "FavContacts", "DimIdle", g_Options.bDimIdle);
	db_set_w(NULL, "FavContacts", "MaxRecent", g_Options.wMaxRecent);
}

static void sttResetListOptions(HWND hwndList)
{
	SendMessage(hwndList, CLM_SETHIDEEMPTYGROUPS, 1, 0);
	SendMessage(hwndList, CLM_SETHIDEOFFLINEROOT, 1, 0);
}

static INT_PTR CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bInitialized = false;
	static MCONTACT hSelectedContact = 0;

	switch (msg) {
	case WM_INITDIALOG:
		bInitialized = false;

		TranslateDialogDefault(hwnd);

		CheckDlgButton(hwnd, IDC_CHK_GROUPS, g_Options.bUseGroups ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_GROUPCOLUMS, g_Options.bUseColumns ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_SECONDLINE, g_Options.bSecondLine ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_AVATARS, g_Options.bAvatars ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_AVATARBORDER, g_Options.bAvatarBorder ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_NOTRANSPARENTBORDER, g_Options.bNoTransparentBorder ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_SYSCOLORS, g_Options.bSysColors ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_CENTERHOTKEY, g_Options.bCenterHotkey ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_RIGHTAVATARS, g_Options.bRightAvatars ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_DIMIDLE, g_Options.bDimIdle ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hwnd, IDC_TXT_RADIUS, g_Options.wAvatarRadius, FALSE);
		SetDlgItemInt(hwnd, IDC_TXT_MAXRECENT, g_Options.wMaxRecent, FALSE);

		SetWindowLongPtr(GetDlgItem(hwnd, IDC_CLIST), GWL_STYLE,
							  GetWindowLongPtr(GetDlgItem(hwnd, IDC_CLIST), GWL_STYLE) | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE | CLS_GROUPCHECKBOXES);
		SendDlgItemMessage(hwnd, IDC_CLIST, CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);
		sttResetListOptions(GetDlgItem(hwnd, IDC_CLIST));

		hSelectedContact = db_find_first();
		{
			for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
				SendDlgItemMessage(hwnd, IDC_CLIST, CLM_SETCHECKMARK,
				SendDlgItemMessage(hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0),
				db_get_b(hContact, "FavContacts", "IsFavourite", 0));
		}

		bInitialized = true;
		PostMessage(hwnd, WM_APP, 0, 0);
		return TRUE;

	case WM_APP:
		{
			BOOL bGroups = IsDlgButtonChecked(hwnd, IDC_CHK_GROUPS);
			EnableWindow(GetDlgItem(hwnd, IDC_CHK_GROUPCOLUMS), bGroups);

			BOOL bAvatars = IsDlgButtonChecked(hwnd, IDC_CHK_AVATARS);
			BOOL bBorders = IsDlgButtonChecked(hwnd, IDC_CHK_AVATARBORDER);
			EnableWindow(GetDlgItem(hwnd, IDC_CHK_AVATARBORDER), bAvatars);
			EnableWindow(GetDlgItem(hwnd, IDC_CHK_RIGHTAVATARS), bAvatars);
			EnableWindow(GetDlgItem(hwnd, IDC_CHK_NOTRANSPARENTBORDER), bAvatars && bBorders);
			EnableWindow(GetDlgItem(hwnd, IDC_TXT_RADIUS), bAvatars && bBorders);
		}
		return TRUE;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
			if (lpdis->CtlID == IDC_CANVAS) {
				MEASUREITEMSTRUCT mis = { 0 };
				DRAWITEMSTRUCT dis = *lpdis;

				FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(COLOR_BTNFACE));
				if (hSelectedContact) {
					Options options;
					options.bSecondLine = IsDlgButtonChecked(hwnd, IDC_CHK_SECONDLINE);
					options.bAvatars = IsDlgButtonChecked(hwnd, IDC_CHK_AVATARS);
					options.bAvatarBorder = IsDlgButtonChecked(hwnd, IDC_CHK_AVATARBORDER);
					options.bNoTransparentBorder = IsDlgButtonChecked(hwnd, IDC_CHK_NOTRANSPARENTBORDER);
					options.bSysColors = IsDlgButtonChecked(hwnd, IDC_CHK_SYSCOLORS);
					options.bCenterHotkey = IsDlgButtonChecked(hwnd, IDC_CHK_CENTERHOTKEY);
					options.bRightAvatars = IsDlgButtonChecked(hwnd, IDC_CHK_RIGHTAVATARS);
					options.bDimIdle = IsDlgButtonChecked(hwnd, IDC_CHK_DIMIDLE);
					options.wAvatarRadius = GetDlgItemInt(hwnd, IDC_TXT_RADIUS, NULL, FALSE);
					options.wMaxRecent = GetDlgItemInt(hwnd, IDC_TXT_MAXRECENT, NULL, FALSE);

					mis.CtlID = 0;
					mis.CtlType = ODT_MENU;
					mis.itemData = (DWORD)hSelectedContact;
					MenuMeasureItem(&mis, &options);
					dis.rcItem.bottom = dis.rcItem.top + mis.itemHeight;

					dis.CtlID = 0;
					dis.CtlType = ODT_MENU;
					dis.itemData = (DWORD)hSelectedContact;
					MenuDrawItem(&dis, &options);

					RECT rc = lpdis->rcItem;
					rc.bottom = rc.top + mis.itemHeight;
					FrameRect(lpdis->hDC, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
				}
				return TRUE;
			}
		}
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHK_SECONDLINE:
		case IDC_CHK_AVATARS:
		case IDC_CHK_AVATARBORDER:
		case IDC_CHK_NOTRANSPARENTBORDER:
		case IDC_CHK_SYSCOLORS:
		case IDC_CHK_CENTERHOTKEY:
		case IDC_CHK_GROUPS:
		case IDC_CHK_GROUPCOLUMS:
		case IDC_CHK_RIGHTAVATARS:
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			RedrawWindow(GetDlgItem(hwnd, IDC_CANVAS), NULL, NULL, RDW_INVALIDATE);
			PostMessage(hwnd, WM_APP, 0, 0);
			break;

		case IDC_BTN_FONTS:
		{
			OPENOPTIONSDIALOG ood = { sizeof(ood) };
			ood.pszGroup = "Customize";
			ood.pszPage = "Fonts and colors";
			ood.pszTab = NULL;
			Options_Open(&ood);
		}
			break;

		case IDC_TXT_RADIUS:
			if ((HIWORD(wParam) == EN_CHANGE) && bInitialized) {
				RedrawWindow(GetDlgItem(hwnd, IDC_CANVAS), NULL, NULL, RDW_INVALIDATE);
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_TXT_MAXRECENT:
			if ((HIWORD(wParam) == EN_CHANGE) && bInitialized)
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		if ((((LPNMHDR)lParam)->idFrom == 0) && (((LPNMHDR)lParam)->code == PSN_APPLY)) {
			g_Options.bSecondLine = IsDlgButtonChecked(hwnd, IDC_CHK_SECONDLINE);
			g_Options.bAvatars = IsDlgButtonChecked(hwnd, IDC_CHK_AVATARS);
			g_Options.bAvatarBorder = IsDlgButtonChecked(hwnd, IDC_CHK_AVATARBORDER);
			g_Options.bNoTransparentBorder = IsDlgButtonChecked(hwnd, IDC_CHK_NOTRANSPARENTBORDER);
			g_Options.bSysColors = IsDlgButtonChecked(hwnd, IDC_CHK_SYSCOLORS);
			g_Options.bCenterHotkey = IsDlgButtonChecked(hwnd, IDC_CHK_CENTERHOTKEY);
			g_Options.bUseGroups = IsDlgButtonChecked(hwnd, IDC_CHK_GROUPS);
			g_Options.bUseColumns = IsDlgButtonChecked(hwnd, IDC_CHK_GROUPCOLUMS);
			g_Options.bRightAvatars = IsDlgButtonChecked(hwnd, IDC_CHK_RIGHTAVATARS);
			g_Options.bDimIdle = IsDlgButtonChecked(hwnd, IDC_CHK_DIMIDLE);
			g_Options.wAvatarRadius = GetDlgItemInt(hwnd, IDC_TXT_RADIUS, NULL, FALSE);
			g_Options.wMaxRecent = GetDlgItemInt(hwnd, IDC_TXT_MAXRECENT, NULL, FALSE);

			sttSaveOptions();

			for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
				BYTE fav = SendDlgItemMessage(hwnd, IDC_CLIST, CLM_GETCHECKMARK,
														SendDlgItemMessage(hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0), 0);
				if (fav != db_get_b(hContact, "FavContacts", "IsFavourite", 0))
					db_set_b(hContact, "FavContacts", "IsFavourite", fav);
				if (fav) CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
			}
		}
		else if (((LPNMHDR)lParam)->idFrom == IDC_CLIST) {
			int iSelection;

			switch (((LPNMHDR)lParam)->code) {
			case CLN_OPTIONSCHANGED:
				sttResetListOptions(GetDlgItem(hwnd, IDC_CLIST));
				break;

			case CLN_NEWCONTACT:
				iSelection = (int)((NMCLISTCONTROL *)lParam)->hItem;
				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
					if (SendDlgItemMessage(hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0) == iSelection) {
						SendDlgItemMessage(hwnd, IDC_CLIST, CLM_SETCHECKMARK, iSelection,
												 db_get_b(hContact, "FavContacts", "IsFavourite", 0));
						break;
					}
				}
				break;

			case CLN_CHECKCHANGED:
				iSelection = (int)((NMCLISTCONTROL *)lParam)->hItem;
				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
					if (SendDlgItemMessage(hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0) == iSelection) {
						hSelectedContact = hContact;
						RedrawWindow(GetDlgItem(hwnd, IDC_CANVAS), NULL, NULL, RDW_INVALIDATE);
					}
				}
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
		}
		break;
	}

	return FALSE;
}

int ProcessOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 100000000;
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszGroup = LPGEN("Contacts");
	odp.pszTitle = LPGEN("Favorites");
	odp.groupPosition = 910000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = OptionsDlgProc;
	Options_AddPage(wParam, &odp);
	return 0;
}

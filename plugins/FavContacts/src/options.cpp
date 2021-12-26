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
	g_Options.bSecondLine = g_plugin.getByte("SecondLine", 1);
	g_Options.bAvatars = g_plugin.getByte("Avatars", 1);
	g_Options.bAvatarBorder = g_plugin.getByte("AvatarBorder", 0);
	g_Options.wAvatarRadius = g_plugin.getWord("AvatarRadius", 3);
	g_Options.bNoTransparentBorder = g_plugin.getByte("NoTransparentBorder",
		!g_plugin.getByte("AvatarBorderTransparent", 1));
	g_Options.bSysColors = g_plugin.getByte("SysColors", 0);
	g_Options.bCenterHotkey = g_plugin.getByte("CenterHotkey", 1);
	g_Options.bUseGroups = g_plugin.getByte("UseGroups", 0);
	g_Options.bUseColumns = g_plugin.getByte("UseColumns", 1);
	g_Options.bRightAvatars = g_plugin.getByte("RightAvatars", 0);
	g_Options.bDimIdle = g_plugin.getByte("DimIdle", 1);

	g_Options.wMaxRecent = g_plugin.getByte("MaxRecent", 10);
}

static void sttSaveOptions()
{
	g_plugin.setByte("SecondLine", g_Options.bSecondLine);
	g_plugin.setByte("Avatars", g_Options.bAvatars);
	g_plugin.setByte("AvatarBorder", g_Options.bAvatarBorder);
	g_plugin.setWord("AvatarRadius", g_Options.wAvatarRadius);
	g_plugin.setByte("NoTransparentBorder", g_Options.bNoTransparentBorder);
	g_plugin.setByte("SysColors", g_Options.bSysColors);
	g_plugin.setByte("CenterHotkey", g_Options.bCenterHotkey);
	g_plugin.setByte("UseGroups", g_Options.bUseGroups);
	g_plugin.setByte("UseColumns", g_Options.bUseColumns);
	g_plugin.setByte("RightAvatars", g_Options.bRightAvatars);
	g_plugin.setByte("DimIdle", g_Options.bDimIdle);
	g_plugin.setWord("MaxRecent", g_Options.wMaxRecent);
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
			for (auto &hContact : Contacts())
				SendDlgItemMessage(hwnd, IDC_CLIST, CLM_SETCHECKMARK,
					SendDlgItemMessage(hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0),
					g_plugin.getByte(hContact, "IsFavourite"));
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
				options.wAvatarRadius = GetDlgItemInt(hwnd, IDC_TXT_RADIUS, nullptr, FALSE);
				options.wMaxRecent = GetDlgItemInt(hwnd, IDC_TXT_MAXRECENT, nullptr, FALSE);

				mis.CtlID = 0;
				mis.CtlType = ODT_MENU;
				mis.itemData = (uint32_t)hSelectedContact;
				MenuMeasureItem(&mis, &options);
				dis.rcItem.bottom = dis.rcItem.top + mis.itemHeight;

				dis.CtlID = 0;
				dis.CtlType = ODT_MENU;
				dis.itemData = (uint32_t)hSelectedContact;
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
			RedrawWindow(GetDlgItem(hwnd, IDC_CANVAS), nullptr, nullptr, RDW_INVALIDATE);
			PostMessage(hwnd, WM_APP, 0, 0);
			break;

		case IDC_BTN_FONTS:
			g_plugin.openOptions(L"Customize", L"Fonts and colors");
			break;

		case IDC_TXT_RADIUS:
			if ((HIWORD(wParam) == EN_CHANGE) && bInitialized) {
				RedrawWindow(GetDlgItem(hwnd, IDC_CANVAS), nullptr, nullptr, RDW_INVALIDATE);
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
			g_Options.wAvatarRadius = GetDlgItemInt(hwnd, IDC_TXT_RADIUS, nullptr, FALSE);
			g_Options.wMaxRecent = GetDlgItemInt(hwnd, IDC_TXT_MAXRECENT, nullptr, FALSE);

			sttSaveOptions();

			for (auto &hContact : Contacts()) {
				uint8_t fav = SendDlgItemMessage(hwnd, IDC_CLIST, CLM_GETCHECKMARK,
					SendDlgItemMessage(hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0), 0);
				if (fav != g_plugin.getByte(hContact, "IsFavourite"))
					g_plugin.setByte(hContact, "IsFavourite", fav);
				if (fav)
					CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
			}
		}
		else if (((LPNMHDR)lParam)->idFrom == IDC_CLIST) {
			int iSelection;

			switch (((LPNMHDR)lParam)->code) {
			case CLN_OPTIONSCHANGED:
				sttResetListOptions(GetDlgItem(hwnd, IDC_CLIST));
				break;

			case CLN_NEWCONTACT:
				iSelection = (INT_PTR)((NMCLISTCONTROL *)lParam)->hItem;
				for (auto &hContact : Contacts()) {
					if (SendDlgItemMessage(hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0) == iSelection) {
						SendDlgItemMessage(hwnd, IDC_CLIST, CLM_SETCHECKMARK, iSelection, g_plugin.getByte(hContact, "IsFavourite"));
						break;
					}
				}
				break;

			case CLN_CHECKCHANGED:
				iSelection = (INT_PTR)((NMCLISTCONTROL *)lParam)->hItem;
				for (auto &hContact : Contacts()) {
					if (SendDlgItemMessage(hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0) == iSelection) {
						hSelectedContact = hContact;
						RedrawWindow(GetDlgItem(hwnd, IDC_CANVAS), nullptr, nullptr, RDW_INVALIDATE);
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
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.szGroup.a = LPGEN("Contacts");
	odp.szTitle.a = LPGEN("Favorites");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = OptionsDlgProc;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

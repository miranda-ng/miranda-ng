/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "hdr/modern_commonheaders.h"
#include "m_clui.h"
#include "hdr/modern_clist.h"
#include "hdr/modern_commonprototypes.h"

static INT_PTR CALLBACK DlgProcItemRowOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_MIN_ROW_HEIGHT_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_MIN_ROW_HEIGHT), 0); // set buddy
		SendDlgItemMessage(hwndDlg, IDC_MIN_ROW_HEIGHT_SPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));
		SendDlgItemMessage(hwndDlg, IDC_MIN_ROW_HEIGHT_SPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "CList", "MinRowHeight", CLCDEFAULT_ROWHEIGHT), 0));

		SendDlgItemMessage(hwndDlg, IDC_ROW_BORDER_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_ROW_BORDER), 0); // set buddy
		SendDlgItemMessage(hwndDlg, IDC_ROW_BORDER_SPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));
		SendDlgItemMessage(hwndDlg, IDC_ROW_BORDER_SPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "CList", "RowBorder", SETTING_ROWBORDER_DEFAULT), 0));

		CheckDlgButton(hwndDlg, IDC_VARIABLE_ROW_HEIGHT, db_get_b(NULL, "CList", "VariableRowHeight", SETTING_VARIABLEROWHEIGHT_DEFAULT) == 1);
		CheckDlgButton(hwndDlg, IDC_ALIGN_TO_LEFT, db_get_b(NULL, "CList", "AlignLeftItemsToLeft", SETTING_ALIGNLEFTTOLEFT_DEFAULT) == 1);
		CheckDlgButton(hwndDlg, IDC_ALIGN_TO_RIGHT, db_get_b(NULL, "CList", "AlignRightItemsToRight", SETTING_ALIGNRIGHTORIGHT_DEFAULT) == 1);

		CheckDlgButton(hwndDlg, IDC_MINIMODE, db_get_b(NULL, "CLC", "CompactMode", SETTING_COMPACTMODE_DEFAULT) == 1);

		SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_LEFTMARGIN), 0); // set buddy
		SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(64, 0));
		SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETPOS, 0, MAKELONG(db_get_b(NULL, "CLC", "LeftMargin", CLCDEFAULT_LEFTMARGIN), 0));

		SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_RIGHTMARGIN), 0); // set buddy
		SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(64, 0));
		SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_SETPOS, 0, MAKELONG(db_get_b(NULL, "CLC", "RightMargin", CLCDEFAULT_RIGHTMARGIN), 0));

		// Listbox
		{
			HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_ORDER);
			for (int i = 0; i < NUM_ITEM_TYPE; i++) {
				char tmp[128];
				int type;
				int pos = 0;

				mir_snprintf(tmp, SIZEOF(tmp), "RowPos%d", i);
				type = db_get_w(NULL, "CList", tmp, i);

				switch (type) {
				case ITEM_AVATAR:
					pos = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TranslateT("Avatar"));
					break;
				case ITEM_ICON:
					pos = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TranslateT("Icon"));
					break;
				case ITEM_TEXT:
					pos = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TranslateT("Text"));
					break;
				case ITEM_EXTRA_ICONS:
					pos = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TranslateT("Extra icons"));
					break;
				case ITEM_CONTACT_TIME:
					pos = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TranslateT("Contact time"));
					break;
				}
				SendMessage(hwndList, LB_SETITEMDATA, pos, type);
			}

			// Buttons
			switch (SendMessage(hwndList, LB_GETCURSEL, 0, 0)) {
			case LB_ERR:
				EnableWindow(GetDlgItem(hwndDlg, IDC_UP), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DOWN), FALSE);
				break;
			case 0:
				EnableWindow(GetDlgItem(hwndDlg, IDC_DOWN), FALSE);
				break;
			case 3:
				EnableWindow(GetDlgItem(hwndDlg, IDC_UP), FALSE);
				break;
			}
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_UP) {
			if (HIWORD(wParam) != BN_CLICKED)
				return 0;

			HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_ORDER);
			int pos = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
			if (pos == LB_ERR)
				return 0;
			int type = SendMessage(hwndList, LB_GETITEMDATA, pos, 0);

			// Switch items
			SendMessage(hwndList, LB_DELETESTRING, pos, 0);

			switch (type) {
			case ITEM_AVATAR:
				pos = SendMessage(hwndList, LB_INSERTSTRING, pos - 1, (LPARAM)TranslateT("Avatar"));
				break;
			case ITEM_ICON:
				pos = SendMessage(hwndList, LB_INSERTSTRING, pos - 1, (LPARAM)TranslateT("Icon"));
				break;
			case ITEM_TEXT:
				pos = SendMessage(hwndList, LB_INSERTSTRING, pos - 1, (LPARAM)TranslateT("Text"));
				break;
			case ITEM_EXTRA_ICONS:
				pos = SendMessage(hwndList, LB_INSERTSTRING, pos - 1, (LPARAM)TranslateT("Extra icons"));
				break;
			case ITEM_CONTACT_TIME:
				pos = SendMessage(hwndList, LB_INSERTSTRING, pos - 1, (LPARAM)TranslateT("Contact time"));
				break;
			}
			SendMessage(hwndList, LB_SETITEMDATA, pos, type);
			SendMessage(hwndList, LB_SETCURSEL, pos, 0);
			SendMessage((GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
		}

		if (LOWORD(wParam) == IDC_DOWN) {
			if (HIWORD(wParam) == BN_CLICKED) {
				HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_ORDER);
				int pos = SendMessage(hwndList, LB_GETCURSEL, 0, 0);

				if (pos != LB_ERR) {
					int type = SendMessage(hwndList, LB_GETITEMDATA, pos, 0);

					// Switch items
					SendMessage(hwndList, LB_DELETESTRING, pos, 0);

					switch (type) {
					case ITEM_AVATAR:
						pos = SendMessage(hwndList, LB_INSERTSTRING, pos + 1, (LPARAM)TranslateT("Avatar"));
						break;
					case ITEM_ICON:
						pos = SendMessage(hwndList, LB_INSERTSTRING, pos + 1, (LPARAM)TranslateT("Icon"));
						break;
					case ITEM_TEXT:
						pos = SendMessage(hwndList, LB_INSERTSTRING, pos + 1, (LPARAM)TranslateT("Text"));
						break;
					case ITEM_EXTRA_ICONS:
						pos = SendMessage(hwndList, LB_INSERTSTRING, pos + 1, (LPARAM)TranslateT("Extra icons"));
						break;
					case ITEM_CONTACT_TIME:
						pos = SendMessage(hwndList, LB_INSERTSTRING, pos + 1, (LPARAM)TranslateT("Contact time"));
						break;
					}
					SendMessage(hwndList, LB_SETITEMDATA, pos, type);
					SendMessage(hwndList, LB_SETCURSEL, pos, 0);

					SendMessage((GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
				}
				else return 0;
			}
			else return 0;
		}

		if (LOWORD(wParam) == IDC_LIST_ORDER || LOWORD(wParam) == IDC_UP || LOWORD(wParam) == IDC_DOWN) {
			int pos = SendMessage(GetDlgItem(hwndDlg, IDC_LIST_ORDER), LB_GETCURSEL, 0, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_UP), pos != LB_ERR && pos > 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DOWN), pos != LB_ERR && pos < 4);
		}

		if (LOWORD(wParam) == IDC_LIST_ORDER) return 0;
		if (LOWORD(wParam) == IDC_MIN_ROW_HEIGHT && HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
			return 0; // dont make apply enabled during buddy set crap
		if ((LOWORD(wParam) == IDC_LEFTMARGIN || LOWORD(wParam) == IDC_RIGHTMARGIN || LOWORD(wParam) == IDC_ROW_BORDER) && HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
			return 0; // dont make apply enabled during buddy set crap

		SendMessage((GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_w(NULL, "CList", "MinRowHeight", (WORD)SendDlgItemMessage(hwndDlg, IDC_MIN_ROW_HEIGHT_SPIN, UDM_GETPOS, 0, 0));
				db_set_w(NULL, "CList", "RowBorder", (WORD)SendDlgItemMessage(hwndDlg, IDC_ROW_BORDER_SPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CList", "VariableRowHeight", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_VARIABLE_ROW_HEIGHT));
				db_set_b(NULL, "CList", "AlignLeftItemsToLeft", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ALIGN_TO_LEFT));
				db_set_b(NULL, "CList", "AlignRightItemsToRight", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ALIGN_TO_RIGHT));
				db_set_b(NULL, "CLC", "CompactMode", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_MINIMODE));
				db_set_b(NULL, "CLC", "LeftMargin", (BYTE)SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CLC", "RightMargin", (BYTE)SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_GETPOS, 0, 0));
				{
					HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_ORDER);
					for (int i = 0; i < NUM_ITEM_TYPE; i++) {
						char tmp[128];
						mir_snprintf(tmp, SIZEOF(tmp), "RowPos%d", i);
						db_set_w(NULL, "CList", tmp, (WORD)SendMessage(hwndList, LB_GETITEMDATA, i, 0));
					}
				}

				ClcOptionsChanged();
				return TRUE;
			}
		}
		break;
	}

	return 0;
}

static INT_PTR CALLBACK DlgProcItemAvatarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_SHOW_AVATARS, db_get_b(NULL, "CList", "AvatarsShow", SETTINGS_SHOWAVATARS_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOW_ANIAVATARS, db_get_b(NULL, "CList", "AvatarsAnimated", ServiceExists(MS_AV_GETAVATARBITMAP)) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AVATAR_FASTDRAW, db_get_b(NULL, "CList", "AvatarsInSeparateWnd", SETTINGS_AVATARINSEPARATE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AVATAR_DRAW_BORDER, db_get_b(NULL, "CList", "AvatarsDrawBorders", SETTINGS_AVATARDRAWBORDER_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AVATAR_ROUND_CORNERS, db_get_b(NULL, "CList", "AvatarsRoundCorners", SETTINGS_AVATARROUNDCORNERS_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK, db_get_b(NULL, "CList", "AvatarsUseCustomCornerSize", SETTINGS_AVATARUSECUTOMCORNERSIZE_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AVATAR_IGNORE_SIZE, db_get_b(NULL, "CList", "AvatarsIgnoreSizeForRow", SETTINGS_AVATARIGNORESIZEFORROW_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AVATAR_OVERLAY_ICONS, db_get_b(NULL, "CList", "AvatarsDrawOverlay", SETTINGS_AVATARDRAWOVERLAY_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);

		switch (db_get_b(NULL, "CList", "AvatarsOverlayType", SETTINGS_AVATAROVERLAYTYPE_DEFAULT)) {
		case SETTING_AVATAR_OVERLAY_TYPE_NORMAL:
			CheckDlgButton(hwndDlg, IDC_AVATAR_OVERLAY_ICON_NORMAL, BST_CHECKED);
			break;
		case SETTING_AVATAR_OVERLAY_TYPE_PROTOCOL:
			CheckDlgButton(hwndDlg, IDC_AVATAR_OVERLAY_ICON_PROTOCOL, BST_CHECKED);
			break;
		case SETTING_AVATAR_OVERLAY_TYPE_CONTACT:
			CheckDlgButton(hwndDlg, IDC_AVATAR_OVERLAY_ICON_CONTACT, BST_CHECKED);
			break;
		}

		SendDlgItemMessage(hwndDlg, IDC_AVATAR_SIZE_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_AVATAR_SIZE), 0); // set buddy
		SendDlgItemMessage(hwndDlg, IDC_AVATAR_SIZE_SPIN, UDM_SETRANGE, 0, MAKELONG(255, 1));
		SendDlgItemMessage(hwndDlg, IDC_AVATAR_SIZE_SPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "CList", "AvatarsSize", SETTING_AVATARHEIGHT_DEFAULT), 0));

		SendDlgItemMessage(hwndDlg, IDC_AVATAR_WIDTH_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_AVATAR_WIDTH), 0); // set buddy
		SendDlgItemMessage(hwndDlg, IDC_AVATAR_WIDTH_SPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));
		SendDlgItemMessage(hwndDlg, IDC_AVATAR_WIDTH_SPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "CList", "AvatarsWidth", SETTING_AVATARWIDTH_DEFAULT), 0));

		SendDlgItemMessage(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE), 0); // set buddy
		SendDlgItemMessage(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN, UDM_SETRANGE, 0, MAKELONG(255, 1));
		SendDlgItemMessage(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "CList", "AvatarsCustomCornerSize", SETTINGS_AVATARCORNERSIZE_DEFAULT), 0));

		SendDlgItemMessage(hwndDlg, IDC_AVATAR_BORDER_COLOR, CPM_SETCOLOUR, 0, (COLORREF)db_get_dw(NULL, "CList", "AvatarsBorderColor", SETTINGS_AVATARBORDERCOLOR_DEFAULT));

		if (!IsDlgButtonChecked(hwndDlg, IDC_SHOW_AVATARS)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_DRAW_BORDER), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_BORDER_COLOR_L), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_BORDER_COLOR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_ROUND_CORNERS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_IGNORE_SIZE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_OVERLAY_ICONS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_OVERLAY_ICON_NORMAL), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_OVERLAY_ICON_PROTOCOL), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_OVERLAY_ICON_CONTACT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_SIZE_L), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_SIZE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_SIZE_SPIN), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_SIZE_PIXELS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_WIDTH), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_WIDTH_SPIN), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_SIZE_PIXELS2), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_ANIAVATARS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_SIZE_PIXELS3), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_FASTDRAW), FALSE);
		}

		if (!IsDlgButtonChecked(hwndDlg, IDC_SHOW_ANIAVATARS))
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_FASTDRAW), FALSE);

		if (!IsDlgButtonChecked(hwndDlg, IDC_AVATAR_DRAW_BORDER)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_BORDER_COLOR_L), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_BORDER_COLOR), FALSE);
		}

		if (!IsDlgButtonChecked(hwndDlg, IDC_AVATAR_ROUND_CORNERS)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN), FALSE);
		}

		if (!IsDlgButtonChecked(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN), FALSE);
		}

		if (!IsDlgButtonChecked(hwndDlg, IDC_AVATAR_OVERLAY_ICONS)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_OVERLAY_ICON_NORMAL), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_OVERLAY_ICON_PROTOCOL), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_OVERLAY_ICON_CONTACT), FALSE);
		}

		if (!ServiceExists(MS_AV_GETAVATARBITMAP)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_ANIAVATARS), FALSE);
			SetDlgItemText(hwndDlg, IDC_SHOW_ANIAVATARS, TranslateT("Animate avatars (GDI+ and Avatar service module (avs.dll) are required)"));
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_SHOW_AVATARS) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_SHOW_AVATARS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_DRAW_BORDER), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_BORDER_COLOR_L), enabled && IsDlgButtonChecked(hwndDlg, IDC_AVATAR_DRAW_BORDER));
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_BORDER_COLOR), enabled && IsDlgButtonChecked(hwndDlg, IDC_AVATAR_DRAW_BORDER));
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_ROUND_CORNERS), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_IGNORE_SIZE), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_OVERLAY_ICONS), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_OVERLAY_ICON_NORMAL), enabled && IsDlgButtonChecked(hwndDlg, IDC_AVATAR_OVERLAY_ICONS));
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_OVERLAY_ICON_PROTOCOL), enabled && IsDlgButtonChecked(hwndDlg, IDC_AVATAR_OVERLAY_ICONS));
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_OVERLAY_ICON_CONTACT), enabled && IsDlgButtonChecked(hwndDlg, IDC_AVATAR_OVERLAY_ICONS));
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_SIZE_L), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_SIZE), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_SIZE_SPIN), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_SIZE_PIXELS2), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_SIZE_PIXELS3), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_ANIAVATARS), enabled && ServiceExists(MS_AV_GETAVATARBITMAP));

			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_WIDTH), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_WIDTH_SPIN), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_SIZE_PIXELS), enabled);

			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_FASTDRAW), enabled && IsDlgButtonChecked(hwndDlg, IDC_SHOW_ANIAVATARS));
		}
		else if (LOWORD(wParam) == IDC_SHOW_ANIAVATARS)
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_FASTDRAW), IsDlgButtonChecked(hwndDlg, IDC_SHOW_ANIAVATARS));

		else if (LOWORD(wParam) == IDC_AVATAR_DRAW_BORDER) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_SHOW_AVATARS) && IsDlgButtonChecked(hwndDlg, IDC_AVATAR_DRAW_BORDER);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_BORDER_COLOR_L), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_BORDER_COLOR), enabled);
		}

		else if (LOWORD(wParam) == IDC_AVATAR_ROUND_CORNERS) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_SHOW_AVATARS) && IsDlgButtonChecked(hwndDlg, IDC_AVATAR_ROUND_CORNERS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN), enabled);
		}

		else if (LOWORD(wParam) == IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_AVATAR_ROUND_CORNERS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE), enabled && IsDlgButtonChecked(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK));
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN), enabled && IsDlgButtonChecked(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK));
		}

		else if (LOWORD(wParam) == IDC_AVATAR_OVERLAY_ICONS) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_SHOW_AVATARS) && IsDlgButtonChecked(hwndDlg, IDC_AVATAR_OVERLAY_ICONS);

			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_OVERLAY_ICON_NORMAL), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_OVERLAY_ICON_PROTOCOL), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_OVERLAY_ICON_CONTACT), enabled);
		}
		else if (LOWORD(wParam) == IDC_AVATAR_SIZE && HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
			return 0; // dont make apply enabled during buddy set crap
		else if (LOWORD(wParam) == IDC_AVATAR_CUSTOM_CORNER_SIZE && HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
			return 0; // dont make apply enabled during buddy set crap

		SendMessage((GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(NULL, "CList", "AvatarsShow", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOW_AVATARS));
				db_set_b(NULL, "CList", "AvatarsAnimated", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOW_ANIAVATARS));
				db_set_b(NULL, "CList", "AvatarsInSeparateWnd", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AVATAR_FASTDRAW));

				db_set_b(NULL, "CList", "AvatarsDrawBorders", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AVATAR_DRAW_BORDER));
				db_set_dw(NULL, "CList", "AvatarsBorderColor", (DWORD)SendDlgItemMessage(hwndDlg, IDC_AVATAR_BORDER_COLOR, CPM_GETCOLOUR, 0, 0));
				db_set_b(NULL, "CList", "AvatarsRoundCorners", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AVATAR_ROUND_CORNERS));
				db_set_b(NULL, "CList", "AvatarsIgnoreSizeForRow", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AVATAR_IGNORE_SIZE));
				db_set_b(NULL, "CList", "AvatarsUseCustomCornerSize", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK));
				db_set_w(NULL, "CList", "AvatarsCustomCornerSize", (WORD)SendDlgItemMessage(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CList", "AvatarsDrawOverlay", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AVATAR_OVERLAY_ICONS));
				db_set_w(NULL, "CList", "AvatarsSize", (WORD)SendDlgItemMessage(hwndDlg, IDC_AVATAR_SIZE_SPIN, UDM_GETPOS, 0, 0));
				db_set_w(NULL, "CList", "AvatarsWidth", (WORD)SendDlgItemMessage(hwndDlg, IDC_AVATAR_WIDTH_SPIN, UDM_GETPOS, 0, 0));

				if (IsDlgButtonChecked(hwndDlg, IDC_AVATAR_OVERLAY_ICON_NORMAL))
					db_set_b(NULL, "CList", "AvatarsOverlayType", SETTING_AVATAR_OVERLAY_TYPE_NORMAL);
				else if (IsDlgButtonChecked(hwndDlg, IDC_AVATAR_OVERLAY_ICON_PROTOCOL))
					db_set_b(NULL, "CList", "AvatarsOverlayType", SETTING_AVATAR_OVERLAY_TYPE_PROTOCOL);
				else if (IsDlgButtonChecked(hwndDlg, IDC_AVATAR_OVERLAY_ICON_CONTACT))
					db_set_b(NULL, "CList", "AvatarsOverlayType", SETTING_AVATAR_OVERLAY_TYPE_CONTACT);
				ClcOptionsChanged();
				AniAva_UpdateOptions();
				return TRUE;
			}
			break;
		}
		break;
	}

	return 0;
}


static INT_PTR CALLBACK DlgProcItemIconOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_HIDE_ICON_ON_AVATAR, db_get_b(NULL, "CList", "IconHideOnAvatar", SETTING_HIDEICONONAVATAR_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DRAW_ON_AVATAR_SPACE, db_get_b(NULL, "CList", "IconDrawOnAvatarSpace", SETTING_ICONONAVATARPLACE_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ICONBLINK, db_get_b(NULL, "CList", "NoIconBlink", SETTING_NOICONBLINF_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HIDE_GROUPSICON, db_get_b(NULL, "CList", "HideGroupsIcon", SETTING_HIDEGROUPSICON_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NOTCHECKICONSIZE, db_get_b(NULL, "CList", "IconIgnoreSizeForRownHeight", SETTING_ICONIGNORESIZE_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ALWAYS_VISIBLEICON, db_get_b(NULL, "CList", "AlwaysShowAlwaysVisIcon", SETTING_ALWAYSVISICON_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_USEXSTATUS, (db_get_b(NULL, "CLC", "DrawOverlayedStatus", SETTING_DRAWOVERLAYEDSTATUS_DEFAULT) & 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DRAWSTATUSOVERLAY, (db_get_b(NULL, "CLC", "DrawOverlayedStatus", SETTING_DRAWOVERLAYEDSTATUS_DEFAULT) & 2) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_DRAWSTATUSOVERLAY), IsDlgButtonChecked(hwndDlg, IDC_USEXSTATUS));

		if (!IsDlgButtonChecked(hwndDlg, IDC_HIDE_ICON_ON_AVATAR))
			EnableWindow(GetDlgItem(hwndDlg, IDC_DRAW_ON_AVATAR_SPACE), FALSE);

		ShowWindow(GetDlgItem(hwndDlg, IDC_ALWAYS_VISIBLEICON), SW_HIDE);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_HIDE_ICON_ON_AVATAR) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_HIDE_ICON_ON_AVATAR);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DRAW_ON_AVATAR_SPACE), enabled);
		}
		else if (LOWORD(wParam) == IDC_USEXSTATUS)
			EnableWindow(GetDlgItem(hwndDlg, IDC_DRAWSTATUSOVERLAY), IsDlgButtonChecked(hwndDlg, IDC_USEXSTATUS));

		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(NULL, "CList", "IconHideOnAvatar", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_HIDE_ICON_ON_AVATAR));
				db_set_b(NULL, "CList", "IconDrawOnAvatarSpace", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DRAW_ON_AVATAR_SPACE));
				db_set_b(NULL, "CList", "HideGroupsIcon", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_HIDE_GROUPSICON));
				db_set_b(NULL, "CList", "NoIconBlink", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ICONBLINK));
				db_set_b(NULL, "CList", "IconIgnoreSizeForRownHeight", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_NOTCHECKICONSIZE));
				db_set_b(NULL, "CList", "AlwaysShowAlwaysVisIcon", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_VISIBLEICON));
				{
					BYTE ovr = IsDlgButtonChecked(hwndDlg, IDC_USEXSTATUS) ? 1 : 0;
					if (ovr) ovr += IsDlgButtonChecked(hwndDlg, IDC_DRAWSTATUSOVERLAY) ? 2 : 0;
					db_set_b(NULL, "CLC", "DrawOverlayedStatus", ovr);
				}
				ClcOptionsChanged();
				return TRUE;
			}
		}
		break;
	}

	return 0;
}


static INT_PTR CALLBACK DlgProcItemContactTimeOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_SHOW, db_get_b(NULL, "CList", "ContactTimeShow", SETTING_SHOWTIME_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOW_ONLY_IF_DIFFERENT, db_get_b(NULL, "CList", "ContactTimeShowOnlyIfDifferent", SETTING_SHOWTIMEIFDIFF_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		break;

	case WM_COMMAND:
		SendMessage((GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(NULL, "CList", "ContactTimeShow", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOW));
				db_set_b(NULL, "CList", "ContactTimeShowOnlyIfDifferent", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOW_ONLY_IF_DIFFERENT));
				ClcOptionsChanged();
				return TRUE;
			}
			break;
		}
		break;
	}

	return 0;
}


static INT_PTR CALLBACK DlgProcItemTextOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_RTL, db_get_b(NULL, "CList", "TextRTL", SETTING_TEXT_RTL_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		//TODO: init IDC_ALIGNGROUPCOMBO from db_get_b(NULL,"CList","AlignGroupCaptions",SETTING_ALIGNGROPCAPTION_DEFAULT);
		{
			int item;
			TCHAR *align[] = { LPGENT("Left align group names"), LPGENT("Center group names"), LPGENT("Right align group names") };
			for (int i = 0; i < sizeof(align) / sizeof(char*); i++)
				item = SendDlgItemMessage(hwndDlg, IDC_ALIGNGROUPCOMBO, CB_ADDSTRING, 0, (LPARAM)TranslateTS(align[i]));
			SendDlgItemMessage(hwndDlg, IDC_ALIGNGROUPCOMBO, CB_SETCURSEL, db_get_b(NULL, "CList", "AlignGroupCaptions", SETTING_ALIGNGROPCAPTION_DEFAULT), 0);
		}
		CheckDlgButton(hwndDlg, IDC_ALIGN_RIGHT, db_get_b(NULL, "CList", "TextAlignToRight", SETTING_TEXT_RIGHTALIGN_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_REPLACE_SMILEYS, db_get_b(NULL, "CList", "TextReplaceSmileys", SETTING_TEXT_SMILEY_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RESIZE_SMILEYS, db_get_b(NULL, "CList", "TextResizeSmileys", SETTING_TEXT_RESIZESMILEY_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_USE_PROTOCOL_SMILEYS, db_get_b(NULL, "CList", "TextUseProtocolSmileys", SETTING_TEXT_PROTOSMILEY_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_IGNORE_SIZE, db_get_b(NULL, "CList", "TextIgnoreSizeForRownHeight", SETTING_TEXT_IGNORESIZE_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_DRAW_SMILEYS_ON_FIRST_LINE, db_get_b(NULL, "CList", "FirstLineDrawSmileys", SETTING_FIRSTLINE_SMILEYS_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_APPEND_NICK, db_get_b(NULL, "CList", "FirstLineAppendNick", SETTING_FIRSTLINE_APPENDNICK_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TRIM_TEXT, db_get_b(NULL, "CList", "TrimText", SETTING_FIRSTLINE_TRIMTEXT_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		{
			bool smileAddPresent = ServiceExists(MS_SMILEYADD_BATCHPARSE) != 0;
			CLUI_ShowWindowMod(GetDlgItem(hwndDlg, IDC_REPLACE_SMILEYS), smileAddPresent ? SW_SHOW : SW_HIDE);
			CLUI_ShowWindowMod(GetDlgItem(hwndDlg, IDC_USE_PROTOCOL_SMILEYS), smileAddPresent ? SW_SHOW : SW_HIDE);
			//CLUI_ShowWindowMod(GetDlgItem(hwndDlg,IDC_RESIZE_SMILEYS), smileAddPresent ? SW_SHOW : SW_HIDE);
			CLUI_ShowWindowMod(GetDlgItem(hwndDlg, IDC_DRAW_SMILEYS_ON_FIRST_LINE), smileAddPresent ? SW_SHOW : SW_HIDE);
		}

		if (!IsDlgButtonChecked(hwndDlg, IDC_REPLACE_SMILEYS))
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_PROTOCOL_SMILEYS), FALSE);

		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_REPLACE_SMILEYS) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_REPLACE_SMILEYS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_PROTOCOL_SMILEYS), enabled);
			//EnableWindow(GetDlgItem(hwndDlg,IDC_RESIZE_SMILEYS),enabled); //Commented out for listening to icon
		}
		if (LOWORD(wParam) != IDC_ALIGNGROUPCOMBO || (LOWORD(wParam) == IDC_ALIGNGROUPCOMBO && HIWORD(wParam) == CBN_SELCHANGE))
			SendMessage((GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(NULL, "CList", "TextRTL", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_RTL));
				//TODO: Store IDC_ALIGNGROUPCOMBO at db_get_b(NULL,"CList","AlignGroupCaptions",SETTING_ALIGNGROPCAPTION_DEFAULT);
				db_set_b(NULL, "CList", "AlignGroupCaptions", (BYTE)SendDlgItemMessage(hwndDlg, IDC_ALIGNGROUPCOMBO, CB_GETCURSEL, 0, 0));
				db_set_b(NULL, "CList", "TextAlignToRight", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ALIGN_RIGHT));
				db_set_b(NULL, "CList", "TextReplaceSmileys", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_REPLACE_SMILEYS));
				db_set_b(NULL, "CList", "TextResizeSmileys", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_RESIZE_SMILEYS));
				db_set_b(NULL, "CList", "TextUseProtocolSmileys", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_USE_PROTOCOL_SMILEYS));
				db_set_b(NULL, "CList", "TextIgnoreSizeForRownHeight", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_IGNORE_SIZE));
				db_set_b(NULL, "CList", "FirstLineDrawSmileys", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DRAW_SMILEYS_ON_FIRST_LINE));
				db_set_b(NULL, "CList", "FirstLineAppendNick", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_APPEND_NICK));
				db_set_b(NULL, "CList", "TrimText", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TRIM_TEXT));
				ClcOptionsChanged();
				return TRUE;
			}
		}
		break;
	}

	return 0;
}

static INT_PTR CALLBACK DlgProcItemSecondLineOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_SHOW, db_get_b(NULL, "CList", "SecondLineShow", SETTING_SECONDLINE_SHOW_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_TOP_SPACE_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_TOP_SPACE), 0); // set buddy
		SendDlgItemMessage(hwndDlg, IDC_TOP_SPACE_SPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));
		SendDlgItemMessage(hwndDlg, IDC_TOP_SPACE_SPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "CList", "SecondLineTopSpace", SETTING_SECONDLINE_TOPSPACE_DEFAULT), 0));

		CheckDlgButton(hwndDlg, IDC_DRAW_SMILEYS, db_get_b(NULL, "CList", "SecondLineDrawSmileys", SETTING_SECONDLINE_SMILEYS_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		{
			ptrT tszText(db_get_tsa(NULL, "CList", "SecondLineText"));
			if (tszText)
				SetWindowText(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), tszText);
		}
		SendMessage(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), EM_SETLIMITTEXT, TEXT_TEXT_MAX_LENGTH, 0);
		{
			int radio = db_get_w(NULL, "CList", "SecondLineType", TEXT_STATUS_MESSAGE);
			CheckDlgButton(hwndDlg, IDC_STATUS, radio == TEXT_STATUS ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NICKNAME, radio == TEXT_NICKNAME ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_STATUS_MESSAGE, radio == TEXT_STATUS_MESSAGE ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LISTENING_TO, radio == TEXT_LISTENING_TO ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CONTACT_TIME, radio == TEXT_CONTACT_TIME ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TEXT, radio == TEXT_TEXT ? BST_CHECKED : BST_UNCHECKED);
		}
		CheckDlgButton(hwndDlg, IDC_XSTATUS_HAS_PRIORITY, db_get_b(NULL, "CList", "SecondLineXStatusHasPriority", SETTING_SECONDLINE_XSTATUS_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOW_STATUS_IF_NOAWAY, db_get_b(NULL, "CList", "SecondLineShowStatusIfNoAway", SETTING_SECONDLINE_STATUSIFNOAWAY_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOW_LISTENING_IF_NOAWAY, db_get_b(NULL, "CList", "SecondLineShowListeningIfNoAway", SETTING_SECONDLINE_LISTENINGIFNOAWAY_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_USE_NAME_AND_MESSAGE, db_get_b(NULL, "CList", "SecondLineUseNameAndMessageForXStatus", SETTING_SECONDLINE_XSTATUSNAMETEXT_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);

		if (!IsDlgButtonChecked(hwndDlg, IDC_SHOW)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_PROTOCOL_SMILEYS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOP_SPACE_SPIN), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOP_SPACE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DRAW_SMILEYS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_NICKNAME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUS_MESSAGE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LISTENING_TO), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CONTACT_TIME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_TOP), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_PIXELS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_TEXT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLES_L), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_STATUS_IF_NOAWAY), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_LISTENING_IF_NOAWAY), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_XSTATUS_HAS_PRIORITY), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_NAME_AND_MESSAGE), FALSE);
		}
		else {
			if (!IsDlgButtonChecked(hwndDlg, IDC_TEXT)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLES_L), FALSE);
			}
			if (!IsDlgButtonChecked(hwndDlg, IDC_STATUS) && !IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_XSTATUS_HAS_PRIORITY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_USE_NAME_AND_MESSAGE), FALSE);
			}
			if (!IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_STATUS_IF_NOAWAY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_LISTENING_IF_NOAWAY), FALSE);
			}
		}

		CLUI_ShowWindowMod(GetDlgItem(hwndDlg, IDC_DRAW_SMILEYS), ServiceExists(MS_SMILEYADD_BATCHPARSE) ? SW_SHOW : SW_HIDE);
		CLUI_ShowWindowMod(GetDlgItem(hwndDlg, IDC_VARIABLES_L), ServiceExists(MS_VARS_FORMATSTRING) ? SW_SHOW : SW_HIDE);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_TEXT || LOWORD(wParam) == IDC_STATUS || LOWORD(wParam) == IDC_NICKNAME || LOWORD(wParam) == IDC_STATUS_MESSAGE
			|| LOWORD(wParam) == IDC_LISTENING_TO || LOWORD(wParam) == IDC_CONTACT_TIME) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), IsDlgButtonChecked(hwndDlg, IDC_TEXT)
				&& IsDlgButtonChecked(hwndDlg, IDC_SHOW));
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLES_L), IsDlgButtonChecked(hwndDlg, IDC_TEXT)
				&& IsDlgButtonChecked(hwndDlg, IDC_SHOW));
			EnableWindow(GetDlgItem(hwndDlg, IDC_XSTATUS_HAS_PRIORITY), IsDlgButtonChecked(hwndDlg, IDC_SHOW) &&
				(IsDlgButtonChecked(hwndDlg, IDC_STATUS) || IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_STATUS_IF_NOAWAY), IsDlgButtonChecked(hwndDlg, IDC_SHOW) &&
				(IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_LISTENING_IF_NOAWAY), IsDlgButtonChecked(hwndDlg, IDC_SHOW) &&
				(IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_NAME_AND_MESSAGE), IsDlgButtonChecked(hwndDlg, IDC_SHOW) &&
				(IsDlgButtonChecked(hwndDlg, IDC_STATUS) || IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)));
		}
		else if (LOWORD(wParam) == IDC_SHOW) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_SHOW);
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_PROTOCOL_SMILEYS), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOP_SPACE_SPIN), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOP_SPACE), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DRAW_SMILEYS), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUS), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_NICKNAME), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CONTACT_TIME), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUS_MESSAGE), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LISTENING_TO), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), enabled && IsDlgButtonChecked(hwndDlg, IDC_TEXT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_TOP), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_PIXELS), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_TEXT), enabled);

			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), enabled && IsDlgButtonChecked(hwndDlg, IDC_TEXT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLES_L), enabled && IsDlgButtonChecked(hwndDlg, IDC_TEXT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_XSTATUS_HAS_PRIORITY), enabled && (IsDlgButtonChecked(hwndDlg, IDC_STATUS)
				|| IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_STATUS_IF_NOAWAY), enabled && IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_LISTENING_IF_NOAWAY), enabled && IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_NAME_AND_MESSAGE), enabled && (IsDlgButtonChecked(hwndDlg, IDC_STATUS)
				|| IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)));
		}

		if (LOWORD(wParam) == IDC_TOP_SPACE && HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
			return 0; // dont make apply enabled during buddy set crap

		SendMessage((GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(NULL, "CList", "SecondLineShow", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOW));
				db_set_w(NULL, "CList", "SecondLineTopSpace", (WORD)SendDlgItemMessage(hwndDlg, IDC_TOP_SPACE_SPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CList", "SecondLineDrawSmileys", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DRAW_SMILEYS));

				int radio;
				if (IsDlgButtonChecked(hwndDlg, IDC_STATUS))
					radio = TEXT_STATUS;
				else if (IsDlgButtonChecked(hwndDlg, IDC_NICKNAME))
					radio = TEXT_NICKNAME;
				else if (IsDlgButtonChecked(hwndDlg, IDC_TEXT))
					radio = TEXT_TEXT;
				else if (IsDlgButtonChecked(hwndDlg, IDC_CONTACT_TIME))
					radio = TEXT_CONTACT_TIME;
				else if (IsDlgButtonChecked(hwndDlg, IDC_LISTENING_TO))
					radio = TEXT_LISTENING_TO;
				else
					radio = TEXT_STATUS_MESSAGE;
				db_set_w(NULL, "CList", "SecondLineType", (WORD)radio);

				TCHAR t[TEXT_TEXT_MAX_LENGTH];
				GetWindowText(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), t, SIZEOF(t));
				t[TEXT_TEXT_MAX_LENGTH - 1] = '\0';
				db_set_ts(NULL, "CList", "SecondLineText", t);

				db_set_b(NULL, "CList", "SecondLineXStatusHasPriority", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_XSTATUS_HAS_PRIORITY));
				db_set_b(NULL, "CList", "SecondLineShowStatusIfNoAway", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOW_STATUS_IF_NOAWAY));
				db_set_b(NULL, "CList", "SecondLineShowListeningIfNoAway", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOW_LISTENING_IF_NOAWAY));
				db_set_b(NULL, "CList", "SecondLineUseNameAndMessageForXStatus", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_USE_NAME_AND_MESSAGE));
				ClcOptionsChanged();
				return TRUE;
			}
			break;
		}
		break;
	}

	return 0;
}

static INT_PTR CALLBACK DlgProcItemThirdLineOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_SHOW, db_get_b(NULL, "CList", "ThirdLineShow", SETTING_THIRDLINE_SHOW_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_TOP_SPACE_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_TOP_SPACE), 0); // set buddy
		SendDlgItemMessage(hwndDlg, IDC_TOP_SPACE_SPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));
		SendDlgItemMessage(hwndDlg, IDC_TOP_SPACE_SPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "CList", "ThirdLineTopSpace", SETTING_THIRDLINE_TOPSPACE_DEFAULT), 0));

		CheckDlgButton(hwndDlg, IDC_DRAW_SMILEYS, db_get_b(NULL, "CList", "ThirdLineDrawSmileys", SETTING_THIRDLINE_SMILEYS_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		{
			DBVARIANT dbv;
			if (!db_get_ts(NULL, "CList", "ThirdLineText", &dbv)) {
				SetWindowText(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), dbv.ptszVal);
				db_free(&dbv);
			}
		}
		SendMessage(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), EM_SETLIMITTEXT, TEXT_TEXT_MAX_LENGTH, 0);
		{
			int radio = db_get_w(NULL, "CList", "ThirdLineType", SETTING_THIRDLINE_TYPE_DEFAULT);
			CheckDlgButton(hwndDlg, IDC_STATUS, radio == TEXT_STATUS ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NICKNAME, radio == TEXT_NICKNAME ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_STATUS_MESSAGE, radio == TEXT_STATUS_MESSAGE ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LISTENING_TO, radio == TEXT_LISTENING_TO ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CONTACT_TIME, radio == TEXT_CONTACT_TIME ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TEXT, radio == TEXT_TEXT ? BST_CHECKED : BST_UNCHECKED);
		}

		CheckDlgButton(hwndDlg, IDC_XSTATUS_HAS_PRIORITY, db_get_b(NULL, "CList", "ThirdLineXStatusHasPriority", SETTING_THIRDLINE_XSTATUS_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOW_STATUS_IF_NOAWAY, db_get_b(NULL, "CList", "ThirdLineShowStatusIfNoAway", SETTING_THIRDLINE_STATUSIFNOAWAY_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOW_LISTENING_IF_NOAWAY, db_get_b(NULL, "CList", "ThirdLineShowListeningIfNoAway", SETTING_THIRDLINE_LISTENINGIFNOAWAY_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_USE_NAME_AND_MESSAGE, db_get_b(NULL, "CList", "ThirdLineUseNameAndMessageForXStatus", SETTING_THIRDLINE_XSTATUSNAMETEXT_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);

		if (!IsDlgButtonChecked(hwndDlg, IDC_SHOW)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_PROTOCOL_SMILEYS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOP_SPACE_SPIN), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOP_SPACE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DRAW_SMILEYS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_NICKNAME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUS_MESSAGE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LISTENING_TO), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CONTACT_TIME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_TOP), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_PIXELS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_TEXT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLES_L), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_XSTATUS_HAS_PRIORITY), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_NAME_AND_MESSAGE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_STATUS_IF_NOAWAY), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_LISTENING_IF_NOAWAY), FALSE);
		}
		else {
			if (!IsDlgButtonChecked(hwndDlg, IDC_TEXT)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLES_L), FALSE);
			}
			if (!IsDlgButtonChecked(hwndDlg, IDC_STATUS) && !IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_XSTATUS_HAS_PRIORITY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_USE_NAME_AND_MESSAGE), FALSE);
			}
			if (!IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_STATUS_IF_NOAWAY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_LISTENING_IF_NOAWAY), FALSE);
			}
		}

		CLUI_ShowWindowMod(GetDlgItem(hwndDlg, IDC_DRAW_SMILEYS), ServiceExists(MS_SMILEYADD_BATCHPARSE) ? SW_SHOW : SW_HIDE);
		CLUI_ShowWindowMod(GetDlgItem(hwndDlg, IDC_VARIABLES_L), ServiceExists(MS_VARS_FORMATSTRING) ? SW_SHOW : SW_HIDE);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_TEXT || LOWORD(wParam) == IDC_STATUS || LOWORD(wParam) == IDC_NICKNAME || LOWORD(wParam) == IDC_STATUS_MESSAGE
			|| LOWORD(wParam) == IDC_LISTENING_TO || LOWORD(wParam) == IDC_CONTACT_TIME) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), IsDlgButtonChecked(hwndDlg, IDC_TEXT)
				&& IsDlgButtonChecked(hwndDlg, IDC_SHOW));
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLES_L), IsDlgButtonChecked(hwndDlg, IDC_TEXT)
				&& IsDlgButtonChecked(hwndDlg, IDC_SHOW));
			EnableWindow(GetDlgItem(hwndDlg, IDC_XSTATUS_HAS_PRIORITY), IsDlgButtonChecked(hwndDlg, IDC_SHOW)
				&& (IsDlgButtonChecked(hwndDlg, IDC_STATUS) || IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_NAME_AND_MESSAGE), IsDlgButtonChecked(hwndDlg, IDC_SHOW) && (IsDlgButtonChecked(hwndDlg, IDC_STATUS) || IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_STATUS_IF_NOAWAY), IsDlgButtonChecked(hwndDlg, IDC_SHOW) && (IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_LISTENING_IF_NOAWAY), IsDlgButtonChecked(hwndDlg, IDC_SHOW) && (IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)));
		}
		else if (LOWORD(wParam) == IDC_SHOW) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_SHOW);
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_PROTOCOL_SMILEYS), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOP_SPACE_SPIN), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOP_SPACE), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DRAW_SMILEYS), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUS), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_NICKNAME), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUS_MESSAGE), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LISTENING_TO), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CONTACT_TIME), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), enabled && IsDlgButtonChecked(hwndDlg, IDC_TEXT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_TOP), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_PIXELS), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_TEXT), enabled);

			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), enabled && IsDlgButtonChecked(hwndDlg, IDC_TEXT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLES_L), enabled && IsDlgButtonChecked(hwndDlg, IDC_TEXT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_XSTATUS_HAS_PRIORITY), enabled && (IsDlgButtonChecked(hwndDlg, IDC_STATUS)
				|| IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_NAME_AND_MESSAGE), enabled && (IsDlgButtonChecked(hwndDlg, IDC_STATUS)
				|| IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_STATUS_IF_NOAWAY), enabled && IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_LISTENING_IF_NOAWAY), enabled && IsDlgButtonChecked(hwndDlg, IDC_STATUS_MESSAGE));
		}

		if (LOWORD(wParam) == IDC_TOP_SPACE && HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
			return 0; // dont make apply enabled during buddy set crap

		SendMessage((GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(NULL, "CList", "ThirdLineShow", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOW));
				db_set_w(NULL, "CList", "ThirdLineTopSpace", (WORD)SendDlgItemMessage(hwndDlg, IDC_TOP_SPACE_SPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CList", "ThirdLineDrawSmileys", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DRAW_SMILEYS));
				{
					int radio;
					if (IsDlgButtonChecked(hwndDlg, IDC_STATUS))
						radio = TEXT_STATUS;
					else if (IsDlgButtonChecked(hwndDlg, IDC_NICKNAME))
						radio = TEXT_NICKNAME;
					else if (IsDlgButtonChecked(hwndDlg, IDC_TEXT))
						radio = TEXT_TEXT;
					else if (IsDlgButtonChecked(hwndDlg, IDC_CONTACT_TIME))
						radio = TEXT_CONTACT_TIME;
					else if (IsDlgButtonChecked(hwndDlg, IDC_LISTENING_TO))
						radio = TEXT_LISTENING_TO;
					else
						radio = TEXT_STATUS_MESSAGE;
					db_set_w(NULL, "CList", "ThirdLineType", (WORD)radio);
				}
				{
					TCHAR t[TEXT_TEXT_MAX_LENGTH];

					GetWindowText(GetDlgItem(hwndDlg, IDC_VARIABLE_TEXT), t, SIZEOF(t));
					t[TEXT_TEXT_MAX_LENGTH - 1] = '\0';

					db_set_ws(NULL, "CList", "ThirdLineText", t);
				}

				db_set_b(NULL, "CList", "ThirdLineXStatusHasPriority", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_XSTATUS_HAS_PRIORITY));
				db_set_b(NULL, "CList", "ThirdLineUseNameAndMessageForXStatus", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_USE_NAME_AND_MESSAGE));
				db_set_b(NULL, "CList", "ThirdLineShowStatusIfNoAway", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOW_STATUS_IF_NOAWAY));
				db_set_b(NULL, "CList", "ThirdLineShowListeningIfNoAway", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOW_LISTENING_IF_NOAWAY));
				ClcOptionsChanged();
				return TRUE;
			}
			break;
		}
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	char *name;				// Tab name
	int id;					// Dialog id
	DLGPROC wnd_proc;		// Dialog function
}
static row_opt_items[] =
{
	{ LPGEN("Row"), IDD_OPT_ITEM_ROW, DlgProcItemRowOpts },
#ifdef _DEBUG
	{ LPGEN("Row design"), IDD_OPT_ROWTMPL, DlgTmplEditorOpts },
#endif
	{ LPGEN("Avatar"), IDD_OPT_ITEM_AVATAR, DlgProcItemAvatarOpts },
	{ LPGEN("Icon"), IDD_OPT_ITEM_ICON, DlgProcItemIconOpts },
	{ LPGEN("Contact time"), IDD_OPT_ITEM_CONTACT_TIME, DlgProcItemContactTimeOpts },
	{ LPGEN("Text"), IDD_OPT_ITEM_TEXT, DlgProcItemTextOpts },
	{ LPGEN("Second line"), IDD_OPT_ITEM_SECOND_LINE, DlgProcItemSecondLineOpts },
	{ LPGEN("Third line"), IDD_OPT_ITEM_THIRD_LINE, DlgProcItemThirdLineOpts },
};

int CListOptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = -200000000;
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ITEMS);
	odp.pszGroup = LPGEN("Contact list");
	odp.pszTitle = LPGEN("Row items");
	odp.flags = ODPF_BOLDGROUPS;

	for (int i = 0; i < SIZEOF(row_opt_items); i++) {
		odp.pszTemplate = MAKEINTRESOURCEA(row_opt_items[i].id);
		odp.pszTab = row_opt_items[i].name;
		odp.pfnDlgProc = row_opt_items[i].wnd_proc;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}

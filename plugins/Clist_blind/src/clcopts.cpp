/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-20 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#include "stdafx.h"

wchar_t* MyDBGetContactSettingTString(MCONTACT hContact, char* module, char* setting, wchar_t* out, size_t len, wchar_t *def);

#define DBFONTF_BOLD       1
#define DBFONTF_ITALIC     2
#define DBFONTF_UNDERLINE  4

struct CheckBoxToStyleEx_t
{
	int id;
	DWORD flag;
	int not;
}

static const checkBoxToStyleEx[] =
{
	{ IDC_TRACKSELECT, CLS_EX_TRACKSELECT, 0 },
	{ IDC_SHOWGROUPCOUNTS, CLS_EX_SHOWGROUPCOUNTS, 0 },
	{ IDC_HIDECOUNTSWHENEMPTY, CLS_EX_HIDECOUNTSWHENEMPTY, 0 },
	{ IDC_LINEWITHGROUPS, CLS_EX_LINEWITHGROUPS, 0 },
	{ IDC_QUICKSEARCHVISONLY, CLS_EX_QUICKSEARCHVISONLY, 0 },
	{ IDC_SORTGROUPSALPHA, CLS_EX_SORTGROUPSALPHA, 0 },
	{ IDC_NOTNOSMOOTHSCROLLING, CLS_EX_NOSMOOTHSCROLLING, 1 }
};

struct CheckBoxValues_t
{
	DWORD  style;
	wchar_t* szDescr;
};

static const struct CheckBoxValues_t greyoutValues[] = {
	{ GREYF_UNFOCUS,  LPGENW("Not focused")   },
	{ MODEF_OFFLINE,  LPGENW("Offline")       },
	{ PF2_ONLINE,     LPGENW("Online")        },
	{ PF2_SHORTAWAY,  LPGENW("Away")          },
	{ PF2_LONGAWAY,   LPGENW("Not available") },
	{ PF2_LIGHTDND,   LPGENW("Occupied")      },
	{ PF2_HEAVYDND,   LPGENW("Do not disturb")},
	{ PF2_FREECHAT,   LPGENW("Free for chat") },
	{ PF2_INVISIBLE,  LPGENW("Invisible")     },
};

static const struct CheckBoxValues_t offlineValues[] = {
	{ MODEF_OFFLINE,  LPGENW("Offline")       },
	{ PF2_ONLINE,     LPGENW("Online")        },
	{ PF2_SHORTAWAY,  LPGENW("Away")          },
	{ PF2_LONGAWAY,   LPGENW("Not available") },
	{ PF2_LIGHTDND,   LPGENW("Occupied")      },
	{ PF2_HEAVYDND,   LPGENW("Do not disturb")},
	{ PF2_FREECHAT,   LPGENW("Free for chat") },
	{ PF2_INVISIBLE,  LPGENW("Invisible")     },
};

static void FillCheckBoxTree(HWND hwndTree, const struct CheckBoxValues_t *values, int nValues, DWORD style)
{
	TVINSERTSTRUCT tvis;
	tvis.hParent = nullptr;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	for (int i = 0; i < nValues; i++) {
		tvis.item.lParam = values[i].style;
		tvis.item.pszText = TranslateW( values[i].szDescr );
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		tvis.item.state = INDEXTOSTATEIMAGEMASK((style & tvis.item.lParam) != 0 ? 2 : 1);
		TreeView_InsertItem( hwndTree, &tvis);
}	}

static DWORD MakeCheckBoxTreeFlags(HWND hwndTree)
{
	DWORD flags = 0;
	TVITEM tvi;

	tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
	tvi.hItem = TreeView_GetRoot(hwndTree);
	while (tvi.hItem) {
		TreeView_GetItem(hwndTree, &tvi);
		if (((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
			flags |= tvi.lParam;
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return flags;
}


static INT_PTR CALLBACK DlgProcClcMainOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	wchar_t tmp[1024];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), GWL_STYLE,
			GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		{
			DWORD exStyle = db_get_dw(0, "CLC", "ExStyle", Clist_GetDefaultExStyle());
			for (auto &it : checkBoxToStyleEx)
				CheckDlgButton(hwndDlg, it.id, (exStyle & it.flag) ^ (it.flag * it.not) ? BST_CHECKED : BST_UNCHECKED);
		}
		{
			UDACCEL accel[2] = { {0, 10} , {2, 50} };
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETRANGE, 0, MAKELONG(999, 0));
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETACCEL, _countof(accel), (LPARAM)& accel);
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETPOS, 0,
				MAKELONG(db_get_w(0, "CLC", "ScrollTime", CLCDEFAULT_SCROLLTIME), 0));
		}
		CheckDlgButton(hwndDlg, IDC_IDLE, db_get_b(0, "CLC", "ShowIdle", CLCDEFAULT_SHOWIDLE) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(64, 0));
		SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETPOS, 0,
			MAKELONG(db_get_b(0, "CLC", "LeftMargin", CLCDEFAULT_LEFTMARGIN), 0));
		SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_SETPOS, 0,
			MAKELONG(db_get_b(0, "CLC", "GroupIndent", CLCDEFAULT_GROUPINDENT), 0));
		EnableWindow(GetDlgItem(hwndDlg, IDC_SMOOTHTIME), IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
		FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), offlineValues, _countof(offlineValues),
			db_get_dw(0, "CLC", "OfflineModes", CLCDEFAULT_OFFLINEMODES));
		CheckDlgButton(hwndDlg, IDC_NOSCROLLBAR, db_get_b(0, "CLC", "NoVScrollBar", 0) ? BST_CHECKED : BST_UNCHECKED);

		SetDlgItemText(hwndDlg, IDC_T_CONTACT, MyDBGetContactSettingTString(NULL, "CLC", "TemplateContact", tmp, 1024, TranslateT("%name% [%status% %protocol%] %status_message%")));
		SendDlgItemMessage(hwndDlg, IDC_T_CONTACT, EM_LIMITTEXT, 256, 0);
		SetDlgItemText(hwndDlg, IDC_T_GROUP, MyDBGetContactSettingTString(NULL, "CLC", "TemplateGroup", tmp, 1024, TranslateT("Group: %name% %count% [%mode%]")));
		SendDlgItemMessage(hwndDlg, IDC_T_GROUP, EM_LIMITTEXT, 256, 0);
		SetDlgItemText(hwndDlg, IDC_T_DIVIDER, MyDBGetContactSettingTString(NULL, "CLC", "TemplateDivider", tmp, 1024, TranslateT("Divider: %s")));
		SendDlgItemMessage(hwndDlg, IDC_T_DIVIDER, EM_LIMITTEXT, 256, 0);
		SetDlgItemText(hwndDlg, IDC_T_INFO, MyDBGetContactSettingTString(NULL, "CLC", "TemplateInfo", tmp, 1024, TranslateT("Info: %s")));
		SendDlgItemMessage(hwndDlg, IDC_T_INFO, EM_LIMITTEXT, 256, 0);
		return TRUE;
	case WM_VSCROLL:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_NOTNOSMOOTHSCROLLING)
			EnableWindow(GetDlgItem(hwndDlg, IDC_SMOOTHTIME), IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
		if ((LOWORD(wParam) == IDC_LEFTMARGIN || LOWORD(wParam) == IDC_SMOOTHTIME || LOWORD(wParam) == IDC_GROUPINDENT)
			&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		if ((LOWORD(wParam) == IDC_T_CONTACT || LOWORD(wParam) == IDC_T_GROUP || LOWORD(wParam) == IDC_T_DIVIDER || LOWORD(wParam) == IDC_T_INFO)
			&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_HIDEOFFLINEOPTS:
			if (((LPNMHDR)lParam)->code == NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti))
					if (hti.flags & TVHT_ONITEMSTATEICON) {
						TVITEM tvi;
						tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
						tvi.hItem = hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						tvi.iImage = tvi.iSelectedImage = tvi.iImage == 1 ? 2 : 1;
						TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
			}
			break;
		case 0:
			if (((LPNMHDR)lParam)->code == PSN_APPLY) {
				int i;
				DWORD exStyle = 0;
				for (i = 0; i < _countof(checkBoxToStyleEx); i++)
					if ((IsDlgButtonChecked(hwndDlg, checkBoxToStyleEx[i].id) == 0) == checkBoxToStyleEx[i].not)
						exStyle |= checkBoxToStyleEx[i].flag;

				db_set_dw(0, "CLC", "ExStyle", exStyle);
				db_set_b(0, "CLC", "ShowIdle", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_IDLE) ? 1 : 0));
				db_set_dw(0, "CLC", "OfflineModes", MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS)));
				db_set_b(0, "CLC", "LeftMargin",
					(BYTE)SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_GETPOS, 0, 0));
				db_set_w(0, "CLC", "ScrollTime",
					(WORD)SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_GETPOS, 0, 0));
				db_set_b(0, "CLC", "GroupIndent",
					(BYTE)SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_GETPOS, 0, 0));
				db_set_b(0, "CLC", "NoVScrollBar", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_NOSCROLLBAR) ? 1 : 0));

				GetDlgItemText(hwndDlg, IDC_T_CONTACT, tmp, _countof(tmp));
				db_set_ws(0, "CLC", "TemplateContact", tmp);
				GetDlgItemText(hwndDlg, IDC_T_GROUP, tmp, _countof(tmp));
				db_set_ws(0, "CLC", "TemplateGroup", tmp);
				GetDlgItemText(hwndDlg, IDC_T_DIVIDER, tmp, _countof(tmp));
				db_set_ws(0, "CLC", "TemplateDivider", tmp);
				GetDlgItemText(hwndDlg, IDC_T_INFO, tmp, _countof(tmp));
				db_set_ws(0, "CLC", "TemplateInfo", tmp);

				Clist_ClcOptionsChanged();
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

int ClcOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.a = LPGEN("Contact list");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLC);
	odp.szTitle.a = LPGEN("List");
	odp.pfnDlgProc = DlgProcClcMainOpts;
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

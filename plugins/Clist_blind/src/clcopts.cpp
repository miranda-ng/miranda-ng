/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

#include "commonheaders.h"

TCHAR* MyDBGetContactSettingTString(MCONTACT hContact, char* module, char* setting, TCHAR* out, size_t len, TCHAR *def);


#define DBFONTF_BOLD       1
#define DBFONTF_ITALIC     2
#define DBFONTF_UNDERLINE  4

struct CheckBoxToStyleEx_t
{
	int id;
	DWORD flag;
	int not;
}

static const checkBoxToStyleEx[] = {
//	{IDC_DISABLEDRAGDROP, CLS_EX_DISABLEDRAGDROP, 0},
//	{IDC_NOTEDITLABELS, CLS_EX_EDITLABELS, 1},
//	{IDC_SHOWSELALWAYS, CLS_EX_SHOWSELALWAYS, 0},
	{IDC_TRACKSELECT, CLS_EX_TRACKSELECT, 0},
	{IDC_SHOWGROUPCOUNTS, CLS_EX_SHOWGROUPCOUNTS, 0},
	{IDC_HIDECOUNTSWHENEMPTY, CLS_EX_HIDECOUNTSWHENEMPTY, 0},
//	{IDC_DIVIDERONOFF, CLS_EX_DIVIDERONOFF, 0},
//	{IDC_NOTNOTRANSLUCENTSEL, CLS_EX_NOTRANSLUCENTSEL, 1},
	{IDC_LINEWITHGROUPS, CLS_EX_LINEWITHGROUPS, 0},
	{IDC_QUICKSEARCHVISONLY, CLS_EX_QUICKSEARCHVISONLY, 0},
	{IDC_SORTGROUPSALPHA, CLS_EX_SORTGROUPSALPHA, 0},
	{IDC_NOTNOSMOOTHSCROLLING, CLS_EX_NOSMOOTHSCROLLING, 1}
};

struct CheckBoxValues_t
{
	DWORD  style;
	TCHAR* szDescr;
};

static const struct CheckBoxValues_t greyoutValues[] = {
	{ GREYF_UNFOCUS,  LPGENT("Not focused")   },
	{ MODEF_OFFLINE,  LPGENT("Offline")       },
	{ PF2_ONLINE,     LPGENT("Online")        },
	{ PF2_SHORTAWAY,  LPGENT("Away")          },
	{ PF2_LONGAWAY,   LPGENT("NA")            },
	{ PF2_LIGHTDND,   LPGENT("Occupied")      },
	{ PF2_HEAVYDND,   LPGENT("DND")           },
	{ PF2_FREECHAT,   LPGENT("Free for chat") },
	{ PF2_INVISIBLE,  LPGENT("Invisible")     },
	{ PF2_OUTTOLUNCH, LPGENT("Out to lunch")  },
	{ PF2_ONTHEPHONE, LPGENT("On the phone")  }
};

static const struct CheckBoxValues_t offlineValues[] = {
	{ MODEF_OFFLINE,  LPGENT("Offline")       },
	{ PF2_ONLINE,     LPGENT("Online")        },
	{ PF2_SHORTAWAY,  LPGENT("Away")          },
	{ PF2_LONGAWAY,   LPGENT("NA")            },
	{ PF2_LIGHTDND,   LPGENT("Occupied")      },
	{ PF2_HEAVYDND,   LPGENT("DND")           },
	{ PF2_FREECHAT,   LPGENT("Free for chat") },
	{ PF2_INVISIBLE,  LPGENT("Invisible")     },
	{ PF2_OUTTOLUNCH, LPGENT("Out to lunch")  },
	{ PF2_ONTHEPHONE, LPGENT("On the phone")  }
};

static void FillCheckBoxTree(HWND hwndTree, const struct CheckBoxValues_t *values, int nValues, DWORD style)
{
	TVINSERTSTRUCT tvis;
	int i;

	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	for (i = 0; i < nValues; i++) {
		tvis.item.lParam = values[i].style;
		tvis.item.pszText = TranslateTS( values[i].szDescr );
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
	TCHAR tmp[1024];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), GWL_STYLE,
			GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), GWL_STYLE,
			GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		{
			int i;
			DWORD exStyle = db_get_dw(NULL, "CLC", "ExStyle", pcli->pfnGetDefaultExStyle());
			for (i = 0; i < SIZEOF(checkBoxToStyleEx); i++)
				CheckDlgButton(hwndDlg, checkBoxToStyleEx[i].id,
				(exStyle & checkBoxToStyleEx[i].flag) ^ (checkBoxToStyleEx[i].flag *
				checkBoxToStyleEx[i].not) ? BST_CHECKED : BST_UNCHECKED);
		}
		{
			UDACCEL accel[2] = { {0, 10} , {2, 50} };
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETRANGE, 0, MAKELONG(999, 0));
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETACCEL, SIZEOF(accel), (LPARAM) & accel);
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETPOS, 0,
				MAKELONG(db_get_w(NULL, "CLC", "ScrollTime", CLCDEFAULT_SCROLLTIME), 0));
		}
		CheckDlgButton(hwndDlg, IDC_IDLE, db_get_b(NULL, "CLC", "ShowIdle", CLCDEFAULT_SHOWIDLE) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(64, 0));
		SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETPOS, 0,
			MAKELONG(db_get_b(NULL, "CLC", "LeftMargin", CLCDEFAULT_LEFTMARGIN), 0));
		SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_SETPOS, 0,
			MAKELONG(db_get_b(NULL, "CLC", "GroupIndent", CLCDEFAULT_GROUPINDENT), 0));
		CheckDlgButton(hwndDlg, IDC_GREYOUT,
			db_get_dw(NULL, "CLC", "GreyoutFlags", CLCDEFAULT_GREYOUTFLAGS) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_SMOOTHTIME), IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
		EnableWindow(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), IsDlgButtonChecked(hwndDlg, IDC_GREYOUT));
		FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), greyoutValues, SIZEOF(greyoutValues),
			db_get_dw(NULL, "CLC", "FullGreyoutFlags", CLCDEFAULT_FULLGREYOUTFLAGS));
		FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), offlineValues, SIZEOF(offlineValues),
			db_get_dw(NULL, "CLC", "OfflineModes", CLCDEFAULT_OFFLINEMODES));
		CheckDlgButton(hwndDlg, IDC_NOSCROLLBAR, db_get_b(NULL, "CLC", "NoVScrollBar", 0) ? BST_CHECKED : BST_UNCHECKED);

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
		if (LOWORD(wParam) == IDC_GREYOUT)
			EnableWindow(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), IsDlgButtonChecked(hwndDlg, IDC_GREYOUT));
		if ((LOWORD(wParam) == IDC_LEFTMARGIN || LOWORD(wParam) == IDC_SMOOTHTIME || LOWORD(wParam) == IDC_GROUPINDENT)
			&& (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
			return 0;
		if ((LOWORD(wParam) == IDC_T_CONTACT || LOWORD(wParam) == IDC_T_GROUP || LOWORD(wParam) == IDC_T_DIVIDER || LOWORD(wParam) == IDC_T_INFO)
			&& (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->idFrom) {
		case IDC_GREYOUTOPTS:
		case IDC_HIDEOFFLINEOPTS:
			if (((LPNMHDR) lParam)->code == NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x = (short) LOWORD(GetMessagePos());
				hti.pt.y = (short) HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR) lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR) lParam)->hwndFrom, &hti))
					if (hti.flags & TVHT_ONITEMSTATEICON) {
						TVITEM tvi;
						tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
						tvi.hItem = hti.hItem;
						TreeView_GetItem(((LPNMHDR) lParam)->hwndFrom, &tvi);
						tvi.iImage = tvi.iSelectedImage = tvi.iImage == 1 ? 2 : 1;
						TreeView_SetItem(((LPNMHDR) lParam)->hwndFrom, &tvi);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
			}
			break;
		case 0:
			if (((LPNMHDR) lParam)->code == PSN_APPLY ) {
				int i;
				DWORD exStyle = 0;
				for (i = 0; i < SIZEOF(checkBoxToStyleEx); i++)
					if ((IsDlgButtonChecked(hwndDlg, checkBoxToStyleEx[i].id) == 0) == checkBoxToStyleEx[i].not)
						exStyle |= checkBoxToStyleEx[i].flag;

				db_set_dw(NULL, "CLC", "ExStyle", exStyle);
				{
					DWORD fullGreyoutFlags = MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS));
					db_set_dw(NULL, "CLC", "FullGreyoutFlags", fullGreyoutFlags);
					if (IsDlgButtonChecked(hwndDlg, IDC_GREYOUT))
						db_set_dw(NULL, "CLC", "GreyoutFlags", fullGreyoutFlags);
					else
						db_set_dw(NULL, "CLC", "GreyoutFlags", 0);
				}
				db_set_b(NULL, "CLC", "ShowIdle", (BYTE) (IsDlgButtonChecked(hwndDlg, IDC_IDLE) ? 1 : 0));
				db_set_dw(NULL, "CLC", "OfflineModes", MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS)));
				db_set_b(NULL, "CLC", "LeftMargin",
					(BYTE) SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_GETPOS, 0, 0));
				db_set_w(NULL, "CLC", "ScrollTime",
					(WORD) SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CLC", "GroupIndent",
					(BYTE) SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CLC", "NoVScrollBar", (BYTE) (IsDlgButtonChecked(hwndDlg, IDC_NOSCROLLBAR) ? 1 : 0));

				GetDlgItemText(hwndDlg, IDC_T_CONTACT, tmp, SIZEOF(tmp));
				db_set_ts(NULL, "CLC", "TemplateContact", tmp);
				GetDlgItemText(hwndDlg, IDC_T_GROUP, tmp, SIZEOF(tmp));
				db_set_ts(NULL, "CLC", "TemplateGroup", tmp);
				GetDlgItemText(hwndDlg, IDC_T_DIVIDER, tmp, SIZEOF(tmp));
				db_set_ts(NULL, "CLC", "TemplateDivider", tmp);
				GetDlgItemText(hwndDlg, IDC_T_INFO, tmp, SIZEOF(tmp));
				db_set_ts(NULL, "CLC", "TemplateInfo", tmp);

				pcli->pfnClcOptionsChanged();
				return TRUE;
			}
			break;
		}
		break;
	case WM_DESTROY:
		ImageList_Destroy(TreeView_GetImageList(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), TVSIL_NORMAL));
		break;
	}
	return FALSE;
}




/****************************************************************************************/

int ClcOptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInst;
	odp.pszGroup = LPGEN("Contact list");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLC);
	odp.pszTitle = LPGEN("List");
	odp.pfnDlgProc = DlgProcClcMainOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}

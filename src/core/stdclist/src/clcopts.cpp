/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#define DBFONTF_BOLD       1
#define DBFONTF_ITALIC     2
#define DBFONTF_UNDERLINE  4

struct CheckBoxToStyleEx_t
{
	int id;
	uint32_t flag;
	int not;
}

static const checkBoxToStyleEx[] =
{
	{ IDC_DISABLEDRAGDROP,      CLS_EX_DISABLEDRAGDROP,     0 },
	{ IDC_NOTEDITLABELS,        CLS_EX_EDITLABELS,          1 },
	{ IDC_SHOWSELALWAYS,        CLS_EX_SHOWSELALWAYS,       0 },
	{ IDC_TRACKSELECT,          CLS_EX_TRACKSELECT,         0 },
	{ IDC_SHOWGROUPCOUNTS,      CLS_EX_SHOWGROUPCOUNTS,     0 },
	{ IDC_HIDECOUNTSWHENEMPTY,  CLS_EX_HIDECOUNTSWHENEMPTY, 0 },
	{ IDC_DIVIDERONOFF,         CLS_EX_DIVIDERONOFF,        0 },
	{ IDC_NOTNOTRANSLUCENTSEL,  CLS_EX_NOTRANSLUCENTSEL,    1 },
	{ IDC_LINEWITHGROUPS,       CLS_EX_LINEWITHGROUPS,      0 },
	{ IDC_QUICKSEARCHVISONLY,   CLS_EX_QUICKSEARCHVISONLY,  0 },
	{ IDC_SORTGROUPSALPHA,      CLS_EX_SORTGROUPSALPHA,     0 },
	{ IDC_NOTNOSMOOTHSCROLLING, CLS_EX_NOSMOOTHSCROLLING,   1 }
};

struct CheckBoxValues_t
{
	uint32_t  style;
	wchar_t* szDescr;
}
static const greyoutValues[] =
{
	{ GREYF_UNFOCUS,  LPGENW("Not focused")   },
	{ MODEF_OFFLINE,  LPGENW("Offline")       },
	{ PF2_ONLINE,     LPGENW("Online")        },
	{ PF2_SHORTAWAY,  LPGENW("Away")          },
	{ PF2_LONGAWAY,   LPGENW("Not available") },
	{ PF2_LIGHTDND,   LPGENW("Occupied")      },
	{ PF2_HEAVYDND,   LPGENW("Do not disturb")},
	{ PF2_FREECHAT,   LPGENW("Free for chat") },
	{ PF2_INVISIBLE,  LPGENW("Invisible")     }
};

static void FillCheckBoxTree(HWND hwndTree, const struct CheckBoxValues_t *values, int nValues, uint32_t style)
{
	TVINSERTSTRUCT tvis;
	tvis.hParent = nullptr;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	for (int i = 0; i < nValues; i++) {
		tvis.item.lParam = values[i].style;
		tvis.item.pszText = TranslateW(values[i].szDescr);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		tvis.item.state = INDEXTOSTATEIMAGEMASK((style & tvis.item.lParam) != 0 ? 2 : 1);
		TreeView_InsertItem(hwndTree, &tvis);
	}
}

static uint32_t MakeCheckBoxTreeFlags(HWND hwndTree)
{
	uint32_t flags = 0;

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

static LONG CalcMinRowHeight()
{
	LONG minHeight = 16;
	HDC hdc = GetDC(nullptr);
	for (int i = 0; i < FONTID_LAST; i++) {
		LOGFONT lf;
		COLORREF color;
		Clist_GetFontSetting(i, &lf, &color);
		lf.lfHeight = -MulDiv(lf.lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		HFONT hFont = CreateFontIndirect(&lf);
		hFont = (HFONT)SelectObject(hdc, hFont);
		SIZE fontSize;
		GetTextExtentPoint32(hdc, L"x", 1, &fontSize);
		if (fontSize.cy > minHeight)
			minHeight = fontSize.cy;
		hFont = (HFONT)SelectObject(hdc, hFont);
		DeleteObject(hFont);
	}
	ReleaseDC(nullptr, hdc);
	return minHeight;
}

static INT_PTR CALLBACK DlgProcClcMainOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), GWL_STYLE,
			GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		{
			uint32_t exStyle = db_get_dw(0, "CLC", "ExStyle", Clist_GetDefaultExStyle());
			for (auto &it : checkBoxToStyleEx)
				CheckDlgButton(hwndDlg, it.id, (exStyle & it.flag) ^ (it.flag * it.not) ? BST_CHECKED : BST_UNCHECKED);
		}
		{
			UDACCEL accel[2] = { { 0, 10 }, { 2, 50 } };
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
		CheckDlgButton(hwndDlg, IDC_GREYOUT,
			db_get_dw(0, "CLC", "GreyoutFlags", CLCDEFAULT_GREYOUTFLAGS) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_SMOOTHTIME), IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
		EnableWindow(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), IsDlgButtonChecked(hwndDlg, IDC_GREYOUT));
		FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), greyoutValues, _countof(greyoutValues),
			db_get_dw(0, "CLC", "FullGreyoutFlags", CLCDEFAULT_FULLGREYOUTFLAGS));
		CheckDlgButton(hwndDlg, IDC_NOSCROLLBAR, db_get_b(0, "CLC", "NoVScrollBar", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_GAMMACORRECT, db_get_b(0, "CLC", "GammaCorrect", CLCDEFAULT_GAMMACORRECT) ? BST_CHECKED : BST_UNCHECKED);
		{
			LONG minHeight = CalcMinRowHeight();
			LONG rowHeight = db_get_b(0, "CLC", "RowHeight", CLCDEFAULT_ROWHEIGHT);
			if (rowHeight < minHeight) {
				rowHeight = minHeight;
			}
			SendDlgItemMessage(hwndDlg, IDC_ROWHEIGHTSPIN, UDM_SETRANGE, 0, MAKELONG(255, minHeight));
			SendDlgItemMessage(hwndDlg, IDC_ROWHEIGHTSPIN, UDM_SETPOS, 0, MAKELONG(rowHeight, 0));
		}
		return TRUE;

	case WM_SETFOCUS:
		{
			LONG minHeight = CalcMinRowHeight();
			LONG rowHeight = SendDlgItemMessage(hwndDlg, IDC_ROWHEIGHTSPIN, UDM_GETPOS, 0, 0);
			if (rowHeight < minHeight) {
				rowHeight = minHeight;
				SendDlgItemMessage(hwndDlg, IDC_ROWHEIGHTSPIN, UDM_SETPOS, 0, MAKELONG(rowHeight, 0));
			}
			SendDlgItemMessage(hwndDlg, IDC_ROWHEIGHTSPIN, UDM_SETRANGE, 0, MAKELONG(255, minHeight));
		}
		break;

	case WM_VSCROLL:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_NOTNOSMOOTHSCROLLING)
			EnableWindow(GetDlgItem(hwndDlg, IDC_SMOOTHTIME), IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
		if (LOWORD(wParam) == IDC_GREYOUT)
			EnableWindow(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), IsDlgButtonChecked(hwndDlg, IDC_GREYOUT));
		if ((LOWORD(wParam) == IDC_LEFTMARGIN || LOWORD(wParam) == IDC_SMOOTHTIME || LOWORD(wParam) == IDC_GROUPINDENT
			|| LOWORD(wParam) == IDC_ROWHEIGHT) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_GREYOUTOPTS:
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
				uint32_t exStyle = 0;
				for (auto &it : checkBoxToStyleEx)
					if ((IsDlgButtonChecked(hwndDlg, it.id) == 0) == it.not)
						exStyle |= it.flag;

				db_set_dw(0, "CLC", "ExStyle", exStyle);
				{
					uint32_t fullGreyoutFlags = MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS));
					db_set_dw(0, "CLC", "FullGreyoutFlags", fullGreyoutFlags);
					if (IsDlgButtonChecked(hwndDlg, IDC_GREYOUT))
						db_set_dw(0, "CLC", "GreyoutFlags", fullGreyoutFlags);
					else
						db_set_dw(0, "CLC", "GreyoutFlags", 0);
				}
				db_set_b(0, "CLC", "ShowIdle", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_IDLE) ? 1 : 0));
				db_set_b(0, "CLC", "LeftMargin", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_GETPOS, 0, 0));
				db_set_w(0, "CLC", "ScrollTime", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_GETPOS, 0, 0));
				db_set_b(0, "CLC", "GroupIndent", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_GETPOS, 0, 0));
				db_set_b(0, "CLC", "NoVScrollBar", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_NOSCROLLBAR) ? 1 : 0));
				db_set_b(0, "CLC", "RowHeight", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_ROWHEIGHTSPIN, UDM_GETPOS, 0, 0));
				db_set_b(0, "CLC", "GammaCorrect", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_GAMMACORRECT));
				Clist_ClcOptionsChanged();
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

static INT_PTR CALLBACK DlgProcClcBkgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_BITMAP,
			db_get_b(0, "CLC", "UseBitmap", CLCDEFAULT_USEBITMAP) ? BST_CHECKED : BST_UNCHECKED);
		SendMessage(hwndDlg, WM_USER + 10, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETDEFAULTCOLOUR, 0, CLCDEFAULT_BKCOLOUR);
		SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETCOLOUR, 0, db_get_dw(0, "CLC", "BkColour", CLCDEFAULT_BKCOLOUR));
		SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_SETDEFAULTCOLOUR, 0, CLCDEFAULT_SELBKCOLOUR);
		SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_SETCOLOUR, 0,
			db_get_dw(0, "CLC", "SelBkColour", CLCDEFAULT_SELBKCOLOUR));
		CheckDlgButton(hwndDlg, IDC_WINCOLOUR, db_get_b(0, "CLC", "UseWinColours", 0) ? BST_CHECKED : BST_UNCHECKED);
		SendMessage(hwndDlg, WM_USER + 11, 0, 0);
		{
			DBVARIANT dbv;
			if (!db_get_s(0, "CLC", "BkBitmap", &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_FILENAME, dbv.pszVal);

				char szPath[MAX_PATH];
				if (PathToAbsolute(dbv.pszVal, szPath))
					SetDlgItemTextA(hwndDlg, IDC_FILENAME, szPath);

				db_free(&dbv);
			}
		}
		{
			uint16_t bmpUse = db_get_w(0, "CLC", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
			CheckDlgButton(hwndDlg, IDC_STRETCHH, bmpUse & CLB_STRETCHH ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_STRETCHV, bmpUse & CLB_STRETCHV ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TILEH, bmpUse & CLBF_TILEH ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TILEV, bmpUse & CLBF_TILEV ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SCROLL, bmpUse & CLBF_SCROLL ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_PROPORTIONAL, bmpUse & CLBF_PROPORTIONAL ? BST_CHECKED : BST_UNCHECKED);
		}

		SHAutoComplete(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
		return TRUE;

	case WM_USER + 10:
		EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_STRETCHH), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_STRETCHV), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_TILEH), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_TILEV), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_PROPORTIONAL), IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		break;

	case WM_USER + 11:
		{
			BOOL b = IsDlgButtonChecked(hwndDlg, IDC_WINCOLOUR);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BKGCOLOUR), !b);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SELCOLOUR), !b);
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BROWSE) {
			wchar_t str[MAX_PATH], filter[512];
			GetDlgItemText(hwndDlg, IDC_FILENAME, str, _countof(str));

			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = hwndDlg;
			ofn.hInstance = nullptr;
			Bitmap_GetFilter(filter, _countof(filter));
			ofn.lpstrFilter = filter;
			ofn.lpstrFile = str;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.nMaxFile = _countof(str);
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrDefExt = L"bmp";
			if (!GetOpenFileName(&ofn))
				break;
			SetDlgItemText(hwndDlg, IDC_FILENAME, str);
		}
		else if (LOWORD(wParam) == IDC_FILENAME && HIWORD(wParam) != EN_CHANGE)
			break;
		if (LOWORD(wParam) == IDC_BITMAP)
			SendMessage(hwndDlg, WM_USER + 10, 0, 0);
		if (LOWORD(wParam) == IDC_WINCOLOUR)
			SendMessage(hwndDlg, WM_USER + 11, 0, 0);
		if (LOWORD(wParam) == IDC_FILENAME && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(0, "CLC", "UseBitmap", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
				{
					COLORREF col;
					col = SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_GETCOLOUR, 0, 0);
					if (col == CLCDEFAULT_BKCOLOUR)
						db_unset(0, "CLC", "BkColour");
					else
						db_set_dw(0, "CLC", "BkColour", col);
					col = SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_GETCOLOUR, 0, 0);
					if (col == CLCDEFAULT_SELBKCOLOUR)
						db_unset(0, "CLC", "SelBkColour");
					else
						db_set_dw(0, "CLC", "SelBkColour", col);
					db_set_b(0, "CLC", "UseWinColours", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_WINCOLOUR)));
				}
				{
					char str[MAX_PATH], strrel[MAX_PATH];
					GetDlgItemTextA(hwndDlg, IDC_FILENAME, str, _countof(str));
					if (PathToRelative(str, strrel))
						db_set_s(0, "CLC", "BkBitmap", strrel);
					else
						db_set_s(0, "CLC", "BkBitmap", str);
				}
				{
					uint16_t flags = 0;
					if (IsDlgButtonChecked(hwndDlg, IDC_STRETCHH))
						flags |= CLB_STRETCHH;
					if (IsDlgButtonChecked(hwndDlg, IDC_STRETCHV))
						flags |= CLB_STRETCHV;
					if (IsDlgButtonChecked(hwndDlg, IDC_TILEH))
						flags |= CLBF_TILEH;
					if (IsDlgButtonChecked(hwndDlg, IDC_TILEV))
						flags |= CLBF_TILEV;
					if (IsDlgButtonChecked(hwndDlg, IDC_SCROLL))
						flags |= CLBF_SCROLL;
					if (IsDlgButtonChecked(hwndDlg, IDC_PROPORTIONAL))
						flags |= CLBF_PROPORTIONAL;
					db_set_w(0, "CLC", "BkBmpUse", flags);
				}
				Clist_ClcOptionsChanged();
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

/****************************************************************************************/

int ClcOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.a = LPGEN("Contact list");
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTab.a = LPGEN("List");
	odp.pfnDlgProc = DlgProcClcMainOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLC);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("List background");
	odp.pfnDlgProc = DlgProcClcBkgOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLCBKG);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

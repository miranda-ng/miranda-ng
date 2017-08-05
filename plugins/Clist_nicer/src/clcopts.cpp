/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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
#include "coolscroll.h"

#include <m_extraicons.h>

#define DBFONTF_BOLD		1
#define DBFONTF_ITALIC		2
#define DBFONTF_UNDERLINE	4

static INT_PTR CALLBACK DlgProcClcMainOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcClcBkgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcViewModesSetup(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcCluiOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcSBarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcGenOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

struct CheckBoxToStyleEx_t {
	int id;
	DWORD flag;
	int not_t;
} static const checkBoxToStyleEx[] = {
	{ IDC_DISABLEDRAGDROP, CLS_EX_DISABLEDRAGDROP, 0 }, { IDC_NOTEDITLABELS, CLS_EX_EDITLABELS, 1 },
	{ IDC_SHOWSELALWAYS, CLS_EX_SHOWSELALWAYS, 0 }, { IDC_TRACKSELECT, CLS_EX_TRACKSELECT, 0 },
	{ IDC_DIVIDERONOFF, CLS_EX_DIVIDERONOFF, 0 }, { IDC_NOTNOTRANSLUCENTSEL, CLS_EX_NOTRANSLUCENTSEL, 1 },
	{ IDC_NOTNOSMOOTHSCROLLING, CLS_EX_NOSMOOTHSCROLLING, 1 }
};

struct CheckBoxToGroupStyleEx_t {
	int id;
	DWORD flag;
	int not_t;
} static const checkBoxToGroupStyleEx[] = {
	{ IDC_SHOWGROUPCOUNTS, CLS_EX_SHOWGROUPCOUNTS, 0 }, { IDC_HIDECOUNTSWHENEMPTY, CLS_EX_HIDECOUNTSWHENEMPTY, 0 },
	{ IDC_LINEWITHGROUPS, CLS_EX_LINEWITHGROUPS, 0 }, { IDC_QUICKSEARCHVISONLY, CLS_EX_QUICKSEARCHVISONLY, 0 },
	{ IDC_SORTGROUPSALPHA, CLS_EX_SORTGROUPSALPHA, 0 }
};

struct CheckBoxValues_t {
	DWORD style;
	wchar_t *szDescr;
};

static const struct CheckBoxValues_t greyoutValues[] = {
	{ GREYF_UNFOCUS, LPGENW("Not focused") }, { MODEF_OFFLINE, LPGENW("Offline") }, { PF2_ONLINE, LPGENW("Online") }, { PF2_SHORTAWAY, LPGENW("Away") },
	{ PF2_LONGAWAY, LPGENW("Not available") }, { PF2_LIGHTDND, LPGENW("Occupied") }, { PF2_HEAVYDND, LPGENW("Do not disturb") }, { PF2_FREECHAT, LPGENW("Free for chat") },
	{ PF2_INVISIBLE, LPGENW("Invisible") }, { PF2_OUTTOLUNCH, LPGENW("Out to lunch") }, { PF2_ONTHEPHONE, LPGENW("On the phone") }
};
static const struct CheckBoxValues_t offlineValues[] = {
	{ MODEF_OFFLINE, LPGENW("Offline") }, { PF2_ONLINE, LPGENW("Online") }, { PF2_SHORTAWAY, LPGENW("Away") }, { PF2_LONGAWAY, LPGENW("Not available") },
	{ PF2_LIGHTDND, LPGENW("Occupied") }, { PF2_HEAVYDND, LPGENW("Do not disturb") }, { PF2_FREECHAT, LPGENW("Free for chat") }, { PF2_INVISIBLE, LPGENW("Invisible") },
	{ PF2_OUTTOLUNCH, LPGENW("Out to lunch") }, { PF2_ONTHEPHONE, LPGENW("On the phone") }
};

static UINT sortCtrlIDs[] = { IDC_SORTPRIMARY, IDC_SORTTHEN, IDC_SORTFINALLY, 0 };

static void FillCheckBoxTree(HWND hwndTree, const struct CheckBoxValues_t *values, int nValues, DWORD style)
{
	TVINSERTSTRUCT tvis;
	int i;

	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	for (i = 0; i < nValues; i++) {
		tvis.item.lParam = values[i].style;
		tvis.item.pszText = TranslateW(values[i].szDescr);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		tvis.item.state = INDEXTOSTATEIMAGEMASK((style & tvis.item.lParam) != 0 ? 2 : 1);
		TreeView_InsertItem(hwndTree, &tvis);
	}
}

static DWORD MakeCheckBoxTreeFlags(HWND hwndTree)
{
	DWORD flags = 0;
	TVITEM tvi;

	tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
	tvi.hItem = TreeView_GetRoot(hwndTree);
	while (tvi.hItem) {
		TreeView_GetItem(hwndTree, &tvi);
		if ((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2)
			flags |= tvi.lParam;
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return flags;
}

static void cfgSetFlag(HWND hwndDlg, int ctrlId, DWORD dwMask)
{
	if (IsDlgButtonChecked(hwndDlg, ctrlId))
		cfg::dat.dwFlags |= dwMask;
	else
		cfg::dat.dwFlags &= ~dwMask;
	db_set_dw(NULL, "CLUI", "Frameflags", cfg::dat.dwFlags);
}

void GetDefaultFontSetting(int i, LOGFONT *lf, COLORREF *colour)
{
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), lf, FALSE);
	*colour = GetSysColor(COLOR_WINDOWTEXT);
	switch (i) {
	case FONTID_GROUPS:
		lf->lfWeight = FW_BOLD;
		break;
	case FONTID_GROUPCOUNTS:
		lf->lfHeight = (int)(lf->lfHeight * .75);
		*colour = GetSysColor(COLOR_3DSHADOW);
		break;
	case FONTID_OFFINVIS:
	case FONTID_INVIS:
		lf->lfItalic = !lf->lfItalic;
		break;
	case FONTID_DIVIDERS:
		lf->lfHeight = (int)(lf->lfHeight * .75);
		break;
	case FONTID_NOTONLIST:
		*colour = GetSysColor(COLOR_3DSHADOW);
		break;
	}
}

static INT_PTR CALLBACK DlgProcDspGroups(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SendDlgItemMessage(hwndDlg, IDC_GROUPALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always left"));
			SendDlgItemMessage(hwndDlg, IDC_GROUPALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always right"));
			SendDlgItemMessage(hwndDlg, IDC_GROUPALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Automatic (RTL)"));

			DWORD exStyle = db_get_dw(NULL, "CLC", "ExStyle", pcli->pfnGetDefaultExStyle());
			for (int i = 0; i < _countof(checkBoxToGroupStyleEx); i++)
				CheckDlgButton(hwndDlg, checkBoxToGroupStyleEx[i].id, (exStyle & checkBoxToGroupStyleEx[i].flag) ^ (checkBoxToGroupStyleEx[i].flag * checkBoxToGroupStyleEx[i].not_t) ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_NOGROUPICON, (cfg::dat.dwFlags & CLUI_FRAME_NOGROUPICON) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CENTERGROUPNAMES, db_get_b(NULL, "CLCExt", "EXBK_CenterGroupnames", 0) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_GROUPALIGN, CB_SETCURSEL, cfg::dat.bGroupAlign, 0);

			SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(64, 0));
			SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETPOS, 0, db_get_b(NULL, "CLC", "LeftMargin", CLCDEFAULT_LEFTMARGIN));
			SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(64, 0));
			SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_SETPOS, 0, db_get_b(NULL, "CLC", "RightMargin", CLCDEFAULT_LEFTMARGIN));
			SendDlgItemMessage(hwndDlg, IDC_ROWGAPSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
			SendDlgItemMessage(hwndDlg, IDC_ROWGAPSPIN, UDM_SETPOS, 0, cfg::dat.bRowSpacing);
			SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
			SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_SETPOS, 0, db_get_b(NULL, "CLC", "GroupIndent", CLCDEFAULT_GROUPINDENT));
			SendDlgItemMessage(hwndDlg, IDC_ROWHEIGHTSPIN, UDM_SETRANGE, 0, MAKELONG(255, 8));
			SendDlgItemMessage(hwndDlg, IDC_ROWHEIGHTSPIN, UDM_SETPOS, 0, db_get_b(NULL, "CLC", "RowHeight", CLCDEFAULT_ROWHEIGHT));
			SendDlgItemMessage(hwndDlg, IDC_GROUPROWHEIGHTSPIN, UDM_SETRANGE, 0, MAKELONG(255, 8));
			SendDlgItemMessage(hwndDlg, IDC_GROUPROWHEIGHTSPIN, UDM_SETPOS, 0, db_get_b(NULL, "CLC", "GRowHeight", CLCDEFAULT_ROWHEIGHT));
			SendDlgItemMessage(hwndDlg, IDC_AVATARPADDINGSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
			SendDlgItemMessage(hwndDlg, IDC_AVATARPADDINGSPIN, UDM_SETPOS, 0, cfg::dat.avatarPadding);
		}
		return TRUE;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_ROWHEIGHT || LOWORD(wParam) == IDC_AVATARPADDING || LOWORD(wParam) == IDC_ROWGAP || LOWORD(wParam) == IDC_RIGHTMARGIN || LOWORD(wParam) == IDC_LEFTMARGIN || LOWORD(wParam) == IDC_GROUPINDENT || LOWORD(wParam) == IDC_GROUPROWHEIGHT)
			&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		if (LOWORD(wParam) == IDC_GROUPALIGN && (HIWORD(wParam) != CBN_SELCHANGE || (HWND)lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == PSN_APPLY) {
				DWORD exStyle = db_get_dw(NULL, "CLC", "ExStyle", pcli->pfnGetDefaultExStyle());

				for (int i = 0; i < _countof(checkBoxToGroupStyleEx); i++) {
					if ((IsDlgButtonChecked(hwndDlg, checkBoxToGroupStyleEx[i].id) == 0) == checkBoxToGroupStyleEx[i].not_t)
						exStyle |= checkBoxToGroupStyleEx[i].flag;
					else
						exStyle &= ~(checkBoxToGroupStyleEx[i].flag);
				}
				db_set_dw(NULL, "CLC", "ExStyle", exStyle);

				cfgSetFlag(hwndDlg, IDC_NOGROUPICON, CLUI_FRAME_NOGROUPICON);

				db_set_b(NULL, "CLCExt", "EXBK_CenterGroupnames", IsDlgButtonChecked(hwndDlg, IDC_CENTERGROUPNAMES) ? 1 : 0);

				LRESULT curSel = SendDlgItemMessage(hwndDlg, IDC_GROUPALIGN, CB_GETCURSEL, 0, 0);
				if (curSel != CB_ERR) {
					cfg::dat.bGroupAlign = (BYTE)curSel;
					db_set_b(NULL, "CLC", "GroupAlign", cfg::dat.bGroupAlign);
				}

				cfg::dat.bRowSpacing = (BYTE)SendDlgItemMessage(hwndDlg, IDC_ROWGAPSPIN, UDM_GETPOS, 0, 0);
				db_set_b(NULL, "CLC", "RowGap", cfg::dat.bRowSpacing);

				BOOL translated;
				cfg::dat.avatarPadding = (BYTE)GetDlgItemInt(hwndDlg, IDC_AVATARPADDING, &translated, FALSE);
				db_set_b(NULL, "CList", "AvatarPadding", cfg::dat.avatarPadding);

				db_set_b(NULL, "CLC", "LeftMargin", (BYTE)SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CLC", "RightMargin", (BYTE)SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CLC", "GroupIndent", (BYTE)SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CLC", "RowHeight", (BYTE)SendDlgItemMessage(hwndDlg, IDC_ROWHEIGHTSPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CLC", "GRowHeight", (BYTE)SendDlgItemMessage(hwndDlg, IDC_GROUPROWHEIGHTSPIN, UDM_GETPOS, 0, 0));
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcDspItems(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);

			for (int i = 0; sortCtrlIDs[i] != 0; i++) {
				SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_INSERTSTRING, -1, (LPARAM)TranslateT("Nothing"));
				SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_INSERTSTRING, -1, (LPARAM)TranslateT("Name"));
				SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_INSERTSTRING, -1, (LPARAM)TranslateT("Protocol"));
				SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_INSERTSTRING, -1, (LPARAM)TranslateT("Status"));
				SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_INSERTSTRING, -1, (LPARAM)TranslateT("Last message"));
				SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_INSERTSTRING, -1, (LPARAM)TranslateT("Message frequency"));
			}
			SendDlgItemMessage(hwndDlg, IDC_CLISTALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Never"));
			SendDlgItemMessage(hwndDlg, IDC_CLISTALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always"));
			SendDlgItemMessage(hwndDlg, IDC_CLISTALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("For RTL only"));
			SendDlgItemMessage(hwndDlg, IDC_CLISTALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("RTL TEXT only"));

			FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), offlineValues, sizeof(offlineValues) / sizeof(offlineValues[0]), db_get_dw(NULL, "CLC", "OfflineModes", CLCDEFAULT_OFFLINEMODES));
			CheckDlgButton(hwndDlg, IDC_EVENTSONTOP, (cfg::dat.dwFlags & CLUI_STICKYEVENTS) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_DONTSEPARATE, cfg::dat.bDontSeparateOffline ? BST_CHECKED : BST_UNCHECKED);
			for (int i = 0; sortCtrlIDs[i] != 0; i++)
				SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_SETCURSEL, cfg::dat.sortOrder[i], 0);

			SendDlgItemMessage(hwndDlg, IDC_CLISTALIGN, CB_SETCURSEL, cfg::dat.bUseDCMirroring, 0);
		}
		return TRUE;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_SORTPRIMARY || LOWORD(wParam) == IDC_SORTTHEN || LOWORD(wParam) == IDC_SORTFINALLY || LOWORD(wParam) == IDC_CLISTALIGN)
			&& (HIWORD(wParam) != CBN_SELCHANGE || (HWND)lParam != GetFocus()))
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
				for (int i = 0; sortCtrlIDs[i] != 0; i++) {
					LRESULT curSel = SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_GETCURSEL, 0, 0);
					if (curSel == 0 || curSel == CB_ERR)
						cfg::dat.sortOrder[i] = 0;
					else
						cfg::dat.sortOrder[i] = (BYTE)curSel;
				}
				db_set_dw(NULL, "CList", "SortOrder", MAKELONG(MAKEWORD(cfg::dat.sortOrder[0], cfg::dat.sortOrder[1]), MAKEWORD(cfg::dat.sortOrder[2], 0)));

				cfg::dat.bDontSeparateOffline = IsDlgButtonChecked(hwndDlg, IDC_DONTSEPARATE) ? 1 : 0;
				db_set_b(NULL, "CList", "DontSeparateOffline", (BYTE)cfg::dat.bDontSeparateOffline);

				db_set_dw(NULL, "CLC", "OfflineModes", MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS)));

				cfgSetFlag(hwndDlg, IDC_EVENTSONTOP, CLUI_STICKYEVENTS);

				cfg::dat.bUseDCMirroring = (BYTE)SendDlgItemMessage(hwndDlg, IDC_CLISTALIGN, CB_GETCURSEL, 0, 0);
				db_set_b(NULL, "CLC", "MirrorDC", cfg::dat.bUseDCMirroring);
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static UINT avatar_controls[] = { IDC_ALIGNMENT, IDC_AVATARSBORDER, IDC_AVATARSROUNDED, IDC_AVATARBORDERCLR, IDC_ALWAYSALIGNNICK, IDC_AVATARHEIGHT, IDC_AVATARSIZESPIN, 0 };

static INT_PTR CALLBACK DlgProcDspAdvanced(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SendDlgItemMessage(hwndDlg, IDC_DUALROWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Never"));
		SendDlgItemMessage(hwndDlg, IDC_DUALROWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always"));
		SendDlgItemMessage(hwndDlg, IDC_DUALROWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("When space allows it"));
		SendDlgItemMessage(hwndDlg, IDC_DUALROWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("When needed"));

		SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("With nickname - left"));
		SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Far left"));
		SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Far right"));
		SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("With nickname - right"));
		{
			int i = 0;
			if (cfg::dat.bAvatarServiceAvail) {
				Utils::enableDlgControl(hwndDlg, IDC_CLISTAVATARS, TRUE);
				while (avatar_controls[i] != 0)
					Utils::enableDlgControl(hwndDlg, avatar_controls[i++], TRUE);
			}
			else {
				Utils::enableDlgControl(hwndDlg, IDC_CLISTAVATARS, FALSE);
				while (avatar_controls[i] != 0)
					Utils::enableDlgControl(hwndDlg, avatar_controls[i++], FALSE);
			}
		}

		CheckDlgButton(hwndDlg, IDC_NOAVATARSOFFLINE, cfg::dat.bNoOfflineAvatars ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_DUALROWMODE, CB_SETCURSEL, cfg::dat.dualRowMode, 0);
		CheckDlgButton(hwndDlg, IDC_CLISTAVATARS, (cfg::dat.dwFlags & CLUI_FRAME_AVATARS) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_AVATARSBORDER, (cfg::dat.dwFlags & CLUI_FRAME_AVATARBORDER) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AVATARSROUNDED, (cfg::dat.dwFlags & CLUI_FRAME_ROUNDAVATAR) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ALWAYSALIGNNICK, (cfg::dat.dwFlags & CLUI_FRAME_ALWAYSALIGNNICK) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWSTATUSMSG, (cfg::dat.dwFlags & CLUI_FRAME_SHOWSTATUSMSG) ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_AVATARBORDERCLR, CPM_SETCOLOUR, 0, cfg::dat.avatarBorder);

		SendDlgItemMessage(hwndDlg, IDC_RADIUSSPIN, UDM_SETRANGE, 0, MAKELONG(10, 2));
		SendDlgItemMessage(hwndDlg, IDC_RADIUSSPIN, UDM_SETPOS, 0, cfg::dat.avatarRadius);

		SendDlgItemMessage(hwndDlg, IDC_AVATARSIZESPIN, UDM_SETRANGE, 0, MAKELONG(100, 16));
		SendDlgItemMessage(hwndDlg, IDC_AVATARSIZESPIN, UDM_SETPOS, 0, cfg::dat.avatarSize);

		Utils::enableDlgControl(hwndDlg, IDC_RADIUS, IsDlgButtonChecked(hwndDlg, IDC_AVATARSROUNDED) ? TRUE : FALSE);
		Utils::enableDlgControl(hwndDlg, IDC_RADIUSSPIN, IsDlgButtonChecked(hwndDlg, IDC_AVATARSROUNDED) ? TRUE : FALSE);
		Utils::enableDlgControl(hwndDlg, IDC_AVATARBORDERCLR, IsDlgButtonChecked(hwndDlg, IDC_AVATARSBORDER) ? TRUE : FALSE);

		CheckDlgButton(hwndDlg, IDC_SHOWLOCALTIME, cfg::dat.bShowLocalTime ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWLOCALTIMEONLYWHENDIFFERENT, cfg::dat.bShowLocalTimeSelective ? BST_CHECKED : BST_UNCHECKED);
		Utils::enableDlgControl(hwndDlg, IDC_SHOWLOCALTIMEONLYWHENDIFFERENT, IsDlgButtonChecked(hwndDlg, IDC_SHOWLOCALTIME));

		if (cfg::dat.dwFlags & CLUI_FRAME_AVATARSLEFT)
			SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_SETCURSEL, 1, 0);
		else if (cfg::dat.dwFlags & CLUI_FRAME_AVATARSRIGHT)
			SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_SETCURSEL, 2, 0);
		else if (cfg::dat.dwFlags & CLUI_FRAME_AVATARSRIGHTWITHNICK)
			SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_SETCURSEL, 3, 0);
		else
			SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_SETCURSEL, 0, 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CLISTAVATARS:
			if ((HWND)lParam != GetFocus())
				return 0;
			break;

		case IDC_SHOWLOCALTIME:
			Utils::enableDlgControl(hwndDlg, IDC_SHOWLOCALTIMEONLYWHENDIFFERENT, IsDlgButtonChecked(hwndDlg, IDC_SHOWLOCALTIME));
			break;

		case IDC_AVATARSROUNDED:
			Utils::enableDlgControl(hwndDlg, IDC_RADIUS, IsDlgButtonChecked(hwndDlg, IDC_AVATARSROUNDED) ? TRUE : FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_RADIUSSPIN, IsDlgButtonChecked(hwndDlg, IDC_AVATARSROUNDED) ? TRUE : FALSE);
			break;

		case IDC_AVATARSBORDER:
			Utils::enableDlgControl(hwndDlg, IDC_AVATARBORDERCLR, IsDlgButtonChecked(hwndDlg, IDC_AVATARSBORDER) ? TRUE : FALSE);
			break;
		}
		if ((LOWORD(wParam) == IDC_RADIUS || LOWORD(wParam) == IDC_AVATARHEIGHT) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		if ((LOWORD(wParam) == IDC_ALIGNMENT || LOWORD(wParam) == IDC_DUALROWMODE) && (HIWORD(wParam) != CBN_SELCHANGE || (HWND)lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			LRESULT sel = SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_GETCURSEL, 0, 0);
			if (sel != CB_ERR) {
				cfg::dat.dwFlags &= ~(CLUI_FRAME_AVATARSLEFT | CLUI_FRAME_AVATARSRIGHT | CLUI_FRAME_AVATARSRIGHTWITHNICK);
				if (sel == 1)
					cfg::dat.dwFlags |= CLUI_FRAME_AVATARSLEFT;
				else if (sel == 2)
					cfg::dat.dwFlags |= CLUI_FRAME_AVATARSRIGHT;
				else if (sel == 3)
					cfg::dat.dwFlags |= CLUI_FRAME_AVATARSRIGHTWITHNICK;
			}

			cfgSetFlag(hwndDlg, IDC_CLISTAVATARS, CLUI_FRAME_AVATARS);
			cfgSetFlag(hwndDlg, IDC_AVATARSBORDER, CLUI_FRAME_AVATARBORDER);
			cfgSetFlag(hwndDlg, IDC_AVATARSROUNDED, CLUI_FRAME_ROUNDAVATAR);
			cfgSetFlag(hwndDlg, IDC_ALWAYSALIGNNICK, CLUI_FRAME_ALWAYSALIGNNICK);
			cfgSetFlag(hwndDlg, IDC_SHOWSTATUSMSG, CLUI_FRAME_SHOWSTATUSMSG);

			cfg::dat.avatarBorder = SendDlgItemMessage(hwndDlg, IDC_AVATARBORDERCLR, CPM_GETCOLOUR, 0, 0);
			db_set_dw(NULL, "CLC", "avatarborder", cfg::dat.avatarBorder);

			BOOL translated;
			cfg::dat.avatarRadius = GetDlgItemInt(hwndDlg, IDC_RADIUS, &translated, FALSE);
			db_set_dw(NULL, "CLC", "avatarradius", cfg::dat.avatarRadius);

			cfg::dat.avatarSize = GetDlgItemInt(hwndDlg, IDC_AVATARHEIGHT, &translated, FALSE);
			db_set_w(NULL, "CList", "AvatarSize", (WORD)cfg::dat.avatarSize);

			cfg::dat.bNoOfflineAvatars = IsDlgButtonChecked(hwndDlg, IDC_NOAVATARSOFFLINE) ? TRUE : FALSE;
			db_set_b(NULL, "CList", "NoOfflineAV", (BYTE)cfg::dat.bNoOfflineAvatars);

			cfg::dat.bShowLocalTime = IsDlgButtonChecked(hwndDlg, IDC_SHOWLOCALTIME) ? 1 : 0;
			db_set_b(NULL, "CLC", "ShowLocalTime", (BYTE)cfg::dat.bShowLocalTime);

			cfg::dat.bShowLocalTimeSelective = IsDlgButtonChecked(hwndDlg, IDC_SHOWLOCALTIMEONLYWHENDIFFERENT) ? 1 : 0;
			db_set_b(NULL, "CLC", "SelectiveLocalTime", (BYTE)cfg::dat.bShowLocalTimeSelective);

			KillTimer(pcli->hwndContactTree, TIMERID_REFRESH);
			if (cfg::dat.bShowLocalTime)
				SetTimer(pcli->hwndContactTree, TIMERID_REFRESH, 65000, NULL);

			cfg::dat.dualRowMode = (BYTE)SendDlgItemMessage(hwndDlg, IDC_DUALROWMODE, CB_GETCURSEL, 0, 0);
			if (cfg::dat.dualRowMode == CB_ERR)
				cfg::dat.dualRowMode = 0;
			db_set_b(NULL, "CLC", "DualRowMode", cfg::dat.dualRowMode);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcIcons(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_XSTATUSASSTATUS, (cfg::dat.dwFlags & CLUI_FRAME_USEXSTATUSASSTATUS) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_SHOWSTATUSICONS, (cfg::dat.dwFlags & CLUI_FRAME_STATUSICONS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWMETA, (cfg::dat.dwFlags & CLUI_USEMETAICONS) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_OVERLAYICONS, (cfg::dat.dwFlags & CLUI_FRAME_OVERLAYICONS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SELECTIVEICONS, (cfg::dat.dwFlags & CLUI_FRAME_SELECTIVEICONS) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_STATUSICONSCENTERED, cfg::dat.bCenterStatusIcons ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_IDLE, db_get_b(NULL, "CLC", "ShowIdle", CLCDEFAULT_SHOWIDLE) ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_COMMAND:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			cfgSetFlag(hwndDlg, IDC_SHOWSTATUSICONS, CLUI_FRAME_STATUSICONS);
			cfgSetFlag(hwndDlg, IDC_SHOWMETA, CLUI_USEMETAICONS);
			cfgSetFlag(hwndDlg, IDC_OVERLAYICONS, CLUI_FRAME_OVERLAYICONS);
			cfgSetFlag(hwndDlg, IDC_XSTATUSASSTATUS, CLUI_FRAME_USEXSTATUSASSTATUS);
			cfgSetFlag(hwndDlg, IDC_SELECTIVEICONS, CLUI_FRAME_SELECTIVEICONS);

			db_set_b(NULL, "CLC", "ShowIdle", IsDlgButtonChecked(hwndDlg, IDC_IDLE) ? 1 : 0);

			cfg::dat.bCenterStatusIcons = IsDlgButtonChecked(hwndDlg, IDC_STATUSICONSCENTERED) ? 1 : 0;
			db_set_b(NULL, "CLC", "si_centered", (BYTE)cfg::dat.bCenterStatusIcons);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

int ClcOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = g_hInst;

	////////////////////////////////////////////////////////////////////////////
	// Main options tabs

	odp.position = -1000000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.szTitle.a = LPGEN("Contact list");
	odp.szTab.a = LPGEN("General");
	odp.pfnDlgProc = DlgProcGenOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLIST);
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("List layout");
	odp.pfnDlgProc = DlgProcClcMainOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLC);
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Window");
	odp.pfnDlgProc = DlgProcCluiOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLUI);
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Background");
	odp.pfnDlgProc = DlgProcClcBkgOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLCBKG);
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Status bar");
	odp.pfnDlgProc = DlgProcSBarOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SBAR);
	Options_AddPage(wParam, &odp);

	////////////////////////////////////////////////////////////////////////////
	// Contact rows tabs

	odp.position = 0;
	odp.szGroup.a = LPGEN("Contact list");
	odp.szTitle.a = LPGEN("Row items");
	odp.szTab.a = LPGEN("Contacts");
	odp.pfnDlgProc = DlgProcDspItems;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_DSPITEMS);
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Groups and layout");
	odp.pfnDlgProc = DlgProcDspGroups;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_DSPGROUPS);
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Advanced");
	odp.pfnDlgProc = DlgProcDspAdvanced;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_DSPADVANCED);
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Icons");
	odp.pfnDlgProc = DlgProcIcons;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ICONS);
	Options_AddPage(wParam, &odp);

	////////////////////////////////////////////////////////////////////////////
	// Other options

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT);
	odp.szGroup.a = LPGEN("Skins");
	odp.szTitle.a = LPGEN("Contact list");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = OptionsDlgProc;
	Options_AddPage(wParam, &odp);
	return 0;
}

static int opt_clc_main_changed = 0;

static INT_PTR CALLBACK DlgProcClcMainOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		opt_clc_main_changed = 0;
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		{
			DWORD exStyle = db_get_dw(NULL, "CLC", "ExStyle", pcli->pfnGetDefaultExStyle());
			UDACCEL accel[2] = { { 0, 10 }, { 2, 50 } };
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETRANGE, 0, MAKELONG(999, 0));
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETACCEL, _countof(accel), (LPARAM)&accel);
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "CLC", "ScrollTime", CLCDEFAULT_SCROLLTIME), 0));

			for (int i = 0; i < _countof(checkBoxToStyleEx); i++)
				CheckDlgButton(hwndDlg, checkBoxToStyleEx[i].id, (exStyle & checkBoxToStyleEx[i].flag) ^ (checkBoxToStyleEx[i].flag * checkBoxToStyleEx[i].not_t) ? BST_CHECKED : BST_UNCHECKED);
		}
		CheckDlgButton(hwndDlg, IDC_FULLROWSELECT, (cfg::dat.dwFlags & CLUI_FULLROWSELECT) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_DBLCLKAVATARS, cfg::dat.bDblClkAvatars ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_GREYOUT, db_get_dw(NULL, "CLC", "GreyoutFlags", CLCDEFAULT_GREYOUTFLAGS) ? BST_CHECKED : BST_UNCHECKED);
		Utils::enableDlgControl(hwndDlg, IDC_SMOOTHTIME, IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
		Utils::enableDlgControl(hwndDlg, IDC_GREYOUTOPTS, IsDlgButtonChecked(hwndDlg, IDC_GREYOUT));
		FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), greyoutValues, sizeof(greyoutValues) / sizeof(greyoutValues[0]), db_get_dw(NULL, "CLC", "FullGreyoutFlags", CLCDEFAULT_FULLGREYOUTFLAGS));
		CheckDlgButton(hwndDlg, IDC_NOSCROLLBAR, db_get_b(NULL, "CLC", "NoVScrollBar", 0) ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_VSCROLL:
		opt_clc_main_changed = 1;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_NOTNOSMOOTHSCROLLING)
			Utils::enableDlgControl(hwndDlg, IDC_SMOOTHTIME, IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
		if (LOWORD(wParam) == IDC_GREYOUT)
			Utils::enableDlgControl(hwndDlg, IDC_GREYOUTOPTS, IsDlgButtonChecked(hwndDlg, IDC_GREYOUT));
		if (LOWORD(wParam) == IDC_SMOOTHTIME && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		opt_clc_main_changed = 1;
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
						opt_clc_main_changed = 1;
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
			}
			break;

		case 0:
			if (((LPNMHDR)lParam)->code == PSN_APPLY) {
				if (!opt_clc_main_changed)
					return TRUE;

				int i;
				DWORD exStyle = db_get_dw(NULL, "CLC", "ExStyle", CLCDEFAULT_EXSTYLE);

				for (i = 0; i < sizeof(checkBoxToStyleEx) / sizeof(checkBoxToStyleEx[0]); i++)
					exStyle &= ~(checkBoxToStyleEx[i].flag);

				for (i = 0; i < sizeof(checkBoxToStyleEx) / sizeof(checkBoxToStyleEx[0]); i++) {
					if ((IsDlgButtonChecked(hwndDlg, checkBoxToStyleEx[i].id) == 0) == checkBoxToStyleEx[i].not_t)
						exStyle |= checkBoxToStyleEx[i].flag;
				}
				db_set_dw(NULL, "CLC", "ExStyle", exStyle);

				DWORD fullGreyoutFlags = MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS));
				db_set_dw(NULL, "CLC", "FullGreyoutFlags", fullGreyoutFlags);
				if (IsDlgButtonChecked(hwndDlg, IDC_GREYOUT))
					db_set_dw(NULL, "CLC", "GreyoutFlags", fullGreyoutFlags);
				else
					db_set_dw(NULL, "CLC", "GreyoutFlags", 0);

				cfgSetFlag(hwndDlg, IDC_FULLROWSELECT, CLUI_FULLROWSELECT);

				db_set_w(NULL, "CLC", "ScrollTime", (WORD)SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CLC", "NoVScrollBar", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_NOSCROLLBAR) ? 1 : 0));
				cfg::dat.bDblClkAvatars = IsDlgButtonChecked(hwndDlg, IDC_DBLCLKAVATARS) ? TRUE : FALSE;
				db_set_b(NULL, "CLC", "dblclkav", (BYTE)cfg::dat.bDblClkAvatars);

				pcli->pfnClcOptionsChanged();
				CoolSB_SetupScrollBar();
				PostMessage(pcli->hwndContactList, CLUIINTM_REDRAW, 0, 0);
				opt_clc_main_changed = 0;
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

static int opt_clc_bkg_changed = 0;

static INT_PTR CALLBACK DlgProcClcBkgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		opt_clc_bkg_changed = 0;
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_BITMAP, db_get_b(NULL, "CLC", "UseBitmap", CLCDEFAULT_USEBITMAP) ? BST_CHECKED : BST_UNCHECKED);
		SendMessage(hwndDlg, WM_USER + 10, 0, 0);
		CheckDlgButton(hwndDlg, IDC_WINCOLOUR, db_get_b(NULL, "CLC", "UseWinColours", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SKINMODE, cfg::dat.bWallpaperMode ? BST_CHECKED : BST_UNCHECKED);
		{
			DBVARIANT dbv;
			if (!db_get_ws(NULL, "CLC", "BkBitmap", &dbv)) {
				wchar_t szPath[MAX_PATH];
				if (PathToAbsoluteW(dbv.ptszVal, szPath))
					SetDlgItemText(hwndDlg, IDC_FILENAME, szPath);

				db_free(&dbv);
			}

			WORD bmpUse = db_get_w(NULL, "CLC", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
			CheckDlgButton(hwndDlg, IDC_STRETCHH, bmpUse & CLB_STRETCHH ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_STRETCHV, bmpUse & CLB_STRETCHV ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TILEH, bmpUse & CLBF_TILEH ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TILEV, bmpUse & CLBF_TILEV ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SCROLL, bmpUse & CLBF_SCROLL ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_PROPORTIONAL, bmpUse & CLBF_PROPORTIONAL ? BST_CHECKED : BST_UNCHECKED);

			SHAutoComplete(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
		}
		return TRUE;

	case WM_USER + 10:
		Utils::enableDlgControl(hwndDlg, IDC_FILENAME, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		Utils::enableDlgControl(hwndDlg, IDC_BROWSE, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		Utils::enableDlgControl(hwndDlg, IDC_STRETCHH, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		Utils::enableDlgControl(hwndDlg, IDC_STRETCHV, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		Utils::enableDlgControl(hwndDlg, IDC_TILEH, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		Utils::enableDlgControl(hwndDlg, IDC_TILEV, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		Utils::enableDlgControl(hwndDlg, IDC_SCROLL, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		Utils::enableDlgControl(hwndDlg, IDC_PROPORTIONAL, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BROWSE) {
			wchar_t str[MAX_PATH], filter[512];
			GetDlgItemText(hwndDlg, IDC_FILENAME, str, _countof(str));
			Bitmap_GetFilter(filter, _countof(filter));

			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = hwndDlg;
			ofn.hInstance = NULL;
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
		opt_clc_bkg_changed = 1;
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				if (!opt_clc_bkg_changed)
					return TRUE;

				db_set_b(NULL, "CLC", "UseBitmap", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
				db_set_b(NULL, "CLC", "UseWinColours", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_WINCOLOUR));

				char str[MAX_PATH], strrel[MAX_PATH];
				GetDlgItemTextA(hwndDlg, IDC_FILENAME, str, _countof(str));
				if (PathToRelative(str, strrel))
					db_set_s(NULL, "CLC", "BkBitmap", strrel);
				else
					db_set_s(NULL, "CLC", "BkBitmap", str);

				WORD flags = 0;
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
				db_set_w(NULL, "CLC", "BkBmpUse", flags);
				cfg::dat.bWallpaperMode = IsDlgButtonChecked(hwndDlg, IDC_SKINMODE) ? 1 : 0;
				db_set_b(NULL, "CLUI", "UseBkSkin", (BYTE)cfg::dat.bWallpaperMode);

				pcli->pfnClcOptionsChanged();
				PostMessage(pcli->hwndContactList, CLUIINTM_REDRAW, 0, 0);
				opt_clc_bkg_changed = 0;
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

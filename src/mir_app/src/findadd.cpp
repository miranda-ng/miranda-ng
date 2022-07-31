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
#include "findadd.h"

#define TIMERID_THROBBER 111

#define HM_SEARCHACK  (WM_USER+10)
#define M_SETGROUPVISIBILITIES  (WM_USER+11)

static HWND hwndFindAdd = nullptr;
static HANDLE hHookModulesLoaded = nullptr;
static HGENMENU hMainMenuItem = nullptr;
static int OnSystemModulesLoaded(WPARAM wParam, LPARAM lParam);

static int FindAddDlgResizer(HWND, LPARAM lParam, UTILRESIZECONTROL *urc)
{
	static int y, nextY, oldTop;
	FindAddDlgData *dat = (FindAddDlgData*)lParam;

	switch (urc->wId) {
	case IDC_RESULTS:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDOK:
		dat->minDlgHeight = nextY + urc->rcItem.bottom - urc->rcItem.top;
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

	case IDC_ADD:
	case IDC_MOREOPTIONS:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

	case IDC_STATUSBAR:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

	case IDC_PROTOIDGROUP:	//the resize is always processed in template order
		nextY = y = urc->rcItem.top;
		if (dat->showProtoId) nextY = y + urc->rcItem.bottom - urc->rcItem.top + 7;
		break;

	case IDC_EMAILGROUP:
		oldTop = urc->rcItem.top;
		y = nextY;
		if (dat->showEmail) nextY = y + urc->rcItem.bottom - urc->rcItem.top + 7;
		OffsetRect(&urc->rcItem, 0, y - oldTop);
		return RD_ANCHORX_LEFT | RD_ANCHORY_CUSTOM;

	case IDC_NAMEGROUP:
		oldTop = urc->rcItem.top;
		y = nextY;
		if (dat->showName) nextY = y + urc->rcItem.bottom - urc->rcItem.top + 7;
		OffsetRect(&urc->rcItem, 0, y - oldTop);
		return RD_ANCHORX_LEFT | RD_ANCHORY_CUSTOM;

	case IDC_ADVANCEDGROUP:
		oldTop = urc->rcItem.top;
		y = nextY;
		if (dat->showAdvanced) nextY = y + urc->rcItem.bottom - urc->rcItem.top + 7;
		OffsetRect(&urc->rcItem, 0, y - oldTop);
		return RD_ANCHORX_LEFT | RD_ANCHORY_CUSTOM;

	case IDC_TINYEXTENDEDGROUP:
		oldTop = urc->rcItem.top;
		y = nextY;
		if (dat->showTiny) {
			int height = urc->dlgNewSize.cy - y - (urc->dlgOriginalSize.cy - urc->rcItem.bottom);
			nextY = y + 200;  //min height for custom dialog
			urc->rcItem.top = urc->rcItem.bottom - height;
		}
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

	case IDC_BYEMAIL:
	case IDC_EMAIL:
	case IDC_BYNAME:
	case IDC_STNAMENICK:
	case IDC_STNAMEFIRST:
	case IDC_STNAMELAST:
	case IDC_NAMENICK:
	case IDC_NAMEFIRST:
	case IDC_NAMELAST:
	case IDC_BYADVANCED:
	case IDC_BYCUSTOM:
	case IDC_ADVANCED:
		OffsetRect(&urc->rcItem, 0, y - oldTop);
		return RD_ANCHORX_LEFT | RD_ANCHORY_CUSTOM;

	case IDC_HEADERBAR:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

static void RenderThrobber(HDC hdc, RECT *rcItem, int *throbbing, int *pivot)
{
	InflateRect(rcItem, -1, 0);
	int width = rcItem->right - rcItem->left;
	int height = rcItem->bottom - rcItem->top;
	int height2 = height / 2;

	if (*throbbing) {
		/* create memdc */
		HDC hMemDC = CreateCompatibleDC(nullptr);
		HBITMAP hBitmap = (HBITMAP)SelectObject(hMemDC, CreateCompatibleBitmap(hdc, width, height));
		/* flush it */
		RECT rc;
		rc.left = rc.top = 0;
		rc.right = width;
		rc.bottom = height;
		HBRUSH hBr = GetSysColorBrush(COLOR_BTNFACE);
		FillRect(hMemDC, &rc, hBr);
		DeleteObject(hBr);
		/* set up the pen */
		HPEN hPen = (HPEN)SelectObject(hMemDC, CreatePen(PS_SOLID, 4, GetSysColor(COLOR_BTNSHADOW)));
		/* draw everything before the pivot */
		int x = *pivot;
		while (x > (-height)) {
			MoveToEx(hMemDC, x + height2, 0, nullptr);
			LineTo(hMemDC, x - height2, height);
			x -= 12;
		}

		/* draw everything after the pivot */
		x = *pivot;
		while (x < width + height) {
			MoveToEx(hMemDC, x + height2, 0, nullptr);
			LineTo(hMemDC, x - height2, height);
			x += 12;
		}

		/* move the pivot */
		*pivot += 2;
		/* reset the pivot point if it gets past the rect */
		if (*pivot > width) *pivot = 0;
		/* put back the old pen and delete the new one */
		DeleteObject(SelectObject(hMemDC, hPen));
		/* cap the top and bottom */
		hPen = (HPEN)SelectObject(hMemDC, CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE)));
		MoveToEx(hMemDC, 0, 0, nullptr);
		LineTo(hMemDC, width, 0);
		MoveToEx(hMemDC, 0, height - 1, nullptr);
		LineTo(hMemDC, width, height - 1);
		/* select in the old pen and delete the new pen */
		DeleteObject(SelectObject(hMemDC, hPen));
		/* paint to screen */
		BitBlt(hdc, rcItem->left, rcItem->top, width, height, hMemDC, 0, 0, SRCCOPY);
		/* select back in the old bitmap and delete the created one, as well as freeing the mem dc. */
		hBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hMemDC);
	}
	else {
		/* just flush the DC */
		HBRUSH hBr = GetSysColorBrush(COLOR_BTNFACE);
		FillRect(hdc, rcItem, hBr);
		DeleteObject(hBr);
	}
}

static void StartThrobber(HWND hwndDlg, FindAddDlgData *dat)
{
	dat->throbbing = 1;
	SetTimer(hwndDlg, TIMERID_THROBBER, 25, nullptr);
}

static void StopThrobber(HWND hwndDlg, FindAddDlgData *dat)
{
	KillTimer(hwndDlg, TIMERID_THROBBER);
	dat->throbbing = 0;
	dat->pivot = 0;
	InvalidateRect(GetDlgItem(hwndDlg, IDC_STATUSBAR), nullptr, FALSE);
}

static LRESULT CALLBACK AdvancedSearchDlgSubclassProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_COMMAND) {
		HWND parentHwnd = GetParent(hwndDlg);
		switch (LOWORD(wParam)) {
		case IDOK:
			SendMessage(parentHwnd, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), (LPARAM)GetDlgItem(parentHwnd, IDOK));
			SetFocus(GetDlgItem(parentHwnd, IDC_ADVANCED));
			break;

		case IDCANCEL:
			CheckDlgButton(parentHwnd, IDC_ADVANCED, BST_UNCHECKED);
			SendMessage(parentHwnd, WM_COMMAND, MAKEWPARAM(IDC_ADVANCED, BN_CLICKED), (LPARAM)GetDlgItem(parentHwnd, IDC_ADVANCED));
			SetFocus(GetDlgItem(parentHwnd, IDC_ADVANCED));
			break;
		}
	}
	return mir_callNextSubclass(hwndDlg, AdvancedSearchDlgSubclassProc, msg, wParam, lParam);
}

static void ShowAdvancedSearchDlg(HWND hwndDlg, FindAddDlgData *dat)
{
	char *szProto = (char*)SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETCURSEL, 0, 0), 0);
	if (szProto == nullptr)
		return;

	if (dat->hwndAdvSearch == nullptr) {
		RECT rc;
		dat->hwndAdvSearch = (HWND)CallProtoServiceInt(0, szProto, PS_CREATEADVSEARCHUI, 0, (LPARAM)hwndDlg);
		if (dat->hwndAdvSearch != nullptr)
			mir_subclassWindow(dat->hwndAdvSearch, AdvancedSearchDlgSubclassProc);
		GetWindowRect(GetDlgItem(hwndDlg, IDC_RESULTS), &rc);
		SetWindowPos(dat->hwndAdvSearch, nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	AnimateWindow(dat->hwndAdvSearch, 150, AW_ACTIVATE | AW_SLIDE | AW_HOR_POSITIVE);
	RedrawWindow(dat->hwndAdvSearch, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

	CheckDlgButton(hwndDlg, IDC_ADVANCED, BST_CHECKED);
}

static void ReposTinySearchDlg(HWND hwndDlg, FindAddDlgData *dat)
{
	if (dat->hwndTinySearch == nullptr)
		return;

	RECT rc;
	RECT clientRect;
	POINT pt = { 0, 0 };
	GetWindowRect(GetDlgItem(hwndDlg, IDC_TINYEXTENDEDGROUP), &rc);
	GetWindowRect(dat->hwndTinySearch, &clientRect);
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient(hwndDlg, &pt);
	SetWindowPos(dat->hwndTinySearch, nullptr, pt.x + 5, pt.y + 15, rc.right - rc.left - 10, rc.bottom - rc.top - 30, SWP_NOZORDER);
}

static void ShowTinySearchDlg(HWND hwndDlg, FindAddDlgData *dat)
{
	char *szProto = (char*)SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETCURSEL, 0, 0), 0);
	if (szProto == nullptr)
		return;

	if (dat->hwndTinySearch == nullptr) {
		dat->hwndTinySearch = (HWND)CallProtoServiceInt(0, szProto, PS_CREATEADVSEARCHUI, 0, (LPARAM)/*GetDlgItem(*/hwndDlg/*, IDC_TINYEXTENDEDGROUP)*/);
		if (dat->hwndTinySearch)
			ReposTinySearchDlg(hwndDlg, dat);
		else
			dat->showTiny = false;
	}
	ShowWindow(dat->hwndTinySearch, SW_SHOW);
}

static void HideAdvancedSearchDlg(HWND hwndDlg, FindAddDlgData *dat)
{
	if (dat->hwndAdvSearch == nullptr)
		return;

	AnimateWindow(dat->hwndAdvSearch, 150, AW_HIDE | AW_BLEND);
	CheckDlgButton(hwndDlg, IDC_ADVANCED, BST_UNCHECKED);
}

void EnableResultButtons(HWND hwndDlg, int enable)
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), enable);
	EnableWindow(GetDlgItem(hwndDlg, IDC_MOREOPTIONS), enable);
}

static const int controls[] = { IDC_BYPROTOID, IDC_BYEMAIL, IDC_BYNAME, IDC_BYADVANCED, IDC_BYCUSTOM };

static void CheckSearchTypeRadioButton(HWND hwndDlg, int idControl)
{
	for (auto &it : controls)
		CheckDlgButton(hwndDlg, it, idControl == it ? BST_CHECKED : BST_UNCHECKED);
}

#define sttErrMsg TranslateT("You haven't filled in the search field. Please enter a search term and try again.")
#define sttErrTitle TranslateT("Search")

static void SetListItemText(HWND hwndList, int idx, int col, wchar_t *szText)
{
	if (szText == nullptr || *szText == 0)
		szText = TranslateT("<not specified>");

	ListView_SetItemText(hwndList, idx, col, szText);
}

static wchar_t* sttDecodeString(uint32_t dwFlags, MAllStrings &src)
{
	if (dwFlags & PSR_UNICODE)
		return mir_wstrdup(src.w);

	if (dwFlags & PSR_UTF8)
		return mir_utf8decodeW(src.a);

	return mir_a2u(src.a);
}

static INT_PTR CALLBACK DlgProcFindAdd(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	FindAddDlgData *dat = (FindAddDlgData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	HWND hwndList = GetDlgItem(hwndDlg, IDC_RESULTS);
	RECT rc, rc2;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		Window_SetSkinIcon_IcoLib(hwndDlg, SKINICON_OTHER_FINDUSER);
		dat = (FindAddDlgData*)mir_calloc(sizeof(FindAddDlgData));
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
		dat->notSearchedYet = 1;
		dat->iLastColumnSortIndex = 1;
		dat->bSortAscending = 1;
		SendDlgItemMessage(hwndDlg, IDC_MOREOPTIONS, BUTTONSETARROW, 1, 0);
		SendDlgItemMessage(hwndDlg, IDOK, BUTTONADDTOOLTIP, (WPARAM)LPGENW("Ctrl+Search add contact"), BATF_UNICODE);

		ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

		GetClientRect(hwndList, &rc);
		{
			LVCOLUMN lvc;
			lvc.mask = LVCF_TEXT | LVCF_WIDTH;
			lvc.pszText = TranslateT("Results");
			lvc.cx = rc.right - 1;
			ListView_InsertColumn(hwndList, 0, &lvc);

			LVITEM lvi;
			lvi.mask = LVIF_TEXT;
			lvi.iItem = 0;
			lvi.iSubItem = 0;
			lvi.pszText = TranslateT("There are no results to display.");
			ListView_InsertItem(hwndList, &lvi);

			// Allocate a reasonable amount of space in the status bar
			HDC hdc = GetDC(GetDlgItem(hwndDlg, IDC_STATUSBAR));
			SelectObject(hdc, (HFONT)SendDlgItemMessage(hwndDlg, IDC_STATUSBAR, WM_GETFONT, 0, 0));

			SIZE textSize;
			GetTextExtentPoint32(hdc, TranslateT("Searching"), (int)mir_wstrlen(TranslateT("Searching")), &textSize);

			int partWidth[3];
			partWidth[0] = textSize.cx;
			GetTextExtentPoint32(hdc, L"01234567890123456789", 20, &textSize);
			partWidth[0] += textSize.cx;
			ReleaseDC(GetDlgItem(hwndDlg, IDC_STATUSBAR), hdc);
			partWidth[1] = partWidth[0] + 150;
			partWidth[2] = -1;
			SendDlgItemMessage(hwndDlg, IDC_STATUSBAR, SB_SETPARTS, _countof(partWidth), (LPARAM)partWidth);
			SendDlgItemMessage(hwndDlg, IDC_STATUSBAR, SB_SETTEXT, 1 | SBT_OWNERDRAW, 0);
			SetStatusBarSearchInfo(GetDlgItem(hwndDlg, IDC_STATUSBAR), dat);

			wchar_t *szProto = nullptr;
			ptrW tszLast(db_get_wsa(0, "FindAdd", "LastSearched"));
			if (tszLast)
				szProto = NEWWSTR_ALLOCA(tszLast);

			int index = 0, cbwidth = 0, netProtoCount = 0;
			for (auto &pa : g_arAccounts) {
				if (!pa->IsEnabled())
					continue;

				uint32_t caps = (uint32_t)CallProtoServiceInt(0, pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0);
				if (caps & PF1_ANYSEARCH)
					netProtoCount++;
			}
			dat->himlComboIcons = ImageList_Create(g_iIconSX, g_iIconSY, ILC_COLOR32 | ILC_MASK, netProtoCount + 1, netProtoCount + 1);
			SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CBEM_SETIMAGELIST, 0, (LPARAM)dat->himlComboIcons);

			COMBOBOXEXITEM cbei;
			cbei.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT | CBEIF_LPARAM;
			cbei.iItem = 0;
			hdc = GetDC(hwndDlg);
			SelectObject(hdc, (HFONT)SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, WM_GETFONT, 0, 0));
			if (netProtoCount > 1) {
				cbei.pszText = TranslateT("All networks");
				GetTextExtentPoint32(hdc, cbei.pszText, (int)mir_wstrlen(cbei.pszText), &textSize);
				if (textSize.cx > cbwidth)
					cbwidth = textSize.cx;
				cbei.iImage = cbei.iSelectedImage = ImageList_AddSkinIcon(dat->himlComboIcons, SKINICON_OTHER_SEARCHALL);
				cbei.lParam = 0;
				SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CBEM_INSERTITEM, 0, (LPARAM)&cbei);
				cbei.iItem++;
			}

			for (auto &pa : g_arAccounts) {
				if (!pa->IsEnabled())
					continue;

				uint32_t caps = (uint32_t)CallProtoServiceInt(0, pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0);
				if (!(caps & PF1_ANYSEARCH))
					continue;

				cbei.pszText = pa->tszAccountName;
				GetTextExtentPoint32(hdc, cbei.pszText, (int)mir_wstrlen(cbei.pszText), &textSize);
				if (textSize.cx > cbwidth)
					cbwidth = textSize.cx;

				HICON hIcon = (HICON)CallProtoServiceInt(0, pa->szModuleName, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
				cbei.iImage = cbei.iSelectedImage = ImageList_AddIcon(dat->himlComboIcons, hIcon);
				DestroyIcon(hIcon);
				cbei.lParam = (LPARAM)pa->szModuleName;
				SendDlgItemMessageA(hwndDlg, IDC_PROTOLIST, CBEM_INSERTITEM, 0, (LPARAM)&cbei);
				if (szProto && cbei.pszText && !mir_wstrcmp(szProto, pa->tszAccountName))
					index = cbei.iItem;
				cbei.iItem++;
			}
			cbwidth += 32;

			RECT rect;
			SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETDROPPEDCONTROLRECT, 0, (LPARAM)&rect);
			if ((rect.right - rect.left) < cbwidth)
				SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_SETDROPPEDWIDTH, cbwidth, 0);
			SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_SETCURSEL, index, 0);

			SendMessage(hwndDlg, M_SETGROUPVISIBILITIES, 0, 0);
			Utils_RestoreWindowPosition(hwndDlg, 0, "FindAdd", "");
		}
		return TRUE;

	case WM_SIZE:
		Utils_ResizeDialog(hwndDlg, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_FINDADD), FindAddDlgResizer, (LPARAM)dat);
		ReposTinySearchDlg(hwndDlg, dat);
		SendDlgItemMessage(hwndDlg, IDC_STATUSBAR, WM_SIZE, 0, 0);
		if (dat->notSearchedYet) {
			GetClientRect(hwndList, &rc);
			ListView_SetColumnWidth(hwndList, 0, rc.right);
		}
		__fallthrough;

	case WM_MOVE:
		if (dat && dat->hwndAdvSearch) {
			GetWindowRect(hwndList, &rc);
			SetWindowPos(dat->hwndAdvSearch, nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
		break;

	case WM_GETMINMAXINFO:
		GetWindowRect(hwndList, &rc);
		GetWindowRect(hwndDlg, &rc2);
		{
			MINMAXINFO *mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = rc.left - rc2.left + 10 + GetSystemMetrics(SM_CXFRAME);
			GetClientRect(GetDlgItem(hwndDlg, IDC_MOREOPTIONS), &rc);
			mmi->ptMinTrackSize.x += rc.right + 5;
			GetClientRect(GetDlgItem(hwndDlg, IDC_ADD), &rc);
			mmi->ptMinTrackSize.x += rc.right + 5;
			GetClientRect(GetDlgItem(hwndDlg, IDC_STATUSBAR), &rc);
			mmi->ptMinTrackSize.y = dat->minDlgHeight + 20 + GetSystemMetrics(SM_CYCAPTION) + 2 * GetSystemMetrics(SM_CYFRAME);
			GetClientRect(GetDlgItem(hwndDlg, IDC_STATUSBAR), &rc);
			mmi->ptMinTrackSize.y += rc.bottom;
		}
		return 0;

	case M_SETGROUPVISIBILITIES:
		dat->showAdvanced = dat->showEmail = dat->showName = dat->showProtoId = dat->showTiny = 0;
		{
			char *szProto = (char*)SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETCURSEL, 0, 0), 0);
			if (szProto == (char *)CB_ERR)
				break;
			if (szProto == nullptr) {
				for (auto &pa : g_arAccounts) {
					if (pa->IsEnabled()) {
						uint32_t protoCaps = (uint32_t)CallProtoServiceInt(0, pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0);
						if (protoCaps & PF1_SEARCHBYEMAIL) dat->showEmail = 1;
						if (protoCaps & PF1_SEARCHBYNAME) dat->showName = 1;
					}
				}
			}
			else {
				uint32_t protoCaps = (uint32_t)CallProtoServiceInt(0, szProto, PS_GETCAPS, PFLAGNUM_1, 0);
				if (protoCaps & PF1_BASICSEARCH) dat->showProtoId = 1;
				if (protoCaps & PF1_SEARCHBYEMAIL) dat->showEmail = 1;
				if (protoCaps & PF1_SEARCHBYNAME) dat->showName = 1;

				if (protoCaps & PF1_EXTSEARCHUI) dat->showAdvanced = 1;
				else if (protoCaps & PF1_EXTSEARCH) dat->showTiny = 1;

				if (protoCaps & PF1_USERIDISEMAIL && dat->showProtoId) { dat->showProtoId = 0; dat->showEmail = 1; }
				if (dat->showProtoId) {
					wchar_t *wszUniqueId = (wchar_t *)CallProtoServiceInt(0, szProto, PS_GETCAPS, PFLAG_UNIQUEIDTEXT, 0);
					if (wszUniqueId)
						SetDlgItemTextW(hwndDlg, IDC_BYPROTOID, wszUniqueId);
					else
						SetDlgItemTextW(hwndDlg, IDC_BYPROTOID, TranslateT("Handle"));

					if (protoCaps & PF1_NUMERICUSERID)
						SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_PROTOID), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_PROTOID), GWL_STYLE) | ES_NUMBER);
					else
						SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_PROTOID), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_PROTOID), GWL_STYLE)&~ES_NUMBER);
				}
			}

			if (dat->showTiny)
				ShowTinySearchDlg(hwndDlg, dat);
			else if (dat->hwndTinySearch) {
				DestroyWindow(dat->hwndTinySearch);
				dat->hwndTinySearch = nullptr;
			}

#define en(id, t) ShowWindow( GetDlgItem(hwndDlg, IDC_##id), dat->show##t?SW_SHOW:SW_HIDE)
			en(PROTOIDGROUP, ProtoId); en(BYPROTOID, ProtoId); en(PROTOID, ProtoId);
			en(EMAILGROUP, Email); en(BYEMAIL, Email); en(EMAIL, Email);
			en(NAMEGROUP, Name);  en(BYNAME, Name);
			en(STNAMENICK, Name); en(NAMENICK, Name);
			en(STNAMEFIRST, Name); en(NAMEFIRST, Name);
			en(STNAMELAST, Name); en(NAMELAST, Name);
			en(ADVANCEDGROUP, Advanced); en(BYADVANCED, Advanced); en(ADVANCED, Advanced);
			en(BYCUSTOM, Tiny); en(TINYEXTENDEDGROUP, Tiny);
#undef en
			int checkmarkVisible = (dat->showAdvanced && IsDlgButtonChecked(hwndDlg, IDC_BYADVANCED)) ||
				(dat->showEmail && IsDlgButtonChecked(hwndDlg, IDC_BYEMAIL)) ||
				(dat->showTiny && IsDlgButtonChecked(hwndDlg, IDC_BYCUSTOM)) ||
				(dat->showName && IsDlgButtonChecked(hwndDlg, IDC_BYNAME)) ||
				(dat->showProtoId && IsDlgButtonChecked(hwndDlg, IDC_BYPROTOID));
			if (!checkmarkVisible) {
				if (dat->showProtoId) CheckSearchTypeRadioButton(hwndDlg, IDC_BYPROTOID);
				else if (dat->showEmail) CheckSearchTypeRadioButton(hwndDlg, IDC_BYEMAIL);
				else if (dat->showName) CheckSearchTypeRadioButton(hwndDlg, IDC_BYNAME);
				else if (dat->showAdvanced) CheckSearchTypeRadioButton(hwndDlg, IDC_BYADVANCED);
				else if (dat->showTiny) CheckSearchTypeRadioButton(hwndDlg, IDC_BYCUSTOM);
			}

			SendMessage(hwndDlg, WM_SIZE, 0, 0);

			MINMAXINFO mmi;
			SendMessage(hwndDlg, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);

			GetWindowRect(hwndDlg, &rc);
			if (rc.bottom - rc.top < mmi.ptMinTrackSize.y)
				SetWindowPos(hwndDlg, nullptr, 0, 0, rc.right - rc.left, mmi.ptMinTrackSize.y, SWP_NOZORDER | SWP_NOMOVE);
		}
		break;

	case WM_TIMER:
		if (wParam == TIMERID_THROBBER) {
			int borders[3];
			SendDlgItemMessage(hwndDlg, IDC_STATUSBAR, SB_GETBORDERS, 0, (LPARAM)borders);

			SendDlgItemMessage(hwndDlg, IDC_STATUSBAR, SB_GETRECT, 1, (LPARAM)&rc);
			InflateRect(&rc, -borders[2] / 2, -borders[1] / 2);
			HDC hdc = GetDC(GetDlgItem(hwndDlg, IDC_STATUSBAR));
			RenderThrobber(hdc, &rc, &dat->throbbing, &dat->pivot);
			ReleaseDC(GetDlgItem(hwndDlg, IDC_STATUSBAR), hdc);
		}
		break;

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT*)lParam;
			if (dis->CtlID == IDC_STATUSBAR && dis->itemID == 1) {
				RenderThrobber(dis->hDC, &dis->rcItem, &dat->throbbing, &dat->pivot);
				return TRUE;
			}
		}
		break;

	case WM_NOTIFY:
		if (wParam == IDC_RESULTS) {
			switch (((LPNMHDR)lParam)->code) {
			case LVN_ITEMCHANGED:
				{
					int count = ListView_GetSelectedCount(hwndList);
					if (dat->notSearchedYet)
						count = 0;
					EnableResultButtons(hwndDlg, count);
				}
				break;

			case LVN_COLUMNCLICK:
				HDITEM hdi;
				hdi.mask = HDI_FORMAT;
				hdi.fmt = HDF_LEFT | HDF_STRING;
				Header_SetItem(ListView_GetHeader(hwndList), dat->iLastColumnSortIndex, &hdi);

				LPNMLISTVIEW nmlv = (LPNMLISTVIEW)lParam;
				if (nmlv->iSubItem != dat->iLastColumnSortIndex) {
					dat->bSortAscending = TRUE;
					dat->iLastColumnSortIndex = nmlv->iSubItem;
				}
				else dat->bSortAscending = !dat->bSortAscending;

				hdi.fmt = HDF_LEFT | HDF_STRING | (dat->bSortAscending ? HDF_SORTDOWN : HDF_SORTUP);
				Header_SetItem(ListView_GetHeader(hwndList), dat->iLastColumnSortIndex, &hdi);

				ListView_SortItemsEx(hwndList, SearchResultsCompareFunc, (LPARAM)hwndDlg);
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_PROTOLIST:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				HideAdvancedSearchDlg(hwndDlg, dat);
				if (dat->hwndAdvSearch) {
					DestroyWindow(dat->hwndAdvSearch);
					dat->hwndAdvSearch = nullptr;
				}
				if (dat->hwndTinySearch) {
					DestroyWindow(dat->hwndTinySearch);
					dat->hwndTinySearch = nullptr;
				}
				SendMessage(hwndDlg, M_SETGROUPVISIBILITIES, 0, 0);
			}
			break;

		case IDC_BYPROTOID:
		case IDC_BYEMAIL:
		case IDC_BYNAME:
			{
				int count = ListView_GetSelectedCount(hwndList);
				if (dat->notSearchedYet)
					count = 0;
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), count);
				HideAdvancedSearchDlg(hwndDlg, dat);
			}
			break;

		case IDC_PROTOID:
			if (HIWORD(wParam) == EN_CHANGE) {
				HideAdvancedSearchDlg(hwndDlg, dat);
				CheckSearchTypeRadioButton(hwndDlg, IDC_BYPROTOID);
			}
			break;

		case IDC_EMAIL:
			if (HIWORD(wParam) == EN_CHANGE) {
				HideAdvancedSearchDlg(hwndDlg, dat);
				CheckSearchTypeRadioButton(hwndDlg, IDC_BYEMAIL);
			}
			break;

		case IDC_NAMENICK:
		case IDC_NAMEFIRST:
		case IDC_NAMELAST:
			if (HIWORD(wParam) == EN_CHANGE) {
				HideAdvancedSearchDlg(hwndDlg, dat);
				CheckSearchTypeRadioButton(hwndDlg, IDC_BYNAME);
			}
			break;

		case IDC_ADVANCED:
			EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), ListView_GetSelectedCount(hwndList) > 0);
			if (IsDlgButtonChecked(hwndDlg, IDC_ADVANCED))
				ShowAdvancedSearchDlg(hwndDlg, dat);
			else
				HideAdvancedSearchDlg(hwndDlg, dat);
			CheckSearchTypeRadioButton(hwndDlg, IDC_BYADVANCED);
			break;

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;

		case IDOK:
			HideAdvancedSearchDlg(hwndDlg, dat);
			if (dat->searchCount) {	 //cancel search
				SetDlgItemText(hwndDlg, IDOK, TranslateT("&Search"));
				if (dat->hResultHook) { UnhookEvent(dat->hResultHook); dat->hResultHook = nullptr; }
				if (dat->search) { mir_free(dat->search); dat->search = nullptr; }
				dat->searchCount = 0;
				StopThrobber(hwndDlg, dat);
				SetStatusBarSearchInfo(GetDlgItem(hwndDlg, IDC_STATUSBAR), dat);
			}
			else {
				char *szProto = (char*)SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETCURSEL, 0, 0), 0);
				if (dat->search)
					mir_free(dat->search), dat->search = nullptr;
				dat->searchCount = 0;
				dat->hResultHook = HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_SEARCHACK);
				if (IsDlgButtonChecked(hwndDlg, IDC_BYCUSTOM))
					BeginSearch(hwndDlg, dat, szProto, PS_SEARCHBYADVANCED, PF1_EXTSEARCHUI, dat->hwndTinySearch);
				else if (IsDlgButtonChecked(hwndDlg, IDC_BYPROTOID)) {
					wchar_t str[256];
					GetDlgItemText(hwndDlg, IDC_PROTOID, str, _countof(str));
					rtrimw(str);
					if (str[0] == 0)
						MessageBoxW(hwndDlg, sttErrMsg, sttErrTitle, MB_ICONERROR | MB_OK);
					else
						BeginSearch(hwndDlg, dat, szProto, PS_BASICSEARCH, PF1_BASICSEARCH, str);
				}
				else if (IsDlgButtonChecked(hwndDlg, IDC_BYEMAIL)) {
					wchar_t str[256];
					GetDlgItemText(hwndDlg, IDC_EMAIL, str, _countof(str));
					rtrimw(str);
					if (str[0] == 0)
						MessageBoxW(hwndDlg, sttErrMsg, sttErrTitle, MB_ICONERROR | MB_OK);
					else
						BeginSearch(hwndDlg, dat, szProto, PS_SEARCHBYEMAIL, PF1_SEARCHBYEMAIL, str);
				}
				else if (IsDlgButtonChecked(hwndDlg, IDC_BYNAME)) {
					wchar_t nick[256], first[256], last[256];
					PROTOSEARCHBYNAME psbn;
					GetDlgItemText(hwndDlg, IDC_NAMENICK, nick, _countof(nick));
					GetDlgItemText(hwndDlg, IDC_NAMEFIRST, first, _countof(first));
					GetDlgItemText(hwndDlg, IDC_NAMELAST, last, _countof(last));
					psbn.pszFirstName = first;
					psbn.pszLastName = last;
					psbn.pszNick = nick;
					if (nick[0] == 0 && first[0] == 0 && last[0] == 0)
						MessageBoxW(hwndDlg, sttErrMsg, sttErrTitle, MB_ICONERROR | MB_OK);
					else
						BeginSearch(hwndDlg, dat, szProto, PS_SEARCHBYNAME, PF1_SEARCHBYNAME, &psbn);
				}
				else if (IsDlgButtonChecked(hwndDlg, IDC_BYADVANCED)) {
					if (dat->hwndAdvSearch == nullptr)
						MessageBoxW(hwndDlg, sttErrMsg, sttErrTitle, MB_ICONERROR | MB_OK);
					else
						BeginSearch(hwndDlg, dat, szProto, PS_SEARCHBYADVANCED, PF1_EXTSEARCHUI, dat->hwndAdvSearch);
				}

				if (dat->searchCount == 0) {
					if (dat->hResultHook) {
						UnhookEvent(dat->hResultHook);
						dat->hResultHook = nullptr;
					}
					break;
				}

				dat->notSearchedYet = 0;
				FreeSearchResults(hwndList);

				CreateResultsColumns(hwndList, dat, szProto);
				SetStatusBarSearchInfo(GetDlgItem(hwndDlg, IDC_STATUSBAR), dat);
				SetStatusBarResultInfo(hwndDlg);
				StartThrobber(hwndDlg, dat);
				SetDlgItemText(hwndDlg, IDOK, TranslateT("Cancel"));
			}
			break;

		case IDC_ADD:
			if (ListView_GetSelectedCount(hwndList) == 1) {
				LVITEM lvi;
				lvi.mask = LVIF_PARAM;
				lvi.iItem = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
				ListView_GetItem(hwndList, &lvi);
				ListSearchResult *lsr = (ListSearchResult*)lvi.lParam;
				Contact::AddBySearch(lsr->szProto, &lsr->psr, hwndDlg);
			}
			else {
				wchar_t str[256];
				GetDlgItemText(hwndDlg, IDC_PROTOID, str, _countof(str));
				if (*rtrimw(str) == 0)
					break;

				char *szProto = (char*)SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETITEMDATA,
					SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETCURSEL, 0, 0), 0);

				PROTOSEARCHRESULT psr = { 0 };
				psr.cbSize = sizeof(psr);
				psr.flags = PSR_UNICODE;
				psr.id.w = str;
				Contact::AddBySearch(szProto, &psr, hwndDlg);
			}
			break;

		case IDC_MOREOPTIONS:
			GetWindowRect(GetDlgItem(hwndDlg, IDC_MOREOPTIONS), &rc);
			ShowMoreOptionsMenu(hwndDlg, rc.left, rc.bottom);
			break;
		}

		if (lParam && dat->hwndTinySearch == (HWND)lParam &&
			HIWORD(wParam) == EN_SETFOCUS && LOWORD(wParam) == 0 &&
			BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_BYCUSTOM)) {
			CheckSearchTypeRadioButton(hwndDlg, IDC_BYCUSTOM);
		}
		break;

	case WM_CONTEXTMENU:
		{
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

			LVHITTESTINFO lvhti;
			lvhti.pt = pt;
			ScreenToClient(hwndDlg, &pt);
			switch (GetDlgCtrlID(ChildWindowFromPoint(hwndDlg, pt))) {
			case IDC_RESULTS:
				if (dat->notSearchedYet)
					return TRUE;
				ScreenToClient(hwndList, &lvhti.pt);
				if (ListView_HitTest(hwndList, &lvhti) == -1)
					break;
				ShowMoreOptionsMenu(hwndDlg, (short)LOWORD(lParam), (short)HIWORD(lParam));
				return TRUE;
			}
		}
		break;

	case HM_SEARCHACK:
		{
			ACKDATA *ack = (ACKDATA*)lParam;
			if (ack->type != ACKTYPE_SEARCH)
				break;

			int i;
			for (i = 0; i < dat->searchCount; i++)
				if (dat->search[i].hProcess == ack->hProcess && dat->search[i].hProcess != nullptr && !mir_strcmp(dat->search[i].szProto, ack->szModule)) break;
			if (i == dat->searchCount)
				break;

			if (ack->result == ACKRESULT_SUCCESS || ack->result == ACKRESULT_FAILED) {
				dat->searchCount--;
				memmove(dat->search + i, dat->search + i + 1, sizeof(struct ProtoSearchInfo)*(dat->searchCount - i));
				if (dat->searchCount == 0) {
					mir_free(dat->search);
					dat->search = nullptr;
					UnhookEvent(dat->hResultHook);
					dat->hResultHook = nullptr;
					SetDlgItemText(hwndDlg, IDOK, TranslateT("&Search"));
					StopThrobber(hwndDlg, dat);
				}
				ListView_SortItemsEx(hwndList, SearchResultsCompareFunc, (LPARAM)hwndDlg);
				SetStatusBarSearchInfo(GetDlgItem(hwndDlg, IDC_STATUSBAR), dat);
			}
			else if (ack->result == ACKRESULT_SEARCHRESULT && ack->lParam) {
				CUSTOMSEARCHRESULTS *csr = (CUSTOMSEARCHRESULTS*)ack->lParam;
				dat->bFlexSearchResult = TRUE;
				PROTOSEARCHRESULT *psr = &csr->psr;
				// check if this is column names data (psr->cbSize == 0)
				if (psr->cbSize == 0) { // blob contain info about columns
					// first remove all exist items
					FreeSearchResults(hwndList);
					ListView_DeleteAllItems(hwndList); //not sure if previous delete list items too

					//second remove all columns
					while (ListView_DeleteColumn(hwndList, 1)); //will delete fist column till it possible

					// now will add columns and captions;
					LVCOLUMN lvc = { 0 };
					lvc.mask = LVCF_TEXT;
					for (int iColumn = 0; iColumn < csr->nFieldCount; iColumn++) {
						lvc.pszText = TranslateW(csr->pszFields[iColumn]);
						ListView_InsertColumn(hwndList, iColumn + 1, &lvc);
					}
				}
				else { // blob contain info about found contacts
					ListSearchResult *lsr = (ListSearchResult*)mir_alloc(offsetof(ListSearchResult, psr) + psr->cbSize);
					lsr->szProto = ack->szModule;
					memcpy(&lsr->psr, psr, psr->cbSize);

					/* Next block is not needed but behavior will be kept */
					lsr->psr.id.w = sttDecodeString(psr->flags, psr->id);
					lsr->psr.nick.w = sttDecodeString(psr->flags, psr->nick);
					lsr->psr.firstName.w = sttDecodeString(psr->flags, psr->firstName);
					lsr->psr.lastName.w = sttDecodeString(psr->flags, psr->lastName);
					lsr->psr.email.w = sttDecodeString(psr->flags, psr->email);
					lsr->psr.flags = psr->flags & ~PSR_UNICODE | PSR_UNICODE;

					LVITEM lvi = { 0 };
					lvi.mask = LVIF_PARAM | LVIF_IMAGE;
					lvi.lParam = (LPARAM)lsr;
					for (i = SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETCOUNT, 0, 0); i--;) {
						char *szComboProto = (char*)SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETITEMDATA, i, 0);
						if (szComboProto == nullptr) continue;
						if (!mir_strcmp(szComboProto, ack->szModule)) {
							COMBOBOXEXITEM cbei = { 0 };
							cbei.mask = CBEIF_IMAGE;
							cbei.iItem = i;
							SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CBEM_GETITEM, 0, (LPARAM)&cbei);
							lvi.iImage = cbei.iImage;
						}
					}
					int iItem = ListView_InsertItem(hwndList, &lvi);
					for (int col = 0; col < csr->nFieldCount; col++)
						SetListItemText(hwndList, iItem, col + 1, csr->pszFields[col]);

					ListView_SortItemsEx(hwndList, SearchResultsCompareFunc, (LPARAM)hwndDlg);
					iItem = 0;
					while (ListView_SetColumnWidth(hwndList, iItem++, LVSCW_AUTOSIZE_USEHEADER));
					SetStatusBarResultInfo(hwndDlg);
				}
				break;
			}
			else if (ack->result == ACKRESULT_DATA) {
				PROTOSEARCHRESULT *psr = (PROTOSEARCHRESULT*)ack->lParam;
				ListSearchResult *lsr = (ListSearchResult*)mir_alloc(offsetof(ListSearchResult, psr) + psr->cbSize);
				lsr->szProto = ack->szModule;

				dat->bFlexSearchResult = FALSE;

				memcpy(&lsr->psr, psr, psr->cbSize);
				lsr->psr.nick.w = sttDecodeString(psr->flags, psr->nick);
				lsr->psr.firstName.w = sttDecodeString(psr->flags, psr->firstName);
				lsr->psr.lastName.w = sttDecodeString(psr->flags, psr->lastName);
				lsr->psr.email.w = sttDecodeString(psr->flags, psr->email);
				lsr->psr.id.w = sttDecodeString(psr->flags, psr->id);
				lsr->psr.flags = psr->flags & ~PSR_UNICODE | PSR_UNICODE;

				LVITEM lvi = { 0 };
				lvi.mask = LVIF_PARAM | LVIF_IMAGE;
				lvi.lParam = (LPARAM)lsr;
				for (i = SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETCOUNT, 0, 0); i--;) {
					char *szComboProto = (char*)SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETITEMDATA, i, 0);
					if (szComboProto == nullptr) continue;
					if (!mir_strcmp(szComboProto, ack->szModule)) {
						COMBOBOXEXITEM cbei = { 0 };
						cbei.mask = CBEIF_IMAGE;
						cbei.iItem = i;
						SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CBEM_GETITEM, 0, (LPARAM)&cbei);
						lvi.iImage = cbei.iImage;
						break;
					}
				}

				int iItem = ListView_InsertItem(hwndList, &lvi);
				SetListItemText(hwndList, iItem, 1, lsr->psr.id.w);
				SetListItemText(hwndList, iItem, 2, lsr->psr.nick.w);
				SetListItemText(hwndList, iItem, 3, lsr->psr.firstName.w);
				SetListItemText(hwndList, iItem, 4, lsr->psr.lastName.w);
				SetListItemText(hwndList, iItem, 5, lsr->psr.email.w);
				SetStatusBarResultInfo(hwndDlg);
			}
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		int len = SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETLBTEXTLEN, SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETCURSEL, 0, 0), 0);
		wchar_t *szProto = (wchar_t*)alloca(sizeof(wchar_t)*(len + 1));
		if (szProto != nullptr) {
			*szProto = '\0';
			SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETLBTEXT, SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_GETCURSEL, 0, 0), (LPARAM)szProto);
			db_set_ws(0, "FindAdd", "LastSearched", szProto);
		}

		SaveColumnSizes(hwndList);
		if (dat->hResultHook != nullptr)
			UnhookEvent(dat->hResultHook);
		FreeSearchResults(hwndList);
		ImageList_Destroy(dat->himlComboIcons);
		mir_free(dat->search);
		if (dat->hwndAdvSearch) {
			DestroyWindow(dat->hwndAdvSearch);
			dat->hwndAdvSearch = nullptr;
		}
		if (dat->hwndTinySearch) {
			DestroyWindow(dat->hwndTinySearch);
			dat->hwndTinySearch = nullptr;
		}
		mir_free(dat);
		Window_FreeIcon_IcoLib(hwndDlg);
		Utils_SaveWindowPosition(hwndDlg, 0, "FindAdd", "");
		break;
	}
	return FALSE;
}

static INT_PTR FindAddCommand(WPARAM, LPARAM)
{
	if (IsWindow(hwndFindAdd)) {
		ShowWindow(hwndFindAdd, SW_SHOWNORMAL);
		SetForegroundWindow(hwndFindAdd);
		SetFocus(hwndFindAdd);
	}
	else {
		int netProtoCount = 0;

		// Make sure we have some networks to search on. This is not ideal since
		// this check will be repeated every time the dialog is requested, but it
		// must be done since this service can be called from other places than the menu.
		// One alternative would be to only create the service if we have network
		// protocols loaded but that would delay the creation until MODULE_LOADED and
		// that is not good either...
		for (auto &pa : g_arAccounts) {
			if (!pa->IsEnabled())
				continue;

			int protoCaps = CallProtoServiceInt(0, pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0);
			if (protoCaps & PF1_ANYSEARCH)
				netProtoCount++;
		}
		if (netProtoCount > 0)
			hwndFindAdd = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FINDADD), nullptr, DlgProcFindAdd);
	}
	return 0;
}

int FindAddPreShutdown(WPARAM, LPARAM)
{
	if (IsWindow(hwndFindAdd))
		DestroyWindow(hwndFindAdd);
	hwndFindAdd = nullptr;
	return 0;
}

int LoadFindAddModule(void)
{
	CreateServiceFunction(MS_FINDADD_FINDADD, FindAddCommand);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnSystemModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, OnSystemModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, FindAddPreShutdown);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x860556b9, 0x1577, 0x4f6f, 0x8c, 0xb0, 0x93, 0x24, 0xa8, 0x2e, 0x20, 0x92);
	mi.position = 500020000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_FINDUSER);
	mi.name.a = LPGEN("&Find/add contacts...");
	mi.pszService = MS_FINDADD_FINDADD;
	hMainMenuItem = Menu_AddMainMenuItem(&mi);
	return 0;
}

static int OnSystemModulesLoaded(WPARAM, LPARAM)
{
	int netProtoCount = 0;

	// Make sure we have some networks to search on.
	for (auto &pa : g_arAccounts) {
		int protoCaps = CallProtoServiceInt(0, pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0);
		if (protoCaps & PF1_ANYSEARCH)
			netProtoCount++;
	}

	Menu_ShowItem(hMainMenuItem, netProtoCount != 0);
	return 0;
}

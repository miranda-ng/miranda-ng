/*
Quick Messages plugin for Miranda IM

Copyright (C) 2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

int g_iOPButtonsCount;
BOOL bNeedRestart = FALSE;
BOOL drag = FALSE, bOptionsInit = TRUE;
HTREEITEM hDragItem = nullptr;
HWND hButtonsList = nullptr;
HWND hMenuTree = nullptr;
HWND hwndEdit = nullptr;

HWND g_opHdlg = nullptr, g_varhelpDlg = nullptr;

INT_PTR CALLBACK HelpDlgProc(HWND hdlg, UINT msg, WPARAM, LPARAM lparam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			LOGFONT logFont;
			HFONT hFont;
			RECT rc;
			g_varhelpDlg = hdlg;
			TranslateDialogDefault(hdlg);
			hFont = (HFONT)SendDlgItemMessage(hdlg, IDC_STATICTITLE, WM_GETFONT, 0, 0);
			GetObject(hFont, sizeof logFont, &logFont);
			logFont.lfWeight = FW_BOLD;
			hFont = CreateFontIndirect(&logFont);
			SendDlgItemMessage(hdlg, IDC_STATICTITLE, WM_SETFONT, (WPARAM)hFont, 0);
			SendDlgItemMessage(hdlg, IDC_STATICTITLE2, WM_SETFONT, (WPARAM)hFont, 0);
			SendDlgItemMessage(hdlg, IDC_VARTEXT, WM_SETFONT, (WPARAM)hFont, 0);
			SendDlgItemMessage(hdlg, IDC_VARCLIP, WM_SETFONT, (WPARAM)hFont, 0);
			SendDlgItemMessage(hdlg, IDC_VARPUNAME, WM_SETFONT, (WPARAM)hFont, 0);
			SendDlgItemMessage(hdlg, IDC_VARPLNAME, WM_SETFONT, (WPARAM)hFont, 0);
			SendDlgItemMessage(hdlg, IDC_VARCNAME, WM_SETFONT, (WPARAM)hFont, 0);
			SendDlgItemMessage(hdlg, IDC_VARCFNAME, WM_SETFONT, (WPARAM)hFont, 0);
			SendDlgItemMessage(hdlg, IDC_VARCLNAME, WM_SETFONT, (WPARAM)hFont, 0);

			GetWindowRect(g_opHdlg, &rc);
			SetWindowPos(hdlg, nullptr, rc.left, rc.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
		}
		break;

	case WM_LBUTTONDOWN:
		PostMessage(hdlg, WM_NCLBUTTONDOWN, HTCAPTION, lparam);
		break;

	case WM_CLOSE:
	case WM_DESTROY:
		DestroyWindow(g_varhelpDlg);
		g_varhelpDlg = nullptr;
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

static LRESULT CALLBACK LabelEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	}
	return mir_callNextSubclass(hwnd, LabelEditSubclassProc, msg, wParam, lParam);
}

static LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hParent = GetParent(hwnd);
	switch (msg) {
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS | DLGC_HASSETSEL;

	case WM_SETFOCUS:
		PostMessage(hwnd, EM_SETSEL, 0, -1);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			if (hwnd == GetDlgItem(hParent, IDC_BUTTONNAME))
				SendMessage(hParent, WM_COMMAND, IDC_BLISTADD, 0);
			else
				SendMessage(hParent, WM_COMMAND, IDC_MTREEADD, 0);
		}
		break;
	}

	return mir_callNextSubclass(hwnd, EditSubclassProc, msg, wParam, lParam);
}

void SetMenuEntryProperties(HWND hdlg)
{
	TVITEM tvi;
	HTREEITEM hItem = nullptr;
	int pos = 0;

	if (TreeView_GetCount(hButtonsList) && (tvi.hItem = TreeView_GetSelection(hButtonsList))) {
		tvi.mask = TVIF_HANDLE | TVIF_PARAM;
		TreeView_GetItem(hButtonsList, &tvi);

		if (tvi.lParam) {
			ListData* ld = (ListData*)tvi.lParam;
			wchar_t szValue[256];
			GetDlgItemText(hdlg, IDC_RCLICKVALUE, szValue, _countof(szValue));
			if (mir_wstrlen(szValue)) {
				if (ld->ptszOPQValue && (ld->ptszOPQValue != ld->ptszQValue))
					mir_free(ld->ptszOPQValue);
				ld->ptszOPQValue = mir_wstrdup(szValue);
			}
			ld->bIsOpServName = IsDlgButtonChecked(hdlg, IDC_ISSERVNAME2);
		}
	}

	tvi.hItem = nullptr;
	if (TreeView_GetCount(hMenuTree) && (tvi.hItem = TreeView_GetSelection(hMenuTree))) {
		tvi.mask = TVIF_HANDLE | TVIF_PARAM;
		TreeView_GetItem(hMenuTree, &tvi);

		if (tvi.lParam) {
			ButtonData *bd = (ButtonData*)tvi.lParam;
			wchar_t szValue[256];
			GetDlgItemText(hdlg, IDC_MENUVALUE, szValue, _countof(szValue));
			if (mir_wstrlen(szValue)) {
				if (mir_wstrlen(bd->pszOpValue) && (bd->pszOpValue != bd->pszValue))
					mir_free(bd->pszOpValue);
				bd->pszOpValue = mir_wstrdup(szValue);
			}
			bd->bOpInQMenu = IsDlgButtonChecked(hdlg, IDC_INQMENU);
			bd->bIsOpServName = IsDlgButtonChecked(hdlg, IDC_ISSERVNAME);
		}

		tvi.mask = TVIF_HANDLE | TVIF_PARAM;
		tvi.hItem = TreeView_GetRoot(hMenuTree);
	}
	while (tvi.hItem) {
		TreeView_GetItem(hMenuTree, &tvi);
		ButtonData *bd = (ButtonData*)tvi.lParam;

		bd->dwOPPos = pos++;

		if (hItem = TreeView_GetChild(hMenuTree, tvi.hItem)) {

			bd->fEntryOpType &= ~QMF_EX_CHILD;

			if (bd->pszOpValue) {
				mir_free(bd->pszOpValue);
				bd->pszOpValue = nullptr;
			}
			tvi.hItem = hItem;
			continue;
		}
		else {
			if (bd->fEntryOpType&QMF_EX_SEPARATOR) {
				if (bd->pszOpValue) {
					mir_free(bd->pszOpValue);
					bd->pszOpValue = nullptr;
				}
			}
			else {
				if (!bd->pszOpValue)
					bd->pszOpValue = mir_wstrdup(LPGENW("Enter Value"));
			}
			if (TreeView_GetParent(hMenuTree, tvi.hItem))
				bd->fEntryOpType |= QMF_EX_CHILD;
			else bd->fEntryOpType &= ~QMF_EX_CHILD;

			if (!(hItem = TreeView_GetNextSibling(hMenuTree, tvi.hItem)))
				tvi.hItem = TreeView_GetNextSibling(hMenuTree, TreeView_GetParent(hMenuTree, tvi.hItem));
			else
				tvi.hItem = hItem;

		}
	}
}

void SaveMenuTree()
{
	int iBl = 0, i = 0, k = 0;
	int iBtd = g_iButtonsCount;
	BOOL bDeleted = FALSE;
	char szMEntry[256] = { '\0' };
	wchar_t strbuf[256];
	HTREEITEM hti = nullptr;
	TVITEM tvi;

	g_iButtonsCount = TreeView_GetCount(hButtonsList);

	tvi.pszText = strbuf;
	tvi.cchTextMax = _countof(strbuf);
	tvi.mask = TVIF_HANDLE | TVIF_TEXT;
	tvi.hItem = TreeView_GetRoot(hButtonsList);

	TreeView_GetItem(hButtonsList, &tvi);

	li_ZeroQuickList(QuickList);

	BalanceButtons(iBtd, g_iButtonsCount);

	while (ButtonsList[iBl]) {
		SortedList * sl = nullptr;
		ListData* ld = ButtonsList[iBl];

		if (!ld->sl) break;

		sl = ld->sl;

		if (ld->dwOPFlags&QMF_DELETNEEDED) {
			bDeleted = (ld->dwOPFlags&QMF_NEW) ? FALSE : TRUE;
			if (bDeleted) CleanSettings(ld->dwPos, -1);
			DestroyButton(iBl);
			continue;
		}
		if (ld->ptszQValue != ld->ptszOPQValue) {
			if (ld->ptszQValue)
				mir_free(ld->ptszQValue);

			ld->ptszQValue = (ld->ptszOPQValue) ? ld->ptszOPQValue : nullptr;
		}

		if (ld->ptszButtonName)
			mir_free(ld->ptszButtonName);

		if (iBl > 0)
			if (hti = TreeView_GetNextSibling(hButtonsList, hti ? hti : tvi.hItem)) {
				tvi.hItem = hti;
				TreeView_GetItem(hButtonsList, &tvi);
			}

		ld->ptszButtonName = mir_wstrdup(tvi.pszText);

		if (ld->ptszQValue) {
			mir_snprintf(szMEntry, "ButtonValue_%u", iBl);
			g_plugin.setWString(szMEntry, ld->ptszQValue);
		}

		if (((ld->dwOPFlags & QMF_NEW) || (ld->dwOPFlags & QMF_RENAMED) || bDeleted)) {
			BBButton bb = {};
			bb.pszModuleName = MODULENAME;
			bb.dwButtonID = iBl;
			bb.pwszTooltip = ld->ptszButtonName;
			Srmm_ModifyButton(&bb);
		}

		mir_snprintf(szMEntry, "ButtonName_%u", iBl);
		g_plugin.setWString(szMEntry, ld->ptszButtonName);

		ld->dwOPFlags = 0;
		ld->dwPos = iBl;
		ld->bIsServName = ld->bIsOpServName;
		mir_snprintf(szMEntry, "RCEntryIsServiceName_%u", iBl);
		g_plugin.setByte(szMEntry, ld->bIsServName);

		bDeleted = FALSE;

		qsort(sl->items, sl->realCount, sizeof(ButtonData *), sstSortButtons);

		for (i = 0; i < sl->realCount; i++) {
			ButtonData * bd = (ButtonData *)sl->items[i];

			if (bd->dwOPFlags & QMF_DELETNEEDED) {
				RemoveMenuEntryNode(sl, i--);
				continue;
			}
			bd->bIsServName = bd->bIsOpServName;
			bd->bInQMenu = bd->bOpInQMenu;
			bd->dwPos = bd->dwOPPos;
			bd->fEntryType = bd->fEntryOpType;
			bd->dwOPFlags = 0;

			if (bd->pszName != bd->pszOpName) {
				if (bd->pszName)
					mir_free(bd->pszName);
				bd->pszName = bd->pszOpName ? bd->pszOpName : nullptr;
			}

			if (bd->pszValue != bd->pszOpValue) {
				if (bd->pszValue)
					mir_free(bd->pszValue);
				bd->pszValue = bd->pszOpValue ? bd->pszOpValue : nullptr;
			}
			if (bd->bInQMenu) {
				QuickData* qd = (QuickData *)mir_alloc(sizeof(QuickData));
				qd->dwPos = k++;
				qd->fEntryType = bd->fEntryType;
				qd->bIsService = bd->bIsServName;
				qd->ptszValue = bd->pszValue;
				qd->ptszValueName = bd->pszName;
				List_InsertPtr(QuickList, qd);
			}

			SaveModuleSettings(iBl, bd);
		}
		CleanSettings(iBl, sl->realCount);

		iBl++;
	}

	g_plugin.setByte("ButtonsCount", (uint8_t)g_iButtonsCount);
}

void RestoreModuleData()
{
	int iBl = 0, i = 0;

	while (ButtonsList[iBl]) {
		SortedList * sl = nullptr;
		ListData* ld = ButtonsList[iBl];

		if (!(sl = ld->sl)) break;

		if (ld->dwOPFlags&QMF_NEW) {
			DestroyButton(iBl);
			continue;
		}

		if (ld->ptszQValue != ld->ptszOPQValue) {
			if (ld->ptszOPQValue)
				mir_free(ld->ptszOPQValue);

			ld->ptszOPQValue = (ld->ptszQValue) ? ld->ptszQValue : nullptr;
		}

		ld->bIsOpServName = ld->bIsServName;
		ld->dwOPFlags = 0;

		qsort(sl->items, sl->realCount, sizeof(ButtonData *), sstSortButtons);

		for (i = 0; i < sl->realCount; i++) {
			ButtonData *bd = (ButtonData *)sl->items[i];

			if (bd->dwOPFlags & QMF_NEW) {
				RemoveMenuEntryNode(sl, i--);
				continue;
			}
			bd->bIsOpServName = bd->bIsServName;
			bd->bOpInQMenu = bd->bInQMenu;
			bd->dwOPPos = bd->dwPos;
			bd->fEntryOpType = bd->fEntryType;
			bd->dwOPFlags = 0;

			if (bd->pszName != bd->pszOpName) {
				if (bd->pszOpName)
					mir_free(bd->pszOpName);
				bd->pszOpName = bd->pszName ? bd->pszName : nullptr;
			}

			if (bd->pszValue != bd->pszOpValue) {
				if (bd->pszOpValue)
					mir_free(bd->pszOpValue);
				bd->pszOpValue = bd->pszValue ? bd->pszValue : nullptr;
			}
		}
		iBl++;
	}
}

static int BuildMenuTree(HWND hToolBarTree, SortedList * sl)
{
	TVINSERTSTRUCT tvis;
	int i;
	HTREEITEM hParent = nullptr;
	tvis.hParent = nullptr;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT;

	TreeView_DeleteAllItems(hToolBarTree);
	if (!sl) return 1;

	qsort(sl->items, sl->realCount, sizeof(ButtonData *), sstOpSortButtons);

	for (i = 0; i < sl->realCount; i++) {
		ButtonData * bd = (ButtonData *)sl->items[i];

		if (bd->dwOPFlags & QMF_DELETNEEDED)
			continue;

		tvis.item.lParam = (LPARAM)bd;
		tvis.item.pszText = bd->pszOpName;

		if (bd->fEntryOpType == 0)
			tvis.hParent = nullptr;

		hParent = TreeView_InsertItem(hToolBarTree, &tvis);
		if (tvis.hParent) TreeView_Expand(hMenuTree, tvis.hParent, TVE_EXPAND);
		if (!bd->pszOpValue&&bd->fEntryOpType == 0)
			tvis.hParent = hParent;
	}

	return 1;
}

static int BuildButtonsList(HWND hToolBarTree)
{
	TVINSERTSTRUCT tvis;
	int i = 0;
	tvis.hParent = nullptr;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT;

	TreeView_DeleteAllItems(hToolBarTree);

	//for(i=0;i<g_iButtonsCount;i++)
	while (ButtonsList[i]) {
		tvis.item.lParam = (LPARAM)ButtonsList[i];
		tvis.item.pszText = ButtonsList[i]->ptszButtonName;

		TreeView_InsertItem(hToolBarTree, &tvis);
		i++;
	}
	return 1;
}
///////////////////////////////////
//From UserInfoEx by DeathAxe
//
void MoveItem(HTREEITEM hItem, HTREEITEM hInsertAfter, BOOLEAN bAsChild)
{
	TVINSERTSTRUCT tvis;
	//wchar_t strbuf[128];
	HTREEITEM hParent, hChild, hNewItem;

	if (!hItem || !hInsertAfter)
		return;
	if (hItem == hInsertAfter)
		return;

	switch ((ULONG_PTR)hInsertAfter) {
	case TVI_ROOT:
	case TVI_FIRST:
	case TVI_LAST:
		hParent = NULL;
		bAsChild = FALSE;
		break;
	default:
		hParent = TreeView_GetParent(hMenuTree, hInsertAfter);
		break;
	}
	// do not move a parent next to its own children!
	if (hItem == hParent)
		return;

	// prepare the insert structure
	{
		wchar_t strbuf[128];
		tvis.item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;

		tvis.item.pszText = strbuf;
		tvis.item.cchTextMax = _countof(strbuf);
		tvis.item.hItem = hItem;
		TreeView_GetItem(hMenuTree, &tvis.item);
	}

	// item should be inserted as the first child of an existing root item
	if (bAsChild) {
		tvis.hParent = hInsertAfter;
		tvis.hInsertAfter = (bAsChild == 2) ? TVI_LAST : TVI_FIRST;
	}
	// item should be inserted after an existing item
	else {
		tvis.hParent = hParent;
		tvis.hInsertAfter = hInsertAfter;
	}
	// insert the item
	if (!(hNewItem = TreeView_InsertItem(hMenuTree, &tvis)))
		return;

	// move children
	hInsertAfter = hNewItem;
	while (hChild = TreeView_GetChild(hMenuTree, hItem)) {
		MoveItem(hChild, hInsertAfter, 2);
	}
	// delete old tree
	TreeView_DeleteItem(hMenuTree, hItem);

	TreeView_SelectItem(hMenuTree, hNewItem);
	//TreeView_Expand( hMenuTree, hNewItem, TVE_EXPAND );
}


///////
INT_PTR CALLBACK OptionsProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_INITDIALOG: {
			uint32_t style;
			g_opHdlg = hdlg;
			bOptionsInit = TRUE;
			TranslateDialogDefault(hdlg);
			if (g_iButtonsCount != g_plugin.getByte("ButtonsCount", 0)) {
				LOGFONT logFont;
				HFONT hFont;
				bNeedRestart = TRUE;
				EnableWindow(GetDlgItem(hdlg, IDC_BUTTONSLIST), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_BLISTADD), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_BLISTREMOVE), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_MENUTREE), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_MTREEADD), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_MTREEREMOVE), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_MENUVALUE), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_RCLICKVALUE), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_BUTTONNAME), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_INQMENU), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_ISSERVNAME), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_MENUNAME), FALSE);
				ShowWindow(GetDlgItem(hdlg, IDC_WARNING), SW_SHOW);

				hFont = (HFONT)SendDlgItemMessage(hdlg, IDC_WARNING, WM_GETFONT, 0, 0);
				GetObject(hFont, sizeof(logFont), &logFont);
				logFont.lfWeight = FW_BOLD;
				hFont = CreateFontIndirect(&logFont);
				SendDlgItemMessage(hdlg, IDC_WARNING, WM_SETFONT, (WPARAM)hFont, 0);
				break;
			}

			g_iOPButtonsCount = g_iButtonsCount;

			hButtonsList = GetDlgItem(hdlg, IDC_BUTTONSLIST);
			hMenuTree = GetDlgItem(hdlg, IDC_MENUTREE);

			style = GetWindowLongPtr(hButtonsList, GWL_STYLE);
			style |= TVS_NOHSCROLL;
			SetWindowLongPtr(hButtonsList, GWL_STYLE, style);

			style = GetWindowLongPtr(hMenuTree, GWL_STYLE);
			style |= TVS_NOHSCROLL;
			SetWindowLongPtr(hMenuTree, GWL_STYLE, style);
			BuildButtonsList(hButtonsList);

			if (!TreeView_GetCount(hButtonsList))
				EnableWindow(GetDlgItem(hdlg, IDC_RCLICKVALUE), FALSE);

			mir_subclassWindow(GetDlgItem(hdlg, IDC_BUTTONNAME), EditSubclassProc);
			mir_subclassWindow(GetDlgItem(hdlg, IDC_MENUNAME), EditSubclassProc);

			EnableWindow(GetDlgItem(hdlg, IDC_MENUVALUE), FALSE);
			EnableWindow(GetDlgItem(hdlg, IDC_INQMENU), FALSE);
			EnableWindow(GetDlgItem(hdlg, IDC_ISSERVNAME), FALSE);
			CheckDlgButton(hdlg, IDC_RAUTOSEND, (g_bRClickAuto = g_plugin.getByte("RClickAuto", 0)) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hdlg, IDC_LAUTOSEND, (g_bLClickAuto = g_plugin.getByte("LClickAuto", 0)) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hdlg, IDC_ENABLEQUICKMENU, (g_bQuickMenu = g_plugin.getByte("QuickMenu", 1)) ? BST_CHECKED : BST_UNCHECKED);

			bOptionsInit = FALSE;
		}break;

	case WM_LBUTTONUP:
		if (drag) {
			TVHITTESTINFO hti;
			HTREEITEM htiAfter = nullptr;
			ButtonData* bd = nullptr;
			TVITEM tvi;
			RECT rc;
			uint8_t height;
			BOOLEAN bAsChild = FALSE;

			TreeView_SetInsertMark(hMenuTree, NULL, 0);
			ReleaseCapture();
			SetCursor(LoadCursor(nullptr, IDC_ARROW));

			hti.pt.x = (SHORT)LOWORD(lparam);
			hti.pt.y = (SHORT)HIWORD(lparam);
			ClientToScreen(hdlg, &hti.pt);
			ScreenToClient(hMenuTree, &hti.pt);
			TreeView_HitTest(hMenuTree, &hti);

			if (TreeView_GetParent(hMenuTree, hti.hItem) && TreeView_GetChild(hMenuTree, hDragItem))
				break;

			if (TreeView_GetChild(hMenuTree, hti.hItem) && TreeView_GetChild(hMenuTree, hDragItem))
				break;


			if (hti.flags & TVHT_ABOVE) {
				htiAfter = TVI_FIRST;
			}
			else
				if (hti.flags & (TVHT_NOWHERE | TVHT_BELOW)) {
					htiAfter = TVI_LAST;
				}
				else
					if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
						// check where over the item, the pointer is
						if (!TreeView_GetItemRect(hMenuTree, hti.hItem, &rc, FALSE)) {
							drag = 0;
							break;
						}
						height = (uint8_t)(rc.bottom - rc.top);

						if (hti.pt.y - (height / 3) < rc.top) {
							HTREEITEM hItem = hti.hItem;

							if (!(hti.hItem = TreeView_GetPrevSibling(hMenuTree, hItem))) {
								if (!(hti.hItem = TreeView_GetParent(hMenuTree, hItem)))
									htiAfter = TVI_FIRST;
								else
									bAsChild = TRUE;
							}
						}
						else
							if (hti.pt.y + (height / 3) <= rc.bottom) {
								bAsChild = TRUE;
							}
					}


			if (TreeView_GetChild(hMenuTree, hDragItem) && bAsChild)
				break;


			if (hti.hItem) {
				tvi.hItem = hti.hItem;
				tvi.mask = TVIF_PARAM | TVIF_HANDLE;
				TreeView_GetItem(hMenuTree, &tvi);
				if ((bd = (ButtonData*)tvi.lParam) && (bd->fEntryOpType&QMF_EX_SEPARATOR))
					bAsChild = FALSE;
			}

			if (TreeView_GetParent(hMenuTree, hti.hItem))
				bAsChild = FALSE;


			MoveItem(hDragItem, htiAfter ? htiAfter : hti.hItem, bAsChild);
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			drag = 0;

		}
		break;

	case WM_MOUSEMOVE:
		if (!drag) break;
		{
			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(lparam);
			hti.pt.y = (short)HIWORD(lparam);
			ClientToScreen(hdlg, &hti.pt);
			ScreenToClient(hMenuTree, &hti.pt);
			TreeView_HitTest(hMenuTree, &hti);
			if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
				RECT rc;
				if (TreeView_GetItemRect(hMenuTree, hti.hItem, &rc, FALSE)) {
					uint8_t height = (uint8_t)(rc.bottom - rc.top);

					if (hti.pt.y - (height / 3) < rc.top) {
						SetCursor(LoadCursor(nullptr, IDC_ARROW));
						TreeView_SetInsertMark(hMenuTree, hti.hItem, 0);
					}
					else
						if (hti.pt.y + (height / 3) > rc.bottom) {
							SetCursor(LoadCursor(nullptr, IDC_ARROW));
							TreeView_SetInsertMark(hMenuTree, hti.hItem, 1);
						}
						else {
							TreeView_SetInsertMark(hMenuTree, NULL, 0);
							SetCursor(LoadCursor(GetModuleHandle(nullptr), MAKEINTRESOURCE(183)));
						}
				}
			}
			else {
				if (hti.flags & TVHT_ABOVE) SendMessage(hMenuTree, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				if (hti.flags & TVHT_BELOW) SendMessage(hMenuTree, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
				TreeView_SetInsertMark(hMenuTree, NULL, 0);
			}
		}
		break;

	case WM_DESTROY:
		if (g_varhelpDlg)
			DestroyWindow(g_varhelpDlg);
		g_varhelpDlg = nullptr;
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lparam)->idFrom) {
		case 0:
			if (((LPNMHDR)lparam)->code == PSN_APPLY) {
				if (!bNeedRestart) {
					SetMenuEntryProperties(hdlg);
					SaveMenuTree();
				}
				g_plugin.setByte("RClickAuto", (uint8_t)(g_bRClickAuto = IsDlgButtonChecked(hdlg, IDC_RAUTOSEND)));
				g_plugin.setByte("LClickAuto", (uint8_t)(g_bLClickAuto = IsDlgButtonChecked(hdlg, IDC_LAUTOSEND)));
				g_plugin.setByte("QuickMenu", (uint8_t)(g_bQuickMenu = IsDlgButtonChecked(hdlg, IDC_ENABLEQUICKMENU)));
				return 1;
			}
			else if (((LPNMHDR)lparam)->code == PSN_RESET) {
				if (!bNeedRestart)
					RestoreModuleData();
				return 1;
			}
			break;

		case IDC_MENUTREE:
			switch (((LPNMHDR)lparam)->code) {
			case TVN_KEYDOWN: {
					TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)((LPNMHDR)lparam);
					if (pTVKeyDown->wVKey == VK_F2)
						TreeView_EditLabel(hMenuTree, TreeView_GetSelection(hMenuTree));
					else if (pTVKeyDown->wVKey == VK_DELETE)
						SendMessage(hdlg, WM_COMMAND, IDC_MTREEREMOVE, 0);
				}break;

			case TVN_BEGINLABELEDIT:
				hwndEdit = TreeView_GetEditControl(hMenuTree);
				mir_subclassWindow(hwndEdit, LabelEditSubclassProc);
				break;

			case TVN_ENDLABELEDIT:
				{
					TVITEM tvi;
					ButtonData* bd = nullptr;
					wchar_t strbuf[256];
					wchar_t szLabel[256];

					tvi.pszText = strbuf;
					tvi.cchTextMax = _countof(strbuf);
					tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
					tvi.hItem = TreeView_GetSelection(hMenuTree);
					TreeView_GetItem(hMenuTree, &tvi);

					GetWindowText(hwndEdit, szLabel, _countof(szLabel));
					hwndEdit = nullptr;
					if (!mir_wstrlen(szLabel)) break;
					if (bd = (ButtonData*)tvi.lParam) {
						if (!mir_wstrcmp(szLabel, L"---")) {
							if (TreeView_GetChild(hMenuTree, tvi.hItem))
								break;
							else {
								bd->fEntryOpType = QMF_EX_SEPARATOR;
								EnableWindow(GetDlgItem(hdlg, IDC_MENUVALUE), FALSE);
								EnableWindow(GetDlgItem(hdlg, IDC_ISSERVNAME), FALSE);
								SetDlgItemText(hdlg, IDC_MENUVALUE, L"");
							}
						}
						else {
							bd->fEntryOpType &= ~QMF_EX_SEPARATOR;
							EnableWindow(GetDlgItem(hdlg, IDC_MENUVALUE), TRUE);
							EnableWindow(GetDlgItem(hdlg, IDC_ISSERVNAME), TRUE);
							SetDlgItemText(hdlg, IDC_MENUVALUE, bd->pszOpValue/*?bd->pszOpValue:bd->pszValue*/);
						}

						bd->pszOpName = mir_wstrdup(szLabel);

						tvi.pszText = szLabel;
						TreeView_SetItem(hMenuTree, &tvi);
						SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
					}
				}
				break;

			case NM_KILLFOCUS:
				TreeView_EndEditLabelNow(hButtonsList, 1);
				break;

			case TVN_BEGINDRAG:
				SetCapture(hdlg);
				drag = 1;
				hDragItem = ((LPNMTREEVIEW)lparam)->itemNew.hItem;
				TreeView_SelectItem(hMenuTree, hDragItem);
				break;

			case TVN_SELCHANGING:
				{
					HTREEITEM hti = TreeView_GetSelection(hMenuTree);
					if (hti == nullptr)
						break;

					TVITEM tvi;
					tvi.hItem = hti;
					tvi.mask = TVIF_HANDLE | TVIF_PARAM;
					TreeView_GetItem(hMenuTree, &tvi);
					if (tvi.lParam == 0)
						break;

					ButtonData *bd = (ButtonData*)tvi.lParam;
					if (bd) {
						wchar_t szValue[256];
						GetDlgItemText(hdlg, IDC_MENUVALUE, szValue, _countof(szValue));
						if (mir_wstrlen(szValue)) {
							if (bd->pszOpValue && (bd->pszOpValue != bd->pszValue))
								mir_free(bd->pszOpValue);
							bd->pszOpValue = mir_wstrdup(szValue);
						}
						bd->bOpInQMenu = IsDlgButtonChecked(hdlg, IDC_INQMENU);
						bd->bIsOpServName = IsDlgButtonChecked(hdlg, IDC_ISSERVNAME);
					}
				}
				break;

			case TVN_SELCHANGED:
				{
					HTREEITEM hti = TreeView_GetSelection(hMenuTree);
					if (hti == nullptr)
						break;

					TVITEM tvi;
					tvi.mask = TVIF_HANDLE | TVIF_PARAM;
					tvi.hItem = hti;
					TreeView_GetItem(hMenuTree, &tvi);

					ButtonData *bd = (ButtonData*)tvi.lParam;
					if (bd) {
						if (!TreeView_GetChild(hMenuTree, tvi.hItem) && !(bd->fEntryOpType&QMF_EX_SEPARATOR)) {
							EnableWindow(GetDlgItem(hdlg, IDC_MENUVALUE), TRUE);
							EnableWindow(GetDlgItem(hdlg, IDC_ISSERVNAME), TRUE);
							EnableWindow(GetDlgItem(hdlg, IDC_INQMENU), TRUE);
							SetDlgItemText(hdlg, IDC_MENUVALUE, bd->pszOpValue/*?bd->pszOpValue:bd->pszValue*/);
						}
						else {
							EnableWindow(GetDlgItem(hdlg, IDC_MENUVALUE), FALSE);
							EnableWindow(GetDlgItem(hdlg, IDC_ISSERVNAME), FALSE);
							if (!(bd->fEntryOpType&QMF_EX_SEPARATOR))
								EnableWindow(GetDlgItem(hdlg, IDC_INQMENU), FALSE);
							SetDlgItemText(hdlg, IDC_MENUVALUE, L"");
						}
						CheckDlgButton(hdlg, IDC_INQMENU, bd->bOpInQMenu ? BST_CHECKED : BST_UNCHECKED);
						CheckDlgButton(hdlg, IDC_ISSERVNAME, bd->bIsOpServName ? BST_CHECKED : BST_UNCHECKED);
					}
				}
			}
			break;

		case IDC_BUTTONSLIST:
			switch (((LPNMHDR)lparam)->code) {
			case TVN_KEYDOWN: {
					TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)((LPNMHDR)lparam);
					if (pTVKeyDown->wVKey == VK_F2)
						TreeView_EditLabel(hButtonsList, TreeView_GetSelection(hButtonsList));
					else if (pTVKeyDown->wVKey == VK_DELETE)
						SendMessage(hdlg, WM_COMMAND, IDC_BLISTREMOVE, 0);
				}break;

			case TVN_BEGINLABELEDIT:
				hwndEdit = TreeView_GetEditControl(hButtonsList);
				mir_subclassWindow(hwndEdit, LabelEditSubclassProc);
				break;

			case TVN_ENDLABELEDIT:
				{
					TVITEM tvi;
					wchar_t strbuf[128];
					wchar_t szLabel[128];

					tvi.pszText = strbuf;
					tvi.cchTextMax = _countof(strbuf);
					tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
					tvi.hItem = TreeView_GetSelection(hButtonsList);
					TreeView_GetItem(hButtonsList, &tvi);

					GetWindowText(hwndEdit, szLabel, _countof(szLabel));
					hwndEdit = nullptr;
					if (!mir_wstrlen(szLabel)) break;

					tvi.pszText = szLabel;
					((ListData*)tvi.lParam)->dwOPFlags |= QMF_RENAMED;

					TreeView_SetItem(hButtonsList, &tvi);
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				}
				break;

			case TVN_SELCHANGING:
				SetMenuEntryProperties(hdlg);
				break;

			case TVN_SELCHANGED:
				{
					HTREEITEM hti = TreeView_GetSelection(hButtonsList);
					if (hti == nullptr || !TreeView_GetCount(hButtonsList)) {
						EnableWindow(GetDlgItem(hdlg, IDC_MENUVALUE), FALSE);
						EnableWindow(GetDlgItem(hdlg, IDC_ISSERVNAME), FALSE);
						EnableWindow(GetDlgItem(hdlg, IDC_INQMENU), FALSE);
						EnableWindow(GetDlgItem(hdlg, IDC_ISSERVNAME2), FALSE);
						SetDlgItemText(hdlg, IDC_MENUVALUE, L"");
						break;
					}

					TVITEM tvi;
					tvi.mask = TVIF_HANDLE | TVIF_PARAM;
					tvi.hItem = hti;
					TreeView_GetItem(hButtonsList, &tvi);

					if (tvi.lParam == 0) break;

					BuildMenuTree(hMenuTree, (SortedList *)((ListData*)tvi.lParam)->sl);

					SetDlgItemText(hdlg, IDC_MENUVALUE, L"");
					EnableWindow(GetDlgItem(hdlg, IDC_RCLICKVALUE), TRUE);
					EnableWindow(GetDlgItem(hdlg, IDC_ISSERVNAME2), TRUE);
					CheckDlgButton(hdlg, IDC_ISSERVNAME2, ((ListData*)tvi.lParam)->bIsOpServName ? BST_CHECKED : BST_UNCHECKED);

					if (((ListData*)tvi.lParam)->ptszOPQValue)
						SetDlgItemText(hdlg, IDC_RCLICKVALUE, ((ListData*)tvi.lParam)->ptszOPQValue);
					else
						SetDlgItemText(hdlg, IDC_RCLICKVALUE, L"");
				}
				break;
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wparam)) {
		case IDC_VARHELP:
			if (!g_varhelpDlg)
				g_varhelpDlg = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_HELPDIALOG), nullptr, HelpDlgProc);
			else
				//ShowWindow(g_varhelpDlg,SW_SHOWDEFAULT);
				SetWindowPos(g_varhelpDlg, nullptr, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
			break;
		case IDC_BLISTADD:
			{
				TVINSERTSTRUCT tvis;
				ListData* ld = nullptr;
				wchar_t namebuff[MAX_PATH] = { '\0' };
				int count = TreeView_GetCount(hButtonsList);
				if (count > 10) break;
				if (g_iOPButtonsCount == 99) {
					MessageBox(nullptr, TranslateT("Congratulation!\r\nYou have clicked this button 100 times!\r\nThere was access violation at this point...\r\nAnd now function for freeing resources must be called...\r\nBut no! there's only break :D"), TranslateT("You win!"), MB_OK);
					break;
				}

				ld = (ListData *)mir_alloc(sizeof(ListData));
				ButtonsList[g_iOPButtonsCount++] = ld;

				ld->sl = List_Create(0, 1);
				ld->dwOPFlags = QMF_NEW;
				ld->bIsOpServName = 0;
				ld->ptszButtonName = nullptr;
				ld->ptszOPQValue = nullptr;
				ld->ptszQValue = nullptr;
				tvis.hParent = nullptr;
				tvis.hInsertAfter = TVI_LAST;

				GetDlgItemText(hdlg, IDC_BUTTONNAME, namebuff, _countof(namebuff));

				tvis.item.mask = TVIF_PARAM | TVIF_TEXT;
				tvis.item.pszText = (mir_wstrlen(namebuff)) ? namebuff : TranslateT("New Button");
				tvis.item.lParam = (LPARAM)ld;
				TreeView_SelectItem(hButtonsList, TreeView_InsertItem(hButtonsList, &tvis));
			}break;

		case IDC_BLISTREMOVE:
			{
				TVITEM tvi;
				ListData* ld;

				if (!(tvi.hItem = TreeView_GetSelection(hButtonsList)))
					break;

				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				TreeView_GetItem(hButtonsList, &tvi);

				ld = (ListData*)tvi.lParam;

				ld->dwOPFlags |= QMF_DELETNEEDED;

				TreeView_DeleteItem(hButtonsList, tvi.hItem);
				if (!TreeView_GetCount(hButtonsList)) {
					TreeView_DeleteAllItems(hMenuTree);
					EnableWindow(GetDlgItem(hdlg, IDC_MENUVALUE), FALSE);
					EnableWindow(GetDlgItem(hdlg, IDC_RCLICKVALUE), FALSE);
					EnableWindow(GetDlgItem(hdlg, IDC_ISSERVNAME), FALSE);
					EnableWindow(GetDlgItem(hdlg, IDC_INQMENU), FALSE);
					EnableWindow(GetDlgItem(hdlg, IDC_ISSERVNAME2), FALSE);
					SetDlgItemText(hdlg, IDC_MENUVALUE, L"");
					SetDlgItemText(hdlg, IDC_RCLICKVALUE, L"");
				}
			}break;

		case IDC_MTREEADD:
			{
				TVINSERTSTRUCT tvis;
				TVITEM tvi;
				ButtonData *bd = nullptr;
				SortedList *sl = nullptr;
				wchar_t namebuff[MAX_PATH] = { '\0' };

				if (!TreeView_GetCount(hButtonsList)) break;
				if (!(tvi.hItem = TreeView_GetSelection(hButtonsList))) break;

				bd = (ButtonData *)mir_alloc(sizeof(ButtonData));
				memset(bd, 0, sizeof(ButtonData));

				GetDlgItemText(hdlg, IDC_MENUNAME, namebuff, _countof(namebuff));

				bd->dwOPPos = TreeView_GetCount(hMenuTree) - 1;
				bd->pszOpName = mir_wstrlen(namebuff) ? mir_wstrdup(namebuff) : mir_wstrdup(TranslateT("New Menu Entry"));
				bd->pszOpValue = mir_wstrdup(bd->pszOpName);
				bd->fEntryOpType = !mir_wstrcmp(namebuff, L"---") ? QMF_EX_SEPARATOR : 0;
				bd->dwOPFlags = QMF_NEW;
				bd->pszName = nullptr;
				bd->pszValue = nullptr;


				tvi.mask = TVIF_HANDLE | TVIF_PARAM;

				TreeView_GetItem(hButtonsList, &tvi);

				sl = ((ListData*)tvi.lParam)->sl;

				List_InsertPtr(sl, bd);

				tvis.hParent = nullptr;
				tvis.hInsertAfter = TVI_LAST;
				tvis.item.mask = TVIF_PARAM | TVIF_TEXT;
				tvis.item.pszText = bd->pszOpName;
				tvis.item.lParam = (LPARAM)bd;
				TreeView_SelectItem(hMenuTree, TreeView_InsertItem(hMenuTree, &tvis));
			}break;

		case IDC_MTREEREMOVE:
			{
				TVITEM tvi;
				TVINSERTSTRUCT tvis;
				HTREEITEM hti = nullptr;
				ButtonData *bd = nullptr;
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				if (!(tvi.hItem = TreeView_GetSelection(hMenuTree)))
					break;
				TreeView_GetItem(hMenuTree, &tvi);
				hti = tvi.hItem;

				bd = (ButtonData*)tvi.lParam;
				bd->dwOPFlags |= QMF_DELETNEEDED;

				if (tvi.hItem = TreeView_GetChild(hMenuTree, tvi.hItem)) {
					wchar_t strbuf[128];
					while (tvi.hItem) {
						tvis.hInsertAfter = hti;
						tvi.pszText = strbuf;
						tvi.cchTextMax = _countof(strbuf);
						tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;

						TreeView_GetItem(hMenuTree, &tvi);
						tvis.hParent = nullptr;
						tvis.item = tvi;
						TreeView_InsertItem(hMenuTree, &tvis);
						tvi.hItem = TreeView_GetNextSibling(hMenuTree, tvi.hItem);
					}
				}

				TreeView_DeleteItem(hMenuTree, hti);
				if (!TreeView_GetCount(hMenuTree)) {
					EnableWindow(GetDlgItem(hdlg, IDC_MENUVALUE), FALSE);
					EnableWindow(GetDlgItem(hdlg, IDC_ISSERVNAME), FALSE);
					EnableWindow(GetDlgItem(hdlg, IDC_INQMENU), FALSE);
					SetDlgItemText(hdlg, IDC_MENUVALUE, L"");
				}
			}break;
		}
		break;

	case WM_CLOSE:
		EndDialog(hdlg, 0);
		return 0;
	}

	if (HIWORD(wparam) == BN_CLICKED && GetFocus() == (HWND)lparam)
		SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
	else if ((HIWORD(wparam) == EN_CHANGE) && (GetFocus() == (HWND)lparam))
		if (!bOptionsInit)	SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);

	return 0;
}

int OptionsInit(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 940000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.szTitle.a = LPGEN("Quick Messages");
	odp.pfnDlgProc = OptionsProc;
	odp.szGroup.a = LPGEN("Message sessions");
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wparam, &odp);
	return 0;
}

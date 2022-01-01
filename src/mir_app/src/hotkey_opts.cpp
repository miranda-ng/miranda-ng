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

#include <m_hotkeys.h>
#include "skin.h"

static wchar_t* sttHokeyVkToName(uint16_t vkKey)
{
	static wchar_t buf[256] = { 0 };
	uint32_t code = MapVirtualKey(vkKey, 0) << 16;

	switch (vkKey) {
	case 0:
	case VK_CONTROL:
	case VK_SHIFT:
	case VK_MENU:
	case VK_LWIN:
	case VK_RWIN:
	case VK_PAUSE:
	case VK_CANCEL:
	case VK_NUMLOCK:
	case VK_CAPITAL:
	case VK_SCROLL:
		return L"";
	case VK_BROWSER_BACK:
		return TranslateT("Browser: Back");
	case VK_BROWSER_FORWARD:
		return TranslateT("Browser: Forward");
	case VK_BROWSER_REFRESH:
		return TranslateT("Browser: Refresh");
	case VK_BROWSER_STOP:
		return TranslateT("Browser: Stop");
	case VK_BROWSER_SEARCH:
		return TranslateT("Browser: Search");
	case VK_BROWSER_FAVORITES:
		return TranslateT("Browser: Fav");
	case VK_BROWSER_HOME:
		return TranslateT("Browser: Home");
	case VK_VOLUME_MUTE:
		return TranslateT("Mute");
	case VK_VOLUME_DOWN:
		return TranslateT("Vol-");
	case VK_VOLUME_UP:
		return TranslateT("Vol+");
	case VK_MEDIA_NEXT_TRACK:
		return TranslateT("Media: Next Track");
	case VK_MEDIA_PREV_TRACK:
		return TranslateT("Media: Prev. Track");
	case VK_MEDIA_STOP:
		return TranslateT("Media: Stop");
	case VK_MEDIA_PLAY_PAUSE:
		return TranslateT("Media: Play/Pause");
	case VK_LAUNCH_MAIL:
		return TranslateT("Mail");
	case VK_LAUNCH_MEDIA_SELECT:
		return TranslateT("Media: Select");
	case VK_LAUNCH_APP1:
		return TranslateT("App 1");
	case VK_LAUNCH_APP2:
		return TranslateT("App 2");

	case VK_DIVIDE:
	case VK_INSERT:
	case VK_HOME:
	case VK_PRIOR:
	case VK_DELETE:
	case VK_END:
	case VK_NEXT:
	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_DOWN:
		code |= (1UL << 24);
	}

	GetKeyNameText(code, buf, 256);
	return buf;
}

void HotkeyToName(wchar_t *buf, int size, uint8_t shift, uint8_t key)
{
	mir_snwprintf(buf, size, L"%s%s%s%s%s",
		(shift & HOTKEYF_CONTROL) ? TranslateT("Ctrl + ") : L"",
		(shift & HOTKEYF_ALT) ? TranslateT("Alt + ") : L"",
		(shift & HOTKEYF_SHIFT) ? TranslateT("Shift + ") : L"",
		(shift & HOTKEYF_EXT) ? TranslateT("Win + ") : L"",
		sttHokeyVkToName(key));
}

///////////////////////////////////////////////////////////////////////////////
// Hotkey control

static LRESULT CALLBACK sttHotkeyEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	THotkeyBoxData *data = (THotkeyBoxData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	BOOL bKeyDown = FALSE;
	if (!data) return 0;

	switch (msg) {
	case HKM_GETHOTKEY:
		return data->key ? MAKEWORD(data->key, data->shift) : 0;

	case HKM_SETHOTKEY:
		{
			wchar_t buf[256] = { 0 };
			data->key = (uint8_t)LOWORD(wParam);
			data->shift = (uint8_t)HIWORD(wParam);
			HotkeyToName(buf, _countof(buf), data->shift, data->key);
			SetWindowText(hwnd, buf);
		}
		return 0;

	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;

	case WM_KILLFOCUS:
		break;

	case WM_CHAR:
	case WM_SYSCHAR:
	case WM_PASTE:
	case WM_CONTEXTMENU:
		return TRUE;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		bKeyDown = TRUE;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			wchar_t buf[256] = { 0 };

			uint8_t shift = 0;
			uint8_t key = wParam;
			wchar_t *name = sttHokeyVkToName(key);
			if (!*name || !bKeyDown)
				key = 0;

			if (GetAsyncKeyState(VK_CONTROL)) shift |= HOTKEYF_CONTROL;
			if (GetAsyncKeyState(VK_MENU))    shift |= HOTKEYF_ALT;
			if (GetAsyncKeyState(VK_SHIFT))   shift |= HOTKEYF_SHIFT;
			if (GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_RWIN)) shift |= HOTKEYF_EXT;

			if (bKeyDown || !data->key) {
				data->shift = shift;
				data->key = key;
			}

			HotkeyToName(buf, _countof(buf), data->shift, data->key);
			SetWindowText(hwnd, buf);

			if (bKeyDown && data->key)
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(GetWindowLongPtr(hwnd, GWL_ID), 0), (LPARAM)hwnd);
		}
		return TRUE;

	case WM_DESTROY:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
		mir_free(data);
	}

	return mir_callNextSubclass(hwnd, sttHotkeyEditProc, msg, wParam, lParam);
}

MIR_APP_DLL(void) Hotkey_Subclass(HWND hwnd)
{
	THotkeyBoxData *data = (THotkeyBoxData *)mir_alloc(sizeof(THotkeyBoxData));
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (ULONG_PTR)data);
	mir_subclassWindow(hwnd, sttHotkeyEditProc);
}

MIR_APP_DLL(void) Hotkey_Unsubclass(HWND hwnd)
{
	THotkeyBoxData *data = (THotkeyBoxData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
	mir_free(data);
}

///////////////////////////////////////////////////////////////////////////////
// Options

enum { COL_NAME, COL_TYPE, COL_KEY, COL_RESET, COL_ADDREMOVE };

static void sttOptionsSetupItem(HWND hwndList, int idx, THotkeyItem *item)
{
	wchar_t buf[256];
	LVITEM lvi = { 0 };
	lvi.iItem = idx;

	if (!item->rootHotkey) {
		lvi.mask = LVIF_TEXT | LVIF_IMAGE;
		lvi.iSubItem = COL_NAME;
		lvi.pszText = item->getDescr();
		lvi.iImage = item->OptType;
		ListView_SetItem(hwndList, &lvi);

		ListView_SetCheckState(hwndList, lvi.iItem, item->Enabled);
	}

	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = COL_KEY;
	HotkeyToName(buf, _countof(buf), HIBYTE(item->OptHotkey), LOBYTE(item->OptHotkey));
	lvi.pszText = buf;
	ListView_SetItem(hwndList, &lvi);

	if (item->rootHotkey) {
		lvi.mask = LVIF_IMAGE;
		lvi.iSubItem = COL_TYPE;
		lvi.iImage = item->OptType;
		ListView_SetItem(hwndList, &lvi);
	}

	lvi.mask = LVIF_IMAGE;
	lvi.iSubItem = COL_RESET;
	lvi.iImage = (item->Hotkey != item->OptHotkey) ? 5 : -1;
	ListView_SetItem(hwndList, &lvi);

	lvi.mask = LVIF_IMAGE | LVIF_TEXT;
	lvi.iSubItem = COL_ADDREMOVE;
	if (item->rootHotkey) {
		lvi.iImage = 4;
		lvi.pszText = TranslateT("Remove shortcut");
	}
	else {
		lvi.iImage = 3;
		lvi.pszText = TranslateT("Add another shortcut");
	}
	ListView_SetItem(hwndList, &lvi);
}

static void sttOptionsDeleteHotkey(HWND hwndList, int idx, THotkeyItem *item)
{
	item->OptDeleted = TRUE;
	ListView_DeleteItem(hwndList, idx);
	if (item->rootHotkey)
		item->rootHotkey->OptChanged = TRUE;
}

static int CALLBACK sttOptionsSortList(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	wchar_t title1[256] = { 0 }, title2[256] = { 0 };
	THotkeyItem *item1 = nullptr, *item2 = nullptr;
	LVITEM lvi = { 0 };
	int res;

	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = lParam1;
	lvi.pszText = title1;
	lvi.cchTextMax = _countof(title1);
	if (ListView_GetItem((HWND)lParamSort, &lvi))
		item1 = (THotkeyItem *)lvi.lParam;

	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = lParam2;
	lvi.pszText = title2;
	lvi.cchTextMax = _countof(title2);
	if (ListView_GetItem((HWND)lParamSort, &lvi))
		item2 = (THotkeyItem *)lvi.lParam;

	if (!item1 && !item2)
		return mir_wstrcmp(title1, title2);

	if (!item1 && item2) {
		if (res = mir_wstrcmp(title1, item2->getSection()))
			return res;
		return -1;
	}

	if (!item2 && item1) {
		if (res = mir_wstrcmp(item1->getSection(), title2))
			return res;
		return 1;
	}
	/* item1 != nullptr && item2 != nullptr */

	if (res = mir_wstrcmp(item1->getSection(), item2->getSection())) return res;
	if (res = mir_wstrcmp(item1->getDescr(), item2->getDescr())) return res;
	if (!item1->rootHotkey && item2->rootHotkey) return -1;
	if (item1->rootHotkey && !item2->rootHotkey) return 1;
	return 0;
}

static void sttOptionsAddHotkey(HWND hwndList, THotkeyItem *item)
{
	char buf[256];
	mir_snprintf(buf, "mir_hotkey_%d_%d", g_pid, g_hkid++);

	THotkeyItem *newItem = (THotkeyItem *)mir_alloc(sizeof(THotkeyItem));
	newItem->pszName = nullptr;
	newItem->pszService = item->pszService ? mir_strdup(item->pszService) : nullptr;
	newItem->pwszSection = mir_wstrdup(item->pwszSection);
	newItem->pwszDescription = mir_wstrdup(item->pwszDescription);
	newItem->lParam = item->lParam;
	newItem->idHotkey = GlobalAddAtomA(buf);
	newItem->rootHotkey = item;
	newItem->Hotkey = newItem->DefHotkey = newItem->OptHotkey = 0;
	newItem->type = newItem->OptType = item->OptType;
	newItem->Enabled = newItem->OptEnabled = TRUE;
	newItem->OptChanged = newItem->OptDeleted = FALSE;
	newItem->OptNew = TRUE;
	hotkeys.insert(newItem);

	SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);

	LVITEM lvi = { 0 };
	lvi.mask |= LVIF_PARAM;
	lvi.lParam = (LPARAM)newItem;
	sttOptionsSetupItem(hwndList, ListView_InsertItem(hwndList, &lvi), newItem);
	ListView_SortItemsEx(hwndList, sttOptionsSortList, (LPARAM)hwndList);

	SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);
	RedrawWindow(hwndList, nullptr, nullptr, RDW_INVALIDATE);

	item->OptChanged = TRUE;
}

static void sttOptionsSetChanged(THotkeyItem *item)
{
	item->OptChanged = TRUE;
	if (item->rootHotkey)
		item->rootHotkey->OptChanged = TRUE;
}

static void sttOptionsSaveItem(THotkeyItem *item)
{
	char buf[MAXMODULELABELLENGTH];

	if (item->rootHotkey) return;
	if (!item->OptChanged) return;

	item->Hotkey = item->OptHotkey;
	item->type = item->OptType;
	item->Enabled = item->OptEnabled;

	db_set_w(0, DBMODULENAME, item->pszName, item->Hotkey);
	db_set_b(0, DBMODULENAME "Off", item->pszName, (uint8_t)!item->Enabled);
	if (item->type != HKT_MANUAL)
		db_set_b(0, DBMODULENAME "Types", item->pszName, (uint8_t)item->type);

	item->nSubHotkeys = 0;
	for (auto &it : hotkeys) {
		if (it->rootHotkey == item) {
			it->Hotkey = it->OptHotkey;
			it->type = it->OptType;

			mir_snprintf(buf, "%s$%d", item->pszName, item->nSubHotkeys);
			db_set_w(0, DBMODULENAME, buf, it->Hotkey);
			if (it->type != HKT_MANUAL)
				db_set_b(0, DBMODULENAME "Types", buf, (uint8_t)it->type);

			++item->nSubHotkeys;
		}
	}

	mir_snprintf(buf, "%s$count", item->pszName);
	db_set_dw(0, DBMODULENAME, buf, item->nSubHotkeys);
}

static void sttBuildHotkeyList(HWND hwndList)
{
	ListView_DeleteAllItems(hwndList);

	int nItems = 0;
	THotkeyItem *prevItem = nullptr;
	for (auto &item : hotkeys) {
		LVITEM lvi = { 0 };

		if (!item->OptDeleted) {
			if (!prevItem || mir_wstrcmp(item->pwszSection, prevItem->pwszSection)) {
				lvi.mask = LVIF_TEXT | LVIF_PARAM;
				lvi.iItem = nItems++;
				lvi.iSubItem = 0;
				lvi.lParam = 0;
				lvi.pszText = item->getSection();
				ListView_InsertItem(hwndList, &lvi);
				ListView_SetCheckState(hwndList, lvi.iItem, TRUE);

				lvi.mask = LVIF_TEXT;
				lvi.iSubItem = 1;
				lvi.pszText = item->pwszSection;
				ListView_SetItem(hwndList, &lvi);

				lvi.iSubItem = 0;
			}

			lvi.mask = LVIF_PARAM | LVIF_INDENT;
			lvi.iIndent = 1;
			lvi.iItem = nItems++;
			lvi.lParam = (LPARAM)item;
			ListView_InsertItem(hwndList, &lvi);
			sttOptionsSetupItem(hwndList, nItems - 1, item);
		}

		prevItem = item;
	}

	ListView_SortItemsEx(hwndList, sttOptionsSortList, (LPARAM)hwndList);
}

static void sttOptionsStartEdit(HWND hwndDlg, HWND hwndHotkey)
{
	LVITEM lvi;
	THotkeyItem *item;
	int iItem = ListView_GetNextItem(hwndHotkey, -1, LVNI_SELECTED);
	if (iItem < 0) return;

	lvi.mask = LVIF_PARAM;
	lvi.iItem = iItem;
	ListView_GetItem(hwndHotkey, &lvi);

	if (item = (THotkeyItem *)lvi.lParam) {
		RECT rc;
		ListView_GetSubItemRect(hwndHotkey, iItem, COL_KEY, LVIR_BOUNDS, &rc);
		MapWindowPoints(hwndHotkey, hwndDlg, (LPPOINT)&rc, 2);
		SendDlgItemMessage(hwndDlg, IDC_HOTKEY, HKM_SETHOTKEY, MAKELONG(LOBYTE(item->OptHotkey), HIBYTE(item->OptHotkey)), 0);

		SetWindowPos(hwndHotkey, HWND_BOTTOM, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
		SetWindowPos(GetDlgItem(hwndDlg, IDC_HOTKEY), HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
		RedrawWindow(GetDlgItem(hwndDlg, IDC_HOTKEY), nullptr, nullptr, RDW_INVALIDATE);

		SetFocus(GetDlgItem(hwndDlg, IDC_HOTKEY));
		RedrawWindow(GetDlgItem(hwndDlg, IDC_HOTKEY), nullptr, nullptr, RDW_INVALIDATE);
	}
}

static void sttOptionsDrawTextChunk(HDC hdc, wchar_t *text, RECT *rc)
{
	DrawText(hdc, text, -1, rc, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS);

	SIZE sz;
	GetTextExtentPoint32(hdc, text, (int)mir_wstrlen(text), &sz);
	rc->left += sz.cx;
}

static INT_PTR CALLBACK sttOptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL initialized = FALSE;
	static int colWidth = 0;
	static uint16_t currentLanguage = 0;

	HWND hwndHotkey = GetDlgItem(hwndDlg, IDC_LV_HOTKEYS);

	switch (msg) {
	case WM_INITDIALOG:
		initialized = FALSE;

		TranslateDialogDefault(hwndDlg);

		Hotkey_Subclass(GetDlgItem(hwndDlg, IDC_HOTKEY));
		{
			HIMAGELIST hIml = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 3, 1);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_WINDOWS);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_MIRANDA);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_WINDOW);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_ADDCONTACT);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_DELETE);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_UNDO);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_GROUPOPEN);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_GROUPSHUT);
			ListView_SetImageList(hwndHotkey, hIml, LVSIL_SMALL);
		}
		ListView_SetExtendedListViewStyle(hwndHotkey, LVS_EX_CHECKBOXES | LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);
		{
			RECT rc;
			GetClientRect(hwndHotkey, &rc);
			colWidth = rc.right - GetSystemMetrics(SM_CXHTHUMB) - 3 * g_iIconSX - 5;

			LVCOLUMN lvc;
			lvc.mask = LVCF_WIDTH;
			lvc.cx = colWidth * 2 / 3;
			ListView_InsertColumn(hwndHotkey, COL_NAME, &lvc);
			lvc.cx = g_iIconSX;
			ListView_InsertColumn(hwndHotkey, COL_TYPE, &lvc);
			lvc.cx = colWidth / 3;
			ListView_InsertColumn(hwndHotkey, COL_KEY, &lvc);
			lvc.cx = g_iIconSX;
			ListView_InsertColumn(hwndHotkey, COL_RESET, &lvc);
			lvc.cx = g_iIconSX;
			ListView_InsertColumn(hwndHotkey, COL_ADDREMOVE, &lvc);

			for (auto &it : hotkeys) {
				it->OptChanged = FALSE;
				it->OptDeleted = it->OptNew = FALSE;
				it->OptEnabled = it->Enabled;
				it->OptHotkey = it->Hotkey;
				it->OptType = it->type;
			}

			currentLanguage = LOWORD(GetKeyboardLayout(0));
			sttBuildHotkeyList(hwndHotkey);
		}
		SetTimer(hwndDlg, 1024, 1000, nullptr);
		initialized = TRUE;
		{
			/* load group states */
			int count = ListView_GetItemCount(hwndHotkey);
			wchar_t buf[128];
			LVITEM lvi = { 0 };
			lvi.pszText = buf;
			lvi.cchTextMax = _countof(buf);
			for (lvi.iItem = 0; lvi.iItem < count; ++lvi.iItem) {
				char *szSetting;

				lvi.mask = LVIF_PARAM;
				lvi.iSubItem = 0;
				ListView_GetItem(hwndHotkey, &lvi);
				if (lvi.lParam) continue;

				lvi.mask = LVIF_TEXT;
				lvi.iSubItem = 1;
				ListView_GetItem(hwndHotkey, &lvi);

				szSetting = mir_u2a(lvi.pszText);

				ListView_SetCheckState(hwndHotkey, lvi.iItem,
					db_get_b(0, DBMODULENAME "UI", szSetting, TRUE));

				mir_free(szSetting);
			}
		}

		g_hwndHkOptions = hwndDlg;
		break;

	case WM_TIMER:
		if (initialized) {
			uint16_t newLanguage = LOWORD(GetKeyboardLayout(0));
			if (newLanguage == currentLanguage)
				break;

			int count = ListView_GetItemCount(hwndHotkey);

			LVITEM lvi = { 0 };
			lvi.mask = LVIF_PARAM;
			for (lvi.iItem = 0; lvi.iItem < count; ++lvi.iItem) {
				ListView_GetItem(hwndHotkey, &lvi);
				if (lvi.lParam)
					sttOptionsSetupItem(hwndHotkey, lvi.iItem, (THotkeyItem *)lvi.lParam);
			}
			currentLanguage = newLanguage;
		}
		break;

	case WM_HOTKEYUNREGISTERED:
		{
			int count = ListView_GetItemCount(hwndHotkey);

			LVITEM lvi = { 0 };
			lvi.mask = LVIF_PARAM;
			for (lvi.iItem = 0; lvi.iItem < count; ++lvi.iItem) {
				ListView_GetItem(hwndHotkey, &lvi);
				if (!lvi.lParam) continue;

				if (((THotkeyItem *)lvi.lParam)->UnregisterHotkey) {
					ListView_DeleteItem(hwndHotkey, lvi.iItem);
					--lvi.iItem;
					--count;
				}
			}
		}
		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
			RECT rc = lpdis->rcItem;
			int prefix = 65;
			int width = (lpdis->rcItem.right - lpdis->rcItem.left - prefix) / 3;
			rc.left += 5;

			HIMAGELIST hIml = ListView_GetImageList(hwndHotkey, LVSIL_SMALL);
			if (lpdis->CtlID == IDC_CANVAS2) {
				sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("Scope:"), &rc);

				rc.left = prefix;
				ImageList_Draw(hIml, 0, lpdis->hDC, rc.left, (rc.top + rc.bottom - 16) / 2, ILD_TRANSPARENT);
				rc.left += 20;
				sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("System"), &rc);

				rc.left = prefix + width;
				ImageList_Draw(hIml, 1, lpdis->hDC, rc.left, (rc.top + rc.bottom - 16) / 2, ILD_TRANSPARENT);
				rc.left += 20;
				sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("Miranda"), &rc);

				rc.left = prefix + width * 2;
				ImageList_Draw(hIml, 2, lpdis->hDC, rc.left, (rc.top + rc.bottom - 16) / 2, ILD_TRANSPARENT);
				rc.left += 20;
				sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("Window"), &rc);
				return TRUE;
			}

			if (lpdis->CtlID == IDC_CANVAS) {
				sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("Actions:"), &rc);

				rc.left = prefix;
				ImageList_Draw(hIml, 5, lpdis->hDC, rc.left, (rc.top + rc.bottom - 16) / 2, ILD_TRANSPARENT);
				rc.left += 20;
				sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("Undo"), &rc);

				rc.left = prefix + width * 1;
				ImageList_Draw(hIml, 3, lpdis->hDC, rc.left, (rc.top + rc.bottom - 16) / 2, ILD_TRANSPARENT);
				rc.left += 20;
				sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("Add binding"), &rc);

				rc.left = prefix + width * 2;
				ImageList_Draw(hIml, 4, lpdis->hDC, rc.left, (rc.top + rc.bottom - 16) / 2, ILD_TRANSPARENT);
				rc.left += 20;
				sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("Remove"), &rc);
				return TRUE;
			}
		}
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_HOTKEY) && ((HIWORD(wParam) == EN_KILLFOCUS) || (HIWORD(wParam) == 0))) {
			LVITEM lvi;
			THotkeyItem *item;
			uint16_t wHotkey = (uint16_t)SendDlgItemMessage(hwndDlg, IDC_HOTKEY, HKM_GETHOTKEY, 0, 0);

			ShowWindow(GetDlgItem(hwndDlg, IDC_HOTKEY), SW_HIDE);
			SetFocus(hwndHotkey);
			if (!wHotkey || (wHotkey == VK_ESCAPE) || (HIWORD(wParam) != 0))
				break;

			lvi.mask = LVIF_PARAM;
			lvi.iItem = ListView_GetNextItem(hwndHotkey, -1, LVNI_SELECTED);
			if (lvi.iItem >= 0) {
				ListView_GetItem(hwndHotkey, &lvi);
				if (item = (THotkeyItem *)lvi.lParam) {
					item->OptHotkey = wHotkey;

					sttOptionsSetupItem(hwndHotkey, lvi.iItem, item);
					sttOptionsSetChanged(item);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}
		}
		break;

	case WM_CONTEXTMENU:
		if (GetWindowLongPtr((HWND)wParam, GWL_ID) == IDC_LV_HOTKEYS) {
			HWND hwndList = (HWND)wParam;
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			LVITEM lvi = { 0 };
			THotkeyItem *item = nullptr;

			lvi.iItem = ListView_GetNextItem(hwndHotkey, -1, LVNI_SELECTED);
			if (lvi.iItem < 0) return FALSE;

			lvi.mask = LVIF_PARAM;
			ListView_GetItem(hwndList, &lvi);
			if (!(item = (THotkeyItem *)lvi.lParam)) return FALSE;

			if ((pt.x == -1) && (pt.y == -1)) {
				RECT rc;
				ListView_GetItemRect(hwndList, lvi.iItem, &rc, LVIR_LABEL);
				pt.x = rc.left;
				pt.y = rc.bottom;
				ClientToScreen(hwndList, &pt);
			}

			enum { MI_CANCEL, MI_CHANGE, MI_SYSTEM, MI_LOCAL, MI_ADD, MI_REMOVE, MI_REVERT };

			MENUITEMINFO mii = { 0 };
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_STATE;
			mii.fState = MFS_DEFAULT;

			HMENU hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING, (UINT_PTR)MI_CHANGE, TranslateT("Modify"));
			SetMenuItemInfo(hMenu, (UINT_PTR)MI_CHANGE, FALSE, &mii);
			if (item->type != HKT_MANUAL) {
				AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
				AppendMenu(hMenu, MF_STRING |
					((item->OptType == HKT_GLOBAL) ? MF_CHECKED : 0),
					(UINT_PTR)MI_SYSTEM, TranslateT("System scope"));
				AppendMenu(hMenu, MF_STRING |
					((item->OptType == HKT_LOCAL) ? MF_CHECKED : 0),
					(UINT_PTR)MI_LOCAL, TranslateT("Miranda scope"));
			}
			AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
			if (!item->rootHotkey)
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)MI_ADD, TranslateT("Add binding"));
			else
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)MI_REMOVE, TranslateT("Remove"));
			if (item->Hotkey != item->OptHotkey) {
				AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)MI_REVERT, TranslateT("Undo"));
			}

			switch (TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, nullptr)) {
			case MI_CHANGE:
				sttOptionsStartEdit(hwndDlg, hwndHotkey);
				break;
			case MI_SYSTEM:
				item->OptType = HKT_GLOBAL;
				sttOptionsSetupItem(hwndList, lvi.iItem, item);
				sttOptionsSetChanged(item);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case MI_LOCAL:
				item->OptType = HKT_LOCAL;
				sttOptionsSetupItem(hwndList, lvi.iItem, item);
				sttOptionsSetChanged(item);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case MI_ADD:
				initialized = FALSE;
				sttOptionsAddHotkey(hwndList, item);
				initialized = FALSE;
				break;
			case MI_REMOVE:
				sttOptionsDeleteHotkey(hwndList, lvi.iItem, item);
				break;
			case MI_REVERT:
				item->OptHotkey = item->Hotkey;
				sttOptionsSetupItem(hwndList, lvi.iItem, item);
				break;
			}
			DestroyMenu(hMenu);
			break;
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;
			switch (lpnmhdr->idFrom) {
			case 0:
				if ((lpnmhdr->code != PSN_APPLY) && (lpnmhdr->code != PSN_RESET))
					break;

				UnregisterHotkeys();

				for (auto &p : hotkeys.rev_iter())
					if (p->OptNew && p->OptDeleted || p->rootHotkey && !p->OptHotkey || (lpnmhdr->code == PSN_APPLY) && p->OptDeleted || (lpnmhdr->code == PSN_RESET) && p->OptNew)
						FreeHotkey(hotkeys.removeItem(&p));

				if (lpnmhdr->code == PSN_APPLY) {
					LVITEM lvi = { 0 };
					int count = ListView_GetItemCount(hwndHotkey);

					for (auto &it : hotkeys)
						sttOptionsSaveItem(it);

					lvi.mask = LVIF_IMAGE;
					lvi.iSubItem = COL_RESET;
					lvi.iImage = -1;
					for (lvi.iItem = 0; lvi.iItem < count; ++lvi.iItem)
						ListView_SetItem(hwndHotkey, &lvi);
				}

				RegisterHotkeys();

				NotifyEventHooks(hEvChanged, 0, 0);
				break;

			case IDC_LV_HOTKEYS:
				switch (lpnmhdr->code) {
				case NM_CLICK:
					{
						THotkeyItem *item = nullptr;
						LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;
						LVHITTESTINFO lvhti = { 0 };
						LVITEM lvi = { 0 };

						lvi.mask = LVIF_PARAM | LVIF_IMAGE;
						lvi.iItem = lpnmia->iItem;
						ListView_GetItem(lpnmia->hdr.hwndFrom, &lvi);
						item = (THotkeyItem *)lvi.lParam;

						lvhti.pt = lpnmia->ptAction;
						lvhti.iItem = lpnmia->iItem;
						lvhti.iSubItem = lpnmia->iSubItem;
						ListView_HitTest(lpnmia->hdr.hwndFrom, &lvhti);

						if (item &&
							(!item->rootHotkey && (lpnmia->iSubItem == COL_NAME) && ((lvhti.flags & LVHT_ONITEM) == LVHT_ONITEMICON) ||
							item->rootHotkey && (lpnmia->iSubItem == COL_TYPE)) &&
							((item->OptType == HKT_GLOBAL) || (item->OptType == HKT_LOCAL))) {
							item->OptType = (item->OptType == HKT_GLOBAL) ? HKT_LOCAL : HKT_GLOBAL;
							sttOptionsSetupItem(lpnmia->hdr.hwndFrom, lpnmia->iItem, item);
							sttOptionsSetChanged(item);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}
						else if (item && (lpnmia->iSubItem == COL_RESET)) {
							item->OptHotkey = item->Hotkey;
							sttOptionsSetupItem(lpnmia->hdr.hwndFrom, lpnmia->iItem, item);
						}
						else if (item && (lpnmia->iSubItem == COL_ADDREMOVE)) {
							if (item->rootHotkey)
								sttOptionsDeleteHotkey(lpnmia->hdr.hwndFrom, lpnmia->iItem, item);
							else {
								initialized = FALSE;
								sttOptionsAddHotkey(lpnmia->hdr.hwndFrom, item);
								initialized = TRUE;
							}
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}
					}
					break;

				case LVN_KEYDOWN:
					{
						LPNMLVKEYDOWN param = (LPNMLVKEYDOWN)lParam;
						if (param->wVKey == VK_SUBTRACT || param->wVKey == VK_LEFT || param->wVKey == VK_ADD || param->wVKey == VK_RIGHT) {
							LVITEM lvi = { 0 };
							lvi.mask = LVIF_PARAM;
							lvi.iItem = ListView_GetNextItem(lpnmhdr->hwndFrom, -1, LVNI_SELECTED);
							if (lvi.iItem < 0)
								break;

							ListView_GetItem(lpnmhdr->hwndFrom, &lvi);
							if (lvi.lParam)
								break;

							if (param->wVKey == VK_ADD || param->wVKey == VK_RIGHT) {
								ListView_SetCheckState(lpnmhdr->hwndFrom, lvi.iItem, TRUE);
							}
							else {
								ListView_SetCheckState(lpnmhdr->hwndFrom, lvi.iItem, FALSE);
							}
						}
						else if (param->wVKey == VK_F2)
							sttOptionsStartEdit(hwndDlg, hwndHotkey);
					}
					break;

				case LVN_ITEMACTIVATE:
					{
						LVITEM lvi = { 0 };
						lvi.mask = LVIF_PARAM;
						lvi.iItem = ListView_GetNextItem(lpnmhdr->hwndFrom, -1, LVNI_SELECTED);
						if (lvi.iItem < 0) break;
						ListView_GetItem(lpnmhdr->hwndFrom, &lvi);

						if (lvi.lParam)
							sttOptionsStartEdit(hwndDlg, hwndHotkey);
						else
							ListView_SetCheckState(lpnmhdr->hwndFrom, lvi.iItem, !ListView_GetCheckState(lpnmhdr->hwndFrom, lvi.iItem));
					}
					break;

				case LVN_ITEMCHANGED:
					if (initialized) {
						LPNMLISTVIEW param = (LPNMLISTVIEW)lParam;
						THotkeyItem *item = (THotkeyItem *)param->lParam;
						if (param->uNewState >> 12 == param->uOldState >> 12)
							break;

						if (item && !item->rootHotkey) {
							item->OptEnabled = ListView_GetCheckState(lpnmhdr->hwndFrom, param->iItem) ? 1 : 0;
							sttOptionsSetChanged(item);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}
						else if (!item) {
							wchar_t buf[256];
							LVITEM lvi = { 0 };
							lvi.mask = LVIF_TEXT;
							lvi.iItem = param->iItem;
							lvi.pszText = buf;
							lvi.cchTextMax = _countof(buf);
							ListView_GetItem(lpnmhdr->hwndFrom, &lvi);

							if (param->uNewState >> 12 == 1) {
								int count = ListView_GetItemCount(lpnmhdr->hwndFrom);
								LVITEM lvi2 = { 0 };
								lvi2.mask = LVIF_PARAM;
								for (lvi2.iItem = 0; lvi2.iItem < count; ++lvi2.iItem) {
									ListView_GetItem(lpnmhdr->hwndFrom, &lvi2);
									item = (THotkeyItem*)lvi2.lParam;
									if (!item) continue;
									if (!mir_wstrcmp(item->getSection(), buf)) {
										ListView_DeleteItem(lpnmhdr->hwndFrom, lvi2.iItem);
										--lvi2.iItem;
										--count;
									}
								}
							}
							else if (param->uNewState >> 12 == 2) {
								int nItems = ListView_GetItemCount(lpnmhdr->hwndFrom);
								initialized = FALSE;
								for (auto &it : hotkeys) {
									LVITEM lvi2 = { 0 };
									if (it->OptDeleted || mir_wstrcmp(buf, it->getSection()))
										continue;

									lvi2.mask = LVIF_PARAM | LVIF_INDENT;
									lvi2.iIndent = 1;
									lvi2.iItem = nItems++;
									lvi2.lParam = (LPARAM)it;
									ListView_InsertItem(lpnmhdr->hwndFrom, &lvi2);
									sttOptionsSetupItem(lpnmhdr->hwndFrom, nItems - 1, it);
								}
								ListView_SortItemsEx(lpnmhdr->hwndFrom, sttOptionsSortList, (LPARAM)lpnmhdr->hwndFrom);
								initialized = TRUE;
							}
						}
					}
					break;

				case NM_CUSTOMDRAW:
					{
						NMLVCUSTOMDRAW *param = (NMLVCUSTOMDRAW *)lParam;
						switch (param->nmcd.dwDrawStage) {
						case CDDS_PREPAINT:
						case CDDS_ITEMPREPAINT:
							SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, CDRF_NOTIFYSUBITEMDRAW);
							return TRUE;

						case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
							{
								THotkeyItem *item;
								wchar_t buf[256];
								LVITEM lvi = { 0 };
								lvi.mask = LVIF_TEXT | LVIF_PARAM;
								lvi.iItem = param->nmcd.dwItemSpec;
								lvi.pszText = buf;
								lvi.cchTextMax = _countof(buf);
								ListView_GetItem(lpnmhdr->hwndFrom, &lvi);

								item = (THotkeyItem *)lvi.lParam;
								if (!item) {
									RECT rc;
									HFONT hfnt;

									ListView_GetSubItemRect(lpnmhdr->hwndFrom, param->nmcd.dwItemSpec, param->iSubItem, LVIR_BOUNDS, &rc);
									FillRect(param->nmcd.hdc, &rc, GetSysColorBrush(param->nmcd.uItemState&CDIS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));
									SetTextColor(param->nmcd.hdc, GetSysColor(param->nmcd.uItemState&CDIS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

									if (param->iSubItem == 0) {
										rc.left += 3;
										HIMAGELIST hIml = ListView_GetImageList(hwndHotkey, LVSIL_SMALL);
										ImageList_Draw(hIml,
											ListView_GetCheckState(hwndHotkey, lvi.iItem) ? 6 : 7,
											param->nmcd.hdc, rc.left, (rc.top + rc.bottom - 16) / 2, ILD_TRANSPARENT);
										rc.left += 18;
										hfnt = (HFONT)SelectObject(param->nmcd.hdc, (HFONT)SendMessage(GetParent(hwndDlg), PSM_GETBOLDFONT, 0, 0));
										DrawText(param->nmcd.hdc, buf, -1, &rc, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
										SelectObject(param->nmcd.hdc, hfnt);
									}

									SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, CDRF_SKIPDEFAULT);
									return TRUE;
								}

								if (item->rootHotkey && (param->iSubItem == 0)) {
									RECT rc;
									ListView_GetSubItemRect(lpnmhdr->hwndFrom, param->nmcd.dwItemSpec, param->iSubItem, LVIR_BOUNDS, &rc);
									FillRect(param->nmcd.hdc, &rc, GetSysColorBrush(COLOR_WINDOW));
									SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, CDRF_SKIPDEFAULT);
									return TRUE;
								}
								break;
							}
						}
						break;
					}
					break;
				}
			}
		}
		break;

	case WM_DESTROY:
		{
			int count = ListView_GetItemCount(hwndHotkey);

			g_hwndHkOptions = nullptr;

			KillTimer(hwndDlg, 1024);

			wchar_t buf[128];
			LVITEM lvi = { 0 };
			lvi.pszText = buf;
			lvi.cchTextMax = _countof(buf);
			for (lvi.iItem = 0; lvi.iItem < count; ++lvi.iItem) {
				lvi.mask = LVIF_PARAM;
				lvi.iSubItem = 0;
				ListView_GetItem(hwndHotkey, &lvi);
				if (lvi.lParam) continue;

				lvi.mask = LVIF_TEXT;
				lvi.iSubItem = 1;
				ListView_GetItem(hwndHotkey, &lvi);

				db_set_b(0, DBMODULENAME "UI", _T2A(lvi.pszText), ListView_GetCheckState(hwndHotkey, lvi.iItem));
			}
		}
	}

	return FALSE;
}

int HotkeyOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = -180000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_HOTKEYS);
	odp.szTitle.a = LPGEN("Hotkeys");
	odp.szGroup.a = LPGEN("Customize");
	odp.pfnDlgProc = sttOptionsDlgProc;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

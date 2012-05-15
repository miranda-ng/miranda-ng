/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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
#include <m_hotkeys.h>

#define DBMODULENAME "SkinHotKeys"
#define WM_HOTKEYUNREGISTERED   (WM_USER+721)

typedef enum { HKT_GLOBAL, HKT_LOCAL, HKT_MANUAL, HKT_COUNT } THotkeyType;

typedef struct _THotkeyItem THotkeyItem;
struct _THotkeyItem
{
	THotkeyType  type;
	char        *pszService, *pszName; // pszName is valid _only_ for "root"   hotkeys
	TCHAR       *ptszSection, *ptszDescription;
	TCHAR       *ptszSection_tr, *ptszDescription_tr;
	LPARAM       lParam;
	WORD         DefHotkey, Hotkey;
	bool         Enabled;
	ATOM         idHotkey;

	THotkeyItem *rootHotkey;
	int          nSubHotkeys;
	bool         allowSubHotkeys;

	bool         OptChanged, OptDeleted, OptNew;
	WORD         OptHotkey;
	THotkeyType  OptType;
	bool         OptEnabled;

	bool         UnregisterHotkey;	// valid only during WM_APP message in options UI, used to remove unregistered hotkeys from options
};

static int sttCompareHotkeys(const THotkeyItem *p1, const THotkeyItem *p2)
{
	int res;
	if ( res = lstrcmp( p1->ptszSection_tr, p2->ptszSection_tr ))
		return res;
	if ( res = lstrcmp( p1->ptszDescription_tr, p2->ptszDescription_tr ))
		return res;
	if (!p1->rootHotkey && p2->rootHotkey)
		return -1;
	if (p1->rootHotkey && !p2->rootHotkey)
		return 1;
	return 0;
}

static LIST<THotkeyItem> hotkeys( 10, sttCompareHotkeys );

static void sttFreeHotkey(THotkeyItem *item);

static BOOL bModuleInitialized = FALSE;
static HWND g_hwndHotkeyHost = NULL;
static HWND g_hwndOptions = NULL;
static DWORD g_pid = 0;
static int g_hotkeyCount = 0;
static HANDLE hEvChanged = 0;

static LRESULT CALLBACK sttHotkeyHostWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static TCHAR *sttHokeyVkToName(WORD vkKey);
static void sttHotkeyEditCreate(HWND hwnd);
static void sttHotkeyEditDestroy(HWND hwnd);
static LRESULT CALLBACK sttHotkeyEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void sttRegisterHotkeys();
static void sttUnregisterHotkeys();

static INT_PTR svcHotkeySubclass(WPARAM wParam, LPARAM lParam);
static INT_PTR svcHotkeyUnsubclass(WPARAM wParam, LPARAM lParam);
static INT_PTR svcHotkeyRegister(WPARAM wParam, LPARAM lParam);
static INT_PTR svcHotkeyUnregister(WPARAM wParam, LPARAM lParam);
static INT_PTR svcHotkeyCheck(WPARAM wParam, LPARAM lParam);

HHOOK hhkKeyboard = NULL;
static LRESULT CALLBACK sttKeyboardProc(int code, WPARAM wParam, LPARAM lParam);

static void sttWordToModAndVk(WORD w, BYTE *mod, BYTE *vk)
{
	*mod = 0;
	if (HIBYTE(w) & HOTKEYF_CONTROL) *mod |= MOD_CONTROL;
	if (HIBYTE(w) & HOTKEYF_SHIFT)   *mod |= MOD_SHIFT;
	if (HIBYTE(w) & HOTKEYF_ALT)     *mod |= MOD_ALT;
	if (HIBYTE(w) & HOTKEYF_EXT)     *mod |= MOD_WIN;
	*vk = LOBYTE(w);
}

static LRESULT CALLBACK sttHotkeyHostWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_HOTKEY ) {
		int i;
		for (i = 0; i < hotkeys.getCount(); i++) {
			THotkeyItem *item = hotkeys[i];
			if (item->type != HKT_GLOBAL) continue;
			if (!item->Enabled) continue;
			if (item->pszService && (wParam == item->idHotkey)) {
				CallService(item->pszService, 0, item->lParam);
				break;
		}	}

		return FALSE;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK sttKeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HC_ACTION && !(HIWORD(lParam) & KF_UP)) {
		int i;
		BYTE mod=0, vk=wParam;

		if ( vk ) {
			if (GetAsyncKeyState(VK_CONTROL)) mod |= MOD_CONTROL;
			if (GetAsyncKeyState(VK_MENU)) mod |= MOD_ALT;
			if (GetAsyncKeyState(VK_SHIFT)) mod |= MOD_SHIFT;
			if (GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_RWIN)) mod |= MOD_WIN;

			for ( i = 0; i < hotkeys.getCount(); i++ ) {
				THotkeyItem *item = hotkeys[i];
				BYTE hkMod, hkVk;
				if (item->type != HKT_LOCAL) continue;
				sttWordToModAndVk(item->Hotkey, &hkMod, &hkVk);
				if (!hkVk) continue;
				if (!item->Enabled) continue;
				if (item->pszService && (vk == hkVk) && (mod == hkMod)) {
					CallService(item->pszService, 0, item->lParam);
					return TRUE;
	}	}	}	}

	return CallNextHookEx(hhkKeyboard, code, wParam, lParam);
}

static INT_PTR svcHotkeySubclass(WPARAM wParam, LPARAM)
{
	sttHotkeyEditCreate((HWND)wParam);
	return 0;
}

static INT_PTR svcHotkeyUnsubclass(WPARAM wParam, LPARAM)
{
	sttHotkeyEditDestroy((HWND)wParam);
	return 0;
}

static INT_PTR svcHotkeyRegister(WPARAM wParam, LPARAM lParam)
{
	HOTKEYDESC *desc = (HOTKEYDESC *)lParam;
	if ( desc->cbSize != sizeof(HOTKEYDESC) && desc->cbSize != HOTKEYDESC_SIZE_V1 )
		return 0;

	THotkeyItem *item = ( THotkeyItem* )mir_alloc(sizeof(THotkeyItem));
	#if defined( _UNICODE )
		DWORD dwFlags = ( desc->cbSize >= sizeof(HOTKEYDESC)) ? desc->dwFlags : 0;
		if ( dwFlags & HKD_UNICODE ) {
			item->ptszSection = mir_tstrdup( desc->ptszSection );
			item->ptszDescription = mir_tstrdup( desc->ptszDescription );
		}
		else {
			item->ptszSection = mir_a2u( desc->pszSection );
			item->ptszDescription = mir_a2u( desc->pszDescription );
		}
	#else
		item->ptszSection = mir_tstrdup( desc->pszSection );
		item->ptszDescription = mir_tstrdup( desc->pszDescription );
	#endif
	item->ptszSection_tr = TranslateTS(item->ptszSection);
	item->ptszDescription_tr = TranslateTS(item->ptszDescription);
	item->allowSubHotkeys = TRUE;
	item->rootHotkey = NULL;
	item->nSubHotkeys = 0;

	if ( item->rootHotkey = hotkeys.find( item )) {
		if (item->rootHotkey->allowSubHotkeys) {
			char nameBuf[MAXMODULELABELLENGTH];
			mir_snprintf(nameBuf, SIZEOF(nameBuf), "%s$%d", item->rootHotkey->pszName, item->rootHotkey->nSubHotkeys);
			item->pszName = mir_strdup(nameBuf);
			item->Enabled = TRUE;

			item->rootHotkey->nSubHotkeys++;
		}
		else {
			mir_free(item->ptszSection);
			mir_free(item->ptszDescription);
			mir_free(item);
			return 0;
		}
	}
	else {
		item->pszName = mir_strdup(desc->pszName);
		item->Enabled = !DBGetContactSettingByte(NULL, DBMODULENAME "Off", item->pszName, 0);
	}

	item->pszService = desc->pszService ? mir_strdup(desc->pszService) : 0;
	item->DefHotkey = desc->DefHotKey & ~HKF_MIRANDA_LOCAL;
	item->Hotkey = DBGetContactSettingWord(NULL, DBMODULENAME, item->pszName, item->DefHotkey);
	item->type = item->pszService ?
		( THotkeyType )DBGetContactSettingByte(NULL, DBMODULENAME "Types", item->pszName,
			(desc->DefHotKey & HKF_MIRANDA_LOCAL) ? HKT_LOCAL : HKT_GLOBAL) : HKT_MANUAL;
	item->lParam = desc->lParam;

	char buf[256];
	mir_snprintf(buf, SIZEOF(buf), "mir_hotkey_%d_%d", g_pid, g_hotkeyCount++);
	item->idHotkey = GlobalAddAtomA(buf);
	if (item->type == HKT_GLOBAL) {
		if (item->Enabled) {
			BYTE mod, vk;
			sttWordToModAndVk(item->Hotkey, &mod, &vk);
			if (vk) RegisterHotKey(g_hwndHotkeyHost, item->idHotkey, mod, vk);
	}	}

	hotkeys.insert( item );

	if ( !item->rootHotkey ) {
		/* try to load alternatives from db */
		int count, i;
		mir_snprintf(buf, SIZEOF(buf), "%s$count", item->pszName);
		count = (int)DBGetContactSettingDword(NULL, DBMODULENAME, buf, -1);
		for (i = 0; i < count; i++) {
			mir_snprintf(buf, SIZEOF(buf), "%s$%d", item->pszName, i);
			if (!DBGetContactSettingWord(NULL, DBMODULENAME, buf, 0))
				continue;

			svcHotkeyRegister(wParam, lParam);
		}
		item->allowSubHotkeys = count < 0;
	}
	else {
		mir_free( item->pszName );
		item->pszName = NULL;
	}

	return item->idHotkey;
}

static INT_PTR svcHotkeyUnregister(WPARAM, LPARAM lParam)
{
	int i;
	char *pszName = (char *)lParam;
	char pszNamePrefix[MAXMODULELABELLENGTH];
	size_t cbNamePrefix;
	mir_snprintf(pszNamePrefix, SIZEOF(pszNamePrefix), "%s$", pszName);
	cbNamePrefix = strlen(pszNamePrefix);

	for (i = 0; i < hotkeys.getCount(); ++i)
	{
		char *pszCurrentName = hotkeys[i]->rootHotkey ?
			hotkeys[i]->rootHotkey->pszName :
			hotkeys[i]->pszName;
		if (!pszCurrentName) continue;

		hotkeys[i]->UnregisterHotkey =
			!lstrcmpA(pszCurrentName, pszName) ||
			!strncmp(pszCurrentName, pszNamePrefix, cbNamePrefix);
	}

	if (g_hwndOptions)
		SendMessage(g_hwndOptions, WM_HOTKEYUNREGISTERED, 0, 0);

	for (i = 0; i < hotkeys.getCount(); ++i)
		if (hotkeys[i]->UnregisterHotkey) {
			sttFreeHotkey(hotkeys[i]);
			List_Remove((SortedList *)&hotkeys, i);
			--i;
		}

	return 0;
}

static INT_PTR svcHotkeyCheck(WPARAM wParam, LPARAM lParam)
{
	MSG *msg = (MSG *)wParam;
	TCHAR *pszSection = mir_a2t((char *)lParam);

	if ((msg->message == WM_KEYDOWN) || (msg->message == WM_SYSKEYDOWN)) {
		int i;
		BYTE mod=0, vk=msg->wParam;

		if (vk) {
			if (GetAsyncKeyState(VK_CONTROL)) mod |= MOD_CONTROL;
			if (GetAsyncKeyState(VK_MENU)) mod |= MOD_ALT;
			if (GetAsyncKeyState(VK_SHIFT)) mod |= MOD_SHIFT;
			if (GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_RWIN)) mod |= MOD_WIN;

			for ( i = 0; i < hotkeys.getCount(); i++ ) {
				THotkeyItem *item = hotkeys[i];
				BYTE hkMod, hkVk;
				if ((item->type != HKT_MANUAL) || lstrcmp(pszSection, item->ptszSection)) continue;
				sttWordToModAndVk(item->Hotkey, &hkMod, &hkVk);
				if (!hkVk) continue;
				if (!item->Enabled) continue;
				if ((vk == hkVk) && (mod == hkMod)) {
					mir_free(pszSection);
					return item->lParam;
	}	}	}	}

	mir_free(pszSection);
	return 0;
}

static void sttFreeHotkey(THotkeyItem *item)
{
	if ( item->type == HKT_GLOBAL && item->Enabled )
		UnregisterHotKey(g_hwndHotkeyHost, item->idHotkey);
	GlobalDeleteAtom(item->idHotkey);
	mir_free(item->pszName);
	mir_free(item->pszService);
	mir_free(item->ptszDescription);
	mir_free(item->ptszSection);
	mir_free(item);
}

static void sttRegisterHotkeys()
{
	int i;
	for ( i = 0; i < hotkeys.getCount(); i++ ) {
		THotkeyItem *item = hotkeys[i];
		UnregisterHotKey(g_hwndHotkeyHost, item->idHotkey);
		if (item->type != HKT_GLOBAL) continue;
		if (item->Enabled) {
			BYTE mod, vk;
			sttWordToModAndVk(item->Hotkey, &mod, &vk);
			if (vk) RegisterHotKey(g_hwndHotkeyHost, item->idHotkey, mod, vk);
}	}	}

static void sttUnregisterHotkeys()
{
	int i;
	for (i = 0; i < hotkeys.getCount(); i++) {
		THotkeyItem *item = hotkeys[i];
		if ( item->type == HKT_GLOBAL && item->Enabled )
			UnregisterHotKey(g_hwndHotkeyHost, item->idHotkey);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Hotkey control
typedef struct
{
	WNDPROC oldWndProc;
	BYTE shift;
	BYTE key;
}
	THotkeyBoxData;

static TCHAR *sttHokeyVkToName(WORD vkKey)
{
	static TCHAR buf[256] = {0};
	DWORD code = MapVirtualKey(vkKey, 0) << 16;

	switch (vkKey)
	{
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
		return _T("");

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

void HotkeyToName(TCHAR *buf, int size, BYTE shift, BYTE key)
{
	mir_sntprintf(buf, size, _T("%s%s%s%s%s"),
		(shift & HOTKEYF_CONTROL)	? _T("Ctrl + ")		: _T(""),
		(shift & HOTKEYF_ALT)		? _T("Alt + ")		: _T(""),
		(shift & HOTKEYF_SHIFT)		? _T("Shift + ")	: _T(""),
		(shift & HOTKEYF_EXT)		? _T("Win + ")		: _T(""),
		sttHokeyVkToName(key));
}

WORD GetHotkeyValue(INT_PTR idHotkey)
{
	for (int i = 0; i < hotkeys.getCount(); i++)
		if (hotkeys[i]->idHotkey == idHotkey)
			return hotkeys[i]->Enabled ? hotkeys[i]->Hotkey : 0;

	return 0;
}

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
			TCHAR buf[256] = {0};
			data->key = (BYTE)LOWORD(wParam);
			data->shift = (BYTE)HIWORD(wParam);
			HotkeyToName(buf, SIZEOF(buf), data->shift, data->key);
			SetWindowText(hwnd, buf);
			return 0;
		}

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
			TCHAR buf[256] = {0};

			BYTE shift = 0;
			BYTE key = wParam;
			TCHAR *name = sttHokeyVkToName(key);
			if (!*name || !bKeyDown) key = 0;

			if (GetAsyncKeyState(VK_CONTROL)) shift |= HOTKEYF_CONTROL;
			if (GetAsyncKeyState(VK_MENU)) shift |= HOTKEYF_ALT;
			if (GetAsyncKeyState(VK_SHIFT)) shift |= HOTKEYF_SHIFT;
			if (GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_RWIN)) shift |= HOTKEYF_EXT;

			if (bKeyDown || !data->key) {
				data->shift = shift;
				data->key = key;
			}

			HotkeyToName(buf, SIZEOF(buf), data->shift, data->key);
			SetWindowText(hwnd, buf);

			if (bKeyDown && data->key)
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(GetWindowLongPtr(hwnd, GWL_ID), 0), (LPARAM)hwnd);
			return TRUE;
		}

		case WM_DESTROY:
		{
			WNDPROC saveOldWndProc = data->oldWndProc;
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)data->oldWndProc);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
			mir_free(data);
			return CallWindowProc(saveOldWndProc, hwnd, msg, wParam, lParam);
	}	}

	return CallWindowProc(data->oldWndProc, hwnd, msg, wParam, lParam);
}

static void sttHotkeyEditCreate(HWND hwnd)
{
	THotkeyBoxData *data = (THotkeyBoxData *)mir_alloc(sizeof(THotkeyBoxData));
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (ULONG_PTR)data);
	data->oldWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (ULONG_PTR)sttHotkeyEditProc);
}

static void sttHotkeyEditDestroy(HWND hwnd)
{
	THotkeyBoxData *data = (THotkeyBoxData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (ULONG_PTR)data->oldWndProc);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
	mir_free(data);
}

///////////////////////////////////////////////////////////////////////////////
// Options

enum { COL_NAME, COL_TYPE, COL_KEY, COL_RESET, COL_ADDREMOVE };

static void sttOptionsSetupItem(HWND hwndList, int idx, THotkeyItem *item)
{
	TCHAR buf[256];
	LVITEM lvi = {0};
	lvi.iItem = idx;

	if ( !item->rootHotkey ) {
		lvi.mask = LVIF_TEXT|LVIF_IMAGE;
		lvi.iSubItem = COL_NAME;
		lvi.pszText = item->ptszDescription_tr;
		lvi.iImage = item->OptType;
		ListView_SetItem(hwndList, &lvi);

		ListView_SetCheckState(hwndList, lvi.iItem, item->Enabled);
	}

	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = COL_KEY;
	HotkeyToName(buf, SIZEOF(buf), HIBYTE(item->OptHotkey), LOBYTE(item->OptHotkey));
	lvi.pszText = buf;
	ListView_SetItem(hwndList, &lvi);

	if ( item->rootHotkey ) {
		lvi.mask = LVIF_IMAGE;
		lvi.iSubItem = COL_TYPE;
		lvi.iImage = item->OptType;
		ListView_SetItem(hwndList, &lvi);
	}

	lvi.mask = LVIF_IMAGE;
	lvi.iSubItem = COL_RESET;
	lvi.iImage = (item->Hotkey != item->OptHotkey) ? 5 : -1;
	ListView_SetItem(hwndList, &lvi);

	lvi.mask = LVIF_IMAGE|LVIF_TEXT;
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
	TCHAR title1[256] = {0}, title2[256] = {0};
	THotkeyItem *item1 = NULL, *item2 = NULL;
	LVITEM lvi = {0};
	int res;

	lvi.mask = LVIF_TEXT|LVIF_PARAM;
	lvi.iItem = lParam1;
	lvi.pszText = title1;
	lvi.cchTextMax = SIZEOF(title1);
	if (ListView_GetItem((HWND)lParamSort, &lvi))
		item1 = (THotkeyItem *)lvi.lParam;

	lvi.mask = LVIF_TEXT|LVIF_PARAM;
	lvi.iItem = lParam2;
	lvi.pszText = title2;
	lvi.cchTextMax = SIZEOF(title2);
	if (ListView_GetItem((HWND)lParamSort, &lvi))
		item2 = (THotkeyItem *)lvi.lParam;

	if (!item1 && !item2)
		return lstrcmp(title1, title2);

	if (!item1) {
		if (res = lstrcmp(title1, item2->ptszSection_tr))
			return res;
		return -1;
	}

	if (!item2) {
		if (res = lstrcmp(item1->ptszSection_tr, title2))
			return res;
		return 1;
	}
	return sttCompareHotkeys(item1, item2);
}

static void sttOptionsAddHotkey(HWND hwndList, THotkeyItem *item)
{
	char buf[256];
	LVITEM lvi = {0};

	THotkeyItem *newItem = (THotkeyItem *)mir_alloc(sizeof(THotkeyItem));
	newItem->pszName = NULL;
	newItem->pszService = item->pszService ? mir_strdup(item->pszService) : NULL;
	newItem->ptszSection = mir_tstrdup(item->ptszSection);
	newItem->ptszDescription = mir_tstrdup(item->ptszDescription);
	newItem->ptszSection_tr = item->ptszSection_tr;
	newItem->ptszDescription_tr = item->ptszDescription_tr;
	newItem->lParam = item->lParam;
	mir_snprintf(buf, SIZEOF(buf), "mir_hotkey_%d_%d", g_pid, g_hotkeyCount++);
	newItem->idHotkey = GlobalAddAtomA(buf);
	newItem->rootHotkey = item;
	newItem->Hotkey = newItem->DefHotkey = newItem->OptHotkey = 0;
	newItem->type = newItem->OptType = item->OptType;
	newItem->Enabled = newItem->OptEnabled = TRUE;
	newItem->OptChanged = newItem->OptDeleted = FALSE;
	newItem->OptNew = TRUE;

	hotkeys.insert( newItem );

	SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);

	lvi.mask |= LVIF_PARAM;
	lvi.lParam = (LPARAM)newItem;
	sttOptionsSetupItem(hwndList, ListView_InsertItem(hwndList, &lvi), newItem);
	ListView_SortItemsEx(hwndList, sttOptionsSortList, (LPARAM)hwndList);

	SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);
	RedrawWindow(hwndList, NULL, NULL, RDW_INVALIDATE);

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
	int i;
	char buf[MAXMODULELABELLENGTH];

	if (item->rootHotkey) return;
	if (!item->OptChanged) return;

	item->Hotkey = item->OptHotkey;
	item->type = item->OptType;
	item->Enabled = item->OptEnabled;

	DBWriteContactSettingWord(NULL, DBMODULENAME, item->pszName, item->Hotkey);
	DBWriteContactSettingByte(NULL, DBMODULENAME "Off", item->pszName, (BYTE)!item->Enabled);
	if (item->type != HKT_MANUAL)
		DBWriteContactSettingByte(NULL, DBMODULENAME "Types", item->pszName, (BYTE)item->type);

	item->nSubHotkeys = 0;
	for (i = 0; i < hotkeys.getCount(); i++) {
		THotkeyItem *subItem = hotkeys[i];
		if (subItem->rootHotkey == item) {
			subItem->Hotkey = subItem->OptHotkey;
			subItem->type = subItem->OptType;

			mir_snprintf(buf, SIZEOF(buf), "%s$%d", item->pszName, item->nSubHotkeys);
			DBWriteContactSettingWord(NULL, DBMODULENAME, buf, subItem->Hotkey);
			if (subItem->type != HKT_MANUAL)
				DBWriteContactSettingByte(NULL, DBMODULENAME "Types", buf, (BYTE)subItem->type);

			++item->nSubHotkeys;
	}	}

	mir_snprintf(buf, SIZEOF(buf), "%s$count", item->pszName);
	DBWriteContactSettingDword(NULL, DBMODULENAME, buf, item->nSubHotkeys);
}

static void sttBuildHotkeyList(HWND hwndList, TCHAR *section)
{
	int i, nItems=0;
	ListView_DeleteAllItems(hwndList);

	for (i = 0; i < hotkeys.getCount(); i++) {
		LVITEM lvi = {0};
		THotkeyItem *item = hotkeys[i];

		if (item->OptDeleted) continue;
		if (section && lstrcmp(section, item->ptszSection)) continue;

		if ( !section && (!i || lstrcmp(item->ptszSection, ((THotkeyItem *)hotkeys[i-1])->ptszSection ))) {
			lvi.mask = LVIF_TEXT|LVIF_PARAM;
			lvi.iItem = nItems++;
			lvi.iSubItem = 0;
			lvi.lParam = 0;
			lvi.pszText = item->ptszSection_tr;
			ListView_InsertItem(hwndList, &lvi);
			ListView_SetCheckState(hwndList, lvi.iItem, TRUE);

			lvi.mask = LVIF_TEXT;
			lvi.iSubItem = 1;
			lvi.pszText = item->ptszSection;
			ListView_SetItem(hwndList, &lvi);

			lvi.iSubItem = 0;
		}

		lvi.mask = LVIF_PARAM;
		if (!section) {
			lvi.mask |= LVIF_INDENT;
			lvi.iIndent = 1;
		}
		lvi.iItem = nItems++;
		lvi.lParam = (LPARAM)item;
		ListView_InsertItem(hwndList, &lvi);
		sttOptionsSetupItem(hwndList, nItems-1, item);
	}
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

		SetWindowPos(hwndHotkey, HWND_BOTTOM, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
		SetWindowPos(GetDlgItem(hwndDlg, IDC_HOTKEY), HWND_TOP, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SWP_SHOWWINDOW);
		RedrawWindow(GetDlgItem(hwndDlg, IDC_HOTKEY), NULL, NULL, RDW_INVALIDATE);

		SetFocus(GetDlgItem(hwndDlg, IDC_HOTKEY));
		RedrawWindow(GetDlgItem(hwndDlg, IDC_HOTKEY), NULL, NULL, RDW_INVALIDATE);
	}
}

static void sttOptionsDrawTextChunk(HDC hdc, TCHAR *text, RECT *rc)
{
	SIZE sz;
	DrawText(hdc, text, lstrlen(text), rc, DT_LEFT|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER|DT_WORD_ELLIPSIS);
	GetTextExtentPoint32(hdc, text, lstrlen(text), &sz);
	rc->left += sz.cx;
}

static INT_PTR CALLBACK sttOptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL initialized = FALSE;
	static int colWidth = 0;
	static WORD currentLanguage = 0;

	HWND hwndHotkey = GetDlgItem(hwndDlg, IDC_LV_HOTKEYS);

	switch (msg) {
	case WM_INITDIALOG:
	{
		int i;
		LVCOLUMN lvc;
		RECT rc;
		HIMAGELIST hIml;

		initialized = FALSE;

		TranslateDialogDefault(hwndDlg);

		sttHotkeyEditCreate(GetDlgItem(hwndDlg, IDC_HOTKEY));

		hIml = ImageList_Create(16, 16, ILC_MASK + (IsWinVerXPPlus() ? ILC_COLOR32 : ILC_COLOR16), 3, 1);
		ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_WINDOWS);
		ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_MIRANDA); 
		ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_WINDOW);
		ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_ADDCONTACT);
		ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_DELETE);
		ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_UNDO);

		// This is added to use for drawing operation only
		ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_GROUPOPEN);
		ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_GROUPSHUT);

		ListView_SetImageList(hwndHotkey, hIml, LVSIL_SMALL);

		ListView_SetExtendedListViewStyle(hwndHotkey, LVS_EX_CHECKBOXES|LVS_EX_SUBITEMIMAGES|LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER|LVS_EX_INFOTIP);

		GetClientRect(hwndHotkey, &rc);
		colWidth = rc.right - GetSystemMetrics(SM_CXHTHUMB) - 3*GetSystemMetrics(SM_CXSMICON) - 5;

		lvc.mask = LVCF_WIDTH;
		lvc.cx = colWidth * 2 / 3;
		ListView_InsertColumn(hwndHotkey, COL_NAME, &lvc);
		lvc.cx = GetSystemMetrics(SM_CXSMICON);
		ListView_InsertColumn(hwndHotkey, COL_TYPE, &lvc);
		lvc.cx = colWidth / 3;
		ListView_InsertColumn(hwndHotkey, COL_KEY, &lvc);
		lvc.cx = GetSystemMetrics(SM_CXSMICON);
		ListView_InsertColumn(hwndHotkey, COL_RESET, &lvc);
		lvc.cx = GetSystemMetrics(SM_CXSMICON);
		ListView_InsertColumn(hwndHotkey, COL_ADDREMOVE, &lvc);

		for (i = 0; i < hotkeys.getCount(); i++) {
			THotkeyItem *item = hotkeys[i];

			item->OptChanged = FALSE;
			item->OptDeleted = item->OptNew = FALSE;
			item->OptEnabled = item->Enabled;
			item->OptHotkey = item->Hotkey;
			item->OptType = item->type;
		}

		currentLanguage = LOWORD(GetKeyboardLayout(0));
		sttBuildHotkeyList(hwndHotkey, NULL);
		SetTimer(hwndDlg, 1024, 1000, NULL);

		initialized = TRUE;

		{	/* load group states */
			int count = ListView_GetItemCount(hwndHotkey);
			TCHAR buf[128];
			LVITEM lvi = {0};
			lvi.pszText = buf;
			lvi.cchTextMax = SIZEOF(buf);
			for (lvi.iItem = 0; lvi.iItem < count; ++lvi.iItem) {
				char *szSetting;

				lvi.mask = LVIF_PARAM;
				lvi.iSubItem = 0;
				ListView_GetItem(hwndHotkey, &lvi);
				if (lvi.lParam) continue;

				lvi.mask = LVIF_TEXT;
				lvi.iSubItem = 1;
				ListView_GetItem(hwndHotkey, &lvi);

				szSetting = mir_t2a(lvi.pszText);

				ListView_SetCheckState(hwndHotkey, lvi.iItem,
					DBGetContactSettingByte(NULL, DBMODULENAME "UI", szSetting, TRUE));

				mir_free(szSetting);
			}
		}

		g_hwndOptions = hwndDlg;

		break;
	}

	case WM_DESTROY:
	{
		int count = ListView_GetItemCount(hwndHotkey);
		TCHAR buf[128];
		LVITEM lvi = {0};

		g_hwndOptions = NULL;

		KillTimer(hwndDlg, 1024);

		lvi.pszText = buf;
		lvi.cchTextMax = SIZEOF(buf);
		for (lvi.iItem = 0; lvi.iItem < count; ++lvi.iItem) {
			char *szSetting;

			lvi.mask = LVIF_PARAM;
			lvi.iSubItem = 0;
			ListView_GetItem(hwndHotkey, &lvi);
			if (lvi.lParam) continue;

			lvi.mask = LVIF_TEXT;
			lvi.iSubItem = 1;
			ListView_GetItem(hwndHotkey, &lvi);

			szSetting = mir_t2a(lvi.pszText);

			DBWriteContactSettingByte(NULL, DBMODULENAME "UI", szSetting,
				(BYTE) ListView_GetCheckState(hwndHotkey, lvi.iItem));

			mir_free(szSetting);
		}
		break;
	}

	case WM_TIMER:
	{
		WORD newLanguage;
		int count;
		LVITEM lvi = {0};

		if (!initialized) break;

		newLanguage = LOWORD(GetKeyboardLayout(0));
		if (newLanguage == currentLanguage) break;

		count = ListView_GetItemCount(hwndHotkey);
		lvi.mask = LVIF_PARAM;
		for (lvi.iItem = 0; lvi.iItem < count; ++lvi.iItem) {
			ListView_GetItem(hwndHotkey, &lvi);
			if (!lvi.lParam) continue;

			sttOptionsSetupItem(hwndHotkey, lvi.iItem, (THotkeyItem *)lvi.lParam);
		}

		currentLanguage = newLanguage;
		break;
	}

	case WM_HOTKEYUNREGISTERED:
	{
		int count;
		LVITEM lvi = {0};

		count = ListView_GetItemCount(hwndHotkey);
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
		break;
	}

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

			rc.left = prefix + width * 0;
			ImageList_Draw(hIml, 0, lpdis->hDC, rc.left, (rc.top+rc.bottom-16)/2, ILD_TRANSPARENT);
			rc.left += 20;
			sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("System"), &rc);

			rc.left = prefix + width * 1;
			ImageList_Draw(hIml, 1, lpdis->hDC, rc.left, (rc.top+rc.bottom-16)/2, ILD_TRANSPARENT);
			rc.left += 20;
			sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("Miranda"), &rc);

			rc.left = prefix + width * 2;
			ImageList_Draw(hIml, 2, lpdis->hDC, rc.left, (rc.top+rc.bottom-16)/2, ILD_TRANSPARENT);
			rc.left += 20;
			sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("Window"), &rc);

			return TRUE;
		}
		
		if (lpdis->CtlID == IDC_CANVAS) {
			sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("Actions:"), &rc);
			rc.left += 10;

			rc.left = prefix + width * 0;
			ImageList_Draw(hIml, 5, lpdis->hDC, rc.left, (rc.top+rc.bottom-16)/2, ILD_TRANSPARENT);
			rc.left += 20;
			sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("Undo"), &rc);

			rc.left = prefix + width * 1;
			ImageList_Draw(hIml, 3, lpdis->hDC, rc.left, (rc.top+rc.bottom-16)/2, ILD_TRANSPARENT);
			rc.left += 20;
			sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("Add binding"), &rc);

			rc.left = prefix + width * 2;
			ImageList_Draw(hIml, 4, lpdis->hDC, rc.left, (rc.top+rc.bottom-16)/2, ILD_TRANSPARENT);
			rc.left += 20;
			sttOptionsDrawTextChunk(lpdis->hDC, TranslateT("Remove"), &rc);

			return TRUE;
		}

		break;
	}

	case WM_COMMAND:
		if (( LOWORD( wParam ) == IDC_HOTKEY) && (( HIWORD( wParam ) == EN_KILLFOCUS) || (HIWORD(wParam) == 0 ))) {
			LVITEM lvi;
			THotkeyItem *item;
			WORD wHotkey = (WORD)SendDlgItemMessage(hwndDlg, IDC_HOTKEY, HKM_GETHOTKEY, 0, 0);

			ShowWindow(GetDlgItem(hwndDlg, IDC_HOTKEY), SW_HIDE);
			SetFocus(hwndHotkey);
			if ( !wHotkey || (wHotkey == VK_ESCAPE) || (HIWORD(wParam) != 0 ))
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
		}	}	}
		break;

	case WM_CONTEXTMENU:
		if (GetWindowLongPtr((HWND)wParam, GWL_ID) == IDC_LV_HOTKEYS)
		{
			HWND hwndList = (HWND)wParam;
			POINT pt = { (signed short)LOWORD( lParam ), (signed short)HIWORD( lParam ) };
			LVITEM lvi = {0};
			THotkeyItem *item = NULL;

			lvi.iItem = ListView_GetNextItem(hwndHotkey, -1, LVNI_SELECTED);
			if (lvi.iItem < 0) return FALSE;

			lvi.mask = LVIF_PARAM;
			ListView_GetItem(hwndList, &lvi);
			if (!(item = (THotkeyItem *)lvi.lParam)) return FALSE;

			if (( pt.x == -1 ) && ( pt.y == -1 )) {
				RECT rc;
				ListView_GetItemRect(hwndList, lvi.iItem, &rc, LVIR_LABEL);
				pt.x = rc.left;
				pt.y = rc.bottom;
				ClientToScreen(hwndList, &pt);
			}

			{
				enum { MI_CANCEL, MI_CHANGE, MI_SYSTEM, MI_LOCAL, MI_ADD, MI_REMOVE, MI_REVERT };

				MENUITEMINFO mii = {0};
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_STATE;
				mii.fState = MFS_DEFAULT;

				HMENU hMenu = CreatePopupMenu();
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)MI_CHANGE, TranslateT("Modify"));
				SetMenuItemInfo(hMenu, (UINT_PTR)MI_CHANGE, FALSE, &mii);
				if (item->type != HKT_MANUAL) {
					AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
					AppendMenu(hMenu, MF_STRING|
						((item->OptType == HKT_GLOBAL) ? MF_CHECKED : 0),
						(UINT_PTR)MI_SYSTEM, TranslateT("System scope"));
					AppendMenu(hMenu, MF_STRING|
						((item->OptType == HKT_LOCAL) ? MF_CHECKED : 0),
						(UINT_PTR)MI_LOCAL, TranslateT("Miranda scope"));
				}
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				if (!item->rootHotkey)
					AppendMenu(hMenu, MF_STRING, (UINT_PTR)MI_ADD, TranslateT("Add binding"));
				else
					AppendMenu(hMenu, MF_STRING, (UINT_PTR)MI_REMOVE, TranslateT("Remove"));
				if (item->Hotkey != item->OptHotkey) {
					AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
					AppendMenu(hMenu, MF_STRING, (UINT_PTR)MI_REVERT, TranslateT("Undo"));
				}

				switch (TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL)) {
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
				DestroyMenu( hMenu );
			}

			break;
		}
		break;

	case WM_NOTIFY:
	{
		LPNMHDR lpnmhdr = (LPNMHDR)lParam;
		switch (lpnmhdr->idFrom) {
		case 0:
			{
				int i;

				if (( lpnmhdr->code != PSN_APPLY) && (lpnmhdr->code != PSN_RESET ))
					break;

				sttUnregisterHotkeys();

				for (i = 0; i < hotkeys.getCount(); i++) {
					THotkeyItem *item = hotkeys[i];
					if (item->OptNew && item->OptDeleted ||
						item->rootHotkey && !item->OptHotkey ||
						(lpnmhdr->code == PSN_APPLY) && item->OptDeleted ||
						(lpnmhdr->code == PSN_RESET) && item->OptNew)
					{
						sttFreeHotkey(item);
						hotkeys.remove( i-- );
					}
				}

				if (lpnmhdr->code == PSN_APPLY) {
					LVITEM lvi = {0};
					int count = ListView_GetItemCount(hwndHotkey);

					for (i = 0; i < hotkeys.getCount(); i++)
						sttOptionsSaveItem(hotkeys[i]);

					lvi.mask = LVIF_IMAGE;
					lvi.iSubItem = COL_RESET;
					lvi.iImage = -1;
					for (lvi.iItem = 0; lvi.iItem < count; ++lvi.iItem)
						ListView_SetItem(hwndHotkey, &lvi);
				}

				sttRegisterHotkeys();

				NotifyEventHooks( hEvChanged, 0, 0 );
				break;
			}
		case IDC_LV_HOTKEYS:
			switch (lpnmhdr->code) {
			case NM_CLICK:
				{
					THotkeyItem *item = NULL;
					LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;
					LVHITTESTINFO lvhti = {0};
					LVITEM lvi = {0};

					lvi.mask = LVIF_PARAM|LVIF_IMAGE;
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
						((item->OptType == HKT_GLOBAL) || (item->OptType == HKT_LOCAL)))
					{
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
					break;
				}
			case LVN_KEYDOWN:
				{
					LPNMLVKEYDOWN param = (LPNMLVKEYDOWN)lParam;
					if ((param->wVKey == VK_SUBTRACT) || (param->wVKey == VK_LEFT) ||
						(param->wVKey == VK_ADD) || (param->wVKey == VK_RIGHT))
					{
						LVITEM lvi = {0};
						lvi.mask = LVIF_PARAM;
						lvi.iItem = ListView_GetNextItem(lpnmhdr->hwndFrom, -1, LVNI_SELECTED);
						if (lvi.iItem < 0) break;
						ListView_GetItem(lpnmhdr->hwndFrom, &lvi);
						if (lvi.lParam) break;

						if ((param->wVKey == VK_ADD) || (param->wVKey == VK_RIGHT))
						{
							ListView_SetCheckState(lpnmhdr->hwndFrom, lvi.iItem, TRUE);
						} else
						// if ((param->wVKey == VK_SUBTRACT) || (param->wVKey == VK_LEFT))
						{
							ListView_SetCheckState(lpnmhdr->hwndFrom, lvi.iItem, FALSE);
						}
					} 
					else if (param->wVKey == VK_F2)
						sttOptionsStartEdit(hwndDlg, hwndHotkey);

					break;
				}
			case LVN_ITEMACTIVATE:
				{
					LVITEM lvi = {0};
					lvi.mask = LVIF_PARAM;
					lvi.iItem = ListView_GetNextItem(lpnmhdr->hwndFrom, -1, LVNI_SELECTED);
					if (lvi.iItem < 0) break;
					ListView_GetItem(lpnmhdr->hwndFrom, &lvi);

					if (lvi.lParam)
						sttOptionsStartEdit(hwndDlg, hwndHotkey);
					else
						ListView_SetCheckState(lpnmhdr->hwndFrom, lvi.iItem, !ListView_GetCheckState(lpnmhdr->hwndFrom, lvi.iItem));
					break;
				}
			case LVN_ITEMCHANGED:
				{
					LPNMLISTVIEW param = (LPNMLISTVIEW)lParam;
					THotkeyItem *item = (THotkeyItem *)param->lParam;
					if (!initialized || (param->uNewState>>12 == param->uOldState>>12))
						break;

					if (item && !item->rootHotkey) {
						item->OptEnabled = ListView_GetCheckState(lpnmhdr->hwndFrom, param->iItem) ? 1 : 0;
						sttOptionsSetChanged(item);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
					else if (!item) {
						TCHAR buf[256];
						LVITEM lvi = {0};
						lvi.mask = LVIF_TEXT;
						lvi.iItem = param->iItem;
						lvi.pszText = buf;
						lvi.cchTextMax = SIZEOF(buf);
						ListView_GetItem(lpnmhdr->hwndFrom, &lvi);

						if (param->uNewState>>12 == 1) {
							int count = ListView_GetItemCount(lpnmhdr->hwndFrom);
							LVITEM lvi = {0};
							lvi.mask = LVIF_PARAM;
							for (lvi.iItem = 0; lvi.iItem < count; ++lvi.iItem) {
								THotkeyItem *item;
								ListView_GetItem(lpnmhdr->hwndFrom, &lvi);
								item = (THotkeyItem *)lvi.lParam;
								if (!item) continue;
								if (!lstrcmp(item->ptszSection_tr, buf)) {
									ListView_DeleteItem(lpnmhdr->hwndFrom, lvi.iItem);
									--lvi.iItem;
									--count;
							}	}
						} 
						else if (param->uNewState>>12 == 2) {
							int i, nItems = ListView_GetItemCount(lpnmhdr->hwndFrom);
							initialized = FALSE;
							for (i = 0; i < hotkeys.getCount(); ++i) {
								LVITEM lvi = {0};
								THotkeyItem *item = hotkeys[i];

								if (item->OptDeleted) continue;
								if (lstrcmp(buf, item->ptszSection_tr)) continue;

								lvi.mask = LVIF_PARAM|LVIF_INDENT;
								lvi.iIndent = 1;
								lvi.iItem = nItems++;
								lvi.lParam = (LPARAM)item;
								ListView_InsertItem(lpnmhdr->hwndFrom, &lvi);
								sttOptionsSetupItem(lpnmhdr->hwndFrom, nItems-1, item);
							}
							ListView_SortItemsEx(lpnmhdr->hwndFrom, sttOptionsSortList, (LPARAM)lpnmhdr->hwndFrom);
							initialized = TRUE;
						}
					}
					break;
				}
			case NM_CUSTOMDRAW:
				{
					NMLVCUSTOMDRAW *param = (NMLVCUSTOMDRAW *) lParam;
					switch (param->nmcd.dwDrawStage) {
					case CDDS_PREPAINT:
					case CDDS_ITEMPREPAINT:
						SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, CDRF_NOTIFYSUBITEMDRAW );
						return TRUE;

					case CDDS_SUBITEM|CDDS_ITEMPREPAINT:
						{
							THotkeyItem *item;
							TCHAR buf[256];
							LVITEM lvi = {0};
							lvi.mask = LVIF_TEXT|LVIF_PARAM;
							lvi.iItem = param->nmcd.dwItemSpec;
							lvi.pszText = buf;
							lvi.cchTextMax = SIZEOF(buf);
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
										param->nmcd.hdc, rc.left, (rc.top+rc.bottom-16)/2, ILD_TRANSPARENT);
									rc.left += 18;
									hfnt = ( HFONT )SelectObject(param->nmcd.hdc, (HFONT)SendMessage(GetParent(hwndDlg), PSM_GETBOLDFONT, 0, 0));
									DrawText(param->nmcd.hdc, buf, -1, &rc, DT_LEFT|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER);
									SelectObject(param->nmcd.hdc, hfnt);
								}

								SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, CDRF_SKIPDEFAULT );
								return TRUE;
							}
							
							if (item->rootHotkey && (param->iSubItem == 0)) {
								RECT rc;
								ListView_GetSubItemRect(lpnmhdr->hwndFrom, param->nmcd.dwItemSpec, param->iSubItem, LVIR_BOUNDS, &rc);
								FillRect(param->nmcd.hdc, &rc, GetSysColorBrush(COLOR_WINDOW));
								SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, CDRF_SKIPDEFAULT );
								return TRUE;
							}
							break;
					}	}
					break;
				}
				break;
		}	}
		break;
	} /* case WM_NOTIFY */
	} /* switch */

	return FALSE;
}

static int sttOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.hInstance = hMirandaInst;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.position = -180000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_HOTKEYS);
	odp.ptszTitle = TranslateT("Hotkeys");
	odp.ptszGroup = TranslateT("Customize");
	odp.pfnDlgProc = sttOptionsDlgProc;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	return 0;
}

static int sttModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_OPT_INITIALISE, sttOptionsInit);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Hotkey manager

static const char* oldSettings[] = { "ShowHide", "ReadMsg", "NetSearch", "ShowOptions" };
static const char* newSettings[] = { "ShowHide", "ReadMessage", "SearchInWeb", "ShowOptions" };

int LoadSkinHotkeys(void)
{
	WNDCLASSEX wcl = {0};

	bModuleInitialized = TRUE;

	wcl.cbSize = sizeof(wcl);
	wcl.lpfnWndProc = sttHotkeyHostWndProc;
	wcl.style = 0;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hInstance = hMirandaInst;
	wcl.hIcon = NULL;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszMenuName = NULL;
	wcl.lpszClassName = _T("MirandaHotkeyHostWnd");
	wcl.hIconSm = NULL;
	RegisterClassEx(&wcl);

	g_pid = GetCurrentProcessId();

	g_hwndHotkeyHost = CreateWindow(_T("MirandaHotkeyHostWnd"), NULL, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, hMirandaInst, NULL);
	SetWindowPos(g_hwndHotkeyHost, 0, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_DEFERERASE|SWP_NOSENDCHANGING|SWP_HIDEWINDOW);

	hhkKeyboard = SetWindowsHookEx(WH_KEYBOARD, sttKeyboardProc, NULL, GetCurrentThreadId());

	hEvChanged = CreateHookableEvent(ME_HOTKEYS_CHANGED);

	CreateServiceFunction(MS_HOTKEY_SUBCLASS, svcHotkeySubclass);
	CreateServiceFunction(MS_HOTKEY_UNSUBCLASS, svcHotkeyUnsubclass);
	CreateServiceFunction(MS_HOTKEY_REGISTER, svcHotkeyRegister);
	CreateServiceFunction(MS_HOTKEY_UNREGISTER, svcHotkeyUnregister);
	CreateServiceFunction(MS_HOTKEY_CHECK, svcHotkeyCheck);

	HookEvent(ME_SYSTEM_MODULESLOADED, sttModulesLoaded);
	{
		WORD key;
		int i;
		for ( i = 0; i < SIZEOF( oldSettings ); i++ ) {
			char szSetting[ 100 ];
			mir_snprintf( szSetting, SIZEOF(szSetting), "HK%s", oldSettings[i] );
			if (( key = DBGetContactSettingWord( NULL, "Clist", szSetting, 0 ))) {
				DBDeleteContactSetting( NULL, "Clist", szSetting );
				DBWriteContactSettingWord( NULL, DBMODULENAME, newSettings[i], key );
			}

			mir_snprintf( szSetting, SIZEOF(szSetting), "HKEn%s", oldSettings[i] );
			if (( key = DBGetContactSettingByte( NULL, "Clist", szSetting, 0 ))) {
				DBDeleteContactSetting( NULL, "Clist", szSetting );
				DBWriteContactSettingByte( NULL, DBMODULENAME "Off", newSettings[i], (BYTE)(key == 0) );
	}	}	}

	return 0;
}

void UnloadSkinHotkeys(void)
{
	int i;

	if ( !bModuleInitialized ) return;

	DestroyHookableEvent(hEvChanged);
	UnhookWindowsHookEx(hhkKeyboard);
	sttUnregisterHotkeys();
	DestroyWindow(g_hwndHotkeyHost);
	for ( i = 0; i < hotkeys.getCount(); i++ )
		sttFreeHotkey(hotkeys[i]);
	hotkeys.destroy();
}

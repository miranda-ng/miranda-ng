/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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
#include "..\..\core\commonheaders.h"

#include <m_hotkeys.h>
#include "skin.h"

static int sttCompareHotkeys(const THotkeyItem *p1, const THotkeyItem *p2)
{
	int res;
	if (res = mir_tstrcmp(p1->ptszSection, p2->ptszSection))
		return res;
	if (res = mir_tstrcmp(p1->ptszDescription, p2->ptszDescription))
		return res;
	if (!p1->rootHotkey && p2->rootHotkey)
		return -1;
	if (p1->rootHotkey && !p2->rootHotkey)
		return 1;
	return 0;
}

LIST<THotkeyItem> hotkeys(10, sttCompareHotkeys);
DWORD g_pid = 0, g_hkid = 1;
HWND g_hwndHotkeyHost = NULL, g_hwndHkOptions = NULL;
HANDLE hEvChanged = 0;

static BOOL bModuleInitialized = FALSE;
static HHOOK hhkKeyboard = NULL;

WORD GetHotkeyValue(INT_PTR idHotkey)
{
	for (int i=0; i < hotkeys.getCount(); i++)
		if (hotkeys[i]->idHotkey == idHotkey)
			return hotkeys[i]->Enabled ? hotkeys[i]->Hotkey : 0;

	return 0;
}

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
	if (msg == WM_HOTKEY) {
		for (int i=0; i < hotkeys.getCount(); i++) {
			THotkeyItem *item = hotkeys[i];
			if (item->type != HKT_GLOBAL || !item->Enabled)
				continue;

			if (item->pszService && (wParam == item->idHotkey)) {
				CallService(item->pszService, 0, item->lParam);
				break;
			}
		}

		return FALSE;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK sttKeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HC_ACTION && !(HIWORD(lParam) & KF_UP)) {
		BYTE mod = 0, vk = wParam;

		if (vk) {
			if (GetAsyncKeyState(VK_CONTROL)) mod |= MOD_CONTROL;
			if (GetAsyncKeyState(VK_MENU)) mod |= MOD_ALT;
			if (GetAsyncKeyState(VK_SHIFT)) mod |= MOD_SHIFT;
			if (GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_RWIN)) mod |= MOD_WIN;

			for (int i=0; i < hotkeys.getCount(); i++) {
				THotkeyItem *item = hotkeys[i];
				if (item->type != HKT_LOCAL || !item->Enabled)
					continue;

				BYTE hkMod, hkVk;
				sttWordToModAndVk(item->Hotkey, &hkMod, &hkVk);
				if (!hkVk) continue;
				if (item->pszService && vk == hkVk && mod == hkMod) {
					CallService(item->pszService, 0, item->lParam);
					return TRUE;
	}	}	}	}

	return CallNextHookEx(hhkKeyboard, code, wParam, lParam);
}

static INT_PTR svcHotkeySubclass(WPARAM wParam, LPARAM)
{
	HotkeyEditCreate((HWND)wParam);
	return 0;
}

static INT_PTR svcHotkeyUnsubclass(WPARAM wParam, LPARAM)
{
	HotkeyEditDestroy((HWND)wParam);
	return 0;
}

static INT_PTR svcHotkeyRegister(WPARAM wParam, LPARAM lParam)
{
	HOTKEYDESC *desc = (HOTKEYDESC *)lParam;
	if (desc->cbSize != sizeof(HOTKEYDESC))
		return 0;

	THotkeyItem *item = (THotkeyItem*)mir_alloc(sizeof(THotkeyItem));
	DWORD dwFlags = (desc->cbSize >= sizeof(HOTKEYDESC)) ? desc->dwFlags : 0;
	if (dwFlags & HKD_UNICODE) {
		item->ptszSection = mir_tstrdup(desc->ptszSection);
		item->ptszDescription = mir_tstrdup(desc->ptszDescription);
	}
	else {
		item->ptszSection = mir_a2u(desc->pszSection);
		item->ptszDescription = mir_a2u(desc->pszDescription);
	}

	item->hLangpack = (int)wParam;
	item->allowSubHotkeys = TRUE;
	item->rootHotkey = NULL;
	item->nSubHotkeys = 0;

	if (item->rootHotkey = hotkeys.find(item)) {
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
		item->Enabled = !db_get_b(NULL, DBMODULENAME "Off", item->pszName, 0);
	}

	item->pszService = desc->pszService ? mir_strdup(desc->pszService) : 0;
	item->DefHotkey = desc->DefHotKey & ~HKF_MIRANDA_LOCAL;
	item->Hotkey = db_get_w(NULL, DBMODULENAME, item->pszName, item->DefHotkey);
	item->type = item->pszService ?
		(THotkeyType)db_get_b(NULL, DBMODULENAME "Types", item->pszName,
			(desc->DefHotKey & HKF_MIRANDA_LOCAL) ? HKT_LOCAL : HKT_GLOBAL) : HKT_MANUAL;
	item->lParam = desc->lParam;

	char buf[256];
	mir_snprintf(buf, SIZEOF(buf), "mir_hotkey_%d_%d", g_pid, g_hkid++);
	item->idHotkey = GlobalAddAtomA(buf);
	if (item->type == HKT_GLOBAL) {
		if (item->Enabled) {
			BYTE mod, vk;
			sttWordToModAndVk(item->Hotkey, &mod, &vk);
			if (vk) RegisterHotKey(g_hwndHotkeyHost, item->idHotkey, mod, vk);
	}	}

	hotkeys.insert(item);

	if (!item->rootHotkey) {
		/* try to load alternatives from db */
		int count, i;
		mir_snprintf(buf, SIZEOF(buf), "%s$count", item->pszName);
		count = (int)db_get_dw(NULL, DBMODULENAME, buf, -1);
		for (i=0; i < count; i++) {
			mir_snprintf(buf, SIZEOF(buf), "%s$%d", item->pszName, i);
			if (!db_get_w(NULL, DBMODULENAME, buf, 0))
				continue;

			svcHotkeyRegister(wParam, lParam);
		}
		item->allowSubHotkeys = count < 0;
	}
	else {
		mir_free(item->pszName);
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

	for (i=0; i < hotkeys.getCount(); i++)
	{
		char *pszCurrentName = hotkeys[i]->rootHotkey ?
			hotkeys[i]->rootHotkey->pszName :
			hotkeys[i]->pszName;
		if (!pszCurrentName) continue;

		hotkeys[i]->UnregisterHotkey =
			!mir_strcmp(pszCurrentName, pszName)  ||
			!strncmp(pszCurrentName, pszNamePrefix, cbNamePrefix);
	}

	if (g_hwndHkOptions)
		SendMessage(g_hwndHkOptions, WM_HOTKEYUNREGISTERED, 0, 0);

	for (i=0; i < hotkeys.getCount(); i++)
		if (hotkeys[i]->UnregisterHotkey) {
			FreeHotkey(hotkeys[i]);
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
		BYTE mod = 0, vk = msg->wParam;

		if (vk) {
			if (GetAsyncKeyState(VK_CONTROL)) mod |= MOD_CONTROL;
			if (GetAsyncKeyState(VK_MENU)) mod |= MOD_ALT;
			if (GetAsyncKeyState(VK_SHIFT)) mod |= MOD_SHIFT;
			if (GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_RWIN)) mod |= MOD_WIN;

			for (i=0; i < hotkeys.getCount(); i++) {
				THotkeyItem *item = hotkeys[i];
				BYTE hkMod, hkVk;
				if ((item->type != HKT_MANUAL) || mir_tstrcmp(pszSection, item->ptszSection)) continue;
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

void FreeHotkey(THotkeyItem *item)
{
	if (item->type == HKT_GLOBAL && item->Enabled)
		UnregisterHotKey(g_hwndHotkeyHost, item->idHotkey);
	GlobalDeleteAtom(item->idHotkey);
	mir_free(item->pszName);
	mir_free(item->pszService);
	mir_free(item->ptszDescription);
	mir_free(item->ptszSection);
	mir_free(item);
}

void RegisterHotkeys()
{
	for (int i=0; i < hotkeys.getCount(); i++) {
		THotkeyItem *item = hotkeys[i];
		UnregisterHotKey(g_hwndHotkeyHost, item->idHotkey);
		if (item->type != HKT_GLOBAL) continue;
		if (item->Enabled) {
			BYTE mod, vk;
			sttWordToModAndVk(item->Hotkey, &mod, &vk);
			if (vk) RegisterHotKey(g_hwndHotkeyHost, item->idHotkey, mod, vk);
}	}	}

void KillModuleHotkeys(int hLangpack)
{
	for (int i = hotkeys.getCount()-1; i >= 0; i--) {
		THotkeyItem *item = hotkeys[i];
		if (item->hLangpack == hLangpack) {
			FreeHotkey(item);
			hotkeys.remove(i);
		}
	}
}

void UnregisterHotkeys()
{
	for (int i=0; i < hotkeys.getCount(); i++) {
		THotkeyItem *item = hotkeys[i];
		if (item->type == HKT_GLOBAL && item->Enabled)
			UnregisterHotKey(g_hwndHotkeyHost, item->idHotkey);
	}
}

static int sttModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_OPT_INITIALISE, HotkeyOptionsInit);
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
	wcl.hInstance = hInst;
	wcl.hIcon = NULL;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszMenuName = NULL;
	wcl.lpszClassName = _T("MirandaHotkeyHostWnd");
	wcl.hIconSm = NULL;
	RegisterClassEx(&wcl);

	g_pid = GetCurrentProcessId();

	g_hwndHotkeyHost = CreateWindow(_T("MirandaHotkeyHostWnd"), NULL, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, hInst, NULL);
	SetWindowPos(g_hwndHotkeyHost, 0, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_DEFERERASE|SWP_NOSENDCHANGING|SWP_HIDEWINDOW);

	hhkKeyboard = SetWindowsHookEx(WH_KEYBOARD, sttKeyboardProc, NULL, hMainThreadId);

	hEvChanged = CreateHookableEvent(ME_HOTKEYS_CHANGED);

	CreateServiceFunction("CoreHotkeys/Register", svcHotkeyRegister);
	CreateServiceFunction(MS_HOTKEY_UNREGISTER, svcHotkeyUnregister);
	CreateServiceFunction(MS_HOTKEY_SUBCLASS, svcHotkeySubclass);
	CreateServiceFunction(MS_HOTKEY_UNSUBCLASS, svcHotkeyUnsubclass);
	CreateServiceFunction(MS_HOTKEY_CHECK, svcHotkeyCheck);

	HookEvent(ME_SYSTEM_MODULESLOADED, sttModulesLoaded);

	for (int i=0; i < SIZEOF(oldSettings); i++) {
		char szSetting[ 100 ];
		mir_snprintf(szSetting, SIZEOF(szSetting), "HK%s", oldSettings[i]);

		WORD key;
		if ((key = db_get_w(NULL, "Clist", szSetting, 0))) {
			db_unset(NULL, "Clist", szSetting);
			db_set_w(NULL, DBMODULENAME, newSettings[i], key);
		}

		mir_snprintf(szSetting, SIZEOF(szSetting), "HKEn%s", oldSettings[i]);
		if ((key = db_get_b(NULL, "Clist", szSetting, 0))) {
			db_unset(NULL, "Clist", szSetting);
			db_set_b(NULL, DBMODULENAME "Off", newSettings[i], (BYTE)(key == 0));
		}
	}

	return 0;
}

void UnloadSkinHotkeys(void)
{
	if (!bModuleInitialized)
		return;

	DestroyHookableEvent(hEvChanged);
	UnhookWindowsHookEx(hhkKeyboard);

	for (int i=0; i < hotkeys.getCount(); i++)
		FreeHotkey(hotkeys[i]);

	DestroyWindow(g_hwndHotkeyHost);
}

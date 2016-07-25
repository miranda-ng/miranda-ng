/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-16 Miranda NG project (http://miranda-ng.org),
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
	for (int i = 0; i < hotkeys.getCount(); i++)
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
		for (int i = 0; i < hotkeys.getCount(); i++) {
			THotkeyItem *p = hotkeys[i];
			if (p->type != HKT_GLOBAL || !p->Enabled)
				continue;

			if (p->pszService && (wParam == p->idHotkey)) {
				CallService(p->pszService, 0, p->lParam);
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

			for (int i = 0; i < hotkeys.getCount(); i++) {
				THotkeyItem *p = hotkeys[i];
				if (p->type != HKT_LOCAL || !p->Enabled)
					continue;

				BYTE hkMod, hkVk;
				sttWordToModAndVk(p->Hotkey, &hkMod, &hkVk);
				if (!hkVk) continue;
				if (p->pszService && vk == hkVk && mod == hkMod) {
					CallService(p->pszService, 0, p->lParam);
					return TRUE;
				}
			}
		}
	}

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

	THotkeyItem *p = (THotkeyItem*)mir_alloc(sizeof(THotkeyItem));
	DWORD dwFlags = (desc->cbSize >= sizeof(HOTKEYDESC)) ? desc->dwFlags : 0;
	if (dwFlags & HKD_UNICODE) {
		p->ptszSection = mir_tstrdup(desc->ptszSection);
		p->ptszDescription = mir_tstrdup(desc->ptszDescription);
	}
	else {
		p->ptszSection = mir_a2u(desc->pszSection);
		p->ptszDescription = mir_a2u(desc->pszDescription);
	}

	p->hLangpack = (int)wParam;
	p->allowSubHotkeys = TRUE;
	p->rootHotkey = NULL;
	p->nSubHotkeys = 0;

	if (p->rootHotkey = hotkeys.find(p)) {
		if (p->rootHotkey->allowSubHotkeys) {
			char nameBuf[MAXMODULELABELLENGTH];
			mir_snprintf(nameBuf, "%s$%d", p->rootHotkey->pszName, p->rootHotkey->nSubHotkeys);
			p->pszName = mir_strdup(nameBuf);
			p->Enabled = TRUE;

			p->rootHotkey->nSubHotkeys++;
		}
		else {
			mir_free(p->ptszSection);
			mir_free(p->ptszDescription);
			mir_free(p);
			return 0;
		}
	}
	else {
		p->pszName = mir_strdup(desc->pszName);
		p->Enabled = !db_get_b(NULL, DBMODULENAME "Off", p->pszName, 0);
	}

	p->pszService = desc->pszService ? mir_strdup(desc->pszService) : 0;
	p->DefHotkey = desc->DefHotKey & ~HKF_MIRANDA_LOCAL;
	p->Hotkey = db_get_w(NULL, DBMODULENAME, p->pszName, p->DefHotkey);
	p->type = p->pszService ?
		(THotkeyType)db_get_b(NULL, DBMODULENAME "Types", p->pszName,
		(desc->DefHotKey & HKF_MIRANDA_LOCAL) ? HKT_LOCAL : HKT_GLOBAL) : HKT_MANUAL;
	p->lParam = desc->lParam;

	char buf[256];
	mir_snprintf(buf, "mir_hotkey_%d_%d", g_pid, g_hkid++);
	p->idHotkey = GlobalAddAtomA(buf);
	if (p->type == HKT_GLOBAL) {
		if (p->Enabled) {
			BYTE mod, vk;
			sttWordToModAndVk(p->Hotkey, &mod, &vk);
			if (vk) RegisterHotKey(g_hwndHotkeyHost, p->idHotkey, mod, vk);
		}
	}

	hotkeys.insert(p);

	if (!p->rootHotkey) {
		/* try to load alternatives from db */
		int count, i;
		mir_snprintf(buf, "%s$count", p->pszName);
		count = (int)db_get_dw(NULL, DBMODULENAME, buf, -1);
		for (i = 0; i < count; i++) {
			mir_snprintf(buf, "%s$%d", p->pszName, i);
			if (!db_get_w(NULL, DBMODULENAME, buf, 0))
				continue;

			svcHotkeyRegister(wParam, lParam);
		}
		p->allowSubHotkeys = count < 0;
	}
	else {
		mir_free(p->pszName);
		p->pszName = NULL;
	}

	return p->idHotkey;
}

static INT_PTR svcHotkeyUnregister(WPARAM, LPARAM lParam)
{
	char *pszName = (char *)lParam;
	char pszNamePrefix[MAXMODULELABELLENGTH];
	size_t cbNamePrefix;
	mir_snprintf(pszNamePrefix, "%s$", pszName);
	cbNamePrefix = mir_strlen(pszNamePrefix);

	for (int i = 0; i < hotkeys.getCount(); i++) {
		char *pszCurrentName = hotkeys[i]->rootHotkey ?
			hotkeys[i]->rootHotkey->pszName :
			hotkeys[i]->pszName;
		if (!pszCurrentName)
			continue;

		hotkeys[i]->UnregisterHotkey =
			!mir_strcmp(pszCurrentName, pszName) ||
			!strncmp(pszCurrentName, pszNamePrefix, cbNamePrefix);
	}

	if (g_hwndHkOptions)
		SendMessage(g_hwndHkOptions, WM_HOTKEYUNREGISTERED, 0, 0);

	for (int i = hotkeys.getCount()-1; i >= 0; i--)
		if (hotkeys[i]->UnregisterHotkey) {
			FreeHotkey(hotkeys[i]);
			hotkeys.remove(i);
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

			for (i = 0; i < hotkeys.getCount(); i++) {
				THotkeyItem *p = hotkeys[i];
				if ((p->type != HKT_MANUAL) || mir_tstrcmp(pszSection, p->ptszSection))
					continue;

				BYTE hkMod, hkVk;
				sttWordToModAndVk(p->Hotkey, &hkMod, &hkVk);
				if (!hkVk) continue;
				if (!p->Enabled) continue;
				if ((vk == hkVk) && (mod == hkMod)) {
					mir_free(pszSection);
					return p->lParam;
				}
			}
		}
	}

	mir_free(pszSection);
	return 0;
}

void FreeHotkey(THotkeyItem *p)
{
	if (p->type == HKT_GLOBAL && p->Enabled)
		UnregisterHotKey(g_hwndHotkeyHost, p->idHotkey);
	GlobalDeleteAtom(p->idHotkey);
	mir_free(p->pszName);
	mir_free(p->pszService);
	mir_free(p->ptszDescription);
	mir_free(p->ptszSection);
	mir_free(p);
}

void RegisterHotkeys()
{
	for (int i = 0; i < hotkeys.getCount(); i++) {
		THotkeyItem *p = hotkeys[i];
		UnregisterHotKey(g_hwndHotkeyHost, p->idHotkey);
		if (p->type != HKT_GLOBAL) continue;
		if (p->Enabled) {
			BYTE mod, vk;
			sttWordToModAndVk(p->Hotkey, &mod, &vk);
			if (vk)
				RegisterHotKey(g_hwndHotkeyHost, p->idHotkey, mod, vk);
		}
	}
}

MIR_APP_DLL(void) KillModuleHotkeys(int _hLang)
{
	for (int i = hotkeys.getCount()-1; i >= 0; i--) {
		THotkeyItem *p = hotkeys[i];
		if (p->hLangpack == _hLang) {
			FreeHotkey(p);
			hotkeys.remove(i);
		}
	}
}

void UnregisterHotkeys()
{
	for (int i = 0; i < hotkeys.getCount(); i++) {
		THotkeyItem *p = hotkeys[i];
		if (p->type == HKT_GLOBAL && p->Enabled)
			UnregisterHotKey(g_hwndHotkeyHost, p->idHotkey);
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
	bModuleInitialized = TRUE;

	WNDCLASSEX wcl = { 0 };
	wcl.cbSize = sizeof(wcl);
	wcl.lpfnWndProc = sttHotkeyHostWndProc;
	wcl.style = 0;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hInstance = g_hInst;
	wcl.hIcon = NULL;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszMenuName = NULL;
	wcl.lpszClassName = L"MirandaHotkeyHostWnd";
	wcl.hIconSm = NULL;
	RegisterClassEx(&wcl);

	g_pid = GetCurrentProcessId();

	g_hwndHotkeyHost = CreateWindow(L"MirandaHotkeyHostWnd", NULL, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, g_hInst, NULL);
	SetWindowPos(g_hwndHotkeyHost, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_HIDEWINDOW);

	hhkKeyboard = SetWindowsHookEx(WH_KEYBOARD, sttKeyboardProc, NULL, hMainThreadId);

	hEvChanged = CreateHookableEvent(ME_HOTKEYS_CHANGED);

	CreateServiceFunction("CoreHotkeys/Register", svcHotkeyRegister);
	CreateServiceFunction(MS_HOTKEY_UNREGISTER, svcHotkeyUnregister);
	CreateServiceFunction(MS_HOTKEY_SUBCLASS, svcHotkeySubclass);
	CreateServiceFunction(MS_HOTKEY_UNSUBCLASS, svcHotkeyUnsubclass);
	CreateServiceFunction(MS_HOTKEY_CHECK, svcHotkeyCheck);

	HookEvent(ME_SYSTEM_MODULESLOADED, sttModulesLoaded);

	for (int i = 0; i < _countof(oldSettings); i++) {
		char szSetting[100];
		mir_snprintf(szSetting, "HK%s", oldSettings[i]);

		WORD key;
		if ((key = db_get_w(NULL, "Clist", szSetting, 0))) {
			db_unset(NULL, "Clist", szSetting);
			db_set_w(NULL, DBMODULENAME, newSettings[i], key);
		}

		mir_snprintf(szSetting, "HKEn%s", oldSettings[i]);
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

	for (int i = 0; i < hotkeys.getCount(); i++)
		FreeHotkey(hotkeys[i]);

	DestroyWindow(g_hwndHotkeyHost);
}

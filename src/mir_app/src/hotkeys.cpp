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

static int sttCompareHotkeys(const THotkeyItem *p1, const THotkeyItem *p2)
{
	int res;
	if (res = mir_wstrcmp(p1->pwszSection, p2->pwszSection))
		return res;
	if (res = mir_wstrcmp(p1->pwszDescription, p2->pwszDescription))
		return res;
	if (!p1->rootHotkey && p2->rootHotkey)
		return -1;
	if (p1->rootHotkey && !p2->rootHotkey)
		return 1;
	return 0;
}

LIST<THotkeyItem> hotkeys(10, sttCompareHotkeys);
uint32_t g_pid = 0, g_hkid = 1;
HWND g_hwndHotkeyHost = nullptr, g_hwndHkOptions = nullptr;
HANDLE hEvChanged = nullptr;

static BOOL bModuleInitialized = FALSE;
static HHOOK hhkKeyboard = nullptr;

uint16_t GetHotkeyValue(INT_PTR idHotkey)
{
	for (auto &it : hotkeys)
		if (it->idHotkey == idHotkey)
			return it->Enabled ? it->Hotkey : 0;

	return 0;
}

static void sttWordToModAndVk(uint16_t w, uint8_t *mod, uint8_t *vk)
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
	if (msg == WM_HOTKEY && g_hwndHkOptions == nullptr) {
		for (auto &p : hotkeys) {
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
	if (code == HC_ACTION && !(HIWORD(lParam) & KF_UP) && g_hwndHkOptions == nullptr) {
		uint8_t mod = 0, vk = wParam;
		if (vk) {
			if (GetAsyncKeyState(VK_CONTROL)) mod |= MOD_CONTROL;
			if (GetAsyncKeyState(VK_MENU)) mod |= MOD_ALT;
			if (GetAsyncKeyState(VK_SHIFT)) mod |= MOD_SHIFT;
			if (GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_RWIN)) mod |= MOD_WIN;

			for (auto &p : hotkeys) {
				if (p->type != HKT_LOCAL || !p->Enabled)
					continue;

				uint8_t hkMod, hkVk;
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

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Hotkey_Register(const HOTKEYDESC *desc, HPLUGIN pPlugin)
{
	THotkeyItem *p = (THotkeyItem*)mir_alloc(sizeof(THotkeyItem));
	if (desc->dwFlags & HKD_UNICODE) {
		p->pwszSection = mir_wstrdup(desc->szSection.w);
		p->pwszDescription = mir_wstrdup(desc->szDescription.w);
	}
	else {
		p->pwszSection = mir_a2u(desc->szSection.a);
		p->pwszDescription = mir_a2u(desc->szDescription.a);
	}

	p->pPlugin = pPlugin;
	p->allowSubHotkeys = TRUE;
	p->rootHotkey = nullptr;
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
			mir_free(p->pwszSection);
			mir_free(p->pwszDescription);
			mir_free(p);
			return 0;
		}
	}
	else {
		p->pszName = mir_strdup(desc->pszName);
		p->Enabled = !db_get_b(0, DBMODULENAME "Off", p->pszName, 0);
	}

	p->pszService = mir_strdup(desc->pszService);
	p->DefHotkey = desc->DefHotKey & ~HKF_MIRANDA_LOCAL;
	p->Hotkey = db_get_w(0, DBMODULENAME, p->pszName, p->DefHotkey);
	p->type = p->pszService ?
		(THotkeyType)db_get_b(0, DBMODULENAME "Types", p->pszName,
		(desc->DefHotKey & HKF_MIRANDA_LOCAL) ? HKT_LOCAL : HKT_GLOBAL) : HKT_MANUAL;
	p->lParam = desc->lParam;

	char buf[256];
	mir_snprintf(buf, "mir_hotkey_%d_%d", g_pid, g_hkid++);
	p->idHotkey = GlobalAddAtomA(buf);
	if (p->type == HKT_GLOBAL) {
		if (p->Enabled) {
			uint8_t mod, vk;
			sttWordToModAndVk(p->Hotkey, &mod, &vk);
			if (vk)
				RegisterHotKey(g_hwndHotkeyHost, p->idHotkey, mod, vk);
		}
	}

	hotkeys.insert(p);

	if (!p->rootHotkey) {
		/* try to load alternatives from db */
		mir_snprintf(buf, "%s$count", p->pszName);
		int count = (int)db_get_dw(0, DBMODULENAME, buf, -1);
		for (int i = 0; i < count; i++) {
			mir_snprintf(buf, "%s$%d", p->pszName, i);
			if (!db_get_w(0, DBMODULENAME, buf, 0))
				continue;

			Hotkey_Register(desc, pPlugin);
		}
		p->allowSubHotkeys = count < 0;
	}
	else {
		mir_free(p->pszName);
		p->pszName = nullptr;
	}

	return p->idHotkey;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Hotkey_Unregister(const char *pszName)
{
	char pszNamePrefix[MAXMODULELABELLENGTH];
	size_t cbNamePrefix;
	mir_snprintf(pszNamePrefix, "%s$", pszName);
	cbNamePrefix = mir_strlen(pszNamePrefix);

	for (auto &it : hotkeys) {
		char *pszCurrentName = it->getName();
		if (!pszCurrentName)
			continue;

		it->UnregisterHotkey =
			!mir_strcmp(pszCurrentName, pszName) ||
			!strncmp(pszCurrentName, pszNamePrefix, cbNamePrefix);
	}

	if (g_hwndHkOptions)
		SendMessage(g_hwndHkOptions, WM_HOTKEYUNREGISTERED, 0, 0);

	for (auto &it : hotkeys.rev_iter())
		if (it->UnregisterHotkey)
			FreeHotkey(hotkeys.removeItem(&it));

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Hotkey_Check(MSG *msg, const char *szSection)
{
	if (msg->message == WM_KEYDOWN || msg->message == WM_SYSKEYDOWN) {
		uint8_t mod = 0, vk = msg->wParam;
		if (vk) {
			if (GetAsyncKeyState(VK_CONTROL)) mod |= MOD_CONTROL;
			if (GetAsyncKeyState(VK_MENU)) mod |= MOD_ALT;
			if (GetAsyncKeyState(VK_SHIFT)) mod |= MOD_SHIFT;
			if (GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_RWIN)) mod |= MOD_WIN;

			ptrW pszSection(mir_a2u(szSection));
			for (auto &p : hotkeys) {
				if ((p->type != HKT_MANUAL) || mir_wstrcmp(pszSection, p->pwszSection))
					continue;

				uint8_t hkMod, hkVk;
				sttWordToModAndVk(p->Hotkey, &hkMod, &hkVk);
				if (!hkVk) continue;
				if (!p->Enabled) continue;
				if (vk == hkVk && mod == hkMod)
					return p->lParam;
			}
		}
	}

	return 0;
}

void FreeHotkey(THotkeyItem *p)
{
	if (p->type == HKT_GLOBAL && p->Enabled)
		UnregisterHotKey(g_hwndHotkeyHost, p->idHotkey);
	GlobalDeleteAtom(p->idHotkey);
	mir_free(p->pszName);
	mir_free(p->pszService);
	mir_free(p->pwszDescription);
	mir_free(p->pwszSection);
	mir_free(p);
}

void RegisterHotkeys()
{
	for (auto &p : hotkeys) {
		UnregisterHotKey(g_hwndHotkeyHost, p->idHotkey);
		if (p->type != HKT_GLOBAL) continue;
		if (p->Enabled) {
			uint8_t mod, vk;
			sttWordToModAndVk(p->Hotkey, &mod, &vk);
			if (vk)
				RegisterHotKey(g_hwndHotkeyHost, p->idHotkey, mod, vk);
		}
	}
}

void KillModuleHotkeys(CMPluginBase *pPlugin)
{
	for (auto &it : hotkeys.rev_iter())
		if (it->pPlugin == pPlugin)
			FreeHotkey(hotkeys.removeItem(&it));
}

void UnregisterHotkeys()
{
	for (auto &p : hotkeys)
		if (p->type == HKT_GLOBAL && p->Enabled)
			UnregisterHotKey(g_hwndHotkeyHost, p->idHotkey);
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
	wcl.hInstance = g_plugin.getInst();
	wcl.hIcon = nullptr;
	wcl.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszMenuName = nullptr;
	wcl.lpszClassName = L"MirandaHotkeyHostWnd";
	wcl.hIconSm = nullptr;
	RegisterClassEx(&wcl);

	g_pid = GetCurrentProcessId();

	g_hwndHotkeyHost = CreateWindow(L"MirandaHotkeyHostWnd", nullptr, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, nullptr, g_plugin.getInst(), nullptr);
	SetWindowPos(g_hwndHotkeyHost, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_HIDEWINDOW);

	hhkKeyboard = SetWindowsHookEx(WH_KEYBOARD, sttKeyboardProc, nullptr, hMainThreadId);

	hEvChanged = CreateHookableEvent(ME_HOTKEYS_CHANGED);

	HookEvent(ME_OPT_INITIALISE, HotkeyOptionsInit);

	for (int i = 0; i < _countof(oldSettings); i++) {
		char szSetting[100];
		mir_snprintf(szSetting, "HK%s", oldSettings[i]);

		uint16_t key;
		if ((key = db_get_w(0, "Clist", szSetting, 0))) {
			db_unset(0, "Clist", szSetting);
			db_set_w(0, DBMODULENAME, newSettings[i], key);
		}

		mir_snprintf(szSetting, "HKEn%s", oldSettings[i]);
		if ((key = db_get_b(0, "Clist", szSetting, 0))) {
			db_unset(0, "Clist", szSetting);
			db_set_b(0, DBMODULENAME "Off", newSettings[i], (uint8_t)(key == 0));
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

	for (auto &p : hotkeys)
		FreeHotkey(p);

	DestroyWindow(g_hwndHotkeyHost);
}

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

#include "../stdafx.h"

MIR_CORE_DLL(int) Utils_SaveWindowPosition(HWND hwnd, MCONTACT hContact, const char *szModule, const char *szNamePrefix)
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(hwnd, &wp);

	char szSettingName[64];
	mir_snprintf(szSettingName, "%sx", szNamePrefix);
	db_set_dw(hContact, szModule, szSettingName, wp.rcNormalPosition.left);

	mir_snprintf(szSettingName, "%sy", szNamePrefix);
	db_set_dw(hContact, szModule, szSettingName, wp.rcNormalPosition.top);

	mir_snprintf(szSettingName, "%swidth", szNamePrefix);
	db_set_dw(hContact, szModule, szSettingName, wp.rcNormalPosition.right-wp.rcNormalPosition.left);

	mir_snprintf(szSettingName, "%sheight", szNamePrefix);
	db_set_dw(hContact, szModule, szSettingName, wp.rcNormalPosition.bottom-wp.rcNormalPosition.top);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

EXTERN_C MIR_CORE_DLL(int) Utils_RestoreWindowPosition(HWND hwnd, MCONTACT hContact, const char *szModule, const char *szNamePrefix, int flags)
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(hwnd, &wp);

	char szSettingName[64];
	mir_snprintf(szSettingName, "%sx", szNamePrefix);
	int x = db_get_dw(hContact, szModule, szSettingName, -1);
	if (x == -1)
		return 1;

	mir_snprintf(szSettingName, "%sy", szNamePrefix);
	int y = (int)db_get_dw(hContact, szModule, szSettingName, -1);

	if (flags & RWPF_NOSIZE)
		OffsetRect(&wp.rcNormalPosition, x-wp.rcNormalPosition.left, y-wp.rcNormalPosition.top);
	else {
		wp.rcNormalPosition.left = x;
		wp.rcNormalPosition.top = y;

		mir_snprintf(szSettingName, "%swidth", szNamePrefix);
		wp.rcNormalPosition.right = wp.rcNormalPosition.left+db_get_dw(hContact, szModule, szSettingName, -1);

		mir_snprintf(szSettingName, "%sheight", szNamePrefix);
		wp.rcNormalPosition.bottom = wp.rcNormalPosition.top+db_get_dw(hContact, szModule, szSettingName, -1);
	}
	wp.flags = 0;
	if (flags & RWPF_HIDDEN)
		wp.showCmd = SW_HIDE;
	if (flags & RWPF_NOACTIVATE)
		wp.showCmd = SW_SHOWNOACTIVATE;

	if (!(flags & RWPF_NOMOVE))
		Utils_AssertInsideScreen(&wp.rcNormalPosition);

	SetWindowPlacement(hwnd, &wp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(int) Utils_AssertInsideScreen(RECT *rc)
{
	if (rc == nullptr)
		return -1;

	RECT rcScreen;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, FALSE);
	if (MonitorFromRect(rc, MONITOR_DEFAULTTONULL))
		return 0;

	MONITORINFO mi = { 0 };
	HMONITOR hMonitor = MonitorFromRect(rc, MONITOR_DEFAULTTONEAREST);
	mi.cbSize = sizeof(mi);
	if (GetMonitorInfo(hMonitor, &mi))
		rcScreen = mi.rcWork;

	if (rc->top >= rcScreen.bottom)
		OffsetRect(rc, 0, rcScreen.bottom - rc->bottom);
	else if (rc->bottom <= rcScreen.top)
		OffsetRect(rc, 0, rcScreen.top - rc->top);
	if (rc->left >= rcScreen.right)
		OffsetRect(rc, rcScreen.right - rc->right, 0);
	else if (rc->right <= rcScreen.left)
		OffsetRect(rc, rcScreen.left - rc->left, 0);

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

static NONCLIENTMETRICSW ncm = {};

MIR_CORE_DLL(int) Utils_CorrectFontSize(int size)
{
	if (!g_bEnableDpiAware)
		return size;

	if (!ncm.cbSize) {
		ncm.cbSize = sizeof(ncm);
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE);
	}

	return size * ncm.lfMessageFont.lfHeight / -12;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(void) Utils_ClipboardCopy(const char *pszText)
{
	size_t cbLen = mir_strlen(pszText);
	if (!cbLen)
		return;
	
	if (!OpenClipboard(nullptr))
		return;

	EmptyClipboard();

	HGLOBAL hData = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, cbLen);
	if (hData) {
		mir_strcpy((char *)GlobalLock(hData), pszText);
		GlobalUnlock(hData);
		SetClipboardData(CF_TEXT, hData);
	}
	CloseClipboard();
}

MIR_CORE_DLL(void) Utils_ClipboardCopy(const wchar_t *pwszText)
{
	size_t cbLen = mir_wstrlen(pwszText);
	if (!cbLen)
		return;

	if (!OpenClipboard(nullptr))
		return;

	EmptyClipboard();

	HGLOBAL hData = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (cbLen + 1) * sizeof(wchar_t));
	if (hData) {
		mir_wstrcpy((wchar_t *)GlobalLock(hData), pwszText);
		GlobalUnlock(hData);
		SetClipboardData(CF_UNICODETEXT, hData);
	}
	CloseClipboard();
}

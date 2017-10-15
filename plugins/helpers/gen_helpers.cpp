/*
	Helper functions for Miranda-IM (www.miranda-im.org)
	Copyright 2006 P. Boon

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "commonheaders.h"
#include "gen_helpers.h"

wchar_t *Hlp_GetProtocolName(const char *proto) {

	char protoname[256];
	if ((!ProtoServiceExists(proto, PS_GETNAME)) || (CallProtoService(proto, PS_GETNAME, (WPARAM)sizeof(protoname), (LPARAM)protoname)))
		return nullptr;

	return mir_a2u(protoname);

}

wchar_t *Hlp_GetDlgItemText(HWND hwndDlg, int nIDDlgItem) {

	int len = SendDlgItemMessage(hwndDlg, nIDDlgItem, WM_GETTEXTLENGTH, 0, 0);
	if (len < 0)
		return nullptr;

	wchar_t *res = (wchar_t*)mir_alloc((len + 1)*sizeof(wchar_t));
	memset(res, 0, ((len + 1) * sizeof(wchar_t)));
	GetDlgItemText(hwndDlg, nIDDlgItem, res, len + 1);

	return res;
}

wchar_t *Hlp_GetWindowText(HWND hwndDlg)
{
	int len = GetWindowTextLength(hwndDlg);
	if (len < 0)
		return nullptr;

	wchar_t *res = (wchar_t*)mir_alloc((len + 1)*sizeof(wchar_t));
	memset(res, 0, ((len + 1) * sizeof(wchar_t)));
	GetWindowText(hwndDlg, res, len + 1);

	return res;
}

/**
 * Modified from Miranda CList, clistsettings.c
 **/

// Logging
static int WriteToDebugLogA(const char *szMsg)
{
	return Netlib_Log(0, szMsg);
}

int AddDebugLogMessageA(const char* fmt, ...)
{
	int res;
	char szText[MAX_DEBUG], szFinal[MAX_DEBUG];
	va_list va;

	va_start(va, fmt);
	mir_vsnprintf(szText, _countof(szText), fmt, va);
	va_end(va);
#ifdef MODULENAME
	mir_snprintf(szFinal, "%s: %s", MODULENAME, szText);
#else
	strncpy(szFinal, szText, _countof(szFinal));
#endif
	res = WriteToDebugLogA(szFinal);

	return res;
}

int AddDebugLogMessage(const wchar_t* fmt, ...)
{

	int res;
	wchar_t tszText[MAX_DEBUG], tszFinal[MAX_DEBUG];
	char *szFinal;
	va_list va;

	va_start(va, fmt);
	mir_vsnwprintf(tszText, _countof(tszText), fmt, va);
	va_end(va);
#ifdef MODULENAME
	mir_snwprintf(tszFinal, L"%s: %s", MODULENAME, tszText);
#else
	wcsncpy(tszFinal, tszText, _countof(tszFinal));
#endif


	szFinal = mir_u2a(tszFinal);

	res = WriteToDebugLogA(szFinal);
	mir_free(szFinal);

	return res;
}

int ttoi(wchar_t *string)
{
	return (string == nullptr) ? 0 : _wtoi(string);
}

wchar_t *itot(int num)
{
	wchar_t tRes[32];
	// check this
	_itow(num, tRes, 10);
	return mir_wstrdup(tRes);
}

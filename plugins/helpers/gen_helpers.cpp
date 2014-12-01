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

TCHAR *Hlp_GetProtocolName(const char *proto) {

	char protoname[256];
	if ((!ProtoServiceExists(proto, PS_GETNAME)) || (CallProtoService(proto, PS_GETNAME, (WPARAM)sizeof(protoname), (LPARAM)protoname)))
		return NULL;

	return mir_a2t(protoname);

}

TCHAR *Hlp_GetDlgItemText(HWND hwndDlg, int nIDDlgItem) {

	int len = SendDlgItemMessage(hwndDlg, nIDDlgItem, WM_GETTEXTLENGTH, 0, 0);
	if (len < 0)
		return NULL;

	TCHAR *res = (TCHAR*)mir_alloc((len+1)*sizeof(TCHAR));
	memset(res, 0, ((len + 1) * sizeof(TCHAR)));
	GetDlgItemText(hwndDlg, nIDDlgItem, res, len+1);

	return res;
}

TCHAR *Hlp_GetWindowText(HWND hwndDlg)
{
	int len = SendMessage(hwndDlg, WM_GETTEXTLENGTH, 0, 0);
	if (len < 0)
		return NULL;

	TCHAR *res = (TCHAR*)mir_alloc((len+1)*sizeof(TCHAR));
	memset(res, 0, ((len + 1) * sizeof(TCHAR)));
	GetWindowText(hwndDlg, res, len+1);

	return res;
}

/**
 * Modified from Miranda CList, clistsettings.c
 **/

// Logging
static int WriteToDebugLogA(const char *szMsg)
{

	int res = 0;
	if (ServiceExists(MS_NETLIB_LOG))
		res = CallService(MS_NETLIB_LOG, 0, (LPARAM)szMsg);
	else {
		OutputDebugStringA(szMsg);
		OutputDebugStringA("\r\n");
	}

	return res;
}

int AddDebugLogMessageA(const char* fmt, ...)
{
	int res;
	char szText[MAX_DEBUG], szFinal[MAX_DEBUG];
	va_list va;

	va_start(va,fmt);
	mir_vsnprintf(szText, SIZEOF(szText), fmt, va);
	va_end(va);
#ifdef MODULENAME
	mir_snprintf(szFinal, SIZEOF(szFinal), "%s: %s", MODULENAME, szText);
#else
	strncpy(szFinal, szText, SIZEOF(szFinal));
#endif
	res = WriteToDebugLogA(szFinal);

	return res;
}

int AddDebugLogMessage(const TCHAR* fmt, ...)
{

	int res;
	TCHAR tszText[MAX_DEBUG], tszFinal[MAX_DEBUG];
	char *szFinal;
	va_list va;

	va_start(va,fmt);
	mir_vsntprintf(tszText, SIZEOF(tszText), fmt, va);
	va_end(va);
#ifdef MODULENAME
	mir_sntprintf(tszFinal, SIZEOF(tszFinal), _T("%s: %s"), MODULENAME, tszText);
#else
	_tcsncpy(tszFinal, tszText, SIZEOF(tszFinal));
#endif


	szFinal = mir_t2a(tszFinal);

	res = WriteToDebugLogA(szFinal);
	mir_free(szFinal);

	return res;
}

int ttoi(TCHAR *string)
{
	return ( string == NULL) ? 0 : _ttoi( string );
}

TCHAR *itot(int num)
{
	TCHAR tRes[32];
	// check this
	_itot(num, tRes, 10);
	return mir_tstrdup(tRes);
}

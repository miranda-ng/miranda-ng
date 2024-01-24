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

wchar_t* Hlp_GetProtocolName(const char *proto)
{
	return mir_a2u(proto);
}

wchar_t* Hlp_GetDlgItemText(HWND hwndDlg, int nIDDlgItem)
{
	int len = SendDlgItemMessage(hwndDlg, nIDDlgItem, WM_GETTEXTLENGTH, 0, 0);
	if (len < 0)
		return nullptr;

	wchar_t *res = (wchar_t*)mir_alloc((len + 1) * sizeof(wchar_t));
	memset(res, 0, ((len + 1) * sizeof(wchar_t)));
	GetDlgItemText(hwndDlg, nIDDlgItem, res, len + 1);
	return res;
}

wchar_t* Hlp_GetWindowText(HWND hwndDlg)
{
	int len = GetWindowTextLength(hwndDlg);
	if (len < 0)
		return nullptr;

	wchar_t *res = (wchar_t*)mir_alloc((len + 1) * sizeof(wchar_t));
	memset(res, 0, ((len + 1) * sizeof(wchar_t)));
	GetWindowText(hwndDlg, res, len + 1);
	return res;
}

wchar_t* _itow(int num)
{
	wchar_t tRes[32];
	// check this
	_itow(num, tRes, 10);
	return mir_wstrdup(tRes);
}

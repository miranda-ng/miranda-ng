/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org),
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
#include <ctype.h>

struct TOpenUrlInfo
{
	TOpenUrlInfo(wchar_t *_url, int _bNew) :
		szUrl(_url),
		newWindow(_bNew)
	{}

	ptrW szUrl;
	int newWindow;
};

static void __cdecl OpenURLThread(TOpenUrlInfo *hUrlInfo)
{
	// wack a protocol on it
	CMStringW tszUrl;
	if ((isalpha(hUrlInfo->szUrl[0]) && hUrlInfo->szUrl[1] == ':') || hUrlInfo->szUrl[0] == '\\')
		tszUrl.Format(L"file:///%s", hUrlInfo->szUrl.get());
	else {
		int i;
		for (i = 0; iswalpha(hUrlInfo->szUrl[i]); i++);
		if (hUrlInfo->szUrl[i] == ':')
			tszUrl = hUrlInfo->szUrl;
		else if (!wcsnicmp(hUrlInfo->szUrl, L"ftp.", 4))
			tszUrl.Format(L"ftp://%s", hUrlInfo->szUrl.get());
		else
			tszUrl.Format(L"http://%s", hUrlInfo->szUrl.get());
	}

	// check user defined browser for opening urls
	ptrW tszBrowser(db_get_wsa(0, "Miranda", "OpenUrlBrowser"));
	if (tszBrowser)
		ShellExecute(nullptr, L"open", tszBrowser, tszUrl, nullptr, (hUrlInfo->newWindow) ? SW_NORMAL : SW_SHOWDEFAULT);
	else
		ShellExecute(nullptr, L"open", tszUrl, nullptr, nullptr, (hUrlInfo->newWindow) ? SW_NORMAL : SW_SHOWDEFAULT);

	delete hUrlInfo;
}

MIR_CORE_DLL(void) Utils_OpenUrl(const char *pszUrl, bool bOpenInNewWindow)
{
	if (pszUrl)
		mir_forkThread<TOpenUrlInfo>(OpenURLThread, new TOpenUrlInfo(mir_a2u(pszUrl), bOpenInNewWindow));
}

MIR_CORE_DLL(void) Utils_OpenUrlW(const wchar_t *pszUrl, bool bOpenInNewWindow)
{
	if (pszUrl)
		mir_forkThread<TOpenUrlInfo>(OpenURLThread, new TOpenUrlInfo(mir_wstrdup(pszUrl), bOpenInNewWindow));
}

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
#include <ctype.h>

typedef struct {
	TCHAR *szUrl;
	int newWindow;
}
	TOpenUrlInfo;

static void OpenURLThread(void *arg)
{
	TOpenUrlInfo *hUrlInfo = (TOpenUrlInfo*)arg;
	if (!hUrlInfo->szUrl)
		return;

	//wack a protocol on it
	size_t size = mir_tstrlen(hUrlInfo->szUrl)+9;
	TCHAR *szResult = (TCHAR*)mir_alloc(sizeof(TCHAR)*size);
	if ((isalpha(hUrlInfo->szUrl[0]) && hUrlInfo->szUrl[1] == ':') || hUrlInfo->szUrl[0] == '\\') {
		mir_sntprintf(szResult, size, _T("file:///%s"), hUrlInfo->szUrl);
	}
	else {
		int i;
		for (i=0; _istalpha(hUrlInfo->szUrl[i]); i++);
		if (hUrlInfo->szUrl[i] == ':')
			szResult = mir_tstrdup(hUrlInfo->szUrl);
		else {
			if (!_tcsnicmp(hUrlInfo->szUrl, _T("ftp."), 4))
				mir_sntprintf(szResult, size, _T("ftp://%s"), hUrlInfo->szUrl);
			else
				mir_sntprintf(szResult, size, _T("http://%s"), hUrlInfo->szUrl);
		}
	}

	// check user defined browser for opening urls
	DBVARIANT dbv;
	if (!db_get_ts(NULL, "Miranda", "OpenUrlBrowser", &dbv)) {
		ShellExecute(NULL, _T("open"), dbv.ptszVal, szResult, NULL, (hUrlInfo->newWindow) ? SW_NORMAL : SW_SHOWDEFAULT);
		db_free(&dbv);
	}
	else ShellExecute(NULL, _T("open"), szResult, NULL, NULL, (hUrlInfo->newWindow) ? SW_NORMAL : SW_SHOWDEFAULT);

	mir_free(szResult);
	mir_free(hUrlInfo->szUrl);
	mir_free(hUrlInfo);
}

static INT_PTR OpenURL(WPARAM wParam, LPARAM lParam)
{
	TOpenUrlInfo *hUrlInfo = (TOpenUrlInfo*)mir_alloc(sizeof(TOpenUrlInfo));
	hUrlInfo->szUrl = (wParam & OUF_UNICODE) ? mir_wstrdup((WCHAR*)lParam) : mir_a2t((char*)lParam);
	hUrlInfo->newWindow = (wParam & OUF_NEWWINDOW) != 0;
	forkthread(OpenURLThread, 0, (void*)hUrlInfo);
	return 0;
}

int InitOpenUrl(void)
{
	CreateServiceFunction(MS_UTILS_OPENURL, OpenURL);
	return 0;
}

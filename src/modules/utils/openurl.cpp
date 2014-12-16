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

struct TOpenUrlInfo
{
	TOpenUrlInfo(TCHAR *_url, int _bNew) :
		szUrl(_url),
		newWindow(_bNew)
	{}

	ptrT szUrl;
	int newWindow;
};

static void OpenURLThread(void *arg)
{
	TOpenUrlInfo *hUrlInfo = (TOpenUrlInfo*)arg;

	// wack a protocol on it
	CMString tszUrl;
	if ((isalpha(hUrlInfo->szUrl[0]) && hUrlInfo->szUrl[1] == ':') || hUrlInfo->szUrl[0] == '\\')
		tszUrl.Format(_T("file:///%s"), hUrlInfo->szUrl);
	else {
		int i;
		for (i = 0; _istalpha(hUrlInfo->szUrl[i]); i++);
		if (hUrlInfo->szUrl[i] == ':')
			tszUrl = hUrlInfo->szUrl;
		else if (!_tcsnicmp(hUrlInfo->szUrl, _T("ftp."), 4))
			tszUrl.Format(_T("ftp://%s"), hUrlInfo->szUrl);
		else
			tszUrl.Format(_T("http://%s"), hUrlInfo->szUrl);
	}

	// check user defined browser for opening urls
	ptrT tszBrowser(db_get_tsa(NULL, "Miranda", "OpenUrlBrowser"));
	if (tszBrowser)
		ShellExecute(NULL, _T("open"), tszBrowser, tszUrl, NULL, (hUrlInfo->newWindow) ? SW_NORMAL : SW_SHOWDEFAULT);
	else
		ShellExecute(NULL, _T("open"), tszUrl, NULL, NULL, (hUrlInfo->newWindow) ? SW_NORMAL : SW_SHOWDEFAULT);

	delete hUrlInfo;
}

static INT_PTR OpenURL(WPARAM wParam, LPARAM lParam)
{
	if (lParam == 0)
		return 1;

	TOpenUrlInfo *hUrlInfo = new TOpenUrlInfo((wParam & OUF_UNICODE) ? mir_wstrdup((WCHAR*)lParam) : mir_a2t((char*)lParam), wParam & OUF_NEWWINDOW);
	forkthread(OpenURLThread, 0, hUrlInfo);
	return 0;
}

int InitOpenUrl(void)
{
	CreateServiceFunction(MS_UTILS_OPENURL, OpenURL);
	return 0;
}

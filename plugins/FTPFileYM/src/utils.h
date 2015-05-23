/*
FTP File YM plugin
Copyright (C) 2007-2010 Jan Holub

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "common.h"
#include "serverlist.h"

class Utils
{
private:
	static INT_PTR CALLBACK DlgProcSetFileName(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam); 

public:	
	static int msgBox(TCHAR *szMsg, UINT uType);
	static int msgBoxA(char *szMsg, UINT uType);
	static int getDeleteTimeMin();
	static HICON loadIconEx(char *szName);

	static TCHAR *getFileNameFromPath(TCHAR *stzPath);
	static TCHAR *getTextFragment(TCHAR *stzText, size_t length, TCHAR *buff);
	static char *makeSafeString(TCHAR *input, char *output);

	static void createFileDownloadLink(char *szUrl, char *fileName, char *buff, int buffSize);
	static void copyToClipboard(char *szText);
	static void curlSetOpt(CURL *hCurl, ServerList::FTP *ftp, char *url, struct curl_slist *headerList, char *errorBuff);

	static bool setFileNameDlg(TCHAR *nameBuff);
	static bool setFileNameDlgA(char *nameBuff);
};

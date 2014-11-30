/*
	ClientChangeNotify - Plugin for Miranda IM
	Copyright (c) 2006-2008 Chervov Dmitry

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

#include "Common.h"
extern BOOL bPopupExists;

__inline void ShowMsg(TCHAR *FirstLine, TCHAR *SecondLine = _T(""), bool IsErrorMsg = false, int Timeout = 0)
{
	if (bPopupExists)
	{
		POPUPDATAT ppd = {0};
		ppd.lchIcon = LoadIcon(NULL, IsErrorMsg ? IDI_EXCLAMATION : IDI_INFORMATION);
		mir_tstrcpy(ppd.lptzContactName, FirstLine);
		mir_tstrcpy(ppd.lptzText, SecondLine);
		ppd.colorBack = IsErrorMsg ? 0x0202E3 : 0xE8F1FD;
		ppd.colorText = IsErrorMsg ? 0xE8F1FD : 0x000000;
		ppd.iSeconds = Timeout;
		PUAddPopupT(&ppd);
	} else
	{
		MessageBox(NULL, SecondLine, FirstLine, MB_OK | (IsErrorMsg ? MB_ICONEXCLAMATION : MB_ICONINFORMATION));
	}
}

__inline void ShowLog(TCString &LogFilePath)
{
	int Result = (int)ShellExecute(NULL, _T("open"), LogFilePath, NULL, NULL, SW_SHOW);
	if (Result <= 32) // Error
	{
		TCHAR szError[64];
		mir_sntprintf(szError, lengthof(szError), TranslateT("Error #%d"), Result);
		ShowMsg(szError, TranslateT("Can't open log file ") + LogFilePath, true);
	}
}

__inline void RecompileRegexps(TCString IgnoreSubstrings)
{
	FreePcreCompileData();
	TCHAR *p = _tcstok(IgnoreSubstrings, _T(";"));
	while (p)
	{
		CompileRegexp(p, p[0] != '/');
		p = _tcstok(NULL, _T(";"));
	}
}

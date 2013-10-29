/*
 *  Smart Auto Replier (SAR) - auto replier plugin for Miranda IM
 *
 *  Copyright (C) 2004 - 2012 by Volodymyr M. Shcherbyna <volodymyr@shcherbyna.com>
 *
 *      This file is part of SAR.
 *
 *  SAR is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SAR is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SAR.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "comfunc.h"
///#include "shlobj.h"

extern LPTSTR g_strPluginName;
extern HINSTANCE hInst;

/// show to user msgbox that settings 
/// file is corrupted...
bool NotifyAboutWrongSettings(LPTSTR szFile)
{
	TCHAR szTxt[MAX_PATH * 2] = {0};
	_tcscpy(szTxt, TEXT("Settings file: "));
	_tcscat(szTxt, szFile);
	_tcscat(szTxt, TEXT(" is corrupted !\n"));
	_tcscat(szTxt, TEXT("As a result plugin can crush.\n"));
	_tcscat(szTxt, TEXT("Reset settings to default?"));

	DWORD dw = MessageBox(NULL, TranslateTS(szTxt), g_strPluginName, MB_YESNO);
	return (dw == IDYES);
}

/// forming path
bool MakeFullPath(LPTSTR lpRetval, DWORD dwSize, LPTSTR lpFile)
{
	/*HRESULT hr	 = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, lpRetval);
	size_t  nLen = _tcslen(lpRetval);

	if (lpRetval[nLen - 1] != TEXT('\\'))
	{
		_tcscat(lpRetval, TEXT("\\Sar\\"));
	}
	else
	{
		_tcscat(lpRetval, TEXT("Sar\\"));
	}

	_tcscat(lpRetval, lpFile);*/

	// RM TODO: fix path, use folders plugin

	GetModuleFileName(hInst, lpRetval, dwSize);
	TCHAR *ptrPos = _tcsrchr(lpRetval, '\\');
	if (ptrPos)
	{
		ptrPos++;
		_tcscpy (ptrPos, lpFile);
	}
	else
	{
		_RPT0(_CRT_WARN, "MakeFullPath - unable to form path");
		return false;
	}
	return true;
}
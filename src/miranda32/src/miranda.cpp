/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "stdafx.h"

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR cmdLine, int)
{
	TCHAR tszPath[MAX_PATH];
	GetModuleFileName(hInstance, tszPath, _countof(tszPath));

	TCHAR *p = _tcsrchr(tszPath, '\\');
	if (p == NULL)
		return 4;

	// if current dir isn't set
	p[1] = 0;
	SetCurrentDirectory(tszPath);

	// all dlls must be moved to libs
	if (!CheckDlls())
		return 3;

	_tcsncat(tszPath, _T("libs"), _TRUNCATE);
	DWORD cbPath = (DWORD)_tcslen(tszPath);
	
	DWORD cbSize = GetEnvironmentVariable(_T("PATH"), NULL, 0);
	TCHAR *ptszVal = new TCHAR[cbSize + MAX_PATH + 2];
	_tcscpy(ptszVal, tszPath);
	_tcscat(ptszVal, _T(";"));
	GetEnvironmentVariable(_T("PATH"), ptszVal + cbPath + 1, cbSize);
	SetEnvironmentVariable(_T("PATH"), ptszVal);

	HINSTANCE hMirApp = LoadLibraryA("mir_app.dll");
	if (hMirApp == NULL) {
		MessageBox(NULL, _T("mir_app.dll cannot be loaded"), _T("Fatal error"), MB_ICONERROR | MB_OK);
		return 1;
	}

	typedef int (WINAPI *pfnMain)(LPTSTR);
	pfnMain fnMain = (pfnMain)GetProcAddress(hMirApp, "mir_main");
	if (fnMain == NULL) {
		MessageBox(NULL, _T("invalid mir_app.dll present, program exiting"), _T("Fatal error"), MB_ICONERROR | MB_OK);
		return 2;
	}

	int retVal = fnMain(cmdLine);
	FreeLibrary(hMirApp);
	return retVal;
}

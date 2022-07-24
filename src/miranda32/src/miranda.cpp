/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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
#include "..\..\build\appstub\appstub.cpp"

typedef int (WINAPI *pfnMain)(LPTSTR);

#ifdef _WIN64
const wchar_t wszRuntimeUrl[] = L"https://download.visualstudio.microsoft.com/download/pr/11100230/15ccb3f02745c7b206ad10373cbca89b/VC_redist.x64.exe";
#else
const wchar_t wszRuntimeUrl[] = L"https://download.visualstudio.microsoft.com/download/pr/11100229/78c1e864d806e36f6035d80a0e80399e/VC_redist.x86.exe";
#endif

const wchar_t wszQuestion[] = L"Miranda NG needs the Visual Studio runtime library, but it cannot be loaded. Do you want to load it from Inernet?";

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPTSTR cmdLine, int)
{
	int retVal;
	HINSTANCE hMirApp = LoadLibraryW(L"mir_app.mir");
	if (hMirApp == nullptr) {
		retVal = 1;

		// zlib depends on runtime only. if it cannot be loaded too, we need to load a runtime
		HINSTANCE hZlib = LoadLibraryW(L"Libs\\zlib.mir");
		if (hZlib == nullptr) {
			if (IDYES == MessageBoxW(nullptr, wszQuestion, L"Missing runtime library", MB_ICONERROR | MB_YESNOCANCEL)) {
				ShellExecuteW(nullptr, L"open", wszRuntimeUrl, nullptr, nullptr, SW_NORMAL);
				retVal = 3;
			}
		}
		else MessageBoxW(nullptr, L"mir_app.mir cannot be loaded", L"Fatal error", MB_ICONERROR | MB_OK);
	}
	else {
		pfnMain fnMain = (pfnMain)GetProcAddress(hMirApp, "mir_main");
		if (fnMain == nullptr) {
			MessageBoxW(nullptr, L"invalid mir_app.mir present, program exiting", L"Fatal error", MB_ICONERROR | MB_OK);
			retVal = 2;
		}
		else
			retVal = fnMain(cmdLine);
		FreeLibrary(hMirApp);
	}
	return retVal;
}

/*
Miranda IM Help Plugin
Copyright (C) 2002 Richard Hughes, 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Help-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

HINSTANCE hInst;
HMODULE hMsftEdit;

extern HWND hwndHelpDlg;

int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {B16DF0B2-A08F-4bc6-8FA2-3D5FFFA2708D}
	{ 0xb16df0b2, 0xa08f, 0x4bc6, { 0x8f, 0xa2, 0x3d, 0x5f, 0xff, 0xa2, 0x70, 0x8d } }
};


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

static void InstallFile(const TCHAR *pszFileName, const TCHAR *pszDestSubDir)
{
	TCHAR szFileFrom[MAX_PATH + 1], szFileTo[MAX_PATH + 1], *p;
	HANDLE hFile;

	if (!GetModuleFileName(hInst, szFileFrom, _countof(szFileFrom) - lstrlen(pszFileName)))
		return;
	p = _tcsrchr(szFileFrom, _T('\\'));
	if (p != NULL)
		*(++p) = 0;
	lstrcat(szFileFrom, pszFileName); /* buffer safe */

	hFile = CreateFile(szFileFrom, 0, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	CloseHandle(hFile);

	if (!GetModuleFileName(NULL, szFileTo, _countof(szFileTo) - lstrlen(pszDestSubDir) - lstrlen(pszFileName)))
		return;
	p = _tcsrchr(szFileTo, _T('\\'));
	if (p != NULL)
		*(++p) = 0;
	lstrcat(szFileTo, pszDestSubDir); /* buffer safe */
	CreateDirectory(szFileTo, NULL);
	lstrcat(szFileTo, pszFileName);  /* buffer safe */

	if (!MoveFile(szFileFrom, szFileTo) && GetLastError() == ERROR_ALREADY_EXISTS) {
		DeleteFile(szFileTo);
		MoveFile(szFileFrom, szFileTo);
	}
}


extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}


extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	INITCOMMONCONTROLSEX icc = { 0 };
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_UPDOWN_CLASS | ICC_TREEVIEW_CLASSES;
	if (!InitCommonControlsEx(&icc))
		return 1;

	hMsftEdit = LoadLibrary(L"riched20.dll");
	if (hMsftEdit == NULL) {
		if (IDYES != MessageBoxEx(NULL,
			TranslateT("The Context help plugin can not be loaded, Msftedit.dll is missing. If you are using WINE, please make sure you have Msftedit.dll installed. Press 'Yes' to continue loading Miranda."),
			TranslateT("Context help plugin"), MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST | MB_TASKMODAL, LANGIDFROMLCID(Langpack_GetDefaultLocale())))
			return 1;
		return 0;
	}

	if (InstallDialogBoxHook())
		return 1;

	InitOptions();
	InitDialogCache();

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UninitOptions();
	RemoveDialogBoxHook();

	if (hwndHelpDlg != NULL)
		DestroyWindow(hwndHelpDlg);

	FreeDialogCache();
	FreeLibrary(hMsftEdit);

	return 0;
}

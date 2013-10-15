/*

'Language Pack Manager'-Plugin for Miranda IM

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (LangMan-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "common.h"

HINSTANCE hInst;

int hLangpack;

static PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX), 
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE, 
	// {D4BDD1EB-56F1-4A87-A187-67246EE919A2}
	{0xd4bdd1eb, 0x56f1, 0x4a87, {0xa1, 0x87, 0x67, 0x24, 0x6e, 0xe9, 0x19, 0xa2}}, 
};

///////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, void*)
{
	hInst = hinstDLL;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

static void InstallFile(const TCHAR *pszFileName, const TCHAR *pszDestSubDir)
{
	TCHAR szFileFrom[MAX_PATH+1], szFileTo[MAX_PATH+1];
	if ( !GetModuleFileName(hInst, szFileFrom, SIZEOF(szFileFrom) - lstrlen(pszFileName)))
		return;

	TCHAR *p = _tcsrchr(szFileFrom, _T('\\'));
	if ( p != NULL ) *(++p) = 0;
	lstrcat(szFileFrom, pszFileName); /* buffer safe */

	HANDLE hFile = CreateFile(szFileFrom, 0, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if ( hFile  ==  INVALID_HANDLE_VALUE ) return;
	CloseHandle(hFile);

	if ( !GetModuleFileName(NULL, szFileTo, SIZEOF(szFileTo) - lstrlen(pszDestSubDir) - lstrlen(pszFileName)))
		return;
	
	p = _tcsrchr(szFileTo, _T('\\'));
	if ( p != NULL ) *(++p) = 0;
	lstrcat(szFileTo, pszDestSubDir); /* buffer safe */
	CreateDirectory(szFileTo, NULL);
	lstrcat(szFileTo, pszFileName);  /* buffer safe */

	if ( !MoveFile(szFileFrom, szFileTo) && GetLastError()  ==  ERROR_ALREADY_EXISTS) {
		DeleteFile(szFileTo);
		MoveFile(szFileFrom, szFileTo);
	}
}

///////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

///////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Load(void)
{

	mir_getLP( &pluginInfo );
	
	/* existance of MS_SYSTEM_GETVERSION and MS_LANGPACK_TRANSLATESTRING
	 * is checked in MirandaPluginInfo().
	 * Not placed in MirandaPluginInfo() to avoid MessageBoxes on plugin options. 
	 * Using ANSI as LANG_UNICODE might not be supported. */

	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_TREEVIEW_CLASSES|ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icc);
	InitOptions();

	/* menu item */

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 2000089999;
	mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RELOAD));
	mi.pszName = LPGEN("Reload langpack");
	mi.pszService = MS_LANGPACK_RELOAD;
	Menu_AddMainMenuItem(&mi);

	/* installation */
	InstallFile(_T("LangMan-Readme.txt"), _T("Docs\\"));
	InstallFile(_T("LangMan-License.txt"), _T("Docs\\"));
	InstallFile(_T("LangMan-SDK.zip"), _T("Docs\\"));
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Unload(void)
{
	UninitOptions();
	return 0;
}
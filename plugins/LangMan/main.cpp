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
#include "version.h"

HINSTANCE hInst;

static HANDLE hHookModulesLoaded;
int hLangpack;

static PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX), 
	"Language Pack Manager", 
	PLUGIN_VERSION, 
	"Helps you manage Language Packs of different languages.", /* autotranslated */
	"H. Herkenrath", 
	"hrathh@users.sourceforge.net",
	"© 2005-2007 H. Herkenrath", 
	PLUGIN_WEBSITE, 
	UNICODE_AWARE, 
	0, 
	// {D4BDD1EB-56F1-4a87-A187-67246EE919A2}
	{0xd4bdd1eb, 0x56f1, 0x4a87, {0xa1, 0x87, 0x67, 0x24, 0x6e, 0xe9, 0x19, 0xa2}}, 
};

///////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, void*)
{
	if (fdwReason  ==  DLL_PROCESS_ATTACH)
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

#ifdef __cplusplus
extern "C" {
#endif 

///////////////////////////////////////////////////////////////////////////////

__declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

///////////////////////////////////////////////////////////////////////////////

static const MUUID interfaces[] = {MIID_LANGMAN, MIID_LAST};

__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{	return interfaces;
}

///////////////////////////////////////////////////////////////////////////////

static int LangManModulesLoaded(WPARAM, LPARAM)
{
	if ( ServiceExists("DBEditorpp/RegisterSingleModule"))
		CallService("DBEditorpp/RegisterSingleModule", (WPARAM)"LangMan", 0);
	return 0;
}

__declspec(dllexport) int Load(void)
{

	mir_getLP( &pluginInfo );
	
	/* existance of MS_SYSTEM_GETVERSION and MS_LANGPACK_TRANSLATESTRING
	 * is checked in MirandaPluginInfo().
	 * Not placed in MirandaPluginInfo() to avoid MessageBoxes on plugin options. 
	 * Using ANSI as LANG_UNICODE might not be supported. */
	if ( CallService(MS_SYSTEM_GETVERSION, 0, 0) < NEEDED_MIRANDA_VERSION ) {
		char szText[256];
		mir_snprintf(szText, sizeof(szText), Translate("The Language Pack Manager Plugin can not be loaded. It requires Miranda IM %hs or later."), NEEDED_MIRANDA_VERSION_STR);
		MessageBoxA(NULL, szText, Translate("Language Pack Manager Plugin"), MB_OK|MB_ICONERROR|MB_SETFOREGROUND|MB_TOPMOST|MB_TASKMODAL);
		return 1;
	}

	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_TREEVIEW_CLASSES|ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icc);
	InitOptions();

	/* menu item */

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.position = 2000089999;
	mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RELOAD));
	mi.pszName = LPGEN("Reload langpack");
	mi.pszService = MS_LANGPACK_RELOAD;
	Menu_AddMainMenuItem(&mi);

	/* installation */
	InstallFile(_T("LangMan-Readme.txt"), _T("Docs\\"));
	InstallFile(_T("LangMan-License.txt"), _T("Docs\\"));
	InstallFile(_T("LangMan-SDK.zip"), _T("Docs\\"));
	hHookModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, LangManModulesLoaded);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

__declspec(dllexport) int Unload(void)
{
	UninitOptions();
	UnhookEvent(hHookModulesLoaded);
	return 0;
}

#ifdef __cplusplus
}
#endif

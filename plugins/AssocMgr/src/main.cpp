/*

'File Association Manager'-Plugin for Miranda IM

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
along with this program (AssocMgr-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "common.h"

HINSTANCE hInst;
static HANDLE hHookModulesLoaded;
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
	// {52685CD7-0EC7-44c1-A1A6-381612418202}
	{0x52685cd7, 0xec7, 0x44c1, {0xa1, 0xa6, 0x38, 0x16, 0x12, 0x41, 0x82, 0x2}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

static void InstallFile(const TCHAR *pszFileName,const TCHAR *pszDestSubDir)
{
	TCHAR szFileFrom[MAX_PATH+1],szFileTo[MAX_PATH+1];
	if ( !GetModuleFileName(hInst,szFileFrom,SIZEOF(szFileFrom)-lstrlen(pszFileName)))
		return;

	TCHAR *p = _tcsrchr(szFileFrom,_T('\\'));
	if (p != NULL)
		*(++p) = 0;
	lstrcat(szFileFrom,pszFileName); /* buffer safe */

	HANDLE hFile = CreateFile(szFileFrom,0,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	CloseHandle(hFile);

	if ( !GetModuleFileName(NULL,szFileTo,SIZEOF(szFileTo)-lstrlen(pszDestSubDir)-lstrlen(pszFileName)))
		return;
	p = _tcsrchr(szFileTo,_T('\\'));
	if (p)
		*(++p)=0;
	lstrcat(szFileTo,pszDestSubDir); /* buffer safe */
	CreateDirectory(szFileTo,NULL);
	lstrcat(szFileTo,pszFileName);  /* buffer safe */

	if ( !MoveFile(szFileFrom,szFileTo) && GetLastError() == ERROR_ALREADY_EXISTS) {
		DeleteFile(szFileTo);
		MoveFile(szFileFrom,szFileTo);
	}
}

static int AssocMgrModulesLoaded(WPARAM,LPARAM)
{
	InitTest();
	return 0;
}

extern "C" __declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	InitAssocList();
	InitDde();

	/* installation */
	InstallFile(_T("AssocMgr-Readme.txt"),_T("Docs\\"));
	InstallFile(_T("AssocMgr-License.txt"),_T("Docs\\"));
	InstallFile(_T("AssocMgr-SDK.zip"),_T("Docs\\"));
	hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,AssocMgrModulesLoaded);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UninitTest();
	UninitDde();
	UninitAssocList();
	UnhookEvent(hHookModulesLoaded);
	return 0;
}
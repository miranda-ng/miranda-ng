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
PLUGINLINK *pluginLink;
struct MM_INTERFACE mmi;
static HANDLE hHookModulesLoaded;
int hLangpack;

static char szEmail[100] = PLUGIN_EMAIL;

static PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Language Pack Manager",
	PLUGIN_VERSION,
#if defined(_DEBUG)
	"Development build not intended for release. ("__DATE__")", /* autotranslated */
#else
	"Helps you manage Language Packs of different languages.", /* autotranslated */
#endif
	"H. Herkenrath",
	szEmail,  /* @ will be set later */
	"© 2005-2007 H. Herkenrath",
	PLUGIN_WEBSITE,
	UNICODE_AWARE,
	0,
#if defined(_UNICODE)
	// {D4BDD1EB-56F1-4a87-A187-67246EE919A2}
	{0xd4bdd1eb,0x56f1,0x4a87,{0xa1,0x87,0x67,0x24,0x6e,0xe9,0x19,0xa2}},
#else
	// {08A6937A-4906-4c23-8B9F-B3690B929D52}
	{0x8a6937a,0x4906,0x4c23,{0x8b,0x9f,0xb3,0x69,0xb,0x92,0x9d,0x52}}
#endif
};
static const MUUID interfaces[]={MIID_LANGMAN,MIID_LAST};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,void *pReserved)
{
	UNREFERENCED_PARAMETER(pReserved);
	if(fdwReason==DLL_PROCESS_ATTACH) {
		/* Do not call this from a DLL that is linked to the static C run-time library (CRT).
		 * The static CRT requires DLL_THREAD_ATTACH and DLL_THREAD_DETATCH notifications
		 * to function properly. */
		DisableThreadLibraryCalls(hInst=hinstDLL);
	}
	return TRUE;
}

static void InstallFile(const TCHAR *pszFileName,const TCHAR *pszDestSubDir)
{
	TCHAR szFileFrom[MAX_PATH+1],szFileTo[MAX_PATH+1],*p;
	HANDLE hFile;

	if ( !GetModuleFileName(hInst, szFileFrom, SIZEOF(szFileFrom) - lstrlen(pszFileName)))
		return;
	p = _tcsrchr(szFileFrom,_T('\\'));
	if ( p != NULL ) *(++p)=0;
	lstrcat(szFileFrom, pszFileName); /* buffer safe */

	hFile = CreateFile(szFileFrom, 0, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if ( hFile == INVALID_HANDLE_VALUE ) return;
	CloseHandle(hFile);

	if ( !GetModuleFileName(NULL, szFileTo, SIZEOF(szFileTo) - lstrlen(pszDestSubDir) - lstrlen(pszFileName)))
		return;
	p = _tcsrchr(szFileTo, _T('\\'));
	if ( p != NULL ) *(++p)=0;
	lstrcat(szFileTo, pszDestSubDir); /* buffer safe */
	CreateDirectory(szFileTo, NULL);
	lstrcat(szFileTo, pszFileName);  /* buffer safe */

	if ( !MoveFile(szFileFrom, szFileTo) && GetLastError() == ERROR_ALREADY_EXISTS) {
		DeleteFile(szFileTo);
		MoveFile(szFileFrom, szFileTo);
	}
}

static int LangManModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	if ( ServiceExists("DBEditorpp/RegisterSingleModule"))
		CallService("DBEditorpp/RegisterSingleModule", (WPARAM)"LangMan", 0);
	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif 

__declspec(dllexport) const PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	if(mirandaVersion<PLUGIN_MAKE_VERSION(0,1,0,1)) return NULL;
	pluginInfo.cbSize = sizeof(PLUGININFO); /* needed as v0.6 does equality check */
	szEmail[PLUGIN_EMAIL_ATT_POS-1] = '@';
	return (PLUGININFO*)&pluginInfo; /* header is the same */
}

__declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	UNREFERENCED_PARAMETER(mirandaVersion);
	pluginInfo.cbSize = sizeof(PLUGININFOEX);
	szEmail[PLUGIN_EMAIL_ATT_POS-1] = '@';
	return &pluginInfo;
}

__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

__declspec(dllexport) int Load(PLUGINLINK *link)
{
	INITCOMMONCONTROLSEX icc;
	pluginLink = link;
	mir_getLP(&pluginInfo);
	
	/* existance of MS_SYSTEM_GETVERSION and MS_LANGPACK_TRANSLATESTRING
	 * is checked in MirandaPluginInfo().
	 * Not placed in MirandaPluginInfo() to avoid MessageBoxes on plugin options. 
	 * Using ANSI as LANG_UNICODE might not be supported. */
	if ( CallService(MS_SYSTEM_GETVERSION, 0, 0) < NEEDED_MIRANDA_VERSION ) {
		char szText[256];
		mir_snprintf(szText,sizeof(szText),Translate("The Language Pack Manager Plugin can not be loaded. It requires Miranda IM %hs or later."),NEEDED_MIRANDA_VERSION_STR);
		MessageBoxA(NULL,szText,Translate("Language Pack Manager Plugin"),MB_OK|MB_ICONERROR|MB_SETFOREGROUND|MB_TOPMOST|MB_TASKMODAL);
		return 1;
	}
	if ( mir_getMMI(&mmi)) return 1;
	icc.dwSize = sizeof(icc);
	icc.dwICC=ICC_TREEVIEW_CLASSES|ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icc);
	InitOptions();

	/* installation */
	InstallFile(_T("LangMan-Readme.txt"),_T("Docs\\"));
	InstallFile(_T("LangMan-License.txt"),_T("Docs\\"));
	InstallFile(_T("LangMan-SDK.zip"),_T("Docs\\"));
	hHookModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, LangManModulesLoaded);
	return 0;
}

__declspec(dllexport) int Unload(void)
{
	UninitOptions();
	UnhookEvent(hHookModulesLoaded);
	return 0;
}

#ifdef __cplusplus
}
#endif

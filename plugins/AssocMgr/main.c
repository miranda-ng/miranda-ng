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
#include "version.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;
struct MM_INTERFACE mmi;
struct UTF8_INTERFACE utfi;
static HANDLE hHookModulesLoaded;
int hLangpack;

static char szPluginInfoEmail[]=PLUGIN_EMAIL;
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"File Association Manager",
	PLUGIN_VERSION,
#if defined(_DEBUG)
	"Development build not intended for release. ("__DATE__")", /* autotranslated */
#else
	"Handles file type associations and URLs like aim, ymsgr, xmpp, wpmsg, gg, tlen.", /* autotranslated */
#endif
	"H. Herkenrath",
	szPluginInfoEmail,  /* @ will be set later */
	"© 2005-2007 H. Herkenrath",
	PLUGIN_WEBSITE,
	UNICODE_AWARE,
	0,
#if defined(_UNICODE)
	// {52685CD7-0EC7-44c1-A1A6-381612418202}
	{0x52685cd7,0xec7,0x44c1,{0xa1,0xa6,0x38,0x16,0x12,0x41,0x82,0x2}},
#else
	// {48692828-D4BA-43b5-BF81-44F384811569}
	{0x48692828,0xd4ba,0x43b5,{0xbf,0x81,0x44,0xf3,0x84,0x81,0x15,0x69}}
#endif
};
static const MUUID interfaces[]={MIID_ASSOCMGR,MIID_AUTORUN,MIID_LAST};

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

	if(!GetModuleFileName(hInst,szFileFrom,SIZEOF(szFileFrom)-lstrlen(pszFileName)))
		return;
	p=_tcsrchr(szFileFrom,_T('\\'));
	if(p!=NULL) *(++p)=0;
	lstrcat(szFileFrom,pszFileName); /* buffer safe */

	hFile=CreateFile(szFileFrom,0,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
	if(hFile==INVALID_HANDLE_VALUE) return;
	CloseHandle(hFile);

	if(!GetModuleFileNameWorkaround(NULL,szFileTo,SIZEOF(szFileTo)-lstrlen(pszDestSubDir)-lstrlen(pszFileName)))
		return;
	p=_tcsrchr(szFileTo,_T('\\'));
	if(p!=NULL) *(++p)=0;
	lstrcat(szFileTo,pszDestSubDir); /* buffer safe */
	CreateDirectory(szFileTo,NULL);
	lstrcat(szFileTo,pszFileName);  /* buffer safe */

	if(!MoveFile(szFileFrom,szFileTo) && GetLastError()==ERROR_ALREADY_EXISTS) {
		DeleteFile(szFileTo);
		MoveFile(szFileFrom,szFileTo);
	}
}

static int AssocMgrModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	if(ServiceExists("DBEditorpp/RegisterSingleModule"))
		CallService("DBEditorpp/RegisterSingleModule",(WPARAM)"AssocMgr",0);
	InitTest();
	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif 

__declspec(dllexport) const PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	if(mirandaVersion<PLUGIN_MAKE_VERSION(0,1,0,1)) return NULL;
	pluginInfo.cbSize=sizeof(PLUGININFO); /* needed as v0.6 does equality check */
	szPluginInfoEmail[PLUGIN_EMAIL_ATT_POS-1]='@'; /* email obfuscated */
	return (PLUGININFO*)&pluginInfo; /* header is the same */
}

__declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	UNREFERENCED_PARAMETER(mirandaVersion);
	pluginInfo.cbSize=sizeof(PLUGININFOEX);
	szPluginInfoEmail[PLUGIN_EMAIL_ATT_POS-1]='@'; /* email obfuscated */
	return &pluginInfo;
}

__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

__declspec(dllexport) int Load(PLUGINLINK *link)
{
	pluginLink=link;
	mir_getLP(&pluginInfo);

	/* existance of MS_SYSTEM_GETVERSION and MS_LANGPACK_TRANSLATESTRING
	 * is checked in MirandaPluginInfo().
	 * Not placed in MirandaPluginInfo() to avoid MessageBoxes on plugin options. 
	 * Using ANSI as LANG_UNICODE might not be supported. */
	if(CallService(MS_SYSTEM_GETVERSION,0,0)<NEEDED_MIRANDA_VERSION) {
		char szText[256];
		mir_snprintf(szText,sizeof(szText),Translate("The File Association Manager Plugin can not be loaded. It requires Miranda IM %hs or later."),NEEDED_MIRANDA_VERSION_STR);
		MessageBoxA(NULL,szText,Translate("File Association Manager Plugin"),MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST|MB_TASKMODAL);
		return 1;
	}

	if(!ServiceExists(MS_DB_CONTACT_GETSETTING_STR)) return 1; /* dbx3x v0.5.1.0 */
	if(mir_getMMI(&mmi)) return 1;
	if(mir_getUTFI(&utfi)) return 1;
	InitAssocList();
	InitDde();

	/* installation */
	InstallFile(_T("AssocMgr-Readme.txt"),_T("Docs\\"));
	InstallFile(_T("AssocMgr-License.txt"),_T("Docs\\"));
	InstallFile(_T("AssocMgr-SDK.zip"),_T("Docs\\"));
	hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,AssocMgrModulesLoaded);
	return 0;
}

__declspec(dllexport) int Unload(void)
{
	UninitTest();
	UninitDde();
	UninitAssocList();
	UnhookEvent(hHookModulesLoaded);
	return 0;
}

#ifdef __cplusplus
}
#endif

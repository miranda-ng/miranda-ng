/*

'AutoShutdown'-Plugin for Miranda IM

Copyright 2004-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Shutdown-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "common.h"
#include "version.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;
struct MM_INTERFACE mmi;
static HANDLE hHookModulesLoaded;
HANDLE hActiveIcon,hInactiveIcon;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"AutoShutdown",
	PLUGIN_VERSION,
#if defined(_DEBUG)
	"Development build not intended for release. ("__DATE__")",
#else
	"Adds the possibility to shutdown the computer when a specified event occurs.", /* autotranslated */
#endif
	"H. Herkenrath",
	PLUGIN_EMAIL,  /* @ will be set later */
	"© 2004-2007 H. Herkenrath",
	PLUGIN_WEBSITE,
	UNICODE_AWARE,
	0,
#if defined(_UNICODE)
	// {9DE24579-5C5C-49aa-80E8-4D38E4344E63}
	{0x9de24579,0x5c5c,0x49aa,{0x80,0xe8,0x4d,0x38,0xe4,0x34,0x4e,0x63}},
#else
	// {7C0DD208-94D8-4283-879A-E6F86135B826}
	{0x7c0dd208,0x94d8,0x4283,{0x87,0x9a,0xe6,0xf8,0x61,0x35,0xb8,0x26}}
#endif
};
static const MUUID interfaces[]={MIID_SHUTDOWN,MIID_LAST};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,void *pReserved)
{
	UNREFERENCED_PARAMETER(pReserved);
	if(fdwReason==DLL_PROCESS_ATTACH)
		/* Do not call this function from a DLL that is linked to the static C run-time library (CRT).
		 * The static CRT requires DLL_THREAD_ATTACH and DLL_THREAD_DETATCH notifications to function properly. */
		DisableThreadLibraryCalls(hInst=hinstDLL);
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

	if(!GetModuleFileName(NULL,szFileTo,SIZEOF(szFileTo)-lstrlen(pszDestSubDir)-lstrlen(pszFileName)))
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

static int ShutdownModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	if(ServiceExists("DBEditorpp/RegisterSingleModule"))
		CallService("DBEditorpp/RegisterSingleModule",(WPARAM)"AutoShutdown",0);
	/* merged thundershutdown plugin */
	if(GetModuleHandleA("tshutdown.dll")) { 
		DBWriteContactSettingByte(NULL,"PluginDisable","tshutdown.dll",1);
		DBWriteContactSettingByte(NULL,"AutoShutdown","WeatherShutdown",1);
	}
	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif 

__declspec(dllexport) const PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	if(mirandaVersion<PLUGIN_MAKE_VERSION(0,1,0,1)) return NULL;
	pluginInfo.cbSize=sizeof(PLUGININFO); /* needed as v0.6 does equality check */
	/* email obfuscated, made .rdata writable */
	pluginInfo.authorEmail[PLUGIN_EMAIL_ATT_POS-1]='@';
	return (PLUGININFO*)&pluginInfo; /* header is the same */
}

__declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	UNREFERENCED_PARAMETER(mirandaVersion);
	pluginInfo.cbSize=sizeof(PLUGININFOEX);
	/* email obfuscated, made .rdata writable */
	pluginInfo.authorEmail[PLUGIN_EMAIL_ATT_POS-1]='@';
	return &pluginInfo;
}

__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

__declspec(dllexport) int Load(PLUGINLINK *link)
{
	INITCOMMONCONTROLSEX icc;
	pluginLink=link;

	/* existance of MS_SYSTEM_GETVERSION and MS_LANGPACK_TRANSLATESTRING
	 * is checked in MirandaPluginInfo().
	 * Not placed in MirandaPluginInfo() to avoid MessageBoxes on plugin options. 
	 * Using ANSI as LANG_UNICODE might not be supported. */
	if(CallService(MS_SYSTEM_GETVERSION,0,0)<NEEDED_MIRANDA_VERSION) {
		char szText[256];
		mir_snprintf(szText,sizeof(szText),Translate("The AutoShutdown plugin can not be loaded. It requires Miranda IM %hs or later."),NEEDED_MIRANDA_VERSION_STR);
		MessageBoxA(NULL,szText,Translate("AutoShutdown Plugin"),MB_OK|MB_ICONERROR|MB_SETFOREGROUND|MB_TOPMOST|MB_TASKMODAL);
		return 1;
	}
	if(!ServiceExists(MS_DB_CONTACT_GETSETTING_STR)) return 1; /* dbx3x v0.5.1.0 */
	if(mir_getMMI(&mmi)) return 1;
	icc.dwSize=sizeof(icc);
	icc.dwICC=ICC_DATE_CLASSES|ICC_UPDOWN_CLASS|ICC_PROGRESS_CLASS;
	if(!InitCommonControlsEx(&icc)) return 1;

	if(InitFrame()) return 1; /* before icons */

	/* shared */
	IcoLib_AddIconRes("AutoShutdown_Header",TranslateT("Automatic Shutdown"),TranslateT("Header"),hInst,IDI_HEADER,TRUE);
	hActiveIcon=IcoLib_AddIconRes("AutoShutdown_Active",TranslateT("Automatic Shutdown"),TranslateT("Active"),hInst,IDI_ACTIVE,FALSE);
	hInactiveIcon=IcoLib_AddIconRes("AutoShutdown_Inactive",TranslateT("Automatic Shutdown"),TranslateT("Inactive"),hInst,IDI_INACTIVE,FALSE);

	InitShutdownSvc();
	InitWatcher(); /* before InitSettingsDlg() */
	InitSettingsDlg();
	InitOptions();

	/* installation */
	InstallFile(_T("Shutdown-Readme.txt"),_T("Docs\\"));
	InstallFile(_T("Shutdown-License.txt"),_T("Docs\\"));
	InstallFile(_T("Shutdown-SDK.zip"),_T("Docs\\"));
	InstallFile(_T("countdown.wav"),_T("Sounds\\"));
	hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,ShutdownModulesLoaded);
	return 0;
}

__declspec(dllexport) int Unload(void)
{
	UnhookEvent(hHookModulesLoaded);

	UninitOptions();
	UninitSettingsDlg(); /* before UninitWatcher() */
	UninitWatcher(); /* before UninitFrame() */
	UninitFrame();
	UninitShutdownSvc();
	return 0;
}

#ifdef __cplusplus
}
#endif 

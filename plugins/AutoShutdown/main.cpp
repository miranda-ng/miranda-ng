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
static HANDLE hHookModulesLoaded;
HANDLE hActiveIcon,hInactiveIcon;
int hLangpack;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"AutoShutdown",
	PLUGIN_VERSION,
	"Adds the possibility to shutdown the computer when a specified event occurs.", /* autotranslated */
	"H. Herkenrath",
	"hrathh@users.sourceforge.net",  /* @ will be set later */
	"© 2004-2007 H. Herkenrath",
	"http://addons.miranda-im.org/details.php?action=viewfile&id=3056",
	UNICODE_AWARE,
	0,
	// {9DE24579-5C5C-49aa-80E8-4D38E4344E63}
	{0x9de24579,0x5c5c,0x49aa,{0x80,0xe8,0x4d,0x38,0xe4,0x34,0x4e,0x63}},
};

static const MUUID interfaces[]={MIID_SHUTDOWN,MIID_LAST};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, VOID *pReserved)
{
	hInst = hinstDLL;
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

extern "C" __declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	INITCOMMONCONTROLSEX icc;
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

extern "C" __declspec(dllexport) int Unload(void)
{
	UnhookEvent(hHookModulesLoaded);

	UninitOptions();
	UninitSettingsDlg(); /* before UninitWatcher() */
	UninitWatcher(); /* before UninitFrame() */
	UninitFrame();
	UninitShutdownSvc();
	return 0;
}
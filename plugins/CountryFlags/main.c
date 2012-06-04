/*
Miranda IM Country Flags Plugin
Copyright (C) 2006-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Flags-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "flags.h"
#include "version.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;
struct MM_INTERFACE mmi;
int nCountriesCount;
struct CountryListEntry *countries;
int hLangpack;
static char szEmail[100] = PLUGIN_EMAIL;

static PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Country Flags",
	PLUGIN_VERSION,
#if defined(_DEBUG)
	"Development build not intended for release. ("__DATE__")",  /* autotranslated */
#else
	"Service offering misc country utilities as flag icons and a IP-to-Country database.",  /* autotranslated */
#endif
	"H. Herkenrath",
	szEmail,  /* @ will be set later */
	"© 2006-2007 H. Herkenrath",
	PLUGIN_WEBSITE,
	UNICODE_AWARE,
	0,
#if defined(_UNICODE)
	// {68C36842-3D95-4f4a-AB81-014D6593863B}
	{0x68c36842,0x3d95,0x4f4a,{0xab,0x81,0x1,0x4d,0x65,0x93,0x86,0x3b}},
#else
	// {E0C4681C-E680-4262-8B44-7A9540C064FF}
	{0xe0c4681c,0xe680,0x4262,{0x8b,0x44,0x7a,0x95,0x40,0xc0,0x64,0xff}}
#endif
};
static const MUUID interfaces[]={MIID_FLAGS,MIID_LAST};

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

	if (!GetModuleFileName(hInst,szFileFrom,SIZEOF(szFileFrom)-lstrlen(pszFileName)))
		return;
	p=_tcsrchr(szFileFrom,_T('\\'));
	if(p!=NULL) *(++p)=0;
	lstrcat(szFileFrom,pszFileName); /* buffer safe */

	hFile=CreateFile(szFileFrom,0,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
	if(hFile==INVALID_HANDLE_VALUE) return;
	CloseHandle(hFile);

	if (!GetModuleFileName(NULL,szFileTo,SIZEOF(szFileTo)-lstrlen(pszDestSubDir)-lstrlen(pszFileName)))
		return;
	p=_tcsrchr(szFileTo,_T('\\'));
	if(p!=NULL) *(++p)=0;
	lstrcat(szFileTo,pszDestSubDir); /* buffer safe */
	CreateDirectory(szFileTo,NULL);
	lstrcat(szFileTo,pszFileName);  /* buffer safe */

	if (!MoveFile(szFileFrom,szFileTo) && GetLastError()==ERROR_ALREADY_EXISTS) {
		DeleteFile(szFileTo);
		MoveFile(szFileFrom,szFileTo);
	}
}

#ifdef __cplusplus
extern "C" {
#endif 

__declspec(dllexport) const PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	if(mirandaVersion<PLUGIN_MAKE_VERSION(0,1,0,1)) return NULL;
	pluginInfo.cbSize=sizeof(PLUGININFO); /* needed as v0.6 does equality check */
	/* email obfuscated, made .rdata writable */
	szEmail[PLUGIN_EMAIL_ATT_POS-1] = '@';
	return (PLUGININFO*)&pluginInfo; /* header is the same */
}

__declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	UNREFERENCED_PARAMETER(mirandaVersion);
	pluginInfo.cbSize=sizeof(PLUGININFOEX);
	/* email obfuscated, made .rdata writable */
	szEmail[PLUGIN_EMAIL_ATT_POS-1] = '@';
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
		mir_snprintf(szText,SIZEOF(szText),Translate("The Country Flags Plugin can not be loaded. It requires Miranda IM %hs or later."),NEEDED_MIRANDA_VERSION_STR);
		MessageBoxA(NULL,szText,Translate("Country Flags Plugin"),MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST|MB_TASKMODAL);
		return 1;
	}
	if(mir_getMMI(&mmi)) return 1;

	PrepareBufferedFunctions();
	InitCountryListExt();
	if(CallService(MS_UTILS_GETCOUNTRYLIST,(WPARAM)&nCountriesCount,(LPARAM)&countries))
		nCountriesCount=0;
	InitIcons();
	InitIpToCountry();
	InitExtraImg();

	/* installation */
	InstallFile(_T("Flags-Readme.txt"),_T("Docs\\"));
	InstallFile(_T("Flags-License.txt"),_T("Docs\\"));
	InstallFile(_T("Flags-SDK.zip"),_T("Docs\\"));
	return 0;
}

__declspec(dllexport) int Unload(void)
{
	KillBufferedFunctions();
	UninitExtraImg();
	UninitIpToCountry();
	UninitIcons();
	UninitCountryListExt();
	return 0;
}

#ifdef __cplusplus
}
#endif 

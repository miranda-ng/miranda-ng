/* 
Copyright (C) 2012 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "Common.h"

HINSTANCE hInst = NULL;
PLUGINLINK *pluginLink;
int hLangpack;
struct MM_INTERFACE mmi;
HANDLE hOptHook = NULL,  hLoadHook = NULL, hOnPreShutdown = NULL, hPrebuildMenuHook = NULL, hPackUpdaterFolder = NULL;
HANDLE hProtoService[7];
HWND hAddFeedDlg;
HWND hChangeFeedDlg;
XML_API xi = {0};
struct UTF8_INTERFACE utfi;
TCHAR tszRoot[MAX_PATH] = {0};
HANDLE hUpdateMutex;
#define NUM_SERVICES 6
HANDLE hService[NUM_SERVICES];

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	0,
	// {56CC3F29-CCBF-4546-A8BA-9856248A412A}
	{0x56cc3f29, 0xccbf, 0x4546, {0xa8, 0xba, 0x98, 0x56, 0x24, 0x8a, 0x41, 0x2a}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if(mirandaVersion < 0x0900)
	{
		MessageBox(NULL, TranslateT("News Aggregator plugin requires Miranda IM 0.9.0.0 or later"), TranslateT("Fatal error"), MB_OK);
		return NULL;
	}
	return &pluginInfoEx;
}

static const MUUID interfaces[] = {{0x29517be5, 0x779a, 0x48e5, {0x89, 0x50, 0xcb, 0x4d, 0xe1, 0xd4, 0x31, 0x72}}, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" __declspec(dllexport) int Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getLP(&pluginInfoEx);
	mir_getMMI(&mmi);
	mir_getXI(&xi);
	mir_getUTFI(&utfi);

	if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
	{
		hPackUpdaterFolder = FoldersRegisterCustomPathT("News Aggregator", "Avatars", MIRANDA_USERDATAT _T("\\Avatars\\")_T(DEFAULT_AVATARS_FOLDER));
		FoldersGetCustomPathT(hPackUpdaterFolder, tszRoot, MAX_PATH, _T(""));
	}
	else
	{
		TCHAR* tszFolder = Utils_ReplaceVarsT(_T("%miranda_userdata%\\"_T(DEFAULT_AVATARS_FOLDER)));
		lstrcpyn(tszRoot, tszFolder, SIZEOF(tszRoot));
		mir_free(tszFolder);
	}
	
	// Add options hook
	hOptHook = HookEvent(ME_OPT_INITIALISE, OptInit);
	hLoadHook = HookEvent(ME_SYSTEM_MODULESLOADED, NewsAggrInit);
	hOnPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, NewsAggrPreShutdown);

	hUpdateMutex = CreateMutex(NULL, FALSE, NULL);

	// register weather protocol
	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize = PROTOCOLDESCRIPTOR_V3_SIZE;
	pd.szName = MODULE;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);

	hProtoService[0] = CreateProtoServiceFunction(MODULE, PS_GETNAME, NewsAggrGetName);
	hProtoService[1] = CreateProtoServiceFunction(MODULE, PS_GETCAPS, NewsAggrGetCaps);
	hProtoService[2] = CreateProtoServiceFunction(MODULE, PS_SETSTATUS, NewsAggrSetStatus);
	hProtoService[3] = CreateProtoServiceFunction(MODULE, PS_GETSTATUS, NewsAggrGetStatus);
	hProtoService[4] = CreateProtoServiceFunction(MODULE, PS_LOADICON, NewsAggrLoadIcon);
	hProtoService[5] = CreateProtoServiceFunction(MODULE, PSS_GETINFO, NewsAggrGetInfo);
	hProtoService[6] = CreateProtoServiceFunction(MODULE, PS_GETAVATARINFO, NewsAggrGetAvatarInfo);

	hService[0] = CreateServiceFunction(MS_NEWSAGGR_CHECKALLFEEDS, CheckAllFeeds);
	hService[1] = CreateServiceFunction(MS_NEWSAGGR_ADDFEED, AddFeed);
	hService[2] = CreateServiceFunction(MS_NEWSAGGR_IMPORTFEEDS, ImportFeeds);
	hService[3] = CreateServiceFunction(MS_NEWSAGGR_EXPORTFEEDS, ExportFeeds);
	hService[4] = CreateServiceFunction(MS_NEWSAGGR_CHECKFEED, CheckFeed);
	hService[5] = CreateServiceFunction(MS_NEWSAGGR_CHANGEFEED, ChangeFeed);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	for (int i = 0;i<NUM_SERVICES;i++)
		DestroyServiceFunction(hService[i]);

	UnhookEvent(hOptHook);
	UnhookEvent(hLoadHook);
	UnhookEvent(hOnPreShutdown);

	DestroyUpdateList();
	CloseHandle(hUpdateMutex);

	return 0;
}
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

int hLangpack;
HANDLE hPrebuildMenuHook = NULL;
HWND hAddFeedDlg;
HANDLE hChangeFeedDlgList = NULL;
XML_API xi = {0};
TCHAR tszRoot[MAX_PATH] = {0};
HANDLE hUpdateMutex;

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
	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);
	mir_getXI(&xi);

	// Add options hook
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, NewsAggrInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, NewsAggrPreShutdown);

	hUpdateMutex = CreateMutex(NULL, FALSE, NULL);
	hChangeFeedDlgList = WindowList_Create();

	// register weather protocol
	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = MODULE;
	pd.type = PROTOTYPE_VIRTUAL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	CreateProtoServiceFunction(MODULE, PS_GETNAME, NewsAggrGetName);
	CreateProtoServiceFunction(MODULE, PS_GETCAPS, NewsAggrGetCaps);
	CreateProtoServiceFunction(MODULE, PS_SETSTATUS, NewsAggrSetStatus);
	CreateProtoServiceFunction(MODULE, PS_GETSTATUS, NewsAggrGetStatus);
	CreateProtoServiceFunction(MODULE, PS_LOADICON, NewsAggrLoadIcon);
	CreateProtoServiceFunction(MODULE, PSS_GETINFO, NewsAggrGetInfo);
	CreateProtoServiceFunction(MODULE, PS_GETAVATARINFOT, NewsAggrGetAvatarInfo);
	CreateProtoServiceFunction(MODULE, PSR_MESSAGE, NewsAggrRecvMessage);

	CreateServiceFunction(MS_NEWSAGGREGATOR_CHECKALLFEEDS, CheckAllFeeds);
	CreateServiceFunction(MS_NEWSAGGREGATOR_ADDFEED, AddFeed);
	CreateServiceFunction(MS_NEWSAGGREGATOR_IMPORTFEEDS, ImportFeeds);
	CreateServiceFunction(MS_NEWSAGGREGATOR_EXPORTFEEDS, ExportFeeds);
	CreateServiceFunction(MS_NEWSAGGREGATOR_CHECKFEED, CheckFeed);
	CreateServiceFunction(MS_NEWSAGGREGATOR_CHANGEFEED, ChangeFeed);
	CreateServiceFunction(MS_NEWSAGGREGATOR_ENABLED, EnableDisable);

	HOTKEYDESC hkd = { sizeof(hkd) };
	hkd.dwFlags = HKD_TCHAR;
	hkd.pszName = "NewsAggregator/CheckAllFeeds";
	hkd.ptszDescription = LPGENT("Check All Feeds");
	hkd.ptszSection = LPGENT("News Aggregator");
	hkd.pszService = MS_NEWSAGGREGATOR_CHECKALLFEEDS;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL+HKCOMB_A, 'O') | HKF_MIRANDA_LOCAL;
	Hotkey_Register(&hkd);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	WindowList_Destroy(hChangeFeedDlgList);
	DestroyUpdateList();
	CloseHandle(hUpdateMutex);
	return 0;
}

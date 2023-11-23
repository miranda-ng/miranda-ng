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

#include "stdafx.h"

HANDLE hPrebuildMenuHook = nullptr;
CDlgBase *pAddFeedDialog = nullptr;
wchar_t tszRoot[MAX_PATH] = {0};
HANDLE hUpdateMutex;

LIST<CFeedEditor> g_arFeeds(1, PtrKeySortT);

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {56CC3F29-CCBF-4546-A8BA-9856248A412A}
	{0x56cc3f29, 0xccbf, 0x4546, {0xa8, 0xba, 0x98, 0x56, 0x24, 0x8a, 0x41, 0x2a}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{
	RegisterProtocol(PROTOTYPE_VIRTUAL);
	SetUniqueId("URL");
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	// Add options hook
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, NewsAggrInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, NewsAggrPreShutdown);

	hUpdateMutex = CreateMutex(nullptr, FALSE, nullptr);

	CreateProtoServiceFunction(MODULENAME, PS_GETNAME, NewsAggrGetName);
	CreateProtoServiceFunction(MODULENAME, PS_GETCAPS, NewsAggrGetCaps);
	CreateProtoServiceFunction(MODULENAME, PS_SETSTATUS, NewsAggrSetStatus);
	CreateProtoServiceFunction(MODULENAME, PS_GETSTATUS, NewsAggrGetStatus);
	CreateProtoServiceFunction(MODULENAME, PS_LOADICON, NewsAggrLoadIcon);
	CreateProtoServiceFunction(MODULENAME, PSS_GETINFO, NewsAggrGetInfo);
	CreateProtoServiceFunction(MODULENAME, PS_GETAVATARINFO, NewsAggrGetAvatarInfo);
	CreateProtoServiceFunction(MODULENAME, PSR_MESSAGE, NewsAggrRecvMessage);

	CreateServiceFunction(MS_NEWSAGGREGATOR_CHECKALLFEEDS, CheckAllFeeds);
	CreateServiceFunction(MS_NEWSAGGREGATOR_ADDFEED, AddFeed);
	CreateServiceFunction(MS_NEWSAGGREGATOR_IMPORTFEEDS, ImportFeeds);
	CreateServiceFunction(MS_NEWSAGGREGATOR_EXPORTFEEDS, ExportFeeds);
	CreateServiceFunction(MS_NEWSAGGREGATOR_CHECKFEED, CheckFeed);
	CreateServiceFunction(MS_NEWSAGGREGATOR_CHANGEFEED, ChangeFeed);
	CreateServiceFunction(MS_NEWSAGGREGATOR_ENABLED, EnableDisable);

	HOTKEYDESC hkd = {};
	hkd.dwFlags = HKD_UNICODE;
	hkd.pszName = "NewsAggregator/CheckAllFeeds";
	hkd.szDescription.w = LPGENW("Check All Feeds");
	hkd.szSection.w = LPGENW("News Aggregator");
	hkd.pszService = MS_NEWSAGGREGATOR_CHECKALLFEEDS;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL+HKCOMB_A, 'O') | HKF_MIRANDA_LOCAL;
	g_plugin.addHotkey(&hkd);

	InitIcons();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	DestroyUpdateList();
	CloseHandle(hUpdateMutex);
	return 0;
}

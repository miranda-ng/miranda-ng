////////////////////////////////////////////////////////////////////////
// NewStory -- new history viewer for Miranda IM
// (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
// Visit http://miranda-im.org/ for details on Miranda Instant Messenger
////////////////////////////////////////////////////////////////////////
// File: main.cpp
// Created by: Victor Pavlychko
// Description:
//    Main module. Responsible for startup/cleanup and Miranda bindings
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

HANDLE hhkModulesLoaded = 0, hhkOptInitialise = 0, hhkTTBLoaded = 0;

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {1AD11C86-CAAB-4372-A0A4-8B1168D51B9E}
	{ 0x1ad11c86, 0xcaab, 0x4372, { 0xa0, 0xa4, 0x8b, 0x11, 0x68, 0xd5, 0x1b, 0x9e } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_UIHISTORY, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

int evtModulesLoaded(WPARAM, LPARAM)
{
	InitIcons();
	InitFonts();
	InitNewstoryControl();
	InitHistory();
	InitOptions();
	LoadTemplates();

	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE;

	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	mi.name.w = L"User Newstory";
	mi.position = 1999990000;
	mi.hIcon = GetIcon(ICO_NEWSTORY);
	Menu_AddContactMenuItem(&mi);

	mi.pszService = "Newstory/System";
	mi.name.w = L"System Newstory";
	mi.position = 1999990000;
	mi.hIcon = GetIcon(ICO_NEWSTORY);
	Menu_AddMainMenuItem(&mi);

	return 0;
}

int CMPlugin::Load()
{
	CreateServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY, svcShowNewstory);
	CreateServiceFunction("Newstory/System", svcShowSystemNewstory);

	hhkModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, evtModulesLoaded);
	/*
		hhkOptInitialise = HookEvent(ME_OPT_INITIALISE, OptInitialise);

		options.fnup = options.fndown = 0;
		options.flags = 0;
		LoadOptions();
	*/
	return 0;
}

//void DoCleanup();

int CMPlugin::Unload()
{
	UnhookEvent(hhkModulesLoaded);
	/*
		UnhookEvent(hhkOptInitialise);
		if (hhkTTBLoaded)
			UnhookEvent(hhkTTBLoaded);

		DoCleanup();
	*/
	FreeHistory();
	return 0;
}

/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "Common.h"

HINSTANCE hInst;
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
	// {08B86253-EC6E-4D09-B7A9-64ACDF0627B8}
    {0x8b86253, 0xec6e, 0x4d09, {0xb7, 0xa9, 0x64, 0xac, 0xdf, 0x6, 0x27, 0xb8}}
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

static INT_PTR PluginMenuCommand(WPARAM wParam, LPARAM lParam)
{
	try
	{
		SoundNotifyDataStorage storage;
		storage.init();
		SettingsDialog dlg(storage);
		if (dlg.DoModal() == IDOK)
			storage.commit();	
	}
	catch (...)
	{
		MessageBox(0, TEXT("Unknown error occured while configuring"), TEXT("XSoundNotify Error"), MB_OK | MB_ICONERROR);
	}

	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

INT processEvent(WPARAM wParam, LPARAM lParam)
{
	try
	{
		if (!isReceiveMessage(lParam))
			return 0;
						
		std::tstring sound = getContactSound((HANDLE)wParam);
		if (!sound.empty())		
			PlaySound(sound.c_str(), nullptr, SND_ASYNC | SND_FILENAME);		
	}
	catch (std::exception & )
	{
		// log 
	}
	catch (...)
	{
		// log
	}

	return 0;
}

extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfo);

	CLISTMENUITEM mi;

	CreateServiceFunction("XSoundNotify/MenuCommand", PluginMenuCommand);
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = -0x7FFFFFFF;
	mi.flags = 0;
	mi.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.pszName = LPGEN("&XSoundNotify Plugin");
	mi.pszService = "XSoundNotify/MenuCommand";
	Menu_AddMainMenuItem(&mi);
	HookEvent(ME_DB_EVENT_ADDED, processEvent);	

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}


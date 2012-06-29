/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "SettingsDialog.h"

#include <windows.h>
#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_database.h>
#include <m_protocols.h>
#include "xsn_utils.h"
//#include "DebugLogger.hpp"

HINSTANCE hInst;
PLUGINLINK *pluginLink;
HANDLE AddEventHook;
	
PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),                // Размер стуктуры
	"XSoundNotify",                		// Короткое имя плагинв
	PLUGIN_MAKE_VERSION(1,0,0,0),            // Версия плагина
	"Provides extended options for sound notifications",                 // Описание плагина
	"Roman Torsten",                // Автор
	"rs_torsten-public@yahoo.com",                // Адресс автора
	"© 2011 Copyright",                // Копирайт
	"https://plus.google.com/117081718640940130539",        // Адресс сайта разработчика плагина
	0,
	0,
    {0x8b86253, 0xec6e, 0x4d09, { 0xb7, 0xa9, 0x64, 0xac, 0xdf, 0x6, 0x27, 0xb8 }} //{08B86253-EC6E-4d09-B7A9-64ACDF0627B8}
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

static int PluginMenuCommand(WPARAM wParam,LPARAM lParam)
{
	try
	{
		SoundNotifyDataStorage storage(pluginLink);
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

static const MUUID interfaces[] = {MIID_TESTPLUGIN, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

INT processEvent(WPARAM wParam, LPARAM lParam)
{
	try
	{
		if (!isReceiveMessage(lParam))
			return 0;
						
		xsn_string sound = getContactSound((HANDLE)wParam);
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

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	CLISTMENUITEM mi;

	pluginLink=link;
	CreateServiceFunction("XSoundNotify/MenuCommand", PluginMenuCommand);
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-0x7FFFFFFF;
	mi.flags=0;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.pszName=LPGEN("&XSoundNotify Plugin");
	mi.pszService="XSoundNotify/MenuCommand";
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);
	AddEventHook = HookEvent(ME_DB_EVENT_ADDED, processEvent);	

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	UnhookEvent(AddEventHook);
	return 0;
}


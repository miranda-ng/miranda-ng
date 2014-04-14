/**
 * @file Name day plugin
 */
#include <windows.h>
#include <newpluginapi.h>

#include "name_day_core.h"

static	name_day_core_t		name_day_core;

HINSTANCE hInst = NULL;
PLUGINLINK *pluginLink = NULL;


/**
 * @brief Plugin info.
 *
 */
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
  #ifdef _WIN64
	"Name Day x64",
  #else
	"Name Day",
  #endif    	
	PLUGIN_MAKE_VERSION(0, 0, 1, 0),
	"The Name Day plugin.",
	"Tibor Szabo, Robert Pösel",
	"robyer@seznam.cz",
	"© 2005 Tibor Szabo, © 2011 Robert Pösel",
	"http://code.google.com/p/robyer",
	UNICODE_AWARE, //not transient
	0,             //doesn't replace anything built-in
	// {E3FFE398-7004-46df-9FF1-9E0B8239FDE2}
	{ 0xe3ffe398, 0x7004, 0x46df, { 0x9f, 0xf1, 0x9e, 0xb, 0x82, 0x39, 0xfd, 0xe2 } }

};

/**
 * @brief DllMain
 *
 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}


/**
 * @brief Plugin Info
 *
 */
extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/**
 * @brief Load
 *
 */
extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;

	//create_menu();

	//check_contacts();

	name_day_core.create_menu();
	name_day_core.perform_name_day_test();
	
	return 0;
}

static const MUUID interfaces[] = {MIID_NAMEDAY, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

/**
 * @brief Unload
 *
 */
extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
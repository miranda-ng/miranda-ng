/**
 * @file Name day plugin
 */
#include <windows.h>
#include "../../headers_c/newpluginapi.h"

#include "name_day_core.h"

static	name_day_core_t		name_day_core;

HINSTANCE hInst;
PLUGINLINK *pluginLink;

/**
 * @brief Plugin info.
 *
 */
PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"Name Day Plugin",
	PLUGIN_MAKE_VERSION(0, 0, 0, 7),
	"The Name Day plugin.",
	"Tibor Szabo",
	"tibor.szabo@gmail.com",
	"© 2005 Tibor Szabo",
	"",
	0,		//not transient
	0		//doesn't replace anything built-in
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
extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
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

/**
 * @brief Unload
 *
 */
extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
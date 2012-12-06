/*
Miranda NG plugin template
http://miranda-ng.org/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include <windows.h>
#include <tchar.h>

#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_skin.h>

HINSTANCE hInst = NULL;
int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Plugin Template",
	PLUGIN_MAKE_VERSION(0,0,0,2),
	"The long description of your plugin, to go in the plugin options dialog",
	"J. Random Hacker",
	"noreply@sourceforge.net",
	"© 2002 J. Random Hacker",
	"http://miranda-ng.org/",
	UNICODE_AWARE,		//not transient
   // Generate your own unique id for your plugin.
   // Do not use this UUID!
   // Use uuidgen.exe to generate the uuuid
   {0x8b86253, 0xec6e, 0x4d09, { 0xb7, 0xa9, 0x64, 0xac, 0xdf, 0x6, 0x27, 0xb8 }} //{08B86253-EC6E-4d09-B7A9-64ACDF0627B8}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

INT_PTR PluginMenuCommand(WPARAM wParam, LPARAM lParam)
{
	MessageBox(NULL, TranslateT("Just groovy, baby!"), TranslateT("Plugin-o-rama"), MB_OK);
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID interfaces[] = {MIID_TESTPLUGIN, MIID_LAST};

extern "C" __declspec(dllexport) int Load()
{
	mir_getLP(&pluginInfo);

	CLISTMENUITEM mi = {0};

	CreateServiceFunction("TestPlug/MenuCommand", PluginMenuCommand);
	mi.cbSize = sizeof(mi);
	mi.position = -0x7FFFFFFF;
	mi.flags = CMIF_TCHAR;
	mi.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.ptszName = LPGENT("&Test Plugin...");
	mi.pszService = "TestPlug/MenuCommand";
	Menu_AddMainMenuItem(&mi);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}

/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/
#include <windows.h>
#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_skin.h>

HINSTANCE hInst;
PLUGINLINK *pluginLink;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Plugin Template",
	PLUGIN_MAKE_VERSION(0,0,0,2),
	"The long description of your plugin, to go in the plugin options dialog",
	"J. Random Hacker",
	"noreply@sourceforge.net",
	"© 2002 J. Random Hacker",
	"http://miranda-icq.sourceforge.net/",
	0,		//not transient
	0,		//doesn't replace anything built-in
    // Generate your own unique id for your plugin.
    // Do not use this UUID!
    // Use uuidgen.exe to generate the uuuid
    {0x8b86253, 0xec6e, 0x4d09, { 0xb7, 0xa9, 0x64, 0xac, 0xdf, 0x6, 0x27, 0xb8 }} //{08B86253-EC6E-4d09-B7A9-64ACDF0627B8}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

static int PluginMenuCommand(WPARAM wParam,LPARAM lParam)
{
	MessageBox(NULL,"Just groovy, baby!","Plugin-o-rama",MB_OK);
	return 0;
}

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_TESTPLUGIN, MIID_LAST};
__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	CLISTMENUITEM mi;

	pluginLink=link;
	CreateServiceFunction("TestPlug/MenuCommand",PluginMenuCommand);
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-0x7FFFFFFF;
	mi.flags=0;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.pszName=LPGEN("&Test Plugin...");
	mi.pszService="TestPlug/MenuCommand";
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);
	return 0;
}

int __declspec(dllexport) Unload(void)
{
	return 0;
}
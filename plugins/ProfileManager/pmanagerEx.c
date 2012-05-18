/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/
#define MIRANDA_VER 0x0A00

#include <windows.h>
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <tchar.h>
#include "resource.h"


HINSTANCE hInst;
PLUGINLINK *pluginLink;
TCHAR fn[MAX_PATH];
TCHAR lmn[MAX_PATH];
TCHAR* pathn;
int hLangpack;

#define SIZEOF(x) (sizeof(x)/sizeof(*x))

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Miranda IM Profile Changer",
	PLUGIN_MAKE_VERSION(0,0,0,3),
	"Adds a menu item to change or load a different profile of Miranda IM.",
	"Roman Gemini",
	"woobind@ukr.net",
	"© 2008 - 2010 Roman Gemini",
	"http://code.google.com/p/alfamar/",
	0,		//not transient
	0,		//doesn't replace anything built-in
    // Generate your own unique id for your plugin.
    // Do not use this UUID!
    // Use uuidgen.exe to generate the uuuid
	{0x7eeeb55e, 0x9d83, 0x4e1a, { 0xa1, 0x2f, 0x8f, 0x13, 0xf1, 0xa1, 0x24, 0xfb } }

};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

static int ChangePM(WPARAM wParam,LPARAM lParam)
{
		GetModuleFileName(GetModuleHandle(NULL), fn, SIZEOF(fn));
 		ShellExecute(0, "open", fn, "/FORCESHOW", "", 1);
		CallService("CloseAction", 0, 0);
		return 0;
}

static int LoadPM(WPARAM wParam,LPARAM lParam)
{
		GetModuleFileName(GetModuleHandle(NULL), fn, SIZEOF(fn));
 		ShellExecute(0, "open", fn, "/FORCESHOW", "", 1);
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
	mir_getLP(&pluginInfo);

	CreateServiceFunction("Database/LoadPM",LoadPM);
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-500200000;
	mi.flags=0;
	mi.hIcon=LoadIcon(hInst, MAKEINTRESOURCE(IDI_LoadPM));
	mi.pszPopupName = "Database";
	mi.pszName=LPGEN("Load profile");
	mi.pszService="Database/LoadPM";
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);
	
	CreateServiceFunction("Database/ChangePM",ChangePM);
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-500200000;
	mi.flags=0;
	mi.hIcon=LoadIcon(hInst, MAKEINTRESOURCE(IDI_ChangePM));
	mi.pszPopupName = "Database";
	mi.pszName=LPGEN("Change profile");
	mi.pszService="Database/ChangePM";
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);
	return 0;
}

int __declspec(dllexport) Unload(void)
{
	return 0;
}
#include "headers.h"

HANDLE hFunc, hTempRemove, hLoadHook;
int hLangpack;

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's extended information

// {553811EE-DEB6-48b8-8902-A8A00C1FD679}
#define MIID_STOPSPAM { 0x553811ee, 0xdeb6, 0x48b8, { 0x89, 0x2, 0xa8, 0xa0, 0xc, 0x1f, 0xd6, 0x79 } }

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	MIID_STOPSPAM
};


HINSTANCE hInst;

_inline unsigned int MakeVer(int a,int b,int c,int d)
{
    return PLUGIN_MAKE_VERSION(a,b,c,d);
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" int __declspec(dllexport) Load(void)
{
	CLISTMENUITEM mi;

	mir_getLP(&pluginInfoEx);

	plSets=new Settings;

	hFunc = CreateServiceFunction(MS_STOPSPAM_CONTACTPASSED, IsContactPassed);

	hLoadHook = HookEvent(ME_SYSTEM_MODULESLOADED, OnSystemModulesLoaded);

	// Add deliting temporary contacts
	hTempRemove = CreateServiceFunction(MS_STOPSPAM_REMTEMPCONTACTS, RemoveTempContacts);
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = -0x7FFFFFFF;
	mi.flags = CMIF_TCHAR;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.ptszName = LPGENT("Remove Temporary Contacts");
	mi.pszService = pluginName"/RemoveTempContacts";
	Menu_AddMainMenuItem(&mi);

	miranda::EventHooker::HookAll();
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	miranda::EventHooker::UnhookAll();
	
	if(hFunc)
	{
		DestroyServiceFunction(hFunc);
		hFunc = 0;
	}
	if(hTempRemove)
	{
		DestroyServiceFunction(hTempRemove);
		hFunc = 0;
	}
	delete plSets;
	
	return 0;
}
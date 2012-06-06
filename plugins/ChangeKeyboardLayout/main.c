#include "commonheaders.h"

struct MM_INTERFACE mmi;
PLUGINLINK *pluginLink;
int hLangpack;

PLUGININFOEX pluginInfoEx={
	sizeof(PLUGININFOEX),
	#if defined (_UNICODE)
		"Change Keyboard Layout (Unicode)",
	#else
		"Change Keyboard Layout (ANSI)",
	#endif
	VERSION,
	"Plugin for change keyboard layout of text (multilayout).",
	"Yasnovidyashii",
	"Yasnovidyashii@gmail.com",
	"© 2006-2009 Mikhail Yur'ev",
	"http://lemnews.com/forum/viewtopic.php?t=1493",
	0,		//not transient
	0,		//doesn't replace anything built-in
	#if defined (_UNICODE)
		MIID_CKL_UNICODE
	#else
		MIID_CKL_ANSI
	#endif
	
};

PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	#if defined (_UNICODE)
		"Change Keyboard Layout (UNICODE)",
	#else
		"Change Keyboard Layout (ANSI)",
	#endif
	VERSION,
	"Plugin for change keyboard layout of text (multilayout)",
	"Yasnovidyashii",
	"Yasnovidyashii@gmail.com",
	"© 2006-2009 Mikhail Yur'ev",
	"http://lemnews.com/forum/viewtopic.php?t=1493",
	0,		//not transient
	0		//doesn't replace anything built-in	
};


LPCTSTR ptszKeybEng=_T("`1234567890-=\\qwertyuiop[]asdfghjkl;'zxcvbnm,./~!@#$%^&*()_+|QWERTYUIOP{}ASDFGHJKL:\"ZXCVBNM<>?");
HKL hklEng=(HKL)0x04090409;

LPCTSTR ptszSeparators=_T(" \t\n\r");

HANDLE hOptionsInitialize;
HANDLE hModulesLoaded;


BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}


__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	dwMirandaVersion = mirandaVersion;
	return &pluginInfoEx;
}

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	dwMirandaVersion = mirandaVersion;
	return &pluginInfo;
}


#if defined (_UNICODE)
	static const MUUID interfaces[] = {MIID_CKL_UNICODE, MIID_LAST};
#else
	static const MUUID interfaces[] = {MIID_CKL_ANSI, MIID_LAST};
#endif

__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	#if defined (_UNICODE)
		pluginInfoEx.flags = 1; // dynamic UNICODE_AWARE
	#endif
	return interfaces;
}


int __declspec(dllexport) Load(PLUGINLINK *link)
{	
	pluginLink=link;
	mir_getMMI(&mmi);
	mir_getLP(&pluginInfoEx);
	ZeroMemory(hklLayouts, 20 * sizeof(HKL));
	bLayNum=GetKeyboardLayoutList(20,hklLayouts);
	if (bLayNum<2) 
		return 1;
	hOptionsInitialize = HookEvent(ME_OPT_INITIALISE, OnOptionsInitialise);
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	return 0;
}

int __declspec(dllexport) Unload(void)
{
	DWORD i;

	for (i=0;i<bLayNum;i++)	
		mir_free(ptszLayStrings[i]);

	UnhookEvent(hOptionsInitialize);
	UnhookEvent(hIcoLibIconsChanged);
	UnhookEvent(hModulesLoaded);
	DestroyServiceFunction(hChangeLayout);
	DestroyServiceFunction(hGetLayoutOfText);
	DestroyServiceFunction(hChangeTextLayout);
	UnhookWindowsHookEx(kbHook_All);
	return 0;
}



#include "stdafx.h"

CMPlugin g_plugin;
wchar_t *ptszLayStrings[20];
HANDLE hChangeLayout, hGetLayoutOfText, hChangeTextLayout;
HICON hPopupIcon, hCopyIcon;
HKL hklLayouts[20];
int bLayNum;
HHOOK kbHook_All;
MainOptions moOptions;
PopupOptions poOptions;

LPCTSTR ptszSeparators = L" \t\n\r";

HANDLE hOptionsInitialize;

static IconItem iconList[] =
{
	{ LPGEN("Copy to clipboard"), "ckl_copy_icon", IDI_COPYICON }
};

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {C5EF53A8-80D4-4CE9-B341-EC90D3EC9156}
	{0xc5ef53a8, 0x80d4, 0x4ce9, {0xb3, 0x41, 0xec, 0x90, 0xd3, 0xec, 0x91, 0x56}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	memset(hklLayouts, 0, sizeof(hklLayouts));
	bLayNum = GetKeyboardLayoutList(20, hklLayouts);
	if (bLayNum < 2)
		return 1;

	for (int i = 0; i < bLayNum; i++)
		if ((UINT_PTR(hklLayouts[i]) & 0xFFFF) == 0x409)
			hklEng = hklLayouts[i];

	HookEvent(ME_OPT_INITIALISE, OnOptionsInitialise);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	// IcoLib support
	g_plugin.registerIcon(MODULENAME, iconList);

	HookEvent(ME_SKIN_ICONSCHANGED, OnIconsChanged);

	OnIconsChanged(0, 0);

	return 0;
}

int CMPlugin::Unload()
{
	for (int i = 0; i < bLayNum; i++)
		mir_free(ptszLayStrings[i]);

	DestroyServiceFunction(hChangeLayout);
	DestroyServiceFunction(hGetLayoutOfText);
	DestroyServiceFunction(hChangeTextLayout);
	UnhookWindowsHookEx(kbHook_All);
	return 0;
}

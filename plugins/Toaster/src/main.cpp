#include "stdafx.h"

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {1FDBD8F0-3929-41BC-92D1-020779460769}
	{ 0x1fdbd8f0, 0x3929, 0x41bc, { 0x92, 0xd1, 0x2, 0x7, 0x79, 0x46, 0x7, 0x69 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_POPUP, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static int SrmmMenu_ProcessIconClick(WPARAM, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;

	if (!mir_strcmp(sicd->szModule, MODULENAME))
		Popup_Enable(!Popup_Enabled());

	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	StatusIconData sid = {};
	sid.szModule = MODULENAME;
	sid.szTooltip.a = LPGEN("Popup mode");
	sid.hIcon = sid.hIconDisabled = Skin_LoadIcon(SKINICON_OTHER_POPUP);
	Srmm_AddIcon(&sid, &g_plugin);

	HookEvent(ME_MSG_ICONPRESSED, SrmmMenu_ProcessIconClick);
}

static int OnShutdown(WPARAM, LPARAM)
{
	CleanupClasses();
	HideAllToasts();
	DeleteDirectoryTreeW(wszTempDir);
	return 0;
}

int CMPlugin::Load()
{
	if (!IsWinVer8Plus()) {
		MessageBox(nullptr, TranslateT("This plugin requires Windows 8 or higher"), _T(MODULENAME), MB_OK | MB_ICONERROR);
		return 1;
	}

	HookEvent(ME_OPT_INITIALISE, OnOptionsInitialized);
	HookEvent(ME_SYSTEM_SHUTDOWN, &OnShutdown);

	if (GetEnvironmentVariableW(L"TEMP", wszTempDir, MAX_PATH) != 0) {
		wcscat_s(wszTempDir, L"\\Miranda.Toaster");
		CreateDirectoryTreeW(wszTempDir);
	}
	else MessageBox(nullptr, TranslateT("Failed to create temporary directory"), _T(MODULENAME), MB_OK | MB_ICONERROR);

	InitServices();

	return 0;
}

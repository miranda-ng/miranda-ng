#include "stdafx.h"
#include "version.h"

CMPlugin g_plugin;
HNETLIBUSER hNetlibUser;
UINT_PTR hTimer;

void InitGraphMenu();

static HGENMENU g_hContactMenuUpdate = nullptr;

static PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {D0D8A5BE-A76A-46B1-92B6-3D69924E211D}
	{ 0xd0d8a5be, 0xa76a, 0x46b1, { 0x92, 0xb6, 0x3d, 0x69, 0x92, 0x4e, 0x21, 0x1d } }
};

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CLibreViewProto>(MODULENAME, pluginInfoEx)
{
	SetUniqueId("PatientId");
}

static INT_PTR PluginMenuCommand(WPARAM hContact, LPARAM)
{
	mir_forkthread(Check_ThreadFunc, g_plugin.getInstance(hContact));
	return 0;
}

static int OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(g_hContactMenuUpdate, g_plugin.getInstance(hContact) != nullptr);
	return 0;
}

static void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	time_t now = time(0);
	for (auto &it : g_plugin.g_arInstances) {
		if (it->m_hContact == 0)
			continue;

		uint32_t minutes = it->UpdateInterval;
		if (minutes == 0)
			continue;

		if (it->tsLastUpdate == 0 || now - it->tsLastUpdate >= time_t(minutes * 60))
			mir_forkthread(Check_ThreadFunc, it);
	}
}

void RestartTimer()
{
	if (hTimer)
		KillTimer(nullptr, hTimer);

	hTimer = SetTimer(nullptr, 0, 60000, TimerProc);
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Main icon"),    "main",  IDI_MAIN  },
	{ LPGEN("Graph viewer"), "graph", IDI_GRAPH },
};

int CMPlugin::Load()
{
	g_plugin.registerIcon(LPGEN("Protocols") "/" MODULENAME, iconList, MODULENAME);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_NOHTTPSOPTION | NUF_UNICODE;
	nlu.szSettingsModule = MODULENAME;
	nlu.szDescriptiveName.w = TranslateT("LibreView");
	hNetlibUser = Netlib_RegisterUser(&nlu);

	CreateServiceFunction(MODULENAME "/Update", PluginMenuCommand);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x82c22dbb, 0x9768, 0x4c1c, 0x9d, 0x72, 0x8d, 0x63, 0x3f, 0xf0, 0xe1, 0xad);
	mi.position = -0x7FFFFFFF;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_MAIN);
	mi.name.a = LPGEN("&Update data");
	mi.pszService = MODULENAME "/Update";
	g_hContactMenuUpdate = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_USERINFO_INITIALISE, UserInfoInit);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);
	InitGraphMenu();
	RestartTimer();
	return 0;
}

int CMPlugin::Unload()
{
	if (hTimer)
		KillTimer(nullptr, hTimer);

	if (hNetlibUser)
		Netlib_CloseHandle(hNetlibUser);
	return 0;
}

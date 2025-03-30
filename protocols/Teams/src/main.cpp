#include "stdafx.h"

CMPlugin g_plugin;

char g_szMirVer[100];
HANDLE g_hCallEvent;
HANDLE hExtraXStatus;

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
	// {DCD56CEC-C61B-4275-A010-8C65C5B48815}
	{ 0xDCD56CEC, 0xC61B, 0x4275, { 0xa0, 0x10, 0x8c, 0x65, 0xc5, 0x84, 0x88, 0x15 }}
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CTeamsProto>("Teams", pluginInfoEx)
{
	SetUniqueId("id");
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] = {
	{ LPGEN("Protocol icon"),        "main",             IDI_TEAMS         },
	{ LPGEN("Create new chat icon"), "conference",       IDI_CONFERENCE    },
	{ LPGEN("Block user icon"),      "user_block",       IDI_BLOCKUSER     },
	{ LPGEN("Unblock user icon"),    "user_unblock",     IDI_UNBLOCKUSER   },
	{ LPGEN("Incoming call icon"),   "inc_call",         IDI_CALL          },
	{ LPGEN("Notification icon"),    "notify",           IDI_NOTIFY        },
	{ LPGEN("Error icon"),           "error",            IDI_ERRORICON     },
	{ LPGEN("Action icon"),          "me_action",        IDI_ACTION_ME     }

};

int CMPlugin::Load()
{
	registerIcon("Protocols/" MODULENAME, iconList, MODULENAME);

	g_hCallEvent = CreateHookableEvent(MODULENAME "/IncomingCall");
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	DestroyHookableEvent(g_hCallEvent);
	return 0;
}

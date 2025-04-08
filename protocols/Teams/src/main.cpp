/*
Copyright (C) 2025 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

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

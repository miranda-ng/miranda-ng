/*
Copyright (c) 2014-17 Robert Pösel, 2017-22 Miranda NG team

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
#include "version.h"

CMPlugin	g_plugin;

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {2A1081D1-AEE3-4091-B70D-AE46D09F9A7F}
	{ 0x2a1081d1, 0xaee3, 0x4091, {0xb7, 0xd, 0xae, 0x46, 0xd0, 0x9f, 0x9a, 0x7f}}
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CDummyProto>("Dummy", pluginInfoEx)
{
	int id = db_get_b(0, m_szModuleName, DUMMY_ID_TEMPLATE, -1);
	if (id < 0 || id >= _countof(templates))
		SetUniqueId(ptrA(db_get_sa(0, m_szModuleName, DUMMY_ID_SETTING)));
	else
		SetUniqueId(templates[id].setting);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////
// OnModulesLoaded - execute some code when all plugins are initialized

static int OnModulesLoaded(WPARAM, LPARAM)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnLoad - initialize the plugin instance

int CMPlugin::Load()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// stubs for obsoleted protocols

struct CMPluginAim : public ACCPROTOPLUGIN<CDummyProto>
{
	CMPluginAim() : ACCPROTOPLUGIN<CDummyProto>("AIM", pluginInfoEx)
	{
		SetUniqueId("SN");
	}
}
static g_pluginAim;

struct CMPluginYahoo : public ACCPROTOPLUGIN<CDummyProto>
{
	CMPluginYahoo() : ACCPROTOPLUGIN<CDummyProto>("YAHOO", pluginInfoEx)
	{
		SetUniqueId("yahoo_id");
	}
}
static g_pluginYahoo;

struct CMPluginTlen : public ACCPROTOPLUGIN<CDummyProto>
{
	CMPluginTlen() : ACCPROTOPLUGIN<CDummyProto>("TLEN", pluginInfoEx)
	{
		SetUniqueId("jid");
	}
}
static g_pluginTlen;

struct CMPluginXFire : public ACCPROTOPLUGIN<CDummyProto>
{
	CMPluginXFire() : ACCPROTOPLUGIN<CDummyProto>("XFire", pluginInfoEx)
	{
		SetUniqueId("Username");
	}
}
static g_pluginXFire;

struct CMPluginWhatsapp : public ACCPROTOPLUGIN<CDummyProto>
{
	CMPluginWhatsapp() : ACCPROTOPLUGIN<CDummyProto>("WhatsApp", pluginInfoEx)
	{
		SetUniqueId("ID");
	}
}
static g_pluginWhatsapp;

struct CMPluginMsn : public ACCPROTOPLUGIN<CDummyProto>
{
	CMPluginMsn() : ACCPROTOPLUGIN<CDummyProto>("MSN", pluginInfoEx)
	{
		SetUniqueId("wlid");
	}
}
static g_pluginMsn;

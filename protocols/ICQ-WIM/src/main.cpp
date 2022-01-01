// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018-22 Miranda NG team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

bool g_bSecureIM, g_bMessageState;

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	{ 0xEFB2355B, 0x82B3, 0x4759, { 0xb7, 0xd8, 0x95, 0xf8, 0xe9, 0x50, 0x62, 0x91 } } // {EFB2355B-82B3-4759-B7D8-95F8E9506291}
};

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

struct CMPluginMra : public ACCPROTOPLUGIN<CIcqProto>
{
	CMPluginMra() : ACCPROTOPLUGIN<CIcqProto>("MRA", pluginInfoEx)
	{
		SetUniqueId(DB_KEY_ID);
	}

	void Register()
	{
		m_hInst = g_plugin.getInst();
		RegisterProtocol(PROTOTYPE_PROTOCOL, g_plugin.fnInit, g_plugin.fnUninit);
	}
}
static g_pluginMra;

/////////////////////////////////////////////////////////////////////////////////////////

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CIcqProto>(MODULENAME, pluginInfoEx)
{
	SetUniqueId(DB_KEY_ID);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static int ModuleLoad(WPARAM, LPARAM)
{
	g_bSecureIM = ServiceExists("SecureIM/IsContactSecured");
	g_bMessageState = ServiceExists(MS_MESSAGESTATE_UPDATE);
	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	ModuleLoad(0, 0);
	return 0;
}

static IconItem iconList[] =
{
	{ LPGEN("E-mail"), "icq_email", IDI_INBOX },
	{ LPGEN("E-mail notification"), "icq_email_notif", IDI_MAIL_NOTIFY }
};

int CMPlugin::Load()
{
	// register the second instance of this plugin as MRA
	g_pluginMra.Register();

	registerIcon("Protocols/ICQ", iconList, "ICQ");

	HookEvent(ME_SYSTEM_MODULELOAD, ModuleLoad);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, ModuleLoad);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	return 0;
}

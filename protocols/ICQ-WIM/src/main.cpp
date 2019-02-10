// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018-19 Miranda NG team
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

bool g_bPopupService, g_bMessageState;

HWND g_hwndHeartbeat;

IconItem iconList[] =
{
	{ LPGEN("Email"), "icq_email", IDI_INBOX },
	{ LPGEN("Email notification"), "icq_email_notif", IDI_MAIL_NOTIFY }
};

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
}
static g_pluginMra;

/////////////////////////////////////////////////////////////////////////////////////////

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CIcqProto>(MODULENAME, pluginInfoEx)
{
	g_pluginMra.setInst(m_hInst);
	SetUniqueId(DB_KEY_ID);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static int OnContactMenu(WPARAM hContact, LPARAM lParam)
{
	Menu_ShowItem(g_plugin.m_hmiRoot, false);

	CIcqProto *proto = CMPlugin::getInstance(hContact);
	return proto ? proto->OnContactMenu(hContact, lParam) : 0;
}

static INT_PTR ICQPermitDeny(WPARAM hContact, LPARAM, LPARAM bAllow)
{
	CIcqProto *proto = CMPlugin::getInstance(hContact);
	if (proto)
		proto->SetPermitDeny(proto->GetUserId(hContact), bAllow != 0);
	return 0;
}

static int ModuleLoad(WPARAM, LPARAM)
{
	g_bPopupService = ServiceExists(MS_POPUP_ADDPOPUPT);
	g_bMessageState = ServiceExists(MS_MESSAGESTATE_UPDATE);
	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	ModuleLoad(0, 0);

	// init menus
	CMenuItem mi(&g_plugin);

	SET_UID(mi, 0x9cd3a933, 0x3bd5, 0x4d1c, 0xbd, 0xf1, 0xa8, 0xf9, 0xbf, 0xf0, 0xd7, 0x28);
	mi.position = 100000;
	mi.name.a = "ICQ";
	mi.hIcolibItem = Skin_LoadProtoIcon(g_plugin.getModule(), ID_STATUS_ONLINE);
	g_plugin.m_hmiRoot = Menu_AddContactMenuItem(&mi);

	mi.flags = CMIF_UNMOVABLE;
	mi.root = g_plugin.m_hmiRoot;
	mi.name.a = LPGEN("Ignore");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REVOKE);
	mi.pszService = "ICQ/Ignore";
	g_plugin.m_hmiIgnore = Menu_AddContactMenuItem(&mi);
	CreateServiceFunctionParam(mi.pszService, ICQPermitDeny, 0);

	mi.name.a = LPGEN("Remove Ignore");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_ADD);
	mi.pszService = "ICQ/RemoveIgnore";
	g_plugin.m_hmiAllow = Menu_AddContactMenuItem(&mi);
	CreateServiceFunctionParam(mi.pszService, ICQPermitDeny, 1);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnContactMenu);
	return 0;
}

int CMPlugin::Load()
{
	g_hwndHeartbeat = CreateWindowEx(0, L"STATIC", nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);

	registerIcon("Protocols/ICQ", iconList, "ICQ");

	HookEvent(ME_SYSTEM_MODULELOAD, ModuleLoad);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, ModuleLoad);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	return 0;
};

int CMPlugin::Unload()
{
	DestroyWindow(g_hwndHeartbeat);
	return 0;
}

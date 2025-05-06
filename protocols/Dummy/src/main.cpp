/*
Copyright (c) 2014-17 Robert Pösel, 2017-25 Miranda NG team

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
	if (id < 0)
		SetUniqueId(ptrA(db_get_sa(0, m_szModuleName, DUMMY_ID_SETTING)));
	else
		SetUniqueId(templates[id].setting);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////
// OnLoad - initialize the plugin instance

static int OnDummyDoubleClicked(WPARAM hContact, LPARAM)
{
	if (auto *pa = Proto_GetContactAccount(hContact))
		if (auto *ppro = (CDummyProto*)pa->ppro)
			if (ppro->GetCaps(1000)) {
				if (Contact::IsGroupChat(hContact) || !ppro->bAllowSending)
					CallService(MS_HISTORY_SHOWCONTACTHISTORY, hContact, 0);
				else
					CallService(MS_MSG_SENDMESSAGE, hContact, 0);
				return 1;
			}

	return 0;
}

int CMPlugin::Load()
{
	InitIcons();

	HookEvent(ME_CLIST_DOUBLECLICKED, OnDummyDoubleClicked);
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

struct CMPluginOmegle : public ACCPROTOPLUGIN<CDummyProto>
{
	CMPluginOmegle() : ACCPROTOPLUGIN<CDummyProto>("Omegle", pluginInfoEx)
	{
		SetUniqueId("nick");
	}
}
static g_pluginOmegle;

struct CMPluginIcq : public ACCPROTOPLUGIN<CDummyProto>
{
	CMPluginIcq() : ACCPROTOPLUGIN<CDummyProto>("ICQ", pluginInfoEx)
	{
		SetUniqueId("aimId");
	}
}
static g_pluginIcq;

struct CMPluginMra : public ACCPROTOPLUGIN<CDummyProto>
{
	CMPluginMra() : ACCPROTOPLUGIN<CDummyProto>("MRA", pluginInfoEx)
	{
		SetUniqueId("aimId");
	}
}
static g_pluginMra;

struct CMPluginSkype : public ACCPROTOPLUGIN<CDummyProto>
{
	CMPluginSkype() : ACCPROTOPLUGIN<CDummyProto>("SKYPE", pluginInfoEx)
	{
		SetUniqueId("SkypeId");
	}
}
static g_pluginSkype;

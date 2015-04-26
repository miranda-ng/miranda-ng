/*
Copyright © 2012-15 Miranda NG team
Copyright © 2009 Jim Porter

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "version.h"

#include "proto.h"
#include "theme.h"

CLIST_INTERFACE* pcli;

HINSTANCE g_hInstance;
int hLangpack = 0;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	//{BC09A71B-B86E-4d33-B18D-82D30451DD3C}
	{ 0xbc09a71b, 0xb86e, 0x4d33, { 0xb1, 0x8d, 0x82, 0xd3, 0x4, 0x51, 0xdd, 0x3c } }
};

/////////////////////////////////////////////////////////////////////////////
// Protocol instances

static int compare_protos(const TwitterProto *p1, const TwitterProto *p2)
{
	return _tcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

OBJLIST<TwitterProto> g_Instances(1, compare_protos);

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInstance = hInstance;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////
// Load

static PROTO_INTERFACE* protoInit(const char *proto_name, const TCHAR *username)
{
	TwitterProto *proto = new TwitterProto(proto_name, username);
	g_Instances.insert(proto);
	return proto;
}

static int protoUninit(PROTO_INTERFACE *proto)
{
	g_Instances.remove(static_cast<TwitterProto*>(proto));
	return 0;
}

static HANDLE g_hEvents[1];

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	mir_getCLI();

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "Twitter";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = protoInit;
	pd.fnUninit = protoUninit;
	CallService(MS_PROTO_REGISTERMODULE, 0, reinterpret_cast<LPARAM>(&pd));

	InitIcons();
	InitContactMenus();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

extern "C" int __declspec(dllexport) Unload(void)
{
	UninitContactMenus();
	for (size_t i = 1; i < SIZEOF(g_hEvents); i++)
		UnhookEvent(g_hEvents[i]);

	return 0;
}
/*

Standard file transfers' plugin for Miranda NG

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "stdafx.h"

int LoadSendRecvFileModule(void);

CMPlugin g_plugin;

ITaskbarList3 * pTaskbarInterface;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	MIRANDA_VERSION_DWORD,
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// 39698dce-7ed4-4334-ac4c-ba8b37a86f13
	{ 0x39698dce, 0x7ed4, 0x4334, {0xac, 0x4c, 0xba, 0x8b, 0x37, 0xa8, 0x6f, 0x13}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	bAutoMin(MODULENAME, "AutoMin", false),
	bAutoClear(MODULENAME, "AutoClear", true),
	bAutoClose(MODULENAME, "AutoClose", false),
	bAutoAccept(MODULENAME, "AutoAccept", false),
	bReverseOrder(MODULENAME, "ReverseOrder", false)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SRFILE, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	if ( IsWinVer7Plus())
		CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_ALL, IID_ITaskbarList3, (void**)&pTaskbarInterface);

	LoadSendRecvFileModule();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	if (pTaskbarInterface)
		pTaskbarInterface->Release();

	return 0;
}

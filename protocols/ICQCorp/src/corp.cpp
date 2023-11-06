/*
	ICQ Corporate protocol plugin for Miranda IM.
	Copyright (C) 2003-2005 Eugene Tarasenko <zlyden13@inbox.ru>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

///////////////////////////////////////////////////////////////////////////////

char protoName[64];
CMPlugin g_plugin;
HNETLIBUSER hNetlibUser;

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
	// {4B5EA279-0014-431F-AB27-F76D4B284893}
	{ 0x4b5ea279, 0x14, 0x431f, { 0xab, 0x27, 0xf7, 0x6d, 0x4b, 0x28, 0x48, 0x93 }}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(protoName, pluginInfoEx)
{
	char fileName[MAX_PATH];
	GetModuleFileNameA(m_hInst, fileName, MAX_PATH);

	WIN32_FIND_DATAA findData;
	FindClose(FindFirstFileA(fileName, &findData));
	findData.cFileName[strlen(findData.cFileName) - 4] = 0;
	strncpy_s(protoName, findData.cFileName, _TRUNCATE);

	Proto_RegisterModule(PROTOTYPE_PROTOCOL, protoName);
	Proto_SetUniqueId(protoName, "UIN");
}

//////////////////////////////////////////////////////////////////////////
// Interface information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

///////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING;
	nlu.szSettingsModule = protoName;
	nlu.szDescriptiveName.a = protoName;
	hNetlibUser = Netlib_RegisterUser(&nlu);

	LoadServices();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	UnloadServices();
	return 0;
}

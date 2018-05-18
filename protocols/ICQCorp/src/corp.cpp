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

int &hLangpack(g_plugin.m_hLang);
char protoName[64];
CMPlugin g_plugin;
HNETLIBUSER hNetlibUser;

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

//////////////////////////////////////////////////////////////////////////
// Interface information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

///////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Load()
{
	mir_getLP(&pluginInfo);

	CMStringA szDescr(FORMAT, "%s connection", protoName);
	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING;
	nlu.szSettingsModule = protoName;
	nlu.szDescriptiveName.a = szDescr.GetBuffer();
	hNetlibUser = Netlib_RegisterUser(&nlu);

	LoadServices();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Unload()
{
	UnloadServices();
	return 0;
}

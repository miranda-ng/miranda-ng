/*
Miranda IM Country Flags Plugin
Copyright (C) 2006-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Flags-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

int nCountriesCount;
CountryListEntry *countries;

HINSTANCE hInst;
int hLangpack;

static PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {68C36842-3D95-4F4A-AB81-014D6593863B}
	{0x68c36842, 0x3d95, 0x4f4a, {0xab, 0x81, 0x1, 0x4d, 0x65, 0x93, 0x86, 0x3b}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	PrepareBufferedFunctions();
	if ( CallService(MS_UTILS_GETCOUNTRYLIST, (WPARAM)&nCountriesCount, (LPARAM)&countries))
		nCountriesCount = 0;

	bShowStatusIcon = db_get_b(NULL, MODULENAME, "ShowStatusIconFlag", SETTING_SHOWSTATUSICONFLAG_DEFAULT) != 0;
	bShowExtraIcon  = db_get_b(NULL, MODULENAME, "ShowExtraImgFlag", SETTING_SHOWEXTRAIMGFLAG_DEFAULT) != 0;
	bUseUnknown     = db_get_b(NULL, MODULENAME, "UseUnknownFlag", SETTING_USEUNKNOWNFLAG_DEFAULT) != 0;
	bUseIpToCountry = db_get_b(NULL, MODULENAME, "UseIpToCountry", SETTING_USEIPTOCOUNTRY_DEFAULT) != 0;

	InitIcons();
	InitIpToCountry();
	InitExtraImg();
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	KillBufferedFunctions();
	UninitIpToCountry();
	return 0;
}

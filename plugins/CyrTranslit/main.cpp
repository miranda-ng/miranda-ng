/**
 * CyrTranslit: the Cyrillic transliteration plug-in for Miranda IM.
 * Copyright 2005 Ivan Krechetov. 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "plugin.h"
#include "MirandaContact.h"


HINSTANCE hInst;
PLUGINLINK *pluginLink;
struct UTF8_INTERFACE utfi;
struct MM_INTERFACE mmi;

PLUGININFOEX pluginInfoEx={
	sizeof(PLUGININFOEX),
    PLG_SHORTNAME,
	PLG_VERSION,
	PLG_DESCRIPTION,
    PLG_AUTHOR,
    PLG_AUTHOREMAIL,
    PLG_COPYRIGHT,
    PLG_HOMEPAGE,
	PLG_FLAGS,
	PLG_REPLACESDEFMODULE,
	MIID_V_CYRTRANSLIT,
};

PLUGININFO pluginInfo = {
    sizeof(PLUGININFO),
    PLG_SHORTNAME,
	PLG_VERSION,
	PLG_DESCRIPTION,
    PLG_AUTHOR,
    PLG_AUTHOREMAIL,
    PLG_COPYRIGHT,
    PLG_HOMEPAGE,
	PLG_FLAGS,
	PLG_REPLACESDEFMODULE,
};

//------------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

//------------------------------------------------------------------------------

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_V_CYRTRANSLIT, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	//#if defined (_UNICODE)
	pluginInfoEx.flags = 1; // dynamic UNICODE_AWARE
	//#endif
	return interfaces;
}


//------------------------------------------------------------------------------

extern "C" __declspec(dllexport) int Load(PLUGINLINK *link)
{
	//system inits:
	pluginLink = link;
    
	memset(&utfi, 0, sizeof(utfi));
	mir_getUTFI(&utfi);

	memset(&mmi, 0, sizeof(mmi));
	mir_getMMI(&mmi);


	//plugin inits: PLACE IT ONLY AFTER THIS LINE

    CyrTranslit::MirandaContact::initialize();


	return 0;
}

//------------------------------------------------------------------------------

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}
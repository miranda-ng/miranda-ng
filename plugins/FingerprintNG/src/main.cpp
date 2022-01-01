/*
Fingerprint NG (client version) icons module for Miranda NG
Copyright © 2006-22 FYR, Bio, nullbie, ghazan, mataes, HierOS, faith_healer and all respective contributors.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

//Start of header
#include "stdafx.h"

#define LIB_REG		2
#define LIB_USE		3

CMPlugin g_plugin;

HANDLE hHeap = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////
// PluginInfoEx

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {687364AF-58B0-4AF2-A4EE-20F40A8D9AFB}
	{ 0x687364af, 0x58b0, 0x4af2, { 0xa4, 0xee, 0x20, 0xf4, 0xa, 0x8d, 0x9a, 0xfb } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load(void)
{
	InitFingerModule();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	HeapDestroy(hHeap);
	ClearFI();
	return 0;
}

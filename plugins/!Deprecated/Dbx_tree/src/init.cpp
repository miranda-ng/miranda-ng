/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "Interface.h"
#include "DatabaseLink.h"

HINSTANCE  hInstance = NULL;
int hLangpack;

static PLUGININFOEX gPluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE | STATIC_PLUGIN,
	// {28F45248-8C9C-4BEE-9307-7BCF3E12BF99}
	{0x28f45248, 0x8c9c, 0x4bee, {0x93, 0x07, 0x7b, 0xcf, 0x3e, 0x12, 0xbf, 0x99}}
};

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD MirandaVersion)
{
	return &gPluginInfoEx;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_DATABASE, MIID_LAST};

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&gPluginInfoEx);

	RegisterDatabasePlugin(&gDBLink);
	CompatibilityRegister();
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	hInstance = hInstDLL;
	return TRUE;
}

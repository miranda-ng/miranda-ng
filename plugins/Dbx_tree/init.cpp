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

HINSTANCE  hInstance = NULL;

static const DWORD gMinMirVer = 0x00080000;
static const MUUID gInterfaces[] = {MIID_DATABASE, MIID_LAST};
// {28F45248-8C9C-4bee-9307-7BCF3E12BF99}
static const MUUID gGUID =
{ 0x28f45248, 0x8c9c, 0x4bee, { 0x93, 0x07, 0x7b, 0xcf, 0x3e, 0x12, 0xbf, 0x99 } };


static PLUGININFOEX gPluginInfoEx = {
	sizeof(PLUGININFOEX),
	gInternalNameLong,
	gVersion,
	gDescription " - build " __DATE__ " @ " __TIME__,
	gAutor,
	gAutorEmail,
	gCopyright,
	"http://www-user.tu-chemnitz.de/~kunmi/?dbx_tree&lang=en",
	UNICODE_AWARE,
	DEFMOD_DB,
  gGUID
};

extern "C" __declspec(dllexport) DATABASELINK* DatabasePluginInfo(void * Reserved)
{
	return &gDBLink;
}

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD MirandaVersion)
{
	if (MirandaVersion < gMinMirVer)
	{
		MessageBoxA( 0, "The dbx_tree plugin cannot be loaded. It requires Miranda IM 0.8.0.0 or later.", gInternalName, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST );
		return NULL;
	}
	return &gPluginInfoEx;
}

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return gInterfaces;
}

extern "C" __declspec(dllexport) int Load(PLUGINLINK * Link)
{
	return 1;
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

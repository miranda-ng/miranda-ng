/*

StartPosition plugin for Miranda NG

Copyright (C) 2005-2008 Felipe Brahm - souFrag
ICQ#50566818
http://www.soufrag.cl

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

HINSTANCE g_hInst;
int hLangpack;

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
	// {211F6277-6F9B-4B77-A939-84D04B26B38C}
	{0x211f6277, 0x6f9b, 0x4b77, {0xa9, 0x39, 0x84, 0xd0, 0x4b, 0x26, 0xb3, 0x8c}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	g_hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	RECT WorkArea;

	SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkArea, 0);

	BYTE clEnableTop = db_get_b(NULL, MODULE_NAME, "CLEnableTop", 1);
	BYTE clEnableBottom = db_get_b(NULL, MODULE_NAME, "CLEnableBottom", 0);
	BYTE clEnableSide = db_get_b(NULL, MODULE_NAME, "CLEnableSide", 1);
	BYTE clEnableWidth = db_get_b(NULL, MODULE_NAME, "CLEnableWidth", 0);

	DWORD clTop = db_get_dw(NULL, MODULE_NAME, "CLpixelsTop", 3);
	DWORD clBottom = db_get_dw(NULL, MODULE_NAME, "CLpixelsBottom", 3);
	DWORD clSide = db_get_dw(NULL, MODULE_NAME, "CLpixelsSide", 3);
	BYTE clAlign = db_get_b(NULL, MODULE_NAME, "CLAlign", RIGHT);
	DWORD clWidth = db_get_dw(NULL, MODULE_NAME, "CLWidth", 180);

	BYTE clEnableState = db_get_b(NULL, MODULE_NAME, "CLEnableState", 0);
	BYTE clState = db_get_b(NULL, MODULE_NAME, "CLState", 2);

	if(clEnableState)
		db_set_b(NULL,"CList", "State", (BYTE)clState);

	if(clEnableWidth) {
		if(clWidth > 0)
			db_set_dw(NULL, "CList", "Width", clWidth);
	} else {
		clWidth = db_get_dw(NULL, "CList", "Width", 180);
	}

	if(clEnableTop || clEnableBottom || clEnableSide)
		db_set_b(NULL,"CList", "Docked", 0);

	if(clEnableTop)
		db_set_dw(NULL, "CList", "y", clTop);

	//thx ValeraVi
	if(clEnableBottom) {
		if(clEnableTop)
			db_set_dw(NULL, "CList", "Height", (WorkArea.bottom - WorkArea.top - clTop - clBottom));
		else
			db_set_dw(NULL, "CList", "y", (WorkArea.bottom - clBottom - (int)db_get_dw(NULL, "CList", "Height", 0)));
	}

	if(clEnableSide) {
		if(clAlign == LEFT)
			db_set_dw(NULL, "CList", "x", (WorkArea.left + clSide));
		else
			db_set_dw(NULL, "CList", "x", (WorkArea.right - clWidth - clSide));
	}

	HookEvent(ME_OPT_INITIALISE, OptInitialise);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}

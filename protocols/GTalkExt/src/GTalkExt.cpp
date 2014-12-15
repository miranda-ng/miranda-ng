//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************
//   GTalkExt.cpp : Defines the exported functions for the DLL application.
//***************************************************************************************

#include "stdafx.h"
#include "options.h"
#include "notifications.h"
#include "handlers.h"
#include "Version.h"
#include "inbox.h"

int   hLangpack;
HICON g_hPopupIcon = 0;

LIST<GoogleTalkAcc> g_accs(1, HandleKeySortT);

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	//{08B86253-EC6E-4d09-B7A9-64ACDF0627B8}
	{ 0x08B86253, 0xEC6E, 0x4d09, { 0xB7, 0xA9, 0x64, 0xAC, 0xDF, 0x06, 0x27, 0xB8 } }
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_NCCREATE:
		return 1;

	case WM_GETMINMAXINFO:
		PMINMAXINFO info = (PMINMAXINFO)lParam;
		info->ptMaxPosition.x = -100;
		info->ptMaxPosition.y = -100;
		info->ptMaxSize.x = 10;
		info->ptMaxSize.y = 10;
		info->ptMaxTrackSize.x = 10;
		info->ptMaxTrackSize.y = 10;
		info->ptMinTrackSize.x = 10;
		info->ptMinTrackSize.y = 10;
		return 0;
	}
	return DefWindowProc(wnd, msg, wParam, lParam);
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	mir_getXI(&xi);

	WNDCLASS cls = { 0 };
	cls.lpfnWndProc = WndProc;
	cls.lpszClassName = TEMP_WINDOW_CLASS_NAME;
	RegisterClass(&cls);

	g_hPopupIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_POPUP));

	InitAvaUnit(TRUE);

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, AccListChanged);

	AddTipperItem();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" int __declspec(dllexport) Unload(void)
{
	InitAvaUnit(FALSE);
	return 0;
}

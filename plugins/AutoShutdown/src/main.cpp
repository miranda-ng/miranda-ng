/*
'AutoShutdown'-Plugin for Miranda IM

Copyright 2004-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Shutdown-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

HINSTANCE hInst;
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
	// {9DE24579-5C5C-49AA-80E8-4D38E4344E63}
	{0x9de24579, 0x5c5c, 0x49aa, {0x80, 0xe8, 0x4d, 0x38, 0xe4, 0x34, 0x4e, 0x63}}
};

IconItem iconList[] = 
{
	{ LPGEN("Header"), "AutoShutdown_Header", IDI_HEADER },
	{ LPGEN("Active"), "AutoShutdown_Active", IDI_ACTIVE },
	{ LPGEN("Inactive"), "AutoShutdown_Inactive", IDI_INACTIVE },
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, VOID*)
{
	hInst = hinstDLL;
	return TRUE;
}

static int ShutdownModulesLoaded(WPARAM,LPARAM)
{
	/* Toolbar Item */
	HookEvent(ME_TTB_MODULELOADED,ToolbarLoaded); /* no service to check for */

	WatcherModulesLoaded();

	return 0;
}

extern "C" __declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	INITCOMMONCONTROLSEX icc;
	icc.dwSize=sizeof(icc);
	icc.dwICC=ICC_DATE_CLASSES|ICC_UPDOWN_CLASS|ICC_PROGRESS_CLASS;
	if (!InitCommonControlsEx(&icc)) return 1;

	if (InitFrame()) return 1; /* before icons */

	/* shared */
	Icon_Register(hInst, "Automatic Shutdown", iconList, SIZEOF(iconList));

	InitShutdownSvc();
	InitWatcher(); /* before InitSettingsDlg() */
	InitSettingsDlg();
	InitOptions();

	HookEvent(ME_SYSTEM_MODULESLOADED, ShutdownModulesLoaded);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UninitOptions();
	UninitSettingsDlg(); /* before UninitWatcher() */
	UninitWatcher(); /* before UninitFrame() */
	UninitFrame();
	UninitShutdownSvc();
	return 0;
}
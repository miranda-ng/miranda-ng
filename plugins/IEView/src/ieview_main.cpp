/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

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
#include "stdafx.h"

char *workingDirUtf8;
CMPlugin g_plugin;

IconItem iconList[] =
{
	{ LPGEN("Code"), "CODE", IDI_CODE },
	{ LPGEN("RTL On"), "RTL_ON", IDI_RTL_ON },
	{ LPGEN("RTL Off"), "RTL_OFF", IDI_RTL_OFF },
	{ LPGEN("Group On"), "GROUP_ON", IDI_GROUP_ON },
	{ LPGEN("Group Off"), "GROUP_OFF", IDI_GROUP_OFF }
};

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// 0495171B-7137-4DED-97F8-CE6FED67D691
	{ 0x0495171b, 0x7137, 0x4ded, { 0x97, 0xf8, 0xce, 0x6f, 0xed, 0x67, 0xd6, 0x91 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static int CustomButtonPressed(WPARAM, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;
	if (mir_strcmp(cbcd->pszModule, MODULENAME))
		return 0;

	if (cbcd->dwButtonId != 1)
		return 1;

	HWND hEdit = GetDlgItem(cbcd->hwndFrom, IDC_SRMM_MESSAGE);
	SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)L"[code][/code]");
	return 0;
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	BBButton bbd = {};
	bbd.pszModuleName = MODULENAME;
	bbd.dwButtonID = 1;
	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISIMBUTTON;
	bbd.dwDefPos = 40;
	bbd.hIcon = g_plugin.getIconHandle(IDI_CODE);
	bbd.pwszText = L"[Code]";
	bbd.pwszTooltip = LPGENW("Format text as code");
	Srmm_AddButton(&bbd, &g_plugin);

	HookEvent(ME_MSG_BUTTONPRESSED, CustomButtonPressed);

	Options::init();
	return 0;
}

int CMPlugin::Load()
{
	int wdsize = GetCurrentDirectory(0, nullptr);
	wchar_t *workingDir = new wchar_t[wdsize];
	GetCurrentDirectory(wdsize, workingDir);
	Utils::convertPath(workingDir);
	workingDirUtf8 = mir_utf8encodeW(workingDir);
	delete[] workingDir;

	hLogger = RegisterSrmmLog("ieview", L"IEView", &logBuilder);

	HookEvent(ME_OPT_INITIALISE, IEViewOptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	CreateServiceFunction(MS_IEVIEW_WINDOW, HandleIEWindow);
	CreateServiceFunction(MS_IEVIEW_EVENT, HandleIEEvent);
	CreateServiceFunction(MS_IEVIEW_NAVIGATE, HandleIENavigate);
	CreateServiceFunction("IEView/ReloadOptions", ReloadOptions);
	hHookOptionsChanged = CreateHookableEvent(ME_IEVIEW_OPTIONSCHANGED);
	g_plugin.registerIcon("IEView", iconList, MODULENAME);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	UnregisterSrmmLog(hLogger);
	Options::uninit();
	DestroyHookableEvent(hHookOptionsChanged);
	IEView::release();
	mir_free(workingDirUtf8);
	return 0;
}

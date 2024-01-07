/*
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

CMPlugin g_plugin;

HANDLE hTTBButton;

bool g_bVarsInstalled, g_bTipperInstalled, g_bFingerInstalled;

int OnOptInit(WPARAM, LPARAM);

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
	// {49BD9F2A-3111-4EB9-87E3-71E69CD97F7C}
	{0x49bd9f2a, 0x3111, 0x4eb9, {0x87, 0xe3, 0x71, 0xe6, 0x9c, 0xd9, 0x7f, 0x7c}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	m_columns(1)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

static int OnTTBLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.dwFlags = TTBBF_VISIBLE;
	ttb.pszService = QS_SHOWSERVICE;
	ttb.hIconHandleDn = ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_QS);
	ttb.name = MODULENAME;
	ttb.pszTooltipUp = ttb.pszTooltipDn = LPGEN("Quick Search");
	hTTBButton = g_plugin.addTTB(&ttb);
	return 0;
}

static INT_PTR OpenSearchWindow(WPARAM wParam, LPARAM)
{
	OpenSrWindow((wchar_t *)wParam);
	return 0;
}

static int OnCheckPlugins(WPARAM, LPARAM)
{
	g_bVarsInstalled = ServiceExists(MS_VARS_FORMATSTRING);
	g_bTipperInstalled = ServiceExists(MS_TIPPER_SHOWTIPW);
	g_bFingerInstalled = Finger_IsPresent();
	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_TTB_MODULELOADED, OnTTBLoaded);

	CreateServiceFunction(QS_SHOWSERVICE, OpenSearchWindow);

	// add menu item
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x98C2A92A, 0xD93D, 0x43E8, 0x91, 0xC3, 0x3B, 0xB6, 0xBE, 0x43, 0x44, 0xF0);
	mi.name.a = LPGEN("Quick Search");
	mi.position = 500050000;
	mi.pszService = QS_SHOWSERVICE;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_QS);
	Menu_AddMainMenuItem(&mi);

	// register hotkey
	HOTKEYDESC hkd = {};
	hkd.pszName = "QS_Global";
	hkd.szDescription.a = LPGEN("Open Quick Search window");
	hkd.szSection.a = LPGEN("Quick Search");
	hkd.pszService = QS_SHOWSERVICE;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_ALT, VK_F3);
	g_plugin.addHotkey(&hkd);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Quick Search"),  "QS",      IDI_QS     },
	{ LPGEN("New Column"),    "New",     IDI_NEW    },
	{ LPGEN("Column Up"),     "Up",      IDI_UP     },
	{ LPGEN("Column Down"),   "Down",    IDI_DOWN   },
	{ LPGEN("Delete Column"), "Delete",  IDI_DELETE },
	{ LPGEN("Default"),       "Default", IDI_DEFAULT},
	{ LPGEN("Reload"),        "Reload",  IDI_RELOAD },
	{ LPGEN("Male"),          "Male",    IDI_MALE   },
	{ LPGEN("Female"),        "Female",  IDI_FEMALE },
};

struct
{
	COLORREF defValue;
	const char *szSetting, *szDescr;
}
static sttColors[color_max] = {
	{ 0x00FFFFFF, "back_norm", LPGEN("Normal background")           },
	{ 0x00000000, "fore_norm", LPGEN("Normal foreground")           },
	{ 0x00EBE6DE, "back_odd" , LPGEN("Odd background")              },
	{ 0x00000000, "fore_odd" , LPGEN("Odd foreground")              },
	{ 0x008080FF, "back_dis" , LPGEN("Disabled account background") },
	{ 0x00000000, "fore_dis" , LPGEN("Disabled account foreground") },
	{ 0x008000FF, "back_del" , LPGEN("Deleted account background")  },
	{ 0x00000000, "fore_del" , LPGEN("Deleted account foreground")  },
	{ 0x0080FFFF, "back_hid" , LPGEN("Hidden contact background")   },
	{ 0x00000000, "fore_hid" , LPGEN("Hidden contact foreground")   },
	{ 0x00BAE699, "back_meta", LPGEN("Metacontact background")      },
	{ 0x00000000, "fore_meta", LPGEN("Metacontact foreground")      },
	{ 0x00B3CCC1, "back_sub" , LPGEN("Subcontact background")       },
	{ 0x00000000, "fore_sub" , LPGEN("Subcontact foreground")       },
};

static int OnColorReload(WPARAM, LPARAM)
{
	for (int i = 0; i < color_max; i++)
		g_plugin.m_colors[i] = Colour_Get(MODULENAME, sttColors[i].szDescr);
	return 0;
}

int CMPlugin::Load()
{
	g_plugin.registerIcon(MODULENAME, iconList);

	ColourID colourid = {};
	strncpy_s(colourid.group, MODULENAME, _TRUNCATE);
	strncpy_s(colourid.dbSettingsGroup, MODULENAME, _TRUNCATE);

	for (auto &it : sttColors) {
		strncpy_s(colourid.name, it.szDescr, _TRUNCATE);
		strncpy_s(colourid.setting, it.szSetting, _TRUNCATE);
		colourid.defcolour = it.defValue;
		colourid.order = int(&it - sttColors);
		g_plugin.addColor(&colourid);
	}
	OnColorReload(0, 0);
	OnCheckPlugins(0, 0);

	HookEvent(ME_COLOUR_RELOAD, OnColorReload);
	HookEvent(ME_OPT_INITIALISE, OnOptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SYSTEM_MODULELOAD, OnCheckPlugins);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, OnCheckPlugins);

	if (!LoadColumns(m_columns))
		LoadDefaultColumns(m_columns);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	if (hTTBButton) {
		CallService(MS_TTB_REMOVEBUTTON, (WPARAM)hTTBButton, 0);
		hTTBButton = 0;
	}

	CloseSrWindow();
	return 0;
}

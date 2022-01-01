/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

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
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "jabber_rc.h"

#include "m_assocmgr.h"
#include "m_folders.h"
#include "m_toptoolbar.h"

#pragma comment(lib, "Dnsapi.lib")
#pragma comment(lib, "Secur32.lib")

HMODULE hMsftedit;

CMPlugin g_plugin;

int g_cbCountries;
CountryListEntry *g_countries;

unsigned int g_nTempFileId;
char szCoreVersion[100];

HANDLE hExtraActivity = nullptr;
HANDLE hExtraMood = nullptr;
HANDLE hExtListInit, hDiscoInfoResult;

void JabberUserInfoInit(void);
void JabberUserInfoUninit(void);

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	{ 0x144e80a2, 0xd198, 0x428b, {0xac, 0xbe, 0x9d, 0x55, 0xda, 0xcc, 0x7f, 0xde }} // {144E80A2-D198-428b-ACBE-9D55DACC7FDE}
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CJabberProto>("JABBER", pluginInfoEx)
{
	char tmp[8];
	Utils_GetRandom(tmp, sizeof(tmp));
	bin2hex(tmp, sizeof(tmp), szRandom);

	SetUniqueId("jid");
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

///////////////////////////////////////////////////////////////////////////////
// OnModulesLoaded - execute some code when all plugins are initialized

static INT_PTR g_SvcParseXmppUri(WPARAM w, LPARAM l)
{
	if (CJabberProto *ppro = JabberChooseInstance(true))
		return ppro->JabberServiceParseXmppURI(w, l);
	return 0;
}

static int OnLoadModule(WPARAM, LPARAM)
{
	g_plugin.bMessageState = ServiceExists(MS_MESSAGESTATE_UPDATE);
	g_plugin.bSecureIM = ServiceExists("SecureIM/IsContactSecured") != 0;
	g_plugin.bMirOTR = GetModuleHandle(L"mirotr.dll") != nullptr;
	g_plugin.bNewGPG = GetModuleHandle(L"new_gpg.dll") != nullptr;
	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_TTB_MODULELOADED, g_OnToolbarInit);

	HookEvent(ME_SYSTEM_MODULELOAD, OnLoadModule);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, OnLoadModule);
	OnLoadModule(0, 0);

	// file associations manager plugin support
	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE)) {
		CreateServiceFunction("JABBER/*" JS_PARSE_XMPP_URI, g_SvcParseXmppUri);
		AssocMgr_AddNewUrlTypeW("xmpp:", TranslateT("Jabber Link Protocol"), g_plugin.getInst(), IDI_JABBER, "JABBER/*" JS_PARSE_XMPP_URI, 0);
	}

	// init fontservice for info frame
	FontIDW fontid = {};
	wcsncpy_s(fontid.group, LPGENW("Jabber"), _TRUNCATE);
	strncpy_s(fontid.dbSettingsGroup, GLOBAL_SETTING_MODULE, _TRUNCATE);
	wcsncpy_s(fontid.backgroundGroup, L"Jabber", _TRUNCATE);
	wcsncpy_s(fontid.backgroundName, L"Background", _TRUNCATE);
	fontid.flags = FIDF_DEFAULTVALID;

	fontid.deffontsettings.charset = DEFAULT_CHARSET;
	fontid.deffontsettings.colour = GetSysColor(COLOR_WINDOWTEXT);
	fontid.deffontsettings.size = -11;
	mir_wstrncpy(fontid.deffontsettings.szFace, L"MS Shell Dlg", _countof(fontid.deffontsettings.szFace));
	fontid.deffontsettings.style = 0;

	wcsncpy_s(fontid.name, LPGENW("Frame title"), _TRUNCATE);
	strncpy_s(fontid.setting, "fntFrameTitle", _TRUNCATE);
	fontid.deffontsettings.style = DBFONTF_BOLD;
	g_plugin.addFont(&fontid);

	wcsncpy_s(fontid.name, LPGENW("Frame text"), _TRUNCATE);
	strncpy_s(fontid.setting, "fntFrameClock", _TRUNCATE);
	fontid.deffontsettings.style = 0;
	g_plugin.addFont(&fontid);

	ColourIDW colourid = {};
	wcsncpy_s(colourid.group, L"Jabber", _TRUNCATE);
	strncpy_s(colourid.dbSettingsGroup, GLOBAL_SETTING_MODULE, _TRUNCATE);

	wcsncpy_s(colourid.name, L"Background", _TRUNCATE);
	strncpy_s(colourid.setting, "clFrameBack", _TRUNCATE);
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	g_plugin.addColor(&colourid);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// OnLoad - initialize the plugin instance

int CMPlugin::Load()
{
	#ifdef _WIN64
	bPlatform = true;
	#else
	bPlatform = false;
	#endif

	Miranda_GetVersionText(szCoreVersion, _countof(szCoreVersion));

	CallService(MS_UTILS_GETCOUNTRYLIST, (WPARAM)&g_cbCountries, (LPARAM)&g_countries);

	hMsftedit = LoadLibrary(L"Msftedit.dll");

	hExtListInit = CreateHookableEvent(ME_JABBER_EXTLISTINIT);
	hDiscoInfoResult = CreateHookableEvent(ME_JABBER_SRVDISCOINFO);

	g_IconsInit();
	g_XstatusIconsInit();

	// Init extra icons
	hExtraActivity = ExtraIcon_RegisterIcolib("activity", LPGEN("Jabber Activity"), "jabber_dancing");
	hExtraMood = ExtraIcon_RegisterIcolib("mood", LPGEN("Jabber Mood"), "jabber_contemplative");
	g_MenuInit();
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	JabberUserInfoInit();

	if (db_get_b(0, "Compatibility", "JabberCaps", 0) < 3) {
		db_delete_module(0, "JabberCaps");
		DeleteFileW(VARSW(L"%miranda_userdata%\\jabberCaps.json"));
		db_set_b(0, "Compatibility", "JabberCaps", 3);
	}

	g_clientCapsManager.Load();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Unload - destroy the plugin instance

int CMPlugin::Unload()
{
	g_clientCapsManager.Save();

	g_XstatusIconsUninit();
	JabberUserInfoUninit();

	if (hMsftedit != nullptr)
		FreeLibrary(hMsftedit);

	DestroyHookableEvent(hExtListInit);
	DestroyHookableEvent(hDiscoInfoResult);

	if (g_nTempFileId != 0) {
		wchar_t tszTempPath[MAX_PATH], tszFilePath[MAX_PATH];
		GetTempPath(_countof(tszTempPath), tszTempPath);
		mir_snwprintf(tszFilePath, L"%sjab*.tmp.*", tszTempPath);

		WIN32_FIND_DATA findData;
		HANDLE hFind = FindFirstFile(tszFilePath, &findData);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				mir_snwprintf(tszFilePath, L"%s%s", tszTempPath, findData.cFileName);
				DeleteFile(tszFilePath);
			} while (FindNextFile(hFind, &findData));
			
			FindClose(hFind);
		}
	}

	g_MenuUninit();
	return 0;
}

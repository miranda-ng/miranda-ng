/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-17 Miranda NG project

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

HINSTANCE hInst;
HMODULE hMsftedit;

int hLangpack;
unsigned int g_nTempFileId;
CHAT_MANAGER *pci;

int g_cbCountries;
CountryListEntry *g_countries;

wchar_t szCoreVersion[100];

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
    {0x144e80a2, 0xd198, 0x428b, {0xac, 0xbe, 0x9d, 0x55, 0xda, 0xcc, 0x7f, 0xde}} // {144E80A2-D198-428b-ACBE-9D55DACC7FDE}
};

CLIST_INTERFACE* pcli;
FI_INTERFACE *FIP = nullptr;

HANDLE hExtraActivity = nullptr;
HANDLE hExtraMood = nullptr;
HANDLE hExtListInit, hDiscoInfoResult;

void JabberUserInfoInit(void);
void JabberUserInfoUninit(void);

bool bSecureIM, bMirOTR, bNewGPG, bPlatform;

/////////////////////////////////////////////////////////////////////////////
// Protocol instances
static int sttCompareProtocols(const CJabberProto *p1, const CJabberProto *p2)
{
	return mir_wstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

LIST<CJabberProto> g_Instances(1, sttCompareProtocols);
/////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD, LPVOID)
{
	hInst = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

///////////////////////////////////////////////////////////////////////////////
// OnModulesLoaded - execute some code when all plugins are initialized

static INT_PTR g_SvcParseXmppUri(WPARAM w, LPARAM l)
{
	if (CJabberProto *ppro = JabberChooseInstance(true))
		return ppro->JabberServiceParseXmppURI(w, l);
	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_TTB_MODULELOADED, g_OnToolbarInit);

	bSecureIM = ServiceExists("SecureIM/IsContactSecured") != 0;
	bMirOTR = GetModuleHandle(L"mirotr.dll") != nullptr;
	bNewGPG = GetModuleHandle(L"new_gpg.dll") != nullptr;
	#ifdef _WIN64
		bPlatform = 1;
	#else
		bPlatform = 0;
	#endif

	// file associations manager plugin support
	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE)) {
		CreateServiceFunction("JABBER/*" JS_PARSE_XMPP_URI, g_SvcParseXmppUri);
		AssocMgr_AddNewUrlTypeT("xmpp:", TranslateT("Jabber Link Protocol"), hInst, IDI_JABBER, "JABBER/*" JS_PARSE_XMPP_URI, 0);
	}

	// init fontservice for info frame
	FontIDW fontid = { 0 };
	fontid.cbSize = sizeof(fontid);
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
	strncpy_s(fontid.prefix, "fntFrameTitle", _TRUNCATE);
	fontid.deffontsettings.style = DBFONTF_BOLD;
	Font_RegisterW(&fontid);

	wcsncpy_s(fontid.name, LPGENW("Frame text"), _TRUNCATE);
	strncpy_s(fontid.prefix, "fntFrameClock", _TRUNCATE);
	fontid.deffontsettings.style = 0;
	Font_RegisterW(&fontid);

	ColourIDW colourid = {0};
	colourid.cbSize = sizeof(colourid);
	wcsncpy_s(colourid.group, L"Jabber", _TRUNCATE);
	strncpy_s(colourid.dbSettingsGroup, GLOBAL_SETTING_MODULE, _TRUNCATE);

	wcsncpy_s(colourid.name, L"Background", _TRUNCATE);
	strncpy_s(colourid.setting, "clFrameBack", _TRUNCATE);
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	Colour_RegisterW(&colourid);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// OnLoad - initialize the plugin instance

static CJabberProto* jabberProtoInit(const char* pszProtoName, const wchar_t *tszUserName)
{
	CJabberProto *ppro = new CJabberProto(pszProtoName, tszUserName);
	g_Instances.insert(ppro);
	return ppro;
}

static int jabberProtoUninit(CJabberProto *ppro)
{
	g_Instances.remove(ppro);
	delete ppro;
	return 0;
}


extern "C" int __declspec(dllexport) Load()
{
	// set the memory, lists & utf8 managers
	mir_getLP(&pluginInfo);
	pci = Chat_GetInterface();
	pcli = Clist_GetInterface();

	INT_PTR result = CallService(MS_IMG_GETINTERFACE, FI_IF_VERSION, (LPARAM)&FIP);
	if (FIP == nullptr || result != S_OK) {
		MessageBoxEx(nullptr, TranslateT("Fatal error, image services not found. Jabber Protocol will be disabled."), L"Error", MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);
		return 1;
	}

	char mirVer[100];
	Miranda_GetVersionText(mirVer, _countof(mirVer));
	mir_wstrcpy(szCoreVersion, _A2T(mirVer));

	CallService(MS_UTILS_GETCOUNTRYLIST, (WPARAM)&g_cbCountries, (LPARAM)&g_countries);

	hMsftedit = LoadLibrary(L"Msftedit.dll");

	hExtListInit = CreateHookableEvent(ME_JABBER_EXTLISTINIT);
	hDiscoInfoResult = CreateHookableEvent(ME_JABBER_SRVDISCOINFO);

	// Register protocol module
	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.szName = "JABBER";
	pd.fnInit = (pfnInitProto)jabberProtoInit;
	pd.fnUninit = (pfnUninitProto)jabberProtoUninit;
	pd.type = PROTOTYPE_PROTOCOL;
	Proto_RegisterModule(&pd);

	g_IconsInit();
	g_XstatusIconsInit();

	// Init extra icons
	hExtraActivity = ExtraIcon_RegisterIcolib("activity", LPGEN("Jabber Activity"), "jabber_dancing");
	hExtraMood = ExtraIcon_RegisterIcolib("mood", LPGEN("Jabber Mood"), "jabber_contemplative");
	g_MenuInit();
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	JabberUserInfoInit();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Unload - destroy the plugin instance

extern "C" int __declspec(dllexport) Unload(void)
{
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

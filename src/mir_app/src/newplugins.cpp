/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "stdafx.h"

#include "clc.h"
#include "plugins.h"
#include "profilemanager.h"
#include "langpack.h"
#include "netlib.h"

#define PLUGINDISABLELIST "PluginDisable"

bool g_bReadyToInitClist = false;

void LoadExtraIconsModule();
void freePluginInstance(HINSTANCE hInst);

static int sttComparePluginsByName(const pluginEntry *p1, const pluginEntry *p2)
{
	return mir_strcmpi(p1->pluginname, p2->pluginname);
}

LIST<pluginEntry>
	pluginList(10, sttComparePluginsByName),
	servicePlugins(5, sttComparePluginsByName),
	clistPlugins(5, sttComparePluginsByName);

/////////////////////////////////////////////////////////////////////////////////////////

MUUID miid_last = MIID_LAST;

/////////////////////////////////////////////////////////////////////////////////////////

static bool bModuleInitialized = FALSE;
HANDLE hevLoadModule, hevUnloadModule;

wchar_t mirandabootini[MAX_PATH];
static int askAboutIgnoredPlugins;

pluginEntry *plugin_checker, *plugin_crshdmp, *plugin_service, *plugin_clist;

/////////////////////////////////////////////////////////////////////////////////////////

struct CPluginName : public CMStringA
{
	CPluginName(const char *szName) :
		CMStringA(szName)
	{
		MakeLower();
		if (Find(".dll") == -1)
			Append(".dll");
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// basic functions

bool hasMuuid(const MUUID *p, const MUUID &uuid)
{
	if (p == nullptr)
		return false;

	for (int i = 0; p[i] != miid_last; i++)
		if (p[i] == uuid)
			return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// banned plugins

static const MUUID pluginBannedList[] =
{
	{ 0x9d6c3213, 0x02b4, 0x4fe1, { 0x92, 0xe6, 0x52, 0x6d, 0xe2, 0x4f, 0x8d, 0x65 } },  // old chat
	{ 0x240a91dc, 0x9464, 0x457a, { 0x97, 0x87, 0xff, 0x1e, 0xa8, 0x8e, 0x77, 0xe3 } },  // old clist
	{ 0x657fe89b, 0xd121, 0x40c2, { 0x8a, 0xc9, 0xb9, 0xfa, 0x57, 0x55, 0xb3, 0x0c } },  // old srmm
	{ 0x112f7d30, 0xcd19, 0x4c74, { 0xa0, 0x3b, 0xbf, 0xbb, 0x76, 0xb7, 0x5b, 0xc4 } },  // extraicons
	{ 0x72765a6f, 0xb017, 0x42f1, { 0xb3, 0x0f, 0x5e, 0x09, 0x41, 0x27, 0x3a, 0x3f } },  // flashavatars
	{ 0x1394a3ab, 0x2585, 0x4196, { 0x8f, 0x72, 0x0e, 0xae, 0xc2, 0x45, 0x0e, 0x11 } },  // db3x
	{ 0x28ff9b91, 0x3e4d, 0x4f1c, { 0xb4, 0x7c, 0xc6, 0x41, 0xb0, 0x37, 0xff, 0x40 } },  // dbx_mmap_sa
	{ 0x28f45248, 0x8c9c, 0x4bee, { 0x93, 0x07, 0x7b, 0xcf, 0x3e, 0x12, 0xbf, 0x99 } },  // dbx_tree
	{ 0x4c4a27cf, 0x5e64, 0x4242, { 0xa3, 0x32, 0xb9, 0x8b, 0x08, 0x24, 0x3e, 0x89 } },  // metacontacts
	{ 0x9c448c61, 0xfc3f, 0x42f9, { 0xb9, 0xf0, 0x4a, 0x30, 0xe1, 0xcf, 0x86, 0x71 } },  // skypekit based skype
	{ 0x49c2cf54, 0x7898, 0x44de, { 0xbe, 0x3a, 0x6d, 0x2e, 0x4e, 0xf9, 0x00, 0x79 } },  // firstrun
	{ 0x0ca63eee, 0xeb2c, 0x4aed, { 0xb3, 0xd0, 0xbc, 0x8e, 0x6e, 0xb3, 0xbf, 0xb8 } },  // stdurl
	{ 0x0aa7bfea, 0x1fc7, 0x45f0, { 0x90, 0x6e, 0x2a, 0x46, 0xb6, 0xe1, 0x19, 0xcf } },  // yahoo
	{ 0x2f3fe8b9, 0x7327, 0x4008, { 0xa6, 0x0d, 0x93, 0xf0, 0xf4, 0xf7, 0xf0, 0xf1 } },  // yahoogroups
	{ 0xf0fdf73a, 0x753d, 0x499d, { 0x8d, 0xba, 0x33, 0x6d, 0xb7, 0x9c, 0xdd, 0x41 } },  // advancedautoaway
	{ 0xa5bb1b7a, 0xb7cd, 0x4cbb, { 0xa7, 0xdb, 0xce, 0xb4, 0xeb, 0x71, 0xda, 0x49 } },  // keepstatus
	{ 0x4b733944, 0x5a70, 0x4b52, { 0xab, 0x2d, 0x68, 0xb1, 0xef, 0x38, 0xff, 0xe8 } },  // startupstatus
	{ 0x8d0a046d, 0x8ea9, 0x4c55, { 0xb5, 0x68, 0x38, 0xda, 0x52, 0x05, 0x64, 0xfd } },  // stdauth
	{ 0x9d6c3213, 0x02b4, 0x4fe1, { 0x92, 0xe6, 0x52, 0x6d, 0xe1, 0x4f, 0x8d, 0x65 } },  // stdchat
	{ 0x1e64fd80, 0x299e, 0x48a0, { 0x94, 0x41, 0xde, 0x28, 0x68, 0x56, 0x3b, 0x6f } },  // stdhelp
	{ 0x53ac190b, 0xe223, 0x4341, { 0x82, 0x5f, 0x70, 0x9d, 0x85, 0x20, 0x21, 0x5b } },  // stdidle
	{ 0x312C4F84, 0x75BE, 0x4404, { 0xBC, 0xB1, 0xC1, 0x03, 0xDB, 0xE5, 0xA3, 0xB8 } },  // stdssl
	{ 0x621f886b, 0xa7f6, 0x457f, { 0x9d, 0x62, 0x8e, 0xe8, 0x4c, 0x27, 0x59, 0x93 } },  // modernopt
	{ 0x08B86253, 0xEC6E, 0x4d09, { 0xB7, 0xA9, 0x64, 0xAC, 0xDF, 0x06, 0x27, 0xB8 } },  // gtalkext
	{ 0x4f1ff7fa, 0x4d75, 0x44b9, { 0x93, 0xb0, 0x2c, 0xed, 0x2e, 0x4f, 0x9e, 0x3e } },  // whatsapp
	{ 0xb908773a, 0x86f7, 0x4a91, { 0x86, 0x74, 0x6a, 0x20, 0xba, 0x0e, 0x67, 0xd1 } },  // dropbox
	{ 0x748f8934, 0x781a, 0x528d, { 0x52, 0x08, 0x00, 0x12, 0x65, 0x40, 0x4a, 0xb3 } },  // tlen
	{ 0x3750a5a3, 0xbf0d, 0x490e, { 0xb6, 0x5d, 0x41, 0xac, 0x4d, 0x29, 0xae, 0xb3 } },  // aim
	{ 0x7c070f7c, 0x459e, 0x46b7, { 0x8e, 0x6d, 0xbc, 0x6e, 0xfa, 0xa2, 0x2f, 0x78 } },  // advaimg
	{ 0x6f376b33, 0xd3f4, 0x4c4f, { 0xa9, 0x6b, 0x77, 0xda, 0x08, 0x04, 0x3b, 0x06 } },  // importtxt
	{ 0xe7c48bab, 0x8ace, 0x4cb3, { 0x84, 0x46, 0xd4, 0xb7, 0x34, 0x81, 0xf4, 0x97 } },  // mra
	{ 0xb649702c, 0x13de, 0x408a, { 0xb6, 0xc2, 0xfb, 0x8f, 0xed, 0x2a, 0x2c, 0x90 } },  // openssl
	{ 0x73a9615c, 0x7d4e, 0x4555, { 0xba, 0xdb, 0xee, 0x05, 0xdc, 0x92, 0x8e, 0xff } },  // icqoscar8
	{ 0xcf97fd5d, 0xb911, 0x47a8, { 0xaf, 0x03, 0xd2, 0x19, 0x68, 0xb5, 0xb8, 0x94 } },  // sms
	{ 0x26a9125d, 0x7863, 0x4e01, { 0xaf, 0x0e, 0xd1, 0x4e, 0xf9, 0x5c, 0x50, 0x54 } },  // old Popup.dll
	{ 0x69b9443b, 0xdc58, 0x4876, { 0xad, 0x39, 0xe3, 0xf4, 0x18, 0xa1, 0x33, 0xc5 } },  // mTextControl
	{ 0x97724af9, 0xf3fb, 0x47d3, { 0xa3, 0xbf, 0xea, 0xa9, 0x35, 0xc7, 0x4e, 0x6d } },  // msn
};

bool isPluginBanned(const MUUID &u1)
{
	for (auto &it : pluginBannedList)
		if (it == u1)
			return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// default plugins

static MuuidReplacement pluginDefault[] =
{
	{ MIID_CLIST,      L"stdclist",      nullptr }, // 0
	{ MIID_SRMM,       L"stdmsg",        nullptr }, // 1
	{ MIID_UIUSERINFO, L"stduserinfo",   nullptr }, // 2
	{ MIID_SREMAIL,    L"stdemail",      nullptr }, // 3
	{ MIID_SRFILE,     L"stdfile",       nullptr }, // 4
	{ MIID_UIHISTORY,  L"stduihist",     nullptr }, // 5
	{ MIID_AUTOAWAY,   L"stdautoaway",   nullptr }, // 6
	{ MIID_USERONLINE, L"stduseronline", nullptr }, // 7
	{ MIID_SRAWAY,     L"stdaway",       nullptr }, // 8
	{ MIID_POPUP,      L"stdpopup",      nullptr }, // 9
};

int getDefaultPluginIdx(const MUUID &muuid)
{
	for (int i = 0; i < _countof(pluginDefault); i++)
		if (muuid == pluginDefault[i].uuid)
			return i;

	return -1;
}

bool MuuidReplacement::Preload()
{
	wchar_t tszPlugName[MAX_PATH];
	mir_snwprintf(tszPlugName, L"%s.dll", stdplugname);

	pluginEntry *ppe = OpenPlugin(tszPlugName, L"Core", VARSW(L"%miranda_path%"));
	if (ppe == nullptr) {
		pImpl = nullptr;
		return false;
	}

	ppe->bIsCore = true;
	pImpl = ppe;
	return true;
}

bool MuuidReplacement::Load()
{
	if (pImpl == nullptr)
		if (!Preload())
			return false;

	if (!TryLoadPlugin(pImpl, true)) {
LBL_Error:
		Plugin_UnloadDyn(pImpl);
		pImpl = nullptr;
		return false;
	}

	CMPluginBase *ppb = pImpl->m_pPlugin;
	if (g_bModulesLoadedFired) {
		if (CallPluginEventHook(ppb->getInst(), ME_SYSTEM_MODULESLOADED, 0, 0) != 0)
			goto LBL_Error;

		NotifyEventHooks(hevLoadModule, (WPARAM)ppb, (LPARAM)ppb->getInst());
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// global functions

static bool validInterfaceList(const MUUID *piface)
{
	if (piface == nullptr)
		return true;

	if (miid_last == piface[0])
		return false;

	return true;
}

bool pluginEntry::checkAPI(wchar_t *plugin)
{
	SetErrorMode(SEM_FAILCRITICALERRORS); // disable error messages
	HINSTANCE h = LoadLibrary(plugin);
	SetErrorMode(0);							  // reset the system default
	if (h == nullptr)
		return false;

	// dll must register itself during LoadLibrary
	CMPluginBase &ppb = GetPluginByInstance(h);
	if (ppb.getInst() != h) {
LBL_Error:
		bFailed = true;
		clear();
		FreeLibrary(h);
		return false;
	}

	m_pPlugin = &ppb;
	pfnLoad = (Miranda_Plugin_Load)GetProcAddress(h, "Load");
	pfnUnload = (Miranda_Plugin_Unload)GetProcAddress(h, "Unload");
	
	m_pInterfaces = (MUUID*)GetProcAddress(h, "MirandaInterfaces");
	if (m_pInterfaces == nullptr) {
		// MirandaPluginInterfaces function is actual only for the only plugin, HistoryPlusPlus
		// plugins written in C++ shall export data directly
		typedef MUUID* (__cdecl * Miranda_Plugin_Interfaces)(void);
		Miranda_Plugin_Interfaces pFunc = (Miranda_Plugin_Interfaces)GetProcAddress(h, "MirandaPluginInterfaces");
		if (pFunc)
			m_pInterfaces = pFunc();
	}

	if (!validInterfaceList(m_pInterfaces))
		goto LBL_Error;

	const PLUGININFOEX &pInfo = ppb.getInfo();
	if (pInfo.cbSize != sizeof(PLUGININFOEX))
		goto LBL_Error;

	if (isPluginBanned(pInfo.uuid))
		goto LBL_Error;

	if (!pInfo.shortName || !pInfo.description || !pInfo.author || !pInfo.copyright || !pInfo.homepage)
		goto LBL_Error;

	// basic API is present
	bHasBasicApi = true;
	return true;
}

// perform any API related tasks to freeing
void Plugin_Uninit(pluginEntry *p)
{
	// if the basic API check had passed, call Unload if Load(void) was ever called
	if (p->bLoaded) {
		p->unload();
		p->bLoaded = false;
	}

	// release the library
	if (p->m_pPlugin != nullptr) {
		HINSTANCE hInst = p->m_pPlugin->getInst();

		// we need to kill all resources which belong to that DLL before calling FreeLibrary
		KillModuleAccounts(hInst);
		KillModuleSubclassing(hInst);
		KillModuleEventHooks(hInst);
		KillModuleServices(hInst);

		FreeLibrary(hInst);
		p->clear();
	}

	if (p == plugin_crshdmp)
		plugin_crshdmp = nullptr;

	for (auto &it : pluginDefault)
		if (it.pImpl == p)
			it.pImpl = nullptr;

	servicePlugins.remove(p);
	pluginList.remove(p);
	mir_free(p);
}

bool Plugin_UnloadDyn(pluginEntry *p)
{
	if (p == nullptr)
		return true;

	MuuidReplacement *stdPlugin = nullptr;

	// mark default plugins to be loaded
	if (!p->bIsCore)
		for (auto &it : pluginDefault)
			if (it.pImpl == p) {
				if (!it.Preload()) {
					MessageBoxW(nullptr,
						CMStringW(FORMAT, TranslateT("Plugin %S cannot be unloaded because the core plugin is missing"), p->pluginname),
						L"Miranda", MB_ICONERROR | MB_OK);
					it.pImpl = p;
					return false;
				}

				stdPlugin = &it;
				break;
			}

	// if plugin has active resources, kill them forcibly
	CMPluginBase *ppb = p->m_pPlugin;
	if (ppb != nullptr) {
		if (HINSTANCE hInst = ppb->getInst()) {
			if (CallPluginEventHook(hInst, ME_SYSTEM_OKTOEXIT, 0, 0) != 0)
				return false;

			KillModuleAccounts(hInst);
			KillModuleSubclassing(hInst);

			CallPluginEventHook(hInst, ME_SYSTEM_PRESHUTDOWN, 0, 0);
			CallPluginEventHook(hInst, ME_SYSTEM_SHUTDOWN, 0, 0);

			KillModuleEventHooks(hInst);
			KillModuleServices(hInst);
		}

		freePluginInstance(ppb->getInst());

		NotifyFastHook(hevUnloadModule, (WPARAM)&ppb, (LPARAM)ppb->getInst());
	}

	Plugin_Uninit(p);

	if (stdPlugin)
		stdPlugin->Load();
	return true;
}

// returns true if the given file is <anything>.dll exactly
static int valid_library_name(wchar_t *name)
{
	wchar_t *dot = wcsrchr(name, '.');
	if (dot != nullptr && mir_wstrcmpi(dot + 1, L"dll") == 0)
		if (dot[4] == 0)
			return 1;

	return 0;
}

void enumPlugins(SCAN_PLUGINS_CALLBACK cb, WPARAM wParam, LPARAM lParam)
{
	// get miranda's exe path
	wchar_t exe[MAX_PATH];
	GetModuleFileName(nullptr, exe, _countof(exe));
	wchar_t *p = wcsrchr(exe, '\\'); if (p) *p = 0;

	// create the search filter
	wchar_t search[MAX_PATH];
	mir_snwprintf(search, L"%s\\Plugins\\*.dll", exe);

	// FFFN will return filenames for things like dot dll+ or dot dllx
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(search, &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do {
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && valid_library_name(ffd.cFileName))
			cb(&ffd, exe, wParam, lParam);
	} while (FindNextFile(hFind, &ffd));
	FindClose(hFind);
}

pluginEntry* OpenPlugin(wchar_t *tszFileName, wchar_t *dir, wchar_t *path)
{
	wchar_t tszFullPath[MAX_PATH];
	mir_snwprintf(tszFullPath, L"%s\\%s\\%s", path, dir, tszFileName);

	// map dll into the memory and check its exports
	bool bIsPlugin = false;
	mir_ptr<MUUID> pIds(GetPluginInterfaces(tszFullPath, bIsPlugin));
	if (!bIsPlugin)
		return nullptr;

	pluginEntry *p = (pluginEntry*)mir_calloc(sizeof(pluginEntry));
	strncpy_s(p->pluginname, _T2A(tszFileName), _TRUNCATE);

	// add it to the list anyway
	pluginList.insert(p);

	// plugin declared that it's a database or a cryptor. load it asap!
	bool bIsDb = hasMuuid(pIds, MIID_DATABASE);
	if (bIsDb || hasMuuid(pIds, MIID_CRYPTO)) {
		if (p->checkAPI(tszFullPath)) {
			// plugin is valid
			p->bIsLast = true;
			if (bIsDb)
				p->bIsDatabase = true;
			else
				p->bIsCrypto = true;

			if (p->load() != 0)
				p->bFailed = true;
			else
				p->bLoaded = true;
		}
	}
	// plugin declared that it's a contact list. mark it for the future load
	else if (hasMuuid(pIds, MIID_CLIST)) {
		// keep a note of this plugin for later
		clistPlugins.insert(p);
		p->bIsClist = true;
	}
	// plugin declared that it's a service mode plugin.
	// load it for a profile manager's window
	else if (hasMuuid(pIds, MIID_SERVICEMODE)) {
		if (p->checkAPI(tszFullPath)) {
			if (hasMuuid(pIds, MIID_SERVICEMODE)) {
				p->bIsService = true;
				servicePlugins.insert(p);
			}
		}
	}
	else if (hasMuuid(pIds, MIID_PROTOCOL) || !mir_wstrcmpi(tszFileName, L"mradio.dll") || !mir_wstrcmpi(tszFileName, L"watrack.dll"))
		p->bIsProtocol = true;

	if (plugin_checker == nullptr && !mir_wstrcmpi(tszFileName, L"dbchecker.dll")) {
		plugin_checker = p;
		p->bIsLast = true;
	}

	if (plugin_crshdmp == nullptr && !mir_wstrcmpi(tszFileName, L"crashdumper.dll")) {
		plugin_crshdmp = p;
		p->bIsLast = true;
	}

	return p;
}

MIR_APP_DLL(void) SetPluginOnWhiteList(const char* szPluginName, bool bAllow)
{
	if (szPluginName == nullptr)
		return;

	CPluginName tmp(szPluginName);
	if (bAllow)
		db_unset(0, PLUGINDISABLELIST, tmp);
	else
		db_set_b(0, PLUGINDISABLELIST, tmp, 1);
}

// returns 1 if the plugin should be enabled within this profile, filename is always lower case
MIR_APP_DLL(bool) IsPluginOnWhiteList(const char *szPluginName)
{
	if (szPluginName == nullptr)
		return false;

	CPluginName tmp(szPluginName);
	int rc = db_get_b(0, PLUGINDISABLELIST, tmp, 0);
	if (rc != 0 && askAboutIgnoredPlugins) {
		wchar_t buf[256];
		mir_snwprintf(buf, TranslateT("'%S' is disabled, re-enable?"), tmp.c_str());
		if (MessageBoxW(nullptr, buf, TranslateT("Re-enable Miranda plugin?"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			SetPluginOnWhiteList(szPluginName, true);
			rc = 0;
		}
	}

	return rc == 0;
}

bool TryLoadPlugin(pluginEntry *p, bool bDynamic)
{
	if (!bDynamic && !IsPluginOnWhiteList(p->pluginname))
		return false;

	if (p->m_pPlugin == nullptr) {
		if (!p->bHasBasicApi) {
			wchar_t exe[MAX_PATH], tszFullPath[MAX_PATH];
			GetModuleFileName(nullptr, exe, _countof(exe));
			wchar_t* slice = wcsrchr(exe, '\\');
			if (slice)
				*slice = 0;

			mir_snwprintf(tszFullPath, L"%s\\%s\\%S", exe, (p->bIsCore) ? L"Core" : L"Plugins", p->pluginname);
			if (!p->checkAPI(tszFullPath))
				return false;
		}

		if (p->m_pInterfaces) {
			MUUID *piface = p->m_pInterfaces;
			for (int i = 0; piface[i] != miid_last; i++) {
				int idx = getDefaultPluginIdx(piface[i]);
				if (idx != -1 && pluginDefault[idx].pImpl) {
					if (!bDynamic) { // this place is already occupied, skip & disable
						SetPluginOnWhiteList(p->pluginname, false);
						return false;
					}

					// we're loading new implementation dynamically, let the old one die
					if (!p->bIsCore)
						Plugin_UnloadDyn(pluginDefault[idx].pImpl);
				}
			}
		}
	}

	// contact list is loaded via clistlink, db - via DATABASELINK
	// so we should call Load() only for usual plugins
	if (!p->bLoaded && !p->bIsClist && !p->bIsDatabase) {
		if (p->load() != 0)
			return false;

		p->bLoaded = true;
		if (p->m_pInterfaces) {
			MUUID *piface = p->m_pInterfaces;
			for (int i = 0; piface[i] != miid_last; i++) {
				int idx = getDefaultPluginIdx(piface[i]);
				if (idx != -1)
					pluginDefault[idx].pImpl = p;
			}
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Contact list plugins support

static bool loadClistModule(wchar_t *exe, pluginEntry *p)
{
	g_bReadyToInitClist = true;

	if (p->checkAPI(exe)) {
		p->bIsLast = true;

		hCListImages = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 13, 0);
		ImageList_AddIcon_NotShared(hCListImages, MAKEINTRESOURCE(IDI_BLANK));

		// now all core skin icons are loaded via icon lib. so lets release them
		for (auto &it : g_statuses)
			ImageList_AddSkinIcon(hCListImages, it.iSkinIcon);

		// see IMAGE_GROUP... in clist.h if you add more images above here
		ImageList_AddSkinIcon(hCListImages, SKINICON_OTHER_GROUPOPEN);
		ImageList_AddSkinIcon(hCListImages, SKINICON_OTHER_GROUPSHUT);

		if (p->load() == 0) {
			p->bLoaded = true;
			pluginDefault[0].pImpl = p;

			LoadExtraIconsModule();
			return true;
		}
		Plugin_Uninit(p);
	}
	return false;
}

static pluginEntry* getCListModule(wchar_t *exe)
{
	wchar_t tszFullPath[MAX_PATH];

	for (auto &p : clistPlugins) {
		if (!IsPluginOnWhiteList(p->pluginname))
			continue;

		mir_snwprintf(tszFullPath, L"%s\\Plugins\\%S", exe, p->pluginname);
		if (loadClistModule(tszFullPath, p))
			return p;
	}

	MuuidReplacement &stdClist = pluginDefault[0];
	if (stdClist.Load()) {
		mir_snwprintf(tszFullPath, L"%s\\Core\\%s.dll", exe, stdClist.stdplugname);
		if (loadClistModule(tszFullPath, stdClist.pImpl))
			return stdClist.pImpl;
	}

	return nullptr;
}

int UnloadPlugin(wchar_t* buf, int bufLen)
{
	ptrA szShortName(mir_u2a(buf));
	for (auto &it : pluginList.rev_iter()) {
		if (!mir_strcmpi(it->pluginname, szShortName)) {
			GetModuleFileName(it->m_pPlugin->getInst(), buf, bufLen);
			Plugin_Uninit(it);
			return TRUE;
		}
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Service plugins functions

static WPARAM g_srvWParam = 0, g_srvLParam = 0;

int LaunchServicePlugin(pluginEntry *p)
{
	// plugin load failed - terminating Miranda
	if (!p->bLoaded) {
		if (p->load() != ERROR_SUCCESS) {
			Plugin_Uninit(p);
			return SERVICE_FAILED;
		}
		p->bLoaded = true;
	}

	INT_PTR res = CallService(MS_SERVICEMODE_LAUNCH, g_srvWParam, g_srvLParam);
	if (res != CALLSERVICE_NOTFOUND)
		return res;

	MessageBoxW(nullptr, TranslateT("Unable to load plugin in service mode!"), _A2T(p->pluginname), MB_ICONSTOP);
	Plugin_Uninit(p);
	return SERVICE_FAILED;
}

MIR_APP_DLL(int) SetServiceModePlugin(const char *szPluginName, WPARAM wParam, LPARAM lParam)
{
	CPluginName tmp(szPluginName);

	for (auto &p : servicePlugins) {
		if (!stricmp(tmp, p->pluginname)) {
			plugin_service = p;
			g_srvWParam = wParam;
			g_srvLParam = lParam;
			return 0;
		}
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Event hook to unload all non-core plugins
// hooked very late, after all the internal plugins, blah

void UnloadNewPlugins(void)
{
	// unload everything but the special db/clist plugins
	for (auto &it : pluginList.rev_iter())
		if (!it->bIsLast && !it->bFailed)
			Plugin_Uninit(it);
}

int LoadProtocolPlugins(void)
{
	wchar_t exe[MAX_PATH];
	GetModuleFileName(nullptr, exe, _countof(exe));
	wchar_t* slice = wcsrchr(exe, '\\');
	if (slice)
		*slice = 0;

	/* now loop thru and load all the other plugins, do this in one pass */
	for (int i = 0; i < pluginList.getCount(); i++) {
		pluginEntry *p = pluginList[i];
		if (!p->bIsProtocol || p->m_pPlugin != nullptr)
			continue;

		wchar_t tszFullPath[MAX_PATH];
		mir_snwprintf(tszFullPath, L"%s\\%s\\%S", exe, L"Plugins", p->pluginname);
		p->checkAPI(tszFullPath);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Loads all plugins

int LoadNewPluginsModule(void)
{
	// make full path to the plugin
	wchar_t exe[MAX_PATH];
	GetModuleFileName(nullptr, exe, _countof(exe));
	wchar_t *slice = wcsrchr(exe, '\\');
	if (slice)
		*slice = 0;

	// remember some useful options
	askAboutIgnoredPlugins = (UINT)GetPrivateProfileInt(L"PluginLoader", L"AskAboutIgnoredPlugins", 0, mirandabootini);

	// if Crash Dumper is present, load it to provide Crash Reports
	if (plugin_crshdmp != nullptr && IsPluginOnWhiteList(plugin_crshdmp->pluginname))
		if (!TryLoadPlugin(plugin_crshdmp, false))
			Plugin_Uninit(plugin_crshdmp);

	// first load the clist cos alot of plugins need that to be present at Load(void)
	plugin_clist = getCListModule(exe);

	/* the loop above will try and get one clist DLL to work, if all fail then just bail now */
	if (plugin_clist == nullptr) {
		// result = 0, no clist_* can be found
		if (clistPlugins.getCount())
			MessageBoxW(nullptr, TranslateT("Unable to start any of the installed contact list plugins, I even ignored your preferences for which contact list couldn't load any."), L"Miranda NG", MB_OK | MB_ICONERROR);
		else
			MessageBoxW(nullptr, TranslateT("Can't find a contact list plugin! You need StdClist or any other contact list plugin."), L"Miranda NG", MB_OK | MB_ICONERROR);
		return 1;
	}

	// enable and disable as needed
	for (auto &p : clistPlugins)
		SetPluginOnWhiteList(p->pluginname, plugin_clist == p);

	// now loop thru and load all the other plugins, do this in one pass
	for (int i = 0; i < pluginList.getCount(); i++) {
		pluginEntry *p = pluginList[i];
		if (!TryLoadPlugin(p, false)) {
			Plugin_Uninit(p);
			i--;
		}
	}

	for (auto &it : servicePlugins.rev_iter())
		if (!IsPluginOnWhiteList(it->pluginname))
			Plugin_Uninit(it);

	HookEvent(ME_OPT_INITIALISE, PluginOptionsInit);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// loads all standard plugins.

int LoadStdPlugins()
{
	for (auto &it : pluginDefault) {
		if (it.pImpl)
			continue;

		if (!it.Load()) {
			MessageBoxW(nullptr, 
				CMStringW(FORMAT, LPGENW("Core plugin '%s' cannot be loaded or missing. Miranda will exit now"), it.stdplugname),
				TranslateT("Fatal error"), MB_OK | MB_ICONSTOP);
			return 1;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Plugins module initialization
// called before anything real is loaded, incl. database

static BOOL scanPluginsDir(WIN32_FIND_DATA *fd, wchar_t *path, WPARAM, LPARAM)
{
	OpenPlugin(fd->cFileName, L"Plugins", path);
	return TRUE;
}

int LoadNewPluginsModuleInfos(void)
{
	bModuleInitialized = true;

	hevLoadModule = CreateHookableEvent(ME_SYSTEM_MODULELOAD);
	hevUnloadModule = CreateHookableEvent(ME_SYSTEM_MODULEUNLOAD);

	// remember where the mirandaboot.ini lays
	PathToAbsoluteW(L"mirandaboot.ini", mirandabootini);

	// look for all *.dll's
	enumPlugins(scanPluginsDir, 0, 0);

	MuuidReplacement stdCrypt = { MIID_CRYPTO, L"stdcrypt", nullptr };
	if (!stdCrypt.Load())
		return 1;

	SetServiceModePlugin(_T2A(CmdLine_GetOption(L"svc")));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Plugins module unloading
// called at the end of module chain unloading, just modular engine left at this point

void UnloadDatabase(void)
{
	auto *currDb = g_pCurrDb;
	if (currDb != nullptr) {
		db_setCurrent(nullptr);
		delete currDb;
	}

	UninitIni();
}

void UnloadNewPluginsModule(void)
{
	if (!bModuleInitialized)
		return;

	DestroyHookableEvent(hevLoadModule);
	DestroyHookableEvent(hevUnloadModule);

	// unload everything but the DB
	for (auto &it : pluginList.rev_iter())
		if (!it->bIsDatabase && !it->bIsCrypto && it != plugin_crshdmp)
			Plugin_Uninit(it);

	if (plugin_crshdmp)
		Plugin_Uninit(plugin_crshdmp);

	UnloadDatabase();

	for (auto &it : pluginList.rev_iter())
		Plugin_Uninit(it);
}

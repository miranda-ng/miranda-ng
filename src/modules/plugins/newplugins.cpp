/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"

#include "plugins.h"
#include "..\database\profilemanager.h"
#include "..\langpack\langpack.h"

void LoadExtraIconsModule();

extern bool bModulesLoadedFired;

static int sttComparePluginsByName(const pluginEntry* p1, const pluginEntry* p2)
{
	return lstrcmp(p1->pluginname, p2->pluginname);
}

LIST<pluginEntry>
	pluginList(10, sttComparePluginsByName),
	servicePlugins(5, sttComparePluginsByName),
	clistPlugins(5, sttComparePluginsByName);

/////////////////////////////////////////////////////////////////////////////////////////

#define MAX_MIR_VER ULONG_MAX

static BOOL bModuleInitialized = FALSE;

TCHAR  mirandabootini[MAX_PATH];
static DWORD mirandaVersion;
static int sttFakeID = -100;
static HANDLE hPluginListHeap = NULL;
static int askAboutIgnoredPlugins;

static pluginEntry *pluginList_freeimg, *pluginList_crshdmp, *serviceModePlugin = NULL;

#define PLUGINDISABLELIST "PluginDisable"

/////////////////////////////////////////////////////////////////////////////////////////
// basic functions

int equalUUID(const MUUID& u1, const MUUID& u2)
{
	return memcmp(&u1, &u2, sizeof(MUUID))?0:1;
}

bool hasMuuid(const MUUID* p, const MUUID& uuid)
{
	if (p == NULL)
		return false;

	for (int i=0; !equalUUID(miid_last, p[i]); i++)
		if ( equalUUID(uuid, p[i]))
			return true;

	return false;
}

bool hasMuuid(const BASIC_PLUGIN_INFO& bpi, const MUUID& uuid)
{
	if (bpi.Interfaces)
		return hasMuuid(bpi.Interfaces, uuid);

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// banned plugins

static const MUUID pluginBannedList[] =
{
	{0x9d6c3213, 0x02b4, 0x4fe1, { 0x92, 0xe6, 0x52, 0x6d, 0xe2, 0x4f, 0x8d, 0x65 }},  // old chat
	{0x240a91dc, 0x9464, 0x457a, { 0x97, 0x87, 0xff, 0x1e, 0xa8, 0x8e, 0x77, 0xe3 }},  // old clist
	{0x657fe89b, 0xd121, 0x40c2, { 0x8a, 0xc9, 0xb9, 0xfa, 0x57, 0x55, 0xb3, 0x0c }},  // old srmm
	{0x112f7d30, 0xcd19, 0x4c74, { 0xa0, 0x3b, 0xbf, 0xbb, 0x76, 0xb7, 0x5b, 0xc4 }},  // extraicons
	{0x72765a6f, 0xb017, 0x42f1, { 0xb3, 0x0f, 0x5e, 0x09, 0x41, 0x27, 0x3a, 0x3f }},  // flashavatars
	{0x1394a3ab, 0x2585, 0x4196, { 0x8f, 0x72, 0x0e, 0xae, 0xc2, 0x45, 0x0e, 0x11 }},  // db3x
	{0x28ff9b91, 0x3e4d, 0x4f1c, { 0xb4, 0x7c, 0xc6, 0x41, 0xb0, 0x37, 0xff, 0x40 }},  // dbx_mmap_sa
	{0x28f45248, 0x8c9c, 0x4bee, { 0x93, 0x07, 0x7b, 0xcf, 0x3e, 0x12, 0xbf, 0x99 }},  // dbx_tree
	{0x4c4a27cf, 0x5e64, 0x4242, { 0xa3, 0x32, 0xb9, 0x8b, 0x08, 0x24, 0x3e, 0x89 }},  // metacontacts
	{0x9c448c61, 0xfc3f, 0x42f9, { 0xb9, 0xf0, 0x4a, 0x30, 0xe1, 0xcf, 0x86, 0x71 }}   // skypekit based skype
};

static bool isPluginBanned(const MUUID& u1)
{
	for (int i=0; i < SIZEOF(pluginBannedList); i++)
		if (equalUUID(pluginBannedList[i], u1))
			return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// default plugins

static MuuidReplacement pluginDefault[] =
{
	{	MIID_UIUSERINFO,      _T("stduserinfo"),   NULL },  // 0
	{	MIID_SRURL,           _T("stdurl"),        NULL },  // 1
	{	MIID_SREMAIL,         _T("stdemail"),      NULL },  // 2
	{	MIID_SRAUTH,          _T("stdauth"),       NULL },  // 3
	{	MIID_SRFILE,          _T("stdfile"),       NULL },  // 4
	{	MIID_UIHELP,          _T("stdhelp"),       NULL },  // 5
	{	MIID_UIHISTORY,       _T("stduihist"),     NULL },  // 6
	{	MIID_IDLE,            _T("stdidle"),       NULL },  // 7
	{	MIID_AUTOAWAY,        _T("stdautoaway"),   NULL },  // 8
	{	MIID_USERONLINE,      _T("stduseronline"), NULL },  // 9
	{	MIID_SRAWAY,          _T("stdaway"),       NULL },  // 10
	{	MIID_CLIST,           _T("stdclist"),      NULL },  // 11
	{	MIID_CHAT,            _T("stdchat"),       NULL },  // 12
	{	MIID_SRMM,            _T("stdmsg"),        NULL }   // 13
};

int getDefaultPluginIdx(const MUUID& muuid)
{
	for (int i=0; i < SIZEOF(pluginDefault); i++)
		if (equalUUID(muuid, pluginDefault[i].uuid))
			return i;

	return -1;
}

int LoadStdPlugins()
{
	for (int i = 0; i < SIZEOF(pluginDefault); i++) {
		if (pluginDefault[i].pImpl)
			continue;

		if (!LoadCorePlugin(pluginDefault[i])) {
			TCHAR tszBuf[512];
			mir_sntprintf(tszBuf, SIZEOF(tszBuf), TranslateT("Core plugin '%s' cannot be loaded or missing. Miranda will exit now"), pluginDefault[i].stdplugname);
			MessageBox(NULL, tszBuf, TranslateT("Fatal error"), MB_OK | MB_ICONSTOP);
			return 1;
		}
	}

	if (pluginDefault[13].pImpl == NULL)
		MessageBox(NULL, TranslateT("No messaging plugins loaded. Please install/enable one of the messaging plugins, for instance, \"StdMsg.dll\""), _T("Miranda NG"), MB_OK | MB_ICONWARNING);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// global functions

char* GetPluginNameByInstance(HINSTANCE hInstance)
{
	if (pluginList.getCount() == 0)
		return NULL;

	for (int i=0; i < pluginList.getCount(); i++) {
		pluginEntry *p = pluginList[i];
		if (p->bpi.pluginInfo && p->bpi.hInst == hInstance)
			return p->bpi.pluginInfo->shortName;
	}
	return NULL;
}

int GetPluginLangByInstance(HINSTANCE hInstance)
{
	if (pluginList.getCount() == 0)
		return NULL;

	for (int i=0; i < pluginList.getCount(); i++) {
		pluginEntry *p = pluginList[i];
		if (p->bpi.pluginInfo && p->bpi.hInst == hInstance)
			return p->hLangpack;
	}
	return NULL;
}

int GetPluginFakeId(const MUUID &uuid, int hLangpack)
{
	for (int i=0; i < pluginList.getCount(); i++) {
		pluginEntry *p = pluginList[i];
		if (!p->bpi.hInst)
			continue;

		if ( equalUUID(p->bpi.pluginInfo->uuid, uuid))
			return p->hLangpack = (hLangpack) ? hLangpack : --sttFakeID;
	}

	return 0;
}

MUUID miid_last = MIID_LAST;
MUUID miid_chat = MIID_CHAT;
MUUID miid_srmm = MIID_SRMM;
MUUID miid_clist = MIID_CLIST;
MUUID miid_database = MIID_DATABASE;
MUUID miid_protocol = MIID_PROTOCOL;
MUUID miid_servicemode = MIID_SERVICEMODE;
MUUID miid_crypto = MIID_CRYPTO;

static bool validInterfaceList(MUUID *piface)
{
	if (piface == NULL)
		return true;

	if (equalUUID(miid_last, piface[0]))
		return false;

	return true;
}

static int checkPI(BASIC_PLUGIN_INFO* bpi, PLUGININFOEX* pi)
{
	if (pi == NULL)
		return FALSE;

	if (bpi->InfoEx == NULL || pi->cbSize != sizeof(PLUGININFOEX))
		return FALSE;

	if (!validInterfaceList(bpi->Interfaces) || isPluginBanned(pi->uuid))
		return FALSE;

	if (pi->shortName == NULL || pi->description == NULL || pi->author == NULL ||
		  pi->authorEmail == NULL || pi->copyright == NULL || pi->homepage == NULL)
		return FALSE;

	return TRUE;
}

int checkAPI(TCHAR* plugin, BASIC_PLUGIN_INFO* bpi, DWORD mirandaVersion, int checkTypeAPI)
{
	HINSTANCE h = NULL;

	h = LoadLibrary(plugin);
	if (h == NULL)
		return 0;

	// loaded, check for exports
	bpi->Load = (Miranda_Plugin_Load) GetProcAddress(h, "Load");
	bpi->Unload = (Miranda_Plugin_Unload) GetProcAddress(h, "Unload");
	bpi->InfoEx = (Miranda_Plugin_InfoEx) GetProcAddress(h, "MirandaPluginInfoEx");

	// if they were present
	if (!bpi->Load || !bpi->Unload || !bpi->InfoEx) {
LBL_Error:
		FreeLibrary(h);
		return 0;
	}

	bpi->Interfaces = (MUUID*) GetProcAddress(h, "MirandaInterfaces");
	if (bpi->Interfaces == NULL) {
		typedef MUUID * (__cdecl * Miranda_Plugin_Interfaces) (void);
		Miranda_Plugin_Interfaces pFunc = (Miranda_Plugin_Interfaces)GetProcAddress(h, "MirandaPluginInterfaces");
		if (pFunc)
			bpi->Interfaces = pFunc();
	}

	PLUGININFOEX* pi = bpi->InfoEx(mirandaVersion);
	if (!checkPI(bpi, pi))
		goto LBL_Error;

	bpi->pluginInfo = pi;
	// basic API is present
	if (checkTypeAPI == CHECKAPI_NONE) {
LBL_Ok:
		bpi->hInst = h;
		return 1;
	}

	// check clist ?
	if (checkTypeAPI == CHECKAPI_CLIST) {
		bpi->clistlink = (CList_Initialise) GetProcAddress(h, "CListInitialise");
		if (pi->flags & UNICODE_AWARE)
		if (bpi->clistlink)
			goto LBL_Ok;
	}

	goto LBL_Error;
}

// perform any API related tasks to freeing
void Plugin_Uninit(pluginEntry *p)
{
	// if the basic API check had passed, call Unload if Load(void) was ever called
	if (p->pclass & PCLASS_LOADED) {
		p->bpi.Unload();
		p->pclass &= ~PCLASS_LOADED;
	}

	// release the library
	HINSTANCE hInst = p->bpi.hInst;
	if (hInst != NULL) {
		// we need to kill all resources which belong to that DLL before calling FreeLibrary
		KillModuleEventHooks(hInst);
		KillModuleServices(hInst);

		FreeLibrary(hInst);
		ZeroMemory(&p->bpi, sizeof(p->bpi));
	}
	UnregisterModule(hInst);
	if (p == pluginList_crshdmp)
		pluginList_crshdmp = NULL;
	pluginList.remove(p);
}

int Plugin_UnloadDyn(pluginEntry *p)
{
	if (p->bpi.hInst) {
		if ( CallPluginEventHook(p->bpi.hInst, hOkToExitEvent, 0, 0) != 0)
			return FALSE;

		KillModuleSubclassing(p->bpi.hInst);

		CallPluginEventHook(p->bpi.hInst, hPreShutdownEvent, 0, 0);
		CallPluginEventHook(p->bpi.hInst, hShutdownEvent, 0, 0);

		KillModuleEventHooks(p->bpi.hInst);
		KillModuleServices(p->bpi.hInst);
	}

	int hLangpack = p->hLangpack;
	if (hLangpack != 0) {
		KillModuleMenus(hLangpack);
		KillModuleFonts(hLangpack);
		KillModuleColours(hLangpack);
		KillModuleEffects(hLangpack);
		KillModuleIcons(hLangpack);
		KillModuleHotkeys(hLangpack);
		KillModuleSounds(hLangpack);
		KillModuleExtraIcons(hLangpack);
		KillModuleSrmmIcons(hLangpack);
	}

	NotifyFastHook(hevUnloadModule, (WPARAM)p->bpi.pluginInfo, (LPARAM)p->bpi.hInst);

	Plugin_Uninit(p);

	// mark default plugins to be loaded
	if (!(p->pclass & PCLASS_CORE))
		for (int i=0; i < SIZEOF(pluginDefault); i++)
			if (pluginDefault[i].pImpl == p)
				pluginDefault[i].pImpl = NULL;

	return TRUE;
}

// returns true if the given file is <anything>.dll exactly
static int valid_library_name(TCHAR *name)
{
	TCHAR *dot = _tcsrchr(name, '.');
	if (dot != NULL && lstrcmpi(dot + 1, _T("dll")) == 0)
		if (dot[4] == 0)
			return 1;

	return 0;
}

void enumPlugins(SCAN_PLUGINS_CALLBACK cb, WPARAM wParam, LPARAM lParam)
{
	// get miranda's exe path
	TCHAR exe[MAX_PATH];
	GetModuleFileName(NULL, exe, SIZEOF(exe));
	TCHAR *p = _tcsrchr(exe, '\\'); if (p) *p = 0;

	// create the search filter
	TCHAR search[MAX_PATH];
	mir_sntprintf(search, SIZEOF(search), _T("%s\\Plugins\\*.dll"), exe);
	{
		// FFFN will return filenames for things like dot dll+ or dot dllx
		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile(search, &ffd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && valid_library_name(ffd.cFileName))
					cb(&ffd, exe, wParam, lParam);
			}
				while (FindNextFile(hFind, &ffd));
			FindClose(hFind);
		} //if
	}
}

pluginEntry* OpenPlugin(TCHAR *tszFileName, TCHAR *dir, TCHAR *path)
{
	pluginEntry *p = (pluginEntry*)HeapAlloc(hPluginListHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, sizeof(pluginEntry));
	_tcsncpy(p->pluginname, tszFileName, SIZEOF(p->pluginname));

	// add it to the list anyway
	pluginList.insert(p);

	TCHAR tszFullPath[MAX_PATH];
	mir_sntprintf(tszFullPath, SIZEOF(tszFullPath), _T("%s\\%s\\%s"), path, dir, tszFileName);

	// map dll into the memory and check its exports
	bool bIsPlugin = false;
	mir_ptr<MUUID> pIds( GetPluginInterfaces(tszFullPath, bIsPlugin));
	if (!bIsPlugin) {
		p->pclass |= PCLASS_FAILED;  // piece of shit
		return p;
	}

	// plugin declared that it's a database or a cryptor. load it asap!
	bool bIsDb = hasMuuid(pIds, miid_database);
	if (bIsDb || hasMuuid(pIds, miid_crypto)) {
		BASIC_PLUGIN_INFO bpi;
		if ( checkAPI(tszFullPath, &bpi, mirandaVersion, CHECKAPI_NONE)) {
			// plugin is valid
			p->pclass |= ((bIsDb ? PCLASS_DB : PCLASS_CRYPT) | PCLASS_BASICAPI);
			// copy the dblink stuff
			p->bpi = bpi;

			RegisterModule(p->bpi.hInst);
			if (bpi.Load() != 0)
				p->pclass |= PCLASS_FAILED;
			else
				p->pclass |= PCLASS_LOADED;
		}
		else p->pclass |= PCLASS_FAILED;
	}
	// plugin declared that it's a contact list. mark it for the future load
	else if ( hasMuuid(pIds, miid_clist)) {
		// keep a note of this plugin for later
		clistPlugins.insert(p);
		p->pclass |= PCLASS_CLIST;
	}
	// plugin declared that it's a service mode plugin.
	// load it for a profile manager's window
	else if ( hasMuuid(pIds, miid_servicemode)) {
		BASIC_PLUGIN_INFO bpi;
		if (checkAPI(tszFullPath, &bpi, mirandaVersion, CHECKAPI_NONE)) {
			p->pclass |= (PCLASS_OK | PCLASS_BASICAPI);
			p->bpi = bpi;
			if ( hasMuuid(bpi, miid_servicemode)) {
				p->pclass |= (PCLASS_SERVICE);
				servicePlugins.insert(p);
			}
		}
		else
			// didn't have basic APIs or DB exports - failed.
			p->pclass |= PCLASS_FAILED;
	}
	return p;
}

void SetPluginOnWhiteList(const TCHAR* pluginname, int allow)
{
	char plugName[MAX_PATH];
	strncpy(plugName, StrConvA(pluginname), SIZEOF(plugName));
	strlwr(plugName);
	db_set_b(NULL, PLUGINDISABLELIST, plugName, allow == 0);
}

// returns 1 if the plugin should be enabled within this profile, filename is always lower case
int isPluginOnWhiteList(const TCHAR* pluginname)
{
	char* pluginnameA = _strlwr(mir_t2a(pluginname));
	int rc = db_get_b(NULL, PLUGINDISABLELIST, pluginnameA, 0);
	mir_free(pluginnameA);
	if (rc != 0 && askAboutIgnoredPlugins) {
		TCHAR buf[256];
		mir_sntprintf(buf, SIZEOF(buf), TranslateT("'%s' is disabled, re-enable?"), pluginname);
		if (MessageBox(NULL, buf, TranslateT("Re-enable Miranda plugin?"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			SetPluginOnWhiteList(pluginname, 1);
			rc = 0;
		}
	}

	return rc == 0;
}

bool TryLoadPlugin(pluginEntry *p, bool bDynamic)
{
	TCHAR exe[MAX_PATH];
	GetModuleFileName(NULL, exe, SIZEOF(exe));
	TCHAR* slice = _tcsrchr(exe, '\\');
	if (slice)
		*slice = 0;

	CharLower(p->pluginname);
	if (!(p->pclass & (PCLASS_LOADED | PCLASS_DB | PCLASS_CLIST))) {
		if (!bDynamic && !isPluginOnWhiteList(p->pluginname))
			return false;

		if (!(p->pclass & PCLASS_BASICAPI)) {
			BASIC_PLUGIN_INFO bpi;
			mir_sntprintf(slice, &exe[SIZEOF(exe)] - slice, _T("\\%s\\%s"), (p->pclass & PCLASS_CORE) ? _T("Core") : _T("Plugins"), p->pluginname);
			if (!checkAPI(exe, &bpi, mirandaVersion, CHECKAPI_NONE)) {
				p->pclass |= PCLASS_FAILED;
				return false;
			}

			p->bpi = bpi;
			p->pclass |= PCLASS_OK | PCLASS_BASICAPI;
		}

		if (p->bpi.Interfaces) {
			MUUID *piface = p->bpi.Interfaces;
			for (int i=0; !equalUUID(miid_last, piface[i]); i++) {
				int idx = getDefaultPluginIdx( piface[i] );
				if (idx != -1 && pluginDefault[idx].pImpl) {
					if (!bDynamic) { // this place is already occupied, skip & disable
						SetPluginOnWhiteList(p->pluginname, 0);
						return false;
					}

					// we're loading new implementation dynamically, let the old one die
					if (!(p->pclass & PCLASS_CORE))
						Plugin_UnloadDyn(pluginDefault[idx].pImpl);
				}
			}
		}

		RegisterModule(p->bpi.hInst);
		if (p->bpi.Load() != 0)
			return false;

		p->pclass |= PCLASS_LOADED;
		if (p->bpi.Interfaces) {
			MUUID *piface = p->bpi.Interfaces;
			for (int i=0; !equalUUID(miid_last, piface[i]); i++) {
				int idx = getDefaultPluginIdx(piface[i]);
				if (idx != -1)
					pluginDefault[idx].pImpl = p;
			}
		}
	}
	else if (p->bpi.hInst != NULL) {
		RegisterModule(p->bpi.hInst);
		p->pclass |= PCLASS_LOADED;
	}
	return true;
}

bool LoadCorePlugin(MuuidReplacement& mr)
{
	TCHAR exe[MAX_PATH], tszPlugName[MAX_PATH];
	GetModuleFileName(NULL, exe, SIZEOF(exe));
	TCHAR *p = _tcsrchr(exe, '\\'); if (p) *p = 0;

	mir_sntprintf(tszPlugName, SIZEOF(tszPlugName), _T("%s.dll"), mr.stdplugname);
	pluginEntry* pPlug = OpenPlugin(tszPlugName, _T("Core"), exe);
	if (pPlug->pclass & PCLASS_FAILED) {
LBL_Error:
		Plugin_UnloadDyn(pPlug);
		mr.pImpl = NULL;
		return FALSE;
	}

	pPlug->pclass |= PCLASS_CORE;

	if (!TryLoadPlugin(pPlug, true))
		goto LBL_Error;

	if (bModulesLoadedFired) {
		if (CallPluginEventHook(pPlug->bpi.hInst, hModulesLoadedEvent, 0, 0) != 0)
			goto LBL_Error;

		NotifyEventHooks(hevLoadModule, (WPARAM)pPlug->bpi.pluginInfo, (LPARAM)pPlug->bpi.hInst);
	}
	mr.pImpl = pPlug;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Contact list plugins support

static bool loadClistModule(TCHAR* exe, pluginEntry *p)
{
	BASIC_PLUGIN_INFO bpi;
	if (checkAPI(exe, &bpi, mirandaVersion, CHECKAPI_CLIST)) {
		p->bpi = bpi;
		p->pclass |= PCLASS_LAST | PCLASS_OK | PCLASS_BASICAPI;
		RegisterModule(p->bpi.hInst);
		if (bpi.clistlink() == 0) {
			p->bpi = bpi;
			p->pclass |= PCLASS_LOADED;
			pluginDefault[11].pImpl = p;

			LoadExtraIconsModule();
			return true;
		}
		Plugin_Uninit(p);
	}
	return false;
}

static pluginEntry* getCListModule(TCHAR *exe, TCHAR *slice)
{
	for (int i=0; i < clistPlugins.getCount(); i++) {
		pluginEntry *p = clistPlugins[i];
		mir_sntprintf(slice, &exe[MAX_PATH] - slice, _T("\\Plugins\\%s"), p->pluginname);
		CharLower(p->pluginname);
		if (!isPluginOnWhiteList(p->pluginname))
			continue;

		if ( loadClistModule(exe, p))
			return p;
	}

	MuuidReplacement& stdClist = pluginDefault[11];
	if ( LoadCorePlugin(stdClist)) {
		mir_sntprintf(slice, &exe[MAX_PATH] - slice, _T("\\Core\\%s.dll"), stdClist.stdplugname);
		if ( loadClistModule(exe, stdClist.pImpl))
			return stdClist.pImpl;
	}

	return NULL;
}

int UnloadPlugin(TCHAR* buf, int bufLen)
{
	for (int i = pluginList.getCount()-1; i >= 0; i--) {
		pluginEntry *p = pluginList[i];
		if (!_tcsicmp(p->pluginname, buf)) {
			GetModuleFileName(p->bpi.hInst, buf, bufLen);
			Plugin_Uninit(p);
			return TRUE;
		}
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Service plugins functions

void SetServiceModePlugin(pluginEntry *p)
{
	serviceModePlugin = p;
}

static int LaunchServicePlugin(pluginEntry *p)
{
	// plugin load failed - terminating Miranda
	if (!(p->pclass & PCLASS_LOADED)) {
		if (p->bpi.Load() != ERROR_SUCCESS) {
			Plugin_Uninit(p);
			return SERVICE_FAILED;
		}
		p->pclass |= PCLASS_LOADED;
	}

	INT_PTR res = CallService(MS_SERVICEMODE_LAUNCH, 0, 0);
	if (res != CALLSERVICE_NOTFOUND)
		return res;

	MessageBox(NULL, TranslateT("Unable to load plugin in service mode!"), p->pluginname, MB_ICONSTOP);
	Plugin_Uninit(p);
	return SERVICE_FAILED;
}

int LoadDefaultServiceModePlugin()
{
	LPCTSTR param = CmdLine_GetOption( _T("svc"));
	if (param == NULL || *param == 0)
		return SERVICE_CONTINUE;

	size_t cbLen = _tcslen(param);
	for (int i=0; i < servicePlugins.getCount(); i++) {
		pluginEntry *p = servicePlugins[i];
		if (!_tcsnicmp(p->pluginname, param, cbLen)) {
			int res = LaunchServicePlugin(p);
			if (res == SERVICE_ONLYDB) // load it later
				serviceModePlugin = p;
			return res;
		}
	}

	return SERVICE_CONTINUE;
}

int LoadServiceModePlugin()
{
	return (serviceModePlugin == NULL) ? SERVICE_CONTINUE : LaunchServicePlugin(serviceModePlugin);
}

void EnsureCheckerLoaded(bool bEnable)
{
	for (int i = 0; i < pluginList.getCount(); i++) {
		pluginEntry *p = pluginList[i];
		if (_tcsicmp(p->pluginname, _T("dbchecker.dll")))
			continue;

		if (bEnable) {
			if (!(p->pclass & PCLASS_LOADED)) {
				if (p->bpi.Load() != ERROR_SUCCESS)
					Plugin_Uninit(p);
				else {
					p->pclass |= PCLASS_LOADED;
					servicePlugins.remove(p);
				}
			}
		}
		else Plugin_Uninit(p);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Event hook to unload all non-core plugins
// hooked very late, after all the internal plugins, blah

void UnloadNewPlugins(void)
{
	// unload everything but the special db/clist plugins
	for (int i = pluginList.getCount() - 1; i >= 0; i--) {
		pluginEntry *p = pluginList[i];
		if (!(p->pclass & PCLASS_LAST) && (p->pclass & PCLASS_OK))
			Plugin_Uninit(p);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Loads all plugins

int LoadNewPluginsModule(void)
{
	int i;

	// make full path to the plugin
	TCHAR exe[MAX_PATH];
	GetModuleFileName(NULL, exe, SIZEOF(exe));
	TCHAR *slice = _tcsrchr(exe, '\\');
	if (slice) *slice = 0;

	// remember some useful options
	askAboutIgnoredPlugins = (UINT)GetPrivateProfileInt(_T("PluginLoader"), _T("AskAboutIgnoredPlugins"), 0, mirandabootini);

	// if Crash Dumper is present, load it to provide Crash Reports
	if (pluginList_crshdmp != NULL && isPluginOnWhiteList(pluginList_crshdmp->pluginname))
		if (!TryLoadPlugin(pluginList_crshdmp, false))
			Plugin_Uninit(pluginList_crshdmp);

	// if freeimage is present, load it to provide the basic core functions
	if (pluginList_freeimg != NULL) {
		BASIC_PLUGIN_INFO bpi;
		mir_sntprintf(slice, &exe[SIZEOF(exe)] - slice, _T("\\Plugins\\%s"), pluginList_freeimg->pluginname);
		if (checkAPI(exe, &bpi, mirandaVersion, CHECKAPI_NONE)) {
			pluginList_freeimg->bpi = bpi;
			pluginList_freeimg->pclass |= PCLASS_OK | PCLASS_BASICAPI;
			if (bpi.Load() == 0)
				pluginList_freeimg->pclass |= PCLASS_LOADED;
			else
				Plugin_Uninit(pluginList_freeimg);
		}
	}

	// first load the clist cos alot of plugins need that to be present at Load(void)
	pluginEntry* clist = getCListModule(exe, slice);

	/* the loop above will try and get one clist DLL to work, if all fail then just bail now */
	if (clist == NULL) {
		// result = 0, no clist_* can be found
		if (clistPlugins.getCount())
			MessageBox(NULL, TranslateT("Unable to start any of the installed contact list plugins, I even ignored your preferences for which contact list couldn't load any."), _T("Miranda NG"), MB_OK | MB_ICONERROR);
		else
			MessageBox(NULL, TranslateT("Can't find a contact list plugin! You need StdClist or any other contact list plugin.") , _T("Miranda NG"), MB_OK | MB_ICONERROR);
		return 1;
	}

	/* enable and disable as needed  */
	for (i = 0; i < clistPlugins.getCount(); i++)
		SetPluginOnWhiteList(clistPlugins[i]->pluginname, clist != clistPlugins[i] ? 0 : 1);

	/* now loop thru and load all the other plugins, do this in one pass */
	for (i = 0; i < pluginList.getCount(); i++) {
		pluginEntry *p = pluginList[i];
		if (!TryLoadPlugin(p, false)) {
			Plugin_Uninit(p);
			i--;
		}
	}

	HookEvent(ME_OPT_INITIALISE, PluginOptionsInit);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Plugins module initialization
// called before anything real is loaded, incl. database

static BOOL scanPluginsDir(WIN32_FIND_DATA *fd, TCHAR *path, WPARAM, LPARAM)
{
	pluginEntry *p = OpenPlugin(fd->cFileName, _T("Plugins"), path);
	if (!(p->pclass & PCLASS_FAILED)) {
		if (pluginList_freeimg == NULL && lstrcmpi(fd->cFileName, _T("advaimg.dll")) == 0)
			pluginList_freeimg = p;

		if (pluginList_crshdmp == NULL && lstrcmpi(fd->cFileName, _T("crashdumper.dll")) == 0) {
			pluginList_crshdmp = p;
			p->pclass |= PCLASS_LAST;
		}
	}

	return TRUE;
}

int LoadNewPluginsModuleInfos(void)
{
	bModuleInitialized = TRUE;

	LoadPluginOptions();

	hPluginListHeap = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
	mirandaVersion = (DWORD)CallService(MS_SYSTEM_GETVERSION, 0, 0);

	// remember where the mirandaboot.ini goes
	PathToAbsoluteT(_T("mirandaboot.ini"), mirandabootini);

	// look for all *.dll's
	enumPlugins(scanPluginsDir, 0, 0);

	MuuidReplacement stdCrypt = { MIID_CRYPTO, _T("stdcrypt"), NULL };
	return !LoadCorePlugin(stdCrypt);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Plugins module unloading
// called at the end of module chain unloading, just modular engine left at this point

void UnloadDatabase(void)
{
	if (currDb != NULL) {
		db_setCurrent(NULL);
		currDblink->Unload(currDb);
		currDb = NULL;
		currDblink = NULL;
	}

	UninitIni();
}

void UnloadNewPluginsModule(void)
{
	if (!bModuleInitialized)
		return;

	UnloadPluginOptions();

	// unload everything but the DB
	for (int i = pluginList.getCount()-1; i >= 0; i--) {
		pluginEntry *p = pluginList[i];
		if (!(p->pclass & (PCLASS_DB | PCLASS_CRYPT)) && p != pluginList_crshdmp)
			Plugin_Uninit(p);
	}

	if (pluginList_crshdmp)
		Plugin_Uninit(pluginList_crshdmp);

	UnloadDatabase();

	for (int k = pluginList.getCount()-1; k >= 0; k--) {
		pluginEntry *p = pluginList[k];
		Plugin_Uninit(p);
	}

	if (hPluginListHeap) HeapDestroy(hPluginListHeap);
	hPluginListHeap = 0;
}

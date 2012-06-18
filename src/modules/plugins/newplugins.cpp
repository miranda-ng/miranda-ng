/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2010 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
#include "..\..\core\commonheaders.h"
#include "plugins.h"

static int sttComparePlugins(const pluginEntry* p1, const pluginEntry* p2)
{	
	if (p1->bpi.hInst == p2->bpi.hInst)
		return 0;

	return (p1->bpi.hInst < p2->bpi.hInst) ? -1 : 1;
}

static int sttComparePluginsByName(const pluginEntry* p1, const pluginEntry* p2)
{	return lstrcmp(p1->pluginname, p2->pluginname);
}

LIST<pluginEntry> pluginList(10, sttComparePluginsByName), pluginListAddr(10, sttComparePlugins);

/////////////////////////////////////////////////////////////////////////////////////////

#define MAX_MIR_VER ULONG_MAX

struct PluginUUIDList {
    MUUID uuid;
    DWORD maxVersion;
} 
static const pluginBannedList[] = 
{
	{{0x7f65393b, 0x7771, 0x4f3f, { 0xa9, 0xeb, 0x5d, 0xba, 0xf2, 0xb3, 0x61, 0xf1 }}, MAX_MIR_VER}, // png2dib
	{{0xe00f1643, 0x263c, 0x4599, { 0xb8, 0x4b, 0x05, 0x3e, 0x5c, 0x51, 0x1d, 0x28 }}, MAX_MIR_VER}, // loadavatars (unicode)
	{{0xc9e01eb0, 0xa119, 0x42d2, { 0xb3, 0x40, 0xe8, 0x67, 0x8f, 0x5f, 0xea, 0xd9 }}, MAX_MIR_VER}, // loadavatars (ansi)
};
const int pluginBannedListCount = SIZEOF(pluginBannedList);

static BOOL bModuleInitialized = FALSE;

PLUGINLINK pluginCoreLink;
TCHAR   mirandabootini[MAX_PATH];
static DWORD mirandaVersion;
static int serviceModeIdx = -1;
static pluginEntry * pluginListSM;
static pluginEntry * pluginListDb;
static pluginEntry * pluginListUI;
static pluginEntry * pluginList_freeimg;
static pluginEntry * pluginList_crshdmp;
static HANDLE hPluginListHeap = NULL;
static pluginEntry * pluginDefModList[DEFMOD_HIGHEST+1]; // do not free this memory
static int askAboutIgnoredPlugins;

int  InitIni(void);
void UninitIni(void);

#define PLUGINDISABLELIST "PluginDisable"

int CallHookSubscribers(HANDLE hEvent, WPARAM wParam, LPARAM lParam);

int LoadDatabaseModule(void);

char* GetPluginNameByInstance(HINSTANCE hInstance)
{
	int i = 0;
	if (pluginList.getCount() == 0) return NULL;
	for (i = 0; i <  pluginList.getCount(); i++) {
		pluginEntry* pe = pluginList[i];
		if (pe->bpi.pluginInfo && pe->bpi.hInst == hInstance)
			return pe->bpi.pluginInfo->shortName;
	}
	return NULL;
}

HINSTANCE GetInstByAddress(void* codePtr)
{
	int idx;
	HINSTANCE result;
	pluginEntry p; p.bpi.hInst = (HINSTANCE)codePtr;

	if (pluginListAddr.getCount() == 0)
		return NULL;

	List_GetIndex((SortedList*)&pluginListAddr, &p, &idx);
	if (idx > 0)
		idx--;

	result = pluginListAddr[idx]->bpi.hInst;

	if (result < hMirandaInst && codePtr > hMirandaInst)
		result = hMirandaInst;
	else if (idx == 0 && codePtr < (void*)result)
		result = NULL;

	return result;
}

int equalUUID(const MUUID& u1, const MUUID& u2)
{
	return memcmp(&u1, &u2, sizeof(MUUID))?0:1;
}

MUUID miid_last = MIID_LAST;
MUUID miid_servicemode = MIID_SERVICEMODE;

static bool validInterfaceList(Miranda_Plugin_Interfaces ifaceProc)
{
	// we don't need'em anymore in the common case
	if (ifaceProc == NULL)
		return true;

	MUUID *piface = ifaceProc();
	if (piface == NULL)
		return false;

	if (equalUUID(miid_last, piface[0]))
		return false;

	return true;
}

static int isPluginBanned(MUUID u1, DWORD dwVersion)
{
	int i;

	for (i=0; i<pluginBannedListCount; i++) {
		if (equalUUID(pluginBannedList[i].uuid, u1)) {
			if (dwVersion<pluginBannedList[i].maxVersion)
				return 1;
			return 0;
		}
	}
	return 0;
}

// returns true if the API exports were good, otherwise, passed in data is returned
#define CHECKAPI_NONE 	 0
#define CHECKAPI_DB 	 1
#define CHECKAPI_CLIST   2

/*
 * historyeditor added by nightwish - plugin is problematic and can ruin database as it does not understand UTF-8 message
 * storage
 */
     
static const TCHAR* modulesToSkip[] = 
{ 
	_T("autoloadavatars.dll"), _T("multiwindow.dll"), _T("fontservice.dll"), 
	_T("icolib.dll"), _T("historyeditor.dll") 
};

// The following plugins will be checked for a valid MUUID or they will not be loaded
static const TCHAR* expiredModulesToSkip[] = 
{ 
	_T("scriver.dll"), _T("nconvers.dll"), _T("tabsrmm.dll"), _T("nhistory.dll"), 
	_T("historypp.dll"), _T("help.dll"), _T("loadavatars.dll"), _T("tabsrmm_unicode.dll"), 
	_T("clist_nicer_plus.dll"), _T("changeinfo.dll"), _T("png2dib.dll"), _T("dbx_mmap.dll"), 
	_T("dbx_3x.dll"), _T("sramm.dll"), _T("srmm_mod.dll"), _T("srmm_mod (no Unicode).dll"), 
	_T("singlemodeSRMM.dll"), _T("msg_export.dll"), _T("clist_modern.dll"), 
	_T("clist_nicer.dll") 
};

static int checkPI(BASIC_PLUGIN_INFO* bpi, PLUGININFOEX* pi)
{
	int bHasValidInfo = FALSE;

	if (pi == NULL)
		return FALSE;

	if (bpi->InfoEx) {
		if (pi->cbSize == sizeof(PLUGININFOEX))
			if ( !validInterfaceList(bpi->Interfaces) || isPluginBanned(pi->uuid, pi->version))
				return FALSE;

		bHasValidInfo = TRUE;
	}
	
	if ( !bHasValidInfo)
		return FALSE;

	if (pi->shortName == NULL || pi->description == NULL || pi->author == NULL  || 
		  pi->authorEmail == NULL || pi->copyright == NULL || pi->homepage == NULL)
		return FALSE;

	if (pi->replacesDefaultModule > DEFMOD_HIGHEST  || 
		  pi->replacesDefaultModule == DEFMOD_REMOVED_UIPLUGINOPTS  || 
		  pi->replacesDefaultModule == DEFMOD_REMOVED_PROTOCOLNETLIB)
		return FALSE;

	return TRUE;
}

int checkAPI(TCHAR* plugin, BASIC_PLUGIN_INFO* bpi, DWORD mirandaVersion, int checkTypeAPI)
{
	HINSTANCE h = NULL;

	// this is evil but these plugins are buggy/old and people are blaming Miranda
	// fontservice plugin is built into the core now
	TCHAR* p = _tcsrchr(plugin, '\\');
	if (p != NULL && ++p) {
		int i;
		for (i = 0; i < SIZEOF(modulesToSkip); i++)
			if (lstrcmpi(p, modulesToSkip[i]) == 0)
				return 0;
	}

	h = LoadLibrary(plugin);
	if (h == NULL)
		return 0;

	// loaded, check for exports
	bpi->Load = (Miranda_Plugin_Load) GetProcAddress(h, "Load");
	bpi->Unload = (Miranda_Plugin_Unload) GetProcAddress(h, "Unload");
	bpi->InfoEx = (Miranda_Plugin_InfoEx) GetProcAddress(h, "MirandaPluginInfoEx");
	bpi->Interfaces = (Miranda_Plugin_Interfaces) GetProcAddress(h, "MirandaPluginInterfaces");

	// if they were present
	if ( !bpi->Load || !bpi->Unload || !bpi->InfoEx) {
LBL_Error:
		FreeLibrary(h);
		return 0;
	}

	PLUGININFOEX* pi = bpi->InfoEx(mirandaVersion);
	if ( !checkPI(bpi, pi))
		goto LBL_Error;

	bpi->pluginInfo = pi;
	// basic API is present
	if (checkTypeAPI == CHECKAPI_NONE) {
LBL_Ok:
		bpi->hInst = h;
		return 1;
	}

	// check for DB?
	if (checkTypeAPI == CHECKAPI_DB) {
		bpi->DbInfo = (Database_Plugin_Info) GetProcAddress(h, "DatabasePluginInfo");
		if (bpi->DbInfo) {
			// fetch internal database function pointers
			bpi->dblink = bpi->DbInfo(NULL);
			// validate returned link structure
			if (bpi->dblink && bpi->dblink->cbSize == sizeof(DATABASELINK))
				goto LBL_Ok;
	}	}

	// check clist ?
	if (checkTypeAPI == CHECKAPI_CLIST) {
		bpi->clistlink = (CList_Initialise) GetProcAddress(h, "CListInitialise");
		if (pi->flags & UNICODE_AWARE)
		if (bpi->clistlink)
			goto LBL_Ok;
	}

	goto LBL_Error;
}

// returns true if the given file is <anything>.dll exactly
static int valid_library_name(TCHAR *name)
{
	TCHAR * dot = _tcsrchr(name, '.');
	if (dot != NULL && lstrcmpi(dot + 1, _T("dll")) == 0)
		if (dot[4] == 0)
			return 1;

	return 0;
}

// returns true if the given file matches dbx_*.dll, which is used to LoadLibrary()
static int validguess_db_name(TCHAR * name)
{
	int rc = 0;
	// this is ONLY SAFE because name -> ffd.cFileName == MAX_PATH
	TCHAR x = name[4];
	name[4]=0;
	rc = lstrcmpi(name, _T("dbx_")) == 0 || lstrcmpi(name, _T("dbrw")) == 0;
	name[4] = x;
	return rc;
}

// returns true if the given file matches clist_*.dll
static int validguess_clist_name(TCHAR * name)
{
	int rc=0;
	// argh evil
	TCHAR x = name[6];
	name[6] = 0;
	rc = lstrcmpi(name, _T("clist_")) == 0;
	name[6] = x;
	return rc;
}

// returns true if the given file matches svc_*.dll
static int validguess_servicemode_name(TCHAR * name)
{
	int rc = 0;
	// argh evil
	TCHAR x = name[4];
	name[4]=0;
	rc = lstrcmpi(name, _T("svc_")) == 0;
	name[4] = x;
	return rc;
}

// perform any API related tasks to freeing
void Plugin_Uninit(pluginEntry* p, bool bDynamic)
{
	if (bDynamic && p->bpi.hInst)
		CallPluginEventHook(p->bpi.hInst, hOkToExitEvent, 0, 0);

	// if it was an installed database plugin, call its unload
	if (p->pclass & PCLASS_DB)
		p->bpi.dblink->Unload(p->pclass & PCLASS_OK);

	// if the basic API check had passed, call Unload if Load() was ever called
	if (p->pclass & PCLASS_LOADED)
		p->bpi.Unload();

	// release the library
	if (p->bpi.hInst != NULL) {
		// we need to kill all resources which belong to that DLL before calling FreeLibrary
		KillModuleEventHooks(p->bpi.hInst);
		KillModuleServices(p->bpi.hInst);

		FreeLibrary(p->bpi.hInst);
		ZeroMemory(&p->bpi, sizeof(p->bpi));
	}
	pluginList.remove(p);
	pluginListAddr.remove(p);
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
				if ( !(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && valid_library_name(ffd.cFileName))
					cb(&ffd, exe, wParam, lParam);
			}
				while (FindNextFile(hFind, &ffd));
			FindClose(hFind);
		} //if
	}
}

// this is called by the db module to return all DBs plugins, then when it finds the one it likes the others are unloaded
static INT_PTR PluginsEnum(WPARAM, LPARAM lParam)
{
	PLUGIN_DB_ENUM * de = (PLUGIN_DB_ENUM *) lParam;
	pluginEntry * x = pluginListDb;
	if (de == NULL || de->cbSize != sizeof(PLUGIN_DB_ENUM) || de->pfnEnumCallback == NULL) return 1;
	while (x != NULL) {
		int rc = de->pfnEnumCallback(StrConvA(x->pluginname), x->bpi.dblink, de->lParam);
		if (rc == DBPE_DONE) {
			// this db has been picked, get rid of all the others
			pluginEntry * y = pluginListDb, * n;
			while (y != NULL) {
				n = y->nextclass;
				if (x != y)
					Plugin_Uninit(y);
				y = n;
			} // while
			x->pclass |= PCLASS_LOADED | PCLASS_OK | PCLASS_LAST;
			return 0;
		}
		else if (rc == DBPE_HALT) return 1;
		x = x->nextclass;
	} // while
	return pluginListDb != NULL ? 1 : -1;
}

static INT_PTR PluginsGetDefaultArray(WPARAM, LPARAM)
{
	return (INT_PTR)&pluginDefModList;
}

pluginEntry* OpenPlugin(TCHAR* tszFileName, TCHAR* path)
{
	int isdb = validguess_db_name(tszFileName);
	BASIC_PLUGIN_INFO bpi;
	pluginEntry* p = (pluginEntry*)HeapAlloc(hPluginListHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, sizeof(pluginEntry));
	_tcsncpy(p->pluginname, tszFileName, SIZEOF(p->pluginname));
	// plugin name suggests its a db module, load it right now
	if (isdb) {
		TCHAR buf[MAX_PATH];
		mir_sntprintf(buf, SIZEOF(buf), _T("%s\\Plugins\\%s"), path, tszFileName);
		if (checkAPI(buf, &bpi, mirandaVersion, CHECKAPI_DB)) {
			// db plugin is valid
			p->pclass |= (PCLASS_DB | PCLASS_BASICAPI);
			// copy the dblink stuff
			p->bpi = bpi;
			// keep a faster list.
			if (pluginListDb != NULL) p->nextclass = pluginListDb;
			pluginListDb = p;
		}
		else
			// didn't have basic APIs or DB exports - failed.
			p->pclass |= PCLASS_FAILED;
	}
	else if (validguess_clist_name(tszFileName)) {
		// keep a note of this plugin for later
		if (pluginListUI != NULL) p->nextclass=pluginListUI;
		pluginListUI=p;
		p->pclass |= PCLASS_CLIST;
	}
	else if (validguess_servicemode_name(tszFileName)) {
		TCHAR buf[MAX_PATH];
		mir_sntprintf(buf, SIZEOF(buf), _T("%s\\Plugins\\%s"), path, tszFileName);
		if (checkAPI(buf, &bpi, mirandaVersion, CHECKAPI_NONE)) {
			p->pclass |= (PCLASS_OK | PCLASS_BASICAPI);
			p->bpi = bpi;
			if (bpi.Interfaces) {
				int i = 0;
				MUUID *piface = bpi.Interfaces();
				while ( !equalUUID(miid_last, piface[i])) {
					if ( !equalUUID(miid_servicemode, piface[i++]))
						continue;
					p->pclass |= (PCLASS_SERVICE);
					if (pluginListSM != NULL) p->nextclass = pluginListSM;
					pluginListSM = p;
					break;
				}
			}
		}
		else
			// didn't have basic APIs or DB exports - failed.
			p->pclass |= PCLASS_FAILED;
	}

	// add it to the list
	pluginList.insert(p);
	return p;
}

// called in the first pass to create pluginEntry* structures and validate database plugins
static BOOL scanPluginsDir(WIN32_FIND_DATA *fd, TCHAR *path, WPARAM, LPARAM)
{
	pluginEntry* p = OpenPlugin(fd->cFileName, path);
	if ( !(p->pclass & PCLASS_FAILED)) {
		if (pluginList_freeimg == NULL && lstrcmpi(fd->cFileName, _T("advaimg.dll")) == 0)
			pluginList_freeimg = p;

		if (pluginList_crshdmp == NULL &&  lstrcmpi(fd->cFileName, _T("svc_crshdmp.dll")) == 0) {
			pluginList_crshdmp = p;
			p->pclass |= PCLASS_LAST;
		}
	}

	return TRUE;
}

void SetPluginOnWhiteList(const TCHAR* pluginname, int allow)
{
	DBWriteContactSettingByte(NULL, PLUGINDISABLELIST, StrConvA(pluginname), allow == 0);
}

// returns 1 if the plugin should be enabled within this profile, filename is always lower case
int isPluginOnWhiteList(const TCHAR* pluginname)
{
	char* pluginnameA = _strlwr(mir_t2a(pluginname));
	int rc = DBGetContactSettingByte(NULL, PLUGINDISABLELIST, pluginnameA, 0);
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
	if (slice) *slice = 0;

	CharLower(p->pluginname);
	if ( !(p->pclass & (PCLASS_LOADED | PCLASS_DB | PCLASS_CLIST))) {
		if ( !bDynamic && !isPluginOnWhiteList(p->pluginname))
			return false;

		BASIC_PLUGIN_INFO bpi;
		mir_sntprintf(slice, &exe[SIZEOF(exe)] - slice, _T("\\Plugins\\%s"), p->pluginname);
		if ( !checkAPI(exe, &bpi, mirandaVersion, CHECKAPI_NONE))
			p->pclass |= PCLASS_FAILED;
		else {
			int rm = bpi.pluginInfo->replacesDefaultModule;
			p->bpi = bpi;
			p->pclass |= PCLASS_OK | PCLASS_BASICAPI;

			if (pluginDefModList[rm] != NULL) {
				SetPluginOnWhiteList(p->pluginname, 0);
				return false;
			}

			pluginListAddr.insert(p);
			if (bpi.Load(&pluginCoreLink) != 0)
				return false;
						
			p->pclass |= PCLASS_LOADED;
			if (rm) pluginDefModList[rm]=p;
		}
	}
	else if (p->bpi.hInst != NULL) {
		pluginListAddr.insert(p);
		p->pclass |= PCLASS_LOADED;
	}
	return true;
}

static pluginEntry* getCListModule(TCHAR * exe, TCHAR * slice, int useWhiteList)
{
	pluginEntry * p = pluginListUI;
	BASIC_PLUGIN_INFO bpi;
	while (p != NULL) {
		mir_sntprintf(slice, &exe[MAX_PATH] - slice, _T("\\Plugins\\%s"), p->pluginname);
		CharLower(p->pluginname);
		if (useWhiteList ? isPluginOnWhiteList(p->pluginname) : 1) {
			if (checkAPI(exe, &bpi, mirandaVersion, CHECKAPI_CLIST)) {
				p->bpi = bpi;
				p->pclass |= PCLASS_LAST | PCLASS_OK | PCLASS_BASICAPI;
				pluginListAddr.insert(p);
				if (bpi.clistlink(&pluginCoreLink) == 0) {
					p->bpi = bpi;
					p->pclass |= PCLASS_LOADED;
					return p;
				}
				else Plugin_Uninit(p);
			} //if
		} //if
		p = p->nextclass;
	}
	return NULL;
}

int UnloadPlugin(TCHAR* buf, int bufLen)
{
	int i;
	for (i = pluginList.getCount()-1; i >= 0; i--) {
		pluginEntry* p = pluginList[i];
		if ( !_tcsicmp(p->pluginname, buf)) {
			GetModuleFileName(p->bpi.hInst, buf, bufLen);
			Plugin_Uninit(p);
			return TRUE;
		}	
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//   Service plugins functions

char **GetServiceModePluginsList(void)
{
	int i = 0;
	char **list = NULL;
	pluginEntry * p = pluginListSM;
	while (p != NULL) {
		i++;
		p = p->nextclass;
	}
	if (i) {
		list = (char**)mir_calloc((i + 1) * sizeof(char*));
		p = pluginListSM;
		i = 0;
		while (p != NULL) {
			list[i++] = p->bpi.pluginInfo->shortName;
			p = p->nextclass;
		}
	}
	return list;
}

void SetServiceModePlugin(int idx)
{
	serviceModeIdx = idx;
}

int LoadServiceModePlugin(void)
{
	int i = 0;
	pluginEntry* p = pluginListSM;

	if (serviceModeIdx < 0)
		return 0;

	while (p != NULL) {
		if (serviceModeIdx == i++) {
			if (p->bpi.Load(&pluginCoreLink) == 0) {
				p->pclass |= PCLASS_LOADED;
				if (CallService(MS_SERVICEMODE_LAUNCH, 0, 0) != CALLSERVICE_NOTFOUND)
					return 1;
				
				MessageBox(NULL, TranslateT("Unable to load plugin in Service Mode!"), p->pluginname, 0);
				return -1;
			}
			Plugin_Uninit(p);
			return -1;
		}
		p = p->nextclass;
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
//   Event hook to unload all non-core plugins
//   hooked very late, after all the internal plugins, blah

void UnloadNewPlugins(void)
{
	// unload everything but the special db/clist plugins
	for (int i = pluginList.getCount()-1; i >= 0; i--) {
		pluginEntry* p = pluginList[i];
		if ( !(p->pclass & PCLASS_LAST) && (p->pclass & PCLASS_OK))
			Plugin_Uninit(p);
}	}

/////////////////////////////////////////////////////////////////////////////////////////
//
//   Loads all plugins

int LoadNewPluginsModule(void)
{
	TCHAR exe[MAX_PATH];
	TCHAR* slice;
	pluginEntry* p;
	pluginEntry* clist = NULL;
	int useWhiteList, i;
	bool msgModule = false;

	// make full path to the plugin
	GetModuleFileName(NULL, exe, SIZEOF(exe));
	slice = _tcsrchr(exe, '\\');
	if (slice) *slice = 0;

	// remember some useful options
	askAboutIgnoredPlugins=(UINT) GetPrivateProfileInt(_T("PluginLoader"), _T("AskAboutIgnoredPlugins"), 0, mirandabootini);

	// if Crash Dumper is present, load it to provide Crash Reports
	if (pluginList_crshdmp != NULL && isPluginOnWhiteList(pluginList_crshdmp->pluginname)) 
    {
		if (pluginList_crshdmp->bpi.Load(&pluginCoreLink) == 0)
			pluginList_crshdmp->pclass |= PCLASS_LOADED | PCLASS_LAST;
		else
			Plugin_Uninit(pluginList_crshdmp);
    }

	// if freeimage is present, load it to provide the basic core functions
	if (pluginList_freeimg != NULL) {
		BASIC_PLUGIN_INFO bpi;
		mir_sntprintf(slice, &exe[SIZEOF(exe)] - slice, _T("\\Plugins\\%s"), pluginList_freeimg->pluginname);
		if (checkAPI(exe, &bpi, mirandaVersion, CHECKAPI_NONE)) {
			pluginList_freeimg->bpi = bpi;
			pluginList_freeimg->pclass |= PCLASS_OK | PCLASS_BASICAPI;
			if (bpi.Load(&pluginCoreLink) == 0)
				pluginList_freeimg->pclass |= PCLASS_LOADED;
			else
				Plugin_Uninit(pluginList_freeimg);
	}	}

	// first load the clist cos alot of plugins need that to be present at Load()
	for (useWhiteList = 1; useWhiteList >= 0 && clist == NULL; useWhiteList--)
		clist=getCListModule(exe, slice, useWhiteList);
	/* the loop above will try and get one clist DLL to work, if all fail then just bail now */
	if (clist == NULL) {
		// result = 0, no clist_* can be found
		if (pluginListUI)
			MessageBox(NULL, TranslateT("Unable to start any of the installed contact list plugins, I even ignored your preferences for which contact list couldn't load any."), _T("Miranda IM"), MB_OK | MB_ICONINFORMATION);
		else
			MessageBox(NULL, TranslateT("Can't find a contact list plugin! you need clist_classic or any other clist plugin.") , _T("Miranda IM"), MB_OK | MB_ICONINFORMATION);
		return 1;
	}

	/* enable and disable as needed  */
	p = pluginListUI;
	while (p != NULL) {
		SetPluginOnWhiteList(p->pluginname, clist != p ? 0 : 1);
		p = p->nextclass;
	}
	/* now loop thru and load all the other plugins, do this in one pass */

	for (i=0; i < pluginList.getCount(); i++) {
		p = pluginList[i];
		if ( !TryLoadPlugin(p, false)) {
			Plugin_Uninit(p);
			i--;
		}
		else if (p->pclass & PCLASS_LOADED)
			msgModule |= (p->bpi.pluginInfo->replacesDefaultModule == DEFMOD_SRMESSAGE);
	}

	if ( !msgModule)
		MessageBox(NULL, TranslateT("No messaging plugins loaded. Please install/enable one of the messaging plugins, for instance, \"srmm.dll\""), _T("Miranda IM"), MB_OK | MB_ICONINFORMATION);

	HookEvent(ME_OPT_INITIALISE, PluginOptionsInit);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//   Plugins module initialization
//   called before anything real is loaded, incl. database

int LoadNewPluginsModuleInfos(void)
{
	bModuleInitialized = TRUE;

	LoadPluginOptions();

	hPluginListHeap = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
	mirandaVersion = (DWORD)CallService(MS_SYSTEM_GETVERSION, 0, 0);

	CreateServiceFunction(MS_PLUGINS_ENUMDBPLUGINS, PluginsEnum);
	CreateServiceFunction(MS_PLUGINS_GETDISABLEDEFAULTARRAY, PluginsGetDefaultArray);

	// make sure plugins can get internal core APIs
	pluginCoreLink.CallService                    = CallService;
	pluginCoreLink.ServiceExists                  = ServiceExists;
	pluginCoreLink.CreateServiceFunction          = CreateServiceFunction;
	pluginCoreLink.CreateServiceFunctionParam     = CreateServiceFunctionParam;
	pluginCoreLink.CreateServiceFunctionObj       = CreateServiceFunctionObj;
	pluginCoreLink.CreateServiceFunctionObjParam  = CreateServiceFunctionObjParam;
	pluginCoreLink.CreateTransientServiceFunction = CreateServiceFunction;
	pluginCoreLink.DestroyServiceFunction         = DestroyServiceFunction;
	pluginCoreLink.CreateHookableEvent            = CreateHookableEvent;
	pluginCoreLink.DestroyHookableEvent           = DestroyHookableEvent;
	pluginCoreLink.HookEvent                      = HookEvent;
	pluginCoreLink.HookEventParam                 = HookEventParam;
	pluginCoreLink.HookEventObj                   = HookEventObj;
	pluginCoreLink.HookEventObjParam              = HookEventObjParam;
	pluginCoreLink.HookEventMessage               = HookEventMessage;
	pluginCoreLink.UnhookEvent                    = UnhookEvent;
	pluginCoreLink.NotifyEventHooks               = NotifyEventHooks;
	pluginCoreLink.SetHookDefaultForHookableEvent = SetHookDefaultForHookableEvent;
	pluginCoreLink.CallServiceSync                = CallServiceSync;
	pluginCoreLink.CallFunctionAsync              = CallFunctionAsync;
	pluginCoreLink.NotifyEventHooksDirect         = CallHookSubscribers;
	pluginCoreLink.CallProtoService               = CallProtoService;
	pluginCoreLink.CallContactService             = CallContactService;
	pluginCoreLink.KillObjectServices             = KillObjectServices;
	pluginCoreLink.KillObjectEventHooks           = KillObjectEventHooks;

	// remember where the mirandaboot.ini goes
	pathToAbsoluteT(_T("mirandaboot.ini"), mirandabootini, NULL);
	// look for all *.dll's
	enumPlugins(scanPluginsDir, 0, 0);
	// the database will select which db plugin to use, or fail if no profile is selected
	if (LoadDatabaseModule()) return 1;
	InitIni();
	//  could validate the plugin entries here but internal modules arent loaded so can't call Load() in one pass
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//   Plugins module unloading
//   called at the end of module chain unloading, just modular engine left at this point

void UnloadNewPluginsModule(void)
{
	int i;

	if ( !bModuleInitialized) return;

	UnloadPluginOptions();

	// unload everything but the DB
	for (i = pluginList.getCount()-1; i >= 0; i--) {
		pluginEntry* p = pluginList[i];
		if ( !(p->pclass & PCLASS_DB) && p != pluginList_crshdmp)
			Plugin_Uninit(p);
	}

    if (pluginList_crshdmp)
		Plugin_Uninit(pluginList_crshdmp);

	// unload the DB
	for (i = pluginList.getCount()-1; i >= 0; i--) {
		pluginEntry* p = pluginList[i];
		Plugin_Uninit(p);
	}

	if (hPluginListHeap) HeapDestroy(hPluginListHeap);
	hPluginListHeap=0;

	pluginList.destroy();
	pluginListAddr.destroy();
	UninitIni();
}

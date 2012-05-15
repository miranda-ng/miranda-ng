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
#include "commonheaders.h"

// block these plugins
#define DEFMOD_REMOVED_UIPLUGINOPTS     21
#define DEFMOD_REMOVED_PROTOCOLNETLIB   22

// basic export prototypes
typedef int (__cdecl * Miranda_Plugin_Load) ( PLUGINLINK * );
typedef int (__cdecl * Miranda_Plugin_Unload) ( void );
// version control
typedef PLUGININFO * (__cdecl * Miranda_Plugin_Info) ( DWORD mirandaVersion );
typedef PLUGININFOEX * (__cdecl * Miranda_Plugin_InfoEx) ( DWORD mirandaVersion );
// prototype for databases
typedef DATABASELINK * (__cdecl * Database_Plugin_Info) ( void * reserved );
// prototype for clists
typedef int (__cdecl * CList_Initialise) ( PLUGINLINK * );
// Interface support
typedef MUUID * (__cdecl * Miranda_Plugin_Interfaces) ( void );

typedef struct { // can all be NULL
	HINSTANCE hInst;
	Miranda_Plugin_Load Load;
	Miranda_Plugin_Unload Unload;
	Miranda_Plugin_Info Info;
	Miranda_Plugin_InfoEx InfoEx;
	Miranda_Plugin_Interfaces Interfaces;
	Database_Plugin_Info DbInfo;
	CList_Initialise clistlink;
	PLUGININFOEX * pluginInfo;	 // must be freed if hInst==NULL then its a copy
	DATABASELINK * dblink;		 // only valid during module being in memory
} BASIC_PLUGIN_INFO;

#define PCLASS_FAILED	 0x1  	// not a valid plugin, or API is invalid, pluginname is valid
#define PCLASS_BASICAPI  0x2  	// has Load, Unload, MirandaPluginInfo() -> PLUGININFO seems valid, this dll is in memory.
#define PCLASS_DB	 	 0x4    // has DatabasePluginInfo() and is valid as can be, and PCLASS_BASICAPI has to be set too
#define PCLASS_LAST		 0x8    // this plugin should be unloaded after everything else
#define PCLASS_OK		 0x10   // plugin should be loaded, if DB means nothing
#define PCLASS_LOADED	 0x20   // Load() has been called, Unload() should be called.
#define PCLASS_STOPPED   0x40 	// wasn't loaded cos plugin name not on white list
#define PCLASS_CLIST 	 0x80   // a CList implementation
#define PCLASS_SERVICE 	 0x100  // has Service Mode implementation

typedef struct pluginEntry {
	TCHAR pluginname[64];
	unsigned int pclass; // PCLASS_*
	BASIC_PLUGIN_INFO bpi;
	struct pluginEntry * nextclass;
} pluginEntry;

static int sttComparePlugins( const pluginEntry* p1, const pluginEntry* p2 )
{	return ( int )( p1->bpi.hInst - p2->bpi.hInst );
}

static int sttComparePluginsByName( const pluginEntry* p1, const pluginEntry* p2 )
{	return lstrcmp( p1->pluginname, p2->pluginname );
}

LIST<pluginEntry> pluginList( 10, sttComparePluginsByName ), pluginListAddr( 10, sttComparePlugins );

/////////////////////////////////////////////////////////////////////////////////

#define MAX_MIR_VER ULONG_MAX

struct PluginUUIDList {
    MUUID uuid;
    DWORD maxVersion;
} 
static const pluginBannedList[] = 
{
    {{0x7f65393b, 0x7771, 0x4f3f, { 0xa9, 0xeb, 0x5d, 0xba, 0xf2, 0xb3, 0x61, 0xf1 }}, MAX_MIR_VER}, // png2dib
    {{0xe00f1643, 0x263c, 0x4599, { 0xb8, 0x4b, 0x5, 0x3e, 0x5c, 0x51, 0x1d, 0x28 }}, MAX_MIR_VER}, // loadavatars (unicode)
    {{0xc9e01eb0, 0xa119, 0x42d2, { 0xb3, 0x40, 0xe8, 0x67, 0x8f, 0x5f, 0xea, 0xd9 }}, MAX_MIR_VER}, // loadavatars (ansi)
    {{0xb4ef58c4, 0x4458, 0x4e47, { 0xa7, 0x67, 0x5c, 0xae, 0xe5, 0xe7, 0xc, 0x81 }}, MAX_MIR_VER}, // 0.7.x AIM Protocol
    {{0xb529402b, 0x53ba, 0x4c81, { 0x9e, 0x27, 0xd4, 0x31, 0xeb, 0xe8, 0xec, 0x36 }}, MAX_MIR_VER}, // 0.7.x IRC Protocol
    {{0x847bb03c, 0x408c, 0x4f9b, { 0xaa, 0x5a, 0xf5, 0xc0, 0xb7, 0xb5, 0x60, 0x1e }}, MAX_MIR_VER}, // 0.7.x ICQ Protocol
    {{0x1ee5af12, 0x26b0, 0x4290, { 0x8f, 0x97, 0x16, 0x77, 0xcb, 0xe, 0xfd, 0x2b }}, MAX_MIR_VER}, // 0.7.x Jabber Protocol (Unicode)
    {{0xf7f5861d, 0x988d, 0x479d, { 0xa5, 0xbb, 0x80, 0xc7, 0xfa, 0x8a, 0xd0, 0xef }}, MAX_MIR_VER}, // 0.7.x Jabber Protocol (Ansi)
    {{0xdc39da8a, 0x8385, 0x4cd9, { 0xb2, 0x98, 0x80, 0x67, 0x7b, 0x8f, 0xe6, 0xe4 }}, MAX_MIR_VER}, // 0.7.x MSN Protocol (Unicode)
    {{0x29aa3a80, 0x3368, 0x4b78, { 0x82, 0xc1, 0xdf, 0xc7, 0x29, 0x6a, 0x58, 0x99 }}, MAX_MIR_VER}, // 0.7.x MSN Protocol (Ansi)
    {{0xa6648b6c, 0x6fb8, 0x4551, { 0xb4, 0xe7, 0x1, 0x36, 0xf9, 0x16, 0xd4, 0x85 }}, MAX_MIR_VER}, // 0.7.x Yahoo Protocol
    {{0x6ca5f042, 0x7a7f, 0x47cc, { 0xa7, 0x15, 0xfc, 0x8c, 0x46, 0xfb, 0xf4, 0x34 }}, PLUGIN_MAKE_VERSION(3, 0, 4, 0)}, // 0.8.x TabSRMM (Unicode)
	{{0x5889a3ef, 0x7c95, 0x4249, { 0x98, 0xbb, 0x34, 0xe9, 0x5, 0x3a, 0x6e, 0xa0 }}, PLUGIN_MAKE_VERSION(3, 0, 4, 0)},  // 0.8.x TabSRMM (ANSI)
	{{0x84636f78, 0x2057, 0x4302, { 0x8a, 0x65, 0x23, 0xa1, 0x6d, 0x46, 0x84, 0x4c }}, PLUGIN_MAKE_VERSION(2, 9, 0, 4)}, // 0.8.x Scriver (Unicode)
    {{0x1e91b6c9, 0xe040, 0x4a6f, { 0xab, 0x56, 0xdf, 0x76, 0x98, 0xfa, 0xcb, 0xf1 }}, PLUGIN_MAKE_VERSION(2, 9, 0, 4)}, // 0.8.x Scriver (ANSI)
    {{0x240a91dc, 0x9464, 0x457a, { 0x97, 0x87, 0xff, 0x1e, 0xa8, 0x8e, 0x77, 0xe3 }}, PLUGIN_MAKE_VERSION(0, 9, 0, 0)}, // 0.8.x CList Classic (Unicode)
    {{0x552cf71a, 0x249f, 0x4650, { 0xbb, 0x2b, 0x7c, 0xdb, 0x1f, 0xe7, 0xd1, 0x78 }}, PLUGIN_MAKE_VERSION(0, 9, 0, 0)}, // 0.8.x CList Classic (ANSI)
    {{0x8f79b4ee, 0xeb48, 0x4a03, { 0x87, 0x3e, 0x27, 0xbe, 0x6b, 0x7e, 0x9a, 0x25 }}, PLUGIN_MAKE_VERSION(0, 9, 1, 0)}, // 0.8.x Clist Nicer (Unicode)
    {{0x5a070cec, 0xb2ab, 0x4bbe, { 0x8e, 0x48, 0x9c, 0x8d, 0xcd, 0xda, 0x14, 0xc3 }}, PLUGIN_MAKE_VERSION(0, 9, 1, 0)}, // 0.8.x Clist Nicer (ANSI)
    {{0x43909b6, 0xaad8, 0x4d82, { 0x8e, 0xb5, 0x9f, 0x64, 0xcf, 0xe8, 0x67, 0xcd }}, PLUGIN_MAKE_VERSION(0, 9, 0, 8)}, // 0.8.x Clist Modern (Unicode)
    {{0xf6588c56, 0x15dc, 0x4cd7, { 0x8c, 0xf9, 0x48, 0xab, 0x6c, 0x5f, 0xd2, 0xf }}, PLUGIN_MAKE_VERSION(0, 9, 0, 8)}, // 0.8.x Clist Modern (ANSI)
	{{0x2a417ab9, 0x16f2, 0x472d, { 0x9a, 0xe3, 0x41, 0x51, 0x3, 0xc7, 0x8a, 0x64 }}, PLUGIN_MAKE_VERSION(0, 9, 0, 0)}, // 0.8.x Clist MW (Unicode)
	{{0x7ab05d31, 0x9972, 0x4406, { 0x82, 0x3e, 0xe, 0xd7, 0x45, 0xef, 0x7c, 0x56 }}, PLUGIN_MAKE_VERSION(0, 9, 0, 0)} // 0.8.x Clist MW (ANSI)
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

int CallHookSubscribers( HANDLE hEvent, WPARAM wParam, LPARAM lParam );

int LoadDatabaseModule(void);

char * GetPluginNameByInstance( HINSTANCE hInstance )
{
	int i = 0;
	if ( pluginList.getCount() == 0) return NULL;
	for (i = 0; i <  pluginList.getCount(); i++)
	{
		pluginEntry* pe = pluginList[i];
		if (pe->bpi.pluginInfo && pe->bpi.hInst == hInstance)
			return pe->bpi.pluginInfo->shortName;
	}
	return NULL;
}

HINSTANCE GetInstByAddress( void* codePtr )
{
	int idx;
	HINSTANCE result;
	pluginEntry p; p.bpi.hInst = ( HINSTANCE )codePtr;

	if ( pluginListAddr.getCount() == 0 )
		return NULL;

	List_GetIndex(( SortedList* )&pluginListAddr, &p, &idx );
	if ( idx > 0 )
		idx--;

	result = pluginListAddr[idx]->bpi.hInst;

	if (result < hMirandaInst && codePtr > hMirandaInst)
		result = hMirandaInst;
	else if ( idx == 0 && codePtr < ( void* )result )
		result = NULL;

	return result;
}

static int uuidToString(const MUUID uuid, char *szStr, int cbLen)
{
	if (cbLen<1||!szStr) return 0;
	mir_snprintf(szStr, cbLen, "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
		uuid.a, uuid.b, uuid.c, uuid.d[0], uuid.d[1], uuid.d[2], uuid.d[3], uuid.d[4], uuid.d[5], uuid.d[6], uuid.d[7]);
	return 1;
}

static int equalUUID(MUUID u1, MUUID u2)
{
    return memcmp(&u1, &u2, sizeof(MUUID))?0:1;
}

static MUUID miid_last = MIID_LAST;
static MUUID miid_servicemode = MIID_SERVICEMODE;

static int validInterfaceList(Miranda_Plugin_Interfaces ifaceProc)
{
	MUUID *piface = ( ifaceProc ) ? ifaceProc() : NULL;
    int i = 0/*, j*/;
    
	if (!piface)
		return 0;
	if (equalUUID(miid_last, piface[0]))
		return 0;
    /*while (!equalUUID(miid_last, piface[i]) ) {
        for (j=0; j<interfaceBannedListCount; j++) {
            if (equalUUID(interfaceBannedList[j].uuid, piface[i]))
                return 0;
            i++;
        }
        break;
    }*/
	return 1;
}

static int isPluginBanned(MUUID u1, DWORD dwVersion) {
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

static int checkPI( BASIC_PLUGIN_INFO* bpi, PLUGININFOEX* pi )
{
	int bHasValidInfo = FALSE;

	if ( pi == NULL )
		return FALSE;

	if ( bpi->InfoEx ) {
		if ( pi->cbSize == sizeof(PLUGININFOEX))
			if ( !validInterfaceList(bpi->Interfaces) || isPluginBanned( pi->uuid, pi->version ))
				return FALSE;

		bHasValidInfo = TRUE;
	}
	
	if ( !bHasValidInfo )
		if ( bpi->Info && pi->cbSize != sizeof(PLUGININFO))
			return FALSE;

	if ( pi->shortName == NULL || pi->description == NULL || pi->author == NULL ||
		  pi->authorEmail == NULL || pi->copyright == NULL || pi->homepage == NULL )
		return FALSE;

	if ( pi->replacesDefaultModule > DEFMOD_HIGHEST ||
		  pi->replacesDefaultModule == DEFMOD_REMOVED_UIPLUGINOPTS ||
		  pi->replacesDefaultModule == DEFMOD_REMOVED_PROTOCOLNETLIB )
		return FALSE;

	return TRUE;
}

static int checkAPI(TCHAR* plugin, BASIC_PLUGIN_INFO* bpi, DWORD mirandaVersion, int checkTypeAPI, int* exports)
{
	HINSTANCE h = NULL;
	// this is evil but these plugins are buggy/old and people are blaming Miranda
	// fontservice plugin is built into the core now
	{
		TCHAR * p = _tcsrchr(plugin, '\\');
		if ( p != NULL && ++p ) {
			int i;
			for ( i = 0; i < SIZEOF(modulesToSkip); i++ )
				if ( lstrcmpi( p, modulesToSkip[i] ) == 0 )
					return 0;
	}	}

	h = LoadLibrary(plugin);
	if ( h == NULL ) return 0;
	// loaded, check for exports
	bpi->Load = (Miranda_Plugin_Load) GetProcAddress(h, "Load");
	bpi->Unload = (Miranda_Plugin_Unload) GetProcAddress(h, "Unload");
	bpi->Info = (Miranda_Plugin_Info) GetProcAddress(h, "MirandaPluginInfo");
	bpi->InfoEx = (Miranda_Plugin_InfoEx) GetProcAddress(h, "MirandaPluginInfoEx");
	bpi->Interfaces = (Miranda_Plugin_Interfaces) GetProcAddress(h, "MirandaPluginInterfaces");

	// if they were present
	if ( bpi->Load && bpi->Unload && ( bpi->Info || ( bpi->InfoEx && bpi->Interfaces ))) {
		PLUGININFOEX* pi = 0;
		if (bpi->InfoEx)
			pi = bpi->InfoEx(mirandaVersion);
		else
			pi = (PLUGININFOEX*)bpi->Info(mirandaVersion);
		{
			// similar to the above hack but these plugins are checked for a valid interface first (in case there are updates to the plugin later)
			TCHAR* p = _tcsrchr(plugin, '\\');
			if ( pi != NULL && p != NULL && ++p ) {
				if ( !bpi->InfoEx || pi->cbSize != sizeof(PLUGININFOEX)) {
					int i;
					for ( i = 0; i < SIZEOF(expiredModulesToSkip); i++ ) {
						if ( lstrcmpi( p, expiredModulesToSkip[i] ) == 0 ) {
							FreeLibrary(h);
							return 0;
		}	}	}	}	}

		if ( checkPI( bpi, pi )) {
			bpi->pluginInfo = pi;
			// basic API is present
			if ( checkTypeAPI == CHECKAPI_NONE ) {
				bpi->hInst=h;
				return 1;
			}
			// check for DB?
			if ( checkTypeAPI == CHECKAPI_DB ) {
				bpi->DbInfo = (Database_Plugin_Info) GetProcAddress(h, "DatabasePluginInfo");
				if ( bpi->DbInfo ) {
					// fetch internal database function pointers
					bpi->dblink = bpi->DbInfo(NULL);
					// validate returned link structure
					if ( bpi->dblink && bpi->dblink->cbSize==sizeof(DATABASELINK) ) {
						bpi->hInst=h;
						return 1;
					}
					// had DB exports
					if ( exports != NULL ) *exports=1;
				} //if
			} //if

			// check clist ?
			if ( checkTypeAPI == CHECKAPI_CLIST ) {
				bpi->clistlink = (CList_Initialise) GetProcAddress(h, "CListInitialise");
				#if defined( _UNICODE )
					if ( pi->flags & UNICODE_AWARE )
				#endif
				if ( bpi->clistlink ) {
					// nothing more can be done here, this export is a load function
					bpi->hInst=h;
					if ( exports != NULL ) *exports=1;
					return 1;
				}
			}

		} // if
		if ( exports != NULL ) *exports=1;
	} //if
	// not found, unload
	FreeLibrary(h);
	return 0;
}

// returns true if the given file is <anything>.dll exactly
static int valid_library_name(TCHAR *name)
{
	TCHAR * dot = _tcsrchr(name, '.');
	if ( dot != NULL && lstrcmpi(dot + 1, _T("dll")) == 0)
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
static void Plugin_Uninit(pluginEntry * p)
{
	// if it was an installed database plugin, call its unload
	if ( p->pclass & PCLASS_DB )
		p->bpi.dblink->Unload( p->pclass & PCLASS_OK );

	// if the basic API check had passed, call Unload if Load() was ever called
	if ( p->pclass & PCLASS_LOADED )
		p->bpi.Unload();

	// release the library
	if ( p->bpi.hInst != NULL ) {
		// we need to kill all resources which belong to that DLL before calling FreeLibrary
		KillModuleEventHooks( p->bpi.hInst );
		KillModuleServices( p->bpi.hInst );

		FreeLibrary( p->bpi.hInst );
		ZeroMemory( &p->bpi, sizeof( p->bpi ));
	}
	pluginList.remove( p );
	pluginListAddr.remove( p );
}

typedef BOOL (*SCAN_PLUGINS_CALLBACK) ( WIN32_FIND_DATA * fd, TCHAR * path, WPARAM wParam, LPARAM lParam );

static void enumPlugins(SCAN_PLUGINS_CALLBACK cb, WPARAM wParam, LPARAM lParam)
{
	TCHAR exe[MAX_PATH];
	TCHAR search[MAX_PATH];
	TCHAR * p = 0;
	// get miranda's exe path
	GetModuleFileName(NULL, exe, SIZEOF(exe));
	// find the last \ and null it out, this leaves no trailing slash
	p = _tcsrchr(exe, '\\'); if (p) *p = 0;
	// create the search filter
	mir_sntprintf(search, SIZEOF(search), _T("%s\\Plugins\\*.dll"), exe);
	{
		// FFFN will return filenames for things like dot dll+ or dot dllx
		HANDLE hFind=INVALID_HANDLE_VALUE;
		WIN32_FIND_DATA ffd;
		hFind = FindFirstFile(search, &ffd);
		if (hFind != INVALID_HANDLE_VALUE) 
		{
			do {
				if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && valid_library_name(ffd.cFileName))
				{
					cb(&ffd, exe, wParam, lParam);
				} //if
			} while (FindNextFile(hFind, &ffd));
			FindClose(hFind);
		} //if
	}
}

// this is called by the db module to return all DBs plugins, then when it finds the one it likes the others are unloaded
static INT_PTR PluginsEnum(WPARAM, LPARAM lParam)
{
	PLUGIN_DB_ENUM * de = (PLUGIN_DB_ENUM *) lParam;
	pluginEntry * x = pluginListDb;
	if ( de == NULL || de->cbSize != sizeof(PLUGIN_DB_ENUM) || de->pfnEnumCallback == NULL ) return 1;
	while ( x != NULL ) 
	{
		int rc = de->pfnEnumCallback(StrConvA(x->pluginname), x->bpi.dblink, de->lParam);
		if (rc == DBPE_DONE) 
		{
			// this db has been picked, get rid of all the others
			pluginEntry * y = pluginListDb, * n;
			while ( y != NULL ) 
			{
				n = y->nextclass;
				if ( x != y )
					Plugin_Uninit(y);
				y = n;
			} // while
			x->pclass |= PCLASS_LOADED | PCLASS_OK | PCLASS_LAST;
			return 0;
		}
		else if ( rc == DBPE_HALT ) return 1;
		x = x->nextclass;
	} // while
	return pluginListDb != NULL ? 1 : -1;
}

static INT_PTR PluginsGetDefaultArray(WPARAM, LPARAM)
{
	return (INT_PTR)&pluginDefModList;
}

// called in the first pass to create pluginEntry* structures and validate database plugins
static BOOL scanPluginsDir (WIN32_FIND_DATA * fd, TCHAR * path, WPARAM, LPARAM)
{
	int isdb = validguess_db_name(fd->cFileName);
	BASIC_PLUGIN_INFO bpi;
	pluginEntry* p = (pluginEntry*)HeapAlloc(hPluginListHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, sizeof(pluginEntry));
	_tcsncpy(p->pluginname, fd->cFileName, SIZEOF(p->pluginname));
	// plugin name suggests its a db module, load it right now
	if ( isdb ) 
	{
		TCHAR buf[MAX_PATH];
		mir_sntprintf(buf, SIZEOF(buf), _T("%s\\Plugins\\%s"), path, fd->cFileName);
		if (checkAPI(buf, &bpi, mirandaVersion, CHECKAPI_DB, NULL)) 
		{
			// db plugin is valid
			p->pclass |= (PCLASS_DB | PCLASS_BASICAPI);
			// copy the dblink stuff
			p->bpi=bpi;
			// keep a faster list.
			if ( pluginListDb != NULL ) p->nextclass = pluginListDb;
			pluginListDb=p;
		}
		else
			// didn't have basic APIs or DB exports - failed.
			p->pclass |= PCLASS_FAILED;
	}
	else if (validguess_clist_name(fd->cFileName)) 
	{
		// keep a note of this plugin for later
		if ( pluginListUI != NULL ) p->nextclass=pluginListUI;
		pluginListUI=p;
		p->pclass |= PCLASS_CLIST;
	}
	else if (validguess_servicemode_name(fd->cFileName))
	{
		TCHAR buf[MAX_PATH];
		mir_sntprintf(buf, SIZEOF(buf), _T("%s\\Plugins\\%s"), path, fd->cFileName);
		if (checkAPI(buf, &bpi, mirandaVersion, CHECKAPI_NONE, NULL)) 
		{
			p->pclass |= (PCLASS_OK | PCLASS_BASICAPI);
			p->bpi = bpi;
			if (bpi.Interfaces) 
			{
				int i = 0;
				MUUID *piface = bpi.Interfaces();
				while (!equalUUID(miid_last, piface[i])) 
				{
					if (!equalUUID(miid_servicemode, piface[i++]))
						continue;
					p->pclass |= (PCLASS_SERVICE);
					if ( pluginListSM != NULL ) p->nextclass = pluginListSM;
					pluginListSM=p;
	                if (pluginList_crshdmp == NULL &&  lstrcmpi(fd->cFileName, _T("svc_crshdmp.dll")) == 0)
					{
		                pluginList_crshdmp = p;
						p->pclass |= PCLASS_LAST;
					}
					break;
				}
			}
		}
		else
			// didn't have basic APIs or DB exports - failed.
			p->pclass |= PCLASS_FAILED;
	}
	else if (pluginList_freeimg == NULL && lstrcmpi(fd->cFileName, _T("advaimg.dll")) == 0)
		pluginList_freeimg = p;

	// add it to the list
	pluginList.insert( p );
	return TRUE;
}

static void SetPluginOnWhiteList(TCHAR * pluginname, int allow)
{
	DBWriteContactSettingByte(NULL, PLUGINDISABLELIST, StrConvA(pluginname), allow  == 0);
}

// returns 1 if the plugin should be enabled within this profile, filename is always lower case
static int isPluginOnWhiteList(TCHAR * pluginname)
{
	char* pluginnameA = _strlwr(mir_t2a(pluginname));
	int rc = DBGetContactSettingByte(NULL, PLUGINDISABLELIST, pluginnameA, 0);
	mir_free(pluginnameA);
	if ( rc != 0 && askAboutIgnoredPlugins ) 
	{
		TCHAR buf[256];
		mir_sntprintf(buf, SIZEOF(buf), TranslateT("'%s' is disabled, re-enable?"), pluginname);
		if (MessageBox(NULL, buf, TranslateT("Re-enable Miranda plugin?"), MB_YESNO | MB_ICONQUESTION) == IDYES) 
		{
			SetPluginOnWhiteList(pluginname, 1);
			rc = 0;
		}	
	}

	return rc == 0;
}

static pluginEntry* getCListModule(TCHAR * exe, TCHAR * slice, int useWhiteList)
{
	pluginEntry * p = pluginListUI;
	BASIC_PLUGIN_INFO bpi;
	while ( p != NULL )
	{
		mir_sntprintf(slice, &exe[MAX_PATH] - slice, _T("\\Plugins\\%s"), p->pluginname);
		CharLower(p->pluginname);
		if ( useWhiteList ? isPluginOnWhiteList(p->pluginname) : 1 ) {
			if ( checkAPI(exe, &bpi, mirandaVersion, CHECKAPI_CLIST, NULL) ) {
				p->bpi = bpi;
				p->pclass |= PCLASS_LAST | PCLASS_OK | PCLASS_BASICAPI;
				pluginListAddr.insert( p );
				if ( bpi.clistlink(&pluginCoreLink) == 0 ) {
					p->bpi = bpi;
					p->pclass |= PCLASS_LOADED;
					return p;
				}
				else Plugin_Uninit( p );
			} //if
		} //if
		p = p->nextclass;
	}
	return NULL;
}

int UnloadPlugin(TCHAR* buf, int bufLen)
{
	int i;
	for ( i = pluginList.getCount()-1; i >= 0; i-- ) 
	{
		pluginEntry* p = pluginList[i];
		if (!_tcsicmp( p->pluginname, buf)) 
		{
			GetModuleFileName( p->bpi.hInst, buf, bufLen);
			Plugin_Uninit( p );
			return TRUE;
		}	
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//   Service plugins functions

char **GetSeviceModePluginsList(void)
{
	int i = 0;
	char **list = NULL;
	pluginEntry * p = pluginListSM;
	while ( p != NULL ) {
		i++;
		p = p->nextclass;
	}
	if ( i ) {
		list = (char**)mir_calloc( (i + 1) * sizeof(char*) );
		p = pluginListSM;
		i = 0;
		while ( p != NULL ) {
			list[i++] = p->bpi.pluginInfo->shortName;
			p = p->nextclass;
		}
	}
	return list;
}

void SetServiceModePlugin( int idx )
{
	serviceModeIdx = idx;
}

int LoadServiceModePlugin(void)
{
	int i = 0;
	pluginEntry * p = pluginListSM;

	if ( serviceModeIdx < 0 )
		return 0;

	while ( p != NULL ) {
		if ( serviceModeIdx == i++ ) {
			if ( p->bpi.Load(&pluginCoreLink) == 0 ) {
				p->pclass |= PCLASS_LOADED;
				if ( CallService( MS_SERVICEMODE_LAUNCH, 0, 0 ) != CALLSERVICE_NOTFOUND )
					return 1;
				else {
					MessageBox(NULL, TranslateT("Unable to load plugin in Service Mode!"), p->pluginname, 0);
					return -1;
				}
			}
			Plugin_Uninit( p );
			return -1;
		}
		p = p->nextclass;
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//   Event hook to unload all non-core plugins
//   hooked very late, after all the internal plugins, blah

void UnloadNewPlugins(void)
{
	int i;

	// unload everything but the special db/clist plugins
	for ( i = pluginList.getCount()-1; i >= 0; i-- ) {
		pluginEntry* p = pluginList[i];
		if ( !(p->pclass & PCLASS_LAST) && (p->pclass & PCLASS_OK))
			Plugin_Uninit( p );
}	}

/////////////////////////////////////////////////////////////////////////////////////////
//
//   Plugins options page dialog

typedef struct
{
	int   flags;
	char* author;
	char* authorEmail;
	char* description;
	char* copyright;
	char* homepage;
	MUUID uuid;
}
	PluginListItemData;

static BOOL dialogListPlugins(WIN32_FIND_DATA * fd, TCHAR * path, WPARAM, LPARAM lParam)
{
	LVITEM it;
	int iRow;
	HWND hwndList=(HWND)lParam;
	BASIC_PLUGIN_INFO pi;
	int exports=0;
	TCHAR buf[MAX_PATH];
	int isdb = 0;
	HINSTANCE gModule;
	PluginListItemData* dat;

	mir_sntprintf(buf, SIZEOF(buf), _T("%s\\Plugins\\%s"), path, fd->cFileName);
	CharLower(fd->cFileName);
	gModule = GetModuleHandle(buf);
	if ( checkAPI(buf, &pi, mirandaVersion, CHECKAPI_NONE, &exports) == 0 ) {
		// failed to load anything, but if exports were good, show some info.
		return TRUE;
	}
	isdb = pi.pluginInfo->replacesDefaultModule == DEFMOD_DB;
	ZeroMemory(&it, sizeof(it));
	it.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	it.pszText = fd->cFileName;
	it.iImage = ( pi.pluginInfo->flags & 1 ) ? 0 : 1;
	it.lParam = (LPARAM)( dat = (PluginListItemData*)mir_alloc( sizeof( PluginListItemData )));
	iRow=SendMessage( hwndList, LVM_INSERTITEM, 0, (LPARAM)&it );
	if ( isPluginOnWhiteList(fd->cFileName) )
		ListView_SetItemState(hwndList, iRow, !isdb ? 0x2000 : 0x3000, LVIS_STATEIMAGEMASK);
	if ( iRow != -1 ) {
		dat->flags = pi.pluginInfo->replacesDefaultModule;
		dat->author = mir_strdup( pi.pluginInfo->author );
		dat->authorEmail = mir_strdup( pi.pluginInfo->authorEmail );
		dat->copyright = mir_strdup( pi.pluginInfo->copyright );
		dat->description = mir_strdup( pi.pluginInfo->description );
		dat->homepage = mir_strdup( pi.pluginInfo->homepage );
		if ( pi.pluginInfo->cbSize == sizeof( PLUGININFOEX ))
			dat->uuid = pi.pluginInfo->uuid;
		else
			memset( &dat->uuid, 0, sizeof(dat->uuid));

		TCHAR *shortNameT = mir_a2t(pi.pluginInfo->shortName);
		ListView_SetItemText(hwndList, iRow, 1, shortNameT);
		mir_free(shortNameT);

		mir_sntprintf(buf, SIZEOF(buf), _T("%d.%d.%d.%d"), HIBYTE(HIWORD(pi.pluginInfo->version)), 
			LOBYTE(HIWORD(pi.pluginInfo->version)), HIBYTE(LOWORD(pi.pluginInfo->version)), 
			LOBYTE(LOWORD(pi.pluginInfo->version)));
		ListView_SetItemText(hwndList, iRow, 2, buf);

		it.mask = LVIF_IMAGE;
		it.iItem = iRow;
		it.iSubItem = 3;
		it.iImage = ( gModule != NULL ) ? 2 : 3;
		ListView_SetItem( hwndList, &it );
	}
	else mir_free( dat );
	FreeLibrary(pi.hInst);
	return TRUE;
}

static void RemoveAllItems( HWND hwnd )
{
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = 0;
	while ( ListView_GetItem( hwnd, &lvi )) {
		PluginListItemData* dat = ( PluginListItemData* )lvi.lParam;
		mir_free( dat->author );
		mir_free( dat->authorEmail );
		mir_free( dat->copyright );
		mir_free( dat->description );
		mir_free( dat->homepage );
		mir_free( dat );
		lvi.iItem ++;
}	}

INT_PTR CALLBACK DlgPluginOpt(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		HWND hwndList=GetDlgItem(hwndDlg,IDC_PLUGLIST);
		LVCOLUMN col;
		HIMAGELIST hIml = ImageList_Create(16, 16, ILC_MASK | (IsWinVerXPPlus()? ILC_COLOR32 : ILC_COLOR16), 4, 0);
		ImageList_AddIcon_IconLibLoaded( hIml, SKINICON_OTHER_UNICODE );
		ImageList_AddIcon_IconLibLoaded( hIml, SKINICON_OTHER_ANSI );
		ImageList_AddIcon_IconLibLoaded( hIml, SKINICON_OTHER_LOADED );
		ImageList_AddIcon_IconLibLoaded( hIml, SKINICON_OTHER_NOTLOADED );
		ListView_SetImageList( hwndList, hIml, LVSIL_SMALL );

		TranslateDialogDefault(hwndDlg);

		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.pszText = TranslateT("Plugin");
		col.cx = 70;//max = 140;
		ListView_InsertColumn(hwndList,0,&col);

		col.pszText=TranslateT("Name");
		col.cx = 70;//max = 220;
		ListView_InsertColumn(hwndList,1,&col);

		col.pszText=TranslateT("Version");
		col.cx=55;
		ListView_InsertColumn(hwndList,2,&col);

		col.pszText=_T("");
		col.cx=20;
		ListView_InsertColumn(hwndList,3,&col);
		//ListView_InsertColumn(hwndList,4,&col);

		// XXX: Won't work on windows 95 without IE3+ or 4.70
		ListView_SetExtendedListViewStyleEx( hwndList, 0, LVS_EX_SUBITEMIMAGES | LVS_EX_CHECKBOXES | LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT );
		// scan the plugin dir for plugins, cos
		enumPlugins( dialogListPlugins, ( WPARAM )hwndDlg, ( LPARAM )hwndList );
		// sort out the headers
        {
            int w, max;

            ListView_SetColumnWidth( hwndList, 0, LVSCW_AUTOSIZE ); // dll name
            w = ListView_GetColumnWidth( hwndList, 0 );
            if (w>140) {
                ListView_SetColumnWidth( hwndList, 0, 140 );
                w = 140;
            }
            max = w<140? 220+140-w:220;
            ListView_SetColumnWidth( hwndList, 1, LVSCW_AUTOSIZE ); // short name
            w = ListView_GetColumnWidth( hwndList, 1 );
            if (w>max)
                ListView_SetColumnWidth( hwndList, 1, max );
        }
		return TRUE;
	}
	case WM_NOTIFY:
	{
		NMLISTVIEW * hdr = (NMLISTVIEW *) lParam;
		if ( hdr && hdr->hdr.code == LVN_ITEMCHANGED && hdr->uOldState != 0
			&& (hdr->uNewState == 0x1000 || hdr->uNewState == 0x2000 ) && IsWindowVisible(hdr->hdr.hwndFrom) ) {
			HWND hwndList = GetDlgItem(hwndDlg,IDC_PLUGLIST);
			PluginListItemData* dat;
			int iRow;
			LVITEM it;
			it.mask=LVIF_PARAM | LVIF_STATE;
			it.iItem = hdr->iItem;
			if ( !ListView_GetItem( hwndList, &it ))
				break;

			dat = ( PluginListItemData* )it.lParam;
			if ( dat->flags == DEFMOD_DB ) {
				ListView_SetItemState(hwndList, hdr->iItem, 0x3000, LVIS_STATEIMAGEMASK);
				return FALSE;
			}
			// if enabling and replaces, find all other replaces and toggle off
			if ( hdr->uNewState & 0x2000 && dat->flags != 0 )  {
				for ( iRow=0; iRow != -1; ) {
					if ( iRow != hdr->iItem ) {
						LVITEM dt;
						dt.mask = LVIF_PARAM;
						dt.iItem = iRow;
						if ( ListView_GetItem( hwndList, &dt )) {
							PluginListItemData* dat2 = ( PluginListItemData* )dt.lParam;
							if ( dat2->flags == dat->flags ) {
								// the lParam is unset, so when the check is unset the clist block doesnt trigger
								int lParam = dat2->flags;
								dat2->flags = 0;
								ListView_SetItemState(hwndList, iRow, 0x1000, LVIS_STATEIMAGEMASK );
								dat2->flags = lParam;
					}	}	}

					iRow = ListView_GetNextItem( hwndList, iRow, LVNI_ALL );
			}	}

			ShowWindow( GetDlgItem(hwndDlg, IDC_RESTART ), TRUE );
			SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
			break;
		}

		if ( hdr && hdr->hdr.code == LVN_ITEMCHANGED && IsWindowVisible(hdr->hdr.hwndFrom) && hdr->iItem != -1 ) {
			TCHAR buf[1024];
			int sel = hdr->uNewState & LVIS_SELECTED;
			HWND hwndList = GetDlgItem(hwndDlg, IDC_PLUGLIST);
			LVITEM lvi = { 0 };
			lvi.mask = LVIF_PARAM;
			lvi.iItem = hdr->iItem;
			if ( ListView_GetItem( hwndList, &lvi )) {
				PluginListItemData* dat = ( PluginListItemData* )lvi.lParam;

				ListView_GetItemText(hwndList, hdr->iItem, 1, buf, SIZEOF(buf));
				SetWindowText(GetDlgItem(hwndDlg,IDC_PLUGININFOFRAME),sel ? buf : _T(""));

				SetWindowTextA(GetDlgItem(hwndDlg,IDC_PLUGINAUTHOR), sel ? dat->author : "" );
				SetWindowTextA(GetDlgItem(hwndDlg,IDC_PLUGINEMAIL), sel ? dat->authorEmail : "" );
				{
					TCHAR* p = LangPackPcharToTchar( dat->description );
					SetWindowText(GetDlgItem(hwndDlg,IDC_PLUGINLONGINFO), sel ? p : _T(""));
					mir_free( p );
				}
				SetWindowTextA(GetDlgItem(hwndDlg,IDC_PLUGINCPYR), sel ? dat->copyright : "" );
				SetWindowTextA(GetDlgItem(hwndDlg,IDC_PLUGINURL), sel ? dat->homepage : "" );
				if(equalUUID(miid_last, dat->uuid))
					SetWindowText(GetDlgItem(hwndDlg,IDC_PLUGINPID), sel ? TranslateT("<none>") : _T(""));
				else
				{
					char szUID[128];
					uuidToString( dat->uuid, szUID, sizeof(szUID));
					SetWindowTextA(GetDlgItem(hwndDlg,IDC_PLUGINPID), sel ? szUID : "" );
				}
		}	}

		if ( hdr && hdr->hdr.code == PSN_APPLY ) {
			HWND hwndList=GetDlgItem(hwndDlg,IDC_PLUGLIST);
			int iRow;
			int iState;
			TCHAR buf[1024];
			for (iRow=0 ; iRow != (-1) ; ) {
				ListView_GetItemText(hwndList, iRow, 0, buf, SIZEOF(buf));
				iState=ListView_GetItemState(hwndList, iRow, LVIS_STATEIMAGEMASK);
				SetPluginOnWhiteList(buf, iState&0x2000 ? 1 : 0);
				iRow=ListView_GetNextItem(hwndList, iRow, LVNI_ALL);
		}	}
		break;
	}

	case WM_COMMAND:
		if ( HIWORD(wParam) == STN_CLICKED ) {
			switch (LOWORD(wParam)) {
				case IDC_PLUGINEMAIL:
				case IDC_PLUGINURL:
				{
					char buf[512];
					char * p = &buf[7];
					lstrcpyA(buf,"mailto:");
					if ( GetWindowTextA(GetDlgItem(hwndDlg, LOWORD(wParam)), p, SIZEOF(buf) - 7) ) {
						CallService(MS_UTILS_OPENURL,0,(LPARAM) (LOWORD(wParam)==IDC_PLUGINEMAIL ? buf : p) );
					}
					break;
				}
				case IDC_GETMOREPLUGINS:
				{
					CallService(MS_UTILS_OPENURL,0,(LPARAM) "http://addons.miranda-im.org/index.php?action=display&id=1" );
					break;
				}
		}	}
		break;

	case WM_DESTROY:
		RemoveAllItems( GetDlgItem( hwndDlg, IDC_PLUGLIST ));
		break;
	}
	return FALSE;
}

static int PluginOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.hInstance = hMirandaInst;
	odp.pfnDlgProc = DlgPluginOpt;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_PLUGINS);
	odp.position = 1300000000;
	odp.pszTitle = LPGEN("Plugins");
	odp.flags = ODPF_BOLDGROUPS;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );
	return 0;
}

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
	askAboutIgnoredPlugins=(UINT) GetPrivateProfileInt( _T("PluginLoader"), _T("AskAboutIgnoredPlugins"), 0, mirandabootini);

	// if Crash Dumper is present, load it to provide Crash Reports
	if (pluginList_crshdmp != NULL && isPluginOnWhiteList(pluginList_crshdmp->pluginname)) 
    {
		if ( pluginList_crshdmp->bpi.Load(&pluginCoreLink) == 0 )
			pluginList_crshdmp->pclass |= PCLASS_LOADED | PCLASS_LAST;
		else
			Plugin_Uninit( pluginList_crshdmp );
    }

	// if freeimage is present, load it to provide the basic core functions
	if ( pluginList_freeimg != NULL ) {
		BASIC_PLUGIN_INFO bpi;
		mir_sntprintf(slice, &exe[SIZEOF(exe)] - slice, _T("\\Plugins\\%s"), pluginList_freeimg->pluginname);
		if ( checkAPI(exe, &bpi, mirandaVersion, CHECKAPI_NONE, NULL) ) {
			pluginList_freeimg->bpi = bpi;
			pluginList_freeimg->pclass |= PCLASS_OK | PCLASS_BASICAPI;
			if ( bpi.Load(&pluginCoreLink) == 0 )
				pluginList_freeimg->pclass |= PCLASS_LOADED;
			else
				Plugin_Uninit( pluginList_freeimg );
	}	}

	// first load the clist cos alot of plugins need that to be present at Load()
	for ( useWhiteList = 1; useWhiteList >= 0 && clist == NULL; useWhiteList-- )
		clist=getCListModule(exe, slice, useWhiteList);
	/* the loop above will try and get one clist DLL to work, if all fail then just bail now */
	if ( clist == NULL ) {
		// result = 0, no clist_* can be found
		if ( pluginListUI )
			MessageBox(NULL, TranslateT("Unable to start any of the installed contact list plugins, I even ignored your preferences for which contact list couldn't load any."), _T("Miranda IM"), MB_OK | MB_ICONINFORMATION);
		else
			MessageBox(NULL, TranslateT("Can't find a contact list plugin! you need clist_classic or any other clist plugin.") , _T("Miranda IM"), MB_OK | MB_ICONINFORMATION);
		return 1;
	}

	/* enable and disable as needed  */
	p = pluginListUI;
	while ( p != NULL ) {
		SetPluginOnWhiteList(p->pluginname, clist != p ? 0 : 1 );
		p = p->nextclass;
	}
	/* now loop thru and load all the other plugins, do this in one pass */

	for ( i=0; i < pluginList.getCount(); i++ ) {
		p = pluginList[i];
		CharLower(p->pluginname);
		if (!(p->pclass & (PCLASS_LOADED | PCLASS_DB | PCLASS_CLIST))) 
		{
			if (isPluginOnWhiteList(p->pluginname))
			{
				BASIC_PLUGIN_INFO bpi;
				mir_sntprintf(slice, &exe[SIZEOF(exe)] - slice, _T("\\Plugins\\%s"), p->pluginname);
				if ( checkAPI(exe, &bpi, mirandaVersion, CHECKAPI_NONE, NULL) ) {
					int rm = bpi.pluginInfo->replacesDefaultModule;
					p->bpi = bpi;
					p->pclass |= PCLASS_OK | PCLASS_BASICAPI;

					if ( pluginDefModList[rm] == NULL ) {
						pluginListAddr.insert( p );
						if ( bpi.Load(&pluginCoreLink) == 0 ) {
							p->pclass |= PCLASS_LOADED;
							msgModule |= (bpi.pluginInfo->replacesDefaultModule == DEFMOD_SRMESSAGE);
						}
						else {
							Plugin_Uninit( p );
							i--;
						}
						if ( rm ) pluginDefModList[rm]=p;
					} //if
					else {
						SetPluginOnWhiteList( p->pluginname, 0 );
						Plugin_Uninit( p );
						i--;
					}
				}
				else p->pclass |= PCLASS_FAILED;
			}
			else {
				Plugin_Uninit( p );
				i--;
			}
		}
		else if ( p->bpi.hInst != NULL )
		{
			pluginListAddr.insert( p );
			p->pclass |= PCLASS_LOADED;
		}
	}
	if (!msgModule)
		MessageBox(NULL, TranslateT("No messaging plugins loaded. Please install/enable one of the messaging plugins, for instance, \"srmm.dll\""), _T("Miranda IM"), MB_OK | MB_ICONINFORMATION);

	HookEvent(ME_OPT_INITIALISE, PluginOptionsInit);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//   Plugins module initialization
//   called before anything real is loaded, incl. database

int LoadNewPluginsModuleInfos(void)
{
	bModuleInitialized = TRUE;

	hPluginListHeap=HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
	mirandaVersion = (DWORD)CallService(MS_SYSTEM_GETVERSION, 0, 0);
	//
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
//
//   Plugins module unloading
//   called at the end of module chain unloading, just modular engine left at this point

void UnloadNewPluginsModule(void)
{
	int i;

	if ( !bModuleInitialized ) return;

	// unload everything but the DB
	for ( i = pluginList.getCount()-1; i >= 0; i-- ) {
		pluginEntry* p = pluginList[i];
		if ( !(p->pclass & PCLASS_DB) && p != pluginList_crshdmp )
			Plugin_Uninit( p );
	}

    if ( pluginList_crshdmp )
		Plugin_Uninit( pluginList_crshdmp );

	// unload the DB
	for ( i = pluginList.getCount()-1; i >= 0; i-- ) {
		pluginEntry* p = pluginList[i];
		Plugin_Uninit( p );
	}

	if ( hPluginListHeap ) HeapDestroy(hPluginListHeap);
	hPluginListHeap=0;

	pluginList.destroy();
	pluginListAddr.destroy();
	UninitIni();
}

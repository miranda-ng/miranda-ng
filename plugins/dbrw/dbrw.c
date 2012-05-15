/*
dbRW

Copyright (c) 2005-2009 Robert Rainwater

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
#include "dbrw.h"

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"dbRW SQLite DB Driver",
	PLUGIN_MAKE_VERSION(DBRW_VER_MAJOR,DBRW_VER_MINOR,0,0),
	#ifdef DBRW_DEBUG
    #ifdef DBRW_VER_ALPHA
	"Miranda IM database driver engine powered by SQLite v" SQLITE_VERSION " [Debug Build Alpha #" DBRW_VER_ALPHA "]",
    #else
	"Miranda IM database driver engine powered by SQLite v" SQLITE_VERSION " [Debug Build]",
    #endif
	#else
    #ifdef DBRW_VER_ALPHA
    "Miranda IM database driver engine powered by SQLite v" SQLITE_VERSION " [Alpha #" DBRW_VER_ALPHA "]",
    #else
	"Miranda IM database driver engine powered by SQLite v" SQLITE_VERSION,
    #endif
	#endif
	"Robert Rainwater",
	"rainwater@miranda-im.org",
	"Copyright © 2005-2011 Robert Rainwater",
	"http://www.miranda-im.org/",
	0,
	DEFMOD_DB,
    {0xf3ca0e5e, 0x249a, 0x40f0, {0x8d, 0x74, 0x80, 0xa8, 0xe, 0xf0, 0xc8, 0x3d}} // {F3CA0E5E-249A-40f0-8D74-80A80EF0C83D}
};

HINSTANCE g_hInst;

struct MM_INTERFACE   mmi;
struct UTF8_INTERFACE utfi;
struct LIST_INTERFACE li;

sqlite3 *g_sqlite;
char g_szDbPath[MAX_PATH];
HANDLE hSettingChangeEvent;
HANDLE hContactDeletedEvent;
HANDLE hContactAddedEvent;
HANDLE hEventFilterAddedEvent;
HANDLE hEventAddedEvent;
HANDLE hEventDeletedEvent;
PLUGINLINK *pluginLink;


enum {
    DBRW_TABLE_SETTINGS = 0,
    DBRW_TABLE_CONTACTS,
    DBRW_TABLE_EVENTS,
	DBRW_TABLE_CORE,
    DBRW_TABLE_COUNT
};
char *dbrw_tables[DBRW_TABLE_COUNT] = {
"create table dbrw_settings (id integer, module varchar(255), setting varchar(255), type integer, val any, primary key(id,module,setting));",
"create table dbrw_contacts (id integer primary key,createtime integer);",
"create table dbrw_events (id integer primary key,eventtime integer,flags integer,eventtype integer, blob any, blobsize integer, contactid integer,modulename varchar(255),inserttime integer);",
"create table dbrw_core (setting varchar(255) primary key not null, val any);"
};

static int dbrw_getCaps(int flags);
static int dbrw_getFriendlyName(char *buf, size_t cch, int shortName);
static int dbrw_makeDatabase(char *profile, int *error);
static int dbrw_grokHeader(char *profile, int *error);
static int dbrw_Load(char *profile, void *link);
static int dbrw_Unload(int wasLoaded);

DATABASELINK dblink = {
	sizeof(DATABASELINK),
    dbrw_getCaps,
    dbrw_getFriendlyName,
    dbrw_makeDatabase,
    dbrw_grokHeader,
    dbrw_Load,
    dbrw_Unload
};

char* utf8_encode(const char* src) {
	size_t len;
	char* result;
	wchar_t* tempBuf;

	if (src==NULL)
		return NULL;
	len = strlen(src);
	result = (char*)malloc(len*3+1);
	if (result==NULL)
		return NULL;
	tempBuf = (wchar_t*)alloca((len+1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, src, -1, tempBuf, (int)len);
	tempBuf[len] = 0;
	{
		wchar_t* s = tempBuf;
		BYTE* d = (BYTE*)result;

		while(*s) {
			int U = *s++;

			if (U<0x80) {
				*d++ = (BYTE)U;
			}
			else if (U<0x800) {
				*d++ = 0xC0+((U>>6)&0x3F);
				*d++ = 0x80+(U&0x003F);
			}
			else {
				*d++ = 0xE0+(U>>12);
				*d++ = 0x80+((U>>6)&0x3F);
				*d++ = 0x80+(U&0x3F);
			}	
		}
		*d = 0;
	}
	return result;
}

static int dbrw_getCaps(int flags) {
    return 0;
}

static int dbrw_getFriendlyName(char *buf, size_t cch, int shortName) {
	strncpy(buf, shortName?"dbRW Driver":pluginInfo.shortName, cch);
	return 0;
}

static int dbrw_makeDatabase(char *profile, int *error) {
	sqlite3 *sql = NULL;
	int rc;
    char *szPath = utf8_encode(profile);
    
	rc = sqlite3_open(szPath, &sql);
    free(szPath);
	if (rc==SQLITE_OK) {
        int x;

		sqlite3_exec(sql, "PRAGMA page_size = 8192;", NULL, NULL, NULL);
        for (x=0;x<DBRW_TABLE_COUNT;x++) {
            rc = sqlite3_exec(sql, dbrw_tables[x], NULL, NULL, NULL);
            if (rc!=SQLITE_OK)
                break;
		}
		if (rc==SQLITE_OK) {
			sqlite3_exec(sql, "CREATE INDEX idx_settings1 ON dbrw_settings(id, module, setting);", NULL, NULL, NULL);
			sqlite3_exec(sql, "CREATE INDEX idx_settings2 ON dbrw_settings(id, module);", NULL, NULL, NULL);
			sqlite3_exec(sql, "CREATE INDEX idx_settings3 ON dbrw_settings(module);", NULL, NULL, NULL);
			sqlite3_exec(sql, "CREATE INDEX idx_events1 ON dbrw_events(contactid);", NULL, NULL, NULL);
			sqlite3_exec(sql, "CREATE INDEX idx_events2 ON dbrw_events(id,contactid);", NULL, NULL, NULL);
			sqlite3_exec(sql, "INSERT INTO dbrw_core VALUES(\"SchemaVersion\",\"" DBRW_SCHEMA_VERSION "\");", NULL, NULL, NULL);
		}
	}
	sqlite3_close(sql);
	rc = (rc==SQLITE_OK) ? 0 : 1;
	if (error != NULL) *error=rc;
	return rc;
}

static int dbrw_grokHeader(char *profile, int *error) {
    HANDLE hFile = CreateFileA(profile, GENERIC_READ, 0, NULL, OPEN_ALWAYS, 0, NULL);
    int rc = 1;
    int err = EGROKPRF_CANTREAD;
	
    if (hFile!=INVALID_HANDLE_VALUE) {
        BOOL r;
        char buf[64];
        DWORD dwRead;

        ZeroMemory(buf, sizeof(buf));
        r = ReadFile(hFile, buf, sizeof(buf), &dwRead, NULL);
        CloseHandle(hFile);
        if (r && memcmp(buf, DBRW_HEADER_STR, strlen(DBRW_HEADER_STR))==0) {
            sqlite3 *sqlcheck = NULL;
            char *szPath = utf8_encode(profile);

            rc = sqlite3_open(szPath, &sqlcheck);
            free(szPath);
            if (rc==SQLITE_OK) {
                sqlite3_stmt *stmt;
                err = EGROKPRF_UNKHEADER;
            
                sqlite3_prepare_v2(sqlcheck, "select * from sqlite_master where type = 'table' and name = 'dbrw_core';", -1, &stmt, NULL);
                if (sqlite3_step(stmt)==SQLITE_ROW) {
                    
                    sqlite3_finalize(stmt);
                    sqlite3_prepare_v2(sqlcheck, "select val from dbrw_core where setting = 'SchemaVersion';", -1, &stmt, NULL);
                    if (sqlite3_step(stmt)==SQLITE_ROW) {
                        int sVersion;
                        
                        sVersion = sqlite3_column_int(stmt, 0);
                        if (sVersion==atoi(DBRW_SCHEMA_VERSION))
                            rc = 0;
                        else {
                            // TODO: Return as valid and upgrade in 
                            // dbrw_Load() if schema version is upgradable
                        }
                    }
                }
                sqlite3_finalize(stmt);
                sqlite3_close(sqlcheck);
            }
        }
		else err = r ? EGROKPRF_UNKHEADER : EGROKPRF_CANTREAD;
    }
    if (error!=NULL) 
		*error = err;
    return rc;
}

static int dbrw_Load(char *profile, void *link) 
{
	pluginLink = (PLUGINLINK*)link;

	mir_getMMI( &mmi );
	mir_getLI( &li );
	mir_getUTFI( &utfi );

    {
        char *szLocalPath = mir_utf8encode(profile);
        mir_snprintf(g_szDbPath, sizeof(g_szDbPath), "%s", szLocalPath);
        dbrw_free(szLocalPath);
    }
    sql_init(); // intialize sqlite library routines
	if (sql_open(g_szDbPath, &g_sqlite)!=SQLITE_OK)
		return 1;
	#ifdef DBRW_LOGGING
	utils_log_init();
	#endif
    #ifdef DBRW_VER_ALPHA
    log3("Loading dbRW v%s alpha #%s (SQLite v%s)", DBRW_VER_STRING, DBRW_VER_ALPHA, SQLITE_VERSION);
    #else
	log2("Loading dbRW v%s (SQLite v%s)", DBRW_VER_STRING, SQLITE_VERSION);
    #endif
    utils_vacuum_check();
	{
        sql_exec(g_sqlite, "BEGIN TRANSACTION;");
		sql_exec(g_sqlite, "PRAGMA locking_mode = EXCLUSIVE;");
		sql_exec(g_sqlite, "PRAGMA synchronous = NORMAL;");
		sql_exec(g_sqlite, "PRAGMA cache_size = 12000;");
		sql_exec(g_sqlite, "PRAGMA temp_store = MEMORY;");
        sql_exec(g_sqlite, "COMMIT;");
	}
    
	// Create Services
	CreateServiceFunction(MS_DB_SETSAFETYMODE, utils_setSafetyMode);
	CreateServiceFunction(MS_DB_CONTACT_GETSETTING, setting_getSetting);
	CreateServiceFunction(MS_DB_CONTACT_GETSETTING_STR, setting_getSettingStr);
	CreateServiceFunction(MS_DB_CONTACT_GETSETTINGSTATIC, setting_getSettingStatic);
	CreateServiceFunction(MS_DB_CONTACT_FREEVARIANT, setting_freeVariant);
	CreateServiceFunction(MS_DB_CONTACT_WRITESETTING, setting_writeSetting);
	CreateServiceFunction(MS_DB_CONTACT_DELETESETTING, setting_deleteSetting);
	CreateServiceFunction(MS_DB_CONTACT_ENUMSETTINGS, setting_enumSettings);
    CreateServiceFunction(MS_DB_SETSETTINGRESIDENT, settings_setResident);
	CreateServiceFunction(MS_DB_CONTACT_GETCOUNT, contacts_getCount);
	CreateServiceFunction(MS_DB_CONTACT_FINDFIRST, contacts_findFirst);
	CreateServiceFunction(MS_DB_CONTACT_FINDNEXT, contacts_findNext);
	CreateServiceFunction(MS_DB_CONTACT_DELETE, contacts_delete);
	CreateServiceFunction(MS_DB_CONTACT_ADD, contacts_add);
	CreateServiceFunction(MS_DB_CONTACT_IS, contacts_isContact);
	CreateServiceFunction(MS_DB_EVENT_GETCOUNT, events_getCount);
	CreateServiceFunction(MS_DB_EVENT_ADD, events_add);
	CreateServiceFunction(MS_DB_EVENT_DELETE, events_delete);
	CreateServiceFunction(MS_DB_EVENT_GETBLOBSIZE, events_getBlobSize);
	CreateServiceFunction(MS_DB_EVENT_GET, events_get);
	CreateServiceFunction(MS_DB_EVENT_MARKREAD, events_markRead);
	CreateServiceFunction(MS_DB_EVENT_GETCONTACT, events_getContact);
	CreateServiceFunction(MS_DB_EVENT_FINDFIRST, events_findFirst);
	CreateServiceFunction(MS_DB_EVENT_FINDFIRSTUNREAD, events_findFirstUnread);
	CreateServiceFunction(MS_DB_EVENT_FINDLAST, events_findLast);
	CreateServiceFunction(MS_DB_EVENT_FINDNEXT, events_findNext);
	CreateServiceFunction(MS_DB_EVENT_FINDPREV, events_findPrev);
	CreateServiceFunction(MS_DB_CRYPT_ENCODESTRING, utils_encodeString);
	CreateServiceFunction(MS_DB_CRYPT_DECODESTRING, utils_decodeString);
	CreateServiceFunction(MS_DB_MODULES_ENUM, setting_modulesEnum);
    
	// Events Hooks
	hSettingChangeEvent = CreateHookableEvent(ME_DB_CONTACT_SETTINGCHANGED);
	hContactDeletedEvent = CreateHookableEvent(ME_DB_CONTACT_DELETED);
	hContactAddedEvent = CreateHookableEvent(ME_DB_CONTACT_ADDED);
	hEventFilterAddedEvent = CreateHookableEvent(ME_DB_EVENT_FILTER_ADD);
	hEventAddedEvent = CreateHookableEvent(ME_DB_EVENT_ADDED);
	hEventDeletedEvent = CreateHookableEvent(ME_DB_EVENT_DELETED);
    
	// Initialize modules
	contacts_init();
	settings_init();
	events_init();

	// Prepare SQL statements
	sql_prepare_statements();

	return 0;
}

static int dbrw_Unload(int wasLoaded) {
    if (!wasLoaded) 
        return 0;
	events_destroy();
	settings_destroy();
	contacts_destroy();
    sql_close(g_sqlite);
	sql_destroy();
    log0("dbRW unloaded sucessfully");
	#ifdef DBRW_LOGGING
	utils_log_destroy();
	#endif
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved) {
    g_hInst = hInstDLL;
    return TRUE;
}

__declspec(dllexport) DATABASELINK* DatabasePluginInfo(void *reserved) {
	return &dblink;
}

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) {
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_DATABASE, MIID_LAST};
__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void) {
	return interfaces;
}

int __declspec(dllexport) Load(PLUGINLINK *link) {
	return 1;
}

int __declspec(dllexport) Unload(void) {
	return 0;
}

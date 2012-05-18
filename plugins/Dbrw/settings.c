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

CRITICAL_SECTION csSettingsDb;
static HANDLE hHeap = 0, hSettingsThread = 0, hSettingsEvent = 0;
static SortedList sSettingNames, sContactSettings, sGlobalSettings, sResidentSettings;
static int mirCp = CP_ACP;

static int settings_cmpSettingNames(void *p1, void *p2);
static int settings_cmpGlobalSettings(void* p1, void* p2);
static int settings_cmpContactSettings(void *p1, void *p2);
static int settings_cmpResidentSettings(void *p1, void *p2);
static char *settings_getCachedSettingName(const char *szModuleName, const char *szSettingName);
static void settings_setCachedValueUpdateStatus(HANDLE hContact, char *szSetting, int update);
static void settings_freeCachedVariant(DBVARIANT* V);
static DBVARIANT *settings_getCachedValue(HANDLE hContact, char *szSetting, int bAllocate);
static int settings_getContactSettingWorker(HANDLE hContact, DBCONTACTGETSETTING *dbcgs, int isStatic);
static void settings_writeToDB(HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *value);
static void settings_writeUpdatedSettings();
static unsigned __stdcall settings_threadProc(void *arg);
static int settings_isResident(char *szSetting);

typedef struct {
	char *name;
	DWORD nameHash;
	DBVARIANT value;
	int update;
} DBCachedGlobalValue;

typedef struct DBCachedContactValue_tag {
	char *name;
	DWORD nameHash;
	DBVARIANT value;
	int update;
	struct DBCachedContactValue_tag *next;
} DBCachedContactValue;

typedef struct {
	HANDLE hContact;
	HANDLE hNext;
	DBCachedContactValue *first;
} DBCachedContactValueList;

typedef struct {
    char *name;
    DWORD nameHash;
    char *module;
    DWORD moduleHash;
} DBCachedResidentSettingValue;

enum {
	SQL_SET_STMT_REPLACE=0,
	SQL_SET_STMT_DELETE,
	SQL_SET_STMT_READ,
	SQL_SET_STMT_ENUM,
	SQL_SET_STMT_ENUMMODULES,
	SQL_SET_STMT_SETTINGCHECK,
    SQL_SET_STMT_DELETECONTACT,
	SQL_SET_STMT_NUM
};
char *settings_stmts[SQL_SET_STMT_NUM] = {
	"REPLACE INTO dbrw_settings VALUES(?,?,?,?,?);",
	"DELETE FROM dbrw_settings WHERE setting = ? AND module = ? AND id = ?;",
	"SELECT type,val FROM dbrw_settings WHERE setting = ? AND module = ? AND id = ? LIMIT 1;",
	"SELECT setting from dbrw_settings where id = ? AND module = ? ORDER by setting;",
	"SELECT DISTINCT module from dbrw_settings;",
    "SELECT count(*) FROM dbrw_settings WHERE setting = ? AND module = ? AND id = ?;",
	"DELETE FROM dbrw_settings WHERE id = ?;"
};
static sqlite3_stmt *settings_stmts_prep[SQL_SET_STMT_NUM] = {0};

void settings_init() {
	InitializeCriticalSection(&csSettingsDb);
	hHeap = HeapCreate(0, 0, 0);
	ZeroMemory(&sSettingNames, sizeof(sSettingNames));
	ZeroMemory(&sContactSettings, sizeof(sContactSettings));
	ZeroMemory(&sGlobalSettings, sizeof(sGlobalSettings));
    ZeroMemory(&sResidentSettings, sizeof(sResidentSettings));
	sSettingNames.increment = 100;
	sSettingNames.sortFunc = settings_cmpSettingNames;
	sContactSettings.increment = 100;
	sContactSettings.sortFunc = settings_cmpContactSettings;
	sGlobalSettings.increment = 100;
	sGlobalSettings.sortFunc = settings_cmpGlobalSettings;
    sResidentSettings.increment = 100;
    sResidentSettings.sortFunc = settings_cmpResidentSettings;
	sql_prepare_add(settings_stmts, settings_stmts_prep, SQL_SET_STMT_NUM);
	mirCp = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
    hSettingsEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hSettingsThread = (HANDLE)mir_forkthreadex(settings_threadProc, 0, 0, 0);
}

void settings_destroy() {
    if (hSettingsEvent) {
        SetEvent(hSettingsEvent);
        WaitForSingleObjectEx(hSettingsThread, INFINITE, FALSE);
        CloseHandle(hSettingsThread);
    }
	settings_writeUpdatedSettings();
	HeapDestroy(hHeap);
	li.List_Destroy(&sSettingNames);
	li.List_Destroy(&sContactSettings);
	li.List_Destroy(&sGlobalSettings);
	li.List_Destroy(&sResidentSettings);
	DeleteCriticalSection(&csSettingsDb);
}

static int settings_cmpSettingNames(void *p1, void *p2) {
	return strcmp((char*)p1, (char*)p2);
}

static int settings_cmpGlobalSettings(void *p1, void *p2) {
	DBCachedGlobalValue *v1 = (DBCachedGlobalValue*)p1;
	DBCachedGlobalValue *v2 = (DBCachedGlobalValue*)p2;
	
	if (v1->nameHash!=v2->nameHash)
		return v1->nameHash-v2->nameHash;
	return strcmp(v1->name, v2->name);
}

static int settings_cmpContactSettings(void *p1, void *p2) {
	if (*(long*)p1==*(long*)p2)
		return 0;
	return *(long*)p1-*(long*)p2;
}

static int settings_cmpResidentSettings(void *p1, void *p2) {
	DBCachedResidentSettingValue *v1 = (DBCachedResidentSettingValue*)p1;
	DBCachedResidentSettingValue *v2 = (DBCachedResidentSettingValue*)p2;
	
	if (v1->nameHash!=v2->nameHash)
		return v1->nameHash-v2->nameHash;
	return strcmp(v1->name, v2->name);
}

static char *settings_getCachedSettingName(const char *szModuleName, const char *szSettingName) {
	size_t nameLen = strlen(szModuleName)+strlen(szSettingName)+2;
	int idx;
	char *szFullName = (char*)alloca(nameLen), *ret;

	mir_snprintf(szFullName, nameLen, "%s/%s", szModuleName, szSettingName);
	if (li.List_GetIndex(&sSettingNames, szFullName, &idx))
		return (char*)sSettingNames.items[idx];
	ret = (char*)HeapAlloc(hHeap, 0, nameLen);
	mir_snprintf(ret, nameLen, "%s", szFullName);
	li.List_Insert(&sSettingNames, ret, idx);
	return ret;
}

static void settings_setCachedValueUpdateStatus(HANDLE hContact, char *szSetting, int update) {
	int idx;

	if (hContact==0) {
		DBCachedGlobalValue Vtemp, *V;

		Vtemp.name = szSetting;
		Vtemp.nameHash = utils_hashString(szSetting);
		if (li.List_GetIndex(&sGlobalSettings, &Vtemp, &idx)) {
			V = (DBCachedGlobalValue*)sGlobalSettings.items[idx];
            if (settings_isResident(szSetting)) {
                V->update = 0;
            }
            else {
                V->update = update;
            }
		}
		return;
	}
	else {
		DBCachedContactValue *V;
		DBCachedContactValueList VLtemp,*VL;
		DWORD hash = utils_hashString(szSetting);

		VLtemp.hContact = hContact;
		if (li.List_GetIndex(&sContactSettings, &VLtemp, &idx)) {
			VL = (DBCachedContactValueList*)sContactSettings.items[idx];
		}
		else return;
		for (V = VL->first; V!=NULL; V=V->next)
			if ((hash==V->nameHash)&&(strcmp(V->name, szSetting)==0)) 
				break;
		if (V) {
            if (settings_isResident(szSetting)) {
                V->update = 0;
            }
            else {
                V->update = update;
            }
        }
	}
}

static void settings_freeCachedVariant(DBVARIANT* V) {
	if ((V->type==DBVT_ASCIIZ||V->type==DBVT_UTF8)&&V->pszVal!=NULL)
		HeapFree(hHeap, 0, V->pszVal);
}

static void settings_setCachedVariant(DBVARIANT *VNew, DBVARIANT *VCached) {
	char *szSave = (VCached->type==DBVT_UTF8||VCached->type==DBVT_ASCIIZ) ? VCached->pszVal : NULL;

	memcpy(VCached, VNew, sizeof(DBVARIANT));
	if ((VNew->type==DBVT_UTF8||VNew->type==DBVT_ASCIIZ )&&VNew->pszVal!=NULL) {
		if (szSave!=NULL)
			VCached->pszVal = (char*)HeapReAlloc(hHeap, 0, szSave, strlen(VNew->pszVal)+1);
		else
			VCached->pszVal = (char*)HeapAlloc(hHeap, 0, strlen(VNew->pszVal)+1);
		strcpy(VCached->pszVal, VNew->pszVal);
	}
}

static DBVARIANT *settings_getCachedValue(HANDLE hContact, char *szSetting, int bAllocate) {
	int idx;

	if (hContact==0) {
		DBCachedGlobalValue Vtemp, *V;

		Vtemp.name = szSetting;
		Vtemp.nameHash = utils_hashString(szSetting);
		if (li.List_GetIndex(&sGlobalSettings, &Vtemp, &idx)) {
			V = (DBCachedGlobalValue*)sGlobalSettings.items[idx];
			if (bAllocate==-1) {
				settings_freeCachedVariant(&V->value);
				li.List_Remove(&sGlobalSettings, idx);
				HeapFree(hHeap, 0, V);
				return NULL;
			}
		}
		else {
			if (bAllocate!=1)
				return NULL;
			V = (DBCachedGlobalValue*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(DBCachedGlobalValue));
			V->name = szSetting;
			V->nameHash = utils_hashString(szSetting);
            V->update = 0;
			li.List_Insert(&sGlobalSettings, V, idx);
		}
		return &V->value;
	}
	else {
		DBCachedContactValue *V, *V1;
		DBCachedContactValueList VLtemp,*VL;
		DWORD hash = utils_hashString(szSetting);

		VLtemp.hContact = hContact;
		if (li.List_GetIndex(&sContactSettings, &VLtemp, &idx)) {
			VL = (DBCachedContactValueList*)sContactSettings.items[idx];
		}
		else {
			if (bAllocate==-1) 
				return NULL;
			VL = (DBCachedContactValueList*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(DBCachedContactValueList));
			VL->hContact = hContact;
			li.List_Insert(&sContactSettings, VL, idx);
		}
		for (V = VL->first; V!=NULL; V=V->next)
			if ((hash==V->nameHash)&&(strcmp(V->name, szSetting)==0)) 
				break;
		if (V==NULL) {	
			if (bAllocate!=1)
				return NULL;
			V = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(DBCachedContactValue));
			V->next = VL->first;
			VL->first = V;
			V->name = szSetting;
			V->nameHash = hash;
            V->update = 0;
		}
		else if (bAllocate==-1) {
			settings_freeCachedVariant(&V->value);
			if (VL->first==V)
				VL->first = V->next;
			for (V1=VL->first; V1!=NULL; V1=V1->next) {
				if (V1->next==V) {
					V1->next = V->next;
					break;
				}
			}
			HeapFree(hHeap, 0, V);
			return NULL;
		}
		return &V->value;
	}
}

static void settings_writeToDB(HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *value) {
	// TODO: Check the parameters
	sqlite3_bind_int(settings_stmts_prep[SQL_SET_STMT_REPLACE], 1, (int)hContact);
	sqlite3_bind_text(settings_stmts_prep[SQL_SET_STMT_REPLACE], 2, szModule, -1, SQLITE_STATIC);
	sqlite3_bind_text(settings_stmts_prep[SQL_SET_STMT_REPLACE], 3, szSetting, -1, SQLITE_STATIC);
	sqlite3_bind_int(settings_stmts_prep[SQL_SET_STMT_REPLACE], 4, (int)value->type);
	switch (value->type) {
		case DBVT_BYTE:
			sqlite3_bind_int(settings_stmts_prep[SQL_SET_STMT_REPLACE], 5, (int)value->bVal);
			break;
		case DBVT_WORD:
			sqlite3_bind_int(settings_stmts_prep[SQL_SET_STMT_REPLACE], 5, (int)value->wVal);
			break;
		case DBVT_DWORD:
			sqlite3_bind_int(settings_stmts_prep[SQL_SET_STMT_REPLACE], 5, (int)value->dVal);
			break;
		case DBVT_UTF8:
		case DBVT_ASCIIZ:
			if (value->pszVal)
				sqlite3_bind_text(settings_stmts_prep[SQL_SET_STMT_REPLACE], 5, value->pszVal, -1, SQLITE_STATIC);
			break;
		case DBVT_BLOB:
            if (value->pbVal) {
				sqlite3_bind_blob(settings_stmts_prep[SQL_SET_STMT_REPLACE], 5, value->pbVal, value->cpbVal, SQLITE_STATIC);
            }
            break;
		default:
			sql_reset(settings_stmts_prep[SQL_SET_STMT_REPLACE]);
            return;
	}
	if (sql_step(settings_stmts_prep[SQL_SET_STMT_REPLACE])!=SQLITE_DONE) {
		log2("Error writing: %s/%s", szModule, szSetting);
	}
    sql_reset(settings_stmts_prep[SQL_SET_STMT_REPLACE]);
}

static void settings_writeUpdatedSettings() {
	int idx, dbWrite = 0;
	DBCachedGlobalValue *V;
	DBCachedContactValueList *VL;
	DBCachedContactValue *VI;
	char *szTok, *szTokTmp1, *szTokTmp2;
    
	EnterCriticalSection(&csSettingsDb);
	for (idx=0; idx<sGlobalSettings.realCount; idx++) {
		V = (DBCachedGlobalValue*)sGlobalSettings.items[idx];
		if (V->update) {
			szTok = dbrw_alloc(strlen(V->name)+1);
			strcpy(szTok, V->name);
			szTokTmp1 = strtok(szTok, "/");
			if (szTokTmp1) {
				szTokTmp2 = szTok+strlen(szTokTmp1)+1;
				if (szTokTmp2) {
                    if (!dbWrite) {
                        sql_stmt_begin();
                        dbWrite = 1;
                    }
					settings_writeToDB(0, szTokTmp1, szTokTmp2, &V->value);
				}
			}
			dbrw_free(szTok);
			V->update = 0;
		}
	}
	for (idx=0; idx<sContactSettings.realCount; idx++) {
		VL = (DBCachedContactValueList*)sContactSettings.items[idx];
		for (VI=VL->first; VI!=NULL; VI=VI->next) {
			if (VI->update) {
				szTok = dbrw_alloc(strlen(VI->name)+1);
				strcpy(szTok, VI->name);
				szTokTmp1 = strtok(szTok, "/");
				if (szTokTmp1) {
					szTokTmp2 = szTok+strlen(szTokTmp1)+1;
					if (szTokTmp2) {
                        if (!dbWrite) {
                            sql_stmt_begin();
                            dbWrite = 1;
                        }
						settings_writeToDB(VL->hContact, szTokTmp1, szTokTmp2, &VI->value);
					}
				}
				dbrw_free(szTok);
				VI->update = 0;
			}
		}
	}
    if (dbWrite)
        sql_stmt_end();
	LeaveCriticalSection(&csSettingsDb);
}

static unsigned __stdcall settings_threadProc(void *arg) {
    DWORD dwWait;
    
    for(;;) {
        dwWait = WaitForSingleObjectEx(hSettingsEvent, DBRW_SETTINGS_FLUSHCACHE, TRUE);

        if (dwWait==WAIT_OBJECT_0) 
            break;
        else if(dwWait == WAIT_TIMEOUT) {
            settings_writeUpdatedSettings();
        }
        else if (dwWait == WAIT_IO_COMPLETION)
            if (Miranda_Terminated()) 
                break;
    }
    CloseHandle(hSettingsEvent);
    hSettingsEvent = NULL;
    return 0;
}

static int settings_getContactSettingWorker(HANDLE hContact, DBCONTACTGETSETTING *dbcgs, int isStatic) {
	char* szCachedSettingName;

	if (!dbcgs->szSetting||!dbcgs->szModule)
		return 1;
	
	szCachedSettingName = settings_getCachedSettingName(dbcgs->szModule, dbcgs->szSetting);
	{
		DBVARIANT *pCachedValue = settings_getCachedValue(hContact, szCachedSettingName, 0);

		if (pCachedValue!=NULL) {
			if (pCachedValue->type==DBVT_ASCIIZ||pCachedValue->type==DBVT_UTF8) {
                size_t cbOrigLen = dbcgs->pValue->cchVal;
                char *cbOrigPtr = dbcgs->pValue->pszVal;
                
                memcpy(dbcgs->pValue, pCachedValue, sizeof(DBVARIANT));
				if (isStatic) {
					size_t cbLen = 0;

					if (pCachedValue->pszVal!=NULL)
						cbLen = strlen(pCachedValue->pszVal);
					cbOrigLen--;
					dbcgs->pValue->pszVal = cbOrigPtr;
					if (cbLen<cbOrigLen)
						cbOrigLen = cbLen;
					memcpy(dbcgs->pValue->pszVal, pCachedValue->pszVal, cbOrigLen);
					dbcgs->pValue->pszVal[cbOrigLen] = 0;
					dbcgs->pValue->cchVal = (WORD)cbLen;
				}
				else {
					dbcgs->pValue->pszVal = (char*)dbrw_alloc(strlen(pCachedValue->pszVal)+1);
					strcpy(dbcgs->pValue->pszVal, pCachedValue->pszVal);
				}
			}
            else {
                memcpy(dbcgs->pValue, pCachedValue, sizeof(DBVARIANT));
            }
			return (pCachedValue->type==DBVT_DELETED) ? 1 : 0;
		}
	}
    if (settings_isResident(szCachedSettingName)) {
        return 1;
    }
	// Read from db
	sqlite3_bind_text(settings_stmts_prep[SQL_SET_STMT_READ], 1, dbcgs->szSetting, -1, SQLITE_STATIC);
	sqlite3_bind_text(settings_stmts_prep[SQL_SET_STMT_READ], 2, dbcgs->szModule, -1, SQLITE_STATIC);
	sqlite3_bind_int(settings_stmts_prep[SQL_SET_STMT_READ], 3, (int)hContact);
	if (sql_step(settings_stmts_prep[SQL_SET_STMT_READ])!=SQLITE_ROW) {
		if (dbcgs->pValue->type!=DBVT_BLOB) {
			DBVARIANT* pCachedValue = settings_getCachedValue(hContact, szCachedSettingName, 1);

			if (pCachedValue!=NULL)
				pCachedValue->type = DBVT_DELETED;
		}
		sql_reset(settings_stmts_prep[SQL_SET_STMT_READ]);
		return 1;
	}
	dbcgs->pValue->type = (int)sqlite3_column_int(settings_stmts_prep[SQL_SET_STMT_READ], 0);
	switch(dbcgs->pValue->type) {
		case DBVT_BYTE:
			dbcgs->pValue->bVal = (BYTE)sqlite3_column_int(settings_stmts_prep[SQL_SET_STMT_READ], 1);
			break;
		case DBVT_WORD:
			dbcgs->pValue->wVal = (WORD)sqlite3_column_int(settings_stmts_prep[SQL_SET_STMT_READ], 1);
			break;
		case DBVT_DWORD:
			dbcgs->pValue->dVal = (DWORD)sqlite3_column_int(settings_stmts_prep[SQL_SET_STMT_READ], 1);
			break;
		case DBVT_UTF8:
		case DBVT_ASCIIZ:
		{
			const char *p = sqlite3_column_text(settings_stmts_prep[SQL_SET_STMT_READ], 1);

			if (p!=NULL) {
				size_t len = strlen(p) + 1;		
				size_t copylen = isStatic ? (len < dbcgs->pValue->cchVal ? len : dbcgs->pValue->cchVal) : len;
				if (!isStatic) 
					dbcgs->pValue->pszVal = dbrw_alloc(len);
				memmove(dbcgs->pValue->pszVal, p, copylen);
			}
			else {
				dbcgs->pValue->pszVal = 0;
			}
			break;
		}
		case DBVT_BLOB:
		{
			size_t len = sqlite3_column_bytes(settings_stmts_prep[SQL_SET_STMT_READ], 1);
            
			if (len) {		
				size_t copylen = isStatic ? ( len < dbcgs->pValue->cpbVal ? len : dbcgs->pValue->cpbVal ) : len;
				if (!isStatic) 
					dbcgs->pValue->pbVal=dbrw_alloc(copylen);
				CopyMemory(dbcgs->pValue->pbVal, sqlite3_column_blob(settings_stmts_prep[SQL_SET_STMT_READ], 1), copylen);
                dbcgs->pValue->cpbVal = (WORD)copylen;
			}
			else {
				dbcgs->pValue = 0;
			}
		}
	}
	sql_reset(settings_stmts_prep[SQL_SET_STMT_READ]);
	// Insert in cache
	if ( dbcgs->pValue->type!=DBVT_BLOB) {
		DBVARIANT *pCachedValue = settings_getCachedValue(hContact, szCachedSettingName, 1);
		if (pCachedValue!=NULL)
			settings_setCachedVariant(dbcgs->pValue, pCachedValue);
	}
	return 0;
}

INT_PTR setting_getSetting(WPARAM wParam, LPARAM lParam) {
	DBCONTACTGETSETTING* dgs = (DBCONTACTGETSETTING*)lParam;

	EnterCriticalSection(&csSettingsDb);
	dgs->pValue->type = 0;
	if (settings_getContactSettingWorker((HANDLE)wParam, dgs, 0)) {
		LeaveCriticalSection(&csSettingsDb);
		return 1;
	}
	if (dgs->pValue->type==DBVT_UTF8 ) {
		WCHAR* tmp = mir_utf8decodeW(dgs->pValue->pszVal);
		if (tmp) {
			BOOL bUsed = FALSE;
			int  result = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, tmp, -1, NULL, 0, NULL, &bUsed);

			mir_free(dgs->pValue->pszVal);

			if (bUsed || result == 0) {
				dgs->pValue->type = DBVT_WCHAR;
				dgs->pValue->pwszVal = tmp;
			}
			else {
				dgs->pValue->type = DBVT_ASCIIZ;
				dgs->pValue->pszVal = mir_alloc(result);
				WideCharToMultiByte(mirCp, WC_NO_BEST_FIT_CHARS, tmp, -1, dgs->pValue->pszVal, result, NULL, NULL);
				mir_free(tmp);
			}
		}
		else {
			dgs->pValue->type = DBVT_ASCIIZ;
			mir_free(tmp);
		}
	}
	LeaveCriticalSection(&csSettingsDb);
	return 0;
}

INT_PTR setting_getSettingStr(WPARAM wParam, LPARAM lParam) {
	DBCONTACTGETSETTING* dgs = (DBCONTACTGETSETTING*)lParam;
	int iSaveType = dgs->pValue->type;
	
	EnterCriticalSection(&csSettingsDb);
	if (settings_getContactSettingWorker((HANDLE)wParam, dgs, 0)) {
		LeaveCriticalSection(&csSettingsDb);
		return 1;
	}
	if (iSaveType==0||iSaveType==dgs->pValue->type) {
		LeaveCriticalSection(&csSettingsDb);
		return 0;
	}
	if (dgs->pValue->type!=DBVT_ASCIIZ&&dgs->pValue->type!=DBVT_UTF8) {
		LeaveCriticalSection(&csSettingsDb);
		return 0;
	}
	if (iSaveType==DBVT_WCHAR) {
		if (dgs->pValue->type!=DBVT_UTF8) {
			int len = MultiByteToWideChar(CP_ACP, 0, dgs->pValue->pszVal, -1, NULL, 0);
			wchar_t* wszResult = (wchar_t*)dbrw_alloc((len+1)*sizeof(wchar_t));

			if (wszResult==NULL) {
				LeaveCriticalSection(&csSettingsDb);
				return 1;
			}
			MultiByteToWideChar(CP_ACP, 0, dgs->pValue->pszVal, -1, wszResult, len);
			wszResult[len] = 0;
			dbrw_free(dgs->pValue->pszVal);
			dgs->pValue->pwszVal = wszResult;
		}
		else {
			char *savePtr = dgs->pValue->pszVal;

			mir_utf8decode(dgs->pValue->pszVal, &dgs->pValue->pwszVal);
			dbrw_free(savePtr);
		}
	}
	else if (iSaveType==DBVT_UTF8) {
		char *tmpBuf = mir_utf8encode(dgs->pValue->pszVal);

		if (tmpBuf==NULL) {
			LeaveCriticalSection(&csSettingsDb);
			return 1;
		}
		dbrw_free(dgs->pValue->pszVal);
		dgs->pValue->pszVal = tmpBuf;
	}
	else if (iSaveType==DBVT_ASCIIZ)
		mir_utf8decode(dgs->pValue->pszVal, NULL);
	dgs->pValue->type = iSaveType;
	LeaveCriticalSection(&csSettingsDb);
	return 0;
}

INT_PTR setting_getSettingStatic(WPARAM wParam, LPARAM lParam) {
	DBCONTACTGETSETTING* dgs = (DBCONTACTGETSETTING*)lParam;

	EnterCriticalSection(&csSettingsDb);
	if (settings_getContactSettingWorker((HANDLE)wParam, dgs, 1)) {
		LeaveCriticalSection(&csSettingsDb);
		return 1;
	}
	if (dgs->pValue->type==DBVT_UTF8 ) {
		mir_utf8decode(dgs->pValue->pszVal, NULL);
		dgs->pValue->type = DBVT_ASCIIZ;
	}
	LeaveCriticalSection(&csSettingsDb);
	return 0;
}

INT_PTR setting_freeVariant(WPARAM wParam, LPARAM lParam) {
	DBVARIANT *dbv = (DBVARIANT*)lParam;
	if (dbv==0) 
		return 1;
	switch (dbv->type) {
		case DBVT_ASCIIZ:
		case DBVT_UTF8:
		case DBVT_WCHAR:
		{
			if (dbv->pszVal) 
				dbrw_free(dbv->pszVal);
			dbv->pszVal = 0;
			break;
		}
		case DBVT_BLOB:
		{
			if (dbv->pbVal) 
				dbrw_free(dbv->pbVal);
			dbv->pbVal = 0;
			break;
		}
	}
	dbv->type = 0;
	return 0;
}

INT_PTR setting_writeSetting(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING*)lParam;
	
	if (dbcws==NULL)
		return 1;
	if (dbcws->value.type==DBVT_WCHAR) {
		if (dbcws->value.pszVal!=NULL) {
			char *val = mir_utf8encodeW(dbcws->value.pwszVal);
			if (val== NULL)
				return 1;
			dbcws->value.pszVal = (char*)alloca(strlen(val)+1);
			strcpy(dbcws->value.pszVal, val);
			dbrw_free(val);
			dbcws->value.type = DBVT_UTF8;
		}
		else return 1;
	}
	if (dbcws->value.type!=DBVT_BYTE&&
		dbcws->value.type!=DBVT_WORD&&
		dbcws->value.type!=DBVT_DWORD&&
		dbcws->value.type!=DBVT_ASCIIZ&&
		dbcws->value.type!=DBVT_UTF8&&
		dbcws->value.type!=DBVT_BLOB)
		return 1;
	if ((!dbcws->szModule)||(!dbcws->szSetting)||
		((dbcws->value.type==DBVT_ASCIIZ||dbcws->value.type==DBVT_UTF8)&&dbcws->value.pszVal == NULL)||
		(dbcws->value.type==DBVT_BLOB&&dbcws->value.pbVal==NULL))
		return 1;
	EnterCriticalSection(&csSettingsDb);
	{
		char *szCachedSettingName = settings_getCachedSettingName(dbcws->szModule, dbcws->szSetting);
		if (dbcws->value.type!=DBVT_BLOB) {
			DBVARIANT *pCachedValue = settings_getCachedValue(hContact, szCachedSettingName, 1);
			
			if (pCachedValue!=NULL) {
				BOOL isIdentical = FALSE;

				if (pCachedValue->type==dbcws->value.type) {
					switch(pCachedValue->type) {
						case DBVT_BYTE:
							isIdentical = pCachedValue->bVal==dbcws->value.bVal;
							break;
						case DBVT_WORD:
							isIdentical = pCachedValue->wVal==dbcws->value.wVal;
							break;
						case DBVT_DWORD:
							isIdentical = pCachedValue->dVal==dbcws->value.dVal;
							break;
						case DBVT_UTF8:
						case DBVT_ASCIIZ:
							isIdentical = strcmp(pCachedValue->pszVal, dbcws->value.pszVal)==0;
							break;
					}
					if (isIdentical) {
						LeaveCriticalSection(&csSettingsDb);
						return 0;
					}
				}
				settings_setCachedVariant(&dbcws->value, pCachedValue);
				// set key to write on timer update
				settings_setCachedValueUpdateStatus(hContact, szCachedSettingName, 1);
			}
		}
		else settings_getCachedValue(hContact, szCachedSettingName, -1);
	}
	// Only write blobs to the db immediately (do we want to cache blobs?)
	if (dbcws->value.type==DBVT_BLOB) {
		settings_writeToDB(hContact, dbcws->szModule, dbcws->szSetting, &dbcws->value);
	}
	LeaveCriticalSection(&csSettingsDb);
	NotifyEventHooks(hSettingChangeEvent, wParam, lParam);
	return 0;
}

INT_PTR setting_deleteSetting(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	DBCONTACTGETSETTING *dbcgs = (DBCONTACTGETSETTING*)lParam;

	if (!dbcgs->szModule||!dbcgs->szSetting)
		return 1;
	EnterCriticalSection(&csSettingsDb);
	{
        int rc = 0;
		char *szCachedSettingName = settings_getCachedSettingName(dbcgs->szModule, dbcgs->szSetting);
		
		// remove setting from cache
		if (szCachedSettingName)
			settings_getCachedValue(hContact, szCachedSettingName, -1);
        
        // check if exists
		sqlite3_bind_text(settings_stmts_prep[SQL_SET_STMT_SETTINGCHECK], 1, dbcgs->szSetting, -1, SQLITE_STATIC);
		sqlite3_bind_text(settings_stmts_prep[SQL_SET_STMT_SETTINGCHECK], 2, dbcgs->szModule, -1, SQLITE_STATIC);
		sqlite3_bind_int(settings_stmts_prep[SQL_SET_STMT_SETTINGCHECK], 3, (int)hContact);
        if (sql_step(settings_stmts_prep[SQL_SET_STMT_SETTINGCHECK])==SQLITE_ROW)
            rc = sqlite3_column_int(settings_stmts_prep[SQL_SET_STMT_SETTINGCHECK], 0);
        if (rc==0) { // should really check not 1 but lets be nice
			sql_reset(settings_stmts_prep[SQL_SET_STMT_SETTINGCHECK]);
			LeaveCriticalSection(&csSettingsDb);
			return 1;
        }
        sql_reset(settings_stmts_prep[SQL_SET_STMT_SETTINGCHECK]);

		// Delete from db
		sqlite3_bind_text(settings_stmts_prep[SQL_SET_STMT_DELETE], 1, dbcgs->szSetting, -1, SQLITE_STATIC);
		sqlite3_bind_text(settings_stmts_prep[SQL_SET_STMT_DELETE], 2, dbcgs->szModule, -1, SQLITE_STATIC);
		sqlite3_bind_int(settings_stmts_prep[SQL_SET_STMT_DELETE], 3, (int)hContact);
		if (sql_step(settings_stmts_prep[SQL_SET_STMT_DELETE])!=SQLITE_DONE) {
			sql_reset(settings_stmts_prep[SQL_SET_STMT_DELETE]);
			LeaveCriticalSection(&csSettingsDb);
			return 1;
		}
		sql_reset(settings_stmts_prep[SQL_SET_STMT_DELETE]);
	}
	LeaveCriticalSection(&csSettingsDb);
	{
		DBCONTACTWRITESETTING dbcws;
		dbcws.szModule = dbcgs->szModule;
		dbcws.szSetting = dbcgs->szSetting;
		dbcws.value.type = DBVT_DELETED;
		NotifyEventHooks(hSettingChangeEvent, wParam, (LPARAM)&dbcws);
	}
	return 0;
}

INT_PTR setting_enumSettings(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	DBCONTACTENUMSETTINGS *dbces = (DBCONTACTENUMSETTINGS*)lParam;
	int rc = -1;

	if (!dbces->szModule)
		return -1;
    settings_writeUpdatedSettings();
	EnterCriticalSection(&csSettingsDb);
	sqlite3_bind_int(settings_stmts_prep[SQL_SET_STMT_ENUM], 1, (int)hContact);
	sqlite3_bind_text(settings_stmts_prep[SQL_SET_STMT_ENUM], 2, dbces->szModule, -1, SQLITE_STATIC);
	while (sql_step(settings_stmts_prep[SQL_SET_STMT_ENUM])==SQLITE_ROW) {
		const char *sczSetting = sqlite3_column_text(settings_stmts_prep[SQL_SET_STMT_ENUM], 0);
		if (sczSetting) {
            char * szCachedSetting = settings_getCachedSettingName(dbces->szModule, sczSetting);
            if (szCachedSetting&&!settings_isResident(szCachedSetting)) {
                rc = (dbces->pfnEnumProc)(sczSetting,dbces->lParam);
            }
		}
	}
	sql_reset(settings_stmts_prep[SQL_SET_STMT_ENUM]);
	LeaveCriticalSection(&csSettingsDb);
	return rc;
}

INT_PTR setting_modulesEnum(WPARAM wParam, LPARAM lParam) {
	DBMODULEENUMPROC proc = (DBMODULEENUMPROC)lParam;
	LPARAM lParamReal = (LPARAM)wParam;
	int rc = 0;
	int offset = 1;
	const char *szModule;
	
    /* Flush db to disk */
    settings_writeUpdatedSettings();
    /* End flush */
	EnterCriticalSection(&csSettingsDb);
	while (sql_step(settings_stmts_prep[SQL_SET_STMT_ENUMMODULES])==SQLITE_ROW && !rc) {
		szModule = sqlite3_column_text(settings_stmts_prep[SQL_SET_STMT_ENUMMODULES], 0);
		LeaveCriticalSection(&csSettingsDb);
		rc = (proc)(szModule, (DWORD)offset++, lParamReal);
		EnterCriticalSection(&csSettingsDb);
	}
	sql_reset(settings_stmts_prep[SQL_SET_STMT_ENUMMODULES]);
	LeaveCriticalSection(&csSettingsDb);
	return rc;
}

// Assume critical section
void settings_deleteContactData(HANDLE hContact) {
	int idx;
	DBCachedContactValueList VLtemp,*VL;
	DBCachedContactValue *V;
    
    EnterCriticalSection(&csSettingsDb);
	VLtemp.hContact = hContact;
	if (li.List_GetIndex(&sContactSettings, &VLtemp, &idx)) {
		VL = (DBCachedContactValueList*)sContactSettings.items[idx];
		
		V = VL->first;
		while (V) {
			settings_freeCachedVariant(&V->value);
			V->value.type = DBVT_DELETED;
			V = V->next;
		}
	}
    sqlite3_bind_int(settings_stmts_prep[SQL_SET_STMT_DELETECONTACT], 1, (int)hContact);
    sql_step(settings_stmts_prep[SQL_SET_STMT_DELETECONTACT]);
    sql_reset(settings_stmts_prep[SQL_SET_STMT_DELETECONTACT]);
    LeaveCriticalSection(&csSettingsDb);
}

INT_PTR settings_setResident(WPARAM wParam, LPARAM lParam) {
    EnterCriticalSection(&csSettingsDb);
    {
        DBCachedResidentSettingValue Vtemp, *V;
        int resident = (int)wParam, idx;
        char *szSetting = (char*)lParam;
        
        if (!szSetting) {
            LeaveCriticalSection(&csSettingsDb);
            return 0;
        }
        Vtemp.name = szSetting;
        Vtemp.nameHash = utils_hashString(szSetting);
        if (li.List_GetIndex(&sResidentSettings, &Vtemp, &idx)) {
            if (!resident) {
                V = (DBCachedResidentSettingValue*)sResidentSettings.items[idx];
                li.List_Remove(&sResidentSettings, idx);
                HeapFree(hHeap, 0, V->name);
                HeapFree(hHeap, 0, V->module);
				HeapFree(hHeap, 0, V);
            }
            LeaveCriticalSection(&csSettingsDb);
            return 0;
        }
        else {
            size_t nameLen = strlen(szSetting)+1;
            
			V = (DBCachedResidentSettingValue*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(DBCachedResidentSettingValue));
			V->name = (char*)HeapAlloc(hHeap, 0, nameLen);
            mir_snprintf(V->name, nameLen, "%s", szSetting);
			V->nameHash = utils_hashString(szSetting);
            V->module = (char*)HeapAlloc(hHeap, 0, nameLen);
            mir_snprintf(V->module, nameLen, "%s", szSetting);
            V->module = strtok(V->module, "/");
            V->moduleHash = utils_hashString(V->module);
			li.List_Insert(&sResidentSettings, V, idx);
        }
        LeaveCriticalSection(&csSettingsDb);
    }
    return 0;
}

// Assume critical section
static int settings_isResident(char *szSetting) {
    DBCachedResidentSettingValue Vtemp;
    int idx;
    
    Vtemp.name = szSetting;
    Vtemp.nameHash = utils_hashString(szSetting);
    if (li.List_GetIndex(&sResidentSettings, &Vtemp, &idx)) {
        return 1;
    }
    return 0;
}

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
#ifndef DBRW_H
#define DBRW_H

#define DBRW_VER_MAJOR           1
#define DBRW_VER_MINOR           4
#define DBRW_VER_STRING          "1.4"
#define DBRW_VER_ALPHA           "1"
#define DBRW_SCHEMA_VERSION      "2"
#define DBRW_HEADER_STR          "SQLite format 3"
#define DBRW_ROT                 5
#define DBRW_SETTINGS_FLUSHCACHE 1000*3
#define DBRW_EVENTS_FLUSHCACHE   1000*60
#define DBRW_COMPACT_DAYS        7

#define MIRANDA_VER 0x0800

#include <m_stdhdr.h>

#include <windows.h>
#include <time.h>

#include "resource.h"

#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_langpack.h>

#include "sqlite3/sqlite3.h"

extern HINSTANCE g_hInst;
extern sqlite3 *g_sqlite;
extern struct LIST_INTERFACE li;
extern char g_szDbPath[MAX_PATH];

extern HANDLE hSettingChangeEvent;
extern HANDLE hContactDeletedEvent;
extern HANDLE hContactAddedEvent;
extern HANDLE hEventFilterAddedEvent;
extern HANDLE hEventAddedEvent;
extern HANDLE hEventDeletedEvent;

#define dbrw_alloc(n)     mir_alloc(n)
#define dbrw_free(p)      mir_free(p)
#define dbrw_realloc(p,s) mir_realloc(p, s)

#ifdef DBRW_LOGGING
#define log0(s)         utils_log_fmt(__FILE__,__LINE__,s)
#define log1(s,a)       utils_log_fmt(__FILE__,__LINE__,s,a)
#define log2(s,a,b)     utils_log_fmt(__FILE__,__LINE__,s,a,b)
#define log3(s,a,b,c)   utils_log_fmt(__FILE__,__LINE__,s,a,b,c)
#define log4(s,a,b,c,d) utils_log_fmt(__FILE__,__LINE__,s,a,b,c,d)
#else
#define log0(s)         
#define log1(s,a)       
#define log2(s,a,b)     
#define log3(s,a,b,c)   
#define log4(s,a,b,c,d) 
#endif

// Temp define to let dbRW compile on  Miranda < v0.7s
#ifndef MS_DB_SETSETTINGRESIDENT
#define MS_DB_SETSETTINGRESIDENT "DB/SetSettingResident"
#endif

// contacts.c
void contacts_init();
void contacts_destroy();
INT_PTR contacts_getCount(WPARAM wParam, LPARAM lParam);
INT_PTR contacts_findFirst(WPARAM wParam, LPARAM lParam);
INT_PTR contacts_findNext(WPARAM wParam, LPARAM lParam);
INT_PTR contacts_delete(WPARAM wParam, LPARAM lParam);
INT_PTR contacts_add(WPARAM wParam, LPARAM lParam);
INT_PTR contacts_isContact(WPARAM wParam, LPARAM lParam);

// events.c
void events_init();
void events_destroy();
INT_PTR events_getCount(WPARAM wParam, LPARAM lParam);
INT_PTR events_add(WPARAM wParam, LPARAM lParam);
INT_PTR events_delete(WPARAM wParam, LPARAM lParam);
INT_PTR events_getBlobSize(WPARAM wParam, LPARAM lParam);
INT_PTR events_get(WPARAM wParam, LPARAM lParam);
INT_PTR events_markRead(WPARAM wParam, LPARAM lParam);
INT_PTR events_getContact(WPARAM wParam, LPARAM lParam);
INT_PTR events_findFirst(WPARAM wParam, LPARAM lParam);
INT_PTR events_findFirstUnread(WPARAM wParam, LPARAM lParam);
INT_PTR events_findLast(WPARAM wParam, LPARAM lParam);
INT_PTR events_findNext(WPARAM wParam, LPARAM lParam);
INT_PTR events_findPrev(WPARAM wParam, LPARAM lParam);
void events_deleteContactData(HANDLE hContact);

// settings.c
void settings_init();
void settings_destroy();
INT_PTR setting_getSetting(WPARAM wParam, LPARAM lParam);
INT_PTR setting_getSettingStr(WPARAM wParam, LPARAM lParam);
INT_PTR setting_getSettingStatic(WPARAM wParam, LPARAM lParam);
INT_PTR setting_freeVariant(WPARAM wParam, LPARAM lParam);
INT_PTR setting_writeSetting(WPARAM wParam, LPARAM lParam);
INT_PTR setting_deleteSetting(WPARAM wParam, LPARAM lParam);
INT_PTR setting_enumSettings(WPARAM wParam, LPARAM lParam);
INT_PTR setting_modulesEnum(WPARAM wParam, LPARAM lParam);
void settings_deleteContactData(HANDLE hContact);
INT_PTR settings_setResident(WPARAM wParam, LPARAM lParam);

// sql.c
void sql_init();
void sql_destroy();
void sql_prepare_add(char **text, sqlite3_stmt **stmts, int len);
void sql_prepare_statements();
int sql_stmt_begin();
int sql_stmt_end();
int sql_step(sqlite3_stmt *stmt);
int sql_reset(sqlite3_stmt *stmt);
int sql_exec(sqlite3 *sql, const char *query);
int sql_open(const char *path, sqlite3 **sql);
int sql_close(sqlite3 *sql);
int sql_prepare(sqlite3 *sql, const char *query, sqlite3_stmt **stmt);
int sql_finalize(sqlite3_stmt *stmt);

// utils.c
#ifdef DBRW_LOGGING
void utils_log_init();
void utils_log_destroy();
void utils_log_fmt(const char *file,int line,const char *fmt,...);
#endif

INT_PTR utils_setSafetyMode(WPARAM wParam, LPARAM lParam);
INT_PTR utils_encodeString(WPARAM wParam,LPARAM lParam);
INT_PTR utils_decodeString(WPARAM wParam,LPARAM lParam);
DWORD utils_hashString(const char *szStr);
int utils_private_setting_get_int(const char *setting, int defval);
int utils_private_setting_set_int(const char *setting, int val);
void utils_vacuum_check();

#endif

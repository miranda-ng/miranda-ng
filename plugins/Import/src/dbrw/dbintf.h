/*

Import plugin for Miranda NG

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)

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

#pragma once

enum SQL_CTC_STMT
{
	SQL_CTC_STMT_COUNT = 0,
	SQL_CTC_STMT_EXISTS,
	SQL_CTC_STMT_NUM
};

static char *ctc_stmts[SQL_CTC_STMT_NUM] =
{
	"SELECT count(*) FROM dbrw_contacts;",
	"SELECT id FROM dbrw_contacts WHERE id = ? LIMIT 1;"
};

enum SQL_EVT_STMT
{
	SQL_EVT_STMT_COUNT = 0,
	SQL_EVT_STMT_BLOBSIZE,
	SQL_EVT_STMT_GET,
	SQL_EVT_STMT_GETFLAGS,
	SQL_EVT_STMT_GETCONTACT,
	SQL_EVT_STMT_FINDFIRST,
	SQL_EVT_STMT_FINDFIRSTUNREAD,
	SQL_EVT_STMT_FINDLAST,
	SQL_EVT_STMT_FINDNEXT,
	SQL_EVT_STMT_FINDPREV,
	SQL_EVT_STMT_NUM
};

static char *evt_stmts[SQL_EVT_STMT_NUM] =
{
	"SELECT count(*) FROM dbrw_events where contactid = ?;",
	"SELECT blobsize FROM dbrw_events where id = ? LIMIT 1;",
	"SELECT * FROM dbrw_events where id = ? LIMIT 1;",
	"SELECT flags FROM dbrw_events where id = ? LIMIT 1;",
	"SELECT contactid FROM dbrw_events where id = ? LIMIT 1;",
	"SELECT id FROM dbrw_events where contactid = ? ORDER by id LIMIT 1;",
	"SELECT flags, id FROM dbrw_events where contactid = ? ORDER by id;",
	"SELECT id FROM dbrw_events where contactid = ? ORDER by id DESC;",
	"SELECT id FROM dbrw_events where contactid = ? AND id > ? ORDER by id LIMIT 1;",
	"SELECT id FROM dbrw_events where contactid = ? AND id < ? ORDER by id DESC LIMIT 1;"
};

enum SQL_SET_STMT
{
	SQL_SET_STMT_READ = 0,
	SQL_SET_STMT_ENUM,
	SQL_SET_STMT_ENUMMODULES,
	SQL_SET_STMT_SETTINGCHECK,
	SQL_SET_STMT_NUM
};

static char *set_stmts[SQL_SET_STMT_NUM] =
{
	"SELECT type, val FROM dbrw_settings WHERE setting = ? AND module = ? AND id = ? LIMIT 1;",
	"SELECT setting from dbrw_settings where id = ? AND module = ? ORDER by setting;",
	"SELECT DISTINCT module from dbrw_settings;",
	"SELECT count(*) FROM dbrw_settings WHERE setting = ? AND module = ? AND id = ?;",
};

struct TSqlMessage {
	int op;
	sqlite3 *pDb;
	sqlite3_stmt *pStmt;
	int retCode;
	const char *zIn;
	HANDLE hDoneEvent;
};

struct CDbxSQLite : public MDatabaseReadonly, public MZeroedObject
{
private:
	sqlite3 *m_sqlite;
	
	int sql_prepare_len;
	char **sql_prepare_text;
	sqlite3_stmt ***sql_prepare_stmt;
	sqlite3_stmt *ctc_stmts_prep[SQL_CTC_STMT_NUM] = { 0 };
	sqlite3_stmt *evt_stmts_prep[SQL_EVT_STMT_NUM] = { 0 };
	sqlite3_stmt *set_stmts_prep[SQL_SET_STMT_NUM] = { 0 };

	void sql_prepare_add(char **text, sqlite3_stmt **stmts, int len);
	void sql_prepare_statements();
	static void CALLBACK sql_server_sync_apc(UINT_PTR dwParam);
	void sql_server_sync(TSqlMessage *msg);
	int sql_step(sqlite3_stmt *stmt);
	int sql_reset(sqlite3_stmt *stmt);
	int sql_exec(sqlite3 *sql, const char *query);
	int sql_open(const char *path, sqlite3 **sql);
	int sql_close(sqlite3 *sql);
	int sql_prepare(sqlite3 *sql, const char *query, sqlite3_stmt **stmt);
	int sql_finalize(sqlite3_stmt *stmt);

	void FillContacts();

public:
	CDbxSQLite();
	~CDbxSQLite();

	int Open(const wchar_t *profile);

	STDMETHODIMP_(BOOL)     IsRelational(void) override { return FALSE; }
	STDMETHODIMP_(void)     SetCacheSafetyMode(BOOL) override;

	STDMETHODIMP_(int)      GetContactCount(void) override;
	STDMETHODIMP_(int)      DeleteContact(MCONTACT contactID) override;
	STDMETHODIMP_(MCONTACT) AddContact(void) override;
	STDMETHODIMP_(BOOL)     IsDbContact(MCONTACT contactID) override;
	STDMETHODIMP_(int)      GetContactSize(void) override;

	STDMETHODIMP_(int)      GetEventCount(MCONTACT contactID) override;
	STDMETHODIMP_(MEVENT)   AddEvent(MCONTACT contactID, const DBEVENTINFO *dbe) override;
	STDMETHODIMP_(BOOL)     DeleteEvent(MEVENT hDbEvent) override;
	STDMETHODIMP_(BOOL)     EditEvent(MCONTACT contactID, MEVENT hDbEvent, const DBEVENTINFO *dbe) override;
	STDMETHODIMP_(int)      GetBlobSize(MEVENT hDbEvent) override;
	STDMETHODIMP_(BOOL)     GetEvent(MEVENT hDbEvent, DBEVENTINFO *dbe) override;
	STDMETHODIMP_(BOOL)     MarkEventRead(MCONTACT contactID, MEVENT hDbEvent) override;
	STDMETHODIMP_(MCONTACT) GetEventContact(MEVENT hDbEvent) override;
	STDMETHODIMP_(MEVENT)   FindFirstEvent(MCONTACT contactID) override;
	STDMETHODIMP_(MEVENT)   FindFirstUnreadEvent(MCONTACT contactID) override;
	STDMETHODIMP_(MEVENT)   FindLastEvent(MCONTACT contactID) override;
	STDMETHODIMP_(MEVENT)   FindNextEvent(MCONTACT contactID, MEVENT hDbEvent) override;
	STDMETHODIMP_(MEVENT)   FindPrevEvent(MCONTACT contactID, MEVENT hDbEvent) override;

	STDMETHODIMP_(BOOL)     EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam) override;

	STDMETHODIMP_(BOOL)     GetContactSettingWorker(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic) override;
	STDMETHODIMP_(BOOL)     EnumContactSettings(MCONTACT hContact, DBSETTINGENUMPROC pfnEnumProc, const char *szModule, void *param) override;

	STDMETHODIMP_(BOOL)     MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub) override;
	STDMETHODIMP_(BOOL)     MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub) override;

	STDMETHODIMP_(MEVENT)   GetEventById(LPCSTR szModule, LPCSTR szId) override;

	STDMETHODIMP_(DATABASELINK *) GetDriver();
};

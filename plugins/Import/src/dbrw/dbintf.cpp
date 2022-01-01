/*

Import plugin for Miranda NG

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

#include "../stdafx.h"

CDbxSQLite::CDbxSQLite()
	: sql_prepare_len(0)
{
	sql_prepare_add(ctc_stmts, ctc_stmts_prep, SQL_CTC_STMT_NUM);
	sql_prepare_add(evt_stmts, evt_stmts_prep, SQL_EVT_STMT_NUM);
	sql_prepare_add(set_stmts, set_stmts_prep, SQL_SET_STMT_NUM);
}

CDbxSQLite::~CDbxSQLite()
{
	sql_close(m_sqlite);

	for (int i = 0; i < sql_prepare_len; i++)
		sql_finalize(*sql_prepare_stmt[i]);
	mir_free(sql_prepare_text);
	mir_free(sql_prepare_stmt);
}

int CDbxSQLite::Open(const wchar_t *profile)
{
	T2Utf path(profile);
	if (sql_open(path, &m_sqlite) != SQLITE_OK)
		return 1;

	//utils_vacuum_check();
	{
		sql_exec(m_sqlite, "BEGIN TRANSACTION;");
		sql_exec(m_sqlite, "PRAGMA locking_mode = EXCLUSIVE;");
		sql_exec(m_sqlite, "PRAGMA synchronous = NORMAL;");
		sql_exec(m_sqlite, "PRAGMA cache_size = 6000;");
		sql_exec(m_sqlite, "PRAGMA temp_store = MEMORY;");
		sql_exec(m_sqlite, "COMMIT;");
	}

	sql_prepare_statements();

	FillContacts();

	return 0;
}

STDMETHODIMP_(void) CDbxSQLite::SetCacheSafetyMode(BOOL safeMode)
{
	if (safeMode)
		sql_exec(m_sqlite, "PRAGMA synchronous = NORMAL;");
	else
		sql_exec(m_sqlite, "PRAGMA synchronous = OFF;");
}

STDMETHODIMP_(BOOL) CDbxSQLite::MetaMergeHistory(DBCachedContact*, DBCachedContact*)
{
	return FALSE;
}

STDMETHODIMP_(BOOL) CDbxSQLite::MetaSplitHistory(DBCachedContact*, DBCachedContact*)
{
	return FALSE;
}

STDMETHODIMP_(MEVENT) CDbxSQLite::GetEventById(LPCSTR, LPCSTR)
{
	return 0;
}

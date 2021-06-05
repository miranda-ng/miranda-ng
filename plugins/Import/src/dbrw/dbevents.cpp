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

#include "../stdafx.h"

STDMETHODIMP_(LONG) CDbxSQLite::GetEventCount(MCONTACT contactID)
{
	mir_cslock lock(m_csDbAccess);

	int res = 0;
	sqlite3_bind_int(evt_stmts_prep[SQL_EVT_STMT_COUNT], 1, contactID);
	if (sql_step(evt_stmts_prep[SQL_EVT_STMT_COUNT]) == SQLITE_ROW)
		res = sqlite3_column_int(evt_stmts_prep[SQL_EVT_STMT_COUNT], 0);
	sql_reset(evt_stmts_prep[SQL_EVT_STMT_COUNT]);
	return res;
}

STDMETHODIMP_(MEVENT) CDbxSQLite::AddEvent(MCONTACT, const DBEVENTINFO*)
{
	return 0;
}

STDMETHODIMP_(BOOL) CDbxSQLite::DeleteEvent(MEVENT)
{
	return FALSE;
}

BOOL CDbxSQLite::EditEvent(MCONTACT, MEVENT, const DBEVENTINFO*)
{
	return 1;
}

STDMETHODIMP_(LONG) CDbxSQLite::GetBlobSize(MEVENT hDbEvent)
{
	mir_cslock lock(m_csDbAccess);

	int res = -1;
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_BLOBSIZE];
	sqlite3_bind_int(stmt, 1, hDbEvent);
	if (sql_step(stmt) == SQLITE_ROW)
		res = sqlite3_column_int(stmt, 0);
	sql_reset(stmt);
	return res;
}

STDMETHODIMP_(BOOL) CDbxSQLite::GetEvent(MEVENT hDbEvent, DBEVENTINFO *dbei)
{
	if (dbei == nullptr)
		return 1;

	if (dbei->cbBlob > 0 && dbei->pBlob == nullptr) {
		dbei->cbBlob = 0;
		return 1;
	}

	mir_cslock lock(m_csDbAccess);

	int res = 1;
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_GET];
	sqlite3_bind_int(stmt, 1, hDbEvent);
	if (sql_step(stmt) == SQLITE_ROW) {
		const void *blob = sqlite3_column_blob(stmt, 4);

		dbei->timestamp = (DWORD)sqlite3_column_int(stmt, 1);
		dbei->flags = (DWORD)sqlite3_column_int(stmt, 2);
		dbei->eventType = (WORD)sqlite3_column_int(stmt, 3);
		dbei->szModule = mir_strdup((char*)sqlite3_column_text(stmt, 7));

		DWORD cbBlob = sqlite3_column_int(stmt, 5);
		size_t bytesToCopy = cbBlob;
		if (dbei->cbBlob == -1)
			dbei->pBlob = (PBYTE)mir_calloc(cbBlob + 2);
		else if (dbei->cbBlob < cbBlob)
			bytesToCopy = dbei->cbBlob;

		dbei->cbBlob = cbBlob;
		if (bytesToCopy && dbei->pBlob)
			memcpy(dbei->pBlob, blob, bytesToCopy);
		res = 0;
	}
	sql_reset(stmt);
	return res;
}

STDMETHODIMP_(BOOL) CDbxSQLite::MarkEventRead(MCONTACT, MEVENT)
{
	return FALSE;
}

STDMETHODIMP_(MCONTACT) CDbxSQLite::GetEventContact(MEVENT hDbEvent)
{
	mir_cslock lock(m_csDbAccess); 
	
	int res = INVALID_CONTACT_ID;
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_GETCONTACT];
	sqlite3_bind_int(stmt, 1, hDbEvent);
	if (sql_step(stmt) == SQLITE_ROW)
		res = sqlite3_column_int(stmt, 0);
	sql_reset(stmt);
	return res;
}

STDMETHODIMP_(MEVENT) CDbxSQLite::FindFirstEvent(MCONTACT contactID)
{
	mir_cslock lock(m_csDbAccess);

	int res = 0;
	sqlite3_bind_int(evt_stmts_prep[SQL_EVT_STMT_FINDFIRST], 1, contactID);
	if (sql_step(evt_stmts_prep[SQL_EVT_STMT_FINDFIRST]) == SQLITE_ROW)
		res = sqlite3_column_int(evt_stmts_prep[SQL_EVT_STMT_FINDFIRST], 0);
	sql_reset(evt_stmts_prep[SQL_EVT_STMT_FINDFIRST]);
	return res;
}

STDMETHODIMP_(MEVENT) CDbxSQLite::FindFirstUnreadEvent(MCONTACT contactID)
{
	mir_cslock lock(m_csDbAccess);

	int res = 0;
	DWORD flags = 0;
	sqlite3_bind_int(evt_stmts_prep[SQL_EVT_STMT_FINDFIRSTUNREAD], 1, contactID);
	while (sql_step(evt_stmts_prep[SQL_EVT_STMT_FINDFIRSTUNREAD]) == SQLITE_ROW) {
		flags = sqlite3_column_int(evt_stmts_prep[SQL_EVT_STMT_FINDFIRSTUNREAD], 0);
		if (!(flags & (DBEF_READ | DBEF_SENT))) {
			res = sqlite3_column_int(evt_stmts_prep[SQL_EVT_STMT_FINDFIRSTUNREAD], 1);
			sql_reset(evt_stmts_prep[SQL_EVT_STMT_FINDFIRSTUNREAD]);
			return res;
		}
	}
	sql_reset(evt_stmts_prep[SQL_EVT_STMT_FINDFIRSTUNREAD]);
	return res;
}

STDMETHODIMP_(MEVENT) CDbxSQLite::FindLastEvent(MCONTACT contactID)
{
	mir_cslock lock(m_csDbAccess);

	int res = 0;
	sqlite3_bind_int(evt_stmts_prep[SQL_EVT_STMT_FINDLAST], 1, contactID);
	if (sql_step(evt_stmts_prep[SQL_EVT_STMT_FINDLAST]) == SQLITE_ROW)
		res = sqlite3_column_int(evt_stmts_prep[SQL_EVT_STMT_FINDLAST], 0);
	sql_reset(evt_stmts_prep[SQL_EVT_STMT_FINDLAST]);
	return res;
}

STDMETHODIMP_(MEVENT) CDbxSQLite::FindNextEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	if (hDbEvent == NULL)
		return 0;

	mir_cslock lock(m_csDbAccess);

	int res = 0;
	sqlite3_bind_int(evt_stmts_prep[SQL_EVT_STMT_FINDNEXT], 1, contactID);
	sqlite3_bind_int(evt_stmts_prep[SQL_EVT_STMT_FINDNEXT], 2, hDbEvent);
	if (sql_step(evt_stmts_prep[SQL_EVT_STMT_FINDNEXT]) == SQLITE_ROW)
		res = sqlite3_column_int(evt_stmts_prep[SQL_EVT_STMT_FINDNEXT], 0);
	sql_reset(evt_stmts_prep[SQL_EVT_STMT_FINDNEXT]);
	return res;
}

STDMETHODIMP_(MEVENT) CDbxSQLite::FindPrevEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	if (!hDbEvent)
		return 0;

	mir_cslock lock(m_csDbAccess);

	int res = 0;
	sqlite3_bind_int(evt_stmts_prep[SQL_EVT_STMT_FINDPREV], 1, contactID);
	sqlite3_bind_int(evt_stmts_prep[SQL_EVT_STMT_FINDPREV], 2, hDbEvent);
	if (sql_step(evt_stmts_prep[SQL_EVT_STMT_FINDPREV]) == SQLITE_ROW)
		res = sqlite3_column_int(evt_stmts_prep[SQL_EVT_STMT_FINDPREV], 0);
	sql_reset(evt_stmts_prep[SQL_EVT_STMT_FINDPREV]);
	return res;
}

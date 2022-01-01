/*

Import plugin for Miranda NG

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

#include "..\stdafx.h"

enum SQL
{
	SQL_STEP,
	SQL_RESET,
	SQL_EXEC,
	SQL_OPEN,
	SQL_CLOSE,
	SQL_PREPARE,
	SQL_FINALIZE
};

void CDbxSQLite::sql_prepare_add(char **text, sqlite3_stmt **stmts, int len)
{
	sql_prepare_text = (char**)mir_realloc(sql_prepare_text, (sql_prepare_len + len) * sizeof(char*));
	sql_prepare_stmt = (sqlite3_stmt***)mir_realloc(sql_prepare_stmt, (sql_prepare_len + len) * sizeof(sqlite3_stmt**));
	for (int i = 0; i < len; i++) {
		sql_prepare_text[sql_prepare_len + i] = text[i];
		sql_prepare_stmt[sql_prepare_len + i] = &stmts[i];
	}
	sql_prepare_len += len;
}

void CDbxSQLite::sql_prepare_statements()
{
	for (int i = 0; i < sql_prepare_len; i++)
		sql_prepare(m_sqlite, sql_prepare_text[i], sql_prepare_stmt[i]);
}

void CALLBACK CDbxSQLite::sql_server_sync_apc(UINT_PTR dwParam)
{
	TSqlMessage *msg = (TSqlMessage*)dwParam;

	if (!msg)
		return;
	
	switch (msg->op) {
	case SQL_STEP:
		msg->retCode = sqlite3_step(msg->pStmt);
		break;
	case SQL_RESET:
		msg->retCode = sqlite3_reset(msg->pStmt);
		break;
	case SQL_EXEC:
		msg->retCode = sqlite3_exec(msg->pDb, msg->zIn, NULL, NULL, NULL);
		break;
	case SQL_OPEN:
		msg->retCode = sqlite3_open(msg->zIn, &msg->pDb);
		break;
	case SQL_CLOSE:
		msg->retCode = sqlite3_close(msg->pDb);
		break;
	case SQL_PREPARE:
		msg->retCode = sqlite3_prepare_v2(msg->pDb, msg->zIn, -1, &msg->pStmt, NULL);
		break;
	case SQL_FINALIZE:
		msg->retCode = sqlite3_finalize(msg->pStmt);
		break;
	default:
		return;
	}
	SetEvent(msg->hDoneEvent);
}

void CDbxSQLite::sql_server_sync(TSqlMessage *msg)
{
	msg->hDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	sql_server_sync_apc((UINT_PTR)msg);
	PostMessage(Miranda_GetSystemWindow()->GetHwnd(), WM_NULL, 0, 0);
	WaitForSingleObject(msg->hDoneEvent, INFINITE);
	CloseHandle(msg->hDoneEvent);
}

int CDbxSQLite::sql_step(sqlite3_stmt *stmt)
{
	TSqlMessage msg;
	msg.op = SQL_STEP;
	msg.pStmt = stmt;
	sql_server_sync(&msg);
	return msg.retCode;
}

int CDbxSQLite::sql_reset(sqlite3_stmt *stmt)
{
	TSqlMessage msg;
	msg.op = SQL_RESET;
	msg.pStmt = stmt;
	sql_server_sync(&msg);
	return msg.retCode;
}

int CDbxSQLite::sql_exec(sqlite3 *sql, const char *query)
{
	TSqlMessage msg;
	msg.op = SQL_EXEC;
	msg.pDb = sql;
	msg.zIn = query;
	sql_server_sync(&msg);
	return msg.retCode;
}

int CDbxSQLite::sql_open(const char *path, sqlite3 **sql)
{
	TSqlMessage msg;
	msg.op = SQL_OPEN;
	msg.zIn = path;
	sql_server_sync(&msg);
	*sql = msg.pDb;
	return msg.retCode;
}

int CDbxSQLite::sql_close(sqlite3 *sql)
{
	TSqlMessage msg;
	msg.op = SQL_CLOSE;
	msg.pDb = sql;
	sql_server_sync(&msg);
	return msg.retCode;
}

int CDbxSQLite::sql_prepare(sqlite3 *sql, const char *query, sqlite3_stmt **stmt)
{
	TSqlMessage msg;
	msg.op = SQL_PREPARE;
	msg.pDb = sql;
	msg.zIn = query;
	sql_server_sync(&msg);
	*stmt = msg.pStmt;
	return msg.retCode;
}

int CDbxSQLite::sql_finalize(sqlite3_stmt *stmt)
{
	TSqlMessage msg;
	msg.op = SQL_FINALIZE;
	msg.pStmt = stmt;
	sql_server_sync(&msg);
	return msg.retCode;
}

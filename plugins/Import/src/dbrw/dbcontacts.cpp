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

void CDbxSQLite::FillContacts()
{
	sqlite3_stmt *st = nullptr;
	if (sql_prepare(m_sqlite, "SELECT c.id, s.val FROM dbrw_contacts as c INNER JOIN dbrw_settings as s on s.id = c.id WHERE s.module = 'Protocol' and s.setting = 'p';", &st) != SQLITE_OK)
		return;

	while (sql_step(st) == SQLITE_ROW) {
		MCONTACT contactID = sqlite3_column_int(st, 0);
		const char *proto = (const char*)sqlite3_column_text(st, 1);
		DBCachedContact *cc = m_cache->AddContactToCache(contactID);
		cc->szProto = mir_strdup(proto);
	}
	sql_finalize(st);
}

STDMETHODIMP_(BOOL) CDbxSQLite::IsDbContact(MCONTACT contactID)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	return (cc != nullptr);
}

STDMETHODIMP_(int) CDbxSQLite::GetContactCount(void)
{
	int res = 0;
	if (sql_step(ctc_stmts_prep[SQL_CTC_STMT_COUNT]) == SQLITE_ROW)
		res = sqlite3_column_int(ctc_stmts_prep[SQL_CTC_STMT_COUNT], 0);
	sql_reset(ctc_stmts_prep[SQL_CTC_STMT_COUNT]);
	return res;
}

STDMETHODIMP_(int) CDbxSQLite::GetContactSize(void)
{
	return sizeof(DBCachedContact);
}

STDMETHODIMP_(MCONTACT) CDbxSQLite::AddContact(void)
{
	return INVALID_CONTACT_ID;
}

STDMETHODIMP_(int) CDbxSQLite::DeleteContact(MCONTACT)
{
	return 1;
}

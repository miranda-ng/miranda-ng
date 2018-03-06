/*

Import plugin for Miranda NG

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org)

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

STDMETHODIMP_(BOOL) CDbxSQLite::IsDbContact(MCONTACT contactID)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	return (cc != nullptr);
}

STDMETHODIMP_(LONG) CDbxSQLite::GetContactCount(void)
{
	int res = 0;
	sqlite3_bind_int(ctc_stmts_prep[SQL_CTC_STMT_COUNT], 1, res);
	if (sql_step(ctc_stmts_prep[SQL_CTC_STMT_COUNT]) == SQLITE_ROW)
		res = sqlite3_column_int(ctc_stmts_prep[SQL_CTC_STMT_COUNT], 0);
	sql_reset(ctc_stmts_prep[SQL_CTC_STMT_COUNT]);
	return res;
}

STDMETHODIMP_(LONG) CDbxSQLite::GetContactSize(void)
{
	return sizeof(DBCachedContact);
}

STDMETHODIMP_(MCONTACT) CDbxSQLite::AddContact(void)
{
	return INVALID_CONTACT_ID;
}

STDMETHODIMP_(LONG) CDbxSQLite::DeleteContact(MCONTACT)
{
	return 1;
}
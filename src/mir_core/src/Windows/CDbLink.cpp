/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (C) 2012-21 Miranda NG team

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

/////////////////////////////////////////////////////////////////////////////////////////
// CDbLink class

CDbLink::CDbLink(const char *szModule, const char *szSetting, BYTE type, DWORD iValue)
	: CDataLink(type)
{
	m_szModule = mir_strdup(szModule);
	m_szSetting = mir_strdup(szSetting);
	m_iDefault = iValue;
	m_szDefault = nullptr;
	dbv.type = DBVT_DELETED;
}

CDbLink::CDbLink(const char *szModule, const char *szSetting, BYTE type, wchar_t *szValue)
	: CDataLink(type),
	m_iDefault(0)
{
	m_szModule = mir_strdup(szModule);
	m_szSetting = mir_strdup(szSetting);
	m_szDefault = mir_wstrdup(szValue);
	dbv.type = DBVT_DELETED;
}

CDbLink::~CDbLink()
{
	mir_free(m_szModule);
	mir_free(m_szSetting);
	mir_free(m_szDefault);
	if (dbv.type != DBVT_DELETED)
		db_free(&dbv);
}

DWORD CDbLink::LoadInt()
{
	switch (m_type) {
		case DBVT_BYTE:  return db_get_b(0, m_szModule, m_szSetting, m_iDefault);
		case DBVT_WORD:  return db_get_w(0, m_szModule, m_szSetting, m_iDefault);
		case DBVT_DWORD: return db_get_dw(0, m_szModule, m_szSetting, m_iDefault);
		default:         return m_iDefault;
	}
}

void CDbLink::SaveInt(DWORD value)
{
	switch (m_type) {
		case DBVT_BYTE:  db_set_b(0, m_szModule, m_szSetting, (BYTE)value); break;
		case DBVT_WORD:  db_set_w(0, m_szModule, m_szSetting, (WORD)value); break;
		case DBVT_DWORD: db_set_dw(0, m_szModule, m_szSetting, value); break;
	}
}

wchar_t* CDbLink::LoadText()
{
	if (dbv.type != DBVT_DELETED) db_free(&dbv);
	if (!db_get_ws(0, m_szModule, m_szSetting, &dbv)) {
		if (dbv.type == DBVT_WCHAR)
			return dbv.pwszVal;
		return m_szDefault;
	}

	dbv.type = DBVT_DELETED;
	return m_szDefault;
}

void CDbLink::SaveText(wchar_t *value)
{
	db_set_ws(0, m_szModule, m_szSetting, value);
}

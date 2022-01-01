/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "stdafx.h"

#define CMP_UINT(x, y) { if ((x) != (y)) return (x) < (y) ? -1 : 1; }

int DBEventIdKey::Compare(const MDBX_val *ax, const MDBX_val *bx) MDBX_CXX17_NOEXCEPT
{
	const DBEventIdKey *a = (DBEventIdKey*)ax->iov_base;
	const DBEventIdKey *b = (DBEventIdKey*)bx->iov_base;
	CMP_UINT(a->iModuleId, b->iModuleId);
	return strcmp(a->szEventId, b->szEventId);
}

int DBEventSortingKey::Compare(const MDBX_val *ax, const MDBX_val *bx) MDBX_CXX17_NOEXCEPT
{
	const DBEventSortingKey *a = (DBEventSortingKey*)ax->iov_base;
	const DBEventSortingKey *b = (DBEventSortingKey*)bx->iov_base;

	CMP_UINT(a->hContact, b->hContact);
	CMP_UINT(a->ts, b->ts);
	CMP_UINT(a->hEvent, b->hEvent);
	return 0;
}

int DBSettingKey::Compare(const MDBX_val *ax, const MDBX_val *bx) MDBX_CXX17_NOEXCEPT
{
	const DBSettingKey *a = (DBSettingKey*)ax->iov_base;
	const DBSettingKey *b = (DBSettingKey*)bx->iov_base;

	CMP_UINT(a->hContact, b->hContact);
	CMP_UINT(a->dwModuleId, b->dwModuleId);
	return strcmp(a->szSettingName, b->szSettingName);
}

/////////////////////////////////////////////////////////////////////////////////////////
// txn_ptr class 

txn_ptr::txn_ptr(CDbxMDBX *_db) :
	pDb(_db),
	txn(_db->StartTran())
{
	_db->m_csDbAccess.Lock();
}

txn_ptr::~txn_ptr()
{
	pDb->m_csDbAccess.Unlock();
}

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

/////////////////////////////////////////////////////////////////////////////////////////
// we are tracing EventsSort table to verify that they have correct event ids

int CDbxMDBX::CheckEvents1(void)
{
	txn_ptr trnlck(this);
	cursor_ptr cursor(trnlck, m_dbEventsSort);

	MDBX_val key, data;
	for (int ret = mdbx_cursor_get(cursor, &key, &data, MDBX_FIRST); ret == MDBX_SUCCESS; ret = mdbx_cursor_get(cursor, &key, &data, MDBX_NEXT)) {
		auto *pData = (DBEventSortingKey *)key.iov_base;

		// if that's not a member of system event, check contact's existence first
		if (pData->hContact != 0) {
			auto *cc = m_cache->GetCachedContact(pData->hContact);
			if (cc == nullptr) {
				mdbx_cursor_del(cursor, MDBX_UPSERT);
				cb->pfnAddLogMessage(STATUS_ERROR, CMStringW(FORMAT, TranslateT("Orphaned sorting event with wrong contact ID %d, deleting"), pData->hContact));
				continue;
			}
		}

		if (GetBlobSize(pData->hEvent) == -1) {
			mdbx_cursor_del(cursor, MDBX_UPSERT);
			cb->pfnAddLogMessage(STATUS_ERROR, CMStringW(FORMAT, TranslateT("Orphaned sorting event with wrong event ID %d:%08X, deleting"), pData->hContact, pData->hEvent));
			continue;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// we are tracing EventId table to verify that they have correct event ids

int CDbxMDBX::CheckEvents2(void)
{
	txn_ptr trnlck(this);
	cursor_ptr cursor(trnlck, m_dbEventIds);

	MDBX_val key, data;
	for (int ret = mdbx_cursor_get(cursor, &key, &data, MDBX_FIRST); ret == MDBX_SUCCESS; ret = mdbx_cursor_get(cursor, &key, &data, MDBX_NEXT)) {
		MEVENT hDbEvent = *(MEVENT *)data.iov_base;
		if (GetBlobSize(hDbEvent) == -1) {
			mdbx_cursor_del(cursor, MDBX_UPSERT);
			cb->pfnAddLogMessage(STATUS_ERROR, CMStringW(FORMAT, TranslateT("Orphaned event id with wrong event ID %08X, deleting"), hDbEvent));
			continue;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// we are tracing Settings table to verify that they have correct contact ids

int CDbxMDBX::CheckEvents3(void)
{
	txn_ptr trnlck(this);
	cursor_ptr cursor(trnlck, m_dbSettings);

	MDBX_val key, data;
	for (int ret = mdbx_cursor_get(cursor, &key, &data, MDBX_FIRST); ret == MDBX_SUCCESS; ret = mdbx_cursor_get(cursor, &key, &data, MDBX_NEXT)) {
		auto *pKey = (DBSettingKey *)key.iov_base;

		if (pKey->hContact) {
			auto *cc = m_cache->GetCachedContact(pKey->hContact);
			if (cc == nullptr) {
				mdbx_cursor_del(cursor, MDBX_UPSERT);
				cb->pfnAddLogMessage(STATUS_ERROR, CMStringW(FORMAT, TranslateT("Orphaned setting with wrong contact ID %08X, deleting"), pKey->hContact));
				continue;
			}
		}
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// MIDatabaseChecker

int CDbxMDBX::CheckDb(int phase)
{
	switch (phase) {
	case 0: return CheckEvents1();
	case 1: return CheckEvents2();
	case 2: return CheckEvents3();
	}

	return ERROR_OUT_OF_PAPER;
}

/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org)
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

CDbxMDBX::CDbxMDBX(const TCHAR *tszFileName, int iMode) :
	m_safetyMode(true),
	m_bReadOnly((iMode & DBMODE_READONLY) != 0),
	m_bShared((iMode & DBMODE_SHARED) != 0),
	m_maxContactId(0)
{
	m_tszProfileName = mir_wstrdup(tszFileName);
	InitDbInstance(this);

	mdbx_env_create(&m_env);
	mdbx_env_set_maxdbs(m_env, 10);
	mdbx_env_set_userctx(m_env, this);
}

CDbxMDBX::~CDbxMDBX()
{
	mdbx_env_close(m_env);

	DestroyServiceFunction(hService);
	UnhookEvent(hHook);

	if (m_crypto)
		m_crypto->destroy();

	DestroyHookableEvent(hContactDeletedEvent);
	DestroyHookableEvent(hContactAddedEvent);
	DestroyHookableEvent(hSettingChangeEvent);
	DestroyHookableEvent(hEventMarkedRead);

	DestroyHookableEvent(hEventAddedEvent);
	DestroyHookableEvent(hEventDeletedEvent);
	DestroyHookableEvent(hEventFilterAddedEvent);

	DestroyDbInstance(this);
	mir_free(m_tszProfileName);
}

int CDbxMDBX::Load(bool bSkipInit)
{
	if (Map() != MDBX_SUCCESS)
		return EGROKPRF_CANTREAD;

	if (!bSkipInit) {
		txn_ptr trnlck(m_env);

		unsigned int defFlags = MDBX_CREATE;

		mdbx_dbi_open(trnlck, "global", defFlags | MDBX_INTEGERKEY, &m_dbGlobal);
		mdbx_dbi_open(trnlck, "crypto", defFlags, &m_dbCrypto);
		mdbx_dbi_open(trnlck, "contacts", defFlags | MDBX_INTEGERKEY, &m_dbContacts);
		mdbx_dbi_open(trnlck, "modules", defFlags | MDBX_INTEGERKEY, &m_dbModules);
		mdbx_dbi_open(trnlck, "events", defFlags | MDBX_INTEGERKEY, &m_dbEvents);

		mdbx_dbi_open_ex(trnlck, "eventsrt", defFlags, &m_dbEventsSort, DBEventSortingKey::Compare, nullptr);
		mdbx_dbi_open_ex(trnlck, "settings", defFlags, &m_dbSettings, DBSettingKey::Compare, nullptr);
		{
			uint32_t keyVal = 1;
			MDBX_val key = { &keyVal, sizeof(keyVal) }, data;
			if (mdbx_get(trnlck, m_dbGlobal, &key, &data) == MDBX_SUCCESS) {
				const DBHeader *hdr = (const DBHeader*)data.iov_base;
				if (hdr->dwSignature != DBHEADER_SIGNATURE)
					return EGROKPRF_DAMAGED;
				if (hdr->dwVersion != DBHEADER_VERSION)
					return EGROKPRF_OBSOLETE;

				m_header = *hdr;
			}
			else {
				m_header.dwSignature = DBHEADER_SIGNATURE;
				m_header.dwVersion = DBHEADER_VERSION;
				data.iov_base = &m_header; data.iov_len = sizeof(m_header);
				mdbx_put(trnlck, m_dbGlobal, &key, &data, 0);
			}
			trnlck.commit();
		}
		{
			MDBX_val key, val;

			mdbx_txn_begin(m_env, nullptr, MDBX_RDONLY, &m_txn);

			mdbx_cursor_open(m_txn, m_dbEvents, &m_curEvents);
			if (mdbx_cursor_get(m_curEvents, &key, &val, MDBX_LAST) == MDBX_SUCCESS)
				m_dwMaxEventId = *(MEVENT*)key.iov_base;

			mdbx_cursor_open(m_txn, m_dbEventsSort, &m_curEventsSort);
			mdbx_cursor_open(m_txn, m_dbSettings, &m_curSettings);
			mdbx_cursor_open(m_txn, m_dbModules, &m_curModules);

			mdbx_cursor_open(m_txn, m_dbContacts, &m_curContacts);
			if (mdbx_cursor_get(m_curContacts, &key, &val, MDBX_LAST) == MDBX_SUCCESS)
				m_maxContactId = *(MCONTACT*)key.iov_base;

			MDBX_stat st;
			mdbx_dbi_stat(m_txn, m_dbContacts, &st, sizeof(st));
			m_contactCount = st.ms_entries;

			mdbx_txn_reset(m_txn);
		}


		if (InitModules()) return EGROKPRF_DAMAGED;
		if (InitCrypt())   return EGROKPRF_DAMAGED;

		// everything is ok, go on
		if (!m_bReadOnly) {
			// retrieve the event handles
			hContactDeletedEvent = CreateHookableEvent(ME_DB_CONTACT_DELETED);
			hContactAddedEvent = CreateHookableEvent(ME_DB_CONTACT_ADDED);
			hSettingChangeEvent = CreateHookableEvent(ME_DB_CONTACT_SETTINGCHANGED);
			hEventMarkedRead = CreateHookableEvent(ME_DB_EVENT_MARKED_READ);

			hEventAddedEvent = CreateHookableEvent(ME_DB_EVENT_ADDED);
			hEventDeletedEvent = CreateHookableEvent(ME_DB_EVENT_DELETED);
			hEventFilterAddedEvent = CreateHookableEvent(ME_DB_EVENT_FILTER_ADD);
		}

		FillContacts();
	}

	return EGROKPRF_NOERROR;
}

int CDbxMDBX::Create(void)
{
	return (Map() == MDBX_SUCCESS) ? 0 : EGROKPRF_CANTREAD;
}

size_t iDefHeaderOffset = 0;
BYTE bDefHeader[] = { 0 };

int CDbxMDBX::Check(void)
{
	FILE *pFile = _wfopen(m_tszProfileName, L"rb");
	if (pFile == nullptr)
		return EGROKPRF_CANTREAD;

	fseek(pFile, (LONG)iDefHeaderOffset, SEEK_SET);
	BYTE buf[_countof(bDefHeader)];
	size_t cbRead = fread(buf, 1, _countof(buf), pFile);
	fclose(pFile);
	if (cbRead != _countof(buf))
		return EGROKPRF_DAMAGED;

	return (memcmp(buf, bDefHeader, _countof(bDefHeader))) ? EGROKPRF_UNKHEADER : 0;
}

int CDbxMDBX::PrepareCheck()
{
	InitModules();
	return InitCrypt();
}

STDMETHODIMP_(void) CDbxMDBX::SetCacheSafetyMode(BOOL bIsSet)
{
	m_safetyMode = bIsSet != 0;
}

int CDbxMDBX::Map()
{
	int rc = mdbx_env_set_geometry(m_env,
		-1 /* minimal lower limit */,
		64ul << 10 /* atleast 64K for now */,
		256ul << 20 /* 256M upper limit */,
		256ul << 10 /* 256K growth step */,
		512ul << 10 /* 512K shrink threshold */,
		-1 /* default page size */);
	if (rc != MDBX_SUCCESS)
		return rc;
	unsigned int mode = MDBX_NOSUBDIR | MDBX_MAPASYNC | MDBX_WRITEMAP | MDBX_NOSYNC;
	if (m_bReadOnly)
		mode |= MDBX_RDONLY;
	return mdbx_env_open(m_env, _T2A(m_tszProfileName), mode, 0664);
}

///////////////////////////////////////////////////////////////////////////////
// MIDatabaseChecker

typedef int (CDbxMDBX::*CheckWorker)(int);

int CDbxMDBX::Start(DBCHeckCallback *callback)
{
	cb = callback;
	return ERROR_SUCCESS;
}

int CDbxMDBX::CheckDb(int, int)
{
	return ERROR_OUT_OF_PAPER;
}

void CDbxMDBX::Destroy()
{
	delete this;
}

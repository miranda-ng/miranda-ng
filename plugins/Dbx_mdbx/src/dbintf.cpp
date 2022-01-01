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
// constructor & destructor

CDbxMDBX::CDbxMDBX(const wchar_t *tszFileName, int iMode) :
	m_bReadOnly((iMode & DBMODE_READONLY) != 0),
	m_pwszProfileName(mir_wstrdup(tszFileName)),
	m_impl(*this)
{
	m_ccDummy.nSubs = -1;
}

CDbxMDBX::~CDbxMDBX()
{
	if (m_pWriteTran)
		mdbx_txn_commit(m_pWriteTran);

	if (m_curEventsSort)
		mdbx_cursor_close(m_curEventsSort);

	mdbx_env_close(m_env);

	if (!m_bReadOnly)
		TouchFile();

	for (auto &it : hService)
		DestroyServiceFunction(it);
	UnhookEvent(hHook);
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDbxMDBX::Backup(const wchar_t *pwszPath)
{
	HANDLE pFile = ::CreateFile(pwszPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (pFile == nullptr) {
		Netlib_Logf(0, "Backup file <%S> cannot be created", pwszPath);
		return 1;
	}

	mir_cslock lck(m_csDbAccess);

	if (m_pWriteTran) {
		mdbx_txn_commit(m_pWriteTran);
		m_pWriteTran = nullptr;
	}

	int res = mdbx_env_copy2fd(m_env, pFile, MDBX_CP_COMPACT);
	if (res != MDBX_SUCCESS) {
		Netlib_Logf(0, "CDbxMDBX::Backup: mdbx_env_copy2fd failed with error code %d", res);
LBL_Fail:
		CloseHandle(pFile);
		DeleteFileW(pwszPath);
		return res;
	}

	res = FlushFileBuffers(pFile);
	if (res == 0) {
		Netlib_Logf(0, "CDbxMDBX::Backup: FlushFileBuffers failed with error code %d", GetLastError());
		goto LBL_Fail;
	}

	CloseHandle(pFile);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

size_t iDefHeaderOffset = 0;
uint8_t bDefHeader[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

int CDbxMDBX::Check(void)
{
	FILE *pFile = _wfopen(m_pwszProfileName, L"rb");
	if (pFile == nullptr)
		return EGROKPRF_CANTREAD;

	fseek(pFile, (LONG)iDefHeaderOffset, SEEK_SET);
	uint8_t buf[_countof(bDefHeader)];
	size_t cbRead = fread(buf, 1, _countof(buf), pFile);
	fclose(pFile);
	if (cbRead != _countof(buf))
		return EGROKPRF_DAMAGED;

	return (memcmp(buf, bDefHeader, _countof(bDefHeader))) ? EGROKPRF_UNKHEADER : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDbxMDBX::Compact()
{
	CMStringW wszTmpFile(FORMAT, L"%s.tmp", m_pwszProfileName.get());

	mir_cslock lck(m_csDbAccess);
	int res = Backup(wszTmpFile);
	if (res)
		return res;

	mdbx_env_close(m_env);

	DeleteFileW(m_pwszProfileName);
	MoveFileW(wszTmpFile, m_pwszProfileName);

	Map();
	Load();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDbxMDBX::DBFlush(bool bForce)
{
	if (bForce) {
		mir_cslock lck(m_csDbAccess);

		if (m_pWriteTran) {
			mdbx_txn_commit(m_pWriteTran);
			mdbx_env_sync(m_env);

			m_pWriteTran = nullptr;
			m_dbError = mdbx_txn_begin(m_env, nullptr, MDBX_TXN_READWRITE, &m_pWriteTran);
			// FIXME: throw an exception
			_ASSERT(m_dbError == MDBX_SUCCESS);
		}
	}
	else if (m_safetyMode)
		m_impl.m_timer.Start(50);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDbxMDBX::Load()
{
	MDBX_db_flags_t defFlags = MDBX_CREATE;

	m_pWriteTran = nullptr;
	m_dbError = mdbx_txn_begin(m_env, nullptr, (m_bReadOnly) ? MDBX_TXN_RDONLY : MDBX_TXN_READWRITE, &m_pWriteTran);
	if (m_pWriteTran == nullptr) {
		if (m_dbError == MDBX_TXN_FULL) {
			if (IDOK == MessageBox(NULL, TranslateT("Your database is in the obsolete format. Click OK to read the upgrade instructions or Cancel to exit"), TranslateT("Error"), MB_ICONERROR | MB_OKCANCEL))
				Utils_OpenUrl("https://www.miranda-ng.org/news/unknown-profile-format");
			return EGROKPRF_OBSOLETE;
		}
		return EGROKPRF_DAMAGED;
	}

	mdbx_dbi_open(m_pWriteTran, "global", defFlags | MDBX_INTEGERKEY, &m_dbGlobal);
	mdbx_dbi_open(m_pWriteTran, "crypto", defFlags, &m_dbCrypto);
	mdbx_dbi_open(m_pWriteTran, "contacts", defFlags | MDBX_INTEGERKEY, &m_dbContacts);
	mdbx_dbi_open(m_pWriteTran, "modules", defFlags | MDBX_INTEGERKEY, &m_dbModules);
	mdbx_dbi_open(m_pWriteTran, "events", defFlags | MDBX_INTEGERKEY, &m_dbEvents);

	mdbx_dbi_open_ex(m_pWriteTran, "eventids", defFlags, &m_dbEventIds, DBEventIdKey::Compare, nullptr);
	mdbx_dbi_open_ex(m_pWriteTran, "eventsrt", defFlags, &m_dbEventsSort, DBEventSortingKey::Compare, nullptr);
	mdbx_dbi_open_ex(m_pWriteTran, "settings", defFlags, &m_dbSettings, DBSettingKey::Compare, nullptr);

	uint32_t keyVal = 1;
	{
		MDBX_val key = { &keyVal, sizeof(keyVal) }, data;
		if (mdbx_get(m_pWriteTran, m_dbGlobal, &key, &data) == MDBX_SUCCESS) {
			const DBHeader *hdr = (const DBHeader *)data.iov_base;
			if (hdr->dwSignature != DBHEADER_SIGNATURE)
				return EGROKPRF_DAMAGED;
			if (hdr->dwVersion != DBHEADER_VERSION)
				return EGROKPRF_OBSOLETE;

			m_header = *hdr;
		} else {
			m_header.dwSignature = DBHEADER_SIGNATURE;
			m_header.dwVersion = DBHEADER_VERSION;
			data.iov_base = &m_header; data.iov_len = sizeof(m_header);
			mdbx_put(m_pWriteTran, m_dbGlobal, &key, &data, MDBX_UPSERT);
			DBFlush();
		}

		keyVal = 2;
		if (mdbx_get(m_pWriteTran, m_dbGlobal, &key, &data) == MDBX_SUCCESS)
			m_ccDummy.dbc = *(const DBContact *)data.iov_base;
	}

	m_curEventsSort = mdbx_cursor_create(nullptr);

	MDBX_val key, val;
	{
		cursor_ptr pCursor(m_pWriteTran, m_dbEvents);
		if (mdbx_cursor_get(pCursor, &key, &val, MDBX_LAST) == MDBX_SUCCESS)
			m_dwMaxEventId = *(MEVENT *)key.iov_base;
	}
	{
		cursor_ptr pCursor(m_pWriteTran, m_dbContacts);
		if (mdbx_cursor_get(pCursor, &key, &val, MDBX_LAST) == MDBX_SUCCESS)
			m_maxContactId = *(MCONTACT *)key.iov_base;
	}

	mdbx_txn_commit(m_pWriteTran); m_pWriteTran = nullptr;

	if (InitModules()) return EGROKPRF_DAMAGED;
	if (InitCrypt())   return EGROKPRF_DAMAGED;

	FillContacts();
	FillSettings();
	return EGROKPRF_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDbxMDBX::Flush()
{
	DBFlush(true);
	return ERROR_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void assert_func(const MDBX_env*, const char *msg, const char *function, unsigned line) MDBX_CXX17_NOEXCEPT
{
	Netlib_Logf(nullptr, "MDBX: assertion failed (%s, %d): %s", function, line, msg);

	#if defined(_DEBUG)
		_wassert(_A2T(msg).get(), _A2T(function).get(), line);
	#endif
}

int CDbxMDBX::Map()
{
	if (!LockName(m_pwszProfileName))
		return EGROKPRF_CANTREAD;

	mdbx_env_create(&m_env);
	mdbx_env_set_maxdbs(m_env, 10);
	mdbx_env_set_userctx(m_env, this);
	mdbx_env_set_assert(m_env, assert_func);

#ifdef _WIN64
	mdbx_env_set_maxreaders(m_env, 1024);
	__int64 upperLimit = 0x400000000ul;
#else
	mdbx_env_set_maxreaders(m_env, 244);
	intptr_t upperLimit = 512ul << 20;
#endif

	int rc = mdbx_env_set_geometry(m_env,
		-1,   // minimal lower limit
		1ul << 20,   // at least 1M for now
		upperLimit,	// 512M upper size
		1ul << 20,   // 1M growth step
		512ul << 10,   // 512K shrink threshold
		-1);  // default page size
	if (rc != MDBX_SUCCESS)
		return EGROKPRF_CANTREAD;

	MDBX_env_flags_t mode = MDBX_NOSUBDIR | MDBX_NOTLS | MDBX_SAFE_NOSYNC | MDBX_COALESCE | MDBX_EXCLUSIVE;
	if (m_bReadOnly)
		mode |= MDBX_RDONLY;

	if (mdbx_env_open(m_env, _T2A(m_pwszProfileName), mode, 0664) != MDBX_SUCCESS)
		return EGROKPRF_CANTREAD;

	return EGROKPRF_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDbxMDBX::PrepareCheck()
{
	InitModules();
	return InitCrypt();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDbxMDBX::SetCacheSafetyMode(BOOL bIsSet)
{
	m_safetyMode = bIsSet != 0;
	DBFlush(true);
}

/////////////////////////////////////////////////////////////////////////////////////////

MDBX_txn* CDbxMDBX::StartTran()
{
	mir_cslock lck(m_csDbAccess);

	if (m_pWriteTran == nullptr) {
		m_dbError = mdbx_txn_begin(m_env, nullptr, (m_bReadOnly) ? MDBX_TXN_RDONLY : MDBX_TXN_READWRITE, &m_pWriteTran);
		// FIXME: throw an exception
		_ASSERT(m_dbError == MDBX_SUCCESS);
	}

	return m_pWriteTran;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDbxMDBX::TouchFile()
{
	SYSTEMTIME st;
	::GetSystemTime(&st);

	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);

	HANDLE hFile = CreateFileW(m_pwszProfileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE) {
		SetFileTime(hFile, nullptr, &ft, &ft);
		CloseHandle(hFile);
	}
}

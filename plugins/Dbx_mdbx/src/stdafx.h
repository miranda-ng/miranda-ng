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

#include <windows.h>
#include <time.h>
#include <process.h>

#include <memory>
#include <vector>
#include <algorithm>
#include <map>
#include <cassert>

#include <newpluginapi.h>
#include <win2k.h>
#include <m_system_cpp.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_icolib.h>
#include <m_options.h>
#include <m_crypto.h>
#include <m_metacontacts.h>
#include <m_protocols.h>
#include <m_netlib.h>
#include <m_gui.h>

#include "libmdbx/mdbx.h"

#ifndef thread_local
#	define thread_local __declspec(thread)
#endif

class txn_ptr
{
	MDBX_txn *txn;
public:
	__forceinline txn_ptr(MDBX_env *pEnv)
	{
		int rc = mdbx_txn_begin(pEnv, nullptr, 0, &txn);
		/* FIXME: throw an exception */
		_ASSERT(rc == MDBX_SUCCESS);
		UNREFERENCED_PARAMETER(rc);
	}

	__forceinline ~txn_ptr()
	{
		if (txn) {
			/* FIXME: see https://github.com/leo-yuriev/libfpta/blob/77a7251fde2030165a3916ee68fd86a1374b3dd8/src/common.cxx#L370 */
			abort();
		}
	}

	__forceinline operator MDBX_txn*() const { return txn; }

	__forceinline int commit()
	{
		int rc = mdbx_txn_commit(txn);
		if (rc != MDBX_SUCCESS) {
			/* FIXME: throw an exception */
			abort();
			return rc;
		}
		txn = nullptr;
		return MDBX_SUCCESS;
	}

	__forceinline void abort()
	{
		int rc = mdbx_txn_abort(txn);
		/* FIXME: throw an exception */
		_ASSERT(rc == MDBX_SUCCESS);
		UNREFERENCED_PARAMETER(rc);
		txn = nullptr;
	}
};

struct CMDBX_txn_ro
{
	MDBX_txn *txn = nullptr;
	mir_cs cs;

	__forceinline operator MDBX_txn* () { return txn; }
	__forceinline MDBX_txn** operator &() { return &txn; }
};

class txn_ptr_ro
{
	CMDBX_txn_ro &txn;
	mir_cslock lock;

public:
	__forceinline txn_ptr_ro(CMDBX_txn_ro &_txn) : txn(_txn), lock(txn.cs)
	{
		int rc = mdbx_txn_renew(txn);
		/* FIXME: throw an exception */
		_ASSERT(rc == MDBX_SUCCESS);
		UNREFERENCED_PARAMETER(rc);
	}
	__forceinline ~txn_ptr_ro()
	{
		int rc = mdbx_txn_reset(txn);
		/* FIXME: throw an exception */
		_ASSERT(rc == MDBX_SUCCESS);
		UNREFERENCED_PARAMETER(rc);
	}
	__forceinline operator MDBX_txn*() const { return txn; }
};

class cursor_ptr
{
	MDBX_cursor *m_cursor;

public:
	__forceinline cursor_ptr(MDBX_txn *_txn, MDBX_dbi _dbi)
	{
		if (mdbx_cursor_open(_txn, _dbi, &m_cursor) != MDBX_SUCCESS)
			m_cursor = nullptr;
	}

	__forceinline ~cursor_ptr()
	{
		if (m_cursor)
			mdbx_cursor_close(m_cursor);
	}

	__forceinline operator MDBX_cursor*() const { return m_cursor; }
};

class cursor_ptr_ro
{
	MDBX_cursor *m_cursor;
public:
	__forceinline cursor_ptr_ro(MDBX_cursor *cursor) : m_cursor(cursor)
	{
		int rc = mdbx_cursor_renew(mdbx_cursor_txn(m_cursor), m_cursor);
		/* FIXME: throw an exception */
		_ASSERT(rc == MDBX_SUCCESS);
		UNREFERENCED_PARAMETER(rc);
	}
	__forceinline operator MDBX_cursor*() const { return m_cursor; }
};

#include "dbintf.h"
#include "resource.h"
#include "version.h"

extern HINSTANCE g_hInst;
extern LIST<CDbxMDBX> g_Dbs;

#include "ui.h"

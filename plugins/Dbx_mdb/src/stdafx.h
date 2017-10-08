/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org)
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

#include "mdbx/mdbx.h"

#ifndef thread_local
#	define thread_local __declspec(thread)
#endif


class txn_ptr
{
	MDBX_txn *m_txn;
public:
	__forceinline txn_ptr(MDBX_env *pEnv)
	{
		mdbx_txn_begin(pEnv, NULL, 0, &m_txn);
	}

	__forceinline ~txn_ptr()
	{
		if (m_txn)
			mdbx_txn_abort(m_txn);
	}

	__forceinline operator MDBX_txn*() const { return m_txn; }

	__forceinline int commit()
	{
		MDBX_txn *tmp = m_txn;
		m_txn = nullptr;
		return mdbx_txn_commit(tmp);
	}

	__forceinline void abort()
	{
		mdbx_txn_abort(m_txn);
		m_txn = NULL;
	}
};

struct CMDBX_txn_ro
{
	MDBX_txn *m_txn;
	bool bIsActive;
	mir_cs cs;

	__forceinline CMDBX_txn_ro() : m_txn(nullptr), bIsActive(false) {}

	__forceinline operator MDBX_txn* () { return m_txn; }
	__forceinline MDBX_txn** operator &() { return &m_txn; }
};

class txn_ptr_ro
{
	CMDBX_txn_ro &m_txn;
	bool bNeedReset;
	mir_cslock lock;
public:
	__forceinline txn_ptr_ro(CMDBX_txn_ro &txn) : m_txn(txn), bNeedReset(!txn.bIsActive), lock(m_txn.cs)
	{
		if (bNeedReset)
		{
			mdbx_txn_renew(m_txn);
			m_txn.bIsActive = true;
		}
	}
	__forceinline ~txn_ptr_ro()
	{
		if (bNeedReset)
		{
			mdbx_txn_reset(m_txn);
			m_txn.bIsActive = false;
		}
	}
	__forceinline operator MDBX_txn*() const { return m_txn; }
};

class cursor_ptr
{
	MDBX_cursor *m_cursor;

public:
	__forceinline cursor_ptr(MDBX_txn *_txn, MDBX_dbi _dbi)
	{
		if (mdbx_cursor_open(_txn, _dbi, &m_cursor) != MDBX_SUCCESS)
			m_cursor = NULL;
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
		mdbx_cursor_renew(mdbx_cursor_txn(m_cursor), m_cursor);
	}
	__forceinline operator MDBX_cursor*() const { return m_cursor; }
};

#define MDBX_CHECK(A,B) \
	switch (A) { \
	case MDBX_SUCCESS: break; \
	case MDBX_MAP_FULL: continue; \
	default: return (B); }




#include "dbintf.h"
#include "resource.h"
#include "version.h"

extern HINSTANCE g_hInst;
extern LIST<CDbxMdb> g_Dbs;

#include "ui.h"

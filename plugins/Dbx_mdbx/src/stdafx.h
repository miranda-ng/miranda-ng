/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-20 Miranda NG team (https://miranda-ng.org)
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

#pragma once

#include <windows.h>

#include <io.h>
#include <malloc.h>
#include <time.h>
#include <process.h>

#include <memory>
#include <vector>
#include <algorithm>
#include <map>
#include <cassert>
#include <string>

#include <newpluginapi.h>
#include <win2k.h>
#include <m_system.h>
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
#include <m_skin.h>

#include "../../Libs/libmdbx/src/mdbx.h"

#ifndef thread_local
#	define thread_local __declspec(thread)
#endif

class txn_ptr
{
	MDBX_txn *txn;
public:
	__forceinline txn_ptr(MDBX_txn *_txn) :
		txn(_txn)
	{}

	__forceinline ~txn_ptr()
	{
		if (txn) {
			/* FIXME: see https://github.com/leo-yuriev/libfpta/blob/77a7251fde2030165a3916ee68fd86a1374b3dd8/src/common.cxx#L370 */
			Abort();
		}
	}

	__forceinline operator MDBX_txn*() const { return txn; }

	__forceinline int Commit()
	{
		int rc = mdbx_txn_commit(txn);
		if (rc != MDBX_SUCCESS) {
			/* FIXME: throw an exception */
			Abort();
			return rc;
		}
		txn = nullptr;
		return MDBX_SUCCESS;
	}

	__forceinline void Abort()
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
	txn_ptr_ro(CMDBX_txn_ro &_txn);
	~txn_ptr_ro();

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

#include "dbintf.h"
#include "resource.h"
#include "version.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};

#include "ui.h"


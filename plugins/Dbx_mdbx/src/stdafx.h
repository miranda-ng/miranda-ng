/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)
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

class txn_ptr
{
	CDbxMDBX *pDb;
	MDBX_txn *txn;

public:
	txn_ptr(CDbxMDBX *_db);
	~txn_ptr();

	__forceinline operator MDBX_txn*() const { return txn; }
};

#include "resource.h"
#include "version.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};


/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x0501

#pragma warning(disable:4509)

#include <windows.h>
#include <time.h>
#include <process.h>
#include <memory>

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

#include "ham/hamsterdb.h"

#include "dbintf.h"
#include "resource.h"
#include "version.h"

class cursor_ptr
{
	ham_cursor_t *m_cursor;

public:
	__forceinline cursor_ptr(ham_db_t *_dbi)
	{
		if (ham_cursor_create(&m_cursor, _dbi, NULL, 0) != HAM_SUCCESS)
			m_cursor = NULL;
	}

	__forceinline ~cursor_ptr()
	{
		if (m_cursor)
			ham_cursor_close(m_cursor);
	}

	__forceinline operator ham_cursor_t*() const { return m_cursor; }
};

extern HINSTANCE g_hInst;
extern LIST<CDbxKV> g_Dbs;

#ifdef __GNUC__
#define mir_i64(x) (x##LL)
#else
#define mir_i64(x) (x##i64)
#endif

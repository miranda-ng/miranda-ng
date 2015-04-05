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

#define OWN_CACHED_CONTACT
#include <m_db_int.h>

#include "dbintf.h"
#include "resource.h"
#include "version.h"

extern HINSTANCE g_hInst;
extern LIST<CDbxKyoto> g_Dbs;

struct VisitorCopy : public TreeDB::Visitor
{
	__forceinline explicit VisitorCopy() : kbuf_(NULL), ksiz_(0) {}

	const char* visit_full(const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz, size_t* sp)
	{
		kbuf_ = kbuf;
		ksiz_ = ksiz;
		return NOP;
	}
	const char* kbuf_;
	size_t ksiz_;
};

struct VisitorCopyRec : public TreeDB::Visitor
{
	__forceinline explicit VisitorCopyRec() : vbuf_(NULL), vsiz_(0) {}

	const char* visit_full(const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz, size_t* sp)
	{
		vbuf_ = vbuf;
		vsiz_ = vsiz;
		return NOP;
	}
	const char* vbuf_;
	size_t vsiz_;
};

class cursor_ptr
{
	TreeDB::Cursor *m_cursor;

public:
	__forceinline cursor_ptr(TreeDB &_db)
	{
		m_cursor = _db.cursor();
	}

	__forceinline ~cursor_ptr()
	{
		delete m_cursor;
	}

	__forceinline TreeDB::Cursor* operator->() const { return m_cursor; }
};

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

#pragma once

#pragma warning(disable:4509)

#include <windows.h>
#include <wincred.h>
#include <WinIoCtl.h>

#include <malloc.h>
#include <time.h>
#include <process.h>
#include <memory>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_icolib.h>
#include <m_options.h>
#include <m_crypto.h>
#include <m_metacontacts.h>
#include <m_protocols.h>
#include <m_gui.h>
#include <m_skin.h>
#include <m_netlib.h>
#include <m_import.h>

#include "database.h"
#include "dbintf.h"
#include "resource.h"
#include "version.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};

extern LIST<CDb3Mmap> g_Dbs;
extern DBSignature dbSignatureU, dbSignatureE, dbSignatureIM, dbSignatureSA, dbSignatureSD;

#ifdef __GNUC__
#define mir_i64(x) (x##LL)
#else
#define mir_i64(x) (x##i64)
#endif

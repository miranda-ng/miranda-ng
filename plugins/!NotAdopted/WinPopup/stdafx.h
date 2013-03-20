/*

WinPopup Protocol plugin for Miranda IM.

Copyright (C) 2004-2011 Nikolay Raspopov

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

// Совместимость
#define WINVER			0x0500	// Windows 2000 compatible
#define _WIN32_WINNT	0x0500	// Windows 2000 compatible
#define _WIN32_WINDOWS	0x0500	// Windows 2000 compatible
#define _WIN32_IE		0x0500	// IE 5 compatible

#define MIRANDA_VER		0x0700	// Miranda 0.7.x compatible

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOCOMM
#define NOSERVICE
#define NOHELP
#define NOSOUND
#define NOPRINT

#define _ATL_NO_COM_SUPPORT
#define _ATL_NO_EXCEPTIONS
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_NO_CRT
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#define _CRT_SECURE_NO_WARNINGS

// Disable warnings enabled by /Wall option
#pragma warning (disable: 4619)	// warning C4619: #pragma warning : there is no warning number
#pragma warning (disable: 4625)	// warning C4625: '' : copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning (disable: 4626)	// warning C4626: '' : assignment operator could not be generated because a base class assignment operator is inaccessible
#pragma warning (disable: 4711)	// warning C4711: function '' selected for automatic inline expansion

// Temporary disable warnings
#pragma warning (disable: 4100)
#pragma warning (disable: 4191)
#pragma warning (disable: 4365)

#include "resource.h"

// Common headers
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <atlfile.h>

#include <prsht.h>
#include <winsock.h>
#include <lm.h>
#include <nb30.h>
#include <process.h>
#include <commctrl.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <shellapi.h>

#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
	#define DEBUG_NEW new(_CLIENT_BLOCK,THIS_FILE,__LINE__)
#else
	#define DEBUG_NEW
#endif // _DEBUG

// Miranda SDK headers
#include "statusmodes.h"
#include "newpluginapi.h"	// uses m_plugins.h
#include "m_system.h"
#include "m_clist.h"
#include "m_database.h"
#include "m_langpack.h"
#include "m_netlib.h"		// uses m_utils.h
#include "m_options.h"
#include "m_popup.h"
#include "m_protocols.h"
#include "m_protomod.h"
#include "m_protosvc.h"
#include "m_userinfo.h"
#include "m_chat.h"
#include "m_avatars.h"

using namespace ATL;

typedef class CComCritSecLock< CComAutoCriticalSection > CLock;

#ifdef _DEBUG
	#define ALMOST_INFINITE (INFINITE)	// бесконечность
#else
	#define ALMOST_INFINITE (20000)		// 20 секунд
#endif

inline LPSTR lstrnchr(LPSTR s, const CHAR c, int n) throw ()
{
	for (; n; --n, ++s)
		if (c == *s)
			return s;
	return NULL;
}

inline LPTSTR lstrnrchr(LPTSTR s, const TCHAR c, int n) throw ()
{
	s += n;
	for (; n; --n, --s)
		if (c == *s)
			return s;
	return NULL;
}

#include "dllLoaderMinimal.h"

// Enable warnings
#pragma warning (default: 4100)
#pragma warning (default: 4191)
#pragma warning (default: 4365)

// #define CHAT_ENABLED

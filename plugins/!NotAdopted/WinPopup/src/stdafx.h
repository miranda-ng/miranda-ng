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

// Совместимость
#define WINVER			0x0500	// Windows 2000 compatible
#define _WIN32_WINNT	0x0500	// Windows 2000 compatible
#define _WIN32_WINDOWS	0x0500	// Windows 2000 compatible
#define _WIN32_IE		0x0500	// IE 5 compatible

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOCOMM
#define NOSERVICE
#define NOHELP
#define NOSOUND
#define NOPRINT

//#define _ATL_NO_COM_SUPPORT
//#define _ATL_NO_EXCEPTIONS
//#define _ATL_NO_AUTOMATIC_NAMESPACE
//#define _ATL_CSTRING_NO_CRT
//#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#define _CRT_SECURE_NO_WARNINGS

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

#include "resource.h"
#include "dllLoaderMinimal.h"
#include "network.h"
#include "winpopup_proto.h"
#include "services.h"
#include "mailslot.h"
#include "netbios_name.h"
#include "netbios.h"
#include "scanner.h"
#include "messenger.h"
#include "messagebox.h"
#include "search.h"
#include "chat.h"
#include "md5.h"
#include "options.h"
#include "user_info.h"
#include "add_dialog.h"
#include "processapi.h"
#include "smbconst.h"

using namespace ATL;

typedef class CComCritSecLock< CComAutoCriticalSection > CLock;

#ifdef _DEBUG
	#define ALMOST_INFINITE (INFINITE)	// бесконечность
#else
	#define ALMOST_INFINITE (20000)		// 20 секунд
#endif

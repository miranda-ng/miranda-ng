// stdafx.h : Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#ifndef _WIN32_WINNT		// Lassen Sie die Verwendung spezifischer Features von Windows XP oder später zu.                   
#define _WIN32_WINNT 0x0501	// Ändern Sie dies in den geeigneten Wert für andere Versionen von Windows.
#endif						

#include <sys/types.h>
#include <stdio.h>
#include <cstdlib>
#include <tchar.h>
#include <iostream>
#include <string>

#define uint unsigned int
#define socklen_t int

#define NO_PTHREAD

#pragma warning(disable:4267)
#pragma warning(disable:4244)
#pragma warning(disable:4312)
#pragma warning(disable:4099)
#pragma warning(disable:4311)
#pragma comment(lib,"ws2_32.lib")

#ifndef NO_PTHREAD
#pragma comment(lib,"pthreads.lib")
#endif

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")

#include <winsock2.h>
#include <gdiplus.h>
#include <Wininet.h>
#include <commctrl.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <string.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <string.h>
#include <Iphlpapi.h>

#include "resource.h"

//Miranda SDK headers
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_protomod.h>
#include <m_database.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_userinfo.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_ignore.h>
#include <m_netlib.h>
#include <m_avatars.h>
#include <m_folders.h>
#include <m_assocmgr.h>
#include <m_icolib.h>
#include <m_genmenu.h>
#include <m_extraicons.h>
#include <m_xstatus.h>

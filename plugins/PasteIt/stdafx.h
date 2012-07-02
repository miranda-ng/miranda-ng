// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <winsock.h>
#include <Shlobj.h>
#include <commdlg.h>
#include <windowsx.h>
#include <time.h>

#include <tchar.h>
#include <string>
#include <map>
#include <list>

#define MIRANDA_VER		0x0700
#define MIRANDA_CUSTOM_LP

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_system.h>
#include <m_utils.h>
#include <m_netlib.h>
#include <m_xml.h>
#include <m_message.h>
#include <m_protosvc.h>
#include <m_icolib.h>
#include <m_options.h>
#include <m_database.h>
#include <m_chat.h>
#include <m_protoint.h>

#include "sdk/m_msg_buttonsbar.h"
#include "sdk/m_updater.h"

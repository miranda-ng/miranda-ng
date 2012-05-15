// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define MIRANDA_VER		0x0700 // for tabbed options

#include <m_stdhdr.h>

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

#include <stddef.h>
#include <time.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <statusmodes.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_system.h>
#include <m_idle.h>
#include <m_skin.h>
#include <m_clui.h>
#include <m_clist.h>
#include <m_clc.h>
#include <m_cluiframes.h>
#include <m_awaymsg.h>
#include <m_utils.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_protomod.h>
#include <m_contacts.h>
#include <m_timezones.h>

#include <m_popup.h>
#include <m_fontservice.h>
#include <m_avatars.h>
#include <m_icq.h>
#include <m_imgsrvc.h>

#include "m_updater.h"
#include "m_variables.h"
#include "m_notify.h"
#include "m_metacontacts.h"

#define MODULE			"Tipper"

extern HMODULE hInst;

extern HFONT hFontTitle, hFontLabels, hFontValues;
extern COLORREF	colTitle, colLabels, colBg, colValues;

extern int code_page;

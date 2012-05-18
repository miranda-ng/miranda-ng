/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

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

#define MIRANDA_VER		0x0A00

#include <m_stdhdr.h>

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <commctrl.h>
#include <stddef.h>
#include <time.h>
#include <windowsx.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <statusmodes.h>
#include <m_avatars.h>
#include <m_awaymsg.h>
#include <m_clc.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_cluiframes.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_fontservice.h>
#include <m_icolib.h>
#include <m_idle.h>
#include <m_imgsrvc.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_system.h>
#include <m_timezones.h>
#include <m_utils.h>
#include <m_icq.h>

#include "resource.h"
#include "m_tipper.h"
#include "m_fingerprint.h"
#include "m_flags.h"
#include "m_folders.h"
#include "m_metacontacts.h"
#include "m_variables.h"
#include "m_updater.h"
#include "m_smileyadd.h"

#if defined _WIN64
// {605103E8-27A1-4d41-B385-BE652047A833}
#define	MIID_TIPPER { 0x605103e8, 0x27a1, 0x4d41, { 0xb3, 0x85, 0xbe, 0x65, 0x20, 0x47, 0xa8, 0x33 } }
#elif defined _UNICODE
// {8392DF1D-9090-4f8e-9DF6-2FE058EDD800}
#define MIID_TIPPER	{ 0x8392df1d, 0x9090, 0x4f8e, { 0x9d, 0xf6, 0x2f, 0xe0, 0x58, 0xed, 0xd8, 0x00 } }
#else
// {63FD0B98-43AD-4c13-BD6E-2B550B9B20EF}
#define	MIID_TIPPER { 0x63fd0b98, 0x43ad, 0x4c13, { 0xbd, 0x6e, 0x2b, 0x55, 0xb0, 0x9b, 0x20, 0xef } }
#endif

#define MODULE					"Tipper"
#define MODULE_ITEMS			"Tipper_Items"
#define DEFAULT_SKIN_FOLDER		"Skins\\Tipper"

extern HMODULE hInst;
extern PLUGINLINK *pluginLink;

extern HFONT hFontTitle, hFontLabels, hFontValues, hFontTrayTitle;
extern COLORREF	colTitle, colLabels, colBg, colValues;

extern int iCodePage;
extern char szMetaModuleName[256];

extern MM_INTERFACE mmi;
extern LIST_INTERFACE li;
extern FI_INTERFACE *fii;
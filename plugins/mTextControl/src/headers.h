/*
Miranda Text Control - Plugin for Miranda IM
Copyright (C) 2005 Victor Pavlychko (nullbie@gmail.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __headers_h__
#define __headers_h__

#if defined(UNICODE) && !defined(_UNICODE)
	#define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
	#define UNICODE
#endif

#ifndef UNICODE
	#define OLE2ANSI
#elif defined(OLE2ANSI)
	#undef OLE2ANSI
#endif

#define _CRT_SECURE_NO_DEPRECATE

//Windows headers
#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define OEMRESOURCE
#include <tchar.h>
#include <windows.h>
#include <richedit.h>
#include <richole.h>
#include <tom.h>
#include <textserv.h>
#include <ole2.h>
#include <commctrl.h>
#include <stdio.h>

//Resources
//#include "../res/resource.h"

#ifdef _cplusplus
extern "C" {
#endif
	//Miranda headers
	#include <newpluginapi.h>
	#include <win2k.h>
	#include <m_system.h>
	#include <m_plugins.h>
	#include <m_clui.h>
	#include <m_clist.h>
	#include <m_options.h>
	#include <m_skin.h>
	#include <m_langpack.h>
	#include <m_database.h>
	#include <m_protocols.h>
	#include <m_utils.h>
//	#include <m_icolib.h>

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MTEXTCONTROL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef MTEXTCONTROL_EXPORTS
	#define DLL_API __declspec(dllexport)
#else
	#define DLL_API __declspec(dllimport)
#endif // MTEXTCONTROL_EXPORTS

#ifndef MTEXT_NOHELPERS
	#define MTEXT_NOHELPERS
#endif // MTEXT_NOHELPERS
	#include "m_text.h"

// API for other plugins
//	#include "../api/m_popup.h"
//	#include "../api/m_popupw.h"
	#include "m_mathmodule.h"
	#include <m_smileyadd.h>
//	#include "../api/m_nconvers.h"
//	#include "../api/m_ticker.h"
//	#include "../api/m_avatars.h"
#ifdef _cplusplus
};
#endif

// globals
extern HINSTANCE		hInst;
extern PLUGININFO		pluginInfo;
extern PLUGININFOEX		pluginInfoEx;

// dlls
//#include "dlls.h"

// Project headers
#include "version.h"
//#include "gdiutils.h"
//#include "bitmap_funcs.h"
//#include "icons.h"
#include "services.h"
#include "textusers.h"
//#include "options.h"
#include "richeditutils.h"
//#include "IconOleObject.h"
#include "ImageDataObjectHlp.h"
#include "FormattedTextDraw.h"
#include "fancy_rtf.h"
//#include "fancy.h"
#include "textcontrol.h"

#endif // __headers_h__

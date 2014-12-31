/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-12 Miranda ICQ/IM project,
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

#ifndef M_LANGPACK_H__
#define M_LANGPACK_H__

#if !defined(M_CORE_H__)
	#include <m_core.h>
#endif

#if !defined(_STATIC)
	#define MIRANDA_CUSTOM_LP
#endif

//translates a single string into the user's local language	  v0.1.1.0+
//wParam = 0
//lParam = (LPARAM)(const char*)szEnglish
//returns a pointer to the localised string. If there is no known translation
//it will return szEnglish. The return value does not need to be freed in any
//way
//Note that the Translate() macro as defined below will crash plugins that are
//loaded into Miranda 0.1.0.1 and earlier. If anyone's actually using one of
//these versions, I pity them.
#define MS_LANGPACK_TRANSLATESTRING  "LangPack/TranslateString"

// If you're storing some string for calling later-on Translate or using it
// with an API call that does translation automatically marked with
// [TRANSLATED-BY-CORE] please wrap it with one of LPGEN macros in order to
// generate proper language pack.
#define LPGEN(s)			s
#define LPGENW(s)			L ## s
#ifdef _UNICODE
	#define LPGENT(s)		_T(s)
#else
	#define LPGENT(s)		s
#endif
//Those macros do NOTHING. They are just markers for lpgen.pl.

//translates a menu into the user's local language	  v0.1.1.0+
//wParam = (WPARAM)(HMENU)hMenu
//lParam = langpack handle (v.0.10.0+)
//returns 0 on success, nonzero on failure
#define MS_LANGPACK_TRANSLATEMENU "LangPack/TranslateMenu"

//returns the codepage used in the language pack 	  v0.4.3.0+
//wParam = 0
//lParam = 0
//returns the codepage stated in the langpack, or CP_ACP if no langpack is present
#define MS_LANGPACK_GETCODEPAGE "LangPack/GetCodePage"

//returns the locale id associated with the language pack    v0.4.3.0+
//wParam = 0
//lParam = 0
//returns the Windows locale id stated in the langpack, or LOCALE_USER_DEFAULT if no langpack is present
#define MS_LANGPACK_GETLOCALE "LangPack/GetLocale"

//returns the strdup/wcsdup of lparam according to the langpack  v0.4.3.0+
//wParam = langpack handle (v.0.10.0+)
//lParam = (LPARAM)(char*)source string
//returns a string converted from char* to TCHAR* using the langpack codepage.
//This string should be freed using mir_free() then
#define MS_LANGPACK_PCHARTOTCHAR "LangPack/PcharToTchar"

#if defined(MIRANDA_CUSTOM_LP)

__forceinline INT_PTR Langpack_PCharToTChar(const char* str)
{	return CallService(MS_LANGPACK_PCHARTOTCHAR, hLangpack, (LPARAM)str);
}

#else

__forceinline INT_PTR Langpack_PCharToTChar(const char* str)
{	return CallService(MS_LANGPACK_PCHARTOTCHAR, 0, (LPARAM)str);
}

#endif

//initializes the plugin-specific translation context  v0.10.0+
//wParam = pointer to the langpack handle
//lParam = PLUGININFOEX* of the caller plugin
//always returns 0
#define MS_LANGPACK_REGISTER "LangPack/Register"

#if defined(MIRANDA_CUSTOM_LP)
__forceinline void mir_getLP(const PLUGININFOEX* pInfo)
{	CallService(MS_LANGPACK_REGISTER, (WPARAM)&hLangpack, (LPARAM)pInfo);
}
#endif

//reloads langpack
//wParam = 0 (ignored)
//lParam = (LPARAM)(TCHAR*)langpack file name or NULL to reload the current one
//always returns 0
#define MS_LANGPACK_RELOAD "LangPack/Reload"

#define ME_LANGPACK_CHANGED "LangPack/Changed"

//retrieves the hLangpack of a plugin by its HINSTANCE
//wParam = 0 (ignored)
//lParam = (LPARAM)(HINSTANCE)plugin's base address
//returns hLangpack if found, or 0 if error occurred
#define MS_LANGPACK_LOOKUPHANDLE "LangPack/LookupHandle"

#endif // M_LANGPACK_H__

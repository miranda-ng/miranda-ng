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

/////////////////////////////////////////////////////////////////////////////////////////
// translates a single string into the user's local language
// returns a pointer to the localised string. If there is no known translation
// it will return szEnglish. The return value does not need to be freed in any way
// Note that the Translate() macro as defined below will crash plugins that are
// loaded into Miranda 0.1.0.1 and earlier. If anyone's actually using one of
// these versions, I pity them.

#define LANG_UNICODE 0x1000

EXTERN_C MIR_CORE_DLL(void)     Langpack_SortDuplicates(void);

EXTERN_C MIR_CORE_DLL(int)      LoadLangPackModule(void);
EXTERN_C MIR_CORE_DLL(int)      LoadLangPack(const TCHAR *szLangPack);
EXTERN_C MIR_CORE_DLL(void)     ReloadLangpack(TCHAR *pszStr);

EXTERN_C MIR_CORE_DLL(char*)    TranslateA_LP(const char* str, int hLang);
EXTERN_C MIR_CORE_DLL(wchar_t*) TranslateW_LP(const wchar_t* str, int hLang);
EXTERN_C MIR_CORE_DLL(void)     TranslateDialog_LP(HWND hDlg, int hLang);

#define Translate(s) TranslateA_LP(s, hLangpack)
#define TranslateW(s) TranslateW_LP(s, hLangpack)
#define TranslateDialogDefault(h) TranslateDialog_LP(h,hLangpack)

#ifdef _UNICODE
	#define TranslateT(s)	 TranslateW_LP(_T(s),hLangpack)
	#define TranslateTS(s)	 TranslateW_LP(s,hLangpack)
	#define TranslateTH(l,s) TranslateW_LP(s,l)
#else
	#define TranslateT(s)	 TranslateA_LP(s,hLangpack)
	#define TranslateTS(s)	 TranslateA_LP(s,hLangpack)
	#define TranslateTH(l,s) TranslateA_LP(s,l)
#endif

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

/////////////////////////////////////////////////////////////////////////////////////////
// translates a menu into the user's local language
// returns 0 on success, nonzero on failure

EXTERN_C MIR_CORE_DLL(void) TranslateMenu_LP(HMENU, int = hLangpack);

#define TranslateMenu(h) TranslateMenu_LP(h,hLangpack)

/////////////////////////////////////////////////////////////////////////////////////////
// returns the codepage used in the language pack
// returns the codepage stated in the langpack, or CP_ACP if no langpack is present

EXTERN_C MIR_CORE_DLL(int) Langpack_GetDefaultCodePage(void);

/////////////////////////////////////////////////////////////////////////////////////////
// returns the locale id associated with the language pack
// returns the Windows locale id stated in the langpack, or LOCALE_USER_DEFAULT if no langpack is present

EXTERN_C MIR_CORE_DLL(int) Langpack_GetDefaultLocale(void);

/////////////////////////////////////////////////////////////////////////////////////////
// returns the strdup/wcsdup of lparam according to the langpack
// returns a string converted from char* to TCHAR* using the langpack codepage.
// This string should be freed using mir_free() then

EXTERN_C MIR_CORE_DLL(TCHAR*) Langpack_PcharToTchar(const char* pszStr);

/////////////////////////////////////////////////////////////////////////////////////////
// initializes the plugin-specific translation context  v0.10.0+
// always returns 0

EXTERN_C MIR_CORE_DLL(void) mir_getLP(const PLUGININFOEX *pInfo, int *_hLang = &hLangpack);

/////////////////////////////////////////////////////////////////////////////////////////
// reloads langpack
// wParam = 0 (ignored)
// lParam = (LPARAM)(TCHAR*)langpack file name or NULL to reload the current one
// always returns 0

#define MS_LANGPACK_RELOAD "LangPack/Reload"

#define ME_LANGPACK_CHANGED "LangPack/Changed"

#endif // M_LANGPACK_H__

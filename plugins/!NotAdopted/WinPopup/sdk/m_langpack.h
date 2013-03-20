/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
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

#define LANG_UNICODE 0x1000

//translates a single string into the user's local language	  v0.1.1.0+
//wParam=0
//lParam=(LPARAM)(const char*)szEnglish
//returns a pointer to the localised string. If there is no known translation
//it will return szEnglish. The return value does not need to be freed in any
//way
//Note that the Translate() macro as defined below will crash plugins that are
//loaded into Miranda 0.1.0.1 and earlier. If anyone's actually using one of
//these versions, I pity them.
#define MS_LANGPACK_TRANSLATESTRING   "LangPack/TranslateString"
#define Translate(s)   ((char*)CallService(MS_LANGPACK_TRANSLATESTRING,0,(LPARAM)(s)))
#define TranslateW(s)   ((WCHAR*)CallService(MS_LANGPACK_TRANSLATESTRING,LANG_UNICODE,(LPARAM)(s)))
#ifdef _UNICODE
	#define TranslateT(s)	TranslateW(_T(s))
	#define TranslateTS(s)	TranslateW(s)
#else
	#define TranslateT(s)	Translate(s)
	#define TranslateTS(s)	Translate(s)
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

//translates a dialog into the user's local language	  v0.1.1.0+
//wParam=0
//lParam=(LPARAM)(LANGPACKTRANSLATEDIALOG*)&lptd
//returns 0 on success, nonzero on failure
//This service only knows about the following controls:
//Window titles, STATIC, EDIT, Hyperlink, BUTTON
typedef struct {
	int cbSize;
	DWORD flags;
	HWND hwndDlg;
	const int *ignoreControls;   //zero-terminated list of control IDs *not* to
	    //translate
} LANGPACKTRANSLATEDIALOG;
#define LPTDF_NOIGNOREEDIT   1     //translate all edit controls. By default
                   //non-read-only edit controls are not translated
#define LPTDF_NOTITLE        2     //do not translate the title of the dialog

#define MS_LANGPACK_TRANSLATEDIALOG  "LangPack/TranslateDialog"
__inline static INT_PTR TranslateDialogDefault(HWND hwndDlg)
{
	LANGPACKTRANSLATEDIALOG lptd;
	lptd.cbSize=sizeof(lptd);
	lptd.flags=0;
	lptd.hwndDlg=hwndDlg;
	lptd.ignoreControls=NULL;
	return CallService(MS_LANGPACK_TRANSLATEDIALOG,0,(LPARAM)&lptd);
}

//translates a menu into the user's local language	  v0.1.1.0+
//wParam=(WPARAM)(HMENU)hMenu
//lParam=0
//returns 0 on success, nonzero on failure
#define MS_LANGPACK_TRANSLATEMENU    "LangPack/TranslateMenu"

//returns the codepage used in the language pack 	  v0.4.3.0+
//wParam=0
//lParam=0
//returns the codepage stated in the langpack, or CP_ACP if no langpack is present
#define MS_LANGPACK_GETCODEPAGE      "LangPack/GetCodePage"

//returns the locale id associated with the language pack    v0.4.3.0+
//wParam=0
//lParam=0
//returns the Windows locale id stated in the langpack, or LOCALE_USER_DEFAULT if no langpack is present
#define MS_LANGPACK_GETLOCALE        "LangPack/GetLocale"

//returns the strdup/wcsdup of lparam according to the langpack  v0.4.3.0+
//wParam=0
//lParam=(LPARAM)(char*)source string
//returns a string converted from char* to TCHAR* using the langpack codepage. 
//This string should be freed using mir_free() then
#define MS_LANGPACK_PCHARTOTCHAR     "LangPack/PcharToTchar"
#endif // M_LANGPACK_H__

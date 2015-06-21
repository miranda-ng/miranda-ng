
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

#ifndef M_UTILS_H__
#define M_UTILS_H__ 1

#ifdef _MSC_VER
	#pragma warning(disable:4201)
#endif

#include <stdio.h>

#if !defined(M_SYSTEM_H__)
#include <m_system.h>
#endif

#if !defined(M_STRING_H__)
#include <m_string.h>
#endif

//this entire module is v0.1.0.1+
//this module cannot be redefined by a plugin, because it's not useful for it
//to be possible
//There are some more utility services in the database for dealing with time
//and simple string scrambling, but they are very db-orientated

/* Opens a URL in the user's default web browser   v0.1.0.1+
wParam = OUF_* flags
lParam = (LPARAM)(const TCHAR*)szUrl
returns 0 always
bOpenInNewWindow should be zero to open the URL in the browser window the user
last used, or nonzero to open in a new browser window. If there's no browser
running, one will be opened to show the URL.
*/

#define OUF_NEWWINDOW   1
#define OUF_UNICODE     2

#if defined( _UNICODE )
	#define OUF_TCHAR OUF_UNICODE
#else
	#define OUF_TCHAR 0
#endif

#define MS_UTILS_OPENURL	"Utils/OpenURL"

/////////////////////////////////////////////////////////////////////////////////////////
// Resizes a dialog by calling a custom routine to move the individual
// Returns 0 on success, or nonzero on failure
// Does not support dialogtemplateex dialog boxes, and will return failure if you try to resize one
// The dialog itself should have been resized prior to calling this service
// pfnResizer is called once for each control in the dialog
// pfnResizer should return a combination of one rd_anchorx_ and one rd_anchory constant

#define RD_ANCHORX_CUSTOM	0	// function did everything required to the x axis, do no more processing
#define RD_ANCHORX_LEFT 	0	// move the control to keep it constant distance from the left edge of the dialog
#define RD_ANCHORX_RIGHT	1	// move the control to keep it constant distance from the right edge of the dialog
#define RD_ANCHORX_WIDTH	2	// size the control to keep it constant distance from both edges of the dialog
#define RD_ANCHORX_CENTRE	4	// move the control to keep it constant distance from the centre of the dialog
#define RD_ANCHORY_CUSTOM	0
#define RD_ANCHORY_TOP		0
#define RD_ANCHORY_BOTTOM	8
#define RD_ANCHORY_HEIGHT	16
#define RD_ANCHORY_CENTRE	32

struct UTILRESIZECONTROL
{
	int cbSize;
	UINT wId;             // control ID
	RECT rcItem;          // original control rectangle, relative to dialog
	                      // modify in-place to specify the new position
	SIZE dlgOriginalSize; // size of dialog client area in template
	SIZE dlgNewSize;      // current size of dialog client area
};

typedef int (*DIALOGRESIZERPROC)(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL *urc);

EXTERN_C MIR_CORE_DLL(int) Utils_ResizeDialog(HWND hwndDlg, HINSTANCE hInstance, LPCSTR lpTemplate, DIALOGRESIZERPROC pfnResizer, LPARAM lParam = 0);

/* Gets the name of a country given its number		v0.1.2.0+
wParam = countryId
lParam = 0
Returns a pointer to the string containing the country name on success,
or NULL on failure
*/
#define MS_UTILS_GETCOUNTRYBYNUMBER   "Utils/GetCountryByNumber"

/* Gets the name of a country given its ISO code	v0.1.2.0+
wParam = (char*)ISOcode
lParam = 0
Returns a pointer to the string containing the country name on success,
or NULL on failure
*/
#define MS_UTILS_GETCOUNTRYBYISOCODE   "Utils/GetCountryByISOCode"

/* Gets the full list of country IDs	 v0.1.2.0+
wParam = (WPARAM)(int*)piCount
lParam = (LPARAM)(struct CountryListEntry**)ppList
Returns 0 always
Neither wParam nor lParam can be NULL.
The list is sorted alphabetically by country name, on the assumption that it's
quicker to search numbers out of order than it is to search names out of order
*/
struct CountryListEntry {
	int id;
	const char *szName;
	char ISOcode[3];
};
#define MS_UTILS_GETCOUNTRYLIST    "Utils/GetCountryList"

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// Window lists //////////////////////////////////////

#if defined(MIR_CORE_EXPORTS)
typedef struct TWindowList *MWindowList;
#else
DECLARE_HANDLE(MWindowList);
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// allocates a window list
// returns a handle to the new window list

EXTERN_C MIR_CORE_DLL(MWindowList) WindowList_Create(void);

/////////////////////////////////////////////////////////////////////////////////////////
// destroys a window list

EXTERN_C MIR_CORE_DLL(void) WindowList_Destroy(MWindowList hList);

/////////////////////////////////////////////////////////////////////////////////////////
// adds a window to the specified window list
// returns 0 on success, nonzero on failure

EXTERN_C MIR_CORE_DLL(int) WindowList_Add(MWindowList hList, HWND hwnd, MCONTACT hContact);

/////////////////////////////////////////////////////////////////////////////////////////
// removes a window from the specified window list
// returns 0 on success, nonzero on failure

EXTERN_C MIR_CORE_DLL(int) WindowList_Remove(MWindowList hList, HWND hwnd);

/////////////////////////////////////////////////////////////////////////////////////////
// finds a window given the hContact
// returns the window handle on success, or NULL on failure

EXTERN_C MIR_CORE_DLL(HWND) WindowList_Find(MWindowList hList, MCONTACT hContact);

/////////////////////////////////////////////////////////////////////////////////////////
// sends a message to all windows in a list using SendMessage
// returns 0 on success, nonzero on failure

EXTERN_C MIR_CORE_DLL(int) WindowList_Broadcast(MWindowList hList, UINT message, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////
// sends a message to all windows in a list using PostMessage
// returns 0 on success, nonzero on failure

EXTERN_C MIR_CORE_DLL(int) WindowList_BroadcastAsync(MWindowList hList, UINT message, WPARAM wParam, LPARAM lParam);

/***************************** Hyperlink windows ********************************/

//there aren't any services here, because you don't need them.
#define WNDCLASS_HYPERLINK	 _T("Hyperlink")
//the control will obey the SS_LEFT (0), SS_CENTER (1), and SS_RIGHT (2) styles
//the control will send STN_CLICKED via WM_COMMAND when the link itself is clicked

/////////////////////////////////////////////////////////////////////////////////////////
// Use this in a SendMessage to set the color of the url when control is enabled
// wParam = DWORD color
// lParam = not used
#define HLK_SETENABLECOLOUR	 (WM_USER+101) // added in 0.3.1

/////////////////////////////////////////////////////////////////////////////////////////
// Use this in a SendMessage to set the color of the url when control is disabled
// wParam = DWORD color
// lParam = not used
#define HLK_SETDISABLECOLOUR (WM_USER+102) // added in 0.3.1

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Window Position Saving ////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// saves the position of a window in the database
// returns 0 on success, nonzero on failure

EXTERN_C MIR_CORE_DLL(int) Utils_SaveWindowPosition(HWND hwnd, MCONTACT hContact, const char *szModule, const char *szNamePrefix);

/////////////////////////////////////////////////////////////////////////////////////////
// restores the position of a window from the database	 v0.1.1.0+
// wParam = flags
// lParam = (LPARAM)(SAVEWINDOWPOS*)&swp
// returns 0 on success, nonzero on failure
// if no position was found in the database, the function returns 1 and does
// nothing
// the NoSize version won't use stored size information: the window is left the same size.

#define RWPF_NOSIZE 	1  //don't use stored size info: leave dialog same size
#define RWPF_NOMOVE 	2  //don't use stored position
#define RWPF_NOACTIVATE 4  //show but don't activate v0.3.3.0+
#define RWPF_HIDDEN		8  //make it hidden

EXTERN_C MIR_CORE_DLL(int) Utils_RestoreWindowPosition(HWND hwnd, MCONTACT hContact, const char *szModule, const char *szNamePrefix, int flags = 0);

__forceinline int Utils_RestoreWindowPositionNoSize(HWND hwnd, MCONTACT hContact, const char *szModule, const char *szNamePrefix)
{	return Utils_RestoreWindowPosition(hwnd, hContact, szModule, szNamePrefix, RWPF_NOSIZE);
}
__forceinline int Utils_RestoreWindowPositionNoMove(HWND hwnd, MCONTACT hContact, const char *szModule, const char *szNamePrefix)
{	return Utils_RestoreWindowPosition(hwnd, hContact, szModule, szNamePrefix, RWPF_NOMOVE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Moves a RECT inside screen if it is outside. It works with multiple monitors
// returns < 0 on error, 0 if not changed the rect, 1 if changed the rect

EXTERN_C MIR_CORE_DLL(int) Utils_AssertInsideScreen(RECT *rc);

/************************ Colour Picker Control (0.1.2.1+) **********************/

#define WNDCLASS_COLOURPICKER  "ColourPicker"

#define CPM_SETCOLOUR		   0x1000	  //lParam = new colour
#define CPM_GETCOLOUR		   0x1001	  //returns colour
#define CPM_SETDEFAULTCOLOUR   0x1002	  //lParam = default, used as first custom colour
#define CPM_GETDEFAULTCOLOUR   0x1003	  //returns colour
#define CPN_COLOURCHANGED	   1		  //sent through WM_COMMAND

/////////////////////////////////////////////////////////////////////////////////////////
// Gets the filter strings for use in the open file dialog
// See the MSDN under OPENFILENAME.lpstrFilter for the formatting
// An 'All Bitmaps' item is always first and 'All Files' is last.
// The returned string is already translated.

EXTERN_C MIR_CORE_DLL(HBITMAP) Bitmap_Load(const TCHAR *ptszFileName);
EXTERN_C MIR_CORE_DLL(void) Bitmap_GetFilter(TCHAR *dest, size_t destLen);

/////////////////////////////////////////////////////////////////////////////////////////
// Converts a path to a relative path
// Only saves as a relative path if the file is in the miranda directory (or
// sub directory)
//
// [pszSrc] is the path to convert and [pszOut] is the buffer that
// the new path is copied too. pszOut MUST be at least of the size MAX_PATH.
//
// [pszBase] is the folder that is treated as root for 'relativity'
// by default = path to miranda32.exe
//
// Returns number of chars copied.

#if defined( __cplusplus )
EXTERN_C MIR_CORE_DLL(int) PathToRelative(const char *pszSrc, char *pszOut, const char* pszBase = 0);
EXTERN_C MIR_CORE_DLL(int) PathToRelativeW(const wchar_t *pwszSrc, wchar_t *pwszOut, const wchar_t* pwszBase = 0);
#else
EXTERN_C MIR_CORE_DLL(int) PathToRelative(const char *pszSrc, char *pszOut, const char* pszBase);
EXTERN_C MIR_CORE_DLL(int) PathToRelativeW(const wchar_t *pwszSrc, wchar_t *pwszOut, const wchar_t* pwszBase);
#endif

#define PathToRelativeT PathToRelativeW

/////////////////////////////////////////////////////////////////////////////////////////
// Saves a path to a absolute path (from the miranda directory)
//
// [pszSrc] is the path to convert and [pszOut] is the buffer that
// the new path is copied too. pszOut MUST be of the size MAX_PATH.
//
// [pszBase] is the folder that is treated as root for 'relativity'
// by default = path to miranda32.exe
//
// Returns numbers of chars copied.

#if defined( __cplusplus )
EXTERN_C MIR_CORE_DLL(int) PathToAbsolute(const char *pszSrc, char *pszOut, const char* pszBase = 0);
EXTERN_C MIR_CORE_DLL(int) PathToAbsoluteW(const wchar_t *pwszSrc, wchar_t *pwszOut, const wchar_t* pwszBase = 0);
#else
EXTERN_C MIR_CORE_DLL(int) PathToAbsolute(const char *pszSrc, char *pszOut, const char* pszBase);
EXTERN_C MIR_CORE_DLL(int) PathToAbsoluteW(const wchar_t *pwszSrc, wchar_t *pwszOut, const wchar_t* pwszBase);
#endif

#define PathToAbsoluteT PathToAbsoluteW

/////////////////////////////////////////////////////////////////////////////////////////
// Creates a directory tree (even more than one directories levels are missing)
// Returns 0 on success or an error code otherwise

EXTERN_C MIR_CORE_DLL(int) CreateDirectoryTree(const char *pszDir);
EXTERN_C MIR_CORE_DLL(int) CreateDirectoryTreeW(const wchar_t *pwszDir);

#define CreateDirectoryTreeT CreateDirectoryTreeW

/////////////////////////////////////////////////////////////////////////////////////////
// Creates all subdirectories required to create a file with the file name given
// Returns 0 on success or an error code otherwise

EXTERN_C MIR_CORE_DLL(void) CreatePathToFile(char *wszFilePath);
EXTERN_C MIR_CORE_DLL(void) CreatePathToFileW(wchar_t *wszFilePath);

#define CreatePathToFileT CreatePathToFileW

/////////////////////////////////////////////////////////////////////////////////////////
// Checks if a file name is absolute or not
// returns TRUE if yes or FALSE if not

EXTERN_C MIR_CORE_DLL(int) PathIsAbsolute(const char *pSrc);
EXTERN_C MIR_CORE_DLL(int) PathIsAbsoluteW(const wchar_t *pSrc);

#define PathIsAbsoluteT PathIsAbsoluteW

/////////////////////////////////////////////////////////////////////////////////////////
// Generates Random number of any length
// wParam = size - length of the random number to generate
// lParam = (LPARAM)(char*)pszArray - pointer to array to fill with random number
// Always returns 0

#define MS_UTILS_GETRANDOM "Utils/GetRandom"

/////////////////////////////////////////////////////////////////////////////////////////
// Replace variables in text
// wParam = (char*/TCHAR*/WCHAR*)string (depends on RVF_UNICODE/RVF_TCHAR flag)
// lParam = (REPLACEVARSDATA *) data about variables, item with key = 0 terminates the list
// returns new string, use mir_free to destroy

// variables known by the core:
// ----------------------------
// %miranda_profilesdir%  -> same as MS_DB_GETPROFILEPATH, base folder for all profiles
// %miranda_userdata% -> the active profile folder (home of the .dat file and all
//                       profile data)
// %miranda_path%     -> home path of the miranda installation (installation path
//                       of miranda32/64.exe
// %miranda_profilename% -> Name of the profile in use. Essentially, the name of the
//                          .dat file without file name extension. Also: the folder name
//                          relative to %miranda_profilesdir% where all profile data is stored.
// %miranda_logpath%     -> base folder for log files. This is \Logs relative to the
//                          current profile folder.
// %miranda_avatarcache% -> base folder for all protocol avatars. internal use only.
//
// the following variables operate on contacts. REPLACEVARSDATA::hContact must be
// supplied by the caller.
//
// %nick%                -> a contact nick name.
// %proto%               -> internal protocol name for a given contact. NOT the user-
//                          defined account name.
// %accountname%         -> user-defined account name for a given contact.
// %userid%              -> Unique ID for a given contact (UIN, JID etc.)

// the following variables are system variables - unrelated to miranda profiles.

// %appdata%			-> same as %APPDATA% environment variable.
// %destkop%            -> location of the desktop folder in a user's profile.
// %mydocuments%        -> location of the "My Documents" shell folder.

typedef struct
{
	union
	{
		TCHAR *lptzKey;
		char *lpszKey;
		WCHAR *lpwzKey;
	};
	union
	{
		TCHAR *lptzValue;
		char *lpszValue;
		WCHAR *lpwzValue;
	};
} REPLACEVARSARRAY;

typedef struct
{
	int cbSize;
	DWORD dwFlags;
	MCONTACT hContact;
	REPLACEVARSARRAY *variables;
} REPLACEVARSDATA;

#define RVF_UNICODE	1
#ifdef _UNICODE
	#define RVF_TCHAR	RVF_UNICODE
#else
	#define RVF_TCHAR	0
#endif

#define MS_UTILS_REPLACEVARS "Utils/ReplaceVars"

__forceinline char* Utils_ReplaceVars(const char *szData) {
	REPLACEVARSDATA dat = { sizeof(dat) };
	return (char*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)szData, (LPARAM)&dat);
}
__forceinline TCHAR* Utils_ReplaceVarsT(const TCHAR *szData) {
	REPLACEVARSDATA dat = { sizeof(dat), RVF_TCHAR };
	return (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)szData, (LPARAM)&dat);
}

#if defined(__cplusplus)
	#if !defined(M_SYSTEM_CPP_H__)
		#include "m_system_cpp.h"
	#endif

	struct VARS : public ptrA
	{
		__forceinline VARS(const char *str) :
			ptrA( Utils_ReplaceVars(str))
			{}
	};

	struct VARST : public ptrT
	{
		__forceinline VARST(const TCHAR *str) :
			ptrT( Utils_ReplaceVarsT(str))
			{}
	};
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// one field form

#define ESF_MULTILINE 1
#define ESF_COMBO     2
#define ESF_RICHEDIT  3
#define ESF_PASSWORD  4

typedef struct
{
	int     cbSize;         // structure size
	int     type;           // one of ESF_* constants
	LPCSTR  szModuleName;   // module name to save window size and combobox strings
	LPCSTR  szDataPrefix;   // prefix for stored database variables
	LPCTSTR caption;        // window caption
	union {
		LPCTSTR ptszInitVal; // initial value (note: the core DOES NOT free it)
		LPTSTR  ptszResult;  // result entered (must be freed via mir_free)
	};
	int     recentCount;    // number of combobox strings to store
	int     timeout;        // timeout for the form auto-close
}
ENTER_STRING;

/////////////////////////////////////////////////////////////////////////////////////////
// enters one string
// wParam = 0 (unused)
// lParam = ENTER_STRING* (form description)
// returns TRUE on pressing OK or FALSE if Cancel was pressed

#define MS_UTILS_ENTERSTRING "Utils/EnterString"

__forceinline BOOL EnterString(ENTER_STRING *pForm)
{
	return (BOOL)CallService(MS_UTILS_ENTERSTRING, 0, (LPARAM)pForm);
}

#endif // M_UTILS_H__

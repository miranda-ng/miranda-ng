
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

/* Resizes a dialog by calling a custom routine to move the individual
controls   v0.1.0.1+
wParam = 0
lParam = (LPARAM)(UTILRESIZEDIALOG*)&urd
Returns 0 on success, or nonzero on failure
Does not support dialogtemplateex dialog boxes, and will return failure if you
try to resize one
The dialog itself should have been resized prior to calling this service
pfnResizer is called once for each control in the dialog
pfnResizer should return a combination of one rd_anchorx_ and one rd_anchory
constant
*/
typedef struct {
	int cbSize;
	UINT wId;				//control ID
	RECT rcItem;			//original control rectangle, relative to dialog
							//modify in-place to specify the new position
	SIZE dlgOriginalSize;	//size of dialog client area in template
	SIZE dlgNewSize;		//current size of dialog client area
} UTILRESIZECONTROL;
typedef int (*DIALOGRESIZERPROC)(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL *urc);
typedef struct {
	int cbSize;
	HWND hwndDlg;
	HINSTANCE hInstance;	//module containing the dialog template
	LPCSTR lpTemplate;		//dialog template
	LPARAM lParam;			//caller-defined
	DIALOGRESIZERPROC pfnResizer;
} UTILRESIZEDIALOG;
#define RD_ANCHORX_CUSTOM	0	//function did everything required to the x axis, do no more processing
#define RD_ANCHORX_LEFT 	0	//move the control to keep it constant distance from the left edge of the dialog
#define RD_ANCHORX_RIGHT	1	//move the control to keep it constant distance from the right edge of the dialog
#define RD_ANCHORX_WIDTH	2	//size the control to keep it constant distance from both edges of the dialog
#define RD_ANCHORX_CENTRE	4	//move the control to keep it constant distance from the centre of the dialog
#define RD_ANCHORY_CUSTOM	0
#define RD_ANCHORY_TOP		0
#define RD_ANCHORY_BOTTOM	8
#define RD_ANCHORY_HEIGHT	16
#define RD_ANCHORY_CENTRE	32
#define MS_UTILS_RESIZEDIALOG	 "Utils/ResizeDialog"

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

/******************************* Window lists *******************************/

/////////////////////////////////////////////////////////////////////////////////////////
// allocates a window list
// wParam = lParam = 0 (unused)
// returns a handle to the new window list

#define MS_UTILS_ALLOCWINDOWLIST "Utils/AllocWindowList"
__forceinline HANDLE WindowList_Create(void)
{	return (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// destroys a window list
// wParam = (HANDLE) window list handle
// lParam = 0 (unused)
// returns a handle to the new window list
#define MS_UTILS_DESTROYWINDOWLIST "Utils/DestroyWindowList"
__forceinline HANDLE WindowList_Destroy(HANDLE hList)
{	return (HANDLE)CallService(MS_UTILS_DESTROYWINDOWLIST, (WPARAM)hList, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// adds a window to the specified window list
// wParam = 0
// lParam = (LPARAM)(WINDOWLISTENTRY*)&wle
// returns 0 on success, nonzero on failure

typedef struct {
	HANDLE hList;
	HWND hwnd;
	MCONTACT hContact;
} WINDOWLISTENTRY;
#define MS_UTILS_ADDTOWINDOWLIST "Utils/AddToWindowList"
__forceinline INT_PTR WindowList_Add(HANDLE hList, HWND hwnd, MCONTACT hContact) {
	WINDOWLISTENTRY wle;
	wle.hList = hList; wle.hwnd = hwnd; wle.hContact = hContact;
	return CallService(MS_UTILS_ADDTOWINDOWLIST, 0, (LPARAM)&wle);
}

/////////////////////////////////////////////////////////////////////////////////////////
// removes a window from the specified window list
// wParam = (WPARAM)(HANDLE)hList
// lParam = (LPARAM)(HWND)hwnd
// returns 0 on success, nonzero on failure

#define MS_UTILS_REMOVEFROMWINDOWLIST "Utils/RemoveFromWindowList"
__forceinline INT_PTR WindowList_Remove(HANDLE hList, HWND hwnd) {
	return CallService(MS_UTILS_REMOVEFROMWINDOWLIST, (WPARAM)hList, (LPARAM)hwnd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// finds a window given the hContact
// wParam = (WPARAM)(HANDLE)hList
// lParam = (MCONTACT)hContact
// returns the window handle on success, or NULL on failure

#define MS_UTILS_FINDWINDOWINLIST "Utils/FindWindowInList"
__forceinline HWND WindowList_Find(HANDLE hList, MCONTACT hContact) {
	return (HWND)CallService(MS_UTILS_FINDWINDOWINLIST, (WPARAM)hList, hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////
// sends a message to all windows in a list using SendMessage
// wParam = (WPARAM)(HANDLE)hList
// lParam = (LPARAM)(MSG*)&msg
// returns 0 on success, nonzero on failure
// Only msg.message, msg.wParam and msg.lParam are used

#define MS_UTILS_BROADCASTTOWINDOWLIST "Utils/BroadcastToWindowList"
__forceinline INT_PTR WindowList_Broadcast(HANDLE hList, UINT message, WPARAM wParam, LPARAM lParam) {
	MSG msg;
	msg.message = message; msg.wParam = wParam; msg.lParam = lParam;
	return CallService(MS_UTILS_BROADCASTTOWINDOWLIST, (WPARAM)hList, (LPARAM)&msg);
}

/////////////////////////////////////////////////////////////////////////////////////////
// sends a message to all windows in a list using PostMessage
// wParam = (WPARAM)(HANDLE)hList
// lParam = (LPARAM)(MSG*)&msg
// returns 0 on success, nonzero on failure
// Only msg.message, msg.wParam and msg.lParam are used

#define MS_UTILS_BROADCASTTOWINDOWLIST_ASYNC "Utils/BroadcastToWindowListAsync"

__forceinline INT_PTR WindowList_BroadcastAsync(HANDLE hList, UINT message, WPARAM wParam, LPARAM lParam) {
	MSG msg;
	msg.message = message; msg.wParam = wParam; msg.lParam = lParam;
	return CallService(MS_UTILS_BROADCASTTOWINDOWLIST_ASYNC, (WPARAM)hList, (LPARAM)&msg);
}

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

/***************************** Window Position Saving ***************************/

/////////////////////////////////////////////////////////////////////////////////////////
// saves the position of a window in the database   v0.1.1.0+
// wParam = 0
// lParam = (LPARAM)(SAVEWINDOWPOS*)&swp
// returns 0 on success, nonzero on failure
typedef struct {
	HWND hwnd;
	MCONTACT hContact;
	const char *szModule;		//module name to store the setting in
	const char *szNamePrefix;	//text to prefix on "x", "width", etc, to form setting names
} SAVEWINDOWPOS;
#define MS_UTILS_SAVEWINDOWPOSITION  "Utils/SaveWindowPos"
__forceinline INT_PTR Utils_SaveWindowPosition(HWND hwnd, MCONTACT hContact, const char *szModule, const char *szNamePrefix) {
	SAVEWINDOWPOS swp;
	swp.hwnd = hwnd; swp.hContact = hContact; swp.szModule = szModule; swp.szNamePrefix = szNamePrefix;
	return CallService(MS_UTILS_SAVEWINDOWPOSITION, 0, (LPARAM)&swp);
}

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
#define MS_UTILS_RESTOREWINDOWPOSITION	"Utils/RestoreWindowPos"
__forceinline INT_PTR Utils_RestoreWindowPositionEx(HWND hwnd, int flags, MCONTACT hContact, const char *szModule, const char *szNamePrefix) {
	SAVEWINDOWPOS swp;
	swp.hwnd = hwnd; swp.hContact = hContact; swp.szModule = szModule; swp.szNamePrefix = szNamePrefix;
	return CallService(MS_UTILS_RESTOREWINDOWPOSITION, flags, (LPARAM)&swp);
}
__forceinline INT_PTR Utils_RestoreWindowPosition(HWND hwnd, MCONTACT hContact, const char *szModule, const char *szNamePrefix) {
	return Utils_RestoreWindowPositionEx(hwnd, 0, hContact, szModule, szNamePrefix);
}
__forceinline INT_PTR Utils_RestoreWindowPositionNoSize(HWND hwnd, MCONTACT hContact, const char *szModule, const char *szNamePrefix) {
	return Utils_RestoreWindowPositionEx(hwnd, RWPF_NOSIZE, hContact, szModule, szNamePrefix);
}
__forceinline INT_PTR Utils_RestoreWindowPositionNoMove(HWND hwnd, MCONTACT hContact, const char *szModule, const char *szNamePrefix) {
	return Utils_RestoreWindowPositionEx(hwnd, RWPF_NOMOVE, hContact, szModule, szNamePrefix);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Moves a RECT inside screen if it is outside.It works with multiple monitors	 v0.9.0.4+
// wParam = RECT *
// lParam = 0
// returns <0 on error, 0 if not changed the rect, 1 if changed the rect

#define MS_UTILS_ASSERTINSIDESCREEN	"Utils/AssertInsideScreen"
__forceinline INT_PTR Utils_AssertInsideScreen(RECT *rc) {
	return CallService(MS_UTILS_ASSERTINSIDESCREEN, (WPARAM)rc, 0);
}

/************************ Colour Picker Control (0.1.2.1+) **********************/

#define WNDCLASS_COLOURPICKER  "ColourPicker"

#define CPM_SETCOLOUR		   0x1000	  //lParam = new colour
#define CPM_GETCOLOUR		   0x1001	  //returns colour
#define CPM_SETDEFAULTCOLOUR   0x1002	  //lParam = default, used as first custom colour
#define CPM_GETDEFAULTCOLOUR   0x1003	  //returns colour
#define CPN_COLOURCHANGED	   1		  //sent through WM_COMMAND

/***************************** Bitmap Filter (0.1.2.1+) *************************/

/////////////////////////////////////////////////////////////////////////////////////////
// Loads a bitmap								v0.1.2.1+
// wParam = 0
// lParam = (LPARAM)(const char*)filename
// returns HBITMAP on success, NULL on failure
// This function uses OleLoadPicturePath() so supports BMP, JPEG and GIF. It may
// support PNG on future versions of Windows (or XP for that matter)
// For speed, if the file extension is .bmp or .rle it'll use LoadImage() so as
// to avoid the big lag loading OLE.
// Remember to DeleteObject() when you're done

#define MS_UTILS_LOADBITMAP "Utils/LoadBitmap"
#define MS_UTILS_LOADBITMAPW "Utils/LoadBitmapW"

#ifdef _UNICODE
	#define MS_UTILS_LOADBITMAPT MS_UTILS_LOADBITMAPW
#else
	#define MS_UTILS_LOADBITMAPT MS_UTILS_LOADBITMAP
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// Gets the filter strings for use in the open file dialog
// See the MSDN under OPENFILENAME.lpstrFilter for the formatting
// An 'All Bitmaps' item is always first and 'All Files' is last.
// The returned string is already translated.

EXTERN_C MIR_CORE_DLL(void) BmpFilterGetStrings(TCHAR *dest, size_t destLen);

/////////////////////////////////////////////////////////////////////////////////////////
// Saves a path to a relative path (from the miranda directory)
// Only saves as a relative path if the file is in the miranda directory (or
// sub directory)
// wParam = (WPARAM)(char*)pszPath
// lParam = (LPARAM)(char*)pszNewPath
// pszPath is the path to convert and pszNewPath is the buffer that
// the new path is copied too.  pszNewPath MUST be of the size MAX_PATH.
// Returns numbers of chars copied.
// Unicode version is available since 0.6.2

#define MS_UTILS_PATHTORELATIVE "Utils/PathToRelative"
#define MS_UTILS_PATHTORELATIVEW "Utils/PathToRelativeW"

#ifdef _UNICODE
	#define MS_UTILS_PATHTORELATIVET MS_UTILS_PATHTORELATIVEW
#else
	#define MS_UTILS_PATHTORELATIVET MS_UTILS_PATHTORELATIVE
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// Saves a path to a absolute path (from the miranda directory)
// wParam = (WPARAM)(char*)pszPath
// lParam = (LPARAM)(char*)pszNewPath
// pszPath is the path to convert and pszNewPath is the buffer that
// the new path is copied too.  pszNewPath MUST be of the size MAX_PATH.
// Returns numbers of chars copied.
// Unicode version is available since 0.6.2

#define MS_UTILS_PATHTOABSOLUTE "Utils/PathToAbsolute"
#define MS_UTILS_PATHTOABSOLUTEW "Utils/PathToAbsoluteW"

#ifdef _UNICODE
	#define MS_UTILS_PATHTOABSOLUTET MS_UTILS_PATHTOABSOLUTEW
#else
	#define MS_UTILS_PATHTOABSOLUTET MS_UTILS_PATHTOABSOLUTE
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// Creates a directory tree (even more than one directories levels are missing) 0.7.0+
// wParam = 0 (unused)
// lParam = (LPARAM)(char*)pszPath - directory to be created
// Returns 0 on success error code otherwise
// Unicode version is available since 0.7.0

#define MS_UTILS_CREATEDIRTREE "Utils/CreateDirTree"
#define MS_UTILS_CREATEDIRTREEW "Utils/CreateDirTreeW"

#ifdef _UNICODE
	#define MS_UTILS_CREATEDIRTREET MS_UTILS_CREATEDIRTREEW
#else
	#define MS_UTILS_CREATEDIRTREET MS_UTILS_CREATEDIRTREE
#endif

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

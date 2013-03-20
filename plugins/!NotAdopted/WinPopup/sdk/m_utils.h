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

#ifndef M_UTILS_H__
#define M_UTILS_H__ 1

#ifdef _MSC_VER
	#pragma warning(disable:4201)
#endif

#include <stdio.h>

#if !defined(M_SYSTEM_H__)
#include "m_system.h"
#endif

//this entire module is v0.1.0.1+
//this module cannot be redefined by a plugin, because it's not useful for it
//to be possible
//There are some more utility services in the database for dealing with time
//and simple string scrambling, but they are very db-orientated

/* Opens a URL in the user's default web browser   v0.1.0.1+
wParam=bOpenInNewWindow
lParam=(LPARAM)(const char*)szUrl
returns 0 always
bOpenInNewWindow should be zero to open the URL in the browser window the user
last used, or nonzero to open in a new browser window. If there's no browser
running, one will be opened to show the URL.
*/
#define MS_UTILS_OPENURL	"Utils/OpenURL"

/* Resizes a dialog by calling a custom routine to move the individual
controls   v0.1.0.1+
wParam=0
lParam=(LPARAM)(UTILRESIZEDIALOG*)&urd
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
typedef int (*DIALOGRESIZERPROC)(HWND hwndDlg,LPARAM lParam,UTILRESIZECONTROL *urc);
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
wParam=countryId
lParam=0
Returns a pointer to the string containing the country name on success,
or NULL on failure
*/
#define MS_UTILS_GETCOUNTRYBYNUMBER   "Utils/GetCountryByNumber"

/* Gets the full list of country IDs	 v0.1.2.0+
wParam=(WPARAM)(int*)piCount
lParam=(LPARAM)(struct CountryListEntry**)ppList
Returns 0 always
Neither wParam nor lParam can be NULL.
The list is sorted alphabetically by country name, on the assumption that it's
quicker to search numbers out of order than it is to search names out of order
*/
struct CountryListEntry {
	int id;
	const char *szName;
};
#define MS_UTILS_GETCOUNTRYLIST    "Utils/GetCountryList"

/******************************* Window lists *******************************/

//allocate a window list   v0.1.0.1+
//wParam=lParam=0
//returns a handle to the new window list
#define MS_UTILS_ALLOCWINDOWLIST "Utils/AllocWindowList"

//adds a window to the specified window list   v0.1.0.1+
//wParam=0
//lParam=(LPARAM)(WINDOWLISTENTRY*)&wle
//returns 0 on success, nonzero on failure
typedef struct {
	HANDLE hList;
	HWND hwnd;
	HANDLE hContact;
} WINDOWLISTENTRY;
#define MS_UTILS_ADDTOWINDOWLIST "Utils/AddToWindowList"
__inline static INT_PTR WindowList_Add(HANDLE hList,HWND hwnd,HANDLE hContact) {
	WINDOWLISTENTRY wle;
	wle.hList=hList; wle.hwnd=hwnd; wle.hContact=hContact;
	return CallService(MS_UTILS_ADDTOWINDOWLIST,0,(LPARAM)&wle);
}
//removes a window from the specified window list  v0.1.0.1+
//wParam=(WPARAM)(HANDLE)hList
//lParam=(LPARAM)(HWND)hwnd
//returns 0 on success, nonzero on failure
#define MS_UTILS_REMOVEFROMWINDOWLIST "Utils/RemoveFromWindowList"
__inline static INT_PTR WindowList_Remove(HANDLE hList,HWND hwnd) {
	return CallService(MS_UTILS_REMOVEFROMWINDOWLIST,(WPARAM)hList,(LPARAM)hwnd);
}

//finds a window given the hContact  v0.1.0.1+
//wParam=(WPARAM)(HANDLE)hList
//lParam=(WPARAM)(HANDLE)hContact
//returns the window handle on success, or NULL on failure
#define MS_UTILS_FINDWINDOWINLIST "Utils/FindWindowInList"
__inline static HWND WindowList_Find(HANDLE hList,HANDLE hContact) {
	return (HWND)CallService(MS_UTILS_FINDWINDOWINLIST,(WPARAM)hList,(LPARAM)hContact);
}

//broadcasts a message to all windows in a list  v0.1.0.1+
//wParam=(WPARAM)(HANDLE)hList
//lParam=(LPARAM)(MSG*)&msg
//returns 0 on success, nonzero on failure
//Only msg.message, msg.wParam and msg.lParam are used
#define MS_UTILS_BROADCASTTOWINDOWLIST "Utils/BroadcastToWindowList"
__inline static INT_PTR WindowList_Broadcast(HANDLE hList,UINT message,WPARAM wParam,LPARAM lParam) {
	MSG msg;
	msg.message=message; msg.wParam=wParam; msg.lParam=lParam;
	return CallService(MS_UTILS_BROADCASTTOWINDOWLIST,(WPARAM)hList,(LPARAM)&msg);
}

/*
	Description: Broadcast a message to all windows in the given list using PostMessage()
	Version: 0.3.0.0+
	Inline helper: WindowList_BroadcastAsync

	wParam=(WPARAM)(HANDLE)hList
	lParam=(LPARAM)(MSG*)&msg

	Returns 0 on success, nonzero on failure, this service does not fail, even if PostMessage() fails for whatever reason

*/
#define MS_UTILS_BROADCASTTOWINDOWLIST_ASYNC "Utils/BroadcastToWindowListAsync"

__inline static INT_PTR WindowList_BroadcastAsync(HANDLE hList,UINT message,WPARAM wParam,LPARAM lParam) {
	MSG msg;
	msg.message=message; msg.wParam=wParam; msg.lParam=lParam;
	return CallService(MS_UTILS_BROADCASTTOWINDOWLIST_ASYNC,(WPARAM)hList,(LPARAM)&msg);
}

/***************************** Hyperlink windows ********************************/

//there aren't any services here, because you don't need them.
#define WNDCLASS_HYPERLINK	 _T("Hyperlink")
//the control will obey the SS_LEFT (0), SS_CENTER (1), and SS_RIGHT (2) styles
//the control will send STN_CLICKED via WM_COMMAND when the link itself is clicked

// Use this in a SendMessage to set the color of the url when control is enabled
// wParam=DWORD color
// lParam=not used
#define HLK_SETENABLECOLOUR	 (WM_USER+101) // added in 0.3.1
// Use this in a SendMessage to set the color of the url when control is disabled
// wParam=DWORD color
// lParam=not used
#define HLK_SETDISABLECOLOUR (WM_USER+102) // added in 0.3.1

/***************************** Window Position Saving ***************************/

//saves the position of a window in the database   v0.1.1.0+
//wParam=0
//lParam=(LPARAM)(SAVEWINDOWPOS*)&swp
//returns 0 on success, nonzero on failure
typedef struct {
	HWND hwnd;
	HANDLE hContact;
	const char *szModule;		//module name to store the setting in
	const char *szNamePrefix;	//text to prefix on "x", "width", etc, to form setting names
} SAVEWINDOWPOS;
#define MS_UTILS_SAVEWINDOWPOSITION  "Utils/SaveWindowPos"
__inline static INT_PTR Utils_SaveWindowPosition(HWND hwnd,HANDLE hContact,const char *szModule,const char *szNamePrefix) {
	SAVEWINDOWPOS swp;
	swp.hwnd=hwnd; swp.hContact=hContact; swp.szModule=szModule; swp.szNamePrefix=szNamePrefix;
	return CallService(MS_UTILS_SAVEWINDOWPOSITION,0,(LPARAM)&swp);
}

//restores the position of a window from the database	 v0.1.1.0+
//wParam=flags
//lParam=(LPARAM)(SAVEWINDOWPOS*)&swp
//returns 0 on success, nonzero on failure
//if no position was found in the database, the function returns 1 and does
//nothing
//the NoSize version won't use stored size information: the window is left the
//same size.
#define RWPF_NOSIZE 	1  //don't use stored size info: leave dialog same size
#define RWPF_NOMOVE 	2  //don't use stored position
#define RWPF_NOACTIVATE 4  //show but don't activate v0.3.3.0+
#define MS_UTILS_RESTOREWINDOWPOSITION	"Utils/RestoreWindowPos"
__inline static INT_PTR Utils_RestoreWindowPosition(HWND hwnd,HANDLE hContact,const char *szModule,const char *szNamePrefix) {
	SAVEWINDOWPOS swp;
	swp.hwnd=hwnd; swp.hContact=hContact; swp.szModule=szModule; swp.szNamePrefix=szNamePrefix;
	return CallService(MS_UTILS_RESTOREWINDOWPOSITION,0,(LPARAM)&swp);
}
__inline static INT_PTR Utils_RestoreWindowPositionNoSize(HWND hwnd,HANDLE hContact,const char *szModule,const char *szNamePrefix) {
	SAVEWINDOWPOS swp;
	swp.hwnd=hwnd; swp.hContact=hContact; swp.szModule=szModule; swp.szNamePrefix=szNamePrefix;
	return CallService(MS_UTILS_RESTOREWINDOWPOSITION,RWPF_NOSIZE,(LPARAM)&swp);
}
__inline static INT_PTR Utils_RestoreWindowPositionNoMove(HWND hwnd,HANDLE hContact,const char *szModule,const char *szNamePrefix) {
	SAVEWINDOWPOS swp;
	swp.hwnd=hwnd; swp.hContact=hContact; swp.szModule=szModule; swp.szNamePrefix=szNamePrefix;
	return CallService(MS_UTILS_RESTOREWINDOWPOSITION,RWPF_NOMOVE,(LPARAM)&swp);
}

//Moves a RECT inside screen if it is outside.It works with multiple monitors	 v0.9.0.4+
//wParam=RECT *
//lParam=0
//returns <0 on error, 0 if not changed the rect, 1 if changed the rect
#define MS_UTILS_ASSERTINSIDESCREEN	"Utils/AssertInsideScreen"
__inline static INT_PTR Utils_AssertInsideScreen(RECT *rc) {
	return CallService(MS_UTILS_ASSERTINSIDESCREEN,(WPARAM)rc,0);
}

/************************ Colour Picker Control (0.1.2.1+) **********************/

#define WNDCLASS_COLOURPICKER  _T("ColourPicker")

#define CPM_SETCOLOUR		   0x1000	  //lParam=new colour
#define CPM_GETCOLOUR		   0x1001	  //returns colour
#define CPM_SETDEFAULTCOLOUR   0x1002	  //lParam=default, used as first custom colour
#define CPM_GETDEFAULTCOLOUR   0x1003	  //returns colour
#define CPN_COLOURCHANGED	   1		  //sent through WM_COMMAND

/***************************** Bitmap Filter (0.1.2.1+) *************************/

//Loads a bitmap								v0.1.2.1+
//wParam=0
//lParam=(LPARAM)(const char*)filename
//returns HBITMAP on success, NULL on failure
//This function uses OleLoadPicturePath() so supports BMP, JPEG and GIF. It may
//support PNG on future versions of Windows (or XP for that matter)
//For speed, if the file extension is .bmp or .rle it'll use LoadImage() so as
//to avoid the big lag loading OLE.
//Remember to DeleteObject() when you're done
#define MS_UTILS_LOADBITMAP   "Utils/LoadBitmap"

//Gets the filter strings for use in the open file dialog	   v0.1.2.1+
//wParam=cbLengthOfBuffer
//lParam=(LPARAM)(char*)pszBuffer
//Returns 0 on success, nonzero on failure
//See the MSDN under OPENFILENAME.lpstrFilter for the formatting
//An 'All Bitmaps' item is always first and 'All Files' is last.
//The returned string is already translated.
#define MS_UTILS_GETBITMAPFILTERSTRINGS  "Utils/GetBitmapFilterStrings"

//Saves a path to a relative path (from the miranda directory)
//Only saves as a relative path if the file is in the miranda directory (or
//sub directory)
//wParam=(WPARAM)(char*)pszPath
//lParam=(LPARAM)(char*)pszNewPath
//pszPath is the path to convert and pszNewPath is the buffer that
//the new path is copied too.  pszNewPath MUST be of the size MAX_PATH.
//Returns numbers of chars copied.
//Unicode version is available since 0.6.2
#define MS_UTILS_PATHTORELATIVE "Utils/PathToRelative"

//Saves a path to a absolute path (from the miranda directory)
//wParam=(WPARAM)(char*)pszPath
//lParam=(LPARAM)(char*)pszNewPath
//pszPath is the path to convert and pszNewPath is the buffer that
//the new path is copied too.  pszNewPath MUST be of the size MAX_PATH.
//Returns numbers of chars copied.
//Unicode version is available since 0.6.2
#define MS_UTILS_PATHTOABSOLUTE "Utils/PathToAbsolute"

//Creates a directory tree (even more than one directories levels are missing) 0.7.0+
//wParam=0 (unused)
//lParam=(LPARAM)(char*)pszPath - directory to be created
//Always returns 0
//Unicode version is available since 0.7.0
#define MS_UTILS_CREATEDIRTREE "Utils/CreateDirTree"

// Generates Random number of any length
//wParam=size - length of the random number to generate
//lParam=(LPARAM)(char*)pszArray - pointer to array to fill with random number
//Always returns 0
#define MS_UTILS_GETRANDOM "Utils/GetRandom"

//Replace variables in text
//wParam=(char*/TCHAR*/WCHAR*)string (depends on RVF_UNICODE/RVF_TCHAR flag)
//lParam=(REPLACEVARSDATA *) data about variables, item with key=0 terminates the list
//returns new string, use mir_free to destroy
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
	HANDLE hContact;
	REPLACEVARSARRAY *variables;
} REPLACEVARSDATA;

#define RVF_UNICODE	1
#ifdef _UNICODE
	#define RVF_TCHAR	RVF_UNICODE
#else
	#define RVF_TCHAR	0
#endif

#define MS_UTILS_REPLACEVARS "Utils/ReplaceVars"

__inline static char* Utils_ReplaceVars(const char *szData) {
	REPLACEVARSDATA dat = {0};
	dat.cbSize = sizeof(dat);
	return (char*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)szData, (LPARAM)&dat);
}
__inline static TCHAR* Utils_ReplaceVarsT(const TCHAR *szData) {
	REPLACEVARSDATA dat = {0};
	dat.cbSize = sizeof(dat);
    dat.dwFlags = RVF_TCHAR;
	return (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)szData, (LPARAM)&dat);
}
#ifdef _UNICODE
	#define MS_UTILS_PATHTORELATIVEW  "Utils/PathToRelativeW"
	#define MS_UTILS_PATHTOABSOLUTEW  "Utils/PathToAbsoluteW"
	#define MS_UTILS_CREATEDIRTREEW   "Utils/CreateDirTreeW"

	#define MS_UTILS_PATHTORELATIVET MS_UTILS_PATHTORELATIVEW
	#define MS_UTILS_PATHTOABSOLUTET MS_UTILS_PATHTOABSOLUTEW
	#define MS_UTILS_CREATEDIRTREET  MS_UTILS_CREATEDIRTREEW
#else
	#define MS_UTILS_PATHTORELATIVET MS_UTILS_PATHTORELATIVE
	#define MS_UTILS_PATHTOABSOLUTET MS_UTILS_PATHTOABSOLUTE
	#define MS_UTILS_CREATEDIRTREET  MS_UTILS_CREATEDIRTREE
#endif

/*
	MD5 interface. 0.7.0.12

	Contains functions for md5 handling
*/
/* Define the state of the MD5 Algorithm. */
typedef unsigned char mir_md5_byte_t; /* 8-bit byte */
typedef unsigned int mir_md5_word_t; /* 32-bit word */

typedef struct mir_md5_state_s {
	mir_md5_word_t count[2];  /* message length in bits, lsw first */
	mir_md5_word_t abcd[4];    /* digest buffer */
	mir_md5_byte_t buf[64];    /* accumulate block */
} mir_md5_state_t;

struct MD5_INTERFACE
{
	int cbSize;
	void (*md5_init) (mir_md5_state_t *pms);
	void (*md5_append) (mir_md5_state_t *pms, const mir_md5_byte_t *data, int nbytes);
	void (*md5_finish) (mir_md5_state_t *pms, mir_md5_byte_t digest[16]);
	void (*md5_hash) (const mir_md5_byte_t *data, int len, mir_md5_byte_t digest[16]);
};

#define MS_SYSTEM_GET_MD5I	"Miranda/System/GetMD5I"

static __inline INT_PTR mir_getMD5I( struct MD5_INTERFACE* dest )
{
	dest->cbSize = sizeof(*dest);
	return CallService( MS_SYSTEM_GET_MD5I, 0, (LPARAM)dest );
}

extern struct MD5_INTERFACE md5i;

#define mir_md5_init(A) 		md5i.md5_init(A)
#define mir_md5_append(A,B,C)	md5i.md5_append(A,B,C)
#define mir_md5_finish(A,B) 	md5i.md5_finish(A,B)
#define mir_md5_hash(A,B,C) 	md5i.md5_hash(A,B,C)

/*
	SHA1 interface. 0.7.0.12

	Contains functions for SHA1 handling
*/
typedef unsigned char mir_sha1_byte_t;
typedef unsigned long mir_sha1_long_t;
#define MIR_SHA1_HASH_SIZE 20

typedef struct {
  mir_sha1_long_t H[5];
  mir_sha1_long_t W[80];
  int lenW;
  mir_sha1_long_t sizeHi,sizeLo;
} mir_sha1_ctx;

struct SHA1_INTERFACE
{
	int cbSize;
	void (*sha1_init) (mir_sha1_ctx *ctx);
	void (*sha1_append) (mir_sha1_ctx *ctx, mir_sha1_byte_t *dataIn, int len);
	void (*sha1_finish) (mir_sha1_ctx *ctx, mir_sha1_byte_t hashout[20]);
	void (*sha1_hash) (mir_sha1_byte_t *dataIn, int len, mir_sha1_byte_t hashout[20]);
};

#define MS_SYSTEM_GET_SHA1I  "Miranda/System/GetSHA1I"

static __inline INT_PTR mir_getSHA1I( struct SHA1_INTERFACE* dest )
{
	dest->cbSize = sizeof(*dest);
	return CallService( MS_SYSTEM_GET_SHA1I, 0, (LPARAM)dest );
}

extern struct SHA1_INTERFACE sha1i;

#define mir_sha1_init(A)		 sha1i.sha1_init(A)
#define mir_sha1_append(A,B,C)	 sha1i.sha1_append(A,B,C)
#define mir_sha1_finish(A,B)	 sha1i.sha1_finish(A,B)
#define mir_sha1_hash(A,B,C)	 sha1i.sha1_hash(A,B,C)

// Added in 0.4.0.1
// Here are some string wrappers that are more safe than the win32 versions

#if MIRANDA_VER < 0x0700
static __inline int mir_snprintf(char *buffer, size_t count, const char* fmt, ...) {
	va_list va;
	int len;

	va_start(va, fmt);
	len = _vsnprintf(buffer, count-1, fmt, va);
	va_end(va);
	buffer[count-1] = 0;
	return len;
}

static __inline int mir_sntprintf(TCHAR *buffer, size_t count, const TCHAR* fmt, ...) {
	va_list va;
	int len;

	va_start(va, fmt);
	len = _vsntprintf(buffer, count-1, fmt, va);
	va_end(va);
	buffer[count-1] = 0;
	return len;
}

static __inline int mir_vsnprintf(char *buffer, size_t count, const char* fmt, va_list va) {
	int len;

	len = _vsnprintf(buffer, count-1, fmt, va);
	buffer[count-1] = 0;
	return len;
}

static __inline int mir_vsntprintf(TCHAR *buffer, size_t count, const TCHAR* fmt, va_list va) {
	int len;

	len = _vsntprintf(buffer, count-1, fmt, va);
	buffer[count-1] = 0;
	return len;
}
#endif

// allows to include TCHAR* strings into mir_snprintf and NetLib_Logf calls
#if defined( _UNICODE )
	#define TCHAR_STR_PARAM "%S"
#else
	#define TCHAR_STR_PARAM "%s"
#endif

#if MIRANDA_VER < 0x0700
static __inline wchar_t* mir_a2u_cp( const char* src, int codepage )
{
	int cbLen = MultiByteToWideChar( codepage, 0, src, -1, NULL, 0 );
	wchar_t* result = ( wchar_t* )mir_alloc( sizeof( wchar_t )*(cbLen+1));
	if ( result == NULL )
		return NULL;

	MultiByteToWideChar( codepage, 0, src, -1, result, cbLen );
	result[ cbLen ] = 0;
	return result;
}

static __inline wchar_t* mir_a2u( const char* src )
{
	return mir_a2u_cp( src, CallService("LangPack/GetCodePage", 0, 0 ));
}

static __inline char* mir_u2a_cp( const wchar_t* src, int codepage )
{
	int cbLen = WideCharToMultiByte( codepage, 0, src, -1, NULL, 0, NULL, NULL );
	char* result = ( char* )mir_alloc( cbLen+1 );
	if ( result == NULL )
		return NULL;

	WideCharToMultiByte( codepage, 0, src, -1, result, cbLen, NULL, NULL );
	result[ cbLen ] = 0;
	return result;
}

static __inline char* mir_u2a( const wchar_t* src )
{
	return mir_u2a_cp( src, CallService("LangPack/GetCodePage", 0, 0 ));
}
#endif

#ifdef _UNICODE
	#define mir_t2a(s) mir_u2a(s)
	#define mir_a2t(s) mir_a2u(s)
	#define mir_t2u(s) mir_wstrdup(s)
	#define mir_u2t(s) mir_wstrdup(s)

	#define mir_t2a_cp(s,c) mir_u2a_cp(s,c)
	#define mir_a2t_cp(s,c) mir_a2u_cp(s,c)
	#define mir_t2u_cp(s,c) mir_wstrdup(s)
	#define mir_u2t_cp(s,c) mir_wstrdup(s)
#else
	#define mir_t2a(s) mir_strdup(s)
	#define mir_a2t(s) mir_strdup(s)
	#define mir_t2u(s) mir_a2u(s)
	#define mir_u2t(s) mir_u2a(s)

	#define mir_t2a_cp(s,c) mir_strdup(s)
	#define mir_a2t_cp(s,c) mir_strdup(s)
	#define mir_t2u_cp(s,c) mir_a2u_cp(s,c)
	#define mir_u2t_cp(s,c) mir_u2a_cp(s,c)
#endif

#endif // M_UTILS_H__

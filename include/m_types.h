/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)
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

#ifndef M_TYPES_H__
#define M_TYPES_H__ 1

///////////////////////////////////////////////////////////////////////////////
// Linux

#ifndef _MSC_VER

#include <wchar.h>
#include <netinet/in.h>

using namespace std;

#define CALLBACK
#define EXTERN_C extern "C"

#define PURE = 0
#define STDMETHOD(method)        virtual HRESULT method
#define STDMETHOD_(ret, method)  virtual ret method
#define STDMETHODIMP_(ret)       ret

#define FALSE 0
#define TRUE  1
#define CP_ACP 0
#define SW_HIDE 0
#define SW_SHOW 5
#define MAX_PATH 260
#define LR_SHARED 0x8000
#define LF_FACESIZE 32
#define _TRUNCATE size_t(-1)
#define INVALID_HANDLE_VALUE HANDLE(-1)

#define MB_OK              0x00000000L
#define MB_OKCANCEL        0x00000001L
#define MB_YESNOCANCEL     0x00000003L
#define MB_YESNO           0x00000004L
#define MB_RETRYCANCEL     0x00000005L

#define MB_ICONSTOP        0x00000010L
#define MB_ICONERROR       0x00000010L
#define MB_ICONQUESTION    0x00000020L
#define MB_ICONEXCLAMATION 0x00000030L
#define MB_ICONWARNING     0x00000030L
#define MB_ICONINFORMATION 0x00000040L

#define IDABORT    3
#define IDCANCEL   2
#define IDCONTINUE 11
#define IDIGNORE   5
#define IDNO       7
#define IDOK       1
#define IDRETRY    4
#define IDTRYAGAIN 10
#define IDYES      6

typedef void          *HANDLE;
typedef int            BOOL, SOCKET;
typedef uint32_t       UINT, COLORREF;
typedef intptr_t       WPARAM, LPARAM, INT_PTR;
typedef uintptr_t      UINT_PTR, DWORD_PTR, LRESULT;
typedef char          *LPSTR;
typedef const char    *LPCSTR;
typedef wchar_t       *LPWSTR, *LPTSTR;
typedef const wchar_t *LPCWSTR, *LPCTSTR;
typedef sockaddr_in    SOCKADDR_IN;

struct RECT { int left, top, right, bottom; };
struct POINT { int x, y; };
struct SIZE { int width, height; };
struct MSG;
struct LOGFONTA;
struct LOGFONTW;
struct WIN32_FIND_DATA;

#define GetCurrentThreadId pthread_self

#define MIR_EXPORT __attribute__((__visibility__("default")))
#define MIR_IMPORT
#define MIR_SYSCALL
#define MIR_CDECL
#define UNREFERENCED_PARAMETER(x)

#define __try try
#define __except catch
#define EXCEPTION_EXECUTE_HANDLER ...

#define _In_z_
#define _Pre_notnull_
#define _Always_(x)
#define _Printf_format_string_
#define _countof(array) (sizeof(array) / sizeof(array[0]))
#define __forceinline inline __attribute__ ((always_inline))
#define __fallthrough

#define InterlockedIncrement(x) __sync_fetch_and_add(x, 1)
#define InterlockedDecrement(x) __sync_fetch_and_add(x, -1)

#define SecureZeroMemory(x, y) memset(x, 0, y)
#define interface struct
#define memcpy_s(a,b,c,d) memcpy(a,c,(b)<(d)?(b):(d))
#define memmove_s(a,b,c,d) memmove(a,c,(b)<(d)?(b):(d))

#define stricmp strcasecmp
#define strnicmp strncasecmp
#define wcsicmp wcscasecmp
#define wcsnicmp wcsncasecmp

#define _vsnprintf vsnprintf
#define _vsnwprintf vswprintf

#define DECLARE_HANDLE(name) struct _##name { int unused; }; typedef struct _##name *name
DECLARE_HANDLE(HDC);
DECLARE_HANDLE(HWND);
DECLARE_HANDLE(HFONT);
DECLARE_HANDLE(HICON);
DECLARE_HANDLE(HMENU);
DECLARE_HANDLE(HBRUSH);
DECLARE_HANDLE(HBITMAP);
DECLARE_HANDLE(HCURSOR);
DECLARE_HANDLE(HTREEITEM);
DECLARE_HANDLE(HINSTANCE);

struct EXCEPTION_POINTERS { int unused; };
struct LOGFONT { int unused; };
struct SYSTEMTIME;
struct MEASUREITEMSTRUCT;
struct DRAWITEMSTRUCT;
struct DELETEITEMSTRUCT;

struct NMHDR;
struct NMLISTVIEW;
struct NMLVDISPINFO;
struct NMLVSCROLL;
struct NMLVGETINFOTIP;
struct NMLVFINDITEM;
struct NMITEMACTIVATE;
struct NMLVKEYDOWN;
struct NMLVCUSTOMDRAW;
struct NMCLISTCONTROL;
struct NMTREEVIEW;
struct NMTVKEYDOWN;
struct NMTVDISPINFO;
struct NMTVGETINFOTIP;
struct NMTVCUSTOMDRAW;

struct LVFINDINFO;
struct LVBKIMAGE;
struct LVCOLUMN;
struct LVGROUP;
struct LVGROUPMETRICS;
struct LVINSERTMARK;
struct LVTILEINFO;
struct LVTILEVIEWINFO;
struct LVITEM;
struct LVHITTESTINFO;
struct LVINSERTGROUPSORTED;
struct LVSETINFOTIP;

struct TVITEMEX;
struct TVHITTESTINFO;
struct TVINSERTSTRUCT;
struct TVSORTCB;
struct _TREEITEM;

#ifdef ELEMENTARY_H
	typedef Evas_Object* MWindow;
#else
	typedef void *MWindow;
#endif

#else
///////////////////////////////////////////////////////////////////////////////
// Windows

#include <tchar.h>

#define MIR_EXPORT __declspec(dllexport)
#define MIR_IMPORT __declspec(dllimport)

#define MIR_SYSCALL __stdcall
#define MIR_CDECL   __cdecl

typedef HWND MWindow;

#endif

#endif // M_TYPES_H__

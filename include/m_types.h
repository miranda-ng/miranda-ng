/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

#ifdef Q_OS_WIN

#define FALSE 0
#define TRUE  1

typedef void*          HANDLE, HICON, HBITMAP;
typedef int            BOOL;
typedef unsigned int   DWORD;
typedef intptr_t       WPARAM, LPARAM;
typedef char*          LPSTR;
typedef const char*    LPCSTR;
typedef wchar_t*       LPWSTR, LPTSTR;
typedef const wchar_t* LPCWSTR, LPCTSTR;

#endif

#endif // M_TYPES_H__

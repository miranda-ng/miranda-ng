/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#ifndef M_SSL_H__
#define M_SSL_H__ 1

#include <newpluginapi.h>

#if !defined(HSSL_DEFINED)
	DECLARE_HANDLE(HSSL);
#endif

typedef HSSL  (__cdecl *pfnConnect)(SOCKET s, const char* host, int verify);
typedef BOOL  (__cdecl *pfnPending)(HSSL ssl);
typedef int   (__cdecl *pfnRead)(HSSL ssl, char *buf, int num, int peek);
typedef int   (__cdecl *pfnWrite)(HSSL ssl, const char *buf, int num);
typedef void  (__cdecl *pfnShutdown)(HSSL ssl);
typedef void  (__cdecl *pfnSfree)(HSSL ssl);
typedef void *(__cdecl *pfnUnique)(HSSL ssl, int *cbLen);

struct SSL_API
{
	int cbSize;

	pfnConnect  connect;
	pfnPending  pending;
	pfnRead     read;
	pfnWrite    write;
	pfnShutdown shutdown;
	pfnSfree    sfree;
	pfnUnique   unique;
};

/*
a service to obtain the SSL API

wParam = 0;
lParam = (LPARAM)(SSL_API*).

returns TRUE if all is Ok, and FALSE otherwise
*/

#define MS_SYSTEM_GET_SI "Miranda/System/GetSslApi"

__forceinline INT_PTR mir_getSI(SSL_API *dest)
{
	dest->cbSize = sizeof(*dest);
	return CallService(MS_SYSTEM_GET_SI, 0, (LPARAM)dest);
}

#endif // M_SSL_H__

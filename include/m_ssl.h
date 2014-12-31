/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

DECLARE_HANDLE(HSSL);

typedef struct
{
	int cbSize;

    HSSL    ( *connect ) ( SOCKET s, const char* host, int verify );
    BOOL    ( *pending ) ( HSSL ssl );
    int     ( *read )    ( HSSL ssl, char *buf, int num, int peek );
    int     ( *write )   ( HSSL ssl, const char *buf, int num );
    void    ( *shutdown )( HSSL ssl );
    void    ( *sfree )   ( HSSL ssl );
}
	SSL_API;

/* every protocol should declare this variable to use the SSL API */
extern SSL_API si;

/*
a service to obtain the SSL API

wParam = 0;
lParam = (LPARAM)(SSL_API*).

returns TRUE if all is Ok, and FALSE otherwise
*/

#define MS_SYSTEM_GET_SI "Miranda/System/GetSslApi"

__forceinline INT_PTR mir_getSI( SSL_API* dest )
{
	dest->cbSize = sizeof(*dest);
	return CallService(MS_SYSTEM_GET_SI, 0, (LPARAM)dest);
}

#endif // M_SSL_H__

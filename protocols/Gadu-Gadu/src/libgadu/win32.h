////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2009 Adam Strzelecki <ono+miranda@java.pl>
// Copyright (c) 2009-2010 Bartosz Bia³ek
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

/* Windows wrappers for missing POSIX functions */

#ifndef __GG_WIN32_H
#define __GG_WIN32_H

#include <winsock2.h>
#include <io.h>

/* Some Visual C++ overrides having no problems with MinGW */
#ifdef _MSC_VER
#define	S_IWUSR		0x0080
/* Make sure we included errno before that */
#include <errno.h>
#endif

#ifdef EINPROGRESS
# undef EINPROGRESS
#endif
#ifdef ENOTCONN
# undef ENOTCONN
#endif
#ifdef EINTR
# undef EINTR
#endif
#ifdef ECONNRESET
# undef ECONNRESET
#endif
#ifdef ETIMEDOUT
# undef ETIMEDOUT
#endif

#define EINPROGRESS WSAEINPROGRESS
#define ENOTCONN    WSAENOTCONN
#define EINTR       WSAEINTR
#define ECONNRESET  WSAECONNRESET
#define ETIMEDOUT   WSAETIMEDOUT

#define WNOHANG     WHOHANG
#define SHUT_RDWR 2

/* Defined in gg.c custom error reporting function */
#ifdef GG_CONFIG_MIRANDA
char *ws_strerror(int code);
#define strerror(x) ws_strerror(x)
#endif

#define fork()			(-1)
int sockpipe(SOCKET filedes[2]);
#define pipe(filedes)	sockpipe(filedes)
#define wait(x)			(-1)
#define waitpid(x,y,z)	(-1)
#define ioctl(fd,request,val) ioctlsocket(fd,request,(unsigned long *)val)

#endif /* __GG_WIN32_H */

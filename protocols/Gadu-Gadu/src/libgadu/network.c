/*
 *  (C) Copyright 2011 Wojtek Kaniewski <wojtekka@irc.pl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License Version
 *  2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
 *  USA.
 */

#include "internal.h"

#include "network.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

/* Code losely based on sockerpair implementation by Nathan C. Meyrs.
 * The original copyright notice follows: */

/* socketpair.c
 * Copyright 2007, 2010 by Nathan C. Myers <ncm@cantrip.org>
 * This code is Free Software. It may be copied freely, in original or
 * modified form, subject only to the restrictions that (1) the author is
 * relieved from all responsibilities for any use for any purpose, and (2)
 * this copyright notice must be retained, unchanged, in its entirety. If
 * for any reason the author might be held responsible for any consequences
 * of copying or use, license is withheld.
 */

int gg_win32_socketpair(int sv[2])
{
	struct sockaddr_in sin;
	socklen_t sin_len = sizeof(sin);
	int server = -1;
	int tmp = 1;
	int errno_copy;

	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sv[0] = -1;
	sv[1] = -1;

	if (server == -1)
		goto fail;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sin.sin_port = 0;

	if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp)) == -1)
		goto fail;

	if (bind(server, (struct sockaddr*) &sin, sin_len) == -1)
		goto fail;

	if (listen(server, 1) == -1)
		goto fail;

	if (getsockname(server, (struct sockaddr*) &sin, &sin_len) == -1)
		goto fail;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	sv[0] = socket(AF_INET, SOCK_STREAM, 0);

	if (sv[0] == -1)
		goto fail;

	if (connect(sv[0], (struct sockaddr*) &sin, sin_len) == -1)
		goto fail;

	sv[1] = accept(server, NULL, NULL);

	if (sv[1] == -1)
		goto fail;

	close(server);

	return 0;

fail:
	errno_copy = errno;
	close(server);
	close(sv[0]);
	close(sv[1]);
	errno = errno_copy;

	return -1;
}

static int gg_win32_map_wsa_error_to_errno(int wsaewouldblock_map)
{
	int wsa_error;

	wsa_error = WSAGetLastError();

	/* Tutaj powinny być tłumaczone wszystkie typy błędów sprawdzane przez
	 * kod libgadu. Dla spójność są również tłumaczone typy błędów ustawiane
	 * przez libgadu.
	 * Ponadto gdyby okazało się, że jakaś aplikacja na Win32 chce móc
	 * polegać jeszcze na innych wartościach errno, można tutaj dodać
	 * ich tłumaczenie. Najpierw jednak zawsze trzeba porównać dokumentacje,
	 * aby upewnić się co do poprawności tłumaczenia (patrz WSAEWOULDBLOCK,
	 * które można tłumaczyć na EWOULDBLOCK lub EAGAIN, a nawet na
	 * EINPROGRESS w przypadku connect()).
	 */
	switch (wsa_error)
	{
	/* Typy błędów sprawdzane przez libgadu. */
	case WSAEINTR:
		return EINTR;
	case WSAEWOULDBLOCK:
		return wsaewouldblock_map;
	/* Typy błędów ustawiane przez libgadu. */
	case WSAECONNRESET:
		return ECONNRESET;
	case WSAEFAULT:
		return EFAULT;
	case WSAEINVAL:
		return EINVAL;
	case WSAENOTCONN:
		return ENOTCONN;
	case WSAETIMEDOUT:
		return ETIMEDOUT;
	default:
		/* Najlepiej zwrócić oryginalny kod błędu. I tak będzie co najwyżej
		 * wyświetlony w komunikacie debugowym, a tym sposobem będzie łatwiej
		 * dojść przyczyny problemu. */
		return wsa_error;
	}
}

#undef accept
int gg_win32_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int res;

	res = accept(sockfd, addr, addrlen);

	if (res == -1)
		errno = gg_win32_map_wsa_error_to_errno(EAGAIN);

	return res;
}

#undef bind
int gg_win32_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int res;

	res = bind(sockfd, addr, addrlen);

	if (res == -1)
		errno = gg_win32_map_wsa_error_to_errno(EAGAIN);

	return res;
}

int gg_win32_close(int sockfd)
{
	int res;

	res = closesocket(sockfd);

	if (res == -1)
		errno = gg_win32_map_wsa_error_to_errno(EAGAIN);

	return res;
}

#undef connect
int gg_win32_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int res;

	res = connect(sockfd, addr, addrlen);

	if (res == -1)
		errno = gg_win32_map_wsa_error_to_errno(EINPROGRESS);

	return res;
}

#undef gethostbyname
struct hostent *gg_win32_gethostbyname(const char *name)
{
	struct hostent *res;

	res = gethostbyname(name);

	if (res == NULL)
		errno = gg_win32_map_wsa_error_to_errno(EAGAIN);

	return res;
}

#undef getsockname
int gg_win32_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int res;

	res = getsockname(sockfd, addr, addrlen);

	if (res == -1)
		errno = gg_win32_map_wsa_error_to_errno(EAGAIN);

	return res;
}

#undef getsockopt
int gg_win32_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
	int res;

	res = getsockopt(sockfd, level, optname, optval, optlen);

	if (res == -1)
		errno = gg_win32_map_wsa_error_to_errno(EAGAIN);

	return res;
}

int gg_win32_ioctl(int d, int request, int *argp)
{
	int res;

	res = ioctlsocket(d, request, (u_long *)argp);

	if (res == -1)
		errno = gg_win32_map_wsa_error_to_errno(EAGAIN);

	return res;
}

#undef listen
int gg_win32_listen(int sockfd, int backlog)
{
	int res;

	res = listen(sockfd, backlog);

	if (res == -1)
		errno = gg_win32_map_wsa_error_to_errno(EAGAIN);

	return res;
}

#undef recv
int gg_win32_recv(int sockfd, void *buf, size_t len, int flags)
{
	int res;

	res = recv(sockfd, buf, len, flags);

	if (res == -1)
		errno = gg_win32_map_wsa_error_to_errno(EAGAIN);

	return res;
}

#undef send
int gg_win32_send(int sockfd, const void *buf, size_t len, int flags)
{
	int res;

	res = send(sockfd, buf, len, flags);

	if (res == -1)
		errno = gg_win32_map_wsa_error_to_errno(EAGAIN);

	return res;
}

#undef setsockopt
int gg_win32_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
	int res;

	res = setsockopt(sockfd, level, optname, optval, optlen);

	if (res == -1)
		errno = gg_win32_map_wsa_error_to_errno(EAGAIN);

	return res;
}

#undef socket
int gg_win32_socket(int domain, int type, int protocol)
{
	int res;

	res = socket(domain, type, protocol);

	if (res == -1)
		errno = gg_win32_map_wsa_error_to_errno(EAGAIN);

	return res;
}

#endif /* _WIN32 */

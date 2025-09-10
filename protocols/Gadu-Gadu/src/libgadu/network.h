/*
 *  (C) Copyright 2001-2002 Wojtek Kaniewski <wojtekka@irc.pl>
 *                          Robert J. Woźny <speedy@ziew.org>
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

/**
 * \file network.h
 *
 * \brief Makra zapewniające kompatybilność API do obsługi sieci na różnych systemach
 */

#ifndef LIBGADU_NETWORK_H
#define LIBGADU_NETWORK_H

#ifdef _WIN32
#  include <ws2tcpip.h>
#  include <stdlib.h>
#  include <stdio.h>
#  include <errno.h>
/* Obecnie na Win32 tylko MSVC definiuje te typy błędów. Na wypadek, gdyby
 * jednak Cygwin bądź MinGW zaczęły je definiować, używamy bardziej ogólnych
 * ifdefów. */
#  ifndef ECONNRESET
#    define ECONNRESET WSAECONNRESET
#  endif
#  ifndef EINPROGRESS
#    define EINPROGRESS WSAEINPROGRESS
#  endif
#  ifndef ENOTCONN
#    define ENOTCONN WSAENOTCONN
#  endif
#  ifndef ETIMEDOUT
#    define ETIMEDOUT WSAETIMEDOUT
#  endif
#  define accept gg_win32_accept
#  define bind gg_win32_bind
#  define close gg_win32_close
#  define connect gg_win32_connect
#  define gethostbyname gg_win32_gethostbyname
#  define getsockname gg_win32_getsockname
#  define getsockopt gg_win32_getsockopt
#  define ioctl gg_win32_ioctl
#  define listen gg_win32_listen
#  define recv gg_win32_recv
#  define send gg_win32_send
#  define setsockopt gg_win32_setsockopt
#  define socket gg_win32_socket
#  define socketpair(a, b, c, d) gg_win32_socketpair(d)
int gg_win32_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int gg_win32_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int gg_win32_close(int sockfd);
int gg_win32_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
struct hostent *gg_win32_gethostbyname(const char *name);
int gg_win32_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int gg_win32_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
int gg_win32_ioctl(int d, int request, int *argp);
int gg_win32_listen(int sockfd, int backlog);
int gg_win32_recv(int sockfd, void *buf, size_t len, int flags);
int gg_win32_send(int sockfd, const void *buf, size_t len, int flags);
int gg_win32_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
int gg_win32_socket(int domain, int type, int protocol);
int gg_win32_socketpair(int sv[2]);

static inline void gg_win32_init_network(void)
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		perror("WSAStartup");
		exit(1);
	}
}

#else
#  include <sys/ioctl.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <unistd.h>
#  ifndef FIONBIO
#    include <fcntl.h>
#  endif
#endif

#ifndef INADDR_NONE
#  define INADDR_NONE ((in_addr_t) 0xffffffff)
#endif

#ifndef AF_LOCAL
#  define AF_LOCAL AF_UNIX
#endif

static inline int gg_fd_set_nonblocking(int fd)
{
	int success;
#ifdef FIONBIO
	int one = 1;
	success = (ioctl(fd, FIONBIO, &one) == 0);
#else
	success = (fcntl(fd, F_SETFL, O_NONBLOCK) == 0);
#endif

	return success;
}

#endif /* LIBGADU_NETWORK_H */

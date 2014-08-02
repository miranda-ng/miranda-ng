/* coding: UTF-8 */
/* $Id: http.c 11370 2010-03-13 16:17:54Z dezred $ */

/*
 *  (C) Copyright 2001-2002 Wojtek Kaniewski <wojtekka@irc.pl>
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
 * \file http.c
 *
 * \brief Obsługa połączeń HTTP
 */

#include <sys/types.h>
#ifdef _WIN32
#include "win32.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* _WIN32 */

#include "compat.h"
#include "libgadu.h"
#include "resolver.h"

#include <ctype.h>
#include <errno.h>
#ifndef _WIN32
#include <netdb.h>
#endif /* _WIN32 */
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif /* _WIN32 */

/**
 * Rozpoczyna połączenie HTTP.
 *
 * Funkcja przeprowadza połączenie HTTP przy połączeniu synchronicznym,
 * zwracając wynik w polach struktury \c gg_http, lub błąd, gdy sesja się
 * nie powiedzie.
 *
 * Przy połączeniu asynchronicznym, funkcja rozpoczyna połączenie, a dalsze
 * etapy będą przeprowadzane po wykryciu zmian (\c watch) na obserwowanym
 * deskryptorze (\c fd) i wywołaniu funkcji \c gg_http_watch_fd().
 *
 * Po zakończeniu, należy zwolnić strukturę za pomocą funkcji
 * \c gg_http_free(). Połączenie asynchroniczne można zatrzymać w każdej
 * chwili za pomocą \c gg_http_stop().
 *
 * \param hostname Adres serwera
 * \param port Port serwera
 * \param async Flaga asynchronicznego połączenia
 * \param method Metoda HTTP
 * \param path Ścieżka do zasobu (musi być poprzedzona znakiem '/')
 * \param header Nagłówek zapytania plus ewentualne dane dla POST
 *
 * \return Zaalokowana struktura \c gg_http lub NULL, jeśli wystąpił błąd.
 *
 * \ingroup http
 */
struct gg_http *gg_http_connect(const char *hostname, int port, int async, const char *method, const char *path, const char *header)
{
	struct gg_http *h;

	if (!hostname || !port || !method || !path || !header) {
		gg_debug(GG_DEBUG_MISC, "// gg_http_connect() invalid arguments\n");
		errno = EFAULT;
		return NULL;
	}

	if (!(h = (gg_http*)malloc(sizeof(*h))))
		return NULL;
	memset(h, 0, sizeof(*h));

	h->async = async;
	h->port = port;
	h->fd = -1;
	h->type = GG_SESSION_HTTP;

	gg_http_set_resolver(h, GG_RESOLVER_DEFAULT);

	if (gg_proxy_enabled) {
		char *auth = gg_proxy_auth();

		h->query = gg_saprintf("%s http://%s:%d%s HTTP/1.0\r\n%s%s",
				method, hostname, port, path, (auth) ? auth :
				"", header);
		hostname = gg_proxy_host;
		h->port = port = gg_proxy_port;
		free(auth);

	} else {
		h->query = gg_saprintf("%s %s HTTP/1.0\r\n%s",
				method, path, header);
	}

	if (!h->query) {
		gg_debug(GG_DEBUG_MISC, "// gg_http_connect() not enough memory for query\n");
		free(h);
		errno = ENOMEM;
		return NULL;
	}

	gg_debug(GG_DEBUG_MISC, "=> -----BEGIN-HTTP-QUERY-----\n%s\n=> -----END-HTTP-QUERY-----\n", h->query);

	if (async) {
		if (h->resolver_start(&h->fd, &h->resolver, hostname) == -1) {
			gg_debug(GG_DEBUG_MISC, "// gg_http_connect() resolver failed\n");
			gg_http_free(h);
			errno = ENOENT;
			return NULL;
		}

		gg_debug(GG_DEBUG_MISC, "// gg_http_connect() resolver = %p\n", h->resolver);

		h->state = GG_STATE_RESOLVING;
		h->check = GG_CHECK_READ;
		h->timeout = GG_DEFAULT_TIMEOUT;
	} else {
		struct in_addr addr;

		if (gg_gethostbyname_real(hostname, &addr, 0) == -1) {
			gg_debug(GG_DEBUG_MISC, "// gg_http_connect() host not found\n");
			gg_http_free(h);
			errno = ENOENT;
			return NULL;
		}

		if ((h->fd = gg_connect(&addr, port, 0)) == -1) {
			gg_debug(GG_DEBUG_MISC, "// gg_http_connect() connection failed (errno=%d, %s)\n", errno, strerror(errno));
			gg_http_free(h);
			return NULL;
		}

		h->state = GG_STATE_CONNECTING;

		while (h->state != GG_STATE_ERROR && h->state != GG_STATE_PARSING) {
			if (gg_http_watch_fd(h) == -1)
				break;
		}

		if (h->state != GG_STATE_PARSING) {
			gg_debug(GG_DEBUG_MISC, "// gg_http_connect() some strange error\n");
			gg_http_free(h);
			return NULL;
		}
	}

	h->callback = gg_http_watch_fd;
	h->destroy = gg_http_free;

	return h;
}

#ifndef DOXYGEN

#define gg_http_error(x) \
	gg_sock_close(h->fd); \
	h->fd = -1; \
	h->state = GG_STATE_ERROR; \
	h->error = x; \
	return 0;

#endif /* DOXYGEN */

/**
 * Funkcja wywoływana po zaobserwowaniu zmian na deskryptorze połączenia.
 *
 * Operacja będzie zakończona, gdy pole \c state będzie równe
 * \c GG_STATE_PARSING. W tym miejscu działanie przejmuje zwykle funkcja
 * korzystająca z \c gg_http_watch_fd(). W przypadku błędu połączenia,
 * pole \c state będzie równe \c GG_STATE_ERROR, a kod błędu znajdzie się
 * w polu \c error.
 *
 * \param h Struktura połączenia
 *
 * \return \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup http
 */
int gg_http_watch_fd(struct gg_http *h)
{
	gg_debug(GG_DEBUG_FUNCTION, "** gg_http_watch_fd(%p);\n", h);

	if (!h) {
		gg_debug(GG_DEBUG_MISC, "// gg_http_watch_fd() invalid arguments\n");
		errno = EFAULT;
		return -1;
	}

	if (h->state == GG_STATE_RESOLVING) {
		struct in_addr a;

		gg_debug(GG_DEBUG_MISC, "=> http, resolving done\n");

		if (gg_sock_read(h->fd, &a, sizeof(a)) < (signed)sizeof(a) || a.s_addr == INADDR_NONE) {
			gg_debug(GG_DEBUG_MISC, "=> http, resolver thread failed\n");
			gg_http_error(GG_ERROR_RESOLVING);
		}

		gg_sock_close(h->fd);
		h->fd = -1;

		h->resolver_cleanup(&h->resolver, 0);

		gg_debug(GG_DEBUG_MISC, "=> http, connecting to %s:%d\n", inet_ntoa(a), h->port);

		if ((h->fd = gg_connect(&a, h->port, h->async)) == -1) {
			gg_debug(GG_DEBUG_MISC, "=> http, connection failed (errno=%d, %s)\n", errno, strerror(errno));
			gg_http_error(GG_ERROR_CONNECTING);
		}

		h->state = GG_STATE_CONNECTING;
		h->check = GG_CHECK_WRITE;
		h->timeout = GG_DEFAULT_TIMEOUT;

		return 0;
	}

	if (h->state == GG_STATE_CONNECTING) {
		int res = 0;
		int res_size = sizeof(res);

		if (h->async && (gg_getsockopt(h->fd, SOL_SOCKET, SO_ERROR, &res, &res_size) || res)) {
			gg_debug(GG_DEBUG_MISC, "=> http, async connection failed (errno=%d, %s)\n", (res) ? res : errno , strerror((res) ? res : errno));
			gg_sock_close(h->fd);
			h->fd = -1;
			h->state = GG_STATE_ERROR;
			h->error = GG_ERROR_CONNECTING;
			if (res)
				errno = res;
			return 0;
		}

		gg_debug(GG_DEBUG_MISC, "=> http, connected, sending request\n");

		h->state = GG_STATE_SENDING_QUERY;
	}

	if (h->state == GG_STATE_SENDING_QUERY) {
		int res;

		if ((res = gg_sock_write(h->fd, h->query, (int)strlen(h->query))) < 1) {
			gg_debug(GG_DEBUG_MISC, "=> http, write() failed (len=%d, res=%d, errno=%d)\n", strlen(h->query), res, errno);
			gg_http_error(GG_ERROR_WRITING);
		}

		if (res < (int)strlen(h->query)) {
			gg_debug(GG_DEBUG_MISC, "=> http, partial header sent (led=%d, sent=%d)\n", strlen(h->query), res);

			memmove(h->query, h->query + res, strlen(h->query) - res + 1);
			h->state = GG_STATE_SENDING_QUERY;
			h->check = GG_CHECK_WRITE;
			h->timeout = GG_DEFAULT_TIMEOUT;
		} else {
			gg_debug(GG_DEBUG_MISC, "=> http, request sent (len=%d)\n", strlen(h->query));
			free(h->query);
			h->query = NULL;

			h->state = GG_STATE_READING_HEADER;
			h->check = GG_CHECK_READ;
			h->timeout = GG_DEFAULT_TIMEOUT;
		}

		return 0;
	}

	if (h->state == GG_STATE_READING_HEADER) {
		char buf[1024], *tmp;
		int res;

		if ((res = gg_sock_read(h->fd, buf, sizeof(buf))) == -1) {
			gg_debug(GG_DEBUG_MISC, "=> http, reading header failed (errno=%d)\n", errno);
			if (h->header) {
				free(h->header);
				h->header = NULL;
			}
			gg_http_error(GG_ERROR_READING);
		}

		if (!res) {
			gg_debug(GG_DEBUG_MISC, "=> http, connection reset by peer\n");
			if (h->header) {
				free(h->header);
				h->header = NULL;
			}
			gg_http_error(GG_ERROR_READING);
		}

		gg_debug(GG_DEBUG_MISC, "=> http, read %d bytes of header\n", res);

		if (!(tmp = (char*)realloc(h->header, h->header_size + res + 1))) {
			gg_debug(GG_DEBUG_MISC, "=> http, not enough memory for header\n");
			free(h->header);
			h->header = NULL;
			gg_http_error(GG_ERROR_READING);
		}

		h->header = tmp;

		memcpy(h->header + h->header_size, buf, res);
		h->header_size += res;

		gg_debug(GG_DEBUG_MISC, "=> http, header_buf=%p, header_size=%d\n", h->header, h->header_size);

		h->header[h->header_size] = 0;

		if ((tmp = strstr(h->header, "\r\n\r\n")) || (tmp = strstr(h->header, "\n\n"))) {
			int sep_len = (*tmp == '\r') ? 4 : 2;
			unsigned int left;
			char *line;

			left = h->header_size - ((long)(tmp) - (long)(h->header) + sep_len);

			gg_debug(GG_DEBUG_MISC, "=> http, got all header (%d bytes, %d left)\n", h->header_size - left, left);

			/* HTTP/1.1 200 OK */
			if (strlen(h->header) < 16 || strncmp(h->header + 9, "200", 3)) {
				gg_debug(GG_DEBUG_MISC, "=> -----BEGIN-HTTP-HEADER-----\n%s\n=> -----END-HTTP-HEADER-----\n", h->header);

				gg_debug(GG_DEBUG_MISC, "=> http, didn't get 200 OK -- no results\n");
				free(h->header);
				h->header = NULL;
				gg_http_error(GG_ERROR_CONNECTING);
			}

			h->body_size = 0;
			line = h->header;
			*tmp = 0;

			gg_debug(GG_DEBUG_MISC, "=> -----BEGIN-HTTP-HEADER-----\n%s\n=> -----END-HTTP-HEADER-----\n", h->header);

			while (line) {
				if (!strncasecmp(line, "Content-length: ", 16)) {
					h->body_size = atoi(line + 16);
				}
				line = strchr(line, '\n');
				if (line)
					line++;
			}

			if (h->body_size <= 0) {
				gg_debug(GG_DEBUG_MISC, "=> http, content-length not found\n");
				h->body_size = left;
			}

			if (left > h->body_size) {
				gg_debug(GG_DEBUG_MISC, "=> http, oversized reply (%d bytes needed, %d bytes left)\n", h->body_size, left);
				h->body_size = left;
			}

			gg_debug(GG_DEBUG_MISC, "=> http, body_size=%d\n", h->body_size);

			if (!(h->body = (char*)malloc(h->body_size + 1))) {
				gg_debug(GG_DEBUG_MISC, "=> http, not enough memory (%d bytes for body_buf)\n", h->body_size + 1);
				free(h->header);
				h->header = NULL;
				gg_http_error(GG_ERROR_READING);
			}

			if (left) {
				memcpy(h->body, tmp + sep_len, left);
				h->body_done = left;
			}

			h->body[left] = 0;

			h->state = GG_STATE_READING_DATA;
			h->check = GG_CHECK_READ;
			h->timeout = GG_DEFAULT_TIMEOUT;
		}

		return 0;
	}

	if (h->state == GG_STATE_READING_DATA) {
		char buf[1024];
		int res;

		if ((res = gg_sock_read(h->fd, buf, sizeof(buf))) == -1) {
			gg_debug(GG_DEBUG_MISC, "=> http, reading body failed (errno=%d)\n", errno);
			if (h->body) {
				free(h->body);
				h->body = NULL;
			}
			gg_http_error(GG_ERROR_READING);
		}

		if (!res) {
			if (h->body_done >= h->body_size) {
				gg_debug(GG_DEBUG_MISC, "=> http, we're done, closing socket\n");
				h->state = GG_STATE_PARSING;
				gg_sock_close(h->fd);
				h->fd = -1;
			} else {
				gg_debug(GG_DEBUG_MISC, "=> http, connection closed while reading (have %d, need %d)\n", h->body_done, h->body_size);
				if (h->body) {
					free(h->body);
					h->body = NULL;
				}
				gg_http_error(GG_ERROR_READING);
			}

			return 0;
		}

		gg_debug(GG_DEBUG_MISC, "=> http, read %d bytes of body\n", res);

		if (h->body_done + res > h->body_size) {
			char *tmp;

			gg_debug(GG_DEBUG_MISC, "=> http, too much data (%d bytes, %d needed), enlarging buffer\n", h->body_done + res, h->body_size);

			if (!(tmp = (char*)realloc(h->body, h->body_done + res + 1))) {
				gg_debug(GG_DEBUG_MISC, "=> http, not enough memory for data (%d needed)\n", h->body_done + res + 1);
				free(h->body);
				h->body = NULL;
				gg_http_error(GG_ERROR_READING);
			}

			h->body = tmp;
			h->body_size = h->body_done + res;
		}

		h->body[h->body_done + res] = 0;
		memcpy(h->body + h->body_done, buf, res);
		h->body_done += res;

		gg_debug(GG_DEBUG_MISC, "=> body_done=%d, body_size=%d\n", h->body_done, h->body_size);

		return 0;
	}

	if (h->fd != -1)
		gg_sock_close(h->fd);

	h->fd = -1;
	h->state = GG_STATE_ERROR;
	h->error = 0;

	return -1;
}

/**
 * Kończy asynchroniczne połączenie HTTP.
 *
 * Po zatrzymaniu należy zwolnić zasoby funkcją \c gg_http_free().
 *
 * \param h Struktura połączenia
 *
 * \ingroup http
 */
void gg_http_stop(struct gg_http *h)
{
	if (!h)
		return;

	if (h->state == GG_STATE_ERROR || h->state == GG_STATE_DONE)
		return;

	if (h->fd != -1) {
		gg_sock_close(h->fd);
		h->fd = -1;
	}

	h->resolver_cleanup(&h->resolver, 1);
}

/**
 * \internal Zwalnia pola struktury \c gg_http.
 *
 * Funkcja zwalnia same pola, nie zwalnia struktury.
 *
 * \param h Struktura połączenia
 */
void gg_http_free_fields(struct gg_http *h)
{
	if (!h)
		return;

	if (h->body) {
		free(h->body);
		h->body = NULL;
	}

	if (h->query) {
		free(h->query);
		h->query = NULL;
	}

	if (h->header) {
		free(h->header);
		h->header = NULL;
	}
}

/**
 * Zwalnia zasoby po połączeniu HTTP.
 *
 * Jeśli połączenie nie zostało jeszcze zakończone, jest przerywane.
 *
 * \param h Struktura połączenia
 *
 * \ingroup http
 */
void gg_http_free(struct gg_http *h)
{
	if (!h)
		return;

	gg_http_stop(h);
	gg_http_free_fields(h);
	free(h);
}

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: notnil
 * End:
 *
 * vim: shiftwidth=8:
 */

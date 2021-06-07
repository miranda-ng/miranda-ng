/* $Id$ */

/*
 *  (C) Copyright 2001-2006 Wojtek Kaniewski <wojtekka@irc.pl>
 *                          Robert J. Woźny <speedy@ziew.org>
 *                          Arkadiusz Miśkiewicz <arekm@pld-linux.org>
 *                          Adam Wysocki <gophi@ekg.chmurka.net>
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
 * \file events.c
 *
 * \brief Obsługa zdarzeń
 */

#ifndef _WIN64
#define _USE_32BIT_TIME_T
#endif

#include <sys/types.h>
#ifdef _WIN32
#include "win32.h"
#else
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* _WIN32 */

#include "compat.h"
#include "libgadu.h"
#include "protocol.h"
#include "internal.h"
#include "encoding.h"
#include "debug.h"
#include "session.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#ifndef _WIN32
#include <unistd.h>
#include <ctype.h>
#endif /* _WIN32 */
#ifndef GG_CONFIG_MIRANDA
#ifdef GG_CONFIG_HAVE_GNUTLS
#  include <gnutls/gnutls.h>
#  include <gnutls/x509.h>
#endif
#ifdef GG_CONFIG_HAVE_OPENSSL
#  include <openssl/err.h>
#  include <openssl/x509.h>
#endif
#endif

/**
 * Zwalnia pamięć zajmowaną przez informację o zdarzeniu.
 *
 * Funkcję należy wywoływać za każdym razem gdy funkcja biblioteki zwróci
 * strukturę \c gg_event.
 *
 * \param e Struktura zdarzenia
 *
 * \ingroup events
 */
void gg_event_free(struct gg_event *e)
{
	gg_debug(GG_DEBUG_FUNCTION, "** gg_event_free(%p);\n", e);

	if (!e)
		return;

	switch (e->type) {
		case GG_EVENT_MSG:
		case GG_EVENT_MULTILOGON_MSG:
			free(e->event.msg.message);
			free(e->event.msg.formats);
			free(e->event.msg.recipients);
			free(e->event.msg.xhtml_message);
			break;

		case GG_EVENT_NOTIFY:
			free(e->event.notify);
			break;

		case GG_EVENT_NOTIFY60:
		{
			int i;

			for (i = 0; e->event.notify60[i].uin; i++)
				free(e->event.notify60[i].descr);

			free(e->event.notify60);

			break;
		}

		case GG_EVENT_STATUS60:
			free(e->event.status60.descr);
			break;

		case GG_EVENT_STATUS:
			free(e->event.status.descr);
			break;

		case GG_EVENT_NOTIFY_DESCR:
			free(e->event.notify_descr.notify);
			free(e->event.notify_descr.descr);
			break;

		case GG_EVENT_DCC_VOICE_DATA:
			free(e->event.dcc_voice_data.data);
			break;

		case GG_EVENT_PUBDIR50_SEARCH_REPLY:
		case GG_EVENT_PUBDIR50_READ:
		case GG_EVENT_PUBDIR50_WRITE:
			gg_pubdir50_free(e->event.pubdir50);
			break;

		case GG_EVENT_USERLIST:
			free(e->event.userlist.reply);
			break;

		case GG_EVENT_IMAGE_REPLY:
			free(e->event.image_reply.filename);
			free(e->event.image_reply.image);
			break;

		case GG_EVENT_XML_EVENT:
			free(e->event.xml_event.data);
			break;

		case GG_EVENT_USER_DATA:
		{
			int i, j;

			for (i = 0; i < e->event.user_data.user_count; i++) {
				for (j = 0; j < e->event.user_data.users[i].attr_count; j++) {
					free(e->event.user_data.users[i].attrs[j].key);
					free(e->event.user_data.users[i].attrs[j].value);
				}

				free(e->event.user_data.users[i].attrs);
			}

			free(e->event.user_data.users);

			break;
		}
	
		case GG_EVENT_MULTILOGON_INFO:
		{
			int i;

			for (i = 0; i < e->event.multilogon_info.count; i++)
				free(e->event.multilogon_info.sessions[i].name);

			free(e->event.multilogon_info.sessions);

			break;
		}

		case GG_EVENT_USERLIST100_REPLY:
			free(e->event.userlist100_reply.reply);
			break;
	}

	free(e);
}

/** \cond internal */

/**
 * \internal Usuwa obrazek z kolejki do wysłania.
 *
 * \param s Struktura sesji
 * \param q Struktura obrazka
 * \param freeq Flaga zwolnienia elementu kolejki
 *
 * \return 0 jeśli się powiodło, -1 jeśli wystąpił błąd
 */
int gg_image_queue_remove(struct gg_session *s, struct gg_image_queue *q, int freeq)
{
	if (!s || !q) {
		errno = EFAULT;
		return -1;
	}

	if (s->images == q)
		s->images = q->next;
	else {
		struct gg_image_queue *qq;

		for (qq = s->images; qq; qq = qq->next) {
			if (qq->next == q) {
				qq->next = q->next;
				break;
			}
		}
	}

	if (freeq) {
		free(q->image);
		free(q->filename);
		free(q);
	}

	return 0;
}

/** \endcond */

/**
 * Funkcja wywoływana po zaobserwowaniu zmian na deskryptorze sesji.
 *
 * Funkcja zwraca strukturę zdarzenia \c gg_event. Jeśli rodzaj zdarzenia
 * to \c GG_EVENT_NONE, nie wydarzyło się jeszcze nic wartego odnotowania.
 * Strukturę zdarzenia należy zwolnić funkcja \c gg_event_free().
 *
 * \param sess Struktura sesji
 *
 * \return Struktura zdarzenia lub \c NULL jeśli wystąpił błąd
 *
 * \ingroup events
 */
struct gg_event *gg_watch_fd(struct gg_session *sess)
{
	struct gg_event *e;
	int res = 0;
	int port = 0;
	int errno2 = 0;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_watch_fd(%p);\n", sess);

	if (!sess) {
		errno = EFAULT;
		return NULL;
	}

	if (!(e = (gg_event*)calloc(1, sizeof(*e)))) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() not enough memory for event data\n");
		return NULL;
	}

	e->type = GG_EVENT_NONE;

	if (sess->send_buf && (sess->state == GG_STATE_READING_REPLY || sess->state == GG_STATE_CONNECTED)) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() sending %d bytes of queued data\n", sess->send_left);

		res = gg_sock_write(sess->fd, sess->send_buf, sess->send_left);

		if (res == -1 && errno != EAGAIN) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() write() failed (errno=%d, %s)\n", errno, strerror(errno));

			if (sess->state == GG_STATE_READING_REPLY)
				e->event.failure = GG_FAILURE_CONNECTING;

			goto fail;
		}

		if (res == sess->send_left) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() sent all queued data\n");
			free(sess->send_buf);
			sess->send_buf = NULL;
			sess->send_left = 0;
		} else if (res > 0) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() sent %d bytes of queued data, %d bytes left\n", res, sess->send_left - res);

			memmove(sess->send_buf, sess->send_buf + res, sess->send_left - res);
			sess->send_left -= res;
		}

		res = 0;
	}

	switch (sess->state) {
		case GG_STATE_RESOLVING:
		{
			struct in_addr addr;
			int failed = 0;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_RESOLVING\n");

			if (gg_sock_read(sess->fd, &addr, sizeof(addr)) < (signed)sizeof(addr) || addr.s_addr == INADDR_NONE) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() resolving failed\n");
				failed = 1;
				errno2 = errno;
			}

			gg_sock_close(sess->fd);
			sess->fd = -1;

			sess->resolver_cleanup(&sess->resolver, 0);

			if (failed) {
				errno = errno2;
				goto fail_proxy_hub;
			}

			/* jeśli jesteśmy w resolverze i mamy ustawiony port
			 * proxy, znaczy, że resolvowaliśmy proxy. zatem
			 * wpiszmy jego adres. */
			if (sess->proxy_port)
				sess->proxy_addr = addr.s_addr;

			/* zapiszmy sobie adres huba i adres serwera (do
			 * bezpośredniego połączenia, jeśli hub leży)
			 * z resolvera. */
			if (sess->proxy_addr && sess->proxy_port)
				port = sess->proxy_port;
			else {
				sess->server_addr = sess->hub_addr = addr.s_addr;
				port = GG_APPMSG_PORT;
			}

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() resolved, connecting to %s:%d\n", inet_ntoa(addr), port);

			/* łączymy się albo z hubem, albo z proxy, zależnie
			 * od tego, co resolvowaliśmy. */
			if ((sess->fd = gg_connect(&addr, port, sess->async)) == -1) {
				/* jeśli w trybie asynchronicznym gg_connect()
				 * zwróci błąd, nie ma sensu próbować dalej. */
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s), critical\n", errno, strerror(errno));
				goto fail_proxy_hub;
			}

			/* jeśli podano serwer i łączmy się przez proxy,
			 * jest to bezpośrednie połączenie, inaczej jest
			 * do huba. */

			if (sess->proxy_addr && sess->proxy_port && sess->server_addr) {
				sess->state = GG_STATE_CONNECTING_GG;
				sess->soft_timeout = 1;
			} else
				sess->state = GG_STATE_CONNECTING_HUB;

			sess->check = GG_CHECK_WRITE;
			sess->timeout = GG_DEFAULT_TIMEOUT;

			break;
		}

		case GG_STATE_CONNECTING_HUB:
		{
			char buf[1024], *client, *auth;
			int res = 0;
			int res_size = sizeof(res);
			const char *host;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_CONNECTING_HUB\n");

			/* jeśli asynchroniczne, sprawdzamy, czy nie wystąpił
			 * przypadkiem jakiś błąd. */
			if (sess->async && (gg_getsockopt(sess->fd, SOL_SOCKET, SO_ERROR, &res, &res_size) || res)) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection to %s failed (errno=%d, %s)\n", (sess->proxy_addr && sess->proxy_port) ? "proxy" : "hub", res, strerror(res));
				goto fail_proxy_hub;
			}

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connected to hub, sending query\n");

			if (sess->client_version != NULL && isdigit(sess->client_version[0]))
				client = gg_urlencode(sess->client_version);
			else
				client = gg_urlencode(GG_DEFAULT_CLIENT_VERSION);

			if (client == NULL) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() out of memory for client version\n");
				goto fail;
			}

			if (!gg_proxy_http_only && sess->proxy_addr && sess->proxy_port)
				host = "http://" GG_APPMSG_HOST;
			else
				host = "";

			auth = gg_proxy_auth();

#ifdef GG_CONFIG_MIRANDA
			if (sess->tls) {
				snprintf(buf, sizeof(buf) - 1,
					"GET %s/appsvc/appmsg_ver11.asp?tls=1&fmnumber=%u&fmt=2&lastmsg=%d&version=%s&age=2&gender=1 HTTP/1.0\r\n"
					"Connection: close\r\n"
					"Host: " GG_APPMSG_HOST "\r\n"
					"%s"
					"\r\n", host, sess->uin, sess->last_sysmsg, client, (auth) ? auth : "");
			} else
#elif defined(GG_CONFIG_HAVE_GNUTLS) || defined(GG_CONFIG_HAVE_OPENSSL)
			if (sess->ssl != NULL) {
				snprintf(buf, sizeof(buf) - 1,
					"GET %s/appsvc/appmsg_ver11.asp?tls=1&fmnumber=%u&fmt=2&lastmsg=%d&version=%s&age=2&gender=1 HTTP/1.0\r\n"
					"Connection: close\r\n"
					"Host: " GG_APPMSG_HOST "\r\n"
					"%s"
					"\r\n", host, sess->uin, sess->last_sysmsg, client, (auth) ? auth : "");
			} else
#endif
			{
				snprintf(buf, sizeof(buf) - 1,
					"GET %s/appsvc/appmsg_ver8.asp?fmnumber=%u&fmt=2&lastmsg=%d&version=%s HTTP/1.0\r\n"
					"Host: " GG_APPMSG_HOST "\r\n"
					"%s"
					"\r\n", host, sess->uin, sess->last_sysmsg, client, (auth) ? auth : "");
			}

			free(auth);
			free(client);

			gg_debug_session(sess, GG_DEBUG_MISC, "=> -----BEGIN-HTTP-QUERY-----\n%s\n=> -----END-HTTP-QUERY-----\n", buf);

			/* zapytanie jest krótkie, więc zawsze zmieści się
			 * do bufora gniazda. jeśli write() zwróci mniej,
			 * stało się coś złego. */
			if (gg_sock_write(sess->fd, buf, (int)strlen(buf)) < (signed)strlen(buf)) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() sending query failed\n");
				goto fail_proxy_hub;
			}

			sess->state = GG_STATE_READING_DATA;
			sess->check = GG_CHECK_READ;
			sess->timeout = GG_DEFAULT_TIMEOUT;

			break;
		}

		case GG_STATE_READING_DATA:
		{
			char buf[1024], *tmp, *host;
			int port = GG_DEFAULT_PORT;
			struct in_addr addr;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_READING_DATA\n");

			/* czytamy linię z gniazda i obcinamy \r\n. */
			gg_read_line(sess->fd, buf, sizeof(buf) - 1);
			gg_chomp(buf);
			gg_debug_session(sess, GG_DEBUG_TRAFFIC, "// gg_watch_fd() received http header (%s)\n", buf);

			/* sprawdzamy, czy wszystko w porządku. */
			if (strncmp(buf, "HTTP/1.", 7) || strncmp(buf + 9, "200", 3)) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() invalid http reply, connection failed\n");
				goto fail_proxy_hub;
			}

			/* ignorujemy resztę nagłówka. */
			while (strcmp(buf, "\r\n") && strcmp(buf, ""))
				gg_read_line(sess->fd, buf, sizeof(buf) - 1);

			/* czytamy pierwszą linię danych. */
			if (gg_read_line(sess->fd, buf, sizeof(buf) - 1) == NULL) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() read error\n");
				goto fail_proxy_hub;
			}
			gg_chomp(buf);

			/* jeśli pierwsza liczba w linii nie jest równa zeru,
			 * oznacza to, że mamy wiadomość systemową. */
			if (atoi(buf)) {
				char tmp[1024], *foo, *sysmsg_buf = NULL;
				int len = 0;

				while (gg_read_line(sess->fd, tmp, sizeof(tmp) - 1)) {
					if (!(foo = (char*)realloc(sysmsg_buf, len + strlen(tmp) + 2))) {
						gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() out of memory for system message, ignoring\n");
						break;
					}

					sysmsg_buf = foo;

					if (!len)
						strcpy(sysmsg_buf, tmp);
					else
						strcat(sysmsg_buf, tmp);

					len += (int)strlen(tmp);
				}

				e->type = GG_EVENT_MSG;
				e->event.msg.msgclass = atoi(buf);
				e->event.msg.sender = 0;
				e->event.msg.message = (char*) sysmsg_buf;
			}

			gg_sock_close(sess->fd);
			sess->fd = -1;

			gg_debug_session(sess, GG_DEBUG_TRAFFIC, "// gg_watch_fd() received http data (%s)\n", buf);

			/* analizujemy otrzymane dane. */
			tmp = buf;

			while (*tmp && *tmp != ' ')
				tmp++;
			while (*tmp && *tmp == ' ')
				tmp++;
			while (*tmp && *tmp != ' ')
				tmp++;
			while (*tmp && *tmp == ' ')
				tmp++;
			host = tmp;
			while (*tmp && *tmp != ' ')
				tmp++;
			*tmp = 0;

			if ((tmp = strchr(host, ':'))) {
				*tmp = 0;
				port = atoi(tmp + 1);
			}

			if (strcmp(host, "") == 0) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() invalid response\n");
				e->event.failure = GG_FAILURE_HUB;
				goto fail;
			}

			if (!strcmp(host, "notoperating")) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() service unavailable\n", errno, strerror(errno));
				e->event.failure = GG_FAILURE_UNAVAILABLE;
				goto fail;
			}

			addr.s_addr = inet_addr(host);
			sess->server_addr = addr.s_addr;

			if (!gg_proxy_http_only && sess->proxy_addr && sess->proxy_port) {
				/* jeśli mamy proxy, łączymy się z nim. */
				if ((sess->fd = gg_connect(&sess->proxy_addr, sess->proxy_port, sess->async)) == -1) {
					/* nie wyszło? trudno. */
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection to proxy failed (errno=%d, %s)\n", errno, strerror(errno));
					e->event.failure = GG_FAILURE_PROXY;
					goto fail;
				}

				sess->state = GG_STATE_CONNECTING_GG;
				sess->check = GG_CHECK_WRITE;
				sess->timeout = GG_DEFAULT_TIMEOUT;
				sess->soft_timeout = 1;
				break;
			}

			sess->port = port;

			/* Jeśli podano nazwę, nie adres serwera... */
			if (sess->server_addr == INADDR_NONE) {
				if (sess->resolver_start(&sess->fd, &sess->resolver, host) == -1) {
					gg_debug(GG_DEBUG_MISC, "// gg_login() resolving failed (errno=%d, %s)\n", errno, strerror(errno));
					goto fail;
				}

				sess->state = GG_STATE_RESOLVING_GG;
				sess->check = GG_CHECK_READ;
				sess->timeout = GG_DEFAULT_TIMEOUT;
				break;
			}

			/* łączymy się z właściwym serwerem. */
			if ((sess->fd = gg_connect(&addr, sess->port, sess->async)) == -1) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s), trying https\n", errno, strerror(errno));

				sess->port = GG_HTTPS_PORT;

				/* nie wyszło? próbujemy portu 443. */
				if ((sess->fd = gg_connect(&addr, GG_HTTPS_PORT, sess->async)) == -1) {
					/* ostatnia deska ratunku zawiodła?
					 * w takim razie zwijamy manatki. */
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s)\n", errno, strerror(errno));
					e->event.failure = GG_FAILURE_CONNECTING;
					goto fail;
				}
			}

			sess->state = GG_STATE_CONNECTING_GG;
			sess->check = GG_CHECK_WRITE;
			sess->timeout = GG_DEFAULT_TIMEOUT;
			sess->soft_timeout = 1;

			break;
		}

		case GG_STATE_RESOLVING_GG:
		{
			struct in_addr addr;
			int failed = 0;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_RESOLVING_GG\n");

			if (gg_sock_read(sess->fd, &addr, sizeof(addr)) < (signed)sizeof(addr) || addr.s_addr == INADDR_NONE) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() resolving failed\n");
				failed = 1;
				errno2 = errno;
			}

			gg_sock_close(sess->fd);
			sess->fd = -1;

			sess->resolver_cleanup(&sess->resolver, 0);

			if (failed) {
				errno = errno2;
				e->event.failure = GG_FAILURE_RESOLVING;
				goto fail;
			}

			sess->server_addr = addr.s_addr;

			/* łączymy się z właściwym serwerem. */
			if ((sess->fd = gg_connect(&addr, sess->port, sess->async)) == -1) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s), trying https\n", errno, strerror(errno));

				sess->port = GG_HTTPS_PORT;

				/* nie wyszło? próbujemy portu 443. */
				if ((sess->fd = gg_connect(&addr, GG_HTTPS_PORT, sess->async)) == -1) {
					/* ostatnia deska ratunku zawiodła?
					 * w takim razie zwijamy manatki. */
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s)\n", errno, strerror(errno));
					e->event.failure = GG_FAILURE_CONNECTING;
					goto fail;
				}
			}

			sess->state = GG_STATE_CONNECTING_GG;
			sess->check = GG_CHECK_WRITE;
			sess->timeout = GG_DEFAULT_TIMEOUT;
			sess->soft_timeout = 1;

			break;
		}

		case GG_STATE_CONNECTING_GG:
		{
			int res = 0;
			int res_size = sizeof(res);

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_CONNECTING_GG\n");

			sess->soft_timeout = 0;

			/* jeśli wystąpił błąd podczas łączenia się... */
			if (sess->async && (sess->timeout == 0 || gg_getsockopt(sess->fd, SOL_SOCKET, SO_ERROR, &res, &res_size) || res)) {
				/* jeśli nie udało się połączenie z proxy,
				 * nie mamy czego próbować więcej. */
				if (sess->proxy_addr && sess->proxy_port) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection to proxy failed (errno=%d, %s)\n", res, strerror(res));
					e->event.failure = GG_FAILURE_PROXY;
					goto fail;
				}

				gg_sock_close(sess->fd);
				sess->fd = -1;

#ifdef ETIMEDOUT
				if (sess->timeout == 0)
					errno = ETIMEDOUT;
#endif

#if defined(GG_CONFIG_HAVE_GNUTLS) || defined(GG_CONFIG_HAVE_OPENSSL)
				/* jeśli logujemy się po TLS, nie próbujemy
				 * się łączyć już z niczym innym w przypadku
				 * błędu. nie dość, że nie ma sensu, to i
				 * trzeba by się bawić w tworzenie na nowo
				 * SSL i SSL_CTX. */

				if (sess->ssl) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s)\n", res, strerror(res));
					e->event.failure = GG_FAILURE_CONNECTING;
					goto fail;
				}
#endif

				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s), trying https\n", res, strerror(res));

				if (sess->port == GG_HTTPS_PORT) {
					e->event.failure = GG_FAILURE_CONNECTING;
					goto fail;
				}

				sess->port = GG_HTTPS_PORT;

				/* próbujemy na port 443. */
				if ((sess->fd = gg_connect(&sess->server_addr, sess->port, sess->async)) == -1) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s)\n", errno, strerror(errno));
					e->event.failure = GG_FAILURE_CONNECTING;
					goto fail;
				}

				sess->state = GG_STATE_CONNECTING_GG;
				sess->check = GG_CHECK_WRITE;
				sess->timeout = GG_DEFAULT_TIMEOUT;
				sess->soft_timeout = 1;

				break;
			}

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connected\n");

			if (gg_proxy_http_only)
				sess->proxy_port = 0;

			/* jeśli mamy proxy, wyślijmy zapytanie. */
			if (sess->proxy_addr && sess->proxy_port) {
				char buf[100], *auth = gg_proxy_auth();
				struct in_addr addr;

				if (sess->server_addr)
					addr.s_addr = sess->server_addr;
				else
					addr.s_addr = sess->hub_addr;

				snprintf(buf, sizeof(buf), "CONNECT %s:%d HTTP/1.0\r\n", inet_ntoa(addr), sess->port);

				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() proxy request:\n//   %s", buf);

				/* wysyłamy zapytanie. jest ono na tyle krótkie,
				 * że musi się zmieścić w buforze gniazda. jeśli
				 * write() zawiedzie, stało się coś złego. */
				if (gg_sock_write(sess->fd, buf, (int)strlen(buf)) < (signed)strlen(buf)) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() can't send proxy request\n");
					free(auth);
					e->event.failure = GG_FAILURE_PROXY;
					goto fail;
				}

				if (auth) {
					gg_debug_session(sess, GG_DEBUG_MISC, "//   %s", auth);
					if (gg_sock_write(sess->fd, auth, (int)strlen(auth)) < (signed)strlen(auth)) {
						gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() can't send proxy request\n");
						free(auth);
						e->event.failure = GG_FAILURE_PROXY;
						goto fail;
					}

					free(auth);
				}

				if (gg_sock_write(sess->fd, "\r\n", 2) < 2) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() can't send proxy request\n");
					e->event.failure = GG_FAILURE_PROXY;
					goto fail;
				}
			}

#ifdef GG_CONFIG_MIRANDA
			if (sess->tls) {
				sess->state = GG_STATE_TLS_NEGOTIATION;
				sess->check = GG_CHECK_WRITE;
				sess->timeout = GG_DEFAULT_TIMEOUT;

				break;
			}
#elif defined(GG_CONFIG_HAVE_GNUTLS) || defined(GG_CONFIG_HAVE_OPENSSL)
			if (sess->ssl != NULL) {
#ifdef GG_CONFIG_HAVE_GNUTLS
				gnutls_transport_set_ptr(GG_SESSION_GNUTLS(sess), (gnutls_transport_ptr_t) sess->fd);
#endif
#ifdef GG_CONFIG_HAVE_OPENSSL
				SSL_set_fd(sess->ssl, (int)sess->fd);
#endif

				sess->state = GG_STATE_TLS_NEGOTIATION;
				sess->check = GG_CHECK_WRITE;
				sess->timeout = GG_DEFAULT_TIMEOUT;

				break;
			}
#endif

			sess->state = GG_STATE_READING_KEY;
			sess->check = GG_CHECK_READ;
			sess->timeout = GG_DEFAULT_TIMEOUT;

			break;
		}

#ifdef GG_CONFIG_MIRANDA
		case GG_STATE_TLS_NEGOTIATION:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_TLS_NEGOTIATION\n");

			sess->ssl = Netlib_SslConnect(sess->fd, 0, 0);

			if (sess->ssl == NULL) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() TLS negotiation failed\n");

				e->type = GG_EVENT_CONN_FAILED;
				e->event.failure = GG_FAILURE_TLS;
				sess->state = GG_STATE_IDLE;
				gg_sock_close(sess->fd);
				sess->fd = -1;
				break;
			}

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() TLS negotiation succeded\n");

			sess->state = GG_STATE_READING_KEY;
			sess->check = GG_CHECK_READ;
			sess->timeout = GG_DEFAULT_TIMEOUT;

			break;
		}
#elif GG_CONFIG_HAVE_GNUTLS
		case GG_STATE_TLS_NEGOTIATION:
		{
			int res;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_TLS_NEGOTIATION\n");

gnutls_handshake_repeat:
			res = gnutls_handshake(GG_SESSION_GNUTLS(sess));

			if (res == GNUTLS_E_AGAIN) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() TLS handshake GNUTLS_E_AGAIN\n");

				sess->state = GG_STATE_TLS_NEGOTIATION;
				if (gnutls_record_get_direction(GG_SESSION_GNUTLS(sess)) == 0)
					sess->check = GG_CHECK_READ;
				else
					sess->check = GG_CHECK_WRITE;
				sess->timeout = GG_DEFAULT_TIMEOUT;
				break;
			}

			if (res == GNUTLS_E_INTERRUPTED) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() TLS handshake GNUTLS_E_INTERRUPTED\n");
				goto gnutls_handshake_repeat;
			}

			if (res != 0) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() TLS handshake error %d\n", res);
				e->type = GG_EVENT_CONN_FAILED;
				e->event.failure = GG_FAILURE_TLS;
				sess->state = GG_STATE_IDLE;
				gg_sock_close(sess->fd);
				sess->fd = -1;
				break;
			}

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() TLS negotiation succeded:\n");
			gg_debug_session(sess, GG_DEBUG_MISC, "//   cipher: VERS-%s:%s:%s:%s:COMP-%s\n",
				gnutls_protocol_get_name(gnutls_protocol_get_version(GG_SESSION_GNUTLS(sess))),
				gnutls_cipher_get_name(gnutls_cipher_get(GG_SESSION_GNUTLS(sess))),
				gnutls_kx_get_name(gnutls_kx_get(GG_SESSION_GNUTLS(sess))),
				gnutls_mac_get_name(gnutls_mac_get(GG_SESSION_GNUTLS(sess))),
				gnutls_compression_get_name(gnutls_compression_get(GG_SESSION_GNUTLS(sess))));

			if (gnutls_certificate_type_get(GG_SESSION_GNUTLS(sess)) == GNUTLS_CRT_X509) {
				unsigned int peer_count;
				const gnutls_datum_t *peers;
				gnutls_x509_crt_t cert;

				if (gnutls_x509_crt_init(&cert) == 0) {
					peers = gnutls_certificate_get_peers(GG_SESSION_GNUTLS(sess), &peer_count);

					if (peers != NULL) {
						char buf[256];
						size_t size;

						if (gnutls_x509_crt_import(cert, &peers[0], GNUTLS_X509_FMT_DER) == 0) {
							size = sizeof(buf);
							gnutls_x509_crt_get_dn(cert, buf, &size);
							gg_debug_session(sess, GG_DEBUG_MISC, "//   cert subject: %s\n", buf);
							size = sizeof(buf);
							gnutls_x509_crt_get_issuer_dn(cert, buf, &size);
							gg_debug_session(sess, GG_DEBUG_MISC, "//   cert issuer: %s\n", buf);
						}
					}

					gnutls_x509_crt_deinit(cert);
				}
			}

			sess->state = GG_STATE_READING_KEY;
			sess->check = GG_CHECK_READ;
			sess->timeout = GG_DEFAULT_TIMEOUT;

			break;
		}
#elif GG_CONFIG_HAVE_OPENSSL
		case GG_STATE_TLS_NEGOTIATION:
		{
			int res;
			X509 *peer;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_TLS_NEGOTIATION\n");

			if ((res = SSL_connect(sess->ssl)) <= 0) {
				int err = SSL_get_error(sess->ssl, res);

				if (res == 0) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() disconnected during TLS negotiation\n");

					e->type = GG_EVENT_CONN_FAILED;
					e->event.failure = GG_FAILURE_TLS;
					sess->state = GG_STATE_IDLE;
					gg_sock_close(sess->fd);
					sess->fd = -1;
					break;
				}

				if (err == SSL_ERROR_WANT_READ) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() SSL_connect() wants to read\n");

					sess->state = GG_STATE_TLS_NEGOTIATION;
					sess->check = GG_CHECK_READ;
					sess->timeout = GG_DEFAULT_TIMEOUT;

					break;
				} else if (err == SSL_ERROR_WANT_WRITE) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() SSL_connect() wants to write\n");

					sess->state = GG_STATE_TLS_NEGOTIATION;
					sess->check = GG_CHECK_WRITE;
					sess->timeout = GG_DEFAULT_TIMEOUT;

					break;
				} else {
					char buf[256];

					ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));

					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() SSL_connect() bailed out: %s\n", buf);

					e->type = GG_EVENT_CONN_FAILED;
					e->event.failure = GG_FAILURE_TLS;
					sess->state = GG_STATE_IDLE;
					gg_sock_close(sess->fd);
					sess->fd = -1;
					break;
				}
			}

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() TLS negotiation succeded:\n//   cipher: %s\n", SSL_get_cipher_name(sess->ssl));

			peer = SSL_get_peer_certificate(sess->ssl);

			if (!peer)
				gg_debug_session(sess, GG_DEBUG_MISC, "//   WARNING! unable to get peer certificate!\n");
			else {
				char buf[256];

				X509_NAME_oneline(X509_get_subject_name(peer), buf, sizeof(buf));
				gg_debug_session(sess, GG_DEBUG_MISC, "//   cert subject: %s\n", buf);

				X509_NAME_oneline(X509_get_issuer_name(peer), buf, sizeof(buf));
				gg_debug_session(sess, GG_DEBUG_MISC, "//   cert issuer: %s\n", buf);
			}

			sess->state = GG_STATE_READING_KEY;
			sess->check = GG_CHECK_READ;
			sess->timeout = GG_DEFAULT_TIMEOUT;

			break;
		}
#endif

		case GG_STATE_READING_KEY:
		case GG_STATE_READING_REPLY:
		case GG_STATE_CONNECTED:
		case GG_STATE_DISCONNECTING:
		{
			struct gg_header *gh;

			if (sess->state == GG_STATE_READING_KEY)
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_READING_KEY\n");
			else if (sess->state == GG_STATE_READING_REPLY)
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_READING_REPLY\n");
			else if (sess->state == GG_STATE_CONNECTED)
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_CONNECTED\n");
			else if (sess->state == GG_STATE_DISCONNECTING)
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_DISCONNECTING\n");

			/* XXX bardzo, bardzo, bardzo głupi pomysł na pozbycie
			 * się tekstu wrzucanego przez proxy. */
			if (sess->state == GG_STATE_READING_KEY && sess->proxy_addr && sess->proxy_port) {
				char buf[100];

				strcpy(buf, "");
				gg_read_line(sess->fd, buf, sizeof(buf) - 1);
				gg_chomp(buf);
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() proxy response:\n//   %s\n", buf);

				while (strcmp(buf, "")) {
					gg_read_line(sess->fd, buf, sizeof(buf) - 1);
					gg_chomp(buf);
					if (strcmp(buf, ""))
						gg_debug_session(sess, GG_DEBUG_MISC, "//   %s\n", buf);
				}

				/* XXX niech czeka jeszcze raz w tej samej
				 * fazie. głupio, ale działa. */
				sess->proxy_port = 0;

				break;
			}

			sess->last_event = (int)time(NULL);

			gh = (gg_header*)gg_recv_packet(sess);

			if (gh == NULL) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() gg_recv_packet failed (errno=%d, %s)\n", errno, strerror(errno));

 				if (errno != EAGAIN)
					goto fail;
			} else {
				if (gg_session_handle_packet(sess, gh->type, (const char *) gh + sizeof(struct gg_header), gh->length, e) == -1) {
					free(gh);
					goto fail;
				}

			free(gh);
			}

			sess->check = GG_CHECK_READ;

			break;
		}
	}

	if (sess->send_buf && (sess->state == GG_STATE_READING_REPLY || sess->state == GG_STATE_CONNECTED))
		sess->check |= GG_CHECK_WRITE;

	return e;

fail_proxy_hub:
	if (sess->proxy_port)
		e->event.failure = GG_FAILURE_PROXY;
	else
		e->event.failure = GG_FAILURE_HUB;

fail:
	sess->resolver_cleanup(&sess->resolver, 1);

	sess->state = GG_STATE_IDLE;

	if (sess->fd != -1) {
		int errno2;

		errno2 = errno;
		gg_sock_close(sess->fd);
		errno = errno2;
		sess->fd = -1;
	}

	if (e->event.failure != 0) {
		e->type = GG_EVENT_CONN_FAILED;
		return e;
	} else {
		free(e);
		return NULL;
	}
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

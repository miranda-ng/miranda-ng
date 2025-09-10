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
 *
 * \todo Poprawna obsługa gg_proxy_http_only
 */

#include "internal.h"

#include "strman.h"
#include "network.h"

#include "protocol.h"
#include "encoding.h"
#include "debug.h"
#include "session.h"
#include "resolver.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#ifdef GG_CONFIG_HAVE_GNUTLS
#  include <gnutls/gnutls.h>
#  include <gnutls/x509.h>
#endif
#ifdef GG_CONFIG_HAVE_OPENSSL
#  include <openssl/err.h>
#  include <openssl/x509.h>
#  include <openssl/rand.h>
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

		case GG_EVENT_JSON_EVENT:
			free(e->event.json_event.data);
			free(e->event.json_event.type);
			break;

		case GG_EVENT_USER_DATA:
		{
			unsigned int i, j;

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

		case GG_EVENT_IMTOKEN:
			free(e->event.imtoken.imtoken);
			break;

		case GG_EVENT_CHAT_INFO:
			free(e->event.chat_info.participants);
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
 * \internal Inicjalizuje struktury SSL.
 *
 * \param gs Struktura sesji
 *
 * \return 0 jeśli się powiodło, -1 jeśli wystąpił błąd
 */
int gg_session_init_ssl(struct gg_session *gs)
{
#ifdef GG_CONFIG_HAVE_GNUTLS
	gg_session_gnutls_t *tmp;

	tmp = (gg_session_gnutls_t*) gs->ssl;

	if (tmp == NULL) {
		tmp = malloc(sizeof(gg_session_gnutls_t));

		if (tmp == NULL) {
			gg_debug(GG_DEBUG_MISC, "// gg_session_connect() out of memory for GnuTLS session\n");
			return -1;
		}

		memset(tmp, 0, sizeof(gg_session_gnutls_t));

		gs->ssl = tmp;

		if (gnutls_global_init() != GNUTLS_E_SUCCESS) {
			gg_debug(GG_DEBUG_MISC, " // gg_session_init_ssl() "
				"gnutls_global_init failed\n");
			return -1;
		}
		tmp->global_init_called = 1;

		if (gnutls_certificate_allocate_credentials(&tmp->xcred) != GNUTLS_E_SUCCESS) {
			gg_debug(GG_DEBUG_MISC, " // gg_session_init_ssl() "
				"gnutls_certificate_allocate_credentials failed\n");
			return -1;
		}
		tmp->xcred_ready = 1;

#ifdef GG_CONFIG_SSL_SYSTEM_TRUST
#ifdef HAVE_GNUTLS_CERTIFICATE_SET_X509_SYSTEM_TRUST
		if (gnutls_certificate_set_x509_system_trust(tmp->xcred) < 0) {
			gg_debug(GG_DEBUG_MISC, " // gg_session_init_ssl() "
				"gnutls_certificate_set_x509_system_trust failed\n");
			return -1;
		}
#else
		if (gnutls_certificate_set_x509_trust_file(tmp->xcred,
			GG_CONFIG_GNUTLS_SYSTEM_TRUST_STORE,
			GNUTLS_X509_FMT_PEM) < 0)
		{
			gg_debug(GG_DEBUG_MISC, " // gg_session_init_ssl() "
				"gnutls_certificate_set_x509_trust_file failed\n");
			return -1;
		}
#endif
#endif
	} else {
		gnutls_deinit(tmp->session);
		tmp->session_ready = 0;
	}

	if (gnutls_init(&tmp->session, GNUTLS_CLIENT) != GNUTLS_E_SUCCESS) {
		gg_debug(GG_DEBUG_MISC, " // gg_session_init_ssl() gnutls_init failed\n");
		return -1;
	}
	tmp->session_ready = 1;

	if (gnutls_set_default_priority(tmp->session) != GNUTLS_E_SUCCESS) {
		gg_debug(GG_DEBUG_MISC, " // gg_session_init_ssl() "
			"gnutls_set_default_priority failed\n");
		return -1;
	}
	if (gnutls_credentials_set(tmp->session, GNUTLS_CRD_CERTIFICATE,
		tmp->xcred) != GNUTLS_E_SUCCESS)
	{
		gg_debug(GG_DEBUG_MISC, " // gg_session_init_ssl() "
			"gnutls_credentials_set failed\n");
		return -1;
	}
	gnutls_transport_set_ptr(tmp->session, (gnutls_transport_ptr_t) (intptr_t) gs->fd);
#endif

#ifdef GG_CONFIG_HAVE_OPENSSL
	char buf[1024];

	OpenSSL_add_ssl_algorithms();

	if (!RAND_status()) {
		char rdata[1024];
		struct {
			time_t time;
			void *ptr;
		} rstruct;

		time(&rstruct.time);
		rstruct.ptr = (void *) &rstruct;

		RAND_seed((void *) rdata, sizeof(rdata));
		RAND_seed((void *) &rstruct, sizeof(rstruct));
	}

	if (gs->ssl_ctx == NULL) {
#ifdef GG_CONFIG_HAVE_TLS_CLIENT_METHOD
		gs->ssl_ctx = SSL_CTX_new(TLS_client_method());
#elif defined(GG_CONFIG_HAVE_TLSV1_2_CLIENT_METHOD)
		gs->ssl_ctx = SSL_CTX_new(TLSv1_2_client_method());
#else
		gs->ssl_ctx = SSL_CTX_new(TLSv1_client_method());
#endif

		if (gs->ssl_ctx == NULL) {
			ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
			gg_debug(GG_DEBUG_MISC, "// gg_session_connect() SSL_CTX_new() failed: %s\n", buf);
			return -1;
		}

		SSL_CTX_set_verify(gs->ssl_ctx, SSL_VERIFY_NONE, NULL);
#ifdef GG_CONFIG_SSL_SYSTEM_TRUST
		SSL_CTX_set_default_verify_paths(gs->ssl_ctx);
#endif
	}

	if (gs->ssl != NULL)
		SSL_free(gs->ssl);

	gs->ssl = SSL_new(gs->ssl_ctx);

	if (gs->ssl == NULL) {
		ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
		gg_debug(GG_DEBUG_MISC, "// gg_session_connect() SSL_new() failed: %s\n", buf);
		return -1;
	}

	SSL_set_fd(gs->ssl, gs->fd);
#endif

	return 0;
}

/**
 * \internal Funkcja próbuje wysłać dane zakolejkowane do wysyłki.
 *
 * \param sess Struktura sesji
 *
 * \return 0 jeśli się powiodło, -1 jeśli wystąpił błąd
 */
static int gg_send_queued_data(struct gg_session *sess)
{
	int res;

	if (sess->send_buf == NULL || sess->send_left == 0)
		return 0;

	gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() sending %d bytes of queued data\n", sess->send_left);

	res = send(sess->fd, sess->send_buf, sess->send_left, 0);

	if (res == -1) {
		if (errno == EAGAIN || errno == EINTR) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd()"
				" non-critical send error (errno=%d, %s)\n",
				errno, strerror(errno));

			return 0;
		}

		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() send() "
			"failed (errno=%d, %s)\n", errno, strerror(errno));

		return -1;
	}

	if (res == sess->send_left) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() sent all queued data\n");
		free(sess->send_buf);
		sess->send_buf = NULL;
		sess->send_left = 0;
	} else if (res > 0) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() sent %d"
			" bytes of queued data, %d bytes left\n",
			res, sess->send_left - res);

		memmove(sess->send_buf, sess->send_buf + res, sess->send_left - res);
		sess->send_left -= res;
	}

	return 0;
}

/**
 * \internal Sprawdza wynik połączenia asynchronicznego.
 * \param gs Struktura sesji
 * \param res_ptr Wskaźnik na kod błędu
 * \return 0 jeśli się powiodło, -1 jeśli wystąpił błąd
 */
static int gg_async_connect_failed(struct gg_session *gs, int *res_ptr)
{
	int res = 0;
	socklen_t res_size = sizeof(res);

	if (!gs->async)
		return 0;

	if (gs->timeout == 0) {
		*res_ptr = ETIMEDOUT;
		return 1;
	}

	if (getsockopt(gs->fd, SOL_SOCKET, SO_ERROR, &res, &res_size) == -1) {
		*res_ptr = errno;
		return 1;
	}

	if (res != 0) {
		*res_ptr = res;
		return 1;
	}

	*res_ptr = 0;

	return 0;
}

typedef enum
{
	GG_ACTION_WAIT,
	GG_ACTION_NEXT,
	GG_ACTION_FAIL
} gg_action_t;

typedef gg_action_t (*gg_state_handler_t)(struct gg_session *gs,
	struct gg_event *ge, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state);

typedef struct
{
	enum gg_state_t state;
	gg_state_handler_t handler;
	enum gg_state_t next_state;
	enum gg_state_t alt_state;
	enum gg_state_t alt2_state;
} gg_state_transition_t;

/* zwraca:
 * -1 w przypadku błędu
 * 0 jeżeli nie ma ustawionego specjalnego managera gniazdek
 * 1 w przypadku powodzenia
 */
static int gg_handle_resolve_custom(struct gg_session *sess, enum gg_state_t next_state)
{
	struct gg_session_private *p = sess->private_data;
	int is_tls = 0;
	int port;

	if (p->socket_manager_type == GG_SOCKET_MANAGER_TYPE_INTERNAL)
		return 0;

	if (p->socket_manager.connect_cb == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC | GG_DEBUG_ERROR,
			"// gg_handle_resolve_custom() socket_manager.connect "
			"callback is empty\n");
		return -1;
	}

	if (p->socket_handle != NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC | GG_DEBUG_ERROR,
			"// gg_handle_resolve_custom() socket_handle is not "
			"NULL\n");
		return -1;
	}

	port = sess->connect_port[sess->connect_index];
	if (next_state == GG_STATE_SEND_HUB)
		port = GG_APPMSG_PORT;

	if (sess->ssl_flag != GG_SSL_DISABLED &&
		next_state == GG_STATE_READING_KEY)
	{
		/* XXX: w tej chwili nie ma możliwości łączenia się do HUBa po
		 * SSL, ale może będzie w przyszłości */
		is_tls = 1;
	}

	if (is_tls && p->socket_manager_type == GG_SOCKET_MANAGER_TYPE_TCP) {
		is_tls = 0;
		next_state = GG_STATE_TLS_NEGOTIATION;
	}

	if (port <= 0) {
		gg_debug_session(sess, GG_DEBUG_MISC | GG_DEBUG_ERROR,
			"// gg_handle_resolve_custom() port <= 0\n");
		return -1;
	}

	p->socket_failure = 0;
	p->socket_next_state = next_state;
	p->socket_handle = p->socket_manager.connect_cb(
		p->socket_manager.cb_data, sess->resolver_host, port, is_tls,
		sess->async, sess);

	if (p->socket_failure != 0) {
		if (p->socket_handle != NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC | GG_DEBUG_WARNING,
				"// gg_handle_resolve_custom() handle should be"
				" empty on error\n");
		}
		return -1;
	}

	if (p->socket_handle == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC | GG_DEBUG_ERROR,
			"// gg_handle_resolve_custom() returned empty "
			"handle\n");
		return -1;
	}

	return 1;
}

static gg_action_t gg_handle_resolve_sync(struct gg_session *sess,
	struct gg_event *e, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state)
{
	struct in_addr addr;
	int res;

	res = gg_handle_resolve_custom(sess, alt_state);
	if (res == 1)
		return GG_ACTION_NEXT;
	else if (res == -1)
		return GG_ACTION_FAIL;

	addr.s_addr = inet_addr(sess->resolver_host);

	if (addr.s_addr == INADDR_NONE) {
		struct in_addr *addr_list = NULL;
		unsigned int addr_count;

		if (gg_gethostbyname_real(sess->resolver_host, &addr_list, &addr_count, 0) == -1) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd()"
				" host %s not found\n", sess->resolver_host);
			e->event.failure = GG_FAILURE_RESOLVING;
			free(addr_list);
			return GG_ACTION_FAIL;
		}

		sess->resolver_result = addr_list;
		sess->resolver_count = addr_count;
		sess->resolver_index = 0;
	} else {
		sess->resolver_result = malloc(sizeof(struct in_addr));

		if (sess->resolver_result == NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() out of memory\n");
			return GG_ACTION_FAIL;
		}

		sess->resolver_result[0].s_addr = addr.s_addr;
		sess->resolver_count = 1;
		sess->resolver_index = 0;
	}

	sess->state = next_state;

	return GG_ACTION_NEXT;
}

static gg_action_t gg_handle_resolve_async(struct gg_session *sess,
	struct gg_event *e, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state)
{
	int res;

	res = gg_handle_resolve_custom(sess, alt_state);
	if (res == 1)
		return GG_ACTION_WAIT;
	else if (res == -1)
		return GG_ACTION_FAIL;

	if (sess->resolver_start(&sess->fd, &sess->resolver, sess->resolver_host) == -1) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() "
			"resolving failed (errno=%d, %s)\n",
			errno, strerror(errno));
		e->event.failure = GG_FAILURE_RESOLVING;
		return GG_ACTION_FAIL;
	}

	sess->state = next_state;
	sess->check = GG_CHECK_READ;
	sess->timeout = GG_DEFAULT_TIMEOUT;

	return GG_ACTION_WAIT;
}

static gg_action_t gg_handle_resolving(struct gg_session *sess,
	struct gg_event *e, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state)
{
	char buf[256];
	int count = -1;
	int res;
	unsigned int i;
	struct in_addr *addrs;

	res = gg_resolver_recv(sess->fd, buf, sizeof(buf));

	if (res == -1 && (errno == EAGAIN || errno == EINTR)) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() "
			"non-critical error (errno=%d, %s)\n",
			errno, strerror(errno));
		return GG_ACTION_WAIT;
	}

	sess->resolver_cleanup(&sess->resolver, 0);

	if (res == -1) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() read "
			"error (errno=%d, %s)\n", errno, strerror(errno));
		e->event.failure = GG_FAILURE_RESOLVING;
		return GG_ACTION_FAIL;
	}

	if (res > 0) {
		char *tmp;

		tmp = realloc(sess->recv_buf, sess->recv_done + res);

		if (tmp == NULL)
			return GG_ACTION_FAIL;

		sess->recv_buf = tmp;
		memcpy(sess->recv_buf + sess->recv_done, buf, res);
		sess->recv_done += res;
	}

	/* Sprawdź, czy mamy listę zakończoną INADDR_NONE */

	addrs = (struct in_addr *)(void *)sess->recv_buf;

	for (i = 0; i < sess->recv_done / sizeof(struct in_addr); i++) {
		if (addrs[i].s_addr == INADDR_NONE) {
			count = i;
			break;
		}
	}

	/* Nie znaleziono hosta */

	if (count == 0) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() host not found\n");
		e->event.failure = GG_FAILURE_RESOLVING;
		return GG_ACTION_FAIL;
	}

	/* Nie mamy pełnej listy, ale połączenie zerwane */

	if (res == 0 && count == -1) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection broken\n");
		e->event.failure = GG_FAILURE_RESOLVING;
		return GG_ACTION_FAIL;
	}

	/* Nie mamy pełnej listy, normalna sytuacja */

	if (count == -1)
		return GG_ACTION_WAIT;

#ifndef GG_DEBUG_DISABLE
	if ((gg_debug_level & GG_DEBUG_DUMP) && (count > 0)) {
		char *list;
		size_t len;

		len = 0;

		for (i = 0; i < (unsigned int) count; i++) {
			if (i > 0)
				len += 2;

			len += strlen(inet_ntoa(addrs[i]));
		}

		list = malloc(len + 1);

		if (list == NULL)
			return GG_ACTION_FAIL;

		list[0] = 0;

		for (i = 0; i < (unsigned int) count; i++) {
			if (i > 0)
				strcat(list, ", ");

			strcat(list, inet_ntoa(addrs[i]));
		}

		gg_debug_session(sess, GG_DEBUG_DUMP, "// gg_watch_fd() resolved: %s\n", list);

		free(list);
	}
#endif

	gg_close(sess);

	sess->state = next_state;
	sess->resolver_result = addrs;
	sess->resolver_count = count;
	sess->resolver_index = 0;
	sess->recv_buf = NULL;
	sess->recv_done = 0;

	return GG_ACTION_NEXT;
}

static gg_action_t gg_handle_connect(struct gg_session *sess,
	struct gg_event *e, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state)
{
	struct in_addr addr;
	int port;

	if (sess->resolver_index >= sess->resolver_count) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() out of addresses to connect to\n");
		e->event.failure = GG_FAILURE_CONNECTING;
		return GG_ACTION_FAIL;
	}

	addr = sess->resolver_result[sess->resolver_index];

	if (sess->state == GG_STATE_CONNECT_HUB) {
		sess->hub_addr = addr.s_addr;
		port = GG_APPMSG_PORT;
	} else {
		sess->proxy_addr = addr.s_addr;
		port = sess->proxy_port;
	}

	gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connecting to %s:%d\n", inet_ntoa(addr), port);

	sess->fd = gg_connect(&addr, port, sess->async);

	if (sess->fd == -1) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() "
			"connection failed (errno=%d, %s)\n",
			errno, strerror(errno));
		sess->resolver_index++;
		return GG_ACTION_NEXT;
	}

	sess->state = next_state;
	sess->check = GG_CHECK_WRITE;
	sess->timeout = GG_DEFAULT_TIMEOUT;
	sess->soft_timeout = 1;

	return GG_ACTION_WAIT;
}

static gg_action_t gg_handle_connecting(struct gg_session *sess,
	struct gg_event *e, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state)
{
	int res;

	sess->soft_timeout = 0;

	if (gg_async_connect_failed(sess, &res)) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() "
			"connection failed (errno=%d, %s)\n",
			res, strerror(res));
		gg_close(sess);
		sess->resolver_index++;
		sess->state = alt_state;
	} else {
		/* Z proxy zwykle łączymy się dwa razy, więc nie zwalniamy
		 * adresów IP po pierwszym połączeniu. */
		if (sess->state != GG_STATE_CONNECTING_PROXY_HUB) {
			free(sess->resolver_result);
			sess->resolver_result = NULL;
		}

		sess->state = next_state;
	}

	return GG_ACTION_NEXT;
}

static gg_action_t gg_handle_connect_gg(struct gg_session *sess,
	struct gg_event *e, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state)
{
	struct in_addr addr;
	uint16_t port;

	gg_debug_session(sess, GG_DEBUG_MISC, "resolver_index=%d, "
		"connect_index=%d, connect_port={%d,%d}\n",
		sess->resolver_index, sess->connect_index,
		sess->connect_port[0], sess->connect_port[1]);

	if ((unsigned int) sess->connect_index >=
		sizeof(sess->connect_port) / sizeof(sess->connect_port[0]) ||
		sess->connect_port[sess->connect_index] == 0)
	{
		sess->connect_index = 0;
		sess->resolver_index++;
		if (sess->resolver_index >= sess->resolver_count) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() out of addresses to connect to\n");
			e->event.failure = GG_FAILURE_CONNECTING;
			return GG_ACTION_FAIL;
		}
	}

	addr = sess->resolver_result[sess->resolver_index];
	port = sess->connect_port[sess->connect_index];

	gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connecting to %s:%d\n", inet_ntoa(addr), port);

	sess->server_addr = addr.s_addr;
	sess->fd = gg_connect(&addr, port, sess->async);

	if (sess->fd == -1) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() "
			"connection failed (errno=%d, %s)\n",
			errno, strerror(errno));
		sess->connect_index++;
		return GG_ACTION_NEXT;
	}

	sess->state = next_state;
	sess->check = GG_CHECK_WRITE;
	sess->timeout = GG_DEFAULT_TIMEOUT;
	sess->soft_timeout = 1;

	return GG_ACTION_WAIT;
}

static gg_action_t gg_handle_connecting_gg(struct gg_session *sess,
	struct gg_event *e, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state)
{
	int res;

	sess->soft_timeout = 0;

	/* jeśli wystąpił błąd podczas łączenia się... */
	if (gg_async_connect_failed(sess, &res)) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() "
			"connection failed (errno=%d, %s)\n",
			res, strerror(res));
		gg_close(sess);
		sess->connect_index++;
		sess->state = alt_state;
		return GG_ACTION_NEXT;
	}

	free(sess->resolver_result);
	sess->resolver_result = NULL;

	gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connected\n");

	if (sess->ssl_flag != GG_SSL_DISABLED) {
		if (gg_session_init_ssl(sess) == -1) {
			e->event.failure = GG_FAILURE_TLS;
			return GG_ACTION_FAIL;
		}

		sess->state = alt2_state;
		sess->check = GG_CHECK_WRITE;
		sess->timeout = GG_DEFAULT_TIMEOUT;

		return GG_ACTION_NEXT;
	} else {
		sess->state = next_state;
		sess->check = GG_CHECK_READ;
		sess->timeout = GG_DEFAULT_TIMEOUT;

		return GG_ACTION_WAIT;
	}
}

static gg_action_t gg_handle_send_hub(struct gg_session *sess,
	struct gg_event *e, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state)
{
	char *req, *client, *auth;
	const char *host;
	int res;
	int proxy;
	size_t req_len;

	if (sess->client_version != NULL && isdigit(sess->client_version[0]))
		client = gg_urlencode(sess->client_version);
	else if (sess->protocol_version <= GG_PROTOCOL_VERSION_100)
		client = gg_urlencode(GG_DEFAULT_CLIENT_VERSION_100);
	else /* sess->protocol_version >= GG_PROTOCOL_VERSION_110 */
		client = gg_urlencode(GG_DEFAULT_CLIENT_VERSION_110);

	if (client == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() out of memory for client version\n");
		return GG_ACTION_FAIL;
	}

	if (sess->proxy_addr && sess->proxy_port) {
		host = "http://" GG_APPMSG_HOST;
		proxy = 1;
	} else {
		host = "";
		proxy = 0;
	}

	auth = gg_proxy_auth();

	if (sess->ssl_flag != GG_SSL_DISABLED) {
		req = gg_saprintf
			("GET %s/appsvc/appmsg_ver11.asp?tls=1&fmnumber=%u&fmt=2&"
			"lastmsg=%d&version=%s&age=2&gender=1 HTTP/1.0\r\n"
			"Connection: close\r\n"
			"Host: " GG_APPMSG_HOST "\r\n"
			"%s"
			"\r\n", host, sess->uin, sess->last_sysmsg, client, (auth) ? auth : "");
	} else {
		req = gg_saprintf
			("GET %s/appsvc/appmsg_ver8.asp?fmnumber=%u&fmt=2&lastmsg=%d&version=%s HTTP/1.0\r\n"
			"Host: " GG_APPMSG_HOST "\r\n"
			"%s"
			"\r\n", host, sess->uin, sess->last_sysmsg, client, (auth) ? auth : "");
	}

	free(auth);
	free(client);

	if (req == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() out of memory\n");
		e->event.failure = GG_FAILURE_PROXY;
		return GG_ACTION_FAIL;
	}

	req_len = strlen(req);

	gg_debug_session(sess, GG_DEBUG_TRAFFIC, "// sending http query:\n%s", req);

	res = send(sess->fd, req, req_len, 0);

	free(req);

	if (res == -1 && errno != EINTR && errno != EAGAIN) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() sending query failed\n");
		e->event.failure = (!proxy) ? GG_FAILURE_HUB : GG_FAILURE_PROXY;
		return GG_ACTION_FAIL;
	}

	if ((size_t) res < req_len) {
		sess->state = alt_state;
		sess->check = GG_CHECK_WRITE;
		sess->timeout = GG_DEFAULT_TIMEOUT;
	} else {
		sess->state = next_state;
		sess->check = GG_CHECK_READ;
		sess->timeout = GG_DEFAULT_TIMEOUT;
	}

	return GG_ACTION_WAIT;
}

static gg_action_t gg_handle_sending_hub_proxy(struct gg_session *sess,
	struct gg_event *e, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state)
{
	if (gg_send_queued_data(sess) == -1) {
		e->event.failure = GG_FAILURE_WRITING;
		return GG_ACTION_FAIL;
	}

	if (sess->send_left > 0)
		return GG_ACTION_WAIT;

	sess->state = next_state;
	sess->check = GG_CHECK_READ;
	sess->timeout = GG_DEFAULT_TIMEOUT;

	return GG_ACTION_WAIT;
}

static gg_action_t gg_handle_reading_hub_proxy(struct gg_session *sess,
	struct gg_event *e, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state)
{
	char buf[1024], *tmp, host[129];
	int port = GG_DEFAULT_PORT;
	int reply;
	const char *body;
	struct in_addr addr;
	int res;
	char **host_white;
	char *host_white_default[] = GG_DEFAULT_HOST_WHITE_LIST;

	res = recv(sess->fd, buf, sizeof(buf), 0);

	if (res == -1 && (errno == EAGAIN || errno == EINTR)) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() "
			"non-critical recv error (errno=%d, %s)\n",
			errno, strerror(errno));
		return GG_ACTION_WAIT;
	}

	if (res == -1) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() recv "
			"error (errno=%d, %s)\n", errno, strerror(errno));
		e->event.failure = GG_FAILURE_CONNECTING;
		return GG_ACTION_FAIL;
	}

	if (res != 0) {
		tmp = realloc(sess->recv_buf, sess->recv_done + res + 1);

		if (tmp == NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() not enough memory for http reply\n");
			return GG_ACTION_FAIL;
		}

		sess->recv_buf = tmp;
		memcpy(sess->recv_buf + sess->recv_done, buf, res);
		sess->recv_done += res;
		sess->recv_buf[sess->recv_done] = 0;
	}

	if (res == 0 && sess->recv_buf == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection closed\n");
		e->event.failure = GG_FAILURE_CONNECTING;
		return GG_ACTION_FAIL;
	}

	if (res != 0)
		return GG_ACTION_WAIT;

	gg_debug_session(sess, GG_DEBUG_TRAFFIC, "// received http reply:\n%s", sess->recv_buf);

	res = sscanf(sess->recv_buf, "HTTP/1.%*d %3d ", &reply);

	/* sprawdzamy, czy wszystko w porządku. */
	if (res != 1 || reply != 200) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() invalid http reply, connection failed\n");
		e->event.failure = GG_FAILURE_CONNECTING;
		return GG_ACTION_FAIL;
	}

	/* szukamy początku treści */
	body = strstr(sess->recv_buf, "\r\n\r\n");

	if (body == NULL) {
		body = strstr(sess->recv_buf, "\n\n");

		if (body == NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() can't find body\n");
			e->event.failure = GG_FAILURE_CONNECTING;
			return GG_ACTION_FAIL;
		} else {
			body += 2;
		}
	} else {
		body += 4;
	}

	/* 17591 0 91.197.13.71:8074 91.197.13.71 */
	res = sscanf(body, "%d %*d %128s", &reply, host);

	if (res != 2) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() invalid hub reply, connection failed\n");
		e->event.failure = GG_FAILURE_CONNECTING;
		return GG_ACTION_FAIL;
	}

	gg_debug_session(sess, GG_DEBUG_MISC, "reply=%d, host=\"%s\"\n", reply, host);

	/* jeśli pierwsza liczba w linii nie jest równa zeru,
	 * oznacza to, że mamy wiadomość systemową. */
	if (reply != 0) {
		tmp = strchr(body, '\n');

		if (tmp != NULL) {
			e->type = GG_EVENT_MSG;
			e->event.msg.msgclass = reply;
			e->event.msg.sender = 0;
			e->event.msg.message = (unsigned char*) strdup(tmp + 1);

			if (e->event.msg.message == NULL) {
				gg_debug_session(sess, GG_DEBUG_MISC,
					"// gg_watch_fd() not enough memory "
					"for system message\n");
				return GG_ACTION_FAIL;
			}
		}
	}

	gg_close(sess);

	tmp = strchr(host, ':');

	if (tmp != NULL) {
		*tmp = 0;
		port = atoi(tmp + 1);
	}

	if (strcmp(host, "notoperating") == 0) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() service unavailable\n");
		e->event.failure = GG_FAILURE_UNAVAILABLE;
		return GG_ACTION_FAIL;
	}

	addr.s_addr = inet_addr(host);
	if (addr.s_addr == INADDR_NONE)
		addr.s_addr = 0;
	sess->server_addr = addr.s_addr;

	free(sess->recv_buf);
	sess->recv_buf = NULL;
	sess->recv_done = 0;

	if (sess->state != GG_STATE_READING_PROXY_HUB) {
		if (sess->port == 0) {
			sess->connect_port[0] = port;
			sess->connect_port[1] = (port != GG_HTTPS_PORT) ? GG_HTTPS_PORT : 0;
		} else {
			sess->connect_port[0] = sess->port;
			sess->connect_port[1] = 0;
		}
	} else {
		sess->connect_port[0] = (sess->port == 0) ? GG_HTTPS_PORT : sess->port;
		sess->connect_port[1] = 0;
	}

	free(sess->connect_host);
	sess->connect_host = strdup(host);

	if (sess->connect_host == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() not enough memory\n");
		return GG_ACTION_FAIL;
	}

	host_white = sess->private_data->host_white_list;
	if (!host_white)
		host_white = host_white_default;

	if (sess->ssl_flag == GG_SSL_REQUIRED && host_white[0] != NULL) {
		int host_ok = 0;
		char **it;
		int host_len;

		host_len = strlen(sess->connect_host);

		for (it = host_white; *it != NULL; it++) {
			const char *white = *it;
			int white_len, dom_offset;

			white_len = strlen(white);
			if (white_len > host_len)
				continue;

			dom_offset = host_len - white_len;
			if (strncasecmp(sess->connect_host + dom_offset, white,
				white_len) != 0)
			{
				continue;
			}

			if (white_len < host_len) {
				if (sess->connect_host[dom_offset - 1] != '.')
					continue;
			}

			host_ok = 1;
			break;
		}

		if (!host_ok) {
			gg_debug_session(sess, GG_DEBUG_MISC | GG_DEBUG_ERROR,
				"// gg_watch_fd() the HUB server returned "
				"a host that is not trusted (%s)\n",
				sess->connect_host);
			e->event.failure = GG_FAILURE_TLS;
			return GG_ACTION_FAIL;
		}
	}

	if (sess->state == GG_STATE_READING_HUB)
		sess->resolver_host = sess->connect_host;

	/* Jeśli łączymy się przez proxy, zacznijmy od początku listy */
	sess->resolver_index = 0;

	sess->state = (sess->async) ? next_state : alt_state;

	return GG_ACTION_NEXT;
}

static gg_action_t gg_handle_send_proxy_gg(struct gg_session *sess,
	struct gg_event *e, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state)
{
	char *req, *auth;
	size_t req_len;
	int res;

	gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() %s\n", gg_debug_state(sess->state));

	if (sess->connect_index > 1 || sess->connect_port[sess->connect_index] == 0) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() out of connection candidates\n");
		e->event.failure = GG_FAILURE_CONNECTING;
		return GG_ACTION_FAIL;
	}

	auth = gg_proxy_auth();

	req = gg_saprintf("CONNECT %s:%d HTTP/1.0\r\n%s\r\n",
		sess->connect_host, sess->connect_port[sess->connect_index],
		(auth) ? auth : "");

	free(auth);

	sess->connect_index++;

	if (req == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() out of memory\n");
		e->event.failure = GG_FAILURE_PROXY;
		return GG_ACTION_FAIL;
	}

	req_len = strlen(req);

	gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() proxy request:\n%s", req);

	res = send(sess->fd, req, req_len, 0);

	free(req);

	if (res == -1 && errno != EINTR && errno != EAGAIN) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() sending query failed\n");
		e->event.failure = GG_FAILURE_PROXY;
		return GG_ACTION_FAIL;
	}

	if ((size_t) res < req_len) {
		sess->state = alt_state;
		sess->check = GG_CHECK_WRITE;
		sess->timeout = GG_DEFAULT_TIMEOUT;
	} else {
		sess->state = next_state;
		sess->check = GG_CHECK_READ;
		sess->timeout = GG_DEFAULT_TIMEOUT;
	}

	return GG_ACTION_WAIT;
}

static gg_action_t gg_handle_tls_negotiation(struct gg_session *sess,
	struct gg_event *e, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state)
{
#if defined(GG_CONFIG_HAVE_GNUTLS) || defined(GG_CONFIG_HAVE_OPENSSL)
	int valid_hostname = 0;
#endif

#ifdef GG_CONFIG_HAVE_GNUTLS
	unsigned int status;
	int res;

	gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_TLS_NEGOTIATION\n");

	for (;;) {
		res = gnutls_handshake(GG_SESSION_GNUTLS(sess));

		if (res == GNUTLS_E_AGAIN) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() TLS handshake GNUTLS_E_AGAIN\n");

			if (gnutls_record_get_direction(GG_SESSION_GNUTLS(sess)) == 0)
				sess->check = GG_CHECK_READ;
			else
				sess->check = GG_CHECK_WRITE;
			sess->timeout = GG_DEFAULT_TIMEOUT;
			return GG_ACTION_WAIT;
		}

		if (res == GNUTLS_E_INTERRUPTED) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() TLS handshake GNUTLS_E_INTERRUPTED\n");
			continue;
		}

		if (res != GNUTLS_E_SUCCESS) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd()"
				" TLS handshake error: %d, %s\n",
				res, gnutls_strerror(res));
			e->event.failure = GG_FAILURE_TLS;
			return GG_ACTION_FAIL;
		}

		break;
	}

	gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() TLS negotiation succeeded:\n");
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
					if (gnutls_x509_crt_get_dn(cert, buf, &size) == 0) {
						gg_debug_session(sess, GG_DEBUG_MISC,
							"//   cert subject: %s\n", buf);
					}
					size = sizeof(buf);
					if (gnutls_x509_crt_get_issuer_dn(cert, buf, &size) == 0) {
						gg_debug_session(sess, GG_DEBUG_MISC,
							"//   cert issuer: %s\n", buf);
					}

					if (gnutls_x509_crt_check_hostname(cert, sess->connect_host) != 0)
						valid_hostname = 1;
				}
			}

			gnutls_x509_crt_deinit(cert);
		}
	}

	res = gnutls_certificate_verify_peers2(GG_SESSION_GNUTLS(sess), &status);

	if (res != 0 || status != 0) {
		gg_debug_session(sess, GG_DEBUG_MISC, "//   WARNING!  unable to"
			" verify peer certificate: 0x%x, %d, %s\n", status, res,
			gnutls_strerror(res));

		if (sess->ssl_flag == GG_SSL_REQUIRED) {
			e->event.failure = GG_FAILURE_TLS;
			return GG_ACTION_FAIL;
		}
	} else {
		gg_debug_session(sess, GG_DEBUG_MISC, "//   verified peer certificate\n");
	}


#elif defined GG_CONFIG_HAVE_OPENSSL

	X509 *peer;
	int res;

	gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() %s\n", gg_debug_state(sess->state));

	res = SSL_connect(GG_SESSION_OPENSSL(sess));

	if (res <= 0) {
		int err;

		err = SSL_get_error(GG_SESSION_OPENSSL(sess), res);

		if (res == 0) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() disconnected during TLS negotiation\n");
			e->event.failure = GG_FAILURE_TLS;
			return GG_ACTION_FAIL;
		}

		if (err == SSL_ERROR_WANT_READ) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() SSL_connect() wants to read\n");

			sess->check = GG_CHECK_READ;
			sess->timeout = GG_DEFAULT_TIMEOUT;
			return GG_ACTION_WAIT;
		} else if (err == SSL_ERROR_WANT_WRITE) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() SSL_connect() wants to write\n");

			sess->check = GG_CHECK_WRITE;
			sess->timeout = GG_DEFAULT_TIMEOUT;
			return GG_ACTION_WAIT;
		} else {
			char buf[256];

			ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() SSL_connect() bailed out: %s\n", buf);

			e->event.failure = GG_FAILURE_TLS;
			return GG_ACTION_FAIL;
		}
	}

	gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() TLS negotiation"
		" succeeded:\n//   cipher: %s\n",
		SSL_get_cipher_name(GG_SESSION_OPENSSL(sess)));

	peer = SSL_get_peer_certificate(GG_SESSION_OPENSSL(sess));

	if (peer == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "//   WARNING! unable to get peer certificate!\n");

		if (sess->ssl_flag == GG_SSL_REQUIRED) {
			e->event.failure = GG_FAILURE_TLS;
			return GG_ACTION_FAIL;
		}
	} else {
		char buf[256];
		long res;

		X509_NAME_oneline(X509_get_subject_name(peer), buf, sizeof(buf));
		gg_debug_session(sess, GG_DEBUG_MISC, "//   cert subject: %s\n", buf);

		X509_NAME_oneline(X509_get_issuer_name(peer), buf, sizeof(buf));
		gg_debug_session(sess, GG_DEBUG_MISC, "//   cert issuer: %s\n", buf);

		res = SSL_get_verify_result(GG_SESSION_OPENSSL(sess));

		if (res != X509_V_OK) {
			gg_debug_session(sess, GG_DEBUG_MISC, "//   WARNING!  "
				"unable to verify peer certificate! "
				"res=%ld\n", res);

			if (sess->ssl_flag == GG_SSL_REQUIRED) {
				e->event.failure = GG_FAILURE_TLS;
				return GG_ACTION_FAIL;
			}
		} else {
			gg_debug_session(sess, GG_DEBUG_MISC, "//   verified peer certificate\n");
		}

		if (X509_NAME_get_text_by_NID(X509_get_subject_name(peer), NID_commonName, buf, sizeof(buf)) == -1)
			buf[0] = 0;

		/* Obsługa certyfikatów z wieloznacznikiem */
		if (strchr(buf, '*') == buf && strchr(buf + 1, '*') == NULL) {
			char *tmp;

			tmp = strchr(sess->connect_host, '.');

			if (tmp != NULL)
				valid_hostname = (strcasecmp(tmp, buf + 1) == 0);
		} else {
			valid_hostname = (strcasecmp(sess->connect_host, buf) == 0);
		}
	}

#else

	gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() no SSL support\n");
	e->event.failure = GG_FAILURE_TLS;
	return GG_ACTION_FAIL;

#endif

#if defined(GG_CONFIG_HAVE_GNUTLS) || defined(GG_CONFIG_HAVE_OPENSSL)
	if (!valid_hostname) {
		gg_debug_session(sess, GG_DEBUG_MISC, "//   WARNING!  unable to verify hostname\n");

		if (sess->ssl_flag == GG_SSL_REQUIRED) {
			e->event.failure = GG_FAILURE_TLS;
			return GG_ACTION_FAIL;
		}
	}

	sess->state = next_state;
	sess->check = GG_CHECK_READ;
	sess->timeout = GG_DEFAULT_TIMEOUT;

	return GG_ACTION_WAIT;
#endif
}

static gg_action_t gg_handle_reading_proxy_gg(struct gg_session *sess,
	struct gg_event *e, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state)
{
	char buf[256];
	int res;
	int reply;
	char *body;

	res = recv(sess->fd, buf, sizeof(buf), 0);

	gg_debug_session(sess, GG_DEBUG_MISC, "recv() = %d\n", res);

	if (res == -1 && (errno == EAGAIN || errno == EINTR)) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() "
			"non-critical recv error (errno=%d, %s)\n",
			errno, strerror(errno));
		return GG_ACTION_WAIT;
	}

	if (res == -1) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() recv "
			"error (errno=%d, %s)\n", errno, strerror(errno));
		e->event.failure = GG_FAILURE_CONNECTING;
		return GG_ACTION_FAIL;
	}

	if (res != 0) {
		char *tmp;

		tmp = realloc(sess->recv_buf, sess->recv_done + res + 1);

		if (tmp == NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() not enough memory for http reply\n");
			return GG_ACTION_FAIL;
		}

		sess->recv_buf = tmp;
		memcpy(sess->recv_buf + sess->recv_done, buf, res);
		sess->recv_done += res;
		sess->recv_buf[sess->recv_done] = 0;
	}

	if (res == 0 && sess->recv_buf == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection closed\n");
		e->event.failure = GG_FAILURE_CONNECTING;
		return GG_ACTION_FAIL;
	}

	/* szukamy początku treści */
	body = strstr(sess->recv_buf, "\r\n\r\n");

	if (body == NULL) {
		body = strstr(sess->recv_buf, "\n\n");

		if (body == NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() can't find body\n");
			e->event.failure = GG_FAILURE_CONNECTING;
			return GG_ACTION_FAIL;
		} else {
			body += 2;
		}
	} else {
		body += 4;
	}

	gg_debug_session(sess, GG_DEBUG_MISC, "// found body!\n");

	gg_debug_session(sess, GG_DEBUG_TRAFFIC, "// received proxy reply:\n%s\n", sess->recv_buf);

	res = sscanf(sess->recv_buf, "HTTP/1.%*d %3d ", &reply);

	gg_debug_session(sess, GG_DEBUG_MISC, "res = %d, reply = %d\n", res, reply);

	/* sprawdzamy, czy wszystko w porządku. */
	if (res != 1 || reply != 200) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() invalid http reply, connection failed\n");
		e->event.failure = GG_FAILURE_CONNECTING;
		return GG_ACTION_FAIL;
	}

	if (sess->ssl_flag != GG_SSL_DISABLED) {
		if (gg_session_init_ssl(sess) == -1) {
			e->event.failure = GG_FAILURE_TLS;
			return GG_ACTION_FAIL;
		}

		/* Teoretycznie SSL jest inicjowany przez klienta, więc serwer
		 * nie powinien niczego wysłać. */
		if (sess->recv_buf + sess->recv_done > body) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() unexpected SSL data\n");
			e->event.failure = GG_FAILURE_TLS;
			return GG_ACTION_FAIL;
		}

		free(sess->recv_buf);
		sess->recv_buf = NULL;
		sess->recv_done = 0;

		sess->state = alt_state;
		sess->check = GG_CHECK_WRITE;
		sess->timeout = GG_DEFAULT_TIMEOUT;

		return GG_ACTION_WAIT;
	}

	sess->state = next_state;
	sess->check = GG_CHECK_READ;
	sess->timeout = GG_DEFAULT_TIMEOUT;	/* Pierwszy pakiet musi przyjść */

	/* Jeśli zbuforowaliśmy za dużo, przeanalizuj */

	if (sess->recv_buf + sess->recv_done > body) {
		sess->recv_done = sess->recv_done - (body - sess->recv_buf);
		memmove(sess->recv_buf, body, sess->recv_done);
		sess->state = alt2_state;
		return GG_ACTION_NEXT;
	} else {
		free(sess->recv_buf);
		sess->recv_buf = NULL;
		sess->recv_done = 0;
	}

	return GG_ACTION_WAIT;
}

static gg_action_t gg_handle_connected(struct gg_session *sess,
	struct gg_event *e, enum gg_state_t next_state,
	enum gg_state_t alt_state, enum gg_state_t alt2_state)
{
#if 0
	char buf[1024];
	int res;

	if (gg_send_queued_data(sess) == -1)
		return GG_ACTION_FAIL;

	res = gg_read(sess, buf, sizeof(buf));

	if (res == -1 && (errno == EAGAIN || errno == EINTR)) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() "
			"non-critical read error (errno=%d, %s)\n",
			errno, strerror(errno));
		return GG_ACTION_WAIT;
	}

	if (res == -1 || res == 0) {
		if (res == -1) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd()"
				" read error (errno=%d, %s)\n",
				errno, strerror(errno));
		} else {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd()"
				" connection closed\n");
		}

		if (sess->state == GG_STATE_DISCONNECTING && res == 0) {
			e->type = GG_EVENT_DISCONNECT_ACK;
		} else if (sess->state == GG_STATE_READING_KEY) {
			e->event.failure = GG_FAILURE_INVALID;
			return GG_ACTION_FAIL;
		}

		return GG_ACTION_FAIL;
	}

	gg_debug_dump(sess, GG_DEBUG_DUMP, buf, res);

	if (gg_session_handle_data(sess, buf, res, e) == -1)
		return GG_ACTION_FAIL;

	if (sess->send_buf != NULL)
		sess->check |= GG_CHECK_WRITE;

	return GG_ACTION_WAIT;
#else
	struct gg_header *gh;

	if (gg_send_queued_data(sess) == -1)
		return GG_ACTION_FAIL;

	gh = gg_recv_packet(sess);

	if (gh == NULL) {
		if (sess->state == GG_STATE_DISCONNECTING) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection broken expectedly\n");
			e->type = GG_EVENT_DISCONNECT_ACK;
			return GG_ACTION_WAIT;
		}

		if (errno != EAGAIN) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd()"
				" gg_recv_packet failed (errno=%d, %s)\n",
				errno, strerror(errno));
			return GG_ACTION_FAIL;
		}
	} else {
		if (gg_session_handle_packet(sess, gh->type,
			(const char *) gh + sizeof(struct gg_header),
			gh->length, e) == -1)
		{
			free(gh);
			return GG_ACTION_FAIL;
		}

		free(gh);
	}

	sess->check = GG_CHECK_READ;

	if (sess->send_buf != NULL)
		sess->check |= GG_CHECK_WRITE;

	return GG_ACTION_WAIT;
#endif
}

static gg_action_t gg_handle_error(struct gg_session *sess, struct gg_event *e,
	enum gg_state_t next_state, enum gg_state_t alt_state,
	enum gg_state_t alt2_state)
{
	struct gg_session_private *p = sess->private_data;

	gg_debug_session(sess, GG_DEBUG_MISC | GG_DEBUG_ERROR, "// gg_handle_error() failure=%d\n", p->socket_failure);

	e->event.failure = p->socket_failure;

	return GG_ACTION_FAIL;
}

static const gg_state_transition_t handlers[] =
{
	/* style:maxlinelength:start-ignore */
	{ GG_STATE_RESOLVE_HUB_SYNC, gg_handle_resolve_sync, GG_STATE_CONNECT_HUB, GG_STATE_SEND_HUB, 0 },
	{ GG_STATE_RESOLVE_GG_SYNC, gg_handle_resolve_sync, GG_STATE_CONNECT_GG, GG_STATE_READING_KEY, 0 },
	{ GG_STATE_RESOLVE_PROXY_HUB_SYNC, gg_handle_resolve_sync, GG_STATE_CONNECT_PROXY_HUB, GG_STATE_SEND_PROXY_HUB, 0 },
	{ GG_STATE_RESOLVE_PROXY_GG_SYNC, gg_handle_resolve_sync, GG_STATE_CONNECT_PROXY_GG, GG_STATE_SEND_PROXY_GG, 0 },

	{ GG_STATE_RESOLVE_HUB_ASYNC, gg_handle_resolve_async, GG_STATE_RESOLVING_HUB, GG_STATE_SEND_HUB, 0 },
	{ GG_STATE_RESOLVE_GG_ASYNC, gg_handle_resolve_async, GG_STATE_RESOLVING_GG, GG_STATE_READING_KEY, 0 },
	{ GG_STATE_RESOLVE_PROXY_HUB_ASYNC, gg_handle_resolve_async, GG_STATE_RESOLVING_PROXY_HUB, GG_STATE_SEND_PROXY_HUB, 0 },
	{ GG_STATE_RESOLVE_PROXY_GG_ASYNC, gg_handle_resolve_async, GG_STATE_RESOLVING_PROXY_GG, GG_STATE_SEND_PROXY_GG, 0 },

	{ GG_STATE_RESOLVING_HUB, gg_handle_resolving, GG_STATE_CONNECT_HUB, 0, 0 },
	{ GG_STATE_RESOLVING_GG, gg_handle_resolving, GG_STATE_CONNECT_GG, 0, 0 },
	{ GG_STATE_RESOLVING_PROXY_HUB, gg_handle_resolving, GG_STATE_CONNECT_PROXY_HUB, 0, 0 },
	{ GG_STATE_RESOLVING_PROXY_GG, gg_handle_resolving, GG_STATE_CONNECT_PROXY_GG, 0, 0 },

	{ GG_STATE_CONNECT_HUB, gg_handle_connect, GG_STATE_CONNECTING_HUB, 0, 0 },
	{ GG_STATE_CONNECT_PROXY_HUB, gg_handle_connect, GG_STATE_CONNECTING_PROXY_HUB, 0, 0 },
	{ GG_STATE_CONNECT_PROXY_GG, gg_handle_connect, GG_STATE_CONNECTING_PROXY_GG, 0, 0 },

	{ GG_STATE_CONNECT_GG, gg_handle_connect_gg, GG_STATE_CONNECTING_GG, 0, 0 },

	{ GG_STATE_CONNECTING_HUB, gg_handle_connecting, GG_STATE_SEND_HUB, GG_STATE_CONNECT_HUB, 0 },
	{ GG_STATE_CONNECTING_PROXY_HUB, gg_handle_connecting, GG_STATE_SEND_PROXY_HUB, GG_STATE_CONNECT_PROXY_HUB, 0 },
	{ GG_STATE_CONNECTING_PROXY_GG, gg_handle_connecting, GG_STATE_SEND_PROXY_GG, GG_STATE_CONNECT_PROXY_GG, 0 },

	{ GG_STATE_CONNECTING_GG, gg_handle_connecting_gg, GG_STATE_READING_KEY, GG_STATE_CONNECT_GG, GG_STATE_TLS_NEGOTIATION },

	{ GG_STATE_SEND_HUB, gg_handle_send_hub, GG_STATE_READING_HUB, GG_STATE_SENDING_HUB, 0 },
	{ GG_STATE_SEND_PROXY_HUB, gg_handle_send_hub, GG_STATE_READING_PROXY_HUB, GG_STATE_SENDING_PROXY_HUB, 0 },

	{ GG_STATE_SEND_PROXY_GG, gg_handle_send_proxy_gg, GG_STATE_READING_PROXY_GG, GG_STATE_SENDING_PROXY_GG, 0 },

	{ GG_STATE_SENDING_HUB, gg_handle_sending_hub_proxy, GG_STATE_READING_HUB, 0, 0 },
	{ GG_STATE_SENDING_PROXY_HUB, gg_handle_sending_hub_proxy, GG_STATE_READING_PROXY_HUB, 0, 0 },
	{ GG_STATE_SENDING_PROXY_GG, gg_handle_sending_hub_proxy, GG_STATE_READING_PROXY_GG, 0, 0 },

	{ GG_STATE_READING_HUB, gg_handle_reading_hub_proxy, GG_STATE_RESOLVE_GG_ASYNC, GG_STATE_RESOLVE_GG_SYNC, 0 },
	{ GG_STATE_READING_PROXY_HUB, gg_handle_reading_hub_proxy, GG_STATE_CONNECT_PROXY_GG, GG_STATE_CONNECT_PROXY_GG, 0 },

	{ GG_STATE_READING_PROXY_GG, gg_handle_reading_proxy_gg, GG_STATE_READING_KEY, GG_STATE_TLS_NEGOTIATION, GG_STATE_READING_KEY },

	{ GG_STATE_TLS_NEGOTIATION, gg_handle_tls_negotiation, GG_STATE_READING_KEY, 0, 0 },

	{ GG_STATE_READING_KEY, gg_handle_connected, 0, 0, 0 },
	{ GG_STATE_READING_REPLY, gg_handle_connected, 0, 0, 0 },
	{ GG_STATE_CONNECTED, gg_handle_connected, 0, 0, 0 },
	{ GG_STATE_DISCONNECTING, gg_handle_connected, 0, 0, 0 },
	{ GG_STATE_ERROR, gg_handle_error, 0, 0, 0 },
	/* style:maxlinelength:end-ignore */
};

struct gg_event *gg_eventqueue_add(struct gg_session *sess)
{
	struct gg_event *ge;
	gg_eventqueue_t *queue_el, *it;

	queue_el = gg_new0(sizeof(gg_eventqueue_t));
	ge = gg_new0(sizeof(struct gg_event));

	if (queue_el == NULL || ge == NULL) {
		free(queue_el);
		free(ge);
		return NULL;
	}

	ge->type = GG_EVENT_NONE;

	queue_el->event = ge;
	if (sess->private_data->event_queue == NULL)
		sess->private_data->event_queue = queue_el;
	else {
		it = sess->private_data->event_queue;
		while (it->next != NULL)
			it = it->next;
		it->next = queue_el;
	}

	return ge;
}

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
	struct gg_event *ge;
	struct gg_session_private *priv;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_watch_fd(%p);\n", sess);

	if (sess == NULL) {
		errno = EFAULT;
		return NULL;
	}

	priv = sess->private_data;

	if (priv->event_queue != NULL) {
		gg_eventqueue_t *next;

		ge = priv->event_queue->event;
		next = priv->event_queue->next;
		free(priv->event_queue);
		priv->event_queue = next;

		if (next == NULL) {
			sess->check = priv->check_after_queue;
			sess->fd = priv->fd_after_queue;
		}
		return ge;
	}

	ge = malloc(sizeof(struct gg_event));

	if (ge == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() not enough memory for event data\n");
		return NULL;
	}

	memset(ge, 0, sizeof(struct gg_event));

	ge->type = GG_EVENT_NONE;

	for (;;) {
		unsigned int i, found = 0;
		gg_action_t res;

		res = GG_ACTION_FAIL;

		for (i = 0; i < sizeof(handlers) / sizeof(handlers[0]); i++) {
			if (handlers[i].state == (enum gg_state_t) sess->state) {
				gg_debug_session(sess, GG_DEBUG_MISC,
					"// gg_watch_fd() %s\n",
					gg_debug_state(sess->state));
				res = (*handlers[i].handler)(sess, ge,
					handlers[i].next_state,
					handlers[i].alt_state,
					handlers[i].alt2_state);
				found = 1;
				break;
			}
		}

		if (!found) {
			gg_debug_session(sess, GG_DEBUG_MISC | GG_DEBUG_ERROR,
				"// gg_watch_fd() invalid state %s\n",
				gg_debug_state(sess->state));
			ge->event.failure = GG_FAILURE_INTERNAL;
		}

		if (!sess->async && ge->type == GG_EVENT_NONE && res == GG_ACTION_WAIT)
			res = GG_ACTION_NEXT;

		switch (res) {
			case GG_ACTION_WAIT:
				if (priv->event_queue != NULL) {
					priv->fd_after_queue = sess->fd;
					priv->check_after_queue = sess->check;
					/* wymuszamy ponowne wywołanie gg_watch_fd */
					sess->fd = gg_get_dummy_fd(sess);
					if (sess->fd < 0)
						sess->fd = priv->fd_after_queue;
					sess->check = GG_CHECK_READ | GG_CHECK_WRITE;
				}
				return ge;

			case GG_ACTION_NEXT:
				continue;

			case GG_ACTION_FAIL:
				sess->state = GG_STATE_IDLE;

				gg_close(sess);

				if (ge->event.failure != 0) {
					ge->type = GG_EVENT_CONN_FAILED;
				} else {
					free(ge);
					ge = NULL;
				}

				return ge;

			/* Celowo nie ma default */
		}
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

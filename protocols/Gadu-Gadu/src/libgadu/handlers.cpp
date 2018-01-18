/*
 *  (C) Copyright 2001-2011 Wojtek Kaniewski <wojtekka@irc.pl>
 *                          Robert J. Woźny <speedy@ziew.org>
 *                          Arkadiusz Miśkiewicz <arekm@pld-linux.org>
 *                          Tomasz Chiliński <chilek@chilan.com>
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
 * \file handlers.c
 *
 * \brief Funkcje obsługi przychodzących pakietów
 */

#ifndef _WIN64
#define _USE_32BIT_TIME_T
#endif

#include <sys/types.h>
#ifdef _WIN32
#include "win32.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#ifdef sun
#  include <sys/filio.h>
#endif
#endif /* _WIN32 */

#include "compat.h"
#include "libgadu.h"
#include "resolver.h"
#include "session.h"
#include "protocol.h"
#include "encoding.h"
#include "message.h"
#include "internal.h"
#include "deflate.h"

#include <errno.h>
#ifndef _WIN32
#include <netdb.h>
#endif /* _WIN32 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifndef _WIN32
#include <unistd.h>
#endif /* _WIN32 */
#include <time.h>
#ifndef GG_CONFIG_MIRANDA
#ifdef GG_CONFIG_HAVE_OPENSSL
#  include <openssl/err.h>
#  include <openssl/rand.h>
#endif
#endif

/**
 * \internal Struktura opisująca funkcję obsługi pakietu.
 */
typedef struct {
	/* Typ pakietu */
	uint32_t type;
	/* Stan w którym pakiet jest obsługiwany */
	int state;
	/* Minimalny rozmiar danych pakietu */
	int min_length;
	/* Funkcja obsługująca pakiet. Patrz gg_session_handle_packet(). */
	int (*handler)(struct gg_session *, uint32_t, const char *, size_t, struct gg_event *);
} gg_packet_handler_t;

/**
 * \internal Obsługuje pakiet GG_WELCOME.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_welcome(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_welcome *w;
	int ret;
	uint8_t hash_buf[64];
	uint32_t local_ip;
	struct sockaddr_in sin;
	int sin_len = sizeof(sin);

	if (len < sizeof(struct gg_welcome)) {
		ge->type = GG_EVENT_CONN_FAILED;
		ge->event.failure = GG_FAILURE_INVALID;
		gs->state = GG_STATE_IDLE;
		close(gs->fd);
		gs->fd = -1;
		return 0;
	}

	w = (struct gg_welcome*) ptr;
	w->key = gg_fix32(w->key);

	memset(hash_buf, 0, sizeof(hash_buf));

	switch (gs->hash_type) {
		case GG_LOGIN_HASH_GG32:
		{
			uint32_t hash;

			hash = gg_fix32(gg_login_hash((unsigned char*) gs->password, w->key));
			gg_debug_session(gs, GG_DEBUG_DUMP, "// gg_watch_fd() challenge %.4x --> GG32 hash %.8x\n", w->key, hash);
			memcpy(hash_buf, &hash, sizeof(hash));

			break;
		}

		case GG_LOGIN_HASH_SHA1:
		{
#ifndef GG_DEBUG_DISABLE
			char tmp[41];
			int i;
#endif

			gg_login_hash_sha1(gs->password, w->key, hash_buf);

#ifndef GG_DEBUG_DISABLE
			for (i = 0; i < 40; i += 2)
				snprintf(tmp + i, sizeof(tmp) - i, "%02x", hash_buf[i / 2]);

			gg_debug_session(gs, GG_DEBUG_DUMP, "// gg_watch_fd() challenge %.4x --> SHA1 hash: %s\n", w->key, tmp);
#endif

			break;
		}

		default:
			break;
	}

#if 0
	if (gs->password != NULL && (gs->flags & (1 << GG_SESSION_FLAG_CLEAR_PASSWORD))) {
		memset(gs->password, 0, strlen(gs->password));
		free(gs->password);
		gs->password = NULL;
	}
#endif

	if (!getsockname(gs->fd, (struct sockaddr*) &sin, &sin_len)) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() detected address to %s\n", inet_ntoa(sin.sin_addr));
		local_ip = sin.sin_addr.s_addr;
	} else {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() unable to detect address\n");
		local_ip = 0;
	}

	if (GG_SESSION_IS_PROTOCOL_8_0(gs)) {
		struct gg_login80 l80;
		const char *client_name, *version, *descr;
		uint32_t client_name_len, version_len, descr_len;

		if (gs->external_addr == 0)
			gs->external_addr = local_ip;

		memset(&l80, 0, sizeof(l80));
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() sending GG_LOGIN80 packet\n");
		l80.uin = gg_fix32(gs->uin);
		memcpy(l80.language, GG8_LANG, sizeof(l80.language));
		l80.hash_type = gs->hash_type;
		memcpy(l80.hash, hash_buf, sizeof(l80.hash));
		l80.status = gg_fix32(gs->initial_status ? gs->initial_status : GG_STATUS_AVAIL);
		l80.flags = gg_fix32(gs->status_flags);
		l80.features = gg_fix32(gs->protocol_features);
		l80.image_size = gs->image_size;
		l80.dunno2 = 0x64;

		if (gs->client_version != NULL && !isdigit(gs->client_version[0])) {
			client_name = "";
			client_name_len = 0;
		} else {
			client_name = GG8_VERSION;
			client_name_len = strlen(GG8_VERSION);
		}

		version = (gs->client_version != NULL) ? gs->client_version : GG_DEFAULT_CLIENT_VERSION;
		version_len = gg_fix32(client_name_len + strlen(version));

		descr = (gs->initial_descr != NULL) ? gs->initial_descr : "";
		descr_len = (gs->initial_descr != NULL) ? gg_fix32(strlen(gs->initial_descr)) : 0;

		ret = gg_send_packet(gs,
				GG_LOGIN80,
				&l80, sizeof(l80),
				&version_len, sizeof(version_len),
				client_name, client_name_len,
				version, strlen(version),
				&descr_len, sizeof(descr_len),
				descr, strlen(descr),
				NULL);
	} else {
		struct gg_login70 l70;

		if (gg_dcc_ip != (unsigned long) inet_addr("255.255.255.255"))
			local_ip = gg_dcc_ip;

		gs->client_addr = local_ip;

		memset(&l70, 0, sizeof(l70));
		l70.uin = gg_fix32(gs->uin);
		l70.hash_type = gs->hash_type;
		memcpy(l70.hash, hash_buf, sizeof(l70.hash));
		l70.status = gg_fix32(gs->initial_status ? gs->initial_status : GG_STATUS_AVAIL);
		l70.version = gg_fix32(gs->protocol_version | gs->protocol_flags);
		if (gs->external_addr && gs->external_port > 1023) {
			l70.local_ip = gs->external_addr;
			l70.local_port = gg_fix16(gs->external_port);
		} else {
			l70.local_ip = local_ip;
			l70.local_port = gg_fix16(gg_dcc_port);
		}

		l70.image_size = gs->image_size;
		l70.dunno2 = 0xbe;

		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() sending GG_LOGIN70 packet\n");
		ret = gg_send_packet(gs, GG_LOGIN70, &l70, sizeof(l70), gs->initial_descr, (gs->initial_descr) ? strlen(gs->initial_descr) : 0, NULL);
	}

	if (ret == -1) {
		int errno_copy;

		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() sending packet failed. (errno=%d, %s)\n", errno, strerror(errno));
		errno_copy = errno;
		close(gs->fd);
		errno = errno_copy;
		gs->fd = -1;
		ge->type = GG_EVENT_CONN_FAILED;
		ge->event.failure = GG_FAILURE_WRITING;
		gs->state = GG_STATE_IDLE;
		return -1;
	}

	gs->state = GG_STATE_READING_REPLY;
	gs->check = GG_CHECK_READ;

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_LOGIN_OK.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_login_ok(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() login succeded\n");
	ge->type = GG_EVENT_CONN_SUCCESS;
	gs->state = GG_STATE_CONNECTED;
	gs->check = GG_CHECK_READ;
	gs->timeout = -1;
	gs->status = (gs->initial_status) ? gs->initial_status : GG_STATUS_AVAIL;
#if 0
	free(gs->status_descr);
	gs->status_descr = gs->initial_descr;
#else
	free(gs->initial_descr);
#endif
	gs->initial_descr = NULL;

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_LOGIN_FAILED.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_login_failed(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	if (type != GG_DISCONNECTING)
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() login failed\n");
	else
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() too many incorrect password attempts\n");
	ge->type = GG_EVENT_CONN_FAILED;
	ge->event.failure = (type != GG_DISCONNECTING) ? GG_FAILURE_PASSWORD : GG_FAILURE_INTRUDER;
	gs->state = GG_STATE_IDLE;
	close(gs->fd);
	gs->fd = -1;
	errno = EACCES;

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_SEND_MSG_ACK.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_send_msg_ack(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_send_msg_ack *s = (struct gg_send_msg_ack*) ptr;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a message ack\n");

	ge->type = GG_EVENT_ACK;
	ge->event.ack.status = gg_fix32(s->status);
	ge->event.ack.recipient = gg_fix32(s->recipient);
	ge->event.ack.seq = gg_fix32(s->seq);

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_PONG.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_pong(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a pong\n");

	ge->type = GG_EVENT_PONG;

	gs->last_pong = time(NULL);

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_DISCONNECTING.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_disconnecting(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received disconnection warning\n");

	ge->type = GG_EVENT_DISCONNECT;

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_DISCONNECT_ACK.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_disconnect_ack(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received logoff acknowledge\n");

	ge->type = GG_EVENT_DISCONNECT_ACK;

	return 0;
}

/**
 * \internal Obsługuje pakiety GG_XML_EVENT i GG_XML_ACTION.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_xml_event(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received XML event\n");

	ge->type = GG_EVENT_XML_EVENT;
	ge->event.xml_event.data = (char*)malloc(len + 1);

	if (ge->event.xml_event.data == NULL) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
		return -1;
	}

	memcpy(ge->event.xml_event.data, ptr, len);
	ge->event.xml_event.data[len] = 0;

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_PUBDIR50_REPLY.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_pubdir50_reply(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received pubdir/search reply\n");

	return gg_pubdir50_handle_reply_sess(gs, ge, ptr, len);
}

/**
 * \internal Obsługuje pakiet GG_USERLIST_REPLY.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_userlist_reply(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	char reply_type;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received userlist reply\n");

	reply_type = ptr[0];

	/* jeśli odpowiedź na eksport, wywołaj zdarzenie tylko
	 * gdy otrzymano wszystkie odpowiedzi */
	if (reply_type == GG_USERLIST_PUT_REPLY || reply_type == GG_USERLIST_PUT_MORE_REPLY) {
		if (--gs->userlist_blocks)
			return 0;

		reply_type = GG_USERLIST_PUT_REPLY;
	}

	if (len > 1) {
		unsigned int reply_len = (gs->userlist_reply != NULL) ? strlen(gs->userlist_reply) : 0;
		char *tmp;

		gg_debug_session(gs, GG_DEBUG_MISC, "userlist_reply=%p, len=%d\n", gs->userlist_reply, len);

		tmp = (char*)realloc(gs->userlist_reply, reply_len + len);

		if (tmp == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			return -1;
		}

		gs->userlist_reply = tmp;
		memcpy(gs->userlist_reply + reply_len, ptr + 1, len - 1);
		gs->userlist_reply[reply_len + len - 1] = 0;
	}

	if (reply_type == GG_USERLIST_GET_MORE_REPLY)
		return 0;

	ge->type = GG_EVENT_USERLIST;
	ge->event.userlist.type = reply_type;
	ge->event.userlist.reply = gs->userlist_reply;

	gs->userlist_reply = NULL;

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_DCC7_ID_REPLY.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_dcc7_id_reply(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 id packet\n");

	return gg_dcc7_handle_id(gs, ge, ptr, len);
}

/**
 * \internal Obsługuje pakiet GG_DCC7_ACCEPT.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_dcc7_accept(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 accept\n");

	return gg_dcc7_handle_accept(gs, ge, ptr, len);
}

/**
 * \internal Obsługuje pakiet GG_DCC7_NEW.
 * 
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_dcc7_new(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 request\n");

	return gg_dcc7_handle_new(gs, ge, ptr, len);
}

/**
 * \internal Obsługuje pakiet GG_DCC7_REJECT.
 * 
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_dcc7_reject(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 reject\n");

	return gg_dcc7_handle_reject(gs, ge, ptr, len);
}

/**
* \internal Obsługuje pakiet GG_DCC7_INFO.
*
* Patrz gg_packet_handler_t
*/
static int gg_session_handle_dcc7_info(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 info\n");
	
	return gg_dcc7_handle_info(gs, ge, ptr, len);
}

/**
 * \internal Analizuje przychodzący pakiet z obrazkiem.
 *
 * \param e Struktura zdarzenia
 * \param p Bufor z danymi
 * \param len Długość bufora
 * \param sess Struktura sesji
 * \param sender Numer nadawcy
 */
static void gg_image_queue_parse(struct gg_event *e, const char *p, unsigned int len, struct gg_session *sess, uin_t sender)
{
	struct gg_msg_image_reply *i = (gg_msg_image_reply*) p;
	struct gg_image_queue *q, *qq;

	if (!p || !sess || !e) {
		errno = EFAULT;
		return;
	}

	/* znajdź dany obrazek w kolejce danej sesji */

	for (qq = sess->images, q = NULL; qq; qq = qq->next) {
		if (sender == qq->sender && i->size == qq->size && i->crc32 == qq->crc32) {
			q = qq;
			break;
		}
	}

	if (!q) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_image_queue_parse() unknown image from %d, size=%d, crc32=%.8x\n", sender, i->size, i->crc32);
		return;
	}

	if (p[0] == GG_MSG_OPTION_IMAGE_REPLY) {
		q->done = 0;

		len -= sizeof(struct gg_msg_image_reply);
		p += sizeof(struct gg_msg_image_reply);

		if (memchr(p, 0, len) == NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_image_queue_parse() malformed packet from %d, unlimited filename\n", sender);
			return;
		}

		if (!(q->filename = strdup(p))) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_image_queue_parse() out of memory\n");
			return;
		}

		len -= strlen(p) + 1;
		p += strlen(p) + 1;
	} else {
		len -= sizeof(struct gg_msg_image_reply);
		p += sizeof(struct gg_msg_image_reply);
	}

	if (q->done + len > q->size)
		len = q->size - q->done;

	memcpy(q->image + q->done, p, len);
	q->done += len;

	/* jeśli skończono odbierać obrazek, wygeneruj zdarzenie */

	if (q->done >= q->size) {
		e->type = GG_EVENT_IMAGE_REPLY;
		e->event.image_reply.sender = sender;
		e->event.image_reply.size = q->size;
		e->event.image_reply.crc32 = q->crc32;
		e->event.image_reply.filename = q->filename;
		e->event.image_reply.image = q->image;

		gg_image_queue_remove(sess, q, 0);

		free(q);
	}
}

/**
 * \internal Analizuje informacje rozszerzone wiadomości.
 * 
 * \param sess Struktura sesji.
 * \param e Struktura zdarzenia.
 * \param sender Numer nadawcy.
 * \param p Wskaźnik na dane rozszerzone.
 * \param packet_end Wskaźnik na koniec pakietu.
 *
 * \return 0 jeśli się powiodło, -1 jeśli wiadomość obsłużono i wynik ma
 * zostać przekazany aplikacji, -2 jeśli wystąpił błąd ogólny, -3 jeśli
 * wiadomość jest niepoprawna.
 */
static int gg_handle_recv_msg_options(struct gg_session *sess, struct gg_event *e, uin_t sender, const char *p, const char *packet_end)
{
	while (p < packet_end) {
		switch (*p) {
			case GG_MSG_OPTION_CONFERENCE:
			{
				struct gg_msg_recipients *m = (gg_msg_recipients*) p;
				uint32_t i, count;

				p += sizeof(*m);

				if (p > packet_end) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() packet out of bounds (1)\n");
					goto malformed;
				}

				count = gg_fix32(m->count);

				if (p + count * sizeof(uin_t) > packet_end || p + count * sizeof(uin_t) < p || count > 0xffff) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() packet out of bounds (1.5)\n");
					goto malformed;
				}

				if (e->event.msg.recipients != NULL) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() e->event.msg.recipients already exist\n");
					goto malformed;
				}

				e->event.msg.recipients = (uin_t*)malloc(count * sizeof(uin_t));

				if (e->event.msg.recipients == NULL) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() not enough memory for recipients data\n");
					goto fail;
				}

				memcpy(e->event.msg.recipients, p, count * sizeof(uin_t));
				p += count * sizeof(uin_t);

				for (i = 0; i < count; i++)
					e->event.msg.recipients[i] = gg_fix32(e->event.msg.recipients[i]);

				e->event.msg.recipients_count = count;

				break;
			}

			case GG_MSG_OPTION_ATTRIBUTES:
			{
				uint16_t len;
				char *buf;

				if (p + 3 > packet_end) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() packet out of bounds (2)\n");
					goto malformed;
				}

				memcpy(&len, p + 1, sizeof(uint16_t));
				len = gg_fix16(len);

				if (e->event.msg.formats != NULL) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() e->event.msg.formats already exist\n");
					goto malformed;
				}

				buf = (char*)malloc(len);

				if (buf == NULL) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() not enough memory for richtext data\n");
					goto fail;
				}

				p += 3;

				if (p + len > packet_end) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() packet out of bounds (3)\n");
					free(buf);
					goto malformed;
				}

				memcpy(buf, p, len);

				e->event.msg.formats = buf;
				e->event.msg.formats_length = len;

				p += len;

				break;
			}

			case GG_MSG_OPTION_IMAGE_REQUEST:
			{
				struct gg_msg_image_request *i = (gg_msg_image_request*) p;

				if (p + sizeof(*i) > packet_end) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg() packet out of bounds (3)\n");
					goto malformed;
				}

				if (e->event.msg.formats != NULL || e->event.msg.recipients != NULL) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() mixed options (1)\n");
					goto malformed;
				}

				e->event.image_request.sender = sender;
				e->event.image_request.size = gg_fix32(i->size);
				e->event.image_request.crc32 = gg_fix32(i->crc32);

				e->type = GG_EVENT_IMAGE_REQUEST;

				goto handled;
			}

			case GG_MSG_OPTION_IMAGE_REPLY:
			case GG_MSG_OPTION_IMAGE_REPLY_MORE:
			{
				struct gg_msg_image_reply *rep = (gg_msg_image_reply*) p;

				if (e->event.msg.formats != NULL || e->event.msg.recipients != NULL) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() mixed options (2)\n");
					goto malformed;
				}

				if (p + sizeof(struct gg_msg_image_reply) == packet_end) {

					/* pusta odpowiedź - klient po drugiej stronie nie ma żądanego obrazka */

					e->type = GG_EVENT_IMAGE_REPLY;
					e->event.image_reply.sender = sender;
					e->event.image_reply.size = 0;
					e->event.image_reply.crc32 = gg_fix32(rep->crc32);
					e->event.image_reply.filename = NULL;
					e->event.image_reply.image = NULL;
					goto handled;

				} else if (p + sizeof(struct gg_msg_image_reply) + 1 > packet_end) {

					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg() packet out of bounds (4)\n");
					goto malformed;
				}

				rep->size = gg_fix32(rep->size);
				rep->crc32 = gg_fix32(rep->crc32);
				gg_image_queue_parse(e, p, (unsigned int)(packet_end - p), sess, sender);

				goto handled;
			}

			default:
			{
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg() unknown payload 0x%.2x\n", *p);
				p = packet_end;
			}
		}
	}

	return 0;

handled:
	return -1;

fail:
	return -2;

malformed:
	return -3;
}

/**
 * \internal Wysyła potwierdzenie odebrania wiadomości.
 *
 * \param gs Struktura sesji
 *
 * \return 0 jeśli się powiodło, -1 jeśli wystąpił błąd
 */
static int gg_session_send_msg_ack(struct gg_session *gs, uint32_t seq)
{
	struct gg_recv_msg_ack pkt;

	gg_debug_session(gs, GG_DEBUG_FUNCTION, "** gg_session_send_msg_ack(%p);\n", gs);

	if ((gs->protocol_features & GG_FEATURE_MSG_ACK) == 0)
		return 0;

	/* Kiedyś zdawało nam się, że mamy wysyłać liczbę odebranych
	 * wiadomości, ale okazało się, że numer sekwencyjny. */
	gs->recv_msg_count++;

	pkt.seq = gg_fix32(seq);

	return gg_send_packet(gs, GG_RECV_MSG_ACK, &pkt, sizeof(pkt), NULL);
}

/**
 * \internal Obsługuje pakiet GG_RECV_MSG.
 * 
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_recv_msg(struct gg_session *sess, uint32_t type, const char *packet, size_t length, struct gg_event *e)
{
	const struct gg_recv_msg *r = (const struct gg_recv_msg*) packet;
	const char *payload = packet + sizeof(struct gg_recv_msg);
	const char *payload_end = packet + length;
	char *tmp;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_handle_recv_msg(%p, %d, %p);\n", packet, length, e);

	if ((r->seq == 0) && (r->msgclass == 0)) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg() oops, silently ignoring the bait\n");
		goto malformed;
	}

	// jednobajtowa wiadomość o treści \x02 to żądanie połączenia DCC
	if (*payload == GG_MSG_CALLBACK && payload == payload_end - 1) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg() received ctcp packet\n");
		length = 1;
	} else {
		const char *options;
		
		options = (const char*)memchr(payload, 0, (size_t) (payload_end - payload));

		if (options == NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg() malformed packet, message out of bounds (0)\n");
			goto malformed;
		}
		
		length = (size_t) (options - payload);

		switch (gg_handle_recv_msg_options(sess, e, gg_fix32(r->sender), options + 1, payload_end)) {
			case -1:	// handled
				gg_session_send_msg_ack(sess, gg_fix32(r->seq));
				return 0;

			case -2:	// failed
				goto fail;

			case -3:	// malformed
				goto malformed;
		}
	}

	e->type = GG_EVENT_MSG;
	e->event.msg.msgclass = gg_fix32(r->msgclass);
	e->event.msg.sender = gg_fix32(r->sender);
	e->event.msg.time = gg_fix32(r->time);
	e->event.msg.seq = gg_fix32(r->seq);

	tmp = gg_encoding_convert(payload, GG_ENCODING_CP1250, sess->encoding, length, -1);
	if (tmp == NULL)
		goto fail;
	e->event.msg.message = tmp;

	gg_session_send_msg_ack(sess, gg_fix32(r->seq));
	return 0;

fail:
	free(e->event.msg.message);
	free(e->event.msg.recipients);
	free(e->event.msg.formats);
	return -1;

malformed:
	e->type = GG_EVENT_NONE;
	free(e->event.msg.message);
	free(e->event.msg.xhtml_message);
	free(e->event.msg.recipients);
	free(e->event.msg.formats);
	gg_session_send_msg_ack(sess, gg_fix32(r->seq));
	return 0;
}

/**
 * \internal Obsługuje pakiet GG_RECV_MSG80.
 * 
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_recv_msg_80(struct gg_session *sess, uint32_t type, const char *packet, size_t length, struct gg_event *e)
{
	const struct gg_recv_msg80 *r = (const struct gg_recv_msg80*) packet;
	uint32_t offset_plain;
	uint32_t offset_attr;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_handle_recv_msg80(%p, %d, %p);\n", packet, length, e);

	if (r->seq == 0 && r->msgclass == 0) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg80() oops, silently ignoring the bait\n");
		goto malformed;
	}

	offset_plain = gg_fix32(r->offset_plain);
	offset_attr = gg_fix32(r->offset_attr);

	if (offset_plain < sizeof(struct gg_recv_msg80) || offset_plain >= length) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg80() malformed packet, message out of bounds (0)\n");
		goto malformed;
	}

	if (offset_attr < sizeof(struct gg_recv_msg80) || offset_attr > length) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg80() malformed packet, attr out of bounds (1)\n");
		offset_attr = 0;	/* nie parsuj attr. */
	}

	/* Normalna sytuacja, więc nie podpada pod powyższy warunek. */
	if (offset_attr == length)
		offset_attr = 0;

	if (memchr(packet + offset_plain, 0, length - offset_plain) == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg80() malformed packet, message out of bounds (2)\n");
		goto malformed;
	}

	if (offset_plain > sizeof(struct gg_recv_msg80) && memchr(packet + sizeof(struct gg_recv_msg80), 0, offset_plain - sizeof(struct gg_recv_msg80)) == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg80() malformed packet, message out of bounds (3)\n");
		goto malformed;
	}

	e->type = (type != GG_RECV_OWN_MSG) ? GG_EVENT_MSG : GG_EVENT_MULTILOGON_MSG;
	e->event.msg.msgclass = gg_fix32(r->msgclass);
	e->event.msg.sender = gg_fix32(r->sender);
	e->event.msg.time = gg_fix32(r->time);
	e->event.msg.seq = gg_fix32(r->seq);

	if (offset_attr != 0) {
		switch (gg_handle_recv_msg_options(sess, e, gg_fix32(r->sender), packet + offset_attr, packet + length)) {
			case -1:	// handled
				gg_session_send_msg_ack(sess, gg_fix32(r->seq));
				return 0;

			case -2:	// failed
				goto fail;

			case -3:	// malformed
				goto malformed;
		}
	}

	if (sess->encoding == GG_ENCODING_CP1250) {
		e->event.msg.message = strdup(packet + offset_plain);
	} else {
		if (offset_plain > sizeof(struct gg_recv_msg80)) {
			int len;

			len = gg_message_html_to_text(NULL, packet + sizeof(struct gg_recv_msg80));
			e->event.msg.message = (char*)malloc(len + 1);

			if (e->event.msg.message == NULL)
				goto fail;

			gg_message_html_to_text((char*) e->event.msg.message, packet + sizeof(struct gg_recv_msg80));
		} else {
			e->event.msg.message = gg_encoding_convert(packet + offset_plain, GG_ENCODING_CP1250, sess->encoding, -1, -1);
		}
	}

	if (offset_plain > sizeof(struct gg_recv_msg80))
		e->event.msg.xhtml_message = gg_encoding_convert(packet + sizeof(struct gg_recv_msg80), GG_ENCODING_UTF8, sess->encoding, -1, -1);
	else
		e->event.msg.xhtml_message = NULL;

	gg_session_send_msg_ack(sess, gg_fix32(r->seq));
	return 0;

fail:
	free(e->event.msg.message);
	free(e->event.msg.xhtml_message);
	free(e->event.msg.recipients);
	free(e->event.msg.formats);
	return -1;

malformed:
	e->type = GG_EVENT_NONE;
	free(e->event.msg.message);
	free(e->event.msg.xhtml_message);
	free(e->event.msg.recipients);
	free(e->event.msg.formats);
	gg_session_send_msg_ack(sess, gg_fix32(r->seq));
	return 0;
}

/**
 * \internal Obsługuje pakiet GG_STATUS.
 * 
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_status(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_status *s = (gg_status*) ptr;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a status change\n");

	ge->type = GG_EVENT_STATUS;
	ge->event.status.uin = gg_fix32(s->uin);
	ge->event.status.status = gg_fix32(s->status);
	ge->event.status.descr = NULL;

	if (len > sizeof(*s)) {
		ge->event.status.descr = gg_encoding_convert(ptr + sizeof(*s), GG_ENCODING_CP1250, gs->encoding, len - sizeof(*s), -1);

		if (ge->event.status.descr == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			return -1;
		}
	}

	return 0;
}

/**
 * \internal Obsługuje pakiety GG_STATUS60, GG_STATUS77 i GG_STATUS80BETA.
 * 
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_status_60_77_80beta(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_status60 *s60 = (gg_status60*) ptr;
	struct gg_status77 *s77 = (gg_status77*) ptr;
	size_t struct_len;
	uint32_t uin;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a status change\n");

	ge->type = GG_EVENT_STATUS60;
	ge->event.status60.descr = NULL;
	ge->event.status60.time = 0;

	if (type == GG_STATUS60) {
		uin = gg_fix32(s60->uin);
		ge->event.status60.status = s60->status;
		ge->event.status60.remote_ip = s60->remote_ip;
		ge->event.status60.remote_port = gg_fix16(s60->remote_port);
		ge->event.status60.version = s60->version;
		ge->event.status60.image_size = s60->image_size;
		struct_len = sizeof(*s60);
	} else {
		uin = gg_fix32(s77->uin);
		ge->event.status60.status = s77->status;
		ge->event.status60.remote_ip = s77->remote_ip;
		ge->event.status60.remote_port = gg_fix16(s77->remote_port);
		ge->event.status60.version = s77->version;
		ge->event.status60.image_size = s77->image_size;
		struct_len = sizeof(*s77);
	}

	ge->event.status60.uin = uin & 0x00ffffff;

	if (uin & 0x40000000)
		ge->event.status60.version |= GG_HAS_AUDIO_MASK;
	if (uin & 0x20000000)
		ge->event.status60.version |= GG_HAS_AUDIO7_MASK;
	if (uin & 0x08000000)
		ge->event.status60.version |= GG_ERA_OMNIX_MASK;

	if (len > struct_len) {
		size_t descr_len;

		descr_len = len - struct_len;

		ge->event.status60.descr = gg_encoding_convert(ptr + struct_len, (type == GG_STATUS80BETA) ? GG_ENCODING_UTF8 : GG_ENCODING_CP1250, gs->encoding, descr_len, -1);

		if (ge->event.status60.descr == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			return -1;
		}

		if (descr_len > 4 && ptr[len - 5] == 0) {
			uint32_t t;
			memcpy(&t, ptr + len - 4, sizeof(uint32_t));
			ge->event.status60.time = gg_fix32(t);
		}
	}

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_NOTIFY_REPLY.
 * 
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_notify_reply(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_notify_reply *n = (gg_notify_reply*) ptr;
	char *descr;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a notify reply\n");

	if (gg_fix32(n->status) == GG_STATUS_BUSY_DESCR || gg_fix32(n->status) == GG_STATUS_NOT_AVAIL_DESCR || gg_fix32(n->status) == GG_STATUS_AVAIL_DESCR) {
		size_t descr_len;

		ge->type = GG_EVENT_NOTIFY_DESCR;

		if (!(ge->event.notify_descr.notify = (gg_notify_reply*) malloc(sizeof(*n) * 2))) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			return -1;
		}
		ge->event.notify_descr.notify[1].uin = 0;
		memcpy(ge->event.notify_descr.notify, ptr, sizeof(*n));
		ge->event.notify_descr.notify[0].uin = gg_fix32(ge->event.notify_descr.notify[0].uin);
		ge->event.notify_descr.notify[0].status = gg_fix32(ge->event.notify_descr.notify[0].status);
		ge->event.notify_descr.notify[0].remote_port = gg_fix16(ge->event.notify_descr.notify[0].remote_port);
		ge->event.notify_descr.notify[0].version = gg_fix32(ge->event.notify_descr.notify[0].version);

		descr_len = len - sizeof(*n);

		descr = gg_encoding_convert(ptr + sizeof(*n), GG_ENCODING_CP1250, gs->encoding, descr_len, -1);

		if (descr == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			return -1;
		}

		ge->event.notify_descr.descr = descr;

	} else {
		unsigned int i, count;

		ge->type = GG_EVENT_NOTIFY;

		if (!(ge->event.notify = (gg_notify_reply*) malloc(len + 2 * sizeof(*n)))) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			return -1;
		}

		memcpy(ge->event.notify, ptr, len);
		count = len / sizeof(*n);
		ge->event.notify[count].uin = 0;

		for (i = 0; i < count; i++) {
			ge->event.notify[i].uin = gg_fix32(ge->event.notify[i].uin);
			ge->event.notify[i].status = gg_fix32(ge->event.notify[i].status);
			ge->event.notify[i].remote_port = gg_fix16(ge->event.notify[i].remote_port);
			ge->event.notify[i].version = gg_fix32(ge->event.notify[i].version);
		}
	}

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_STATUS80.
 * 
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_status_80(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_notify_reply80 *n = (gg_notify_reply80*) ptr;
	size_t descr_len;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a status change\n");

	ge->type = GG_EVENT_STATUS60;
	ge->event.status60.uin = gg_fix32(n->uin);
	ge->event.status60.status = gg_fix32(n->status);
	ge->event.status60.remote_ip = n->remote_ip;
	ge->event.status60.remote_port = gg_fix16(n->remote_port);
	ge->event.status60.version = 0;
	ge->event.status60.image_size = n->image_size;
	ge->event.status60.descr = NULL;
	ge->event.status60.time = 0;

	descr_len = gg_fix32(n->descr_len);

	if (descr_len != 0 && sizeof(struct gg_notify_reply80) + descr_len <= len) {
		ge->event.status60.descr = gg_encoding_convert((char*) n + sizeof(struct gg_notify_reply80), GG_ENCODING_UTF8, gs->encoding, descr_len, -1);

		if (ge->event.status60.descr == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			return -1;
		}

		/* XXX czas */
	}

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_NOTIFY_REPLY80.
 * 
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_notify_reply_80(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_notify_reply80 *n = (gg_notify_reply80*) ptr;
	unsigned int length = len, i = 0;

	// TODO: najpierw przeanalizować strukturę i określić
	// liczbę rekordów, żeby obyć się bez realloc()

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a notify reply\n");

	ge->type = GG_EVENT_NOTIFY60;
	ge->event.notify60 = (gg_event_notify60*)malloc(sizeof(*ge->event.notify60));

	if (!ge->event.notify60) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
		return -1;
	}

	ge->event.notify60[0].uin = 0;

	while (length >= sizeof(struct gg_notify_reply80)) {
		uin_t uin = gg_fix32(n->uin);
		int descr_len;
		char *tmp;

		ge->event.notify60[i].uin = uin;
		ge->event.notify60[i].status = gg_fix32(n->status);
		ge->event.notify60[i].remote_ip = n->remote_ip;
		ge->event.notify60[i].remote_port = gg_fix16(n->remote_port);
		ge->event.notify60[i].version = 0;
		ge->event.notify60[i].image_size = n->image_size;
		ge->event.notify60[i].descr = NULL;
		ge->event.notify60[i].time = 0;

		descr_len = gg_fix32(n->descr_len);

		if (descr_len != 0) {
			if (sizeof(struct gg_notify_reply80) + descr_len <= length) {
				ge->event.notify60[i].descr = gg_encoding_convert((char*) n + sizeof(struct gg_notify_reply80), GG_ENCODING_UTF8, gs->encoding, descr_len, -1);

				if (ge->event.notify60[i].descr == NULL) {
					gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
					return -1;
				}

				/* XXX czas */

				length -= sizeof(struct gg_notify_reply80) + descr_len;
				n = (gg_notify_reply80*) ((char*) n + sizeof(struct gg_notify_reply80) + descr_len);
			} else {
				length = 0;
			}

		} else {
			length -= sizeof(struct gg_notify_reply80);
			n = (gg_notify_reply80*) ((char*) n + sizeof(struct gg_notify_reply80));
		}

		if (!(tmp = (char*)realloc(ge->event.notify60, (i + 2) * sizeof(*ge->event.notify60)))) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			free(ge->event.notify60);
			return -1;
		}

		ge->event.notify60 = (gg_event_notify60*) tmp;
		ge->event.notify60[++i].uin = 0;
	}

	return 0;
}

/**
 * \internal Obsługuje pakiety GG_NOTIFY_REPLY77 i GG_NOTIFY_REPLY80BETA.
 * 
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_notify_reply_77_80beta(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_notify_reply77 *n = (gg_notify_reply77*) ptr;
	unsigned int length = len, i = 0;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a notify reply\n");

	ge->type = GG_EVENT_NOTIFY60;
	ge->event.notify60 = (gg_event_notify60*)malloc(sizeof(*ge->event.notify60));

	if (ge->event.notify60 == NULL) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
		return -1;
	}

	ge->event.notify60[0].uin = 0;

	while (length >= sizeof(struct gg_notify_reply77)) {
		uin_t uin = gg_fix32(n->uin);
		char *tmp;

		ge->event.notify60[i].uin = uin & 0x00ffffff;
		ge->event.notify60[i].status = n->status;
		ge->event.notify60[i].remote_ip = n->remote_ip;
		ge->event.notify60[i].remote_port = gg_fix16(n->remote_port);
		ge->event.notify60[i].version = n->version;
		ge->event.notify60[i].image_size = n->image_size;
		ge->event.notify60[i].descr = NULL;
		ge->event.notify60[i].time = 0;

		if (uin & 0x40000000)
			ge->event.notify60[i].version |= GG_HAS_AUDIO_MASK;
		if (uin & 0x20000000)
			ge->event.notify60[i].version |= GG_HAS_AUDIO7_MASK;
		if (uin & 0x08000000)
			ge->event.notify60[i].version |= GG_ERA_OMNIX_MASK;

		if (GG_S_D(n->status)) {
			unsigned char descr_len = *((char*) n + sizeof(struct gg_notify_reply77));

			if (sizeof(struct gg_notify_reply77) + descr_len <= length) {
				ge->event.notify60[i].descr = gg_encoding_convert((char*) n + sizeof(struct gg_notify_reply77) + 1, (type == GG_NOTIFY_REPLY80BETA) ? GG_ENCODING_UTF8 : GG_ENCODING_CP1250, gs->encoding, descr_len, -1);

				if (ge->event.notify60[i].descr == NULL) {
					gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
					return -1;
				}

				/* XXX czas */
					
				length -= sizeof(struct gg_notify_reply77) + descr_len + 1;
				n = (gg_notify_reply77*) ((char*) n + sizeof(struct gg_notify_reply77) + descr_len + 1);
			} else {
				length = 0;
			}

		} else {
			length -= sizeof(struct gg_notify_reply77);
			n = (gg_notify_reply77*) ((char*) n + sizeof(struct gg_notify_reply77));
		}

		if (!(tmp = (char*)realloc(ge->event.notify60, (i + 2) * sizeof(*ge->event.notify60)))) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			free(ge->event.notify60);
			return -1;
		}

		ge->event.notify60 = (gg_event_notify60*) tmp;
		ge->event.notify60[++i].uin = 0;
	}

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_NOTIFY_REPLY60.
 * 
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_notify_reply_60(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_notify_reply60 *n = (gg_notify_reply60*) ptr;
	unsigned int length = len, i = 0;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a notify reply\n");

	ge->type = GG_EVENT_NOTIFY60;
	ge->event.notify60 = (gg_event_notify60*)malloc(sizeof(*ge->event.notify60));

	if (ge->event.notify60 == NULL) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
		return -1;
	}

	ge->event.notify60[0].uin = 0;

	while (length >= sizeof(struct gg_notify_reply60)) {
		uin_t uin = gg_fix32(n->uin);
		char *tmp;

		ge->event.notify60[i].uin = uin & 0x00ffffff;
		ge->event.notify60[i].status = n->status;
		ge->event.notify60[i].remote_ip = n->remote_ip;
		ge->event.notify60[i].remote_port = gg_fix16(n->remote_port);
		ge->event.notify60[i].version = n->version;
		ge->event.notify60[i].image_size = n->image_size;
		ge->event.notify60[i].descr = NULL;
		ge->event.notify60[i].time = 0;

		if (uin & 0x40000000)
			ge->event.notify60[i].version |= GG_HAS_AUDIO_MASK;
		if (uin & 0x08000000)
			ge->event.notify60[i].version |= GG_ERA_OMNIX_MASK;

		if (GG_S_D(n->status)) {
			unsigned char descr_len = *((char*) n + sizeof(struct gg_notify_reply60));

			if (sizeof(struct gg_notify_reply60) + descr_len <= length) {
				char *descr;

				descr = gg_encoding_convert((char*) n + sizeof(struct gg_notify_reply60) + 1, GG_ENCODING_CP1250, gs->encoding, descr_len, -1);

				if (descr == NULL) {
					gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
					return -1;
				}

				ge->event.notify60[i].descr = descr;

				/* XXX czas */
					
				length -= sizeof(struct gg_notify_reply60) + descr_len + 1;
				n = (gg_notify_reply60*) ((char*) n + sizeof(struct gg_notify_reply60) + descr_len + 1);
			} else {
				length = 0;
			}

		} else {
			length -= sizeof(struct gg_notify_reply60);
			n = (gg_notify_reply60*) ((char*) n + sizeof(struct gg_notify_reply60));
		}

		if (!(tmp = (char*)realloc(ge->event.notify60, (i + 2) * sizeof(*ge->event.notify60)))) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			free(ge->event.notify60);
			return -1;
		}

		ge->event.notify60 = (gg_event_notify60*) tmp;
		ge->event.notify60[++i].uin = 0;
	}

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_USER_DATA.
 * 
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_user_data(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_user_data d;
	char *p = (char*) ptr;
	char *packet_end = (char*) ptr + len;
	struct gg_event_user_data_user *users;
	int i, j;
	int res = 0;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received user data\n");

	ge->event.user_data.user_count = 0;
	ge->event.user_data.users = NULL;

	if (ptr + sizeof(d) > packet_end)
		goto malformed;

	memcpy(&d, p, sizeof(d));
	p += sizeof(d);

	d.type = gg_fix32(d.type);
	d.user_count = gg_fix32(d.user_count);

	if (d.user_count > 0xffff) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_user_data() malformed packet (1)\n");
		goto malformed;
	}

	if (d.user_count > 0) {
		users = (gg_event_user_data_user*)calloc(d.user_count, sizeof(struct gg_event_user_data_user));

		if (users == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_user_data() out of memory (%d*%d)\n", d.user_count, sizeof(struct gg_event_user_data_user));
			goto fail;
		}
	} else {
		users = NULL;
	}

	ge->type = GG_EVENT_USER_DATA;
	ge->event.user_data.type = d.type;
	ge->event.user_data.user_count = d.user_count;
	ge->event.user_data.users = users;
	
	gg_debug_session(gs, GG_DEBUG_DUMP, "type=%d, count=%d\n", d.type, d.user_count);

	for (i = 0; i < d.user_count; i++) {
		struct gg_user_data_user u;
		struct gg_event_user_data_attr *attrs;

		if (p + sizeof(u) > packet_end) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_user_data() malformed packet (2)\n");
			goto malformed;
		}

		memcpy(&u, p, sizeof(u));
		p += sizeof(u);

		u.uin = gg_fix32(u.uin);
		u.attr_count = gg_fix32(u.attr_count);

		if (u.attr_count > 0xffff) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_user_data() malformed packet (2)\n");
			goto malformed;
		}

		if (u.attr_count > 0) {
			attrs = (gg_event_user_data_attr*)calloc(u.attr_count, sizeof(struct gg_event_user_data_attr));

			if (attrs == NULL) {
				gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_user_data() out of memory (%d*%d)\n", u.attr_count, sizeof(struct gg_event_user_data_attr));
				goto fail;
			}
		} else {
			attrs = NULL;
		}

		users[i].uin = u.uin;
		users[i].attr_count = u.attr_count;
		users[i].attrs = attrs;

		gg_debug_session(gs, GG_DEBUG_DUMP, "    uin=%d, count=%d\n", u.uin, u.attr_count);

		for (j = 0; j < u.attr_count; j++) {
			uint32_t key_size;
			uint32_t attr_type;
			uint32_t value_size;
			char *key;
			char *value;

			if (p + sizeof(key_size) > packet_end) {
				gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_user_data() malformed packet (3)\n");
				goto malformed;
			}

			memcpy(&key_size, p, sizeof(key_size));
			p += sizeof(key_size);

			key_size = gg_fix32(key_size);

			if (key_size > 0xffff || p + key_size > packet_end) {
				gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_user_data() malformed packet (3)\n");
				goto malformed;
			}

			key = (char*)malloc(key_size + 1);

			if (key == NULL) {
				gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_user_data() out of memory (%d)\n", key_size + 1);
				goto fail;
			}

			memcpy(key, p, key_size);
			p += key_size;

			key[key_size] = 0;

			attrs[j].key = key;

			if (p + sizeof(attr_type) + sizeof(value_size) > packet_end) {
				gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_user_data() malformed packet (4)\n");
				goto malformed;
			}

			memcpy(&attr_type, p, sizeof(attr_type));
			p += sizeof(attr_type);
			memcpy(&value_size, p, sizeof(value_size));
			p += sizeof(value_size);

			attrs[j].type = gg_fix32(attr_type);
			value_size = gg_fix32(value_size);

			if (value_size > 0xffff || p + value_size > packet_end) {
				gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_user_data() malformed packet (5)\n");
				goto malformed;
			}

			value = (char*)malloc(value_size + 1);

			if (value == NULL) {
				gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_user_data() out of memory (%d)\n", value_size + 1);
				goto fail;
			}

			memcpy(value, p, value_size);
			p += value_size;

			value[value_size] = 0;

			attrs[j].value = value;

			gg_debug_session(gs, GG_DEBUG_DUMP, "        key=\"%s\", type=%d, value=\"%s\"\n", key, attr_type, value);
		}
	}

	return 0;

fail:
	res = -1;

malformed:
	ge->type = GG_EVENT_NONE;

	for (i = 0; i < ge->event.user_data.user_count; i++) {
		for (j = 0; j < ge->event.user_data.users[i].attr_count; j++) {
			free(ge->event.user_data.users[i].attrs[j].key);
			free(ge->event.user_data.users[i].attrs[j].value);
		}

		free(ge->event.user_data.users[i].attrs);
	}

	free(ge->event.user_data.users);

	return res;
}

/**
 * \internal Obsługuje pakiet GG_TYPING_NOTIFICATION.
 * 
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_typing_notification(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_typing_notification *n = (struct gg_typing_notification*) ptr;
	uin_t uin;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received typing notification\n");

	memcpy(&uin, &n->uin, sizeof(uin_t));

	ge->type = GG_EVENT_TYPING_NOTIFICATION;
	ge->event.typing_notification.uin = gg_fix32(uin);
	ge->event.typing_notification.length = gg_fix16(n->length);

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_MULTILOGON_INFO.
 * 
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_multilogon_info(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	char *packet_end = (char*) ptr + len;
	struct gg_multilogon_info *info = (struct gg_multilogon_info*) ptr;
	char *p = (char*) ptr + sizeof(*info);
	struct gg_multilogon_session *sessions = NULL;
	size_t count;
	size_t i;
	int res = 0;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received multilogon info\n");

	count = gg_fix32(info->count);

	if (count > 0xffff) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_handle_multilogon_info() malformed packet (1)\n");
		goto malformed;
	}

	sessions = (gg_multilogon_session*)calloc(count, sizeof(struct gg_multilogon_session));

	if (sessions == NULL) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_handle_multilogon_info() out of memory (%d*%d)\n", count, sizeof(struct gg_multilogon_session));
		return -1;
	}
	
	ge->type = GG_EVENT_MULTILOGON_INFO;
	ge->event.multilogon_info.count = count;
	ge->event.multilogon_info.sessions = sessions;

	for (i = 0; i < count; i++) {
		struct gg_multilogon_info_item item;
		size_t name_size;

		if (p + sizeof(item) > packet_end) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_handle_multilogon_info() malformed packet (2)\n");
			goto malformed;
		}

		memcpy(&item, p, sizeof(item));

		sessions[i].id = item.conn_id;
		sessions[i].remote_addr = item.addr;
		sessions[i].status_flags = gg_fix32(item.flags);
		sessions[i].protocol_features = gg_fix32(item.features);
		sessions[i].logon_time = gg_fix32(item.logon_time);

		p += sizeof(item);

		name_size = gg_fix32(item.name_size);

		if (name_size > 0xffff || p + name_size > packet_end) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_handle_multilogon_info() malformed packet (3)\n");
			goto malformed;
		}

		sessions[i].name = (char*)malloc(name_size + 1);

		if (sessions[i].name == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_handle_multilogon_info() out of memory (%d)\n", name_size);
			goto fail;
		}

		memcpy(sessions[i].name, p, name_size);
		sessions[i].name[name_size] = 0;

		p += name_size;
	}

	return 0;

fail:
	res = -1;

malformed:
	ge->type = GG_EVENT_NONE;

	for (i = 0; i < ge->event.multilogon_info.count; i++)
		free(ge->event.multilogon_info.sessions[i].name);

	free(ge->event.multilogon_info.sessions);

	return res;
}

/**
 * \internal Obsługuje pakiet GG_USERLIST100_VERSION.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_userlist_100_version(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_userlist100_version *version = (struct gg_userlist100_version*) ptr;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received userlist 100 version\n");

	ge->type = GG_EVENT_USERLIST100_VERSION;
	ge->event.userlist100_version.version = gg_fix32(version->version);

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_USERLIST100_REPLY.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_userlist_100_reply(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_userlist100_reply *reply = (struct gg_userlist100_reply*) ptr;
	char *data = NULL;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received userlist 100 reply\n");

	if (len > sizeof(*reply)) {
		data = gg_inflate((const unsigned char*) ptr + sizeof(*reply), len - sizeof(*reply));
		
		if (data == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_handle_userlist_100_reply() gg_inflate() failed\n");
			return -1;
		}
	}

	ge->type = GG_EVENT_USERLIST100_REPLY;
	ge->event.userlist100_reply.type = reply->type;
	ge->event.userlist100_reply.version = gg_fix32(reply->version);
	ge->event.userlist100_reply.format_type = reply->format_type;
	ge->event.userlist100_reply.reply = data;

	return 0;
}

/**
 * \internal Tablica obsługiwanych pakietów
 */
static const gg_packet_handler_t handlers[] =
{
	{ GG_WELCOME, GG_STATE_READING_KEY, 0, gg_session_handle_welcome },
	{ GG_LOGIN_OK, GG_STATE_READING_REPLY, 0, gg_session_handle_login_ok },
	{ GG_LOGIN80_OK, GG_STATE_READING_REPLY, 0, gg_session_handle_login_ok },
	{ GG_NEED_EMAIL, GG_STATE_READING_REPLY, 0, gg_session_handle_login_ok },
	{ GG_LOGIN_FAILED, GG_STATE_READING_REPLY, 0, gg_session_handle_login_failed },
	{ GG_LOGIN80_FAILED, GG_STATE_READING_REPLY, 0, gg_session_handle_login_failed },
	{ GG_SEND_MSG_ACK, GG_STATE_CONNECTED, sizeof(struct gg_send_msg_ack), gg_session_handle_send_msg_ack },
	{ GG_PONG, GG_STATE_CONNECTED, 0, gg_session_handle_pong },
	{ GG_DISCONNECTING, GG_STATE_CONNECTED, 0, gg_session_handle_disconnecting },
	{ GG_DISCONNECT_ACK, GG_STATE_DISCONNECTING, 0, gg_session_handle_disconnect_ack },
	{ GG_XML_EVENT, GG_STATE_CONNECTED, 0, gg_session_handle_xml_event },
	{ GG_PUBDIR50_REPLY, GG_STATE_CONNECTED, 0, gg_session_handle_pubdir50_reply },
	{ GG_USERLIST_REPLY, GG_STATE_CONNECTED, 0, gg_session_handle_userlist_reply },
	{ GG_DCC7_ID_REPLY, GG_STATE_CONNECTED, sizeof(struct gg_dcc7_id_reply), gg_session_handle_dcc7_id_reply },
	{ GG_DCC7_ACCEPT, GG_STATE_CONNECTED, sizeof(struct gg_dcc7_accept), gg_session_handle_dcc7_accept },
	{ GG_DCC7_NEW, GG_STATE_CONNECTED, sizeof(struct gg_dcc7_new), gg_session_handle_dcc7_new },
	{ GG_DCC7_REJECT, GG_STATE_CONNECTED, sizeof(struct gg_dcc7_reject), gg_session_handle_dcc7_reject },
	{ GG_DCC7_INFO, GG_STATE_CONNECTED, sizeof(struct gg_dcc7_info), gg_session_handle_dcc7_info },
	{ GG_RECV_MSG, GG_STATE_CONNECTED, sizeof(struct gg_recv_msg), gg_session_handle_recv_msg },
	{ GG_RECV_MSG80, GG_STATE_CONNECTED, sizeof(struct gg_recv_msg80), gg_session_handle_recv_msg_80 },
	{ GG_STATUS, GG_STATE_CONNECTED, sizeof(struct gg_status), gg_session_handle_status },
	{ GG_STATUS60, GG_STATE_CONNECTED, sizeof(struct gg_status60), gg_session_handle_status_60_77_80beta },
	{ GG_STATUS77, GG_STATE_CONNECTED, sizeof(struct gg_status77), gg_session_handle_status_60_77_80beta },
	{ GG_STATUS80BETA, GG_STATE_CONNECTED, sizeof(struct gg_status77), gg_session_handle_status_60_77_80beta },
	{ GG_STATUS80, GG_STATE_CONNECTED, sizeof(struct gg_notify_reply80), gg_session_handle_status_80 },
	{ GG_NOTIFY_REPLY, GG_STATE_CONNECTED, sizeof(struct gg_notify_reply), gg_session_handle_notify_reply },
	{ GG_NOTIFY_REPLY60, GG_STATE_CONNECTED, sizeof(struct gg_notify_reply60), gg_session_handle_notify_reply_60 },
	{ GG_NOTIFY_REPLY77, GG_STATE_CONNECTED, sizeof(struct gg_notify_reply77), gg_session_handle_notify_reply_77_80beta },
	{ GG_NOTIFY_REPLY80BETA, GG_STATE_CONNECTED, sizeof(struct gg_notify_reply77), gg_session_handle_notify_reply_77_80beta },
	{ GG_NOTIFY_REPLY80, GG_STATE_CONNECTED, sizeof(struct gg_notify_reply80), gg_session_handle_notify_reply_80 },
	{ GG_USER_DATA, GG_STATE_CONNECTED, sizeof(struct gg_user_data), gg_session_handle_user_data },
	{ GG_TYPING_NOTIFICATION, GG_STATE_CONNECTED, sizeof(struct gg_typing_notification), gg_session_handle_typing_notification },
	{ GG_MULTILOGON_INFO, GG_STATE_CONNECTED, sizeof(struct gg_multilogon_info), gg_session_handle_multilogon_info },
	{ GG_XML_ACTION, GG_STATE_CONNECTED, 0, gg_session_handle_xml_event },
	{ GG_RECV_OWN_MSG, GG_STATE_CONNECTED, sizeof(struct gg_recv_msg80), gg_session_handle_recv_msg_80 },
	{ GG_USERLIST100_VERSION, GG_STATE_CONNECTED, sizeof(struct gg_userlist100_version), gg_session_handle_userlist_100_version },
	{ GG_USERLIST100_REPLY, GG_STATE_CONNECTED, sizeof(struct gg_userlist100_reply), gg_session_handle_userlist_100_reply },
};

/**
 * \internal Analizuje przychodzący pakiet danych.
 *
 * \param gs Struktura sesji
 * \param type Typ pakietu
 * \param ptr Wskaźnik do bufora pakietu
 * \param len Długość bufora pakietu
 * \param[out] ge Struktura zdarzenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_session_handle_packet(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	int i;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_packet(%d, %p, %d)\n", type, ptr, len);

	gs->last_event = time(NULL);

#if 0
	if ((gs->flags & (1 << GG_SESSION_FLAG_RAW_PACKET)) != 0) {
		char *tmp;

		tmp = malloc(len);

		if (tmp == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_packet() out of memory (%d bytes)\n", len);
			return -1;
		}

		memcpy(tmp, ptr, len);

		ge->type = GG_EVENT_RAW_PACKET;
		ge->event.raw_packet.type = type;
		ge->event.raw_packet.length = len;
		ge->event.raw_packet.data = tmp;

		return 0;
	}
#endif

	for (i = 0; i < sizeof(handlers) / sizeof(handlers[0]); i++) {
		if (handlers[i].type != 0 && handlers[i].type != type)
			continue;

		if (handlers[i].state != 0 && handlers[i].state != gs->state) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_packet() packet 0x%02x unexpected in state %d\n", type, gs->state);
			continue;
		}

		if (len < handlers[i].min_length) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_packet() packet 0x%02x too short (%d bytes)\n", type, len);
			continue;
		}

		return (*handlers[i].handler)(gs, type, ptr, len, ge);
	}

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_packet() unhandled packet 0x%02x, len %d, state %d\n", type, len, gs->state);

	return 0;
}

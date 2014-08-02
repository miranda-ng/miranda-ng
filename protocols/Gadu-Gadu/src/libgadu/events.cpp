/* coding: UTF-8 */
/* $Id: events.c 13583 2011-04-12 12:51:18Z dezred $ */

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

		case GG_EVENT_XML_ACTION:
			free(e->event.xml_action.data);
			break;

		case GG_EVENT_USER_DATA:
		{
			unsigned i, j;

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

/**
 * \internal Analizuje przychodzący pakiet z obrazkiem.
 *
 * \param e Struktura zdarzenia
 * \param p Bufor z danymi
 * \param len Długość bufora
 * \param sess Struktura sesji
 * \param sender Numer nadawcy
 * \param size Rozmiar pliku (z nagłówka)
 * \param crc32 Suma kontrolna (z nagłówka)
 */
static void gg_image_queue_parse(struct gg_event *e, char *p, unsigned int len, struct gg_session *sess, uin_t sender, uint32_t size, uint32_t crc32)
{
	struct gg_image_queue *q, *qq;

	if (!p || !sess || !e) {
		errno = EFAULT;
		return;
	}

	/* znajdź dany obrazek w kolejce danej sesji */

	for (qq = sess->images, q = NULL; qq; qq = qq->next) {
		if (sender == qq->sender && size == qq->size && crc32 == qq->crc32) {
			q = qq;
			break;
		}
	}

	if (!q) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_image_queue_parse() unknown image from %d, size=%d, crc32=%.8x\n", sender, size, crc32);
		return;
	}

	if (p[0] == 0x05) {
		q->done = 0;

		len -= sizeof(struct gg_msg_image_reply);
		p += sizeof(struct gg_msg_image_reply);

		if (memchr(p, 0, len) == NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_image_queue_parse() malformed packet from %d, unlimited filename\n", sender);
			return;
		}

		if (!(q->filename = strdup(p))) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_image_queue_parse() not enough memory for filename\n");
			return;
		}

		len -= (unsigned int)strlen(p) + 1;
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
static int gg_handle_recv_msg_options(struct gg_session *sess, struct gg_event *e, uin_t sender, char *p, char *packet_end)
{
	while (p < packet_end) {
		switch (*p) {
			case 0x01:		/* konferencja */
			{
				struct gg_msg_recipients *m = (gg_msg_recipients*)p;
				uint32_t i, count;

				if (p + sizeof(*m) > packet_end) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() packet out of bounds (1)\n");
					goto malformed;
				}

				memcpy(&count, &m->count, sizeof(count));
				count = gg_fix32(count);
				p += sizeof(*m);

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

			case 0x02:		/* richtext */
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

			case 0x04:		/* image_request */
			{
				struct gg_msg_image_request *i = (gg_msg_image_request*)p;

				if (p + sizeof(*i) > packet_end) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() packet out of bounds (3.5)\n");
					goto malformed;
				}

				if (e->event.msg.formats != NULL || e->event.msg.recipients != NULL) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() mixed options (1)\n");
					goto malformed;
				}

				memcpy(&e->event.image_request.size, &i->size, sizeof(i->size));
				memcpy(&e->event.image_request.crc32, &i->crc32, sizeof(i->crc32));

				e->event.image_request.sender = sender;
				e->event.image_request.size = gg_fix32(e->event.image_request.size);
				e->event.image_request.crc32 = gg_fix32(e->event.image_request.crc32);

				e->type = GG_EVENT_IMAGE_REQUEST;

				goto handled;
			}

			case 0x05:		/* image_reply */
			case 0x06:
			{
				struct gg_msg_image_reply *rep = (gg_msg_image_reply*)p;
				uint32_t size;
				uint32_t crc32;

				if (e->event.msg.formats != NULL || e->event.msg.recipients != NULL) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() mixed options (2)\n");
					goto malformed;
				}

				if (p + sizeof(struct gg_msg_image_reply) + 1 > packet_end) {

					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() packet out of bounds (4)\n");
					goto malformed;
				}

				memcpy(&size, &rep->size, sizeof(size));
				memcpy(&crc32, &rep->crc32, sizeof(crc32));
				size = gg_fix32(size);
				crc32 = gg_fix32(crc32);

				if (p + sizeof(struct gg_msg_image_reply) == packet_end) {
					/* pusta odpowiedź - klient po drugiej stronie nie ma żądanego obrazka */

					e->type = GG_EVENT_IMAGE_REPLY;
					e->event.image_reply.sender = sender;
					e->event.image_reply.size = 0;
					e->event.image_reply.crc32 = crc32;
					e->event.image_reply.filename = NULL;
					e->event.image_reply.image = NULL;
					goto handled;

				}

				gg_image_queue_parse(e, p, (unsigned int)(packet_end - p), sess, sender, size, crc32);

				goto handled;
			}

			default:
			{
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg_options() unknown payload 0x%.2x\n", *p);
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
 * \internal Analizuje przychodzący pakiet z wiadomością.
 *
 * Rozbija pakiet na poszczególne składniki -- tekst, informacje
 * o konferencjach, formatowani itd.
 *
 * \param h Wskaźnik do odebranego pakietu
 * \param e Struktura zdarzenia
 * \param sess Struktura sesji
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_handle_recv_msg(struct gg_header *h, struct gg_event *e, struct gg_session *sess)
{
	struct gg_recv_msg *r = (struct gg_recv_msg*) ((char*) h + sizeof(struct gg_header));
	char *p, *packet_end = (char*) r + h->length;
	int ctcp = 0;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_handle_recv_msg(%p, %p);\n", h, e);

	if (!r->seq && !r->msgclass) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg() oops, silently ignoring the bait\n");
		e->type = GG_EVENT_NONE;
		return 0;
	}

	/* znajdź \0 */
	for (p = (char*) r + sizeof(*r); ; p++) {
		if (p >= packet_end) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg() malformed packet, message out of bounds (0)\n");
			goto malformed;
		}

		if (*p == 0x02 && p == packet_end - 1) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg() received ctcp packet\n");
			ctcp = 1;
			break;
		}

		if (!*p)
			break;
	}

	p++;

	switch (gg_handle_recv_msg_options(sess, e, gg_fix32(r->sender), p, packet_end)) {
		case -1:	// handled
			return 0;

		case -2:	// failed
			goto fail;

		case -3:	// malformed
			goto malformed;
	}

	e->type = GG_EVENT_MSG;
	e->event.msg.msgclass = gg_fix32(r->msgclass);
	e->event.msg.sender = gg_fix32(r->sender);
	e->event.msg.time = gg_fix32(r->time);
	e->event.msg.seq = gg_fix32(r->seq);
	if (ctcp)
		e->event.msg.message = (char*) strdup("\x02");
	else
		e->event.msg.message = (char*) strdup((char*) r + sizeof(*r));


	return 0;

malformed:
	e->type = GG_EVENT_NONE;
	free(e->event.msg.message);
	free(e->event.msg.recipients);
	free(e->event.msg.formats);

	return 0;

fail:
	free(e->event.msg.message);
	free(e->event.msg.recipients);
	free(e->event.msg.formats);
	return -1;
}

/**
 * \internal Zamienia tekst w formacie HTML na czysty tekst.
 *
 * \param dst Bufor wynikowy (może być \c NULL)
 * \param html Tekst źródłowy
 *
 * \note Dokleja \c \\0 na końcu bufora wynikowego.
 *
 * \return Długość tekstu wynikowego bez \c \\0 (nawet jeśli \c dst to \c NULL).
 */
static int gg_convert_from_html(char *dst, const char *html)
{
	const char *src, *entity, *tag;
	int len, in_tag, in_entity;

	len = 0;
	in_tag = 0;
	tag = NULL;
	in_entity = 0;
	entity = NULL;

	for (src = html; *src != 0; src++) {
		if (*src == '<') {
			tag = src;
			in_tag = 1;
			continue;
		}

		if (in_tag && (*src == '>')) {
			if (strncmp(tag, "<br", 3) == 0) {
				if (dst != NULL)
					dst[len] = '\n';
				len++;
			}
			in_tag = 0;
			continue;
		}

		if (in_tag)
			continue;

		if (*src == '&') {
			in_entity = 1;
			entity = src;
			continue;
		}

		if (in_entity && *src == ';') {
			in_entity = 0;
			if (dst != NULL) {
				if (strncmp(entity, "&lt;", 4) == 0)
					dst[len] = '<';
				else if (strncmp(entity, "&gt;", 4) == 0)
					dst[len] = '>';
				else if (strncmp(entity, "&quot;", 6) == 0)
					dst[len] = '"';
				else if (strncmp(entity, "&apos;", 6) == 0)
					dst[len] = '\'';
				else if (strncmp(entity, "&amp;", 5) == 0)
					dst[len] = '&';
				else
					dst[len] = '?';
			}
			len++;
			continue;
		}

		if (in_entity && !(isalnum(*src) || *src == '#'))
			in_entity = 0;

		if (in_entity)
			continue;

		if (dst != NULL)
			dst[len] = *src;

		len++;
	}

	if (dst != NULL)
		dst[len] = 0;
	
	return len;
}

/**
 * \internal Analizuje przychodzący pakiet z wiadomością protokołu Gadu-Gadu 8.0.
 *
 * Rozbija pakiet na poszczególne składniki -- tekst, informacje
 * o konferencjach, formatowani itd.
 *
 * \param h Wskaźnik do odebranego pakietu
 * \param e Struktura zdarzenia
 * \param sess Struktura sesji
 * \param event Typ zdarzenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_handle_recv_msg80(struct gg_header *h, struct gg_event *e, struct gg_session *sess, int event)
{
	char *packet = (char*) h + sizeof(struct gg_header);
	struct gg_recv_msg80 *r = (struct gg_recv_msg80*) packet;
	uint32_t offset_plain;
	uint32_t offset_attr;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_handle_recv_msg80(%p, %p);\n", h, e);

	if (!r->seq && !r->msgclass) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg80() oops, silently ignoring the bait\n");
		goto malformed;
	}

	offset_plain = gg_fix32(r->offset_plain);
	offset_attr  = gg_fix32(r->offset_attr);

	if (offset_plain < sizeof(struct gg_recv_msg80) || offset_plain >= h->length) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg80() malformed packet, message out of bounds (0)\n");
		goto malformed;
	}

	if (offset_attr < sizeof(struct gg_recv_msg80) || offset_attr > h->length) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg80() malformed packet, attr out of bounds (1)\n");
		offset_attr = 0;	/* nie parsuj attr. */
		/* goto ignore; */
	}

	/* Normalna sytuacja, więc nie podpada pod powyższy warunek. */
	if (offset_attr == h->length)
		offset_attr = 0;

	if (memchr(packet + offset_plain, 0, h->length - offset_plain) == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg80() malformed packet, message out of bounds (2)\n");
		goto malformed;
	}

	if (offset_plain > sizeof(struct gg_recv_msg80) && memchr(packet + sizeof(struct gg_recv_msg80), 0, offset_plain - sizeof(struct gg_recv_msg80)) == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg80() malformed packet, message out of bounds (3)\n");
		goto malformed;
	}

	e->type = event;
	e->event.msg.msgclass = gg_fix32(r->msgclass);
	e->event.msg.sender = gg_fix32(r->sender);
	e->event.msg.time = gg_fix32(r->time);
	e->event.msg.seq = gg_fix32(r->seq);

	if (offset_attr != 0) {
		switch (gg_handle_recv_msg_options(sess, e, gg_fix32(r->sender), packet + offset_attr, packet + h->length)) {
			case -1:	// handled
				return 0;

			case -2:	// failed
				goto fail;

			case -3:	// malformed
				goto malformed;
		}
	}

	if (sess->encoding == GG_ENCODING_CP1250) {
		e->event.msg.message = (char*) strdup(packet + offset_plain);
	} else {
		if (offset_plain > sizeof(struct gg_recv_msg80)) {
			int len;

			len = gg_convert_from_html(NULL, packet + sizeof(struct gg_recv_msg80));

			e->event.msg.message = (char*)malloc(len + 1);

			if (e->event.msg.message == NULL)
				goto fail;

			gg_convert_from_html((char*) e->event.msg.message, packet + sizeof(struct gg_recv_msg80));
		} else {
			e->event.msg.message = (char*) gg_cp_to_utf8(packet + offset_plain);
		}
	}

	if (offset_plain > sizeof(struct gg_recv_msg80)) {
		if (sess->encoding == GG_ENCODING_UTF8)
			e->event.msg.xhtml_message = strdup(packet + sizeof(struct gg_recv_msg80));
		else
			e->event.msg.xhtml_message = gg_utf8_to_cp(packet + sizeof(struct gg_recv_msg80));
	} else {
		e->event.msg.xhtml_message = NULL;
	}

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
	return 0;
}

/**
 * \internal Wysyła potwierdzenie odebrania wiadomości.
 *
 * \param sess Struktura sesji
 *
 * \return 0 jeśli się powiodło, -1 jeśli wystąpił błąd
 */
static int gg_handle_recv_msg_ack(struct gg_header *h, struct gg_session *sess)
{
	char *packet = (char*) h + sizeof(struct gg_header);
	struct gg_recv_msg80 *r = (struct gg_recv_msg80*) packet;
	struct gg_recv_msg_ack pkt;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_handle_recv_msg_ack(%p);\n", sess);

	if ((sess->protocol_features & GG_FEATURE_MSG_ACK) == 0)
		return 0;

	pkt.seq = gg_fix32(r->seq);

	return gg_send_packet(sess, GG_RECV_MSG_ACK, &pkt, sizeof(pkt), NULL);
}

/**
 * \internal Analizuje przychodzący pakiet z danymi kontaktów.
 *
 * \param sess Struktura sesji
 * \param e Struktura zdarzenia
 * \param payload Treść pakietu
 * \param len Długość pakietu
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_handle_user_data(struct gg_session *sess, struct gg_event *e, void *packet, size_t len)
{
	struct gg_user_data d;
	char *p = (char*) packet;
	char *packet_end = (char*) packet + len;
	struct gg_event_user_data_user *users;
	unsigned i, j;
	int res = 0;

	gg_debug_session(sess, GG_DEBUG_MISC, "** gg_handle_user_data(%p, %p, %p, %d);\n", sess, e, packet, len);

	e->event.user_data.user_count = 0;
	e->event.user_data.users = NULL;

	if (p + sizeof(d) > packet_end)
		goto malformed;

	memcpy(&d, p, sizeof(d));
	p += sizeof(d);

	d.type = gg_fix32(d.type);
	d.user_count = gg_fix32(d.user_count);

	if (d.user_count > 0xffff) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_user_data() malformed packet (1)\n");
		goto malformed;
	}

	if (d.user_count > 0) {
		users = (gg_event_user_data_user*)calloc(d.user_count, sizeof(struct gg_event_user_data_user));

		if (users == NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_user_data() out of memory (%d*%d)\n", d.user_count, sizeof(struct gg_event_user_data_user));
			goto fail;
		}
	} else {
		users = NULL;
	}

	e->type = GG_EVENT_USER_DATA;
	e->event.user_data.type = d.type;
	e->event.user_data.user_count = d.user_count;
	e->event.user_data.users = users;
	
	gg_debug_session(sess, GG_DEBUG_DUMP, "type=%d, count=%d\n", d.type, d.user_count);

	for (i = 0; i < d.user_count; i++) {
		struct gg_user_data_user u;
		struct gg_event_user_data_attr *attrs;

		if (p + sizeof(u) > packet_end) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_user_data() malformed packet (2)\n");
			goto malformed;
		}

		memcpy(&u, p, sizeof(u));
		p += sizeof(u);

		u.uin = gg_fix32(u.uin);
		u.attr_count = gg_fix32(u.attr_count);

		if (u.attr_count > 0xffff) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_user_data() malformed packet (2)\n");
			goto malformed;
		}

		if (u.attr_count > 0) {
			attrs = (gg_event_user_data_attr*)calloc(u.attr_count, sizeof(struct gg_event_user_data_attr));

			if (attrs == NULL) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_user_data() out of memory (%d*%d)\n", u.attr_count, sizeof(struct gg_event_user_data_attr));
				goto fail;
			}
		} else {
			attrs = NULL;
		}

		users[i].uin = u.uin;
		users[i].attr_count = u.attr_count;
		users[i].attrs = attrs;

		gg_debug_session(sess, GG_DEBUG_DUMP, "    uin=%d, count=%d\n", u.uin, u.attr_count);

		for (j = 0; j < u.attr_count; j++) {
			uint32_t key_size;
			uint32_t attr_type;
			uint32_t value_size;
			char *key;
			char *value;

			if (p + sizeof(key_size) > packet_end) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_user_data() malformed packet (3)\n");
				goto malformed;
			}

			memcpy(&key_size, p, sizeof(key_size));
			p += sizeof(key_size);

			key_size = gg_fix32(key_size);

			if (key_size > 0xffff || p + key_size > packet_end) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_user_data() malformed packet (3)\n");
				goto malformed;
			}

			key = (char*)malloc(key_size + 1);

			if (key == NULL) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_user_data() out of memory (%d)\n", key_size + 1);
				goto fail;
			}

			memcpy(key, p, key_size);
			p += key_size;

			key[key_size] = 0;

			attrs[j].key = key;

			if (p + sizeof(attr_type) + sizeof(value_size) > packet_end) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_user_data() malformed packet (4)\n");
				goto malformed;
			}

			memcpy(&attr_type, p, sizeof(attr_type));
			p += sizeof(attr_type);
			memcpy(&value_size, p, sizeof(value_size));
			p += sizeof(value_size);

			attrs[j].type = gg_fix32(attr_type);
			value_size = gg_fix32(value_size);

			if (value_size > 0xffff || p + value_size > packet_end) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_user_data() malformed packet (5)\n");
				goto malformed;
			}

			value = (char*)malloc(value_size + 1);

			if (value == NULL) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_user_data() out of memory (%d)\n", value_size + 1);
				goto fail;
			}

			memcpy(value, p, value_size);
			p += value_size;

			value[value_size] = 0;

			attrs[j].value = value;

			gg_debug_session(sess, GG_DEBUG_DUMP, "        key=\"%s\", type=%d, value=\"%s\"\n", key, attr_type, value);
		}
	}

	return 0;

fail:
	res = -1;

malformed:
	e->type = GG_EVENT_NONE;

	for (i = 0; i < e->event.user_data.user_count; i++) {
		for (j = 0; j < e->event.user_data.users[i].attr_count; j++) {
			free(e->event.user_data.users[i].attrs[j].key);
			free(e->event.user_data.users[i].attrs[j].value);
		}

		free(e->event.user_data.users[i].attrs);
	}

	free(e->event.user_data.users);

	return res;
}

/**
 * \internal Analizuje przychodzący pakiet z listą sesji multilogowania.
 *
 * \param sess Struktura sesji
 * \param e Struktura zdarzenia
 * \param payload Treść pakietu
 * \param len Długość pakietu
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_handle_multilogon_info(struct gg_session *sess, struct gg_event *e, void *packet, size_t len)
{
	char *packet_end = (char*) packet + len;
	struct gg_multilogon_info *info = (struct gg_multilogon_info*) packet;
	char *p = (char*) packet + sizeof(*info);
	struct gg_multilogon_session *sessions = NULL;
	size_t count;
	size_t i;
	int res = 0;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_handle_multilogon_info(%p, %p, %p, %d);\n", sess, e, packet, len);

	count = gg_fix32(info->count);

	if (count > 0xffff) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_multilogon_info() malformed packet (1)\n");
		goto malformed;
	}

	sessions = (gg_multilogon_session*)calloc(count, sizeof(struct gg_multilogon_session));

	if (sessions == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_multilogon_info() out of memory (%d*%d)\n", count, sizeof(struct gg_multilogon_session));
		return -1;
	}
	
	e->type = GG_EVENT_MULTILOGON_INFO;
	e->event.multilogon_info.count = (int)count;
	e->event.multilogon_info.sessions = sessions;

	for (i = 0; i < count; i++) {
		struct gg_multilogon_info_item item;
		size_t name_size;

		if (p + sizeof(item) > packet_end) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_multilogon_info() malformed packet (2)\n");
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
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_multilogon_info() malformed packet (3)\n");
			goto malformed;
		}

		sessions[i].name = (char*)malloc(name_size + 1);

		if (sessions[i].name == NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_multilogon_info() out of memory (%d)\n", name_size);
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
	e->type = GG_EVENT_NONE;

	for (i = 0; i < (size_t)e->event.multilogon_info.count; i++)
		free(e->event.multilogon_info.sessions[i].name);

	free(e->event.multilogon_info.sessions);

	return res;
}

/**
 * \internal Odbiera pakiet od serwera.
 *
 * Analizuje pakiet i wypełnia strukturę zdarzenia.
 *
 * \param sess Struktura sesji
 * \param e Struktura zdarzenia
 *
 * \return 0 jeśli się powiodło, -1 jeśli wystąpił błąd
 */
static int gg_watch_fd_connected(struct gg_session *sess, struct gg_event *e)
{
	struct gg_header *h = NULL;
	char *p;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_watch_fd_connected(%p, %p);\n", sess, e);

	if (!sess) {
		errno = EFAULT;
		return -1;
	}

	if (!(h = (gg_header*)gg_recv_packet(sess))) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() gg_recv_packet failed (errno=%d, %s)\n", errno, strerror(errno));
		goto fail;
	}

	p = (char*) h + sizeof(struct gg_header);

	switch (h->type) {
		case GG_RECV_MSG:
		{
			if (h->length >= sizeof(struct gg_recv_msg)) {
				if (gg_handle_recv_msg(h, e, sess) != -1)
					gg_handle_recv_msg_ack(h, sess);
				else
					goto fail;
			}

			break;
		}

		case GG_RECV_MSG80:
		{
			if (h->length >= sizeof(struct gg_recv_msg80)) {
				if (gg_handle_recv_msg80(h, e, sess, GG_EVENT_MSG) != -1)
					gg_handle_recv_msg_ack(h, sess);
				else
					goto fail;
			}

			break;
		}

		case GG_RECV_OWN_MSG:
		{
			if (h->length >= sizeof(struct gg_recv_msg80)) {
				if (gg_handle_recv_msg80(h, e, sess, GG_EVENT_MULTILOGON_MSG) == -1)
					goto fail;
			}

			break;
		}

		case GG_NOTIFY_REPLY:
		{
			struct gg_notify_reply *n = (gg_notify_reply*)p;
			unsigned int count, i;
			char *tmp;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a notify reply\n");

			if (h->length < sizeof(*n)) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() incomplete packet\n");
				errno = EINVAL;
				goto fail;
			}

			if (gg_fix32(n->status) == GG_STATUS_BUSY_DESCR || gg_fix32(n->status) == GG_STATUS_NOT_AVAIL_DESCR || gg_fix32(n->status) == GG_STATUS_AVAIL_DESCR) {
				e->type = GG_EVENT_NOTIFY_DESCR;

				if (!(e->event.notify_descr.notify = (gg_notify_reply*)malloc(sizeof(*n) * 2))) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for notify data\n");
					goto fail;
				}
				e->event.notify_descr.notify[1].uin = 0;
				memcpy(e->event.notify_descr.notify, p, sizeof(*n));
				e->event.notify_descr.notify[0].uin = gg_fix32(e->event.notify_descr.notify[0].uin);
				e->event.notify_descr.notify[0].status = gg_fix32(e->event.notify_descr.notify[0].status);
				e->event.notify_descr.notify[0].remote_port = gg_fix16(e->event.notify_descr.notify[0].remote_port);
				e->event.notify_descr.notify[0].version = gg_fix32(e->event.notify_descr.notify[0].version);

				count = h->length - sizeof(*n);
				if (!(tmp = (char*)malloc(count + 1))) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for notify data\n");
					goto fail;
				}
				memcpy(tmp, p + sizeof(*n), count);
				tmp[count] = 0;
				e->event.notify_descr.descr = tmp;

			} else {
				e->type = GG_EVENT_NOTIFY;

				if (!(e->event.notify = (gg_notify_reply*)malloc(h->length + 2 * sizeof(*n)))) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for notify data\n");
					goto fail;
				}

				memcpy(e->event.notify, p, h->length);
				count = h->length / sizeof(*n);
				e->event.notify[count].uin = 0;

				for (i = 0; i < count; i++) {
					e->event.notify[i].uin = gg_fix32(e->event.notify[i].uin);
					e->event.notify[i].status = gg_fix32(e->event.notify[i].status);
					e->event.notify[i].remote_port = gg_fix16(e->event.notify[i].remote_port);
					e->event.notify[i].version = gg_fix32(e->event.notify[i].version);
				}
			}

			break;
		}

		case GG_STATUS:
		{
			struct gg_status *s = (gg_status*)p;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a status change\n");

			if (h->length >= sizeof(*s)) {
				e->type = GG_EVENT_STATUS;
				memcpy(&e->event.status, p, sizeof(*s));
				e->event.status.uin = gg_fix32(e->event.status.uin);
				e->event.status.status = gg_fix32(e->event.status.status);
				if (h->length > sizeof(*s)) {
					int len = h->length - sizeof(*s);
					char *buf = (char*)malloc(len + 1);
					if (buf) {
						memcpy(buf, p + sizeof(*s), len);
						buf[len] = 0;
					}
					e->event.status.descr = buf;
				} else
					e->event.status.descr = NULL;
			}

			break;
		}

		case GG_NOTIFY_REPLY77:
		case GG_NOTIFY_REPLY80BETA:
		{
			struct gg_notify_reply77 *n = (gg_notify_reply77*)p;
			unsigned int length = h->length, i = 0;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a notify reply\n");

			e->type = GG_EVENT_NOTIFY60;
			e->event.notify60 = (gg_event_notify60*)malloc(sizeof(*e->event.notify60));

			if (!e->event.notify60) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for notify data\n");
				goto fail;
			}

			e->event.notify60[0].uin = 0;

			while (length >= sizeof(struct gg_notify_reply77)) {
				uin_t uin = gg_fix32(n->uin);
				char *tmp;

				e->event.notify60[i].uin = uin & 0x00ffffff;
				e->event.notify60[i].status = n->status;
				e->event.notify60[i].remote_ip = n->remote_ip;
				e->event.notify60[i].remote_port = gg_fix16(n->remote_port);
				e->event.notify60[i].version = n->version;
				e->event.notify60[i].image_size = n->image_size;
				e->event.notify60[i].descr = NULL;
				e->event.notify60[i].time = 0;

				if (uin & 0x40000000)
					e->event.notify60[i].version |= GG_HAS_AUDIO_MASK;
				if (uin & 0x20000000)
					e->event.notify60[i].version |= GG_HAS_AUDIO7_MASK;
				if (uin & 0x08000000)
					e->event.notify60[i].version |= GG_ERA_OMNIX_MASK;

				if (GG_S_D(n->status)) {
					unsigned char descr_len = *((char*) n + sizeof(struct gg_notify_reply77));

					if (sizeof(struct gg_notify_reply77) + descr_len <= length) {
						char *descr;

						if (!(descr = (char*)malloc(descr_len + 1))) {
							gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for notify data\n");
							goto fail;
						}

						memcpy(descr, (char*) n + sizeof(struct gg_notify_reply77) + 1, descr_len);
						descr[descr_len] = 0;

						if (h->type == GG_NOTIFY_REPLY80BETA && sess->encoding != GG_ENCODING_UTF8) {
							char *cp_descr = gg_utf8_to_cp(descr);

							if (!cp_descr) {
								gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for notify data\n");
								free(descr);
								goto fail;
							}

							free(descr);
							descr = cp_descr;
						}

						e->event.notify60[i].descr = descr;

						/* XXX czas */
							
						length -= sizeof(struct gg_notify_reply77) + descr_len + 1;
						n = (gg_notify_reply77*)((char*)n + sizeof(struct gg_notify_reply77) + descr_len + 1);
					} else {
						length = 0;
					}

				} else {
					length -= sizeof(struct gg_notify_reply77);
					n = (gg_notify_reply77*)((char*)n + sizeof(struct gg_notify_reply77));
				}

				if (!(tmp = (char*)realloc(e->event.notify60, (i + 2) * sizeof(*e->event.notify60)))) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for notify data\n");
					free(e->event.notify60);
					goto fail;
				}

				e->event.notify60 = (gg_event_notify60*)tmp;
				e->event.notify60[++i].uin = 0;
			}

			break;
		}

		case GG_STATUS77:
		case GG_STATUS80BETA:
		{
			struct gg_status77 *s = (gg_status77*)p;
			uint32_t uin;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a status change\n");

			if (h->length < sizeof(*s))
				break;

			uin = gg_fix32(s->uin);

			e->type = GG_EVENT_STATUS60;
			e->event.status60.uin = uin & 0x00ffffff;
			e->event.status60.status = s->status;
			e->event.status60.remote_ip = s->remote_ip;
			e->event.status60.remote_port = gg_fix16(s->remote_port);
			e->event.status60.version = s->version;
			e->event.status60.image_size = s->image_size;
			e->event.status60.descr = NULL;
			e->event.status60.time = 0;

			if (uin & 0x40000000)
				e->event.status60.version |= GG_HAS_AUDIO_MASK;
			if (uin & 0x20000000)
				e->event.status60.version |= GG_HAS_AUDIO7_MASK;
			if (uin & 0x08000000)
				e->event.status60.version |= GG_ERA_OMNIX_MASK;

			if (h->length > sizeof(*s)) {
				int len = h->length - sizeof(*s);
				char *buf = (char*)malloc(len + 1);

				/* XXX, jesli malloc() sie nie uda to robic tak samo jak przy GG_NOTIFY_REPLY* ?
				 * 	- goto fail; (?)
				 */
				if (buf) {
					memcpy(buf, (char*) p + sizeof(*s), len);
					buf[len] = 0;

					if (h->type == GG_STATUS80BETA && sess->encoding != GG_ENCODING_UTF8) {
						char *cp_buf = gg_utf8_to_cp(buf);
						free(buf);
						buf = cp_buf;
					}
				}

				e->event.status60.descr = buf;

				if (len > 4 && p[h->length - 5] == 0) {
					uint32_t t;
					memcpy(&t, p + h->length - 4, sizeof(uint32_t));
					e->event.status60.time = gg_fix32(t);
				}
			}

			break;
		}

		case GG_NOTIFY_REPLY60:
		{
			struct gg_notify_reply60 *n = (gg_notify_reply60*)p;
			unsigned int length = h->length, i = 0;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a notify reply\n");

			e->type = GG_EVENT_NOTIFY60;
			e->event.notify60 = (gg_event_notify60*)malloc(sizeof(*e->event.notify60));

			if (!e->event.notify60) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for notify data\n");
				goto fail;
			}

			e->event.notify60[0].uin = 0;

			while (length >= sizeof(struct gg_notify_reply60)) {
				uin_t uin = gg_fix32(n->uin);
				char *tmp;

				e->event.notify60[i].uin = uin & 0x00ffffff;
				e->event.notify60[i].status = n->status;
				e->event.notify60[i].remote_ip = n->remote_ip;
				e->event.notify60[i].remote_port = gg_fix16(n->remote_port);
				e->event.notify60[i].version = n->version;
				e->event.notify60[i].image_size = n->image_size;
				e->event.notify60[i].descr = NULL;
				e->event.notify60[i].time = 0;

				if (uin & 0x40000000)
					e->event.notify60[i].version |= GG_HAS_AUDIO_MASK;
				if (uin & 0x08000000)
					e->event.notify60[i].version |= GG_ERA_OMNIX_MASK;

				if (GG_S_D(n->status)) {
					unsigned char descr_len = *((char*) n + sizeof(struct gg_notify_reply60));

					if (sizeof(struct gg_notify_reply60) + descr_len <= length) {
						if (!(e->event.notify60[i].descr = (char*)malloc(descr_len + 1))) {
							gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for notify data\n");
							goto fail;
						}

						memcpy(e->event.notify60[i].descr, (char*) n + sizeof(struct gg_notify_reply60) + 1, descr_len);
						e->event.notify60[i].descr[descr_len] = 0;

						/* XXX czas */
							
						length -= sizeof(struct gg_notify_reply60) + descr_len + 1;
						n = (gg_notify_reply60*)((char*)n + sizeof(struct gg_notify_reply60) + descr_len + 1);
					} else {
						length = 0;
					}

				} else {
					length -= sizeof(struct gg_notify_reply60);
					n = (gg_notify_reply60*)((char*)n + sizeof(struct gg_notify_reply60));
				}

				if (!(tmp = (char*)realloc(e->event.notify60, (i + 2) * sizeof(*e->event.notify60)))) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for notify data\n");
					free(e->event.notify60);
					goto fail;
				}

				e->event.notify60 = (gg_event_notify60*)tmp;
				e->event.notify60[++i].uin = 0;
			}

			break;
		}

		case GG_STATUS60:
		{
			struct gg_status60 *s = (gg_status60*)p;
			uint32_t uin;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a status change\n");

			if (h->length < sizeof(*s))
				break;

			uin = gg_fix32(s->uin);

			e->type = GG_EVENT_STATUS60;
			e->event.status60.uin = uin & 0x00ffffff;
			e->event.status60.status = s->status;
			e->event.status60.remote_ip = s->remote_ip;
			e->event.status60.remote_port = gg_fix16(s->remote_port);
			e->event.status60.version = s->version;
			e->event.status60.image_size = s->image_size;
			e->event.status60.descr = NULL;
			e->event.status60.time = 0;

			if (uin & 0x40000000)
				e->event.status60.version |= GG_HAS_AUDIO_MASK;
			if (uin & 0x08000000)
				e->event.status60.version |= GG_ERA_OMNIX_MASK;

			if (h->length > sizeof(*s)) {
				int len = h->length - sizeof(*s);
				char *buf = (char*)malloc(len + 1);

				if (buf) {
					memcpy(buf, (char*) p + sizeof(*s), len);
					buf[len] = 0;
				}

				e->event.status60.descr = buf;

				if (len > 4 && p[h->length - 5] == 0) {
					uint32_t t;
					memcpy(&t, p + h->length - 4, sizeof(uint32_t));
					e->event.status60.time = gg_fix32(t);
				}
			}

			break;
		}

		case GG_STATUS80:
		{
			struct gg_notify_reply80 *s = (gg_notify_reply80*)p;
			uint32_t descr_len;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a status change\n");

			if (h->length < sizeof(*s))
				break;

			e->type = GG_EVENT_STATUS60;
			e->event.status60.uin		= gg_fix32(s->uin);
			e->event.status60.status	= gg_fix32(s->status);
			e->event.status60.remote_ip	= s->remote_ip;
			e->event.status60.remote_port	= gg_fix16(s->remote_port);
			e->event.status60.image_size	= s->image_size;
			e->event.status60.descr		= NULL;
			e->event.status60.version	= 0x00;	/* not-supported */
			e->event.status60.time		= 0;	/* not-supported */

			descr_len = gg_fix32(s->descr_len);

			if (descr_len > 0 && h->length-sizeof(*s) >= descr_len) {
				char *buf = (char*)malloc(descr_len + 1);

				if (buf) {
					memcpy(buf, (char*) p + sizeof(*s), descr_len);
					buf[descr_len] = 0;

					if (sess->encoding != GG_ENCODING_UTF8) {
						char *cp_buf = gg_utf8_to_cp(buf);
						free(buf);
						buf = cp_buf;
					}
				}

				e->event.status60.descr = buf;
			}
			break;
		}

		case GG_NOTIFY_REPLY80:
		{
			struct gg_notify_reply80 *n = (gg_notify_reply80*)p;
			unsigned int length = h->length, i = 0;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a notify reply\n");

			e->type = GG_EVENT_NOTIFY60;
			e->event.notify60 = (gg_event_notify60*)malloc(sizeof(*e->event.notify60));

			if (!e->event.notify60) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for notify data\n");
				goto fail;
			}

			e->event.notify60[0].uin = 0;

			while (length >= sizeof(struct gg_notify_reply80)) {
				uint32_t descr_len;
				char *tmp;

				e->event.notify60[i].uin	= gg_fix32(n->uin);
				e->event.notify60[i].status	= gg_fix32(n->status);
				e->event.notify60[i].remote_ip	= n->remote_ip;
				e->event.notify60[i].remote_port= gg_fix16(n->remote_port);
				e->event.notify60[i].image_size	= n->image_size;
				e->event.notify60[i].descr	= NULL;
				e->event.notify60[i].version	= 0x00;	/* not-supported */
				e->event.notify60[i].time	= 0;	/* not-supported */

				descr_len = gg_fix32(n->descr_len);

				length -= sizeof(struct gg_notify_reply80);
				n = (gg_notify_reply80*)((char*)n + sizeof(struct gg_notify_reply80));

				if (descr_len) {
					if (length >= descr_len) {
						/* XXX, GG_S_D(n->status) */
						char *descr;

						if (!(descr = (char*)malloc(descr_len + 1))) {
							gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for notify data\n");
							goto fail;
						}

						memcpy(descr, n, descr_len);
						descr[descr_len] = 0;

						if (sess->encoding != GG_ENCODING_UTF8) {
							char *cp_descr = gg_utf8_to_cp(descr);

							if (!cp_descr) {
								gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for notify data\n");
								free(descr);
								goto fail;
							}

							free(descr);
							descr = cp_descr;
						}
						e->event.notify60[i].descr = descr;

						length -= descr_len;
						n = (gg_notify_reply80*)((char*)n + descr_len);
					} else
						length = 0;
				}

				if (!(tmp = (char*)realloc(e->event.notify60, (i + 2) * sizeof(*e->event.notify60)))) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for notify data\n");
					free(e->event.notify60);
					goto fail;
				}

				e->event.notify60 = (gg_event_notify60*)tmp;
				e->event.notify60[++i].uin = 0;
			}
			break;
		}

		case GG_SEND_MSG_ACK:
		{
			struct gg_send_msg_ack *s = (gg_send_msg_ack*)p;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a message ack\n");

			if (h->length < sizeof(*s))
				break;

			e->type = GG_EVENT_ACK;
			e->event.ack.status = gg_fix32(s->status);
			e->event.ack.recipient = gg_fix32(s->recipient);
			e->event.ack.seq = gg_fix32(s->seq);

			break;
		}

		case GG_PONG:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a pong\n");

			e->type = GG_EVENT_PONG;
			sess->last_pong = (int)time(NULL);

			break;
		}

		case GG_DISCONNECTING:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received disconnection warning\n");
			e->type = GG_EVENT_DISCONNECT;
			break;
		}

		case GG_DISCONNECT_ACK:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received disconnection acknowledge\n");
			e->type = GG_EVENT_DISCONNECT_ACK;
			break;
		}

		case GG_XML_EVENT:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received XML event\n");
			e->type = GG_EVENT_XML_EVENT;
			if (!(e->event.xml_event.data = (char *) malloc(h->length + 1))) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for XML event data\n");
				goto fail;
			}
			memcpy(e->event.xml_event.data, p, h->length);
			e->event.xml_event.data[h->length] = 0;
			break;
		}

		case GG_XML_ACTION:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received XML action\n");
			e->type = GG_EVENT_XML_ACTION;
			if (!(e->event.xml_action.data = (char *) malloc(h->length + 1))) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for XML action data\n");
				goto fail;
			}
			memcpy(e->event.xml_action.data, p, h->length);
			e->event.xml_action.data[h->length] = 0;
			break;
		}

		case GG_PUBDIR50_REPLY:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received pubdir/search reply\n");
			if (gg_pubdir50_handle_reply_sess(sess, e, p, h->length) == -1)
				goto fail;
			break;
		}

		case GG_USERLIST_REPLY:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received userlist reply\n");

			if (h->length < 1)
				break;

			/* jeśli odpowiedź na eksport, wywołaj zdarzenie tylko
			 * gdy otrzymano wszystkie odpowiedzi */
			if (p[0] == GG_USERLIST_PUT_REPLY || p[0] == GG_USERLIST_PUT_MORE_REPLY) {
				if (--sess->userlist_blocks)
					break;

				p[0] = GG_USERLIST_PUT_REPLY;
			}

			if (h->length > 1) {
				char *tmp;
				unsigned int len = (sess->userlist_reply) ? (unsigned int)strlen(sess->userlist_reply) : 0;

				gg_debug_session(sess, GG_DEBUG_MISC, "userlist_reply=%p, len=%d\n", sess->userlist_reply, len);

				if (!(tmp = (char*)realloc(sess->userlist_reply, len + h->length))) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() not enough memory for userlist reply\n");
					free(sess->userlist_reply);
					sess->userlist_reply = NULL;
					goto fail;
				}

				sess->userlist_reply = tmp;
				sess->userlist_reply[len + h->length - 1] = 0;
				memcpy(sess->userlist_reply + len, p + 1, h->length - 1);
			}

			if (p[0] == GG_USERLIST_GET_MORE_REPLY)
				break;

			e->type = GG_EVENT_USERLIST;
			e->event.userlist.type = p[0];
			e->event.userlist.reply = sess->userlist_reply;
			sess->userlist_reply = NULL;

			break;
		}

		case GG_DCC7_ID_REPLY:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 id packet\n");

			if (h->length < sizeof(struct gg_dcc7_id_reply))
				break;

			if (gg_dcc7_handle_id(sess, e, p, h->length) == -1)
				goto fail;

			break;
		}

		case GG_DCC7_ACCEPT:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 accept\n");

			if (h->length < sizeof(struct gg_dcc7_accept))
				break;

			if (gg_dcc7_handle_accept(sess, e, p, h->length) == -1)
				goto fail;

			break;
		}

		case GG_DCC7_NEW:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 request\n");

			if (h->length < sizeof(struct gg_dcc7_new))
				break;

			if (gg_dcc7_handle_new(sess, e, p, h->length) == -1)
				goto fail;

			break;
		}

		case GG_DCC7_REJECT:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 reject\n");

			if (h->length < sizeof(struct gg_dcc7_reject))
				break;

			if (gg_dcc7_handle_reject(sess, e, p, h->length) == -1)
				goto fail;

			break;
		}

		case GG_DCC7_ABORT:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 abort\n");

			if (h->length < sizeof(struct gg_dcc7_aborted))
				break;

			if (gg_dcc7_handle_abort(sess, e, p, h->length) == -1)
				goto fail;

			break;
		}

		case GG_DCC7_INFO:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 info\n");

			if (h->length < sizeof(struct gg_dcc7_info))
				break;

			if (gg_dcc7_handle_info(sess, e, p, h->length) == -1)
				goto fail;

			break;
		}

		case GG_USER_DATA:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received user data\n");

			if (h->length < sizeof(struct gg_user_data))
				break;

			if (gg_handle_user_data(sess, e, p, h->length) == -1)
				goto fail;

			break;
		}

		case GG_TYPING_NOTIFICATION:
		{
			struct gg_typing_notification *n = (struct gg_typing_notification*)p;
			uin_t uin;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received typing notification\n");

			if (h->length < sizeof(*n))
				break;

			memcpy(&uin, &n->uin, sizeof(uin_t));

			e->type = GG_EVENT_TYPING_NOTIFICATION;
			e->event.typing_notification.uin = gg_fix32(uin);
			e->event.typing_notification.length = gg_fix16(n->length);

			break;
		}

		case GG_MULTILOGON_INFO:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received multilogon info\n");

			if (h->length < sizeof(struct gg_multilogon_info))
				break;

			if (gg_handle_multilogon_info(sess, e, p, h->length) == -1)
				goto fail;

			break;
		}

		default:
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd_connected() received unknown packet 0x%.2x\n", h->type);
	}

	free(h);
	return 0;

fail:
	free(h);
	return -1;
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
				goto fail_connecting;
			else
				goto done;
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
				goto fail_resolving;
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
				goto fail_connecting;
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
				if (sess->proxy_addr && sess->proxy_port)
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection to proxy failed (errno=%d, %s)\n", res, strerror(res));
				else
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection to hub failed (errno=%d, %s)\n", res, strerror(res));

				goto fail_connecting;
			}

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connected to hub, sending query\n");

			if (!(client = gg_urlencode((sess->client_version) ? sess->client_version : GG_DEFAULT_CLIENT_VERSION))) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() out of memory for client version\n");
				goto fail_connecting;
			}

			if (!gg_proxy_http_only && sess->proxy_addr && sess->proxy_port)
				host = "http://" GG_APPMSG_HOST;
			else
				host = "";

			auth = gg_proxy_auth();

#ifdef GG_CONFIG_MIRANDA
			if (sess->tls) {
				snprintf(buf, sizeof(buf) - 1,
					"GET %s/appsvc/appmsg_ver10.asp?fmnumber=%u&fmt=2&lastmsg=%d&version=%s&age=2&gender=1 HTTP/1.0\r\n"
					"Connection: close\r\n"
					"Host: " GG_APPMSG_HOST "\r\n"
					"%s"
					"\r\n", host, sess->uin, sess->last_sysmsg, client, (auth) ? auth : "");
			} else
#elif GG_CONFIG_HAVE_OPENSSL
			if (sess->ssl != NULL) {
				snprintf(buf, sizeof(buf) - 1,
					"GET %s/appsvc/appmsg_ver10.asp?fmnumber=%u&fmt=2&lastmsg=%d&version=%s&age=2&gender=1 HTTP/1.0\r\n"
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

			/* zwolnij pamięć po wersji klienta. */
			if (sess->client_version) {
				free(sess->client_version);
				sess->client_version = NULL;
			}

			gg_debug_session(sess, GG_DEBUG_MISC, "=> -----BEGIN-HTTP-QUERY-----\n%s\n=> -----END-HTTP-QUERY-----\n", buf);

			/* zapytanie jest krótkie, więc zawsze zmieści się
			 * do bufora gniazda. jeśli write() zwróci mniej,
			 * stało się coś złego. */
			if (gg_sock_write(sess->fd, buf, (int)strlen(buf)) < (signed)strlen(buf)) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() sending query failed\n");

				e->type = GG_EVENT_CONN_FAILED;
				e->event.failure = GG_FAILURE_WRITING;
				sess->state = GG_STATE_IDLE;
				gg_sock_close(sess->fd);
				sess->fd = -1;
				break;
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
				goto fail_connecting;
			}

			/* ignorujemy resztę nagłówka. */
			while (strcmp(buf, "\r\n") && strcmp(buf, ""))
				gg_read_line(sess->fd, buf, sizeof(buf) - 1);

			/* czytamy pierwszą linię danych. */
			gg_read_line(sess->fd, buf, sizeof(buf) - 1);
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

			if (!strcmp(host, "notoperating")) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() service unavailable\n", errno, strerror(errno));
				sess->fd = -1;
				goto fail_unavailable;
			}

			addr.s_addr = inet_addr(host);
			sess->server_addr = addr.s_addr;

			if (!gg_proxy_http_only && sess->proxy_addr && sess->proxy_port) {
				/* jeśli mamy proxy, łączymy się z nim. */
				if ((sess->fd = gg_connect(&sess->proxy_addr, sess->proxy_port, sess->async)) == -1) {
					/* nie wyszło? trudno. */
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection to proxy failed (errno=%d, %s)\n", errno, strerror(errno));
					goto fail_connecting;
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
					goto fail_resolving;
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
					goto fail_connecting;
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
				goto fail_resolving;
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
					goto fail_connecting;
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
					goto fail_connecting;
				}

				gg_sock_close(sess->fd);
				sess->fd = -1;

#ifdef ETIMEDOUT
				if (sess->timeout == 0)
					errno = ETIMEDOUT;
#endif

#ifdef GG_CONFIG_HAVE_OPENSSL
				/* jeśli logujemy się po TLS, nie próbujemy
				 * się łączyć już z niczym innym w przypadku
				 * błędu. nie dość, że nie ma sensu, to i
				 * trzeba by się bawić w tworzenie na nowo
				 * SSL i SSL_CTX. */

				if (sess->ssl) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s)\n", res, strerror(res));
					goto fail_connecting;
				}
#endif

				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s), trying https\n", res, strerror(res));

				if (sess->port == GG_HTTPS_PORT)
					goto fail_connecting;

				sess->port = GG_HTTPS_PORT;

				/* próbujemy na port 443. */
				if ((sess->fd = gg_connect(&sess->server_addr, sess->port, sess->async)) == -1) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s)\n", errno, strerror(errno));
					goto fail_connecting;
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
					goto fail_connecting;
				}

				if (auth) {
					gg_debug_session(sess, GG_DEBUG_MISC, "//   %s", auth);
					if (gg_sock_write(sess->fd, auth, (int)strlen(auth)) < (signed)strlen(auth)) {
						gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() can't send proxy request\n");
						free(auth);
						goto fail_connecting;
					}

					free(auth);
				}

				if (gg_sock_write(sess->fd, "\r\n", 2) < 2) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() can't send proxy request\n");
					goto fail_connecting;
				}
			}

#ifdef GG_CONFIG_MIRANDA
			if (sess->tls) {
				sess->state = GG_STATE_TLS_NEGOTIATION;
				sess->check = GG_CHECK_WRITE;
				sess->timeout = GG_DEFAULT_TIMEOUT;

				break;
			}
#elif GG_CONFIG_HAVE_OPENSSL
			if (sess->ssl != NULL) {
				SSL_set_fd(sess->ssl, (int)sess->fd);

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

			sess->ssl = si.connect(sess->fd, 0, 0);

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
		{
			struct gg_header *h;
			struct gg_welcome *w;
			unsigned char *password = (unsigned char*) sess->password;
			int ret;
			uint8_t login_hash[64];

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_READING_KEY\n");

			memset(login_hash, 0, sizeof(login_hash));

			/* XXX bardzo, bardzo, bardzo głupi pomysł na pozbycie
			 * się tekstu wrzucanego przez proxy. */
			if (sess->proxy_addr && sess->proxy_port) {
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

			/* czytaj pierwszy pakiet. */
			if (!(h = (gg_header*)gg_recv_packet(sess))) {
				if (errno == EAGAIN) {
					sess->check = GG_CHECK_READ;
					break;
				}

				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() didn't receive packet (errno=%d, %s)\n", errno, strerror(errno));

				e->type = GG_EVENT_CONN_FAILED;
				e->event.failure = GG_FAILURE_READING;
				sess->state = GG_STATE_IDLE;
				errno2 = errno;
				gg_sock_close(sess->fd);
				errno = errno2;
				sess->fd = -1;
				break;
			}

			if (h->type != GG_WELCOME) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() invalid packet received\n");
				free(h);
				gg_sock_close(sess->fd);
				sess->fd = -1;
				errno = EINVAL;
				e->type = GG_EVENT_CONN_FAILED;
				e->event.failure = GG_FAILURE_INVALID;
				sess->state = GG_STATE_IDLE;
				break;
			}

			w = (struct gg_welcome*) ((char*) h + sizeof(struct gg_header));
			w->key = gg_fix32(w->key);

			switch (sess->hash_type) {
				case GG_LOGIN_HASH_GG32:
				{
					unsigned int hash;

					hash = gg_fix32(gg_login_hash(password, w->key));
					gg_debug_session(sess, GG_DEBUG_DUMP, "// gg_watch_fd() challenge %.4x --> GG32 hash %.8x\n", w->key, hash);
					memcpy(login_hash, &hash, sizeof(hash));

					break;
				}

				case GG_LOGIN_HASH_SHA1:
				{
					char tmp[41];
					gg_login_hash_sha1((char*) password, w->key, login_hash);
					bin2hex(login_hash, 20, tmp);
					gg_debug_session(sess, GG_DEBUG_DUMP, "// gg_watch_fd() challenge %.4x --> SHA1 hash: %s\n", w->key, tmp);

					break;
				}
			}

			free(h);
			free(sess->password);
			sess->password = NULL;

			if (gg_dcc_ip == (unsigned long) inet_addr("255.255.255.255")) {
				struct sockaddr_in sin;
				int sin_len = sizeof(sin);

				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() detecting address\n");

				if (!getsockname(sess->fd, (struct sockaddr*) &sin, &sin_len)) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() detected address to %s\n", inet_ntoa(sin.sin_addr));
					sess->client_addr = sin.sin_addr.s_addr;
				} else {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() unable to detect address\n");
					sess->client_addr = 0;
				}
			} else
				sess->client_addr = gg_dcc_ip;

			if (sess->protocol_version >= 0x2e) {
				struct gg_login80 l;
				const char *version = (sess->client_version) ? sess->client_version : GG_DEFAULT_CLIENT_VERSION;
				uint32_t tmp_version_len	= gg_fix32((uint32_t)strlen(GG8_VERSION) + (uint32_t)strlen(version));
				uint32_t tmp_descr_len		= gg_fix32((sess->initial_descr) ? (uint32_t)strlen(sess->initial_descr) : 0);
				
				memset(&l, 0, sizeof(l));
				l.uin           = gg_fix32(sess->uin);
				memcpy(l.language, GG8_LANG, sizeof(l.language));
				l.hash_type     = sess->hash_type;
				memcpy(l.hash, login_hash, sizeof(login_hash));
				l.status        = gg_fix32(sess->initial_status ? sess->initial_status : GG_STATUS_AVAIL);
				l.flags		= gg_fix32(sess->status_flags);
				l.features	= gg_fix32(sess->protocol_features);
				l.image_size    = sess->image_size;
				l.dunno2        = 0x64;

				gg_debug_session(sess, GG_DEBUG_TRAFFIC, "// gg_watch_fd() sending GG_LOGIN80 packet\n");
				ret = gg_send_packet(sess, GG_LOGIN80, 
						&l, sizeof(l), 
						&tmp_version_len, sizeof(uint32_t), GG8_VERSION, strlen(GG8_VERSION), version, strlen(version),
						&tmp_descr_len, sizeof(uint32_t), sess->initial_descr, (sess->initial_descr) ? strlen(sess->initial_descr) : 0,
						NULL);

			} else if (sess->protocol_version == 0x2d) {
				struct gg_login70 l;

				memset(&l, 0, sizeof(l));
				l.uin		= gg_fix32(sess->uin);
				l.local_ip	= (sess->external_addr) ? sess->external_addr : sess->client_addr;
				l.local_port	= gg_fix16((uint16_t)((sess->external_port > 1023) ? sess->external_port : gg_dcc_port));
				l.hash_type	= sess->hash_type;
				memcpy(l.hash, login_hash, sizeof(login_hash));
				l.image_size	= sess->image_size;
				l.dunno2 	= 0x64;
				l.status	= gg_fix32(sess->initial_status ? sess->initial_status : GG_STATUS_AVAIL);
				l.version	= gg_fix32(sess->protocol_version | sess->protocol_flags);

				gg_debug_session(sess, GG_DEBUG_TRAFFIC, "// gg_watch_fd() sending GG_LOGIN80BETA packet\n");
				ret = gg_send_packet(sess, GG_LOGIN80BETA,
						&l, sizeof(l),
						sess->initial_descr, (sess->initial_descr) ? strlen(sess->initial_descr) : 0,
						(sess->initial_descr) ? "\0" : NULL, (sess->initial_descr) ? 1 : 0,
						NULL);
			} else {
				struct gg_login70 l;

				memset(&l, 0, sizeof(l));
				l.local_ip	= (sess->external_addr) ? sess->external_addr : sess->client_addr;
				l.uin		= gg_fix32(sess->uin);
				l.local_port	= gg_fix16((uint16_t)((sess->external_port > 1023) ? sess->external_port : gg_dcc_port));
				l.hash_type	= sess->hash_type;
				memcpy(l.hash, login_hash, sizeof(login_hash));
				l.image_size	= sess->image_size;
				l.dunno2	= 0xbe;
				l.status	= gg_fix32(sess->initial_status ? sess->initial_status : GG_STATUS_AVAIL);
				l.version	= gg_fix32(sess->protocol_version | sess->protocol_flags);

				gg_debug_session(sess, GG_DEBUG_TRAFFIC, "// gg_watch_fd() sending GG_LOGIN70 packet\n");
				ret = gg_send_packet(sess, GG_LOGIN70,
						&l, sizeof(l),
						sess->initial_descr, (sess->initial_descr) ? strlen(sess->initial_descr) : 0,
						NULL);
			}

			free(sess->initial_descr);
			sess->initial_descr = NULL;

			if (ret == -1) {
				gg_debug_session(sess, GG_DEBUG_TRAFFIC, "// gg_watch_fd() sending packet failed. (errno=%d, %s)\n", errno, strerror(errno));
				errno2 = errno;
				gg_sock_close(sess->fd);
				errno = errno2;
				sess->fd = -1;
				e->type = GG_EVENT_CONN_FAILED;
				e->event.failure = GG_FAILURE_WRITING;
				sess->state = GG_STATE_IDLE;
				break;
			}

			sess->state = GG_STATE_READING_REPLY;
			sess->check = GG_CHECK_READ;

			break;
		}

		case GG_STATE_READING_REPLY:
		{
			struct gg_header *h;

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_READING_REPLY\n");

			if (!(h = (gg_header*)gg_recv_packet(sess))) {
				if (errno == EAGAIN) {
					sess->check = GG_CHECK_READ;
					break;
				}

				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() didn't receive packet (errno=%d, %s)\n", errno, strerror(errno));
				e->type = GG_EVENT_CONN_FAILED;
				e->event.failure = GG_FAILURE_READING;
				sess->state = GG_STATE_IDLE;
				errno2 = errno;
				gg_sock_close(sess->fd);
				errno = errno2;
				sess->fd = -1;
				break;
			}

			if (h->type == GG_LOGIN_OK || h->type == GG_NEED_EMAIL || h->type == GG_LOGIN80_OK) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() login succeded\n");
				e->type = GG_EVENT_CONN_SUCCESS;
				sess->state = GG_STATE_CONNECTED;
				sess->check = GG_CHECK_READ;
				sess->timeout = -1;
				sess->status = (sess->initial_status) ? sess->initial_status : GG_STATUS_AVAIL;
				free(h);
				break;
			}

			if (h->type == GG_LOGIN_FAILED || h->type == GG_LOGIN80_FAILED) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() login failed\n");
				e->event.failure = GG_FAILURE_PASSWORD;
				errno = EACCES;
			} else if (h->type == GG_DISCONNECTING) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() too many incorrect password attempts\n");
				e->event.failure = GG_FAILURE_INTRUDER;
				errno = EACCES;
			} else {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() invalid packet\n");
				e->event.failure = GG_FAILURE_INVALID;
				errno = EINVAL;
			}

			e->type = GG_EVENT_CONN_FAILED;
			sess->state = GG_STATE_IDLE;
			errno2 = errno;
			gg_sock_close(sess->fd);
			errno = errno2;
			sess->fd = -1;
			free(h);

			break;
		}

		case GG_STATE_CONNECTED:
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() GG_STATE_CONNECTED\n");

			sess->last_event = (int)time(NULL);

			if ((res = gg_watch_fd_connected(sess, e)) == -1) {

				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_watch_fd() watch_fd_connected failed (errno=%d, %s)\n", errno, strerror(errno));

 				if (errno == EAGAIN) {
					e->type = GG_EVENT_NONE;
					res = 0;
				} else
					res = -1;
			}

			sess->check = GG_CHECK_READ;

			break;
		}
	}

done:
	if (res == -1) {
		free(e);
		e = NULL;
	} else {
		if (sess->send_buf && (sess->state == GG_STATE_READING_REPLY || sess->state == GG_STATE_CONNECTED))
			sess->check |= GG_CHECK_WRITE;
	}

	return e;

fail_connecting:
	if (sess->fd != -1) {
		errno2 = errno;
		gg_sock_close(sess->fd);
		errno = errno2;
		sess->fd = -1;
	}
	e->type = GG_EVENT_CONN_FAILED;
	e->event.failure = GG_FAILURE_CONNECTING;
	sess->state = GG_STATE_IDLE;
	goto done;

fail_resolving:
	e->type = GG_EVENT_CONN_FAILED;
	e->event.failure = GG_FAILURE_RESOLVING;
	sess->state = GG_STATE_IDLE;
	goto done;

fail_unavailable:
	e->type = GG_EVENT_CONN_FAILED;
	e->event.failure = GG_FAILURE_UNAVAILABLE;
	sess->state = GG_STATE_IDLE;
	goto done;
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

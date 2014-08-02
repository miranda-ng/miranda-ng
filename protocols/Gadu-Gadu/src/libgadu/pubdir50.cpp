/* coding: UTF-8 */
/* $Id: pubdir50.c 11370 2010-03-13 16:17:54Z dezred $ */

/*
 *  (C) Copyright 2003 Wojtek Kaniewski <wojtekka@irc.pl>
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
 * \file pubdir50.c
 *
 * \brief Obsługa katalogu publicznego od wersji Gadu-Gadu 5.x
 */

#ifndef _WIN64
#define _USE_32BIT_TIME_T
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include "win32.h"
#undef small
#endif

#include "libgadu.h"
#include "internal.h"

/**
 * Tworzy nowe zapytanie katalogu publicznego.
 *
 * \param type Rodzaj zapytania
 *
 * \return Zmienna \c gg_pubdir50_t lub \c NULL w przypadku błędu.
 *
 * \ingroup pubdir50
 */
gg_pubdir50_t gg_pubdir50_new(int type)
{
	gg_pubdir50_t res = (gg_pubdir50_t)malloc(sizeof(struct gg_pubdir50_s));

	gg_debug(GG_DEBUG_FUNCTION, "** gg_pubdir50_new(%d);\n", type);

	if (!res) {
		gg_debug(GG_DEBUG_MISC, "// gg_pubdir50_new() out of memory\n");
		return NULL;
	}

	memset(res, 0, sizeof(struct gg_pubdir50_s));

	res->type = type;

	return res;
}

/**
 * \internal Dodaje lub zastępuje pole zapytania lub odpowiedzi katalogu
 * publicznego.
 *
 * \param req Zapytanie lub odpowiedź
 * \param num Numer wyniku odpowiedzi (0 dla zapytania)
 * \param field Nazwa pola
 * \param value Wartość pola
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_pubdir50_add_n(gg_pubdir50_t req, int num, const char *field, const char *value)
{
	struct gg_pubdir50_entry *tmp = NULL, *entry;
	char *dupfield, *dupvalue;
	int i;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_pubdir50_add_n(%p, %d, \"%s\", \"%s\");\n", req, num, field, value);

	if (!(dupvalue = strdup(value))) {
		gg_debug(GG_DEBUG_MISC, "// gg_pubdir50_add_n() out of memory\n");
		return -1;
	}

	for (i = 0; i < req->entries_count; i++) {
		if (req->entries[i].num != num || strcmp(req->entries[i].field, field))
			continue;

		free(req->entries[i].value);
		req->entries[i].value = dupvalue;

		return 0;
	}
		
	if (!(dupfield = strdup(field))) {
		gg_debug(GG_DEBUG_MISC, "// gg_pubdir50_add_n() out of memory\n");
		free(dupvalue);
		return -1;
	}

	if (!(tmp = (gg_pubdir50_entry*)realloc(req->entries, sizeof(struct gg_pubdir50_entry) * (req->entries_count + 1)))) {
		gg_debug(GG_DEBUG_MISC, "// gg_pubdir50_add_n() out of memory\n");
		free(dupfield);
		free(dupvalue);
		return -1;
	}

	req->entries = tmp;

	entry = &req->entries[req->entries_count];
	entry->num = num;
	entry->field = dupfield;
	entry->value = dupvalue;

	req->entries_count++;

	return 0;
}

/**
 * Dodaje pole zapytania.
 *
 * \param req Zapytanie
 * \param field Nazwa pola
 * \param value Wartość pola
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup pubdir50
 */
int gg_pubdir50_add(gg_pubdir50_t req, const char *field, const char *value)
{
	return gg_pubdir50_add_n(req, 0, field, value);
}

/**
 * Ustawia numer sekwencyjny zapytania.
 *
 * \param req Zapytanie
 * \param seq Numer sekwencyjny
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup pubdir50
 */
int gg_pubdir50_seq_set(gg_pubdir50_t req, uint32_t seq)
{
	gg_debug(GG_DEBUG_FUNCTION, "** gg_pubdir50_seq_set(%p, %d);\n", req, seq);
	
	if (!req) {
		gg_debug(GG_DEBUG_MISC, "// gg_pubdir50_seq_set() invalid arguments\n");
		errno = EFAULT;
		return -1;
	}

	req->seq = seq;

	return 0;
}

/**
 * Zwalnia zasoby po zapytaniu lub odpowiedzi katalogu publicznego.
 *
 * \param s Zapytanie lub odpowiedź
 *
 * \ingroup pubdir50
 */
void gg_pubdir50_free(gg_pubdir50_t s)
{
	int i;

	if (!s)
		return;
	
	for (i = 0; i < s->entries_count; i++) {
		free(s->entries[i].field);
		free(s->entries[i].value);
	}

	free(s->entries);
	free(s);
}

/**
 * Wysyła zapytanie katalogu publicznego do serwera.
 *
 * \param sess Struktura sesji
 * \param req Zapytanie
 *
 * \return Numer sekwencyjny zapytania lub 0 w przypadku błędu
 *
 * \ingroup pubdir50
 */
uint32_t gg_pubdir50(struct gg_session *sess, gg_pubdir50_t req)
{
	size_t size = 5;
	int i;
	uint32_t res;
	char *buf, *p;
	struct gg_pubdir50_request *r;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_pubdir50(%p, %p);\n", sess, req);
	
	if (!sess || !req) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_pubdir50() invalid arguments\n");
		errno = EFAULT;
		return 0;
	}

	if (sess->state != GG_STATE_CONNECTED) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_pubdir50() not connected\n");
		errno = ENOTCONN;
		return 0;
	}

	for (i = 0; i < req->entries_count; i++) {
		/* wyszukiwanie bierze tylko pierwszy wpis */
		if (req->entries[i].num)
			continue;
		
		if (sess->encoding == GG_ENCODING_CP1250) {
			size += strlen(req->entries[i].field) + 1;
			size += strlen(req->entries[i].value) + 1;
		} else {
			char *tmp;

			tmp = gg_utf8_to_cp(req->entries[i].field);

			if (tmp == NULL)
				return -1;

			size += strlen(tmp) + 1;

			free(tmp);

			tmp = gg_utf8_to_cp(req->entries[i].value);

			if (tmp == NULL)
				return -1;

			size += strlen(tmp) + 1;

			free(tmp);
		}
	}

	if (!(buf = (char*)malloc(size))) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_pubdir50() out of memory (%d bytes)\n", size);
		return 0;
	}

	if (!req->seq)
		req->seq = (uint32_t)time(NULL);

	res = req->seq;

	r = (struct gg_pubdir50_request*) buf;
	r->type = req->type;
	r->seq = gg_fix32(req->seq);

	for (i = 0, p = buf + 5; i < req->entries_count; i++) {
		if (req->entries[i].num)
			continue;

		if (sess->encoding == GG_ENCODING_CP1250) {
			strcpy(p, req->entries[i].field);
			p += strlen(p) + 1;

			strcpy(p, req->entries[i].value);
			p += strlen(p) + 1;
		} else {
			char *tmp;

			tmp = gg_utf8_to_cp(req->entries[i].field);

			if (tmp == NULL) {
				free(buf);
				return -1;
			}

			strcpy(p, tmp);
			p += strlen(tmp) + 1;
			free(tmp);

			tmp = gg_utf8_to_cp(req->entries[i].value);

			if (tmp == NULL) {
				free(buf);
				return -1;
			}

			strcpy(p, tmp);
			p += strlen(tmp) + 1;
			free(tmp);
		}
	}

	if (gg_send_packet(sess, GG_PUBDIR50_REQUEST, buf, size, NULL, 0) == -1)
		res = 0;

	free(buf);

	return res;
}

/*
 * \internal Analizuje przychodzący pakiet odpowiedzi i zapisuje wynik
 * w strukturze \c gg_event.
 *
 * \param sess Struktura sesji
 * \param e Struktura zdarzenia
 * \param packet Pakiet odpowiedzi
 * \param length Długość pakietu odpowiedzi
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_pubdir50_handle_reply_sess(struct gg_session *sess, struct gg_event *e, const char *packet, int length)
{
	const char *end = packet + length, *p;
	struct gg_pubdir50_reply *r = (struct gg_pubdir50_reply*) packet;
	gg_pubdir50_t res;
	int num = 0;
	
	gg_debug(GG_DEBUG_FUNCTION, "** gg_pubdir50_handle_reply_sess(%p, %p, %p, %d);\n", sess, e, packet, length);

	if (!sess || !e || !packet) {
		gg_debug(GG_DEBUG_MISC, "// gg_pubdir50_handle_reply() invalid arguments\n");
		errno = EFAULT;
		return -1;
	}

	if (length < 5) {
		gg_debug(GG_DEBUG_MISC, "// gg_pubdir50_handle_reply() packet too short\n");
		errno = EINVAL;
		return -1;
	}

	if (!(res = gg_pubdir50_new(r->type))) {
		gg_debug(GG_DEBUG_MISC, "// gg_pubdir50_handle_reply() unable to allocate reply\n");
		return -1;
	}

	e->event.pubdir50 = res;

	res->seq = gg_fix32(r->seq);

	switch (res->type) {
		case GG_PUBDIR50_READ:
			e->type = GG_EVENT_PUBDIR50_READ;
			break;

		case GG_PUBDIR50_WRITE:
			e->type = GG_EVENT_PUBDIR50_WRITE;
			break;

		default:
			e->type = GG_EVENT_PUBDIR50_SEARCH_REPLY;
			break;
	}

	/* brak wyników? */
	if (length == 5)
		return 0;

	/* pomiń początek odpowiedzi */
	p = packet + 5;

	while (p < end) {
		const char *field, *value;

		field = p;

		/* sprawdź, czy nie mamy podziału na kolejne pole */
		if (!*field) {
			num++;
			field++;
		}

		value = NULL;
		
		for (p = field; p < end; p++) {
			/* jeśli mamy koniec tekstu... */
			if (!*p) {
				/* ...i jeszcze nie mieliśmy wartości pola to
				 * wiemy, że po tym zerze jest wartość... */
				if (!value)
					value = p + 1;
				else
					/* ...w przeciwym wypadku koniec
					 * wartości i możemy wychodzić
					 * grzecznie z pętli */
					break;
			}
		}
		
		/* sprawdźmy, czy pole nie wychodzi poza pakiet, żeby nie
		 * mieć segfaultów, jeśli serwer przestanie zakańczać pakietów
		 * przez \0 */

		if (p == end) {
			gg_debug(GG_DEBUG_MISC, "// gg_pubdir50_handle_reply() premature end of packet\n");
			goto failure;
		}

		p++;

		/* jeśli dostaliśmy namier na następne wyniki, to znaczy że
		 * mamy koniec wyników i nie jest to kolejna osoba. */
		if (!strcasecmp(field, "nextstart")) {
			res->next = atoi(value);
			num--;
		} else {
			if (sess->encoding == GG_ENCODING_CP1250) {
				if (gg_pubdir50_add_n(res, num, field, value) == -1)
					goto failure;
			} else {
				char *tmp;

				tmp = gg_cp_to_utf8(value);

				if (tmp == NULL)
					goto failure;

				if (gg_pubdir50_add_n(res, num, field, tmp) == -1) {
					free(tmp);
					goto failure;
				}

				free(tmp);
			}
		}
	}	

	res->count = num + 1;
	
	return 0;

failure:
	gg_pubdir50_free(res);
	return -1;
}

/**
 * Pobiera pole z odpowiedzi katalogu publicznego.
 *
 * \param res Odpowiedź
 * \param num Numer wyniku odpowiedzi
 * \param field Nazwa pola (wielkość liter nie ma znaczenia)
 *
 * \return Wartość pola lub \c NULL jeśli nie znaleziono
 *
 * \ingroup pubdir50
 */
const char *gg_pubdir50_get(gg_pubdir50_t res, int num, const char *field)
{
	char *value = NULL;
	int i;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_pubdir50_get(%p, %d, \"%s\");\n", res, num, field);

	if (!res || num < 0 || !field) {
		gg_debug(GG_DEBUG_MISC, "// gg_pubdir50_get() invalid arguments\n");
		errno = EINVAL;
		return NULL;
	}

	for (i = 0; i < res->entries_count; i++) {
		if (res->entries[i].num == num && !strcasecmp(res->entries[i].field, field)) {
			value = res->entries[i].value;
			break;
		}
	}

	return value;
}

/**
 * Zwraca liczbę wyników odpowiedzi.
 *
 * \param res Odpowiedź
 *
 * \return Liczba wyników lub -1 w przypadku błędu
 *
 * \ingroup pubdir50
 */
int gg_pubdir50_count(gg_pubdir50_t res)
{
	return (!res) ? -1 : res->count;
}

/**
 * Zwraca rodzaj zapytania lub odpowiedzi.
 *
 * \param res Zapytanie lub odpowiedź
 *
 * \return Rodzaj lub -1 w przypadku błędu
 *
 * \ingroup pubdir50
 */
int gg_pubdir50_type(gg_pubdir50_t res)
{
	return (!res) ? -1 : res->type;
}

/**
 * Zwraca numer, od którego należy rozpocząc kolejne wyszukiwanie.
 *
 * Dłuższe odpowiedzi katalogu publicznego są wysyłane przez serwer
 * w mniejszych paczkach. Po otrzymaniu odpowiedzi, jeśli numer kolejnego
 * wyszukiwania jest większy od zera, dalsze wyniki można otrzymać przez
 * wywołanie kolejnego zapytania z określonym numerem początkowym.
 *
 * \param res Odpowiedź
 *
 * \return Numer lub -1 w przypadku błędu
 *
 * \ingroup pubdir50
 */
uin_t gg_pubdir50_next(gg_pubdir50_t res)
{
	return (!res) ? (unsigned) -1 : res->next;
}

/**
 * Zwraca numer sekwencyjny zapytania lub odpowiedzi.
 *
 * \param res Zapytanie lub odpowiedź
 *
 * \return Numer sekwencyjny lub -1 w przypadku błędu
 *
 * \ingroup pubdir50
 */
uint32_t gg_pubdir50_seq(gg_pubdir50_t res)
{
	return (!res) ? (unsigned) -1 : res->seq;
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

/*
 *  (C) Copyright 2001-2010 Wojtek Kaniewski <wojtekka@irc.pl>
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
 * \file message.c
 *
 * \brief Obsługa wiadomości
 * 
 * Plik zawiera funkcje dotyczące obsługi "klasy" gg_message_t, które
 * w przyszłości zostaną dołączone do API. Obecnie używane są funkcje
 * konwersji między tekstem z atrybutami i HTML.
 */

#ifndef _WIN64
#define _USE_32BIT_TIME_T
#endif

#ifdef _WIN32
#include "win32.h"
#endif /* _WIN32 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

#include "message.h"

#if 0

gg_message_t *gg_message_new(void)
{
	gg_message_t *gm;

	gm = malloc(sizeof(gg_message_t));

	if (gm == NULL)
		return NULL;

	memset(gm, 0, sizeof(gg_message_t));

	gm->msgclass = GG_CLASS_CHAT;
	gm->seq = (uint32_t) -1;

	return gm;
}

int gg_message_init(gg_message_t *gm, int msgclass, int seq, uin_t *recipients, size_t recipient_count, char *text, char *html, char *attributes, size_t attributes_length, int auto_convert)
{
	GG_MESSAGE_CHECK(gm, -1);

	memset(gm, 0, sizeof(gg_message_t));
	gm->recipients = recipients;
	gm->recipient_count = recipient_count;
	gm->text = text;
	gm->html = html;
	gm->attributes = attributes;
	gm->attributes_length = attributes_length;
	gm->msgclass = msgclass;
	gm->seq = seq;
	gm->auto_convert = auto_convert;

	return 0;
}

void gg_message_free(gg_message_t *gm)
{
	if (gm == NULL) {
		errno = EINVAL;
		return;
	}	

	free(gm->text);
	free(gm->text_converted);
	free(gm->html);
	free(gm->html_converted);
	free(gm->recipients);
	free(gm->attributes);

	free(gm);
}

int gg_message_set_auto_convert(gg_message_t *gm, int auto_convert)
{
	GG_MESSAGE_CHECK(gm, -1);

	gm->auto_convert = !!auto_convert;

	if (!gm->auto_convert) {
		free(gm->text_converted);
		free(gm->html_converted);
		gm->text_converted = NULL;
		gm->html_converted = NULL;
	}

	return 0;
}

int gg_message_get_auto_convert(gg_message_t *gm)
{
	GG_MESSAGE_CHECK(gm, -1);

	return gm->auto_convert;
}

int gg_message_set_recipients(gg_message_t *gm, const uin_t *recipients, size_t recipient_count)
{
	GG_MESSAGE_CHECK(gm, -1);

	if (recipient_count >= INT_MAX / sizeof(uin_t)) {
		errno = EINVAL;
		return -1;
	}	

	if ((recipients == NULL) || (recipient_count == 0)) {
		free(gm->recipients);
		gm->recipients = NULL;
		gm->recipient_count = 0;
	} else {
		uin_t *tmp;

		tmp = realloc(gm->recipients, recipient_count * sizeof(uin_t));

		if (tmp == NULL)
			return -1;

		memcpy(tmp, recipients, recipient_count * sizeof(uin_t));

		gm->recipients = tmp;
		gm->recipient_count = recipient_count;
	}
	
	return 0;
}

int gg_message_set_recipient(gg_message_t *gm, uin_t recipient)
{
	return gg_message_set_recipients(gm, &recipient, 1);
}

int gg_message_get_recipients(gg_message_t *gm, const uin_t **recipients, size_t *recipient_count)
{
	GG_MESSAGE_CHECK(gm, -1);

	if (recipients != NULL)
		*recipients = gm->recipients;

	if (recipient_count != NULL)
		*recipient_count = gm->recipient_count;

	return 0;
}

uin_t gg_message_get_recipient(gg_message_t *gm)
{
	GG_MESSAGE_CHECK(gm, (uin_t) -1);

	if ((gm->recipients == NULL) || (gm->recipient_count < 1)) {
		// errno = XXX;
		return (uin_t) -1;
	}

	return gm->recipients[0];
}

int gg_message_set_class(gg_message_t *gm, uint32_t msgclass)
{
	GG_MESSAGE_CHECK(gm, -1);

	gm->msgclass = msgclass;

	return 0;
}

uint32_t gg_message_get_class(gg_message_t *gm)
{
	GG_MESSAGE_CHECK(gm, (uint32_t) -1);

	return gm->msgclass;
}

int gg_message_set_seq(gg_message_t *gm, uint32_t seq)
{
	GG_MESSAGE_CHECK(gm, -1);

	gm->seq = seq;

	return 0;
}

uint32_t gg_message_get_seq(gg_message_t *gm)
{
	GG_MESSAGE_CHECK(gm, (uint32_t) -1);

	return gm->seq;
}

int gg_message_set_text(gg_message_t *gm, const char *text)
{
	GG_MESSAGE_CHECK(gm, -1);

	if (text == NULL) {
		free(gm->text);
		gm->text = NULL;
	} else {
		char *tmp;

		tmp = strdup(text);

		if (tmp == NULL)
			return -1;

		free(gm->text);
		gm->text = tmp;
	}

	free(gm->html_converted);
	gm->html_converted = NULL;

	return 0;
}

const char *gg_message_get_text(gg_message_t *gm)
{
	GG_MESSAGE_CHECK(gm, NULL);

	if (gm->text_converted != NULL)
		return gm->text_converted;

	if (gm->text == NULL && gm->html != NULL && gm->auto_convert) {
		size_t len;

		free(gm->text_converted);

		len = gg_message_html_to_text(NULL, gm->html);

		gm->text_converted = malloc(len + 1);

		if (gm->text_converted == NULL)
			return NULL;

		gg_message_html_to_text(gm->text_converted, gm->html);

		return gm->text_converted;
	}

	return gm->text;
}

int gg_message_set_html(gg_message_t *gm, const char *html)
{
	GG_MESSAGE_CHECK(gm, -1);

	if (html == NULL) {
		free(gm->html);
		gm->html = NULL;
	} else {
		char *tmp;

		tmp = strdup(html);

		if (tmp == NULL)
			return -1;

		free(gm->html);
		gm->html = tmp;
	}

	free(gm->text_converted);
	gm->text_converted = NULL;

	return 0;
}

const char *gg_message_get_html(gg_message_t *gm)
{
	GG_MESSAGE_CHECK(gm, NULL);

	if (gm->html_converted != NULL)
		return gm->html_converted;

	if (gm->html == NULL && gm->text != NULL && gm->auto_convert) {
		size_t len;

		free(gm->html_converted);

		len = gg_message_text_to_html(NULL, gm->text, gm->attributes, gm->attributes_length);

		gm->html_converted = malloc(len + 1);

		if (gm->html_converted == NULL)
			return NULL;

		gg_message_text_to_html(gm->html_converted, gm->text, gm->attributes, gm->attributes_length);

		return gm->html_converted;
	}

	return gm->html;
}

int gg_message_set_attributes(gg_message_t *gm, const char *attributes, size_t length)
{
	GG_MESSAGE_CHECK(gm, -1);

	if (length > 0xfffd) {
		// errno = XXX;
		return -1;
	}

	if ((attributes == NULL) || (length == 0)) {
		free(gm->attributes);
		gm->attributes = NULL;
		gm->attributes_length = 0;
	} else {
		char *tmp;

		tmp = realloc(gm->attributes, length);

		if (tmp == NULL)
			return -1;

		gm->attributes = tmp;
		gm->attributes_length = length;
	}

	free(gm->html_converted);
	gm->html_converted = NULL;

	return 0;
}

int gg_message_get_attributes(gg_message_t *gm, const char **attributes, size_t *attributes_length)
{
	GG_MESSAGE_CHECK(gm, -1);

	if (attributes != NULL)
		*attributes = gm->attributes;

	if (attributes_length != NULL)
		*attributes_length = gm->attributes_length;

	return 0;
}

#endif

/**
 * \internal Dodaje tekst na koniec bufora.
 * 
 * \param dst Wskaźnik na bufor roboczy
 * \param pos Wskaźnik na aktualne położenie w buforze roboczym
 * \param src Dodawany tekst
 * \param len Długość dodawanego tekstu
 */
static void gg_append(char *dst, size_t *pos, const void *src, int len)
{
	if (dst != NULL)
		memcpy(&dst[*pos], src, len);

	*pos += len;
}

/**
 * \internal Zamienia tekst z formatowaniem Gadu-Gadu na HTML.
 *
 * \param dst Bufor wynikowy (może być \c NULL)
 * \param src Tekst źródłowy w UTF-8
 * \param format Atrybuty tekstu źródłowego
 * \param format_len Długość bloku atrybutów tekstu źródłowego
 *
 * \note Wynikowy tekst nie jest idealnym kodem HTML, ponieważ ma jak
 * dokładniej odzwierciedlać to, co wygenerowałby oryginalny klient.
 *
 * \note Dokleja \c \\0 na końcu bufora wynikowego.
 *
 * \return Długość tekstu wynikowego bez \c \\0 (nawet jeśli \c dst to \c NULL).
 */
size_t gg_message_text_to_html(char *dst, const char *src, const char *format, size_t format_len)
{
	const char span_fmt[] = "<span style=\"color:#%02x%02x%02x; font-family:'MS Shell Dlg 2'; font-size:9pt; \">";
	const int span_len = 75;
	const char img_fmt[] = "<img name=\"%02x%02x%02x%02x%02x%02x%02x%02x\">";
	const int img_len = 29;
	int char_pos = 0;
	int format_idx = 0;
	unsigned char old_attr = 0;
	const unsigned char *color = (const unsigned char*) "\x00\x00\x00";
	int i;
	size_t len;
	const unsigned char *format_ = (const unsigned char*) format;

	len = 0;

	/* Nie mamy atrybutów dla pierwsze znaku, a tekst nie jest pusty, więc
	 * tak czy inaczej trzeba otworzyć <span>. */

	if (src[0] != 0 && (format_idx + 3 > format_len || (format_[format_idx] | (format_[format_idx + 1] << 8)) != 0)) {
		if (dst != NULL)
			sprintf(&dst[len], span_fmt, 0, 0, 0);

		len += span_len;
	}

	/* Pętla przechodzi też przez kończące \0, żeby móc dokleić obrazek
	 * na końcu tekstu. */

	for (i = 0; ; i++) {
		/* Analizuj atrybuty tak długo jak dotyczą aktualnego znaku. */
		for (;;) {
			unsigned char attr;
			int attr_pos;

			if (format_idx + 3 > format_len)
				break;

			attr_pos = format_[format_idx] | (format_[format_idx + 1] << 8);

			if (attr_pos != char_pos)
				break;

			attr = format_[format_idx + 2];

			/* Nie doklejaj atrybutów na końcu, co najwyżej obrazki. */

			if (src[i] == 0)
				attr &= ~(GG_FONT_BOLD | GG_FONT_ITALIC | GG_FONT_UNDERLINE | GG_FONT_COLOR);

			format_idx += 3;

			if ((attr & (GG_FONT_BOLD | GG_FONT_ITALIC | GG_FONT_UNDERLINE | GG_FONT_COLOR)) != 0 || (attr == 0 && old_attr != 0)) {
				if (char_pos != 0) {
					if ((old_attr & GG_FONT_UNDERLINE) != 0)
						gg_append(dst, &len, "</u>", 4);

					if ((old_attr & GG_FONT_ITALIC) != 0)
						gg_append(dst, &len, "</i>", 4);

					if ((old_attr & GG_FONT_BOLD) != 0)
						gg_append(dst, &len, "</b>", 4);

					if (src[i] != 0)
						gg_append(dst, &len, "</span>", 7);
				}

				if (((attr & GG_FONT_COLOR) != 0) && (format_idx + 3 <= format_len)) {
					color = &format_[format_idx];
					format_idx += 3;
				} else {
					color = (unsigned char*) "\x00\x00\x00";
				}

				if (src[i] != 0) {
					if (dst != NULL)
						sprintf(&dst[len], span_fmt, color[0], color[1], color[2]);
					len += span_len;
				}
			} else if (char_pos == 0 && src[0] != 0) {
				if (dst != NULL)
					sprintf(&dst[len], span_fmt, 0, 0, 0);
				len += span_len;
			}

			if ((attr & GG_FONT_BOLD) != 0)
				gg_append(dst, &len, "<b>", 3);

			if ((attr & GG_FONT_ITALIC) != 0)
				gg_append(dst, &len, "<i>", 3);

			if ((attr & GG_FONT_UNDERLINE) != 0)
				gg_append(dst, &len, "<u>", 3);

			if (((attr & GG_FONT_IMAGE) != 0) && (format_idx + 10 <= format_len)) {
				if (dst != NULL) {
					sprintf(&dst[len], img_fmt,
						format_[format_idx + 9],
						format_[format_idx + 8], 
						format_[format_idx + 7],
						format_[format_idx + 6], 
						format_[format_idx + 5],
						format_[format_idx + 4],
						format_[format_idx + 3],
						format_[format_idx + 2]);
				}

				len += img_len;
				format_idx += 10;
			}

			old_attr = attr;
		}

		/* Doklej znak zachowując htmlowe escapowanie. */

		switch (src[i]) {
			case '&':
				gg_append(dst, &len, "&amp;", 5);
				break;
			case '<':
				gg_append(dst, &len, "&lt;", 4);
				break;
			case '>':
				gg_append(dst, &len, "&gt;", 4);
				break;
			case '\'':
				gg_append(dst, &len, "&apos;", 6);
				break;
			case '\"':
				gg_append(dst, &len, "&quot;", 6);
				break;
			case '\n':
				gg_append(dst, &len, "<br>", 4);
				break;
			case '\r':
			case 0:
				break;
			default:
				if (dst != NULL)
					dst[len] = src[i];
				len++;
		}

		/* Sprawdź, czy bajt nie jest kontynuacją znaku unikodowego. */

		if ((src[i] & 0xc0) != 0xc0)
			char_pos++;

		if (src[i] == 0)
			break;
	}

	/* Zamknij tagi. */

	if ((old_attr & GG_FONT_UNDERLINE) != 0)
		gg_append(dst, &len, "</u>", 4);

	if ((old_attr & GG_FONT_ITALIC) != 0)
		gg_append(dst, &len, "</i>", 4);

	if ((old_attr & GG_FONT_BOLD) != 0)
		gg_append(dst, &len, "</b>", 4);

	if (src[0] != 0)
		gg_append(dst, &len, "</span>", 7);

	if (dst != NULL)
		dst[len] = 0;

	return len;
}

/**
 * \internal Zamienia tekst w formacie HTML na czysty tekst.
 *
 * \param dst Bufor wynikowy (może być \c NULL)
 * \param html Tekst źródłowy
 *
 * \note Dokleja \c \\0 na końcu bufora wynikowego.
 *
 * \note Funkcja służy do zachowania kompatybilności przy przesyłaniu
 * wiadomości HTML do klientów, które tego formatu nie obsługują. Z tego
 * powodu funkcja nie zachowuje formatowania, a jedynie usuwa tagi i
 * zamienia podstawowe encje na ich odpowiedniki ASCII.
 *
 * \return Długość tekstu wynikowego bez \c \\0 (nawet jeśli \c dst to \c NULL).
 */
size_t gg_message_html_to_text(char *dst, const char *html)
{
	const char *src, *entity, *tag;
	int in_tag, in_entity;
	size_t len;

	len = 0;
	in_tag = 0;
	tag = NULL;
	in_entity = 0;
	entity = NULL;

	for (src = html; *src != 0; src++) {
		if (in_entity && !(isalnum(*src) || *src == '#' || *src == ';')) {
			in_entity = 0;
			gg_append(dst, &len, entity, src - entity);
		}

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
					dst[len++] = '<';
				else if (strncmp(entity, "&gt;", 4) == 0)
					dst[len++] = '>';
				else if (strncmp(entity, "&quot;", 6) == 0)
					dst[len++] = '"';
				else if (strncmp(entity, "&apos;", 6) == 0)
					dst[len++] = '\'';
				else if (strncmp(entity, "&amp;", 5) == 0)
					dst[len++] = '&';
				else if (strncmp(entity, "&nbsp;", 6) == 0) {
					dst[len++] = 0xc2;
					dst[len++] = 0xa0;
				} else
					dst[len++] = '?';
			} else {
				if (strncmp(entity, "&nbsp;", 6) == 0)
					len += 2;
				else
					len++;
			}

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

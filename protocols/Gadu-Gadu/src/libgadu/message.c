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

#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>

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

int gg_message_init(gg_message_t *gm, int msgclass, int seq, uin_t *recipients,
	size_t recipient_count, char *text, char *html, char *attributes,
	size_t attributes_length, int auto_convert)
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
		/* errno = XXX; */
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

		len = gg_message_text_to_html(NULL, gm->text, GG_ENCODING_UTF8, gm->attributes, gm->attributes_length);

		gm->html_converted = malloc(len + 1);

		if (gm->html_converted == NULL)
			return NULL;

		gg_message_text_to_html(gm->html_converted, gm->text,
			GG_ENCODING_UTF8, gm->attributes, gm->attributes_length);

		return gm->html_converted;
	}

	return gm->html;
}

int gg_message_set_attributes(gg_message_t *gm, const char *attributes, size_t length)
{
	GG_MESSAGE_CHECK(gm, -1);

	if (length > 0xfffd) {
		/* errno = XXX; */
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
static void gg_append(char *dst, size_t *pos, const void *src, size_t len)
{
	if (dst != NULL)
		memcpy(&dst[*pos], src, len);

	*pos += len;
}

/**
 * \internal Zamienia tekst z formatowaniem Gadu-Gadu na HTML.
 *
 * \param dst Bufor wynikowy (może być \c NULL)
 * \param src Tekst źródłowy
 * \param encoding Kodowanie tekstu źródłowego oraz wynikowego
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
size_t gg_message_text_to_html(char *dst, const char *src,
	gg_encoding_t encoding, const unsigned char *format, size_t format_len)
{
	const char span_fmt[] = "<span style=\"color:#%02x%02x%02x; font-family:'MS Shell Dlg 2'; font-size:9pt; \">";
	const size_t span_len = 75;
	const char img_fmt[] = "<img name=\"%02x%02x%02x%02x%02x%02x%02x%02x\">";
	const size_t img_len = 29;
	size_t char_pos = 0;
	unsigned char old_attr = 0;
	const unsigned char default_color[] = {'\x00', '\x00', '\x00'};
	const unsigned char *old_color = NULL;
	int in_span = 0;
	unsigned int i;
	size_t len = 0;

	if (format == NULL)
		format_len = 0;

	/* Pętla przechodzi też przez kończące \0, żeby móc dokleić obrazek
	 * na końcu tekstu. */

	for (i = 0; ; i++) {
		int in_char = 0;
		size_t format_idx = 0;

		/* Sprawdź, czy bajt jest kontynuacją znaku UTF-8. */
		if (encoding == GG_ENCODING_UTF8 && (src[i] & 0xc0) == 0x80)
			in_char = 1;

		/* GG_FONT_IMAGE powinno dotyczyć tylko jednego znaku, więc czyścimy stary atrybut */

		if (!in_char && (old_attr & GG_FONT_IMAGE) != 0)
			old_attr &= ~GG_FONT_IMAGE;

		/* Analizuj wszystkie atrybuty dotyczące aktualnego znaku. */
		for (;;) {
			unsigned char attr;
			size_t attr_pos;

			/* Nie wstawiamy niczego wewnątrz wielobajtowego znaku UTF-8. */
			if (in_char)
				break;

			if (format_idx + 3 > format_len)
				break;

			/* (format_idx + 3 <= format_len) && (format_idx > 0)
			 * 3 < format_len
			 * 0 != format_len
			 * format != NULL
			 */
			assert(format != NULL);

			attr_pos = format[format_idx] | (format[format_idx + 1] << 8);
			attr = format[format_idx + 2];

			/* Nie doklejaj atrybutów na końcu, co najwyżej obrazki. */

			if (src[i] == 0)
				attr &= ~(GG_FONT_BOLD | GG_FONT_ITALIC | GG_FONT_UNDERLINE | GG_FONT_COLOR);

			format_idx += 3;

			if (attr_pos != char_pos) {
				if ((attr & GG_FONT_COLOR) != 0)
					format_idx += 3;
				if ((attr & GG_FONT_IMAGE) != 0)
					format_idx += 10;

				continue;
			}

			if ((old_attr & GG_FONT_UNDERLINE) != 0)
				gg_append(dst, &len, "</u>", 4);

			if ((old_attr & GG_FONT_ITALIC) != 0)
				gg_append(dst, &len, "</i>", 4);

			if ((old_attr & GG_FONT_BOLD) != 0)
				gg_append(dst, &len, "</b>", 4);

			if ((attr & (GG_FONT_BOLD | GG_FONT_ITALIC | GG_FONT_UNDERLINE | GG_FONT_COLOR)) != 0) {
				const unsigned char *color;

				if (((attr & GG_FONT_COLOR) != 0) && (format_idx + 3 <= format_len)) {
					color = &format[format_idx];
					format_idx += 3;
				} else {
					color = default_color;
				}

				if (old_color == NULL || memcmp(color, old_color, 3) != 0) {
					if (in_span) {
						gg_append(dst, &len, "</span>", 7);
						in_span = 0;
					}

					if (src[i] != 0) {
						if (dst != NULL)
							sprintf(&dst[len], span_fmt, color[0], color[1], color[2]);

						len += span_len;
						in_span = 1;
						old_color = color;
					}
				}
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
						format[format_idx + 9],
						format[format_idx + 8],
						format[format_idx + 7],
						format[format_idx + 6],
						format[format_idx + 5],
						format[format_idx + 4],
						format[format_idx + 3],
						format[format_idx + 2]);
				}

				len += img_len;
				format_idx += 10;
			}

			old_attr = attr;
		}

		if (src[i] == 0)
			break;

		/* Znaki oznaczone jako GG_FONT_IMAGE nie są częścią wiadomości. */

		if ((old_attr & GG_FONT_IMAGE) != 0) {
			if (!in_char)
				char_pos++;

			continue;
		}

		/* Jesteśmy na początku tekstu i choć nie było atrybutów dla pierwszego
		* znaku, ponieważ tekst nie jest pusty, trzeba otworzyć <span>. */

		if (!in_span) {
			if (dst != NULL)
				sprintf(&dst[len], span_fmt, default_color[0], default_color[1], default_color[2]);

			len += span_len;
			in_span = 1;
			old_color = default_color;
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
				break;
			default:
				if (dst != NULL)
					dst[len] = src[i];
				len++;
		}

		if (!in_char)
			char_pos++;
	}

	/* Zamknij tagi. */

	if ((old_attr & GG_FONT_UNDERLINE) != 0)
		gg_append(dst, &len, "</u>", 4);

	if ((old_attr & GG_FONT_ITALIC) != 0)
		gg_append(dst, &len, "</i>", 4);

	if ((old_attr & GG_FONT_BOLD) != 0)
		gg_append(dst, &len, "</b>", 4);

	if (in_span)
		gg_append(dst, &len, "</span>", 7);

	if (dst != NULL)
		dst[len] = 0;

	return len;
}

/**
 * \internal Dokleja nowe atrybuty formatowania, jeśli konieczne, oraz inkrementuje pozycję znaku w tekście.
 *
 * \param pos Wskaźnik na zmienną przechowującą pozycję znaku w tekście
 * \param attr_flag Aktualna flaga atrybutu formatowania
 * \param old_attr_flag Wskaźnik na poprzednią flagę atrybutu formatowania
 * \param color Wskaźnik na tablicę z aktualnym kolorem RGB (jeśli \p attr_flag
 *        nie zawiera flagi \c GG_FONT_COLOR, ignorowane)
 * \param old_color Wskaźnik na tablicę z poprzednim kolorem RGB
 * \param imgs_size Rozmiar atrybutów formatowania obrazków znajdujących się
 *        obecnie w tablicy atrybutów formatowania, w bajtach
 * \param format Wskaźnik na wskaźnik do tablicy atrybutów formatowania
 * \param format_len Wskaźnik na zmienną zawierającą długość tablicy atrybutów
 *        formatowania, w bajtach (może być \c NULL)
 */
static void gg_after_append_formatted_char(uint16_t *pos,
	unsigned char attr_flag, unsigned char *old_attr_flag,
	const unsigned char *color, unsigned char *old_color, size_t imgs_size,
	unsigned char **format, size_t *format_len)
{
	const size_t color_size = 3;
	int has_color = 0;

	if ((attr_flag & GG_FONT_COLOR) != 0)
		has_color = 1;

	if (*old_attr_flag != attr_flag || (has_color && memcmp(old_color, color, color_size) != 0)) {
		size_t attr_size = sizeof(*pos) + sizeof(attr_flag) + (has_color ? color_size : 0);

		if (*format != NULL) {
			/* Staramy się naśladować oryginalnego klienta i atrybuty obrazków trzymamy na końcu */

			*format -= imgs_size;
			memmove(*format + attr_size, *format, imgs_size);

			**format = (unsigned char) (*pos & (uint16_t) 0x00ffU);
			*format += 1;
			**format = (unsigned char) ((*pos & (uint16_t) 0xff00U) >> 8);
			*format += 1;

			**format = attr_flag;
			*format += 1;

			if (has_color) {
				memcpy(*format, color, color_size);
				*format += color_size;
			}

			*format += imgs_size;
		}

		if (format_len != NULL)
			*format_len += attr_size;

		*old_attr_flag = attr_flag;
		if (has_color)
			memcpy(old_color, color, color_size);
	}

	*pos += 1;
}

/**
 * \internal Zamienia tekst w formacie HTML na czysty tekst.
 *
 * \param dst Bufor wynikowy (może być \c NULL)
 * \param format Bufor wynikowy z atrybutami formatowania (może być \c NULL)
 * \param format_len Wskaźnik na zmienną, do której zostanie zapisana potrzebna
 *                   wielkość bufora wynikowego z atrybutami formatowania,
 *                   w bajtach (może być \c NULL)
 * \param html Tekst źródłowy
 * \param encoding Kodowanie tekstu źródłowego oraz wynikowego
 *
 * \note Dokleja \c \\0 na końcu bufora wynikowego.
 *
 * \return Długość bufora wynikowego bez \c \\0 (nawet jeśli \c dst to \c NULL).
 */
size_t gg_message_html_to_text(char *dst, unsigned char *format,
	size_t *format_len, const char *html, gg_encoding_t encoding)
{
	const char *src, *entity = NULL, *tag = NULL;
	int in_tag = 0, in_entity = 0, in_bold = 0, in_italic = 0, in_underline = 0;
	unsigned char color[3] = { 0 }, old_color[3] = { 0 };
	unsigned char attr_flag = 0, old_attr_flag = 0;
	uint16_t pos = 0;
	size_t len = 0, imgs_size = 0;

	if (format_len != NULL)
		*format_len = 0;

	for (src = html; *src != 0; src++) {
		if (in_entity && !(isalnum(*src) || *src == '#' || *src == ';')) {
			int first = 1;
			size_t i, append_len = src - entity;

			gg_append(dst, &len, entity, append_len);
			for (i = 0; i < append_len; i++) {
				if (encoding != GG_ENCODING_UTF8 || (entity[i] & 0xc0) != 0x80) {
					if (first) {
						gg_after_append_formatted_char(&pos, attr_flag,
							&old_attr_flag, color, old_color, imgs_size,
							&format, format_len);
						first = 0;
					} else {
						pos++;
					}
				}
			}

			in_entity = 0;
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

				gg_after_append_formatted_char(&pos, attr_flag,
					&old_attr_flag, color, old_color,
					imgs_size, &format, format_len);
			} else if (strncmp(tag, "<img name=\"", 11) == 0 || strncmp(tag, "<img name=\'", 11) == 0) {
				tag += 11;

				/* 17 bo jeszcze cudzysłów musi być zamknięty */
				if (tag + 17 <= src) {
					int i, ok = 1;

					for (i = 0; i < 16; i++) {
						if (!isxdigit(tag[i])) {
							ok = 0;
							break;
						}
					}

					if (ok) {
						unsigned char img_attr[13];

						if (format != NULL) {
							char buf[3] = { 0 };

							img_attr[0] = (unsigned char) (pos & (uint16_t) 0x00ffU);
							img_attr[1] = (unsigned char) ((pos & (uint16_t) 0xff00U) >> 8);
							img_attr[2] = GG_FONT_IMAGE;
							img_attr[3] = '\x09';
							img_attr[4] = '\x01';
							for (i = 0; i < 16; i += 2) {
								buf[0] = tag[i];
								buf[1] = tag[i + 1];
								/* buf[2] to '\0' */
								img_attr[12 - i / 2] =
									(unsigned char)strtoul(buf, NULL, 16);
							}

							memcpy(format, img_attr, sizeof(img_attr));
							format += sizeof(img_attr);
						}

						if (format_len != NULL)
							*format_len += sizeof(img_attr);
						imgs_size += sizeof(img_attr);

						if (dst != NULL) {
							if (encoding == GG_ENCODING_UTF8)
								dst[len++] = '\xc2';
							dst[len++] = '\xa0';
						} else {
							len += 2;
						}

						/* Nie używamy tutaj gg_after_append_formatted_char().
						 * Po pierwsze to praktycznie niczego by nie
						 * zmieniło, a po drugie nie wszystkim klientom
						 * mogłaby się spodobać redefinicja atrybutów
						 * formatowania dla jednego znaku (bo np. najpierw
						 * byśmy zdefiniowali bolda od znaku 10, a potem
						 * by się okazało, że znak 10 to obrazek).
						 */

						pos++;

						/* Resetujemy atrybuty, aby je w razie czego
						 * redefiniować od następnego znaku, co by sobie
						 * nikt przypadkiem nie pomyślał, że GG_FONT_IMAGE
						 * dotyczy więcej, niż jednego znaku.
						 * Tak samo robi oryginalny klient.
						 */

						old_attr_flag = -1;
					}
				}
			} else if (strncmp(tag, "<b>", 3) == 0) {
				in_bold++;
				attr_flag |= GG_FONT_BOLD;
			} else if (strncmp(tag, "</b>", 4) == 0) {
				if (in_bold > 0) {
					in_bold--;
					if (in_bold == 0)
						attr_flag &= ~GG_FONT_BOLD;
				}
			} else if (strncmp(tag, "<i>", 3) == 0) {
				in_italic++;
				attr_flag |= GG_FONT_ITALIC;
			} else if (strncmp(tag, "</i>", 4) == 0) {
				if (in_italic > 0) {
					in_italic--;
					if (in_italic == 0)
						attr_flag &= ~GG_FONT_ITALIC;
				}
			} else if (strncmp(tag, "<u>", 3) == 0) {
				in_underline++;
				attr_flag |= GG_FONT_UNDERLINE;
			} else if (strncmp(tag, "</u>", 4) == 0) {
				if (in_underline > 0) {
					in_underline--;
					if (in_underline == 0)
						attr_flag &= ~GG_FONT_UNDERLINE;
				}
			} else if (strncmp(tag, "<span ", 6) == 0) {
				for (tag += 6; tag < src - 8; tag++) {
					if (*tag == '\"' || *tag == '\'' || *tag == ' ') {
						if (strncmp(tag + 1, "color:#", 7) == 0) {
							int i, ok = 1;
							char buf[3] = { 0 };

							tag += 8;
							if (tag + 6 > src)
								break;

							for (i = 0; i < 6; i++) {
								if (!isxdigit(tag[i])) {
									ok = 0;
									break;
								}
							}

							if (!ok)
								break;

							for (i = 0; i < 6; i += 2) {
								buf[0] = tag[i];
								buf[1] = tag[i + 1];
								/* buf[2] to '\0' */
								color[i / 2] = (unsigned char) strtoul(buf, NULL, 16);
							}

							attr_flag |= GG_FONT_COLOR;
						}
					}
				}
			} else if (strncmp(tag, "</span", 6) == 0) {
				/* Można by trzymać kolory na stosie i tutaj
				 * przywracać poprzedni, ale to raczej zbędne */

				attr_flag &= ~GG_FONT_COLOR;
			}

			tag = NULL;
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
					if (encoding == GG_ENCODING_UTF8)
						dst[len++] = '\xc2';
					dst[len++] = '\xa0';
				} else
					dst[len++] = '?';
			} else {
				if (strncmp(entity, "&nbsp;", 6) == 0)
					len += 2;
				else
					len++;
			}

			gg_after_append_formatted_char(&pos, attr_flag,
				&old_attr_flag, color, old_color, imgs_size,
				&format, format_len);

			continue;
		}

		if (in_entity && !(isalnum(*src) || *src == '#'))
			in_entity = 0;

		if (in_entity)
			continue;

		if (dst != NULL)
			dst[len] = *src;
		len++;

		if (encoding != GG_ENCODING_UTF8 || (*src & 0xc0) != 0x80) {
			gg_after_append_formatted_char(&pos, attr_flag,
				&old_attr_flag, color, old_color, imgs_size,
				&format, format_len);
		}
	}

	if (dst != NULL)
		dst[len] = '\0';

	return len;
}

static size_t gg_message_html_to_text_110_buff(char *dst, const char *html)
{
	return gg_message_html_to_text(dst, NULL, NULL, html, GG_ENCODING_UTF8);
}

static size_t gg_message_text_to_html_110_buff(char *dst, const char *text,
	ssize_t text_len)
{
	size_t i, dst_len;

	if (text_len == -1)
		text_len = strlen(text);
	dst_len = 0;

	gg_append(dst, &dst_len, "<span>", 6);

	for (i = 0; i < (size_t)text_len; i++) {
		char c = text[i];
		if (c == '<')
			gg_append(dst, &dst_len, "&lt;", 4);
		else if (c == '>')
			gg_append(dst, &dst_len, "&gt;", 4);
		else if (c == '&')
			gg_append(dst, &dst_len, "&amp;", 5);
		else if (c == '"')
			gg_append(dst, &dst_len, "&quot;", 6);
		else if (c == '\'')
			gg_append(dst, &dst_len, "&apos;", 6);
		else if (c == '\n')
			gg_append(dst, &dst_len, "<br>", 4);
		else if (c == '\r')
			continue;
		else if (c == '\xc2' && text[i + 1] == '\xa0') {
			gg_append(dst, &dst_len, "&nbsp;", 6);
			i++;
		} else {
			if (dst)
				dst[dst_len] = c;
			dst_len++;
		}
	}

	gg_append(dst, &dst_len, "</span>", 7);

	if (dst)
		dst[dst_len] = '\0';

	return dst_len;
}

char *gg_message_html_to_text_110(const char *html)
{
	size_t dst_len;
	char *dst;

	dst_len = gg_message_html_to_text_110_buff(NULL, html) + 1;
	dst = malloc(dst_len);
	if (!dst)
		return NULL;
	gg_message_html_to_text_110_buff(dst, html);

	return dst;
}

char *gg_message_text_to_html_110(const char *text, ssize_t text_len)
{
	size_t dst_len;
	char *dst;

	dst_len = gg_message_text_to_html_110_buff(NULL, text, text_len) + 1;
	dst = malloc(dst_len);
	if (!dst)
		return NULL;
	gg_message_text_to_html_110_buff(dst, text, text_len);

	return dst;
}

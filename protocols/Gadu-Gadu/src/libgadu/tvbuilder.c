/*
 *  (C) Copyright 2012 Tomek Wasilczyk <www.wasilczyk.pl>
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
 * \file tvbuilder.c
 *
 * \brief Bufor wspierający budowanie pakietów typu Type-Value(s)
 */

#include "internal.h"

#include <stdlib.h>
#include <string.h>

#include "tvbuilder.h"

#include "fileio.h"

#include <errno.h>

struct gg_tvbuilder
{
	char *buffer;
	size_t length;
	size_t alloc_length;
	int valid;

	struct gg_session *gs;
	struct gg_event *ge;
};

static char *gg_tvbuilder_extend(gg_tvbuilder_t *tvb, size_t length);

/**
 * \internal Tworzy nową instancję bufora.
 *
 * \param gs Struktura sesji
 * \param ge Struktura zdarzenia
 *
 * \return Zaalokowany bufor - musi być zwolniony przez gg_tvbuilder_free,
 *         gg_tvbuilder_fail lub gg_tvbuilder_send.
 */
gg_tvbuilder_t *gg_tvbuilder_new(struct gg_session *gs, struct gg_event *ge)
{
	gg_tvbuilder_t *tvb;

	tvb = malloc(sizeof(gg_tvbuilder_t));
	if (tvb == NULL)
		return NULL;
	memset(tvb, 0, sizeof(gg_tvbuilder_t));

	if (gs == NULL) {
		gg_debug(GG_DEBUG_ERROR, "// gg_tvbuilder_new() "
			"invalid arguments\n");
		tvb->valid = 0;
		return tvb;
	}

	tvb->buffer = NULL;
	tvb->length = 0;
	tvb->alloc_length = 0;
	tvb->valid = 1;

	tvb->gs = gs;
	tvb->ge = ge;

	return tvb;
}

/**
 * \internal Zwalnia bufor.
 *
 * \param tvb Bufor
 */
void gg_tvbuilder_free(gg_tvbuilder_t *tvb)
{
	if (tvb == NULL)
		return;

	free(tvb->buffer);
	free(tvb);
}

/**
 * \internal Zwalnia bufor i generuje błąd połączenia.
 *
 * \param tvb     Bufor
 * \param failure Powód błędu
 */
void gg_tvbuilder_fail(gg_tvbuilder_t *tvb, enum gg_failure_t failure)
{
	int errno_copy;

	if (tvb == NULL) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuilder_fail() "
			"NULL tvbuilder\n");
		return;
	}

	errno_copy = errno;
	close(tvb->gs->fd);
	tvb->gs->fd = -1;
	errno = errno_copy;

	if (tvb->ge) {
		tvb->ge->type = GG_EVENT_CONN_FAILED;
		tvb->ge->event.failure = failure;
	}
	tvb->gs->state = GG_STATE_IDLE;

	gg_tvbuilder_free(tvb);
}

/**
 * \internal Próbuje wysłać zawartość bufora i go zwalnia.
 *
 * \param tvb  Bufor
 * \param type Typ pakietu
 *
 * \return 1 jeśli się powiodło, 0 w p.p.
 */
int gg_tvbuilder_send(gg_tvbuilder_t *tvb, int type)
{
	int ret;
	enum gg_failure_t failure;

	if (tvb == NULL) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuilder_send() "
			"NULL tvbuilder\n");
		return 0;
	}

	if (!gg_tvbuilder_is_valid(tvb)) {
		gg_debug_session(tvb->gs, GG_DEBUG_ERROR, "// gg_tvbuilder_send() "
			"invalid buffer\n");
		ret = -1;
		failure = GG_FAILURE_INTERNAL;
	} else {
		const char *buffer = (tvb->length > 0) ? tvb->buffer : "";
		ret = gg_send_packet(tvb->gs, type, buffer, tvb->length, NULL);
		if (ret == -1) {
			failure = GG_FAILURE_WRITING;
			gg_debug_session(tvb->gs, GG_DEBUG_ERROR,
				"// gg_tvbuilder_send() "
				"sending packet %#x failed. (errno=%d, %s)\n",
				type, errno, strerror(errno));
		}
	}

	if (ret == -1) {
		gg_tvbuilder_fail(tvb, failure);
		return 0;
	}

	gg_tvbuilder_free(tvb);
	return 1;
}

/**
 * \internal Sprawdza, czy wszystkie zapisy do  bufora były prawidłowe.
 *
 * \param tvb Builder.
 *
 * \return Wartość różna od 0, jeżeli wszystkie zapisy były prawidłowe.
 */
int gg_tvbuilder_is_valid(const gg_tvbuilder_t *tvb)
{
	if (tvb == NULL)
		return 0;
	return tvb->valid;
}

/**
 * \internal Sprawdza rozmiar bufora.
 *
 * \param tvb Bufor
 *
 * \return Rozmiar bufora
 */
size_t gg_tvbuilder_get_size(const gg_tvbuilder_t *tvb)
{
	if (!gg_tvbuilder_is_valid(tvb))
		return 0;

	return tvb->length;
}

/**
 * \internal Określa oczekiwaną liczbę bajtów, o którą zostanie rozszerzony
 * bufor.
 *
 * Funkcja powoduje jedynie wzrost wydajności poprzez zmniejszenie ilości
 * realokacji.
 *
 * \param tvb    Builder.
 * \param length Oczekiwana liczba bajtów.
 */
void gg_tvbuilder_expected_size(gg_tvbuilder_t *tvb, size_t length)
{
	size_t length_new;
	char *buff_new;

	if (!gg_tvbuilder_is_valid(tvb) || length == 0)
		return;

	length_new = tvb->length + length;

	if (length_new <= tvb->alloc_length)
		return;

	if (tvb->alloc_length > 0) {
		gg_debug(GG_DEBUG_MISC, "// gg_tvbuilder_expected_size(%p, %"
			GG_SIZE_FMT ") realloc from %" GG_SIZE_FMT " to %"
			GG_SIZE_FMT "\n",
			tvb, length, tvb->alloc_length, length_new);
	}

	buff_new = realloc(tvb->buffer, length_new);
	if (buff_new != NULL) {
		tvb->buffer = buff_new;
		tvb->alloc_length = length_new;
		return;
	}

	gg_debug(GG_DEBUG_ERROR, "// gg_tvbuilder_expected_size(%p, %"
		GG_SIZE_FMT ") out of memory (new length: %" GG_SIZE_FMT
		")\n", tvb, length, length_new);
	free(tvb->buffer);
	tvb->buffer = NULL;
	tvb->length = 0;
	tvb->alloc_length = 0;
	tvb->valid = 0;
}

/**
 * \internal Poszerza bufor o podaną liczbę bajtów.
 *
 * \param tvb    Bufor
 * \param length Liczba bajtów do dodania
 *
 * \return Początek nowo dodanego bloku bufora
 */
static char * gg_tvbuilder_extend(gg_tvbuilder_t *tvb, size_t length)
{
	size_t length_old;

	gg_tvbuilder_expected_size(tvb, length);
	if (!gg_tvbuilder_is_valid(tvb))
		return NULL;

	length_old = tvb->length;
	tvb->length += length;

	return tvb->buffer + length_old;
}

/**
 * \internal Skraca bufor o podaną liczbę bajtów
 *
 * \param tvb Bufor
 * \param length Ilość bajtów do skrócenia
 */
void gg_tvbuilder_strip(gg_tvbuilder_t *tvb, size_t length)
{
	if (!gg_tvbuilder_is_valid(tvb))
		return;

	if (length > tvb->length) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuilder_strip() "
			"out of range\n");
		tvb->valid = 0;
		return;
	}

	tvb->length = length;
}

/**
 * \internal Zapisuje do bufora liczbę 8-bitową.
 *
 * \param tvb   Bufor
 * \param value Wartość do zapisania
 */
void gg_tvbuilder_write_uint8(gg_tvbuilder_t *tvb, uint8_t value)
{
	gg_tvbuilder_write_buff(tvb, (const char *)&value, 1);
}

/**
 * \internal Zapisuje do bufora liczbę 32-bitową.
 *
 * \param tvb   Bufor
 * \param value Wartość do zapisania
 */
void gg_tvbuilder_write_uint32(gg_tvbuilder_t *tvb, uint32_t value)
{
	value = gg_fix32(value);
	gg_tvbuilder_write_buff(tvb, (const char *)&value, 4);
}

/**
 * \internal Zapisuje do bufora liczbę 64-bitową.
 *
 * \param tvb   Bufor
 * \param value Wartość do zapisania
 */
void gg_tvbuilder_write_uint64(gg_tvbuilder_t *tvb, uint64_t value)
{
	value = gg_fix64(value);
	gg_tvbuilder_write_buff(tvb, (const char *)&value, 8);
}

/**
 * \internal Zapisuje do bufora liczbę 1-9 bajtową.
 *
 * \param tvb   Bufor
 * \param value Wartość do zapisania
 *
 * \see gg_tvbuff_read_packed_uint
 */
void gg_tvbuilder_write_packed_uint(gg_tvbuilder_t *tvb, uint64_t value)
{
	uint8_t buff[9];
	uint64_t val_curr;
	int i, val_len = 0;

	if (!gg_tvbuilder_is_valid(tvb))
		return;

	val_curr = value;
	while (val_curr > 0) {
		val_curr >>= 7;
		val_len++;
	}
	if (val_len == 0)
		val_len = 1;

	if (val_len > 9) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuilder_write_packed_uint() "
			"int size too big (%d): %" PRIu64 "\n", val_len, value);
		tvb->valid = 0;
		return;
	}

	val_curr = value;
	for (i = 0; i < val_len; i++) {
		uint8_t raw = val_curr & 0x7F;
		val_curr >>= 7;
		if (i + 1 < val_len)
			raw |= 0x80;
		buff[i] = raw;
	}

	gg_tvbuilder_write_buff(tvb, (const char*)buff, val_len);
}

/**
 * \internal Zapisuje do bufora zawartość innego bufora.
 *
 * \param tvb    Bufor docelowy
 * \param buffer Bufor źródłowy
 * \param length Ilość danych do skopiowania
 */
void gg_tvbuilder_write_buff(gg_tvbuilder_t *tvb, const char *buffer,
	size_t length)
{
	char *buff = gg_tvbuilder_extend(tvb, length);
	if (!buff)
		return;

	memcpy(buff, buffer, length);
}

/**
 * \internal Zapisuje do bufora ciąg tekstowy (mogący zawierać znaki \0).
 *
 * \param tvb    Bufor docelowy
 * \param buffer Bufor źródłowy
 * \param length Długość tekstu, lub -1, jeżeli ma zostać wyliczona
 *               automatycznie (do pierwszego znaku \0)
 */
void gg_tvbuilder_write_str(gg_tvbuilder_t *tvb, const char *buffer,
	ssize_t length)
{
	if (!gg_tvbuilder_is_valid(tvb))
		return;

	if (length == -1)
		length = strlen(buffer);

	gg_tvbuilder_write_packed_uint(tvb, length);
	gg_tvbuilder_write_buff(tvb, buffer, length);
}

/**
 * \internal Zapisuje do bufora identyfikator użytkownika.
 *
 * \param tvb          Bufor
 * \param uin          Identyfikator użytkownika
 */
void gg_tvbuilder_write_uin(gg_tvbuilder_t *tvb, uin_t uin)
{
	char uin_str[16];
	int uin_len;

	uin_len = snprintf(uin_str, sizeof(uin_str), "%u", uin);

	gg_tvbuilder_write_uint8(tvb, 0x00);
	gg_tvbuilder_write_str(tvb, uin_str, uin_len);
}

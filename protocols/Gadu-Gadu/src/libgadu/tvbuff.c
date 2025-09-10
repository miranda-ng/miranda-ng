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
 * \file tvbuff.c
 *
 * \brief Bufor wspierający obsługę pakietów typu Type-Value(s)
 */

#include "internal.h"

#include <stdlib.h>
#include <string.h>

#include "tvbuff.h"

struct gg_tvbuff
{
	const char *buffer;
	size_t length;
	size_t offset;
	int valid;
};

/**
 * \internal Tworzy nową instancję bufora.
 *
 * \param buffer Bufor źródłowy; nie może być modyfikowany (w szczególności
 *        zwalniany) przez cały okres korzystania z jego opakowanej wersji.
 * \param length Długość bufora źródłowego.
 *
 * \return Zaalokowane opakowanie bufora - musi być zwolnione przez free lub
 *         gg_tvbuff_close.
 */
gg_tvbuff_t *gg_tvbuff_new(const char *buffer, size_t length)
{
	gg_tvbuff_t *tvb;

	tvb = malloc(sizeof(gg_tvbuff_t));
	if (tvb == NULL)
		return NULL;
	memset(tvb, 0, sizeof(gg_tvbuff_t));

	if (buffer == NULL && length > 0) {
		gg_debug(GG_DEBUG_ERROR, "// gg_tvbuff_new() "
			"invalid arguments\n");
		tvb->valid = 0;
		return tvb;
	}

	tvb->buffer = buffer;
	tvb->length = length;
	tvb->offset = 0;
	tvb->valid = 1;

	return tvb;
}

/**
 * \internal Zwalnia opakowanie bufora. Przed zwolnieniem sprawdza, czy
 * przeczytano go do końca.
 *
 * \param tvb Bufor.
 *
 * \return Wartość różna od 0, jeżeli bufor tuż przed zwolnieniem był oznaczony
 *         jako prawidłowy
 */
int gg_tvbuff_close(gg_tvbuff_t *tvb)
{
	int valid;

	gg_tvbuff_expected_eob(tvb);
	valid = gg_tvbuff_is_valid(tvb);
	free(tvb);

	return valid;
}

/**
 * \internal Sprawdza, czy wszystkie odczyty z bufora były prawidłowe.
 *
 * \param tvb Bufor.
 *
 * \return Wartość różna od 0, jeżeli wszystkie odczyty były prawidłowe.
 */
int gg_tvbuff_is_valid(const gg_tvbuff_t *tvb)
{
	if (tvb == NULL)
		return 0;
	return tvb->valid;
}

/**
 * \internal Zwraca pozostałą do odczytania liczbę bajtów w buforze.
 *
 * \param tvb Bufor.
 *
 * \return Pozostała liczba bajtów do odczytania.
 */
size_t gg_tvbuff_get_remaining(const gg_tvbuff_t *tvb)
{
	if (!gg_tvbuff_is_valid(tvb))
		return 0;

	return tvb->length - tvb->offset;
}

/**
 * \internal Sprawdza, czy w buforze pozostała określona liczba bajtów do
 * odczytania. Jeżeli nie została - oznacza bufor jako nieprawidłowy.
 *
 * \param tvb    Bufor.
 * \param length Ilość bajtów do odczytania.
 *
 * \return Wartość różna od 0, jeżeli można odczytać podaną liczbę bajtów.
 */
int gg_tvbuff_have_remaining(gg_tvbuff_t *tvb, size_t length)
{
	if (!gg_tvbuff_is_valid(tvb))
		return 0;

	if (gg_tvbuff_get_remaining(tvb) >= length)
		return 1;

	gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_have_remaining() failed "
		"(%" GG_SIZE_FMT " < %" GG_SIZE_FMT ")\n",
		gg_tvbuff_get_remaining(tvb), length);
	tvb->valid = 0;
	return 0;
}

/**
 * \internal Pomija określoną liczbę bajtów w buforze. Jeżeli w wyniku ich
 * pominięcia wyjdzie poza zakres, oznacza bufor jako nieprawidłowy.
 *
 * \param tvb    Bufor
 * \param amount Liczba bajtów do pominięcia
 */
void gg_tvbuff_skip(gg_tvbuff_t *tvb, size_t amount)
{
	if (!gg_tvbuff_is_valid(tvb))
		return;

	if (tvb->offset + amount > tvb->length) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_skip() failed\n");
		tvb->valid = 0;
		return;
	}

	tvb->offset += amount;
}

/**
 * \internal Cofa się o określoną liczbę bajtów w buforze. Jeżeli cofnie przed
 * pierwszy znak, oznacza bufor jako nieprawidłowy.
 *
 * \param tvb    Bufor
 * \param amount Liczba bajtów do cofnięcia
 */
void gg_tvbuff_rewind(gg_tvbuff_t *tvb, size_t amount)
{
	if (!gg_tvbuff_is_valid(tvb))
		return;

	if (tvb->offset < amount) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_rewind() failed\n");
		tvb->valid = 0;
		return;
	}

	tvb->offset -= amount;
}

/**
 * \internal Sprawdza, czy pod aktualną pozycją w buforze występuje podana
 * wartość. Jeżeli tak, przesuwa aktualną pozycję do przodu.
 *
 * \param tvb   Bufor.
 * \param value Wartość do sprawdzenia
 *
 * \return Wartość różna od 0, jeżeli znaleziono podaną wartość.
 */
int gg_tvbuff_match(gg_tvbuff_t *tvb, uint8_t value)
{
	if (!gg_tvbuff_is_valid(tvb))
		return 0;

	if (!gg_tvbuff_have_remaining(tvb, 1)) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_match() failed\n");
		return 0;
	}

	if (tvb->buffer[tvb->offset] != value)
		return 0;

	tvb->offset++;
	return 1;
}

/**
 * \internal Odczytuje z bufora liczbę 8-bitową.
 *
 * \param tvb Bufor
 *
 * \return Odczytana liczba
 */
uint8_t gg_tvbuff_read_uint8(gg_tvbuff_t *tvb)
{
	if (!gg_tvbuff_is_valid(tvb))
		return 0;

	if (!gg_tvbuff_have_remaining(tvb, 1)) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_read_uint8() "
			"failed at %" GG_SIZE_FMT "\n", tvb->offset);
		return 0;
	}

	return tvb->buffer[tvb->offset++];
}

/**
 * \internal Odczytuje z bufora liczbę 32-bitową.
 *
 * \param tvb Bufor
 *
 * \return Odczytana liczba
 */
uint32_t gg_tvbuff_read_uint32(gg_tvbuff_t *tvb)
{
	uint32_t val;

	if (!gg_tvbuff_is_valid(tvb))
		return 0;

	if (!gg_tvbuff_have_remaining(tvb, 4)) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_read_uint32() "
			"failed at %" GG_SIZE_FMT "\n", tvb->offset);
		return 0;
	}

	memcpy(&val, tvb->buffer + tvb->offset, 4);
	tvb->offset += 4;

	return gg_fix32(val);
}

/**
 * \internal Odczytuje z bufora liczbę 64-bitową.
 *
 * \param tvb Bufor
 *
 * \return Odczytana liczba
 */
uint64_t gg_tvbuff_read_uint64(gg_tvbuff_t *tvb)
{
	uint64_t val;

	if (!gg_tvbuff_is_valid(tvb))
		return 0;

	if (!gg_tvbuff_have_remaining(tvb, 8)) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_read_uint64() "
			"failed at %" GG_SIZE_FMT "\n", tvb->offset);
		return 0;
	}

	memcpy(&val, tvb->buffer + tvb->offset, 8);
	tvb->offset += 8;

	return gg_fix64(val);
}

/**
 * \internal Odczytuje z bufora skompresowaną liczbę całkowitą.
 * Liczba taka może być zapisana w buforze na 1-9 bajtach, w zależności
 * od jej wartości.
 *
 * Skompresowana liczba jest zapisywana od najmłodszego bajtu do najstarszego
 * niezerowego. W każdym bajcie zapisuje się bit sterujący (równy 0, jeżeli jest
 * to ostatni bajt do przeczytania, lub 1 w p.p.) oraz 7 kolejnych bitów z
 * kompresowanej liczby.
 *
 * Przykładowo, liczby mniejsze od 128 (1000.0000b) są zapisywane dokładnie tak,
 * jak uint8_t; a np. 12345 (0011.0000.0011.1001b) zostanie zapisana jako 0x60B9
 * (0110.0000.1011.1001b).
 *
 * \param tvb Bufor.
 *
 * \return Odczytana liczba.
 */
uint64_t gg_tvbuff_read_packed_uint(gg_tvbuff_t *tvb)
{
	uint64_t val = 0;
	int i, val_len = 0;

	if (!gg_tvbuff_is_valid(tvb))
		return 0;

	while (gg_tvbuff_have_remaining(tvb, 1)) {
		val_len++;
		if (!(gg_tvbuff_read_uint8(tvb) & 0x80))
			break;
	}

	if (!gg_tvbuff_is_valid(tvb)) {
		gg_debug(GG_DEBUG_WARNING,
			"// gg_tvbuff_read_packed_uint() failed\n");
		return 0;
	}

	if (val_len > 9) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_read_packed_uint() "
			"packed uint size too big: %d\n", val_len);
		tvb->valid = 0;
		return 0;
	}

	for (i = 1; i <= val_len; i++) {
		uint64_t old_val = val;
		val <<= 7;
		if (old_val != (val >> 7)) {
			gg_debug(GG_DEBUG_WARNING,
				"// gg_tvbuff_read_packed_uint() overflow\n");
			tvb->valid = 0;
			return 0;
		}
		val |= (uint8_t)(tvb->buffer[tvb->offset - i] & ~0x80);
	}

	return val;
}

/**
 * \internal Odczytuje z bufora podciąg bez kopiowania danych.
 *
 * \param tvb    Bufor źródłowy
 * \param length Ilość bajtów do odczytania
 *
 * \return Wskaźnik na początek odczytanych danych, lub NULL w przypadku
 *         niepowodzenia
 */
const char *gg_tvbuff_read_buff(gg_tvbuff_t *tvb, size_t length)
{
	const char *buff;

	if (!gg_tvbuff_is_valid(tvb))
		return NULL;

	if (!gg_tvbuff_have_remaining(tvb, length)) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_get_buff() "
			"failed at %" GG_SIZE_FMT ":%" GG_SIZE_FMT "\n",
			tvb->offset, length);
		return NULL;
	}

	buff = tvb->buffer + tvb->offset;
	tvb->offset += length;
	return buff;
}

/**
 * \internal Odczytuje z bufora podciąg kopiując go do nowego obszaru pamięci.
 *
 * \param tvb    Bufor źródłowy
 * \param buffer Bufor docelowy
 * \param length Ilość bajtów do odczytania
 */
void gg_tvbuff_read_buff_cpy(gg_tvbuff_t *tvb, char *buffer, size_t length)
{
	if (!gg_tvbuff_is_valid(tvb))
		return;

	if (!gg_tvbuff_have_remaining(tvb, length)) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_read_buff() "
			"failed at %" GG_SIZE_FMT ":%" GG_SIZE_FMT "\n",
			tvb->offset, length);
		return;
	}

	if (buffer == NULL && length > 0) {
		gg_debug(GG_DEBUG_ERROR, "// gg_tvbuff_new() "
			"invalid arguments\n");
		tvb->valid = 0;
		return;
	}

	memcpy(buffer, tvb->buffer + tvb->offset, length);
	tvb->offset += length;
}

/**
 * \internal Odczytuje z bufora ciąg tekstowy (mogący zawierać dowolne znaki,
 * również \0) bez kopiowania danych.
 *
 * \param tvb    Bufor źródłowy
 * \param length Zmienna, do której zostanie zapisana długość odczytanego ciągu
 *
 * \return Wskaźnik na początek odczytanych danych, lub NULL w przypadku
 *         niepowodzenia
 */
const char *gg_tvbuff_read_str(gg_tvbuff_t *tvb, size_t *length)
{
	size_t offset;
	uint32_t str_len;
	const char *str;

	if (!gg_tvbuff_is_valid(tvb))
		return NULL;

	offset = tvb->offset;
	str_len = gg_tvbuff_read_packed_uint(tvb);
	if (!gg_tvbuff_is_valid(tvb) ||
		!gg_tvbuff_have_remaining(tvb, str_len))
	{
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_read_str() failed at "
			"%" GG_SIZE_FMT ":%d\n", offset, str_len);
		return NULL;
	}

	str = gg_tvbuff_read_buff(tvb, str_len);

	if (!gg_tvbuff_is_valid(tvb))
		return NULL;

	if (length != NULL)
		*length = str_len;
	if (str_len == 0)
		return NULL;
	return str;
}

/**
 * \internal Odczytuje z bufora ciąg tekstowy (mogący zawierać dowolne znaki,
 * również \0) kopiując go do nowego obszaru pamięci. Zwrócony ciąg będzie
 * zawsze zakończony znakiem \0.
 *
 * \param tvb Bufor źródłowy
 * \param dst Zmienna, do której zostanie zapisany wskaźnik na odczytany ciąg.
 *            Po użyciu, blok ten powinien zostać zwolniony za pomocą \c free()
 *
 * \return Wskaźnik na początek odczytanych danych, lub NULL w przypadku
 *         niepowodzenia
 */
void gg_tvbuff_read_str_dup(gg_tvbuff_t *tvb, char **dst)
{
	size_t offset;
	uint32_t str_len;
	char *str;

	if (!gg_tvbuff_is_valid(tvb))
		return;

	offset = tvb->offset;
	str_len = gg_tvbuff_read_packed_uint(tvb);
	if (!gg_tvbuff_is_valid(tvb) ||
		!gg_tvbuff_have_remaining(tvb, str_len))
	{
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_read_str_dup() failed "
			"at %" GG_SIZE_FMT ":%d\n", offset, str_len);
		return;
	}

	str = malloc(str_len + 1);
	if (str == NULL) {
		gg_debug(GG_DEBUG_ERROR, "// gg_tvbuff_read_str_dup() "
			"not enough free memory: %d + 1\n", str_len);
		tvb->valid = 0;
		return;
	}

	gg_tvbuff_read_buff_cpy(tvb, str, str_len);
	str[str_len] = '\0';

	if (*dst != NULL) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_read_str_dup() "
			"destination already filled, freeing it...\n");
		free(*dst);
	}
	*dst = str;
}

/**
 * \internal Odczytuje z bufora identyfikator użytkownika.
 *
 * \param tvb Bufor
 *
 * \return Identyfikator użytkownika, lub 0 w przypadku niepowodzenia
 */
uin_t gg_tvbuff_read_uin(gg_tvbuff_t *tvb)
{
	uin_t uin = 0;
	uint32_t uin_len, full_len;
	uint8_t uin_type;
	const char *raw;

	if (!gg_tvbuff_is_valid(tvb))
		return 0;

	full_len = gg_tvbuff_read_packed_uint(tvb);
	uin_type = gg_tvbuff_read_uint8(tvb);
	uin_len = gg_tvbuff_read_uint8(tvb);

	if (!gg_tvbuff_is_valid(tvb))
		return 0;

	if (full_len != uin_len + 2 ||
		uin_type != 0 ||
		uin_len > 10)
	{
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_read_uin() failed (1)\n");
		tvb->valid = 0;
		return 0;
	}

	raw = gg_tvbuff_read_buff(tvb, uin_len);
	if (raw)
		uin = gg_str_to_uin(raw, uin_len);

	if (uin == 0) {
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_read_uin() failed (2)\n");
		tvb->valid = 0;
		return 0;
	}

	return uin;
}

/**
 * \internal Odczytuje z bufora liczbę 8-bitową i porównuje z podaną. Jeżeli te
 * się różnią, zostaje wygenerowane ostrzeżenie.
 *
 * \param tvb   Bufor
 * \param value Oczekiwana wartość
 */
void gg_tvbuff_expected_uint8(gg_tvbuff_t *tvb, uint8_t value)
{
	uint8_t got;
	size_t offset;

	offset = tvb->offset;
	got = gg_tvbuff_read_uint8(tvb);
	if (!gg_tvbuff_is_valid(tvb))
		return;

	if (got != value)
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_expected_uint8() "
			"expected %#02x, but %#02x found at %" GG_SIZE_FMT "\n",
			value, got, offset);
}

/**
 * \internal Odczytuje z bufora liczbę 32-bitową i porównuje z podaną. Jeżeli te
 * się różnią, zostaje wygenerowane ostrzeżenie.
 *
 * \param tvb   Bufor
 * \param value Oczekiwana wartość
 */
void gg_tvbuff_expected_uint32(gg_tvbuff_t *tvb, uint32_t value)
{
	uint32_t got;
	size_t offset;

	offset = tvb->offset;
	got = gg_tvbuff_read_uint32(tvb);
	if (!gg_tvbuff_is_valid(tvb))
		return;

	if (got != value)
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_expected_uint32() "
			"expected %#08x, but %#08x found at %" GG_SIZE_FMT "\n",
			value, got, offset);
}

/**
 * \internal Oczekuje końca bufora. Jeżeli w buforze są jeszcze dane do
 * przeczytania, generuje ostrzeżenie.
 *
 * \param tvb Bufor.
 */
void gg_tvbuff_expected_eob(const gg_tvbuff_t *tvb)
{
	if (!gg_tvbuff_is_valid(tvb))
		return;

	if (gg_tvbuff_get_remaining(tvb) != 0)
		gg_debug(GG_DEBUG_WARNING, "// gg_tvbuff_expected_eob() "
			"unexpected %" GG_SIZE_FMT " bytes, first=%#02x\n",
			gg_tvbuff_get_remaining(tvb),
			tvb->buffer[tvb->offset]);
}

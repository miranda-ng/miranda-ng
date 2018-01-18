/*
 *  (C) Copyright 2008-2009 Jakub Zawadzki <darkjames@darkjames.ath.cx>
 *                          Wojtek Kaniewski <wojtekka@irc.pl>
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

#ifdef _WIN32
#include "win32.h"
#endif /* _WIN32 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "libgadu.h"
#include "encoding.h"

/**
 * \file encoding.c
 *
 * \brief Funkcje konwersji kodowania tekstu
 */

/**
 * \internal Tablica konwersji CP1250 na Unikod.
 */
static const uint16_t table_cp1250[] =
{
	0x20ac, '?',    0x201a, '?',    0x201e, 0x2026, 0x2020, 0x2021,
	'?',    0x2030, 0x0160, 0x2039, 0x015a, 0x0164, 0x017d, 0x0179,
	'?',    0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
	'?',    0x2122, 0x0161, 0x203a, 0x015b, 0x0165, 0x017e, 0x017a,
	0x00a0, 0x02c7, 0x02d8, 0x0141, 0x00a4, 0x0104, 0x00a6, 0x00a7,
	0x00a8, 0x00a9, 0x015e, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x017b,
	0x00b0, 0x00b1, 0x02db, 0x0142, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
	0x00b8, 0x0105, 0x015f, 0x00bb, 0x013d, 0x02dd, 0x013e, 0x017c,
	0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7,
	0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e,
	0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7,
	0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df,
	0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7,
	0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f,
	0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7,
	0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9,
};

/**
 * \internal Zamienia tekst kodowany CP1250 na UTF-8.
 *
 * \param src Tekst źródłowy w CP1250.
 * \param src_length Długość ciągu źródłowego (nigdy ujemna).
 * \param dst_length Długość ciągu docelowego (jeśli -1, nieograniczona).
 *
 * \return Zaalokowany bufor z tekstem w UTF-8.
 */
static char *gg_encoding_convert_cp1250_utf8(const char *src, int src_length, int dst_length)
{
	int i, j, len;
	char *result = NULL;

	for (i = 0, len = 0; (src[i] != 0) && (i < src_length); i++) {
		uint16_t uc;

		if ((unsigned char) src[i] < 0x80)
			uc = (unsigned char) src[i];
		else
			uc = table_cp1250[(unsigned char) src[i] - 128];

		if (uc < 0x80)
			len += 1;
		else if (uc < 0x800)
			len += 2;
		else
			len += 3;
	}

	if ((dst_length != -1) && (len > dst_length))
		len = dst_length;

	result = (char *)malloc(len + 1);

	if (result == NULL) 
		return NULL;

	for (i = 0, j = 0; (src[i] != 0) && (i < src_length) && (j < len); i++) {
		uint16_t uc;

		if ((unsigned char) src[i] < 0x80)
			uc = (unsigned char) src[i];
		else
			uc = table_cp1250[(unsigned char) src[i] - 128];

		if (uc < 0x80) 
			result[j++] = uc;
		else if (uc < 0x800) {
			if (j + 1 > len)
				break;
			result[j++] = 0xc0 | ((uc >> 6) & 0x1f);
			result[j++] = 0x80 | (uc & 0x3f);
		} else {
			if (j + 2 > len)
				break;
			result[j++] = 0xe0 | ((uc >> 12) & 0x1f);
			result[j++] = 0x80 | ((uc >> 6) & 0x3f);
			result[j++] = 0x80 | (uc & 0x3f);
		}
	}

	result[j] = 0;

	return result;
}

/**
 * \internal Zamienia tekst kodowany UTF-8 na CP1250.
 *
 * \param src Tekst źródłowy w UTF-8.
 * \param src_length Długość ciągu źródłowego (nigdy ujemna).
 * \param dst_length Długość ciągu docelowego (jeśli -1, nieograniczona).
 *
 * \return Zaalokowany bufor z tekstem w CP1250.
 */
static char *gg_encoding_convert_utf8_cp1250(const char *src, int src_length, int dst_length)
{
	char *result;
	int i, j, len, uc_left = 0;
	uint32_t uc = 0, uc_min = 0;

	for (i = 0, len = 0; (src[i] != 0) && (i < src_length); i++) {
		if ((src[i] & 0xc0) != 0x80)
			len++;
	}

	if ((dst_length != -1) && (len > dst_length))
		len = dst_length;

	result = (char *)malloc(len + 1);

	if (result == NULL)
		return NULL;

	for (i = 0, j = 0; (src[i] != 0) && (i < src_length) && (j < len); i++) {
		if ((unsigned char) src[i] >= 0xf5) {
			if (uc_left != 0) 
				result[j++] = '?';
			/* Restricted sequences */
			result[j++] = '?';
			uc_left = 0;
		} else if ((src[i] & 0xf8) == 0xf0) {
			if (uc_left != 0) 
				result[j++] = '?';
			uc = src[i] & 0x07;
			uc_left = 3;
			uc_min = 0x10000;
		} else if ((src[i] & 0xf0) == 0xe0) {
			if (uc_left != 0) 
				result[j++] = '?';
			uc = src[i] & 0x0f;
			uc_left = 2;
			uc_min = 0x800;
		} else if ((src[i] & 0xe0) == 0xc0) {
			if (uc_left != 0) 
				result[j++] = '?';
			uc = src[i] & 0x1f;
			uc_left = 1;
			uc_min = 0x80;
		} else if ((src[i] & 0xc0) == 0x80) {
			if (uc_left > 0) {
				uc <<= 6;
				uc |= src[i] & 0x3f;
				uc_left--;

				if (uc_left == 0) {
					int valid = 0;
					int k;

					if (uc >= uc_min) {
						for (k = 0; k < 128; k++) {
							if (uc == table_cp1250[k]) {
								result[j++] = k + 128;
								valid = 1;
								break;
							}
						}
					}

					if (!valid && uc != 0xfeff)	/* Byte Order Mark */
						result[j++] = '?';
				}
			}
		} else {
			if (uc_left != 0) {
				result[j++] = '?';
				uc_left = 0;
			}
			result[j++] = src[i];
		}
	}

	if ((uc_left != 0) && (src[i] == 0))
		result[j++] = '?';

	result[j] = 0;

	return result;
}

/**
 * \internal Zamienia kodowanie tekstu.
 *
 * \param src Tekst źródłowy.
 * \param src_encoding Kodowanie tekstu źródłowego.
 * \param dst_encoding Kodowanie tekstu docelowego.
 * \param src_length Długość ciągu źródłowego w bajtach (nigdy ujemna).
 * \param dst_length Długość ciągu docelowego w bajtach (jeśli -1, nieograniczona).
 *
 * \return Zaalokowany bufor z tekstem w kodowaniu docelowym.
 */
char *gg_encoding_convert(const char *src, gg_encoding_t src_encoding, gg_encoding_t dst_encoding, int src_length, int dst_length)
{
	char *result;

	if (src == NULL) {
		errno = EINVAL;
		return NULL;
	}

	// specjalny przypadek obsługiwany ekspresowo
	if ((dst_encoding == src_encoding) && (dst_length == -1) && (src_length == -1))
		return strdup(src);

	if (src_length == -1)
		src_length = strlen(src);

	if (dst_encoding == src_encoding) {
		int len;

		if (dst_length == -1)
			len = src_length;
		else
			len = (src_length < dst_length) ? src_length : dst_length;

		result = (char *)malloc(len + 1);

		if (result == NULL)
			return NULL;

		strncpy(result, src, len);
		result[len] = 0;

		return result;
	}

	if (dst_encoding == GG_ENCODING_CP1250 && src_encoding == GG_ENCODING_UTF8)
		return gg_encoding_convert_utf8_cp1250(src, src_length, dst_length);

	if (dst_encoding == GG_ENCODING_UTF8 && src_encoding == GG_ENCODING_CP1250)
		return gg_encoding_convert_cp1250_utf8(src, src_length, dst_length);

	errno = EINVAL;
	return NULL;
}

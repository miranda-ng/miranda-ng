/*
 *  (C) Copyright 2011 Bartosz Brachaczek <b.brachaczek@gmail.com>
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
 * \file deflate.c
 *
 * \brief Funkcje kompresji Deflate
 */

#include "internal.h"

#include <stdlib.h>
#include <string.h>

#include "deflate.h"

#ifdef GG_CONFIG_HAVE_ZLIB
#include <zlib.h>
#endif

/**
 * \internal Kompresuje dane wejściowe algorytmem Deflate z najwyższym
 * stopniem kompresji, tak samo jak oryginalny klient.
 *
 * Wynik funkcji należy zwolnić za pomocą \c free.
 *
 * \param in Ciąg znaków do skompresowania, zakończony \c \\0
 * \param out_lenp Wskaźnik na zmienną, do której zostanie zapisana
 *                 długość bufora wynikowego
 *
 * \return Skompresowany ciąg znaków lub \c NULL w przypadku niepowodzenia.
 */
unsigned char *gg_deflate(const char *in, size_t *out_lenp)
{
#ifdef GG_CONFIG_HAVE_ZLIB
	int ret;
	z_stream strm;
	unsigned char *out, *out2;
	size_t out_len;

	if (in == NULL || out_lenp == NULL)
		return NULL;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = strlen(in);
	strm.next_in = (unsigned char*) in;

	ret = deflateInit(&strm, Z_BEST_COMPRESSION);
	if (ret != Z_OK) {
		gg_debug(GG_DEBUG_MISC, "// gg_deflate() deflateInit() failed (%d)\n", ret);
		return NULL;
	}

	out_len = deflateBound(&strm, strm.avail_in);
	out = malloc(out_len);

	if (out == NULL) {
		gg_debug(GG_DEBUG_MISC, "// gg_deflate() not enough memory for "
			"output data (%" GG_SIZE_FMT ")\n", out_len);
		goto fail;
	}

	strm.avail_out = out_len;
	strm.next_out = out;

	for (;;) {
		ret = deflate(&strm, Z_FINISH);

		if (ret == Z_STREAM_END)
			break;

		/* raczej nie powinno się zdarzyć przy Z_FINISH i out_len == deflateBound(),
		 * ale dokumentacja zlib nie wyklucza takiej możliwości */
		if (ret == Z_OK) {
			out_len *= 2;
			out2 = realloc(out, out_len);

			if (out2 == NULL) {
				gg_debug(GG_DEBUG_MISC, "// gg_deflate() not "
					"enough memory for output data (%"
					GG_SIZE_FMT ")\n", out_len);
				goto fail;
			}

			out = out2;

			strm.avail_out = out_len / 2;
			strm.next_out = out + out_len / 2;
		} else {
			gg_debug(GG_DEBUG_MISC, "// gg_deflate() deflate() "
				"failed (ret=%d, msg=%s)\n", ret,
				strm.msg != NULL ? strm.msg :
				"no error message provided");
			goto fail;
		}
	}

	out_len = strm.total_out;
	out2 = realloc(out, out_len);

	if (out2 == NULL) {
		gg_debug(GG_DEBUG_MISC, "// gg_deflate() not enough memory for "
			"output data (%" GG_SIZE_FMT ")\n", out_len);
		goto fail;
	}

	*out_lenp = out_len;
	deflateEnd(&strm);

	return out2;

fail:
	*out_lenp = 0;
	deflateEnd(&strm);
	free(out);
#endif
	return NULL;
}

/**
 * \internal Dekompresuje dane wejściowe w formacie Deflate.
 *
 * Wynik funkcji należy zwolnić za pomocą \c free.
 *
 * \param in Bufor danych skompresowanych algorytmem Deflate
 * \param length Długość bufora wejściowego
 *
 * \note Dokleja \c \\0 na końcu bufora wynikowego.
 *
 * \return Zdekompresowany ciąg znaków, zakończony \c \\0,
 *         lub \c NULL w przypadku niepowodzenia.
 */
char *gg_inflate(const unsigned char *in, size_t length)
{
#ifdef GG_CONFIG_HAVE_ZLIB
	int ret;
	z_stream strm;
	char *out = NULL, *out2;
	size_t out_len = 1024;
	int first = 1;

	if (in == NULL)
		return NULL;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = length;
	strm.next_in = (unsigned char*) in;

	ret = inflateInit(&strm);
	if (ret != Z_OK) {
		gg_debug(GG_DEBUG_MISC, "// gg_inflate() inflateInit() failed (%d)\n", ret);
		return NULL;
	}

	do {
		out_len *= 2;
		out2 = realloc(out, out_len);

		if (out2 == NULL) {
			gg_debug(GG_DEBUG_MISC, "// gg_inflate() not enough "
				"memory for output data (%" GG_SIZE_FMT ")\n", out_len);
			goto fail;
		}

		out = out2;

		if (first) {
			strm.avail_out = out_len;
			strm.next_out = (unsigned char*) out;
		} else {
			strm.avail_out = out_len / 2;
			strm.next_out = (unsigned char*) out + out_len / 2;
		}

		ret = inflate(&strm, Z_NO_FLUSH);

		if (ret != Z_OK && ret != Z_STREAM_END) {
			gg_debug(GG_DEBUG_MISC, "// gg_inflate() inflate() "
				"failed (ret=%d, msg=%s)\n", ret,
				strm.msg != NULL ? strm.msg :
				"no error message provided");
			goto fail;
		}

		first = 0;
	} while (ret != Z_STREAM_END);

	/* rezerwujemy ostatni znak na NULL-a */
	out_len = strm.total_out + 1;
	out2 = realloc(out, out_len);

	if (out2 == NULL) {
		gg_debug(GG_DEBUG_MISC, "// gg_inflate() not enough memory for "
			"output data (%" GG_SIZE_FMT ")\n", out_len);
		goto fail;
	}

	out = out2;
	out[out_len - 1] = '\0';

	inflateEnd(&strm);

	return out;

fail:
	inflateEnd(&strm);
	free(out);
#endif
	return NULL;
}

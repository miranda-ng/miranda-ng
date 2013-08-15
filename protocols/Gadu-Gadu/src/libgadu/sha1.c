/* coding: UTF-8 */
/* $Id: sha1.c,v 1.4 2007-07-20 23:00:50 wojtekka Exp $ */

/*
 *  (C) Copyright 2007 Wojtek Kaniewski <wojtekka@irc.pl>
 *
 *  Public domain SHA-1 implementation by Steve Reid <steve@edmweb.com>
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
 * \file sha1.c
 *
 * \brief Funkcje wyznaczania skrĂłtu SHA1
 */

#include <string.h>
#include <sys/types.h>
#ifdef _WIN32
#include "win32.h"
#else
#include <unistd.h>
#endif

#include "libgadu.h"

/**
 * \internal Liczy skrĂłt SHA1 z ziarna i hasĹa.
 *
 * \param password HasĹo
 * \param seed Ziarno
 * \param result Bufor na wynik funkcji skrĂłtu (20 bajtĂłw)
 */
void gg_login_hash_sha1(const char *password, uint32_t seed, uint8_t *result)
{
	mir_sha1_ctx ctx;
	
	mir_sha1_init(&ctx);
	mir_sha1_append(&ctx, (uint8_t*)password, (unsigned int)strlen(password));
	seed = gg_fix32(seed);
	mir_sha1_append(&ctx, (uint8_t*)&seed, 4);
	mir_sha1_finish(&ctx, result);
}

/**
 * \internal Liczy skrĂłt SHA1 z pliku.
 *
 * \param fd Deskryptor pliku
 * \param result WskaĹşnik na skrĂłt
 *
 * \return 0 lub -1
 */
int gg_file_hash_sha1(int fd, uint8_t *result)
{
	unsigned char buf[4096];
	mir_sha1_ctx ctx;
	off_t pos, len;
	int res;

	if ((pos = lseek(fd, 0, SEEK_CUR)) == (off_t) -1)
		return -1;

	if ((len = lseek(fd, 0, SEEK_END)) == (off_t) -1)
		return -1;

	if (lseek(fd, 0, SEEK_SET) == (off_t) -1)
		return -1;

	mir_sha1_init(&ctx);

	if (len <= 10485760) {
		while ((res = read(fd, buf, sizeof(buf))) > 0)
			mir_sha1_append(&ctx, buf, res);
	}
	else {
		int i;

		for (i = 0; i < 9; i++) {
			int j;

			if (lseek(fd, (len - 1048576) / 9 * i, SEEK_SET) == (off_t) - 1)
				return -1;

			for (j = 0; j < 1048576 / sizeof(buf); j++) {
				if ((res = read(fd, buf, sizeof(buf))) != sizeof(buf)) {
					res = -1;
					break;
				}

				mir_sha1_append(&ctx, buf, res);
			}

			if (res == -1)
				break;
		}
	}

	if (res == -1)
		return -1;

	mir_sha1_finish(&ctx, result);

	if (lseek(fd, pos, SEEK_SET) == (off_t) -1)
		return -1;

	return 0;
}

/** \endcond */

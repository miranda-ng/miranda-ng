/*
 *  Off-the-Record Messaging Toolkit
 *  Copyright (C) 2004-2008  Ian Goldberg, Chris Alexander, Nikita Borisov
 *                           <otr@cypherpunks.ca>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public License as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* system headers */
#include <stdio.h>
#include <stdlib.h>

/* libgcrypt headers */
#include <gcrypt.h>

/* Implementation of SHA1-HMAC.  We're rolling our own just to
 * double-check that the calls libotr makes to libgcrypt are in fact
 * doing the right thing. */
void sha1hmac(unsigned char digest[20], unsigned char key[20],
	unsigned char *data, size_t datalen)
{
    unsigned char ipad[64], opad[64];
    size_t i;
    gcry_md_hd_t sha1;
    gcry_error_t err;
    unsigned char hash[20];

    memset(ipad, 0, 64);
    memset(opad, 0, 64);
    memmove(ipad, key, 20);
    memmove(opad, key, 20);
    for(i=0;i<64;++i) {
	ipad[i] ^= 0x36;
	opad[i] ^= 0x5c;
    }

    err = gcry_md_open(&sha1, GCRY_MD_SHA1, 0);
    if (err) {
	fprintf(stderr, "Error: %s\n", gcry_strerror(err));
	exit(1);
    }
    gcry_md_write(sha1, ipad, 64);
    gcry_md_write(sha1, data, datalen);
    memmove(hash, gcry_md_read(sha1, 0), 20);
    gcry_md_reset(sha1);
    gcry_md_write(sha1, opad, 64);
    gcry_md_write(sha1, hash, 20);
    memmove(digest, gcry_md_read(sha1, 0), 20);
    gcry_md_close(sha1);
}

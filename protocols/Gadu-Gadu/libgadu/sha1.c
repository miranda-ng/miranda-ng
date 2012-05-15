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

/** \cond ignore */

#if defined(GG_CONFIG_HAVE_OPENSSL) && !defined(GG_CONFIG_MIRANDA)

#include <openssl/sha.h>

#else

/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain

Modified by Wojtek Kaniewski <wojtekka@toxygen.net> for compatibility
with libgadu and OpenSSL API.

Test Vectors (from FIPS PUB 180-1)
"abc"
  A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
  84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
A million repetitions of "a"
  34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

/* #define LITTLE_ENDIAN * This should be #define'd if true. */
/* #define SHA1HANDSOFF * Copies data before messing with it. */

#include <string.h>

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA_CTX;

static void SHA1_Transform(uint32_t state[5], const unsigned char buffer[64]);
static void SHA1_Init(SHA_CTX* context);
static void SHA1_Update(SHA_CTX* context, const unsigned char* data, unsigned int len);
static void SHA1_Final(unsigned char digest[20], SHA_CTX* context);

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#ifndef GG_CONFIG_BIGENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);


/* Hash a single 512-bit block. This is the core of the algorithm. */

static void SHA1_Transform(uint32_t state[5], const unsigned char buffer[64])
{
uint32_t a, b, c, d, e;
typedef union {
    unsigned char c[64];
    uint32_t l[16];
} CHAR64LONG16;
CHAR64LONG16* block;
static unsigned char workspace[64];
    block = (CHAR64LONG16*)workspace;
    memcpy(block, buffer, 64);
    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    /* Wipe variables */
    a = b = c = d = e = 0;
}


/* SHA1_Init - Initialize new context */

static void SHA1_Init(SHA_CTX* context)
{
    /* SHA1 initialization constants */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */

static void SHA1_Update(SHA_CTX* context, const unsigned char* data, unsigned int len)
{
unsigned int i, j;

    j = (context->count[0] >> 3) & 63;
    if ((context->count[0] += len << 3) < (len << 3)) context->count[1]++;
    context->count[1] += (len >> 29);
    if ((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64-j));
        SHA1_Transform(context->state, context->buffer);
        for ( ; i + 63 < len; i += 64) {
            SHA1_Transform(context->state, &data[i]);
        }
        j = 0;
    }
    else i = 0;
    memcpy(&context->buffer[j], &data[i], len - i);
}


/* Add padding and return the message digest. */

static void SHA1_Final(unsigned char digest[20], SHA_CTX* context)
{
uint32_t i, j;
unsigned char finalcount[8];

    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
         >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
    }
    SHA1_Update(context, (unsigned char *)"\200", 1);
    while ((context->count[0] & 504) != 448) {
        SHA1_Update(context, (unsigned char *)"\0", 1);
    }
    SHA1_Update(context, finalcount, 8);  /* Should cause a SHA1_Transform() */
    for (i = 0; i < 20; i++) {
        digest[i] = (unsigned char)
         ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
    }
    /* Wipe variables */
    i = j = 0;
    memset(context->buffer, 0, 64);
    memset(context->state, 0, 20);
    memset(context->count, 0, 8);
    memset(&finalcount, 0, 8);
#ifdef SHA1HANDSOFF  /* make SHA1_Transform overwrite it's own static vars */
    SHA1_Transform(context->state, context->buffer);
#endif
}

#endif /* GG_CONFIG_HAVE_OPENSSL */

/** \endcond */

/** \cond internal */

/**
 * \internal Liczy skrĂłt SHA1 z ziarna i hasĹa.
 *
 * \param password HasĹo
 * \param seed Ziarno
 * \param result Bufor na wynik funkcji skrĂłtu (20 bajtĂłw)
 */
void gg_login_hash_sha1(const char *password, uint32_t seed, uint8_t *result)
{
	SHA_CTX ctx;
	
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, (const unsigned char*) password, (unsigned int)strlen(password));
	seed = gg_fix32(seed);
	SHA1_Update(&ctx, (uint8_t*) &seed, 4);
	
	SHA1_Final(result, &ctx);
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
	SHA_CTX ctx;
	off_t pos, len;
	int res;

	if ((pos = lseek(fd, 0, SEEK_CUR)) == (off_t) -1)
		return -1;

	if ((len = lseek(fd, 0, SEEK_END)) == (off_t) -1)
		return -1;

	if (lseek(fd, 0, SEEK_SET) == (off_t) -1)
		return -1;

	SHA1_Init(&ctx);

	if (len <= 10485760) {
		while ((res = read(fd, buf, sizeof(buf))) > 0)
			SHA1_Update(&ctx, buf, res);
	} else {
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

				SHA1_Update(&ctx, buf, res);
			}

			if (res == -1)
				break;
		}
	}

	if (res == -1)
		return -1;

	SHA1_Final(result, &ctx);

	if (lseek(fd, pos, SEEK_SET) == (off_t) -1)
		return -1;

	return 0;
}

/** \endcond */

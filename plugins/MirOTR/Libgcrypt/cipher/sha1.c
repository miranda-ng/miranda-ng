/* sha1.c - SHA1 hash function
 * Copyright (C) 1998, 2001, 2002, 2003, 2008 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */


/*  Test vectors:
 *
 *  "abc"
 *  A999 3E36 4706 816A BA3E  2571 7850 C26C 9CD0 D89D
 *
 *  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
 *  8498 3E44 1C3B D26E BAAE  4AA1 F951 29E5 E546 70F1
 */


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#include "g10lib.h"
#include "bithelp.h"
#include "bufhelp.h"
#include "cipher.h"
#include "hash-common.h"


/* USE_SSSE3 indicates whether to compile with Intel SSSE3 code. */
#undef USE_SSSE3
#if defined(__x86_64__) && defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_SSSE3)
# define USE_SSSE3 1
#endif


/* A macro to test whether P is properly aligned for an u32 type.
   Note that config.h provides a suitable replacement for uintptr_t if
   it does not exist in stdint.h.  */
/* #if __GNUC__ >= 2 */
/* # define U32_ALIGNED_P(p) (!(((uintptr_t)p) % __alignof__ (u32))) */
/* #else */
/* # define U32_ALIGNED_P(p) (!(((uintptr_t)p) % sizeof (u32))) */
/* #endif */

typedef struct
{
  gcry_md_block_ctx_t bctx;
  u32           h0,h1,h2,h3,h4;
#ifdef USE_SSSE3
  unsigned int use_ssse3:1;
#endif
} SHA1_CONTEXT;

static unsigned int
transform (void *c, const unsigned char *data);


static void
sha1_init (void *context, unsigned int flags)
{
  SHA1_CONTEXT *hd = context;

  (void)flags;

  hd->h0 = 0x67452301;
  hd->h1 = 0xefcdab89;
  hd->h2 = 0x98badcfe;
  hd->h3 = 0x10325476;
  hd->h4 = 0xc3d2e1f0;

  hd->bctx.nblocks = 0;
  hd->bctx.nblocks_high = 0;
  hd->bctx.count = 0;
  hd->bctx.blocksize = 64;
  hd->bctx.bwrite = transform;

#ifdef USE_SSSE3
  hd->use_ssse3 = (_gcry_get_hw_features () & HWF_INTEL_SSSE3) != 0;
#endif
}


/* Round function macros. */
#define K1  0x5A827999L
#define K2  0x6ED9EBA1L
#define K3  0x8F1BBCDCL
#define K4  0xCA62C1D6L
#define F1(x,y,z)   ( z ^ ( x & ( y ^ z ) ) )
#define F2(x,y,z)   ( x ^ y ^ z )
#define F3(x,y,z)   ( ( x & y ) | ( z & ( x | y ) ) )
#define F4(x,y,z)   ( x ^ y ^ z )
#define M(i) ( tm =    x[ i    &0x0f]  \
                     ^ x[(i-14)&0x0f]  \
	 	     ^ x[(i-8) &0x0f]  \
                     ^ x[(i-3) &0x0f], \
                     (x[i&0x0f] = rol(tm, 1)))
#define R(a,b,c,d,e,f,k,m)  do { e += rol( a, 5 )     \
	                              + f( b, c, d )  \
		 		      + k	      \
			 	      + m;	      \
				 b = rol( b, 30 );    \
			       } while(0)


/*
 * Transform NBLOCKS of each 64 bytes (16 32-bit words) at DATA.
 */
static unsigned int
_transform (void *ctx, const unsigned char *data)
{
  SHA1_CONTEXT *hd = ctx;
  const u32 *idata = (const void *)data;
  register u32 a, b, c, d, e; /* Local copies of the chaining variables.  */
  register u32 tm;            /* Helper.  */
  u32 x[16];                  /* The array we work on. */

#define I(i) (x[i] = buf_get_be32(idata + i))

      /* Get the values of the chaining variables. */
      a = hd->h0;
      b = hd->h1;
      c = hd->h2;
      d = hd->h3;
      e = hd->h4;

      /* Transform. */
      R( a, b, c, d, e, F1, K1, I( 0) );
      R( e, a, b, c, d, F1, K1, I( 1) );
      R( d, e, a, b, c, F1, K1, I( 2) );
      R( c, d, e, a, b, F1, K1, I( 3) );
      R( b, c, d, e, a, F1, K1, I( 4) );
      R( a, b, c, d, e, F1, K1, I( 5) );
      R( e, a, b, c, d, F1, K1, I( 6) );
      R( d, e, a, b, c, F1, K1, I( 7) );
      R( c, d, e, a, b, F1, K1, I( 8) );
      R( b, c, d, e, a, F1, K1, I( 9) );
      R( a, b, c, d, e, F1, K1, I(10) );
      R( e, a, b, c, d, F1, K1, I(11) );
      R( d, e, a, b, c, F1, K1, I(12) );
      R( c, d, e, a, b, F1, K1, I(13) );
      R( b, c, d, e, a, F1, K1, I(14) );
      R( a, b, c, d, e, F1, K1, I(15) );
      R( e, a, b, c, d, F1, K1, M(16) );
      R( d, e, a, b, c, F1, K1, M(17) );
      R( c, d, e, a, b, F1, K1, M(18) );
      R( b, c, d, e, a, F1, K1, M(19) );
      R( a, b, c, d, e, F2, K2, M(20) );
      R( e, a, b, c, d, F2, K2, M(21) );
      R( d, e, a, b, c, F2, K2, M(22) );
      R( c, d, e, a, b, F2, K2, M(23) );
      R( b, c, d, e, a, F2, K2, M(24) );
      R( a, b, c, d, e, F2, K2, M(25) );
      R( e, a, b, c, d, F2, K2, M(26) );
      R( d, e, a, b, c, F2, K2, M(27) );
      R( c, d, e, a, b, F2, K2, M(28) );
      R( b, c, d, e, a, F2, K2, M(29) );
      R( a, b, c, d, e, F2, K2, M(30) );
      R( e, a, b, c, d, F2, K2, M(31) );
      R( d, e, a, b, c, F2, K2, M(32) );
      R( c, d, e, a, b, F2, K2, M(33) );
      R( b, c, d, e, a, F2, K2, M(34) );
      R( a, b, c, d, e, F2, K2, M(35) );
      R( e, a, b, c, d, F2, K2, M(36) );
      R( d, e, a, b, c, F2, K2, M(37) );
      R( c, d, e, a, b, F2, K2, M(38) );
      R( b, c, d, e, a, F2, K2, M(39) );
      R( a, b, c, d, e, F3, K3, M(40) );
      R( e, a, b, c, d, F3, K3, M(41) );
      R( d, e, a, b, c, F3, K3, M(42) );
      R( c, d, e, a, b, F3, K3, M(43) );
      R( b, c, d, e, a, F3, K3, M(44) );
      R( a, b, c, d, e, F3, K3, M(45) );
      R( e, a, b, c, d, F3, K3, M(46) );
      R( d, e, a, b, c, F3, K3, M(47) );
      R( c, d, e, a, b, F3, K3, M(48) );
      R( b, c, d, e, a, F3, K3, M(49) );
      R( a, b, c, d, e, F3, K3, M(50) );
      R( e, a, b, c, d, F3, K3, M(51) );
      R( d, e, a, b, c, F3, K3, M(52) );
      R( c, d, e, a, b, F3, K3, M(53) );
      R( b, c, d, e, a, F3, K3, M(54) );
      R( a, b, c, d, e, F3, K3, M(55) );
      R( e, a, b, c, d, F3, K3, M(56) );
      R( d, e, a, b, c, F3, K3, M(57) );
      R( c, d, e, a, b, F3, K3, M(58) );
      R( b, c, d, e, a, F3, K3, M(59) );
      R( a, b, c, d, e, F4, K4, M(60) );
      R( e, a, b, c, d, F4, K4, M(61) );
      R( d, e, a, b, c, F4, K4, M(62) );
      R( c, d, e, a, b, F4, K4, M(63) );
      R( b, c, d, e, a, F4, K4, M(64) );
      R( a, b, c, d, e, F4, K4, M(65) );
      R( e, a, b, c, d, F4, K4, M(66) );
      R( d, e, a, b, c, F4, K4, M(67) );
      R( c, d, e, a, b, F4, K4, M(68) );
      R( b, c, d, e, a, F4, K4, M(69) );
      R( a, b, c, d, e, F4, K4, M(70) );
      R( e, a, b, c, d, F4, K4, M(71) );
      R( d, e, a, b, c, F4, K4, M(72) );
      R( c, d, e, a, b, F4, K4, M(73) );
      R( b, c, d, e, a, F4, K4, M(74) );
      R( a, b, c, d, e, F4, K4, M(75) );
      R( e, a, b, c, d, F4, K4, M(76) );
      R( d, e, a, b, c, F4, K4, M(77) );
      R( c, d, e, a, b, F4, K4, M(78) );
      R( b, c, d, e, a, F4, K4, M(79) );

      /* Update the chaining variables. */
      hd->h0 += a;
      hd->h1 += b;
      hd->h2 += c;
      hd->h3 += d;
      hd->h4 += e;

  return /* burn_stack */ 88+4*sizeof(void*);
}


#ifdef USE_SSSE3
unsigned int
_gcry_sha1_transform_amd64_ssse3 (void *state, const unsigned char *data);
#endif


static unsigned int
transform (void *ctx, const unsigned char *data)
{
  SHA1_CONTEXT *hd = ctx;

#ifdef USE_SSSE3
  if (hd->use_ssse3)
    return _gcry_sha1_transform_amd64_ssse3 (&hd->h0, data)
           + 4 * sizeof(void*);
#endif

  return _transform (hd, data);
}


/* The routine final terminates the computation and
 * returns the digest.
 * The handle is prepared for a new cycle, but adding bytes to the
 * handle will the destroy the returned buffer.
 * Returns: 20 bytes representing the digest.
 */

static void
sha1_final(void *context)
{
  SHA1_CONTEXT *hd = context;
  u32 t, th, msb, lsb;
  unsigned char *p;
  unsigned int burn;

  _gcry_md_block_write (hd, NULL, 0); /* flush */;

  t = hd->bctx.nblocks;
  if (sizeof t == sizeof hd->bctx.nblocks)
    th = hd->bctx.nblocks_high;
  else
    th = hd->bctx.nblocks >> 32;

  /* multiply by 64 to make a byte count */
  lsb = t << 6;
  msb = (th << 6) | (t >> 26);
  /* add the count */
  t = lsb;
  if( (lsb += hd->bctx.count) < t )
    msb++;
  /* multiply by 8 to make a bit count */
  t = lsb;
  lsb <<= 3;
  msb <<= 3;
  msb |= t >> 29;

  if( hd->bctx.count < 56 )  /* enough room */
    {
      hd->bctx.buf[hd->bctx.count++] = 0x80; /* pad */
      while( hd->bctx.count < 56 )
        hd->bctx.buf[hd->bctx.count++] = 0;  /* pad */
    }
  else  /* need one extra block */
    {
      hd->bctx.buf[hd->bctx.count++] = 0x80; /* pad character */
      while( hd->bctx.count < 64 )
        hd->bctx.buf[hd->bctx.count++] = 0;
      _gcry_md_block_write(hd, NULL, 0);  /* flush */;
      memset(hd->bctx.buf, 0, 56 ); /* fill next block with zeroes */
    }
  /* append the 64 bit count */
  buf_put_be32(hd->bctx.buf + 56, msb);
  buf_put_be32(hd->bctx.buf + 60, lsb);
  burn = transform( hd, hd->bctx.buf );
  _gcry_burn_stack (burn);

  p = hd->bctx.buf;
#define X(a) do { *(u32*)p = be_bswap32(hd->h##a) ; p += 4; } while(0)
  X(0);
  X(1);
  X(2);
  X(3);
  X(4);
#undef X

}

static unsigned char *
sha1_read( void *context )
{
  SHA1_CONTEXT *hd = context;

  return hd->bctx.buf;
}

/****************
 * Shortcut functions which puts the hash value of the supplied buffer
 * into outbuf which must have a size of 20 bytes.
 */
void
_gcry_sha1_hash_buffer (void *outbuf, const void *buffer, size_t length)
{
  SHA1_CONTEXT hd;

  sha1_init (&hd, 0);
  _gcry_md_block_write (&hd, buffer, length);
  sha1_final (&hd);
  memcpy (outbuf, hd.bctx.buf, 20);
}


/* Variant of the above shortcut function using a multiple buffers.  */
void
_gcry_sha1_hash_buffers (void *outbuf, const gcry_buffer_t *iov, int iovcnt)
{
  SHA1_CONTEXT hd;

  sha1_init (&hd, 0);
  for (;iovcnt > 0; iov++, iovcnt--)
    _gcry_md_block_write (&hd,
                          (const char*)iov[0].data + iov[0].off, iov[0].len);
  sha1_final (&hd);
  memcpy (outbuf, hd.bctx.buf, 20);
}



/*
     Self-test section.
 */


static gpg_err_code_t
selftests_sha1 (int extended, selftest_report_func_t report)
{
  const char *what;
  const char *errtxt;

  what = "short string";
  errtxt = _gcry_hash_selftest_check_one
    (GCRY_MD_SHA1, 0,
     "abc", 3,
     "\xA9\x99\x3E\x36\x47\x06\x81\x6A\xBA\x3E"
     "\x25\x71\x78\x50\xC2\x6C\x9C\xD0\xD8\x9D", 20);
  if (errtxt)
    goto failed;

  if (extended)
    {
      what = "long string";
      errtxt = _gcry_hash_selftest_check_one
        (GCRY_MD_SHA1, 0,
         "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56,
         "\x84\x98\x3E\x44\x1C\x3B\xD2\x6E\xBA\xAE"
         "\x4A\xA1\xF9\x51\x29\xE5\xE5\x46\x70\xF1", 20);
      if (errtxt)
        goto failed;

      what = "one million \"a\"";
      errtxt = _gcry_hash_selftest_check_one
        (GCRY_MD_SHA1, 1,
         NULL, 0,
         "\x34\xAA\x97\x3C\xD4\xC4\xDA\xA4\xF6\x1E"
         "\xEB\x2B\xDB\xAD\x27\x31\x65\x34\x01\x6F", 20);
      if (errtxt)
        goto failed;
    }

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("digest", GCRY_MD_SHA1, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


/* Run a full self-test for ALGO and return 0 on success.  */
static gpg_err_code_t
run_selftests (int algo, int extended, selftest_report_func_t report)
{
  gpg_err_code_t ec;

  switch (algo)
    {
    case GCRY_MD_SHA1:
      ec = selftests_sha1 (extended, report);
      break;
    default:
      ec = GPG_ERR_DIGEST_ALGO;
      break;

    }
  return ec;
}




static unsigned char asn[15] = /* Object ID is 1.3.14.3.2.26 */
  { 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03,
    0x02, 0x1a, 0x05, 0x00, 0x04, 0x14 };

static gcry_md_oid_spec_t oid_spec_sha1[] =
  {
    /* iso.member-body.us.rsadsi.pkcs.pkcs-1.5 (sha1WithRSAEncryption) */
    { "1.2.840.113549.1.1.5" },
    /* iso.member-body.us.x9-57.x9cm.3 (dsaWithSha1)*/
    { "1.2.840.10040.4.3" },
    /* from NIST's OIW  (sha1) */
    { "1.3.14.3.2.26" },
    /* from NIST OIW (sha-1WithRSAEncryption) */
    { "1.3.14.3.2.29" },
    /* iso.member-body.us.ansi-x9-62.signatures.ecdsa-with-sha1 */
    { "1.2.840.10045.4.1" },
    { NULL },
  };

gcry_md_spec_t _gcry_digest_spec_sha1 =
  {
    GCRY_MD_SHA1, {0, 1},
    "SHA1", asn, DIM (asn), oid_spec_sha1, 20,
    sha1_init, _gcry_md_block_write, sha1_final, sha1_read,
    sizeof (SHA1_CONTEXT),
    run_selftests
  };

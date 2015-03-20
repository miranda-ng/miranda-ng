/* gostr3411-94.c - GOST R 34.11-94 hash function
 * Copyright (C) 2012 Free Software Foundation, Inc.
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


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g10lib.h"
#include "bithelp.h"
#include "cipher.h"
#include "hash-common.h"

#include "gost.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))

typedef struct {
  gcry_md_block_ctx_t bctx;
  GOST28147_context hd;
  byte h[32];
  byte sigma[32];
  u32 len;
} GOSTR3411_CONTEXT;

static unsigned int
transform (void *c, const unsigned char *data);

static void
gost3411_init (void *context, unsigned int flags)
{
  GOSTR3411_CONTEXT *hd = context;

  (void)flags;

  memset (&hd->hd, 0, sizeof(hd->hd));
  memset (hd->h, 0, 32);
  memset (hd->sigma, 0, 32);

  hd->bctx.nblocks = 0;
  hd->bctx.count = 0;
  hd->bctx.blocksize = 32;
  hd->bctx.bwrite = transform;
}

static void
do_p (unsigned char *p, unsigned char *u, unsigned char *v)
{
  int i, k;
  for (k = 0; k < 8; k++)
    {
      for (i = 0; i < 4; i++)
        {
          p[i + 4 * k] = u[8 * i + k] ^ v[8 * i + k];
        }
    }
}

static void
do_a (unsigned char *u)
{
  unsigned char temp[8];
  int i;
  memcpy (temp, u, 8);
  memmove (u, u+8, 24);
  for (i = 0; i < 8; i++)
    {
      u[24 + i] = u[i] ^ temp[i];
    }
}
/* apply do_a twice: 1 2 3 4 -> 3 4 1^2 2^3 */
static void
do_a2 (unsigned char *u)
{
  unsigned char temp[16];
  int i;
  memcpy (temp, u, 16);
  memcpy (u, u + 16, 16);
  for (i = 0; i < 8; i++)
    {
      u[16 + i] = temp[i] ^ temp[8 + i];
      u[24 + i] =    u[i] ^ temp[8 + i];
    }
}

static void
do_apply_c2 (unsigned char *u)
{
  u[ 1] ^= 0xff;
  u[ 3] ^= 0xff;
  u[ 5] ^= 0xff;
  u[ 7] ^= 0xff;

  u[ 8] ^= 0xff;
  u[10] ^= 0xff;
  u[12] ^= 0xff;
  u[14] ^= 0xff;

  u[17] ^= 0xff;
  u[18] ^= 0xff;
  u[20] ^= 0xff;
  u[23] ^= 0xff;

  u[24] ^= 0xff;
  u[28] ^= 0xff;
  u[29] ^= 0xff;
  u[31] ^= 0xff;
}

#define do_phi_step(e, i) \
  e[(0 + 2*i) % 32] ^= e[(2 + 2*i) % 32] ^ e[(4 + 2*i) % 32] ^ e[(6 + 2*i) % 32] ^ e[(24 + 2*i) % 32] ^ e[(30 + 2*i) % 32]; \
  e[(1 + 2*i) % 32] ^= e[(3 + 2*i) % 32] ^ e[(5 + 2*i) % 32] ^ e[(7 + 2*i) % 32] ^ e[(25 + 2*i) % 32] ^ e[(31 + 2*i) % 32];

static void
do_phi_submix (unsigned char *e, unsigned char *x, int round)
{
  int i;
  round *= 2;
  for (i = 0; i < 32; i++)
    {
      e[(i + round) % 32] ^= x[i];
    }
}

static void
do_add (unsigned char *s, unsigned char *a)
{
  unsigned temp = 0;
  int i;

  for (i = 0; i < 32; i++)
    {
      temp = s[i] + a[i] + (temp >> 8);
      s[i] = temp & 0xff;
    }
}

static unsigned int
do_hash_step (GOST28147_context *hd, unsigned char *h, unsigned char *m)
{
  unsigned char u[32], v[32], s[32];
  unsigned char k[32];
  unsigned int burn;
  int i;

  memcpy (u, h, 32);
  memcpy (v, m, 32);

  for (i = 0; i < 4; i++) {
    do_p (k, u, v);

    burn = _gcry_gost_enc_one (hd, k, s + i*8, h + i*8);

    do_a (u);
    if (i == 1)
      do_apply_c2 (u);
    do_a2 (v);
  }

  for (i = 0; i < 5; i++)
    {
      do_phi_step (s, 0);
      do_phi_step (s, 1);
      do_phi_step (s, 2);
      do_phi_step (s, 3);
      do_phi_step (s, 4);
      do_phi_step (s, 5);
      do_phi_step (s, 6);
      do_phi_step (s, 7);
      do_phi_step (s, 8);
      do_phi_step (s, 9);
      /* That is in total 12 + 1 + 61 = 74 = 16 * 4 + 10 rounds */
      if (i == 4)
        break;
      do_phi_step (s, 10);
      do_phi_step (s, 11);
      if (i == 0)
        do_phi_submix(s, m, 12);
      do_phi_step (s, 12);
      if (i == 0)
        do_phi_submix(s, h, 13);
      do_phi_step (s, 13);
      do_phi_step (s, 14);
      do_phi_step (s, 15);
    }

  memcpy (h, s+20, 12);
  memcpy (h+12, s, 20);

  return /* burn_stack */ 4 * sizeof(void*) /* func call (ret addr + args) */ +
                          4 * 32 + 2 * sizeof(int) /* stack */ +
                          max(burn /* _gcry_gost_enc_one */,
                              sizeof(void*) * 2 /* do_a2 call */ +
                              16 + sizeof(int) /* do_a2 stack */ );
}


static unsigned int
transform (void *ctx, const unsigned char *data)
{
  GOSTR3411_CONTEXT *hd = ctx;
  byte m[32];
  unsigned int burn;

  memcpy (m, data, 32);
  burn = do_hash_step (&hd->hd, hd->h, m);
  do_add (hd->sigma, m);

  return /* burn_stack */ burn + 3 * sizeof(void*) + 32 + 2 * sizeof(void*);
}

/*
   The routine finally terminates the computation and returns the
   digest.  The handle is prepared for a new cycle, but adding bytes
   to the handle will the destroy the returned buffer.  Returns: 32
   bytes with the message the digest.  */
static void
gost3411_final (void *context)
{
  GOSTR3411_CONTEXT *hd = context;
  size_t padlen = 0;
  byte l[32];
  int i;
  u32 nblocks;

  if (hd->bctx.count > 0)
    {
      padlen = 32 - hd->bctx.count;
      memset (hd->bctx.buf + hd->bctx.count, 0, padlen);
      hd->bctx.count += padlen;
      _gcry_md_block_write (hd, NULL, 0); /* flush */;
    }

  if (hd->bctx.count != 0)
    return; /* Something went wrong */

  memset (l, 0, 32);

  nblocks = hd->bctx.nblocks;
  if (padlen)
    {
      nblocks --;
      l[0] = 256 - padlen * 8;
    }

  for (i = 1; i < 32 && nblocks != 0; i++)
    {
      l[i] = nblocks % 256;
      nblocks /= 256;
    }

  do_hash_step (&hd->hd, hd->h, l);
  do_hash_step (&hd->hd, hd->h, hd->sigma);
}

static byte *
gost3411_read (void *context)
{
  GOSTR3411_CONTEXT *hd = context;

  return hd->h;
}
gcry_md_spec_t _gcry_digest_spec_gost3411_94 =
  {
    GCRY_MD_GOSTR3411_94, {0, 0},
    "GOSTR3411_94", NULL, 0, NULL, 32,
    gost3411_init, _gcry_md_block_write, gost3411_final, gost3411_read,
    sizeof (GOSTR3411_CONTEXT)
  };

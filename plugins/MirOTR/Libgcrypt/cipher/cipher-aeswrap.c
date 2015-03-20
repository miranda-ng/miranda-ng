/* cipher-aeswrap.c  - Generic AESWRAP mode implementation
 * Copyright (C) 2009, 2011 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
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
#include <errno.h>

#include "g10lib.h"
#include "cipher.h"
#include "ath.h"
#include "bufhelp.h"
#include "./cipher-internal.h"


/* Perform the AES-Wrap algorithm as specified by RFC3394.  We
   implement this as a mode usable with any cipher algorithm of
   blocksize 128.  */
gcry_err_code_t
_gcry_cipher_aeswrap_encrypt (gcry_cipher_hd_t c,
                              byte *outbuf, size_t outbuflen,
                              const byte *inbuf, size_t inbuflen )
{
  int j, x;
  size_t n, i;
  unsigned char *r, *a, *b;
  unsigned char t[8];
  unsigned int burn, nburn;

#if MAX_BLOCKSIZE < 8
#error Invalid block size
#endif
  /* We require a cipher with a 128 bit block length.  */
  if (c->spec->blocksize != 16)
    return GPG_ERR_INV_LENGTH;

  /* The output buffer must be able to hold the input data plus one
     additional block.  */
  if (outbuflen < inbuflen + 8)
    return GPG_ERR_BUFFER_TOO_SHORT;
  /* Input data must be multiple of 64 bits.  */
  if (inbuflen % 8)
    return GPG_ERR_INV_ARG;

  n = inbuflen / 8;

  /* We need at least two 64 bit blocks.  */
  if (n < 2)
    return GPG_ERR_INV_ARG;

  burn = 0;

  r = outbuf;
  a = outbuf;  /* We store A directly in OUTBUF.  */
  b = c->u_ctr.ctr;  /* B is also used to concatenate stuff.  */

  /* If an IV has been set we use that IV as the Alternative Initial
     Value; if it has not been set we use the standard value.  */
  if (c->marks.iv)
    memcpy (a, c->u_iv.iv, 8);
  else
    memset (a, 0xa6, 8);

  /* Copy the inbuf to the outbuf. */
  memmove (r+8, inbuf, inbuflen);

  memset (t, 0, sizeof t); /* t := 0.  */

  for (j = 0; j <= 5; j++)
    {
      for (i = 1; i <= n; i++)
        {
          /* B := AES_k( A | R[i] ) */
          memcpy (b, a, 8);
          memcpy (b+8, r+i*8, 8);
          nburn = c->spec->encrypt (&c->context.c, b, b);
          burn = nburn > burn ? nburn : burn;
          /* t := t + 1  */
	  for (x = 7; x >= 0; x--)
	    {
	      t[x]++;
	      if (t[x])
		break;
	    }
          /* A := MSB_64(B) ^ t */
	  buf_xor(a, b, t, 8);
          /* R[i] := LSB_64(B) */
          memcpy (r+i*8, b+8, 8);
        }
   }

  if (burn > 0)
    _gcry_burn_stack (burn + 4 * sizeof(void *));

  return 0;
}

/* Perform the AES-Unwrap algorithm as specified by RFC3394.  We
   implement this as a mode usable with any cipher algorithm of
   blocksize 128.  */
gcry_err_code_t
_gcry_cipher_aeswrap_decrypt (gcry_cipher_hd_t c,
                              byte *outbuf, size_t outbuflen,
                              const byte *inbuf, size_t inbuflen)
{
  int j, x;
  size_t n, i;
  unsigned char *r, *a, *b;
  unsigned char t[8];
  unsigned int burn, nburn;

#if MAX_BLOCKSIZE < 8
#error Invalid block size
#endif
  /* We require a cipher with a 128 bit block length.  */
  if (c->spec->blocksize != 16)
    return GPG_ERR_INV_LENGTH;

  /* The output buffer must be able to hold the input data minus one
     additional block.  Fixme: The caller has more restrictive checks
     - we may want to fix them for this mode.  */
  if (outbuflen + 8  < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  /* Input data must be multiple of 64 bits.  */
  if (inbuflen % 8)
    return GPG_ERR_INV_ARG;

  n = inbuflen / 8;

  /* We need at least three 64 bit blocks.  */
  if (n < 3)
    return GPG_ERR_INV_ARG;

  burn = 0;

  r = outbuf;
  a = c->lastiv;  /* We use c->LASTIV as buffer for A.  */
  b = c->u_ctr.ctr;     /* B is also used to concatenate stuff.  */

  /* Copy the inbuf to the outbuf and save A. */
  memcpy (a, inbuf, 8);
  memmove (r, inbuf+8, inbuflen-8);
  n--; /* Reduce to actual number of data blocks.  */

  /* t := 6 * n  */
  i = n * 6;  /* The range is valid because: n = inbuflen / 8 - 1.  */
  for (x=0; x < 8 && x < sizeof (i); x++)
    t[7-x] = i >> (8*x);
  for (; x < 8; x++)
    t[7-x] = 0;

  for (j = 5; j >= 0; j--)
    {
      for (i = n; i >= 1; i--)
        {
          /* B := AES_k^1( (A ^ t)| R[i] ) */
	  buf_xor(b, a, t, 8);
          memcpy (b+8, r+(i-1)*8, 8);
          nburn = c->spec->decrypt (&c->context.c, b, b);
          burn = nburn > burn ? nburn : burn;
          /* t := t - 1  */
	  for (x = 7; x >= 0; x--)
	    {
	      t[x]--;
	      if (t[x] != 0xff)
		break;
	    }
          /* A := MSB_64(B) */
          memcpy (a, b, 8);
          /* R[i] := LSB_64(B) */
          memcpy (r+(i-1)*8, b+8, 8);
        }
   }

  /* If an IV has been set we compare against this Alternative Initial
     Value; if it has not been set we compare against the standard IV.  */
  if (c->marks.iv)
    j = memcmp (a, c->u_iv.iv, 8);
  else
    {
      for (j=0, x=0; x < 8; x++)
        if (a[x] != 0xa6)
          {
            j=1;
            break;
          }
    }

  if (burn > 0)
    _gcry_burn_stack (burn + 4 * sizeof(void *));

  return j? GPG_ERR_CHECKSUM : 0;
}

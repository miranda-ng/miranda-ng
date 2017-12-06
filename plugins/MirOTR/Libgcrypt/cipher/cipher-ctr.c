/* cipher-ctr.c  - Generic CTR mode implementation
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003
 *               2005, 2007, 2008, 2009, 2011 Free Software Foundation, Inc.
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


gcry_err_code_t
_gcry_cipher_ctr_encrypt (gcry_cipher_hd_t c,
                          unsigned char *outbuf, size_t outbuflen,
                          const unsigned char *inbuf, size_t inbuflen)
{
  size_t n;
  int i;
  gcry_cipher_encrypt_t enc_fn = c->spec->encrypt;
  unsigned int blocksize = c->spec->blocksize;
  size_t nblocks;
  unsigned int burn, nburn;

  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;

  burn = 0;

  /* First process a left over encrypted counter.  */
  if (c->unused)
    {
      gcry_assert (c->unused < blocksize);
      i = blocksize - c->unused;
      n = c->unused > inbuflen ? inbuflen : c->unused;
      buf_xor(outbuf, inbuf, &c->lastiv[i], n);
      c->unused -= n;
      inbuf  += n;
      outbuf += n;
      inbuflen -= n;
    }

  /* Use a bulk method if available.  */
  nblocks = inbuflen / blocksize;
  if (nblocks && c->bulk.ctr_enc)
    {
      c->bulk.ctr_enc (&c->context.c, c->u_ctr.ctr, outbuf, inbuf, nblocks);
      inbuf  += nblocks * blocksize;
      outbuf += nblocks * blocksize;
      inbuflen -= nblocks * blocksize;
    }

  /* If we don't have a bulk method use the standard method.  We also
     use this method for the a remaining partial block.  */
  if (inbuflen)
    {
      unsigned char tmp[MAX_BLOCKSIZE];

      do {
        nburn = enc_fn (&c->context.c, tmp, c->u_ctr.ctr);
        burn = nburn > burn ? nburn : burn;

        for (i = blocksize; i > 0; i--)
          {
            c->u_ctr.ctr[i-1]++;
            if (c->u_ctr.ctr[i-1] != 0)
              break;
          }

        n = blocksize < inbuflen ? blocksize : inbuflen;
        buf_xor(outbuf, inbuf, tmp, n);

        inbuflen -= n;
        outbuf += n;
        inbuf += n;
      } while (inbuflen);

      /* Save the unused bytes of the counter.  */
      c->unused = blocksize - n;
      if (c->unused)
        buf_cpy (c->lastiv+n, tmp+n, c->unused);

      wipememory (tmp, sizeof tmp);
    }

  if (burn > 0)
    _gcry_burn_stack (burn + 4 * sizeof(void *));

  return 0;
}

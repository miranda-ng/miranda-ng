/* cipher-cbc.c  - Generic CBC mode implementation
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
#include "./cipher-internal.h"
#include "bufhelp.h"



gcry_err_code_t
_gcry_cipher_cbc_encrypt (gcry_cipher_hd_t c,
                          unsigned char *outbuf, size_t outbuflen,
                          const unsigned char *inbuf, size_t inbuflen)
{
  size_t n;
  unsigned char *ivp;
  int i;
  size_t blocksize = c->spec->blocksize;
  gcry_cipher_encrypt_t enc_fn = c->spec->encrypt;
  size_t nblocks = inbuflen / blocksize;
  unsigned int burn, nburn;

  if (outbuflen < ((c->flags & GCRY_CIPHER_CBC_MAC)? blocksize : inbuflen))
    return GPG_ERR_BUFFER_TOO_SHORT;

  if ((inbuflen % blocksize)
      && !(inbuflen > blocksize
           && (c->flags & GCRY_CIPHER_CBC_CTS)))
    return GPG_ERR_INV_LENGTH;

  burn = 0;

  if ((c->flags & GCRY_CIPHER_CBC_CTS) && inbuflen > blocksize)
    {
      if ((inbuflen % blocksize) == 0)
	nblocks--;
    }

  if (c->bulk.cbc_enc)
    {
      c->bulk.cbc_enc (&c->context.c, c->u_iv.iv, outbuf, inbuf, nblocks,
                       (c->flags & GCRY_CIPHER_CBC_MAC));
      inbuf  += nblocks * blocksize;
      if (!(c->flags & GCRY_CIPHER_CBC_MAC))
        outbuf += nblocks * blocksize;
    }
  else
    {
      ivp = c->u_iv.iv;

      for (n=0; n < nblocks; n++ )
        {
          buf_xor (outbuf, inbuf, ivp, blocksize);
          nburn = enc_fn ( &c->context.c, outbuf, outbuf );
          burn = nburn > burn ? nburn : burn;
          ivp = outbuf;
          inbuf  += blocksize;
          if (!(c->flags & GCRY_CIPHER_CBC_MAC))
            outbuf += blocksize;
        }

      if (ivp != c->u_iv.iv)
        buf_cpy (c->u_iv.iv, ivp, blocksize );
    }

  if ((c->flags & GCRY_CIPHER_CBC_CTS) && inbuflen > blocksize)
    {
      /* We have to be careful here, since outbuf might be equal to
         inbuf.  */
      size_t restbytes;
      unsigned char b;

      if ((inbuflen % blocksize) == 0)
        restbytes = blocksize;
      else
        restbytes = inbuflen % blocksize;

      outbuf -= blocksize;
      for (ivp = c->u_iv.iv, i = 0; i < restbytes; i++)
        {
          b = inbuf[i];
          outbuf[blocksize + i] = outbuf[i];
          outbuf[i] = b ^ *ivp++;
        }
      for (; i < blocksize; i++)
        outbuf[i] = 0 ^ *ivp++;

      nburn = enc_fn (&c->context.c, outbuf, outbuf);
      burn = nburn > burn ? nburn : burn;
      buf_cpy (c->u_iv.iv, outbuf, blocksize);
    }

  if (burn > 0)
    _gcry_burn_stack (burn + 4 * sizeof(void *));

  return 0;
}


gcry_err_code_t
_gcry_cipher_cbc_decrypt (gcry_cipher_hd_t c,
                          unsigned char *outbuf, size_t outbuflen,
                          const unsigned char *inbuf, size_t inbuflen)
{
  size_t n;
  int i;
  size_t blocksize = c->spec->blocksize;
  gcry_cipher_decrypt_t dec_fn = c->spec->decrypt;
  size_t nblocks = inbuflen / blocksize;
  unsigned int burn, nburn;

  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;

  if ((inbuflen % blocksize)
      && !(inbuflen > blocksize
           && (c->flags & GCRY_CIPHER_CBC_CTS)))
    return GPG_ERR_INV_LENGTH;

  burn = 0;

  if ((c->flags & GCRY_CIPHER_CBC_CTS) && inbuflen > blocksize)
    {
      nblocks--;
      if ((inbuflen % blocksize) == 0)
	nblocks--;
      buf_cpy (c->lastiv, c->u_iv.iv, blocksize);
    }

  if (c->bulk.cbc_dec)
    {
      c->bulk.cbc_dec (&c->context.c, c->u_iv.iv, outbuf, inbuf, nblocks);
      inbuf  += nblocks * blocksize;
      outbuf += nblocks * blocksize;
    }
  else
    {
      for (n=0; n < nblocks; n++ )
        {
          /* Because outbuf and inbuf might be the same, we must not overwrite
             the original ciphertext block.  We use LASTIV as intermediate
             storage here because it is not used otherwise.  */
          nburn = dec_fn ( &c->context.c, c->lastiv, inbuf );
          burn = nburn > burn ? nburn : burn;
          buf_xor_n_copy_2(outbuf, c->lastiv, c->u_iv.iv, inbuf, blocksize);
          inbuf  += blocksize;
          outbuf += blocksize;
        }
    }

  if ((c->flags & GCRY_CIPHER_CBC_CTS) && inbuflen > blocksize)
    {
      size_t restbytes;

      if ((inbuflen % blocksize) == 0)
        restbytes = blocksize;
      else
        restbytes = inbuflen % blocksize;

      buf_cpy (c->lastiv, c->u_iv.iv, blocksize );         /* Save Cn-2. */
      buf_cpy (c->u_iv.iv, inbuf + blocksize, restbytes ); /* Save Cn. */

      nburn = dec_fn ( &c->context.c, outbuf, inbuf );
      burn = nburn > burn ? nburn : burn;
      buf_xor(outbuf, outbuf, c->u_iv.iv, restbytes);

      buf_cpy (outbuf + blocksize, outbuf, restbytes);
      for(i=restbytes; i < blocksize; i++)
        c->u_iv.iv[i] = outbuf[i];
      nburn = dec_fn (&c->context.c, outbuf, c->u_iv.iv);
      burn = nburn > burn ? nburn : burn;
      buf_xor(outbuf, outbuf, c->lastiv, blocksize);
      /* c->lastiv is now really lastlastiv, does this matter? */
    }

  if (burn > 0)
    _gcry_burn_stack (burn + 4 * sizeof(void *));

  return 0;
}

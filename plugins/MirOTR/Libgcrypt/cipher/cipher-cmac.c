/* cmac.c - CMAC, Cipher-based MAC.
 * Copyright © 2013 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
#include "cipher.h"
#include "cipher-internal.h"
#include "bufhelp.h"


#define set_burn(burn, nburn) do { \
  unsigned int __nburn = (nburn); \
  (burn) = (burn) > __nburn ? (burn) : __nburn; } while (0)


static void
cmac_write (gcry_cipher_hd_t c, const byte * inbuf, size_t inlen)
{
  gcry_cipher_encrypt_t enc_fn = c->spec->encrypt;
  const unsigned int blocksize = c->spec->blocksize;
  byte outbuf[MAX_BLOCKSIZE];
  unsigned int burn = 0;
  unsigned int nblocks;

  if (!inlen || !inbuf)
    return;

  /* Last block is needed for cmac_final.  */
  if (c->unused + inlen <= blocksize)
    {
      for (; inlen && c->unused < blocksize; inlen--)
        c->lastiv[c->unused++] = *inbuf++;
      return;
    }

  if (c->unused)
    {
      for (; inlen && c->unused < blocksize; inlen--)
        c->lastiv[c->unused++] = *inbuf++;

      buf_xor (c->u_iv.iv, c->u_iv.iv, c->lastiv, blocksize);
      set_burn (burn, enc_fn (&c->context.c, c->u_iv.iv, c->u_iv.iv));

      c->unused = 0;
    }

  if (c->bulk.cbc_enc && inlen > blocksize)
    {
      nblocks = inlen / blocksize;
      nblocks -= (nblocks * blocksize == inlen);

      c->bulk.cbc_enc (&c->context.c, c->u_iv.iv, outbuf, inbuf, nblocks, 1);
      inbuf += nblocks * blocksize;
      inlen -= nblocks * blocksize;

      wipememory (outbuf, sizeof (outbuf));
    }
  else
    while (inlen > blocksize)
      {
        buf_xor (c->u_iv.iv, c->u_iv.iv, inbuf, blocksize);
        set_burn (burn, enc_fn (&c->context.c, c->u_iv.iv, c->u_iv.iv));
        inlen -= blocksize;
        inbuf += blocksize;
      }

  /* Make sure that last block is passed to cmac_final.  */
  if (inlen == 0)
    BUG ();

  for (; inlen && c->unused < blocksize; inlen--)
    c->lastiv[c->unused++] = *inbuf++;

  if (burn)
    _gcry_burn_stack (burn + 4 * sizeof (void *));
}


static void
cmac_generate_subkeys (gcry_cipher_hd_t c)
{
  const unsigned int blocksize = c->spec->blocksize;
  byte rb, carry, t, bi;
  unsigned int burn;
  int i, j;
  union
  {
    size_t _aligned;
    byte buf[MAX_BLOCKSIZE];
  } u;

  if (MAX_BLOCKSIZE < blocksize)
    BUG ();

  /* encrypt zero block */
  memset (u.buf, 0, blocksize);
  burn = c->spec->encrypt (&c->context.c, u.buf, u.buf);

  /* Currently supported blocksizes are 16 and 8. */
  rb = blocksize == 16 ? 0x87 : 0x1B /*blocksize == 8 */ ;

  for (j = 0; j < 2; j++)
    {
      /* Generate subkeys K1 and K2 */
      carry = 0;
      for (i = blocksize - 1; i >= 0; i--)
        {
          bi = u.buf[i];
          t = carry | (bi << 1);
          carry = bi >> 7;
          u.buf[i] = t & 0xff;
          c->u_mode.cmac.subkeys[j][i] = u.buf[i];
        }
      u.buf[blocksize - 1] ^= carry ? rb : 0;
      c->u_mode.cmac.subkeys[j][blocksize - 1] = u.buf[blocksize - 1];
    }

  wipememory (&u, sizeof (u));
  if (burn)
    _gcry_burn_stack (burn + 4 * sizeof (void *));
}


static void
cmac_final (gcry_cipher_hd_t c)
{
  const unsigned int blocksize = c->spec->blocksize;
  unsigned int count = c->unused;
  unsigned int burn;
  byte *subkey;

  if (count == blocksize)
    subkey = c->u_mode.cmac.subkeys[0];        /* K1 */
  else
    {
      subkey = c->u_mode.cmac.subkeys[1];      /* K2 */
      c->lastiv[count++] = 0x80;
      while (count < blocksize)
        c->lastiv[count++] = 0;
    }

  buf_xor (c->lastiv, c->lastiv, subkey, blocksize);

  buf_xor (c->u_iv.iv, c->u_iv.iv, c->lastiv, blocksize);
  burn = c->spec->encrypt (&c->context.c, c->u_iv.iv, c->u_iv.iv);
  if (burn)
    _gcry_burn_stack (burn + 4 * sizeof (void *));

  c->unused = 0;
}


static gcry_err_code_t
cmac_tag (gcry_cipher_hd_t c, unsigned char *tag, size_t taglen, int check)
{
  if (!tag || taglen == 0 || taglen > c->spec->blocksize)
    return GPG_ERR_INV_ARG;

  if (!c->u_mode.cmac.tag)
    {
      cmac_final (c);
      c->u_mode.cmac.tag = 1;
    }

  if (!check)
    {
      memcpy (tag, c->u_iv.iv, taglen);
      return GPG_ERR_NO_ERROR;
    }
  else
    {
      return buf_eq_const (tag, c->u_iv.iv, taglen) ?
        GPG_ERR_NO_ERROR : GPG_ERR_CHECKSUM;
    }
}


gcry_err_code_t
_gcry_cipher_cmac_authenticate (gcry_cipher_hd_t c,
                                const unsigned char *abuf, size_t abuflen)
{
  if (abuflen > 0 && !abuf)
    return GPG_ERR_INV_ARG;
  if (c->u_mode.cmac.tag)
    return GPG_ERR_INV_STATE;
  /* To support new blocksize, update cmac_generate_subkeys() then add new
     blocksize here. */
  if (c->spec->blocksize != 16 && c->spec->blocksize != 8)
    return GPG_ERR_INV_CIPHER_MODE;

  cmac_write (c, abuf, abuflen);

  return GPG_ERR_NO_ERROR;
}


gcry_err_code_t
_gcry_cipher_cmac_get_tag (gcry_cipher_hd_t c,
                           unsigned char *outtag, size_t taglen)
{
  return cmac_tag (c, outtag, taglen, 0);
}


gcry_err_code_t
_gcry_cipher_cmac_check_tag (gcry_cipher_hd_t c,
                             const unsigned char *intag, size_t taglen)
{
  return cmac_tag (c, (unsigned char *) intag, taglen, 1);
}

gcry_err_code_t
_gcry_cipher_cmac_set_subkeys (gcry_cipher_hd_t c)
{
  cmac_generate_subkeys (c);

  return GPG_ERR_NO_ERROR;
}

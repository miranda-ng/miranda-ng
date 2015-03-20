/* mac-cmac.c  -  CMAC glue for MAC API
 * Copyright © 2013 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
#include "./mac-internal.h"


static int
map_mac_algo_to_cipher (int mac_algo)
{
  switch (mac_algo)
    {
    default:
      return GCRY_CIPHER_NONE;
    case GCRY_MAC_CMAC_AES:
      return GCRY_CIPHER_AES;
    case GCRY_MAC_CMAC_3DES:
      return GCRY_CIPHER_3DES;
    case GCRY_MAC_CMAC_CAMELLIA:
      return GCRY_CIPHER_CAMELLIA128;
    case GCRY_MAC_CMAC_IDEA:
      return GCRY_CIPHER_IDEA;
    case GCRY_MAC_CMAC_CAST5:
      return GCRY_CIPHER_CAST5;
    case GCRY_MAC_CMAC_BLOWFISH:
      return GCRY_CIPHER_BLOWFISH;
    case GCRY_MAC_CMAC_TWOFISH:
      return GCRY_CIPHER_TWOFISH;
    case GCRY_MAC_CMAC_SERPENT:
      return GCRY_CIPHER_SERPENT128;
    case GCRY_MAC_CMAC_SEED:
      return GCRY_CIPHER_SEED;
    case GCRY_MAC_CMAC_RFC2268:
      return GCRY_CIPHER_RFC2268_128;
    case GCRY_MAC_CMAC_GOST28147:
      return GCRY_CIPHER_GOST28147;
    }
}


static gcry_err_code_t
cmac_open (gcry_mac_hd_t h)
{
  gcry_err_code_t err;
  gcry_cipher_hd_t hd;
  int secure = (h->magic == CTX_MAGIC_SECURE);
  int cipher_algo;
  unsigned int flags;

  cipher_algo = map_mac_algo_to_cipher (h->spec->algo);
  flags = (secure ? GCRY_CIPHER_SECURE : 0);

  err = _gcry_cipher_open_internal (&hd, cipher_algo, GCRY_CIPHER_MODE_CMAC,
                                    flags);
  if (err)
    return err;

  h->u.cmac.cipher_algo = cipher_algo;
  h->u.cmac.ctx = hd;
  h->u.cmac.blklen = _gcry_cipher_get_algo_blklen (cipher_algo);
  return 0;
}


static void
cmac_close (gcry_mac_hd_t h)
{
  _gcry_cipher_close (h->u.cmac.ctx);
  h->u.cmac.ctx = NULL;
}


static gcry_err_code_t
cmac_setkey (gcry_mac_hd_t h, const unsigned char *key, size_t keylen)
{
  return _gcry_cipher_setkey (h->u.cmac.ctx, key, keylen);
}


static gcry_err_code_t
cmac_reset (gcry_mac_hd_t h)
{
  return _gcry_cipher_reset (h->u.cmac.ctx);
}


static gcry_err_code_t
cmac_write (gcry_mac_hd_t h, const unsigned char *buf, size_t buflen)
{
  return _gcry_cipher_cmac_authenticate (h->u.cmac.ctx, buf, buflen);
}


static gcry_err_code_t
cmac_read (gcry_mac_hd_t h, unsigned char *outbuf, size_t * outlen)
{
  if (*outlen > h->u.cmac.blklen)
    *outlen = h->u.cmac.blklen;
  return _gcry_cipher_cmac_get_tag (h->u.cmac.ctx, outbuf, *outlen);
}


static gcry_err_code_t
cmac_verify (gcry_mac_hd_t h, const unsigned char *buf, size_t buflen)
{
  return _gcry_cipher_cmac_check_tag (h->u.cmac.ctx, buf, buflen);
}


static unsigned int
cmac_get_maclen (int algo)
{
  return _gcry_cipher_get_algo_blklen (map_mac_algo_to_cipher (algo));
}


static unsigned int
cmac_get_keylen (int algo)
{
  return _gcry_cipher_get_algo_keylen (map_mac_algo_to_cipher (algo));
}


static gcry_mac_spec_ops_t cmac_ops = {
  cmac_open,
  cmac_close,
  cmac_setkey,
  NULL,
  cmac_reset,
  cmac_write,
  cmac_read,
  cmac_verify,
  cmac_get_maclen,
  cmac_get_keylen
};


#if USE_BLOWFISH
gcry_mac_spec_t _gcry_mac_type_spec_cmac_blowfish = {
  GCRY_MAC_CMAC_BLOWFISH, {0, 0}, "CMAC_BLOWFISH",
  &cmac_ops
};
#endif
#if USE_DES
gcry_mac_spec_t _gcry_mac_type_spec_cmac_tripledes = {
  GCRY_MAC_CMAC_3DES, {0, 1}, "CMAC_3DES",
  &cmac_ops
};
#endif
#if USE_CAST5
gcry_mac_spec_t _gcry_mac_type_spec_cmac_cast5 = {
  GCRY_MAC_CMAC_CAST5, {0, 0}, "CMAC_CAST5",
  &cmac_ops
};
#endif
#if USE_AES
gcry_mac_spec_t _gcry_mac_type_spec_cmac_aes = {
  GCRY_MAC_CMAC_AES, {0, 1}, "CMAC_AES",
  &cmac_ops
};
#endif
#if USE_TWOFISH
gcry_mac_spec_t _gcry_mac_type_spec_cmac_twofish = {
  GCRY_MAC_CMAC_TWOFISH, {0, 0}, "CMAC_TWOFISH",
  &cmac_ops
};
#endif
#if USE_SERPENT
gcry_mac_spec_t _gcry_mac_type_spec_cmac_serpent = {
  GCRY_MAC_CMAC_SERPENT, {0, 0}, "CMAC_SERPENT",
  &cmac_ops
};
#endif
#if USE_RFC2268
gcry_mac_spec_t _gcry_mac_type_spec_cmac_rfc2268 = {
  GCRY_MAC_CMAC_RFC2268, {0, 0}, "CMAC_RFC2268",
  &cmac_ops
};
#endif
#if USE_SEED
gcry_mac_spec_t _gcry_mac_type_spec_cmac_seed = {
  GCRY_MAC_CMAC_SEED, {0, 0}, "CMAC_SEED",
  &cmac_ops
};
#endif
#if USE_CAMELLIA
gcry_mac_spec_t _gcry_mac_type_spec_cmac_camellia = {
  GCRY_MAC_CMAC_CAMELLIA, {0, 0}, "CMAC_CAMELLIA",
  &cmac_ops
};
#endif
#ifdef USE_IDEA
gcry_mac_spec_t _gcry_mac_type_spec_cmac_idea = {
  GCRY_MAC_CMAC_IDEA, {0, 0}, "CMAC_IDEA",
  &cmac_ops
};
#endif
#if USE_GOST28147
gcry_mac_spec_t _gcry_mac_type_spec_cmac_gost28147 = {
  GCRY_MAC_CMAC_GOST28147, {0, 0}, "CMAC_GOST28147",
  &cmac_ops
};
#endif

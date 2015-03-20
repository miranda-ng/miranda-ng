/* cipher.c  -	cipher dispatcher
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003
 *               2005, 2007, 2008, 2009, 2011 Free Software Foundation, Inc.
 * Copyright (C) 2013 g10 Code GmbH
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


/* This is the list of the default ciphers, which are included in
   libgcrypt.  */
static gcry_cipher_spec_t *cipher_list[] =
  {
#if USE_BLOWFISH
     &_gcry_cipher_spec_blowfish,
#endif
#if USE_DES
     &_gcry_cipher_spec_des,
     &_gcry_cipher_spec_tripledes,
#endif
#if USE_ARCFOUR
     &_gcry_cipher_spec_arcfour,
#endif
#if USE_CAST5
     &_gcry_cipher_spec_cast5,
#endif
#if USE_AES
     &_gcry_cipher_spec_aes,
     &_gcry_cipher_spec_aes192,
     &_gcry_cipher_spec_aes256,
#endif
#if USE_TWOFISH
     &_gcry_cipher_spec_twofish,
     &_gcry_cipher_spec_twofish128,
#endif
#if USE_SERPENT
     &_gcry_cipher_spec_serpent128,
     &_gcry_cipher_spec_serpent192,
     &_gcry_cipher_spec_serpent256,
#endif
#if USE_RFC2268
     &_gcry_cipher_spec_rfc2268_40,
     &_gcry_cipher_spec_rfc2268_128,
#endif
#if USE_SEED
     &_gcry_cipher_spec_seed,
#endif
#if USE_CAMELLIA
     &_gcry_cipher_spec_camellia128,
     &_gcry_cipher_spec_camellia192,
     &_gcry_cipher_spec_camellia256,
#endif
#ifdef USE_IDEA
     &_gcry_cipher_spec_idea,
#endif
#if USE_SALSA20
     &_gcry_cipher_spec_salsa20,
     &_gcry_cipher_spec_salsa20r12,
#endif
#if USE_GOST28147
     &_gcry_cipher_spec_gost28147,
#endif
    NULL
  };




static int
map_algo (int algo)
{
  return algo;
}


/* Return the spec structure for the cipher algorithm ALGO.  For
   an unknown algorithm NULL is returned.  */
static gcry_cipher_spec_t *
spec_from_algo (int algo)
{
  int idx;
  gcry_cipher_spec_t *spec;

  algo = map_algo (algo);

  for (idx = 0; (spec = cipher_list[idx]); idx++)
    if (algo == spec->algo)
      return spec;
  return NULL;
}


/* Lookup a cipher's spec by its name.  */
static gcry_cipher_spec_t *
spec_from_name (const char *name)
{
  gcry_cipher_spec_t *spec;
  int idx;
  const char **aliases;

  for (idx=0; (spec = cipher_list[idx]); idx++)
    {
      if (!stricmp (name, spec->name))
        return spec;
      if (spec->aliases)
        {
          for (aliases = spec->aliases; *aliases; aliases++)
            if (!stricmp (name, *aliases))
              return spec;
        }
    }

  return NULL;
}


/* Lookup a cipher's spec by its OID.  */
static gcry_cipher_spec_t *
spec_from_oid (const char *oid)
{
  gcry_cipher_spec_t *spec;
  gcry_cipher_oid_spec_t *oid_specs;
  int idx, j;

  for (idx=0; (spec = cipher_list[idx]); idx++)
    {
      oid_specs = spec->oids;
      if (oid_specs)
        {
          for (j = 0; oid_specs[j].oid; j++)
            if (!stricmp (oid, oid_specs[j].oid))
              return spec;
        }
    }

  return NULL;
}


/* Locate the OID in the oid table and return the spec or NULL if not
   found.  An optional "oid." or "OID." prefix in OID is ignored, the
   OID is expected to be in standard IETF dotted notation.  A pointer
   to the OID specification of the module implementing this algorithm
   is return in OID_SPEC unless passed as NULL.*/
static gcry_cipher_spec_t *
search_oid (const char *oid, gcry_cipher_oid_spec_t *oid_spec)
{
  gcry_cipher_spec_t *spec;
  int i;

  if (oid && ((! strncmp (oid, "oid.", 4))
	      || (! strncmp (oid, "OID.", 4))))
    oid += 4;

  spec = spec_from_oid (oid);
  if (spec && spec->oids)
    {
      for (i = 0; spec->oids[i].oid; i++)
	if (!stricmp (oid, spec->oids[i].oid))
	  {
	    if (oid_spec)
	      *oid_spec = spec->oids[i];
            return spec;
	  }
    }

  return NULL;
}


/* Map STRING to the cipher algorithm identifier.  Returns the
   algorithm ID of the cipher for the given name or 0 if the name is
   not known.  It is valid to pass NULL for STRING which results in a
   return value of 0. */
int
_gcry_cipher_map_name (const char *string)
{
  gcry_cipher_spec_t *spec;

  if (!string)
    return 0;

  /* If the string starts with a digit (optionally prefixed with
     either "OID." or "oid."), we first look into our table of ASN.1
     object identifiers to figure out the algorithm */

  spec = search_oid (string, NULL);
  if (spec)
    return spec->algo;

  spec = spec_from_name (string);
  if (spec)
    return spec->algo;

  return 0;
}


/* Given a STRING with an OID in dotted decimal notation, this
   function returns the cipher mode (GCRY_CIPHER_MODE_*) associated
   with that OID or 0 if no mode is known.  Passing NULL for string
   yields a return value of 0. */
int
_gcry_cipher_mode_from_oid (const char *string)
{
  gcry_cipher_spec_t *spec;
  gcry_cipher_oid_spec_t oid_spec;

  if (!string)
    return 0;

  spec = search_oid (string, &oid_spec);
  if (spec)
    return oid_spec.mode;

  return 0;
}


/* Map the cipher algorithm identifier ALGORITHM to a string
   representing this algorithm.  This string is the default name as
   used by Libgcrypt.  A "?" is returned for an unknown algorithm.
   NULL is never returned. */
const char *
_gcry_cipher_algo_name (int algorithm)
{
  gcry_cipher_spec_t *spec;

  spec = spec_from_algo (algorithm);
  return spec? spec->name : "?";
}


/* Flag the cipher algorithm with the identifier ALGORITHM as
   disabled.  There is no error return, the function does nothing for
   unknown algorithms.  Disabled algorithms are virtually not
   available in Libgcrypt.  This is not thread safe and should thus be
   called early. */
static void
disable_cipher_algo (int algo)
{
  gcry_cipher_spec_t *spec = spec_from_algo (algo);

  if (spec)
    spec->flags.disabled = 1;
}


/* Return 0 if the cipher algorithm with identifier ALGORITHM is
   available. Returns a basic error code value if it is not
   available.  */
static gcry_err_code_t
check_cipher_algo (int algorithm)
{
  gcry_cipher_spec_t *spec;

  spec = spec_from_algo (algorithm);
  if (spec && !spec->flags.disabled)
    return 0;

  return GPG_ERR_CIPHER_ALGO;
}


/* Return the standard length in bits of the key for the cipher
   algorithm with the identifier ALGORITHM.  */
static unsigned int
cipher_get_keylen (int algorithm)
{
  gcry_cipher_spec_t *spec;
  unsigned len = 0;

  spec = spec_from_algo (algorithm);
  if (spec)
    {
      len = spec->keylen;
      if (!len)
	log_bug ("cipher %d w/o key length\n", algorithm);
    }

  return len;
}


/* Return the block length of the cipher algorithm with the identifier
   ALGORITHM.  This function return 0 for an invalid algorithm.  */
static unsigned int
cipher_get_blocksize (int algorithm)
{
  gcry_cipher_spec_t *spec;
  unsigned len = 0;

  spec = spec_from_algo (algorithm);
  if (spec)
    {
      len = spec->blocksize;
      if (!len)
        log_bug ("cipher %d w/o blocksize\n", algorithm);
    }

  return len;
}


/*
   Open a cipher handle for use with cipher algorithm ALGORITHM, using
   the cipher mode MODE (one of the GCRY_CIPHER_MODE_*) and return a
   handle in HANDLE.  Put NULL into HANDLE and return an error code if
   something goes wrong.  FLAGS may be used to modify the
   operation.  The defined flags are:

   GCRY_CIPHER_SECURE:  allocate all internal buffers in secure memory.
   GCRY_CIPHER_ENABLE_SYNC:  Enable the sync operation as used in OpenPGP.
   GCRY_CIPHER_CBC_CTS:  Enable CTS mode.
   GCRY_CIPHER_CBC_MAC:  Enable MAC mode.

   Values for these flags may be combined using OR.
 */
gcry_err_code_t
_gcry_cipher_open (gcry_cipher_hd_t *handle,
                   int algo, int mode, unsigned int flags)
{
  gcry_err_code_t rc;
  gcry_cipher_hd_t h = NULL;

  if (mode >= GCRY_CIPHER_MODE_INTERNAL)
    rc = GPG_ERR_INV_CIPHER_MODE;
  else
    rc = _gcry_cipher_open_internal (&h, algo, mode, flags);

  *handle = rc ? NULL : h;

  return rc;
}


gcry_err_code_t
_gcry_cipher_open_internal (gcry_cipher_hd_t *handle,
			    int algo, int mode, unsigned int flags)
{
  int secure = (flags & GCRY_CIPHER_SECURE);
  gcry_cipher_spec_t *spec;
  gcry_cipher_hd_t h = NULL;
  gcry_err_code_t err;

  /* If the application missed to call the random poll function, we do
     it here to ensure that it is used once in a while. */
  _gcry_fast_random_poll ();

  spec = spec_from_algo (algo);
  if (!spec)
    err = GPG_ERR_CIPHER_ALGO;
  else if (spec->flags.disabled)
    err = GPG_ERR_CIPHER_ALGO;
  else
    err = 0;

  /* check flags */
  if ((! err)
      && ((flags & ~(0
		     | GCRY_CIPHER_SECURE
		     | GCRY_CIPHER_ENABLE_SYNC
		     | GCRY_CIPHER_CBC_CTS
		     | GCRY_CIPHER_CBC_MAC))
	  || (flags & GCRY_CIPHER_CBC_CTS & GCRY_CIPHER_CBC_MAC)))
    err = GPG_ERR_CIPHER_ALGO;

  /* check that a valid mode has been requested */
  if (! err)
    switch (mode)
      {
      case GCRY_CIPHER_MODE_CCM:
#ifdef HAVE_U64_TYPEDEF
	if (spec->blocksize != GCRY_CCM_BLOCK_LEN)
	  err = GPG_ERR_INV_CIPHER_MODE;
	if (!spec->encrypt || !spec->decrypt)
	  err = GPG_ERR_INV_CIPHER_MODE;
	break;
#else
        err = GPG_ERR_NOT_SUPPORTED;
#endif

      case GCRY_CIPHER_MODE_ECB:
      case GCRY_CIPHER_MODE_CBC:
      case GCRY_CIPHER_MODE_CFB:
      case GCRY_CIPHER_MODE_OFB:
      case GCRY_CIPHER_MODE_CTR:
      case GCRY_CIPHER_MODE_AESWRAP:
      case GCRY_CIPHER_MODE_CMAC:
      case GCRY_CIPHER_MODE_GCM:
	if (!spec->encrypt || !spec->decrypt)
	  err = GPG_ERR_INV_CIPHER_MODE;
	break;

      case GCRY_CIPHER_MODE_STREAM:
	if (!spec->stencrypt || !spec->stdecrypt)
	  err = GPG_ERR_INV_CIPHER_MODE;
	break;

      case GCRY_CIPHER_MODE_NONE:
        /* This mode may be used for debugging.  It copies the main
           text verbatim to the ciphertext.  We do not allow this in
           fips mode or if no debug flag has been set.  */
	if (fips_mode () || !_gcry_get_debug_flag (0))
          err = GPG_ERR_INV_CIPHER_MODE;
	break;

      default:
	err = GPG_ERR_INV_CIPHER_MODE;
      }

  /* Perform selftest here and mark this with a flag in cipher_table?
     No, we should not do this as it takes too long.  Further it does
     not make sense to exclude algorithms with failing selftests at
     runtime: If a selftest fails there is something seriously wrong with the system and thus we better die immediately. */

  if (! err)
    {
      size_t size = (sizeof (*h)
                     + 2 * spec->contextsize
                     - sizeof (cipher_context_alignment_t)
#ifdef NEED_16BYTE_ALIGNED_CONTEXT
                     + 15  /* Space for leading alignment gap.  */
#endif /*NEED_16BYTE_ALIGNED_CONTEXT*/
                     );

      if (secure)
	h = xtrycalloc_secure (1, size);
      else
	h = xtrycalloc (1, size);

      if (! h)
	err = gpg_err_code_from_syserror ();
      else
	{
          size_t off = 0;

#ifdef NEED_16BYTE_ALIGNED_CONTEXT
          if ( ((unsigned long)h & 0x0f) )
            {
              /* The malloced block is not aligned on a 16 byte
                 boundary.  Correct for this.  */
              off = 16 - ((unsigned long)h & 0x0f);
              h = (void*)((char*)h + off);
            }
#endif /*NEED_16BYTE_ALIGNED_CONTEXT*/

	  h->magic = secure ? CTX_MAGIC_SECURE : CTX_MAGIC_NORMAL;
          h->actual_handle_size = size - off;
          h->handle_offset = off;
	  h->spec = spec;
          h->algo = algo;
	  h->mode = mode;
	  h->flags = flags;

          /* Setup bulk encryption routines.  */
          switch (algo)
            {
#ifdef USE_AES
            case GCRY_CIPHER_AES128:
            case GCRY_CIPHER_AES192:
            case GCRY_CIPHER_AES256:
              h->bulk.cfb_enc = _gcry_aes_cfb_enc;
              h->bulk.cfb_dec = _gcry_aes_cfb_dec;
              h->bulk.cbc_enc = _gcry_aes_cbc_enc;
              h->bulk.cbc_dec = _gcry_aes_cbc_dec;
              h->bulk.ctr_enc = _gcry_aes_ctr_enc;
              break;
#endif /*USE_AES*/
#ifdef USE_BLOWFISH
	    case GCRY_CIPHER_BLOWFISH:
              h->bulk.cfb_dec = _gcry_blowfish_cfb_dec;
              h->bulk.cbc_dec = _gcry_blowfish_cbc_dec;
              h->bulk.ctr_enc = _gcry_blowfish_ctr_enc;
              break;
#endif /*USE_BLOWFISH*/
#ifdef USE_CAST5
	    case GCRY_CIPHER_CAST5:
              h->bulk.cfb_dec = _gcry_cast5_cfb_dec;
              h->bulk.cbc_dec = _gcry_cast5_cbc_dec;
              h->bulk.ctr_enc = _gcry_cast5_ctr_enc;
              break;
#endif /*USE_CAMELLIA*/
#ifdef USE_CAMELLIA
	    case GCRY_CIPHER_CAMELLIA128:
	    case GCRY_CIPHER_CAMELLIA192:
	    case GCRY_CIPHER_CAMELLIA256:
              h->bulk.cbc_dec = _gcry_camellia_cbc_dec;
              h->bulk.cfb_dec = _gcry_camellia_cfb_dec;
              h->bulk.ctr_enc = _gcry_camellia_ctr_enc;
              break;
#endif /*USE_CAMELLIA*/
#ifdef USE_SERPENT
	    case GCRY_CIPHER_SERPENT128:
	    case GCRY_CIPHER_SERPENT192:
	    case GCRY_CIPHER_SERPENT256:
              h->bulk.cbc_dec = _gcry_serpent_cbc_dec;
              h->bulk.cfb_dec = _gcry_serpent_cfb_dec;
              h->bulk.ctr_enc = _gcry_serpent_ctr_enc;
              break;
#endif /*USE_SERPENT*/
#ifdef USE_TWOFISH
	    case GCRY_CIPHER_TWOFISH:
	    case GCRY_CIPHER_TWOFISH128:
              h->bulk.cbc_dec = _gcry_twofish_cbc_dec;
              h->bulk.cfb_dec = _gcry_twofish_cfb_dec;
              h->bulk.ctr_enc = _gcry_twofish_ctr_enc;
              break;
#endif /*USE_TWOFISH*/

            default:
              break;
            }
	}
    }

  /* Done.  */

  *handle = err ? NULL : h;

  return gcry_error (err);
}


/* Release all resources associated with the cipher handle H. H may be
   NULL in which case this is a no-operation. */
void
_gcry_cipher_close (gcry_cipher_hd_t h)
{
  size_t off;

  if (!h)
    return;

  if ((h->magic != CTX_MAGIC_SECURE)
      && (h->magic != CTX_MAGIC_NORMAL))
    _gcry_fatal_error(GPG_ERR_INTERNAL,
		      "gcry_cipher_close: already closed/invalid handle");
  else
    h->magic = 0;

  /* We always want to wipe out the memory even when the context has
     been allocated in secure memory.  The user might have disabled
     secure memory or is using his own implementation which does not
     do the wiping.  To accomplish this we need to keep track of the
     actual size of this structure because we have no way to known
     how large the allocated area was when using a standard malloc. */
  off = h->handle_offset;
  wipememory (h, h->actual_handle_size);

  xfree ((char*)h - off);
}


/* Set the key to be used for the encryption context C to KEY with
   length KEYLEN.  The length should match the required length. */
static gcry_err_code_t
cipher_setkey (gcry_cipher_hd_t c, byte *key, size_t keylen)
{
  gcry_err_code_t rc;

  rc = c->spec->setkey (&c->context.c, key, keylen);
  if (!rc)
    {
      /* Duplicate initial context.  */
      memcpy ((void *) ((char *) &c->context.c + c->spec->contextsize),
              (void *) &c->context.c,
              c->spec->contextsize);
      c->marks.key = 1;

      switch (c->mode)
        {
        case GCRY_CIPHER_MODE_CMAC:
          _gcry_cipher_cmac_set_subkeys (c);
          break;

        case GCRY_CIPHER_MODE_GCM:
          _gcry_cipher_gcm_setkey (c);
          break;

        default:
          break;
        };
    }
  else
    c->marks.key = 0;

  return rc;
}


/* Set the IV to be used for the encryption context C to IV with
   length IVLEN.  The length should match the required length. */
static gcry_err_code_t
cipher_setiv (gcry_cipher_hd_t c, const byte *iv, size_t ivlen)
{
  /* GCM has its own IV handler */
  if (c->mode == GCRY_CIPHER_MODE_GCM)
    return _gcry_cipher_gcm_setiv (c, iv, ivlen);

  /* If the cipher has its own IV handler, we use only this one.  This
     is currently used for stream ciphers requiring a nonce.  */
  if (c->spec->setiv)
    {
      c->spec->setiv (&c->context.c, iv, ivlen);
      return 0;
    }

  memset (c->u_iv.iv, 0, c->spec->blocksize);
  if (iv)
    {
      if (ivlen != c->spec->blocksize)
        {
          log_info ("WARNING: cipher_setiv: ivlen=%u blklen=%u\n",
                    (unsigned int)ivlen, (unsigned int)c->spec->blocksize);
          fips_signal_error ("IV length does not match blocklength");
        }
      if (ivlen > c->spec->blocksize)
        ivlen = c->spec->blocksize;
      memcpy (c->u_iv.iv, iv, ivlen);
      c->marks.iv = 1;
    }
  else
      c->marks.iv = 0;
  c->unused = 0;

  return 0;
}


/* Reset the cipher context to the initial context.  This is basically
   the same as an release followed by a new. */
static void
cipher_reset (gcry_cipher_hd_t c)
{
  unsigned int marks_key;

  marks_key = c->marks.key;

  memcpy (&c->context.c,
	  (char *) &c->context.c + c->spec->contextsize,
	  c->spec->contextsize);
  memset (&c->marks, 0, sizeof c->marks);
  memset (c->u_iv.iv, 0, c->spec->blocksize);
  memset (c->lastiv, 0, c->spec->blocksize);
  memset (c->u_ctr.ctr, 0, c->spec->blocksize);
  c->unused = 0;

  c->marks.key = marks_key;

  switch (c->mode)
    {
    case GCRY_CIPHER_MODE_CMAC:
      /* Only clear 'tag' for cmac, keep subkeys. */
      c->u_mode.cmac.tag = 0;
      break;

    case GCRY_CIPHER_MODE_GCM:
      /* Only clear head of u_mode, keep ghash_key and gcm_table. */
      {
        byte *u_mode_pos = (void *)&c->u_mode;
        byte *ghash_key_pos = c->u_mode.gcm.u_ghash_key.key;
        size_t u_mode_head_length = ghash_key_pos - u_mode_pos;

        memset (&c->u_mode, 0, u_mode_head_length);
      }
      break;

#ifdef HAVE_U64_TYPEDEF
    case GCRY_CIPHER_MODE_CCM:
      memset (&c->u_mode.ccm, 0, sizeof c->u_mode.ccm);
      break;
#endif

    default:
      break; /* u_mode unused by other modes. */
    }
}



static gcry_err_code_t
do_ecb_crypt (gcry_cipher_hd_t c,
              unsigned char *outbuf, size_t outbuflen,
              const unsigned char *inbuf, size_t inbuflen,
              gcry_cipher_encrypt_t crypt_fn)
{
  unsigned int blocksize = c->spec->blocksize;
  size_t n, nblocks;
  unsigned int burn, nburn;

  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if ((inbuflen % blocksize))
    return GPG_ERR_INV_LENGTH;

  nblocks = inbuflen / blocksize;
  burn = 0;

  for (n=0; n < nblocks; n++ )
    {
      nburn = crypt_fn (&c->context.c, outbuf, inbuf);
      burn = nburn > burn ? nburn : burn;
      inbuf  += blocksize;
      outbuf += blocksize;
    }

  if (burn > 0)
    _gcry_burn_stack (burn + 4 * sizeof(void *));

  return 0;
}

static gcry_err_code_t
do_ecb_encrypt (gcry_cipher_hd_t c,
                unsigned char *outbuf, size_t outbuflen,
                const unsigned char *inbuf, size_t inbuflen)
{
  return do_ecb_crypt (c, outbuf, outbuflen, inbuf, inbuflen, c->spec->encrypt);
}

static gcry_err_code_t
do_ecb_decrypt (gcry_cipher_hd_t c,
                unsigned char *outbuf, size_t outbuflen,
                const unsigned char *inbuf, size_t inbuflen)
{
  return do_ecb_crypt (c, outbuf, outbuflen, inbuf, inbuflen, c->spec->decrypt);
}


/****************
 * Encrypt INBUF to OUTBUF with the mode selected at open.
 * inbuf and outbuf may overlap or be the same.
 * Depending on the mode some constraints apply to INBUFLEN.
 */
static gcry_err_code_t
cipher_encrypt (gcry_cipher_hd_t c, byte *outbuf, size_t outbuflen,
		const byte *inbuf, size_t inbuflen)
{
  gcry_err_code_t rc;

  switch (c->mode)
    {
    case GCRY_CIPHER_MODE_ECB:
      rc = do_ecb_encrypt (c, outbuf, outbuflen, inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_CBC:
      rc = _gcry_cipher_cbc_encrypt (c, outbuf, outbuflen, inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_CFB:
      rc = _gcry_cipher_cfb_encrypt (c, outbuf, outbuflen, inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_OFB:
      rc = _gcry_cipher_ofb_encrypt (c, outbuf, outbuflen, inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_CTR:
      rc = _gcry_cipher_ctr_encrypt (c, outbuf, outbuflen, inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_AESWRAP:
      rc = _gcry_cipher_aeswrap_encrypt (c, outbuf, outbuflen,
                                         inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_CCM:
      rc = _gcry_cipher_ccm_encrypt (c, outbuf, outbuflen, inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_CMAC:
      rc = GPG_ERR_INV_CIPHER_MODE;
      break;

    case GCRY_CIPHER_MODE_GCM:
      rc = _gcry_cipher_gcm_encrypt (c, outbuf, outbuflen, inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_STREAM:
      c->spec->stencrypt (&c->context.c,
                          outbuf, (byte*)/*arggg*/inbuf, inbuflen);
      rc = 0;
      break;

    case GCRY_CIPHER_MODE_NONE:
      if (fips_mode () || !_gcry_get_debug_flag (0))
        {
          fips_signal_error ("cipher mode NONE used");
          rc = GPG_ERR_INV_CIPHER_MODE;
        }
      else
        {
          if (inbuf != outbuf)
            memmove (outbuf, inbuf, inbuflen);
          rc = 0;
        }
      break;

    default:
      log_fatal ("cipher_encrypt: invalid mode %d\n", c->mode );
      rc = GPG_ERR_INV_CIPHER_MODE;
      break;
    }

  return rc;
}


/****************
 * Encrypt IN and write it to OUT.  If IN is NULL, in-place encryption has
 * been requested.
 */
gcry_err_code_t
_gcry_cipher_encrypt (gcry_cipher_hd_t h, void *out, size_t outsize,
                      const void *in, size_t inlen)
{
  gcry_err_code_t rc;

  if (!in)  /* Caller requested in-place encryption.  */
    {
      in = out;
      inlen = outsize;
    }

  rc = cipher_encrypt (h, out, outsize, in, inlen);

  /* Failsafe: Make sure that the plaintext will never make it into
     OUT if the encryption returned an error.  */
  if (rc && out)
    memset (out, 0x42, outsize);

  return rc;
}



/****************
 * Decrypt INBUF to OUTBUF with the mode selected at open.
 * inbuf and outbuf may overlap or be the same.
 * Depending on the mode some some contraints apply to INBUFLEN.
 */
static gcry_err_code_t
cipher_decrypt (gcry_cipher_hd_t c, byte *outbuf, size_t outbuflen,
                const byte *inbuf, size_t inbuflen)
{
  gcry_err_code_t rc;

  switch (c->mode)
    {
    case GCRY_CIPHER_MODE_ECB:
      rc = do_ecb_decrypt (c, outbuf, outbuflen, inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_CBC:
      rc = _gcry_cipher_cbc_decrypt (c, outbuf, outbuflen, inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_CFB:
      rc = _gcry_cipher_cfb_decrypt (c, outbuf, outbuflen, inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_OFB:
      rc = _gcry_cipher_ofb_encrypt (c, outbuf, outbuflen, inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_CTR:
      rc = _gcry_cipher_ctr_encrypt (c, outbuf, outbuflen, inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_AESWRAP:
      rc = _gcry_cipher_aeswrap_decrypt (c, outbuf, outbuflen,
                                         inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_CCM:
      rc = _gcry_cipher_ccm_decrypt (c, outbuf, outbuflen, inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_CMAC:
      rc = GPG_ERR_INV_CIPHER_MODE;
      break;

    case GCRY_CIPHER_MODE_GCM:
      rc = _gcry_cipher_gcm_decrypt (c, outbuf, outbuflen, inbuf, inbuflen);
      break;

    case GCRY_CIPHER_MODE_STREAM:
      c->spec->stdecrypt (&c->context.c,
                          outbuf, (byte*)/*arggg*/inbuf, inbuflen);
      rc = 0;
      break;

    case GCRY_CIPHER_MODE_NONE:
      if (fips_mode () || !_gcry_get_debug_flag (0))
        {
          fips_signal_error ("cipher mode NONE used");
          rc = GPG_ERR_INV_CIPHER_MODE;
        }
      else
        {
          if (inbuf != outbuf)
            memmove (outbuf, inbuf, inbuflen);
          rc = 0;
        }
      break;

    default:
      log_fatal ("cipher_decrypt: invalid mode %d\n", c->mode );
      rc = GPG_ERR_INV_CIPHER_MODE;
      break;
    }

  return rc;
}


gcry_err_code_t
_gcry_cipher_decrypt (gcry_cipher_hd_t h, void *out, size_t outsize,
                      const void *in, size_t inlen)
{
  if (!in) /* Caller requested in-place encryption. */
    {
      in = out;
      inlen = outsize;
    }

  return cipher_decrypt (h, out, outsize, in, inlen);
}



/****************
 * Used for PGP's somewhat strange CFB mode. Only works if
 * the corresponding flag is set.
 */
static void
cipher_sync (gcry_cipher_hd_t c)
{
  if ((c->flags & GCRY_CIPHER_ENABLE_SYNC) && c->unused)
    {
      memmove (c->u_iv.iv + c->unused,
               c->u_iv.iv, c->spec->blocksize - c->unused);
      memcpy (c->u_iv.iv,
              c->lastiv + c->spec->blocksize - c->unused, c->unused);
      c->unused = 0;
    }
}


gcry_err_code_t
_gcry_cipher_setkey (gcry_cipher_hd_t hd, const void *key, size_t keylen)
{
  return cipher_setkey (hd, (void*)key, keylen);
}


gcry_err_code_t
_gcry_cipher_setiv (gcry_cipher_hd_t hd, const void *iv, size_t ivlen)
{
  gcry_err_code_t rc = 0;

  switch (hd->mode)
    {
      case GCRY_CIPHER_MODE_CCM:
        rc = _gcry_cipher_ccm_set_nonce (hd, iv, ivlen);
        break;

      default:
        rc = cipher_setiv (hd, iv, ivlen);
        break;
    }
  return rc;
}

/* Set counter for CTR mode.  (CTR,CTRLEN) must denote a buffer of
   block size length, or (NULL,0) to set the CTR to the all-zero
   block. */
gpg_err_code_t
_gcry_cipher_setctr (gcry_cipher_hd_t hd, const void *ctr, size_t ctrlen)
{
  if (ctr && ctrlen == hd->spec->blocksize)
    {
      memcpy (hd->u_ctr.ctr, ctr, hd->spec->blocksize);
      hd->unused = 0;
    }
  else if (!ctr || !ctrlen)
    {
      memset (hd->u_ctr.ctr, 0, hd->spec->blocksize);
      hd->unused = 0;
    }
  else
    return GPG_ERR_INV_ARG;

  return 0;
}


gcry_err_code_t
_gcry_cipher_authenticate (gcry_cipher_hd_t hd, const void *abuf,
                           size_t abuflen)
{
  gcry_err_code_t rc;

  switch (hd->mode)
    {
    case GCRY_CIPHER_MODE_CCM:
      rc = _gcry_cipher_ccm_authenticate (hd, abuf, abuflen);
      break;

    case GCRY_CIPHER_MODE_CMAC:
      rc = _gcry_cipher_cmac_authenticate (hd, abuf, abuflen);
      break;

    case GCRY_CIPHER_MODE_GCM:
      rc = _gcry_cipher_gcm_authenticate (hd, abuf, abuflen);
      break;

    default:
      log_error ("gcry_cipher_authenticate: invalid mode %d\n", hd->mode);
      rc = GPG_ERR_INV_CIPHER_MODE;
      break;
    }

  return rc;
}


gcry_err_code_t
_gcry_cipher_gettag (gcry_cipher_hd_t hd, void *outtag, size_t taglen)
{
  gcry_err_code_t rc;

  switch (hd->mode)
    {
    case GCRY_CIPHER_MODE_CCM:
      rc = _gcry_cipher_ccm_get_tag (hd, outtag, taglen);
      break;

    case GCRY_CIPHER_MODE_CMAC:
      rc = _gcry_cipher_cmac_get_tag (hd, outtag, taglen);
      break;

    case GCRY_CIPHER_MODE_GCM:
      rc = _gcry_cipher_gcm_get_tag (hd, outtag, taglen);
      break;

    default:
      log_error ("gcry_cipher_gettag: invalid mode %d\n", hd->mode);
      rc = GPG_ERR_INV_CIPHER_MODE;
      break;
    }

  return rc;
}


gcry_err_code_t
_gcry_cipher_checktag (gcry_cipher_hd_t hd, const void *intag, size_t taglen)
{
  gcry_err_code_t rc;

  switch (hd->mode)
    {
    case GCRY_CIPHER_MODE_CCM:
      rc = _gcry_cipher_ccm_check_tag (hd, intag, taglen);
      break;

    case GCRY_CIPHER_MODE_CMAC:
      rc = _gcry_cipher_cmac_check_tag (hd, intag, taglen);
      break;

    case GCRY_CIPHER_MODE_GCM:
      rc = _gcry_cipher_gcm_check_tag (hd, intag, taglen);
      break;

    default:
      log_error ("gcry_cipher_checktag: invalid mode %d\n", hd->mode);
      rc = GPG_ERR_INV_CIPHER_MODE;
      break;
    }

  return rc;
}


gcry_err_code_t
_gcry_cipher_ctl (gcry_cipher_hd_t h, int cmd, void *buffer, size_t buflen)
{
  gcry_err_code_t rc = 0;

  switch (cmd)
    {
    case GCRYCTL_RESET:
      cipher_reset (h);
      break;

    case GCRYCTL_CFB_SYNC:
      cipher_sync( h );
      break;

    case GCRYCTL_SET_CBC_CTS:
      if (buflen)
	if (h->flags & GCRY_CIPHER_CBC_MAC)
	  rc = GPG_ERR_INV_FLAG;
	else
	  h->flags |= GCRY_CIPHER_CBC_CTS;
      else
	h->flags &= ~GCRY_CIPHER_CBC_CTS;
      break;

    case GCRYCTL_SET_CBC_MAC:
      if (buflen)
	if (h->flags & GCRY_CIPHER_CBC_CTS)
	  rc = GPG_ERR_INV_FLAG;
	else
	  h->flags |= GCRY_CIPHER_CBC_MAC;
      else
	h->flags &= ~GCRY_CIPHER_CBC_MAC;
      break;

    case GCRYCTL_SET_CCM_LENGTHS:
#ifdef HAVE_U64_TYPEDEF
      {
        u64 params[3];
        size_t encryptedlen;
        size_t aadlen;
        size_t authtaglen;

        if (h->mode != GCRY_CIPHER_MODE_CCM)
          return gcry_error (GPG_ERR_INV_CIPHER_MODE);

        if (!buffer || buflen != 3 * sizeof(u64))
          return gcry_error (GPG_ERR_INV_ARG);

        /* This command is used to pass additional length parameters needed
           by CCM mode to initialize CBC-MAC.  */
        memcpy (params, buffer, sizeof(params));
        encryptedlen = params[0];
        aadlen = params[1];
        authtaglen = params[2];

        rc = _gcry_cipher_ccm_set_lengths (h, encryptedlen, aadlen, authtaglen);
      }
#else
      rc = GPG_ERR_NOT_SUPPORTED;
#endif
      break;

    case GCRYCTL_DISABLE_ALGO:
      /* This command expects NULL for H and BUFFER to point to an
         integer with the algo number.  */
      if( h || !buffer || buflen != sizeof(int) )
	return gcry_error (GPG_ERR_CIPHER_ALGO);
      disable_cipher_algo( *(int*)buffer );
      break;

    case 61:  /* Disable weak key detection (private).  */
      if (h->spec->set_extra_info)
        rc = h->spec->set_extra_info
          (&h->context.c, CIPHER_INFO_NO_WEAK_KEY, NULL, 0);
      else
        rc = GPG_ERR_NOT_SUPPORTED;
      break;

    case 62: /* Return current input vector (private).  */
      /* This is the input block as used in CFB and OFB mode which has
         initially been set as IV.  The returned format is:
           1 byte  Actual length of the block in bytes.
           n byte  The block.
         If the provided buffer is too short, an error is returned. */
      if (buflen < (1 + h->spec->blocksize))
        rc = GPG_ERR_TOO_SHORT;
      else
        {
          unsigned char *ivp;
          unsigned char *dst = buffer;
          int n = h->unused;

          if (!n)
            n = h->spec->blocksize;
          gcry_assert (n <= h->spec->blocksize);
          *dst++ = n;
          ivp = h->u_iv.iv + h->spec->blocksize - n;
          while (n--)
            *dst++ = *ivp++;
        }
      break;

    default:
      rc = GPG_ERR_INV_OP;
    }

  return rc;
}


/* Return information about the cipher handle H.  CMD is the kind of
   information requested.  BUFFER and NBYTES are reserved for now.

   There are no values for CMD yet defined.

   The function always returns GPG_ERR_INV_OP.

 */
gcry_err_code_t
_gcry_cipher_info (gcry_cipher_hd_t h, int cmd, void *buffer, size_t *nbytes)
{
  gcry_err_code_t rc = 0;

  (void)h;
  (void)buffer;
  (void)nbytes;

  switch (cmd)
    {
    default:
      rc = GPG_ERR_INV_OP;
    }

  return rc;
}

/* Return information about the given cipher algorithm ALGO.

   WHAT select the kind of information returned:

    GCRYCTL_GET_KEYLEN:
  	Return the length of the key.  If the algorithm ALGO
  	supports multiple key lengths, the maximum supported key length
  	is returned.  The key length is returned as number of octets.
  	BUFFER and NBYTES must be zero.

    GCRYCTL_GET_BLKLEN:
  	Return the blocklength of the algorithm ALGO counted in octets.
  	BUFFER and NBYTES must be zero.

    GCRYCTL_TEST_ALGO:
  	Returns 0 if the specified algorithm ALGO is available for use.
  	BUFFER and NBYTES must be zero.

   Note: Because this function is in most cases used to return an
   integer value, we can make it easier for the caller to just look at
   the return value.  The caller will in all cases consult the value
   and thereby detecting whether a error occurred or not (i.e. while
   checking the block size)
 */
gcry_err_code_t
_gcry_cipher_algo_info (int algo, int what, void *buffer, size_t *nbytes)
{
  gcry_err_code_t rc = 0;
  unsigned int ui;

  switch (what)
    {
    case GCRYCTL_GET_KEYLEN:
      if (buffer || (! nbytes))
	rc = GPG_ERR_CIPHER_ALGO;
      else
	{
	  ui = cipher_get_keylen (algo);
	  if ((ui > 0) && (ui <= 512))
	    *nbytes = (size_t) ui / 8;
	  else
	    /* The only reason for an error is an invalid algo.  */
	    rc = GPG_ERR_CIPHER_ALGO;
	}
      break;

    case GCRYCTL_GET_BLKLEN:
      if (buffer || (! nbytes))
	rc = GPG_ERR_CIPHER_ALGO;
      else
	{
	  ui = cipher_get_blocksize (algo);
	  if ((ui > 0) && (ui < 10000))
	    *nbytes = ui;
	  else
            {
              /* The only reason is an invalid algo or a strange
                 blocksize.  */
              rc = GPG_ERR_CIPHER_ALGO;
            }
	}
      break;

    case GCRYCTL_TEST_ALGO:
      if (buffer || nbytes)
	rc = GPG_ERR_INV_ARG;
      else
	rc = check_cipher_algo (algo);
      break;

      default:
	rc = GPG_ERR_INV_OP;
    }

  return rc;
}


/* This function returns length of the key for algorithm ALGO.  If the
   algorithm supports multiple key lengths, the maximum supported key
   length is returned.  On error 0 is returned.  The key length is
   returned as number of octets.

   This is a convenience functions which should be preferred over
   gcry_cipher_algo_info because it allows for proper type
   checking.  */
size_t
_gcry_cipher_get_algo_keylen (int algo)
{
  size_t n;

  if (_gcry_cipher_algo_info (algo, GCRYCTL_GET_KEYLEN, NULL, &n))
    n = 0;
  return n;
}


/* This functions returns the blocklength of the algorithm ALGO
   counted in octets.  On error 0 is returned.

   This is a convenience functions which should be preferred over
   gcry_cipher_algo_info because it allows for proper type
   checking.  */
size_t
_gcry_cipher_get_algo_blklen (int algo)
{
  size_t n;

  if (_gcry_cipher_algo_info( algo, GCRYCTL_GET_BLKLEN, NULL, &n))
    n = 0;
  return n;
}


/* Explicitly initialize this module.  */
gcry_err_code_t
_gcry_cipher_init (void)
{
  return 0;
}


/* Run the selftests for cipher algorithm ALGO with optional reporting
   function REPORT.  */
gpg_error_t
_gcry_cipher_selftest (int algo, int extended, selftest_report_func_t report)
{
  gcry_err_code_t ec = 0;
  gcry_cipher_spec_t *spec;

  spec = spec_from_algo (algo);
  if (spec && !spec->flags.disabled && spec->selftest)
    ec = spec->selftest (algo, extended, report);
  else
    {
      ec = GPG_ERR_CIPHER_ALGO;
      if (report)
        report ("cipher", algo, "module",
                (spec && !spec->flags.disabled)?
                "no selftest available" :
                spec? "algorithm disabled" : "algorithm not found");
    }

  return gpg_error (ec);
}

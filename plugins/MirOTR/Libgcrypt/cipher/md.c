/* md.c  -  message digest dispatcher
 * Copyright (C) 1998, 1999, 2002, 2003, 2006,
 *               2008 Free Software Foundation, Inc.
 * Copyright (C) 2013, 2014 g10 Code GmbH
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

#include "rmd.h"


/* This is the list of the digest implementations included in
   libgcrypt.  */
static gcry_md_spec_t *digest_list[] =
  {
#if USE_CRC
     &_gcry_digest_spec_crc32,
     &_gcry_digest_spec_crc32_rfc1510,
     &_gcry_digest_spec_crc24_rfc2440,
#endif
#if USE_SHA1
     &_gcry_digest_spec_sha1,
#endif
#if USE_SHA256
     &_gcry_digest_spec_sha256,
     &_gcry_digest_spec_sha224,
#endif
#if USE_SHA512
     &_gcry_digest_spec_sha512,
     &_gcry_digest_spec_sha384,
#endif
#ifdef USE_GOST_R_3411_94
     &_gcry_digest_spec_gost3411_94,
#endif
#ifdef USE_GOST_R_3411_12
     &_gcry_digest_spec_stribog_256,
     &_gcry_digest_spec_stribog_512,
#endif
#if USE_WHIRLPOOL
     &_gcry_digest_spec_whirlpool,
#endif
#if USE_RMD160
     &_gcry_digest_spec_rmd160,
#endif
#if USE_TIGER
     &_gcry_digest_spec_tiger,
     &_gcry_digest_spec_tiger1,
     &_gcry_digest_spec_tiger2,
#endif
#if USE_MD5
     &_gcry_digest_spec_md5,
#endif
#if USE_MD4
     &_gcry_digest_spec_md4,
#endif
    NULL
  };


typedef struct gcry_md_list
{
  gcry_md_spec_t *spec;
  struct gcry_md_list *next;
  size_t actual_struct_size;     /* Allocated size of this structure. */
  PROPERLY_ALIGNED_TYPE context;
} GcryDigestEntry;

/* This structure is put right after the gcry_md_hd_t buffer, so that
 * only one memory block is needed. */
struct gcry_md_context
{
  int  magic;
  size_t actual_handle_size;     /* Allocated size of this handle. */
  FILE  *debug;
  struct {
    unsigned int secure: 1;
    unsigned int finalized:1;
    unsigned int bugemu1:1;
  } flags;
  GcryDigestEntry *list;
  byte *macpads;
  int macpads_Bsize;             /* Blocksize as used for the HMAC pads. */
};


#define CTX_MAGIC_NORMAL 0x11071961
#define CTX_MAGIC_SECURE 0x16917011

static gcry_err_code_t md_enable (gcry_md_hd_t hd, int algo);
static void md_close (gcry_md_hd_t a);
static void md_write (gcry_md_hd_t a, const void *inbuf, size_t inlen);
static byte *md_read( gcry_md_hd_t a, int algo );
static int md_get_algo( gcry_md_hd_t a );
static int md_digest_length( int algo );
static void md_start_debug ( gcry_md_hd_t a, const char *suffix );
static void md_stop_debug ( gcry_md_hd_t a );



static int
map_algo (int algo)
{
  return algo;
}


/* Return the spec structure for the hash algorithm ALGO.  For an
   unknown algorithm NULL is returned.  */
static gcry_md_spec_t *
spec_from_algo (int algo)
{
  int idx;
  gcry_md_spec_t *spec;

  algo = map_algo (algo);

  for (idx = 0; (spec = digest_list[idx]); idx++)
    if (algo == spec->algo)
      return spec;
  return NULL;
}


/* Lookup a hash's spec by its name.  */
static gcry_md_spec_t *
spec_from_name (const char *name)
{
  gcry_md_spec_t *spec;
  int idx;

  for (idx=0; (spec = digest_list[idx]); idx++)
    {
      if (!stricmp (name, spec->name))
        return spec;
    }

  return NULL;
}


/* Lookup a hash's spec by its OID.  */
static gcry_md_spec_t *
spec_from_oid (const char *oid)
{
  gcry_md_spec_t *spec;
  gcry_md_oid_spec_t *oid_specs;
  int idx, j;

  for (idx=0; (spec = digest_list[idx]); idx++)
    {
      oid_specs = spec->oids;
      if (oid_specs)
        {
          for (j = 0; oid_specs[j].oidstring; j++)
            if (!stricmp (oid, oid_specs[j].oidstring))
              return spec;
        }
    }

  return NULL;
}


static gcry_md_spec_t *
search_oid (const char *oid, gcry_md_oid_spec_t *oid_spec)
{
  gcry_md_spec_t *spec;
  int i;

  if (oid && ((! strncmp (oid, "oid.", 4))
	      || (! strncmp (oid, "OID.", 4))))
    oid += 4;

  spec = spec_from_oid (oid);
  if (spec && spec->oids)
    {
      for (i = 0; spec->oids[i].oidstring; i++)
	if (!stricmp (oid, spec->oids[i].oidstring))
	  {
	    if (oid_spec)
	      *oid_spec = spec->oids[i];
	    return spec;
	  }
    }

  return NULL;
}


/****************
 * Map a string to the digest algo
 */
int
_gcry_md_map_name (const char *string)
{
  gcry_md_spec_t *spec;

  if (!string)
    return 0;

  /* If the string starts with a digit (optionally prefixed with
     either "OID." or "oid."), we first look into our table of ASN.1
     object identifiers to figure out the algorithm */
  spec = search_oid (string, NULL);
  if (spec)
    return spec->algo;

  /* Not found, search a matching digest name.  */
  spec = spec_from_name (string);
  if (spec)
    return spec->algo;

  return 0;
}


/****************
 * This function simply returns the name of the algorithm or some constant
 * string when there is no algo.  It will never return NULL.
 * Use	the macro gcry_md_test_algo() to check whether the algorithm
 * is valid.
 */
const char *
_gcry_md_algo_name (int algorithm)
{
  gcry_md_spec_t *spec;

  spec = spec_from_algo (algorithm);
  return spec ? spec->name : "?";
}


static gcry_err_code_t
check_digest_algo (int algorithm)
{
  gcry_md_spec_t *spec;

  spec = spec_from_algo (algorithm);
  if (spec && !spec->flags.disabled)
    return 0;

  return GPG_ERR_DIGEST_ALGO;

}


/****************
 * Open a message digest handle for use with algorithm ALGO.
 * More algorithms may be added by md_enable(). The initial algorithm
 * may be 0.
 */
static gcry_err_code_t
md_open (gcry_md_hd_t *h, int algo, unsigned int flags)
{
  gcry_err_code_t err = 0;
  int secure = !!(flags & GCRY_MD_FLAG_SECURE);
  int hmac =   !!(flags & GCRY_MD_FLAG_HMAC);
  int bufsize = secure ? 512 : 1024;
  struct gcry_md_context *ctx;
  gcry_md_hd_t hd;
  size_t n;

  /* Allocate a memory area to hold the caller visible buffer with it's
   * control information and the data required by this module. Set the
   * context pointer at the beginning to this area.
   * We have to use this strange scheme because we want to hide the
   * internal data but have a variable sized buffer.
   *
   *	+---+------+---........------+-------------+
   *	!ctx! bctl !  buffer	     ! private	   !
   *	+---+------+---........------+-------------+
   *	  !			      ^
   *	  !---------------------------!
   *
   * We have to make sure that private is well aligned.
   */
  n = sizeof (struct gcry_md_handle) + bufsize;
  n = ((n + sizeof (PROPERLY_ALIGNED_TYPE) - 1)
       / sizeof (PROPERLY_ALIGNED_TYPE)) * sizeof (PROPERLY_ALIGNED_TYPE);

  /* Allocate and set the Context pointer to the private data */
  if (secure)
    hd = xtrymalloc_secure (n + sizeof (struct gcry_md_context));
  else
    hd = xtrymalloc (n + sizeof (struct gcry_md_context));

  if (! hd)
    err = gpg_err_code_from_errno (errno);

  if (! err)
    {
      hd->ctx = ctx = (struct gcry_md_context *) ((char *) hd + n);
      /* Setup the globally visible data (bctl in the diagram).*/
      hd->bufsize = n - sizeof (struct gcry_md_handle) + 1;
      hd->bufpos = 0;

      /* Initialize the private data. */
      memset (hd->ctx, 0, sizeof *hd->ctx);
      ctx->magic = secure ? CTX_MAGIC_SECURE : CTX_MAGIC_NORMAL;
      ctx->actual_handle_size = n + sizeof (struct gcry_md_context);
      ctx->flags.secure = secure;
      ctx->flags.bugemu1 = !!(flags & GCRY_MD_FLAG_BUGEMU1);

      if (hmac)
	{
	  switch (algo)
            {
              case GCRY_MD_SHA384:
              case GCRY_MD_SHA512:
                ctx->macpads_Bsize = 128;
                break;
              case GCRY_MD_GOSTR3411_94:
                ctx->macpads_Bsize = 32;
                break;
              default:
                ctx->macpads_Bsize = 64;
                break;
            }
          ctx->macpads = xtrymalloc_secure (2*(ctx->macpads_Bsize));
	  if (!ctx->macpads)
	    {
	      err = gpg_err_code_from_errno (errno);
	      md_close (hd);
	    }
	}
    }

  if (! err)
    {
      /* Hmmm, should we really do that? - yes [-wk] */
      _gcry_fast_random_poll ();

      if (algo)
	{
	  err = md_enable (hd, algo);
	  if (err)
	    md_close (hd);
	}
    }

  if (! err)
    *h = hd;

  return err;
}

/* Create a message digest object for algorithm ALGO.  FLAGS may be
   given as an bitwise OR of the gcry_md_flags values.  ALGO may be
   given as 0 if the algorithms to be used are later set using
   gcry_md_enable. H is guaranteed to be a valid handle or NULL on
   error.  */
gcry_err_code_t
_gcry_md_open (gcry_md_hd_t *h, int algo, unsigned int flags)
{
  gcry_err_code_t rc;
  gcry_md_hd_t hd;

  if ((flags & ~(GCRY_MD_FLAG_SECURE
                 | GCRY_MD_FLAG_HMAC
                 | GCRY_MD_FLAG_BUGEMU1)))
    rc = GPG_ERR_INV_ARG;
  else
    rc = md_open (&hd, algo, flags);

  *h = rc? NULL : hd;
  return rc;
}



static gcry_err_code_t
md_enable (gcry_md_hd_t hd, int algorithm)
{
  struct gcry_md_context *h = hd->ctx;
  gcry_md_spec_t *spec;
  GcryDigestEntry *entry;
  gcry_err_code_t err = 0;

  for (entry = h->list; entry; entry = entry->next)
    if (entry->spec->algo == algorithm)
      return 0; /* Already enabled */

  spec = spec_from_algo (algorithm);
  if (!spec)
    {
      log_debug ("md_enable: algorithm %d not available\n", algorithm);
      err = GPG_ERR_DIGEST_ALGO;
    }


  if (!err && algorithm == GCRY_MD_MD5 && fips_mode ())
    {
      _gcry_inactivate_fips_mode ("MD5 used");
      if (_gcry_enforced_fips_mode () )
        {
          /* We should never get to here because we do not register
             MD5 in enforced fips mode. But better throw an error.  */
          err = GPG_ERR_DIGEST_ALGO;
        }
    }

  if (!err)
    {
      size_t size = (sizeof (*entry)
                     + spec->contextsize
                     - sizeof (entry->context));

      /* And allocate a new list entry. */
      if (h->flags.secure)
	entry = xtrymalloc_secure (size);
      else
	entry = xtrymalloc (size);

      if (! entry)
	err = gpg_err_code_from_errno (errno);
      else
	{
	  entry->spec = spec;
	  entry->next = h->list;
          entry->actual_struct_size = size;
	  h->list = entry;

	  /* And init this instance. */
	  entry->spec->init (&entry->context.c,
                             h->flags.bugemu1? GCRY_MD_FLAG_BUGEMU1:0);
	}
    }

  return err;
}


gcry_err_code_t
_gcry_md_enable (gcry_md_hd_t hd, int algorithm)
{
  return md_enable (hd, algorithm);
}


static gcry_err_code_t
md_copy (gcry_md_hd_t ahd, gcry_md_hd_t *b_hd)
{
  gcry_err_code_t err = 0;
  struct gcry_md_context *a = ahd->ctx;
  struct gcry_md_context *b;
  GcryDigestEntry *ar, *br;
  gcry_md_hd_t bhd;
  size_t n;

  if (ahd->bufpos)
    md_write (ahd, NULL, 0);

  n = (char *) ahd->ctx - (char *) ahd;
  if (a->flags.secure)
    bhd = xtrymalloc_secure (n + sizeof (struct gcry_md_context));
  else
    bhd = xtrymalloc (n + sizeof (struct gcry_md_context));

  if (! bhd)
    err = gpg_err_code_from_errno (errno);

  if (! err)
    {
      bhd->ctx = b = (struct gcry_md_context *) ((char *) bhd + n);
      /* No need to copy the buffer due to the write above. */
      gcry_assert (ahd->bufsize == (n - sizeof (struct gcry_md_handle) + 1));
      bhd->bufsize = ahd->bufsize;
      bhd->bufpos = 0;
      gcry_assert (! ahd->bufpos);
      memcpy (b, a, sizeof *a);
      b->list = NULL;
      b->debug = NULL;
      if (a->macpads)
	{
	  b->macpads = xtrymalloc_secure (2*(a->macpads_Bsize));
	  if (! b->macpads)
	    {
	      err = gpg_err_code_from_errno (errno);
	      md_close (bhd);
	    }
	  else
	    memcpy (b->macpads, a->macpads, (2*(a->macpads_Bsize)));
	}
    }

  /* Copy the complete list of algorithms.  The copied list is
     reversed, but that doesn't matter. */
  if (!err)
    {
      for (ar = a->list; ar; ar = ar->next)
        {
          if (a->flags.secure)
            br = xtrymalloc_secure (sizeof *br
                                    + ar->spec->contextsize
                                    - sizeof(ar->context));
          else
            br = xtrymalloc (sizeof *br
                             + ar->spec->contextsize
                             - sizeof (ar->context));
          if (!br)
            {
	      err = gpg_err_code_from_errno (errno);
              md_close (bhd);
              break;
            }

          memcpy (br, ar, (sizeof (*br) + ar->spec->contextsize
                           - sizeof (ar->context)));
          br->next = b->list;
          b->list = br;
        }
    }

  if (a->debug && !err)
    md_start_debug (bhd, "unknown");

  if (!err)
    *b_hd = bhd;

  return err;
}


gcry_err_code_t
_gcry_md_copy (gcry_md_hd_t *handle, gcry_md_hd_t hd)
{
  gcry_err_code_t rc;

  rc = md_copy (hd, handle);
  if (rc)
    *handle = NULL;
  return rc;
}


/*
 * Reset all contexts and discard any buffered stuff.  This may be used
 * instead of a md_close(); md_open().
 */
void
_gcry_md_reset (gcry_md_hd_t a)
{
  GcryDigestEntry *r;

  /* Note: We allow this even in fips non operational mode.  */

  a->bufpos = a->ctx->flags.finalized = 0;

  for (r = a->ctx->list; r; r = r->next)
    {
      memset (r->context.c, 0, r->spec->contextsize);
      (*r->spec->init) (&r->context.c,
                        a->ctx->flags.bugemu1? GCRY_MD_FLAG_BUGEMU1:0);
    }
  if (a->ctx->macpads)
    md_write (a, a->ctx->macpads, a->ctx->macpads_Bsize); /* inner pad */
}


static void
md_close (gcry_md_hd_t a)
{
  GcryDigestEntry *r, *r2;

  if (! a)
    return;
  if (a->ctx->debug)
    md_stop_debug (a);
  for (r = a->ctx->list; r; r = r2)
    {
      r2 = r->next;
      wipememory (r, r->actual_struct_size);
      xfree (r);
    }

  if (a->ctx->macpads)
    {
      wipememory (a->ctx->macpads, 2*(a->ctx->macpads_Bsize));
      xfree(a->ctx->macpads);
    }

  wipememory (a, a->ctx->actual_handle_size);
  xfree(a);
}


void
_gcry_md_close (gcry_md_hd_t hd)
{
  /* Note: We allow this even in fips non operational mode.  */
  md_close (hd);
}


static void
md_write (gcry_md_hd_t a, const void *inbuf, size_t inlen)
{
  GcryDigestEntry *r;

  if (a->ctx->debug)
    {
      if (a->bufpos && fwrite (a->buf, a->bufpos, 1, a->ctx->debug) != 1)
	BUG();
      if (inlen && fwrite (inbuf, inlen, 1, a->ctx->debug) != 1)
	BUG();
    }

  for (r = a->ctx->list; r; r = r->next)
    {
      if (a->bufpos)
	(*r->spec->write) (&r->context.c, a->buf, a->bufpos);
      (*r->spec->write) (&r->context.c, inbuf, inlen);
    }
  a->bufpos = 0;
}


void
_gcry_md_write (gcry_md_hd_t hd, const void *inbuf, size_t inlen)
{
  md_write (hd, inbuf, inlen);
}


static void
md_final (gcry_md_hd_t a)
{
  GcryDigestEntry *r;

  if (a->ctx->flags.finalized)
    return;

  if (a->bufpos)
    md_write (a, NULL, 0);

  for (r = a->ctx->list; r; r = r->next)
    (*r->spec->final) (&r->context.c);

  a->ctx->flags.finalized = 1;

  if (a->ctx->macpads)
    {
      /* Finish the hmac. */
      int algo = md_get_algo (a);
      byte *p = md_read (a, algo);
      size_t dlen = md_digest_length (algo);
      gcry_md_hd_t om;
      gcry_err_code_t err;

      err = md_open (&om, algo,
                     ((a->ctx->flags.secure? GCRY_MD_FLAG_SECURE:0)
                      | (a->ctx->flags.bugemu1? GCRY_MD_FLAG_BUGEMU1:0)));
      if (err)
	_gcry_fatal_error (err, NULL);
      md_write (om,
                (a->ctx->macpads)+(a->ctx->macpads_Bsize),
                a->ctx->macpads_Bsize);
      md_write (om, p, dlen);
      md_final (om);
      /* Replace our digest with the mac (they have the same size). */
      memcpy (p, md_read (om, algo), dlen);
      md_close (om);
    }
}


static gcry_err_code_t
prepare_macpads (gcry_md_hd_t hd, const unsigned char *key, size_t keylen)
{
  int i;
  int algo = md_get_algo (hd);
  unsigned char *helpkey = NULL;
  unsigned char *ipad, *opad;

  if (!algo)
    return GPG_ERR_DIGEST_ALGO; /* Might happen if no algo is enabled.  */

  if ( keylen > hd->ctx->macpads_Bsize )
    {
      helpkey = xtrymalloc_secure (md_digest_length (algo));
      if (!helpkey)
        return gpg_err_code_from_errno (errno);
      _gcry_md_hash_buffer (algo, helpkey, key, keylen);
      key = helpkey;
      keylen = md_digest_length (algo);
      gcry_assert ( keylen <= hd->ctx->macpads_Bsize );
    }

  memset ( hd->ctx->macpads, 0, 2*(hd->ctx->macpads_Bsize) );
  ipad = hd->ctx->macpads;
  opad = (hd->ctx->macpads)+(hd->ctx->macpads_Bsize);
  memcpy ( ipad, key, keylen );
  memcpy ( opad, key, keylen );
  for (i=0; i < hd->ctx->macpads_Bsize; i++ )
    {
      ipad[i] ^= 0x36;
      opad[i] ^= 0x5c;
    }
  xfree (helpkey);

  return 0;
}


gcry_err_code_t
_gcry_md_ctl (gcry_md_hd_t hd, int cmd, void *buffer, size_t buflen)
{
  gcry_err_code_t rc = 0;

  (void)buflen; /* Currently not used.  */

  switch (cmd)
    {
    case GCRYCTL_FINALIZE:
      md_final (hd);
      break;
    case GCRYCTL_START_DUMP:
      md_start_debug (hd, buffer);
      break;
    case GCRYCTL_STOP_DUMP:
      md_stop_debug ( hd );
      break;
    default:
      rc = GPG_ERR_INV_OP;
    }
  return rc;
}


gcry_err_code_t
_gcry_md_setkey (gcry_md_hd_t hd, const void *key, size_t keylen)
{
  gcry_err_code_t rc;

  if (!hd->ctx->macpads)
    rc = GPG_ERR_CONFLICT;
  else
    {
      rc = prepare_macpads (hd, key, keylen);
      if (!rc)
	_gcry_md_reset (hd);
    }

  return rc;
}


/* The new debug interface.  If SUFFIX is a string it creates an debug
   file for the context HD.  IF suffix is NULL, the file is closed and
   debugging is stopped.  */
void
_gcry_md_debug (gcry_md_hd_t hd, const char *suffix)
{
  if (suffix)
    md_start_debug (hd, suffix);
  else
    md_stop_debug (hd);
}


/****************
 * If ALGO is null get the digest for the used algo (which should be
 * only one)
 */
static byte *
md_read( gcry_md_hd_t a, int algo )
{
  GcryDigestEntry *r = a->ctx->list;

  if (! algo)
    {
      /* Return the first algorithm */
      if (r)
        {
          if (r->next)
            log_debug ("more than one algorithm in md_read(0)\n");
          return r->spec->read (&r->context.c);
        }
    }
  else
    {
      for (r = a->ctx->list; r; r = r->next)
	if (r->spec->algo == algo)
	  return r->spec->read (&r->context.c);
    }
  BUG();
  return NULL;
}


/*
 * Read out the complete digest, this function implictly finalizes
 * the hash.
 */
byte *
_gcry_md_read (gcry_md_hd_t hd, int algo)
{
  /* This function is expected to always return a digest, thus we
     can't return an error which we actually should do in
     non-operational state.  */
  _gcry_md_ctl (hd, GCRYCTL_FINALIZE, NULL, 0);
  return md_read (hd, algo);
}


/*
 * Read out an intermediate digest.  Not yet functional.
 */
gcry_err_code_t
_gcry_md_get (gcry_md_hd_t hd, int algo, byte *buffer, int buflen)
{
  (void)hd;
  (void)algo;
  (void)buffer;
  (void)buflen;

  /*md_digest ... */
  fips_signal_error ("unimplemented function called");
  return GPG_ERR_INTERNAL;
}


/*
 * Shortcut function to hash a buffer with a given algo. The only
 * guaranteed supported algorithms are RIPE-MD160 and SHA-1. The
 * supplied digest buffer must be large enough to store the resulting
 * hash.  No error is returned, the function will abort on an invalid
 * algo.  DISABLED_ALGOS are ignored here.  */
void
_gcry_md_hash_buffer (int algo, void *digest,
                      const void *buffer, size_t length)
{
  if (algo == GCRY_MD_SHA1)
    _gcry_sha1_hash_buffer (digest, buffer, length);
  else if (algo == GCRY_MD_RMD160 && !fips_mode () )
    _gcry_rmd160_hash_buffer (digest, buffer, length);
  else
    {
      /* For the others we do not have a fast function, so we use the
	 normal functions. */
      gcry_md_hd_t h;
      gpg_err_code_t err;

      if (algo == GCRY_MD_MD5 && fips_mode ())
        {
          _gcry_inactivate_fips_mode ("MD5 used");
          if (_gcry_enforced_fips_mode () )
            {
              /* We should never get to here because we do not register
                 MD5 in enforced fips mode.  */
              _gcry_fips_noreturn ();
            }
        }

      err = md_open (&h, algo, 0);
      if (err)
	log_bug ("gcry_md_open failed for algo %d: %s",
                 algo, gpg_strerror (gcry_error(err)));
      md_write (h, (byte *) buffer, length);
      md_final (h);
      memcpy (digest, md_read (h, algo), md_digest_length (algo));
      md_close (h);
    }
}


/* Shortcut function to hash multiple buffers with a given algo.  In
   contrast to gcry_md_hash_buffer, this function returns an error on
   invalid arguments or on other problems; disabled algorithms are
   _not_ ignored but flagged as an error.

   The data to sign is taken from the array IOV which has IOVCNT items.

   The only supported flag in FLAGS is GCRY_MD_FLAG_HMAC which turns
   this function into a HMAC function; the first item in IOV is then
   used as the key.

   On success 0 is returned and resulting hash or HMAC is stored at
   DIGEST which must have been provided by the caller with an
   appropriate length.  */
gpg_err_code_t
_gcry_md_hash_buffers (int algo, unsigned int flags, void *digest,
                       const gcry_buffer_t *iov, int iovcnt)
{
  int hmac;

  if (!iov || iovcnt < 0)
    return GPG_ERR_INV_ARG;
  if (flags & ~(GCRY_MD_FLAG_HMAC))
    return GPG_ERR_INV_ARG;

  hmac = !!(flags & GCRY_MD_FLAG_HMAC);
  if (hmac && iovcnt < 1)
    return GPG_ERR_INV_ARG;

  if (algo == GCRY_MD_SHA1 && !hmac)
    _gcry_sha1_hash_buffers (digest, iov, iovcnt);
  else
    {
      /* For the others we do not have a fast function, so we use the
	 normal functions. */
      gcry_md_hd_t h;
      gpg_err_code_t rc;

      if (algo == GCRY_MD_MD5 && fips_mode ())
        {
          _gcry_inactivate_fips_mode ("MD5 used");
          if (_gcry_enforced_fips_mode () )
            {
              /* We should never get to here because we do not register
                 MD5 in enforced fips mode.  */
              _gcry_fips_noreturn ();
            }
        }

      rc = md_open (&h, algo, (hmac? GCRY_MD_FLAG_HMAC:0));
      if (rc)
        return rc;

      if (hmac)
        {
          rc = _gcry_md_setkey (h,
                                (const char*)iov[0].data + iov[0].off,
                                iov[0].len);
          if (rc)
            {
              md_close (h);
              return rc;
            }
          iov++; iovcnt--;
        }
      for (;iovcnt; iov++, iovcnt--)
        md_write (h, (const char*)iov[0].data + iov[0].off, iov[0].len);
      md_final (h);
      memcpy (digest, md_read (h, algo), md_digest_length (algo));
      md_close (h);
    }

  return 0;
}


static int
md_get_algo (gcry_md_hd_t a)
{
  GcryDigestEntry *r = a->ctx->list;

  if (r && r->next)
    {
      fips_signal_error ("possible usage error");
      log_error ("WARNING: more than one algorithm in md_get_algo()\n");
    }
  return r ? r->spec->algo : 0;
}


int
_gcry_md_get_algo (gcry_md_hd_t hd)
{
  return md_get_algo (hd);
}


/****************
 * Return the length of the digest
 */
static int
md_digest_length (int algorithm)
{
  gcry_md_spec_t *spec;

  spec = spec_from_algo (algorithm);
  return spec? spec->mdlen : 0;
}


/****************
 * Return the length of the digest in bytes.
 * This function will return 0 in case of errors.
 */
unsigned int
_gcry_md_get_algo_dlen (int algorithm)
{
  return md_digest_length (algorithm);
}


/* Hmmm: add a mode to enumerate the OIDs
 *	to make g10/sig-check.c more portable */
static const byte *
md_asn_oid (int algorithm, size_t *asnlen, size_t *mdlen)
{
  gcry_md_spec_t *spec;
  const byte *asnoid = NULL;

  spec = spec_from_algo (algorithm);
  if (spec)
    {
      if (asnlen)
	*asnlen = spec->asnlen;
      if (mdlen)
	*mdlen = spec->mdlen;
      asnoid = spec->asnoid;
    }
  else
    log_bug ("no ASN.1 OID for md algo %d\n", algorithm);

  return asnoid;
}


/****************
 * Return information about the given cipher algorithm
 * WHAT select the kind of information returned:
 *  GCRYCTL_TEST_ALGO:
 *	Returns 0 when the specified algorithm is available for use.
 *	buffer and nbytes must be zero.
 *  GCRYCTL_GET_ASNOID:
 *	Return the ASNOID of the algorithm in buffer. if buffer is NULL, only
 *	the required length is returned.
 *  GCRYCTL_SELFTEST
 *      Helper for the regression tests - shall not be used by applications.
 *
 * Note:  Because this function is in most cases used to return an
 * integer value, we can make it easier for the caller to just look at
 * the return value.  The caller will in all cases consult the value
 * and thereby detecting whether a error occurred or not (i.e. while checking
 * the block size)
 */
gcry_err_code_t
_gcry_md_algo_info (int algo, int what, void *buffer, size_t *nbytes)
{
  gcry_err_code_t rc;

  switch (what)
    {
    case GCRYCTL_TEST_ALGO:
      if (buffer || nbytes)
	rc = GPG_ERR_INV_ARG;
      else
	rc = check_digest_algo (algo);
      break;

    case GCRYCTL_GET_ASNOID:
      /* We need to check that the algo is available because
         md_asn_oid would otherwise raise an assertion. */
      rc = check_digest_algo (algo);
      if (!rc)
        {
          const char unsigned *asn;
          size_t asnlen;

          asn = md_asn_oid (algo, &asnlen, NULL);
          if (buffer && (*nbytes >= asnlen))
            {
              memcpy (buffer, asn, asnlen);
              *nbytes = asnlen;
            }
          else if (!buffer && nbytes)
            *nbytes = asnlen;
          else
            {
              if (buffer)
                rc = GPG_ERR_TOO_SHORT;
              else
                rc = GPG_ERR_INV_ARG;
            }
        }
      break;

    case GCRYCTL_SELFTEST:
      /* Helper function for the regression tests.  */
      rc = gpg_err_code (_gcry_md_selftest (algo, nbytes? (int)*nbytes : 0,
                                             NULL));
      break;

    default:
      rc = GPG_ERR_INV_OP;
      break;
  }

  return rc;
}


static void
md_start_debug ( gcry_md_hd_t md, const char *suffix )
{
  static int idx=0;
  char buf[50];

  if (fips_mode ())
    return;

  if ( md->ctx->debug )
    {
      log_debug("Oops: md debug already started\n");
      return;
    }
  idx++;
  snprintf (buf, DIM(buf)-1, "dbgmd-%05d.%.10s", idx, suffix );
  md->ctx->debug = fopen(buf, "w");
  if ( !md->ctx->debug )
    log_debug("md debug: can't open %s\n", buf );
}


static void
md_stop_debug( gcry_md_hd_t md )
{
  if ( md->ctx->debug )
    {
      if ( md->bufpos )
        md_write ( md, NULL, 0 );
      fclose (md->ctx->debug);
      md->ctx->debug = NULL;
    }

#ifdef HAVE_U64_TYPEDEF
  {  /* a kludge to pull in the __muldi3 for Solaris */
    volatile u32 a = (u32)(ulong)md;
    volatile u64 b = 42;
    volatile u64 c;
    c = a * b;
    (void)c;
  }
#endif
}



/*
 * Return information about the digest handle.
 *  GCRYCTL_IS_SECURE:
 *	Returns 1 when the handle works on secured memory
 *	otherwise 0 is returned.  There is no error return.
 *  GCRYCTL_IS_ALGO_ENABLED:
 *     Returns 1 if the algo is enabled for that handle.
 *     The algo must be passed as the address of an int.
 */
gcry_err_code_t
_gcry_md_info (gcry_md_hd_t h, int cmd, void *buffer, size_t *nbytes)
{
  gcry_err_code_t rc = 0;

  switch (cmd)
    {
    case GCRYCTL_IS_SECURE:
      *nbytes = h->ctx->flags.secure;
      break;

    case GCRYCTL_IS_ALGO_ENABLED:
      {
	GcryDigestEntry *r;
	int algo;

	if ( !buffer || !nbytes || *nbytes != sizeof (int))
	  rc = GPG_ERR_INV_ARG;
	else
	  {
	    algo = *(int*)buffer;

	    *nbytes = 0;
	    for(r=h->ctx->list; r; r = r->next ) {
	      if (r->spec->algo == algo)
		{
		  *nbytes = 1;
		  break;
		}
	    }
	  }
	break;
      }

  default:
    rc = GPG_ERR_INV_OP;
  }

  return rc;
}


/* Explicitly initialize this module.  */
gcry_err_code_t
_gcry_md_init (void)
{
  return 0;
}


int
_gcry_md_is_secure (gcry_md_hd_t a)
{
  size_t value;

  if (_gcry_md_info (a, GCRYCTL_IS_SECURE, NULL, &value))
    value = 1; /* It seems to be better to assume secure memory on
                  error. */
  return value;
}


int
_gcry_md_is_enabled (gcry_md_hd_t a, int algo)
{
  size_t value;

  value = sizeof algo;
  if (_gcry_md_info (a, GCRYCTL_IS_ALGO_ENABLED, &algo, &value))
    value = 0;
  return value;
}


/* Run the selftests for digest algorithm ALGO with optional reporting
   function REPORT.  */
gpg_error_t
_gcry_md_selftest (int algo, int extended, selftest_report_func_t report)
{
  gcry_err_code_t ec = 0;
  gcry_md_spec_t *spec;

  spec = spec_from_algo (algo);
  if (spec && !spec->flags.disabled && spec->selftest)
    ec = spec->selftest (algo, extended, report);
  else
    {
      ec = (spec && spec->selftest) ? GPG_ERR_DIGEST_ALGO
        /* */                       : GPG_ERR_NOT_IMPLEMENTED;
      if (report)
        report ("digest", algo, "module",
                (spec && !spec->flags.disabled)?
                "no selftest available" :
                spec? "algorithm disabled" : "algorithm not found");
    }

  return gpg_error (ec);
}

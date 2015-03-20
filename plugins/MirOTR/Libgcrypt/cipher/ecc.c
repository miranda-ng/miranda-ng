/* ecc.c  -  Elliptic Curve Cryptography
 * Copyright (C) 2007, 2008, 2010, 2011 Free Software Foundation, Inc.
 * Copyright (C) 2013 g10 Code GmbH
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

/* This code is originally based on the Patch 0.1.6 for the gnupg
   1.4.x branch as retrieved on 2007-03-21 from
   http://www.calcurco.cat/eccGnuPG/src/gnupg-1.4.6-ecc0.2.0beta1.diff.bz2
   The original authors are:
     Written by
      Sergi Blanch i Torne <d4372211 at alumnes.eup.udl.es>,
      Ramiro Moreno Chiral <ramiro at eup.udl.es>
     Maintainers
      Sergi Blanch i Torne
      Ramiro Moreno Chiral
      Mikael Mylnikov (mmr)
  For use in Libgcrypt the code has been heavily modified and cleaned
  up. In fact there is not much left of the orginally code except for
  some variable names and the text book implementaion of the sign and
  verification algorithms.  The arithmetic functions have entirely
  been rewritten and moved to mpi/ec.c.

  ECDH encrypt and decrypt code written by Andrey Jivsov.
*/


/* TODO:

  - In mpi/ec.c we use mpi_powm for x^2 mod p: Either implement a
    special case in mpi_powm or check whether mpi_mulm is faster.

*/


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "g10lib.h"
#include "mpi.h"
#include "cipher.h"
#include "context.h"
#include "ec-context.h"
#include "pubkey-internal.h"
#include "ecc-common.h"


static const char *ecc_names[] =
  {
    "ecc",
    "ecdsa",
    "ecdh",
    "eddsa",
    "gost",
    NULL,
  };


/* Registered progress function and its callback value. */
static void (*progress_cb) (void *, const char*, int, int, int);
static void *progress_cb_data;



/* Local prototypes. */
static void test_keys (ECC_secret_key * sk, unsigned int nbits);
static unsigned int ecc_get_nbits (gcry_sexp_t parms);




void
_gcry_register_pk_ecc_progress (void (*cb) (void *, const char *,
                                            int, int, int),
                                void *cb_data)
{
  progress_cb = cb;
  progress_cb_data = cb_data;
}

/* static void */
/* progress (int c) */
/* { */
/*   if (progress_cb) */
/*     progress_cb (progress_cb_data, "pk_ecc", c, 0, 0); */
/* } */



/* Standard version of the key generation.  */
static gpg_err_code_t
nist_generate_key (ECC_secret_key *sk, elliptic_curve_t *E, mpi_ec_t ctx,
                   gcry_random_level_t random_level, unsigned int nbits)
{
  mpi_point_struct Q;

  point_init (&Q);

  /* Generate a secret.  */
  if (ctx->dialect == ECC_DIALECT_ED25519)
    {
      char *rndbuf;

      sk->d = mpi_snew (256);
      rndbuf = _gcry_random_bytes_secure (32, random_level);
      rndbuf[0] &= 0x7f;  /* Clear bit 255. */
      rndbuf[0] |= 0x40;  /* Set bit 254.   */
      rndbuf[31] &= 0xf8; /* Clear bits 2..0 so that d mod 8 == 0  */
      _gcry_mpi_set_buffer (sk->d, rndbuf, 32, 0);
      xfree (rndbuf);
    }
  else
    sk->d = _gcry_dsa_gen_k (E->n, random_level);


  /* Compute Q.  */
  _gcry_mpi_ec_mul_point (&Q, sk->d, &E->G, ctx);

  /* Copy the stuff to the key structures. */
  sk->E.model = E->model;
  sk->E.dialect = E->dialect;
  sk->E.p = mpi_copy (E->p);
  sk->E.a = mpi_copy (E->a);
  sk->E.b = mpi_copy (E->b);
  point_init (&sk->E.G);
  point_set (&sk->E.G, &E->G);
  sk->E.n = mpi_copy (E->n);
  point_init (&sk->Q);

  /* We want the Q=(x,y) be a "compliant key" in terms of the
   * http://tools.ietf.org/html/draft-jivsov-ecc-compact, which simply
   * means that we choose either Q=(x,y) or -Q=(x,p-y) such that we
   * end up with the min(y,p-y) as the y coordinate.  Such a public
   * key allows the most efficient compression: y can simply be
   * dropped because we know that it's a minimum of the two
   * possibilities without any loss of security.  Note that we don't
   * do that for Ed25519 so that we do not violate the special
   * construction of the secret key.  */
  if (E->dialect == ECC_DIALECT_ED25519)
    point_set (&sk->Q, &Q);
  else
    {
      gcry_mpi_t x, y, negative;
      const unsigned int pbits = mpi_get_nbits (E->p);

      x = mpi_new (pbits);
      y = mpi_new (pbits);
      negative = mpi_new (pbits);

      if (_gcry_mpi_ec_get_affine (x, y, &Q, ctx))
        log_fatal ("ecgen: Failed to get affine coordinates for %s\n", "Q");

      if (E->model == MPI_EC_WEIERSTRASS)
        mpi_sub (negative, E->p, y);      /* negative = p - y */
      else
        mpi_sub (negative, E->p, x);      /* negative = p - x */

      if (mpi_cmp (negative, y) < 0)   /* p - y < p */
        {
          /* We need to end up with -Q; this assures that new Q's y is
             the smallest one */
          mpi_sub (sk->d, E->n, sk->d);   /* d = order - d */
          if (E->model == MPI_EC_WEIERSTRASS)
            mpi_point_snatch_set (&sk->Q, x, negative,
                                       mpi_alloc_set_ui (1));
          else
            mpi_point_snatch_set (&sk->Q, negative, y, mpi_alloc_set_ui (1));

          if (DBG_CIPHER)
            log_debug ("ecgen converted Q to a compliant point\n");
        }
      else /* p - y >= p */
        {
          /* No change is needed exactly 50% of the time: just copy. */
          point_set (&sk->Q, &Q);
          if (DBG_CIPHER)
            log_debug ("ecgen didn't need to convert Q to a compliant point\n");

          mpi_free (negative);
          if (E->model == MPI_EC_WEIERSTRASS)
            mpi_free (x);
          else
            mpi_free (y);
        }

      if (E->model == MPI_EC_WEIERSTRASS)
        mpi_free (y);
      else
        mpi_free (x);
    }

  point_free (&Q);
  /* Now we can test our keys (this should never fail!).  */
  test_keys (sk, nbits - 64);

  return 0;
}


/*
 * To verify correct skey it use a random information.
 * First, encrypt and decrypt this dummy value,
 * test if the information is recuperated.
 * Second, test with the sign and verify functions.
 */
static void
test_keys (ECC_secret_key *sk, unsigned int nbits)
{
  ECC_public_key pk;
  gcry_mpi_t test = mpi_new (nbits);
  mpi_point_struct R_;
  gcry_mpi_t c = mpi_new (nbits);
  gcry_mpi_t out = mpi_new (nbits);
  gcry_mpi_t r = mpi_new (nbits);
  gcry_mpi_t s = mpi_new (nbits);

  if (DBG_CIPHER)
    log_debug ("Testing key.\n");

  point_init (&R_);

  pk.E = _gcry_ecc_curve_copy (sk->E);
  point_init (&pk.Q);
  point_set (&pk.Q, &sk->Q);

  _gcry_mpi_randomize (test, nbits, GCRY_WEAK_RANDOM);

  if (_gcry_ecc_ecdsa_sign (test, sk, r, s, 0, 0) )
    log_fatal ("ECDSA operation: sign failed\n");

  if (_gcry_ecc_ecdsa_verify (test, &pk, r, s))
    {
      log_fatal ("ECDSA operation: sign, verify failed\n");
    }

  if (DBG_CIPHER)
    log_debug ("ECDSA operation: sign, verify ok.\n");

  point_free (&pk.Q);
  _gcry_ecc_curve_free (&pk.E);

  point_free (&R_);
  mpi_free (s);
  mpi_free (r);
  mpi_free (out);
  mpi_free (c);
  mpi_free (test);
}


/*
 * To check the validity of the value, recalculate the correspondence
 * between the public value and the secret one.
 */
static int
check_secret_key (ECC_secret_key *sk, mpi_ec_t ec, int flags)
{
  int rc = 1;
  mpi_point_struct Q;
  gcry_mpi_t x1, y1;
  gcry_mpi_t x2 = NULL;
  gcry_mpi_t y2 = NULL;

  point_init (&Q);
  x1 = mpi_new (0);
  y1 = mpi_new (0);

  /* G in E(F_p) */
  if (!_gcry_mpi_ec_curve_point (&sk->E.G, ec))
    {
      if (DBG_CIPHER)
        log_debug ("Bad check: Point 'G' does not belong to curve 'E'!\n");
      goto leave;
    }

  /* G != PaI */
  if (!mpi_cmp_ui (sk->E.G.z, 0))
    {
      if (DBG_CIPHER)
        log_debug ("Bad check: 'G' cannot be Point at Infinity!\n");
      goto leave;
    }

  /* Check order of curve.  */
  if (sk->E.dialect != ECC_DIALECT_ED25519)
    {
      _gcry_mpi_ec_mul_point (&Q, sk->E.n, &sk->E.G, ec);
      if (mpi_cmp_ui (Q.z, 0))
        {
          if (DBG_CIPHER)
            log_debug ("check_secret_key: E is not a curve of order n\n");
          goto leave;
        }
    }

  /* Pubkey cannot be PaI */
  if (!mpi_cmp_ui (sk->Q.z, 0))
    {
      if (DBG_CIPHER)
        log_debug ("Bad check: Q can not be a Point at Infinity!\n");
      goto leave;
    }

  /* pubkey = [d]G over E */
  if (!_gcry_ecc_compute_public (&Q, ec, &sk->E.G, sk->d))
    {
      if (DBG_CIPHER)
        log_debug ("Bad check: computation of dG failed\n");
      goto leave;
    }
  if (_gcry_mpi_ec_get_affine (x1, y1, &Q, ec))
    {
      if (DBG_CIPHER)
        log_debug ("Bad check: Q can not be a Point at Infinity!\n");
      goto leave;
    }

  if ((flags & PUBKEY_FLAG_EDDSA))
    ; /* Fixme: EdDSA is special.  */
  else if (!mpi_cmp_ui (sk->Q.z, 1))
    {
      /* Fast path if Q is already in affine coordinates.  */
      if (mpi_cmp (x1, sk->Q.x) || mpi_cmp (y1, sk->Q.y))
        {
          if (DBG_CIPHER)
            log_debug
              ("Bad check: There is NO correspondence between 'd' and 'Q'!\n");
          goto leave;
        }
    }
  else
    {
      x2 = mpi_new (0);
      y2 = mpi_new (0);
      if (_gcry_mpi_ec_get_affine (x2, y2, &sk->Q, ec))
        {
          if (DBG_CIPHER)
            log_debug ("Bad check: Q can not be a Point at Infinity!\n");
          goto leave;
        }

      if (mpi_cmp (x1, x2) || mpi_cmp (y1, y2))
        {
          if (DBG_CIPHER)
            log_debug
              ("Bad check: There is NO correspondence between 'd' and 'Q'!\n");
          goto leave;
        }
    }
  rc = 0; /* Okay.  */

 leave:
  mpi_free (x2);
  mpi_free (x1);
  mpi_free (y1);
  mpi_free (y2);
  point_free (&Q);
  return rc;
}



/*********************************************
 **************  interface  ******************
 *********************************************/

static gcry_err_code_t
ecc_generate (const gcry_sexp_t genparms, gcry_sexp_t *r_skey)
{
  gpg_err_code_t rc;
  unsigned int nbits;
  elliptic_curve_t E;
  ECC_secret_key sk;
  gcry_mpi_t x = NULL;
  gcry_mpi_t y = NULL;
  char *curve_name = NULL;
  gcry_sexp_t l1;
  gcry_random_level_t random_level;
  mpi_ec_t ctx = NULL;
  gcry_sexp_t curve_info = NULL;
  gcry_sexp_t curve_flags = NULL;
  gcry_mpi_t base = NULL;
  gcry_mpi_t public = NULL;
  gcry_mpi_t secret = NULL;
  int flags = 0;

  memset (&E, 0, sizeof E);
  memset (&sk, 0, sizeof sk);

  rc = _gcry_pk_util_get_nbits (genparms, &nbits);
  if (rc)
    return rc;

  /* Parse the optional "curve" parameter. */
  l1 = sexp_find_token (genparms, "curve", 0);
  if (l1)
    {
      curve_name = _gcry_sexp_nth_string (l1, 1);
      sexp_release (l1);
      if (!curve_name)
        return GPG_ERR_INV_OBJ; /* No curve name or value too large. */
    }

  /* Parse the optional flags list.  */
  l1 = sexp_find_token (genparms, "flags", 0);
  if (l1)
    {
      rc = _gcry_pk_util_parse_flaglist (l1, &flags, NULL);
      sexp_release (l1);
      if (rc)
        goto leave;
    }

  /* Parse the deprecated optional transient-key flag.  */
  l1 = sexp_find_token (genparms, "transient-key", 0);
  if (l1)
    {
      flags |= PUBKEY_FLAG_TRANSIENT_KEY;
      sexp_release (l1);
    }

  /* NBITS is required if no curve name has been given.  */
  if (!nbits && !curve_name)
    return GPG_ERR_NO_OBJ; /* No NBITS parameter. */

  rc = _gcry_ecc_fill_in_curve (nbits, curve_name, &E, &nbits);
  xfree (curve_name); curve_name = NULL;
  if (rc)
    goto leave;

  if (DBG_CIPHER)
    {
      log_debug ("ecgen curve info: %s/%s\n",
                 _gcry_ecc_model2str (E.model),
                 _gcry_ecc_dialect2str (E.dialect));
      if (E.name)
        log_debug ("ecgen curve used: %s\n", E.name);
      log_printmpi ("ecgen curve   p", E.p);
      log_printmpi ("ecgen curve   a", E.a);
      log_printmpi ("ecgen curve   b", E.b);
      log_printmpi ("ecgen curve   n", E.n);
      log_printpnt ("ecgen curve G", &E.G, NULL);
    }

  if ((flags & PUBKEY_FLAG_TRANSIENT_KEY))
    random_level = GCRY_STRONG_RANDOM;
  else
    random_level = GCRY_VERY_STRONG_RANDOM;

  ctx = _gcry_mpi_ec_p_internal_new (E.model, E.dialect, 0, E.p, E.a, E.b);
  x = mpi_new (0);
  y = mpi_new (0);

  if ((flags & PUBKEY_FLAG_EDDSA))
    rc = _gcry_ecc_eddsa_genkey (&sk, &E, ctx, random_level);
  else
    rc = nist_generate_key (&sk, &E, ctx, random_level, nbits);
  if (rc)
    goto leave;

  /* Copy data to the result.  */
  if (_gcry_mpi_ec_get_affine (x, y, &sk.E.G, ctx))
    log_fatal ("ecgen: Failed to get affine coordinates for %s\n", "G");
  base = _gcry_ecc_ec2os (x, y, sk.E.p);
  if (sk.E.dialect == ECC_DIALECT_ED25519 && !(flags & PUBKEY_FLAG_NOCOMP))
    {
      unsigned char *encpk;
      unsigned int encpklen;

      rc = _gcry_ecc_eddsa_encodepoint (&sk.Q, ctx, x, y,
                                        !!(flags & PUBKEY_FLAG_COMP),
                                        &encpk, &encpklen);
      if (rc)
        return rc;
      public = mpi_new (0);
      mpi_set_opaque (public, encpk, encpklen*8);
      encpk = NULL;
    }
  else
    {
      if (_gcry_mpi_ec_get_affine (x, y, &sk.Q, ctx))
        log_fatal ("ecgen: Failed to get affine coordinates for %s\n", "Q");
      public = _gcry_ecc_ec2os (x, y, sk.E.p);
    }
  secret = sk.d; sk.d = NULL;
  if (E.name)
    {
      rc = sexp_build (&curve_info, NULL, "(curve %s)", E.name);
      if (rc)
        goto leave;
    }

  if ((flags & PUBKEY_FLAG_PARAM) || (flags & PUBKEY_FLAG_EDDSA))
    {
      rc = sexp_build
        (&curve_flags, NULL,
         ((flags & PUBKEY_FLAG_PARAM) && (flags & PUBKEY_FLAG_EDDSA))?
         "(flags param eddsa)" :
         ((flags & PUBKEY_FLAG_PARAM))?
         "(flags param)" :
         "(flags eddsa)");
      if (rc)
        goto leave;
    }

  if ((flags & PUBKEY_FLAG_PARAM) && E.name)
    rc = sexp_build (r_skey, NULL,
                     "(key-data"
                     " (public-key"
                     "  (ecc%S%S(p%m)(a%m)(b%m)(g%m)(n%m)(q%m)))"
                     " (private-key"
                     "  (ecc%S%S(p%m)(a%m)(b%m)(g%m)(n%m)(q%m)(d%m)))"
                     " )",
                     curve_info, curve_flags,
                     sk.E.p, sk.E.a, sk.E.b, base, sk.E.n, public,
                     curve_info, curve_flags,
                     sk.E.p, sk.E.a, sk.E.b, base, sk.E.n, public, secret);
  else
    rc = sexp_build (r_skey, NULL,
                     "(key-data"
                     " (public-key"
                     "  (ecc%S%S(q%m)))"
                     " (private-key"
                     "  (ecc%S%S(q%m)(d%m)))"
                     " )",
                     curve_info, curve_flags,
                     public,
                     curve_info, curve_flags,
                     public, secret);
  if (rc)
    goto leave;

  if (DBG_CIPHER)
    {
      log_printmpi ("ecgen result  p", sk.E.p);
      log_printmpi ("ecgen result  a", sk.E.a);
      log_printmpi ("ecgen result  b", sk.E.b);
      log_printmpi ("ecgen result  G", base);
      log_printmpi ("ecgen result  n", sk.E.n);
      log_printmpi ("ecgen result  Q", public);
      log_printmpi ("ecgen result  d", secret);
      if ((flags & PUBKEY_FLAG_EDDSA))
        log_debug ("ecgen result  using Ed25519+EdDSA\n");
    }

 leave:
  mpi_free (secret);
  mpi_free (public);
  mpi_free (base);
  {
    _gcry_ecc_curve_free (&sk.E);
    point_free (&sk.Q);
    mpi_free (sk.d);
  }
  _gcry_ecc_curve_free (&E);
  mpi_free (x);
  mpi_free (y);
  _gcry_mpi_ec_free (ctx);
  sexp_release (curve_flags);
  sexp_release (curve_info);
  return rc;
}


static gcry_err_code_t
ecc_check_secret_key (gcry_sexp_t keyparms)
{
  gcry_err_code_t rc;
  gcry_sexp_t l1 = NULL;
  int flags = 0;
  char *curvename = NULL;
  gcry_mpi_t mpi_g = NULL;
  gcry_mpi_t mpi_q = NULL;
  ECC_secret_key sk;
  mpi_ec_t ec = NULL;

  memset (&sk, 0, sizeof sk);

  /* Look for flags. */
  l1 = sexp_find_token (keyparms, "flags", 0);
  if (l1)
    {
      rc = _gcry_pk_util_parse_flaglist (l1, &flags, NULL);
      if (rc)
        goto leave;
    }

  /* Extract the parameters.  */
  if ((flags & PUBKEY_FLAG_PARAM))
    rc = sexp_extract_param (keyparms, NULL, "-p?a?b?g?n?/q?+d",
                             &sk.E.p, &sk.E.a, &sk.E.b, &mpi_g, &sk.E.n,
                             &mpi_q, &sk.d, NULL);
  else
    rc = sexp_extract_param (keyparms, NULL, "/q?+d",
                             &mpi_q, &sk.d, NULL);
  if (rc)
    goto leave;

  /* Add missing parameters using the optional curve parameter.  */
  sexp_release (l1);
  l1 = sexp_find_token (keyparms, "curve", 5);
  if (l1)
    {
      curvename = sexp_nth_string (l1, 1);
      if (curvename)
        {
          rc = _gcry_ecc_update_curve_param (curvename,
                                             &sk.E.model, &sk.E.dialect,
                                             &sk.E.p, &sk.E.a, &sk.E.b,
                                             &mpi_g, &sk.E.n);
          if (rc)
            return rc;
        }
    }
  if (mpi_g)
    {
      point_init (&sk.E.G);
      rc = _gcry_ecc_os2ec (&sk.E.G, mpi_g);
      if (rc)
        goto leave;
    }

  /* Guess required fields if a curve parameter has not been given.
     FIXME: This is a crude hacks.  We need to fix that.  */
  if (!curvename)
    {
      sk.E.model = ((flags & PUBKEY_FLAG_EDDSA)
               ? MPI_EC_TWISTEDEDWARDS
               : MPI_EC_WEIERSTRASS);
      sk.E.dialect = ((flags & PUBKEY_FLAG_EDDSA)
                      ? ECC_DIALECT_ED25519
                      : ECC_DIALECT_STANDARD);
    }
  if (DBG_CIPHER)
    {
      log_debug ("ecc_testkey inf: %s/%s\n",
                 _gcry_ecc_model2str (sk.E.model),
                 _gcry_ecc_dialect2str (sk.E.dialect));
      if (sk.E.name)
        log_debug  ("ecc_testkey nam: %s\n", sk.E.name);
      log_printmpi ("ecc_testkey   p", sk.E.p);
      log_printmpi ("ecc_testkey   a", sk.E.a);
      log_printmpi ("ecc_testkey   b", sk.E.b);
      log_printpnt ("ecc_testkey g",   &sk.E.G, NULL);
      log_printmpi ("ecc_testkey   n", sk.E.n);
      log_printmpi ("ecc_testkey   q", mpi_q);
      if (!fips_mode ())
        log_printmpi ("ecc_testkey   d", sk.d);
    }
  if (!sk.E.p || !sk.E.a || !sk.E.b || !sk.E.G.x || !sk.E.n || !sk.d)
    {
      rc = GPG_ERR_NO_OBJ;
      goto leave;
    }

  ec = _gcry_mpi_ec_p_internal_new (sk.E.model, sk.E.dialect, 0,
                                    sk.E.p, sk.E.a, sk.E.b);

  if (mpi_q)
    {
      point_init (&sk.Q);
      if (ec->dialect == ECC_DIALECT_ED25519)
        rc = _gcry_ecc_eddsa_decodepoint (mpi_q, ec, &sk.Q, NULL, NULL);
      else
        rc = _gcry_ecc_os2ec (&sk.Q, mpi_q);
      if (rc)
        goto leave;
    }
  else
    {
      /* The secret key test requires Q.  */
      rc = GPG_ERR_NO_OBJ;
      goto leave;
    }

  if (check_secret_key (&sk, ec, flags))
    rc = GPG_ERR_BAD_SECKEY;

 leave:
  _gcry_mpi_ec_free (ec);
  _gcry_mpi_release (sk.E.p);
  _gcry_mpi_release (sk.E.a);
  _gcry_mpi_release (sk.E.b);
  _gcry_mpi_release (mpi_g);
  point_free (&sk.E.G);
  _gcry_mpi_release (sk.E.n);
  _gcry_mpi_release (mpi_q);
  point_free (&sk.Q);
  _gcry_mpi_release (sk.d);
  xfree (curvename);
  sexp_release (l1);
  if (DBG_CIPHER)
    log_debug ("ecc_testkey   => %s\n", gpg_strerror (rc));
  return rc;
}


static gcry_err_code_t
ecc_sign (gcry_sexp_t *r_sig, gcry_sexp_t s_data, gcry_sexp_t keyparms)
{
  gcry_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_mpi_t data = NULL;
  gcry_sexp_t l1 = NULL;
  char *curvename = NULL;
  gcry_mpi_t mpi_g = NULL;
  gcry_mpi_t mpi_q = NULL;
  ECC_secret_key sk;
  gcry_mpi_t sig_r = NULL;
  gcry_mpi_t sig_s = NULL;

  memset (&sk, 0, sizeof sk);

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_SIGN, 0);

  /* Extract the data.  */
  rc = _gcry_pk_util_data_to_mpi (s_data, &data, &ctx);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_mpidump ("ecc_sign   data", data);

  /*
   * Extract the key.
   */
  if ((ctx.flags & PUBKEY_FLAG_PARAM))
    rc = sexp_extract_param (keyparms, NULL, "-p?a?b?g?n?/q?+d",
                             &sk.E.p, &sk.E.a, &sk.E.b, &mpi_g, &sk.E.n,
                             &mpi_q, &sk.d, NULL);
  else
    rc = sexp_extract_param (keyparms, NULL, "/q?+d",
                             &mpi_q, &sk.d, NULL);
  if (rc)
    goto leave;
  if (mpi_g)
    {
      point_init (&sk.E.G);
      rc = _gcry_ecc_os2ec (&sk.E.G, mpi_g);
      if (rc)
        goto leave;
    }
  /* Add missing parameters using the optional curve parameter.  */
  sexp_release (l1);
  l1 = sexp_find_token (keyparms, "curve", 5);
  if (l1)
    {
      curvename = sexp_nth_string (l1, 1);
      if (curvename)
        {
          rc = _gcry_ecc_fill_in_curve (0, curvename, &sk.E, NULL);
          if (rc)
            return rc;
        }
    }
  /* Guess required fields if a curve parameter has not been given.
     FIXME: This is a crude hacks.  We need to fix that.  */
  if (!curvename)
    {
      sk.E.model = ((ctx.flags & PUBKEY_FLAG_EDDSA)
                    ? MPI_EC_TWISTEDEDWARDS
                    : MPI_EC_WEIERSTRASS);
      sk.E.dialect = ((ctx.flags & PUBKEY_FLAG_EDDSA)
                      ? ECC_DIALECT_ED25519
                      : ECC_DIALECT_STANDARD);
    }
  if (DBG_CIPHER)
    {
      log_debug ("ecc_sign   info: %s/%s%s\n",
                 _gcry_ecc_model2str (sk.E.model),
                 _gcry_ecc_dialect2str (sk.E.dialect),
                 (ctx.flags & PUBKEY_FLAG_EDDSA)? "+EdDSA":"");
      if (sk.E.name)
        log_debug  ("ecc_sign   name: %s\n", sk.E.name);
      log_printmpi ("ecc_sign      p", sk.E.p);
      log_printmpi ("ecc_sign      a", sk.E.a);
      log_printmpi ("ecc_sign      b", sk.E.b);
      log_printpnt ("ecc_sign    g",   &sk.E.G, NULL);
      log_printmpi ("ecc_sign      n", sk.E.n);
      log_printmpi ("ecc_sign      q", mpi_q);
      if (!fips_mode ())
        log_printmpi ("ecc_sign      d", sk.d);
    }
  if (!sk.E.p || !sk.E.a || !sk.E.b || !sk.E.G.x || !sk.E.n || !sk.d)
    {
      rc = GPG_ERR_NO_OBJ;
      goto leave;
    }


  sig_r = mpi_new (0);
  sig_s = mpi_new (0);
  if ((ctx.flags & PUBKEY_FLAG_EDDSA))
    {
      /* EdDSA requires the public key.  */
      rc = _gcry_ecc_eddsa_sign (data, &sk, sig_r, sig_s, ctx.hash_algo, mpi_q);
      if (!rc)
        rc = sexp_build (r_sig, NULL,
                         "(sig-val(eddsa(r%M)(s%M)))", sig_r, sig_s);
    }
  else if ((ctx.flags & PUBKEY_FLAG_GOST))
    {
      rc = _gcry_ecc_gost_sign (data, &sk, sig_r, sig_s);
      if (!rc)
        rc = sexp_build (r_sig, NULL,
                         "(sig-val(gost(r%M)(s%M)))", sig_r, sig_s);
    }
  else
    {
      rc = _gcry_ecc_ecdsa_sign (data, &sk, sig_r, sig_s,
                                 ctx.flags, ctx.hash_algo);
      if (!rc)
        rc = sexp_build (r_sig, NULL,
                         "(sig-val(ecdsa(r%M)(s%M)))", sig_r, sig_s);
    }


 leave:
  _gcry_mpi_release (sk.E.p);
  _gcry_mpi_release (sk.E.a);
  _gcry_mpi_release (sk.E.b);
  _gcry_mpi_release (mpi_g);
  point_free (&sk.E.G);
  _gcry_mpi_release (sk.E.n);
  _gcry_mpi_release (mpi_q);
  point_free (&sk.Q);
  _gcry_mpi_release (sk.d);
  _gcry_mpi_release (sig_r);
  _gcry_mpi_release (sig_s);
  xfree (curvename);
  _gcry_mpi_release (data);
  sexp_release (l1);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("ecc_sign      => %s\n", gpg_strerror (rc));
  return rc;
}


static gcry_err_code_t
ecc_verify (gcry_sexp_t s_sig, gcry_sexp_t s_data, gcry_sexp_t s_keyparms)
{
  gcry_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_sexp_t l1 = NULL;
  char *curvename = NULL;
  gcry_mpi_t mpi_g = NULL;
  gcry_mpi_t mpi_q = NULL;
  gcry_mpi_t sig_r = NULL;
  gcry_mpi_t sig_s = NULL;
  gcry_mpi_t data = NULL;
  ECC_public_key pk;
  int sigflags;

  memset (&pk, 0, sizeof pk);
  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_VERIFY,
                                   ecc_get_nbits (s_keyparms));

  /* Extract the data.  */
  rc = _gcry_pk_util_data_to_mpi (s_data, &data, &ctx);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_mpidump ("ecc_verify data", data);

  /*
   * Extract the signature value.
   */
  rc = _gcry_pk_util_preparse_sigval (s_sig, ecc_names, &l1, &sigflags);
  if (rc)
    goto leave;
  rc = sexp_extract_param (l1, NULL, (sigflags & PUBKEY_FLAG_EDDSA)? "/rs":"rs",
                           &sig_r, &sig_s, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    {
      log_mpidump ("ecc_verify  s_r", sig_r);
      log_mpidump ("ecc_verify  s_s", sig_s);
    }
  if ((ctx.flags & PUBKEY_FLAG_EDDSA) ^ (sigflags & PUBKEY_FLAG_EDDSA))
    {
      rc = GPG_ERR_CONFLICT; /* Inconsistent use of flag/algoname.  */
      goto leave;
    }


  /*
   * Extract the key.
   */
  if ((ctx.flags & PUBKEY_FLAG_PARAM))
    rc = sexp_extract_param (s_keyparms, NULL, "-p?a?b?g?n?/q",
                             &pk.E.p, &pk.E.a, &pk.E.b, &mpi_g, &pk.E.n,
                             &mpi_q, NULL);
  else
    rc = sexp_extract_param (s_keyparms, NULL, "/q",
                             &mpi_q, NULL);
  if (rc)
    goto leave;
  if (mpi_g)
    {
      point_init (&pk.E.G);
      rc = _gcry_ecc_os2ec (&pk.E.G, mpi_g);
      if (rc)
        goto leave;
    }
  /* Add missing parameters using the optional curve parameter.  */
  sexp_release (l1);
  l1 = sexp_find_token (s_keyparms, "curve", 5);
  if (l1)
    {
      curvename = sexp_nth_string (l1, 1);
      if (curvename)
        {
          rc = _gcry_ecc_fill_in_curve (0, curvename, &pk.E, NULL);
          if (rc)
            return rc;
        }
    }
  /* Guess required fields if a curve parameter has not been given.
     FIXME: This is a crude hacks.  We need to fix that.  */
  if (!curvename)
    {
      pk.E.model = ((sigflags & PUBKEY_FLAG_EDDSA)
                    ? MPI_EC_TWISTEDEDWARDS
                    : MPI_EC_WEIERSTRASS);
      pk.E.dialect = ((sigflags & PUBKEY_FLAG_EDDSA)
                      ? ECC_DIALECT_ED25519
                      : ECC_DIALECT_STANDARD);
    }

  if (DBG_CIPHER)
    {
      log_debug ("ecc_verify info: %s/%s%s\n",
                 _gcry_ecc_model2str (pk.E.model),
                 _gcry_ecc_dialect2str (pk.E.dialect),
                 (sigflags & PUBKEY_FLAG_EDDSA)? "+EdDSA":"");
      if (pk.E.name)
        log_debug  ("ecc_verify name: %s\n", pk.E.name);
      log_printmpi ("ecc_verify    p", pk.E.p);
      log_printmpi ("ecc_verify    a", pk.E.a);
      log_printmpi ("ecc_verify    b", pk.E.b);
      log_printpnt ("ecc_verify  g",   &pk.E.G, NULL);
      log_printmpi ("ecc_verify    n", pk.E.n);
      log_printmpi ("ecc_verify    q", mpi_q);
    }
  if (!pk.E.p || !pk.E.a || !pk.E.b || !pk.E.G.x || !pk.E.n || !mpi_q)
    {
      rc = GPG_ERR_NO_OBJ;
      goto leave;
    }


  /*
   * Verify the signature.
   */
  if ((sigflags & PUBKEY_FLAG_EDDSA))
    {
      rc = _gcry_ecc_eddsa_verify (data, &pk, sig_r, sig_s,
                                   ctx.hash_algo, mpi_q);
    }
  else if ((sigflags & PUBKEY_FLAG_GOST))
    {
      point_init (&pk.Q);
      rc = _gcry_ecc_os2ec (&pk.Q, mpi_q);
      if (rc)
        goto leave;

      rc = _gcry_ecc_gost_verify (data, &pk, sig_r, sig_s);
    }
  else
    {
      point_init (&pk.Q);
      if (pk.E.dialect == ECC_DIALECT_ED25519)
        {
          mpi_ec_t ec;

          /* Fixme: Factor the curve context setup out of eddsa_verify
             and ecdsa_verify. So that we don't do it twice.  */
          ec = _gcry_mpi_ec_p_internal_new (pk.E.model, pk.E.dialect, 0,
                                            pk.E.p, pk.E.a, pk.E.b);

          rc = _gcry_ecc_eddsa_decodepoint (mpi_q, ec, &pk.Q, NULL, NULL);
          _gcry_mpi_ec_free (ec);
        }
      else
        {
          rc = _gcry_ecc_os2ec (&pk.Q, mpi_q);
        }
      if (rc)
        goto leave;

      if (mpi_is_opaque (data))
        {
          const void *abuf;
          unsigned int abits, qbits;
          gcry_mpi_t a;

          qbits = mpi_get_nbits (pk.E.n);

          abuf = mpi_get_opaque (data, &abits);
          rc = _gcry_mpi_scan (&a, GCRYMPI_FMT_USG, abuf, (abits+7)/8, NULL);
          if (!rc)
            {
              if (abits > qbits)
                mpi_rshift (a, a, abits - qbits);

              rc = _gcry_ecc_ecdsa_verify (a, &pk, sig_r, sig_s);
              _gcry_mpi_release (a);
            }
        }
      else
        rc = _gcry_ecc_ecdsa_verify (data, &pk, sig_r, sig_s);
    }

 leave:
  _gcry_mpi_release (pk.E.p);
  _gcry_mpi_release (pk.E.a);
  _gcry_mpi_release (pk.E.b);
  _gcry_mpi_release (mpi_g);
  point_free (&pk.E.G);
  _gcry_mpi_release (pk.E.n);
  _gcry_mpi_release (mpi_q);
  point_free (&pk.Q);
  _gcry_mpi_release (data);
  _gcry_mpi_release (sig_r);
  _gcry_mpi_release (sig_s);
  xfree (curvename);
  sexp_release (l1);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("ecc_verify    => %s\n", rc?gpg_strerror (rc):"Good");
  return rc;
}


/* ecdh raw is classic 2-round DH protocol published in 1976.
 *
 * Overview of ecc_encrypt_raw and ecc_decrypt_raw.
 *
 * As with any PK operation, encrypt version uses a public key and
 * decrypt -- private.
 *
 * Symbols used below:
 *     G - field generator point
 *     d - private long-term scalar
 *    dG - public long-term key
 *     k - ephemeral scalar
 *    kG - ephemeral public key
 *   dkG - shared secret
 *
 * ecc_encrypt_raw description:
 *   input:
 *     data[0] : private scalar (k)
 *   output: A new S-expression with the parameters:
 *     s : shared point (kdG)
 *     e : generated ephemeral public key (kG)
 *
 * ecc_decrypt_raw description:
 *   input:
 *     data[0] : a point kG (ephemeral public key)
 *   output:
 *     result[0] : shared point (kdG)
 */
static gcry_err_code_t
ecc_encrypt_raw (gcry_sexp_t *r_ciph, gcry_sexp_t s_data, gcry_sexp_t keyparms)
{
  gcry_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_sexp_t l1 = NULL;
  char *curvename = NULL;
  gcry_mpi_t mpi_g = NULL;
  gcry_mpi_t mpi_q = NULL;
  gcry_mpi_t mpi_s = NULL;
  gcry_mpi_t mpi_e = NULL;
  gcry_mpi_t data = NULL;
  ECC_public_key pk;
  mpi_ec_t ec = NULL;

  memset (&pk, 0, sizeof pk);
  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_ENCRYPT,
                                   ecc_get_nbits (keyparms));

  /*
   * Extract the data.
   */
  rc = _gcry_pk_util_data_to_mpi (s_data, &data, &ctx);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_mpidump ("ecc_encrypt data", data);
  if (mpi_is_opaque (data))
    {
      rc = GPG_ERR_INV_DATA;
      goto leave;
    }


  /*
   * Extract the key.
   */
  rc = sexp_extract_param (keyparms, NULL, "-p?a?b?g?n?+q",
                           &pk.E.p, &pk.E.a, &pk.E.b, &mpi_g, &pk.E.n,
                           &mpi_q, NULL);
  if (rc)
    goto leave;
  if (mpi_g)
    {
      point_init (&pk.E.G);
      rc = _gcry_ecc_os2ec (&pk.E.G, mpi_g);
      if (rc)
        goto leave;
    }
  /* Add missing parameters using the optional curve parameter.  */
  sexp_release (l1);
  l1 = sexp_find_token (keyparms, "curve", 5);
  if (l1)
    {
      curvename = sexp_nth_string (l1, 1);
      if (curvename)
        {
          rc = _gcry_ecc_fill_in_curve (0, curvename, &pk.E, NULL);
          if (rc)
            return rc;
        }
    }
  /* Guess required fields if a curve parameter has not been given.  */
  if (!curvename)
    {
      pk.E.model = MPI_EC_WEIERSTRASS;
      pk.E.dialect = ECC_DIALECT_STANDARD;
    }

  if (DBG_CIPHER)
    {
      log_debug ("ecc_encrypt info: %s/%s\n",
                 _gcry_ecc_model2str (pk.E.model),
                 _gcry_ecc_dialect2str (pk.E.dialect));
      if (pk.E.name)
        log_debug  ("ecc_encrypt name: %s\n", pk.E.name);
      log_printmpi ("ecc_encrypt    p", pk.E.p);
      log_printmpi ("ecc_encrypt    a", pk.E.a);
      log_printmpi ("ecc_encrypt    b", pk.E.b);
      log_printpnt ("ecc_encrypt  g",   &pk.E.G, NULL);
      log_printmpi ("ecc_encrypt    n", pk.E.n);
      log_printmpi ("ecc_encrypt    q", mpi_q);
    }
  if (!pk.E.p || !pk.E.a || !pk.E.b || !pk.E.G.x || !pk.E.n || !mpi_q)
    {
      rc = GPG_ERR_NO_OBJ;
      goto leave;
    }

  /* Convert the public key.  */
  if (mpi_q)
    {
      point_init (&pk.Q);
      rc = _gcry_ecc_os2ec (&pk.Q, mpi_q);
      if (rc)
        goto leave;
    }

  /* Compute the encrypted value.  */
  ec = _gcry_mpi_ec_p_internal_new (pk.E.model, pk.E.dialect, 0,
                                    pk.E.p, pk.E.a, pk.E.b);

  /* The following is false: assert( mpi_cmp_ui( R.x, 1 )==0 );, so */
  {
    mpi_point_struct R;  /* Result that we return.  */
    gcry_mpi_t x, y;

    x = mpi_new (0);
    y = mpi_new (0);

    point_init (&R);

    /* R = kQ  <=>  R = kdG  */
    _gcry_mpi_ec_mul_point (&R, data, &pk.Q, ec);

    if (_gcry_mpi_ec_get_affine (x, y, &R, ec))
      log_fatal ("ecdh: Failed to get affine coordinates for kdG\n");
    mpi_s = _gcry_ecc_ec2os (x, y, pk.E.p);

    /* R = kG */
    _gcry_mpi_ec_mul_point (&R, data, &pk.E.G, ec);

    if (_gcry_mpi_ec_get_affine (x, y, &R, ec))
      log_fatal ("ecdh: Failed to get affine coordinates for kG\n");
    mpi_e = _gcry_ecc_ec2os (x, y, pk.E.p);

    mpi_free (x);
    mpi_free (y);

    point_free (&R);
  }

  rc = sexp_build (r_ciph, NULL, "(enc-val(ecdh(s%m)(e%m)))", mpi_s, mpi_e);

 leave:
  _gcry_mpi_release (pk.E.p);
  _gcry_mpi_release (pk.E.a);
  _gcry_mpi_release (pk.E.b);
  _gcry_mpi_release (mpi_g);
  point_free (&pk.E.G);
  _gcry_mpi_release (pk.E.n);
  _gcry_mpi_release (mpi_q);
  point_free (&pk.Q);
  _gcry_mpi_release (data);
  _gcry_mpi_release (mpi_s);
  _gcry_mpi_release (mpi_e);
  xfree (curvename);
  _gcry_mpi_ec_free (ec);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("ecc_encrypt    => %s\n", gpg_strerror (rc));
  return rc;
}


/*  input:
 *     data[0] : a point kG (ephemeral public key)
 *   output:
 *     resaddr[0] : shared point kdG
 *
 *  see ecc_encrypt_raw for details.
 */
static gcry_err_code_t
ecc_decrypt_raw (gcry_sexp_t *r_plain, gcry_sexp_t s_data, gcry_sexp_t keyparms)
{
  gpg_err_code_t rc;
  struct pk_encoding_ctx ctx;
  gcry_sexp_t l1 = NULL;
  gcry_mpi_t data_e = NULL;
  ECC_secret_key sk;
  gcry_mpi_t mpi_g = NULL;
  char *curvename = NULL;
  mpi_ec_t ec = NULL;
  mpi_point_struct kG;
  mpi_point_struct R;
  gcry_mpi_t r = NULL;

  memset (&sk, 0, sizeof sk);
  point_init (&kG);
  point_init (&R);

  _gcry_pk_util_init_encoding_ctx (&ctx, PUBKEY_OP_DECRYPT,
                                   ecc_get_nbits (keyparms));

  /*
   * Extract the data.
   */
  rc = _gcry_pk_util_preparse_encval (s_data, ecc_names, &l1, &ctx);
  if (rc)
    goto leave;
  rc = sexp_extract_param (l1, NULL, "e", &data_e, NULL);
  if (rc)
    goto leave;
  if (DBG_CIPHER)
    log_printmpi ("ecc_decrypt  d_e", data_e);
  if (mpi_is_opaque (data_e))
    {
      rc = GPG_ERR_INV_DATA;
      goto leave;
    }

  /*
   * Extract the key.
   */
  rc = sexp_extract_param (keyparms, NULL, "-p?a?b?g?n?+d",
                           &sk.E.p, &sk.E.a, &sk.E.b, &mpi_g, &sk.E.n,
                           &sk.d, NULL);
  if (rc)
    goto leave;
  if (mpi_g)
    {
      point_init (&sk.E.G);
      rc = _gcry_ecc_os2ec (&sk.E.G, mpi_g);
      if (rc)
        goto leave;
    }
  /* Add missing parameters using the optional curve parameter.  */
  sexp_release (l1);
  l1 = sexp_find_token (keyparms, "curve", 5);
  if (l1)
    {
      curvename = sexp_nth_string (l1, 1);
      if (curvename)
        {
          rc = _gcry_ecc_fill_in_curve (0, curvename, &sk.E, NULL);
          if (rc)
            return rc;
        }
    }
  /* Guess required fields if a curve parameter has not been given.  */
  if (!curvename)
    {
      sk.E.model = MPI_EC_WEIERSTRASS;
      sk.E.dialect = ECC_DIALECT_STANDARD;
    }
  if (DBG_CIPHER)
    {
      log_debug ("ecc_decrypt info: %s/%s\n",
                 _gcry_ecc_model2str (sk.E.model),
                 _gcry_ecc_dialect2str (sk.E.dialect));
      if (sk.E.name)
        log_debug  ("ecc_decrypt name: %s\n", sk.E.name);
      log_printmpi ("ecc_decrypt    p", sk.E.p);
      log_printmpi ("ecc_decrypt    a", sk.E.a);
      log_printmpi ("ecc_decrypt    b", sk.E.b);
      log_printpnt ("ecc_decrypt  g",   &sk.E.G, NULL);
      log_printmpi ("ecc_decrypt    n", sk.E.n);
      if (!fips_mode ())
        log_printmpi ("ecc_decrypt    d", sk.d);
    }
  if (!sk.E.p || !sk.E.a || !sk.E.b || !sk.E.G.x || !sk.E.n || !sk.d)
    {
      rc = GPG_ERR_NO_OBJ;
      goto leave;
    }


  /*
   * Compute the plaintext.
   */
  rc = _gcry_ecc_os2ec (&kG, data_e);
  if (rc)
    {
      point_free (&kG);
      return rc;
    }

  ec = _gcry_mpi_ec_p_internal_new (sk.E.model, sk.E.dialect, 0,
                                    sk.E.p, sk.E.a, sk.E.b);

  /* R = dkG */
  _gcry_mpi_ec_mul_point (&R, sk.d, &kG, ec);

  /* The following is false: assert( mpi_cmp_ui( R.x, 1 )==0 );, so:  */
  {
    gcry_mpi_t x, y;

    x = mpi_new (0);
    y = mpi_new (0);

    if (_gcry_mpi_ec_get_affine (x, y, &R, ec))
      log_fatal ("ecdh: Failed to get affine coordinates\n");

    r = _gcry_ecc_ec2os (x, y, sk.E.p);
    if (!r)
      rc = gpg_err_code_from_syserror ();
    else
      rc = 0;
    mpi_free (x);
    mpi_free (y);
  }
  if (DBG_CIPHER)
    log_printmpi ("ecc_decrypt  res", r);

  if (!rc)
    rc = sexp_build (r_plain, NULL, "(value %m)", r);

 leave:
  point_free (&R);
  point_free (&kG);
  _gcry_mpi_release (r);
  _gcry_mpi_release (sk.E.p);
  _gcry_mpi_release (sk.E.a);
  _gcry_mpi_release (sk.E.b);
  _gcry_mpi_release (mpi_g);
  point_free (&sk.E.G);
  _gcry_mpi_release (sk.E.n);
  _gcry_mpi_release (sk.d);
  _gcry_mpi_release (data_e);
  xfree (curvename);
  sexp_release (l1);
  _gcry_mpi_ec_free (ec);
  _gcry_pk_util_free_encoding_ctx (&ctx);
  if (DBG_CIPHER)
    log_debug ("ecc_decrypt    => %s\n", gpg_strerror (rc));
  return rc;
}


/* Return the number of bits for the key described by PARMS.  On error
 * 0 is returned.  The format of PARMS starts with the algorithm name;
 * for example:
 *
 *   (ecc
 *     (p <mpi>)
 *     (a <mpi>)
 *     (b <mpi>)
 *     (g <mpi>)
 *     (n <mpi>)
 *     (q <mpi>))
 *
 * More parameters may be given currently P is needed.  FIXME: We
 * need allow for a "curve" parameter.
 */
static unsigned int
ecc_get_nbits (gcry_sexp_t parms)
{
  gcry_sexp_t l1;
  gcry_mpi_t p;
  unsigned int nbits = 0;
  char *curve;

  l1 = sexp_find_token (parms, "p", 1);
  if (!l1)
    { /* Parameter P not found - check whether we have "curve".  */
      l1 = sexp_find_token (parms, "curve", 5);
      if (!l1)
        return 0; /* Neither P nor CURVE found.  */

      curve = sexp_nth_string (l1, 1);
      sexp_release (l1);
      if (!curve)
        return 0;  /* No curve name given (or out of core). */

      if (_gcry_ecc_fill_in_curve (0, curve, NULL, &nbits))
        nbits = 0;
      xfree (curve);
    }
  else
    {
      p = sexp_nth_mpi (l1, 1, GCRYMPI_FMT_USG);
      sexp_release (l1);
      if (p)
        {
          nbits = mpi_get_nbits (p);
          _gcry_mpi_release (p);
        }
    }
  return nbits;
}


/* See rsa.c for a description of this function.  */
static gpg_err_code_t
compute_keygrip (gcry_md_hd_t md, gcry_sexp_t keyparms)
{
#define N_COMPONENTS 6
  static const char names[N_COMPONENTS+1] = "pabgnq";
  gpg_err_code_t rc;
  gcry_sexp_t l1;
  gcry_mpi_t values[N_COMPONENTS];
  int idx;
  char *curvename = NULL;
  int flags = 0;
  enum gcry_mpi_ec_models model = 0;
  enum ecc_dialects dialect = 0;

  /* Clear the values first.  */
  for (idx=0; idx < N_COMPONENTS; idx++)
    values[idx] = NULL;


  /* Look for flags. */
  l1 = sexp_find_token (keyparms, "flags", 0);
  if (l1)
    {
      rc = _gcry_pk_util_parse_flaglist (l1, &flags, NULL);
      if (rc)
        goto leave;
    }

  /* Extract the parameters.  */
  if ((flags & PUBKEY_FLAG_PARAM))
    {
      if ((flags & PUBKEY_FLAG_EDDSA))
        rc = sexp_extract_param (keyparms, NULL, "p?a?b?g?n?/q",
                                 &values[0], &values[1], &values[2],
                                 &values[3], &values[4], &values[5],
                                 NULL);
      else
        rc = sexp_extract_param (keyparms, NULL, "p?a?b?g?n?q",
                                 &values[0], &values[1], &values[2],
                                 &values[3], &values[4], &values[5],
                                 NULL);
    }
  else
    {
      if ((flags & PUBKEY_FLAG_EDDSA))
        rc = sexp_extract_param (keyparms, NULL, "/q",
                                 &values[5], NULL);
      else
        rc = sexp_extract_param (keyparms, NULL, "q",
                                 &values[5], NULL);
    }
  if (rc)
    goto leave;

  /* Check whether a curve parameter is available and use that to fill
     in missing values.  */
  sexp_release (l1);
  l1 = sexp_find_token (keyparms, "curve", 5);
  if (l1)
    {
      curvename = sexp_nth_string (l1, 1);
      if (curvename)
        {
          rc = _gcry_ecc_update_curve_param (curvename,
                                             &model, &dialect,
                                             &values[0], &values[1], &values[2],
                                             &values[3], &values[4]);
          if (rc)
            goto leave;
        }
    }

  /* Guess required fields if a curve parameter has not been given.
     FIXME: This is a crude hacks.  We need to fix that.  */
  if (!curvename)
    {
      model = ((flags & PUBKEY_FLAG_EDDSA)
               ? MPI_EC_TWISTEDEDWARDS
               : MPI_EC_WEIERSTRASS);
      dialect = ((flags & PUBKEY_FLAG_EDDSA)
                 ? ECC_DIALECT_ED25519
                 : ECC_DIALECT_STANDARD);
    }

  /* Check that all parameters are known and normalize all MPIs (that
     should not be required but we use an internal function later and
     thus we better make 100% sure that they are normalized). */
  for (idx = 0; idx < N_COMPONENTS; idx++)
    if (!values[idx])
      {
        rc = GPG_ERR_NO_OBJ;
        goto leave;
      }
    else
      _gcry_mpi_normalize (values[idx]);

  /* Uncompress the public key with the exception of EdDSA where
     compression is the default and we thus compute the keygrip using
     the compressed version.  Because we don't support any non-eddsa
     compression, the only thing we need to do is to compress
     EdDSA.  */
  if ((flags & PUBKEY_FLAG_EDDSA))
    {
      if (dialect == ECC_DIALECT_ED25519)
        rc = _gcry_ecc_eddsa_ensure_compact (values[5], 256);
      else
        rc = GPG_ERR_NOT_IMPLEMENTED;
      if (rc)
        goto leave;
    }

  /* Hash them all.  */
  for (idx = 0; idx < N_COMPONENTS; idx++)
    {
      char buf[30];

      if (mpi_is_opaque (values[idx]))
        {
          const unsigned char *raw;
          unsigned int n;

          raw = mpi_get_opaque (values[idx], &n);
          n = (n + 7)/8;
          snprintf (buf, sizeof buf, "(1:%c%u:", names[idx], n);
          _gcry_md_write (md, buf, strlen (buf));
          _gcry_md_write (md, raw, n);
          _gcry_md_write (md, ")", 1);
        }
      else
        {
          unsigned char *rawmpi;
          unsigned int rawmpilen;

          rawmpi = _gcry_mpi_get_buffer (values[idx], 0, &rawmpilen, NULL);
          if (!rawmpi)
            {
              rc = gpg_err_code_from_syserror ();
              goto leave;
            }
          snprintf (buf, sizeof buf, "(1:%c%u:", names[idx], rawmpilen);
          _gcry_md_write (md, buf, strlen (buf));
          _gcry_md_write (md, rawmpi, rawmpilen);
          _gcry_md_write (md, ")", 1);
          xfree (rawmpi);
        }
    }

 leave:
  xfree (curvename);
  sexp_release (l1);
  for (idx = 0; idx < N_COMPONENTS; idx++)
    _gcry_mpi_release (values[idx]);

  return rc;
#undef N_COMPONENTS
}



/*
   Low-level API helper functions.
 */

/* This is the worker function for gcry_pubkey_get_sexp for ECC
   algorithms.  Note that the caller has already stored NULL at
   R_SEXP.  */
gpg_err_code_t
_gcry_pk_ecc_get_sexp (gcry_sexp_t *r_sexp, int mode, mpi_ec_t ec)
{
  gpg_err_code_t rc;
  gcry_mpi_t mpi_G = NULL;
  gcry_mpi_t mpi_Q = NULL;

  if (!ec->p || !ec->a || !ec->b || !ec->G || !ec->n)
    return GPG_ERR_BAD_CRYPT_CTX;

  if (mode == GCRY_PK_GET_SECKEY && !ec->d)
    return GPG_ERR_NO_SECKEY;

  /* Compute the public point if it is missing.  */
  if (!ec->Q && ec->d)
    ec->Q = _gcry_ecc_compute_public (NULL, ec, NULL, NULL);

  /* Encode G and Q.  */
  mpi_G = _gcry_mpi_ec_ec2os (ec->G, ec);
  if (!mpi_G)
    {
      rc = GPG_ERR_BROKEN_PUBKEY;
      goto leave;
    }
  if (!ec->Q)
    {
      rc = GPG_ERR_BAD_CRYPT_CTX;
      goto leave;
    }

  if (ec->dialect == ECC_DIALECT_ED25519)
    {
      unsigned char *encpk;
      unsigned int encpklen;

      rc = _gcry_ecc_eddsa_encodepoint (ec->Q, ec, NULL, NULL, 0,
                                        &encpk, &encpklen);
      if (rc)
        goto leave;
      mpi_Q = mpi_set_opaque (NULL, encpk, encpklen*8);
      encpk = NULL;
    }
  else
    {
      mpi_Q = _gcry_mpi_ec_ec2os (ec->Q, ec);
    }
  if (!mpi_Q)
    {
      rc = GPG_ERR_BROKEN_PUBKEY;
      goto leave;
    }

  /* Fixme: We should return a curve name instead of the parameters if
     if know that they match a curve.  */

  if (ec->d && (!mode || mode == GCRY_PK_GET_SECKEY))
    {
      /* Let's return a private key. */
      rc = sexp_build (r_sexp, NULL,
                       "(private-key(ecc(p%m)(a%m)(b%m)(g%m)(n%m)(q%m)(d%m)))",
                       ec->p, ec->a, ec->b, mpi_G, ec->n, mpi_Q, ec->d);
    }
  else if (ec->Q)
    {
      /* Let's return a public key.  */
      rc = sexp_build (r_sexp, NULL,
                       "(public-key(ecc(p%m)(a%m)(b%m)(g%m)(n%m)(q%m)))",
                       ec->p, ec->a, ec->b, mpi_G, ec->n, mpi_Q);
    }
  else
    rc = GPG_ERR_BAD_CRYPT_CTX;

 leave:
  mpi_free (mpi_Q);
  mpi_free (mpi_G);
  return rc;
}



/*
     Self-test section.
 */


static gpg_err_code_t
selftests_ecdsa (selftest_report_func_t report)
{
  const char *what;
  const char *errtxt;

  what = "low-level";
  errtxt = NULL; /*selftest ();*/
  if (errtxt)
    goto failed;

  /* FIXME:  need more tests.  */

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("pubkey", GCRY_PK_ECC, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


/* Run a full self-test for ALGO and return 0 on success.  */
static gpg_err_code_t
run_selftests (int algo, int extended, selftest_report_func_t report)
{
  (void)extended;

  if (algo != GCRY_PK_ECC)
    return GPG_ERR_PUBKEY_ALGO;

  return selftests_ecdsa (report);
}




gcry_pk_spec_t _gcry_pubkey_spec_ecc =
  {
    GCRY_PK_ECC, { 0, 0 },
    (GCRY_PK_USAGE_SIGN | GCRY_PK_USAGE_ENCR),
    "ECC", ecc_names,
    "pabgnq", "pabgnqd", "sw", "rs", "pabgnq",
    ecc_generate,
    ecc_check_secret_key,
    ecc_encrypt_raw,
    ecc_decrypt_raw,
    ecc_sign,
    ecc_verify,
    ecc_get_nbits,
    run_selftests,
    compute_keygrip,
    _gcry_ecc_get_curve,
    _gcry_ecc_get_param_sexp
  };

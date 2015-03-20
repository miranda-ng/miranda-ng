/* ecc-ecdsa.c  -  Elliptic Curve ECDSA signatures
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


/* Compute an ECDSA signature.
 * Return the signature struct (r,s) from the message hash.  The caller
 * must have allocated R and S.
 */
gpg_err_code_t
_gcry_ecc_ecdsa_sign (gcry_mpi_t input, ECC_secret_key *skey,
                      gcry_mpi_t r, gcry_mpi_t s,
                      int flags, int hashalgo)
{
  gpg_err_code_t rc = 0;
  int extraloops = 0;
  gcry_mpi_t k, dr, sum, k_1, x;
  mpi_point_struct I;
  gcry_mpi_t hash;
  const void *abuf;
  unsigned int abits, qbits;
  mpi_ec_t ctx;

  if (DBG_CIPHER)
    log_mpidump ("ecdsa sign hash  ", input );

  qbits = mpi_get_nbits (skey->E.n);

  /* Convert the INPUT into an MPI if needed.  */
  rc = _gcry_dsa_normalize_hash (input, &hash, qbits);
  if (rc)
    return rc;

  k = NULL;
  dr = mpi_alloc (0);
  sum = mpi_alloc (0);
  k_1 = mpi_alloc (0);
  x = mpi_alloc (0);
  point_init (&I);

  ctx = _gcry_mpi_ec_p_internal_new (skey->E.model, skey->E.dialect, 0,
                                     skey->E.p, skey->E.a, skey->E.b);

  /* Two loops to avoid R or S are zero.  This is more of a joke than
     a real demand because the probability of them being zero is less
     than any hardware failure.  Some specs however require it.  */
  do
    {
      do
        {
          mpi_free (k);
          k = NULL;
          if ((flags & PUBKEY_FLAG_RFC6979) && hashalgo)
            {
              /* Use Pornin's method for deterministic DSA.  If this
                 flag is set, it is expected that HASH is an opaque
                 MPI with the to be signed hash.  That hash is also
                 used as h1 from 3.2.a.  */
              if (!mpi_is_opaque (input))
                {
                  rc = GPG_ERR_CONFLICT;
                  goto leave;
                }

              abuf = mpi_get_opaque (input, &abits);
              rc = _gcry_dsa_gen_rfc6979_k (&k, skey->E.n, skey->d,
                                            abuf, (abits+7)/8,
                                            hashalgo, extraloops);
              if (rc)
                goto leave;
              extraloops++;
            }
          else
            k = _gcry_dsa_gen_k (skey->E.n, GCRY_STRONG_RANDOM);

          _gcry_mpi_ec_mul_point (&I, k, &skey->E.G, ctx);
          if (_gcry_mpi_ec_get_affine (x, NULL, &I, ctx))
            {
              if (DBG_CIPHER)
                log_debug ("ecc sign: Failed to get affine coordinates\n");
              rc = GPG_ERR_BAD_SIGNATURE;
              goto leave;
            }
          mpi_mod (r, x, skey->E.n);  /* r = x mod n */
        }
      while (!mpi_cmp_ui (r, 0));

      mpi_mulm (dr, skey->d, r, skey->E.n); /* dr = d*r mod n  */
      mpi_addm (sum, hash, dr, skey->E.n);  /* sum = hash + (d*r) mod n  */
      mpi_invm (k_1, k, skey->E.n);         /* k_1 = k^(-1) mod n  */
      mpi_mulm (s, k_1, sum, skey->E.n);    /* s = k^(-1)*(hash+(d*r)) mod n */
    }
  while (!mpi_cmp_ui (s, 0));

  if (DBG_CIPHER)
    {
      log_mpidump ("ecdsa sign result r ", r);
      log_mpidump ("ecdsa sign result s ", s);
    }

 leave:
  _gcry_mpi_ec_free (ctx);
  point_free (&I);
  mpi_free (x);
  mpi_free (k_1);
  mpi_free (sum);
  mpi_free (dr);
  mpi_free (k);

  if (hash != input)
    mpi_free (hash);

  return rc;
}


/* Verify an ECDSA signature.
 * Check if R and S verifies INPUT.
 */
gpg_err_code_t
_gcry_ecc_ecdsa_verify (gcry_mpi_t input, ECC_public_key *pkey,
                        gcry_mpi_t r, gcry_mpi_t s)
{
  gpg_err_code_t err = 0;
  gcry_mpi_t hash, h, h1, h2, x;
  mpi_point_struct Q, Q1, Q2;
  mpi_ec_t ctx;
  unsigned int nbits;

  if( !(mpi_cmp_ui (r, 0) > 0 && mpi_cmp (r, pkey->E.n) < 0) )
    return GPG_ERR_BAD_SIGNATURE; /* Assertion	0 < r < n  failed.  */
  if( !(mpi_cmp_ui (s, 0) > 0 && mpi_cmp (s, pkey->E.n) < 0) )
    return GPG_ERR_BAD_SIGNATURE; /* Assertion	0 < s < n  failed.  */

  nbits = mpi_get_nbits (pkey->E.n);
  err = _gcry_dsa_normalize_hash (input, &hash, nbits);
  if (err)
    return err;

  h  = mpi_alloc (0);
  h1 = mpi_alloc (0);
  h2 = mpi_alloc (0);
  x = mpi_alloc (0);
  point_init (&Q);
  point_init (&Q1);
  point_init (&Q2);

  ctx = _gcry_mpi_ec_p_internal_new (pkey->E.model, pkey->E.dialect, 0,
                                     pkey->E.p, pkey->E.a, pkey->E.b);

  /* h  = s^(-1) (mod n) */
  mpi_invm (h, s, pkey->E.n);
  /* h1 = hash * s^(-1) (mod n) */
  mpi_mulm (h1, hash, h, pkey->E.n);
  /* Q1 = [ hash * s^(-1) ]G  */
  _gcry_mpi_ec_mul_point (&Q1, h1, &pkey->E.G, ctx);
  /* h2 = r * s^(-1) (mod n) */
  mpi_mulm (h2, r, h, pkey->E.n);
  /* Q2 = [ r * s^(-1) ]Q */
  _gcry_mpi_ec_mul_point (&Q2, h2, &pkey->Q, ctx);
  /* Q  = ([hash * s^(-1)]G) + ([r * s^(-1)]Q) */
  _gcry_mpi_ec_add_points (&Q, &Q1, &Q2, ctx);

  if (!mpi_cmp_ui (Q.z, 0))
    {
      if (DBG_CIPHER)
          log_debug ("ecc verify: Rejected\n");
      err = GPG_ERR_BAD_SIGNATURE;
      goto leave;
    }
  if (_gcry_mpi_ec_get_affine (x, NULL, &Q, ctx))
    {
      if (DBG_CIPHER)
        log_debug ("ecc verify: Failed to get affine coordinates\n");
      err = GPG_ERR_BAD_SIGNATURE;
      goto leave;
    }
  mpi_mod (x, x, pkey->E.n); /* x = x mod E_n */
  if (mpi_cmp (x, r))   /* x != r */
    {
      if (DBG_CIPHER)
        {
          log_mpidump ("     x", x);
          log_mpidump ("     r", r);
          log_mpidump ("     s", s);
        }
      err = GPG_ERR_BAD_SIGNATURE;
      goto leave;
    }

 leave:
  _gcry_mpi_ec_free (ctx);
  point_free (&Q2);
  point_free (&Q1);
  point_free (&Q);
  mpi_free (x);
  mpi_free (h2);
  mpi_free (h1);
  mpi_free (h);
  if (hash != input)
    mpi_free (hash);

  return err;
}

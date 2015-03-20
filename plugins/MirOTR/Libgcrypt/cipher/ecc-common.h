/* ecc-common.h - Declarations of common ECC code
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

#ifndef GCRY_ECC_COMMON_H
#define GCRY_ECC_COMMON_H


/* Definition of a curve.  */
typedef struct
{
  enum gcry_mpi_ec_models model;/* The model descrinbing this curve.  */
  enum ecc_dialects dialect;    /* The dialect used with the curve.   */
  gcry_mpi_t p;         /* Prime specifying the field GF(p).  */
  gcry_mpi_t a;         /* First coefficient of the Weierstrass equation.  */
  gcry_mpi_t b;         /* Second coefficient of the Weierstrass equation.
                           or d as used by Twisted Edwards curves.  */
  mpi_point_struct G;   /* Base point (generator).  */
  gcry_mpi_t n;         /* Order of G.  */
  const char *name;     /* Name of the curve or NULL.  */
} elliptic_curve_t;


typedef struct
{
  elliptic_curve_t E;
  mpi_point_struct Q; /* Q = [d]G  */
} ECC_public_key;


typedef struct
{
  elliptic_curve_t E;
  mpi_point_struct Q;
  gcry_mpi_t d;
} ECC_secret_key;



/* Set the value from S into D.  */
static inline void
point_set (mpi_point_t d, mpi_point_t s)
{
  mpi_set (d->x, s->x);
  mpi_set (d->y, s->y);
  mpi_set (d->z, s->z);
}

#define point_init(a)  _gcry_mpi_point_init ((a))
#define point_free(a)  _gcry_mpi_point_free_parts ((a))


/*-- ecc-curves.c --*/
gpg_err_code_t _gcry_ecc_fill_in_curve (unsigned int nbits,
                                        const char *name,
                                        elliptic_curve_t *curve,
                                        unsigned int *r_nbits);
gpg_err_code_t _gcry_ecc_update_curve_param (const char *name,
                                             enum gcry_mpi_ec_models *model,
                                             enum ecc_dialects *dialect,
                                             gcry_mpi_t *p, gcry_mpi_t *a,
                                             gcry_mpi_t *b, gcry_mpi_t *g,
                                             gcry_mpi_t *n);

const char *_gcry_ecc_get_curve (gcry_sexp_t keyparms,
                                 int iterator,
                                 unsigned int *r_nbits);
gcry_sexp_t _gcry_ecc_get_param_sexp (const char *name);

/*-- ecc-misc.c --*/
void _gcry_ecc_curve_free (elliptic_curve_t *E);
elliptic_curve_t _gcry_ecc_curve_copy (elliptic_curve_t E);
const char *_gcry_ecc_model2str (enum gcry_mpi_ec_models model);
const char *_gcry_ecc_dialect2str (enum ecc_dialects dialect);
gcry_mpi_t   _gcry_ecc_ec2os (gcry_mpi_t x, gcry_mpi_t y, gcry_mpi_t p);
gcry_err_code_t _gcry_ecc_os2ec (mpi_point_t result, gcry_mpi_t value);

mpi_point_t  _gcry_ecc_compute_public (mpi_point_t Q, mpi_ec_t ec,
                                       mpi_point_t G, gcry_mpi_t d);

/*-- ecc.c --*/

/*-- ecc-ecdsa.c --*/
gpg_err_code_t _gcry_ecc_ecdsa_sign (gcry_mpi_t input, ECC_secret_key *skey,
                                     gcry_mpi_t r, gcry_mpi_t s,
                                     int flags, int hashalgo);
gpg_err_code_t _gcry_ecc_ecdsa_verify (gcry_mpi_t input, ECC_public_key *pkey,
                                       gcry_mpi_t r, gcry_mpi_t s);

/*-- ecc-eddsa.c --*/
gpg_err_code_t _gcry_ecc_eddsa_recover_x (gcry_mpi_t x, gcry_mpi_t y, int sign,
                                          mpi_ec_t ec);
gpg_err_code_t _gcry_ecc_eddsa_encodepoint (mpi_point_t point, mpi_ec_t ctx,
                                            gcry_mpi_t x, gcry_mpi_t y,
                                            int with_prefix,
                                            unsigned char **r_buffer,
                                            unsigned int *r_buflen);
gpg_err_code_t _gcry_ecc_eddsa_ensure_compact (gcry_mpi_t value,
                                               unsigned int nbits);
gpg_err_code_t _gcry_ecc_eddsa_decodepoint (gcry_mpi_t pk, mpi_ec_t ctx,
                                            mpi_point_t result,
                                            unsigned char **r_encpk,
                                            unsigned int *r_encpklen);
gpg_err_code_t _gcry_ecc_eddsa_compute_h_d (unsigned char **r_digest,
                                            gcry_mpi_t d, mpi_ec_t ec);

gpg_err_code_t _gcry_ecc_eddsa_genkey (ECC_secret_key *sk,
                                       elliptic_curve_t *E,
                                       mpi_ec_t ctx,
                                       gcry_random_level_t random_level);
gpg_err_code_t _gcry_ecc_eddsa_sign (gcry_mpi_t input,
                                     ECC_secret_key *sk,
                                     gcry_mpi_t r_r, gcry_mpi_t s,
                                     int hashalgo, gcry_mpi_t pk);
gpg_err_code_t _gcry_ecc_eddsa_verify (gcry_mpi_t input,
                                       ECC_public_key *pk,
                                       gcry_mpi_t r, gcry_mpi_t s,
                                       int hashalgo, gcry_mpi_t pkmpi);

/*-- ecc-gost.c --*/
gpg_err_code_t _gcry_ecc_gost_sign (gcry_mpi_t input, ECC_secret_key *skey,
                                    gcry_mpi_t r, gcry_mpi_t s);
gpg_err_code_t _gcry_ecc_gost_verify (gcry_mpi_t input, ECC_public_key *pkey,
                                      gcry_mpi_t r, gcry_mpi_t s);


#endif /*GCRY_ECC_COMMON_H*/

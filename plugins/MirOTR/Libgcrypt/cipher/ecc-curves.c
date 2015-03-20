/* ecc-curves.c  -  Elliptic Curve parameter mangement
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


/* This tables defines aliases for curve names.  */
static const struct
{
  const char *name;  /* Our name.  */
  const char *other; /* Other name. */
} curve_aliases[] =
  {
  /*{ "Curve25519", "1.3.6.1.4.1.3029.1.5.1" },*/
    { "Ed25519",    "1.3.6.1.4.1.11591.15.1" },

    { "NIST P-192", "1.2.840.10045.3.1.1" }, /* X9.62 OID  */
    { "NIST P-192", "prime192v1" },          /* X9.62 name.  */
    { "NIST P-192", "secp192r1"  },          /* SECP name.  */
    { "NIST P-192", "nistp192"   },          /* rfc5656.  */

    { "NIST P-224", "secp224r1" },
    { "NIST P-224", "1.3.132.0.33" },        /* SECP OID.  */
    { "NIST P-224", "nistp224"   },          /* rfc5656.  */

    { "NIST P-256", "1.2.840.10045.3.1.7" }, /* From NIST SP 800-78-1.  */
    { "NIST P-256", "prime256v1" },
    { "NIST P-256", "secp256r1"  },
    { "NIST P-256", "nistp256"   },          /* rfc5656.  */

    { "NIST P-384", "secp384r1" },
    { "NIST P-384", "1.3.132.0.34" },
    { "NIST P-384", "nistp384"   },          /* rfc5656.  */

    { "NIST P-521", "secp521r1" },
    { "NIST P-521", "1.3.132.0.35" },
    { "NIST P-521", "nistp521"   },          /* rfc5656.  */

    { "brainpoolP160r1", "1.3.36.3.3.2.8.1.1.1" },
    { "brainpoolP192r1", "1.3.36.3.3.2.8.1.1.3" },
    { "brainpoolP224r1", "1.3.36.3.3.2.8.1.1.5" },
    { "brainpoolP256r1", "1.3.36.3.3.2.8.1.1.7" },
    { "brainpoolP320r1", "1.3.36.3.3.2.8.1.1.9" },
    { "brainpoolP384r1", "1.3.36.3.3.2.8.1.1.11"},
    { "brainpoolP512r1", "1.3.36.3.3.2.8.1.1.13"},

    { NULL, NULL}
  };


typedef struct
{
  const char *desc;           /* Description of the curve.  */
  unsigned int nbits;         /* Number of bits.  */
  unsigned int fips:1;        /* True if this is a FIPS140-2 approved curve. */

  /* The model describing this curve.  This is mainly used to select
     the group equation. */
  enum gcry_mpi_ec_models model;

  /* The actual ECC dialect used.  This is used for curve specific
     optimizations and to select encodings etc. */
  enum ecc_dialects dialect;

  const char *p;              /* The prime defining the field.  */
  const char *a, *b;          /* The coefficients.  For Twisted Edwards
                                 Curves b is used for d.  */
  const char *n;              /* The order of the base point.  */
  const char *g_x, *g_y;      /* Base point.  */
} ecc_domain_parms_t;


/* This static table defines all available curves.  */
static const ecc_domain_parms_t domain_parms[] =
  {
    {
      /* (-x^2 + y^2 = 1 + dx^2y^2) */
      "Ed25519", 256, 0,
      MPI_EC_TWISTEDEDWARDS, ECC_DIALECT_ED25519,
      "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED",
      "-0x01",
      "-0x2DFC9311D490018C7338BF8688861767FF8FF5B2BEBE27548A14B235ECA6874A",
      "0x1000000000000000000000000000000014DEF9DEA2F79CD65812631A5CF5D3ED",
      "0x216936D3CD6E53FEC0A4E231FDD6DC5C692CC7609525A7B2C9562D608F25D51A",
      "0x6666666666666666666666666666666666666666666666666666666666666658"
    },
    {
      "NIST P-192", 192, 1,
      MPI_EC_WEIERSTRASS, ECC_DIALECT_STANDARD,
      "0xfffffffffffffffffffffffffffffffeffffffffffffffff",
      "0xfffffffffffffffffffffffffffffffefffffffffffffffc",
      "0x64210519e59c80e70fa7e9ab72243049feb8deecc146b9b1",
      "0xffffffffffffffffffffffff99def836146bc9b1b4d22831",

      "0x188da80eb03090f67cbf20eb43a18800f4ff0afd82ff1012",
      "0x07192b95ffc8da78631011ed6b24cdd573f977a11e794811"
    },
    {
      "NIST P-224", 224, 1,
      MPI_EC_WEIERSTRASS, ECC_DIALECT_STANDARD,
      "0xffffffffffffffffffffffffffffffff000000000000000000000001",
      "0xfffffffffffffffffffffffffffffffefffffffffffffffffffffffe",
      "0xb4050a850c04b3abf54132565044b0b7d7bfd8ba270b39432355ffb4",
      "0xffffffffffffffffffffffffffff16a2e0b8f03e13dd29455c5c2a3d" ,

      "0xb70e0cbd6bb4bf7f321390b94a03c1d356c21122343280d6115c1d21",
      "0xbd376388b5f723fb4c22dfe6cd4375a05a07476444d5819985007e34"
    },
    {
      "NIST P-256", 256, 1,
      MPI_EC_WEIERSTRASS, ECC_DIALECT_STANDARD,
      "0xffffffff00000001000000000000000000000000ffffffffffffffffffffffff",
      "0xffffffff00000001000000000000000000000000fffffffffffffffffffffffc",
      "0x5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b",
      "0xffffffff00000000ffffffffffffffffbce6faada7179e84f3b9cac2fc632551",

      "0x6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",
      "0x4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5"
    },
    {
      "NIST P-384", 384, 1,
      MPI_EC_WEIERSTRASS, ECC_DIALECT_STANDARD,
      "0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe"
      "ffffffff0000000000000000ffffffff",
      "0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe"
      "ffffffff0000000000000000fffffffc",
      "0xb3312fa7e23ee7e4988e056be3f82d19181d9c6efe8141120314088f5013875a"
      "c656398d8a2ed19d2a85c8edd3ec2aef",
      "0xffffffffffffffffffffffffffffffffffffffffffffffffc7634d81f4372ddf"
      "581a0db248b0a77aecec196accc52973",

      "0xaa87ca22be8b05378eb1c71ef320ad746e1d3b628ba79b9859f741e082542a38"
      "5502f25dbf55296c3a545e3872760ab7",
      "0x3617de4a96262c6f5d9e98bf9292dc29f8f41dbd289a147ce9da3113b5f0b8c0"
      "0a60b1ce1d7e819d7a431d7c90ea0e5f"
    },
    {
      "NIST P-521", 521, 1,
      MPI_EC_WEIERSTRASS, ECC_DIALECT_STANDARD,
      "0x01ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
      "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
      "0x01ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
      "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc",
      "0x051953eb9618e1c9a1f929a21a0b68540eea2da725b99b315f3b8b489918ef10"
      "9e156193951ec7e937b1652c0bd3bb1bf073573df883d2c34f1ef451fd46b503f00",
      "0x1fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
      "ffa51868783bf2f966b7fcc0148f709a5d03bb5c9b8899c47aebb6fb71e91386409",

      "0x00c6858e06b70404e9cd9e3ecb662395b4429c648139053fb521f828af606b4d"
      "3dbaa14b5e77efe75928fe1dc127a2ffa8de3348b3c1856a429bf97e7e31c2e5bd66",
      "0x011839296a789a3bc0045c8a5fb42c7d1bd998f54449579b446817afbd17273e"
      "662c97ee72995ef42640c550b9013fad0761353c7086a272c24088be94769fd16650"
    },

    { "brainpoolP160r1", 160, 0,
      MPI_EC_WEIERSTRASS, ECC_DIALECT_STANDARD,
      "0xe95e4a5f737059dc60dfc7ad95b3d8139515620f",
      "0x340e7be2a280eb74e2be61bada745d97e8f7c300",
      "0x1e589a8595423412134faa2dbdec95c8d8675e58",
      "0xe95e4a5f737059dc60df5991d45029409e60fc09",
      "0xbed5af16ea3f6a4f62938c4631eb5af7bdbcdbc3",
      "0x1667cb477a1a8ec338f94741669c976316da6321"
    },

    { "brainpoolP192r1", 192, 0,
      MPI_EC_WEIERSTRASS, ECC_DIALECT_STANDARD,
      "0xc302f41d932a36cda7a3463093d18db78fce476de1a86297",
      "0x6a91174076b1e0e19c39c031fe8685c1cae040e5c69a28ef",
      "0x469a28ef7c28cca3dc721d044f4496bcca7ef4146fbf25c9",
      "0xc302f41d932a36cda7a3462f9e9e916b5be8f1029ac4acc1",
      "0xc0a0647eaab6a48753b033c56cb0f0900a2f5c4853375fd6",
      "0x14b690866abd5bb88b5f4828c1490002e6773fa2fa299b8f"
    },

    { "brainpoolP224r1", 224, 0,
      MPI_EC_WEIERSTRASS, ECC_DIALECT_STANDARD,
      "0xd7c134aa264366862a18302575d1d787b09f075797da89f57ec8c0ff",
      "0x68a5e62ca9ce6c1c299803a6c1530b514e182ad8b0042a59cad29f43",
      "0x2580f63ccfe44138870713b1a92369e33e2135d266dbb372386c400b",
      "0xd7c134aa264366862a18302575d0fb98d116bc4b6ddebca3a5a7939f",
      "0x0d9029ad2c7e5cf4340823b2a87dc68c9e4ce3174c1e6efdee12c07d",
      "0x58aa56f772c0726f24c6b89e4ecdac24354b9e99caa3f6d3761402cd"
    },

    { "brainpoolP256r1", 256, 0,
      MPI_EC_WEIERSTRASS, ECC_DIALECT_STANDARD,
      "0xa9fb57dba1eea9bc3e660a909d838d726e3bf623d52620282013481d1f6e5377",
      "0x7d5a0975fc2c3057eef67530417affe7fb8055c126dc5c6ce94a4b44f330b5d9",
      "0x26dc5c6ce94a4b44f330b5d9bbd77cbf958416295cf7e1ce6bccdc18ff8c07b6",
      "0xa9fb57dba1eea9bc3e660a909d838d718c397aa3b561a6f7901e0e82974856a7",
      "0x8bd2aeb9cb7e57cb2c4b482ffc81b7afb9de27e1e3bd23c23a4453bd9ace3262",
      "0x547ef835c3dac4fd97f8461a14611dc9c27745132ded8e545c1d54c72f046997"
    },

    { "brainpoolP320r1", 320, 0,
      MPI_EC_WEIERSTRASS, ECC_DIALECT_STANDARD,
      "0xd35e472036bc4fb7e13c785ed201e065f98fcfa6f6f40def4f92b9ec7893ec28"
      "fcd412b1f1b32e27",
      "0x3ee30b568fbab0f883ccebd46d3f3bb8a2a73513f5eb79da66190eb085ffa9f4"
      "92f375a97d860eb4",
      "0x520883949dfdbc42d3ad198640688a6fe13f41349554b49acc31dccd88453981"
      "6f5eb4ac8fb1f1a6",
      "0xd35e472036bc4fb7e13c785ed201e065f98fcfa5b68f12a32d482ec7ee8658e9"
      "8691555b44c59311",
      "0x43bd7e9afb53d8b85289bcc48ee5bfe6f20137d10a087eb6e7871e2a10a599c7"
      "10af8d0d39e20611",
      "0x14fdd05545ec1cc8ab4093247f77275e0743ffed117182eaa9c77877aaac6ac7"
      "d35245d1692e8ee1"
    },

    { "brainpoolP384r1", 384, 0,
      MPI_EC_WEIERSTRASS, ECC_DIALECT_STANDARD,
      "0x8cb91e82a3386d280f5d6f7e50e641df152f7109ed5456b412b1da197fb71123"
      "acd3a729901d1a71874700133107ec53",
      "0x7bc382c63d8c150c3c72080ace05afa0c2bea28e4fb22787139165efba91f90f"
      "8aa5814a503ad4eb04a8c7dd22ce2826",
      "0x04a8c7dd22ce28268b39b55416f0447c2fb77de107dcd2a62e880ea53eeb62d5"
      "7cb4390295dbc9943ab78696fa504c11",
      "0x8cb91e82a3386d280f5d6f7e50e641df152f7109ed5456b31f166e6cac0425a7"
      "cf3ab6af6b7fc3103b883202e9046565",
      "0x1d1c64f068cf45ffa2a63a81b7c13f6b8847a3e77ef14fe3db7fcafe0cbd10e8"
      "e826e03436d646aaef87b2e247d4af1e",
      "0x8abe1d7520f9c2a45cb1eb8e95cfd55262b70b29feec5864e19c054ff9912928"
      "0e4646217791811142820341263c5315"
    },

    { "brainpoolP512r1", 512, 0,
      MPI_EC_WEIERSTRASS, ECC_DIALECT_STANDARD,
      "0xaadd9db8dbe9c48b3fd4e6ae33c9fc07cb308db3b3c9d20ed6639cca70330871"
      "7d4d9b009bc66842aecda12ae6a380e62881ff2f2d82c68528aa6056583a48f3",
      "0x7830a3318b603b89e2327145ac234cc594cbdd8d3df91610a83441caea9863bc"
      "2ded5d5aa8253aa10a2ef1c98b9ac8b57f1117a72bf2c7b9e7c1ac4d77fc94ca",
      "0x3df91610a83441caea9863bc2ded5d5aa8253aa10a2ef1c98b9ac8b57f1117a7"
      "2bf2c7b9e7c1ac4d77fc94cadc083e67984050b75ebae5dd2809bd638016f723",
      "0xaadd9db8dbe9c48b3fd4e6ae33c9fc07cb308db3b3c9d20ed6639cca70330870"
      "553e5c414ca92619418661197fac10471db1d381085ddaddb58796829ca90069",
      "0x81aee4bdd82ed9645a21322e9c4c6a9385ed9f70b5d916c1b43b62eef4d0098e"
      "ff3b1f78e2d0d48d50d1687b93b97d5f7c6d5047406a5e688b352209bcb9f822",
      "0x7dde385d566332ecc0eabfa9cf7822fdf209f70024a57b1aa000c55b881f8111"
      "b2dcde494a5f485e5bca4bd88a2763aed1ca2b2fa8f0540678cd1e0f3ad80892"
    },
    {
      "GOST2001-test", 256, 0,
      MPI_EC_WEIERSTRASS, ECC_DIALECT_STANDARD,
      "0x8000000000000000000000000000000000000000000000000000000000000431",
      "0x0000000000000000000000000000000000000000000000000000000000000007",
      "0x5fbff498aa938ce739b8e022fbafef40563f6e6a3472fc2a514c0ce9dae23b7e",
      "0x8000000000000000000000000000000150fe8a1892976154c59cfc193accf5b3",

      "0x0000000000000000000000000000000000000000000000000000000000000002",
      "0x08e2a8a0e65147d4bd6316030e16d19c85c97f0a9ca267122b96abbcea7e8fc8",
    },

    {
      "GOST2012-test", 511, 0,
      MPI_EC_WEIERSTRASS, ECC_DIALECT_STANDARD,
      "0x4531acd1fe0023c7550d267b6b2fee80922b14b2ffb90f04d4eb7c09b5d2d15d"
      "f1d852741af4704a0458047e80e4546d35b8336fac224dd81664bbf528be6373",
      "0x0000000000000000000000000000000000000000000000000000000000000007",
      "0x1cff0806a31116da29d8cfa54e57eb748bc5f377e49400fdd788b649eca1ac4"
      "361834013b2ad7322480a89ca58e0cf74bc9e540c2add6897fad0a3084f302adc",
      "0x4531acd1fe0023c7550d267b6b2fee80922b14b2ffb90f04d4eb7c09b5d2d15d"
      "a82f2d7ecb1dbac719905c5eecc423f1d86e25edbe23c595d644aaf187e6e6df",

      "0x24d19cc64572ee30f396bf6ebbfd7a6c5213b3b3d7057cc825f91093a68cd762"
      "fd60611262cd838dc6b60aa7eee804e28bc849977fac33b4b530f1b120248a9a",
      "0x2bb312a43bd2ce6e0d020613c857acddcfbf061e91e5f2c3f32447c259f39b2"
      "c83ab156d77f1496bf7eb3351e1ee4e43dc1a18b91b24640b6dbb92cb1add371e",
    },

    { NULL, 0, 0, 0, 0, NULL, NULL, NULL, NULL }
  };




/* Return a copy of POINT.  */
static gcry_mpi_point_t
point_copy (gcry_mpi_point_t point)
{
  gcry_mpi_point_t newpoint;

  if (point)
    {
      newpoint = mpi_point_new (0);
      point_set (newpoint, point);
    }
  else
    newpoint = NULL;
  return newpoint;
}


/* Helper to scan a hex string. */
static gcry_mpi_t
scanval (const char *string)
{
  gpg_err_code_t rc;
  gcry_mpi_t val;

  rc = _gcry_mpi_scan (&val, GCRYMPI_FMT_HEX, string, 0, NULL);
  if (rc)
    log_fatal ("scanning ECC parameter failed: %s\n", gpg_strerror (rc));
  return val;
}


/* Return the index of the domain_parms table for a curve with NAME.
   Return -1 if not found.  */
static int
find_domain_parms_idx (const char *name)
{
  int idx, aliasno;

  /* First check our native curves.  */
  for (idx = 0; domain_parms[idx].desc; idx++)
    if (!strcmp (name, domain_parms[idx].desc))
      return idx;

  /* If not found consult the alias table.  */
  if (!domain_parms[idx].desc)
    {
      for (aliasno = 0; curve_aliases[aliasno].name; aliasno++)
        if (!strcmp (name, curve_aliases[aliasno].other))
          break;
      if (curve_aliases[aliasno].name)
        {
          for (idx = 0; domain_parms[idx].desc; idx++)
            if (!strcmp (curve_aliases[aliasno].name, domain_parms[idx].desc))
              return idx;
        }
    }

  return -1;
}


/* Generate the crypto system setup.  This function takes the NAME of
   a curve or the desired number of bits and stores at R_CURVE the
   parameters of the named curve or those of a suitable curve.  If
   R_NBITS is not NULL, the chosen number of bits is stored there.
   NULL may be given for R_CURVE, if the value is not required and for
   example only a quick test for availability is desired.  Note that
   the curve fields should be initialized to zero because fields which
   are not NULL are skipped.  */
gpg_err_code_t
_gcry_ecc_fill_in_curve (unsigned int nbits, const char *name,
                         elliptic_curve_t *curve, unsigned int *r_nbits)
{
  int idx;
  const char *resname = NULL; /* Set to a found curve name.  */

  if (name)
    idx = find_domain_parms_idx (name);
  else
    {
      for (idx = 0; domain_parms[idx].desc; idx++)
        if (nbits == domain_parms[idx].nbits
            && domain_parms[idx].model == MPI_EC_WEIERSTRASS)
          break;
      if (!domain_parms[idx].desc)
        idx = -1;
    }
  if (idx < 0)
    return GPG_ERR_UNKNOWN_CURVE;

  resname = domain_parms[idx].desc;

  /* In fips mode we only support NIST curves.  Note that it is
     possible to bypass this check by specifying the curve parameters
     directly.  */
  if (fips_mode () && !domain_parms[idx].fips )
    return GPG_ERR_NOT_SUPPORTED;

  switch (domain_parms[idx].model)
    {
    case MPI_EC_WEIERSTRASS:
    case MPI_EC_TWISTEDEDWARDS:
      break;
    case MPI_EC_MONTGOMERY:
      return GPG_ERR_NOT_SUPPORTED;
    default:
      return GPG_ERR_BUG;
    }


  if (r_nbits)
    *r_nbits = domain_parms[idx].nbits;

  if (curve)
    {
      curve->model = domain_parms[idx].model;
      curve->dialect = domain_parms[idx].dialect;
      if (!curve->p)
        curve->p = scanval (domain_parms[idx].p);
      if (!curve->a)
        curve->a = scanval (domain_parms[idx].a);
      if (!curve->b)
        curve->b = scanval (domain_parms[idx].b);
      if (!curve->n)
        curve->n = scanval (domain_parms[idx].n);
      if (!curve->G.x)
        curve->G.x = scanval (domain_parms[idx].g_x);
      if (!curve->G.y)
        curve->G.y = scanval (domain_parms[idx].g_y);
      if (!curve->G.z)
        curve->G.z = mpi_alloc_set_ui (1);
      if (!curve->name)
        curve->name = resname;
    }

  return 0;
}


/* Give the name of the curve NAME, store the curve parameters into P,
   A, B, G, and N if they point to NULL value.  Note that G is returned
   in standard uncompressed format.  Also update MODEL and DIALECT if
   they are not NULL. */
gpg_err_code_t
_gcry_ecc_update_curve_param (const char *name,
                              enum gcry_mpi_ec_models *model,
                              enum ecc_dialects *dialect,
                              gcry_mpi_t *p, gcry_mpi_t *a, gcry_mpi_t *b,
                              gcry_mpi_t *g, gcry_mpi_t *n)
{
  int idx;

  idx = find_domain_parms_idx (name);
  if (idx < 0)
    return GPG_ERR_UNKNOWN_CURVE;

  if (g)
    {
      char *buf;
      size_t len;

      len = 4;
      len += strlen (domain_parms[idx].g_x+2);
      len += strlen (domain_parms[idx].g_y+2);
      len++;
      buf = xtrymalloc (len);
      if (!buf)
        return gpg_err_code_from_syserror ();
      strcpy (stpcpy (stpcpy (buf, "0x04"), domain_parms[idx].g_x+2),
              domain_parms[idx].g_y+2);
      _gcry_mpi_release (*g);
      *g = scanval (buf);
      xfree (buf);
    }
  if (model)
    *model = domain_parms[idx].model;
  if (dialect)
    *dialect = domain_parms[idx].dialect;
  if (p)
    {
      _gcry_mpi_release (*p);
      *p = scanval (domain_parms[idx].p);
    }
  if (a)
    {
      _gcry_mpi_release (*a);
      *a = scanval (domain_parms[idx].a);
    }
  if (b)
    {
      _gcry_mpi_release (*b);
      *b = scanval (domain_parms[idx].b);
    }
  if (n)
    {
      _gcry_mpi_release (*n);
      *n = scanval (domain_parms[idx].n);
    }
  return 0;
}


/* Return the name matching the parameters in PKEY.  This works only
   with curves described by the Weierstrass equation. */
const char *
_gcry_ecc_get_curve (gcry_sexp_t keyparms, int iterator, unsigned int *r_nbits)
{
  gpg_err_code_t rc;
  const char *result = NULL;
  elliptic_curve_t E;
  gcry_mpi_t mpi_g = NULL;
  gcry_mpi_t tmp = NULL;
  int idx;

  memset (&E, 0, sizeof E);

  if (r_nbits)
    *r_nbits = 0;

  if (!keyparms)
    {
      idx = iterator;
      if (idx >= 0 && idx < DIM (domain_parms))
        {
          result = domain_parms[idx].desc;
          if (r_nbits)
            *r_nbits = domain_parms[idx].nbits;
        }
      return result;
    }


  /*
   * Extract the curve parameters..
   */
  rc = gpg_err_code (sexp_extract_param (keyparms, NULL, "-pabgn",
                                         &E.p, &E.a, &E.b, &mpi_g, &E.n,
                                         NULL));
  if (rc == GPG_ERR_NO_OBJ)
    {
      /* This might be the second use case of checking whether a
         specific curve given by name is supported.  */
      gcry_sexp_t l1;
      char *name;

      l1 = sexp_find_token (keyparms, "curve", 5);
      if (!l1)
        goto leave;  /* No curve name parameter.  */

      name = sexp_nth_string (l1, 1);
      sexp_release (l1);
      if (!name)
        goto leave;  /* Name missing or out of core. */

      idx = find_domain_parms_idx (name);
      xfree (name);
      if (idx >= 0)  /* Curve found.  */
        {
          result = domain_parms[idx].desc;
          if (r_nbits)
            *r_nbits = domain_parms[idx].nbits;
        }
      return result;
    }

  if (rc)
    goto leave;

  if (mpi_g)
    {
      _gcry_mpi_point_init (&E.G);
      if (_gcry_ecc_os2ec (&E.G, mpi_g))
        goto leave;
    }

  for (idx = 0; domain_parms[idx].desc; idx++)
    {
      mpi_free (tmp);
      tmp = scanval (domain_parms[idx].p);
      if (!mpi_cmp (tmp, E.p))
        {
          mpi_free (tmp);
          tmp = scanval (domain_parms[idx].a);
          if (!mpi_cmp (tmp, E.a))
            {
              mpi_free (tmp);
              tmp = scanval (domain_parms[idx].b);
              if (!mpi_cmp (tmp, E.b))
                {
                  mpi_free (tmp);
                  tmp = scanval (domain_parms[idx].n);
                  if (!mpi_cmp (tmp, E.n))
                    {
                      mpi_free (tmp);
                      tmp = scanval (domain_parms[idx].g_x);
                      if (!mpi_cmp (tmp, E.G.x))
                        {
                          mpi_free (tmp);
                          tmp = scanval (domain_parms[idx].g_y);
                          if (!mpi_cmp (tmp, E.G.y))
                            {
                              result = domain_parms[idx].desc;
                              if (r_nbits)
                                *r_nbits = domain_parms[idx].nbits;
                              goto leave;
                            }
                        }
                    }
                }
            }
        }
    }

 leave:
  _gcry_mpi_release (tmp);
  _gcry_mpi_release (E.p);
  _gcry_mpi_release (E.a);
  _gcry_mpi_release (E.b);
  _gcry_mpi_release (mpi_g);
  _gcry_mpi_point_free_parts (&E.G);
  _gcry_mpi_release (E.n);
  return result;
}


/* Helper to extract an MPI from key parameters.  */
static gpg_err_code_t
mpi_from_keyparam (gcry_mpi_t *r_a, gcry_sexp_t keyparam, const char *name)
{
  gcry_err_code_t ec = 0;
  gcry_sexp_t l1;

  l1 = sexp_find_token (keyparam, name, 0);
  if (l1)
    {
      *r_a = sexp_nth_mpi (l1, 1, GCRYMPI_FMT_USG);
      sexp_release (l1);
      if (!*r_a)
        ec = GPG_ERR_INV_OBJ;
    }
  return ec;
}

/* Helper to extract a point from key parameters.  If no parameter
   with NAME is found, the functions tries to find a non-encoded point
   by appending ".x", ".y" and ".z" to NAME.  ".z" is in this case
   optional and defaults to 1.  EC is the context which at this point
   may not be fully initialized. */
static gpg_err_code_t
point_from_keyparam (gcry_mpi_point_t *r_a,
                     gcry_sexp_t keyparam, const char *name, mpi_ec_t ec)
{
  gcry_err_code_t rc;
  gcry_sexp_t l1;
  gcry_mpi_point_t point;

  l1 = sexp_find_token (keyparam, name, 0);
  if (l1)
    {
      gcry_mpi_t a;

      a = sexp_nth_mpi (l1, 1, GCRYMPI_FMT_OPAQUE);
      sexp_release (l1);
      if (!a)
        return GPG_ERR_INV_OBJ;

      point = mpi_point_new (0);
      if (ec && ec->dialect == ECC_DIALECT_ED25519)
        rc = _gcry_ecc_eddsa_decodepoint (a, ec, point, NULL, NULL);
      else
        rc = _gcry_ecc_os2ec (point, a);
      mpi_free (a);
      if (rc)
        {
          mpi_point_release (point);
          return rc;
        }
    }
  else
    {
      char *tmpname;
      gcry_mpi_t x = NULL;
      gcry_mpi_t y = NULL;
      gcry_mpi_t z = NULL;

      tmpname = xtrymalloc (strlen (name) + 2 + 1);
      if (!tmpname)
        return gpg_err_code_from_syserror ();
      strcpy (stpcpy (tmpname, name), ".x");
      rc = mpi_from_keyparam (&x, keyparam, tmpname);
      if (rc)
        {
          xfree (tmpname);
          return rc;
        }
      strcpy (stpcpy (tmpname, name), ".y");
      rc = mpi_from_keyparam (&y, keyparam, tmpname);
      if (rc)
        {
          mpi_free (x);
          xfree (tmpname);
          return rc;
        }
      strcpy (stpcpy (tmpname, name), ".z");
      rc = mpi_from_keyparam (&z, keyparam, tmpname);
      if (rc)
        {
          mpi_free (y);
          mpi_free (x);
          xfree (tmpname);
          return rc;
        }
      if (!z)
        z = mpi_set_ui (NULL, 1);
      if (x && y)
        point = mpi_point_snatch_set (NULL, x, y, z);
      else
        {
          mpi_free (x);
          mpi_free (y);
          mpi_free (z);
          point = NULL;
        }
      xfree (tmpname);
    }

  if (point)
    *r_a = point;
  return 0;
}


/* This function creates a new context for elliptic curve operations.
   Either KEYPARAM or CURVENAME must be given.  If both are given and
   KEYPARAM has no curve parameter, CURVENAME is used to add missing
   parameters.  On success 0 is returned and the new context stored at
   R_CTX.  On error NULL is stored at R_CTX and an error code is
   returned.  The context needs to be released using
   gcry_ctx_release.  */
gpg_err_code_t
_gcry_mpi_ec_new (gcry_ctx_t *r_ctx,
                  gcry_sexp_t keyparam, const char *curvename)
{
  gpg_err_code_t errc;
  gcry_ctx_t ctx = NULL;
  enum gcry_mpi_ec_models model = MPI_EC_WEIERSTRASS;
  enum ecc_dialects dialect = ECC_DIALECT_STANDARD;
  gcry_mpi_t p = NULL;
  gcry_mpi_t a = NULL;
  gcry_mpi_t b = NULL;
  gcry_mpi_point_t G = NULL;
  gcry_mpi_t n = NULL;
  gcry_mpi_point_t Q = NULL;
  gcry_mpi_t d = NULL;
  int flags = 0;
  gcry_sexp_t l1;

  *r_ctx = NULL;

  if (keyparam)
    {
      /* Parse an optional flags list.  */
      l1 = sexp_find_token (keyparam, "flags", 0);
      if (l1)
        {
          errc = _gcry_pk_util_parse_flaglist (l1, &flags, NULL);
          sexp_release (l1);
          l1 = NULL;
          if (errc)
            goto leave;
        }

      /* Check whether a curve name was given.  */
      l1 = sexp_find_token (keyparam, "curve", 5);

      /* If we don't have a curve name or if override parameters have
         explicitly been requested, parse them.  */
      if (!l1 || (flags & PUBKEY_FLAG_PARAM))
        {
          errc = mpi_from_keyparam (&p, keyparam, "p");
          if (errc)
            goto leave;
          errc = mpi_from_keyparam (&a, keyparam, "a");
          if (errc)
            goto leave;
          errc = mpi_from_keyparam (&b, keyparam, "b");
          if (errc)
            goto leave;
          errc = point_from_keyparam (&G, keyparam, "g", NULL);
          if (errc)
            goto leave;
          errc = mpi_from_keyparam (&n, keyparam, "n");
          if (errc)
            goto leave;
        }
    }
  else
    l1 = NULL; /* No curvename.  */

  /* Check whether a curve parameter is available and use that to fill
     in missing values.  If no curve parameter is available try an
     optional provided curvename.  If only the curvename has been
     given use that one. */
  if (l1 || curvename)
    {
      char *name;
      elliptic_curve_t *E;

      if (l1)
        {
          name = sexp_nth_string (l1, 1);
          sexp_release (l1);
          if (!name)
            {
              errc = GPG_ERR_INV_OBJ; /* Name missing or out of core. */
              goto leave;
            }
        }
      else
        name = NULL;

      E = xtrycalloc (1, sizeof *E);
      if (!E)
        {
          errc = gpg_err_code_from_syserror ();
          xfree (name);
          goto leave;
        }

      errc = _gcry_ecc_fill_in_curve (0, name? name : curvename, E, NULL);
      xfree (name);
      if (errc)
        {
          xfree (E);
          goto leave;
        }

      model = E->model;
      dialect = E->dialect;

      if (!p)
        {
          p = E->p;
          E->p = NULL;
        }
      if (!a)
        {
          a = E->a;
          E->a = NULL;
        }
      if (!b)
        {
          b = E->b;
          E->b = NULL;
        }
      if (!G)
        {
          G = mpi_point_snatch_set (NULL, E->G.x, E->G.y, E->G.z);
          E->G.x = NULL;
          E->G.y = NULL;
          E->G.z = NULL;
        }
      if (!n)
        {
          n = E->n;
          E->n = NULL;
        }
      _gcry_ecc_curve_free (E);
      xfree (E);
    }


  errc = _gcry_mpi_ec_p_new (&ctx, model, dialect, flags, p, a, b);
  if (!errc)
    {
      mpi_ec_t ec = _gcry_ctx_get_pointer (ctx, CONTEXT_TYPE_EC);

      if (b)
        {
          mpi_free (ec->b);
          ec->b = b;
          b = NULL;
        }
      if (G)
        {
          ec->G = G;
          G = NULL;
        }
      if (n)
        {
          ec->n = n;
          n = NULL;
        }

      /* Now that we know the curve name we can look for the public key
         Q.  point_from_keyparam needs to know the curve parameters so
         that it is able to use the correct decompression.  Parsing
         the private key D could have been done earlier but it is less
         surprising if we do it here as well.  */
      if (keyparam)
        {
          errc = point_from_keyparam (&Q, keyparam, "q", ec);
          if (errc)
            goto leave;
          errc = mpi_from_keyparam (&d, keyparam, "d");
          if (errc)
            goto leave;
        }

      if (Q)
        {
          ec->Q = Q;
          Q = NULL;
        }
      if (d)
        {
          ec->d = d;
          d = NULL;
        }

      *r_ctx = ctx;
      ctx = NULL;
    }

 leave:
  _gcry_ctx_release (ctx);
  mpi_free (p);
  mpi_free (a);
  mpi_free (b);
  _gcry_mpi_point_release (G);
  mpi_free (n);
  _gcry_mpi_point_release (Q);
  mpi_free (d);
  return errc;
}


/* Return the parameters of the curve NAME as an S-expression.  */
gcry_sexp_t
_gcry_ecc_get_param_sexp (const char *name)
{
  unsigned int nbits;
  elliptic_curve_t E;
  mpi_ec_t ctx;
  gcry_mpi_t g_x, g_y;
  gcry_mpi_t pkey[6];
  gcry_sexp_t result;
  int i;

  memset (&E, 0, sizeof E);
  if (_gcry_ecc_fill_in_curve (0, name, &E, &nbits))
    return NULL;

  g_x = mpi_new (0);
  g_y = mpi_new (0);
  ctx = _gcry_mpi_ec_p_internal_new (MPI_EC_WEIERSTRASS,
                                     ECC_DIALECT_STANDARD,
                                     0,
                                     E.p, E.a, NULL);
  if (_gcry_mpi_ec_get_affine (g_x, g_y, &E.G, ctx))
    log_fatal ("ecc get param: Failed to get affine coordinates\n");
  _gcry_mpi_ec_free (ctx);
  _gcry_mpi_point_free_parts (&E.G);

  pkey[0] = E.p;
  pkey[1] = E.a;
  pkey[2] = E.b;
  pkey[3] = _gcry_ecc_ec2os (g_x, g_y, E.p);
  pkey[4] = E.n;
  pkey[5] = NULL;

  mpi_free (g_x);
  mpi_free (g_y);

  if (sexp_build (&result, NULL,
                  "(public-key(ecc(p%m)(a%m)(b%m)(g%m)(n%m)))",
                  pkey[0], pkey[1], pkey[2], pkey[3], pkey[4]))
    result = NULL;

  for (i=0; pkey[i]; i++)
    _gcry_mpi_release (pkey[i]);

  return result;
}


/* Return an MPI (or opaque MPI) described by NAME and the context EC.
   If COPY is true a copy is returned, if not a const MPI may be
   returned.  In any case mpi_free must be used.  */
gcry_mpi_t
_gcry_ecc_get_mpi (const char *name, mpi_ec_t ec, int copy)
{
  if (!*name)
    return NULL;

  if (!strcmp (name, "p") && ec->p)
    return mpi_is_const (ec->p) && !copy? ec->p : mpi_copy (ec->p);
  if (!strcmp (name, "a") && ec->a)
    return mpi_is_const (ec->a) && !copy? ec->a : mpi_copy (ec->a);
  if (!strcmp (name, "b") && ec->b)
    return mpi_is_const (ec->b) && !copy? ec->b : mpi_copy (ec->b);
  if (!strcmp (name, "n") && ec->n)
    return mpi_is_const (ec->n) && !copy? ec->n : mpi_copy (ec->n);
  if (!strcmp (name, "d") && ec->d)
    return mpi_is_const (ec->d) && !copy? ec->d : mpi_copy (ec->d);

  /* Return a requested point coordinate.  */
  if (!strcmp (name, "g.x") && ec->G && ec->G->x)
    return mpi_is_const (ec->G->x) && !copy? ec->G->x : mpi_copy (ec->G->x);
  if (!strcmp (name, "g.y") && ec->G && ec->G->y)
    return mpi_is_const (ec->G->y) && !copy? ec->G->y : mpi_copy (ec->G->y);
  if (!strcmp (name, "q.x") && ec->Q && ec->Q->x)
    return mpi_is_const (ec->Q->x) && !copy? ec->Q->x : mpi_copy (ec->Q->x);
  if (!strcmp (name, "q.y") && ec->Q && ec->Q->y)
    return mpi_is_const (ec->G->y) && !copy? ec->Q->y : mpi_copy (ec->Q->y);

  /* If the base point has been requested, return it in standard
     encoding.  */
  if (!strcmp (name, "g") && ec->G)
    return _gcry_mpi_ec_ec2os (ec->G, ec);

  /* If the public key has been requested, return it by default in
     standard uncompressed encoding or if requested in other
     encodings.  */
  if (*name == 'q' && (!name[1] || name[1] == '@'))
    {
      /* If only the private key is given, compute the public key.  */
      if (!ec->Q)
        ec->Q = _gcry_ecc_compute_public (NULL, ec, NULL, NULL);

      if (!ec->Q)
        return NULL;

      if (name[1] != '@')
        return _gcry_mpi_ec_ec2os (ec->Q, ec);

      if (!strcmp (name+2, "eddsa") && ec->model == MPI_EC_TWISTEDEDWARDS)
        {
          unsigned char *encpk;
          unsigned int encpklen;

          if (!_gcry_ecc_eddsa_encodepoint (ec->Q, ec, NULL, NULL, 0,
                                            &encpk, &encpklen))
            return mpi_set_opaque (NULL, encpk, encpklen*8);
        }
    }

  return NULL;
}


/* Return a point described by NAME and the context EC.  */
gcry_mpi_point_t
_gcry_ecc_get_point (const char *name, mpi_ec_t ec)
{
  if (!strcmp (name, "g") && ec->G)
    return point_copy (ec->G);
  if (!strcmp (name, "q"))
    {
      /* If only the private key is given, compute the public key.  */
      if (!ec->Q)
        ec->Q = _gcry_ecc_compute_public (NULL, ec, NULL, NULL);

      if (ec->Q)
        return point_copy (ec->Q);
    }

  return NULL;
}


/* Store the MPI NEWVALUE into the context EC under NAME. */
gpg_err_code_t
_gcry_ecc_set_mpi (const char *name, gcry_mpi_t newvalue, mpi_ec_t ec)
{
  gpg_err_code_t rc = 0;

  if (!*name)
    ;
  else if (!strcmp (name, "p"))
    {
      mpi_free (ec->p);
      ec->p = mpi_copy (newvalue);
      _gcry_mpi_ec_get_reset (ec);
    }
  else if (!strcmp (name, "a"))
    {
      mpi_free (ec->a);
      ec->a = mpi_copy (newvalue);
      _gcry_mpi_ec_get_reset (ec);
    }
  else if (!strcmp (name, "b"))
    {
      mpi_free (ec->b);
      ec->b = mpi_copy (newvalue);
    }
  else if (!strcmp (name, "n"))
    {
      mpi_free (ec->n);
      ec->n = mpi_copy (newvalue);
    }
  else if (*name == 'q' && (!name[1] || name[1] == '@'))
    {
      if (newvalue)
        {
          if (!ec->Q)
            ec->Q = mpi_point_new (0);
          if (ec->dialect == ECC_DIALECT_ED25519)
            rc = _gcry_ecc_eddsa_decodepoint (newvalue, ec, ec->Q, NULL, NULL);
          else
            rc = _gcry_ecc_os2ec (ec->Q, newvalue);
        }
      if (rc || !newvalue)
        {
          _gcry_mpi_point_release (ec->Q);
          ec->Q = NULL;
        }
      /* Note: We assume that Q matches d and thus do not reset d.  */
    }
  else if (!strcmp (name, "d"))
    {
      mpi_free (ec->d);
      ec->d = mpi_copy (newvalue);
      if (ec->d)
        {
          /* We need to reset the public key because it may not
             anymore match.  */
          _gcry_mpi_point_release (ec->Q);
          ec->Q = NULL;
        }
    }
  else
   rc = GPG_ERR_UNKNOWN_NAME;

  return rc;
}


/* Store the point NEWVALUE into the context EC under NAME.  */
gpg_err_code_t
_gcry_ecc_set_point (const char *name, gcry_mpi_point_t newvalue, mpi_ec_t ec)
{
  if (!strcmp (name, "g"))
    {
      _gcry_mpi_point_release (ec->G);
      ec->G = point_copy (newvalue);
    }
  else if (!strcmp (name, "q"))
    {
      _gcry_mpi_point_release (ec->Q);
      ec->Q = point_copy (newvalue);
    }
  else
    return GPG_ERR_UNKNOWN_NAME;

  return 0;
}

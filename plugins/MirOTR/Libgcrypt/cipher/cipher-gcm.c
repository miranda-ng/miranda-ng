/* cipher-gcm.c  - Generic Galois Counter Mode implementation
 * Copyright (C) 2013 Dmitry Eremin-Solenikov
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
#include "ath.h"
#include "bufhelp.h"
#include "./cipher-internal.h"

#ifdef GCM_USE_TABLES
static const u16 gcmR[256] = {
  0x0000, 0x01c2, 0x0384, 0x0246, 0x0708, 0x06ca, 0x048c, 0x054e,
  0x0e10, 0x0fd2, 0x0d94, 0x0c56, 0x0918, 0x08da, 0x0a9c, 0x0b5e,
  0x1c20, 0x1de2, 0x1fa4, 0x1e66, 0x1b28, 0x1aea, 0x18ac, 0x196e,
  0x1230, 0x13f2, 0x11b4, 0x1076, 0x1538, 0x14fa, 0x16bc, 0x177e,
  0x3840, 0x3982, 0x3bc4, 0x3a06, 0x3f48, 0x3e8a, 0x3ccc, 0x3d0e,
  0x3650, 0x3792, 0x35d4, 0x3416, 0x3158, 0x309a, 0x32dc, 0x331e,
  0x2460, 0x25a2, 0x27e4, 0x2626, 0x2368, 0x22aa, 0x20ec, 0x212e,
  0x2a70, 0x2bb2, 0x29f4, 0x2836, 0x2d78, 0x2cba, 0x2efc, 0x2f3e,
  0x7080, 0x7142, 0x7304, 0x72c6, 0x7788, 0x764a, 0x740c, 0x75ce,
  0x7e90, 0x7f52, 0x7d14, 0x7cd6, 0x7998, 0x785a, 0x7a1c, 0x7bde,
  0x6ca0, 0x6d62, 0x6f24, 0x6ee6, 0x6ba8, 0x6a6a, 0x682c, 0x69ee,
  0x62b0, 0x6372, 0x6134, 0x60f6, 0x65b8, 0x647a, 0x663c, 0x67fe,
  0x48c0, 0x4902, 0x4b44, 0x4a86, 0x4fc8, 0x4e0a, 0x4c4c, 0x4d8e,
  0x46d0, 0x4712, 0x4554, 0x4496, 0x41d8, 0x401a, 0x425c, 0x439e,
  0x54e0, 0x5522, 0x5764, 0x56a6, 0x53e8, 0x522a, 0x506c, 0x51ae,
  0x5af0, 0x5b32, 0x5974, 0x58b6, 0x5df8, 0x5c3a, 0x5e7c, 0x5fbe,
  0xe100, 0xe0c2, 0xe284, 0xe346, 0xe608, 0xe7ca, 0xe58c, 0xe44e,
  0xef10, 0xeed2, 0xec94, 0xed56, 0xe818, 0xe9da, 0xeb9c, 0xea5e,
  0xfd20, 0xfce2, 0xfea4, 0xff66, 0xfa28, 0xfbea, 0xf9ac, 0xf86e,
  0xf330, 0xf2f2, 0xf0b4, 0xf176, 0xf438, 0xf5fa, 0xf7bc, 0xf67e,
  0xd940, 0xd882, 0xdac4, 0xdb06, 0xde48, 0xdf8a, 0xddcc, 0xdc0e,
  0xd750, 0xd692, 0xd4d4, 0xd516, 0xd058, 0xd19a, 0xd3dc, 0xd21e,
  0xc560, 0xc4a2, 0xc6e4, 0xc726, 0xc268, 0xc3aa, 0xc1ec, 0xc02e,
  0xcb70, 0xcab2, 0xc8f4, 0xc936, 0xcc78, 0xcdba, 0xcffc, 0xce3e,
  0x9180, 0x9042, 0x9204, 0x93c6, 0x9688, 0x974a, 0x950c, 0x94ce,
  0x9f90, 0x9e52, 0x9c14, 0x9dd6, 0x9898, 0x995a, 0x9b1c, 0x9ade,
  0x8da0, 0x8c62, 0x8e24, 0x8fe6, 0x8aa8, 0x8b6a, 0x892c, 0x88ee,
  0x83b0, 0x8272, 0x8034, 0x81f6, 0x84b8, 0x857a, 0x873c, 0x86fe,
  0xa9c0, 0xa802, 0xaa44, 0xab86, 0xaec8, 0xaf0a, 0xad4c, 0xac8e,
  0xa7d0, 0xa612, 0xa454, 0xa596, 0xa0d8, 0xa11a, 0xa35c, 0xa29e,
  0xb5e0, 0xb422, 0xb664, 0xb7a6, 0xb2e8, 0xb32a, 0xb16c, 0xb0ae,
  0xbbf0, 0xba32, 0xb874, 0xb9b6, 0xbcf8, 0xbd3a, 0xbf7c, 0xbebe,
};

#ifdef GCM_TABLES_USE_U64
static void
bshift (u64 * b0, u64 * b1)
{
  u64 t[2], mask;

  t[0] = *b0;
  t[1] = *b1;
  mask = t[1] & 1 ? 0xe1 : 0;
  mask <<= 56;

  *b1 = (t[1] >> 1) ^ (t[0] << 63);
  *b0 = (t[0] >> 1) ^ mask;
}

static void
do_fillM (unsigned char *h, u64 *M)
{
  int i, j;

  M[0 + 0] = 0;
  M[0 + 16] = 0;

  M[8 + 0] = buf_get_be64 (h + 0);
  M[8 + 16] = buf_get_be64 (h + 8);

  for (i = 4; i > 0; i /= 2)
    {
      M[i + 0] = M[2 * i + 0];
      M[i + 16] = M[2 * i + 16];

      bshift (&M[i], &M[i + 16]);
    }

  for (i = 2; i < 16; i *= 2)
    for (j = 1; j < i; j++)
      {
        M[(i + j) + 0] = M[i + 0] ^ M[j + 0];
        M[(i + j) + 16] = M[i + 16] ^ M[j + 16];
      }
}

static inline unsigned int
do_ghash (unsigned char *result, const unsigned char *buf, const u64 *gcmM)
{
  u64 V[2];
  u64 tmp[2];
  const u64 *M;
  u64 T;
  u32 A;
  int i;

  buf_xor (V, result, buf, 16);
  V[0] = be_bswap64 (V[0]);
  V[1] = be_bswap64 (V[1]);

  /* First round can be manually tweaked based on fact that 'tmp' is zero. */
  i = 15;

  M = &gcmM[(V[1] & 0xf)];
  V[1] >>= 4;
  tmp[0] = (M[0] >> 4) ^ ((u64) gcmR[(M[16] & 0xf) << 4] << 48);
  tmp[1] = (M[16] >> 4) ^ (M[0] << 60);
  tmp[0] ^= gcmM[(V[1] & 0xf) + 0];
  tmp[1] ^= gcmM[(V[1] & 0xf) + 16];
  V[1] >>= 4;

  --i;
  while (1)
    {
      M = &gcmM[(V[1] & 0xf)];
      V[1] >>= 4;

      A = tmp[1] & 0xff;
      T = tmp[0];
      tmp[0] = (T >> 8) ^ ((u64) gcmR[A] << 48) ^ gcmM[(V[1] & 0xf) + 0];
      tmp[1] = (T << 56) ^ (tmp[1] >> 8) ^ gcmM[(V[1] & 0xf) + 16];

      tmp[0] ^= (M[0] >> 4) ^ ((u64) gcmR[(M[16] & 0xf) << 4] << 48);
      tmp[1] ^= (M[16] >> 4) ^ (M[0] << 60);

      if (i == 0)
        break;
      else if (i == 8)
        V[1] = V[0];
      else
        V[1] >>= 4;
      --i;
    }

  buf_put_be64 (result + 0, tmp[0]);
  buf_put_be64 (result + 8, tmp[1]);

  return (sizeof(V) + sizeof(T) + sizeof(tmp) +
          sizeof(int)*2 + sizeof(void*)*5);
}

#else

static void
bshift (u32 * M, int i)
{
  u32 t[4], mask;

  t[0] = M[i * 4 + 0];
  t[1] = M[i * 4 + 1];
  t[2] = M[i * 4 + 2];
  t[3] = M[i * 4 + 3];
  mask = t[3] & 1 ? 0xe1 : 0;

  M[i * 4 + 3] = (t[3] >> 1) ^ (t[2] << 31);
  M[i * 4 + 2] = (t[2] >> 1) ^ (t[1] << 31);
  M[i * 4 + 1] = (t[1] >> 1) ^ (t[0] << 31);
  M[i * 4 + 0] = (t[0] >> 1) ^ (mask << 24);
}

static void
do_fillM (unsigned char *h, u32 *M)
{
  int i, j;

  M[0 * 4 + 0] = 0;
  M[0 * 4 + 1] = 0;
  M[0 * 4 + 2] = 0;
  M[0 * 4 + 3] = 0;

  M[8 * 4 + 0] = buf_get_be32 (h + 0);
  M[8 * 4 + 1] = buf_get_be32 (h + 4);
  M[8 * 4 + 2] = buf_get_be32 (h + 8);
  M[8 * 4 + 3] = buf_get_be32 (h + 12);

  for (i = 4; i > 0; i /= 2)
    {
      M[i * 4 + 0] = M[2 * i * 4 + 0];
      M[i * 4 + 1] = M[2 * i * 4 + 1];
      M[i * 4 + 2] = M[2 * i * 4 + 2];
      M[i * 4 + 3] = M[2 * i * 4 + 3];

      bshift (M, i);
    }

  for (i = 2; i < 16; i *= 2)
    for (j = 1; j < i; j++)
      {
        M[(i + j) * 4 + 0] = M[i * 4 + 0] ^ M[j * 4 + 0];
        M[(i + j) * 4 + 1] = M[i * 4 + 1] ^ M[j * 4 + 1];
        M[(i + j) * 4 + 2] = M[i * 4 + 2] ^ M[j * 4 + 2];
        M[(i + j) * 4 + 3] = M[i * 4 + 3] ^ M[j * 4 + 3];
      }
}

static inline unsigned int
do_ghash (unsigned char *result, const unsigned char *buf, const u32 *gcmM)
{
  byte V[16];
  u32 tmp[4];
  u32 v;
  const u32 *M, *m;
  u32 T[3];
  int i;

  buf_xor (V, result, buf, 16); /* V is big-endian */

  /* First round can be manually tweaked based on fact that 'tmp' is zero. */
  i = 15;

  v = V[i];
  M = &gcmM[(v & 0xf) * 4];
  v = (v & 0xf0) >> 4;
  m = &gcmM[v * 4];
  v = V[--i];

  tmp[0] = (M[0] >> 4) ^ ((u64) gcmR[(M[3] << 4) & 0xf0] << 16) ^ m[0];
  tmp[1] = (M[1] >> 4) ^ (M[0] << 28) ^ m[1];
  tmp[2] = (M[2] >> 4) ^ (M[1] << 28) ^ m[2];
  tmp[3] = (M[3] >> 4) ^ (M[2] << 28) ^ m[3];

  while (1)
    {
      M = &gcmM[(v & 0xf) * 4];
      v = (v & 0xf0) >> 4;
      m = &gcmM[v * 4];

      T[0] = tmp[0];
      T[1] = tmp[1];
      T[2] = tmp[2];
      tmp[0] = (T[0] >> 8) ^ ((u32) gcmR[tmp[3] & 0xff] << 16) ^ m[0];
      tmp[1] = (T[0] << 24) ^ (tmp[1] >> 8) ^ m[1];
      tmp[2] = (T[1] << 24) ^ (tmp[2] >> 8) ^ m[2];
      tmp[3] = (T[2] << 24) ^ (tmp[3] >> 8) ^ m[3];

      tmp[0] ^= (M[0] >> 4) ^ ((u64) gcmR[(M[3] << 4) & 0xf0] << 16);
      tmp[1] ^= (M[1] >> 4) ^ (M[0] << 28);
      tmp[2] ^= (M[2] >> 4) ^ (M[1] << 28);
      tmp[3] ^= (M[3] >> 4) ^ (M[2] << 28);

      if (i == 0)
        break;

      v = V[--i];
    }

  buf_put_be32 (result + 0, tmp[0]);
  buf_put_be32 (result + 4, tmp[1]);
  buf_put_be32 (result + 8, tmp[2]);
  buf_put_be32 (result + 12, tmp[3]);

  return (sizeof(V) + sizeof(T) + sizeof(tmp) +
          sizeof(int)*2 + sizeof(void*)*6);
}
#endif /* !HAVE_U64_TYPEDEF || SIZEOF_UNSIGNED_LONG != 8 */

#define fillM(c, h) do_fillM (h, c->u_mode.gcm.gcm_table)
#define GHASH(c, result, buf) do_ghash (result, buf, c->u_mode.gcm.gcm_table)

#else

static unsigned long
bshift (unsigned long *b)
{
  unsigned long c;
  int i;
  c = b[3] & 1;
  for (i = 3; i > 0; i--)
    {
      b[i] = (b[i] >> 1) | (b[i - 1] << 31);
    }
  b[i] >>= 1;
  return c;
}

static unsigned int
do_ghash (unsigned char *hsub, unsigned char *result, const unsigned char *buf)
{
  unsigned long V[4];
  int i, j;
  byte *p;

#ifdef WORDS_BIGENDIAN
  p = result;
#else
  unsigned long T[4];

  buf_xor (V, result, buf, 16);
  for (i = 0; i < 4; i++)
    {
      V[i] = (V[i] & 0x00ff00ff) << 8 | (V[i] & 0xff00ff00) >> 8;
      V[i] = (V[i] & 0x0000ffff) << 16 | (V[i] & 0xffff0000) >> 16;
    }
  p = (byte *) T;
#endif

  memset (p, 0, 16);

  for (i = 0; i < 16; i++)
    {
      for (j = 0x80; j; j >>= 1)
        {
          if (hsub[i] & j)
            buf_xor (p, p, V, 16);
          if (bshift (V))
            V[0] ^= 0xe1000000;
        }
    }
#ifndef WORDS_BIGENDIAN
  for (i = 0, p = (byte *) T; i < 16; i += 4, p += 4)
    {
      result[i + 0] = p[3];
      result[i + 1] = p[2];
      result[i + 2] = p[1];
      result[i + 3] = p[0];
    }
#endif

  return (sizeof(V) + sizeof(T) + sizeof(int)*2 + sizeof(void*)*5);
}

#define fillM(c, h) do { } while (0)
#define GHASH(c, result, buf) do_ghash (c->u_mode.gcm.u_ghash_key.key, result, buf)

#endif /* !GCM_USE_TABLES */


#ifdef GCM_USE_INTEL_PCLMUL
/*
 Intel PCLMUL ghash based on white paper:
  "Intel® Carry-Less Multiplication Instruction and its Usage for Computing the
   GCM Mode - Rev 2.01"; Shay Gueron, Michael E. Kounavis.
 */
static inline void gfmul_pclmul(void)
{
  /* Input: XMM0 and XMM1, Output: XMM1. Input XMM0 stays unmodified.
     Input must be converted to little-endian.
   */
  asm volatile (/* gfmul, xmm0 has operator a and xmm1 has operator b. */
                "pshufd $78, %%xmm0, %%xmm2\n\t"
                "pshufd $78, %%xmm1, %%xmm4\n\t"
                "pxor %%xmm0, %%xmm2\n\t" /* xmm2 holds a0+a1 */
                "pxor %%xmm1, %%xmm4\n\t" /* xmm4 holds b0+b1 */

                "movdqa %%xmm0, %%xmm3\n\t"
                "pclmulqdq $0, %%xmm1, %%xmm3\n\t"  /* xmm3 holds a0*b0 */
                "movdqa %%xmm0, %%xmm6\n\t"
                "pclmulqdq $17, %%xmm1, %%xmm6\n\t" /* xmm6 holds a1*b1 */
                "movdqa %%xmm3, %%xmm5\n\t"
                "pclmulqdq $0, %%xmm2, %%xmm4\n\t"  /* xmm4 holds (a0+a1)*(b0+b1) */

                "pxor %%xmm6, %%xmm5\n\t" /* xmm5 holds a0*b0+a1*b1 */
                "pxor %%xmm5, %%xmm4\n\t" /* xmm4 holds a0*b0+a1*b1+(a0+a1)*(b0+b1) */
                "movdqa %%xmm4, %%xmm5\n\t"
                "psrldq $8, %%xmm4\n\t"
                "pslldq $8, %%xmm5\n\t"
                "pxor %%xmm5, %%xmm3\n\t"
                "pxor %%xmm4, %%xmm6\n\t" /* <xmm6:xmm3> holds the result of the
                                             carry-less multiplication of xmm0
                                             by xmm1 */

                /* shift the result by one bit position to the left cope for
                   the fact that bits are reversed */
                "movdqa %%xmm3, %%xmm4\n\t"
                "movdqa %%xmm6, %%xmm5\n\t"
                "pslld $1, %%xmm3\n\t"
                "pslld $1, %%xmm6\n\t"
                "psrld $31, %%xmm4\n\t"
                "psrld $31, %%xmm5\n\t"
                "movdqa %%xmm4, %%xmm1\n\t"
                "pslldq $4, %%xmm5\n\t"
                "pslldq $4, %%xmm4\n\t"
                "psrldq $12, %%xmm1\n\t"
                "por %%xmm4, %%xmm3\n\t"
                "por %%xmm5, %%xmm6\n\t"
                "por %%xmm6, %%xmm1\n\t"

                /* first phase of the reduction */
                "movdqa %%xmm3, %%xmm6\n\t"
                "movdqa %%xmm3, %%xmm7\n\t"
                "pslld $31, %%xmm6\n\t"  /* packed right shifting << 31 */
                "movdqa %%xmm3, %%xmm5\n\t"
                "pslld $30, %%xmm7\n\t"  /* packed right shifting shift << 30 */
                "pslld $25, %%xmm5\n\t"  /* packed right shifting shift << 25 */
                "pxor %%xmm7, %%xmm6\n\t" /* xor the shifted versions */
                "pxor %%xmm5, %%xmm6\n\t"
                "movdqa %%xmm6, %%xmm7\n\t"
                "pslldq $12, %%xmm6\n\t"
                "psrldq $4, %%xmm7\n\t"
                "pxor %%xmm6, %%xmm3\n\t" /* first phase of the reduction
                                             complete */

                /* second phase of the reduction */
                "movdqa %%xmm3, %%xmm2\n\t"
                "movdqa %%xmm3, %%xmm4\n\t"
                "psrld $1, %%xmm2\n\t"    /* packed left shifting >> 1 */
                "movdqa %%xmm3, %%xmm5\n\t"
                "psrld $2, %%xmm4\n\t"    /* packed left shifting >> 2 */
                "psrld $7, %%xmm5\n\t"    /* packed left shifting >> 7 */
                "pxor %%xmm4, %%xmm2\n\t" /* xor the shifted versions */
                "pxor %%xmm5, %%xmm2\n\t"
                "pxor %%xmm7, %%xmm2\n\t"
                "pxor %%xmm2, %%xmm3\n\t"
                "pxor %%xmm3, %%xmm1\n\t" /* the result is in xmm1 */
                ::: "cc" );
}

#ifdef __x86_64__
static inline void gfmul_pclmul_aggr4(void)
{
  /* Input:
      H¹: XMM0		X_i            : XMM6
      H²: XMM8		X_(i-1)        : XMM3
      H³: XMM9		X_(i-2)        : XMM2
      H⁴: XMM10		X_(i-3)⊕Y_(i-4): XMM1
     Output:
      Y_i: XMM1
     Inputs XMM0 stays unmodified.
     Input must be converted to little-endian.
   */
  asm volatile (/* perform clmul and merge results... */
                "pshufd $78, %%xmm10, %%xmm11\n\t"
                "pshufd $78, %%xmm1, %%xmm12\n\t"
                "pxor %%xmm10, %%xmm11\n\t" /* xmm11 holds 4:a0+a1 */
                "pxor %%xmm1, %%xmm12\n\t" /* xmm12 holds 4:b0+b1 */

                "pshufd $78, %%xmm9, %%xmm13\n\t"
                "pshufd $78, %%xmm2, %%xmm14\n\t"
                "pxor %%xmm9, %%xmm13\n\t" /* xmm13 holds 3:a0+a1 */
                "pxor %%xmm2, %%xmm14\n\t" /* xmm14 holds 3:b0+b1 */

                "pshufd $78, %%xmm8, %%xmm5\n\t"
                "pshufd $78, %%xmm3, %%xmm15\n\t"
                "pxor %%xmm8, %%xmm5\n\t" /* xmm1 holds 2:a0+a1 */
                "pxor %%xmm3, %%xmm15\n\t" /* xmm2 holds 2:b0+b1 */

                "movdqa %%xmm10, %%xmm4\n\t"
                "movdqa %%xmm9, %%xmm7\n\t"
                "pclmulqdq $0, %%xmm1, %%xmm4\n\t"   /* xmm4 holds 4:a0*b0 */
                "pclmulqdq $0, %%xmm2, %%xmm7\n\t"   /* xmm7 holds 3:a0*b0 */
                "pclmulqdq $17, %%xmm10, %%xmm1\n\t" /* xmm1 holds 4:a1*b1 */
                "pclmulqdq $17, %%xmm9, %%xmm2\n\t"  /* xmm9 holds 3:a1*b1 */
                "pclmulqdq $0, %%xmm11, %%xmm12\n\t" /* xmm12 holds 4:(a0+a1)*(b0+b1) */
                "pclmulqdq $0, %%xmm13, %%xmm14\n\t" /* xmm14 holds 3:(a0+a1)*(b0+b1) */

                "pshufd $78, %%xmm0, %%xmm10\n\t"
                "pshufd $78, %%xmm6, %%xmm11\n\t"
                "pxor %%xmm0, %%xmm10\n\t" /* xmm10 holds 1:a0+a1 */
                "pxor %%xmm6, %%xmm11\n\t" /* xmm11 holds 1:b0+b1 */

                "pxor %%xmm4, %%xmm7\n\t"   /* xmm7 holds 3+4:a0*b0 */
                "pxor %%xmm2, %%xmm1\n\t"   /* xmm1 holds 3+4:a1*b1 */
                "pxor %%xmm14, %%xmm12\n\t" /* xmm12 holds 3+4:(a0+a1)*(b0+b1) */

                "movdqa %%xmm8, %%xmm13\n\t"
                "pclmulqdq $0, %%xmm3, %%xmm13\n\t"  /* xmm13 holds 2:a0*b0 */
                "pclmulqdq $17, %%xmm8, %%xmm3\n\t"  /* xmm3 holds 2:a1*b1 */
                "pclmulqdq $0, %%xmm5, %%xmm15\n\t" /* xmm15 holds 2:(a0+a1)*(b0+b1) */

                "pxor %%xmm13, %%xmm7\n\t" /* xmm7 holds 2+3+4:a0*b0 */
                "pxor %%xmm3, %%xmm1\n\t"  /* xmm1 holds 2+3+4:a1*b1 */
                "pxor %%xmm15, %%xmm12\n\t" /* xmm12 holds 2+3+4:(a0+a1)*(b0+b1) */

                "movdqa %%xmm0, %%xmm3\n\t"
                "pclmulqdq $0, %%xmm6, %%xmm3\n\t"  /* xmm3 holds 1:a0*b0 */
                "pclmulqdq $17, %%xmm0, %%xmm6\n\t" /* xmm6 holds 1:a1*b1 */
                "movdqa %%xmm11, %%xmm4\n\t"
                "pclmulqdq $0, %%xmm10, %%xmm4\n\t" /* xmm4 holds 1:(a0+a1)*(b0+b1) */

                "pxor %%xmm7, %%xmm3\n\t"  /* xmm3 holds 1+2+3+4:a0*b0 */
                "pxor %%xmm1, %%xmm6\n\t"  /* xmm6 holds 1+2+3+4:a1*b1 */
                "pxor %%xmm12, %%xmm4\n\t" /* xmm4 holds 1+2+3+4:(a0+a1)*(b0+b1) */

                /* aggregated reduction... */
                "movdqa %%xmm3, %%xmm5\n\t"
                "pxor %%xmm6, %%xmm5\n\t" /* xmm5 holds a0*b0+a1*b1 */
                "pxor %%xmm5, %%xmm4\n\t" /* xmm4 holds a0*b0+a1*b1+(a0+a1)*(b0+b1) */
                "movdqa %%xmm4, %%xmm5\n\t"
                "psrldq $8, %%xmm4\n\t"
                "pslldq $8, %%xmm5\n\t"
                "pxor %%xmm5, %%xmm3\n\t"
                "pxor %%xmm4, %%xmm6\n\t" /* <xmm6:xmm3> holds the result of the
                                             carry-less multiplication of xmm0
                                             by xmm1 */

                /* shift the result by one bit position to the left cope for
                   the fact that bits are reversed */
                "movdqa %%xmm3, %%xmm4\n\t"
                "movdqa %%xmm6, %%xmm5\n\t"
                "pslld $1, %%xmm3\n\t"
                "pslld $1, %%xmm6\n\t"
                "psrld $31, %%xmm4\n\t"
                "psrld $31, %%xmm5\n\t"
                "movdqa %%xmm4, %%xmm1\n\t"
                "pslldq $4, %%xmm5\n\t"
                "pslldq $4, %%xmm4\n\t"
                "psrldq $12, %%xmm1\n\t"
                "por %%xmm4, %%xmm3\n\t"
                "por %%xmm5, %%xmm6\n\t"
                "por %%xmm6, %%xmm1\n\t"

                /* first phase of the reduction */
                "movdqa %%xmm3, %%xmm6\n\t"
                "movdqa %%xmm3, %%xmm7\n\t"
                "pslld $31, %%xmm6\n\t"  /* packed right shifting << 31 */
                "movdqa %%xmm3, %%xmm5\n\t"
                "pslld $30, %%xmm7\n\t"  /* packed right shifting shift << 30 */
                "pslld $25, %%xmm5\n\t"  /* packed right shifting shift << 25 */
                "pxor %%xmm7, %%xmm6\n\t" /* xor the shifted versions */
                "pxor %%xmm5, %%xmm6\n\t"
                "movdqa %%xmm6, %%xmm7\n\t"
                "pslldq $12, %%xmm6\n\t"
                "psrldq $4, %%xmm7\n\t"
                "pxor %%xmm6, %%xmm3\n\t" /* first phase of the reduction
                                             complete */

                /* second phase of the reduction */
                "movdqa %%xmm3, %%xmm2\n\t"
                "movdqa %%xmm3, %%xmm4\n\t"
                "psrld $1, %%xmm2\n\t"    /* packed left shifting >> 1 */
                "movdqa %%xmm3, %%xmm5\n\t"
                "psrld $2, %%xmm4\n\t"    /* packed left shifting >> 2 */
                "psrld $7, %%xmm5\n\t"    /* packed left shifting >> 7 */
                "pxor %%xmm4, %%xmm2\n\t" /* xor the shifted versions */
                "pxor %%xmm5, %%xmm2\n\t"
                "pxor %%xmm7, %%xmm2\n\t"
                "pxor %%xmm2, %%xmm3\n\t"
                "pxor %%xmm3, %%xmm1\n\t" /* the result is in xmm1 */
                :::"cc");
}
#endif

#endif /*GCM_USE_INTEL_PCLMUL*/


static unsigned int
ghash (gcry_cipher_hd_t c, byte *result, const byte *buf,
       size_t nblocks)
{
  const unsigned int blocksize = GCRY_GCM_BLOCK_LEN;
  unsigned int burn;

  if (nblocks == 0)
    return 0;

  if (0)
    ;
#ifdef GCM_USE_INTEL_PCLMUL
  else if (c->u_mode.gcm.use_intel_pclmul)
    {
      static const unsigned char be_mask[16] __attribute__ ((aligned (16))) =
        { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

      /* Preload hash and H1. */
      asm volatile ("movdqu %[hash], %%xmm1\n\t"
                    "movdqa %[hsub], %%xmm0\n\t"
                    "pshufb %[be_mask], %%xmm1\n\t" /* be => le */
                    :
                    : [hash] "m" (*result), [be_mask] "m" (*be_mask),
                      [hsub] "m" (*c->u_mode.gcm.u_ghash_key.key));

#ifdef __x86_64__
      if (nblocks >= 4)
        {
          do
            {
              asm volatile ("movdqa %[be_mask], %%xmm4\n\t"
                            "movdqu 0*16(%[buf]), %%xmm5\n\t"
                            "movdqu 1*16(%[buf]), %%xmm2\n\t"
                            "movdqu 2*16(%[buf]), %%xmm3\n\t"
                            "movdqu 3*16(%[buf]), %%xmm6\n\t"
                            "pshufb %%xmm4, %%xmm5\n\t" /* be => le */

                            /* Load H2, H3, H4. */
                            "movdqu 2*16(%[h_234]), %%xmm10\n\t"
                            "movdqu 1*16(%[h_234]), %%xmm9\n\t"
                            "movdqu 0*16(%[h_234]), %%xmm8\n\t"

                            "pxor %%xmm5, %%xmm1\n\t"
                            "pshufb %%xmm4, %%xmm2\n\t" /* be => le */
                            "pshufb %%xmm4, %%xmm3\n\t" /* be => le */
                            "pshufb %%xmm4, %%xmm6\n\t" /* be => le */
                            :
                            : [buf] "r" (buf), [be_mask] "m" (*be_mask),
                              [h_234] "r" (c->u_mode.gcm.gcm_table));

              gfmul_pclmul_aggr4 ();

              buf += 4 * blocksize;
              nblocks -= 4;
            }
          while (nblocks >= 4);

          /* Clear used x86-64/XMM registers. */
          asm volatile( "pxor %%xmm8, %%xmm8\n\t"
                        "pxor %%xmm9, %%xmm9\n\t"
                        "pxor %%xmm10, %%xmm10\n\t"
                        "pxor %%xmm11, %%xmm11\n\t"
                        "pxor %%xmm12, %%xmm12\n\t"
                        "pxor %%xmm13, %%xmm13\n\t"
                        "pxor %%xmm14, %%xmm14\n\t"
                        "pxor %%xmm15, %%xmm15\n\t"
                        ::: "cc" );
        }
#endif

      while (nblocks--)
        {
          asm volatile ("movdqu %[buf], %%xmm2\n\t"
                        "pshufb %[be_mask], %%xmm2\n\t" /* be => le */
                        "pxor %%xmm2, %%xmm1\n\t"
                        :
                        : [buf] "m" (*buf), [be_mask] "m" (*be_mask));

          gfmul_pclmul ();

          buf += blocksize;
        }

      /* Store hash. */
      asm volatile ("pshufb %[be_mask], %%xmm1\n\t" /* be => le */
                    "movdqu %%xmm1, %[hash]\n\t"
                    : [hash] "=m" (*result)
                    : [be_mask] "m" (*be_mask));

      /* Clear used registers. */
      asm volatile( "pxor %%xmm0, %%xmm0\n\t"
                    "pxor %%xmm1, %%xmm1\n\t"
                    "pxor %%xmm2, %%xmm2\n\t"
                    "pxor %%xmm3, %%xmm3\n\t"
                    "pxor %%xmm4, %%xmm4\n\t"
                    "pxor %%xmm5, %%xmm5\n\t"
                    "pxor %%xmm6, %%xmm6\n\t"
                    "pxor %%xmm7, %%xmm7\n\t"
                    ::: "cc" );
      burn = 0;
    }
#endif
  else
    {
      while (nblocks)
        {
          burn = GHASH (c, result, buf);
          buf += blocksize;
          nblocks--;
        }
    }

  return burn + (burn ? 5*sizeof(void*) : 0);
}


static void
setupM (gcry_cipher_hd_t c, byte *h)
{
  if (0)
    ;
#ifdef GCM_USE_INTEL_PCLMUL
  else if (_gcry_get_hw_features () & HWF_INTEL_PCLMUL)
    {
      u64 tmp[2];

      c->u_mode.gcm.use_intel_pclmul = 1;

      /* Swap endianness of hsub. */
      tmp[0] = buf_get_be64(c->u_mode.gcm.u_ghash_key.key + 8);
      tmp[1] = buf_get_be64(c->u_mode.gcm.u_ghash_key.key + 0);
      buf_cpy (c->u_mode.gcm.u_ghash_key.key, tmp, GCRY_GCM_BLOCK_LEN);

#ifdef __x86_64__
      asm volatile ("movdqu %[h_1], %%xmm0\n\t"
                    "movdqa %%xmm0, %%xmm1\n\t"
                    :
                    : [h_1] "m" (*tmp));

      gfmul_pclmul (); /* H•H => H² */

      asm volatile ("movdqu %%xmm1, 0*16(%[h_234])\n\t"
                    "movdqa %%xmm1, %%xmm8\n\t"
                    :
                    : [h_234] "r" (c->u_mode.gcm.gcm_table)
                    : "memory");

      gfmul_pclmul (); /* H•H² => H³ */

      asm volatile ("movdqa %%xmm8, %%xmm0\n\t"
                    "movdqu %%xmm1, 1*16(%[h_234])\n\t"
                    "movdqa %%xmm8, %%xmm1\n\t"
                    :
                    : [h_234] "r" (c->u_mode.gcm.gcm_table)
                    : "memory");

      gfmul_pclmul (); /* H²•H² => H⁴ */

      asm volatile ("movdqu %%xmm1, 2*16(%[h_234])\n\t"
                    :
                    : [h_234] "r" (c->u_mode.gcm.gcm_table)
                    : "memory");

      /* Clear used registers. */
      asm volatile( "pxor %%xmm0, %%xmm0\n\t"
                    "pxor %%xmm1, %%xmm1\n\t"
                    "pxor %%xmm2, %%xmm2\n\t"
                    "pxor %%xmm3, %%xmm3\n\t"
                    "pxor %%xmm4, %%xmm4\n\t"
                    "pxor %%xmm5, %%xmm5\n\t"
                    "pxor %%xmm6, %%xmm6\n\t"
                    "pxor %%xmm7, %%xmm7\n\t"
                    "pxor %%xmm8, %%xmm8\n\t"
                    ::: "cc" );
#endif

      wipememory (tmp, sizeof(tmp));
    }
#endif
  else
    fillM (c, h);
}


static inline void
gcm_bytecounter_add (u32 ctr[2], size_t add)
{
  if (sizeof(add) > sizeof(u32))
    {
      u32 high_add = ((add >> 31) >> 1) & 0xffffffff;
      ctr[1] += high_add;
    }

  ctr[0] += add;
  if (ctr[0] >= add)
    return;
  ++ctr[1];
}


static inline u32
gcm_add32_be128 (byte *ctr, unsigned int add)
{
  /* 'ctr' must be aligned to four bytes. */
  const unsigned int blocksize = GCRY_GCM_BLOCK_LEN;
  u32 *pval = (u32 *)(void *)(ctr + blocksize - sizeof(u32));
  u32 val;

  val = be_bswap32(*pval) + add;
  *pval = be_bswap32(val);

  return val; /* return result as host-endian value */
}


static inline int
gcm_check_datalen (u32 ctr[2])
{
  /* len(plaintext) <= 2^39-256 bits == 2^36-32 bytes == 2^32-2 blocks */
  if (ctr[1] > 0xfU)
    return 0;
  if (ctr[1] < 0xfU)
    return 1;

  if (ctr[0] <= 0xffffffe0U)
    return 1;

  return 0;
}


static inline int
gcm_check_aadlen_or_ivlen (u32 ctr[2])
{
  /* len(aad/iv) <= 2^64-1 bits ~= 2^61-1 bytes */
  if (ctr[1] > 0x1fffffffU)
    return 0;
  if (ctr[1] < 0x1fffffffU)
    return 1;

  if (ctr[0] <= 0xffffffffU)
    return 1;

  return 0;
}


static void
do_ghash_buf(gcry_cipher_hd_t c, byte *hash, const byte *buf,
             size_t buflen, int do_padding)
{
  unsigned int blocksize = GCRY_GCM_BLOCK_LEN;
  unsigned int unused = c->u_mode.gcm.mac_unused;
  size_t nblocks, n;
  unsigned int burn = 0;

  if (buflen == 0 && (unused == 0 || !do_padding))
    return;

  do
    {
      if (buflen + unused < blocksize || unused > 0)
        {
          n = blocksize - unused;
          n = n < buflen ? n : buflen;

          buf_cpy (&c->u_mode.gcm.macbuf[unused], buf, n);

          unused += n;
          buf += n;
          buflen -= n;
        }
      if (!buflen)
        {
          if (!do_padding)
            break;

          while (unused < blocksize)
            c->u_mode.gcm.macbuf[unused++] = 0;
        }

      if (unused > 0)
        {
          gcry_assert (unused == blocksize);

          /* Process one block from macbuf.  */
          burn = ghash (c, hash, c->u_mode.gcm.macbuf, 1);
          unused = 0;
        }

      nblocks = buflen / blocksize;

      if (nblocks)
        {
          burn = ghash (c, hash, buf, nblocks);
          buf += blocksize * nblocks;
          buflen -= blocksize * nblocks;
        }
    }
  while (buflen > 0);

  c->u_mode.gcm.mac_unused = unused;

  if (burn)
    _gcry_burn_stack (burn);
}


gcry_err_code_t
_gcry_cipher_gcm_encrypt (gcry_cipher_hd_t c,
                          byte *outbuf, size_t outbuflen,
                          const byte *inbuf, size_t inbuflen)
{
  static const unsigned char zerobuf[MAX_BLOCKSIZE];
  gcry_err_code_t err;

  if (c->spec->blocksize != GCRY_GCM_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (c->u_mode.gcm.datalen_over_limits)
    return GPG_ERR_INV_LENGTH;
  if (c->marks.tag || c->u_mode.gcm.ghash_data_finalized)
    return GPG_ERR_INV_STATE;

  if (!c->marks.iv)
    _gcry_cipher_gcm_setiv (c, zerobuf, GCRY_GCM_BLOCK_LEN);

  if (c->u_mode.gcm.disallow_encryption_because_of_setiv_in_fips_mode)
    return GPG_ERR_INV_STATE;

  if (!c->u_mode.gcm.ghash_aad_finalized)
    {
      /* Start of encryption marks end of AAD stream. */
      do_ghash_buf(c, c->u_mode.gcm.u_tag.tag, NULL, 0, 1);
      c->u_mode.gcm.ghash_aad_finalized = 1;
    }

  gcm_bytecounter_add(c->u_mode.gcm.datalen, inbuflen);
  if (!gcm_check_datalen(c->u_mode.gcm.datalen))
    {
      c->u_mode.gcm.datalen_over_limits = 1;
      return GPG_ERR_INV_LENGTH;
    }

  err = _gcry_cipher_ctr_encrypt(c, outbuf, outbuflen, inbuf, inbuflen);
  if (err != 0)
    return err;

  do_ghash_buf(c, c->u_mode.gcm.u_tag.tag, outbuf, inbuflen, 0);

  return 0;
}


gcry_err_code_t
_gcry_cipher_gcm_decrypt (gcry_cipher_hd_t c,
                          byte *outbuf, size_t outbuflen,
                          const byte *inbuf, size_t inbuflen)
{
  static const unsigned char zerobuf[MAX_BLOCKSIZE];

  if (c->spec->blocksize != GCRY_GCM_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (c->u_mode.gcm.datalen_over_limits)
    return GPG_ERR_INV_LENGTH;
  if (c->marks.tag || c->u_mode.gcm.ghash_data_finalized)
    return GPG_ERR_INV_STATE;

  if (!c->marks.iv)
    _gcry_cipher_gcm_setiv (c, zerobuf, GCRY_GCM_BLOCK_LEN);

  if (!c->u_mode.gcm.ghash_aad_finalized)
    {
      /* Start of decryption marks end of AAD stream. */
      do_ghash_buf(c, c->u_mode.gcm.u_tag.tag, NULL, 0, 1);
      c->u_mode.gcm.ghash_aad_finalized = 1;
    }

  gcm_bytecounter_add(c->u_mode.gcm.datalen, inbuflen);
  if (!gcm_check_datalen(c->u_mode.gcm.datalen))
    {
      c->u_mode.gcm.datalen_over_limits = 1;
      return GPG_ERR_INV_LENGTH;
    }

  do_ghash_buf(c, c->u_mode.gcm.u_tag.tag, inbuf, inbuflen, 0);

  return _gcry_cipher_ctr_encrypt(c, outbuf, outbuflen, inbuf, inbuflen);
}


gcry_err_code_t
_gcry_cipher_gcm_authenticate (gcry_cipher_hd_t c,
                               const byte * aadbuf, size_t aadbuflen)
{
  static const unsigned char zerobuf[MAX_BLOCKSIZE];

  if (c->spec->blocksize != GCRY_GCM_BLOCK_LEN)
    return GPG_ERR_CIPHER_ALGO;
  if (c->u_mode.gcm.datalen_over_limits)
    return GPG_ERR_INV_LENGTH;
  if (c->marks.tag || c->u_mode.gcm.ghash_aad_finalized ||
      c->u_mode.gcm.ghash_data_finalized)
    return GPG_ERR_INV_STATE;

  if (!c->marks.iv)
    _gcry_cipher_gcm_setiv (c, zerobuf, GCRY_GCM_BLOCK_LEN);

  gcm_bytecounter_add(c->u_mode.gcm.aadlen, aadbuflen);
  if (!gcm_check_aadlen_or_ivlen(c->u_mode.gcm.aadlen))
    {
      c->u_mode.gcm.datalen_over_limits = 1;
      return GPG_ERR_INV_LENGTH;
    }

  do_ghash_buf(c, c->u_mode.gcm.u_tag.tag, aadbuf, aadbuflen, 0);

  return 0;
}


void
_gcry_cipher_gcm_setkey (gcry_cipher_hd_t c)
{
  memset (c->u_mode.gcm.u_ghash_key.key, 0, GCRY_GCM_BLOCK_LEN);

  c->spec->encrypt (&c->context.c, c->u_mode.gcm.u_ghash_key.key,
                    c->u_mode.gcm.u_ghash_key.key);
  setupM (c, c->u_mode.gcm.u_ghash_key.key);
}


static gcry_err_code_t
_gcry_cipher_gcm_initiv (gcry_cipher_hd_t c, const byte *iv, size_t ivlen)
{
  memset (c->u_mode.gcm.aadlen, 0, sizeof(c->u_mode.gcm.aadlen));
  memset (c->u_mode.gcm.datalen, 0, sizeof(c->u_mode.gcm.datalen));
  memset (c->u_mode.gcm.u_tag.tag, 0, GCRY_GCM_BLOCK_LEN);
  c->u_mode.gcm.datalen_over_limits = 0;
  c->u_mode.gcm.ghash_data_finalized = 0;
  c->u_mode.gcm.ghash_aad_finalized = 0;

  if (ivlen == 0)
    return GPG_ERR_INV_LENGTH;

  if (ivlen != GCRY_GCM_BLOCK_LEN - 4)
    {
      u32 iv_bytes[2] = {0, 0};
      u32 bitlengths[2][2];

      memset(c->u_ctr.ctr, 0, GCRY_GCM_BLOCK_LEN);

      gcm_bytecounter_add(iv_bytes, ivlen);
      if (!gcm_check_aadlen_or_ivlen(iv_bytes))
        {
          c->u_mode.gcm.datalen_over_limits = 1;
          return GPG_ERR_INV_LENGTH;
        }

      do_ghash_buf(c, c->u_ctr.ctr, iv, ivlen, 1);

      /* iv length, 64-bit */
      bitlengths[1][1] = be_bswap32(iv_bytes[0] << 3);
      bitlengths[1][0] = be_bswap32((iv_bytes[0] >> 29) |
                                    (iv_bytes[1] << 3));
      /* zeros, 64-bit */
      bitlengths[0][1] = 0;
      bitlengths[0][0] = 0;

      do_ghash_buf(c, c->u_ctr.ctr, (byte*)bitlengths, GCRY_GCM_BLOCK_LEN, 1);

      wipememory (iv_bytes, sizeof iv_bytes);
      wipememory (bitlengths, sizeof bitlengths);
    }
  else
    {
      /* 96-bit IV is handled differently. */
      memcpy (c->u_ctr.ctr, iv, ivlen);
      c->u_ctr.ctr[12] = c->u_ctr.ctr[13] = c->u_ctr.ctr[14] = 0;
      c->u_ctr.ctr[15] = 1;
    }

  c->spec->encrypt (&c->context.c, c->u_mode.gcm.tagiv, c->u_ctr.ctr);

  gcm_add32_be128 (c->u_ctr.ctr, 1);

  c->unused = 0;
  c->marks.iv = 1;
  c->marks.tag = 0;

  return 0;
}


gcry_err_code_t
_gcry_cipher_gcm_setiv (gcry_cipher_hd_t c, const byte *iv, size_t ivlen)
{
  c->marks.iv = 0;
  c->marks.tag = 0;
  c->u_mode.gcm.disallow_encryption_because_of_setiv_in_fips_mode = 0;

  if (fips_mode ())
    {
      /* Direct invocation of GCM setiv in FIPS mode disables encryption. */
      c->u_mode.gcm.disallow_encryption_because_of_setiv_in_fips_mode = 1;
    }

  return _gcry_cipher_gcm_initiv (c, iv, ivlen);
}


#if 0 && TODO
void
_gcry_cipher_gcm_geniv (gcry_cipher_hd_t c,
                        byte *ivout, size_t ivoutlen, const byte *nonce,
                        size_t noncelen)
{
  /* nonce:    user provided part (might be null) */
  /* noncelen: check if proper length (if nonce not null) */
  /* ivout:    iv used to initialize gcm, output to user */
  /* ivoutlen: check correct size */
  byte iv[IVLEN];

  if (!ivout)
    return GPG_ERR_INV_ARG;
  if (ivoutlen != IVLEN)
    return GPG_ERR_INV_LENGTH;
  if (nonce != NULL && !is_nonce_ok_len(noncelen))
    return GPG_ERR_INV_ARG;

  gcm_generate_iv(iv, nonce, noncelen);

  c->marks.iv = 0;
  c->marks.tag = 0;
  c->u_mode.gcm.disallow_encryption_because_of_setiv_in_fips_mode = 0;

  _gcry_cipher_gcm_initiv (c, iv, IVLEN);

  buf_cpy(ivout, iv, IVLEN);
  wipememory(iv, sizeof(iv));
}
#endif


static gcry_err_code_t
_gcry_cipher_gcm_tag (gcry_cipher_hd_t c,
                      byte * outbuf, size_t outbuflen, int check)
{
  if (outbuflen < GCRY_GCM_BLOCK_LEN)
    return GPG_ERR_BUFFER_TOO_SHORT;
  if (c->u_mode.gcm.datalen_over_limits)
    return GPG_ERR_INV_LENGTH;

  if (!c->marks.tag)
    {
      u32 bitlengths[2][2];

      /* aad length */
      bitlengths[0][1] = be_bswap32(c->u_mode.gcm.aadlen[0] << 3);
      bitlengths[0][0] = be_bswap32((c->u_mode.gcm.aadlen[0] >> 29) |
                                    (c->u_mode.gcm.aadlen[1] << 3));
      /* data length */
      bitlengths[1][1] = be_bswap32(c->u_mode.gcm.datalen[0] << 3);
      bitlengths[1][0] = be_bswap32((c->u_mode.gcm.datalen[0] >> 29) |
                                    (c->u_mode.gcm.datalen[1] << 3));

      /* Finalize data-stream. */
      do_ghash_buf(c, c->u_mode.gcm.u_tag.tag, NULL, 0, 1);
      c->u_mode.gcm.ghash_aad_finalized = 1;
      c->u_mode.gcm.ghash_data_finalized = 1;

      /* Add bitlengths to tag. */
      do_ghash_buf(c, c->u_mode.gcm.u_tag.tag, (byte*)bitlengths,
                   GCRY_GCM_BLOCK_LEN, 1);
      buf_xor (c->u_mode.gcm.u_tag.tag, c->u_mode.gcm.tagiv,
               c->u_mode.gcm.u_tag.tag, GCRY_GCM_BLOCK_LEN);
      c->marks.tag = 1;

      wipememory (bitlengths, sizeof (bitlengths));
      wipememory (c->u_mode.gcm.macbuf, GCRY_GCM_BLOCK_LEN);
      wipememory (c->u_mode.gcm.tagiv, GCRY_GCM_BLOCK_LEN);
      wipememory (c->u_mode.gcm.aadlen, sizeof (c->u_mode.gcm.aadlen));
      wipememory (c->u_mode.gcm.datalen, sizeof (c->u_mode.gcm.datalen));
    }

  if (!check)
    {
      memcpy (outbuf, c->u_mode.gcm.u_tag.tag, outbuflen);
      return GPG_ERR_NO_ERROR;
    }
  else
    {
      return buf_eq_const(outbuf, c->u_mode.gcm.u_tag.tag, outbuflen) ?
               GPG_ERR_NO_ERROR : GPG_ERR_CHECKSUM;
    }

  return 0;
}


gcry_err_code_t
_gcry_cipher_gcm_get_tag (gcry_cipher_hd_t c, unsigned char *outtag,
                          size_t taglen)
{
  /* Outputting authentication tag is part of encryption. */
  if (c->u_mode.gcm.disallow_encryption_because_of_setiv_in_fips_mode)
    return GPG_ERR_INV_STATE;

  return _gcry_cipher_gcm_tag (c, outtag, taglen, 0);
}

gcry_err_code_t
_gcry_cipher_gcm_check_tag (gcry_cipher_hd_t c, const unsigned char *intag,
                            size_t taglen)
{
  return _gcry_cipher_gcm_tag (c, (unsigned char *) intag, taglen, 1);
}

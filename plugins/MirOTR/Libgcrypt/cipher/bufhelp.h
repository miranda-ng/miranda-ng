/* bufhelp.h  -  Some buffer manipulation helpers
 *	Copyright © 2012 Jussi Kivilinna <jussi.kivilinna@mbnet.fi>
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
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifndef G10_BUFHELP_H
#define G10_BUFHELP_H

#include <config.h>

#ifdef HAVE_STDINT_H
# include <stdint.h> /* uintptr_t */
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#else
/* In this case, uintptr_t is provided by config.h. */
#endif

#include "bithelp.h"


#if defined(__i386__) || defined(__x86_64__) || \
    defined(__powerpc__) || defined(__powerpc64__) || \
    (defined(__arm__) && defined(__ARM_FEATURE_UNALIGNED)) || \
    defined(__aarch64__)
/* These architectures are able of unaligned memory accesses and can
   handle those fast.
 */
# define BUFHELP_FAST_UNALIGNED_ACCESS 1
#endif


/* Optimized function for small buffer copying */
static inline void
buf_cpy(void *_dst, const void *_src, size_t len)
{
#if __GNUC__ >= 4 && (defined(__x86_64__) || defined(__i386__))
  /* For AMD64 and i386, memcpy is faster.  */
  memcpy(_dst, _src, len);
#else
  byte *dst = _dst;
  const byte *src = _src;
  uintptr_t *ldst;
  const uintptr_t *lsrc;
#ifndef BUFHELP_FAST_UNALIGNED_ACCESS
  const unsigned int longmask = sizeof(uintptr_t) - 1;

  /* Skip fast processing if buffers are unaligned.  */
  if (((uintptr_t)dst | (uintptr_t)src) & longmask)
    goto do_bytes;
#endif

  ldst = (uintptr_t *)(void *)dst;
  lsrc = (const uintptr_t *)(const void *)src;

  for (; len >= sizeof(uintptr_t); len -= sizeof(uintptr_t))
    *ldst++ = *lsrc++;

  dst = (byte *)ldst;
  src = (const byte *)lsrc;

#ifndef BUFHELP_FAST_UNALIGNED_ACCESS
do_bytes:
#endif
  /* Handle tail.  */
  for (; len; len--)
    *dst++ = *src++;
#endif /*__GNUC__ >= 4 && (__x86_64__ || __i386__)*/
}


/* Optimized function for buffer xoring */
static inline void
buf_xor(void *_dst, const void *_src1, const void *_src2, size_t len)
{
  byte *dst = _dst;
  const byte *src1 = _src1;
  const byte *src2 = _src2;
  uintptr_t *ldst;
  const uintptr_t *lsrc1, *lsrc2;
#ifndef BUFHELP_FAST_UNALIGNED_ACCESS
  const unsigned int longmask = sizeof(uintptr_t) - 1;

  /* Skip fast processing if buffers are unaligned.  */
  if (((uintptr_t)dst | (uintptr_t)src1 | (uintptr_t)src2) & longmask)
    goto do_bytes;
#endif

  ldst = (uintptr_t *)(void *)dst;
  lsrc1 = (const uintptr_t *)(const void *)src1;
  lsrc2 = (const uintptr_t *)(const void *)src2;

  for (; len >= sizeof(uintptr_t); len -= sizeof(uintptr_t))
    *ldst++ = *lsrc1++ ^ *lsrc2++;

  dst = (byte *)ldst;
  src1 = (const byte *)lsrc1;
  src2 = (const byte *)lsrc2;

#ifndef BUFHELP_FAST_UNALIGNED_ACCESS
do_bytes:
#endif
  /* Handle tail.  */
  for (; len; len--)
    *dst++ = *src1++ ^ *src2++;
}


/* Optimized function for buffer xoring with two destination buffers.  Used
   mainly by CFB mode encryption.  */
static inline void
buf_xor_2dst(void *_dst1, void *_dst2, const void *_src, size_t len)
{
  byte *dst1 = _dst1;
  byte *dst2 = _dst2;
  const byte *src = _src;
  uintptr_t *ldst1, *ldst2;
  const uintptr_t *lsrc;
#ifndef BUFHELP_FAST_UNALIGNED_ACCESS
  const unsigned int longmask = sizeof(uintptr_t) - 1;

  /* Skip fast processing if buffers are unaligned.  */
  if (((uintptr_t)src | (uintptr_t)dst1 | (uintptr_t)dst2) & longmask)
    goto do_bytes;
#endif

  ldst1 = (uintptr_t *)(void *)dst1;
  ldst2 = (uintptr_t *)(void *)dst2;
  lsrc = (const uintptr_t *)(const void *)src;

  for (; len >= sizeof(uintptr_t); len -= sizeof(uintptr_t))
    *ldst1++ = (*ldst2++ ^= *lsrc++);

  dst1 = (byte *)ldst1;
  dst2 = (byte *)ldst2;
  src = (const byte *)lsrc;

#ifndef BUFHELP_FAST_UNALIGNED_ACCESS
do_bytes:
#endif
  /* Handle tail.  */
  for (; len; len--)
    *dst1++ = (*dst2++ ^= *src++);
}


/* Optimized function for combined buffer xoring and copying.  Used by mainly
   CBC mode decryption.  */
static inline void
buf_xor_n_copy_2(void *_dst_xor, const void *_src_xor, void *_srcdst_cpy,
		 const void *_src_cpy, size_t len)
{
  byte *dst_xor = _dst_xor;
  byte *srcdst_cpy = _srcdst_cpy;
  const byte *src_xor = _src_xor;
  const byte *src_cpy = _src_cpy;
  byte temp;
  uintptr_t *ldst_xor, *lsrcdst_cpy;
  const uintptr_t *lsrc_cpy, *lsrc_xor;
  uintptr_t ltemp;
#ifndef BUFHELP_FAST_UNALIGNED_ACCESS
  const unsigned int longmask = sizeof(uintptr_t) - 1;

  /* Skip fast processing if buffers are unaligned.  */
  if (((uintptr_t)src_cpy | (uintptr_t)src_xor | (uintptr_t)dst_xor |
       (uintptr_t)srcdst_cpy) & longmask)
    goto do_bytes;
#endif

  ldst_xor = (uintptr_t *)(void *)dst_xor;
  lsrc_xor = (const uintptr_t *)(void *)src_xor;
  lsrcdst_cpy = (uintptr_t *)(void *)srcdst_cpy;
  lsrc_cpy = (const uintptr_t *)(const void *)src_cpy;

  for (; len >= sizeof(uintptr_t); len -= sizeof(uintptr_t))
    {
      ltemp = *lsrc_cpy++;
      *ldst_xor++ = *lsrcdst_cpy ^ *lsrc_xor++;
      *lsrcdst_cpy++ = ltemp;
    }

  dst_xor = (byte *)ldst_xor;
  src_xor = (const byte *)lsrc_xor;
  srcdst_cpy = (byte *)lsrcdst_cpy;
  src_cpy = (const byte *)lsrc_cpy;

#ifndef BUFHELP_FAST_UNALIGNED_ACCESS
do_bytes:
#endif
  /* Handle tail.  */
  for (; len; len--)
    {
      temp = *src_cpy++;
      *dst_xor++ = *srcdst_cpy ^ *src_xor++;
      *srcdst_cpy++ = temp;
    }
}


/* Optimized function for combined buffer xoring and copying.  Used by mainly
   CFB mode decryption.  */
static inline void
buf_xor_n_copy(void *_dst_xor, void *_srcdst_cpy, const void *_src, size_t len)
{
  buf_xor_n_copy_2(_dst_xor, _src, _srcdst_cpy, _src, len);
}


/* Constant-time compare of two buffers.  Returns 1 if buffers are equal,
   and 0 if buffers differ.  */
static inline int
buf_eq_const(const void *_a, const void *_b, size_t len)
{
  const byte *a = _a;
  const byte *b = _b;
  size_t diff, i;

  /* Constant-time compare. */
  for (i = 0, diff = 0; i < len; i++)
    diff -= !!(a[i] - b[i]);

  return !diff;
}


#ifndef BUFHELP_FAST_UNALIGNED_ACCESS

/* Functions for loading and storing unaligned u32 values of different
   endianness.  */
static inline u32 buf_get_be32(const void *_buf)
{
  const byte *in = _buf;
  return ((u32)in[0] << 24) | ((u32)in[1] << 16) | \
         ((u32)in[2] << 8) | (u32)in[3];
}

static inline u32 buf_get_le32(const void *_buf)
{
  const byte *in = _buf;
  return ((u32)in[3] << 24) | ((u32)in[2] << 16) | \
         ((u32)in[1] << 8) | (u32)in[0];
}

static inline void buf_put_be32(void *_buf, u32 val)
{
  byte *out = _buf;
  out[0] = val >> 24;
  out[1] = val >> 16;
  out[2] = val >> 8;
  out[3] = val;
}

static inline void buf_put_le32(void *_buf, u32 val)
{
  byte *out = _buf;
  out[3] = val >> 24;
  out[2] = val >> 16;
  out[1] = val >> 8;
  out[0] = val;
}

#ifdef HAVE_U64_TYPEDEF
/* Functions for loading and storing unaligned u64 values of different
   endianness.  */
static inline u64 buf_get_be64(const void *_buf)
{
  const byte *in = _buf;
  return ((u64)in[0] << 56) | ((u64)in[1] << 48) | \
         ((u64)in[2] << 40) | ((u64)in[3] << 32) | \
         ((u64)in[4] << 24) | ((u64)in[5] << 16) | \
         ((u64)in[6] << 8) | (u64)in[7];
}

static inline u64 buf_get_le64(const void *_buf)
{
  const byte *in = _buf;
  return ((u64)in[7] << 56) | ((u64)in[6] << 48) | \
         ((u64)in[5] << 40) | ((u64)in[4] << 32) | \
         ((u64)in[3] << 24) | ((u64)in[2] << 16) | \
         ((u64)in[1] << 8) | (u64)in[0];
}

static inline void buf_put_be64(void *_buf, u64 val)
{
  byte *out = _buf;
  out[0] = val >> 56;
  out[1] = val >> 48;
  out[2] = val >> 40;
  out[3] = val >> 32;
  out[4] = val >> 24;
  out[5] = val >> 16;
  out[6] = val >> 8;
  out[7] = val;
}

static inline void buf_put_le64(void *_buf, u64 val)
{
  byte *out = _buf;
  out[7] = val >> 56;
  out[6] = val >> 48;
  out[5] = val >> 40;
  out[4] = val >> 32;
  out[3] = val >> 24;
  out[2] = val >> 16;
  out[1] = val >> 8;
  out[0] = val;
}
#endif /*HAVE_U64_TYPEDEF*/

#else /*BUFHELP_FAST_UNALIGNED_ACCESS*/

/* Functions for loading and storing unaligned u32 values of different
   endianness.  */
static inline u32 buf_get_be32(const void *_buf)
{
  return be_bswap32(*(const u32 *)_buf);
}

static inline u32 buf_get_le32(const void *_buf)
{
  return le_bswap32(*(const u32 *)_buf);
}

static inline void buf_put_be32(void *_buf, u32 val)
{
  u32 *out = _buf;
  *out = be_bswap32(val);
}

static inline void buf_put_le32(void *_buf, u32 val)
{
  u32 *out = _buf;
  *out = le_bswap32(val);
}

#ifdef HAVE_U64_TYPEDEF
/* Functions for loading and storing unaligned u64 values of different
   endianness.  */
static inline u64 buf_get_be64(const void *_buf)
{
  return be_bswap64(*(const u64 *)_buf);
}

static inline u64 buf_get_le64(const void *_buf)
{
  return le_bswap64(*(const u64 *)_buf);
}

static inline void buf_put_be64(void *_buf, u64 val)
{
  u64 *out = _buf;
  *out = be_bswap64(val);
}

static inline void buf_put_le64(void *_buf, u64 val)
{
  u64 *out = _buf;
  *out = le_bswap64(val);
}
#endif /*HAVE_U64_TYPEDEF*/

#endif /*BUFHELP_FAST_UNALIGNED_ACCESS*/

#endif /*G10_BITHELP_H*/

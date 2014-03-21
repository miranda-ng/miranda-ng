/*
  mpi.c
  
  by Michael J. Fromberger <http://www.dartmouth.edu/~sting/>
  Copyright (C) 1998 Michael J. Fromberger, All Rights Reserved
  
  Arbitrary precision integer arithmetic library

  modified for use in Meanwhile as a convenience library
*/

#include "mpi.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if MP_DEBUG
#include <stdio.h>

#define DIAG(T,V) {fprintf(stderr,T);mw_mp_print(V,stderr);fputc('\n',stderr);}
#else
#define DIAG(T,V)
#endif

/* 
   If MP_LOGTAB is not defined, use the math library to compute the
   logarithms on the fly.  Otherwise, use the static table below.
   Pick which works best for your system.
 */
#if MP_LOGTAB

/* {{{ s_logv_2[] - log table for 2 in various bases */

/*
  A table of the logs of 2 for various bases (the 0 and 1 entries of
  this table are meaningless and should not be referenced).  

  This table is used to compute output lengths for the mw_mp_toradix()
  function.  Since a number n in radix r takes up about log_r(n)
  digits, we estimate the output size by taking the least integer
  greater than log_r(n), where:

  log_r(n) = log_2(n) * log_r(2)

  This table, therefore, is a table of log_r(2) for 2 <= r <= 36,
  which are the output bases supported.  
 */

#include "logtab.h"

/* }}} */
#define LOG_V_2(R)  s_logv_2[(R)]

#else

#include <math.h>
#define LOG_V_2(R)  (log(2.0)/log(R))

#endif

/* Default precision for newly created mw_mp_int's      */
static unsigned int s_mw_mp_defprec = MP_DEFPREC;

/* {{{ Digit arithmetic macros */

/*
  When adding and multiplying digits, the results can be larger than
  can be contained in an mw_mp_digit.  Thus, an mw_mp_word is used.  These
  macros mask off the upper and lower digits of the mw_mp_word (the
  mw_mp_word may be more than 2 mw_mp_digits wide, but we only concern
  ourselves with the low-order 2 mw_mp_digits)

  If your mw_mp_word DOES have more than 2 mw_mp_digits, you need to
  uncomment the first line, and comment out the second.
 */

/* #define  CARRYOUT(W)  (((W)>>DIGIT_BIT)&MP_DIGIT_MAX) */
#define  CARRYOUT(W)  ((W)>>DIGIT_BIT)
#define  ACCUM(W)     ((W)&MP_DIGIT_MAX)

/* }}} */

/* {{{ Comparison constants */

#define  MP_LT       -1
#define  MP_EQ        0
#define  MP_GT        1

/* }}} */

/* {{{ Constant strings */

/* Constant strings returned by mw_mp_strerror() */
static const char *mw_mp_err_string[] = {
  "unknown result code",     /* say what?            */
  "boolean true",            /* MP_OKAY, MP_YES      */
  "boolean false",           /* MP_NO                */
  "out of memory",           /* MP_MEM               */
  "argument out of range",   /* MP_RANGE             */
  "invalid input parameter", /* MP_BADARG            */
  "result is undefined"      /* MP_UNDEF             */
};

/* Value to digit maps for radix conversion   */

/* s_dmap_1 - standard digits and letters */
static const char *s_dmap_1 = 
  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";

#if 0
/* s_dmap_2 - base64 ordering for digits  */
static const char *s_dmap_2 =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#endif

/* }}} */

/* {{{ Static function declarations */

/* 
   If MP_MACRO is false, these will be defined as actual functions;
   otherwise, suitable macro definitions will be used.  This works
   around the fact that ANSI C89 doesn't support an 'inline' keyword
   (although I hear C9x will ... about bloody time).  At present, the
   macro definitions are identical to the function bodies, but they'll
   expand in place, instead of generating a function call.

   I chose these particular functions to be made into macros because
   some profiling showed they are called a lot on a typical workload,
   and yet they are primarily housekeeping.
 */
#if MP_MACRO == 0
 void     s_mw_mp_setz(mw_mp_digit *dp, mw_mp_size count); /* zero digits           */
 void     s_mw_mp_copy(mw_mp_digit *sp, mw_mp_digit *dp, mw_mp_size count); /* copy    */
 void    *s_mw_mp_alloc(size_t nb, size_t ni);       /* general allocator     */
 void     s_mw_mp_free(void *ptr);                   /* general free function */
#else

 /* Even if these are defined as macros, we need to respect the settings
    of the MP_MEMSET and MP_MEMCPY configuration options...
  */
 #if MP_MEMSET == 0
  #define  s_mw_mp_setz(dp, count) \
       {int ix;for(ix=0;ix<(count);ix++)(dp)[ix]=0;}
 #else
  #define  s_mw_mp_setz(dp, count) memset(dp, 0, (count) * sizeof(mw_mp_digit))
 #endif /* MP_MEMSET */

 #if MP_MEMCPY == 0
  #define  s_mw_mp_copy(sp, dp, count) \
       {int ix;for(ix=0;ix<(count);ix++)(dp)[ix]=(sp)[ix];}
 #else
  #define  s_mw_mp_copy(sp, dp, count) memcpy(dp, sp, (count) * sizeof(mw_mp_digit))
 #endif /* MP_MEMCPY */

 #define  s_mw_mp_alloc(nb, ni)  calloc(nb, ni)
 #define  s_mw_mp_free(ptr) {if(ptr) free(ptr);}
#endif /* MP_MACRO */

mw_mp_err   s_mw_mp_grow(mw_mp_int *mp, mw_mp_size min);   /* increase allocated size */
mw_mp_err   s_mw_mp_pad(mw_mp_int *mp, mw_mp_size min);    /* left pad with zeroes    */

void     s_mw_mp_clamp(mw_mp_int *mp);               /* clip leading zeroes     */

void     s_mw_mp_exch(mw_mp_int *a, mw_mp_int *b);      /* swap a and b in place   */

mw_mp_err   s_mw_mp_lshd(mw_mp_int *mp, mw_mp_size p);     /* left-shift by p digits  */
void     s_mw_mp_rshd(mw_mp_int *mp, mw_mp_size p);     /* right-shift by p digits */
void     s_mw_mp_div_2d(mw_mp_int *mp, mw_mp_digit d);  /* divide by 2^d in place  */
void     s_mw_mp_mod_2d(mw_mp_int *mp, mw_mp_digit d);  /* modulo 2^d in place     */
mw_mp_err   s_mw_mp_mul_2d(mw_mp_int *mp, mw_mp_digit d);  /* multiply by 2^d in place*/
void     s_mw_mp_div_2(mw_mp_int *mp);               /* divide by 2 in place    */
mw_mp_err   s_mw_mp_mul_2(mw_mp_int *mp);               /* multiply by 2 in place  */
mw_mp_digit s_mw_mp_norm(mw_mp_int *a, mw_mp_int *b);      /* normalize for division  */
mw_mp_err   s_mw_mp_add_d(mw_mp_int *mp, mw_mp_digit d);   /* unsigned digit addition */
mw_mp_err   s_mw_mp_sub_d(mw_mp_int *mp, mw_mp_digit d);   /* unsigned digit subtract */
mw_mp_err   s_mw_mp_mul_d(mw_mp_int *mp, mw_mp_digit d);   /* unsigned digit multiply */
mw_mp_err   s_mw_mp_div_d(mw_mp_int *mp, mw_mp_digit d, mw_mp_digit *r);
		                               /* unsigned digit divide   */
mw_mp_err   s_mw_mp_reduce(mw_mp_int *x, mw_mp_int *m, mw_mp_int *mu);
                                               /* Barrett reduction       */
mw_mp_err   s_mw_mp_add(mw_mp_int *a, mw_mp_int *b);       /* magnitude addition      */
mw_mp_err   s_mw_mp_sub(mw_mp_int *a, mw_mp_int *b);       /* magnitude subtract      */
mw_mp_err   s_mw_mp_mul(mw_mp_int *a, mw_mp_int *b);       /* magnitude multiply      */
#if 0
void     s_mw_mp_kmul(mw_mp_digit *a, mw_mp_digit *b, mw_mp_digit *out, mw_mp_size len);
                                               /* multiply buffers in place */
#endif
#if MP_SQUARE
mw_mp_err   s_mw_mp_sqr(mw_mp_int *a);                  /* magnitude square        */
#else
#define  s_mw_mp_sqr(a) s_mw_mp_mul(a, a)
#endif
mw_mp_err   s_mw_mp_div(mw_mp_int *a, mw_mp_int *b);       /* magnitude divide        */
mw_mp_err   s_mw_mp_2expt(mw_mp_int *a, mw_mp_digit k);    /* a = 2^k                 */
int      s_mw_mp_cmp(mw_mp_int *a, mw_mp_int *b);       /* magnitude comparison    */
int      s_mw_mp_cmw_mp_d(mw_mp_int *a, mw_mp_digit d);    /* magnitude digit compare */
int      s_mw_mp_ispow2(mw_mp_int *v);               /* is v a power of 2?      */
int      s_mw_mp_ispow2d(mw_mp_digit d);             /* is d a power of 2?      */

int      s_mw_mp_tovalue(char ch, int r);          /* convert ch to value    */
char     s_mw_mp_todigit(int val, int r, int low); /* convert val to digit   */
int      s_mw_mp_outlen(int bits, int r);          /* output length in bytes */

/* }}} */

/* {{{ Default precision manipulation */

unsigned int mw_mp_get_prec(void)
{
  return s_mw_mp_defprec;

} /* end mw_mp_get_prec() */

void         mw_mp_set_prec(unsigned int prec)
{
  if(prec == 0)
    s_mw_mp_defprec = MP_DEFPREC;
  else
    s_mw_mp_defprec = prec;

} /* end mw_mp_set_prec() */

/* }}} */

/*------------------------------------------------------------------------*/
/* {{{ mw_mp_init(mp) */

/*
  mw_mp_init(mp)

  Initialize a new zero-valued mw_mp_int.  Returns MP_OKAY if successful,
  MP_MEM if memory could not be allocated for the structure.
 */

mw_mp_err mw_mp_init(mw_mp_int *mp)
{
  return mw_mp_init_size(mp, s_mw_mp_defprec);

} /* end mw_mp_init() */

/* }}} */

/* {{{ mw_mp_init_array(mp[], count) */

mw_mp_err mw_mp_init_array(mw_mp_int mp[], int count)
{
  mw_mp_err  res;
  int     pos;

  ARGCHK(mp !=NULL && count > 0, MP_BADARG);

  for(pos = 0; pos < count; ++pos) {
    if((res = mw_mp_init(&mp[pos])) != MP_OKAY)
      goto CLEANUP;
  }

  return MP_OKAY;

 CLEANUP:
  while(--pos >= 0) 
    mw_mp_clear(&mp[pos]);

  return res;

} /* end mw_mp_init_array() */

/* }}} */

/* {{{ mw_mp_init_size(mp, prec) */

/*
  mw_mp_init_size(mp, prec)

  Initialize a new zero-valued mw_mp_int with at least the given
  precision; returns MP_OKAY if successful, or MP_MEM if memory could
  not be allocated for the structure.
 */

mw_mp_err mw_mp_init_size(mw_mp_int *mp, mw_mp_size prec)
{
  ARGCHK(mp != NULL && prec > 0, MP_BADARG);

  if((DIGITS(mp) = s_mw_mp_alloc(prec, sizeof(mw_mp_digit))) == NULL)
    return MP_MEM;

  SIGN(mp) = MP_ZPOS;
  USED(mp) = 1;
  ALLOC(mp) = prec;

  return MP_OKAY;

} /* end mw_mp_init_size() */

/* }}} */

/* {{{ mw_mp_init_copy(mp, from) */

/*
  mw_mp_init_copy(mp, from)

  Initialize mp as an exact copy of from.  Returns MP_OKAY if
  successful, MP_MEM if memory could not be allocated for the new
  structure.
 */

mw_mp_err mw_mp_init_copy(mw_mp_int *mp, mw_mp_int *from)
{
  ARGCHK(mp != NULL && from != NULL, MP_BADARG);

  if(mp == from)
    return MP_OKAY;

  if((DIGITS(mp) = s_mw_mp_alloc(USED(from), sizeof(mw_mp_digit))) == NULL)
    return MP_MEM;

  s_mw_mp_copy(DIGITS(from), DIGITS(mp), USED(from));
  USED(mp) = USED(from);
  ALLOC(mp) = USED(from);
  SIGN(mp) = SIGN(from);

  return MP_OKAY;

} /* end mw_mp_init_copy() */

/* }}} */

/* {{{ mw_mp_copy(from, to) */

/*
  mw_mp_copy(from, to)

  Copies the mw_mp_int 'from' to the mw_mp_int 'to'.  It is presumed that
  'to' has already been initialized (if not, use mw_mp_init_copy()
  instead). If 'from' and 'to' are identical, nothing happens.
 */

mw_mp_err mw_mp_copy(mw_mp_int *from, mw_mp_int *to)
{
  ARGCHK(from != NULL && to != NULL, MP_BADARG);

  if(from == to)
    return MP_OKAY;

  { /* copy */
    mw_mp_digit   *tmp;

    /*
      If the allocated buffer in 'to' already has enough space to hold
      all the used digits of 'from', we'll re-use it to avoid hitting
      the memory allocater more than necessary; otherwise, we'd have
      to grow anyway, so we just allocate a hunk and make the copy as
      usual
     */
    if(ALLOC(to) >= USED(from)) {
      s_mw_mp_setz(DIGITS(to) + USED(from), ALLOC(to) - USED(from));
      s_mw_mp_copy(DIGITS(from), DIGITS(to), USED(from));
      
    } else {
      if((tmp = s_mw_mp_alloc(USED(from), sizeof(mw_mp_digit))) == NULL)
	return MP_MEM;

      s_mw_mp_copy(DIGITS(from), tmp, USED(from));

      if(DIGITS(to) != NULL) {
#if MP_CRYPTO
	s_mw_mp_setz(DIGITS(to), ALLOC(to));
#endif
	s_mw_mp_free(DIGITS(to));
      }

      DIGITS(to) = tmp;
      ALLOC(to) = USED(from);
    }

    /* Copy the precision and sign from the original */
    USED(to) = USED(from);
    SIGN(to) = SIGN(from);
  } /* end copy */

  return MP_OKAY;

} /* end mw_mp_copy() */

/* }}} */

/* {{{ mw_mp_exch(mp1, mp2) */

/*
  mw_mp_exch(mp1, mp2)

  Exchange mp1 and mp2 without allocating any intermediate memory
  (well, unless you count the stack space needed for this call and the
  locals it creates...).  This cannot fail.
 */

void mw_mp_exch(mw_mp_int *mp1, mw_mp_int *mp2)
{
#if MP_ARGCHK == 2
  assert(mp1 != NULL && mp2 != NULL);
#else
  if(mp1 == NULL || mp2 == NULL)
    return;
#endif

  s_mw_mp_exch(mp1, mp2);

} /* end mw_mp_exch() */

/* }}} */

/* {{{ mw_mp_clear(mp) */

/*
  mw_mp_clear(mp)

  Release the storage used by an mw_mp_int, and void its fields so that
  if someone calls mw_mp_clear() again for the same int later, we won't
  get tollchocked.
 */

void   mw_mp_clear(mw_mp_int *mp)
{
  if(mp == NULL)
    return;

  if(DIGITS(mp) != NULL) {
#if MP_CRYPTO
    s_mw_mp_setz(DIGITS(mp), ALLOC(mp));
#endif
    s_mw_mp_free(DIGITS(mp));
    DIGITS(mp) = NULL;
  }

  USED(mp) = 0;
  ALLOC(mp) = 0;

} /* end mw_mp_clear() */

/* }}} */

/* {{{ mw_mp_clear_array(mp[], count) */

void   mw_mp_clear_array(mw_mp_int mp[], int count)
{
  ARGCHK(mp != NULL && count > 0, MP_BADARG);

  while(--count >= 0) 
    mw_mp_clear(&mp[count]);

} /* end mw_mp_clear_array() */

/* }}} */

/* {{{ mw_mp_zero(mp) */

/*
  mw_mp_zero(mp) 

  Set mp to zero.  Does not change the allocated size of the structure,
  and therefore cannot fail (except on a bad argument, which we ignore)
 */
void   mw_mp_zero(mw_mp_int *mp)
{
  if(mp == NULL)
    return;

  s_mw_mp_setz(DIGITS(mp), ALLOC(mp));
  USED(mp) = 1;
  SIGN(mp) = MP_ZPOS;

} /* end mw_mp_zero() */

/* }}} */

/* {{{ mw_mp_set(mp, d) */

void   mw_mp_set(mw_mp_int *mp, mw_mp_digit d)
{
  if(mp == NULL)
    return;

  mw_mp_zero(mp);
  DIGIT(mp, 0) = d;

} /* end mw_mp_set() */

/* }}} */

/* {{{ mw_mp_set_int(mp, z) */

mw_mp_err mw_mp_set_int(mw_mp_int *mp, long z)
{
  int            ix;
  unsigned long  v = abs(z);
  mw_mp_err         res;

  ARGCHK(mp != NULL, MP_BADARG);

  mw_mp_zero(mp);
  if(z == 0)
    return MP_OKAY;  /* shortcut for zero */

  for(ix = sizeof(long) - 1; ix >= 0; ix--) {

    if((res = s_mw_mp_mul_2d(mp, CHAR_BIT)) != MP_OKAY)
      return res;

    res = s_mw_mp_add_d(mp, 
		     (mw_mp_digit)((v >> (ix * CHAR_BIT)) & UCHAR_MAX));
    if(res != MP_OKAY)
      return res;

  }

  if(z < 0)
    SIGN(mp) = MP_NEG;

  return MP_OKAY;

} /* end mw_mp_set_int() */

/* }}} */

/*------------------------------------------------------------------------*/
/* {{{ Digit arithmetic */

/* {{{ mw_mp_add_d(a, d, b) */

/*
  mw_mp_add_d(a, d, b)

  Compute the sum b = a + d, for a single digit d.  Respects the sign of
  its primary addend (single digits are unsigned anyway).
 */

mw_mp_err mw_mp_add_d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *b)
{
  mw_mp_err   res = MP_OKAY;

  ARGCHK(a != NULL && b != NULL, MP_BADARG);

  if((res = mw_mp_copy(a, b)) != MP_OKAY)
    return res;

  if(SIGN(b) == MP_ZPOS) {
    res = s_mw_mp_add_d(b, d);
  } else if(s_mw_mp_cmw_mp_d(b, d) >= 0) {
    res = s_mw_mp_sub_d(b, d);
  } else {
    SIGN(b) = MP_ZPOS;

    DIGIT(b, 0) = d - DIGIT(b, 0);
  }

  return res;

} /* end mw_mp_add_d() */

/* }}} */

/* {{{ mw_mp_sub_d(a, d, b) */

/*
  mw_mp_sub_d(a, d, b)

  Compute the difference b = a - d, for a single digit d.  Respects the
  sign of its subtrahend (single digits are unsigned anyway).
 */

mw_mp_err mw_mp_sub_d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *b)
{
  mw_mp_err   res;

  ARGCHK(a != NULL && b != NULL, MP_BADARG);

  if((res = mw_mp_copy(a, b)) != MP_OKAY)
    return res;

  if(SIGN(b) == MP_NEG) {
    if((res = s_mw_mp_add_d(b, d)) != MP_OKAY)
      return res;

  } else if(s_mw_mp_cmw_mp_d(b, d) >= 0) {
    if((res = s_mw_mp_sub_d(b, d)) != MP_OKAY)
      return res;

  } else {
    mw_mp_neg(b, b);

    DIGIT(b, 0) = d - DIGIT(b, 0);
    SIGN(b) = MP_NEG;
  }

  if(s_mw_mp_cmw_mp_d(b, 0) == 0)
    SIGN(b) = MP_ZPOS;

  return MP_OKAY;

} /* end mw_mp_sub_d() */

/* }}} */

/* {{{ mw_mp_mul_d(a, d, b) */

/*
  mw_mp_mul_d(a, d, b)

  Compute the product b = a * d, for a single digit d.  Respects the sign
  of its multiplicand (single digits are unsigned anyway)
 */

mw_mp_err mw_mp_mul_d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *b)
{
  mw_mp_err  res;

  ARGCHK(a != NULL && b != NULL, MP_BADARG);

  if(d == 0) {
    mw_mp_zero(b);
    return MP_OKAY;
  }

  if((res = mw_mp_copy(a, b)) != MP_OKAY)
    return res;

  res = s_mw_mp_mul_d(b, d);

  return res;

} /* end mw_mp_mul_d() */

/* }}} */

/* {{{ mw_mp_mul_2(a, c) */

mw_mp_err mw_mp_mul_2(mw_mp_int *a, mw_mp_int *c)
{
  mw_mp_err  res;

  ARGCHK(a != NULL && c != NULL, MP_BADARG);

  if((res = mw_mp_copy(a, c)) != MP_OKAY)
    return res;

  return s_mw_mp_mul_2(c);

} /* end mw_mp_mul_2() */

/* }}} */

/* {{{ mw_mp_div_d(a, d, q, r) */

/*
  mw_mp_div_d(a, d, q, r)

  Compute the quotient q = a / d and remainder r = a mod d, for a
  single digit d.  Respects the sign of its divisor (single digits are
  unsigned anyway).
 */

mw_mp_err mw_mp_div_d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *q, mw_mp_digit *r)
{
  mw_mp_err   res;
  mw_mp_digit rem;
  int      pow;

  ARGCHK(a != NULL, MP_BADARG);

  if(d == 0)
    return MP_RANGE;

  /* Shortcut for powers of two ... */
  if((pow = s_mw_mp_ispow2d(d)) >= 0) {
    mw_mp_digit  mask;

    mask = (1 << pow) - 1;
    rem = DIGIT(a, 0) & mask;

    if(q) {
      mw_mp_copy(a, q);
      s_mw_mp_div_2d(q, pow);
    }

    if(r)
      *r = rem;

    return MP_OKAY;
  }

  /*
    If the quotient is actually going to be returned, we'll try to
    avoid hitting the memory allocator by copying the dividend into it
    and doing the division there.  This can't be any _worse_ than
    always copying, and will sometimes be better (since it won't make
    another copy)

    If it's not going to be returned, we need to allocate a temporary
    to hold the quotient, which will just be discarded.
   */
  if(q) {
    if((res = mw_mp_copy(a, q)) != MP_OKAY)
      return res;

    res = s_mw_mp_div_d(q, d, &rem);
    if(s_mw_mp_cmw_mp_d(q, 0) == MP_EQ)
      SIGN(q) = MP_ZPOS;

  } else {
    mw_mp_int  qp;

    if((res = mw_mp_init_copy(&qp, a)) != MP_OKAY)
      return res;

    res = s_mw_mp_div_d(&qp, d, &rem);
    if(s_mw_mp_cmw_mp_d(&qp, 0) == 0)
      SIGN(&qp) = MP_ZPOS;

    mw_mp_clear(&qp);
  }

  if(r)
    *r = rem;

  return res;

} /* end mw_mp_div_d() */

/* }}} */

/* {{{ mw_mp_div_2(a, c) */

/*
  mw_mp_div_2(a, c)

  Compute c = a / 2, disregarding the remainder.
 */

mw_mp_err mw_mp_div_2(mw_mp_int *a, mw_mp_int *c)
{
  mw_mp_err  res;

  ARGCHK(a != NULL && c != NULL, MP_BADARG);

  if((res = mw_mp_copy(a, c)) != MP_OKAY)
    return res;

  s_mw_mp_div_2(c);

  return MP_OKAY;

} /* end mw_mp_div_2() */

/* }}} */

/* {{{ mw_mp_expt_d(a, d, b) */

mw_mp_err mw_mp_expt_d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *c)
{
  mw_mp_int   s, x;
  mw_mp_err   res;
  mw_mp_sign  cs = MP_ZPOS;

  ARGCHK(a != NULL && c != NULL, MP_BADARG);

  if((res = mw_mp_init(&s)) != MP_OKAY)
    return res;
  if((res = mw_mp_init_copy(&x, a)) != MP_OKAY)
    goto X;

  DIGIT(&s, 0) = 1;

  if((d % 2) == 1)
    cs = SIGN(a);

  while(d != 0) {
    if(d & 1) {
      if((res = s_mw_mp_mul(&s, &x)) != MP_OKAY)
	goto CLEANUP;
    }

    d >>= 1;

    if((res = s_mw_mp_sqr(&x)) != MP_OKAY)
      goto CLEANUP;
  }

  SIGN(&s) = cs;

  s_mw_mp_exch(&s, c);

CLEANUP:
  mw_mp_clear(&x);
X:
  mw_mp_clear(&s);

  return res;

} /* end mw_mp_expt_d() */

/* }}} */

/* }}} */

/*------------------------------------------------------------------------*/
/* {{{ Full arithmetic */

/* {{{ mw_mp_abs(a, b) */

/*
  mw_mp_abs(a, b)

  Compute b = |a|.  'a' and 'b' may be identical.
 */

mw_mp_err mw_mp_abs(mw_mp_int *a, mw_mp_int *b)
{
  mw_mp_err   res;

  ARGCHK(a != NULL && b != NULL, MP_BADARG);

  if((res = mw_mp_copy(a, b)) != MP_OKAY)
    return res;

  SIGN(b) = MP_ZPOS;

  return MP_OKAY;

} /* end mw_mp_abs() */

/* }}} */

/* {{{ mw_mp_neg(a, b) */

/*
  mw_mp_neg(a, b)

  Compute b = -a.  'a' and 'b' may be identical.
 */

mw_mp_err mw_mp_neg(mw_mp_int *a, mw_mp_int *b)
{
  mw_mp_err   res;

  ARGCHK(a != NULL && b != NULL, MP_BADARG);

  if((res = mw_mp_copy(a, b)) != MP_OKAY)
    return res;

  if(s_mw_mp_cmw_mp_d(b, 0) == MP_EQ) 
    SIGN(b) = MP_ZPOS;
  else 
    SIGN(b) = (SIGN(b) == MP_NEG) ? MP_ZPOS : MP_NEG;

  return MP_OKAY;

} /* end mw_mp_neg() */

/* }}} */

/* {{{ mw_mp_add(a, b, c) */

/*
  mw_mp_add(a, b, c)

  Compute c = a + b.  All parameters may be identical.
 */

mw_mp_err mw_mp_add(mw_mp_int *a, mw_mp_int *b, mw_mp_int *c)
{
  mw_mp_err  res;
  int     cmp;

  ARGCHK(a != NULL && b != NULL && c != NULL, MP_BADARG);

  if(SIGN(a) == SIGN(b)) { /* same sign:  add values, keep sign */

    /* Commutativity of addition lets us do this in either order,
       so we avoid having to use a temporary even if the result 
       is supposed to replace the output
     */
    if(c == b) {
      if((res = s_mw_mp_add(c, a)) != MP_OKAY)
	return res;
    } else {
      if(c != a && (res = mw_mp_copy(a, c)) != MP_OKAY)
	return res;

      if((res = s_mw_mp_add(c, b)) != MP_OKAY) 
	return res;
    }

  } else if((cmp = s_mw_mp_cmp(a, b)) > 0) {  /* different sign: a > b   */

    /* If the output is going to be clobbered, we will use a temporary
       variable; otherwise, we'll do it without touching the memory 
       allocator at all, if possible
     */
    if(c == b) {
      mw_mp_int  tmp;

      if((res = mw_mp_init_copy(&tmp, a)) != MP_OKAY)
	return res;
      if((res = s_mw_mp_sub(&tmp, b)) != MP_OKAY) {
	mw_mp_clear(&tmp);
	return res;
      }

      s_mw_mp_exch(&tmp, c);
      mw_mp_clear(&tmp);

    } else {

      if(c != a && (res = mw_mp_copy(a, c)) != MP_OKAY)
	return res;
      if((res = s_mw_mp_sub(c, b)) != MP_OKAY)
	return res;

    }

  } else if(cmp == 0) {             /* different sign, a == b   */

    mw_mp_zero(c);
    return MP_OKAY;

  } else {                          /* different sign: a < b    */

    /* See above... */
    if(c == a) {
      mw_mp_int  tmp;

      if((res = mw_mp_init_copy(&tmp, b)) != MP_OKAY)
	return res;
      if((res = s_mw_mp_sub(&tmp, a)) != MP_OKAY) {
	mw_mp_clear(&tmp);
	return res;
      }

      s_mw_mp_exch(&tmp, c);
      mw_mp_clear(&tmp);

    } else {

      if(c != b && (res = mw_mp_copy(b, c)) != MP_OKAY)
	return res;
      if((res = s_mw_mp_sub(c, a)) != MP_OKAY)
	return res;

    }
  }

  if(USED(c) == 1 && DIGIT(c, 0) == 0)
    SIGN(c) = MP_ZPOS;

  return MP_OKAY;

} /* end mw_mp_add() */

/* }}} */

/* {{{ mw_mp_sub(a, b, c) */

/*
  mw_mp_sub(a, b, c)

  Compute c = a - b.  All parameters may be identical.
 */

mw_mp_err mw_mp_sub(mw_mp_int *a, mw_mp_int *b, mw_mp_int *c)
{
  mw_mp_err  res;
  int     cmp;

  ARGCHK(a != NULL && b != NULL && c != NULL, MP_BADARG);

  if(SIGN(a) != SIGN(b)) {
    if(c == a) {
      if((res = s_mw_mp_add(c, b)) != MP_OKAY)
	return res;
    } else {
      if(c != b && ((res = mw_mp_copy(b, c)) != MP_OKAY))
	return res;
      if((res = s_mw_mp_add(c, a)) != MP_OKAY)
	return res;
      SIGN(c) = SIGN(a);
    }

  } else if((cmp = s_mw_mp_cmp(a, b)) > 0) { /* Same sign, a > b */
    if(c == b) {
      mw_mp_int  tmp;

      if((res = mw_mp_init_copy(&tmp, a)) != MP_OKAY)
	return res;
      if((res = s_mw_mp_sub(&tmp, b)) != MP_OKAY) {
	mw_mp_clear(&tmp);
	return res;
      }
      s_mw_mp_exch(&tmp, c);
      mw_mp_clear(&tmp);

    } else {
      if(c != a && ((res = mw_mp_copy(a, c)) != MP_OKAY))
	return res;

      if((res = s_mw_mp_sub(c, b)) != MP_OKAY)
	return res;
    }

  } else if(cmp == 0) {  /* Same sign, equal magnitude */
    mw_mp_zero(c);
    return MP_OKAY;

  } else {               /* Same sign, b > a */
    if(c == a) {
      mw_mp_int  tmp;

      if((res = mw_mp_init_copy(&tmp, b)) != MP_OKAY)
	return res;

      if((res = s_mw_mp_sub(&tmp, a)) != MP_OKAY) {
	mw_mp_clear(&tmp);
	return res;
      }
      s_mw_mp_exch(&tmp, c);
      mw_mp_clear(&tmp);

    } else {
      if(c != b && ((res = mw_mp_copy(b, c)) != MP_OKAY)) 
	return res;

      if((res = s_mw_mp_sub(c, a)) != MP_OKAY)
	return res;
    }

    SIGN(c) = !SIGN(b);
  }

  if(USED(c) == 1 && DIGIT(c, 0) == 0)
    SIGN(c) = MP_ZPOS;

  return MP_OKAY;

} /* end mw_mp_sub() */

/* }}} */

/* {{{ mw_mp_mul(a, b, c) */

/*
  mw_mp_mul(a, b, c)

  Compute c = a * b.  All parameters may be identical.
 */

mw_mp_err mw_mp_mul(mw_mp_int *a, mw_mp_int *b, mw_mp_int *c)
{
  mw_mp_err   res;
  mw_mp_sign  sgn;

  ARGCHK(a != NULL && b != NULL && c != NULL, MP_BADARG);

  sgn = (SIGN(a) == SIGN(b)) ? MP_ZPOS : MP_NEG;

  if(c == b) {
    if((res = s_mw_mp_mul(c, a)) != MP_OKAY)
      return res;

  } else {
    if((res = mw_mp_copy(a, c)) != MP_OKAY)
      return res;

    if((res = s_mw_mp_mul(c, b)) != MP_OKAY)
      return res;
  }
  
  if(sgn == MP_ZPOS || s_mw_mp_cmw_mp_d(c, 0) == MP_EQ)
    SIGN(c) = MP_ZPOS;
  else
    SIGN(c) = sgn;
  
  return MP_OKAY;

} /* end mw_mp_mul() */

/* }}} */

/* {{{ mw_mp_mul_2d(a, d, c) */

/*
  mw_mp_mul_2d(a, d, c)

  Compute c = a * 2^d.  a may be the same as c.
 */

mw_mp_err mw_mp_mul_2d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *c)
{
  mw_mp_err   res;

  ARGCHK(a != NULL && c != NULL, MP_BADARG);

  if((res = mw_mp_copy(a, c)) != MP_OKAY)
    return res;

  if(d == 0)
    return MP_OKAY;

  return s_mw_mp_mul_2d(c, d);

} /* end mw_mp_mul() */

/* }}} */

/* {{{ mw_mp_sqr(a, b) */

#if MP_SQUARE
mw_mp_err mw_mp_sqr(mw_mp_int *a, mw_mp_int *b)
{
  mw_mp_err   res;

  ARGCHK(a != NULL && b != NULL, MP_BADARG);

  if((res = mw_mp_copy(a, b)) != MP_OKAY)
    return res;

  if((res = s_mw_mp_sqr(b)) != MP_OKAY)
    return res;

  SIGN(b) = MP_ZPOS;

  return MP_OKAY;

} /* end mw_mp_sqr() */
#endif

/* }}} */

/* {{{ mw_mp_div(a, b, q, r) */

/*
  mw_mp_div(a, b, q, r)

  Compute q = a / b and r = a mod b.  Input parameters may be re-used
  as output parameters.  If q or r is NULL, that portion of the
  computation will be discarded (although it will still be computed)

  Pay no attention to the hacker behind the curtain.
 */

mw_mp_err mw_mp_div(mw_mp_int *a, mw_mp_int *b, mw_mp_int *q, mw_mp_int *r)
{
  mw_mp_err   res;
  mw_mp_int   qtmp, rtmp;
  int      cmp;

  ARGCHK(a != NULL && b != NULL, MP_BADARG);

  if(mw_mp_cmw_mp_z(b) == MP_EQ)
    return MP_RANGE;

  /* If a <= b, we can compute the solution without division, and
     avoid any memory allocation
   */
  if((cmp = s_mw_mp_cmp(a, b)) < 0) {
    if(r) {
      if((res = mw_mp_copy(a, r)) != MP_OKAY)
	return res;
    }

    if(q) 
      mw_mp_zero(q);

    return MP_OKAY;

  } else if(cmp == 0) {

    /* Set quotient to 1, with appropriate sign */
    if(q) {
      int qneg = (SIGN(a) != SIGN(b));

      mw_mp_set(q, 1);
      if(qneg)
	SIGN(q) = MP_NEG;
    }

    if(r)
      mw_mp_zero(r);

    return MP_OKAY;
  }

  /* If we get here, it means we actually have to do some division */

  /* Set up some temporaries... */
  if((res = mw_mp_init_copy(&qtmp, a)) != MP_OKAY)
    return res;
  if((res = mw_mp_init_copy(&rtmp, b)) != MP_OKAY)
    goto CLEANUP;

  if((res = s_mw_mp_div(&qtmp, &rtmp)) != MP_OKAY)
    goto CLEANUP;

  /* Compute the signs for the output  */
  SIGN(&rtmp) = SIGN(a); /* Sr = Sa              */
  if(SIGN(a) == SIGN(b))
    SIGN(&qtmp) = MP_ZPOS;  /* Sq = MP_ZPOS if Sa = Sb */
  else
    SIGN(&qtmp) = MP_NEG;   /* Sq = MP_NEG if Sa != Sb */

  if(s_mw_mp_cmw_mp_d(&qtmp, 0) == MP_EQ)
    SIGN(&qtmp) = MP_ZPOS;
  if(s_mw_mp_cmw_mp_d(&rtmp, 0) == MP_EQ)
    SIGN(&rtmp) = MP_ZPOS;

  /* Copy output, if it is needed      */
  if(q) 
    s_mw_mp_exch(&qtmp, q);

  if(r) 
    s_mw_mp_exch(&rtmp, r);

CLEANUP:
  mw_mp_clear(&rtmp);
  mw_mp_clear(&qtmp);

  return res;

} /* end mw_mp_div() */

/* }}} */

/* {{{ mw_mp_div_2d(a, d, q, r) */

mw_mp_err mw_mp_div_2d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *q, mw_mp_int *r)
{
  mw_mp_err  res;

  ARGCHK(a != NULL, MP_BADARG);

  if(q) {
    if((res = mw_mp_copy(a, q)) != MP_OKAY)
      return res;

    s_mw_mp_div_2d(q, d);
  }

  if(r) {
    if((res = mw_mp_copy(a, r)) != MP_OKAY)
      return res;

    s_mw_mp_mod_2d(r, d);
  }

  return MP_OKAY;

} /* end mw_mp_div_2d() */

/* }}} */

/* {{{ mw_mp_expt(a, b, c) */

/*
  mw_mp_expt(a, b, c)

  Compute c = a ** b, that is, raise a to the b power.  Uses a
  standard iterative square-and-multiply technique.
 */

mw_mp_err mw_mp_expt(mw_mp_int *a, mw_mp_int *b, mw_mp_int *c)
{
  mw_mp_int   s, x;
  mw_mp_err   res;
  mw_mp_digit d;

  /// Miranda NG adaptation start - MSVC
  ///int      dig, bit;
  int bit;
  mw_mp_size dig;
  /// Miranda NG adaptation end

  ARGCHK(a != NULL && b != NULL && c != NULL, MP_BADARG);

  if(mw_mp_cmw_mp_z(b) < 0)
    return MP_RANGE;

  if((res = mw_mp_init(&s)) != MP_OKAY)
    return res;

  mw_mp_set(&s, 1);

  if((res = mw_mp_init_copy(&x, a)) != MP_OKAY)
    goto X;

  /* Loop over low-order digits in ascending order */
  for(dig = 0; dig < (USED(b) - 1); dig++) {
    d = DIGIT(b, dig);

    /* Loop over bits of each non-maximal digit */
    for(bit = 0; bit < DIGIT_BIT; bit++) {
      if(d & 1) {
	if((res = s_mw_mp_mul(&s, &x)) != MP_OKAY) 
	  goto CLEANUP;
      }

      d >>= 1;
      
      if((res = s_mw_mp_sqr(&x)) != MP_OKAY)
	goto CLEANUP;
    }
  }

  /* Consider now the last digit... */
  d = DIGIT(b, dig);

  while(d) {
    if(d & 1) {
      if((res = s_mw_mp_mul(&s, &x)) != MP_OKAY)
	goto CLEANUP;
    }

    d >>= 1;

    if((res = s_mw_mp_sqr(&x)) != MP_OKAY)
      goto CLEANUP;
  }
  
  if(mw_mp_iseven(b))
    SIGN(&s) = SIGN(a);

  res = mw_mp_copy(&s, c);

CLEANUP:
  mw_mp_clear(&x);
X:
  mw_mp_clear(&s);

  return res;

} /* end mw_mp_expt() */

/* }}} */

/* {{{ mw_mp_2expt(a, k) */

/* Compute a = 2^k */

mw_mp_err mw_mp_2expt(mw_mp_int *a, mw_mp_digit k)
{
  ARGCHK(a != NULL, MP_BADARG);

  return s_mw_mp_2expt(a, k);

} /* end mw_mp_2expt() */

/* }}} */

/* {{{ mw_mp_mod(a, m, c) */

/*
  mw_mp_mod(a, m, c)

  Compute c = a (mod m).  Result will always be 0 <= c < m.
 */

mw_mp_err mw_mp_mod(mw_mp_int *a, mw_mp_int *m, mw_mp_int *c)
{
  mw_mp_err  res;
  int     mag;

  ARGCHK(a != NULL && m != NULL && c != NULL, MP_BADARG);

  if(SIGN(m) == MP_NEG)
    return MP_RANGE;

  /*
     If |a| > m, we need to divide to get the remainder and take the
     absolute value.  

     If |a| < m, we don't need to do any division, just copy and adjust
     the sign (if a is negative).

     If |a| == m, we can simply set the result to zero.

     This order is intended to minimize the average path length of the
     comparison chain on common workloads -- the most frequent cases are
     that |a| != m, so we do those first.
   */
  if((mag = s_mw_mp_cmp(a, m)) > 0) {
    if((res = mw_mp_div(a, m, NULL, c)) != MP_OKAY)
      return res;
    
    if(SIGN(c) == MP_NEG) {
      if((res = mw_mp_add(c, m, c)) != MP_OKAY)
	return res;
    }

  } else if(mag < 0) {
    if((res = mw_mp_copy(a, c)) != MP_OKAY)
      return res;

    if(mw_mp_cmw_mp_z(a) < 0) {
      if((res = mw_mp_add(c, m, c)) != MP_OKAY)
	return res;

    }
    
  } else {
    mw_mp_zero(c);

  }

  return MP_OKAY;

} /* end mw_mp_mod() */

/* }}} */

/* {{{ mw_mp_mod_d(a, d, c) */

/*
  mw_mp_mod_d(a, d, c)

  Compute c = a (mod d).  Result will always be 0 <= c < d
 */
mw_mp_err mw_mp_mod_d(mw_mp_int *a, mw_mp_digit d, mw_mp_digit *c)
{
  mw_mp_err   res;
  mw_mp_digit rem;

  ARGCHK(a != NULL && c != NULL, MP_BADARG);

  if(s_mw_mp_cmw_mp_d(a, d) > 0) {
    if((res = mw_mp_div_d(a, d, NULL, &rem)) != MP_OKAY)
      return res;

  } else {
    if(SIGN(a) == MP_NEG)
      rem = d - DIGIT(a, 0);
    else
      rem = DIGIT(a, 0);
  }

  if(c)
    *c = rem;

  return MP_OKAY;

} /* end mw_mp_mod_d() */

/* }}} */

/* {{{ mw_mp_sqrt(a, b) */

/*
  mw_mp_sqrt(a, b)

  Compute the integer square root of a, and store the result in b.
  Uses an integer-arithmetic version of Newton's iterative linear
  approximation technique to determine this value; the result has the
  following two properties:

     b^2 <= a
     (b+1)^2 >= a

  It is a range error to pass a negative value.
 */
mw_mp_err mw_mp_sqrt(mw_mp_int *a, mw_mp_int *b)
{
  mw_mp_int   x, t;
  mw_mp_err   res;

  ARGCHK(a != NULL && b != NULL, MP_BADARG);

  /* Cannot take square root of a negative value */
  if(SIGN(a) == MP_NEG)
    return MP_RANGE;

  /* Special cases for zero and one, trivial     */
  if(mw_mp_cmw_mp_d(a, 0) == MP_EQ || mw_mp_cmw_mp_d(a, 1) == MP_EQ) 
    return mw_mp_copy(a, b);
    
  /* Initialize the temporaries we'll use below  */
  if((res = mw_mp_init_size(&t, USED(a))) != MP_OKAY)
    return res;

  /* Compute an initial guess for the iteration as a itself */
  if((res = mw_mp_init_copy(&x, a)) != MP_OKAY)
    goto X;

  for(;;) {
    /* t = (x * x) - a */
    mw_mp_copy(&x, &t);      /* can't fail, t is big enough for original x */
    if((res = mw_mp_sqr(&t, &t)) != MP_OKAY ||
       (res = mw_mp_sub(&t, a, &t)) != MP_OKAY)
      goto CLEANUP;

    /* t = t / 2x       */
    s_mw_mp_mul_2(&x);
    if((res = mw_mp_div(&t, &x, &t, NULL)) != MP_OKAY)
      goto CLEANUP;
    s_mw_mp_div_2(&x);

    /* Terminate the loop, if the quotient is zero */
    if(mw_mp_cmw_mp_z(&t) == MP_EQ)
      break;

    /* x = x - t       */
    if((res = mw_mp_sub(&x, &t, &x)) != MP_OKAY)
      goto CLEANUP;

  }

  /* Copy result to output parameter */
  mw_mp_sub_d(&x, 1, &x);
  s_mw_mp_exch(&x, b);

 CLEANUP:
  mw_mp_clear(&x);
 X:
  mw_mp_clear(&t); 

  return res;

} /* end mw_mp_sqrt() */

/* }}} */

/* }}} */

/*------------------------------------------------------------------------*/
/* {{{ Modular arithmetic */

#if MP_MODARITH
/* {{{ mw_mp_addmod(a, b, m, c) */

/*
  mw_mp_addmod(a, b, m, c)

  Compute c = (a + b) mod m
 */

mw_mp_err mw_mp_addmod(mw_mp_int *a, mw_mp_int *b, mw_mp_int *m, mw_mp_int *c)
{
  mw_mp_err  res;

  ARGCHK(a != NULL && b != NULL && m != NULL && c != NULL, MP_BADARG);

  if((res = mw_mp_add(a, b, c)) != MP_OKAY)
    return res;
  if((res = mw_mp_mod(c, m, c)) != MP_OKAY)
    return res;

  return MP_OKAY;

}

/* }}} */

/* {{{ mw_mp_submod(a, b, m, c) */

/*
  mw_mp_submod(a, b, m, c)

  Compute c = (a - b) mod m
 */

mw_mp_err mw_mp_submod(mw_mp_int *a, mw_mp_int *b, mw_mp_int *m, mw_mp_int *c)
{
  mw_mp_err  res;

  ARGCHK(a != NULL && b != NULL && m != NULL && c != NULL, MP_BADARG);

  if((res = mw_mp_sub(a, b, c)) != MP_OKAY)
    return res;
  if((res = mw_mp_mod(c, m, c)) != MP_OKAY)
    return res;

  return MP_OKAY;

}

/* }}} */

/* {{{ mw_mp_mulmod(a, b, m, c) */

/*
  mw_mp_mulmod(a, b, m, c)

  Compute c = (a * b) mod m
 */

mw_mp_err mw_mp_mulmod(mw_mp_int *a, mw_mp_int *b, mw_mp_int *m, mw_mp_int *c)
{
  mw_mp_err  res;

  ARGCHK(a != NULL && b != NULL && m != NULL && c != NULL, MP_BADARG);

  if((res = mw_mp_mul(a, b, c)) != MP_OKAY)
    return res;
  if((res = mw_mp_mod(c, m, c)) != MP_OKAY)
    return res;

  return MP_OKAY;

}

/* }}} */

/* {{{ mw_mp_sqrmod(a, m, c) */

#if MP_SQUARE
mw_mp_err mw_mp_sqrmod(mw_mp_int *a, mw_mp_int *m, mw_mp_int *c)
{
  mw_mp_err  res;

  ARGCHK(a != NULL && m != NULL && c != NULL, MP_BADARG);

  if((res = mw_mp_sqr(a, c)) != MP_OKAY)
    return res;
  if((res = mw_mp_mod(c, m, c)) != MP_OKAY)
    return res;

  return MP_OKAY;

} /* end mw_mp_sqrmod() */
#endif

/* }}} */

/* {{{ mw_mp_exptmod(a, b, m, c) */

/*
  mw_mp_exptmod(a, b, m, c)

  Compute c = (a ** b) mod m.  Uses a standard square-and-multiply
  method with modular reductions at each step. (This is basically the
  same code as mw_mp_expt(), except for the addition of the reductions)
  
  The modular reductions are done using Barrett's algorithm (see
  s_mw_mp_reduce() below for details)
 */

mw_mp_err mw_mp_exptmod(mw_mp_int *a, mw_mp_int *b, mw_mp_int *m, mw_mp_int *c)
{
  mw_mp_int   s, x, mu;
  mw_mp_err   res;
  mw_mp_digit d, *db = DIGITS(b);
  mw_mp_size  ub = USED(b);
  /// Miranda NG adaptation start - MSVC
  ///int      dig, bit;
  int bit;
  mw_mp_size dig;
  /// Miranda NG adaptation end

  ARGCHK(a != NULL && b != NULL && c != NULL, MP_BADARG);

  if(mw_mp_cmw_mp_z(b) < 0 || mw_mp_cmw_mp_z(m) <= 0)
    return MP_RANGE;

  if((res = mw_mp_init(&s)) != MP_OKAY)
    return res;
  if((res = mw_mp_init_copy(&x, a)) != MP_OKAY)
    goto X;
  if((res = mw_mp_mod(&x, m, &x)) != MP_OKAY ||
     (res = mw_mp_init(&mu)) != MP_OKAY)
    goto MU;

  mw_mp_set(&s, 1);

  /* mu = b^2k / m */
  s_mw_mp_add_d(&mu, 1); 
  s_mw_mp_lshd(&mu, 2 * USED(m));
  if((res = mw_mp_div(&mu, m, &mu, NULL)) != MP_OKAY)
    goto CLEANUP;

  /* Loop over digits of b in ascending order, except highest order */
  for(dig = 0; dig < (ub - 1); dig++) {
    d = *db++;

    /* Loop over the bits of the lower-order digits */
    for(bit = 0; bit < DIGIT_BIT; bit++) {
      if(d & 1) {
	if((res = s_mw_mp_mul(&s, &x)) != MP_OKAY)
	  goto CLEANUP;
	if((res = s_mw_mp_reduce(&s, m, &mu)) != MP_OKAY)
	  goto CLEANUP;
      }

      d >>= 1;

      if((res = s_mw_mp_sqr(&x)) != MP_OKAY)
	goto CLEANUP;
      if((res = s_mw_mp_reduce(&x, m, &mu)) != MP_OKAY)
	goto CLEANUP;
    }
  }

  /* Now do the last digit... */
  d = *db;

  while(d) {
    if(d & 1) {
      if((res = s_mw_mp_mul(&s, &x)) != MP_OKAY)
	goto CLEANUP;
      if((res = s_mw_mp_reduce(&s, m, &mu)) != MP_OKAY)
	goto CLEANUP;
    }

    d >>= 1;

    if((res = s_mw_mp_sqr(&x)) != MP_OKAY)
      goto CLEANUP;
    if((res = s_mw_mp_reduce(&x, m, &mu)) != MP_OKAY)
      goto CLEANUP;
  }

  s_mw_mp_exch(&s, c);

 CLEANUP:
  mw_mp_clear(&mu);
 MU:
  mw_mp_clear(&x);
 X:
  mw_mp_clear(&s);

  return res;

} /* end mw_mp_exptmod() */

/* }}} */

/* {{{ mw_mp_exptmod_d(a, d, m, c) */

mw_mp_err mw_mp_exptmod_d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *m, mw_mp_int *c)
{
  mw_mp_int   s, x;
  mw_mp_err   res;

  ARGCHK(a != NULL && c != NULL, MP_BADARG);

  if((res = mw_mp_init(&s)) != MP_OKAY)
    return res;
  if((res = mw_mp_init_copy(&x, a)) != MP_OKAY)
    goto X;

  mw_mp_set(&s, 1);

  while(d != 0) {
    if(d & 1) {
      if((res = s_mw_mp_mul(&s, &x)) != MP_OKAY ||
	 (res = mw_mp_mod(&s, m, &s)) != MP_OKAY)
	goto CLEANUP;
    }

    d /= 2;

    if((res = s_mw_mp_sqr(&x)) != MP_OKAY ||
       (res = mw_mp_mod(&x, m, &x)) != MP_OKAY)
      goto CLEANUP;
  }

  s_mw_mp_exch(&s, c);

CLEANUP:
  mw_mp_clear(&x);
X:
  mw_mp_clear(&s);

  return res;

} /* end mw_mp_exptmod_d() */

/* }}} */
#endif /* if MP_MODARITH */

/* }}} */

/*------------------------------------------------------------------------*/
/* {{{ Comparison functions */

/* {{{ mw_mp_cmw_mp_z(a) */

/*
  mw_mp_cmw_mp_z(a)

  Compare a <=> 0.  Returns <0 if a<0, 0 if a=0, >0 if a>0.
 */

int    mw_mp_cmw_mp_z(mw_mp_int *a)
{
  if(SIGN(a) == MP_NEG)
    return MP_LT;
  else if(USED(a) == 1 && DIGIT(a, 0) == 0)
    return MP_EQ;
  else
    return MP_GT;

} /* end mw_mp_cmw_mp_z() */

/* }}} */

/* {{{ mw_mp_cmw_mp_d(a, d) */

/*
  mw_mp_cmw_mp_d(a, d)

  Compare a <=> d.  Returns <0 if a<d, 0 if a=d, >0 if a>d
 */

int    mw_mp_cmw_mp_d(mw_mp_int *a, mw_mp_digit d)
{
  ARGCHK(a != NULL, MP_EQ);

  if(SIGN(a) == MP_NEG)
    return MP_LT;

  return s_mw_mp_cmw_mp_d(a, d);

} /* end mw_mp_cmw_mp_d() */

/* }}} */

/* {{{ mw_mp_cmp(a, b) */

int    mw_mp_cmp(mw_mp_int *a, mw_mp_int *b)
{
  ARGCHK(a != NULL && b != NULL, MP_EQ);

  if(SIGN(a) == SIGN(b)) {
    int  mag;

    if((mag = s_mw_mp_cmp(a, b)) == MP_EQ)
      return MP_EQ;

    if(SIGN(a) == MP_ZPOS)
      return mag;
    else
      return -mag;

  } else if(SIGN(a) == MP_ZPOS) {
    return MP_GT;
  } else {
    return MP_LT;
  }

} /* end mw_mp_cmp() */

/* }}} */

/* {{{ mw_mp_cmw_mp_mag(a, b) */

/*
  mw_mp_cmw_mp_mag(a, b)

  Compares |a| <=> |b|, and returns an appropriate comparison result
 */

int    mw_mp_cmw_mp_mag(mw_mp_int *a, mw_mp_int *b)
{
  ARGCHK(a != NULL && b != NULL, MP_EQ);

  return s_mw_mp_cmp(a, b);

} /* end mw_mp_cmw_mp_mag() */

/* }}} */

/* {{{ mw_mp_cmw_mp_int(a, z) */

/*
  This just converts z to an mw_mp_int, and uses the existing comparison
  routines.  This is sort of inefficient, but it's not clear to me how
  frequently this wil get used anyway.  For small positive constants,
  you can always use mw_mp_cmw_mp_d(), and for zero, there is mw_mp_cmw_mp_z().
 */
int    mw_mp_cmw_mp_int(mw_mp_int *a, long z)
{
  mw_mp_int  tmp;
  int     out;

  ARGCHK(a != NULL, MP_EQ);
  
  mw_mp_init(&tmp); mw_mp_set_int(&tmp, z);
  out = mw_mp_cmp(a, &tmp);
  mw_mp_clear(&tmp);

  return out;

} /* end mw_mp_cmw_mp_int() */

/* }}} */

/* {{{ mw_mp_isodd(a) */

/*
  mw_mp_isodd(a)

  Returns a true (non-zero) value if a is odd, false (zero) otherwise.
 */
int    mw_mp_isodd(mw_mp_int *a)
{
  ARGCHK(a != NULL, 0);

  return (DIGIT(a, 0) & 1);

} /* end mw_mp_isodd() */

/* }}} */

/* {{{ mw_mp_iseven(a) */

int    mw_mp_iseven(mw_mp_int *a)
{
  return !mw_mp_isodd(a);

} /* end mw_mp_iseven() */

/* }}} */

/* }}} */

/*------------------------------------------------------------------------*/
/* {{{ Number theoretic functions */

#if MP_NUMTH
/* {{{ mw_mp_gcd(a, b, c) */

/*
  Like the old mw_mp_gcd() function, except computes the GCD using the
  binary algorithm due to Josef Stein in 1961 (via Knuth).
 */
mw_mp_err mw_mp_gcd(mw_mp_int *a, mw_mp_int *b, mw_mp_int *c)
{
  mw_mp_err   res;
  mw_mp_int   u, v, t;
  mw_mp_size  k = 0;

  ARGCHK(a != NULL && b != NULL && c != NULL, MP_BADARG);

  if(mw_mp_cmw_mp_z(a) == MP_EQ && mw_mp_cmw_mp_z(b) == MP_EQ)
      return MP_RANGE;
  if(mw_mp_cmw_mp_z(a) == MP_EQ) {
    if((res = mw_mp_copy(b, c)) != MP_OKAY)
      return res;
    SIGN(c) = MP_ZPOS; return MP_OKAY;
  } else if(mw_mp_cmw_mp_z(b) == MP_EQ) {
    if((res = mw_mp_copy(a, c)) != MP_OKAY)
      return res;
    SIGN(c) = MP_ZPOS; return MP_OKAY;
  }

  if((res = mw_mp_init(&t)) != MP_OKAY)
    return res;
  if((res = mw_mp_init_copy(&u, a)) != MP_OKAY)
    goto U;
  if((res = mw_mp_init_copy(&v, b)) != MP_OKAY)
    goto V;

  SIGN(&u) = MP_ZPOS;
  SIGN(&v) = MP_ZPOS;

  /* Divide out common factors of 2 until at least 1 of a, b is even */
  while(mw_mp_iseven(&u) && mw_mp_iseven(&v)) {
    s_mw_mp_div_2(&u);
    s_mw_mp_div_2(&v);
    ++k;
  }

  /* Initialize t */
  if(mw_mp_isodd(&u)) {
    if((res = mw_mp_copy(&v, &t)) != MP_OKAY)
      goto CLEANUP;
    
    /* t = -v */
    if(SIGN(&v) == MP_ZPOS)
      SIGN(&t) = MP_NEG;
    else
      SIGN(&t) = MP_ZPOS;
    
  } else {
    if((res = mw_mp_copy(&u, &t)) != MP_OKAY)
      goto CLEANUP;

  }

  for(;;) {
    while(mw_mp_iseven(&t)) {
      s_mw_mp_div_2(&t);
    }

    if(mw_mp_cmw_mp_z(&t) == MP_GT) {
      if((res = mw_mp_copy(&t, &u)) != MP_OKAY)
	goto CLEANUP;

    } else {
      if((res = mw_mp_copy(&t, &v)) != MP_OKAY)
	goto CLEANUP;

      /* v = -t */
      if(SIGN(&t) == MP_ZPOS)
	SIGN(&v) = MP_NEG;
      else
	SIGN(&v) = MP_ZPOS;
    }

    if((res = mw_mp_sub(&u, &v, &t)) != MP_OKAY)
      goto CLEANUP;

    if(s_mw_mp_cmw_mp_d(&t, 0) == MP_EQ)
      break;
  }

  s_mw_mp_2expt(&v, k);       /* v = 2^k   */
  res = mw_mp_mul(&u, &v, c); /* c = u * v */

 CLEANUP:
  mw_mp_clear(&v);
 V:
  mw_mp_clear(&u);
 U:
  mw_mp_clear(&t);

  return res;

} /* end mw_mp_bgcd() */

/* }}} */

/* {{{ mw_mp_lcm(a, b, c) */

/* We compute the least common multiple using the rule:

   ab = [a, b](a, b)

   ... by computing the product, and dividing out the gcd.
 */

mw_mp_err mw_mp_lcm(mw_mp_int *a, mw_mp_int *b, mw_mp_int *c)
{
  mw_mp_int  gcd, prod;
  mw_mp_err  res;

  ARGCHK(a != NULL && b != NULL && c != NULL, MP_BADARG);

  /* Set up temporaries */
  if((res = mw_mp_init(&gcd)) != MP_OKAY)
    return res;
  if((res = mw_mp_init(&prod)) != MP_OKAY)
    goto GCD;

  if((res = mw_mp_mul(a, b, &prod)) != MP_OKAY)
    goto CLEANUP;
  if((res = mw_mp_gcd(a, b, &gcd)) != MP_OKAY)
    goto CLEANUP;

  res = mw_mp_div(&prod, &gcd, c, NULL);

 CLEANUP:
  mw_mp_clear(&prod);
 GCD:
  mw_mp_clear(&gcd);

  return res;

} /* end mw_mp_lcm() */

/* }}} */

/* {{{ mw_mp_xgcd(a, b, g, x, y) */

/*
  mw_mp_xgcd(a, b, g, x, y)

  Compute g = (a, b) and values x and y satisfying Bezout's identity
  (that is, ax + by = g).  This uses the extended binary GCD algorithm
  based on the Stein algorithm used for mw_mp_gcd()
 */

mw_mp_err mw_mp_xgcd(mw_mp_int *a, mw_mp_int *b, mw_mp_int *g, mw_mp_int *x, mw_mp_int *y)
{
  mw_mp_int   gx, xc, yc, u, v, A, B, C, D;
  mw_mp_int  *clean[9];
  mw_mp_err   res;
  int      last = -1;

  if(mw_mp_cmw_mp_z(b) == 0)
    return MP_RANGE;

  /* Initialize all these variables we need */
  if((res = mw_mp_init(&u)) != MP_OKAY) goto CLEANUP;
  clean[++last] = &u;
  if((res = mw_mp_init(&v)) != MP_OKAY) goto CLEANUP;
  clean[++last] = &v;
  if((res = mw_mp_init(&gx)) != MP_OKAY) goto CLEANUP;
  clean[++last] = &gx;
  if((res = mw_mp_init(&A)) != MP_OKAY) goto CLEANUP;
  clean[++last] = &A;
  if((res = mw_mp_init(&B)) != MP_OKAY) goto CLEANUP;
  clean[++last] = &B;
  if((res = mw_mp_init(&C)) != MP_OKAY) goto CLEANUP;
  clean[++last] = &C;
  if((res = mw_mp_init(&D)) != MP_OKAY) goto CLEANUP;
  clean[++last] = &D;
  if((res = mw_mp_init_copy(&xc, a)) != MP_OKAY) goto CLEANUP;
  clean[++last] = &xc;
  mw_mp_abs(&xc, &xc);
  if((res = mw_mp_init_copy(&yc, b)) != MP_OKAY) goto CLEANUP;
  clean[++last] = &yc;
  mw_mp_abs(&yc, &yc);

  mw_mp_set(&gx, 1);

  /* Divide by two until at least one of them is even */
  while(mw_mp_iseven(&xc) && mw_mp_iseven(&yc)) {
    s_mw_mp_div_2(&xc);
    s_mw_mp_div_2(&yc);
    if((res = s_mw_mp_mul_2(&gx)) != MP_OKAY)
      goto CLEANUP;
  }

  mw_mp_copy(&xc, &u);
  mw_mp_copy(&yc, &v);
  mw_mp_set(&A, 1); mw_mp_set(&D, 1);

  /* Loop through binary GCD algorithm */
  for(;;) {
    while(mw_mp_iseven(&u)) {
      s_mw_mp_div_2(&u);

      if(mw_mp_iseven(&A) && mw_mp_iseven(&B)) {
	s_mw_mp_div_2(&A); s_mw_mp_div_2(&B);
      } else {
	if((res = mw_mp_add(&A, &yc, &A)) != MP_OKAY) goto CLEANUP;
	s_mw_mp_div_2(&A);
	if((res = mw_mp_sub(&B, &xc, &B)) != MP_OKAY) goto CLEANUP;
	s_mw_mp_div_2(&B);
      }
    }

    while(mw_mp_iseven(&v)) {
      s_mw_mp_div_2(&v);

      if(mw_mp_iseven(&C) && mw_mp_iseven(&D)) {
	s_mw_mp_div_2(&C); s_mw_mp_div_2(&D);
      } else {
	if((res = mw_mp_add(&C, &yc, &C)) != MP_OKAY) goto CLEANUP;
	s_mw_mp_div_2(&C);
	if((res = mw_mp_sub(&D, &xc, &D)) != MP_OKAY) goto CLEANUP;
	s_mw_mp_div_2(&D);
      }
    }

    if(mw_mp_cmp(&u, &v) >= 0) {
      if((res = mw_mp_sub(&u, &v, &u)) != MP_OKAY) goto CLEANUP;
      if((res = mw_mp_sub(&A, &C, &A)) != MP_OKAY) goto CLEANUP;
      if((res = mw_mp_sub(&B, &D, &B)) != MP_OKAY) goto CLEANUP;

    } else {
      if((res = mw_mp_sub(&v, &u, &v)) != MP_OKAY) goto CLEANUP;
      if((res = mw_mp_sub(&C, &A, &C)) != MP_OKAY) goto CLEANUP;
      if((res = mw_mp_sub(&D, &B, &D)) != MP_OKAY) goto CLEANUP;

    }

    /* If we're done, copy results to output */
    if(mw_mp_cmw_mp_z(&u) == 0) {
      if(x)
	if((res = mw_mp_copy(&C, x)) != MP_OKAY) goto CLEANUP;

      if(y)
	if((res = mw_mp_copy(&D, y)) != MP_OKAY) goto CLEANUP;
      
      if(g)
	if((res = mw_mp_mul(&gx, &v, g)) != MP_OKAY) goto CLEANUP;

      break;
    }
  }

 CLEANUP:
  while(last >= 0)
    mw_mp_clear(clean[last--]);

  return res;

} /* end mw_mp_xgcd() */

/* }}} */

/* {{{ mw_mp_invmod(a, m, c) */

/*
  mw_mp_invmod(a, m, c)

  Compute c = a^-1 (mod m), if there is an inverse for a (mod m).
  This is equivalent to the question of whether (a, m) = 1.  If not,
  MP_UNDEF is returned, and there is no inverse.
 */

mw_mp_err mw_mp_invmod(mw_mp_int *a, mw_mp_int *m, mw_mp_int *c)
{
  mw_mp_int  g, x;
  mw_mp_sign sa;
  mw_mp_err  res;

  ARGCHK(a && m && c, MP_BADARG);

  if(mw_mp_cmw_mp_z(a) == 0 || mw_mp_cmw_mp_z(m) == 0)
    return MP_RANGE;

  sa = SIGN(a);

  if((res = mw_mp_init(&g)) != MP_OKAY)
    return res;
  if((res = mw_mp_init(&x)) != MP_OKAY)
    goto X;

  if((res = mw_mp_xgcd(a, m, &g, &x, NULL)) != MP_OKAY)
    goto CLEANUP;

  if(mw_mp_cmw_mp_d(&g, 1) != MP_EQ) {
    res = MP_UNDEF;
    goto CLEANUP;
  }

  res = mw_mp_mod(&x, m, c);
  SIGN(c) = sa;

CLEANUP:
  mw_mp_clear(&x);
X:
  mw_mp_clear(&g);

  return res;

} /* end mw_mp_invmod() */

/* }}} */
#endif /* if MP_NUMTH */

/* }}} */

/*------------------------------------------------------------------------*/
/* {{{ mw_mp_print(mp, ofp) */

#if MP_IOFUNC
/*
  mw_mp_print(mp, ofp)

  Print a textual representation of the given mw_mp_int on the output
  stream 'ofp'.  Output is generated using the internal radix.
 */

void   mw_mp_print(mw_mp_int *mp, FILE *ofp)
{
  int   ix;

  if(mp == NULL || ofp == NULL)
    return;

  fputc((SIGN(mp) == MP_NEG) ? '-' : '+', ofp);

  for(ix = USED(mp) - 1; ix >= 0; ix--) {
    fprintf(ofp, DIGIT_FMT, DIGIT(mp, ix));
  }

} /* end mw_mp_print() */

#endif /* if MP_IOFUNC */

/* }}} */

/*------------------------------------------------------------------------*/
/* {{{ More I/O Functions */

/* {{{ mw_mp_read_signed_bin(mp, str, len) */

/* 
   mw_mp_read_signed_bin(mp, str, len)

   Read in a raw value (base 256) into the given mw_mp_int
 */

mw_mp_err  mw_mp_read_signed_bin(mw_mp_int *mp, unsigned char *str, int len)
{
  mw_mp_err         res;

  ARGCHK(mp != NULL && str != NULL && len > 0, MP_BADARG);

  if((res = mw_mp_read_unsigned_bin(mp, str + 1, len - 1)) == MP_OKAY) {
    /* Get sign from first byte */
    if(str[0])
      SIGN(mp) = MP_NEG;
    else
      SIGN(mp) = MP_ZPOS;
  }

  return res;

} /* end mw_mp_read_signed_bin() */

/* }}} */

/* {{{ mw_mp_signed_bin_size(mp) */

int    mw_mp_signed_bin_size(mw_mp_int *mp)
{
  ARGCHK(mp != NULL, 0);

  return mw_mp_unsigned_bin_size(mp) + 1;

} /* end mw_mp_signed_bin_size() */

/* }}} */

/* {{{ mw_mp_to_signed_bin(mp, str) */

mw_mp_err mw_mp_to_signed_bin(mw_mp_int *mp, unsigned char *str)
{
  ARGCHK(mp != NULL && str != NULL, MP_BADARG);

  /* Caller responsible for allocating enough memory (use mw_mp_raw_size(mp)) */
  str[0] = (char)SIGN(mp);

  return mw_mp_to_unsigned_bin(mp, str + 1);

} /* end mw_mp_to_signed_bin() */

/* }}} */

/* {{{ mw_mp_read_unsigned_bin(mp, str, len) */

/*
  mw_mp_read_unsigned_bin(mp, str, len)

  Read in an unsigned value (base 256) into the given mw_mp_int
 */

mw_mp_err  mw_mp_read_unsigned_bin(mw_mp_int *mp, unsigned char *str, int len)
{
  int     ix;
  mw_mp_err  res;

  ARGCHK(mp != NULL && str != NULL && len > 0, MP_BADARG);

  mw_mp_zero(mp);

  for(ix = 0; ix < len; ix++) {
    if((res = s_mw_mp_mul_2d(mp, CHAR_BIT)) != MP_OKAY)
      return res;

    if((res = mw_mp_add_d(mp, str[ix], mp)) != MP_OKAY)
      return res;
  }
  
  return MP_OKAY;
  
} /* end mw_mp_read_unsigned_bin() */

/* }}} */

/* {{{ mw_mp_unsigned_bin_size(mp) */

int     mw_mp_unsigned_bin_size(mw_mp_int *mp) 
{
  mw_mp_digit   topdig;
  int        count;

  ARGCHK(mp != NULL, 0);

  /* Special case for the value zero */
  if(USED(mp) == 1 && DIGIT(mp, 0) == 0)
    return 1;

  count = (USED(mp) - 1) * sizeof(mw_mp_digit);
  topdig = DIGIT(mp, USED(mp) - 1);

  while(topdig != 0) {
    ++count;
    topdig >>= CHAR_BIT;
  }

  return count;

} /* end mw_mp_unsigned_bin_size() */

/* }}} */

/* {{{ mw_mp_to_unsigned_bin(mp, str) */

mw_mp_err mw_mp_to_unsigned_bin(mw_mp_int *mp, unsigned char *str)
{
  mw_mp_digit      *dp, *end, d;
  unsigned char *spos;

  ARGCHK(mp != NULL && str != NULL, MP_BADARG);

  dp = DIGITS(mp);
  end = dp + USED(mp) - 1;
  spos = str;

  /* Special case for zero, quick test */
  if(dp == end && *dp == 0) {
    *str = '\0';
    return MP_OKAY;
  }

  /* Generate digits in reverse order */
  while(dp < end) {
    int      ix;

    d = *dp;
    for(ix = 0; ix < sizeof(mw_mp_digit); ++ix) {
      *spos = d & UCHAR_MAX;
      d >>= CHAR_BIT;
      ++spos;
    }

    ++dp;
  }

  /* Now handle last digit specially, high order zeroes are not written */
  d = *end;
  while(d != 0) {
    *spos = d & UCHAR_MAX;
    d >>= CHAR_BIT;
    ++spos;
  }

  /* Reverse everything to get digits in the correct order */
  while(--spos > str) {
    unsigned char t = *str;
    *str = *spos;
    *spos = t;

    ++str;
  }

  return MP_OKAY;

} /* end mw_mp_to_unsigned_bin() */

/* }}} */

/* {{{ mw_mp_count_bits(mp) */

int    mw_mp_count_bits(mw_mp_int *mp)
{
  int      len;
  mw_mp_digit d;

  ARGCHK(mp != NULL, MP_BADARG);

  len = DIGIT_BIT * (USED(mp) - 1);
  d = DIGIT(mp, USED(mp) - 1);

  while(d != 0) {
    ++len;
    d >>= 1;
  }

  return len;
  
} /* end mw_mp_count_bits() */

/* }}} */

/* {{{ mw_mp_read_radix(mp, str, radix) */

/*
  mw_mp_read_radix(mp, str, radix)

  Read an integer from the given string, and set mp to the resulting
  value.  The input is presumed to be in base 10.  Leading non-digit
  characters are ignored, and the function reads until a non-digit
  character or the end of the string.
 */

mw_mp_err  mw_mp_read_radix(mw_mp_int *mp, unsigned char *str, int radix)
{
  int     ix = 0, val = 0;
  mw_mp_err  res;
  mw_mp_sign sig = MP_ZPOS;

  ARGCHK(mp != NULL && str != NULL && radix >= 2 && radix <= MAX_RADIX, 
	 MP_BADARG);

  mw_mp_zero(mp);

  /* Skip leading non-digit characters until a digit or '-' or '+' */
  while(str[ix] && 
	(s_mw_mp_tovalue(str[ix], radix) < 0) && 
	str[ix] != '-' &&
	str[ix] != '+') {
    ++ix;
  }

  if(str[ix] == '-') {
    sig = MP_NEG;
    ++ix;
  } else if(str[ix] == '+') {
    sig = MP_ZPOS; /* this is the default anyway... */
    ++ix;
  }

  while((val = s_mw_mp_tovalue(str[ix], radix)) >= 0) {
    if((res = s_mw_mp_mul_d(mp, radix)) != MP_OKAY)
      return res;
    if((res = s_mw_mp_add_d(mp, val)) != MP_OKAY)
      return res;
    ++ix;
  }

  if(s_mw_mp_cmw_mp_d(mp, 0) == MP_EQ)
    SIGN(mp) = MP_ZPOS;
  else
    SIGN(mp) = sig;

  return MP_OKAY;

} /* end mw_mp_read_radix() */

/* }}} */

/* {{{ mw_mp_radix_size(mp, radix) */

int    mw_mp_radix_size(mw_mp_int *mp, int radix)
{
  int  len;
  ARGCHK(mp != NULL, 0);

  len = s_mw_mp_outlen(mw_mp_count_bits(mp), radix) + 1; /* for NUL terminator */

  if(mw_mp_cmw_mp_z(mp) < 0)
    ++len; /* for sign */

  return len;

} /* end mw_mp_radix_size() */

/* }}} */

/* {{{ mw_mp_value_radix_size(num, qty, radix) */

/* num = number of digits
   qty = number of bits per digit
   radix = target base
   
   Return the number of digits in the specified radix that would be
   needed to express 'num' digits of 'qty' bits each.
 */
int    mw_mp_value_radix_size(int num, int qty, int radix)
{
  ARGCHK(num >= 0 && qty > 0 && radix >= 2 && radix <= MAX_RADIX, 0);

  return s_mw_mp_outlen(num * qty, radix);

} /* end mw_mp_value_radix_size() */

/* }}} */

/* {{{ mw_mp_toradix(mp, str, radix) */

mw_mp_err mw_mp_toradix(mw_mp_int *mp, unsigned char *str, int radix)
{
  int  ix, pos = 0;

  ARGCHK(mp != NULL && str != NULL, MP_BADARG);
  ARGCHK(radix > 1 && radix <= MAX_RADIX, MP_RANGE);

  if(mw_mp_cmw_mp_z(mp) == MP_EQ) {
    str[0] = '0';
    str[1] = '\0';
  } else {
    mw_mp_err   res;
    mw_mp_int   tmp;
    mw_mp_sign  sgn;
    mw_mp_digit rem, rdx = (mw_mp_digit)radix;
    char     ch;

    if((res = mw_mp_init_copy(&tmp, mp)) != MP_OKAY)
      return res;

    /* Save sign for later, and take absolute value */
    sgn = SIGN(&tmp); SIGN(&tmp) = MP_ZPOS;

    /* Generate output digits in reverse order      */
    while(mw_mp_cmw_mp_z(&tmp) != 0) {
      if((res = s_mw_mp_div_d(&tmp, rdx, &rem)) != MP_OKAY) {
	mw_mp_clear(&tmp);
	return res;
      }

      /* Generate digits, use capital letters */
      ch = s_mw_mp_todigit(rem, radix, 0);

      str[pos++] = ch;
    }

    /* Add - sign if original value was negative */
    if(sgn == MP_NEG)
      str[pos++] = '-';

    /* Add trailing NUL to end the string        */
    str[pos--] = '\0';

    /* Reverse the digits and sign indicator     */
    ix = 0;
    while(ix < pos) {
      char tmp = str[ix];

      str[ix] = str[pos];
      str[pos] = tmp;
      ++ix;
      --pos;
    }
    
    mw_mp_clear(&tmp);
  }

  return MP_OKAY;

} /* end mw_mp_toradix() */

/* }}} */

/* {{{ mw_mp_char2value(ch, r) */

int    mw_mp_char2value(char ch, int r)
{
  return s_mw_mp_tovalue(ch, r);

} /* end mw_mp_tovalue() */

/* }}} */

/* }}} */

/* {{{ mw_mp_strerror(ec) */

/*
  mw_mp_strerror(ec)

  Return a string describing the meaning of error code 'ec'.  The
  string returned is allocated in static memory, so the caller should
  not attempt to modify or free the memory associated with this
  string.
 */
const char  *mw_mp_strerror(mw_mp_err ec)
{
  int   aec = (ec < 0) ? -ec : ec;

  /* Code values are negative, so the senses of these comparisons
     are accurate */
  if(ec < MP_LAST_CODE || ec > MP_OKAY) {
    return mw_mp_err_string[0];  /* unknown error code */
  } else {
    return mw_mp_err_string[aec + 1];
  }

} /* end mw_mp_strerror() */

/* }}} */

/*========================================================================*/
/*------------------------------------------------------------------------*/
/* Static function definitions (internal use only)                        */

/* {{{ Memory management */

/* {{{ s_mw_mp_grow(mp, min) */

/* Make sure there are at least 'min' digits allocated to mp              */
mw_mp_err   s_mw_mp_grow(mw_mp_int *mp, mw_mp_size min)
{
  if(min > ALLOC(mp)) {
    mw_mp_digit   *tmp;

    /* Set min to next nearest default precision block size */
    min = ((min + (s_mw_mp_defprec - 1)) / s_mw_mp_defprec) * s_mw_mp_defprec;

    if((tmp = s_mw_mp_alloc(min, sizeof(mw_mp_digit))) == NULL)
      return MP_MEM;

    s_mw_mp_copy(DIGITS(mp), tmp, USED(mp));

#if MP_CRYPTO
    s_mw_mp_setz(DIGITS(mp), ALLOC(mp));
#endif
    s_mw_mp_free(DIGITS(mp));
    DIGITS(mp) = tmp;
    ALLOC(mp) = min;
  }

  return MP_OKAY;

} /* end s_mw_mp_grow() */

/* }}} */

/* {{{ s_mw_mp_pad(mp, min) */

/* Make sure the used size of mp is at least 'min', growing if needed     */
mw_mp_err   s_mw_mp_pad(mw_mp_int *mp, mw_mp_size min)
{
  if(min > USED(mp)) {
    mw_mp_err  res;

    /* Make sure there is room to increase precision  */
    if(min > ALLOC(mp) && (res = s_mw_mp_grow(mp, min)) != MP_OKAY)
      return res;

    /* Increase precision; should already be 0-filled */
    USED(mp) = min;
  }

  return MP_OKAY;

} /* end s_mw_mp_pad() */

/* }}} */

/* {{{ s_mw_mp_setz(dp, count) */

#if MP_MACRO == 0
/* Set 'count' digits pointed to by dp to be zeroes                       */
void s_mw_mp_setz(mw_mp_digit *dp, mw_mp_size count)
{
#if MP_MEMSET == 0
  int  ix;

  for(ix = 0; ix < count; ix++)
    dp[ix] = 0;
#else
  memset(dp, 0, count * sizeof(mw_mp_digit));
#endif

} /* end s_mw_mp_setz() */
#endif

/* }}} */

/* {{{ s_mw_mp_copy(sp, dp, count) */

#if MP_MACRO == 0
/* Copy 'count' digits from sp to dp                                      */
void s_mw_mp_copy(mw_mp_digit *sp, mw_mp_digit *dp, mw_mp_size count)
{
#if MP_MEMCPY == 0
  int  ix;

  for(ix = 0; ix < count; ix++)
    dp[ix] = sp[ix];
#else
  memcpy(dp, sp, count * sizeof(mw_mp_digit));
#endif

} /* end s_mw_mp_copy() */
#endif

/* }}} */

/* {{{ s_mw_mp_alloc(nb, ni) */

#if MP_MACRO == 0
/* Allocate ni records of nb bytes each, and return a pointer to that     */
void    *s_mw_mp_alloc(size_t nb, size_t ni)
{
  return calloc(nb, ni);

} /* end s_mw_mp_alloc() */
#endif

/* }}} */

/* {{{ s_mw_mp_free(ptr) */

#if MP_MACRO == 0
/* Free the memory pointed to by ptr                                      */
void     s_mw_mp_free(void *ptr)
{
  if(ptr)
    free(ptr);

} /* end s_mw_mp_free() */
#endif

/* }}} */

/* {{{ s_mw_mp_clamp(mp) */

/* Remove leading zeroes from the given value                             */
void     s_mw_mp_clamp(mw_mp_int *mp)
{
  mw_mp_size   du = USED(mp);
  mw_mp_digit *zp = DIGITS(mp) + du - 1;

  while(du > 1 && !*zp--)
    --du;

  if(du == 1 && *zp == 0)
    SIGN(mp) = MP_ZPOS;

  USED(mp) = du;

} /* end s_mw_mp_clamp() */


/* }}} */

/* {{{ s_mw_mp_exch(a, b) */

/* Exchange the data for a and b; (b, a) = (a, b)                         */
void     s_mw_mp_exch(mw_mp_int *a, mw_mp_int *b)
{
  mw_mp_int   tmp;

  tmp = *a;
  *a = *b;
  *b = tmp;

} /* end s_mw_mp_exch() */

/* }}} */

/* }}} */

/* {{{ Arithmetic helpers */

/* {{{ s_mw_mp_lshd(mp, p) */

/* 
   Shift mp leftward by p digits, growing if needed, and zero-filling
   the in-shifted digits at the right end.  This is a convenient
   alternative to multiplication by powers of the radix
 */   

mw_mp_err   s_mw_mp_lshd(mw_mp_int *mp, mw_mp_size p)
{
  mw_mp_err   res;
  mw_mp_size  pos;
  mw_mp_digit *dp;
  int     ix;

  if(p == 0)
    return MP_OKAY;

  if((res = s_mw_mp_pad(mp, USED(mp) + p)) != MP_OKAY)
    return res;

  pos = USED(mp) - 1;
  dp = DIGITS(mp);

  /* Shift all the significant figures over as needed */
  /// Miranda NG adaptation start - MSVC
  ///for(ix = pos - p; ix >= 0; ix--) 
  for(ix = (int)(pos - p); ix >= 0; ix--) 
  /// Miranda NG adaptation end
    dp[ix + p] = dp[ix];

  /* Fill the bottom digits with zeroes */
  /// Miranda NG adaptation start - MSVC
  ///for(ix = 0; ix < p; ix++)
  for(ix = 0; ix < (int)p; ix++) 
  /// Miranda NG adaptation end
    dp[ix] = 0;

  return MP_OKAY;

} /* end s_mw_mp_lshd() */

/* }}} */

/* {{{ s_mw_mp_rshd(mp, p) */

/* 
   Shift mp rightward by p digits.  Maintains the invariant that
   digits above the precision are all zero.  Digits shifted off the
   end are lost.  Cannot fail.
 */

void     s_mw_mp_rshd(mw_mp_int *mp, mw_mp_size p)
{
  mw_mp_size  ix;
  mw_mp_digit *dp;

  if(p == 0)
    return;

  /* Shortcut when all digits are to be shifted off */
  if(p >= USED(mp)) {
    s_mw_mp_setz(DIGITS(mp), ALLOC(mp));
    USED(mp) = 1;
    SIGN(mp) = MP_ZPOS;
    return;
  }

  /* Shift all the significant figures over as needed */
  dp = DIGITS(mp);
  for(ix = p; ix < USED(mp); ix++)
    dp[ix - p] = dp[ix];

  /* Fill the top digits with zeroes */
  ix -= p;
  while(ix < USED(mp))
    dp[ix++] = 0;

  /* Strip off any leading zeroes    */
  s_mw_mp_clamp(mp);

} /* end s_mw_mp_rshd() */

/* }}} */

/* {{{ s_mw_mp_div_2(mp) */

/* Divide by two -- take advantage of radix properties to do it fast      */
void     s_mw_mp_div_2(mw_mp_int *mp)
{
  s_mw_mp_div_2d(mp, 1);

} /* end s_mw_mp_div_2() */

/* }}} */

/* {{{ s_mw_mp_mul_2(mp) */

mw_mp_err s_mw_mp_mul_2(mw_mp_int *mp)
{
  /// Miranda NG adaptation start - MSVC
  //int      ix;
  mw_mp_size ix;
  /// Miranda NG adaptation end
  mw_mp_digit kin = 0, kout, *dp = DIGITS(mp);
  mw_mp_err   res;

  /* Shift digits leftward by 1 bit */
  for(ix = 0; ix < USED(mp); ix++) {
    kout = (dp[ix] >> (DIGIT_BIT - 1)) & 1;
    dp[ix] = (dp[ix] << 1) | kin;

    kin = kout;
  }

  /* Deal with rollover from last digit */
  if(kin) {
    if(ix >= ALLOC(mp)) {
      if((res = s_mw_mp_grow(mp, ALLOC(mp) + 1)) != MP_OKAY)
	return res;
      dp = DIGITS(mp);
    }

    dp[ix] = kin;
    USED(mp) += 1;
  }

  return MP_OKAY;

} /* end s_mw_mp_mul_2() */

/* }}} */

/* {{{ s_mw_mp_mod_2d(mp, d) */

/*
  Remainder the integer by 2^d, where d is a number of bits.  This
  amounts to a bitwise AND of the value, and does not require the full
  division code
 */
void     s_mw_mp_mod_2d(mw_mp_int *mp, mw_mp_digit d)
{
  unsigned int  ndig = (d / DIGIT_BIT), nbit = (d % DIGIT_BIT);
  unsigned int  ix;
  mw_mp_digit      dmask, *dp = DIGITS(mp);

  if(ndig >= USED(mp))
    return;

  /* Flush all the bits above 2^d in its digit */
  dmask = (1 << nbit) - 1;
  dp[ndig] &= dmask;

  /* Flush all digits above the one with 2^d in it */
  for(ix = ndig + 1; ix < USED(mp); ix++)
    dp[ix] = 0;

  s_mw_mp_clamp(mp);

} /* end s_mw_mp_mod_2d() */

/* }}} */

/* {{{ s_mw_mp_mul_2d(mp, d) */

/*
  Multiply by the integer 2^d, where d is a number of bits.  This
  amounts to a bitwise shift of the value, and does not require the
  full multiplication code.
 */
mw_mp_err    s_mw_mp_mul_2d(mw_mp_int *mp, mw_mp_digit d)
{
  mw_mp_err   res;
  mw_mp_digit save, next, mask, *dp;
  mw_mp_size  used;
  /// Miranda NG adaptation start - MSVC
  ///int      ix;
  mw_mp_size ix;
  /// Miranda NG adaptation end

  if((res = s_mw_mp_lshd(mp, d / DIGIT_BIT)) != MP_OKAY)
    return res;

  dp = DIGITS(mp); used = USED(mp);
  d %= DIGIT_BIT;

  mask = (1 << d) - 1;

  /* If the shift requires another digit, make sure we've got one to
     work with */
  if((dp[used - 1] >> (DIGIT_BIT - d)) & mask) {
    if((res = s_mw_mp_grow(mp, used + 1)) != MP_OKAY)
      return res;
    dp = DIGITS(mp);
  }

  /* Do the shifting... */
  save = 0;
  for(ix = 0; ix < used; ix++) {
    next = (dp[ix] >> (DIGIT_BIT - d)) & mask;
    dp[ix] = (dp[ix] << d) | save;
    save = next;
  }

  /* If, at this point, we have a nonzero carryout into the next
     digit, we'll increase the size by one digit, and store it...
   */
  if(save) {
    dp[used] = save;
    USED(mp) += 1;
  }

  s_mw_mp_clamp(mp);
  return MP_OKAY;

} /* end s_mw_mp_mul_2d() */

/* }}} */

/* {{{ s_mw_mp_div_2d(mp, d) */

/*
  Divide the integer by 2^d, where d is a number of bits.  This
  amounts to a bitwise shift of the value, and does not require the
  full division code (used in Barrett reduction, see below)
 */
void     s_mw_mp_div_2d(mw_mp_int *mp, mw_mp_digit d)
{
  int       ix;
  mw_mp_digit  save, next, mask, *dp = DIGITS(mp);

  s_mw_mp_rshd(mp, d / DIGIT_BIT);
  d %= DIGIT_BIT;

  mask = (1 << d) - 1;

  save = 0;
  for(ix = USED(mp) - 1; ix >= 0; ix--) {
    next = dp[ix] & mask;
    dp[ix] = (dp[ix] >> d) | (save << (DIGIT_BIT - d));
    save = next;
  }

  s_mw_mp_clamp(mp);

} /* end s_mw_mp_div_2d() */

/* }}} */

/* {{{ s_mw_mp_norm(a, b) */

/*
  s_mw_mp_norm(a, b)

  Normalize a and b for division, where b is the divisor.  In order
  that we might make good guesses for quotient digits, we want the
  leading digit of b to be at least half the radix, which we
  accomplish by multiplying a and b by a constant.  This constant is
  returned (so that it can be divided back out of the remainder at the
  end of the division process).

  We multiply by the smallest power of 2 that gives us a leading digit
  at least half the radix.  By choosing a power of 2, we simplify the 
  multiplication and division steps to simple shifts.
 */
mw_mp_digit s_mw_mp_norm(mw_mp_int *a, mw_mp_int *b)
{
  mw_mp_digit  t, d = 0;

  t = DIGIT(b, USED(b) - 1);
  while(t < (RADIX / 2)) {
    t <<= 1;
    ++d;
  }
    
  if(d != 0) {
    s_mw_mp_mul_2d(a, d);
    s_mw_mp_mul_2d(b, d);
  }

  return d;

} /* end s_mw_mp_norm() */

/* }}} */

/* }}} */

/* {{{ Primitive digit arithmetic */

/* {{{ s_mw_mp_add_d(mp, d) */

/* Add d to |mp| in place                                                 */
mw_mp_err   s_mw_mp_add_d(mw_mp_int *mp, mw_mp_digit d)    /* unsigned digit addition */
{
  mw_mp_word   w, k = 0;
  mw_mp_size   ix = 1, used = USED(mp);
  mw_mp_digit *dp = DIGITS(mp);

  w = dp[0] + d;
  dp[0] = ACCUM(w);
  k = CARRYOUT(w);

  while(ix < used && k) {
    w = dp[ix] + k;
    dp[ix] = ACCUM(w);
    k = CARRYOUT(w);
    ++ix;
  }

  if(k != 0) {
    mw_mp_err  res;

    if((res = s_mw_mp_pad(mp, USED(mp) + 1)) != MP_OKAY)
      return res;

    DIGIT(mp, ix) = k;
  }

  return MP_OKAY;

} /* end s_mw_mp_add_d() */

/* }}} */

/* {{{ s_mw_mp_sub_d(mp, d) */

/* Subtract d from |mp| in place, assumes |mp| > d                        */
mw_mp_err   s_mw_mp_sub_d(mw_mp_int *mp, mw_mp_digit d)    /* unsigned digit subtract */
{
  mw_mp_word   w, b = 0;
  mw_mp_size   ix = 1, used = USED(mp);
  mw_mp_digit *dp = DIGITS(mp);

  /* Compute initial subtraction    */
  w = (RADIX + dp[0]) - d;
  b = CARRYOUT(w) ? 0 : 1;
  dp[0] = ACCUM(w);

  /* Propagate borrows leftward     */
  while(b && ix < used) {
    w = (RADIX + dp[ix]) - b;
    b = CARRYOUT(w) ? 0 : 1;
    dp[ix] = ACCUM(w);
    ++ix;
  }

  /* Remove leading zeroes          */
  s_mw_mp_clamp(mp);

  /* If we have a borrow out, it's a violation of the input invariant */
  if(b)
    return MP_RANGE;
  else
    return MP_OKAY;

} /* end s_mw_mp_sub_d() */

/* }}} */

/* {{{ s_mw_mp_mul_d(a, d) */

/* Compute a = a * d, single digit multiplication                         */
mw_mp_err   s_mw_mp_mul_d(mw_mp_int *a, mw_mp_digit d)
{
  mw_mp_word w, k = 0;
  mw_mp_size ix, max;
  mw_mp_err  res;
  mw_mp_digit *dp = DIGITS(a);

  /*
    Single-digit multiplication will increase the precision of the
    output by at most one digit.  However, we can detect when this
    will happen -- if the high-order digit of a, times d, gives a
    two-digit result, then the precision of the result will increase;
    otherwise it won't.  We use this fact to avoid calling s_mw_mp_pad()
    unless absolutely necessary.
   */
  max = USED(a);
  w = dp[max - 1] * d;
  if(CARRYOUT(w) != 0) {
    if((res = s_mw_mp_pad(a, max + 1)) != MP_OKAY)
      return res;
    dp = DIGITS(a);
  }

  for(ix = 0; ix < max; ix++) {
    w = (dp[ix] * d) + k;
    dp[ix] = ACCUM(w);
    k = CARRYOUT(w);
  }

  /* If there is a precision increase, take care of it here; the above
     test guarantees we have enough storage to do this safely.
   */
  if(k) {
    dp[max] = k; 
    USED(a) = max + 1;
  }

  s_mw_mp_clamp(a);

  return MP_OKAY;
  
} /* end s_mw_mp_mul_d() */

/* }}} */

/* {{{ s_mw_mp_div_d(mp, d, r) */

/*
  s_mw_mp_div_d(mp, d, r)

  Compute the quotient mp = mp / d and remainder r = mp mod d, for a
  single digit d.  If r is null, the remainder will be discarded.
 */

mw_mp_err   s_mw_mp_div_d(mw_mp_int *mp, mw_mp_digit d, mw_mp_digit *r)
{
  mw_mp_word   w = 0, t;
  mw_mp_int    quot;
  mw_mp_err    res;
  mw_mp_digit *dp = DIGITS(mp), *qp;
  int       ix;

  if(d == 0)
    return MP_RANGE;

  /* Make room for the quotient */
  if((res = mw_mp_init_size(&quot, USED(mp))) != MP_OKAY)
    return res;

  USED(&quot) = USED(mp); /* so clamping will work below */
  qp = DIGITS(&quot);

  /* Divide without subtraction */
  for(ix = USED(mp) - 1; ix >= 0; ix--) {
    w = (w << DIGIT_BIT) | dp[ix];

    if(w >= d) {
      t = w / d;
      w = w % d;
    } else {
      t = 0;
    }

    qp[ix] = t;
  }

  /* Deliver the remainder, if desired */
  if(r)
    *r = w;

  s_mw_mp_clamp(&quot);
  mw_mp_exch(&quot, mp);
  mw_mp_clear(&quot);

  return MP_OKAY;

} /* end s_mw_mp_div_d() */

/* }}} */

/* }}} */

/* {{{ Primitive full arithmetic */

/* {{{ s_mw_mp_add(a, b) */

/* Compute a = |a| + |b|                                                  */
mw_mp_err   s_mw_mp_add(mw_mp_int *a, mw_mp_int *b)        /* magnitude addition      */
{
  mw_mp_word   w = 0;
  mw_mp_digit *pa, *pb;
  mw_mp_size   ix, used = USED(b);
  mw_mp_err    res;

  /* Make sure a has enough precision for the output value */
  if((used > USED(a)) && (res = s_mw_mp_pad(a, used)) != MP_OKAY)
    return res;

  /*
    Add up all digits up to the precision of b.  If b had initially
    the same precision as a, or greater, we took care of it by the
    padding step above, so there is no problem.  If b had initially
    less precision, we'll have to make sure the carry out is duly
    propagated upward among the higher-order digits of the sum.
   */
  pa = DIGITS(a);
  pb = DIGITS(b);
  for(ix = 0; ix < used; ++ix) {
    w += *pa + *pb++;
    *pa++ = ACCUM(w);
    w = CARRYOUT(w);
  }

  /* If we run out of 'b' digits before we're actually done, make
     sure the carries get propagated upward...  
   */
  used = USED(a);
  while(w && ix < used) {
    w += *pa;
    *pa++ = ACCUM(w);
    w = CARRYOUT(w);
    ++ix;
  }

  /* If there's an overall carry out, increase precision and include
     it.  We could have done this initially, but why touch the memory
     allocator unless we're sure we have to?
   */
  if(w) {
    if((res = s_mw_mp_pad(a, used + 1)) != MP_OKAY)
      return res;

    DIGIT(a, ix) = w;  /* pa may not be valid after s_mw_mp_pad() call */
  }

  return MP_OKAY;

} /* end s_mw_mp_add() */

/* }}} */

/* {{{ s_mw_mp_sub(a, b) */

/* Compute a = |a| - |b|, assumes |a| >= |b|                              */
mw_mp_err   s_mw_mp_sub(mw_mp_int *a, mw_mp_int *b)        /* magnitude subtract      */
{
  mw_mp_word   w = 0;
  mw_mp_digit *pa, *pb;
  mw_mp_size   ix, used = USED(b);

  /*
    Subtract and propagate borrow.  Up to the precision of b, this
    accounts for the digits of b; after that, we just make sure the
    carries get to the right place.  This saves having to pad b out to
    the precision of a just to make the loops work right...
   */
  pa = DIGITS(a);
  pb = DIGITS(b);

  for(ix = 0; ix < used; ++ix) {
    w = (RADIX + *pa) - w - *pb++;
    *pa++ = ACCUM(w);
    w = CARRYOUT(w) ? 0 : 1;
  }

  used = USED(a);
  while(ix < used) {
    w = RADIX + *pa - w;
    *pa++ = ACCUM(w);
    w = CARRYOUT(w) ? 0 : 1;
    ++ix;
  }

  /* Clobber any leading zeroes we created    */
  s_mw_mp_clamp(a);

  /* 
     If there was a borrow out, then |b| > |a| in violation
     of our input invariant.  We've already done the work,
     but we'll at least complain about it...
   */
  if(w)
    return MP_RANGE;
  else
    return MP_OKAY;

} /* end s_mw_mp_sub() */

/* }}} */

/* {{{ s_mw_mp_mul(a, b) */

/* Compute a = |a| * |b|                                                  */
mw_mp_err   s_mw_mp_mul(mw_mp_int *a, mw_mp_int *b)
{
  mw_mp_word   w, k = 0;
  mw_mp_int    tmp;
  mw_mp_err    res;
  mw_mp_size   ix, jx, ua = USED(a), ub = USED(b);
  mw_mp_digit *pa, *pb, *pt, *pbt;

  if((res = mw_mp_init_size(&tmp, ua + ub)) != MP_OKAY)
    return res;

  /* This has the effect of left-padding with zeroes... */
  USED(&tmp) = ua + ub;

  /* We're going to need the base value each iteration */
  pbt = DIGITS(&tmp);

  /* Outer loop:  Digits of b */

  pb = DIGITS(b);
  for(ix = 0; ix < ub; ++ix, ++pb) {
    if(*pb == 0) 
      continue;

    /* Inner product:  Digits of a */
    pa = DIGITS(a);
    for(jx = 0; jx < ua; ++jx, ++pa) {
      pt = pbt + ix + jx;
      w = *pb * *pa + k + *pt;
      *pt = ACCUM(w);
      k = CARRYOUT(w);
    }

    pbt[ix + jx] = k;
    k = 0;
  }

  s_mw_mp_clamp(&tmp);
  s_mw_mp_exch(&tmp, a);

  mw_mp_clear(&tmp);

  return MP_OKAY;

} /* end s_mw_mp_mul() */

/* }}} */

/* {{{ s_mw_mp_kmul(a, b, out, len) */

#if 0
void   s_mw_mp_kmul(mw_mp_digit *a, mw_mp_digit *b, mw_mp_digit *out, mw_mp_size len)
{
  mw_mp_word   w, k = 0;
  mw_mp_size   ix, jx;
  mw_mp_digit *pa, *pt;

  for(ix = 0; ix < len; ++ix, ++b) {
    if(*b == 0)
      continue;
    
    pa = a;
    for(jx = 0; jx < len; ++jx, ++pa) {
      pt = out + ix + jx;
      w = *b * *pa + k + *pt;
      *pt = ACCUM(w);
      k = CARRYOUT(w);
    }

    out[ix + jx] = k;
    k = 0;
  }

} /* end s_mw_mp_kmul() */
#endif

/* }}} */

/* {{{ s_mw_mp_sqr(a) */

/*
  Computes the square of a, in place.  This can be done more
  efficiently than a general multiplication, because many of the
  computation steps are redundant when squaring.  The inner product
  step is a bit more complicated, but we save a fair number of
  iterations of the multiplication loop.
 */
#if MP_SQUARE
mw_mp_err   s_mw_mp_sqr(mw_mp_int *a)
{
  mw_mp_word  w, k = 0;
  mw_mp_int   tmp;
  mw_mp_err   res;
  mw_mp_size  ix, jx, kx, used = USED(a);
  mw_mp_digit *pa1, *pa2, *pt, *pbt;

  if((res = mw_mp_init_size(&tmp, 2 * used)) != MP_OKAY)
    return res;

  /* Left-pad with zeroes */
  USED(&tmp) = 2 * used;

  /* We need the base value each time through the loop */
  pbt = DIGITS(&tmp);

  pa1 = DIGITS(a);
  for(ix = 0; ix < used; ++ix, ++pa1) {
    if(*pa1 == 0)
      continue;

    w = DIGIT(&tmp, ix + ix) + (*pa1 * *pa1);

    pbt[ix + ix] = ACCUM(w);
    k = CARRYOUT(w);

    /*
      The inner product is computed as:

         (C, S) = t[i,j] + 2 a[i] a[j] + C

      This can overflow what can be represented in an mw_mp_word, and
      since C arithmetic does not provide any way to check for
      overflow, we have to check explicitly for overflow conditions
      before they happen.
     */
    for(jx = ix + 1, pa2 = DIGITS(a) + jx; jx < used; ++jx, ++pa2) {
      mw_mp_word  u = 0, v;
      
      /* Store this in a temporary to avoid indirections later */
      pt = pbt + ix + jx;

      /* Compute the multiplicative step */
      w = *pa1 * *pa2;

      /* If w is more than half MP_WORD_MAX, the doubling will
	 overflow, and we need to record a carry out into the next
	 word */
      u = (w >> (MP_WORD_BIT - 1)) & 1;

      /* Double what we've got, overflow will be ignored as defined
	 for C arithmetic (we've already noted if it is to occur)
       */
      w *= 2;

      /* Compute the additive step */
      v = *pt + k;

      /* If we do not already have an overflow carry, check to see
	 if the addition will cause one, and set the carry out if so 
       */
      u |= ((MP_WORD_MAX - v) < w);

      /* Add in the rest, again ignoring overflow */
      w += v;

      /* Set the i,j digit of the output */
      *pt = ACCUM(w);

      /* Save carry information for the next iteration of the loop.
	 This is why k must be an mw_mp_word, instead of an mw_mp_digit */
      k = CARRYOUT(w) | (u << DIGIT_BIT);

    } /* for(jx ...) */

    /* Set the last digit in the cycle and reset the carry */
    k = DIGIT(&tmp, ix + jx) + k;
    pbt[ix + jx] = ACCUM(k);
    k = CARRYOUT(k);

    /* If we are carrying out, propagate the carry to the next digit
       in the output.  This may cascade, so we have to be somewhat
       circumspect -- but we will have enough precision in the output
       that we won't overflow 
     */
    kx = 1;
    while(k) {
      k = pbt[ix + jx + kx] + 1;
      pbt[ix + jx + kx] = ACCUM(k);
      k = CARRYOUT(k);
      ++kx;
    }
  } /* for(ix ...) */

  s_mw_mp_clamp(&tmp);
  s_mw_mp_exch(&tmp, a);

  mw_mp_clear(&tmp);

  return MP_OKAY;

} /* end s_mw_mp_sqr() */
#endif

/* }}} */

/* {{{ s_mw_mp_div(a, b) */

/*
  s_mw_mp_div(a, b)

  Compute a = a / b and b = a mod b.  Assumes b > a.
 */

mw_mp_err   s_mw_mp_div(mw_mp_int *a, mw_mp_int *b)
{
  mw_mp_int   quot, rem, t;
  mw_mp_word  q;
  mw_mp_err   res;
  mw_mp_digit d;
  int      ix;

  if(mw_mp_cmw_mp_z(b) == 0)
    return MP_RANGE;

  /* Shortcut if b is power of two */
  if((ix = s_mw_mp_ispow2(b)) >= 0) {
    mw_mp_copy(a, b);  /* need this for remainder */
    s_mw_mp_div_2d(a, (mw_mp_digit)ix);
    s_mw_mp_mod_2d(b, (mw_mp_digit)ix);

    return MP_OKAY;
  }

  /* Allocate space to store the quotient */
  if((res = mw_mp_init_size(&quot, USED(a))) != MP_OKAY)
    return res;

  /* A working temporary for division     */
  if((res = mw_mp_init_size(&t, USED(a))) != MP_OKAY)
    goto T;

  /* Allocate space for the remainder     */
  if((res = mw_mp_init_size(&rem, USED(a))) != MP_OKAY)
    goto REM;

  /* Normalize to optimize guessing       */
  d = s_mw_mp_norm(a, b);

  /* Perform the division itself...woo!   */
  ix = USED(a) - 1;

  while(ix >= 0) {
    /* Find a partial substring of a which is at least b */
    while(s_mw_mp_cmp(&rem, b) < 0 && ix >= 0) {
      if((res = s_mw_mp_lshd(&rem, 1)) != MP_OKAY) 
	goto CLEANUP;

      if((res = s_mw_mp_lshd(&quot, 1)) != MP_OKAY)
	goto CLEANUP;

      DIGIT(&rem, 0) = DIGIT(a, ix);
      s_mw_mp_clamp(&rem);
      --ix;
    }

    /* If we didn't find one, we're finished dividing    */
    if(s_mw_mp_cmp(&rem, b) < 0) 
      break;    

    /* Compute a guess for the next quotient digit       */
    q = DIGIT(&rem, USED(&rem) - 1);
    if(q <= DIGIT(b, USED(b) - 1) && USED(&rem) > 1)
      q = (q << DIGIT_BIT) | DIGIT(&rem, USED(&rem) - 2);

    q /= DIGIT(b, USED(b) - 1);

    /* The guess can be as much as RADIX + 1 */
    if(q >= RADIX)
      q = RADIX - 1;

    /* See what that multiplies out to                   */
    mw_mp_copy(b, &t);
    if((res = s_mw_mp_mul_d(&t, q)) != MP_OKAY)
      goto CLEANUP;

    /* 
       If it's too big, back it off.  We should not have to do this
       more than once, or, in rare cases, twice.  Knuth describes a
       method by which this could be reduced to a maximum of once, but
       I didn't implement that here.
     */
    while(s_mw_mp_cmp(&t, &rem) > 0) {
      --q;
      s_mw_mp_sub(&t, b);
    }

    /* At this point, q should be the right next digit   */
    if((res = s_mw_mp_sub(&rem, &t)) != MP_OKAY)
      goto CLEANUP;

    /*
      Include the digit in the quotient.  We allocated enough memory
      for any quotient we could ever possibly get, so we should not
      have to check for failures here
     */
    DIGIT(&quot, 0) = q;
  }

  /* Denormalize remainder                */
  if(d != 0) 
    s_mw_mp_div_2d(&rem, d);

  s_mw_mp_clamp(&quot);
  s_mw_mp_clamp(&rem);

  /* Copy quotient back to output         */
  s_mw_mp_exch(&quot, a);
  
  /* Copy remainder back to output        */
  s_mw_mp_exch(&rem, b);

CLEANUP:
  mw_mp_clear(&rem);
REM:
  mw_mp_clear(&t);
T:
  mw_mp_clear(&quot);

  return res;

} /* end s_mw_mp_div() */

/* }}} */

/* {{{ s_mw_mp_2expt(a, k) */

mw_mp_err   s_mw_mp_2expt(mw_mp_int *a, mw_mp_digit k)
{
  mw_mp_err    res;
  mw_mp_size   dig, bit;

  dig = k / DIGIT_BIT;
  bit = k % DIGIT_BIT;

  mw_mp_zero(a);
  if((res = s_mw_mp_pad(a, dig + 1)) != MP_OKAY)
    return res;
  
  DIGIT(a, dig) |= (1 << bit);

  return MP_OKAY;

} /* end s_mw_mp_2expt() */

/* }}} */

/* {{{ s_mw_mp_reduce(x, m, mu) */

/*
  Compute Barrett reduction, x (mod m), given a precomputed value for
  mu = b^2k / m, where b = RADIX and k = #digits(m).  This should be
  faster than straight division, when many reductions by the same
  value of m are required (such as in modular exponentiation).  This
  can nearly halve the time required to do modular exponentiation,
  as compared to using the full integer divide to reduce.

  This algorithm was derived from the _Handbook of Applied
  Cryptography_ by Menezes, Oorschot and VanStone, Ch. 14,
  pp. 603-604.  
 */

mw_mp_err   s_mw_mp_reduce(mw_mp_int *x, mw_mp_int *m, mw_mp_int *mu)
{
  mw_mp_int   q;
  mw_mp_err   res;
  mw_mp_size  um = USED(m);

  if((res = mw_mp_init_copy(&q, x)) != MP_OKAY)
    return res;

  s_mw_mp_rshd(&q, um - 1);       /* q1 = x / b^(k-1)  */
  s_mw_mp_mul(&q, mu);            /* q2 = q1 * mu      */
  s_mw_mp_rshd(&q, um + 1);       /* q3 = q2 / b^(k+1) */

  /* x = x mod b^(k+1), quick (no division) */
  s_mw_mp_mod_2d(x, DIGIT_BIT * (um + 1));

  /* q = q * m mod b^(k+1), quick (no division) */
  s_mw_mp_mul(&q, m);
  s_mw_mp_mod_2d(&q, DIGIT_BIT * (um + 1));

  /* x = x - q */
  if((res = mw_mp_sub(x, &q, x)) != MP_OKAY)
    goto CLEANUP;

  /* If x < 0, add b^(k+1) to it */
  if(mw_mp_cmw_mp_z(x) < 0) {
    mw_mp_set(&q, 1);
    if((res = s_mw_mp_lshd(&q, um + 1)) != MP_OKAY)
      goto CLEANUP;
    if((res = mw_mp_add(x, &q, x)) != MP_OKAY)
      goto CLEANUP;
  }

  /* Back off if it's too big */
  while(mw_mp_cmp(x, m) >= 0) {
    if((res = s_mw_mp_sub(x, m)) != MP_OKAY)
      break;
  }

 CLEANUP:
  mw_mp_clear(&q);

  return res;

} /* end s_mw_mp_reduce() */

/* }}} */

/* }}} */

/* {{{ Primitive comparisons */

/* {{{ s_mw_mp_cmp(a, b) */

/* Compare |a| <=> |b|, return 0 if equal, <0 if a<b, >0 if a>b           */
int      s_mw_mp_cmp(mw_mp_int *a, mw_mp_int *b)
{
  mw_mp_size   ua = USED(a), ub = USED(b);

  if(ua > ub)
    return MP_GT;
  else if(ua < ub)
    return MP_LT;
  else {
    int      ix = ua - 1;
    mw_mp_digit *ap = DIGITS(a) + ix, *bp = DIGITS(b) + ix;

    while(ix >= 0) {
      if(*ap > *bp)
	return MP_GT;
      else if(*ap < *bp)
	return MP_LT;

      --ap; --bp; --ix;
    }

    return MP_EQ;
  }

} /* end s_mw_mp_cmp() */

/* }}} */

/* {{{ s_mw_mp_cmw_mp_d(a, d) */

/* Compare |a| <=> d, return 0 if equal, <0 if a<d, >0 if a>d             */
int      s_mw_mp_cmw_mp_d(mw_mp_int *a, mw_mp_digit d)
{
  mw_mp_size  ua = USED(a);
  mw_mp_digit *ap = DIGITS(a);

  if(ua > 1)
    return MP_GT;

  if(*ap < d) 
    return MP_LT;
  else if(*ap > d)
    return MP_GT;
  else
    return MP_EQ;

} /* end s_mw_mp_cmw_mp_d() */

/* }}} */

/* {{{ s_mw_mp_ispow2(v) */

/*
  Returns -1 if the value is not a power of two; otherwise, it returns
  k such that v = 2^k, i.e. lg(v).
 */
int      s_mw_mp_ispow2(mw_mp_int *v)
{
  mw_mp_digit d, *dp;
  mw_mp_size  uv = USED(v);
  int      extra = 0, ix;

  d = DIGIT(v, uv - 1); /* most significant digit of v */

  while(d && ((d & 1) == 0)) {
    d >>= 1;
    ++extra;
  }

  if(d == 1) {
    ix = uv - 2;
    dp = DIGITS(v) + ix;

    while(ix >= 0) {
      if(*dp)
	return -1; /* not a power of two */

      --dp; --ix;
    }

    return ((uv - 1) * DIGIT_BIT) + extra;
  } 

  return -1;

} /* end s_mw_mp_ispow2() */

/* }}} */

/* {{{ s_mw_mp_ispow2d(d) */

int      s_mw_mp_ispow2d(mw_mp_digit d)
{
  int   pow = 0;

  while((d & 1) == 0) {
    ++pow; d >>= 1;
  }

  if(d == 1)
    return pow;

  return -1;

} /* end s_mw_mp_ispow2d() */

/* }}} */

/* }}} */

/* {{{ Primitive I/O helpers */

/* {{{ s_mw_mp_tovalue(ch, r) */

/*
  Convert the given character to its digit value, in the given radix.
  If the given character is not understood in the given radix, -1 is
  returned.  Otherwise the digit's numeric value is returned.

  The results will be odd if you use a radix < 2 or > 62, you are
  expected to know what you're up to.
 */
int      s_mw_mp_tovalue(char ch, int r)
{
  int    val, xch;
  
  if(r > 36)
    xch = ch;
  else
    xch = toupper(ch);

  if(isdigit(xch))
    val = xch - '0';
  else if(isupper(xch))
    val = xch - 'A' + 10;
  else if(islower(xch))
    val = xch - 'a' + 36;
  else if(xch == '+')
    val = 62;
  else if(xch == '/')
    val = 63;
  else 
    return -1;

  if(val < 0 || val >= r)
    return -1;

  return val;

} /* end s_mw_mp_tovalue() */

/* }}} */

/* {{{ s_mw_mp_todigit(val, r, low) */

/*
  Convert val to a radix-r digit, if possible.  If val is out of range
  for r, returns zero.  Otherwise, returns an ASCII character denoting
  the value in the given radix.

  The results may be odd if you use a radix < 2 or > 64, you are
  expected to know what you're doing.
 */
  
char     s_mw_mp_todigit(int val, int r, int low)
{
  char   ch;

  if(val < 0 || val >= r)
    return 0;

  ch = s_dmap_1[val];

  if(r <= 36 && low)
    ch = tolower(ch);

  return ch;

} /* end s_mw_mp_todigit() */

/* }}} */

/* {{{ s_mw_mp_outlen(bits, radix) */

/* 
   Return an estimate for how long a string is needed to hold a radix
   r representation of a number with 'bits' significant bits.

   Does not include space for a sign or a NUL terminator.
 */
int      s_mw_mp_outlen(int bits, int r)
{
  return (int)((double)bits * LOG_V_2(r) + 0.5);

} /* end s_mw_mp_outlen() */

/* }}} */

/* }}} */

/*------------------------------------------------------------------------*/
/* HERE THERE BE DRAGONS                                                  */

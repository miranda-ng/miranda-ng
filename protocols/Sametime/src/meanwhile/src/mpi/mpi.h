/*
  mpi.h
  
  by Michael J. Fromberger <http://www.dartmouth.edu/~sting/>
  Copyright (C) 1998 Michael J. Fromberger, All Rights Reserved
  
  Arbitrary precision integer arithmetic library
  
  modified for use in Meanwhile as a convenience library
*/

#ifndef _H_MPI_
#define _H_MPI_

#include "mpi-config.h"

#if MP_DEBUG
#undef MP_IOFUNC
#define MP_IOFUNC 1
#endif

#if MP_IOFUNC
#include <stdio.h>
#include <ctype.h>
#endif

#include <limits.h>

#define  MP_NEG  1
#define  MP_ZPOS 0

/* Included for compatibility... */
#define  NEG     MP_NEG
#define  ZPOS    MP_ZPOS

#define  MP_OKAY          0 /* no error, all is well */
#define  MP_YES           0 /* yes (boolean result)  */
#define  MP_NO           -1 /* no (boolean result)   */
#define  MP_MEM          -2 /* out of memory         */
#define  MP_RANGE        -3 /* argument out of range */
#define  MP_BADARG       -4 /* invalid parameter     */
#define  MP_UNDEF        -5 /* answer is undefined   */
#define  MP_LAST_CODE    MP_UNDEF

#include "mpi-types.h"

/* Included for compatibility... */
#define DIGIT_BIT         MP_DIGIT_BIT
#define DIGIT_MAX         MP_DIGIT_MAX

/* Macros for accessing the mw_mp_int internals           */
#define  SIGN(MP)     ((MP)->sign)
#define  USED(MP)     ((MP)->used)
#define  ALLOC(MP)    ((MP)->alloc)
#define  DIGITS(MP)   ((MP)->dp)
#define  DIGIT(MP,N)  (MP)->dp[(N)]

#if MP_ARGCHK == 1
#define  ARGCHK(X,Y)  {if(!(X)){return (Y);}}
#elif MP_ARGCHK == 2
#include <assert.h>
#define  ARGCHK(X,Y)  assert(X)
#else
#define  ARGCHK(X,Y)  /*  */
#endif

/* This defines the maximum I/O base (minimum is 2)   */
#define MAX_RADIX         64

typedef struct {
  mw_mp_sign       sign;    /* sign of this quantity      */
  mw_mp_size       alloc;   /* how many digits allocated  */
  mw_mp_size       used;    /* how many digits used       */
  mw_mp_digit     *dp;      /* the digits themselves      */
} mw_mp_int;

/*------------------------------------------------------------------------*/
/* Default precision                                                      */

unsigned int mw_mp_get_prec(void);
void         mw_mp_set_prec(unsigned int prec);

/*------------------------------------------------------------------------*/
/* Memory management                                                      */

mw_mp_err mw_mp_init(mw_mp_int *mp);
mw_mp_err mw_mp_init_array(mw_mp_int mp[], int count);
mw_mp_err mw_mp_init_size(mw_mp_int *mp, mw_mp_size prec);
mw_mp_err mw_mp_init_copy(mw_mp_int *mp, mw_mp_int *from);
mw_mp_err mw_mp_copy(mw_mp_int *from, mw_mp_int *to);
void   mw_mp_exch(mw_mp_int *mp1, mw_mp_int *mp2);
void   mw_mp_clear(mw_mp_int *mp);
void   mw_mp_clear_array(mw_mp_int mp[], int count);
void   mw_mp_zero(mw_mp_int *mp);
void   mw_mp_set(mw_mp_int *mp, mw_mp_digit d);
mw_mp_err mw_mp_set_int(mw_mp_int *mp, long z);

/*------------------------------------------------------------------------*/
/* Single digit arithmetic                                                */

mw_mp_err mw_mp_add_d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *b);
mw_mp_err mw_mp_sub_d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *b);
mw_mp_err mw_mp_mul_d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *b);
mw_mp_err mw_mp_mul_2(mw_mp_int *a, mw_mp_int *c);
mw_mp_err mw_mp_div_d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *q, mw_mp_digit *r);
mw_mp_err mw_mp_div_2(mw_mp_int *a, mw_mp_int *c);
mw_mp_err mw_mp_expt_d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *c);

/*------------------------------------------------------------------------*/
/* Sign manipulations                                                     */

mw_mp_err mw_mp_abs(mw_mp_int *a, mw_mp_int *b);
mw_mp_err mw_mp_neg(mw_mp_int *a, mw_mp_int *b);

/*------------------------------------------------------------------------*/
/* Full arithmetic                                                        */

mw_mp_err mw_mp_add(mw_mp_int *a, mw_mp_int *b, mw_mp_int *c);
mw_mp_err mw_mp_sub(mw_mp_int *a, mw_mp_int *b, mw_mp_int *c);
mw_mp_err mw_mp_mul(mw_mp_int *a, mw_mp_int *b, mw_mp_int *c);
mw_mp_err mw_mp_mul_2d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *c);
#if MP_SQUARE
mw_mp_err mw_mp_sqr(mw_mp_int *a, mw_mp_int *b);
#else
#define mw_mp_sqr(a, b) mw_mp_mul(a, a, b)
#endif
mw_mp_err mw_mp_div(mw_mp_int *a, mw_mp_int *b, mw_mp_int *q, mw_mp_int *r);
mw_mp_err mw_mp_div_2d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *q, mw_mp_int *r);
mw_mp_err mw_mp_expt(mw_mp_int *a, mw_mp_int *b, mw_mp_int *c);
mw_mp_err mw_mp_2expt(mw_mp_int *a, mw_mp_digit k);
mw_mp_err mw_mp_sqrt(mw_mp_int *a, mw_mp_int *b);

/*------------------------------------------------------------------------*/
/* Modular arithmetic                                                     */

#if MP_MODARITH
mw_mp_err mw_mp_mod(mw_mp_int *a, mw_mp_int *m, mw_mp_int *c);
mw_mp_err mw_mp_mod_d(mw_mp_int *a, mw_mp_digit d, mw_mp_digit *c);
mw_mp_err mw_mp_addmod(mw_mp_int *a, mw_mp_int *b, mw_mp_int *m, mw_mp_int *c);
mw_mp_err mw_mp_submod(mw_mp_int *a, mw_mp_int *b, mw_mp_int *m, mw_mp_int *c);
mw_mp_err mw_mp_mulmod(mw_mp_int *a, mw_mp_int *b, mw_mp_int *m, mw_mp_int *c);
#if MP_SQUARE
mw_mp_err mw_mp_sqrmod(mw_mp_int *a, mw_mp_int *m, mw_mp_int *c);
#else
#define mw_mp_sqrmod(a, m, c) mw_mp_mulmod(a, a, m, c)
#endif
mw_mp_err mw_mp_exptmod(mw_mp_int *a, mw_mp_int *b, mw_mp_int *m, mw_mp_int *c);
mw_mp_err mw_mp_exptmod_d(mw_mp_int *a, mw_mp_digit d, mw_mp_int *m, mw_mp_int *c);
#endif /* MP_MODARITH */

/*------------------------------------------------------------------------*/
/* Comparisons                                                            */

int    mw_mp_cmw_mp_z(mw_mp_int *a);
int    mw_mp_cmw_mp_d(mw_mp_int *a, mw_mp_digit d);
int    mw_mp_cmp(mw_mp_int *a, mw_mp_int *b);
int    mw_mp_cmw_mp_mag(mw_mp_int *a, mw_mp_int *b);
int    mw_mp_cmw_mp_int(mw_mp_int *a, long z);
int    mw_mp_isodd(mw_mp_int *a);
int    mw_mp_iseven(mw_mp_int *a);

/*------------------------------------------------------------------------*/
/* Number theoretic                                                       */

#if MP_NUMTH
mw_mp_err mw_mp_gcd(mw_mp_int *a, mw_mp_int *b, mw_mp_int *c);
mw_mp_err mw_mp_lcm(mw_mp_int *a, mw_mp_int *b, mw_mp_int *c);
mw_mp_err mw_mp_xgcd(mw_mp_int *a, mw_mp_int *b, mw_mp_int *g, mw_mp_int *x, mw_mp_int *y);
mw_mp_err mw_mp_invmod(mw_mp_int *a, mw_mp_int *m, mw_mp_int *c);
#endif /* end MP_NUMTH */

/*------------------------------------------------------------------------*/
/* Input and output                                                       */

#if MP_IOFUNC
void   mw_mp_print(mw_mp_int *mp, FILE *ofp);
#endif /* end MP_IOFUNC */

/*------------------------------------------------------------------------*/
/* Base conversion                                                        */

#define BITS     1
#define BYTES    CHAR_BIT

mw_mp_err mw_mp_read_signed_bin(mw_mp_int *mp, unsigned char *str, int len);
int    mw_mp_signed_bin_size(mw_mp_int *mp);
mw_mp_err mw_mp_to_signed_bin(mw_mp_int *mp, unsigned char *str);

mw_mp_err mw_mp_read_unsigned_bin(mw_mp_int *mp, unsigned char *str, int len);
int    mw_mp_unsigned_bin_size(mw_mp_int *mp);
mw_mp_err mw_mp_to_unsigned_bin(mw_mp_int *mp, unsigned char *str);

int    mw_mp_count_bits(mw_mp_int *mp);

#if MP_COMPAT_MACROS
#define mw_mp_read_raw(mp, str, len) mw_mp_read_signed_bin((mp), (str), (len))
#define mw_mp_raw_size(mp)           mw_mp_signed_bin_size(mp)
#define mw_mp_toraw(mp, str)         mw_mp_to_signed_bin((mp), (str))
#define mw_mp_read_mag(mp, str, len) mw_mp_read_unsigned_bin((mp), (str), (len))
#define mw_mp_mag_size(mp)           mw_mp_unsigned_bin_size(mp)
#define mw_mp_tomag(mp, str)         mw_mp_to_unsigned_bin((mp), (str))
#endif

mw_mp_err mw_mp_read_radix(mw_mp_int *mp, unsigned char *str, int radix);
int    mw_mp_radix_size(mw_mp_int *mp, int radix);
int    mw_mp_value_radix_size(int num, int qty, int radix);
mw_mp_err mw_mp_toradix(mw_mp_int *mp, unsigned char *str, int radix);

int    mw_mp_char2value(char ch, int r);

#define mw_mp_tobinary(M, S)  mw_mp_toradix((M), (S), 2)
#define mw_mp_tooctal(M, S)   mw_mp_toradix((M), (S), 8)
#define mw_mp_todecimal(M, S) mw_mp_toradix((M), (S), 10)
#define mw_mp_tohex(M, S)     mw_mp_toradix((M), (S), 16)

/*------------------------------------------------------------------------*/
/* Error strings                                                          */

const  char  *mw_mp_strerror(mw_mp_err ec);

#endif /* end _H_MPI_ */

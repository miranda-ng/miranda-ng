/* g10lib.h - Internal definitions for libgcrypt
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2005
 *               2007, 2011 Free Software Foundation, Inc.
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

/* This header is to be used inside of libgcrypt in place of gcrypt.h.
   This way we can better distinguish between internal and external
   usage of gcrypt.h. */

#ifndef G10LIB_H
#define G10LIB_H 1

#ifdef _GCRYPT_H
#error  gcrypt.h already included
#endif

#ifndef _GCRYPT_IN_LIBGCRYPT
#error something is wrong with config.h
#endif

#include <stdio.h>
#include <stdarg.h>

#include "visibility.h"
#include "types.h"




/* Attribute handling macros.  */

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 5 )
#define JNLIB_GCC_M_FUNCTION 1
#define JNLIB_GCC_A_NR 	     __attribute__ ((noreturn))
#define JNLIB_GCC_A_PRINTF( f, a )  __attribute__ ((format (printf,f,a)))
#define JNLIB_GCC_A_NR_PRINTF( f, a ) \
			    __attribute__ ((noreturn, format (printf,f,a)))
#define GCC_ATTR_NORETURN  __attribute__ ((__noreturn__))
#else
#define JNLIB_GCC_A_NR
#define JNLIB_GCC_A_PRINTF( f, a )
#define JNLIB_GCC_A_NR_PRINTF( f, a )
#define GCC_ATTR_NORETURN
#endif

#if __GNUC__ >= 3
/* According to glibc this attribute is available since 2.8 however we
   better play safe and use it only with gcc 3 or newer. */
#define GCC_ATTR_FORMAT_ARG(a)  __attribute__ ((format_arg (a)))
#else
#define GCC_ATTR_FORMAT_ARG(a)
#endif

/* I am not sure since when the unused attribute is really supported.
   In any case it it only needed for gcc versions which print a
   warning.  Thus let us require gcc >= 3.5.  */
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 5 )
#define GCC_ATTR_UNUSED  __attribute__ ((unused))
#else
#define GCC_ATTR_UNUSED
#endif

/* Gettext macros.  */

#define _(a)  _gcry_gettext(a)
#define N_(a) (a)

/* Some handy macros */
#ifndef STR
#define STR(v) #v
#endif
#define STR2(v) STR(v)
#define DIM(v) (sizeof(v)/sizeof((v)[0]))
#define DIMof(type,member)   DIM(((type *)0)->member)



/*-- src/global.c -*/
int _gcry_global_is_operational (void);
gcry_error_t _gcry_vcontrol (enum gcry_ctl_cmds cmd, va_list arg_ptr);
void  _gcry_check_heap (const void *a);
int _gcry_get_debug_flag (unsigned int mask);

/* Malloc functions and common wrapper macros.  */
void *_gcry_malloc (size_t n) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_calloc (size_t n, size_t m) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_malloc_secure (size_t n) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_calloc_secure (size_t n, size_t m) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_realloc (void *a, size_t n);
char *_gcry_strdup (const char *string) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_xmalloc (size_t n) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_xcalloc (size_t n, size_t m) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_xmalloc_secure (size_t n) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_xcalloc_secure (size_t n, size_t m) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_xrealloc (void *a, size_t n);
char *_gcry_xstrdup (const char * a) _GCRY_GCC_ATTR_MALLOC;
void  _gcry_free (void *a);
int   _gcry_is_secure (const void *a) _GCRY_GCC_ATTR_PURE;

#define xtrymalloc(a)    _gcry_malloc ((a))
#define xtrycalloc(a,b)  _gcry_calloc ((a),(b))
#define xtrymalloc_secure(a)   _gcry_malloc_secure ((a))
#define xtrycalloc_secure(a,b) _gcry_calloc_secure ((a),(b))
#define xtryrealloc(a,b) _gcry_realloc ((a),(b))
#define xtrystrdup(a)    _gcry_strdup ((a))
#define xmalloc(a)       _gcry_xmalloc ((a))
#define xcalloc(a,b)     _gcry_xcalloc ((a),(b))
#define xmalloc_secure(a)   _gcry_xmalloc_secure ((a))
#define xcalloc_secure(a,b) _gcry_xcalloc_secure ((a),(b))
#define xrealloc(a,b)    _gcry_xrealloc ((a),(b))
#define xstrdup(a)       _gcry_xstrdup ((a))
#define xfree(a)         _gcry_free ((a))


/*-- src/misc.c --*/

#if defined(JNLIB_GCC_M_FUNCTION) || __STDC_VERSION__ >= 199901L
void _gcry_bug (const char *file, int line,
                const char *func) GCC_ATTR_NORETURN;
void _gcry_assert_failed (const char *expr, const char *file, int line,
                          const char *func) GCC_ATTR_NORETURN;
#else
void _gcry_bug (const char *file, int line);
void _gcry_assert_failed (const char *expr, const char *file, int line);
#endif

void _gcry_divide_by_zero (void) JNLIB_GCC_A_NR;

const char *_gcry_gettext (const char *key) GCC_ATTR_FORMAT_ARG(1);
void _gcry_fatal_error(int rc, const char *text ) JNLIB_GCC_A_NR;
void _gcry_logv (int level,
                 const char *fmt, va_list arg_ptr) JNLIB_GCC_A_PRINTF(2,0);
void _gcry_log( int level, const char *fmt, ... ) JNLIB_GCC_A_PRINTF(2,3);
void _gcry_log_bug( const char *fmt, ... )   JNLIB_GCC_A_NR_PRINTF(1,2);
void _gcry_log_fatal( const char *fmt, ... ) JNLIB_GCC_A_NR_PRINTF(1,2);
void _gcry_log_error( const char *fmt, ... ) JNLIB_GCC_A_PRINTF(1,2);
void _gcry_log_info( const char *fmt, ... )  JNLIB_GCC_A_PRINTF(1,2);
int  _gcry_log_info_with_dummy_fp (FILE *fp, const char *fmt, ... )
                                             JNLIB_GCC_A_PRINTF(2,3);
void _gcry_log_debug( const char *fmt, ... ) JNLIB_GCC_A_PRINTF(1,2);
void _gcry_log_printf ( const char *fmt, ... ) JNLIB_GCC_A_PRINTF(1,2);
void _gcry_log_printhex (const char *text, const void *buffer, size_t length);
void _gcry_log_printmpi (const char *text, gcry_mpi_t mpi);
void _gcry_log_printsxp (const char *text, gcry_sexp_t sexp);

void _gcry_set_log_verbosity( int level );
int _gcry_log_verbosity( int level );

#ifdef JNLIB_GCC_M_FUNCTION
#define BUG() _gcry_bug( __FILE__ , __LINE__, __FUNCTION__ )
#define gcry_assert(expr) ((expr)? (void)0 \
         : _gcry_assert_failed (STR(expr), __FILE__, __LINE__, __FUNCTION__))
#elif __STDC_VERSION__ >= 199901L
#define BUG() _gcry_bug( __FILE__ , __LINE__, __func__ )
#define gcry_assert(expr) ((expr)? (void)0 \
         : _gcry_assert_failed (STR(expr), __FILE__, __LINE__, __func__))
#else
#define BUG() _gcry_bug( __FILE__ , __LINE__ )
#define gcry_assert(expr) ((expr)? (void)0 \
         : _gcry_assert_failed (STR(expr), __FILE__, __LINE__))
#endif


#define log_bug     _gcry_log_bug
#define log_fatal   _gcry_log_fatal
#define log_error   _gcry_log_error
#define log_info    _gcry_log_info
#define log_debug   _gcry_log_debug
#define log_printf  _gcry_log_printf
#define log_printhex _gcry_log_printhex
#define log_printmpi _gcry_log_printmpi
#define log_printsxp _gcry_log_printsxp

/* Compatibility macro.  */
#define log_mpidump _gcry_log_printmpi


/*-- src/hwfeatures.c --*/
/* (Do not change these values unless synced with the asm code.)  */
#define HWF_PADLOCK_RNG  1
#define HWF_PADLOCK_AES  2
#define HWF_PADLOCK_SHA  4
#define HWF_PADLOCK_MMUL 8

#define HWF_INTEL_CPU    16
#define HWF_INTEL_BMI2   32
#define HWF_INTEL_SSSE3  64
#define HWF_INTEL_PCLMUL 128
#define HWF_INTEL_AESNI  256
#define HWF_INTEL_RDRAND 512
#define HWF_INTEL_AVX    1024
#define HWF_INTEL_AVX2   2048

#define HWF_ARM_NEON     4096


gpg_err_code_t _gcry_disable_hw_feature (const char *name);
void _gcry_detect_hw_features (void);
unsigned int _gcry_get_hw_features (void);
const char *_gcry_enum_hw_features (int idx, unsigned int *r_feature);


/*-- mpi/mpiutil.c --*/
const char *_gcry_mpi_get_hw_config (void);


/*-- cipher/pubkey.c --*/

/* FIXME: shouldn't this go into mpi.h?  */
#ifndef mpi_powm
#define mpi_powm(w,b,e,m)   gcry_mpi_powm( (w), (b), (e), (m) )
#endif

/*-- primegen.c --*/
gcry_err_code_t _gcry_primegen_init (void);
gcry_mpi_t _gcry_generate_secret_prime (unsigned int nbits,
                                 gcry_random_level_t random_level,
                                 int (*extra_check)(void*, gcry_mpi_t),
                                 void *extra_check_arg);
gcry_mpi_t _gcry_generate_public_prime (unsigned int nbits,
                                 gcry_random_level_t random_level,
                                 int (*extra_check)(void*, gcry_mpi_t),
                                 void *extra_check_arg);
gcry_err_code_t _gcry_generate_elg_prime (int mode,
                                          unsigned int pbits,
                                          unsigned int qbits,
                                          gcry_mpi_t g,
                                          gcry_mpi_t *r_prime,
                                          gcry_mpi_t **factors);
gcry_mpi_t _gcry_derive_x931_prime (const gcry_mpi_t xp,
                                    const gcry_mpi_t xp1, const gcry_mpi_t xp2,
                                    const gcry_mpi_t e,
                                    gcry_mpi_t *r_p1, gcry_mpi_t *r_p2);
gpg_err_code_t _gcry_generate_fips186_2_prime
                 (unsigned int pbits, unsigned int qbits,
                  const void *seed, size_t seedlen,
                  gcry_mpi_t *r_q, gcry_mpi_t *r_p,
                  int *r_counter,
                  void **r_seed, size_t *r_seedlen);
gpg_err_code_t _gcry_generate_fips186_3_prime
                 (unsigned int pbits, unsigned int qbits,
                  const void *seed, size_t seedlen,
                  gcry_mpi_t *r_q, gcry_mpi_t *r_p,
                  int *r_counter,
                  void **r_seed, size_t *r_seedlen, int *r_hashalgo);


/* Replacements of missing functions (missing-string.c).  */
#ifndef HAVE_STPCPY
char *stpcpy (char *a, const char *b);
#endif
#ifndef HAVE_STRCASECMP
int strcasecmp (const char *a, const char *b) _GCRY_GCC_ATTR_PURE;
#endif

#include "../compat/libcompat.h"


/* Macros used to rename missing functions.  */
#ifndef HAVE_STRTOUL
#define strtoul(a,b,c)  ((unsigned long)strtol((a),(b),(c)))
#endif
#ifndef HAVE_MEMMOVE
#define memmove(d, s, n) bcopy((s), (d), (n))
#endif
#ifndef HAVE_STRICMP
#define stricmp(a,b)	 strcasecmp( (a), (b) )
#endif
#ifndef HAVE_ATEXIT
#define atexit(a)    (on_exit((a),0))
#endif
#ifndef HAVE_RAISE
#define raise(a) kill(getpid(), (a))
#endif


/* Stack burning.  */

#ifdef HAVE_GCC_ASM_VOLATILE_MEMORY
#define  __gcry_burn_stack_dummy() asm volatile ("":::"memory")
#else
void __gcry_burn_stack_dummy (void);
#endif

void __gcry_burn_stack (unsigned int bytes);
#define _gcry_burn_stack(bytes) \
	do { __gcry_burn_stack (bytes); \
	     __gcry_burn_stack_dummy (); } while(0)


/* To avoid that a compiler optimizes certain memset calls away, these
   macros may be used instead. */
#define wipememory2(_ptr,_set,_len) do { \
              volatile char *_vptr=(volatile char *)(_ptr); \
              size_t _vlen=(_len); \
              unsigned char _vset=(_set); \
              fast_wipememory2(_vptr,_vset,_vlen); \
              while(_vlen) { *_vptr=(_vset); _vptr++; _vlen--; } \
                  } while(0)
#define wipememory(_ptr,_len) wipememory2(_ptr,0,_len)

#ifdef HAVE_U64_TYPEDEF
  #define FASTWIPE_T u64
  #define FASTWIPE_MULT (U64_C(0x0101010101010101))
#else
  #define FASTWIPE_T u32
  #define FASTWIPE_MULT (0x01010101U)
#endif

/* Following architectures can handle unaligned accesses fast.  */
#if defined(__i386__) || defined(__x86_64__) || \
    defined(__powerpc__) || defined(__powerpc64__) || \
    (defined(__arm__) && defined(__ARM_FEATURE_UNALIGNED)) || \
    defined(__aarch64__)
#define fast_wipememory2_unaligned_head(_ptr,_set,_len) /*do nothing*/
#else
#define fast_wipememory2_unaligned_head(_vptr,_vset,_vlen) do { \
              while((size_t)(_vptr)&(sizeof(FASTWIPE_T)-1) && _vlen) \
                { *_vptr=(_vset); _vptr++; _vlen--; } \
                  } while(0)
#endif

/* fast_wipememory2 may leave tail bytes unhandled, in which case tail bytes
   are handled by wipememory2. */
#define fast_wipememory2(_vptr,_vset,_vlen) do { \
              FASTWIPE_T _vset_long = _vset; \
              fast_wipememory2_unaligned_head(_vptr,_vset,_vlen); \
              if (_vlen < sizeof(FASTWIPE_T)) \
                break; \
              _vset_long *= FASTWIPE_MULT; \
              do { \
                volatile FASTWIPE_T *_vptr_long = (volatile void *)_vptr; \
                *_vptr_long = _vset_long; \
                _vlen -= sizeof(FASTWIPE_T); \
                _vptr += sizeof(FASTWIPE_T); \
              } while (_vlen >= sizeof(FASTWIPE_T)); \
                  } while (0)


/* Digit predicates.  */

#define digitp(p)   (*(p) >= '0' && *(p) <= '9')
#define octdigitp(p) (*(p) >= '0' && *(p) <= '7')
#define alphap(a)    (   (*(a) >= 'A' && *(a) <= 'Z')  \
                      || (*(a) >= 'a' && *(a) <= 'z'))
#define hexdigitp(a) (digitp (a)                     \
                      || (*(a) >= 'A' && *(a) <= 'F')  \
                      || (*(a) >= 'a' && *(a) <= 'f'))

/* Init functions.  */

gcry_err_code_t _gcry_cipher_init (void);
gcry_err_code_t _gcry_md_init (void);
gcry_err_code_t _gcry_pk_init (void);
gcry_err_code_t _gcry_secmem_module_init (void);
gcry_err_code_t _gcry_mpi_init (void);

/* Memory management.  */
#define GCRY_ALLOC_FLAG_SECURE (1 << 0)


/*-- sexp.c --*/
gcry_err_code_t _gcry_sexp_vbuild (gcry_sexp_t *retsexp, size_t *erroff,
                                   const char *format, va_list arg_ptr);
char *_gcry_sexp_nth_string (const gcry_sexp_t list, int number);
gpg_err_code_t _gcry_sexp_vextract_param (gcry_sexp_t sexp, const char *path,
                                          const char *list, va_list arg_ptr);


/*-- fips.c --*/

void _gcry_initialize_fips_mode (int force);

int _gcry_fips_mode (void);
#define fips_mode() _gcry_fips_mode ()

int _gcry_enforced_fips_mode (void);

void _gcry_set_enforced_fips_mode (void);

void _gcry_inactivate_fips_mode (const char *text);
int _gcry_is_fips_mode_inactive (void);


void _gcry_fips_signal_error (const char *srcfile,
                              int srcline,
                              const char *srcfunc,
                              int is_fatal,
                              const char *description);
#ifdef JNLIB_GCC_M_FUNCTION
# define fips_signal_error(a) \
           _gcry_fips_signal_error (__FILE__, __LINE__, __FUNCTION__, 0, (a))
# define fips_signal_fatal_error(a) \
           _gcry_fips_signal_error (__FILE__, __LINE__, __FUNCTION__, 1, (a))
#else
# define fips_signal_error(a) \
           _gcry_fips_signal_error (__FILE__, __LINE__, NULL, 0, (a))
# define fips_signal_fatal_error(a) \
           _gcry_fips_signal_error (__FILE__, __LINE__, NULL, 1, (a))
#endif

int _gcry_fips_is_operational (void);
#define fips_is_operational()   (_gcry_global_is_operational ())
#define fips_not_operational()  (GPG_ERR_NOT_OPERATIONAL)

int _gcry_fips_test_operational (void);
int _gcry_fips_test_error_or_operational (void);

gpg_err_code_t _gcry_fips_run_selftests (int extended);

void _gcry_fips_noreturn (void);
#define fips_noreturn()  (_gcry_fips_noreturn ())



#endif /* G10LIB_H */

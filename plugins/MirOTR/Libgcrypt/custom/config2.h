/* config.h.in.  Generated from configure.ac by autoheader.  */


#ifndef _GCRYPT_CONFIG_H_INCLUDED
#define _GCRYPT_CONFIG_H_INCLUDED
/* need this, because some autoconf tests rely on this (e.g. stpcpy)
 * and it should be used for new programs  */
#define _GNU_SOURCE  1


/* Define if building universal (internal helper macro) */
#undef AC_APPLE_UNIVERSAL_BUILD

/* Subversion revision used to build this package */
#undef BUILD_REVISION

/* configure did not test for endianess */
#undef DISABLED_ENDIAN_CHECK

/* Define if you don't want the default EGD socket name. For details see
   cipher/rndegd.c */
#undef EGD_SOCKET_NAME

/* Define to support an HMAC based integrity check */
#undef ENABLE_HMAC_BINARY_CHECK

/* Enable support for the PadLock engine. */
#undef ENABLE_PADLOCK_SUPPORT

/* Define to use the GNU C visibility attribute. */
#undef GCRY_USE_VISIBILITY

/* The default error source for libgcrypt. */
#undef GPG_ERR_SOURCE_DEFAULT

/* Define to 1 if you have the `atexit' function. */
#undef HAVE_ATEXIT

/* Defined if the mlock() call does not work */
#undef HAVE_BROKEN_MLOCK

/* Defined if a `byte' is typedef'd */
#undef HAVE_BYTE_TYPEDEF

/* Define to 1 if you have the `clock_gettime' function. */
#undef HAVE_CLOCK_GETTIME

/* Define to 1 if you have the declaration of `sys_siglist', and to 0 if you
   don't. */
#undef HAVE_DECL_SYS_SIGLIST

/* defined if the system supports a random device */
#undef HAVE_DEV_RANDOM

/* Define to 1 if you have the <dlfcn.h> header file. */
#undef HAVE_DLFCN_H

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
#undef HAVE_DOPRNT

/* defined if we run on some of the PCDOS like systems (DOS, Windoze. OS/2)
   with special properties like no file modes */
#undef HAVE_DOSISH_SYSTEM

/* defined if we must run on a stupid file system */
#undef HAVE_DRIVE_LETTERS

/* Define to 1 if you have the `fcntl' function. */
#undef HAVE_FCNTL

/* Define to 1 if you have the `ftruncate' function. */
#undef HAVE_FTRUNCATE

/* Define to 1 if you have the `gethrtime' function. */
#undef HAVE_GETHRTIME

/* Define to 1 if you have the `getpagesize' function. */
#undef HAVE_GETPAGESIZE

/* Define to 1 if you have the `getrusage' function. */
#undef HAVE_GETRUSAGE

/* Define to 1 if you have the `gettimeofday' function. */
#undef HAVE_GETTIMEOFDAY

/* Define to 1 if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define to 1 if you have the `rt' library (-lrt). */
#undef HAVE_LIBRT

/* Define to 1 if you have the `memmove' function. */
#undef HAVE_MEMMOVE

/* Define to 1 if you have the <memory.h> header file. */
#undef HAVE_MEMORY_H

/* Defined if the system supports an mlock() call */
#undef HAVE_MLOCK

/* Define to 1 if you have the `mmap' function. */
#undef HAVE_MMAP

/* Defined if the GNU Pth is available */
#undef HAVE_PTH

/* Define to 1 if you have the `raise' function. */
#undef HAVE_RAISE

/* Define to 1 if you have the `rand' function. */
#undef HAVE_RAND

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#undef HAVE_STDLIB_H

/* Define to 1 if you have the `stpcpy' function. */
#undef HAVE_STPCPY

/* Define to 1 if you have the `strcasecmp' function. */
#undef HAVE_STRCASECMP

/* Define to 1 if you have the `strerror' function. */
#undef HAVE_STRERROR

/* Define to 1 if you have the `stricmp' function. */
#undef HAVE_STRICMP

/* Define to 1 if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#undef HAVE_STRING_H

/* Define to 1 if you have the `strtoul' function. */
#undef HAVE_STRTOUL

/* Define to 1 if you have the `sysconf' function. */
#undef HAVE_SYSCONF

/* Define to 1 if you have the `syslog' function. */
#undef HAVE_SYSLOG

/* Define to 1 if you have the <sys/capability.h> header file. */
#undef HAVE_SYS_CAPABILITY_H

/* Define to 1 if you have the <sys/mman.h> header file. */
#undef HAVE_SYS_MMAN_H

/* Define to 1 if you have the <sys/select.h> header file. */
#undef HAVE_SYS_SELECT_H

/* Define to 1 if you have the <sys/socket.h> header file. */
#undef HAVE_SYS_SOCKET_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#undef HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/types.h> header file. */
#undef HAVE_SYS_TYPES_H

/* Defined if a `u16' is typedef'd */
#undef HAVE_U16_TYPEDEF

/* Defined if a `u32' is typedef'd */
#undef HAVE_U32_TYPEDEF

/* Define to 1 if the system has the type `uintptr_t'. */
#undef HAVE_UINTPTR_T

/* Defined if a `ulong' is typedef'd */
#undef HAVE_ULONG_TYPEDEF

/* Define to 1 if you have the <unistd.h> header file. */
#undef HAVE_UNISTD_H

/* Defined if a `ushort' is typedef'd */
#undef HAVE_USHORT_TYPEDEF

/* Define to 1 if you have the `vprintf' function. */
#undef HAVE_VPRINTF

/* Defined if we run on a W32 API based system */
#undef HAVE_W32_SYSTEM

/* Define to 1 if you have the `wait4' function. */
#undef HAVE_WAIT4

/* Define to 1 if you have the `waitpid' function. */
#undef HAVE_WAITPID

/* Define to 1 if you have the <winsock2.h> header file. */
#undef HAVE_WINSOCK2_H

/* Define to 1 if you have the <ws2tcpip.h> header file. */
#undef HAVE_WS2TCPIP_H

/* Defined if this is not a regular release */
#undef IS_DEVELOPMENT_VERSION

/* List of available cipher algorithms */
#undef LIBGCRYPT_CIPHERS

/* List of available digest algorithms */
#undef LIBGCRYPT_DIGESTS

/* List of available public key cipher algorithms */
#undef LIBGCRYPT_PUBKEY_CIPHERS

/* Define to use the (obsolete) malloc guarding feature */
#undef M_GUARD

/* defined to the name of the strong random device */
#undef NAME_OF_DEV_RANDOM

/* defined to the name of the weaker random device */
#undef NAME_OF_DEV_URANDOM

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
#undef NO_MINUS_C_MINUS_O

/* Name of this package */
#undef PACKAGE

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the home page for this package. */
#undef PACKAGE_URL

/* Define to the version of this package. */
#undef PACKAGE_VERSION

/* A human readable text with the name of the OS */
#undef PRINTABLE_OS_NAME

/* Define as the return type of signal handlers (`int' or `void'). */
#undef RETSIGTYPE

/* The size of `unsigned int', as computed by sizeof. */
#undef SIZEOF_UNSIGNED_INT

/* The size of `unsigned long', as computed by sizeof. */
#undef SIZEOF_UNSIGNED_LONG

/* The size of `unsigned long long', as computed by sizeof. */
#undef SIZEOF_UNSIGNED_LONG_LONG

/* The size of `unsigned short', as computed by sizeof. */
#undef SIZEOF_UNSIGNED_SHORT

/* Define to 1 if you have the ANSI C header files. */
#undef STDC_HEADERS

/* Defined if this module should be included */
#undef USE_AES

/* Defined if this module should be included */
#undef USE_ARCFOUR

/* Defined if this module should be included */
#undef USE_BLOWFISH

/* Defined if this module should be included */
#undef USE_CAMELLIA

/* define if capabilities should be used */
#undef USE_CAPABILITIES

/* Defined if this module should be included */
#undef USE_CAST5

/* Defined if this module should be included */
#undef USE_CRC

/* Defined if this module should be included */
#undef USE_DES

/* Defined if this module should be included */
#undef USE_DSA

/* Defined if this module should be included */
#undef USE_ECC

/* Defined if this module should be included */
#undef USE_ELGAMAL

/* Defined if the GNU Portable Thread Library should be used */
#undef USE_GNU_PTH

/* Defined if this module should be included */
#undef USE_MD4

/* Defined if this module should be included */
#undef USE_MD5

/* set this to limit filenames to the 8.3 format */
#undef USE_ONLY_8DOT3

/* Define to support the experimental random daemon */
#undef USE_RANDOM_DAEMON

/* Defined if this module should be included */
#undef USE_RFC2268

/* Defined if this module should be included */
#undef USE_RMD160

/* Defined if the EGD based RNG should be used. */
#undef USE_RNDEGD

/* Defined if the /dev/random RNG should be used. */
#undef USE_RNDLINUX

/* Defined if the default Unix RNG should be used. */
#undef USE_RNDUNIX

/* Defined if the Windows specific RNG should be used. */
#undef USE_RNDW32

/* Defined if this module should be included */
#undef USE_RSA

/* Defined if this module should be included */
#undef USE_SEED

/* Defined if this module should be included */
#undef USE_SERPENT

/* Defined if this module should be included */
#undef USE_SHA1

/* Defined if this module should be included */
#undef USE_SHA256

/* Defined if this module should be included */
#undef USE_SHA512

/* Defined if this module should be included */
#undef USE_TIGER

/* Defined if this module should be included */
#undef USE_TWOFISH

/* Defined if this module should be included */
#undef USE_WHIRLPOOL

/* Version of this package */
#undef VERSION

/* Defined if compiled symbols have a leading underscore */
#undef WITH_SYMBOL_UNDERSCORE

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
#  undef WORDS_BIGENDIAN
# endif
#endif

/* To allow the use of Libgcrypt in multithreaded programs we have to use
    special features from the library. */
#ifndef _REENTRANT
# define _REENTRANT 1
#endif


/* Define to empty if `const' does not conform to ANSI C. */
#undef const

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#undef inline
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
#undef size_t

/* type to use in place of socklen_t if not defined */
#undef socklen_t

/* Define to the type of an unsigned integer type wide enough to hold a
   pointer, if such a type exists, and if the system does not define it. */
#undef uintptr_t


#define _GCRYPT_IN_LIBGCRYPT 1

/* If the configure check for endianness has been disabled, get it from
   OS macros.  This is intended for making fat binary builds on OS X.  */
#ifdef DISABLED_ENDIAN_CHECK
# if defined(__BIG_ENDIAN__)
#  define WORDS_BIGENDIAN 1
# elif defined(__LITTLE_ENDIAN__)
#  undef WORDS_BIGENDIAN
# else
#  error "No endianness found"
# endif
#endif /*DISABLED_ENDIAN_CHECK*/

/* We basically use the original Camellia source.  Make sure the symbols 
   properly prefixed.  */
#define CAMELLIA_EXT_SYM_PREFIX _gcry_

/* This error code is only available with gpg-error 1.7.  Thus
   we define it here with the usual gcry prefix.  */
#define GCRY_GPG_ERR_NOT_OPERATIONAL  176


#endif /*_GCRYPT_CONFIG_H_INCLUDED*/


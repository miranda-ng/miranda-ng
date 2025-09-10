/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Defined if libgadu was compiled for bigendian machine. */
/* #undef GG_CONFIG_BIGENDIAN */

/* use the given file as GnuTLS default trust store */
#define GG_CONFIG_GNUTLS_SYSTEM_TRUST_STORE "/etc/ssl/certs/ca-certificates.crt"

/* Defined if this machine has C99-compiliant vsnprintf(). */
#define GG_CONFIG_HAVE_C99_VSNPRINTF /**/

/* Defined if this machine has fork(). */
/* #undef GG_CONFIG_HAVE_FORK */

/* Defined if this machine has gethostbyname_r(). */
/* #undef GG_CONFIG_HAVE_GETHOSTBYNAME_R */

/* Defined if uintX_t types are defined in <inttypes.h>. */
#define GG_CONFIG_HAVE_INTTYPES_H /**/

/* Defined if this machine supports long long. */
#define GG_CONFIG_HAVE_LONG_LONG /**/

/* Defined if libgadu was compiled and linked with OpenSSL support. */
#define GG_CONFIG_HAVE_OPENSSL

/* Defined if libgadu was compiled and linked with pthread support. */
#define GG_CONFIG_HAVE_PTHREAD /**/

/* Defined if uintX_t types are defined in <stdint.h>. */
/* #undef GG_CONFIG_HAVE_STDINT_H */

/* Defined if uintX_t types are defined in <sys/inttypes.h>. */
/* #undef GG_CONFIG_HAVE_SYS_INTTYPES_H */

/* Defined if uintX_t types are defined in <sys/int_types.h>. */
/* #undef GG_CONFIG_HAVE_SYS_INT_TYPES_H */

/* Defined if uintX_t types are defined in <sys/types.h>. */
/* #undef GG_CONFIG_HAVE_SYS_TYPES_H */

/* Defined if OpenSSL features TLSv1_2_method_client() function. */
#define GG_CONFIG_HAVE_TLSV1_2_CLIENT_METHOD

/* Defined if OpenSSL features TLS_method_client() function. */
#define GG_CONFIG_HAVE_TLS_CLIENT_METHOD

/* Defined if this machine has uint64_t. */
#define GG_CONFIG_HAVE_UINT64_T /**/

/* Defined if this machine has va_copy(). */
#define GG_CONFIG_HAVE_VA_COPY /**/

/* Defined if libgadu was compiled and linked with zlib support. */
#define GG_CONFIG_HAVE_ZLIB /**/

/* Defined if this machine has __va_copy(). */
#define GG_CONFIG_HAVE___VA_COPY /**/

/* Defined if libgadu is GPL compliant (was not linked with OpenSSL or any
   other non-GPL compliant library support). */
#define GG_CONFIG_IS_GPL_COMPLIANT /**/

/* Defined if libgadu uses system defalt trusted CAs */
#define GG_CONFIG_SSL_SYSTEM_TRUST /**/

/* Library version */
#define GG_LIBGADU_VERSION "1.12.3"

/* Define to 1 if you have the <curl/curl.h> header file. */
#define HAVE_CURL_CURL_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <expat.h> header file. */
#define HAVE_EXPAT_H 1

/* Define to 1 if you have the 'fork' function. */
#define HAVE_FORK 1

/* Define to 1 if you have the 'gethostbyname_r' function. */
#define HAVE_GETHOSTBYNAME_R 1

/* Define to 1 if you have the 'gnutls_certificate_set_x509_system_trust'
   function. */
#define HAVE_GNUTLS_CERTIFICATE_SET_X509_SYSTEM_TRUST 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Defined if libxml2 is available. */
#define HAVE_LIBXML2 /**/

/* Define to 1 if you have the 'mkstemp' function. */
#define HAVE_MKSTEMP 1

/* define if you have OpenSSL */
/* #undef HAVE_OPENSSL */

/* Define if you have POSIX threads libraries and header files. */
/* #undef HAVE_PTHREAD */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the 'strtoull' function. */
#define HAVE_STRTOULL 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* define if you have zlib */
#define HAVE_ZLIB 1

/* Define to 1 if you have the '_exit' function. */
#define HAVE__EXIT 1

/* Define to 1 if you have the '_strtoui64' function. */
/* #undef HAVE__STRTOUI64 */

/* Define to 1 if you have the '_vscprintf' function. */
/* #undef HAVE__VSCPRINTF */

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "libgadu"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "libgadu"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libgadu 1.12.3"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libgadu"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.12.3"

/* Define to the necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* Define to 1 if all of the C89 standard headers exist (not just the ones
   required in a freestanding environment). This macro is provided for
   backward compatibility; new code need not use it. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "1.12.3-pre"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define to empty if 'const' does not conform to ANSI C. */
/* #undef const */

/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the <inttypes.h> header file. */
/* #undef HAVE_INTTYPES_H */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H */

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
/* #undef HAVE_UNISTD_H */

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "libotr"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "otr@cypherpunks.ca"

/* Define to the full name of this package. */
#define PACKAGE_NAME "libotr"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libotr 4.1.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libotr"

/* Define to the home page for this package. */
#define PACKAGE_URL "https://otr.cypherpunks.ca"

/* Define to the version of this package. */
#define PACKAGE_VERSION "4.1.0"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "4.1.0"

/* Miranda NG modifications */
#define GPGRT_ENABLE_ES_MACROS 1
#include <string.h>
#define strdup _strdup
#define stricmp _stricmp
#define snprintf _snprintf
#ifdef _WIN64
#	define __x86_64__ 1
#else
#	define __i386__ 1
#endif

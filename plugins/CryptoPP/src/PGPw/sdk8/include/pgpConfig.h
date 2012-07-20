/*
 * pgpConfig.h -- Configuration for the PGPsdk.  This file contains
 * the configuration information for the PGPsdk, and it should be
 * included in all PGPsdk source files.
 *
 * $Id: pgpConfig.h,v 1.1.1.1 1999/08/08 19:38:33 heller Exp $
 */

/* Define to empty if the compiler does not support 'const' variables. */
/* #undef const */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */


#ifndef Included_pgpConfig_h	/* [ */
#define Included_pgpConfig_h

#include "pgpPFLConfig.h"





#ifndef Included_pgpPFLConfig_h	/* [ */

#define HAVE_STDARG_H	1
#define HAVE_STDLIB_H	1
#define HAVE_UNISTD_H	0
#define HAVE_USHORT		0
#define HAVE_UINT		0
#define HAVE_ULONG		0
#define NO_LIMITS_H		0
#define NO_POPEN		1

#if defined( _MSC_VER )
#define PGP_HAVE64		1
typedef __int64				PGPInt64;
typedef unsigned __int64	PGPUInt64;

#elif  defined( __MWERKS__ )

#define PGP_HAVE64		0
	
#endif



#endif /*Included_pgpPFLConfig_h*/	/* ] */


/* Checks for various types */
#define HAVE_UCHAR 0

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Checks for various specific header files */
#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
#define HAVE_SYS_IOCTL_H 0
#define HAVE_SYS_TIME_H 0
#define HAVE_SYS_TIMEB_H 1
#define HAVE_SYS_PARAM_H 0

/* Check if <sys/time.h> is broken and #includes <time.h> wrong */
#define TIME_WITH_SYS_TIME 0

/* Checks for various functions */
#define HAVE_GETHRTIME 0
#define HAVE_CLOCK_GETTIME 0
#define HAVE_CLOCK_GETRES 0
#define HAVE_GETTIMEOFDAY 0
#define HAVE_GETITIMER 0
#define HAVE_SETITIMER 0
#define HAVE_FTIME 1
#define HAVE_MKSTEMP 0


#if  defined( __MWERKS__ )

#define PGPTTYE	 /* nothing */
	
#elif defined( _MSC_VER )

/* Tags for exported functions, needed for dynamic linking on some platforms */
#define PGPTTYE	 /* nothing */

#endif



#endif /* ] Included_pgpConfig_h */

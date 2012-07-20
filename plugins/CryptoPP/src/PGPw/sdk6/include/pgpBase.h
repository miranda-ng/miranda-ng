/*____________________________________________________________________________
	pgpBase.h
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	This file deals with system dependencies to derive our very basic data
	types.  It should not contain any higher level types.

	$Id: pgpBase.h,v 1.27.16.1 1999/08/04 18:35:43 sluu Exp $
____________________________________________________________________________*/
#ifndef Included_pgpBase_h	/* [ */
#define Included_pgpBase_h

#include "pgpPFLConfig.h"


#include <stddef.h>



#if !( defined(PGP_MACINTOSH) || defined(PGP_UNIX) || defined(PGP_WIN32) )
#error one of {PGP_MACINTOSH, PGP_UNIX, PGP_WIN32} must be defined
#endif

#if PGP_MACINTOSH
#include <stddef.h>
#if __MWERKS__ && ! defined( __dest_os )
	#include <ansi_parms.h>
	#define __dest_os __mac_os
#endif
#else
	/* aCC bars on <sys/time.h> if this file is not included first */
	#if PGP_COMPILER_HPUX
		#include <sys/sigevent.h>
	#endif /* PGP_COMPILER_HPUX */
	#include <sys/types.h>
#endif

#if PGP_WIN32
#include <stddef.h>		/* For size_t */
#endif

#if ! NO_LIMITS_H
#include <limits.h>
#endif


/*____________________________________________________________________________
	PGP basic types
____________________________________________________________________________*/

typedef unsigned char	PGPBoolean;	/* can be TRUE or FALSE */
#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif


/* PGPUInt8, PGPInt8 */
#if UCHAR_MAX == 0xff

typedef unsigned char	PGPUInt8;
typedef signed char		PGPInt8;
#define MAX_PGPUInt8	UCHAR_MAX
#define MAX_PGPInt8		SCHAR_MAX

#else
#error This machine has no 8-bit type
#endif


/* PGPUInt16, PGPInt16 */
#if UINT_MAX == 0xffff

typedef unsigned int	PGPUInt16;
typedef int				PGPInt16;
#define MAX_PGPUInt16	UINT_MAX
#define MAX_PGPInt16	INT_MAX

#elif USHRT_MAX == 0xffff

typedef unsigned short	PGPUInt16;
typedef short			PGPInt16;
#define MAX_PGPUInt16	USHRT_MAX
#define MAX_PGPInt16	SHRT_MAX

#else
#error This machine has no 16-bit type
#endif


/* PGPUInt32, PGPInt32 */
#if UINT_MAX == 0xfffffffful

typedef unsigned int	PGPUInt32;
typedef int				PGPInt32;
#define MAX_PGPUInt32	UINT_MAX
#define MAX_PGPInt32	INT_MAX

#elif ULONG_MAX == 0xfffffffful

typedef unsigned long	PGPUInt32;
typedef long			PGPInt32;
#define MAX_PGPUInt32	ULONG_MAX
#define MAX_PGPInt32	LONG_MAX

#elif USHRT_MAX == 0xfffffffful

typedef unsigned short	PGPUInt32;
typedef short			PGPInt32;
#define MAX_PGPUInt32	USHRT_MAX
#define MAX_PGPInt32	SHRT_MAX

#else
#error This machine has no 32-bit type
#endif


/*____________________________________________________________________________
	PGPUInt64, PGPInt64
	
	Find a 64-bit data type, if possible.
	The conditions here are more complicated to avoid using numbers that
	will choke lesser preprocessors (like 0xffffffffffffffff) unless
	we're reasonably certain that they'll be acceptable.
 
	Some *preprocessors* choke on constants that long even if the
	compiler can accept them, so it doesn't work reliably to test values.
	So cross our fingers and hope that it's a 64-bit type.
	
	GCC uses ULONG_LONG_MAX.  Solaris uses ULLONG_MAX.
	IRIX uses ULONGLONG_MAX.  Are there any other names for this?
____________________________________________________________________________*/
#if ULONG_MAX > 0xfffffffful
#if ULONG_MAX == 0xfffffffffffffffful

typedef ulong		PGPUInt64;
typedef long		PGPInt64;
#define PGP_HAVE64	1

#endif
#endif


#ifndef PGP_HAVE64

#if defined(ULONG_LONG_MAX) || defined (ULLONG_MAX) || defined(ULONGLONG_MAX)
typedef unsigned long long	PGPUInt64;
typedef long long			PGPInt64;
#define PGP_HAVE64			1

#endif
#endif


#ifndef PGP_HAVE64
#if defined(__MWERKS__)
#if __option( longlong )

typedef unsigned long long	PGPUInt64;
typedef long long			PGPInt64;
#define PGP_HAVE64			1

#endif
#endif
#endif

#if PGP_HAVE64
/* too painful to test all the variants above, so just do it this way */
#define MAX_PGPUInt64	((PGPUInt64)0xfffffffffffffffful)
#define MAX_PGPInt64	((PGPInt64)0x7fffffffffffffff)
#endif


#if INT_MAX == 0x7FFFFFFFL
#define PGPENUM_TYPEDEF( enumName, typeName )	typedef enum enumName typeName
#else
#define PGPENUM_TYPEDEF( enumName, typeName )	typedef PGPInt32 typeName
#endif
#define kPGPEnumMaxValue		INT_MAX

#define PGP_ENUM_FORCE( enumName )		\
		k ## enumName ## force = kPGPEnumMaxValue


typedef PGPUInt8			PGPByte;

typedef PGPInt32			PGPError;

/* a simple value sufficient to hold any numeric or pointer type */
typedef void *				PGPUserValue;

/* A PGPSize refers to in memory sizes. Use PGPFileOffset for file offsets */
#if PGP_UNIX_HPUX
/* HPUX has conflicting types for size_t.  This forces a PGPSize to 
 * always be the same type. */
typedef unsigned long       PGPSize;
#else
typedef size_t				PGPSize;
#endif

#define MAX_PGPSize			( ~(PGPSize)0 )

/* An offset or size of a file */
#if PGP_UNIX
typedef off_t				PGPFileOffset;
#else
#if PGP_HAVE64
typedef PGPInt64			PGPFileOffset;
#else
typedef PGPInt32			PGPFileOffset;
#endif
#endif

typedef PGPUInt32			PGPFlags;
typedef PGPUInt32			PGPTime;
typedef PGPUInt32			PGPTimeInterval;	/* In milliseconds */

typedef struct PGPVersion
{
	PGPUInt16	majorVersion;
	PGPUInt16	minorVersion;
	
} PGPVersion;

/*____________________________________________________________________________
	These macros should surround all C declarations in public
	header files which define function or data symbols.
____________________________________________________________________________*/

#ifdef __cplusplus	/* [ */

#define PGP_BEGIN_C_DECLARATIONS	extern "C" {
#define PGP_END_C_DECLARATIONS		}

#else	/* ] __cplusplus [ */

#define PGP_BEGIN_C_DECLARATIONS
#define PGP_END_C_DECLARATIONS

#endif	/* ] __cplusplus */




#ifndef pgpMin
#define pgpMin(x,y) (((x)<(y)) ? (x) : (y))
#endif

#ifndef pgpMax
#define pgpMax(x,y) (((x)>(y)) ? (x) : (y))
#endif

#ifndef PGP_DEPRECATED
#define PGP_DEPRECATED	1
#endif

#endif /* ] Included_pgpBase_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

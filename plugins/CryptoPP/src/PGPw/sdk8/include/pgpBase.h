/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	This file deals with system dependencies to derive our very basic data
	types.  It should not contain any higher level types.


	$Id: pgpBase.h,v 1.30 2004/02/09 23:30:31 vinnie Exp $
____________________________________________________________________________*/

#ifndef Included_pgpBase_h	/* [ */
#define Included_pgpBase_h

#include "pgpPFLConfig.h"

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

#ifndef PGP_UNICODE
#define PGP_UNICODE		0
#endif

#if PGP_WIN32
	/* check for inconsistent usage of UNICODE symbols */
	#if PGP_UNICODE
		#if !defined(UNICODE) || !defined(_UNICODE)
			#error UNICODE and _UNICODE must be defined
		#endif
	#else
		#if defined(UNICODE) || defined(_UNICODE)
			#error UNICODE and _UNICODE should not be defined
		#endif
	#endif
#endif


/*____________________________________________________________________________
	PGP basic types
____________________________________________________________________________*/

typedef unsigned char	PGPBoolean;		/* can be TRUE or FALSE */

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

#if defined(ULONG_LONG_MAX) || defined (ULLONG_MAX) || defined(ULONGLONG_MAX) || defined(__LONG_LONG_MAX__)
typedef unsigned long long	PGPUInt64;
typedef long long			PGPInt64;
#define PGP_HAVE64			1

#endif
#endif

/*____________________________________________________________________________
	This was added because for some reason or another, __LONG_LONG_MAX__ is 
	not defined on Linux 6.1.  Hopefully this doesn't break older versions of
	Linux but you never know.....
____________________________________________________________________________*/
#if defined(PGP_UNIX_LINUX) && !defined(PGP_HAVE64)
typedef long long			PGPInt64;
typedef unsigned long long	PGPUInt64;
#define PGP_HAVE64			1
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


/*____________________________________________________________________________
	PGPInt, PGPUInt, PGPLong, PGPULong
	
	Architecture-specific data types. 

	PGPLong will be 64 bits on 64-bit architectures (Linux on AMD64 or WIN64). 
	PGPLong is hardware-supported integeral type such that 
	sizeof(PGPULong)==sizeof(void*)

	PGPInt is the most efficient harware-supported integeral type that is at 
	least 32 bit long. It should be the main type for internal counters, sizes, 
	offsets etc unless possibly the larger space of PGPSize is required.
____________________________________________________________________________*/

/* note that this should be changed for 16 bit platform */
typedef int				PGPInt;
typedef unsigned int	PGPUInt;

#if PGP_WIN32
	#if defined(_WIN64)
	#if PGP_HAVE_64_HARDWARE==0
	#error "Configuration error: PGP_HAVE_64_HARDWARE must be 1 for WIN64"
	#endif
	typedef __int64			PGPLong;
	typedef unsigned __int64 PGPULong;
	#else
	#ifndef _W64
	/* (this definition is only need  for WINNT) */
	#define _W64
	#endif
	typedef _W64 __int32	PGPLong;
	typedef _W64 unsigned __int32 PGPULong;
	#endif
#else
	typedef unsigned long	PGPULong;
	typedef long			PGPLong;
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
typedef size_t				PGPSize;
#define MAX_PGPSize			( ~(PGPSize)0 )

/* An offset or size of a file */
#if PGP_UNIX
#ifdef HAVE_64BIT_FILES
typedef off64_t				PGPFileOffset;
#else /* !HAVE_64BIT_FILES	*/
typedef off_t				PGPFileOffset;
#endif /* HAVE_64BIT_FILES	*/
#else
#if PGP_HAVE64
typedef PGPInt64			PGPFileOffset;
#else
typedef PGPInt32			PGPFileOffset;
#endif
#endif

typedef PGPUInt32			PGPFlags;
typedef time_t				PGPTime;
typedef PGPULong			PGPTimeInterval;	/* In milliseconds */

#define kPGPMaxTimeInterval (~(PGPTimeInterval)0)

typedef struct PGPVersion
{
	PGPUInt16	majorVersion;
	PGPUInt16	minorVersion;
	
} PGPVersion;

/* character types useful for Unicode issues */
typedef	char				PGPChar8;
typedef	PGPUInt16			PGPChar16;
typedef	PGPUInt32			PGPChar32;
typedef	unsigned char		PGPUTF8;

#if PGP_UNICODE
typedef	PGPUInt16			PGPChar;
#else
typedef	char				PGPChar;
#endif

typedef char				PFLChar;
typedef char				SDKUIChar;


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
#define PGP_DEPRECATED		1
#endif

#if PGP_WIN32
# define BIG_ENDIAN		123
# define LITTLE_ENDIAN	321
# define BYTE_ORDER		LITTLE_ENDIAN
#endif

#if BYTE_ORDER == BIG_ENDIAN
#	define PGP_WORDSBIGENDIAN		1
#	define PGP_WORDSLITTLEENDIAN	0
#elif BYTE_ORDER == LITTLE_ENDIAN
#	define PGP_WORDSBIGENDIAN		0
#	define PGP_WORDSLITTLEENDIAN	1
#else
#	error define your byte order
#endif

/*____________________________________________________________________________
 * The PGP equivalent of the MS "TEXT" macro.  PGPTEXT wraps a string literal
 * and causes it to compile as 8 or 16 bit characters on the basis of the
 * PGP_UNICODE symbol. 
 */
#if PGP_UNICODE
  #define PGPTEXT(literal)				L##literal
#else
  #define PGPTEXT(literal)				literal
#endif

/*____________________________________________________________________________
 * Macros for wrapping text literals.  These macros serve two purposes:
 * (a) to indicate to the reader of the source code the way in which the
 * literal is used (and therefore why the string should not be externalized
 * and localized), and (b) to indicate to the compiler whether the literal 
 * should be compiled as 8-bit or 16-bit characters.
 *
 * To the right of each macro is the abbreviation to use when naming 
 * string resources.
 */
 
/* PGPTXT_USER should be used for strings which are to be displayed
 * to the user, but which we have decided not to translate, for whatever 
 * reason.
 */
#define PGPTXT_USER(literal)			PGPTEXT(literal)	/* USR */
#define PGPTXT_USER8(literal)			literal
#define PGPTXT_USER16(literal)			L##literal

/* PGPTXT_ADMIN is for messages to be seen by an admin; we may choose to 
 * translate these in the future.
 */
#define PGPTXT_ADMIN(literal)			PGPTEXT(literal)	/* ADM */

/* PGPTXT_MACHINE strings are meant to be read by a machine.  That is, 
 * the usual usage would be that this string is never seen by anyone, 
 * neither users, developers, admins nor qa; it is only seen by programs.  
 * This includes textual material in tables where that is meant to be 
 * compared against hardcoded strings looking for a match.  Explicit
 * 8- and 16-bit versions are provided.
 */
#define PGPTXT_MACHINE(literal)			PGPTEXT(literal)	/* MAC */
#define PGPTXT_MACHINE8(literal)		literal
#define PGPTXT_MACHINE16(literal)		L##literal

#define PFLTXT_MACHINE					PGPTXT_MACHINE8
#define SDKUITXT_MACHINE	 			PGPTXT_DEBUG8
#define SDKUITXT_USER					PGPTXT_MACHINE8

/* String literals in obsolete sections of code may be left in for
 * clarity or historical reasons.  They should be marked with the
 * PGPTXT_OBSOLETE macro.
 */
#define PGPTXT_OBSOLETE(literal)		literal				/* OBS */

/* PGPTXT_FIXBEFORESHIP is for strings for which the tagger is not sure 
 * what to do with them, but which will need to be decided eventually. 
 */
#define PGPTXT_FIXBEFORESHIP(literal)	literal				/* FIX */

/* PGPTXT_DEBUG should be used for strings which are to be seen only by
 * developers or testers.  This would include compiled-out self-test
 * code, debugging code, printf's, messageboxes, debug logs, and asserts.
 */
#define PGPTXT_DEBUG(literal)			PGPTEXT(literal)	/* DBG */
#define PGPTXT_DEBUG8(literal)			literal
#define PGPTXT_DEBUG16(literal)			L##literal

#define PFLTXT_DEBUG 					PGPTXT_DEBUG8
#define SDKUITXT_DEBUG	 				PGPTXT_DEBUG8

/* PGPTXT_DEFERRED is used to mark text for which externalization
 * has been deferred because the text is not actually used in the
 * current implementation but it may be someday.  Externalizing
 * such text would create unnecessary work for the localizers at
 * this point in time.  
 */
#define PGPTXT_DEFERRED(literal)		PGPTEXT(literal)
#define PGPTXT_DEFERRED8(literal)		literal
#define PGPTXT_DEFERRED16(literal)		L##literal

#if defined(__GNUC__) && (__GNUC__ >= 3) && (__GNUC_MINOR__ >= 1) && PGP_OSX
#define PGP_WEAK_IMPORT  __attribute__((weak_import))
#else
#define PGP_WEAK_IMPORT 
#endif

#endif /* ] Included_pgpBase_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

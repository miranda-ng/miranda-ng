
#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(P128)
#endif

/*********************************************************************/
/*                                                                   */
/* Licensed Materials - Property of IBM                              */
/*                                                                   */
/* L-GHUS-5VMPGW, L-GHUS-5S3PEE                                      */
/* (C) Copyright IBM Corp. 1989, 2005  All Rights Reserved           */
/*                                                                   */
/* US Government Users Restricted Rights - Use, duplication or       */
/* disclosure restricted by GSA ADP Schedule Contract with           */
/* IBM Corp.                                                         */
/*                                                                   */
/*********************************************************************/



/*	Global Definitions */

#ifndef GLOBAL_DEFS
#define GLOBAL_DEFS

/*	Define the specific operating system products we run on:
*	Currently defined as:
*
*		- OS2_1x = IBM OS/2 16-bit V1.x (NOW DESUPPORTED)
*		- OS2_2x = IBM OS/2 32-bit V2.x
*		- NT =  Microsoft NT operating system.
*		- DOSW16 = Microsoft (DOS-based) Windows/16 V3.x (and NOT UNIX-based Win16)
*		- MAC_68K = Apple Macintosh on Motorola 680x0 processors
*		- MAC_POWERPC = Apple Macintosh on PowerMac 60x processors
*		- SUN = Sun OS (Solaris 1.0) Unix
*		- SOLARIS = Sun Solaris 2.0 Unix
*		- SOLX86 = Sun Solaris 2.x Unix on Intel processors
*		- OS400 = i5/OS for IBM iSeries
*		- AIX = IBM AIX Unix
*		- OS390 = IBM OS/390
*		- HPUX = HP-UX
*		- ODT = SCO OpenDeskTop
*		- UNIXWARE = Novell UnixWare
*		- OSF = Digital Unix
*		- NLM = Novell Netware Loadable Module
*
*	Define the operating system API (may be used on multiple operating systems)
*
*		- W =   Microsoft Windows API
*		- W16 = Microsoft 16-bit Windows API
*		- W32 = Microsoft 32-bit Windows API
*		- MIRAGE = Uses UNIX Mirage product which emulates Windows/16 API
*		- UNIX = UNIX API (common to all UNIX platforms)
*		- OS2 = OS/2 Base Kernel Services API (either V1.x or V2.x)
*		- PM =	OS/2 Presentation Manager API (either V1.x or V2.x)
*		- DOS = DOS INT21 API
*		- MAC = Macintosh Toolbox API
*
*	Define OS architecture
*
*		- "PREEMPTIVE" - OS supports preemptive multitasking
*		- "SEMAPHORES" - OS supports semaphores protecting critical regions,
*						 as opposed to simply using Yields as critical
*						 region protection mechanism
*		- "SEGMENTATION" - Memory addresses stay same across reallocations
*		- "THREADS" - OS supported "threads" of execution that implicitly
*						share DS, allocated data and code.
*		- "SINGLE_INSTANCE_DLL_DS" - DS in DLLs is shared system-wide
*						(as opposed to each process having its own copy)
*		- "LOCAL" - Addressing of "Local" memory (NEAR or FAR)
*		- "NOGUI" - OS has no graphical interface (e.g., NLM or VMS)
*		- "ENABLE_TRANSACTIONS"	- OS supports Transaction Tracking
*		- "UNICODE" - OS API requires UNICODE strings, not single-byte strings
*
*	Define the ways our executables are linked together:
*
*		- "LINKED_WITH_STATIC_DRIVERS"
*					The network,database,ix drivers are statically linked
*					into the base executables (either single executable or
*					single shared library), and are NOT dynamically loaded
*					into memory when configured by the user.
*					If not defined, then the drivers are dynamically loaded
*					into memory when needed via OSLoadLibrary.
*					This symbol is tested to determine whether to load
*					the library or simply call the statically linked driver
*					directly.
*		- "LINKED_UI_TOGETHER"
*					The user interface portion of the client product (nem,
*					desk, view, edit) are statically linked into a single
*					executable, rather than in separate shared library DLLs.
*					This symbol is tested if the various subprograms need
*					to share copies of variables such as "gi" rather than
*					each having their own copy of the same variable of the
*					same name (which would be necessary if each subprogram
*					is in its own DLL).
*		- "LINKED_EVERYTHING_TOGETHER"
*					The user interface portion is statically linked with
*					the back end portion into a gigantic single executable.
*					Used on platforms where import/export shared libraries
*					do not exist.
*		- "RUNTIME_LOAD_LIBRARY_SUPPORTED"
*					TRUE if OSLoadLibrary supports the ability to dynamically
*					load a library at runtime, and return its main entry point.
*					If FALSE, OSLoadLibrary will at least support connecting
*					to statically linked pseudo-libraries within our own
*					executable, but not to customer-written addin DLLs.
*
*	Define the byte ordering scheme:
*
*		- "BIG_ENDIAN_ORDER" - Bytes are ordered MSB to LSB (e.g. 68000)
*		- "LITTLE_ENDIAN_ORDER" - Bytes are ordered LSB to MSB (e.g. 8086, VAX)
*
*	Define the machine limitations:
*
*		- ODS_NEEDED = Machine cannot support byte-aligned references
*						and requires struct padding and aligned read/reads
*		- REQUIRED_ALIGNMENT = Alignment required by processor to store largest data type to avoid a bus trap (usually DWORD)
*		- NATURAL_ALIGNMENT = Natural compiler alignment of the largest data type (usually a pointer)
*		- MEMALLOC_ALIGNMENT = Memory Allocation alignment: same as NATURAL_ALIGNMENT except for OS400 with 64 bit pointers
*		- COMPILER_DWORD_ALIGNMENT = Compiler's alignment of a DWORD within a struct
*		- COMPILER_NUMBER_ALIGNMENT = Compiler's alignment of NUMBER (usually a double)
*										This is the alignment the compiler will use for the "Alignment_do_not_use"
*										field of the ALIGNED_NUMBER union.
*
*	Define the type/capabilities of the compiler we are using.
*	Currently defined as:
*
*		- "MSC" = Microsoft C
*		- "GCC" = Gnu C
*		- "SYMANTEC_C" = Symantec C/C++ running under MPW on the Mac. (No longer supported in V5)
*		- "MWERKS_C" = Metrowerks C/C++ running under MPW on the Mac.
*		- "MR_C" = Apple MrC running under MPW on the Mac. (obsolete)
*		- "PPC_C" = PPC C on the Mac or PowerMac. (obsolete)
*		- "IBM_C" = IBM Cset/2 on OS2 2.0
*		- "WATCOM_C" = WATCOM C/386 in any env, such as OS/2 2 or Netware
*		- "SUN_C" = Sun native C compiler
*		- "HP_C" = HP native C compiler
*		- "XLC_C" = IBM AIX native C compiler (obsolete)
*		- "OS390_C" = IBM MVS/ESA C/C++ compiler
*		- "UW_C" = UnixWare native C compiler
*		- "ZORTECH_CPP" = Zortech C++ compiler for OS/2
*		- "ILEC400" = OS/400 ILE C cross-compiler for AIX
*
*	Define the configuration's official name, used in the undocumented
*		@Platform function for some our templates such as NAMES.NSF.
*
*		- PLATFORM_NAME
*
*	Define some datatypes which are considered standard by all modules:
*
*			BYTE	= unsigned 8 bit integer
*			SBYTE	= signed 8 bit integer
*			WORD	= unsigned 16 bit integer
*			SWORD	= signed 16 bit integer
*			DWORD	= unsigned 32 bit integer
*			LONG	= signed 32 bit integer
*			BOOLBYTE= boolean value which occupies exactly 1 byte
*			STATUS	= Notes-specific 16-bit error code and string resource identifier
*			NUMBER	= A platform-independent IEEE-64 floating point number
*			char	= A byte which is part of a LMBCS character string
*
*			BOOL	= boolean value (used in arguments, arbitrary size)
*					(DO NOT USE IN ON-DISK STRUCTURES - MACHINE DEPENDENT SIZE)
*			FLAG	= boolean value, ONLY used in bit fields in structs (arbitrary size)
*					(DO NOT USE IN ON-DISK STRUCTURES - MACHINE DEPENDENT POSITION)
*			short	= signed integer at least 8 bits wide
*					(DO NOT USE IN ON-DISK STRUCTURES - MACHINE DEPENDENT SIZE)
*			int		= signed integer at least 16 bits wide
*					(DO NOT USE IN ON-DISK STRUCTURES - MACHINE DEPENDENT SIZE)
*			long	= signed integer at least 32 bits wide
*					(DO NOT USE IN ON-DISK STRUCTURES - MACHINE DEPENDENT SIZE)
*			NCHAR	= A platform-specific character (1 or 2 bytes depending if UNICODE is enabled)
*					(DO NOT USE IN ON-DISK STRUCTURES - MACHINE DEPENDENT SIZE)
*/

#if defined(DOSW16) || defined(DOS)
	#ifdef _MSC_VER
		#define MSC
	#endif
	#define LITTLE_ENDIAN_ORDER
	#if defined(W) || defined(W16) || defined(DOSW16)
		#ifndef W16
			#define	W16
		#endif
		#ifndef W
			#define	W
		#endif
		#ifndef DOSW16
			#define DOSW16
		#endif
		#define	SEGMENTATION
		#define	SEMAPHORES
		#define	SEMSPIN
		#define	PLATFORM_NAME "Windows/16"
	#else
		#define	PLATFORM_NAME "MS-DOS"
		#define NOGUI
	#endif
	#ifndef DOS
		#define DOS
	#endif
	#define SINGLE_INSTANCE_DLL_DS
	#define ODS_NEEDED FALSE
	#define	COMPILER_INT_SIZE 2
	#define REQUIRED_ALIGNMENT 1
	#define COMPILER_DWORD_ALIGNMENT 1
	#define COMPILER_NUMBER_ALIGNMENT 1
	#define VIM_USE_MSWIN
#elif defined(OS400)	/* This should appear before the OS2 test below, because that section tests for __IBMCPP__,
					    	which our C++ compiler also generates */
	#define PLATFORM_NAME "OS/400"
	#ifndef UNIX
		#define UNIX
	#endif
	#ifndef W32
		#define	W32				/* Compile for Windows 32 API - its emulated */
	#endif
	#ifndef W
		#define	W				/* Compile for Windows 32 API - its emulated */
	#endif
	#if !defined(MIRAGE) && !defined(INTERNOTES)
		#define MIRAGE
	#endif
	#ifndef HANDLE_IS_32BITS
		#define HANDLE_IS_32BITS
	#endif
	#define BIG_ENDIAN_ORDER
	#define PREEMPTIVE
	#define	SEMAPHORES
	#define	SEMALLOC
	#define ODS_NEEDED TRUE
	#define	COMPILER_INT_SIZE 4
  #ifdef OS400_64	/* OS400 with 64 bit pointers */
	#define REQUIRED_ALIGNMENT 8
	#define NATURAL_ALIGNMENT 8
	#define MEMALLOC_ALIGNMENT 16	/* some pointers will still be 128 bits */
  #else				/* OS400 with 128 bit pointers */
	#define REQUIRED_ALIGNMENT 16
	#define NATURAL_ALIGNMENT 16
  #endif
	#define PTRISNOT32BIT
	#define COMPILER_DWORD_ALIGNMENT 4
	#define COMPILER_NUMBER_ALIGNMENT 8
	#define VIM_USE_UNIX
	#define THREADS
	#ifndef NOGUI
		#define NOGUI
	#endif
	#if defined(__ILEC400__) || defined(__AIXxiCC__) || defined(__AIXxxlC400__)
		#define ILEC400
	#endif
#elif defined(OS2) || defined(OS2_2x) || defined(PM)
	#ifndef OS2
		#define OS2
	#endif
	#ifndef OS2_2x
		#define OS2_2x
	#endif
	#ifndef PM
		#define PM
	#endif
	#if defined(__IBMC__) || defined(__IBMCPP__)
		#define IBM_C
	#endif
	#ifdef __WATCOMC__
		#define WATCOM_C
	#endif
	#define	PLATFORM_NAME "OS/2v2"
	#define LITTLE_ENDIAN_ORDER
	#define PREEMPTIVE
	#define	SEMAPHORES
	#define	SEMALLOC
	#define THREADS
	#define ODS_NEEDED FALSE
	#define	COMPILER_INT_SIZE 4
	#define REQUIRED_ALIGNMENT 1
	#define COMPILER_DWORD_ALIGNMENT 1
	#define COMPILER_NUMBER_ALIGNMENT 1
	#define VIM_USE_OS2_32
#elif defined(OS2_1x)
	#ifndef OS2
		#define OS2
	#endif
	#ifndef OS2_1x
		#define OS2_1x
	#endif
	#ifndef PM
		#define PM
	#endif
	#define	PLATFORM_NAME "OS/2v1"
	#ifdef __ZTC__
	#define ZORTECH_CPP
	#endif
	#define MSC
	#define LITTLE_ENDIAN_ORDER
	#define PREEMPTIVE
	#define	SEMAPHORES
	#define	SEGMENTATION
	#define THREADS
	#define ODS_NEEDED FALSE
	#define	COMPILER_INT_SIZE 2
	#define REQUIRED_ALIGNMENT 1
	#define COMPILER_DWORD_ALIGNMENT 1
	#define COMPILER_NUMBER_ALIGNMENT 1
	#define VIM_USE_OS2_16
#elif defined(NT) || (defined(W32) && !defined(UNIX))
	#ifndef W32
		#define	W32
	#endif
	#ifndef W
		#define	W
	#endif
	#ifndef NT
		#define	NT
	#endif
	#define	PLATFORM_NAME "Windows/32"
	#ifdef _MSC_VER
		#define MSC
	#endif
	#if defined(__IBMC__) || defined(__IBMCPP__)
		#define IBM_C
	#endif
	#define THREADS
	#define LITTLE_ENDIAN_ORDER
	#define PREEMPTIVE
	#define	SEMAPHORES
	#define	SEMALLOC
	#ifdef _X86_
		#define ODS_NEEDED FALSE
		/* Suppress "used #pragma pack to change alignment" */
		#pragma warning(disable:4103)
		/* Note: If you change the packing, you must also change inc\globpack.h */
		#pragma pack(1)
		#define REQUIRED_ALIGNMENT 1
		#define COMPILER_DWORD_ALIGNMENT 1
		#define COMPILER_NUMBER_ALIGNMENT 1
	#else
		#define ODS_NEEDED TRUE
		#define REQUIRED_ALIGNMENT 4
		#define COMPILER_DWORD_ALIGNMENT 4
		#define COMPILER_NUMBER_ALIGNMENT 4
	#endif
	#define	COMPILER_INT_SIZE 4
	#define VIM_USE_MSWIN_NT
	#define	OLEW32
	/*	Enable some OLE definitions which are only available for NT 4.0 and
		above.  The programmer must be careful not to use them on other
		Win32 platforms.
	*/
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT	0x0400
#endif
#elif defined(MAC) || defined(applec) || defined(__PPCC__) || defined(__MWERKS__) || defined(__SC__) || defined(__MRC__)  || defined(THINK_C)
	#ifndef MAC
	#define MAC
	#endif
	/*	We have dropped support for System 6.  Turning on this switch causes
		Apple's headers to explicitly generate inline traps into the MacOS
		rather than statically linking glue code into our executable.  This
		does 2 things: A) reduces size of our app slightly, and B) makes it
		possible for Apple to fix a bug in the code (since the code resides
		in the MacOS itself rather than statically linked into our app). */
	#ifndef SystemSevenOrLater
	#define SystemSevenOrLater TRUE
	#endif
	#if defined(__MWERKS__)			/* Metrowerks MPW compiler */
		#ifdef __powerc
			#ifndef MAC_POWERPC
			#define MAC_POWERPC
			#endif
		#else
			#ifndef MAC_68K
			#define MAC_68K
			#endif
		#endif
		#ifndef MWERKS_C
		#define MWERKS_C
		#endif
	#elif defined(__PPCC__)			/* Apple "Macintosh on RISC" PPC cross-dev MPW compiler */
		#ifndef MAC_POWERPC
		#define MAC_POWERPC
		#endif
		#ifndef PPC_C
		#define PPC_C
		#endif
	#elif defined(__xlc) || defined(__xlC) /* IBM "XLC" PPC native AIX-based compiler */
		#ifndef MAC_POWERPC
		#define MAC_POWERPC
		#endif
		#ifndef XLC_C
		#define XLC_C
		#endif
	#elif defined(_MSC_VER)			/* Microsoft Visual C++ 2.0 cross-dev compiler */
		#ifndef MAC_68K
		#define MAC_68K
		#endif
		#ifndef MSC
		#define MSC
		#endif
	#elif defined(__MRC__)	/* Apple/Symantec MrC MPW compiler */
		#ifndef MAC_POWERPC
		#define MAC_POWERPC
		#endif
		#ifndef MR_C
		#define MR_C
		#endif
	#elif defined(applec)			/* 68K MPW compiler */
		#if defined(__SC__)
			#ifndef MAC_68K
			#define MAC_68K
			#endif
			#ifndef SYMANTEC_C
			#define SYMANTEC_C
			#endif
		#endif
	#elif defined(__GNUC__)			/* gcc compiler on Mac OS X */
		#ifndef MAC_POWERPC
		#define MAC_POWERPC
		#endif
	#elif !defined(THINK_C)
		#error Unknown Mac development environment
	#endif
	#define	PLATFORM_NAME "Macintosh"
	#define BIG_ENDIAN_ORDER
	#define THREADS
	#define SEMAPHORES
	#define	SEMSPIN
	#define ODS_NEEDED TRUE
	#define	COMPILER_INT_SIZE 4
	#if defined(MAC_POWERPC) && defined(XLC_C) /* All other compilers use "-align mac68k" (2 byte alignment) */
		#define REQUIRED_ALIGNMENT 1		/* PowerPC supports odd-byte addressing if truly required */
		#define COMPILER_DWORD_ALIGNMENT 4	/* Compiler will pad DWORDs to DWORD boundary if possible */
		#define COMPILER_NUMBER_ALIGNMENT 4
	#else
		#define REQUIRED_ALIGNMENT 1		/* 68020 or later CPUs only (not 68000) */
		#define COMPILER_DWORD_ALIGNMENT 2	/* Compiler will pad DWORDs to WORD boundary, because any further alignment is unnecessary */
		#define COMPILER_NUMBER_ALIGNMENT 2
	#endif
	#define VIM_USE_MAC
	/* Tdanalzye is a rudimentary parser, that is failing to correctly parse this long #if #elif statements.
	 * It is re-processing the UNIX section after it has already processed the OS400 section and incorrectly
	 * using a COMPILER_NUMBER_ALIGNMENT of 4.  So for now, add a redundant !OS400, until tdanalzye is fixed.
	 */
#elif defined(UNIX)  && !defined(OS400)
	#ifndef W32
		#define	W32				/* Compile for Windows 32 API - its emulated */
	#endif
	#ifndef W
		#define	W				/* Compile for Windows 16 API - its emulated */
	#endif
	#if !defined(MIRAGE) && !defined(INTERNOTES)
		#define MIRAGE
	#endif
	#define	PLATFORM_NAME "UNIX"
	#if defined(AIX)
		#define XLC_C
	#elif defined(SUN) || defined(SOLARIS)
		#define SUN_C
	#elif defined(HPUX)
		#define HP_C
	#elif defined(OS390)
		#define OS390_C
	#elif defined(UNIXWARE)
		#define UW_C
	#elif defined(__osf__)
		#if !defined(OSF)
			#define OSF
		#endif
		#define DEC_C
	#elif defined(LINUX)
		#define GCC
	#endif
	#if (defined(ODT) || defined(SOLX86) || defined(UNIXWARE) || defined(OSF) || defined(LINUX)) && !defined(ZLINUX)
		#define LITTLE_ENDIAN_ORDER
	#else
		#define BIG_ENDIAN_ORDER
	#endif
	#define PREEMPTIVE
	#define	SEMAPHORES
	#define	SEMALLOC
	/* All UNIX type platforms except Solaris,HPUX, and AIX are 32 bit handle platforms */
	/* This needs to be here so that it is also included in the SDK */
	#if !(defined(SOLARIS) || defined(HPUX) || defined(SOLX86) || defined(AIX))
	#ifndef HANDLE_IS_32BITS
		#define HANDLE_IS_32BITS
	#endif
	#endif
	#if (defined(SOLARIS) || defined(AIX) || defined (HPUX) || defined(OS390) || defined(LINUX)) && !defined(DISABLE_THREADS)
		#define THREADS
	#endif
	/*	ODS needed, even for ODT (on x86 machines), because X includes
		don't support having the compiler set for tight packing (-Zp1),
		that is, they assume padding.  With padding enabled, we must use
		ODS even when theoretically unncessary.  Perhaps the X headers
		can be kludged to explicitly pad so that this is unnecessary. */
	#if defined(ODT_LATER)
		#define	COMPILER_INT_SIZE 4
		#define REQUIRED_ALIGNMENT 1
		#define COMPILER_DWORD_ALIGNMENT 2
		#define COMPILER_NUMBER_ALIGNMENT 2
	#else
		#define ODS_NEEDED TRUE
		#define	COMPILER_INT_SIZE 4
		#if defined(OSF)
			#define REQUIRED_ALIGNMENT 8
		#else
			#define REQUIRED_ALIGNMENT 4
		#endif
		#define COMPILER_DWORD_ALIGNMENT 4
		#if defined(SOLARIS) || defined(HPUX) || defined(OS390) || defined(ZLINUX)
			#define COMPILER_NUMBER_ALIGNMENT 8
		#else
			#define COMPILER_NUMBER_ALIGNMENT 4
		#endif
	#endif
	#define VIM_USE_UNIX

#elif defined(NLM)
	#ifdef __WATCOMC__
		#define WATCOM_C
	#endif
	#ifdef _MSC_VER
		#define MSC
	#endif
	#define	PLATFORM_NAME "NetWare"
	#define LITTLE_ENDIAN_ORDER
	#define	SEMAPHORES
	#define	SEMALLOC
	#define	THREADS
	#define SINGLE_INSTANCE_DLL_DS
	#define ODS_NEEDED FALSE
	#define	COMPILER_INT_SIZE 4
	#define REQUIRED_ALIGNMENT 1
	#define COMPILER_DWORD_ALIGNMENT 1
	#define COMPILER_NUMBER_ALIGNMENT 1
	#define NOGUI
	#define	ENABLE_TRANSACTIONS
	#define VIM_USE_NLM
#else
	#error Must specify type of OS ("DOS", "OS2", etc) on C command line!
#endif
#if (defined(SOLARIS) || defined(AIX)) && defined(Enable_New_SpinLocks)
	#define USE_COND_SLEEP_ON_SPINLOCK 1
#endif


/*	Nullify the certain compiler keywords if not supported by compiler */

#if !defined(MSC) || defined(NT) || defined(MAC)

	#if !defined(MAC)	/* On Mac, "pascal" (lowercase) is a reserved word */
		#ifdef pascal
			#undef pascal
		#endif

		#if defined(_MSC_VER) && _MSC_VER >= 800
			#define pascal __stdcall
		#else
			#define pascal
		#endif
	#endif	/* !defined(MAC) */

	#if defined(MAC)
		#define PASCAL
	#endif

	#ifndef PASCAL
		#define PASCAL
	#endif

	#ifdef cdecl
		#undef cdecl
	#endif

	#define cdecl

	#if !defined(MWERKS_C)	/* In MWERKS_C, "far" is a reserved word, and cannot appear in an #ifdef expression */
		#ifdef far
			#undef far
		#endif
	#endif

	#define far

	#ifndef FAR
		#define FAR
	#endif

	#ifdef near
		#undef near
	#endif

	#define near

	#ifndef NEAR
		#define NEAR
	#endif

	#ifdef huge
		#undef huge
	#endif

	#define huge

#endif	/* !defined(MSC) || defined(NT) || defined(MAC) */

/*	Nullify C keywords that are not supported by some compilers */

#if defined(GCC)
#if !defined(LINUX)  /* These two are already handled with latest GCC headers on LINUX */
	#define volatile
	#define signed
	#define const
#endif
#elif defined(HP_C) && defined(__cplusplus)
	#define signed
#elif defined(SUN_C) && defined(__cplusplus)
	#define signed
#endif

/*	Remove certain definitions if the Netware header files have defined
	them previously, potentially in conflict with OUR definitions */

#ifdef NLM
	#undef BYTE
	#undef WORD
	#undef LONG
#endif




/*	Handle loading of os2def.h up-front, since everyone needs it. */

#ifdef OS2
	#ifndef APIENTRY
		#include "os2def.h"
		#include "bsedos.h"
	#endif
#endif

/*	Same goes for NT include files */

#ifdef NT
	#ifndef _WINDEF_
		#ifndef RC_INVOKED
		/*	Save packing around inclusion of Microsoft headers, just to
			protect ourselves against their failure to restore packing
			if they change it.  This may not be needed anymore... */
		#pragma pack(push, WinIncludes)
		/*	Set packing to be 4, to temporarily workaround a bug in wincon.h
			where Microsoft incorrectly assumes natural packing instead of
			setting it explicitly. */
		#pragma pack()
		#endif

		#undef PASCAL
		#undef FAR
		#undef NEAR

		/*	Windows.h defines WINVER, but we seldom include windows.h
			when using other windows headers.
		*/
		#ifndef WINVER
		#define WINVER 0x0500
		#endif

		/*	Determine the processor type as required by the following Windows
			includes */

		#if !defined(_PPC_) && !defined(_ALPHA_) && !defined(_MIPS_) && !defined(_X86_) && defined(_M_IX86)
		#define _X86_
		#endif

		#if !defined(_PPC_) && !defined(_ALPHA_) && !defined(_X86_) && !defined(_MIPS_) && defined(_M_MRX000)
		#define _MIPS_
		#endif

		#if !defined(_PPC_) && !defined(_ALPHA_) && !defined(_X86_) && !defined(_MIPS_) && defined(_M_ALPHA)
		#define _ALPHA_
		#endif

		#if !defined(_PPC_) && !defined(_ALPHA_) && !defined(_X86_) && !defined(_MIPS_) && defined(_M_PPC)
		#define _PPC_
		#endif

		#include <stdarg.h>
		#define NOMINMAX
		#include <windef.h>
		#undef NOMINMAX
		#include <winbase.h>
		#include <WinVer.h>

		#ifndef RC_INVOKED
		#pragma pack(pop, WinIncludes)
		#endif
	#endif
#endif


#if defined(MAC)
#if defined(MAC_OSX)
        #include <Carbon/Carbon.h>
#else
	#include <MacTypes.h>	/* Needed for basic "Handle", "Point", "Rect" typedefs */
	#include <MacMemory.h>	/* Needed for BlockMove() define */
#endif
#endif


/*	Determine what has been included BEFORE this module */

#if defined(_INC_WINDOWS) /* WIN16 */ || defined(_WINDEF_) /* WIN32 */
	#define WINDOWS_INCLUDED
#endif

#ifdef SEVERITY_NOERROR
	#define OS2DEF_INCLUDED
#endif



/*	Define datatypes which are also defined by other include files */

#ifndef FAR
	#define FAR far
#endif
#ifndef NEAR
	#define NEAR near
#endif
#ifndef PASCAL
	#define PASCAL pascal
#endif

#if !defined(OS2DEF_INCLUDED) && !defined(WINDOWS_INCLUDED)
	#define VOID void
	typedef unsigned char BYTE;
	#if defined(LONGIS64BIT)
		typedef int LONG;
	#else
		typedef long LONG;
	#endif

	#if defined(UNIX)
	typedef int BOOL;
	#elif defined(OS2_2x)
	typedef unsigned long BOOL;
	#elif defined(OS2_1x)
	typedef unsigned short BOOL;
	#elif defined(DOSW16)
	typedef int BOOL;
	#else
	typedef short BOOL;
	#endif
	#define LOBYTE(w)       ((BYTE)w)
	#define HIBYTE(w)       (((WORD)w >> 8) & 0xff)
#endif

#ifndef OS2DEF_INCLUDED
	#ifndef _WINDEF_   /* USHORT, ULONG and UCHAR are already defined in windef.h */
	typedef unsigned short int USHORT;
	#if defined(LONGIS64BIT)
		typedef unsigned int ULONG;
	#else
		typedef unsigned long ULONG;
	#endif
	typedef unsigned char UCHAR;
	#endif  /* _WINDEF_ */
	#ifndef WINDOWS_INCLUDED	/* This is also defined in windows.h */
		typedef unsigned int UINT;
	#endif
#endif


/*	Define datatype which is used to represent a "resource file" */

#ifndef HMODULE
#if !defined(OS2)	/* OS2 already defines this */
	#define HMODULE		HANDLE
#endif
#endif

#if !defined(WINDOWS_INCLUDED)

/*	These symbols/typedefs ARE COPIED from WINDOWS.H, and are the
	ONLY symbols that should be defined in this section!  (This
	is a help for modules that can't afford to include windows.h
	and want to use a few of these helpful definitions. */

	typedef unsigned short WORD;			/* WORD = unsigned 16 bit integer */
	#if defined(LONGIS64BIT)
		typedef unsigned int DWORD;			/* DWORD = unsigned 32 bit integer */
	#else
		typedef unsigned long DWORD;
	#endif
/*	Windows defines a HANDLE as unsigned int, but the Mac needs it to be
	unsigned short. In C, both are equivalent, but C++ is pickier */

#if defined(DOSW16)
	typedef unsigned int HANDLE;	/* really a short, but compiler is picky */
#elif defined(HANDLE_IS_32BITS)
	typedef unsigned int HANDLE;	/* 32-bit HANDLEs */
#else
	typedef unsigned short HANDLE;
#endif


	#define LOWORD(l)       ((WORD)(DWORD)(l))
	#define HIWORD(l)       ((WORD)(((DWORD)(l) >> 16) & 0xffff))

#else	/* if WINDOWS_INCLUDED */

	/*	TEMPORARY: Fix a bug in the standard Windows LOWORD macro (see
		also a copy of it above), which produces a C compiler warning
		regarding loss of segment portion of a pointer. */

	#undef LOWORD
	#define LOWORD(l)       ((WORD)(DWORD)(l))

#endif




/*	Define our Notes-specific global data types */

#define FLAG unsigned					/* FLAG = 1-bit boolean value */
										/* (Unsized, use ONLY in bitfields!) */
#define BOOLBYTE BYTE					/* BOOLBYTE = boolean used in DS */
#define UBYTE BYTE						/* UBYTE = unsigned 8 bit integer */
#define SBYTE signed char				/* SBYTE = signed 8 bit integer */
#define SWORD signed short				/* SWORD = signed 16 bit integer */

typedef DWORD MEMHANDLE;				/* MEMHANDLE = Handle returned by OSMemoryAllocate */
#define NULLMEMHANDLE (MEMHANDLE) 0

typedef WORD STATUS;					/* STATUS = Status code (ERR_xxx) */
										/* High byte is subsystem; */
										/* Low byte is status code */
typedef DWORD FONTID;					/* Font identifier - see FONTID.H */

typedef struct {
	WORD	width;
	WORD	height;
} RECTSIZE;


//commented for LotusNotify Miranga NG plugin beacouse of conflicts with miranda headers (struct LIST at m_system_cpp.h) 
///*	List structure */
//typedef struct {
//	USHORT ListEntries;			/* list entries following */
//								/* now come the list entries */
//} LIST;

/*	Range structure */

typedef struct {
	USHORT ListEntries;			/* list entries following */
	USHORT RangeEntries;		/* range entries following */
								/* now come the list entries */
								/* now come the range entries */
} RANGE;


/*	Basic time/date structure --
	The following 2 structures are ONLY intended to be interpreted using
	the "Time" package (see misc.h) conversion routines.  These structures
	are "bit-encoded", and cannot be parsed/interpreted easily. */

typedef struct tagTIMEDATE {
	DWORD Innards[2];
} TIMEDATE;

typedef struct {						/* a timedate range entry */
	TIMEDATE Lower;
	TIMEDATE Upper;
} TIMEDATE_PAIR;

#ifndef OS2_2x	/* OS2 2.x already defines this itself */
#if defined(LONGIS64BIT)
typedef long QWORD;
#else
//wsx22 change - commented typedef
//http://social.msdn.microsoft.com/Forums/en/vcmfcatl/thread/ba3ce55a-dc91-4917-8bd6-c326595b4594
//typedef struct {DWORD Dwords[2];} QWORD;
#endif
#endif

/*	Basic floating-point number structure --
	This structure is EXACTLY the same format as a 64-bit IEEE floating
	point number, usually defined in most C compilers as "double". */


typedef double NUMBER;					/* NSF floating type (IEEE 64-bit) */
typedef NUMBER ALIGNED_NUMBER;
#if defined(DOS) || defined(OS2) || defined(MAC)		/* This doesn't work for NT, which has its own FLOAT */
#define FLOAT NUMBER					/* for pre-V3 backward compatibility ONLY */
#define FLOAT_PAIR NUMBER_PAIR			/* for pre-V3 backward compatibility ONLY */
#endif	/* DOS or OS2 */


typedef struct {						/* a float range entry */
	NUMBER Lower;
	NUMBER Upper;
} NUMBER_PAIR;

typedef NUMBER_PAIR ALIGNED_NUMBER_PAIR;

/*	This is the structure that defines a license number.  Do not attempt
	to interpret this structure too exactly, since it may change in future
	releases.  For now, simply treat the entire strucure as a unique ID. */

typedef struct {
	BYTE ID[5];						/* license number */
	BYTE Product;					/* product code, mfgr-specific */
	BYTE Check[2];					/* validity check field, mfgr-specific */
} LICENSEID;

typedef WORD BLOCK;					/* pool block handle */

/*	Define symbols for boolean true/false */

#ifndef TRUE
	#define FALSE 0
	#define TRUE !FALSE
#endif


/*	Define the NULL symbol (ignoring previous ones so that it is always a simple "0"). */

#ifdef NULL
#undef NULL
#endif
#define NULL 0

/*	Define the Notes calling convention macros. */

#if !defined(OS2)

#define	LNPUBLIC 		FAR PASCAL			/*	Routines called from outside
												a subsystem (includes routines
												in the published API). */
#define LNCALLBACK		FAR PASCAL			/*	Routines specified as callbacks
												within Notes or an API program */
#if defined(OS390) && defined(__XPLINK__)
#define LNCALLBACKPTR   LNCALLBACK FAR * __callback	/*	Macro for defining a pointer to
												a callback routine. __callback added
												for XPLINK DLL to nonXPLINK DLL calls */
#else
#define LNCALLBACKPTR	LNCALLBACK FAR *	/*	Macro for defining a pointer to
												a callback routine */
#endif /* OS390 */
#define	LNVARARGS		FAR cdecl			/*	Routines with a variable number
												of arguments */
#else

/*	OS/2 requires separate macros because the ordering of function
	modifiers for function pointer is different.  This prevents us
	from inserting _System in a uniform place (e.g. a replacemet
	for PASCAL).  See the comments above for their use.*/

#define	LNPUBLIC 		_System
#define LNCALLBACK		_System
#define LNCALLBACKPTR	* _System
#define	LNVARARGS		_System
#endif

/*  Define some obsolete macros that used to be used for OS/2 32-bit
	programs that used the 16-bit Notes API.  These macros are now
	obsolete, but are here to help in backward compatibility for
	these older programs. */

#define NOTESMAIN				LNPUBLIC
#define NOTESAPI				LNPUBLIC
#define NOTESAPICDECL			LNVARARGS
#define NOTESCALLBACK			LNCALLBACK
#define NOTESCALLBACKPTR		LNCALLBACKPTR
#define NOTESPTR				FAR *
#define NOTESBOOL				BOOL

#ifdef NULLHANDLE
#undef NULLHANDLE				/* Override any existing defn (e.g. OS2 2.0) */
#endif
#define NULLHANDLE 0

/*	Define some miscellaneous constants */

#ifndef MAXDWORD
#define MAXDWORD ((DWORD) 0xffffffff)
#endif
#ifndef MAXWORD
#define MAXWORD ((WORD) 0xffff)
#endif
#ifndef MAXBYTE
#define MAXBYTE ((BYTE) 0xff)
#endif

#ifndef MAXINT
#define MAXINT ((int) (((unsigned int) -1) >> 1))
#endif

#ifndef MININT
#define MININT ((int) (~(unsigned int) MAXINT))
#endif

#ifndef BITS_PER_BYTE
#define	BITS_PER_BYTE 8
#endif


/*	Define a platform-independent method of obtaining the address of
	the "..." variable arguments in a "cdecl" "..." routine.

	void far cdecl SampleRoutine(char *String, WORD LastNamedArgument, ...)
		{
		DWORD temp1;
		WORD temp2;
		VARARG_PTR ap;

		VARARG_START(ap,LastNamedArgument); * Start out just past last named argument
		temp1 = VARARG_GET(ap,DWORD);		* Get next DWORD into temp1 and advance
		temp2 = VARARG_PEEK(ap,WORD);		* Peek at next WORD, but no advance
		temp2 = VARARG_GET(ap,WORD);		* Get next WORD into temp2 and advance
		...
		}
*/
#define VARARG_ROUND_UP(n,unit) ((((n) + (unit) - 1) / (unit)) * (unit))

#if defined(OSF) || (defined(NT) && defined(_ALPHA_)) || defined(LINUX) || defined(SOLARIS)
#include <stdarg.h>
#define VARARG_PTR	va_list
#elif defined(OS400)
#include <stdarg.h>
typedef struct
	{
	char* vaold;
	char* vanew;
	} VARARG_PTR;
#else
typedef char * VARARG_PTR;
#endif

/*
 The C++ 3.0.1 compiler on X86 has a bug. The  __builtin_va_arg_incr
 doesn't work with short data type.
 It works fine with 'cc'.  So use the definition under 'LITTLE_ENDIAN_ORDER'
*/
#if defined(SOLARIS) && !defined(SOLX86)
#define VARARG_START(_AP,_LASTNAMEDARG) \
					((_AP) = (char *) &__builtin_va_alist)
#elif defined(SUN) && defined(GCC)
#define VARARG_START(_AP,_LASTNAMEDARG) \
					(__builtin_saveregs(), \
					(_AP) = (char *) &(_LASTNAMEDARG) + sizeof(_LASTNAMEDARG))
#elif defined(HPUX)
#ifdef __GNUC__

#define VARARG_START(_AP,_LASTNAMEDARG) \
					(_AP = __builtin_saveregs())
#else
#ifdef __cplusplus
#define VARARG_START(_AP,_LASTNAMEDARG) \
					 (_AP) = (char *) &(_LASTNAMEDARG);

#else
#define VARARG_START(_AP,_LASTNAMEDARG) \
					(__builtin_va_start(_AP,&_LASTNAMEDARG), \
					(_AP) = (char *) &(_LASTNAMEDARG))
#endif
#endif
#elif defined(OSF)  || defined(LINUX) || (defined(NT) && defined(_ALPHA_))
#define VARARG_START(_AP,_LASTNAMEDARG) \
					(va_start(_AP,_LASTNAMEDARG))
#elif defined(OS400)
#define VARARG_START(_AP, _LASTNAMEDARG) \
					( (_AP).vanew = ((char *) &(_LASTNAMEDARG)) + sizeof(_LASTNAMEDARG) )

#elif defined(LITTLE_ENDIAN_ORDER) || defined(XLC_C)
#define VARARG_START(_AP,_LASTNAMEDARG) \
					((_AP) = (char *) &(_LASTNAMEDARG) + VARARG_ROUND_UP(sizeof(_LASTNAMEDARG),sizeof(int)))
#else /* BIG_ENDIAN_ORDER */
#define VARARG_START(_AP,_LASTNAMEDARG) \
					((_AP) = (char *) &(_LASTNAMEDARG) + sizeof(_LASTNAMEDARG))
#endif


#if defined(SOLARIS) && !defined(SOLX86)
#define VARARG_GET(_AP,_TYPE) \
					((_TYPE *)__builtin_va_arg_incr((_TYPE *)_AP))[0]
#elif defined(HPUX)
#define TYPE_MASK(_TYPE) (sizeof(_TYPE) > 4 ? 0xFFFFFFF8 : 0xFFFFFFFC)
#define VARARG_GET(_AP,_TYPE) \
					((_AP) = (VARARG_PTR)((long)((_AP) - sizeof(int)) \
										& TYPE_MASK(_TYPE)) , \
					 *(_TYPE *)((_AP) + ((8 - sizeof(_TYPE)) % 4)))
#elif defined(GCC3)
/* GCC3 doesn't allow variable arguments shorter than 4 bytes.  If any argument is less than
   4 bytes and is passed through the parameter list (...), it will be converted to 4 bytes */
#define VARARG_GET(_AP,_TYPE) \
					((sizeof(_TYPE) < sizeof(DWORD)) ? ((_TYPE) va_arg(_AP,DWORD)) : (va_arg(_AP,_TYPE)))

#elif defined(OSF) || defined(LINUX) || (defined(NT) && defined(_ALPHA_))
#define VARARG_GET(_AP,_TYPE) \
					(va_arg(_AP,_TYPE))
#elif defined(OS400)
#define __VARARG_GET(_AP, _TYPE) \
					( (_AP).vaold = __bndup((_AP).vanew, _TYPE),\
					  (_AP).vanew = (_AP).vaold + sizeof(_TYPE),  \
					  *(_TYPE *)((_AP).vaold ) )
/* The C++ compiler will complain about the conversion of DWORD to a struct datatype. */
/* e.g. typedef struct { WORD foo; WORD bar} FOOBAR    VARARG_GET(ap, FOOBAR);  So eliminate */
/* this unnecessary cast. */
#ifdef __cplusplus
	#define __VARARG_GET1(_AP, _TYPE) \
					( (_AP).vaold = __bndup((_AP).vanew, DWORD),\
					  (_AP).vanew = (_AP).vaold + sizeof(DWORD),  \
					  *(_TYPE *)((_AP).vaold + (sizeof(DWORD)- sizeof(_TYPE))) )
#else
	#define __VARARG_GET1(_AP, _TYPE) __VARARG_GET(_AP, DWORD)
#endif

#define VARARG_GET(_AP,_TYPE) \
					(sizeof(_TYPE) < sizeof(DWORD) ? (_TYPE) __VARARG_GET1(_AP,_TYPE) : __VARARG_GET(_AP,_TYPE))
#elif defined(LITTLE_ENDIAN_ORDER)
#define VARARG_GET(_AP,_TYPE) \
					((_AP) += VARARG_ROUND_UP(sizeof(_TYPE),sizeof(int)), \
					 *(_TYPE *)((_AP) - VARARG_ROUND_UP(sizeof(_TYPE),sizeof(int))))
#else /* BIG_ENDIAN_ORDER */
#define VARARG_GET(_AP,_TYPE) \
					((_AP) += VARARG_ROUND_UP(sizeof(_TYPE),sizeof(int)), \
					 *(_TYPE *)((_AP) - sizeof(_TYPE)))
#endif

#if defined(HPUX)
#define VARARG_PEEK(_AP,_TYPE) (*(_TYPE *)((VARARG_PTR)((long)((_AP) \
							- sizeof(_TYPE)) & TYPE_MASK(_TYPE)) \
							+ ((8 - sizeof(_TYPE)) % 4)))
#elif defined(NT) && defined(_ALPHA_)
/*	Just like va_arg from stdargs.h, but without the increment of _AP.offset	*/
#define VARARG_PEEK(_AP,_TYPE)	\
	(*(_TYPE *)((_AP).a0 + (_AP).offset + (((int)sizeof(_TYPE) + 7) & -8)- \
	                    ((__builtin_isfloat(_TYPE) && (_AP).offset <= (6 * 8)) ? \
	                        (6 * 8) + 8 : ((int)sizeof(_TYPE) + 7) & -8)))
#elif defined(OSF)

/*	Just like va_arg from stdargs.h, but without the increment of _AP.offset	*/
#define VARARG_PEEK(_AP,_TYPE)	\
	(*(_TYPE *)((_AP)._a0 + (_AP)._offset + (((int)sizeof(_TYPE) + 7) & -8)- \
	                    ((__builtin_isfloat(_TYPE) && (_AP)._offset <= (6 * 8)) ? \
	                        (6 * 8) + 8 : ((int)sizeof(_TYPE) + 7) & -8)))

#elif defined(OS400)
/* Just like VARARG_GET, but without the increment of _AP.vanew */
#define VARARG_PEEK(_AP,_TYPE)  \
	(sizeof(_TYPE) < sizeof(DWORD) ? (_TYPE)(*(DWORD *)__bndup((_AP).vanew,DWORD)) : (*(_TYPE *)(__bndup((_AP).vanew,_TYPE))))
#elif defined(LITTLE_ENDIAN_ORDER)
#define VARARG_PEEK(_AP,_TYPE) (*(_TYPE *)(_AP))
#else /* BIG_ENDIAN_ORDER */
#if defined(MAC)
#define VARARG_PEEK(_AP,_TYPE) (*(_TYPE *)((_AP) + VARARG_ROUND_UP(sizeof(_TYPE),sizeof(int)) - sizeof(_TYPE)))
#else
#ifndef ZLINUX
#define VARARG_PEEK(_AP,_TYPE) (*(_TYPE *)((CHAR*)(_AP) + VARARG_ROUND_UP(sizeof(_TYPE),sizeof(int)) - sizeof(_TYPE)))
#endif
#endif
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
   Platforms that strictly comply to the C standard do not allow passing of
   variable argument lists by reference using '&' and '*' operators because
   the standard states that you may get undetermined results.  However, you
   are allowed to pass a variable argument list without these operators.  On
   zLinux, whenever you pass a variable argument list the contents will always
   be modified upon returning from the function passing it in as a parameter
   (assuming some function below accesses the list).  For these types of
   platforms, define the following macros to just pass the list without using
   the '&' operator.  All platforms can still use a variable argument list
   as a pointer by simply using the VARARG_PTR_P type.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#if defined(ZLINUX)
typedef VARARG_PTR VARARG_PTR_P;
#define VARARG_ADDR(_AAP) _AAP
#define VARARG_DEREF(_AAP) _AAP
#define VARARG_COPY __va_copy
#else
typedef VARARG_PTR * VARARG_PTR_P;
#define VARARG_ADDR(_AAP) &_AAP
#define VARARG_DEREF(_AAP) *_AAP
#define VARARG_COPY(dest, src) (dest)=(src)
#endif


#ifdef __cplusplus
extern "C" {
#endif

/*	Define entry point used by all "SDK" utility programs */

STATUS LNPUBLIC NotesMain (int argc, char far *argv[]);

/*	Define routines indirectly (or directly) used by "SDK" utility programs */

STATUS LNPUBLIC NotesInitIni (char far *pConfigFileName);
STATUS LNPUBLIC NotesInit (void);
STATUS LNPUBLIC NotesInitExtended (int argc, char far * far *argv);
void   LNPUBLIC NotesTerm (void);
void LNPUBLIC NotesInitModule (HMODULE far *rethModule, HMODULE far *rethInstance, HMODULE far *rethPrevInstance);
#ifdef NLM
typedef	void EXPORTED_LIBRARY_PROC(void);
STATUS LNPUBLIC NotesLibraryMain (int argc, char far * far *argv, EXPORTED_LIBRARY_PROC initproc);
#endif /* NLM */
STATUS LNPUBLIC NotesInitThread (void);
void LNPUBLIC NotesTermThread (void);

#ifdef __cplusplus
}
#endif

/*	OBSOLETE symbol (used to be used to force bootstrap to be pulled in.
	Now replaced by explicitly linking with notes0.obj and notesai0.obj). */

#define NotesSDKMainModule

/*	Define error code packages */

#include "globerr.h"
#if defined(NT)
#define ADMIN_PLATFORM			/* Defined for Admin Client platforms */
#endif
#if defined (NT) && defined(_X86_)
#define OLE_HTMLOBJ_PLATFORM
#endif


/* end of global definitions */

#endif	/* GLOBAL_DEFS */

#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(pop)
#endif


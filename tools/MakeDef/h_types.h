#if !defined( __TTYPES_H )
#define __TTYPES_H

#include <stddef.h>

#pragma warning( disable:4190 4200 4201 4250 4251 4275 4291 4512 4514 4705 4710 )
#define __FLAT__
#define __LITTLE_ENDIAN__

#define __EXPORT__     __declspec(dllexport)
#define __IMPORT__     __declspec(dllimport)

#define DO_EXPORT( T ) __declspec(dllexport) T
#define DO_IMPORT( T ) __declspec(dllimport) T

#define __BYTE_BOUNDARY__    pack( 1 )
#define __RESTORE_BOUNDARY__ pack()

#if !defined( __USHORT_T )
#define __USHORT_T
typedef unsigned short ushort;
#endif

#if !defined( __UCHAR_T )
#define __UCHAR_T
typedef unsigned char uchar;
#endif

#if !defined( __UINT_T )
#define __UINT_T
typedef unsigned int uint;
#endif

#if !defined( __ULONG_T )
#define __ULONG_T
typedef unsigned long ulong;
#endif

#if !defined( EOS )
#define EOS '\0'
#endif

#if defined( __FLAT__ )
	#define maxCollectionSize 1000000000
#else
	#define maxCollectionSize 16384
#endif

#endif   // __TTYPES_H

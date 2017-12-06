/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	Contains the definition of the PGPMemoryMgr object.

	$Id: pgpMemoryMgr.h,v 1.21 1999/05/07 01:55:45 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpMemoryMgr_h	/* [ */
#define Included_pgpMemoryMgr_h


#include "pgpBase.h"

/*____________________________________________________________________________
	Mini-tutorial:
	
	A PGPMemoryMgr is an object which implements memory management, including
	allocation, reallocation, deallocation, and secure versions of the same.
	
	*** Using it ***
	A typical sequence of calls is as follows:
		PGPNewMemoryMgr
		...
		PGPNewData or PGPNewSecureData
		PGPFreeData
		...
		PGPFreeMemoryMgr
	
	Typically, a program will create one PGPMemoryMgr per thread at
	thread creation time and use that memory mgr until the thread dies.
	Generally, an individual PGPMemoryMgr instance is not thread-safe;
	you must either synchronize or use one PGPMemoryMgr per thread.
	
	
	*** Custom Allocators ***
	
	Default allocators are supplied, but the client can create a custom
	PGPMemoryMgr using PGPNewMemoryMgrCustom() which uses client-supplied
	routines.
	Custom routines need only concern themselves with the actual
	allocation and deallocation.
	The following should be kept in mind for user supplied routines:
		- they can ignore the allocation flags passed
		- leaks, memory clearing, etc is done by the PGPMemoryMgr
		- secure allocator must set 'isNonPageable' to TRUE only if the
		memory really can't be paged.
		- the user value is not interpreted by the PGPMemoryMgr. Typically,
		it would be a pointer to some data the allocation routines use
		to store state.
	
	
	*** Secure memory allocation ***
	
	Blocks can be allocated as "Secure" blocks.  Secure blocks are guaranteed
	to be wiped when they are deallocated.  Additionally, if the operating
	system and the current conditions allow, the block will be allocated
	in non-pageable memory.  You can determine the attributes of a block using
	PGPGetMemoryMgrDataInfo().
	
	
	*** Leaks tracking ***
	
	Leaks tracking is implemented when debugging is on,
	but currently reporting is limited to reporting the number of leaks
	outstanding when the PGPMemoryMgr is disposed.
	
	
	*** Debugging ***
	
	For debugging purposes, blocks may be larger in debug mode to accomodate
	various schemes to detect stray pointers, etc.
____________________________________________________________________________*/


#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

enum 
{
	kPGPMemoryMgrFlags_None		=	0,
	kPGPMemoryMgrFlags_Clear	=	1
};

typedef PGPFlags	PGPMemoryMgrFlags;

typedef struct PGPMemoryMgr	PGPMemoryMgr;
typedef PGPMemoryMgr	*	PGPMemoryMgrRef;

#define	kInvalidPGPMemoryMgrRef			((PGPMemoryMgrRef) NULL)
#define PGPMemoryMgrRefIsValid(ref)		((ref) != kInvalidPGPMemoryMgrRef)

typedef	void	*(*PGPMemoryMgrAllocationProc)( PGPMemoryMgrRef mgr,
						PGPUserValue userValue,
						PGPSize requestSize, PGPMemoryMgrFlags flags );

/* realloc not be implemented using PGPNewData() */
typedef	PGPError (*PGPMemoryMgrReallocationProc)( PGPMemoryMgrRef mgr,
						PGPUserValue userValue,
						void **allocation, PGPSize newAllocationSize,
						PGPMemoryMgrFlags flags, PGPSize existingSize );

typedef	PGPError (*PGPMemoryMgrDeallocationProc)( PGPMemoryMgrRef mgr,
						PGPUserValue userValue,
						void *allocation, PGPSize allocationSize );



typedef	void	*(*PGPMemoryMgrSecureAllocationProc)( PGPMemoryMgrRef mgr,
						PGPUserValue userValue,
						PGPSize requestSize, PGPMemoryMgrFlags flags,
						PGPBoolean *isNonPageable );
						

/* deallocation proc need not clear the memory upon deallocation since
	PGPFreeData() does it automatically */
typedef	PGPError (*PGPMemoryMgrSecureDeallocationProc)( PGPMemoryMgrRef mgr,
						PGPUserValue userValue,
						void *allocation, PGPSize allocationSize,
						PGPBoolean	wasLocked );


typedef struct PGPNewMemoryMgrStruct
{
	/* sizeofStruct must be inited to sizeof( PGPNewMemoryMgrStruct ) */
	PGPUInt32		sizeofStruct;
	PGPFlags		reservedFlags;
	
	PGPMemoryMgrAllocationProc		allocProc;
	PGPMemoryMgrReallocationProc	reallocProc;
	PGPMemoryMgrDeallocationProc	deallocProc;
	
	PGPMemoryMgrSecureAllocationProc		secureAllocProc;
	void *									reserved;	/* MUST be zeroed */
	PGPMemoryMgrSecureDeallocationProc		secureDeallocProc;
	
	PGPUserValue					customValue;
	void *							pad[ 8 ];	/* MUST be zeroed */
} PGPNewMemoryMgrStruct;

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


/*____________________________________________________________________________
	Memory Mgr routines
____________________________________________________________________________*/
PGPBoolean	PGPMemoryMgrIsValid( PGPMemoryMgrRef mgr );
#define PGPValidateMemoryMgr( mgr )	\
		PGPValidateParam( PGPMemoryMgrIsValid( mgr ) )

PGPError	PGPNewMemoryMgr( PGPFlags reserved,
				PGPMemoryMgrRef *newMemoryMgr );

PGPError	PGPNewMemoryMgrCustom( PGPNewMemoryMgrStruct const * custom,
				PGPMemoryMgrRef *newMemoryMgr );

PGPError	PGPFreeMemoryMgr( PGPMemoryMgrRef mgr );

PGPError	PGPGetMemoryMgrCustomValue( PGPMemoryMgrRef mgr,
					PGPUserValue *customValue );
PGPError	PGPSetMemoryMgrCustomValue( PGPMemoryMgrRef mgr,
				PGPUserValue customValue );

/* allocate a block of the specified size */
void *  	PGPNewData( PGPMemoryMgrRef mgr,
				PGPSize requestSize, PGPMemoryMgrFlags flags );
				
/* allocate a block of the specified size in non-pageable memory */
/* *isSecure is TRUE if the block definitely can't be paged */
void *  	PGPNewSecureData( PGPMemoryMgrRef mgr,
				PGPSize requestSize, PGPMemoryMgrFlags flags );

/* properly reallocs secure or non-secure blocks */
/* WARNING: the block may move, even if its size is being reduced */
PGPError  	PGPReallocData( PGPMemoryMgrRef mgr,
					void **allocation, PGPSize newAllocationSize,
					PGPMemoryMgrFlags flags );
					
/* properly frees secure or non-secure blocks */
PGPError 	PGPFreeData( void *allocation );


/*____________________________________________________________________________
	Block Info:
		kPGPMemoryMgrBlockInfo_Valid		it's a valid block
		kPGPMemoryMgrBlockInfo_Secure		block is a secure allocation
		kPGPMemoryMgrBlockInfo_NonPageable	block cannot be paged by VM
		
	Secure blocks are always wiped before being disposed,
	but may or may not be pageable, depending on the OS facilities.  Some
	OSs may not provide the ability to make blocks non-pageable.
	
	You should check these flags if the information matters to you.
____________________________________________________________________________*/
#define kPGPMemoryMgrBlockInfo_Valid		( ((PGPFlags)1) << 0 )
#define kPGPMemoryMgrBlockInfo_Secure		( ((PGPFlags)1) << 1 )
#define kPGPMemoryMgrBlockInfo_NonPageable	( ((PGPFlags)1) << 2 )
PGPFlags		PGPGetMemoryMgrDataInfo( void *allocation );


/*____________________________________________________________________________
	Default memory manager routines:
____________________________________________________________________________*/

PGPMemoryMgrRef	PGPGetDefaultMemoryMgr(void);
PGPError		PGPSetDefaultMemoryMgr(PGPMemoryMgrRef memoryMgr);


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS



#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif


#endif /* ] Included_pgpMemoryMgr_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

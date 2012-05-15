/*____________________________________________________________________________
	pgpHash.h
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpHash.h,v 1.15 1999/03/10 02:51:20 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpHashing_h	/* [ */
#define Included_pgpHashing_h

#include "pgpPubTypes.h"
#include "pgpMemoryMgr.h"


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

/*____________________________________________________________________________
	Create a new hash of the specified algorithm.
	
	If the algorithm is not available then kPGPError_AlgorithmNotAvailable is
	returned.
____________________________________________________________________________*/
PGPError 	PGPNewHashContext( PGPMemoryMgrRef memoryMgr,
					PGPHashAlgorithm	algorithm,
					PGPHashContextRef *	outRef );


/*____________________________________________________________________________
	Any existing intermediate hash is lost.
____________________________________________________________________________*/
PGPError 	PGPFreeHashContext( PGPHashContextRef ref );


/*____________________________________________________________________________
	An exact duplicate of the hash is made.
____________________________________________________________________________*/
PGPError 	PGPCopyHashContext( PGPHashContextRef ref,
					PGPHashContextRef * outRef);



/*____________________________________________________________________________
	Reset a hash as if it had been created anew.  Any existing intermediate
	hash is lost.
____________________________________________________________________________*/
PGPError 	PGPResetHash( PGPHashContextRef ref );


/*____________________________________________________________________________
	Continue the hash, accumulating an intermediate result
____________________________________________________________________________*/
PGPError 	PGPContinueHash( PGPHashContextRef ref,
					const void *in, PGPSize numBytes );


/*____________________________________________________________________________
	Finalize the hash, depositing the result into 'hashOut'.
	
	After calling this routine, the hash is reset via PGPResetHash().
	If you want an intermediate result, use PGPCopyHash() and finalize the
	copy.
____________________________________________________________________________*/
PGPError 	PGPFinalizeHash( PGPHashContextRef ref, void *hashOut );


/*____________________________________________________________________________
	Determine size of resulting hash in bytes e.g. a 160 bit hash yields 20.
	Used for generic code which may not know how big a hash is being produced.
	
	Question: can we reasonably assume 8 bits per byte?  If not, how does
	PGPFinalizeHash return its result?
____________________________________________________________________________*/
PGPError 	PGPGetHashSize( PGPHashContextRef ref, PGPSize *hashSize );


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpHashing_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

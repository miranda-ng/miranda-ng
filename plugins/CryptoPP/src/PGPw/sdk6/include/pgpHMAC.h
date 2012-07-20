/*____________________________________________________________________________
	pgpHMAC.h
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: pgpHMAC.h,v 1.3 1999/03/10 02:51:38 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpHMAC_h	/* [ */
#define Included_pgpHMAC_h

#include "pgpPubTypes.h"
#include "pgpMemoryMgr.h"


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

/*____________________________________________________________________________
	Create a new HMAC of the specified algorithm.
	
	If the algorithm is not available then kPGPError_AlgorithmNotAvailable is
	returned.
____________________________________________________________________________*/
PGPError 	PGPNewHMACContext( PGPMemoryMgrRef memoryMgr,
					PGPHashAlgorithm	algorithm,
					PGPByte *			secret,
					PGPSize				secretLen,
					PGPHMACContextRef *	outRef );

/*____________________________________________________________________________
	Any existing intermediate HMAC is lost.
____________________________________________________________________________*/
PGPError 	PGPFreeHMACContext( PGPHMACContextRef ref );

/*____________________________________________________________________________
	Reset an HMAC as if it had been created anew.  Any existing intermediate
	hash is lost.
____________________________________________________________________________*/
PGPError 	PGPResetHMAC( PGPHMACContextRef ref );

/*____________________________________________________________________________
	Continue the HMAC, accumulating an intermediate result
____________________________________________________________________________*/
PGPError 	PGPContinueHMAC( PGPHMACContextRef ref,
					const void *in, PGPSize numBytes );


/*____________________________________________________________________________
	Finalize the HMAC, depositing the result into 'hmacOut'.
	
	This size of the output will be the same size as the hash
	algorithm output.
____________________________________________________________________________*/
PGPError 	PGPFinalizeHMAC( PGPHMACContextRef ref, void *hmacOut );


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpHMAC_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

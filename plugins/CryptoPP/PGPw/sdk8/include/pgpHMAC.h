/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id: pgpHMAC.h,v 1.6 2002/08/06 20:11:16 dallen Exp $
____________________________________________________________________________*/

#ifndef Included_pgpHMAC_h	/* [ */
#define Included_pgpHMAC_h

#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS

/*____________________________________________________________________________
	Create a new HMAC of the specified algorithm.
	
	If the algorithm is not available then kPGPError_AlgorithmNotAvailable is
	returned.
____________________________________________________________________________*/

PGPError 	PGPNewHMACContext( PGPContextRef context,
					PGPHashAlgorithm algorithm, PGPByte *secret,
					PGPSize secretLen, PGPHMACContextRef *outRef );

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

PGPError 	PGPContinueHMAC( PGPHMACContextRef ref, const void *in,
					PGPSize numBytes );

/*____________________________________________________________________________
	Finalize the HMAC, depositing the result into 'hmacOut'.
	
	This size of the output will be the same size as the hash
	algorithm output.
____________________________________________________________________________*/

PGPError 	PGPFinalizeHMAC( PGPHMACContextRef ref, void *hmacOut );

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpHMAC_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

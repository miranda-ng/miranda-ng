/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id: pgpCBC.h,v 1.6 2002/08/06 20:11:16 dallen Exp $
____________________________________________________________________________*/

#ifndef Included_pgpCBC_h	/* [ */
#define Included_pgpCBC_h

#include "pgpSymmetricCipher.h"


PGP_BEGIN_C_DECLARATIONS

/*____________________________________________________________________________
	A CBC context requires use of a symmetric cipher which has been created
	and whose key has been set. An error will be returned if this is not
	the case.
	
	After the call, the CBCContextRef "owns" the
	symmetric ref and will dispose of it properly (even if an error
	occurs).  The caller should no longer reference it.
____________________________________________________________________________*/

PGPError 	PGPNewCBCContext( PGPSymmetricCipherContextRef ref,
					PGPCBCContextRef *outRef );

/*____________________________________________________________________________
	Disposal clears all data in memory before releasing it.
____________________________________________________________________________*/

PGPError 	PGPFreeCBCContext( PGPCBCContextRef ref );

/*____________________________________________________________________________
	Make an exact copy, including current state.  Original is not changed.
____________________________________________________________________________*/

PGPError 	PGPCopyCBCContext( PGPCBCContextRef ref, PGPCBCContextRef *outRef );

/*____________________________________________________________________________
	IV size is implicit (same size as the symmetric cipher block size).
	IV is *copied*.
	Caller may want to destroy the original after passing it in.
____________________________________________________________________________*/

PGPError 	PGPInitCBC( PGPCBCContextRef ref, const void *key,
					const void *initializationVector );

/*____________________________________________________________________________
	Call repeatedly to process arbitrary amounts of data.  Each call must
	have bytesIn be a multiple of the cipher block size.
____________________________________________________________________________*/

PGPError 	PGPCBCEncrypt( PGPCBCContextRef ref, const void *in,
					PGPSize bytesIn, void *out );
					
PGPError 	PGPCBCDecrypt( PGPCBCContextRef ref, const void *in,
					PGPSize bytesIn, void *out );

/*____________________________________________________________________________
	Get the symmetric cipher being used for this CBC context.
	You can use this to determine useful things about the underlying cipher
	such as its block size.
____________________________________________________________________________*/

PGPError 	PGPCBCGetSymmetricCipher( PGPCBCContextRef ref,
					PGPSymmetricCipherContextRef *outRef );

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpCBC_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

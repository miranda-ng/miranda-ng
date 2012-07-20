/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id: pgpCFB.h,v 1.6 2002/08/06 20:11:16 dallen Exp $
____________________________________________________________________________*/

#ifndef Included_pgpCFB_h	/* [ */
#define Included_pgpCFB_h

#include "pgpSymmetricCipher.h"

PGP_BEGIN_C_DECLARATIONS

/*____________________________________________________________________________
	A CFB context requires use of a symmetric cipher which has been created
	and whose key has been set. An error will be returned if this is not
	the case.
	
	After the call, the CFBRef "owns" the symmetric ref and will
	dispose of it properly (even if an error occurs).
	The caller should no longer reference it.
____________________________________________________________________________*/

PGPError 	PGPNewCFBContext( PGPSymmetricCipherContextRef ref,
					PGPUInt16 interleaveFactor,
					PGPCFBContextRef *outRef );

/*____________________________________________________________________________
	Disposal clears all data in memory before releasing it.
____________________________________________________________________________*/

PGPError 	PGPFreeCFBContext( PGPCFBContextRef ref );

/*____________________________________________________________________________
	Make an exact copy, including current state.  Original is not changed.
____________________________________________________________________________*/

PGPError 	PGPCopyCFBContext( PGPCFBContextRef ref,
					PGPCFBContextRef *outRef );

/*____________________________________________________________________________
	IV size is implicit (same size as the symmetric cipher block size).
	IV is *copied*.
	Caller may want to destroy the original after passing it in.
	Calling this implicitly calls PGPResetCFB().
____________________________________________________________________________*/

PGPError 	PGPInitCFB( PGPCFBContextRef ref, const void *key,
					const void *initializationVector );

/*____________________________________________________________________________
	Call repeatedly to process arbitrary amounts of data.
____________________________________________________________________________*/

PGPError 	PGPCFBEncrypt( PGPCFBContextRef ref, const void *in,
					PGPSize bytesIn, void *out );
					
PGPError 	PGPCFBDecrypt( PGPCFBContextRef ref, const void *in,
					PGPSize bytesIn, void *out );

/*____________________________________________________________________________
	Get the symmetric cipher being used for this CFB context.
	You can use this to determine useful things about the underlying cipher
	such as its block size.
____________________________________________________________________________*/

PGPError 	PGPCFBGetSymmetricCipher(PGPCFBContextRef ref,
					PGPSymmetricCipherContextRef *outRef );
					
/*____________________________________________________________________________
	Reset the feedback mechanism to use whatever we have so far, plus previous
	bytes for a total of the cipher block size bytes.  This effectively
	changes the cipher block boundary.
____________________________________________________________________________*/

PGPError 	PGPCFBSync( PGPCFBContextRef ref );

/*____________________________________________________________________________
	Fetch random bytes from the cipher.  Returns the actual number of
	random bytes obtained.
____________________________________________________________________________*/

PGPError 	PGPCFBGetRandom( PGPCFBContextRef ref, PGPSize requestCount,
					void *out, PGPSize *outCount);
					
/*____________________________________________________________________________
	Make more random bytes available using the supplied salt, which must
	be the same as the symmetric cipher block size.
____________________________________________________________________________*/

PGPError 	PGPCFBRandomCycle( PGPCFBContextRef ref, const void *salt);

/*____________________________________________________________________________
	Make more random bytes available using the supplied salt, which must
	be the same as the symmetric cipher block size.
____________________________________________________________________________*/

PGPError 	PGPCFBRandomWash( PGPCFBContextRef ref, const void *in,
					PGPSize bytesIn );


PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpCFB_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

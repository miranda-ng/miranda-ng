/*____________________________________________________________________________
	pgpSymmetricCipher.h
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	public header file for symmetric ciphers

	$Id: pgpSymmetricCipher.h,v 1.22 1999/03/10 02:58:47 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpSymmetricCipher_h	/* [ */
#define Included_pgpSymmetricCipher_h

#include "pgpPubTypes.h"
#include "pgpMemoryMgr.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



/*____________________________________________________________________________
	Create a new cipher of the specified algorithm.  Cannot be used until
	PGPSetSymmetricCipherKey() has been called.
	
	If the algorithm is not available then kPGPError_AlgorithmNotAvailable is
	returned.
	
	Existing algorithms have only one key size.  Values:
		kPGPCipherAlgorithm_CAST5		128 / 8 = 16
		kPGPCipherAlgorithm_3DES		192 / 8 = 24
		kPGPCipherAlgorithm_IDEA		128 / 8 = 16
	In the future symmetric ciphers could be added that have different key
	sizes for the same algorithm.
____________________________________________________________________________*/
PGPError 	PGPNewSymmetricCipherContext( PGPMemoryMgrRef memoryMgr,
					PGPCipherAlgorithm	algorithm, PGPSize keySizeInBytes,
					PGPSymmetricCipherContextRef *outRef );


/*____________________________________________________________________________
	Disposal clears all data in memory before releasing it.
____________________________________________________________________________*/
PGPError 	PGPFreeSymmetricCipherContext( PGPSymmetricCipherContextRef ref );


/*____________________________________________________________________________
	Make an exact copy of the cipher, including the key.
____________________________________________________________________________*/
PGPError 	PGPCopySymmetricCipherContext( PGPSymmetricCipherContextRef ref,
					PGPSymmetricCipherContextRef *outRef );


/*____________________________________________________________________________
	The key must be set before using; a cipher can be repeatedly reset and
	reused with different keys to avoid having to create and destroy new
	contexts each time (and it's also cryptographically better not to reuse
	a key).
	
	kKey size is implicit based on algorithm.  'key' is *copied*.  Caller
	may want to destroy the original after passing it in.
____________________________________________________________________________*/
PGPError 	PGPInitSymmetricCipher( PGPSymmetricCipherContextRef ref,
					const void *key );
					
/*____________________________________________________________________________
	Wipe any sensitive data in the cipher.  Cipher remains alive, but
	key must be set before any data is encrypted.
____________________________________________________________________________*/
PGPError 	PGPWipeSymmetricCipher( PGPSymmetricCipherContextRef ref );


/*____________________________________________________________________________
	"Wash" the symmetric cipher
____________________________________________________________________________*/
PGPError 	PGPWashSymmetricCipher( PGPSymmetricCipherContextRef ref,
					void const *buf, PGPSize len);


/*____________________________________________________________________________
	Encrypt or decrypt one "block" of data.  The block size is determined
	by the cipher (see PGPGetSymmetricCipherBlockSize()).
____________________________________________________________________________*/
PGPError 	PGPSymmetricCipherEncrypt( PGPSymmetricCipherContextRef ref,
					const void *in, void *out );
					
PGPError 	PGPSymmetricCipherDecrypt( PGPSymmetricCipherContextRef ref,
					const void *in, void *out );



					
/*____________________________________________________________________________
	Determine key and block size for specified algorithm.  Stateless routine
	does not need a context.
____________________________________________________________________________*/
PGPError 	PGPGetSymmetricCipherSizes(
					PGPSymmetricCipherContextRef	ref,
					PGPSize *keySize , PGPSize *blockSize );



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS



#endif /* ] Included_pgpSymmetricCipher_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

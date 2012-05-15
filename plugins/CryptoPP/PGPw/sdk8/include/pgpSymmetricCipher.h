/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id: pgpSymmetricCipher.h,v 1.7 2003/10/07 01:29:44 ajivsov Exp $
____________________________________________________________________________*/

#ifndef Included_pgpSymmetricCipher_h	/* [ */
#define Included_pgpSymmetricCipher_h

#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS

/*____________________________________________________________________________
	Create a new cipher of the specified algorithm.  The cipher cannot be used
	until PGPInitSymmetricCipher() has been called.
	
	If the algorithm is not available then kPGPError_AlgorithmNotAvailable is
	returned.
____________________________________________________________________________*/

PGPError 	PGPNewSymmetricCipherContext( PGPContextRef context,
					PGPCipherAlgorithm algorithm,
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
	Key size is implicit based on algorithm.  'key' is *copied*.  Caller
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
	by the cipher (see PGPGetSymmetricCipherSizes()).
____________________________________________________________________________*/

PGPError 	PGPSymmetricCipherEncrypt( PGPSymmetricCipherContextRef ref,
					const void *in, void *out );
					
PGPError 	PGPSymmetricCipherDecrypt( PGPSymmetricCipherContextRef ref,
					const void *in, void *out );
									
/*____________________________________________________________________________
	Determine key and block size for specified algorithm.  Stateless routine
	does not need a context.
____________________________________________________________________________*/
PGPError 	PGPGetSymmetricCipherSizes( PGPSymmetricCipherContextRef ref,
					PGPSize *keySize, PGPSize *blockSize );

/*____________________________________________________________________________
 * This function integrates stream ciphers into framework of block API. 
 * To encrypt/decrypt data, the caller works with stream cipher in ECB block 
 * mode and calls this function before and after the last block. 
 * There is a feedback between the previous block and the current 
 * block for stream ciphers, so ECB for stream ciphers is similiar to CBC 
 * with fixed IV for block ciphers. 
 *
 * To indicate the real size of input data the caller must call this function 
 * with 'size'=0xffffffff before the last block to save a state and then 
 * call this function again after the last block was encrypted/decrypted 
 * with the 'size' set to the real number of bytes in the buffer. This
 * operation is similiar to padding for block ciphers.
____________________________________________________________________________*/
PGPError	PGPSymmetricCipherRollback( PGPSymmetricCipherContextRef ref, 
					PGPSize size );


PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpSymmetricCipher_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id: pgpPublicKey.h,v 1.9 2003/10/03 00:38:40 vinnie Exp $
____________________________________________________________________________*/

#ifndef Included_pgpPublicKey_h	/* [ */
#define Included_pgpPublicKey_h

#include "pgpPubTypes.h"
#include "pgpOptionList.h"

/*____________________________________________________________________________
	Encryption/Signature Message Formats
____________________________________________________________________________*/

enum PGPPublicKeyMessageFormat_
{
	kPGPPublicKeyMessageFormat_PGP		= 1,
	kPGPPublicKeyMessageFormat_PKCS1	= 2,
	kPGPPublicKeyMessageFormat_X509		= 3,
	kPGPPublicKeyMessageFormat_IKE		= 4,

	kPGPPublicKeyMessageFormat_PKCS1_OAEP = 5,	/* default is MGF1 with SHA1 */

	PGP_ENUM_FORCE( PGPPublicKeyMessageFormat_ )
};
PGPENUM_TYPEDEF( PGPPublicKeyMessageFormat_, PGPPublicKeyMessageFormat );

PGP_BEGIN_C_DECLARATIONS

/*____________________________________________________________________________
	Public-key operations

	Return a context for public-key operations based on the specified key.
	The specified message format is used for all operations with this
	context.
____________________________________________________________________________*/

PGPError 	PGPNewPublicKeyContext( PGPKeyDBObjRef publicKeyDBObjRef,
					PGPPublicKeyMessageFormat messageFormat,
					PGPPublicKeyContextRef *outRef );
					
/*____________________________________________________________________________
	Dispose of a public-key context.
____________________________________________________________________________*/

PGPError 	PGPFreePublicKeyContext( PGPPublicKeyContextRef ref );

/*____________________________________________________________________________
	Determine maximum sizes for inputs and outputs.
____________________________________________________________________________*/

PGPError	PGPGetPublicKeyOperationSizes( PGPPublicKeyContextRef ref,
					PGPSize *maxDecryptedBufferSize,
					PGPSize *maxEncryptedBufferSize,
					PGPSize *maxSignatureSize );

/*____________________________________________________________________________
	Encrypt one block of data, using PKCS-1 padding.  Output buffer must
	be of size maxEncryptedBufferSize from PGPGetPublicKeyEncryptionSize.
	outSize is a return parameter.  For some formatting modes the actual
	output size may be less than the maximum possible.
____________________________________________________________________________*/

PGPError	PGPPublicKeyEncrypt( PGPPublicKeyContextRef ref,
					void const *in, PGPSize inSize, void *out,
					PGPSize *outSize );

/*____________________________________________________________________________
	Verify a signature on a message hash.  Returns kPGPError_NoErr on
	correct verification, else an error code.  The message hash is
	finalized and freed by this call (and should not have been finalized
	prior to the call).
____________________________________________________________________________*/
	
PGPError	PGPPublicKeyVerifySignature( PGPPublicKeyContextRef ref,
					PGPHashContextRef hashContext, void const *signature,
					PGPSize signatureSize );

/*____________________________________________________________________________
	Verify a signature on a low-level buffer.  Returns kPGPError_NOErr
	correct verification, else an error code.  Not valid with
    kPGPPublicKeyMessageFormat_PGP contexts.
____________________________________________________________________________*/

PGPError	PGPPublicKeyVerifyRaw( PGPPublicKeyContextRef ref,
					void const *signedData, PGPSize signedDataSize,
					void const *signature, PGPSize signatureSize );

/*____________________________________________________________________________
	Private-key operations

	Return a context for private-key operations based on the specified
	key (which must have a private part).  The specified message
	format is used for all operations with this context.  Unlocks key
	data using passphrase.
____________________________________________________________________________*/
	
PGPError	PGPNewPrivateKeyContext( PGPKeyDBObjRef privateKeyDBObjRef,
					PGPPublicKeyMessageFormat messageFormat,
					PGPPrivateKeyContextRef *outRef,
					PGPOptionListRef firstOption, ...);

/*____________________________________________________________________________
	Dispose of a private-key context.  All sensitive data is wiped before
	being deleted.
____________________________________________________________________________*/

PGPError 	PGPFreePrivateKeyContext( PGPPrivateKeyContextRef ref );

/*____________________________________________________________________________
	Determine maximum sizes for inputs and outputs.
____________________________________________________________________________*/

PGPError	PGPGetPrivateKeyOperationSizes( PGPPrivateKeyContextRef ref,
					PGPSize *maxDecryptedBufferSize,
					PGPSize *maxEncryptedBufferSize,
					PGPSize *maxSignatureSize);

/*____________________________________________________________________________
	Decrypt one block of data.  Output buffer must be of size at least
	maxDecryptedBufferSize from PGPGetPrivateKeyDecryptionSize.
	outSize is a return parameter.  For some formatting modes the actual
	output size may be less than the maximum possible.
____________________________________________________________________________*/

PGPError	PGPPrivateKeyDecrypt( PGPPrivateKeyContextRef ref, void const *in,
					PGPSize inSize, void *out, PGPSize *outSize );

/*____________________________________________________________________________
	Sign a message hash.  Output signature buffer must be of size at
	least maxSignatureSize from PGPGetPrivateKeyDecryptionSize.
	signatureSize is a return parameter.  For some formatting modes
	the actual signature size may be less than the maximum possible.
	The message hash is finalized and freed by this call (and should
	not have been finalized prior to the call).
____________________________________________________________________________*/
	
PGPError	PGPPrivateKeySign( PGPPrivateKeyContextRef ref,
					PGPHashContextRef hashContext, void *signature,
					PGPSize *signatureSize );

/*____________________________________________________________________________
	Sign a low level signedData buffer.  Output signature buffer must be
    of size at least maxSignatureSize from PGPGetPrivateKeyDecryptionSize.
	signatureSize is a return parameter.  Not valid with
    kPGPPublicKeyMessageFormat_PGP contexts.
____________________________________________________________________________*/

PGPError	PGPPrivateKeySignRaw( PGPPrivateKeyContextRef ref,
					void const *signedData, PGPSize signedDataSize,
					void *signature, PGPSize *signatureSize );

/*____________________________________________________________________________
	Miscellaneous operations
____________________________________________________________________________*/

/*____________________________________________________________________________
	Given the size of a prime modulus in bits, this returns an appropriate
	size for an exponent in bits, such that the work factor to find a
	discrete log modulo the modulus is approximately equal to half the
	length of the exponent.  This makes the exponent an appropriate size
	for a subgroup in a discrete log signature scheme.  For encryption
	schemes, where decryption attacks can be stealthy and undetected, we
	use 3/2 times the returned exponent size.
____________________________________________________________________________*/

PGPError	PGPDiscreteLogExponentBits( PGPUInt32 modulusBits,
					PGPUInt32 *exponentBits );


/*____________________________________________________________________________
    Low level Function used for verifying FIPS 186-2 DSA signatures
    using P, Q, G, Y key values.  
____________________________________________________________________________*/

PGPError 	PGPDSAKeyVerifyRaw(
				   PGPContextRef   context,
				   PGPByte const*	P,
				   PGPSize 			PLen,
				   PGPByte const*	Q,
				   PGPSize 			QLen,
				   PGPByte const*	G,
				   PGPSize 			GLen,
				   PGPByte const*	Y,
				   PGPSize 			YLen,
				   PGPByte const*	sig,
				   PGPSize         	sigLen,
				   PGPByte const * 	hash,
				   PGPSize         	hashLen);


PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpPublicKey_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

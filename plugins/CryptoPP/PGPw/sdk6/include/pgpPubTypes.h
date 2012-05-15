/*____________________________________________________________________________
	pgpPubTypes.h
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	Opaque types for various modules go in this file.

	$Id: pgpPubTypes.h,v 1.49.6.1 1999/06/11 06:14:33 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpPubTypes_h	/* [ */
#define Included_pgpPubTypes_h


#include "pgpConfig.h"
#include "pgpBase.h"


/*____________________________________________________________________________
	General data types used by PGPsdk
____________________________________________________________________________*/
typedef struct PGPContext *		PGPContextRef;
typedef struct PGPFileSpec *	PGPFileSpecRef;
typedef struct PGPOptionList * 	PGPOptionListRef;

/*____________________________________________________________________________
	Data types used by the key manipulation functions
____________________________________________________________________________*/

typedef struct PGPKeyDB *		PGPKeyDBRef;
typedef struct PGPKey *			PGPKeyRef;
typedef struct PGPSubKey *  	PGPSubKeyRef;
typedef struct PGPUserID *		PGPUserIDRef;
typedef struct PGPSig *			PGPSigRef;
typedef struct PGPKeySet *		PGPKeySetRef;
typedef struct PGPKeyList *		PGPKeyListRef;
typedef struct PGPKeyIter *		PGPKeyIterRef;
typedef struct PGPFilter *		PGPFilterRef;

typedef struct PGPKeyID
{
	/* do not attempt to interpret these bytes; they *will* change */
	PGPByte		opaqueBytes[ 34 ];
} PGPKeyID;


/*____________________________________________________________________________
	Data types used by symmetric ciphers, cipher modes, hashing
____________________________________________________________________________*/
typedef struct PGPHashContext *				PGPHashContextRef;
typedef struct PGPHMACContext *				PGPHMACContextRef;
typedef struct PGPPublicKeyContext *		PGPPublicKeyContextRef;
typedef struct PGPPrivateKeyContext *		PGPPrivateKeyContextRef;
typedef struct PGPCBCContext *				PGPCBCContextRef;
typedef struct PGPCFBContext *				PGPCFBContextRef;
typedef struct PGPSymmetricCipherContext *	PGPSymmetricCipherContextRef;

/*____________________________________________________________________________
	Data types used by keyserver code
____________________________________________________________________________*/

typedef struct PGPKeyServer *	PGPKeyServerRef;

/*____________________________________________________________________________
	Invalid values for each of the "ref" data types. Use these for assignment
	and initialization only. Use the PGPXXXRefIsValid macros (below) to test
	for valid/invalid values.
____________________________________________________________________________*/

#define	kInvalidPGPContextRef		((PGPContextRef) NULL)
#define	kInvalidPGPFileSpecRef		((PGPFileSpecRef) NULL)
#define	kInvalidPGPOptionListRef	((PGPOptionListRef) NULL)

#define	kInvalidPGPKeyDBRef			((PGPKeyDBRef) NULL)
#define	kInvalidPGPKeyRef			((PGPKeyRef) NULL)
#define	kInvalidPGPSubKeyRef		((PGPSubKeyRef) NULL)
#define	kInvalidPGPUserIDRef		((PGPUserIDRef) NULL)
#define	kInvalidPGPSigRef			((PGPSigRef) NULL)
#define	kInvalidPGPKeySetRef		((PGPKeySetRef) NULL)
#define	kInvalidPGPKeyListRef		((PGPKeyListRef) NULL)
#define	kInvalidPGPKeyIterRef		((PGPKeyIterRef) NULL)
#define	kInvalidPGPFilterRef		((PGPFilterRef) NULL)
#define	kInvalidPGPKeyServerRef		((PGPKeyServerRef) NULL)


#define	kInvalidPGPHashContextRef					\
			((PGPHashContextRef) NULL)
#define	kInvalidPGPHMACContextRef					\
			((PGPHMACContextRef) NULL)
#define	kInvalidPGPCFBContextRef			\
			((PGPCFBContextRef) NULL)
#define	kInvalidPGPCBCContextRef			\
			((PGPCBCContextRef) NULL)
#define	kInvalidPGPSymmetricCipherContextRef		\
			((PGPSymmetricCipherContextRef) NULL)
#define kInvalidPGPPublicKeyContextRef		\
			((PGPPublicKeyContextRef) NULL)
#define kInvalidPGPPrivateKeyContextRef		\
			((PGPPrivateKeyContextRef) NULL)

/*	kPGPInvalidRef is deprecated. Please use a type-specific version  */
#define kPGPInvalidRef				NULL

/*____________________________________________________________________________
	Macros to test for ref validity. Use these in preference to comparing
	directly with the kInvalidXXXRef values.
____________________________________________________________________________*/

#define PGPContextRefIsValid( ref )		( (ref) != kInvalidPGPContextRef )
#define PGPFileSpecRefIsValid( ref )	( (ref) != kInvalidPGPFileSpecRef )
#define PGPOptionListRefIsValid( ref )	( (ref) != kInvalidPGPOptionListRef )

#define PGPKeyDBRefIsValid( ref )		( (ref) != kInvalidPGPKeyDBRef )
#define PGPKeyRefIsValid( ref )			( (ref) != kInvalidPGPKeyRef )
#define PGPSubKeyRefIsValid( ref )		( (ref) != kInvalidPGPSubKeyRef )
#define PGPUserIDRefIsValid( ref )		( (ref) != kInvalidPGPUserIDRef )
#define PGPSigRefIsValid( ref )			( (ref) != kInvalidPGPSigRef )
#define PGPKeySetRefIsValid( ref )		( (ref) != kInvalidPGPKeySetRef )
#define PGPKeyListRefIsValid( ref )		( (ref) != kInvalidPGPKeyListRef )
#define PGPKeyIterRefIsValid( ref )		( (ref) != kInvalidPGPKeyIterRef )
#define PGPFilterRefIsValid( ref )		( (ref) != kInvalidPGPFilterRef )
#define PGPKeyServerRefIsValid( ref )	( (ref) != kInvalidPGPKeyServerRef )

#define PGPHashContextRefIsValid( ref )							\
			( (ref) != kInvalidPGPHashContextRef )
#define PGPHMACContextRefIsValid( ref )							\
			( (ref) != kInvalidPGPHMACContextRef )
#define PGPCFBContextRefIsValid( ref )				\
			( (ref) != kInvalidPGPCFBContextRef )
#define PGPCBCContextRefIsValid( ref )				\
			( (ref) != kInvalidPGPCBCContextRef )
#define PGPSymmetricCipherContextRefIsValid( ref )				\
			( (ref) != kInvalidPGPSymmetricCipherContextRef )
#define PGPPublicKeyContextRefIsValid( ref )				\
			( (ref) != kInvalidPGPPublicKeyContextRef )
#define PGPPrivateKeyContextRefIsValid( ref )				\
			( (ref) != kInvalidPGPPrivateKeyContextRef )

/*	PGPRefIsValid() is deprecated. Please use a type-specific version  */
#define PGPRefIsValid( ref )			( (ref) != kPGPInvalidRef )

/*____________________________________________________________________________
	Symmetric Ciphers
____________________________________________________________________________*/

enum PGPCipherAlgorithm_
{
	/* do NOT change these values */
	kPGPCipherAlgorithm_None	= 0,
	kPGPCipherAlgorithm_IDEA	= 1,
	kPGPCipherAlgorithm_3DES	= 2,
	kPGPCipherAlgorithm_CAST5	= 3,
	
	kPGPCipherAlgorithm_First			= kPGPCipherAlgorithm_IDEA,
	kPGPCipherAlgorithm_Last			= kPGPCipherAlgorithm_CAST5,

	PGP_ENUM_FORCE( PGPCipherAlgorithm_ )
};
PGPENUM_TYPEDEF( PGPCipherAlgorithm_, PGPCipherAlgorithm );

/*____________________________________________________________________________
	Hash algorithms
____________________________________________________________________________*/

enum PGPHashAlgorithm_
{
	/* do NOT change these values */
	kPGPHashAlgorithm_Invalid	= 0,
	kPGPHashAlgorithm_MD5		= 1,
	kPGPHashAlgorithm_SHA		= 2,
	kPGPHashAlgorithm_RIPEMD160	= 3,

	kPGPHashAlgorithm_First			= kPGPHashAlgorithm_MD5,
	kPGPHashAlgorithm_Last			= kPGPHashAlgorithm_RIPEMD160,

	PGP_ENUM_FORCE( PGPHashAlgorithm_ )
};
PGPENUM_TYPEDEF( PGPHashAlgorithm_, PGPHashAlgorithm );

/*____________________________________________________________________________
	Public/Private key algorithms
____________________________________________________________________________*/
enum PGPPublicKeyAlgorithm_
{
	/* note: do NOT change these values */
	kPGPPublicKeyAlgorithm_Invalid			= 0xFFFFFFFF,
	kPGPPublicKeyAlgorithm_RSA				= 1,
	kPGPPublicKeyAlgorithm_RSAEncryptOnly	= 2,
	kPGPPublicKeyAlgorithm_RSASignOnly		= 3,
	kPGPPublicKeyAlgorithm_ElGamal			= 0x10, /* A.K.A.Diffie-Hellman */
	kPGPPublicKeyAlgorithm_DSA				= 0x11,
	
	kPGPPublicKeyAlgorithm_First			= kPGPPublicKeyAlgorithm_RSA,
	kPGPPublicKeyAlgorithm_Last				= kPGPPublicKeyAlgorithm_DSA,
	
	PGP_ENUM_FORCE( PGPPublicKeyAlgorithm_ )
};
PGPENUM_TYPEDEF( PGPPublicKeyAlgorithm_, PGPPublicKeyAlgorithm );

/*____________________________________________________________________________
	Trust values, used to set validity values
____________________________________________________________________________*/

#define kPGPKeyTrust_Mask       0x7u
#define kPGPKeyTrust_Undefined	0x0u
#define kPGPKeyTrust_Unknown	0x1u
#define kPGPKeyTrust_Never		0x2u
#define kPGPKeyTrust_Marginal	0x5u
#define kPGPKeyTrust_Complete	0x6u
#define kPGPKeyTrust_Ultimate	0x7u

#define kPGPNameTrust_Mask      0x3u
#define kPGPNameTrust_Unknown	0x0u
#define kPGPNameTrust_Untrusted	0x1u
#define kPGPNameTrust_Marginal	0x2u
#define kPGPNameTrust_Complete	0x3u

/*____________________________________________________________________________
	Validity levels, used for thresholds in options
____________________________________________________________________________*/

enum PGPValidity_
{
	kPGPValidity_Unknown	= kPGPNameTrust_Unknown,
	kPGPValidity_Invalid	= kPGPNameTrust_Untrusted,
	kPGPValidity_Marginal	= kPGPNameTrust_Marginal,
	kPGPValidity_Complete	= kPGPNameTrust_Complete,

	PGP_ENUM_FORCE( PGPValidity_ )
} ;
PGPENUM_TYPEDEF( PGPValidity_, PGPValidity );

/*____________________________________________________________________________
	Line endings types
____________________________________________________________________________*/

enum PGPLineEndType_
{
	kPGPLineEnd_Default	= 0,
	kPGPLineEnd_LF		= 1,
	kPGPLineEnd_CR		= 2,
	kPGPLineEnd_CRLF	= (kPGPLineEnd_LF | kPGPLineEnd_CR),
	PGP_ENUM_FORCE( PGPLineEndType_ )
};
PGPENUM_TYPEDEF( PGPLineEndType_, PGPLineEndType );

/*____________________________________________________________________________
	Local encoding types
	
	Only one of Force or Auto should be used. The other values are modifiers
____________________________________________________________________________*/

#define kPGPLocalEncoding_None				0x0	/* nothing on */
#define kPGPLocalEncoding_Force				0x01
#define kPGPLocalEncoding_Auto				0x02
#define kPGPLocalEncoding_NoMacBinCRCOkay	0x04

typedef PGPFlags	PGPLocalEncodingFlags;


/* max length is 255; the +1 is for the trailing \0 */
#define kPGPMaxUserIDSize		( (PGPSize)255 + 1 )

/* Size of buffer for PGP-MIME separator (null terminated) */
#define kPGPMimeSeparatorSize	81

#endif /* ] Included_pgpPubTypes_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

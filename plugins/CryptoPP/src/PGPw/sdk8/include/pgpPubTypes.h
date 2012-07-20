/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id: pgpPubTypes.h,v 1.22 2004/04/13 20:50:53 ajivsov Exp $
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
typedef struct PGPKeyDBObj *	PGPKeyDBObjRef;
typedef struct PGPKeySet *		PGPKeySetRef;
typedef struct PGPKeyList *		PGPKeyListRef;
typedef struct PGPKeyIter *		PGPKeyIterRef;
typedef struct PGPFilter *		PGPFilterRef;

typedef struct PGPKeyID
{
	/* do not attempt to interpret these bytes; they *will* change */
	PGPByte		opaqueBytes[ 36 ];
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
Disk Wiping Patterns	
____________________________________________________________________________*/

typedef struct PGPWipePatternContext *		PGPWipePatternContextRef;

/*____________________________________________________________________________
PGP Tar Cache, Objects and Iterators
____________________________________________________________________________*/
typedef struct PGPTARCache *				PGPTARCacheRef;
typedef struct PGPTARCacheIter *			PGPTARCacheIterRef;
typedef struct PGPTARCacheObj *				PGPTARCacheObjRef;
  
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
#define	kInvalidPGPKeyDBObjRef		((PGPKeyDBObjRef) NULL)
#define	kInvalidPGPKeySetRef		((PGPKeySetRef) NULL)
#define	kInvalidPGPKeyListRef		((PGPKeyListRef) NULL)
#define	kInvalidPGPKeyIterRef		((PGPKeyIterRef) NULL)
#define	kInvalidPGPFilterRef		((PGPFilterRef) NULL)
#define	kInvalidPGPKeyServerRef		((PGPKeyServerRef) NULL)
#define	kInvalidPGPHashContextRef	((PGPHashContextRef) NULL)
#define	kInvalidPGPHMACContextRef	((PGPHMACContextRef) NULL)
#define	kInvalidPGPCFBContextRef	((PGPCFBContextRef) NULL)
#define	kInvalidPGPCBCContextRef	((PGPCBCContextRef) NULL)

#define	kInvalidPGPSymmetricCipherContextRef		\
			((PGPSymmetricCipherContextRef) NULL)
#define kInvalidPGPPublicKeyContextRef		\
			((PGPPublicKeyContextRef) NULL)
#define kInvalidPGPPrivateKeyContextRef		\
			((PGPPrivateKeyContextRef) NULL)
#define	kInvalidPGWipePatternContextRef		\
            ((PGPWipePatternContextRef) NULL)
#define	kInvalidPGPTARCacheRef				\
			((PGPTARCacheRef) NULL)
#define	kInvalidPGPTARCacheIterRef			\
			((PGPTARCacheIterRef) NULL)
#define	kInvalidPGPTARCacheObjRef			\
			((PGPTARCacheObjRef) NULL)


/*____________________________________________________________________________
	Macros to test for ref validity. Use these in preference to comparing
	directly with the kInvalidXXXRef values.
____________________________________________________________________________*/

#define PGPContextRefIsValid( ref )		( (ref) != kInvalidPGPContextRef )
#define PGPFileSpecRefIsValid( ref )	( (ref) != kInvalidPGPFileSpecRef )
#define PGPOptionListRefIsValid( ref )	( (ref) != kInvalidPGPOptionListRef )
#define PGPKeyDBRefIsValid( ref )		( (ref) != kInvalidPGPKeyDBRef )
#define PGPKeyDBObjRefIsValid( ref )	( (ref) != kInvalidPGPKeyDBObjRef )
#define PGPKeySetRefIsValid( ref )		( (ref) != kInvalidPGPKeySetRef )
#define PGPKeyListRefIsValid( ref )		( (ref) != kInvalidPGPKeyListRef )
#define PGPKeyIterRefIsValid( ref )		( (ref) != kInvalidPGPKeyIterRef )
#define PGPFilterRefIsValid( ref )		( (ref) != kInvalidPGPFilterRef )
#define PGPKeyServerRefIsValid( ref )	( (ref) != kInvalidPGPKeyServerRef )
#define PGPHashContextRefIsValid( ref )	( (ref) != kInvalidPGPHashContextRef )
#define PGPHMACContextRefIsValid( ref )	( (ref) != kInvalidPGPHMACContextRef )
#define PGPCFBContextRefIsValid( ref )	( (ref) != kInvalidPGPCFBContextRef )
#define PGPCBCContextRefIsValid( ref )	( (ref) != kInvalidPGPCBCContextRef )

#define PGPSymmetricCipherContextRefIsValid( ref )				\
			( (ref) != kInvalidPGPSymmetricCipherContextRef )
#define PGPPublicKeyContextRefIsValid( ref )				\
			( (ref) != kInvalidPGPPublicKeyContextRef )
#define PGPPrivateKeyContextRefIsValid( ref )				\
			( (ref) != kInvalidPGPPrivateKeyContextRef )
#define PGPWipePatternContextRefIsValid( ref )				\
           ( (ref) != kInvalidPGWipePatternContextRef )
 #define PGPTARCacheRefIsValid( ref )						\
			( (ref) != kInvalidPGPTARCacheRef )
#define PGPTARCacheObjRefIsValid( ref )						\
			( (ref) != kInvalidPGPTARCacheObjRef )
#define PGPTARCacheIterRefIsValid( ref)						\
			( (ref) != kInvalidPGPTARCacheIterRef )

/*____________________________________________________________________________
	Symmetric Ciphers
____________________________________________________________________________*/

enum PGPCipherAlgorithm_
{
	kPGPCipherAlgorithm_None		= 0,
	kPGPCipherAlgorithm_IDEA		= 1,
	kPGPCipherAlgorithm_3DES		= 2,
	kPGPCipherAlgorithm_CAST5		= 3,
	kPGPCipherAlgorithm_Blowfish	= 4,
	kPGPCipherAlgorithm_AES128		= 7,
	kPGPCipherAlgorithm_AES192		= 8,
	kPGPCipherAlgorithm_AES256		= 9,
	kPGPCipherAlgorithm_Twofish256	= 10,
	
	PGP_ENUM_FORCE( PGPCipherAlgorithm_ )
};
PGPENUM_TYPEDEF( PGPCipherAlgorithm_, PGPCipherAlgorithm );

/*____________________________________________________________________________
	Hash algorithms
____________________________________________________________________________*/

enum PGPHashAlgorithm_
{
	kPGPHashAlgorithm_Invalid		= 0,
	kPGPHashAlgorithm_MD5			= 1,
	kPGPHashAlgorithm_SHA			= 2,
	kPGPHashAlgorithm_RIPEMD160		= 3,
	kPGPHashAlgorithm_SHA256		= 8,	/* from draft-ietf-openpgp-rfc2440bis-03.txt */
	kPGPHashAlgorithm_SHA384		= 9,
	kPGPHashAlgorithm_SHA512		= 10,

	PGP_ENUM_FORCE( PGPHashAlgorithm_ )
};
PGPENUM_TYPEDEF( PGPHashAlgorithm_, PGPHashAlgorithm );

/*____________________________________________________________________________
	Public/Private key algorithms
____________________________________________________________________________*/
enum PGPPublicKeyAlgorithm_
{
#ifdef __MVS__
	kPGPPublicKeyAlgorithm_Invalid			= -1,
#else
	kPGPPublicKeyAlgorithm_Invalid			= 0xFFFFFFFF,
#endif
	kPGPPublicKeyAlgorithm_RSA				= 1,
	kPGPPublicKeyAlgorithm_RSAEncryptOnly	= 2,
	kPGPPublicKeyAlgorithm_RSASignOnly		= 3,
	kPGPPublicKeyAlgorithm_ElGamal			= 0x10, /* A.K.A.Diffie-Hellman */
	kPGPPublicKeyAlgorithm_DSA				= 0x11,
	kPGPPublicKeyAlgorithm_ECEncrypt		= 0x12,
	kPGPPublicKeyAlgorithm_ECSign			= 0x13,
	
	PGP_ENUM_FORCE( PGPPublicKeyAlgorithm_ )
};
PGPENUM_TYPEDEF( PGPPublicKeyAlgorithm_, PGPPublicKeyAlgorithm );

/*____________________________________________________________________________
	Trust values, used to set validity values
____________________________________________________________________________*/

#define kPGPKeyTrust_Mask       	0x7u
#define kPGPKeyTrust_Undefined		0x0u
#define kPGPKeyTrust_Unknown		0x1u
#define kPGPKeyTrust_Never			0x2u
#define kPGPKeyTrust_Marginal		0x5u
#define kPGPKeyTrust_Complete		0x6u
#define kPGPKeyTrust_Ultimate		0x7u

#define kPGPUserIDTrust_Mask		0x3u
#define kPGPUserIDTrust_Unknown		0x0u
#define kPGPUserIDTrust_Untrusted	0x1u
#define kPGPUserIDTrust_Marginal	0x2u
#define kPGPUserIDTrust_Complete	0x3u

/*____________________________________________________________________________
	Validity levels, used for thresholds in options
____________________________________________________________________________*/

enum PGPValidity_
{
	kPGPValidity_Unknown	= kPGPUserIDTrust_Unknown,
	kPGPValidity_Invalid	= kPGPUserIDTrust_Untrusted,
	kPGPValidity_Marginal	= kPGPUserIDTrust_Marginal,
	kPGPValidity_Complete	= kPGPUserIDTrust_Complete,

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

enum PGPTokenProperty_
{
	kPGPTokenProperty_Invalid 				= 0,
	
	/* boolean properties */	
	kPGPTokenProperty_DirectAuth 			= 1,
	
	/* numeric properties */
	kPGPTokenProperty_KeyGenAlgorithm		= 100,
	kPGPTokenProperty_PrivateKeys			= 101,
	kPGPTokenProperty_PublicKeys			= 102,
	kPGPTokenProperty_SlotID				= 103,
	kPGPTokenProperty_MinPINSize			= 104,
	kPGPTokenProperty_MaxPINSize			= 105,
	kPGPTokenProperty_MinKeySize			= 106,
	kPGPTokenProperty_MaxKeySize			= 107,

	/* string / data properties */
	kPGPTokenProperty_SerialNumber			= 500,
	kPGPTokenProperty_Model					= 501,
	kPGPTokenProperty_ManufacturerID		= 502,
	kPGPTokenProperty_TokenLabel			= 503,
	
    PGP_ENUM_FORCE( PGPTokenProperty_ )
};
PGPENUM_TYPEDEF( PGPTokenProperty_, PGPTokenProperty );

/* Token capabilities / information. 4 byte packing. 
 * Deprecated, use PGPTokenInfoGet*() with PGPTokenProperty
 * 
 */
typedef struct _PGPTokenInfo  {
	/* The size of the structure, including this field.
	   Must be the first field */
	PGPUInt32   size;

	/* Identification information */
	PGPByte     manufacturerID[32];
	PGPByte     model[16];
	PGPByte     serialNumber[16];

	PGPBoolean	bRsa;
	PGPUInt32	minRsaKeySize;
	PGPUInt32	maxRsaKeySize;

	/* General capabilities */
	PGPUInt32   minPinLen;
	PGPUInt32   maxPinLen;

	/* Number of objects */
	PGPUInt32	numPrivKeys;	/* Possibly, non-PGP keys */
	PGPUInt32	numPubKeys;		/* Possibly, non-PGP keys */

	PGPByte		reserved[4];
} PGPTokenInfo;

/*____________________________________________________________________________
	Compression algorithms	
____________________________________________________________________________*/

enum PGPCompressionAlgorithm_
{
	kPGPCompressionAlgorithm_None		= 0,
	kPGPCompressionAlgorithm_ZIP		= 1,
	kPGPCompressionAlgorithm_ZLIB		= 2,
	kPGPCompressionAlgorithm_BZIP2		= 3,
	
	PGP_ENUM_FORCE( PGPCompressionAlgorithm_ )
};
PGPENUM_TYPEDEF( PGPCompressionAlgorithm_, PGPCompressionAlgorithm );

#endif /* ] Included_pgpPubTypes_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

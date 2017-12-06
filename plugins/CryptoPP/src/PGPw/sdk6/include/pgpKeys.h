/*____________________________________________________________________________
	pgpKeys.h
	
	Copyright(C) 1996,1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	Public definitions for PGP KeyDB Library

	$Id: pgpKeys.h,v 1.132 1999/05/18 19:38:45 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpKeys_h	/* [ */
#define Included_pgpKeys_h



#include "pgpPubTypes.h"
#include "pgpHash.h"
#include "pgpOptionList.h"



/* Key ordering */
enum PGPKeyOrdering_
{
	kPGPInvalidOrdering					= 0,
	kPGPAnyOrdering 					= 1,
	kPGPUserIDOrdering					= 2,
	kPGPReverseUserIDOrdering			= 3,
	kPGPKeyIDOrdering					= 4,
	kPGPReverseKeyIDOrdering			= 5,
	kPGPValidityOrdering				= 6,
	kPGPReverseValidityOrdering			= 7,
	kPGPTrustOrdering					= 8,
	kPGPReverseTrustOrdering			= 9,
	kPGPEncryptKeySizeOrdering			= 10,
	kPGPReverseEncryptKeySizeOrdering	= 11,
	kPGPSigKeySizeOrdering				= 12,
	kPGPReverseSigKeySizeOrdering		= 13,
	kPGPCreationOrdering				= 14,
	kPGPReverseCreationOrdering			= 15,
	kPGPExpirationOrdering				= 16,
	kPGPReverseExpirationOrdering		= 17,
	
	PGP_ENUM_FORCE( PGPKeyOrdering_ )
} ;
PGPENUM_TYPEDEF( PGPKeyOrdering_, PGPKeyOrdering );



/* Key properties */

enum PGPKeyPropName_
{	
/* String properties */
	kPGPKeyPropFingerprint				= 1,
	kPGPKeyPropPreferredAlgorithms		= 2,
	kPGPKeyPropThirdPartyRevocationKeyID= 3,
	kPGPKeyPropKeyData					= 4,
	kPGPKeyPropX509MD5Hash				= 5,
	
	/* Number properties */
	kPGPKeyPropAlgID 					= 20,
	kPGPKeyPropBits						= 21,	
	kPGPKeyPropTrust					= 22,	/* old trust model only */
	kPGPKeyPropValidity					= 23,	/* both trust models */
	kPGPKeyPropLockingAlgID				= 24,
	kPGPKeyPropLockingBits				= 25,
	kPGPKeyPropFlags					= 26,

	/* Time properties */
	kPGPKeyPropCreation 				= 40,
	kPGPKeyPropExpiration				= 41,
	kPGPKeyPropCRLThisUpdate			= 42,
	kPGPKeyPropCRLNextUpdate			= 43,

	/* PGPBoolean properties */
	kPGPKeyPropIsSecret 				= 60,
	kPGPKeyPropIsAxiomatic				= 61,
	kPGPKeyPropIsRevoked				= 62,
	kPGPKeyPropIsDisabled				= 63,
	kPGPKeyPropIsNotCorrupt				= 64,
	kPGPKeyPropIsExpired				= 65,
	kPGPKeyPropNeedsPassphrase			= 66,
	kPGPKeyPropHasUnverifiedRevocation	= 67,
	kPGPKeyPropCanEncrypt				= 68,
	kPGPKeyPropCanDecrypt				= 69,
	kPGPKeyPropCanSign					= 70,
	kPGPKeyPropCanVerify				= 71,
	kPGPKeyPropIsEncryptionKey			= 72,
	kPGPKeyPropIsSigningKey				= 73,
	kPGPKeyPropIsSecretShared			= 74,
	kPGPKeyPropIsRevocable				= 75,
	kPGPKeyPropHasThirdPartyRevocation	= 76,
	kPGPKeyPropHasCRL					= 77,
	
	PGP_ENUM_FORCE( PGPKeyPropName_ )
} ;
PGPENUM_TYPEDEF( PGPKeyPropName_, PGPKeyPropName );


/* kPGPKeyPropFlags bits */
enum	/* PGPKeyPropFlags */
{
	kPGPKeyPropFlags_UsageSignUserIDs			= (1UL << 0 ),
	kPGPKeyPropFlags_UsageSignMessages			= (1UL << 1 ),
	kPGPKeyPropFlags_UsageEncryptCommunications	= (1UL << 2 ),
	kPGPKeyPropFlags_UsageEncryptStorage		= (1UL << 3 ),

	kPGPKeyPropFlags_PrivateSplit				= (1UL << 4 ),
	kPGPKeyPropFlags_PrivateShared				= (1UL << 7 )
} ;
typedef PGPFlags		PGPKeyPropFlags;



/* User ID properties */

enum PGPUserIDPropName_
{	
	/* String properties */
	kPGPUserIDPropName 					= 80,
	kPGPUserIDPropAttributeData			= 81,
	kPGPUserIDPropCommonName			= 82,
	kPGPUserIDPropEmailAddress			= 83,
	
	/* Number properties */
	kPGPUserIDPropValidity 				= 100,	/* both trust models */
	kPGPUserIDPropConfidence			= 101,	/* new trust model only */
	kPGPUserIDPropAttributeType			= 102,

	/* Time properties */

	/* PGPBoolean properties */
	kPGPUserIDPropIsAttribute			= 110,
	
	PGP_ENUM_FORCE( PGPUserIDPropName_ )
} ;
PGPENUM_TYPEDEF( PGPUserIDPropName_, PGPUserIDPropName );

/* Signature properties */

enum PGPSigPropName_
{
	/* String properties */
	kPGPSigPropKeyID 					= 120,
	kPGPSigPropX509Certificate			= 121,
	kPGPSigPropX509IASN					= 122,
	kPGPSigPropX509LongName				= 123,
	kPGPSigPropX509IssuerLongName		= 124,
	kPGPSigPropX509DNSName				= 125,
	kPGPSigPropX509IPAddress			= 126,
	kPGPSigPropX509DERDName				= 127,
	
	/* Number properties */
	kPGPSigPropAlgID 					= 140,
	kPGPSigPropTrustLevel				= 141,
	kPGPSigPropTrustValue				= 142,
	
	/* Time properties */
	kPGPSigPropCreation 				= 160,
	kPGPSigPropExpiration				= 161,
	
	/* PGPBoolean properties */
	kPGPSigPropIsRevoked 				= 180,
	kPGPSigPropIsNotCorrupt				= 181,
	kPGPSigPropIsTried					= 182,
	kPGPSigPropIsVerified				= 183,
	kPGPSigPropIsMySig					= 184,
	kPGPSigPropIsExportable				= 185,
	kPGPSigPropHasUnverifiedRevocation	= 186,
	kPGPSigPropIsExpired				= 187,
	kPGPSigPropIsX509					= 188,
	
	PGP_ENUM_FORCE( PGPSigPropName_ )
} ;
PGPENUM_TYPEDEF( PGPSigPropName_, PGPSigPropName );
/*
 * Note on kPGPSigPropIsMySig.  This is a convenience property for
 * determining whether the certification was made by one of the
 * caller's own private keys.  This can only return true if the
 * signing key is in the same base keyset as the certification. If the
 * signing key is( suspected to be) in a different base keyset, call
 * PGPGetSigCertifierKey( certset, signerset, &key) followed by
 * PGPGetKeyBoolean( key, kPGPKeyPropIsSecret, &secret).
 */


/* Attribute types, for use with kPGPUserIDPropAttributeType */
enum PGPAttributeType_
{
	kPGPAttribute_Image						= 1,
	kPGPAttribute_IPAddress					= 10,
	kPGPAttribute_DNSName					= 11,
	kPGPAttribute_Notation					= 20,

	PGP_ENUM_FORCE( PGPAttributeType_ )
} ;
PGPENUM_TYPEDEF( PGPAttributeType_, PGPAttributeType );


enum	/* PGPKeyRingOpenFlags */
{
	kPGPKeyRingOpenFlags_None				= 0,
	kPGPKeyRingOpenFlags_Reserved			= (1UL << 0 ),
	kPGPKeyRingOpenFlags_Mutable			= (1UL << 1 ),
	kPGPKeyRingOpenFlags_Create				= (1UL << 2 ),

	/* The following flags are only used by PGPOpenKeyRing */
	kPGPKeyRingOpenFlags_Private			= (1UL << 8 ),
	kPGPKeyRingOpenFlags_Trusted			= (1UL << 9 )
} ;
typedef PGPFlags		PGPKeyRingOpenFlags;

/*
 * Used by filtering functions to specify type of match.
 */

enum PGPMatchCriterion_
{
	kPGPMatchDefault = 1,
	kPGPMatchEqual = 1,			 /* searched val == supplied val */
	kPGPMatchGreaterOrEqual = 2, /* searched val >= supplied val */
	kPGPMatchLessOrEqual = 3, /* searched val <= supplied val */
	kPGPMatchSubString = 4,	 /* searched val is contained in supplied val */

	PGP_ENUM_FORCE( PGPMatchCriterion_ )
} ;
PGPENUM_TYPEDEF( PGPMatchCriterion_, PGPMatchCriterion );


/* This is the value of the expiration time which means "never expires" */
#define kPGPExpirationTime_Never	( (PGPTime)0 )

/* Secret sharing header size */
#define kPGPShareHeaderSize		4

/* Public entry points */

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


/* Functions for setting up key filters. */

PGPError 		PGPIncFilterRefCount ( PGPFilterRef filter );

PGPError 		PGPFreeFilter ( PGPFilterRef filter );

PGPError 		PGPNewKeyIDFilter( PGPContextRef context, 
						PGPKeyID const * keyID, PGPFilterRef *outFilter);

PGPError		PGPNewSubKeyIDFilter( PGPContextRef		context,
						PGPKeyID const *	subKeyID,  
						PGPFilterRef *		outFilter);

PGPError 		PGPNewKeyEncryptAlgorithmFilter( PGPContextRef context, 
						PGPPublicKeyAlgorithm encryptAlgorithm,
						PGPFilterRef *outFilter );

PGPError 		PGPNewKeyFingerPrintFilter( PGPContextRef context, 
						void const *fingerPrint, 
						PGPSize fingerPrintLength, PGPFilterRef *outFilter );

PGPError 		PGPNewKeyCreationTimeFilter( PGPContextRef context, 
						PGPTime creationTime, PGPMatchCriterion match, 
						PGPFilterRef *outFilter );

PGPError 		PGPNewKeyExpirationTimeFilter( PGPContextRef context, 
						PGPTime expirationTime, 
						PGPMatchCriterion match, PGPFilterRef *outFilter );

PGPError 		PGPNewKeyRevokedFilter( PGPContextRef context,
						PGPBoolean revoked, PGPFilterRef *outFilter );

PGPError		PGPNewKeyDisabledFilter( PGPContextRef context, 
						PGPBoolean			disabled, 
						PGPFilterRef *		outFilter);

PGPError 		PGPNewKeySigAlgorithmFilter( PGPContextRef context,
						PGPPublicKeyAlgorithm sigAlgorithm, 
						PGPFilterRef *outFilter );

PGPError 		PGPNewKeyEncryptKeySizeFilter( PGPContextRef context,
						PGPUInt32 keySize, PGPMatchCriterion match,
						PGPFilterRef *outFilter );

PGPError 		PGPNewKeySigKeySizeFilter( PGPContextRef context,
						PGPUInt32 keySize, PGPMatchCriterion match,
						PGPFilterRef *outFilter );

PGPError		PGPNewUserIDStringFilter( PGPContextRef context,
						char const *		userIDString, 
						PGPMatchCriterion	match,
						PGPFilterRef *		outFilter);

PGPError 		PGPNewUserIDEmailFilter( PGPContextRef context,
						char const *		emailString, 
						PGPMatchCriterion	match,
						PGPFilterRef *		outFilter);

PGPError 		PGPNewUserIDNameFilter( PGPContextRef context,
						char const *		nameString, 
						PGPMatchCriterion	match,
						PGPFilterRef *		outFilter);

PGPError 		PGPNewSigKeyIDFilter( PGPContextRef context,
						PGPKeyID const * keyID,
						PGPFilterRef *outFilter );

PGPError 		PGPNewKeyBooleanFilter( PGPContextRef context,
						PGPKeyPropName property, PGPBoolean match,
						PGPFilterRef *outFilter );

PGPError 		PGPNewKeyNumberFilter( PGPContextRef context,
						PGPKeyPropName property, PGPUInt32 value,
						PGPMatchCriterion match, PGPFilterRef *outFilter );

PGPError 		PGPNewKeyTimeFilter( PGPContextRef context,
						PGPKeyPropName property, PGPTime value,
						PGPMatchCriterion match, PGPFilterRef *outFilter );

PGPError 		PGPNewKeyPropertyBufferFilter( PGPContextRef context,
						PGPKeyPropName property, void *buffer, PGPSize length,
						PGPMatchCriterion match, PGPFilterRef *outFilter );

PGPError 		PGPNewSubKeyBooleanFilter( PGPContextRef context,
						PGPKeyPropName property, PGPBoolean match,
						PGPFilterRef *outFilter );

PGPError 		PGPNewSubKeyNumberFilter( PGPContextRef context,
						PGPKeyPropName property, PGPUInt32 value,
						PGPMatchCriterion match, PGPFilterRef *outFilter );

PGPError 		PGPNewSubKeyTimeFilter( PGPContextRef context,
						PGPKeyPropName property, PGPTime value,
						PGPMatchCriterion match, PGPFilterRef *outFilter );

PGPError 		PGPNewSubKeyPropertyBufferFilter( PGPContextRef context,
						PGPKeyPropName property, void *buffer, PGPSize length,
						PGPMatchCriterion match, PGPFilterRef *outFilter );

PGPError 		PGPNewUserIDBooleanFilter( PGPContextRef context,
						PGPUserIDPropName property, PGPBoolean match,
						PGPFilterRef *outFilter );

PGPError 		PGPNewUserIDNumberFilter( PGPContextRef context,
						PGPUserIDPropName property, PGPUInt32 value,
						PGPMatchCriterion match, PGPFilterRef *outFilter );

PGPError 		PGPNewUserIDStringBufferFilter( PGPContextRef context,
						PGPUserIDPropName property,
						void *buffer, PGPSize length,
						PGPMatchCriterion match, PGPFilterRef *outFilter );

PGPError 		PGPNewSigBooleanFilter( PGPContextRef context,
						PGPSigPropName property, PGPBoolean match,
						PGPFilterRef *outFilter );

PGPError 		PGPNewSigNumberFilter( PGPContextRef context,
						PGPSigPropName property, PGPUInt32 value,
						PGPMatchCriterion match, PGPFilterRef *outFilter );

PGPError 		PGPNewSigTimeFilter( PGPContextRef context,
						PGPSigPropName property, PGPTime value,
						PGPMatchCriterion match, PGPFilterRef *outFilter );

PGPError 		PGPNewSigPropertyBufferFilter( PGPContextRef context,
						PGPSigPropName property, void *buffer, PGPSize length,
						PGPMatchCriterion match, PGPFilterRef *outFilter );

/* freeing outfilter will call PGPFreeFilter on filter */
PGPError 		PGPNegateFilter( PGPFilterRef filter,
						PGPFilterRef *outFilter);

/* freeing outfilter will call PGPFreeFilter on filter1, filter2 */
PGPError 		PGPIntersectFilters( PGPFilterRef filter1,
						PGPFilterRef filter2, PGPFilterRef *outFilter);

/* freeing outfilter will call PGPFreeFilter on filter1, filter2 */
PGPError 		PGPUnionFilters( PGPFilterRef filter1,
						PGPFilterRef filter2, PGPFilterRef *outFilter);


PGPError 		PGPFilterKeySet( PGPKeySetRef origSet,
						PGPFilterRef filter, PGPKeySetRef *resultSet);


/* Keyserver filter functions */

PGPError 		PGPLDAPQueryFromFilter( PGPFilterRef filter,
						char **queryOut );

PGPError 		PGPHKSQueryFromFilter( PGPFilterRef	filter,
						char **queryOut );
						
PGPError		PGPNetToolsCAHTTPQueryFromFilter( PGPFilterRef filter,
						char **queryOut );

/* KeySet manipulations */


/* Creates a new memory-based KeyDB and returns its root set */
PGPError  		PGPNewKeySet(PGPContextRef context, PGPKeySetRef *keySet);


/* Used for building arbitrary subsets of an existing KeyDB */
PGPError  		PGPNewEmptyKeySet(PGPKeySetRef origSet, PGPKeySetRef *newSet);
PGPError  		PGPNewSingletonKeySet(PGPKeyRef key, PGPKeySetRef *keySet);
PGPError		PGPUnionKeySets(PGPKeySetRef set1, PGPKeySetRef set2,
								PGPKeySetRef *newSet);

PGPError  		PGPOpenDefaultKeyRings( PGPContextRef context,
						PGPKeyRingOpenFlags openFlags, PGPKeySetRef *keySet);

PGPError  		PGPOpenKeyRingPair( PGPContextRef context,
						PGPKeyRingOpenFlags openFlags, 
						PGPFileSpecRef pubFileRef, PGPFileSpecRef secFileRef,
						PGPKeySetRef *keySet);

PGPError  		PGPOpenKeyRing (PGPContextRef context,
						PGPKeyRingOpenFlags openFlags, PGPFileSpecRef fileRef,
						PGPKeySetRef *keySet);

PGPError 		PGPCommitKeyRingChanges (PGPKeySetRef keys);

PGPError 		PGPPropagateTrust (PGPKeySetRef keys);

PGPError 		PGPRevertKeyRingChanges (PGPKeySetRef keys);

PGPError 		PGPCheckKeyRingSigs (PGPKeySetRef keysToCheck,
						PGPKeySetRef keysSigning, PGPBoolean checkAll,
						PGPEventHandlerProcPtr eventHandler,
						PGPUserValue eventHandlerData);

PGPError 		PGPReloadKeyRings (PGPKeySetRef keys);

PGPError 		PGPGetKeyByKeyID (PGPKeySetRef keys,
						PGPKeyID const * keyID,
						PGPPublicKeyAlgorithm pubKeyAlgorithm,
						PGPKeyRef *outRef);

PGPBoolean 		PGPKeySetIsMember(PGPKeyRef key, PGPKeySetRef set);

PGPError 		PGPCountKeys( PGPKeySetRef keys, PGPUInt32 *numKeys);

PGPError 		PGPIncKeySetRefCount( PGPKeySetRef keys);

PGPError 		PGPFreeKeySet( PGPKeySetRef keys);

PGPBoolean 		PGPKeySetIsMutable( PGPKeySetRef keys);

PGPBoolean 		PGPKeySetNeedsCommit( PGPKeySetRef keys);


PGPError 		PGPAddKeys( PGPKeySetRef keysToAdd, PGPKeySetRef set );

PGPError 		PGPRemoveKeys( PGPKeySetRef keysToRemove, PGPKeySetRef set );



/* Key manipulation functions */

PGPError 		PGPDisableKey( PGPKeyRef key);

PGPError 		PGPEnableKey( PGPKeyRef key);

PGPError 		PGPRemoveSubKey( PGPSubKeyRef subkey);

PGPError 		PGPRemoveUserID( PGPUserIDRef userID);

PGPError 		PGPSetPrimaryUserID( PGPUserIDRef userid);

PGPError 		PGPCertifyPrimaryUserID( PGPUserIDRef userid,
						PGPOptionListRef firstOption, ...);

PGPError 		PGPGetSigCertifierKey( PGPSigRef cert, PGPKeySetRef allkeys,
						PGPKeyRef *certkey);

PGPError 		PGPGetSigX509CertifierSig( PGPSigRef cert,
						PGPKeySetRef allkeys, PGPSigRef *certsig);

PGPError 		PGPRemoveSig( PGPSigRef cert);

PGPError		PGPCountAdditionalRecipientRequests( PGPKeyRef basekey,
					PGPUInt32 * numARKeys);
					
PGPError 		PGPGetIndexedAdditionalRecipientRequestKey(
						PGPKeyRef basekey, PGPKeySetRef allkeys, PGPUInt32 nth,
						PGPKeyRef *arkey, PGPKeyID *arkeyid,
						PGPByte *arclass );

PGPError		PGPCountRevocationKeys( PGPKeyRef basekey,
					PGPUInt32 * numRevKeys);

PGPError 		PGPGetIndexedRevocationKey(
						PGPKeyRef basekey, PGPKeySetRef allkeys, PGPUInt32 nth,
						PGPKeyRef *revkey, PGPKeyID *revkeyid );
PGPError		PGPGetCRLDistributionPoints(
						PGPKeyRef cakey, PGPKeySetRef keyset,
						PGPUInt32 *pnDistPoints, PGPByte **pDpoints,
						PGPSize **pdpointLengths );


/* Wrapper functions */
PGPError			PGPGenerateKey( PGPContextRef context, PGPKeyRef *key,
						PGPOptionListRef firstOption, ...);
PGPError			PGPGenerateSubKey( PGPContextRef context,
						PGPSubKeyRef *subkey,
						PGPOptionListRef firstOption, ...);
PGPUInt32			PGPGetKeyEntropyNeeded( PGPContextRef context,
						PGPOptionListRef firstOption, ...);
PGPError			PGPExportKeySet( PGPKeySetRef keys,
						PGPOptionListRef firstOption, ...);
PGPError			PGPExport( PGPContextRef context,
						PGPOptionListRef firstOption, ...);
PGPError			PGPImportKeySet( PGPContextRef context,
						PGPKeySetRef *keys,
						PGPOptionListRef firstOption, ...);
PGPError			PGPSignUserID( PGPUserIDRef userID,
						PGPKeyRef certifyingKey,
						PGPOptionListRef firstOption, ...);
PGPError			PGPAddUserID( PGPKeyRef key, char const *userID,
						PGPOptionListRef firstOption, ...);
PGPError			PGPAddAttributeUserID( PGPKeyRef key,
						PGPAttributeType attributeType,
						PGPByte *attributeData, PGPSize attributeLength,
						PGPOptionListRef firstOption, ...);
PGPError			PGPRevokeSig( PGPSigRef cert, PGPKeySetRef allkeys,
						PGPOptionListRef firstOption, ...);
PGPError			PGPRevokeKey( PGPKeyRef key,
						PGPOptionListRef firstOption, ...);
PGPError			PGPRevokeSubKey( PGPSubKeyRef subkey,
						PGPOptionListRef firstOption, ...);
PGPError	 		PGPChangePassphrase( PGPKeyRef key,
						PGPOptionListRef firstOption, ...);
PGPError	 		PGPChangeSubKeyPassphrase( PGPSubKeyRef subkey,
						PGPOptionListRef firstOption, ...);
PGPBoolean			PGPPassphraseIsValid( PGPKeyRef key,
						PGPOptionListRef firstOption, ...);
PGPError	 		PGPSetKeyAxiomatic( PGPKeyRef key,
						PGPOptionListRef firstOption, ...);



/*
 * Trust values for PGPSetKeyTrust and kPGPKeyPropTrust property:
 * 
 * kPGPKeyTrust_Undefined	(do not pass to PGPSetKeyTrust)
 * kPGPKeyTrust_Unknown		(unknown)
 * kPGPKeyTrust_Never		(never)
 * kPGPKeyTrust_Marginal	(sometimes)
 * kPGPKeyTrust_Complete	(always)
 * kPGPKeyTrust_Ultimate	(do not pass to PGPSetKeyTrust)
 */

 /* old trust model */
PGPError 		PGPSetKeyTrust( PGPKeyRef key, PGPUInt32 trust);

PGPError 		PGPUnsetKeyAxiomatic( PGPKeyRef key);

/*  Get property functions */

PGPError 		PGPGetKeyBoolean( PGPKeyRef key, PGPKeyPropName propname,
						PGPBoolean *prop);

PGPError 		PGPGetKeyNumber( PGPKeyRef key, PGPKeyPropName propname,
						PGPInt32 *prop);

/* 'buffer' is NOT null-terminated */
/* returns kPGPError_BufferTooSmall if buffer is too small, but
fill buffer to capacity */
PGPError 		PGPGetKeyPropertyBuffer( PGPKeyRef key,
						PGPKeyPropName propname,
						PGPSize bufferSize, void *data, PGPSize *fullSize);

PGPError 		PGPGetKeyTime( PGPKeyRef key, PGPKeyPropName propname,
						PGPTime *prop);

PGPError 		PGPGetSubKeyBoolean( PGPSubKeyRef subkey,
						PGPKeyPropName propname, PGPBoolean *prop);

PGPError 		PGPGetSubKeyNumber( PGPSubKeyRef subkey,
						PGPKeyPropName propname, PGPInt32 *prop);

/* returns kPGPError_BufferTooSmall if buffer is too small, but
fill buffer to capacity */
PGPError 		PGPGetSubKeyPropertyBuffer( PGPSubKeyRef subkey,
						PGPKeyPropName propname,
						PGPSize bufferSize, void *prop, PGPSize *fullSize);

PGPError 		PGPGetSubKeyTime( PGPSubKeyRef subkey,
						PGPKeyPropName propname, PGPTime *prop);

PGPError 		PGPGetUserIDNumber( PGPUserIDRef userID,
						PGPUserIDPropName propname, PGPInt32 *prop);

PGPError 		PGPGetUserIDBoolean( PGPUserIDRef userID,
						PGPUserIDPropName propname, PGPBoolean *prop);

/* 'string' is always NULL-terminated */
/* returns kPGPError_BufferTooSmall if buffer is too small, but
fill buffer to capacity */
PGPError 		PGPGetUserIDStringBuffer( PGPUserIDRef userID,
						PGPUserIDPropName propname,
						PGPSize bufferSize,
						char *		string,
						PGPSize *	fullSize);

/* 'buffer' is NOT null-terminated */
/* returns kPGPError_BufferTooSmall if buffer is too small, but
fill buffer to capacity */
PGPError 		PGPGetSigPropertyBuffer( PGPSigRef cert,
						PGPSigPropName propname,
						PGPSize bufferSize, void *data, PGPSize *fullSize);

PGPError 		PGPGetSigBoolean( PGPSigRef cert,
						PGPSigPropName propname, PGPBoolean *prop);

PGPError 		PGPGetSigNumber( PGPSigRef cert, PGPSigPropName propname,
						PGPInt32 *prop);
						
PGPError 		PGPGetKeyIDOfCertifier( PGPSigRef sig, PGPKeyID *outID );
	
PGPError 		PGPGetSigTime( PGPSigRef cert, PGPSigPropName propname,
						PGPTime *prop);

PGPError 		PGPGetHashAlgUsed( PGPKeyRef key, PGPHashAlgorithm *hashAlg);

/* Convenience property functions */

PGPError  		PGPGetPrimaryUserID( PGPKeyRef key, PGPUserIDRef *outRef );
PGPError		PGPGetPrimaryAttributeUserID (PGPKeyRef key,
						PGPAttributeType attributeType, PGPUserIDRef *outRef);

/* 'string' is always a C string and
	*fullSize includes the '\0' terminator */
/* returns kPGPError_BufferTooSmall if buffer is too small, but
fill buffer to capacity */
PGPError 		PGPGetPrimaryUserIDNameBuffer( PGPKeyRef key, 
					PGPSize bufferSize,
					char *	string, PGPSize *fullSize );

PGPError 		PGPGetPrimaryUserIDValidity(PGPKeyRef key,
					PGPValidity *validity);


PGPInt32 		PGPCompareKeys(PGPKeyRef a, PGPKeyRef b,
						PGPKeyOrdering order);

PGPInt32 		PGPCompareUserIDStrings(char const *a, char const *b);

PGPError  		PGPOrderKeySet( PGPKeySetRef src,
						PGPKeyOrdering order, PGPKeyListRef *outRef );

PGPError 		PGPIncKeyListRefCount( PGPKeyListRef keys);

PGPError 		PGPFreeKeyList( PGPKeyListRef keys);


/* Key iteration functions */



PGPError 		PGPNewKeyIter (PGPKeyListRef keys, PGPKeyIterRef *outRef);

PGPError 		PGPCopyKeyIter (PGPKeyIterRef orig, PGPKeyIterRef *outRef);
	
PGPError 		PGPFreeKeyIter (PGPKeyIterRef iter);

PGPInt32 		PGPKeyIterIndex (PGPKeyIterRef iter);

PGPError 		PGPKeyIterRewind (PGPKeyIterRef iter);

PGPInt32 		PGPKeyIterSeek (PGPKeyIterRef iter, PGPKeyRef key);

PGPError 		PGPKeyIterMove (PGPKeyIterRef iter, PGPInt32 relOffset,
						PGPKeyRef *outRef);

PGPError 		PGPKeyIterNext (PGPKeyIterRef iter, PGPKeyRef *outRef);

PGPError 		PGPKeyIterPrev (PGPKeyIterRef iter, PGPKeyRef *outRef);

PGPError 		PGPKeyIterKey (PGPKeyIterRef iter, PGPKeyRef *outRef);

PGPError  		PGPKeyIterSubKey (PGPKeyIterRef iter,
						PGPSubKeyRef *outRef );

PGPError  		PGPKeyIterUserID (PGPKeyIterRef iter,
						PGPUserIDRef *outRef );
PGPError  		PGPKeyIterSig (PGPKeyIterRef iter,
						PGPSigRef *outRef );

PGPError  		PGPKeyIterNextSubKey (PGPKeyIterRef iter,
						PGPSubKeyRef *outRef );

PGPError  		PGPKeyIterPrevSubKey (PGPKeyIterRef iter,
						PGPSubKeyRef *outRef);

PGPError 		PGPKeyIterRewindSubKey (PGPKeyIterRef iter);

PGPError  		PGPKeyIterNextUserID (PGPKeyIterRef iter,
						PGPUserIDRef *outRef);

PGPError  		PGPKeyIterPrevUserID (PGPKeyIterRef iter,
						PGPUserIDRef *outRef);

PGPError 		PGPKeyIterRewindUserID (PGPKeyIterRef iter);

PGPError  		PGPKeyIterNextUIDSig (PGPKeyIterRef iter,
						PGPSigRef *outRef);

PGPError  		PGPKeyIterPrevUIDSig (PGPKeyIterRef iter,
						PGPSigRef *outRef);

PGPError 		PGPKeyIterRewindUIDSig (PGPKeyIterRef iter);


/* Get/set default private key */

PGPError 		PGPGetDefaultPrivateKey( PGPKeySetRef keyset,
						PGPKeyRef *outRef );

PGPError 		PGPSetDefaultPrivateKey( PGPKeyRef key);



/* Get/set user value */

PGPError 		PGPSetKeyUserVal( PGPKeyRef key, PGPUserValue userValue);

PGPError 		PGPSetUserIDUserVal( PGPUserIDRef userid,
						PGPUserValue userValue);

PGPError 		PGPSetSubKeyUserVal( PGPSubKeyRef subkey,
						PGPUserValue userValue);

PGPError 		PGPSetSigUserVal( PGPSigRef cert,
						PGPUserValue userValue);

PGPError 		PGPGetKeyUserVal( PGPKeyRef key,
						PGPUserValue *userValue);

PGPError 		PGPGetUserIDUserVal( PGPUserIDRef userid,
						PGPUserValue *userValue);

PGPError 		PGPGetSubKeyUserVal( PGPSubKeyRef subkey,
						PGPUserValue *userValue);

PGPError 		PGPGetSigUserVal( PGPSigRef cert,
						PGPUserValue *userValue);

/* Passphrase conversion to passkeybuffer */
/* The size of the output buffer is from the kPGPKeyPropLockingBits property */

PGPError		PGPGetKeyPasskeyBuffer ( PGPKeyRef key,
						void *passkeyBuffer, PGPOptionListRef firstOption,...);

PGPError		PGPGetSubKeyPasskeyBuffer ( PGPSubKeyRef subkey,
						void *passkeyBuffer, PGPOptionListRef firstOption,...);


/* Change key options which are stored in self signatures internally */

PGPError		PGPAddKeyOptions( PGPKeyRef key,
						PGPOptionListRef firstOption, ...);

PGPError		PGPRemoveKeyOptions( PGPKeyRef key,
						PGPOptionListRef firstOption, ...);

PGPError		PGPUpdateKeyOptions( PGPKeyRef key,
						PGPOptionListRef firstOption, ...);



/*____________________________________________________________________________
	Key IDs
____________________________________________________________________________*/
PGPError 	PGPGetKeyIDFromString(
				const char *string, PGPKeyID *id );
					
PGPError 	PGPGetKeyIDFromKey( PGPKeyRef key, PGPKeyID *id );

PGPError 	PGPGetKeyIDFromSubKey( PGPSubKeyRef key, PGPKeyID * id );

enum PGPKeyIDStringType_
{
	kPGPKeyIDString_Abbreviated = 2,
	kPGPKeyIDString_Full = 3,
	PGP_ENUM_FORCE( PGPKeyIDStringType_ )
};
PGPENUM_TYPEDEF( PGPKeyIDStringType_, PGPKeyIDStringType );


#define kPGPMaxKeyIDStringSize		( 127 + 1 )
PGPError 	PGPGetKeyIDString( PGPKeyID const * ref,
					PGPKeyIDStringType type,
					char outString[ kPGPMaxKeyIDStringSize ] );


/* outputs opaque string of bytes for storage of maximum size as keyID*/
/* do NOT attempt to parse the output; it is opaque to you */
#define kPGPMaxExportedKeyIDSize		( sizeof( PGPKeyID ) )
PGPError 	PGPExportKeyID( PGPKeyID const * keyID,
					PGPByte exportedData[ kPGPMaxExportedKeyIDSize ],
					PGPSize *exportedLength );

/* must be in format output by PGPExportKeyID */
PGPError 	PGPImportKeyID(  void const * data, PGPKeyID * id );

/* returns 0 if equal, -1 if key1 < key2, 1 if key1 > key2 */
PGPInt32 	PGPCompareKeyIDs( PGPKeyID const * key, PGPKeyID const * key2);




/*____________________________________________________________________________
	Getting contexts back from key related items.  If the key is invalid,
	you get kPGPInvalidRef back.
____________________________________________________________________________*/

PGPContextRef	PGPGetKeyListContext( PGPKeyListRef ref );
PGPContextRef	PGPGetKeySetContext( PGPKeySetRef ref );
PGPContextRef	PGPGetKeyIterContext( PGPKeyIterRef ref );
PGPContextRef	PGPGetKeyContext( PGPKeyRef ref );
PGPContextRef	PGPGetSubKeyContext( PGPSubKeyRef ref );
PGPContextRef	PGPGetUserIDContext( PGPUserIDRef ref );

/*____________________________________________________________________________
	Getting parent objects from key related items.  If the input is invalid,
	you get kPGPInvalidRef back.
____________________________________________________________________________*/
PGPKeyRef		PGPGetUserIDKey( PGPUserIDRef ref );
PGPUserIDRef	PGPGetSigUserID( PGPSigRef ref );
PGPKeyRef		PGPGetSigKey( PGPSigRef ref );


/*____________________________________________________________________________
    Secret sharing functionality
____________________________________________________________________________*/

PGPError		PGPSecretShareData(PGPContextRef context,
						void const * input, PGPSize inputBytes,
						PGPUInt32 threshold, PGPUInt32 nShares, void * output);

PGPError		PGPSecretReconstructData(PGPContextRef context,
						void * input, PGPSize outputBytes,
						PGPUInt32 nShares, void * output);


/*____________________________________________________________________________
    X509 certificate specific
____________________________________________________________________________*/

PGPError		PGPVerifyX509CertificateChain (PGPContextRef context,
						PGPByte *certchain, PGPByte *rootcerts);

PGPError		PGPCreateDistinguishedName( PGPContextRef context,
						char const *str,
						PGPByte **pdname, PGPSize *pdnamelen );


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpKeys_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id: pgpKeys.h,v 1.76 2004/05/19 00:09:07 ajivsov Exp $
____________________________________________________________________________*/

#ifndef Included_pgpKeys_h	/* [ */
#define Included_pgpKeys_h

#include "pgpPubTypes.h"
#include "pgpHash.h"
#include "pgpOptionList.h"

/* Key ordering */

enum PGPKeyOrdering_
{
	kPGPKeyOrdering_Invalid				= 0,
	kPGPKeyOrdering_Any					= 1,
	kPGPKeyOrdering_UserID				= 2,
	kPGPKeyOrdering_KeyID				= 3,
	kPGPKeyOrdering_Validity			= 4,
	kPGPKeyOrdering_Trust				= 5,
	kPGPKeyOrdering_EncryptionKeySize	= 6,
	kPGPKeyOrdering_SigningKeySize		= 7,
	kPGPKeyOrdering_CreationDate		= 8,
	kPGPKeyOrdering_ExpirationDate		= 9,
	
	PGP_ENUM_FORCE( PGPKeyOrdering_ )
} ;
PGPENUM_TYPEDEF( PGPKeyOrdering_, PGPKeyOrdering );


/* Key properties */

enum PGPKeyDBObjProperty_
{
	kPGPKeyDBObjProperty_Invalid 				= 0,
	
	/* Generic numeric properties */
	kPGPKeyDBObjProperty_ObjectType				= 20,
	
	/* Key boolean properties */	
	kPGPKeyProperty_IsSecret 					= 100,
	kPGPKeyProperty_IsAxiomatic					= 101,
	kPGPKeyProperty_IsRevoked					= 102,
	kPGPKeyProperty_IsDisabled					= 103,
	kPGPKeyProperty_IsNotCorrupt				= 104,
	kPGPKeyProperty_IsExpired					= 105,
	kPGPKeyProperty_NeedsPassphrase				= 106,
	kPGPKeyProperty_HasUnverifiedRevocation		= 107,
	kPGPKeyProperty_CanEncrypt					= 108,
	kPGPKeyProperty_CanDecrypt					= 109,
	kPGPKeyProperty_CanSign						= 110,
	kPGPKeyProperty_CanVerify					= 111,
	kPGPKeyProperty_IsEncryptionKey				= 112,
	kPGPKeyProperty_IsSigningKey				= 113,
	kPGPKeyProperty_IsSecretShared				= 114,
	kPGPKeyProperty_IsRevocable					= 115,
	kPGPKeyProperty_HasThirdPartyRevocation		= 116,
	kPGPKeyProperty_HasCRL						= 117,
	kPGPKeyProperty_IsOnToken					= 118,
	kPGPKeyProperty_IsStubKey					= 119,	/* used to filter dummy keys from inclusive key set */

	/* Key numeric properties */	
	kPGPKeyProperty_AlgorithmID 				= 200,
	kPGPKeyProperty_Bits						= 201,	
	kPGPKeyProperty_Trust						= 202,
	kPGPKeyProperty_Validity					= 203,
	kPGPKeyProperty_LockingAlgorithmID			= 204,
	kPGPKeyProperty_LockingBits					= 205,
	kPGPKeyProperty_Flags						= 206,
	kPGPKeyProperty_HashAlgorithmID				= 207,
	kPGPKeyProperty_Version						= 208,
	kPGPKeyProperty_KeyServerPreferences		= 209,
	kPGPKeyProperty_TokenNum					= 210,
	kPGPKeyProperty_Features					= 211,	

	/* Key time properties */	
	kPGPKeyProperty_Creation 					= 300,
	kPGPKeyProperty_Expiration					= 301,
	kPGPKeyProperty_CRLThisUpdate				= 302,
	kPGPKeyProperty_CRLNextUpdate				= 303,

	/* Key data (variable sized) properties */
	kPGPKeyProperty_Fingerprint					= 401,
	kPGPKeyProperty_KeyID						= 402,
	kPGPKeyProperty_PreferredAlgorithms			= 403,
	kPGPKeyProperty_ThirdPartyRevocationKeyID 	= 404,
	kPGPKeyProperty_KeyData						= 405,
	kPGPKeyProperty_X509MD5Hash					= 406,
	kPGPKeyProperty_PreferredKeyServer			= 407,
	kPGPKeyProperty_PreferredCompressionAlgorithms = 408,
	
	/* SubKey boolean properties */	
	kPGPSubKeyProperty_IsRevoked				= 501,
	kPGPSubKeyProperty_IsNotCorrupt				= 502,
	kPGPSubKeyProperty_IsExpired				= 503,
	kPGPSubKeyProperty_NeedsPassphrase			= 504,
	kPGPSubKeyProperty_HasUnverifiedRevocation	= 505,
	kPGPSubKeyProperty_IsRevocable				= 506,
	kPGPSubKeyProperty_HasThirdPartyRevocation	= 507,
	kPGPSubKeyProperty_IsOnToken				= 508,

	/* SubKey numeric properties */
	kPGPSubKeyProperty_AlgorithmID 				= 600,
	kPGPSubKeyProperty_Bits						= 601,	
	kPGPSubKeyProperty_LockingAlgorithmID		= 602,
	kPGPSubKeyProperty_LockingBits				= 603,
	kPGPSubKeyProperty_Version					= 604,
	kPGPSubKeyProperty_Flags					= 605,

	/* SubKey time properties */	
	kPGPSubKeyProperty_Creation 				= 700,
	kPGPSubKeyProperty_Expiration				= 701,

	/* SubKey data (variable sized) properties */	
	kPGPSubKeyProperty_KeyData					= 800,
	kPGPSubKeyProperty_KeyID					= 801,

	/* User ID boolean properties */	
	kPGPUserIDProperty_IsAttribute				= 900,
	kPGPUserIDProperty_IsRevoked				= 901,

	/* User ID numeric properties */	
	kPGPUserIDProperty_Validity 				= 1000,
	kPGPUserIDProperty_Confidence				= 1001,
	kPGPUserIDProperty_AttributeType			= 1002,

	/* No User ID time properties */	

	/* User ID data (variable sized) properties */
	kPGPUserIDProperty_Name 					= 1200,
	kPGPUserIDProperty_AttributeData			= 1201,
	kPGPUserIDProperty_CommonName				= 1202,
	kPGPUserIDProperty_EmailAddress				= 1203,
	kPGPUserIDProperty_SMIMEPreferredAlgorithms	= 1204,
	
	/* Signature boolean properties */
	kPGPSigProperty_IsRevoked 					= 1300,
	kPGPSigProperty_IsNotCorrupt				= 1301,
	kPGPSigProperty_IsTried						= 1302,
	kPGPSigProperty_IsVerified					= 1303,
	kPGPSigProperty_IsMySig						= 1304,
	kPGPSigProperty_IsExportable				= 1305,
	kPGPSigProperty_HasUnverifiedRevocation		= 1306,
	kPGPSigProperty_IsExpired					= 1307,
	kPGPSigProperty_IsX509						= 1308,

	/* Signature numeric properties */
	kPGPSigProperty_AlgorithmID					= 1400,
	kPGPSigProperty_TrustLevel					= 1401,
	kPGPSigProperty_TrustValue					= 1402,
	
	/* Signature time properties */
	kPGPSigProperty_Creation 					= 1500,
	kPGPSigProperty_Expiration					= 1501,

	/* Signature data (variable sized) properties */
	kPGPSigProperty_KeyID 						= 1600,
	kPGPSigProperty_X509Certificate				= 1601,
	kPGPSigProperty_X509IASN					= 1602,
	kPGPSigProperty_X509LongName				= 1603,
	kPGPSigProperty_X509IssuerLongName			= 1604,
	kPGPSigProperty_X509DNSName					= 1605,
	kPGPSigProperty_X509IPAddress				= 1606,
	kPGPSigProperty_X509DERDName				= 1607,
	kPGPSigProperty_RegularExpression			= 1608,

	PGP_ENUM_FORCE( PGPKeyDBObjProperty_ )
} ;
PGPENUM_TYPEDEF( PGPKeyDBObjProperty_, PGPKeyDBObjProperty );


/* For kPGPKeyProperty_Version */
enum PGPKeyVersion_
{
	kPGPKeyVersion_V3					= 3,
	kPGPKeyVersion_V4					= 4,
	
	PGP_ENUM_FORCE( PGPKeyVersion_ )
} ;
PGPENUM_TYPEDEF( PGPKeyVersion_, PGPKeyVersion );


/* kPGPKeyPropFlags bits */

enum	/* PGPKeyPropertyFlags */
{
	kPGPKeyPropertyFlags_UsageSignUserIDs			= (1UL << 0 ),
	kPGPKeyPropertyFlags_UsageSignMessages			= (1UL << 1 ),
	kPGPKeyPropertyFlags_UsageEncryptCommunications	= (1UL << 2 ),
	kPGPKeyPropertyFlags_UsageEncryptStorage		= (1UL << 3 ),

	kPGPKeyPropertyFlags_PrivateSplit				= (1UL << 4 ),
	kPGPKeyPropertyFlags_PrivateShared				= (1UL << 7 )
} ;
typedef PGPFlags		PGPKeyPropertyFlags;
#define kPGPKeyPropertyFlags_UsageSign \
					(kPGPKeyPropertyFlags_UsageSignUserIDs | \
					 kPGPKeyPropertyFlags_UsageSignMessages)
#define kPGPKeyPropertyFlags_UsageEncrypt \
					(kPGPKeyPropertyFlags_UsageEncryptCommunications | \
					 kPGPKeyPropertyFlags_UsageEncryptStorage)
#define kPGPKeyPropertyFlags_UsageSignEncrypt \
					(kPGPKeyPropertyFlags_UsageSign | \
					 kPGPKeyPropertyFlags_UsageEncrypt)

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

/* kPGPKeyPropFlags bits */

enum	/* PGPKeyServerPreferencesFlags */
{
	kPGPKeyServerPreferences_NoModify			= (1UL << 7 )
} ;
typedef PGPFlags		PGPKeyServerPreferencesFlags;

/* Implementation features like Modification Detection support */

enum
{
	kPGPKeyFeatures_ModificationDetection	= 1
} ;
typedef PGPFlags		PGPKeyFeaturesFlags;


/* Key DB object types for the property kPGPKeyDBObjProperty_ObjectType */ 
enum PGPKeyDBObjType_
{
	kPGPKeyDBObjType_Invalid	= 0,
	kPGPKeyDBObjType_Key		= (1UL << 0),
	kPGPKeyDBObjType_SubKey		= (1UL << 1),
	kPGPKeyDBObjType_UserID		= (1UL << 2),
	kPGPKeyDBObjType_Signature	= (1UL << 3),
#ifdef __MVS__
	kPGPKeyDBObjType_Any		= -1,
#else
	kPGPKeyDBObjType_Any		= 0xFFFFFFFF,
#endif

	PGP_ENUM_FORCE( PGPKeyDBObjType_ )
} ;
PGPENUM_TYPEDEF( PGPKeyDBObjType_, PGPKeyDBObjType );
	
/*
 * Used by filtering functions to specify type of match.
 */

enum PGPMatchCriterion_
{
	kPGPMatchCriterion_Equal 			= 1,	/* searched == val */
	kPGPMatchCriterion_GreaterOrEqual	= 2,	/* searched >= val */
	kPGPMatchCriterion_LessOrEqual 		= 3,	/* searched <= val */
	kPGPMatchCriterion_SubString 		= 4,	/* searched is contained in supplied */

	PGP_ENUM_FORCE( PGPMatchCriterion_ )
} ;
PGPENUM_TYPEDEF( PGPMatchCriterion_, PGPMatchCriterion );


/* This is the value of the expiration time which means "never expires" */
#define kPGPExpirationTime_Never	( (PGPTime)0 )

enum PGPOpenKeyDBFileOptions_
{
	kPGPOpenKeyDBFileOptions_None		= 0,
	kPGPOpenKeyDBFileOptions_Mutable	= (1UL << 0 ),
	kPGPOpenKeyDBFileOptions_Create		= (1UL << 1 ),

	PGP_ENUM_FORCE( PGPOpenKeyDBFileOptions_ )
} ;

PGPENUM_TYPEDEF( PGPOpenKeyDBFileOptions_, PGPOpenKeyDBFileOptions );

/* Public entry points */

PGP_BEGIN_C_DECLARATIONS

/*____________________________________________________________________________
	Key DB functions
____________________________________________________________________________*/

/* Creat a new, in-memory temporary key DB */
PGPError	PGPNewKeyDB( PGPContextRef context, PGPKeyDBRef *keyDBRef );

/* Open a (possibly) existing key ring pair on disk */
PGPError	PGPOpenKeyDBFile( PGPContextRef context,
					PGPOpenKeyDBFileOptions options,
					PGPFileSpecRef pubKeysFileSpec,
					PGPFileSpecRef privKeysFileSpec,
					PGPKeyDBRef *keyDBRef );
 
PGPError	PGPFreeKeyDB( PGPKeyDBRef keyDBRef );

PGPError	PGPFlushKeyDB( PGPKeyDBRef keyDBRef );

PGPError	PGPIncKeyDBRefCount( PGPKeyDBRef keyDBRef );

PGPBoolean	PGPKeyDBIsMutable( PGPKeyDBRef keyDBRef );

PGPError 	PGPFindKeyByKeyID( PGPKeyDBRef keyDBRef, const PGPKeyID * keyID,
					PGPKeyDBObjRef *keyRef);

PGPError 	PGPCountKeysInKeyDB( PGPKeyDBRef keyDBRef, PGPUInt32 *numKeys );

PGPError	PGPKeyDBIsUpdated( PGPKeyDBRef keyDBRef, PGPBoolean *isUpdated );

/* Cache a keydb in memory for specified number of seconds */
PGPError	PGPCacheKeyDB( PGPKeyDBRef keyDBRef, PGPUInt32 timeoutSeconds );

/* Remove all cached keydbs from memory */
PGPError	PGPPurgeKeyDBCache( PGPContextRef context );

/*____________________________________________________________________________
	Key set functions
____________________________________________________________________________*/

/* Create a new key set containing all of the keys in the key DB */
PGPError  	PGPNewKeySet( PGPKeyDBRef keyDB, PGPKeySetRef *keySet );
						
/* Create a new, empty key set */
PGPError  	PGPNewEmptyKeySet( PGPKeyDBRef keyDB, PGPKeySetRef *keySet );

/* Create a new key set containing a single key */
PGPError  	PGPNewOneKeySet( PGPKeyDBObjRef key, PGPKeySetRef *keySet );

/* Like PGPNewKeySet but allows certain stub key objects */
PGPError	PGPNewEmptyInclusiveKeySet( PGPKeyDBRef keyDB, PGPKeySetRef *pset );

/* Like PGPNewOneKeySet but allows certain stub key objects */
PGPError  	PGPNewOneInclusiveKeySet( PGPKeyDBObjRef key, PGPKeySetRef *keySet );

PGPError 	PGPFreeKeySet( PGPKeySetRef keys);

PGPError 	PGPIncKeySetRefCount( PGPKeySetRef keys);

PGPBoolean 	PGPKeySetIsMember( PGPKeyDBObjRef key, PGPKeySetRef set );

PGPError 	PGPCountKeys( PGPKeySetRef keys, PGPUInt32 *numKeys );

PGPError 	PGPAddKey( PGPKeyDBObjRef keyToAdd, PGPKeySetRef set );

PGPError 	PGPAddKeys( PGPKeySetRef keysToAdd, PGPKeySetRef set );

PGPKeyDBRef		PGPPeekKeySetKeyDB( PGPKeySetRef keySet );
PGPKeySetRef	PGPPeekKeyDBRootKeySet( PGPKeyDBRef keyDB );

/*____________________________________________________________________________
	Key DB object properties
____________________________________________________________________________*/

PGPError	PGPGetKeyDBObjBooleanProperty( PGPKeyDBObjRef key,
					PGPKeyDBObjProperty whichProperty, PGPBoolean *prop );

PGPError 	PGPGetKeyDBObjNumericProperty( PGPKeyDBObjRef key,
					PGPKeyDBObjProperty whichProperty, PGPInt32 *prop );

PGPError 	PGPGetKeyDBObjTimeProperty( PGPKeyDBObjRef key,
					PGPKeyDBObjProperty whichProperty, PGPTime *prop);
/*
**	Get the data for a binary property. Returns kPGPError_BufferTooSmall if
**	the buffer is too small. Both buffer and dataSize can be NULL.
*/

#undef		PGPGetKeyDBObjDataProperty
PGPError 	PGPGetKeyDBObjDataProperty( PGPKeyDBObjRef key,
					PGPKeyDBObjProperty whichProperty, void *buffer,
					PGPSize bufferSize, PGPSize *dataSize);
/*
**	Get the data for a binary property using an allocated output buffer. The
**	allocated buffer must be freed with PGPFreeData(). For convenience, the
**	allocated buffer is null-terminated. The terminating null byte is NOT included
**	is the output dataSize parameter.
*/

PGPError 	PGPGetKeyDBObjAllocatedDataProperty( PGPKeyDBObjRef key,
					PGPKeyDBObjProperty whichProperty, void **buffer,
					PGPSize *dataSize);

PGPError 	PGPSetKeyEnabled( PGPKeyDBObjRef key, PGPBoolean enable );

PGPError	PGPSetKeyAxiomatic( PGPKeyDBObjRef key, PGPBoolean setAxiomatic,
					PGPOptionListRef firstOption, ...);

/*____________________________________________________________________________
	Key DB object property convenience functions
____________________________________________________________________________*/

/* Get the key ID of a key or subkey key DB object */
PGPError	PGPGetKeyID( PGPKeyDBObjRef key, PGPKeyID *keyID );

PGPError  	PGPGetPrimaryUserID( PGPKeyDBObjRef key, PGPKeyDBObjRef *outRef );

PGPError	PGPGetPrimaryAttributeUserID (PGPKeyDBObjRef key,
						PGPAttributeType attributeType, PGPKeyDBObjRef *outRef);

PGPError 	PGPGetPrimaryUserIDValidity(PGPKeyDBObjRef key,
				PGPValidity *validity);

PGPError 	PGPGetPrimaryUserIDName(PGPKeyDBObjRef key, void *buffer,
					PGPSize bufferSize, PGPSize *dataSize);

PGPError 	PGPGetKeyForUsage( PGPKeyDBObjRef key, PGPUInt32 usageFlags,
					PGPKeyDBObjRef *outRef );


				
/*____________________________________________________________________________
	Key filters
____________________________________________________________________________*/

PGPError 	PGPNewKeyDBObjBooleanFilter( PGPContextRef context,
					PGPKeyDBObjProperty whichProperty, PGPBoolean match,
					PGPFilterRef *outFilter );

PGPError 	PGPNewKeyDBObjNumericFilter( PGPContextRef context,
					PGPKeyDBObjProperty whichProperty, PGPUInt32 matchValue,
					PGPMatchCriterion matchCriteria, PGPFilterRef *outFilter );

PGPError 	PGPNewKeyDBObjTimeFilter( PGPContextRef context,
					PGPKeyDBObjProperty whichProperty, PGPTime matchValue,
					PGPMatchCriterion matchCriteria, PGPFilterRef *outFilter );

PGPError 	PGPNewKeyDBObjDataFilter( PGPContextRef context,
					PGPKeyDBObjProperty whichProperty, const void *matchData,
					PGPSize matchDataSize, PGPMatchCriterion matchCriteria,
					PGPFilterRef *outFilter );

PGPError 	PGPFreeFilter( PGPFilterRef filter );

PGPError 	PGPIncFilterRefCount( PGPFilterRef filter );

PGPError 	PGPFilterChildObjects( PGPFilterRef filter,
					PGPBoolean filterChildren );

/* freeing outfilter will call PGPFreeFilter on filter */
PGPError 	PGPNegateFilter( PGPFilterRef filter, PGPFilterRef *outFilter);

/* freeing outfilter will call PGPFreeFilter on filter1, filter2 */
PGPError 	PGPIntersectFilters( PGPFilterRef filter1, PGPFilterRef filter2,
					PGPFilterRef *outFilter);

/* freeing outfilter will call PGPFreeFilter on filter1, filter2 */
PGPError 	PGPUnionFilters( PGPFilterRef filter1, PGPFilterRef filter2,
					PGPFilterRef *outFilter);

PGPError 	PGPFilterKeySet( PGPKeySetRef origSet, PGPFilterRef filter,
					PGPKeySetRef *resultSet );

PGPError	PGPFilterKeyDB( PGPKeyDBRef keyDB, PGPFilterRef filter,
					PGPKeySetRef *resultSet );

/* Keyserver filter functions */
#undef		PGPLDAPQueryFromFilter
PGPError 	PGPLDAPQueryFromFilter( PGPFilterRef filter, PGPChar8 **queryOut );

#undef		PGPLDAPX509QueryFromFilter
PGPError	PGPLDAPX509QueryFromFilter( PGPFilterRef filter,
					PGPChar8 **queryOut );

#undef		PGPHKSQueryFromFilter
PGPError 	PGPHKSQueryFromFilter( PGPFilterRef	filter, PGPChar8 **queryOut );

#undef		PGPNetToolsCAHTTPQueryFromFilter
PGPError	PGPNetToolsCAHTTPQueryFromFilter( PGPFilterRef filter,
					PGPChar8 **queryOut );

/*____________________________________________________________________________
	Key/signature validation
____________________________________________________________________________*/

PGPError	PGPCalculateTrust( PGPKeySetRef keySet, PGPKeyDBRef optionalKeyDB);

PGPError 	PGPCheckKeyRingSigs( PGPKeySetRef keysToCheck,
					PGPKeyDBRef optionalSigningKeyDB, PGPBoolean checkAll,
					PGPEventHandlerProcPtr eventHandler,
					PGPUserValue eventHandlerData );

/*____________________________________________________________________________
	Key DB object creation/deletion
____________________________________________________________________________*/

PGPError	PGPGenerateKey( PGPContextRef context, PGPKeyDBObjRef *key,
					PGPOptionListRef firstOption, ...);
					
PGPError	PGPGenerateSubKey( PGPContextRef context, PGPKeyDBObjRef *subkey,
					PGPOptionListRef firstOption, ...);
					
PGPUInt32	PGPGetKeyEntropyNeeded( PGPContextRef context,
					PGPOptionListRef firstOption, ...);

#undef		PGPAddUserID
PGPError	PGPAddUserID( PGPKeyDBObjRef key, PGPChar8 const *userID,
					PGPOptionListRef firstOption, ...);

PGPError	PGPAddAttributeUserID( PGPKeyDBObjRef key,
					PGPAttributeType attributeType,
					PGPByte *attributeData, PGPSize attributeLength,
					PGPOptionListRef firstOption, ...);
						
PGPError	PGPCertifyUserID( PGPKeyDBObjRef userID,
					PGPKeyDBObjRef certifyingKey,
					PGPOptionListRef firstOption, ...);

PGPError 	PGPCopyKeyDBObj( PGPKeyDBObjRef keyDBObj, PGPKeyDBRef destKeyDB,
					PGPKeyDBObjRef *destKeyDBObj );

PGPError 	PGPCopyKeys( PGPKeySetRef keySet, PGPKeyDBRef destKeyDB,
					PGPKeySetRef *destKeySet );

PGPError 	PGPDeleteKeyDBObj( PGPKeyDBObjRef keyDBObj );

PGPError 	PGPDeleteKeys( PGPKeySetRef keySet );

/*____________________________________________________________________________
	Key manipulation
____________________________________________________________________________*/

PGPError 	PGPSetPrimaryUserID( PGPKeyDBObjRef userID,
					PGPOptionListRef firstOption, ...);

PGPError 	PGPGetSigCertifierKey( PGPKeyDBObjRef cert,
					PGPKeyDBRef searchKeyDB, PGPKeyDBObjRef *certkey);

PGPError 	PGPGetSigX509CertifierSig( PGPKeyDBObjRef cert,
					PGPKeyDBRef searchKeyDB, PGPKeyDBObjRef *certsig);

PGPError	PGPGetSigX509TopSig( PGPKeyDBObjRef sig,
				    PGPKeyDBRef otherdb, PGPKeyDBRef cadb, 
					PGPBoolean *knownCA, PGPKeyDBObjRef *certsig );

PGPError	PGPGetKnownX509CAs( PGPContextRef context, PGPKeyDBRef *keydbout );

PGPError	PGPX509MatchNetworkName( PGPKeyDBObjRef sig, const PGPByte *networkName,
					PGPBoolean *matched );

PGPError	PGPCountAdditionalRecipientRequests( PGPKeyDBObjRef basekey,
					PGPUInt32 * numARKeys);
					
PGPError 	PGPGetIndexedAdditionalRecipientRequestKey( PGPKeyDBObjRef basekey,
					PGPUInt32 nth, PGPKeyDBObjRef *arkey,
					PGPKeyID *arkeyid, PGPByte *arclass );

PGPError	PGPGetAdditionalRecipientRequests( PGPKeyDBObjRef basekey,
					PGPKeySetRef keySet );

PGPError	PGPCountRevocationKeys( PGPKeyDBObjRef basekey,
					PGPUInt32 * numRevKeys);

PGPError 	PGPGetIndexedRevocationKey( PGPKeyDBObjRef basekey, PGPUInt32 nth,
					PGPKeyDBObjRef *revkey, PGPKeyID *revkeyid );
						
PGPError 	PGPGetRevocationKeys( PGPKeyDBObjRef basekey, PGPKeySetRef keySet );

PGPError	PGPGetCRLDistributionPoints( PGPKeyDBObjRef cakey,
					PGPUInt32 *pnDistPoints, PGPByte **pDpoints,
					PGPSize **pdpointLengths );

PGPError	PGPGetCRLDistributionPointsPrintable( PGPKeyDBObjRef cakey,
				    PGPUInt32 *pnDistPoints, PGPByte **pDpoints );

PGPError	PGPExport( PGPContextRef context,
					PGPOptionListRef firstOption, ... );

PGPError	PGPImport( PGPContextRef context, PGPKeyDBRef *importedKeysDB,
					PGPOptionListRef firstOption, ...);

PGPError	PGPRevokeSig( PGPKeyDBObjRef cert,
					PGPOptionListRef firstOption, ...);
					
PGPError	PGPRevoke( PGPKeyDBObjRef key,
					PGPOptionListRef firstOption, ...);
					
PGPError	PGPChangePassphrase( PGPKeyDBObjRef key,
					PGPOptionListRef firstOption, ...);
					
PGPBoolean	PGPPassphraseIsValid( PGPKeyDBObjRef key,
					PGPOptionListRef firstOption, ...);

PGPError	PGPPurgePassphraseCache( PGPContextRef context );

PGPError	PGPCountCachedPassphrases( PGPContextRef context,
					PGPUInt32 *pnLocal, PGPUInt32 *pnGlobal,
					PGPUInt32 *pnOtherLocal );

PGPBoolean	PGPTokenAuthIsValid( PGPContextRef context, 
					PGPOptionListRef firstOption, ... );

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

PGPError 	PGPSetKeyTrust( PGPKeyDBObjRef key, PGPUInt32 trust);

PGPInt32 	PGPCompareKeys( PGPKeyDBObjRef a, PGPKeyDBObjRef b,
						PGPKeyOrdering order );

#undef		PGPCompareUserIDStrings
PGPInt32 	PGPCompareUserIDStrings(PGPChar8 const *a, PGPChar8 const *b);

/*____________________________________________________________________________
	Key lists
____________________________________________________________________________*/

PGPError  	PGPOrderKeySet( PGPKeySetRef src, PGPKeyOrdering order,
						PGPBoolean reverseOrder, PGPKeyListRef *outRef );

PGPError 	PGPIncKeyListRefCount( PGPKeyListRef keys);

PGPError 	PGPFreeKeyList( PGPKeyListRef keys );

/*____________________________________________________________________________
	Key list iteration
____________________________________________________________________________*/

PGPError 	PGPNewKeyIter( PGPKeyListRef keys, PGPKeyIterRef *outRef);

PGPError 	PGPNewKeyIterFromKeySet( PGPKeySetRef keys, PGPKeyIterRef *outRef);

PGPError 	PGPNewKeyIterFromKeyDB( PGPKeyDBRef keyDB, PGPKeyIterRef *outRef);

PGPError 	PGPCopyKeyIter( PGPKeyIterRef orig, PGPKeyIterRef *outRef);
	
PGPError 	PGPFreeKeyIter( PGPKeyIterRef iter);

PGPInt32 	PGPKeyIterIndex( PGPKeyIterRef iter);

PGPError 	PGPKeyIterRewind( PGPKeyIterRef iter, PGPKeyDBObjType objectType);

PGPInt32 	PGPKeyIterSeek( PGPKeyIterRef iter, PGPKeyDBObjRef key);

PGPError 	PGPKeyIterMove( PGPKeyIterRef iter, PGPInt32 relOffset,
					PGPKeyDBObjRef *outRef);

PGPError 	PGPKeyIterNextKeyDBObj( PGPKeyIterRef iter,
					PGPKeyDBObjType objectType, PGPKeyDBObjRef *outRef);

PGPError 	PGPKeyIterPrevKeyDBObj( PGPKeyIterRef iter,
					PGPKeyDBObjType objectType, PGPKeyDBObjRef *outRef);

PGPError 	PGPKeyIterGetKeyDBObj( PGPKeyIterRef iter,
					PGPKeyDBObjType objectType, PGPKeyDBObjRef *outRef);


/*____________________________________________________________________________
	Get/set user value
____________________________________________________________________________*/

PGPError 	PGPSetKeyDBObjUserValue( PGPKeyDBObjRef key,
					PGPUserValue userValue);

PGPError 	PGPGetKeyDBObjUserValue( PGPKeyDBObjRef key,
					PGPUserValue *userValue);

/* Passphrase conversion to passkeybuffer */
/* The size of the output buffer is from the kPGPKeyPropLockingBits property */

PGPError	PGPGetPasskeyBuffer ( PGPKeyDBObjRef key,
					void *passkeyBuffer, PGPOptionListRef firstOption,...);

/* Change key options which are stored in self signatures internally */

PGPError	PGPAddKeyOptions( PGPKeyDBObjRef key,
					PGPOptionListRef firstOption, ...);

PGPError	PGPRemoveKeyOptions( PGPKeyDBObjRef key,
					PGPOptionListRef firstOption, ...);

PGPError	PGPUpdateKeyOptions( PGPKeyDBObjRef key,
					PGPOptionListRef firstOption, ...);

/*____________________________________________________________________________
	Key IDs
____________________________________________________________________________*/

PGPError 	PGPNewKeyID( const PGPByte *keyIDBytes, PGPSize numKeyIDBytes,
					PGPPublicKeyAlgorithm pkalg, PGPKeyID *id );
#undef		PGPNewKeyIDFromString
PGPError 	PGPNewKeyIDFromString( const PGPChar8 *string,
					PGPPublicKeyAlgorithm pkalg, PGPKeyID *id );
PGPError 	PGPGetKeyIDAlgorithm( const PGPKeyID *keyID,
					PGPPublicKeyAlgorithm *pkalg );
					
enum PGPKeyIDStringType_
{
	kPGPKeyIDString_Abbreviated 	= 1,
	kPGPKeyIDString_Full 			= 2,
	
	PGP_ENUM_FORCE( PGPKeyIDStringType_ )
};
PGPENUM_TYPEDEF( PGPKeyIDStringType_, PGPKeyIDStringType );


#define kPGPMaxKeyIDStringSize		( 127 + 1 )
#undef		PGPGetKeyIDString
PGPError 	PGPGetKeyIDString( PGPKeyID const * ref, PGPKeyIDStringType type,
					PGPChar8 outString[ kPGPMaxKeyIDStringSize ] );

/* returns 0 if equal, -1 if key1 < key2, 1 if key1 > key2 */
PGPInt32 	PGPCompareKeyIDs( PGPKeyID const * key, PGPKeyID const * key2);

/*____________________________________________________________________________
	Token functions
____________________________________________________________________________*/

PGPError	PGPCountTokens( PGPContextRef context, PGPUInt32 *numTokens );

PGPError	PGPDeleteKeyOnToken( PGPKeyDBObjRef key, 
					PGPUInt32 tokNumber /*-1 for any token*/, 
					const PGPByte *pin, PGPSize pinLen );
PGPError	PGPWipeToken( PGPContextRef context, PGPUInt32 tokNumber,
					PGPByte const *passphrase, PGPSize passphraseLength );
PGPError	PGPFormatToken( PGPContextRef context, PGPUInt32 tokNumber,
					PGPByte const *	adminPin,	PGPSize	adminPinLen, 
					PGPByte const *	newUserPin,	PGPSize	newUserPinLen );
PGPError	PGPTokenPassphraseIsValid( PGPContextRef context,
					PGPUInt32 tokNumber, PGPByte const *passphrase,
					PGPSize passphraseLength );

#undef		PGPSetPKCS11DrvFile
PGPError    PGPSetPKCS11DrvFile( PGPChar8 *module );

PGPError    PGPSyncTokenKeys( PGPContextRef context, PGPUInt32 tokenNum, 
					PGPKeyDBRef destKeyDB, PGPBoolean *hadChanges );

PGPError	PGPGetTokenInfoBooleanProperty( PGPContextRef context,
					PGPUInt32 tokenNumber, PGPTokenProperty prop,
					PGPBoolean *value );
PGPError	PGPGetTokenInfoNumericProperty( PGPContextRef context,
					PGPUInt32 tokenNumber, PGPTokenProperty prop,
					PGPUInt32 *value );
#undef		PGPGetTokenInfoDataProperty
PGPError	PGPGetTokenInfoDataProperty( PGPContextRef context,
					PGPUInt32 tokenNumber, PGPTokenProperty prop,
					void *value, PGPSize size, PGPSize *sizeout );

/* Deprecated, use PGPGetTokenInfo*() */
PGPError	PGPGetTokenInfo( PGPContextRef context, 
                    PGPUInt32 tokNumber, PGPTokenInfo *tokenInfo);

/*____________________________________________________________________________
	Getting contexts back from key related items.
____________________________________________________________________________*/

PGPContextRef	PGPPeekKeyDBContext( PGPKeyDBRef ref );
PGPContextRef	PGPPeekKeyDBObjContext( PGPKeyDBObjRef ref );
PGPContextRef	PGPPeekKeyListContext( PGPKeyListRef ref );
PGPContextRef	PGPPeekKeySetContext( PGPKeySetRef ref );
PGPContextRef	PGPPeekKeyIterContext( PGPKeyIterRef ref );

/*____________________________________________________________________________
	Getting parent objects from key related items.  If the input is invalid,
	you get kInvalidPGPKeyDBObjRef back.
____________________________________________________________________________*/

PGPKeyDBRef		PGPPeekKeyDBObjKeyDB( PGPKeyDBObjRef ref );
PGPKeyDBObjRef	PGPPeekKeyDBObjKey( PGPKeyDBObjRef ref );
PGPKeyDBObjRef	PGPPeekKeyDBObjUserID( PGPKeyDBObjRef ref );

/*____________________________________________________________________________
    Secret sharing functionality
____________________________________________________________________________*/

PGPError	PGPSecretShareData(PGPContextRef context,
					void const * input, PGPSize inputBytes,
					PGPUInt32 threshold, PGPUInt32 nShares, void * output);

PGPError	PGPSecretReconstructData(PGPContextRef context,
					void * input, PGPSize outputBytes,
					PGPUInt32 nShares, void * output);

/*____________________________________________________________________________
    X509 certificate specific
____________________________________________________________________________*/

PGPError	PGPVerifyX509CertificateChain (PGPContextRef context,
					PGPByte *certchain, PGPByte *rootcerts);

#undef		PGPCreateDistinguishedName
PGPError	PGPCreateDistinguishedName( PGPContextRef context,
					PGPChar8 const *str, PGPByte **pdname, PGPSize *pdnamelen );

PGPError	PGPCreateX509Certificate(PGPKeyDBObjRef signingSig,
					PGPKeyDBObjRef signedKey, PGPKeyDBObjRef *newSig,
					PGPOptionListRef firstOption, ...);

/* Pass PKCS-10 format request in PGPOInput.... */
PGPError	PGPCreateX509CertificateFromRequest(PGPKeyDBObjRef signingSig,
					PGPKeyDBObjRef *newSig, PGPOptionListRef firstOption, ...);

PGPError	PGPCreateSelfSignedX509Certificate(PGPKeyDBObjRef signingKey,
					PGPKeyDBObjRef *newSig, PGPOptionListRef firstOption, ...);

PGPError	PGPCreateX509CRL(PGPKeyDBObjRef signingKey,
					PGPKeySetRef revokedSigs,
					PGPOptionListRef firstOption, ...);



PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpKeys_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

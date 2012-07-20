/*____________________________________________________________________________
	pgpErrors.h

	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	Error codes for all PGP errors can be found in this file.

	$Id: pgpErrors.h,v 1.96 1999/05/11 19:01:17 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpErrors_h	/* [ */
#define Included_pgpErrors_h

#include "pgpPubTypes.h"


#include "pgpPFLErrors.h"

#define kPGPErrorRange		1000

enum PGPError_
{
	/*
		NOTE: error code values must not be changed;
		compiled client code depends on them.
	*/
	kPGPError_FirstError		= -11500,
	kPGPError_Last				= -10500,

	kPGPError_BadPassphrase		= -11500,
	kPGPError_OptionNotFound	= -11499,

/* Errors from pgpEncode */
	kPGPError_RedundantOptions	= -11498,
	kPGPError_KeyRevoked		= -11497,
	kPGPError_KeyExpired		= -11496,
	kPGPError_KeyDisabled		= -11495,
	kPGPError_KeyInvalid		= -11494,
	kPGPError_KeyUnusableForEncryption			= -11493,
	kPGPError_KeyUnusableForSignature			= -11492,
	kPGPError_OutputBufferTooSmall				= -11491,
	kPGPError_InconsistentEncryptionAlgorithms	= -11490, 
	kPGPError_MissingPassphrase					= -11489,
	kPGPError_CombinedConventionalAndPublicEncryption= -11488, 
	kPGPError_DetachedSignatureWithoutSigningKey= -11487, 
	kPGPError_DetachedSignatureWithEncryption	= -11486, 
	kPGPError_NoInputOptions					= -11485,
	kPGPError_MultipleInputOptions				= -11484,
	kPGPError_InputFile							= -11483,
	kPGPError_NoOutputOptions					= -11482,
	kPGPError_MultipleOutputOptions				= -11481,
	kPGPError_MissingEventHandler				= -11480,
	kPGPError_MissingKeySet						= -11479,
	kPGPError_DetachedSignatureFound			= -11478,
	kPGPError_NoDecryptionKeyFound				= -11477,
	kPGPError_CorruptSessionKey					= -11476,
	kPGPError_SkipSection						= -11475,
	kPGPError_Interrupted						= -11474,
	kPGPError_TooManyARRKs						= -11473,
	kPGPError_KeyUnusableForDecryption			= -11472,
	
	kPGPError_IncompatibleAPI					= -11460,

/* misc errors */
	kPGPError_NotMacBinary						= -11450,
	kPGPError_NoMacBinaryTranslationAvailable	= -11449,
	kPGPError_BadSignature						= -11448,
	kPGPError_CAPIUnsupportedKey				= -11447,
	kPGPError_SelfTestFailed					= -11446,
	kPGPError_SelfTestsNotExecuted				= -11445,
	kPGPError_BadIntegrity						= -11444,
	kPGPError_DeCompressionFailed				= -11443,
	
/* filter errors */

	kPGPError_InconsistentFilterClasses			= -11440,
	kPGPError_UnsupportedLDAPFilter				= -11439,
	kPGPError_UnsupportedHKPFilter				= -11438,
	kPGPError_UnknownFilterType					= -11437,
	kPGPError_InvalidFilterParameter			= -11436,
	kPGPError_UnsupportedNetToolsCAFilter		= -11435,
	
/* old errors: */

	kPGPError_OutOfRings						= -11420,
	kPGPError_BadHashNumber						= -11419,
	kPGPError_BadCipherNumber					= -11418,
	kPGPError_BadKeyLength						= -11417,
	kPGPError_SizeAdviseFailure					= -11416,
	kPGPError_ConfigParseFailure				= -11415,
	kPGPError_ConfigParseFailureBadFunction		= -11414,
	kPGPError_ConfigParseFailureBadOptions		= -11413,
	kPGPError_KeyIsLocked						= -11412,
	kPGPError_CantDecrypt						= -11411,
	kPGPError_UnknownString2Key					= -11410,
	kPGPError_BadSessionKeySize					= -11409,
	kPGPError_UnknownVersion					= -11408,
	kPGPError_BadSessionKeyAlgorithm			= -11407,
	kPGPError_UnknownSignatureType				= -11406,
	kPGPError_BadSignatureSize					= -11405,
	kPGPError_SignatureBitsWrong				= -11404,
	kPGPError_ExtraDateOnSignature				= -11403,
	kPGPError_SecretKeyNotFound					= -11402,
	kPGPError_AdditionalRecipientRequestKeyNotFound	= -11401,
	kPGPError_InvalidCommit						= -11400,
	kPGPError_CantHash							= -11399,
	kPGPError_UnbalancedScope					= -11398,
	kPGPError_WrongScope						= -11397,
	kPGPError_FIFOReadError						= -11396,
	kPGPError_RandomSeedTooSmall				= -11395,
	kPGPError_EnvPriorityTooLow					= -11394,
	kPGPError_UnknownCharMap					= -11393,
	kPGPError_AsciiParseIncomplete				= -11392,
	kPGPError_BadPacket							= -11391,

	kPGPError_TroubleKeySubKey					= -11390,
	kPGPError_TroubleSigSubKey					= -11389,
	kPGPError_TroubleBadTrust					= -11388,
	kPGPError_TroubleUnknownPacketByte			= -11387,
	kPGPError_TroubleUnexpectedSubKey			= -11386,
	kPGPError_TroubleUnexpectedName				= -11385,
	kPGPError_TroubleUnexpectedSignature		= -11384,
	kPGPError_TroubleUnexpectedUnknown			= -11383,
	kPGPError_TroubleUnexpectedTrust			= -11382,
	kPGPError_TroubleKeyTooBig					= -11381,
	kPGPError_TroubleSecretKeyTooBig			= -11380,
	kPGPError_TroubleNameTooBig					= -11379,
	kPGPError_TroubleSignatureTooBig			= -11378,
	kPGPError_TroubleUnknownTooBig				= -11377,
	kPGPError_TroubleDuplicateKeyID				= -11376,
	kPGPError_TroubleDuplicateKey				= -11375,
	kPGPError_TroubleDuplicateSecretKey			= -11374,
	kPGPError_TroubleDuplicateName				= -11373,
	kPGPError_TroubleDuplicateSignature			= -11372,
	kPGPError_TroubleDuplicateUnknown			= -11371,
	kPGPError_TroubleBareKey					= -11370,
	kPGPError_TroubleVersionBugPrev				= -11369,
	kPGPError_TroubleVersionBugCur				= -11368,
	kPGPError_TroubleOldSecretKey				= -11367,
	kPGPError_TroubleNewSecretKey				= -11366,
	kPGPError_TroubleImportingNonexportableSignature= -11365,
	kPGPError_TroubleDuplicateCRL				= -11364,
	kPGPError_TroubleCRLTooBig					= -11363,

	/*
	 * The set of errors in this range are the ones which will NOT abort
	 * a keyring check operation.  These errors just make us skip the key
	 * and go on to the next.
	 */
kPGPError_KEY_MIN							= -11350,
	kPGPError_KEY_LONG							= kPGPError_KEY_MIN,
	kPGPError_KeyPacketTruncated				= -11349,
	kPGPError_UnknownKeyVersion					= -11348,
	kPGPError_UnknownPublicKeyAlgorithm			= -11347,
	kPGPError_MalformedKeyModulus				= -11346,
	kPGPError_MalformedKeyExponent				= -11345,
	kPGPError_RSAPublicModulusIsEven			= -11344,
	kPGPError_RSAPublicExponentIsEven			= -11343,
	kPGPError_MalformedKeyComponent				= -11342,
	kPGPError_KeyTooLarge						= -11341,
	kPGPError_PublicKeyTooSmall					= -11340,
	kPGPError_PublicKeyTooLarge					= -11339,
	kPGPError_PublicKeyUnimplemented			= -11338,
	kPGPError_CRLPacketTruncated				= -11337,
kPGPError_KEY_MAX				= kPGPError_PublicKeyUnimplemented,


/* kPGPError_SIG_MAX */
	kPGPError_SIG_LONG							= -11330,
	kPGPError_TruncatedSignature				= -11329,
	kPGPError_MalformedSignatureInteger			= -11328,
	kPGPError_UnknownSignatureAlgorithm			= -11327,
	kPGPError_ExtraSignatureMaterial			= -11326,
	kPGPError_UnknownSignatureVersion			= -11325,
	kPGPError_RevocationKeyNotFound				= -11324,
/* kPGPError_SIG_MIN */

/*  kPGPError_KEYDB_MAX */
	kPGPError_OutOfEntropy						= -11320,
	kPGPError_ItemIsReadOnly					= -11319,
	kPGPError_InvalidProperty					= -11318,
	kPGPError_FileCorrupt						= -11317,
	kPGPError_DuplicateCert						= -11316,
	kPGPError_DuplicateUserID					= -11315,
	kPGPError_CertifyingKeyDead					= -11314,
	kPGPError_ItemWasDeleted					= -11313,
	kPGPError_KeyDBMismatch						= -11312,
/* kPGPError_KEYDB_MIN = kPGPError_KeyDBMismatch */

/* kPGPError_SERVER_MAX */
	kPGPError_ServerInProgress					= -11300,
	kPGPError_ServerOperationNotSupported		= -11299,
	kPGPError_ServerInvalidProtocol				= -11298,
	kPGPError_ServerRequestFailed				= -11297,
	kPGPError_ServerOpen						= -11296,
	kPGPError_ServerNotOpen						= -11295,
	kPGPError_ServerKeyAlreadyExists			= -11294,
	kPGPError_ServerNotInitialized				= -11293,
	kPGPError_ServerPartialAddFailure			= -11292,
	kPGPError_ServerCorruptKeyBlock				= -11291,
	kPGPError_ServerUnknownResponse				= -11290,
	kPGPError_ServerTimedOut					= -11289,
	kPGPError_ServerOpenFailed					= -11288,
	kPGPError_ServerAuthorizationRequired		= -11287,
	kPGPError_ServerAuthorizationFailed			= -11286,
	kPGPError_ServerSearchFailed				= -11285,
	kPGPError_ServerPartialSearchResults		= -11284,
	kPGPError_ServerBadKeysInSearchResults		= -11283,
	kPGPError_ServerKeyFailedPolicy				= -11282,
	kPGPError_ServerOperationRequiresTLS		= -11281,
	kPGPError_ServerNoStaticStorage				= -11280,
	kPGPError_ServerCertNotFound				= -11279,

/* TLS errors */
	kPGPError_TLSUnexpectedClose				= -11250,
	kPGPError_TLSProtocolViolation				= -11249,
	kPGPError_TLSVersionUnsupported				= -11248,
	kPGPError_TLSWrongState						= -11247,
	kPGPError_TLSAlertReceived					= -11246,
	kPGPError_TLSKeyUnusable					= -11245,
	kPGPError_TLSNoCommonCipher					= -11244,
	kPGPError_TLSWouldBlock						= -11243,
	kPGPError_TLSRcvdHandshakeRequest			= -11242,

/* X509 certificate errors */
	kPGPError_X509NeededCertNotAvailable		= -11240,
	kPGPError_X509SelfSignedCert				= -11239,
	kPGPError_X509InvalidCertificateSignature	= -11238,
	kPGPError_X509InvalidCertificateFormat		= -11237,

	kPGPError_BigNumNoInverse					= -11150,
	
/* PGPSockets errors */
	kPGPError_SocketsNetworkDown				= -11100,
	kPGPError_SocketsNotInitialized				= -11099,
	kPGPError_SocketsInProgress					= -11098,
	kPGPError_SocketsNotConnected				= -11097,
	kPGPError_SocketsNotBound					= -11096,
	kPGPError_SocketsOperationNotSupported		= -11095,
	kPGPError_SocketsProtocolNotSupported		= -11094,
	kPGPError_SocketsAddressFamilyNotSupported	= -11093,
	kPGPError_SocketsNotASocket					= -11092,
	kPGPError_SocketsAddressInUse				= -11091,
	kPGPError_SocketsBufferOverflow				= -11090,
	kPGPError_SocketsListenQueueFull			= -11089,
	kPGPError_SocketsAddressNotAvailable		= -11088,
	kPGPError_SocketsAlreadyConnected			= -11087,
	kPGPError_SocketsTimedOut					= -11086,
	kPGPError_SocketsNoStaticStorage			= -11085,
	
	kPGPError_SocketsHostNotFound				= -11050,
	kPGPError_SocketsDomainServerError			= -11049,

/* Errors from X.509 layer */
    kPGPError_X509AttributeNotSupported			= -10999,
    kPGPError_InvalidPKCS7Encoding				= -10998,
    kPGPError_CMSInitialization					= -10997,
    kPGPError_InvalidDistinguishedName			= -10996,
    kPGPError_CertRequestCreationFailure		= -10995,
    kPGPError_MissingX509Certificate			= -10994,
    kPGPError_PKCS7SignFailure					= -10993,
    kPGPError_ASNPackFailure					= -10992,
    kPGPError_InvalidInputFormat				= -10991,
    kPGPError_InvalidOutputFormat				= -10990,
	kPGPError_InvalidCertificateExtension		= -10989,
	kPGPError_PublicKeyNotFound					= -10988,

	kPGPError_CRSMissingRequiredAttribute		= -10979,
	kPGPError_CRSInvalidCharacter				= -10978,
	kPGPError_CRSInvalidAttributeType			= -10977,
	kPGPError_CRSInvalidCertType				= -10976,
	kPGPError_CRSInvalidAttributeValueLength	= -10975,
	kPGPError_CRSInvalidAuthenticateValue		= -10974,


	kPGPError_DummyEnumValue
	/* kPGPError_Last */
} ;


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

PGPError 	PGPGetErrorString( PGPError theError,
				PGPSize bufferSize, char * theString );

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpErrors_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

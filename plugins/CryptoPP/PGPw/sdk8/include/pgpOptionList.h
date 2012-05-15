/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	This file contains the types and prototypes for functions which manipulate
	PGPOptionList data structures.

	$Id: pgpOptionList.h,v 1.43 2004/05/15 02:48:39 ajivsov Exp $
____________________________________________________________________________*/

#ifndef Included_pgpOptionList_h	/* [ */
#define Included_pgpOptionList_h

#include <stdarg.h>

#include "pgpPubTypes.h"

#if PGP_MACINTOSH
#include <Files.h>
#elif PGP_OSX
	struct FSSpec;
#endif

/* Standard event callback declaration */
struct PGPEvent;
typedef PGPError (*PGPEventHandlerProcPtr)(PGPContextRef context,
						struct PGPEvent *event, PGPUserValue userValue);

/* Export formats for exporting functions */
enum PGPExportFormat_
{
	kPGPExportFormat_Basic				= 1,
	kPGPExportFormat_Complete			= 255,

    kPGPExportFormat_TokenKeyContainer  = 256,
	
	kPGPExportFormat_X509Cert			= 10000,
	kPGPExportFormat_PKCS8,

	kPGPExportFormat_X509CertReq		= 11000,
	kPGPExportFormat_NetToolsCAV1_CertReq,
	kPGPExportFormat_VerisignV1_CertReq,
	kPGPExportFormat_EntrustV1_CertReq,
	kPGPExportFormat_NetscapeV1_CertReq,
	kPGPExportFormat_MicrosoftV1_CertReq,

	kPGPExportFormat_X509GetCertInitial	= 11010,
	kPGPExportFormat_NetToolsCAV1_GetCertInitial,
	kPGPExportFormat_VerisignV1_GetCertInitial,
	kPGPExportFormat_EntrustV1_GetCertInitial,
	kPGPExportFormat_NetscapeV1_GetCertInitial,
	kPGPExportFormat_MicrosoftV1_GetCertInitial,
	
	kPGPExportFormat_X509GetCRL	= 11020,
	kPGPExportFormat_NetToolsCAV1_GetCRL,
	kPGPExportFormat_VerisignV1_GetCRL,
	kPGPExportFormat_EntrustV1_GetCRL,
	kPGPExportFormat_NetscapeV1_GetCRL,
	kPGPExportFormat_MicrosoftV1_GetCRL,

	PGP_ENUM_FORCE( PGPExportFormat_ )
};
PGPENUM_TYPEDEF( PGPExportFormat_, PGPExportFormat );

/* Input formats for PGPOInputFormat */
enum PGPInputFormat_
{
	kPGPInputFormat_Unknown 		= 0,
	kPGPInputFormat_PGP 			= 1,
	
	kPGPInputFormat_X509DataInPKCS7 = 10000,
	kPGPInputFormat_NetToolsCAV1_DataInPKCS7,
	kPGPInputFormat_VerisignV1_DataInPKCS7,
	kPGPInputFormat_EntrustV1_DataInPKCS7,
	kPGPInputFormat_MicrosoftV1_DataInPKCS7,
	kPGPInputFormat_NetscapeV1_DataInPKCS7,

	kPGPInputFormat_PEMEncodedX509Cert,
	kPGPInputFormat_NetToolsCAV1_PEMEncoded,
	kPGPInputFormat_VerisignV1_PEMEncoded,
	kPGPInputFormat_EntrustV1_PEMEncoded,
	kPGPInputFormat_MicrosoftV1_PEMEncoded,
	kPGPInputFormat_NetscapeV1_PEMEncoded,

	/* Input formats for X.509 private keys */
	kPGPInputFormat_PrivateKeyInfo,
	kPGPInputFormat_PKCS12,

	kPGPInputFormat_SMIMEBody = 12000,
	kPGPInputFormat_SMIMEBodySignedData = 12001,
	kPGPInputFormat_SMIMEBodyEncryptedData = 12002,

	PGP_ENUM_FORCE( PGPInputFormat_ )
};
PGPENUM_TYPEDEF( PGPInputFormat_, PGPInputFormat );

/* Output formats for PGPOOutputFormat */
enum PGPOutputFormat_
{
	kPGPOutputFormat_Unknown 			= 0,
	kPGPOutputFormat_PGP 				= 1,
	
	kPGPOutputFormat_X509CertReqInPKCS7 = 10000,
	kPGPOutputFormat_NetToolsCAV1_CertReqInPKCS7,
	kPGPOutputFormat_VerisignV1_CertReqInPKCS7,
	kPGPOutputFormat_EntrustV1_CertReqInPKCS7,
	kPGPOutputFormat_NetscapeV1_CertReqInPKCS7,
	kPGPOutputFormat_MicrosoftV1_CertReqInPKCS7,

	kPGPOutputFormat_X509GetCertInitialInPKCS7 = 10010,
	kPGPOutputFormat_NetToolsCAV1_GetCertInitialInPKCS7,
	kPGPOutputFormat_VerisignV1_GetCertInitialInPKCS7,
	kPGPOutputFormat_EntrustV1_GetCertInitialInPKCS7,
	kPGPOutputFormat_NetscapeV1_GetCertInitialInPKCS7,
	kPGPOutputFormat_MicrosoftV1_GetCertInitialInPKCS7,

	kPGPOutputFormat_X509GetCRLInPKCS7 = 10020,
	kPGPOutputFormat_NetToolsCAV1_GetCRLInPKCS7,
	kPGPOutputFormat_VerisignV1_GetCRLInPKCS7,
	kPGPOutputFormat_EntrustV1_GetCRLInPKCS7,
	kPGPOutputFormat_NetscapeV1_GetCRLInPKCS7,
	kPGPOutputFormat_MicrosoftV1_GetCRLInPKCS7,

	kPGPOutputFormat_SMIMEBodySignedData = 12001,
	kPGPOutputFormat_SMIMEBodyEncryptedData = 12002,
	
	PGP_ENUM_FORCE( PGPOutputFormat_ )
};
PGPENUM_TYPEDEF( PGPOutputFormat_, PGPOutputFormat );

/* Attribute-Value structure for PGPOAttributeValue */
enum PGPAVAttribute_
{
	/* Pointer properties */
	kPGPAVAttributeFirstPointer = 0,
	kPGPAVAttribute_CommonName = kPGPAVAttributeFirstPointer,
	kPGPAVAttribute_Email,
	kPGPAVAttribute_OrganizationName,
	kPGPAVAttribute_OrganizationalUnitName,
	kPGPAVAttribute_SurName,
	kPGPAVAttribute_SerialNumber,
	kPGPAVAttribute_Country,
	kPGPAVAttribute_Locality,
	kPGPAVAttribute_State,
	kPGPAVAttribute_StreetAddress,
	kPGPAVAttribute_Title,
	kPGPAVAttribute_Description,
	kPGPAVAttribute_PostalCode,
	kPGPAVAttribute_POBOX,
	kPGPAVAttribute_PhysicalDeliveryOfficeName,
	kPGPAVAttribute_TelephoneNumber,
	kPGPAVAttribute_X121Address,
	kPGPAVAttribute_ISDN,
	kPGPAVAttribute_DestinationIndicator,
	kPGPAVAttribute_Name,
	kPGPAVAttribute_GivenName,
	kPGPAVAttribute_Initials,
	kPGPAVAttribute_HouseIdentifier,
	kPGPAVAttribute_DirectoryManagementDomain,
	kPGPAVAttribute_DomainComponent,
	kPGPAVAttribute_UnstructuredName,
	kPGPAVAttribute_UnstructuredAddress,
	kPGPAVAttribute_RFC822Name,
	kPGPAVAttribute_DNSName,
	kPGPAVAttribute_AnotherName,
	kPGPAVAttribute_IPAddress,
	kPGPAVAttribute_CertificateExtension,
	kPGPAVAttribute_SCEPChallenge,

	/* Verisign specific */
	kPGPAVAttribute_Challenge,
	kPGPAVAttribute_CertType,
    kPGPAVAttribute_MailFirstName,
    kPGPAVAttribute_MailMiddleName,
    kPGPAVAttribute_MailLastName,
    kPGPAVAttribute_EmployeeID,
    kPGPAVAttribute_MailStop,
    kPGPAVAttribute_AdditionalField4,
    kPGPAVAttribute_AdditionalField5,
    kPGPAVAttribute_AdditionalField6,
    kPGPAVAttribute_Authenticate,

	/* Generic pointer properties continue */
	kPGPAVAttribute_KeyID = kPGPAVAttribute_Authenticate+100,

	kPGPAVAttributeLastPointer = kPGPAVAttribute_KeyID,

	/* Boolean properties */
	kPGPAVAttributeFirstBoolean = 1000,

	/* Verisign specific */
    kPGPAVAttribute_EmbedEmail,


	/* Numeric (PGPUInt32) properties */
	kPGPAVAttributeFirstNumber = 2000,

	PGP_ENUM_FORCE( PGPAVAttribute_ )
} ;
PGPENUM_TYPEDEF( PGPAVAttribute_, PGPAVAttribute );

typedef struct PGPAttributeValue
{
	PGPAVAttribute	attribute;
	PGPUInt32			size;
	union
	{
		PGPBoolean  booleanvalue;
		PGPUInt32   longvalue;
		void       *pointervalue;
		
	} value;      
	    
	PGPUInt32	 	unused;
} PGPAttributeValue ;

enum PGPSMIMEMatchCriterion_
{
	kPGPSMIMEMatchCriterion_Exact	= 1,	/* exact matching per SMIME spec (default) */
	kPGPSMIMEMatchCriterion_Related	= 2,	/* use Issuer from IASN, use Exact first then Any for keyIdentifier */
	kPGPSMIMEMatchCriterion_Any		= 3,	/* try all */

	PGP_ENUM_FORCE( PGPSMIMEMatchCriterion_ )
} ;
PGPENUM_TYPEDEF( PGPSMIMEMatchCriterion_, PGPSMIMEMatchCriterion );

PGP_BEGIN_C_DECLARATIONS


PGPError	PGPNewOptionList( PGPContextRef context, PGPOptionListRef *outList );
PGPError 	PGPAppendOptionList( PGPOptionListRef optionList,
					PGPOptionListRef firstOption, ... );
PGPError 	PGPBuildOptionList( PGPContextRef context,
					PGPOptionListRef *outList,
					PGPOptionListRef firstOption, ... );
PGPError	PGPCopyOptionList( PGPOptionListRef optionList,
					PGPOptionListRef *outList );
PGPError 	PGPFreeOptionList( PGPOptionListRef optionList );

/*
**	The following functions are used to create PGPOptionListRef's for
**	specifying the various options to several SDK functions. The
**	functions can be used as inline parameters in a temporary manner or
**	used with PGPBuildOptionList() to create persistent lists.
*/

/*
** Special PGPOptionListRef to mark last option passed to those functions
** which take variable lists of PGPOptionListRef's:
*/

PGPOptionListRef	PGPOLastOption( PGPContextRef context );

/* Special PGPOptionListRef which is always ignored: */

PGPOptionListRef	PGPONullOption( PGPContextRef context);

/* Data input (required): */

PGPOptionListRef 	PGPOInputFile( PGPContextRef context,
							PGPFileSpecRef fileRef );
PGPOptionListRef 	PGPOInputBuffer( PGPContextRef context,
							void const *buffer, PGPSize bufferSize );
#if PGP_MACINTOSH || PGP_OSX
PGPOptionListRef 	PGPOInputFileFSSpec( PGPContextRef context,
							const struct FSSpec *fileSpec );
#endif
	
/* Data output (optional, generates event if missing): */

PGPOptionListRef 	PGPOOutputFile( PGPContextRef context,
							PGPFileSpecRef fileRef );
PGPOptionListRef 	PGPOOutputBuffer( PGPContextRef context,
							void *buffer, PGPSize bufferSize,
							PGPSize *outputDataLength );
PGPOptionListRef 	PGPOOutputDirectory( PGPContextRef context,
							PGPFileSpecRef fileRef );
#if PGP_MACINTOSH || PGP_OSX
PGPOptionListRef 	PGPOOutputFileFSSpec( PGPContextRef context,
							const struct FSSpec *fileSpec );
#endif

/* Filepath modifiers (used for Archiving )*/

PGPOptionListRef	PGPORelativePath( PGPContextRef context,
							PGPFileSpecRef dirRef );
							
PGPOptionListRef	PGPORootPath( PGPContextRef context, 
							PGPFileSpecRef dirRef );

/* '*buffer' must be disposed of via PGPFreeData() */
/* maximum memory usage will be no more than maximumBufferSize */
PGPOptionListRef 	PGPOAllocatedOutputBuffer(PGPContextRef context,
							void **buffer, PGPSize maximumBufferSize,
							PGPSize *actualBufferSize);
PGPOptionListRef 	PGPOAppendOutput( PGPContextRef context,
							PGPBoolean appendOutput );
PGPOptionListRef 	PGPODiscardOutput( PGPContextRef context,
							PGPBoolean discardOutput );
PGPOptionListRef    PGPOAllocatedOutputKeyContainer(PGPContextRef context,
	                        void **keyContName, PGPSize maximumKeyContNameSize, 
                            PGPSize *actualKeyContNameSize );

/* Encrypting and signing */

PGPOptionListRef 		PGPOEncryptToKeyDBObj( PGPContextRef context,
							PGPKeyDBObjRef keyDBObjRef);
PGPOptionListRef 		PGPOEncryptToKeySet( PGPContextRef context,
							PGPKeySetRef keySetRef);
PGPOptionListRef 		PGPOIntegrityProtection( PGPContextRef context,
							PGPBoolean integrity);
PGPOptionListRef 		PGPOSignWithKey( PGPContextRef context,
							PGPKeyDBObjRef keyDBObjRef,
							PGPOptionListRef firstOption, ...);
PGPOptionListRef 		PGPOConventionalEncrypt( PGPContextRef context,
							PGPOptionListRef firstOption,
								...);
#undef 					PGPOPassphraseBuffer
PGPOptionListRef 		PGPOPassphraseBuffer( PGPContextRef context,
							const PGPChar8 *passphrase,
							PGPSize passphraseLength);
#undef					PGPOPassphrase
PGPOptionListRef 		PGPOPassphrase( PGPContextRef context,
							const PGPChar8 *passphrase);
PGPOptionListRef 		PGPOPasskeyBuffer( PGPContextRef context,
							const void *passkey, PGPSize passkeyLength);

PGPOptionListRef 		PGPOCachePassphrase( PGPContextRef context,
							PGPUInt32 timeOutSeconds, PGPBoolean globalCache);
PGPOptionListRef 		PGPOSessionKey( PGPContextRef context,
							const void *sessionKey, PGPSize sessionKeyLength);
PGPOptionListRef		PGPOSignedHash( PGPContextRef context,
							void const *hash, PGPSize hashSize );
PGPOptionListRef 		PGPOAllowBareESKs( PGPContextRef context,
							PGPBoolean allowBareESKs );
PGPOptionListRef 		PGPOAskUserForEntropy( PGPContextRef context,
							PGPBoolean askUserForEntropy );
PGPOptionListRef 		PGPORawPGPInput( PGPContextRef context,
							PGPBoolean rawPGPInput );
PGPOptionListRef 		PGPOCompression( PGPContextRef context,
							PGPBoolean compression );

PGPOptionListRef		PGPOLocalEncoding( PGPContextRef context,
							PGPLocalEncodingFlags localEncode);
PGPOptionListRef		PGPOOutputLineEndType(PGPContextRef context,
							PGPLineEndType lineEnd);
#undef					PGPOPGPMIMEEncoding
PGPOptionListRef		PGPOPGPMIMEEncoding(PGPContextRef context,
							PGPBoolean mimeEncoding, PGPSize *mimeBodyOffset,
							PGPChar8 mimeSeparator[ kPGPMimeSeparatorSize ]);
PGPOptionListRef		PGPOOmitMIMEVersion( PGPContextRef context,
							PGPBoolean omitVersion);
PGPOptionListRef		PGPOX509Encoding( PGPContextRef context,
							PGPBoolean x509Encoding);
							
PGPOptionListRef 		PGPODetachedSig( PGPContextRef context,
							PGPOptionListRef firstOption,
								...);

PGPOptionListRef 		PGPOCipherAlgorithm( PGPContextRef context,
							PGPCipherAlgorithm algorithm);
PGPOptionListRef 		PGPOHashAlgorithm( PGPContextRef context,
							PGPHashAlgorithm algorithm);
PGPOptionListRef 		PGPOCompressionAlgorithm( PGPContextRef context,
							PGPCompressionAlgorithm algorithm);

PGPOptionListRef 		PGPOFailBelowValidity( PGPContextRef context,
							PGPValidity minValidity);
PGPOptionListRef 		PGPOWarnBelowValidity( PGPContextRef context,
							PGPValidity minValidity);


PGPOptionListRef 		PGPOEventHandler( PGPContextRef context,
							PGPEventHandlerProcPtr eventHandler,
								PGPUserValue eventHandlerData);
PGPOptionListRef 		PGPOSendNullEvents( PGPContextRef context,
							PGPTimeInterval approxInterval);
	
PGPOptionListRef 		PGPOArmorOutput( PGPContextRef context,
							PGPBoolean armorOutput );
PGPOptionListRef 		PGPODataIsASCII( PGPContextRef context,
							PGPBoolean dataIsASCII );
PGPOptionListRef 		PGPOClearSign( PGPContextRef context,
							PGPBoolean clearSign );
PGPOptionListRef 		PGPOForYourEyesOnly( PGPContextRef context,
							PGPBoolean forYourEyesOnly );
PGPOptionListRef 		PGPOKeyDBRef( PGPContextRef context,
							PGPKeyDBRef keydbRef);

PGPOptionListRef 		PGPOExportKeySet( PGPContextRef context,
							PGPKeySetRef keysetRef);
PGPOptionListRef 		PGPOExportKeyDBObj( PGPContextRef context,
							PGPKeyDBObjRef keyDBObjRef);

PGPOptionListRef 		PGPOImportKeysTo( PGPContextRef context,
							PGPKeyDBRef keydbRef);
PGPOptionListRef 		PGPOSendEventIfKeyFound( PGPContextRef context,
							PGPBoolean sendEventIfKeyFound );
PGPOptionListRef 		PGPOPassThroughIfUnrecognized( PGPContextRef context,
							PGPBoolean passThroughIfUnrecognized );
PGPOptionListRef 		PGPOPassThroughClearSigned( PGPContextRef context,
							PGPBoolean passThroughClearSigned );
PGPOptionListRef 		PGPOPassThroughKeys( PGPContextRef context,
							PGPBoolean passThroughKeys );
PGPOptionListRef 		PGPORecursivelyDecode( PGPContextRef context,
							PGPBoolean recurse );

PGPOptionListRef		PGPOKeyGenParams( PGPContextRef context,
							PGPPublicKeyAlgorithm pubKeyAlg,
							PGPUInt32 bits);
								
#undef					PGPOKeyGenName
PGPOptionListRef		PGPOKeyGenName( PGPContextRef context,
							const void *name, PGPSize nameLength);
							
PGPOptionListRef		PGPOCreationDate( PGPContextRef context,
							PGPTime creationDate);
PGPOptionListRef		PGPOExpiration( PGPContextRef context,
							PGPUInt32 expirationDays);
							
PGPOptionListRef		PGPOAdditionalRecipientRequestKeySet(
							PGPContextRef context, 
							PGPKeySetRef arKeySetRef, PGPByte arkClass);
								
PGPOptionListRef		PGPORevocationKeySet(PGPContextRef	context,
							PGPKeySetRef raKeySetRef);

PGPOptionListRef		PGPOKeyGenMasterKey( PGPContextRef context,
							PGPKeyDBObjRef masterKeyDBObjRef);
							
PGPOptionListRef		PGPOPreferredAlgorithms(
							PGPContextRef context, 
							PGPCipherAlgorithm const *prefAlg,
							PGPUInt32 numAlgs);
								
#undef					PGPOPreferredKeyServer
PGPOptionListRef		PGPOPreferredKeyServer(
							PGPContextRef context,
							PGPChar8 const * server );
								
PGPOptionListRef		PGPOKeyFeatures( PGPContextRef context,
							PGPUInt32 features);

PGPOptionListRef		PGPOKeyFlags( PGPContextRef context,
							PGPUInt32 flags);
							
PGPOptionListRef		PGPOKeyServerPreferences( PGPContextRef context,
							PGPUInt32 preferences);
							
PGPOptionListRef		PGPOKeyGenFast( PGPContextRef context,
							PGPBoolean fastGen);
						
PGPOptionListRef		PGPOTokenNumber( PGPContextRef context,
							PGPUInt32 tokenID);
#define					PGPOKeyGenOnToken PGPOTokenNumber

PGPOptionListRef        PGPOOutputToken( PGPContextRef context, PGPUInt32 token );

PGPOptionListRef        PGPOKeyContainer( PGPContextRef context, 
                            const PGPByte *keyContainerName, 
                            PGPSize keyContainerNameSize );
							
PGPOptionListRef		PGPOKeyGenUseExistingEntropy( PGPContextRef context,
							PGPBoolean useExistingEntropy);
							
PGPOptionListRef		PGPOPreferredCompressionAlgorithms(
							PGPContextRef context, 
							PGPCompressionAlgorithm const *prefAlg,
							PGPUInt32 numAlgs);

#undef					PGPOCommentString
PGPOptionListRef		PGPOCommentString( PGPContextRef context,
							PGPChar8 const *comment);

#undef					PGPOVersionString
PGPOptionListRef		PGPOVersionString( PGPContextRef context,
							PGPChar8 const *version);
							
#undef					PGPOFileNameString
PGPOptionListRef		PGPOFileNameString( PGPContextRef context,
							PGPChar8 const *fileName);
							
#undef					PGPOSigRegularExpression
PGPOptionListRef		PGPOSigRegularExpression(PGPContextRef context,
							PGPChar8 const *regularExpression);
							
PGPOptionListRef		PGPOExportPrivateKeys( PGPContextRef context,
							PGPBoolean exportKeys);
							
PGPOptionListRef		PGPOExportPrivateSubkeys( PGPContextRef context,
							PGPBoolean exportSubkeys);
							
PGPOptionListRef		PGPOExportFormat(PGPContextRef context,
							PGPExportFormat exportFormat);

PGPOptionListRef		PGPOExportable( PGPContextRef context,
							PGPBoolean exportable);
							
PGPOptionListRef		PGPOSigTrust( PGPContextRef context,
							PGPUInt32 trustLevel,
							PGPUInt32 trustValue);

PGPOptionListRef		PGPOInputFormat( PGPContextRef context,
							PGPInputFormat inputFormat );

PGPOptionListRef		PGPOOutputFormat( PGPContextRef context,
							PGPOutputFormat outputFormat );

PGPOptionListRef		PGPOAttributeValue( PGPContextRef context,
							PGPAttributeValue *attributeValue,
							PGPUInt32 attributeValueCount);

PGPOptionListRef		PGPOInputTARCache( PGPContextRef context,
							PGPFileSpecRef cacheFileRef, 
	   						const PGPByte *sessionKey, 
							PGPSize sessionKeySize );

PGPOptionListRef		PGPOOutputTARCache( PGPContextRef context,
							PGPFileSpecRef cacheFileRef );

PGPOptionListRef		PGPOIssueRevocations( PGPContextRef context,
							PGPBoolean issueRevocations );

PGPOptionListRef		PGPOSMIMEMatchCriterion( PGPContextRef context,
							PGPSMIMEMatchCriterion criterion );

PGPOptionListRef		PGPOSMIMESigner( PGPContextRef context,
							PGPKeyDBObjRef signer );


PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpOptionList_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

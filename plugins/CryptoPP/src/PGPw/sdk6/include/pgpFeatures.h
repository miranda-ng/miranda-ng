/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	Determine which features are present in the PGPsdk.  This is the only
	way to correctly determine which features are present.  The version
	number may be the same for different builds that lack some features.

	$Id: pgpFeatures.h,v 1.18.20.1 1999/08/17 20:04:30 cpeterson Exp $
____________________________________________________________________________*/
#ifndef Included_pgpFeatures_h	/* [ */
#define Included_pgpFeatures_h


#include "pgpPubTypes.h"



#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k	/* [ */
#endif


/*____________________________________________________________________________
	Each selector designates a PGPFlags word, which can be obtained via
	PGPGetFeatureFlags().  The flags can be tested using the 
	supplied masks.  We can add more selectors as needed.  The masks
	are not intended to be restricted to a single bit.
	Flags should not be used for attributes that have unknown length.
	
	A kPGPError_ItemNotFound will be returned if the caller specifies
	a selector which is not recognized by the PGPsdk.  This could
	occur if an app links to an older version of the SDK.
____________________________________________________________________________*/

/* selectors which are passed to PGPGetFeatureFlags */
enum PGPFeatureSelector_
{
	kPGPFeatures_GeneralSelector		= 1,
	kPGPFeatures_ImplementationSelector	= 2,
	PGP_ENUM_FORCE( PGPFeatureSelector_ )
};
PGPENUM_TYPEDEF( PGPFeatureSelector_, PGPFeatureSelector );


/* flags for kPGPFeatures_GeneralSelector */
enum
{
	kPGPFeatureMask_CanEncrypt			= 0x1,
	kPGPFeatureMask_CanDecrypt			= 0x2,
	kPGPFeatureMask_CanSign				= 0x4,
	kPGPFeatureMask_CanVerify			= 0x8,
	kPGPFeatureMask_CanGenerate			= 0x10,
	kPGPFeatureMask_RngHardware			= 0x20
};


/* flags for kPGPFeatures_ImplementationSelector */
enum
{
	kPGPFeatureMask_IsDebugBuild	= 0x1,
	kPGPFeatureMask_HasTimeout		= 0x2
};



typedef struct PGPAlgorithmInfo
{
	char		shortName[ 32 ];
	char		longName[ 96 ];
	char		copyright[ 128 ];
	PGPFlags	flags;			/* reserved; 0 for now */
	PGPUInt32	reserved[ 16 ];	/* reserved; 0 for now */
} PGPAlgorithmInfo;


typedef struct PGPPublicKeyAlgorithmInfo
{
	PGPAlgorithmInfo		info;
	
	PGPPublicKeyAlgorithm	algID;
	
	PGPBoolean				canEncrypt;
	PGPBoolean				canDecrypt;
	PGPBoolean				canSign;
	PGPBoolean				canVerify;
	PGPBoolean				canGenerate;
	PGPBoolean				reserved1;
	PGPBoolean				reserved2;
	PGPBoolean				reserved3;
	
	PGPUInt32				reserved[ 8 ];
} PGPPublicKeyAlgorithmInfo;


typedef struct PGPSymmetricCipherInfo
{
	PGPAlgorithmInfo	info;
	
	PGPCipherAlgorithm	algID;
	
	PGPUInt32			reserved[ 8 ];
} PGPSymmetricCipherInfo;



#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset			/* ] */
#endif





PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


/* see kPGPsdkAPIVersion in pgpUtilities.h for version format */
PGPError 	PGPGetSDKVersion( PGPUInt32 *version );


/*____________________________________________________________________________
	Return a C string of the form:

		"PGPsdk version 1.0 (C) Network Associates, Inc"
____________________________________________________________________________*/
PGPError	PGPGetSDKString( char theString[ 256 ] );



/* return a flags word for the feature selector */
PGPError 	PGPGetFeatureFlags( PGPFeatureSelector selector,
					PGPFlags *flags );

/* use this to test whether a feature exists after getting flags */
#define PGPFeatureExists( flags, maskValue )							\
			( ( (flags) & (maskValue) ) != 0 )


/*____________________________________________________________________________
	Routines to determine which algorithms are present.
	
	To determine if a specific algorithm is available, you will need to
	index through the available algorithms and check the algorithm ID.
____________________________________________________________________________*/
PGPError	PGPCountPublicKeyAlgorithms( PGPUInt32 *numPKAlgs );
PGPError	PGPGetIndexedPublicKeyAlgorithmInfo( PGPUInt32 theIndex,
				PGPPublicKeyAlgorithmInfo *info);
				
PGPError	PGPCountSymmetricCiphers( PGPUInt32 *numPKAlgs );
PGPError	PGPGetIndexedSymmetricCipherInfo( PGPUInt32 theIndex,
				PGPSymmetricCipherInfo *info);



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS




#endif /* ] Included_pgpFeatures_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

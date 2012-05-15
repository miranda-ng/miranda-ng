/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	Determine which features are present in the PGPsdk.  This is the only
	way to correctly determine which features are present.  The version
	number may be the same for different builds that lack some features.

	$Id: pgpFeatures.h,v 1.13 2004/03/15 19:18:55 vinnie Exp $
____________________________________________________________________________*/

#ifndef Included_pgpFeatures_h	/* [ */
#define Included_pgpFeatures_h

#include "pgpPubTypes.h"

#if PGP_MACINTOSH
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
	kPGPFeatureMask_CanEncrypt			= (1UL << 0),
	kPGPFeatureMask_CanDecrypt			= (1UL << 1),
	kPGPFeatureMask_CanSign				= (1UL << 2),
	kPGPFeatureMask_CanVerify			= (1UL << 3),
	kPGPFeatureMask_CanGenerateKey		= (1UL << 4),
	kPGPFeatureMask_RngHardware			= (1UL << 5),
	kPGPFeatureMask_FIPSmode			= (1UL << 6)
};

/* flags for kPGPFeatures_ImplementationSelector */
enum
{
	kPGPFeatureMask_IsDebugBuild	= (1UL << 0),
	kPGPFeatureMask_HasTimeout		= (1UL << 0)
};

#define kPGPAlgorithmInfoFlags_FIPS		(PGPFlags)(1UL << 1 )

typedef struct PGPAlgorithmInfo
{
	PGPChar8	shortName[ 32 ];
	PGPChar8	longName[ 96 ];
	PGPChar8	copyright[ 128 ];
	PGPFlags	flags;			 
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
	PGPBoolean				canGenerateKey;
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

#if PGP_MACINTOSH
#pragma options align=reset			/* ] */
#endif

PGP_BEGIN_C_DECLARATIONS

/* return a flags word for the feature selector */
PGPError 	PGPGetFeatureFlags( PGPFeatureSelector selector,
					PGPFlags *flags );

/* use this to test whether a feature exists after getting flags */
#define PGPFeatureExists( flags, maskValue ) ( ( (flags) & (maskValue) ) != 0 )


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

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpFeatures_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

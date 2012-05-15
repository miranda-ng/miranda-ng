/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id: pgpReconstruct.h,v 1.1 2003/12/29 08:12:20 wprice Exp $
____________________________________________________________________________*/
#ifndef Included_pgpReconstruct_h	/* [ */
#define Included_pgpReconstruct_h

#include "pgpPubTypes.h"
#include "pgpKeyServer.h"

#define kPGPRecon_NumShares			5
#define kPGPRecon_Threshold			3
#define kPGPRecon_MaxPromptLen		( 96 - 1 )
#define kPGPRecon_MaxURLLen			( 256 - 1 )
#define kPGPRecon_MaxPassLen		( 256 - 1 )
#define kPGPRecon_MaxUserNameLen	( 128 - 1 )
#define kPGPRecon_MaxPasswordLen	( 128 - 1 )


typedef struct PGPReconContext *			PGPReconContextRef;

#define	kInvalidPGPReconContextRef			((PGPReconContextRef) NULL)
#define PGPReconContextRefIsValid( ref )	( (ref) != kInvalidPGPReconContextRef )

typedef char PGPReconPrompts[kPGPRecon_NumShares][kPGPRecon_MaxPromptLen + 1];

typedef char PGPReconPasses[kPGPRecon_NumShares][kPGPRecon_MaxPassLen + 1];

typedef PGPError (*PGPReconstructEventHandler)(PGPContextRef recon,
				PGPEvent *event, PGPUserValue userValue);

/*	inAuthUser and inAuthPass are not needed if the server class
	is kPGPKeyServerClass_PGP.	*/
	PGPError
PGPNewReconstruct(
	PGPKeyDBObjRef				inTargetKey,
	PGPUTF8						*inAuthUser,		/* can be NULL */
	PGPUTF8						*inAuthPass,		/* can be NULL */
	PGPReconstructEventHandler	inHandler,
	PGPUserValue				inUserValue,
	PGPReconContextRef			*outRef );

/*	This is only needed if you have to change the event handler after
	allocating the PGPReconContextRef */
	PGPError
PGPSetReconstructionEventHandler(
	PGPReconContextRef			reconRef,
	PGPReconstructEventHandler	inHandler,
	PGPUserValue				inUserValue );
	

/*	I don't think it makes sense to support split keys for reconstruction,
	so we only take a passphrase below */
	PGPError
PGPMakeReconstruction(
	PGPReconContextRef			reconRef,
	PGPReconPrompts 			inPromptInfo,
	PGPReconPasses				inPassInfo,
	PGPUTF8						*inPassphrase );

	PGPError
PGPGetReconstruction(
	PGPReconContextRef			reconRef,
	PGPByte 					**reconData,	/* must be freed by caller */
	PGPSize 					*reconDataSize );

	PGPError
PGPSendReconstruction(
	PGPReconContextRef			reconRef );

	PGPError
PGPGetReconstructionPrompts(
	PGPReconContextRef			reconRef,
	PGPReconPrompts				outPromptInfo );

	PGPError
PGPGetReconstructionData(
	PGPReconContextRef			reconRef,
	PGPReconPasses				inPassInfo,
	PGPByte						**outReconData,	/* must be freed by caller */
	PGPSize						*outReconSize );

/*	Reconstructed private key will be returned in
	outReconstructedKey if successful.  The imported
	key will have no passphrase and thus the user must
	then be forced to choose a new passphrase.		*/
	PGPError
PGPReconstruct(
	PGPReconContextRef			reconRef,
	PGPReconPasses				inPassInfo,
	PGPByte						*inReconData,
	PGPSize						inReconDataSize,
	PGPKeyDBRef					*outReconstructedKey );

	PGPError
PGPFreeReconstruct(
	PGPReconContextRef			reconRef );

	PGPError
PGPSetReconstructionServerURL(
	PGPReconContextRef			reconRef,
	PGPUTF8						*pszServerURL,
	PGPKeyServerClass			dwServerType );

#endif /* ] Included_pgpReconstruct_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

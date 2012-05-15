/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id: pgpShare.h,v 1.1 2004/04/01 11:45:40 wprice Exp $
____________________________________________________________________________*/
#ifndef Included_pgpShare_h	/* [ */
#define Included_pgpShare_h

#include "pgpConfig.h"
#include "pgpBase.h"
#include "pgpKeys.h"

typedef struct PGPShare *	PGPShareRef;

typedef struct PGPShareID_
{
	PGPByte 	data[8];
} PGPShareID;

#define	kInvalidPGPShareRef			((PGPShareRef) NULL)
#define PGPShareRefIsValid( ref )	( (ref) != kInvalidPGPShareRef )

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

PGPError PGPCreateShares(PGPContextRef context, PGPKeyDBObjRef key, 
				PGPUInt32 threshold, PGPUInt32 numShares, 
				PGPShareRef *share);

/* The passkey needs to be freed with PGPFreeData(passkey) */
PGPError PGPGetPasskeyFromShares(PGPShareRef share, PGPByte **passkey,
				PGPSize *passkeySize);

PGPError PGPSplitShares(PGPShareRef share, PGPUInt32 numShares, 
				PGPShareRef *splitShares);

/* The share objects being combined are NOT freed by this function */
PGPError PGPCombineShares(PGPShareRef firstShare, PGPShareRef secondShare,
				PGPShareRef *combinedShares);

PGPError PGPFreeShares(PGPShareRef share);

PGPError PGPGetKeyIDFromShares(PGPShareRef share, PGPKeyID *id);

PGPError PGPGetShareID(PGPShareRef share, PGPShareID *id);

PGPUInt32 PGPGetShareThreshold(PGPShareRef share);

/* This is the number of shares contained in the share object */
PGPUInt32 PGPGetNumberOfShares(PGPShareRef share);

/* The share object may contain less than the total number of shares */
PGPUInt32 PGPGetTotalNumberOfShares(PGPShareRef share);

PGPBoolean IsSamePGPShares(PGPShareRef firstShare, PGPShareRef secondShare);

/* If firstID < secondID, -1 */
/* If firstID > secondID,  1 */
/* If firstID = secondID,  0 */
PGPInt32 PGPCompareShareIDs(PGPShareID firstID, PGPShareID secondID);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpShare_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

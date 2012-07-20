/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id: pgpShareFile.h,v 1.1 2004/04/01 11:45:40 wprice Exp $
____________________________________________________________________________*/
#ifndef Included_pgpShareFile_h	/* [ */
#define Included_pgpShareFile_h

#include "pgpConfig.h"
#include "pgpBase.h"
#include "pgpErrors.h"
#include "pgpPubTypes.h"
#include "pflTypes.h"
#include "pgpShare.h"

typedef struct PGPShareFile *	PGPShareFileRef;

#define	kInvalidPGPShareFileRef			((PGPShareFileRef) NULL)
#define PGPShareFileRefIsValid( ref )	( (ref) != kInvalidPGPShareFileRef )

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

PGPError PGPNewShareFile(PFLFileSpecRef shareFileSpec, 
				PGPShareFileRef *shareFileRef);

PGPError PGPFreeShareFile(PGPShareFileRef shareFileRef);

/* The share object needs to be freed with PGPFreeShares(shares) */
PGPError PGPCopySharesFromFile(PGPContextRef context, 
				PGPShareFileRef shareFileRef, PGPOptionListRef optionList,
				PGPShareRef *shares);

PGPError PGPCopySharesToFile(PGPContextRef context, 
				PGPShareFileRef shareFileRef, PGPOptionListRef optionList,
				PGPShareRef shares);

PGPError PGPGetShareFileUserID(PGPShareFileRef shareFileRef,
				PGPSize bufferSize, PGPUTF8 *userID, PGPSize *fullSize);

PGPError PGPSetShareFileUserID(PGPShareFileRef shareFileRef,
				const PGPUTF8 *userID);

PGPError PGPOpenShareFile(PFLFileSpecRef shareFileSpec, 
				PGPShareFileRef *shareFileRef);

PGPError PGPSaveShareFile(PGPShareFileRef shareFileRef);

PGPError PGPGetShareFileSpec(PGPShareFileRef shareFileRef,
				PFLFileSpecRef *shareFileSpec);

PGPError PGPGetShareFileShareID(PGPShareFileRef shareFileRef, 
				PGPShareID *id);

PGPError PGPGetShareFileOwnerKeyID(PGPShareFileRef shareFileRef, 
				PGPKeyID *id);

PGPError PGPSetShareFileOwnerKeyID(PGPShareFileRef shareFileRef, 
				const PGPKeyID id);

PGPError PGPGetShareFileOwnerFingerprint(PGPShareFileRef shareFileRef, 
				PGPSize bufferSize, void *fingerprint, PGPSize *fullSize);

PGPError PGPSetShareFileOwnerFingerprint(PGPShareFileRef shareFileRef,
				PGPSize bufferSize, const void *fingerprint);

PGPError PGPGetShareFileSharedKeyID(PGPShareFileRef shareFileRef, 
				PGPKeyID *id);

PGPUInt32 PGPGetShareThresholdInFile(PGPShareFileRef shareFileRef);

PGPUInt32 PGPGetNumSharesInFile(PGPShareFileRef shareFileRef);

PGPBoolean IsSamePGPSharesInFiles(PGPShareFileRef firstFile, 
				PGPShareFileRef secondFile);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpShareFile_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

/*____________________________________________________________________________
	pgpUtilities.h
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	This file contains miscellaneous utility functions needed for the PGPsdk.

	$Id: pgpUtilities.h,v 1.68 1999/03/25 01:46:38 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpUtilities_h	/* [ */
#define Included_pgpUtilities_h

#if PGP_MACINTOSH	/* [ */
#include <Files.h>
#endif	/* ] PGP_MACINTOSH */

#include "pgpPubTypes.h"
#include "pgpMemoryMgr.h"

#include <time.h>


#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k	/* [ */
#endif


#if PGP_MACINTOSH	/* [ */

#define kPGPMacFileCreator_Keys				'pgpK'
#define kPGPMacFileCreator_DecryptedBinary	'????'
#define kPGPMacFileCreator_DecryptedText	'ttxt'
#define kPGPMacFileCreator_Tools			'pgpM'

#define kPGPMacFileType_ArmorFile		'TEXT'
#define kPGPMacFileType_EncryptedData	'pgEF'
#define kPGPMacFileType_SignedData		'pgSF'
#define kPGPMacFileType_DetachedSig		'pgDS'
#define kPGPMacFileType_RandomSeed		'pgRS'
#define kPGPMacFileType_PrivRing		'pgRR'
#define kPGPMacFileType_PubRing			'pgPR'
#define kPGPMacFileType_Groups			'pgGR'
#define kPGPMacFileType_Preferences		'pref'
#define kPGPMacFileType_DecryptedText	'TEXT'
#define kPGPMacFileType_DecryptedBinary	'BINA'
#define kPGPMacFileType_KeyShares		'pgSK'
#define kPGPMacFileType_Exported509Keys	'pgX5'

#endif	/* ] PGP_MACINTOSH */

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

/*____________________________________________________________________________
	Context routines
____________________________________________________________________________*/

/*____________________________________________________________________________
	API version:
	Top byte is major, next 3 nibbles minor, next 2 bug fix,
	last nibble reserved:  0xMMmmmrrR
	
	example: 1.0.0 = 0x01000000
	
	0x01000000		SDK 1.0.0
	0x02000000		SDK 1.1.0
	0x02000010		SDK 1.1.1
	0x03000000		SDK 1.5
	0x03000010		SDK 1.5.1
	0x03000020		SDK 1.5.2
	0x03001000		SDK 1.6
	0x03002000		SDK 1.7
	0x03002010		SDK 1.7.1
_____________________________________________________________________________*/
#define kPGPsdkAPIVersion		( (PGPUInt32)0x03002010 )
#define PGPMajorVersion( v )	( ( ((PGPUInt32)(v)) & 0xFF000000 ) >> 24 )
#define PGPMinorVersion( v )	( ( ((PGPUInt32)(v)) & 0x00FFF000 ) >> 16 )
#define PGPRevVersion( v )		( ( ((PGPUInt32)(v)) & 0x00000FF0 ) >> 4 )

typedef struct PGPNewContextStruct
{
	/* sizeofStruct must be inited to sizeof( PGPNewContextStruct ) */
	PGPUInt32				sizeofStruct;
	PGPMemoryMgrRef			memoryMgr;
} PGPNewContextStruct;



/* always pass kPGPsdkVersion */
PGPError 	PGPNewContext( PGPUInt32 clientAPIVersion,
				PGPContextRef *newContext );
PGPError 	PGPNewContextCustom( PGPUInt32 clientAPIVersion,
				PGPNewContextStruct	const *custom,
				PGPContextRef *newContext );

PGPError 	PGPFreeContext( PGPContextRef context );

PGPError 	PGPGetContextUserValue( PGPContextRef context,
						PGPUserValue *userValue );
PGPError 	PGPSetContextUserValue( PGPContextRef context,
						PGPUserValue userValue );

PGPError	PGPContextGetRandomBytes(PGPContextRef context,
						void *buf, PGPSize len );


PGPMemoryMgrRef	PGPGetContextMemoryMgr( PGPContextRef context );



/*____________________________________________________________________________
	FileRefs
____________________________________________________________________________*/

PGPError 		PGPCopyFileSpec(PGPFileSpecRef fileRef,
					PGPFileSpecRef *ref);
PGPError 		PGPFreeFileSpec(PGPFileSpecRef fileRef);

#if PGP_MACINTOSH	/* [ */

PGPError 		PGPNewFileSpecFromFSSpec(PGPContextRef context,
						const FSSpec *spec, PGPFileSpecRef *ref);
PGPError 		PGPGetFSSpecFromFileSpec(PGPFileSpecRef fileRef,
						FSSpec *spec);
#else

PGPError 		PGPNewFileSpecFromFullPath( PGPContextRef context,
						char const *path, PGPFileSpecRef *ref);

/* *fullPathPtr ***must*** be freed with PGPFreeData */
PGPError 		PGPGetFullPathFromFileSpec( PGPFileSpecRef	fileRef,
						char ** fullPathPtr);
						
#endif	/* ] PGP_MACINTOSH */


						
/*____________________________________________________________________________
	Time
____________________________________________________________________________*/
PGPTime 			PGPGetTime(void);

/* these use time_t type as returned by time() in Std C libraries */
time_t 			PGPGetStdTimeFromPGPTime(PGPTime theTime);
PGPTime 		PGPGetPGPTimeFromStdTime(time_t theTime);


/* year, month, day may be NULL if desired */
void 			PGPGetYMDFromPGPTime( PGPTime theTime,
						PGPUInt16 *year, PGPUInt16 *month, PGPUInt16 *day );


#if PGP_MACINTOSH	/* [ */
PGPUInt32 		PGPTimeToMacTime(PGPTime theTime);
PGPTime 		PGPTimeFromMacTime(PGPUInt32 theTime);
#endif	/* ] PGP_MACINTOSH */



/*____________________________________________________________________________
	MacBinary support
____________________________________________________________________________*/


/*____________________________________________________________________________
	Examine the input file to see if it's a MacBinary file.  If it is
	not a MacBinary file, then the original file is unaltered.
	Otherwise, the file is converted and the original file is deleted.
	
	The resulting file is designated by 'outPGPSpec' and may have a different
	name than the original.
	
	If the file is a TEXT file, appropriate line-end conversion is done.
	
	creator and type code pointers may be
	null but otherwise contain the mac creator and type.
	
	This routine can be called on a Mac, but generally doesn't need to be.
____________________________________________________________________________*/
PGPError	PGPMacBinaryToLocal( PGPFileSpecRef inSpec,
				PGPFileSpecRef * outSpec,
				PGPUInt32 * macCreator, PGPUInt32 * macTypeCode );




/*____________________________________________________________________________
	Library initialization

	Call PGPsdkXXXInit() before using that particular library.
	Call PGPsdkXXXCleanup() when you are done (but after disposing of any
		PGPContexts).
	
	You may call PGPsdkXXXInit multiple times (with no effect), but be sure
	to call the matching PGPsdkXXXCleanup() for each call to PGPsdkXXXInit().
____________________________________________________________________________*/

PGPError	PGPsdkInit( void );
PGPError	PGPsdkCleanup( void );

PGPError	PGPsdkNetworkLibInit( void );
PGPError	PGPsdkNetworkLibCleanup( void );

PGPError	PGPsdkUILibInit( void );
PGPError	PGPsdkUILibCleanup( void );

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS



#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset			/* ] */
#endif

#endif /* ] Included_pgpUtilities_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

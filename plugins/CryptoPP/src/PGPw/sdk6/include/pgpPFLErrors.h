/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	Error codes.

	$Id: pgpPFLErrors.h,v 1.24.6.1 1999/06/13 20:27:13 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpPFLErrors_h	/* [ */
#define Included_pgpPFLErrors_h

#include "pgpBase.h"

#define kPGPPFLErrorBase	-12000
#define kPGPPFLErrorRange	500

enum
{
	/*
	NOTE: error numbers must not be changed as compile clients depend on them.
	*/
	
	kPGPError_NoErr				= 0,
	
	kPGPError_BadParams			= -12000,
	kPGPError_OutOfMemory		= -11999,
	kPGPError_BufferTooSmall	= -11998,
	
	kPGPError_FileNotFound		= -11997,
	kPGPError_CantOpenFile		= -11996,
	kPGPError_FilePermissions	= -11995,
	kPGPError_FileLocked		= -11994,
	/* Was kPGPError_DiskFull	= -11993,	*/
	kPGPError_IllegalFileOp		= -11992,
	kPGPError_FileOpFailed		= -11991,
	kPGPError_ReadFailed		= -11990,
	kPGPError_WriteFailed		= -11989,
	kPGPError_EOF				= -11988,
	
	kPGPError_UserAbort			= -11987,
	kPGPError_UnknownRequest	= -11986,
	kPGPError_LazyProgrammer	= -11985,
	kPGPError_ItemNotFound		= -11984,
	kPGPError_ItemAlreadyExists	= -11983,
	kPGPError_AssertFailed		= -11982,
	kPGPError_BadMemAddress		= -11981,
	kPGPError_UnknownError		= -11980,
	
	kPGPError_PrefNotFound		= -11979,
	kPGPError_EndOfIteration	= -11978,
	kPGPError_ImproperInitialization	= -11977,
	kPGPError_CorruptData				= -11976,
	kPGPError_FeatureNotAvailable		= -11975,
	
	kPGPError_DiskFull			= -11960,
	kPGPError_DiskLocked		= -11959,
	
	
	kPGPError_LastPFLError = kPGPPFLErrorBase + kPGPPFLErrorRange - 1
};

#define IsPGPError( err )		( (err) != kPGPError_NoErr )
#define IsntPGPError( err )		( (err) == kPGPError_NoErr )



#define PGPValidateParam( expr )	\
	if ( ! (expr ) )	\
	{\
		return( kPGPError_BadParams );\
	}

#define PGPValidatePtr( ptr )	\
			PGPValidateParam( (ptr) != NULL )


PGP_BEGIN_C_DECLARATIONS


PGPError 	PGPGetPFLErrorString( PGPError theError,
				PGPSize bufferSize, char * theString );


PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpPFLErrors_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

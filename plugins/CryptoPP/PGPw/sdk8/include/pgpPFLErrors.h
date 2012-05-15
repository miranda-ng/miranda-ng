/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id: pgpPFLErrors.h,v 1.20 2004/04/25 03:31:03 bzhao Exp $
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
	/* Unused					= -11993,	*/
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
	kPGPError_MatchNotFound				= -11974,
	kPGPError_ResourceUnavailable		= -11973,
	kPGPError_InsufficientPrivileges	= -11972,
	kPGPError_AlreadyInUse				= -11971,
	kPGPError_Deadlocked				= -11970,
	kPGPError_NotConnected				= -11969,
	kPGPError_BufferTooLarge			= -11968,
	kPGPError_SizeTooLarge				= -11967,
	
	kPGPError_DiskFull			= -11960,
	kPGPError_DiskLocked		= -11959,
	
	kPGPError_GraphicsOpFailed	= -11956,
	kPGPError_MemoryOpFailed	= -11955,
	kPGPError_NetworkOpFailed	= -11954,
	kPGPError_SecurityOpFailed	= -11953,
	kPGPError_StringOpFailed	= -11952,
	kPGPError_SyncObjOpFailed	= -11951,
	kPGPError_ThreadOpFailed	= -11950,
	kPGPError_VolumeOpFailed	= -11949,
	
	kPGPError_NTDrvIopOpFailed		= -11947,
	kPGPError_NTDrvObjectOpFailed	= -11946,

	kPGPError_Win32COMOpFailed		= -11945,
	kPGPError_Win32CommCtrlOpFailed	= -11944,
	kPGPError_Win32DllOpFailed		= -11943,
	kPGPError_Win32RegistryOpFailed	= -11942,
	kPGPError_Win32ResourceOpFailed	= -11941,
	kPGPError_Win32WindowOpFailed	= -11940,
	kPGPError_RPCFailed				= -11939,
	kPGPError_RPCGarbledMsg			= -11938,
	kPGPError_NoUnicodeEquivalent	= -11937,
	
	kPGPError_NetLARefused = -11936,
	kPGPError_NetLAMismatch = -11935,
	kPGPError_NetLATooManyRetrievals = -11934,
	kPGPError_LNCorrupt	= -11933,
	kPGPError_LACorrupt	= -11932,
	kPGPError_LNInvalid	= -11931,
	kPGPError_NetInvalidProdID = -11930,
	// PGPwde error code
	kPGPError_DiskNotFound = -11929,
	kPGPError_LastPFLError = kPGPPFLErrorBase + kPGPPFLErrorRange - 1
};

#define IsPGPError( err )		( (err) != kPGPError_NoErr )
#define IsntPGPError( err )		( (err) == kPGPError_NoErr )

PGP_BEGIN_C_DECLARATIONS

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpPFLErrors_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

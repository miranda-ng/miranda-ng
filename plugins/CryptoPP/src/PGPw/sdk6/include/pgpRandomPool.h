/*____________________________________________________________________________
	pgpRandomPool.h
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpRandomPool.h,v 1.13 1999/04/13 17:43:18 cpeterson Exp $
____________________________________________________________________________*/
#ifndef Included_pgpRandomPool_h	/* [ */
#define Included_pgpRandomPool_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

PGPUInt32 	PGPGlobalRandomPoolAddKeystroke( PGPInt32 event);
PGPUInt32 	PGPGlobalRandomPoolMouseMoved(void);
PGPError	PGPGlobalRandomPoolAddSystemState(void);

/* Extra functions for entropy estimation */
PGPUInt32 	PGPGlobalRandomPoolGetEntropy( void );
PGPUInt32 	PGPGlobalRandomPoolGetSize( void );
PGPUInt32	PGPGlobalRandomPoolGetMinimumEntropy( void );
PGPBoolean	PGPGlobalRandomPoolHasMinimumEntropy( void );


#if PGP_DEPRECATED
PGPUInt32 	PGPGlobalRandomPoolAddMouse( PGPUInt32 x, PGPUInt32 y);
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpRandomPool_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

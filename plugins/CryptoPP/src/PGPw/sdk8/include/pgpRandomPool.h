/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id: pgpRandomPool.h,v 1.7 2002/08/06 20:11:16 dallen Exp $
____________________________________________________________________________*/

#ifndef Included_pgpRandomPool_h	/* [ */
#define Included_pgpRandomPool_h

#include "pgpBase.h"
#include "pflTypes.h"

PGP_BEGIN_C_DECLARATIONS

PGPError	PGPSetRandSeedFile( PFLFileSpecRef randSeedFile );

PGPUInt32 	PGPGlobalRandomPoolAddKeystroke( PGPInt32 event);
PGPUInt32 	PGPGlobalRandomPoolMouseMoved(void);
PGPError	PGPGlobalRandomPoolAddSystemState(void);
PGPBoolean	PGPGlobalRandomPoolHasIntelRNG(void);

/* Extra functions for entropy estimation */
PGPUInt32 	PGPGlobalRandomPoolGetEntropy( void );
PGPUInt32 	PGPGlobalRandomPoolGetSize( void );
PGPUInt32	PGPGlobalRandomPoolGetMinimumEntropy( void );
PGPBoolean	PGPGlobalRandomPoolHasMinimumEntropy( void );


#if PGP_DEPRECATED
PGPUInt32 	PGPGlobalRandomPoolAddMouse( PGPUInt32 x, PGPUInt32 y);
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

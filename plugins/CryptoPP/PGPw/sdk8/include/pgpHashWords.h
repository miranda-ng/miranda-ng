/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id: pgpHashWords.h,v 1.2 2004/01/03 02:25:31 bgaiser Exp $
____________________________________________________________________________*/
#ifndef HASHWORDLIST_H
#define HASHWORDLIST_H

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

/*____________________________________________________________________________
	Hash word list types
____________________________________________________________________________*/
enum PGPHashWordList_
{
	kPGPHashWordList_Even					= 0,
	kPGPHashWordList_Odd					= 1,
	
	PGP_ENUM_FORCE( PGPHashWordList_ )
};
PGPENUM_TYPEDEF( PGPHashWordList_, PGPHashWordList );

/*____________________________________________________________________________
	Hash word functions
____________________________________________________________________________*/

	PGPError 
PGPGetHashWordString(
	PGPUInt32			index,
	PGPHashWordList		list,
	PGPChar8			hashWordString[ 12 ]);


PGP_END_C_DECLARATIONS

#endif


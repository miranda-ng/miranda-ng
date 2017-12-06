/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id: pgpEC.h,v 1.12 2003/12/13 01:20:39 dallen Exp $
____________________________________________________________________________*/

#ifndef PGP_EC_PUB_H
#define PGP_EC_PUB_H

PGP_BEGIN_C_DECLARATIONS

/* Wrapper for the ec* routines and data types. 
   These are exported functions from PGPsdk
 */

typedef signed char ** PGPECContextRef;
typedef signed short ** PGPECPointRef;
typedef signed int ** PGPECScalarRef;

enum PGPECMemoryUsage_
{	
	kPGPECMemoryMedium = 0,
	kPGPECMemoryHigh = 1,

	PGP_ENUM_FORCE( PGPECMemoryUsage_ )
};
PGPENUM_TYPEDEF( PGPECMemoryUsage_, PGPECMemoryUsage );

PGPError PGPECCreate2mContext( PGPMemoryMgrRef memoryMgr, 
	PGPSize bitsize, PGPECMemoryUsage memUsage, 
	PGPECContextRef *c );

PGPError PGPECFreeContext( PGPECContextRef p );

PGPError PGPECSetEC2mParamA( PGPECContextRef c, const unsigned *a );
PGPError PGPECSetEC2mParamB( PGPECContextRef c, const unsigned *b );
PGPError PGPECSetEC2mParamAInt( PGPECContextRef c, unsigned a );
PGPError PGPECSetEC2mParamBInt( PGPECContextRef c, unsigned b );

PGPError PGPECGetBufferSize( PGPECContextRef c, 
		PGPSize *coordinateSize,	PGPSize *scalarSize,
		PGPSize *pointDecomprSize,	PGPSize *pointComprSize );

PGPError PGPECScalarCreate( PGPECContextRef c, PGPECScalarRef *s, PGPBoolean isSecure );
PGPError PGPECScalarFree( PGPECScalarRef s );
PGPError PGPECScalarInsertBytes( PGPECScalarRef s, 
						const PGPByte *scalar /*network order*/, PGPECScalarRef G_ord );

PGPError PGPECPointCreate( PGPECContextRef c, PGPECPointRef *p );
PGPError PGPECPointFree( PGPECPointRef p );

PGPError PGPECPointExtractBytes( PGPECPointRef p, PGPByte *out, unsigned flags );
PGPError PGPECPointExtractXYBytes( PGPECPointRef p, PGPByte *out_x, PGPByte *out_y, unsigned flags );
PGPError PGPECPointInsertBytes( PGPECPointRef p, const PGPByte *in, unsigned flags );

PGPError PGPECPointAssignContext( PGPECPointRef p, PGPECContextRef c );

PGPError PGPECPointPrefBasis( PGPECPointRef p );

PGPError PGPECPointMul( PGPECPointRef p, 
	PGPECScalarRef scalar, 
	PGPBoolean isPrecomputed, /* PGPECPointPrecomp already called */
	PGPECPointRef out );

PGPError PGPECPointAdd( PGPECPointRef p0, const PGPECPointRef p1, PGPECPointRef sum );

PGPError PGPECPointSetZero( PGPECPointRef p );
PGPBoolean PGPECPointIsZero( PGPECPointRef p );

PGPBoolean PGPECPointIsConsistent( PGPECPointRef p );

PGPError PGPECPointCompress( PGPECPointRef p );
PGPError PGPECPointDecompress( PGPECPointRef p );

PGP_END_C_DECLARATIONS

#endif /* PGP_EC_PUB_H */

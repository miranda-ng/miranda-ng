/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id: pgpBigNum.h,v 1.7 2002/08/06 20:11:16 dallen Exp $
____________________________________________________________________________*/

#ifndef Included_pgpBigNum_h
#define Included_pgpBigNum_h

#include "pgpBase.h"
#include "pgpUtilities.h"

PGP_BEGIN_C_DECLARATIONS

typedef struct PGPBigNum *		PGPBigNumRef;
#define kPGPInvalidBigNumRef	( (PGPBigNumRef)NULL )

/*____________________________________________________________________________
	Bignum basics
____________________________________________________________________________*/
PGPError	PGPNewBigNum( PGPContextRef context, PGPBoolean secure,
					PGPBigNumRef *newBN );

PGPError	PGPFreeBigNum( PGPBigNumRef bn );

PGPError 	PGPCopyBigNum( PGPBigNumRef src, PGPBigNumRef * dest );
PGPError 	PGPAssignBigNum( PGPBigNumRef src, PGPBigNumRef dest );
PGPError	PGPSwapBigNum( PGPBigNumRef a, PGPBigNumRef b);

/*
** Move bytes between the given buffer and the given BigNum encoded in
** base 256.  I.e. after either of these, the buffer will be equal to
** (bn / 256^lsbyte) % 256^len.  The difference is which is altered to
** match the other!
*/
PGPError	PGPBigNumExtractBigEndianBytes( PGPBigNumRef bn,
					PGPByte *dest, PGPUInt32 lsbyte, PGPUInt32	len );
				
PGPError	PGPBigNumInsertBigEndianBytes(PGPBigNumRef bn,
					PGPByte const *src, PGPUInt32 lsbyte, PGPUInt32 len );

/* The same, but the buffer is little-endian. */
PGPError	PGPBigNumExtractLittleEndianBytes( PGPBigNumRef bn,
					PGPByte *dest, PGPUInt32 lsbyte, PGPUInt32	len );
				
PGPError	PGPBigNumInsertLittleEndianBytes(PGPBigNumRef bn,
					PGPByte const *src, PGPUInt32 lsbyte, PGPUInt32 len );

/* Return the least-significant bits (at least 16) of the BigNum */
PGPUInt16	PGPBigNumGetLSWord( PGPBigNumRef bn );

/*
 * Return the number of significant bits in the BigNum.
 * 0 or 1+floor(log2(src))
 */
PGPUInt32	PGPBigNumGetSignificantBits( PGPBigNumRef bn );

/*
 * Adds two bignums into dest.  Faster if dest is same as lhs or rhs.
 */
PGPError	PGPBigNumAdd( PGPBigNumRef lhs, PGPBigNumRef rhs,
					PGPBigNumRef dest );

/*
 * lhs-rhs.  dest and src may be the same, but bnSetQ(dest, 0) is faster.
 * if dest < src, returns error and dest is undefined.
 */
PGPError	PGPBigNumSubtract( PGPBigNumRef lhs, PGPBigNumRef rhs,
					PGPBigNumRef dest, PGPBoolean *underflow );

/* Return sign (-1, 0, +1) of a-b.  a <=> b --> bnCmpQ(a, b) <=> 0 */
PGPInt32	PGPBigNumCompareQ( PGPBigNumRef bn, PGPUInt16 sm );

/* dest = src, where 0 <= src < 2^16. */
PGPError	PGPBigNumSetQ( PGPBigNumRef dest, PGPUInt16 sm );

/* dest = bn + sm, where 0 <= sm < 2^16 */
PGPError	PGPBigNumAddQ( PGPBigNumRef bn, PGPUInt16 sm,
					PGPBigNumRef dest);

/* dest = bn + sm, where 0 <= sm < 2^16 */
PGPError	PGPBigNumSubtractQ( PGPBigNumRef bn, PGPUInt16 sm,
					PGPBigNumRef dest, PGPBoolean *underflow);

/* Return sign (-1, 0, +1) of a-b.  a <=> b --> bnCmp(a, b) <=> 0 */
PGPInt32	PGPBigNumCompare( PGPBigNumRef lhs, PGPBigNumRef rhs);

/* dest = src * src.  dest may be the same as src, but it costs time. */
PGPError	PGPBigNumSquare( PGPBigNumRef src, PGPBigNumRef dest);

/* dest = a * b.  dest may be the same as a or b, but it costs time. */
PGPError	PGPBigNumMultiply( PGPBigNumRef lhs, PGPBigNumRef rhs,
					PGPBigNumRef dest);

/* dest = a * b, where 0 <= b < 2^16.  dest and a may be the same. */
PGPError	PGPBigNumMultiplyQ( PGPBigNumRef lhs, PGPUInt16 sm,
					PGPBigNumRef dest);

/*
 * q = n/d, r = n%d.  r may be the same as n, but not d,
 * and q may not be the same as n or d.
 * re-entrancy issue: this temporarily modifies d, but restores
 * it for return.
 */
PGPError	PGPBigNumDivide( PGPBigNumRef numerator, PGPBigNumRef denominator,
					PGPBigNumRef quotient, PGPBigNumRef remainder);
/*
 * dest = n % d.  dest and src may be the same, but not dest and d.
 * re-entrancy issue: this temporarily modifies d, but restores
 * it for return.
 */
PGPError	PGPBigNumMod( PGPBigNumRef numerator, PGPBigNumRef denominator,
					PGPBigNumRef dest );

/* return src % d, where 0 <= d < 2^16.  */
PGPUInt16	PGPBigNumModQ( PGPBigNumRef numerator, PGPUInt16 denominator );

/* n = n^exp, modulo "mod"   "mod" *must* be odd */
PGPError	PGPBigNumExpMod( PGPBigNumRef n, PGPBigNumRef exponent,
					PGPBigNumRef mod, PGPBigNumRef dest );

/*
 * dest = n1^e1 * n2^e2, modulo "mod".  "mod" *must* be odd.
 * dest may be the same as n1 or n2.
 */
PGPError	PGPBigNumDoubleExpMod( PGPBigNumRef n1, PGPBigNumRef exponent1,
					PGPBigNumRef n2, PGPBigNumRef exponent2,
					PGPBigNumRef mod, PGPBigNumRef dest );

/* dest = 2^exp, modulo "mod"   "mod" *must* be odd */
PGPError	PGPBigNumTwoExpMod( PGPBigNumRef exponent, PGPBigNumRef mod,
					PGPBigNumRef dest );

/* dest = gcd(a, b).  The inputs may overlap arbitrarily. */
PGPError	PGPBigNumGCD( PGPBigNumRef a, PGPBigNumRef b, PGPBigNumRef dest );

/* dest = src^-1, modulo "mod".  dest may be the same as src. */
PGPError	PGPBigNumInv( PGPBigNumRef src, PGPBigNumRef mod,
					PGPBigNumRef dest );


/* Shift dest left "amt" places */
PGPError	PGPBigNumLeftShift( PGPBigNumRef dest, PGPUInt32 amt );

/* Shift dest right "amt" places, discarding low-order bits */
PGPError	PGPBigNumRightShift( PGPBigNumRef dest, PGPUInt32 amt );

/* right shift all low order 0-bits, return number of bits shifted */
PGPUInt16	PGPBigNumMakeOdd( PGPBigNumRef dest );


PGP_END_C_DECLARATIONS

#endif	/* Included_pgpBigNum_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/













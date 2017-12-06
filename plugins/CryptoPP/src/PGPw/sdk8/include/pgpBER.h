/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
    All rights reserved.

    $Id: pgpBER.h,v 1.15 2003/08/08 04:40:39 ajivsov Exp $
____________________________________________________________________________*/

/*
 * For extremely detailed explanation of all the BER types and
 * encoding, see X.208 (Specification of Abstract Syntax Notation One (ASN.1))
 * and X.209 (Specification of Basic Encoding Rules for Abstract Syntax
 * Notation One (ASN.1)).
 *
 * These two sources will tell you everything you would ever need (and want)
 * to know about BER and ASN.1.
 */

#ifndef Included_pgpBER_h  /* [ */
#define Included_pgpBER_h

#include "pgpPubTypes.h"
#include "pgpMemoryMgr.h"
#include "pgpSockets.h"

/* BER types */
/* The X.209 BER specification actually defines a lot more
 * types than is listed here, but these are the only ones
 * the current PGPldap implementation uses. */
enum PGPberType_
{
    kPGPberType_Boolean     = 0x01, /* tag = 0x01 */
    kPGPberType_Int         = 0x02, /* tag = 0x02 */
    kPGPberType_Bitstring   = 0x03, /* tag = 0x03 */
    kPGPberType_Octetstring = 0x04, /* tag = 0x04 */
    kPGPberType_NULL        = 0x05, /* tag = 0x05 */
	kPGPberType_ObjectID	= 0x06, /* tag = 0x06 */
    kPGPberType_Enumeration = 0x0A, /* tag = 0x0A */
	kPGPberType_PrintableString = 0x0D, /* tag = 0x0D */
    kPGPberType_Sequence    = 0x30, /* constructed, tag = 0x10 */
    kPGPberType_Set         = 0x31, /* constructed, tag = 0x11 */

    kPGPberType_None        = 0xFFFFFFFF
};
PGPENUM_TYPEDEF (PGPberType_, PGPberType);

enum PGPberFormatSpecifier_
{
    kPGPberFormatSpecifier_Boolean      = 'b',
    kPGPberFormatSpecifier_Int          = 'i',
    kPGPberFormatSpecifier_Octetstring  = 'o',
    kPGPberFormatSpecifier_String       = 's',
    kPGPberFormatSpecifier_StringVector = 'v',
    kPGPberFormatSpecifier_BERVector    = 'V',  /* strings and lengths */
    kPGPberFormatSpecifier_NULL         = 'n',
    kPGPberFormatSpecifier_Enumeration  = 'e',
    kPGPberFormatSpecifier_Tag          = 't',
    kPGPberFormatSpecifier_BeginSequence= '{',
    kPGPberFormatSpecifier_EndSequence  = '}',
    kPGPberFormatSpecifier_BeginSet     = '[',
    kPGPberFormatSpecifier_EndSet       = ']',

	kPGPberFormatSpecifier_Force		= '*'
};
PGPENUM_TYPEDEF (PGPberFormatSpecifier_, PGPberFormatSpecifier);


typedef struct PGPberElement *	PGPberElementRef;

#define kInvalidPGPberElementRef	( (PGPberElementRef) NULL)
#define PGPberElementRefIsValid(ber)	\
			( (ber) != kInvalidPGPberElementRef )
#define PGPValidateBERElementRef(ber) \
			PGPValidateParam( PGPberElementRefIsValid( ber ) )

typedef struct PGPberValue
{
    PGPSize     length;
    PGPByte     *value;
} PGPberValue;

/* Functions */

	PGPError
PGPNewBERElement(
	PGPContextRef		context,
	PGPberElementRef *	ber );

/* ber_free */
	PGPError
PGPFreeBERElement(
	PGPberElementRef	ber );

/* BER encoding functions */
    PGPError
PGPberAppend(
    PGPberElementRef	ber, 
    const PGPChar8 *	s, 
	... );

	PGPError
PGPberGetEncoding(
	PGPberElementRef	ber,
	PGPByte **			encoding );

/* BER decoding functions */
	PGPError
PGPberRead(
	PGPberElementRef	ber,
	const PGPChar8 *	fmt,
	... );

	PGPError
PGPberGetLength(
	PGPberElementRef	ber,
	PGPSize *			length );

	PGPError
PGPberRewind(
	PGPberElementRef	ber );

	PGPError
PGPberNextPrimitive(
	PGPberElementRef	ber );

	PGPError
PGPberNextConstructed(
	PGPberElementRef	ber );

	PGPError
PGPberNext(
	PGPberElementRef	ber );

	PGPError
PGPberSkip(
	PGPberElementRef	ber );

	PGPError
PGPberPeek(
	PGPberElementRef	ber,
	PGPberType *		tag,
	PGPSize *			len );

	PGPError
PGPberGetIndex(
	PGPberElementRef	ber,
	PGPUInt32 *			index );

	PGPError
PGPberSetIndex(
	PGPberElementRef	ber,
	PGPUInt32			index );

	PGPError
PGPberReadResponse(
	PGPberElementRef	ber,
	PGPSocketRef		sock );

	PGPError
PGPberSetData(
	PGPberElementRef	ber,
	PGPByte *			data,
	PGPSize				len );


#endif /* ] Included_pgpBER_h */


/*__Editor_settings____

    Local Variables:
    tab-width: 4
    End:
    vi: ts=4 sw=4
    vim: si
_____________________*/

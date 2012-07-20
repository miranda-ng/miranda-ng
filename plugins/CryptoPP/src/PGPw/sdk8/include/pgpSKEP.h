/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id: pgpSKEP.h,v 1.1 2004/04/01 11:45:40 wprice Exp $
____________________________________________________________________________*/
#ifndef Included_pgpSKEP_h	/* [ */
#define Included_pgpSKEP_h

#include "pgpConfig.h"
#include "pgpBase.h"
#include "pgpErrors.h"
#include "pgpShare.h"
#include "pgpKeys.h"
#include "pgpTLS.h"

typedef struct PGPskep *	PGPskepRef;

#define	kInvalidPGPskepRef			((PGPskepRef) NULL)
#define PGPskepRefIsValid( ref )	( (ref) != kInvalidPGPskepRef )

typedef enum _PGPskepEventType
{
	kPGPskepEvent_NullEvent			= 0,	/* Nothing is happening */
	kPGPskepEvent_ListenEvent		= 1,	/* Listening for data */
	kPGPskepEvent_ConnectEvent		= 2,	/* Connection established */
	kPGPskepEvent_AuthenticateEvent = 3,	/* Remote site authenticated */
	kPGPskepEvent_ProgressEvent		= 4,	/* Data flow progress */
	kPGPskepEvent_CloseEvent		= 5,	/* Connection closing */
	kPGPskepEvent_ShareEvent		= 6,	/* Share received */
	kPGPskepEvent_PassphraseEvent	= 7		/* Passphrase needed */
} PGPskepEventType;

typedef struct _PGPskepEventAuthenticateData
{
	PGPKeyDBObjRef			remoteKey;
	const char *			remoteHostname;
	const char *			remoteIPAddress;
	PGPtlsCipherSuiteNum	tlsCipher;
} PGPskepEventAuthenticateData;

typedef struct _PGPskepEventProgressData
{
	PGPUInt32	bytesSoFar;
	PGPUInt32	bytesTotal;
} PGPskepEventProgressData;

typedef struct _PGPskepEventShareData
{
	PGPShareRef	shares;
} PGPskepEventShareData;

typedef struct _PGPskepEventPassphraseData
{
	char *		passphrase;
	PGPByte *	passkey;
	PGPSize		passkeySize;
} PGPskepEventPassphraseData;

typedef union _PGPskepEventData
{
	PGPskepEventAuthenticateData	ad;
	PGPskepEventProgressData		pd;
	PGPskepEventShareData			sd;
	PGPskepEventPassphraseData		ppd;
} PGPskepEventData;

typedef struct _PGPskepEvent
{
	PGPskepEventType	type;
	PGPskepEventData	data;
} PGPskepEvent;

typedef PGPError (*PGPskepEventHandler)(PGPskepRef skep,
						PGPskepEvent *event, PGPUserValue userValue);


PGP_BEGIN_C_DECLARATIONS

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

PGPError 		PGPNewSKEP(PGPContextRef context, 
						PGPtlsContextRef tlsContext,
						PGPskepRef *skep);

PGPError		PGPskepSetEventHandler(PGPskepRef skep,
						PGPskepEventHandler handler, PGPUserValue userValue);

PGPError 		PGPskepSendShares(PGPskepRef skep, PGPKeyDBObjRef authKey,
						const char *passphrase, PGPShareRef shares,
						const char *destSocketAddress);

PGPError 		PGPskepReceiveShares(PGPskepRef skep, PGPKeyDBObjRef authKey,
						const char *passphrase);

PGPError 		PGPskepCancel(PGPskepRef skep);

PGPError 		PGPFreeSKEP(PGPskepRef skep);

PGPContextRef	PGPGetSKEPContext(PGPskepRef skep);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpSKEP_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

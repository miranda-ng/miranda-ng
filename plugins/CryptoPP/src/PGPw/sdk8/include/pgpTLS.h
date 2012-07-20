/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id: pgpTLS.h,v 1.13 2004/04/06 05:00:58 wprice Exp $
____________________________________________________________________________*/
#ifndef Included_PGPtls_h	/* [ */
#define Included_PGPtls_h

#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS


typedef struct PGPtlsContext *			PGPtlsContextRef;
typedef const struct PGPtlsContext *	PGPtlsConstContextRef;

#define	kInvalidPGPtlsContextRef		((PGPtlsContextRef) NULL)
#define PGPtlsContextRefIsValid( ref )	( (ref) != kInvalidPGPtlsContextRef )

typedef struct PGPtlsSession *			PGPtlsSessionRef;
typedef const struct PGPtlsSession *	PGPtlsConstSessionRef;

#define	kInvalidPGPtlsSessionRef		((PGPtlsSessionRef) NULL)
#define PGPtlsSessionRefIsValid( ref )	( (ref) != kInvalidPGPtlsSessionRef )


typedef PGPFlags		PGPtlsFlags;
#define kPGPtlsFlags_ServerSide				0x01
#define kPGPtlsFlags_ClientSide				0x02
#define kPGPtlsFlags_RequestClientCert		0x04
#define kPGPtlsFlags_NonBlockingIO			0x08

enum PGPtlsCipherSuiteNum_
{
	kPGPtls_TLS_NULL_WITH_NULL_NULL					= 0,
	kPGPtls_TLS_PGP_DHE_DSS_WITH_CAST_CBC_SHA		= 1,
	kPGPtls_TLS_PGP_DHE_RSA_WITH_CAST_CBC_SHA		= 2,
	kPGPtls_TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA		= 3,
	kPGPtls_TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA		= 4,
	kPGPtls_TLS_RSA_WITH_3DES_EDE_CBC_SHA			= 5,
	kPGPtls_TLS_RSA_WITH_IDEA_CBC_SHA				= 6,
	kPGPtls_TLS_PGP_RSA_WITH_CAST_CBC_SHA			= 7,
	kPGPtls_TLS_PGP_DHE_DSS_WITH_NULL_SHA			= 8,
	kPGPtls_TLS_DHE_DSS_WITH_NULL_SHA				= 9,
	kPGPtls_TLS_RSA_WITH_ARC4_128_SHA				= 10,

	kPGPtls_TLS_RSA_WITH_AES_128_CBC_SHA			= 11,
	kPGPtls_TLS_DHE_DSS_WITH_AES_128_CBC_SHA		= 12,
	kPGPtls_TLS_DHE_RSA_WITH_AES_128_CBC_SHA		= 13,

	kPGPtls_TLS_RSA_WITH_AES_256_CBC_SHA			= 15,
	kPGPtls_TLS_DHE_DSS_WITH_AES_256_CBC_SHA		= 16,
	kPGPtls_TLS_DHE_RSA_WITH_AES_256_CBC_SHA		= 17,

	PGP_ENUM_FORCE( PGPtlsCipherSuiteNum_ )
};
PGPENUM_TYPEDEF( PGPtlsCipherSuiteNum_, PGPtlsCipherSuiteNum );

enum PGPtlsProtocolState_
{
	kPGPtls_IdleState			= 0,
	kPGPtls_FatalErrorState		= 1,
	kPGPtls_ClosedState			= 2,
	kPGPtls_HandshakeState		= 3,
	kPGPtls_ReadyState			= 4,

	PGP_ENUM_FORCE( PGPtlsProtocolState_ )
};
PGPENUM_TYPEDEF( PGPtlsProtocolState_, PGPtlsProtocolState );

enum PGPtlsPrime_
{
	kPGPtls_DHPrime1024		= 0,
	kPGPtls_DHPrime1536		= 1,
	kPGPtls_DHPrime2048		= 2,
	kPGPtls_DHPrime3072		= 3,
	kPGPtls_DHPrime4096		= 4,

	PGP_ENUM_FORCE( PGPtlsPrime_ )
};
PGPENUM_TYPEDEF( PGPtlsPrime_, PGPtlsPrime );

enum PGPtlsAlert_
{
	kPGPtls_AT_CloseNotify			= 0,
	kPGPtls_AT_UnexpectedMessage	= 10,	/* FATAL */
	kPGPtls_AT_BadRecordMAC			= 20,	/* FATAL */
	kPGPtls_AT_DecryptionFailed		= 21,	/* FATAL */
	kPGPtls_AT_RecordOverflow		= 22,	/* FATAL */
	kPGPtls_AT_DecompressionFailure	= 30,	/* FATAL */
	kPGPtls_AT_HandshakeFailure		= 40,	/* FATAL */
	kPGPtls_AT_NoCertificate		= 41,	/* SSL3  */
	kPGPtls_AT_BadCertificate		= 42,
	kPGPtls_AT_UnsupportedCert		= 43,
	kPGPtls_AT_CertRevoked			= 44,
	kPGPtls_AT_CertExpired			= 45,
	kPGPtls_AT_CertUnknown			= 46,
	kPGPtls_AT_IllegalParameter		= 47,	/* FATAL */
	kPGPtls_AT_UnknownCA			= 48,	/* FATAL */
	kPGPtls_AT_AccessDenied			= 49,	/* FATAL */
	kPGPtls_AT_DecodeError			= 50,	/* FATAL */
	kPGPtls_AT_DecryptError			= 51,
	kPGPtls_AT_ExportRestriction	= 60,	/* FATAL */
	kPGPtls_AT_ProtocolVersion		= 70,	/* FATAL */
	kPGPtls_AT_InsufficientSecurity	= 71,	/* FATAL */
	kPGPtls_AT_InternalError		= 80,	/* FATAL */
	kPGPtls_AT_UserCancelled		= 90,
	kPGPtls_AT_NoRenegotiation		= 100,
	
	kPGPtls_AT_None					= 255,

	PGP_ENUM_FORCE( PGPtlsAlert_ )
};
PGPENUM_TYPEDEF( PGPtlsAlert_, PGPtlsAlert );

/* The Send and Receive function pointers should return
	kPGPError_TLSWouldBlock when the socket is non-blocking and the
	call would block.  The Send and Receive functions passed in will
	need to translate the platform-specific socket error in appropriate
	cases by using calls such as WSAGetLastError() on Win32.  Remember
	to call PGPtlsSendQueueIdle for non-blocking sockets also if
	kPGPError_TLSWouldBlock is returned from a send on a non-blocking
	socket. */

typedef PGPInt32 (* PGPtlsReceiveProcPtr)(void *inData, void *outBuffer,
										PGPInt32 outBufferSize);
typedef PGPInt32 (* PGPtlsSendProcPtr)(void *inData, const void *inBuffer,
										PGPInt32 inBufferLength);
typedef PGPInt32 (* PGPtlsPeekProcPtr)(void *inData, void *outBuffer, 
										PGPInt32 outBufferSize);

PGPError 	PGPNewTLSContext( PGPContextRef context,
					PGPtlsContextRef *outRef );

PGPError 	PGPFreeTLSContext( PGPtlsContextRef ref );

/*____________________________________________________________________________
	The following function activates or deactivates the session key cache
	for TLS sessions.  This defaults to on but can be deactivated with this
	function to force all connections to proceed through the entire
	handshake.
____________________________________________________________________________*/
PGPError	PGPtlsSetCache( PGPtlsContextRef ref, PGPBoolean useCache );

PGPError	PGPtlsClearCache( PGPtlsContextRef ref );

PGPError	PGPNewTLSSession( PGPtlsContextRef ref, PGPtlsSessionRef *outRef );

PGPError 	PGPFreeTLSSession( PGPtlsSessionRef ref );

PGPError	PGPCopyTLSSession( PGPtlsSessionRef ref, PGPtlsSessionRef *outRef );

/* Default options are client side and no client cert request */
PGPError	PGPtlsSetProtocolOptions( PGPtlsSessionRef ref,
					PGPtlsFlags options );
					

/*____________________________________________________________________________
	The following function must be called to cleanly close a TLS
	connection.  If it is not called, the session will not be able
	to be resumed from the session cache.
	
	In the event the application determines any problem with the
	connection such as the remote key not being valid, call this
	function with dontCache set to true in order to not cache the
	session keys.
____________________________________________________________________________*/
PGPError	PGPtlsClose( PGPtlsSessionRef	ref,
					PGPBoolean				dontCache );

/*____________________________________________________________________________
	The following function must be called to identify cached TLS session.
	
	During a handshake resulting in reused TLS sessions no certificate exchange
	is taking place, so certificate or certificate chain cannot be queried by 
	a call to PGPtlsGetRemoteAuthenticatedKey(). Remote peer's authentication 
	keys are assumed to be authorized by the application when it earlier placed
   	a session into a cache by calling PGPtlsClose(session, FALSE), so the "TRUE"
   	return value means "already authorized session".

	There is no need to call this function if no caching is done, for example,  
   	if PGPtlsClose(session, FALSE) is never called or cache is 
	disabled / cleared in PGPtlsContextRef.
____________________________________________________________________________*/
PGPError	PGPtlsIsReusedSession( PGPtlsSessionRef ref, PGPBoolean *reused );

/*____________________________________________________________________________
	The following function must be called to initiate the PGPtls session.
	Once a TLS session has been assigned to a socket, no data can be sent
	over that socket by the application until the handshake is completed.
	Handshake completion is indicated by completion of this call without
	error or by checking the state of the PGPtlsSession.  It will be
	kPGPtls_ReadyState when the application layer may send and receive
	data securely.
	
	This function performs all negotiation of the TLS connection.
____________________________________________________________________________*/
PGPError	PGPtlsHandshake( PGPtlsSessionRef ref );

/*____________________________________________________________________________
	The following function should be called before PGPtlsHandshake.
	In the general case, the remoteID will be an IP address.  This
	is provided to PGPtls in order to allow it to cache the current
	session and be able to look it up later.  If the remoteID passed
	into a future session is the same as a previously cached session,
	PGPtls will attempt to resume the session.
____________________________________________________________________________*/
PGPError	PGPtlsSetRemoteUniqueID( PGPtlsSessionRef ref,
					PGPUInt32 remoteID );

/*____________________________________________________________________________
	The following function sets the local private authenticating key.
	
	The passphrase and key are retained in memory.  By default, no
	key is specified and a client side session will return no key in the
	client key exchange message to the server.
	It is an error not to specify a key on a server side TLS session.
	This function must be passed either PGPOPassphrase or PGPOPasskeyBuffer.
	You may pass in just a PGP key, PGP w/ X.509 cert, or both -- and they
	must be the same -- the cert must be from the key.  For an X.509 cert,
	the inCertChain keyset must contain the keys of all keys in the
	certificate chain for that certificate up to the root.  The inCertChain
	keyset must remain valid for the lifetime of the TLS connection.
____________________________________________________________________________*/
PGPError	PGPtlsSetLocalPrivateKey( PGPtlsSessionRef ref,
					PGPKeyDBObjRef		inKeyObject,
					PGPKeySetRef		inCertChain,
					PGPOptionListRef	firstOption, ... );
					
/*____________________________________________________________________________
	The following function sets the preferred cipher suite.
	
	There is no guarantee that cipher will actually be negotiated,
	but it will be attempted in preference to others.
____________________________________________________________________________*/
PGPError	PGPtlsSetPreferredCipherSuite( PGPtlsSessionRef ref,
					PGPtlsCipherSuiteNum cipher );

/*____________________________________________________________________________
	The following function sets the desired DH prime.
	
	The requested primes are drawn from a set of primes hard-coded
	into PGPtls.  New primes can be added in a fully compatible
	fashion since the server sends the prime to the client, but this
	version of the API does not support passing in a desired prime.  The
	default prime if this function is not called is kPGPtls_DHPrime2048.
____________________________________________________________________________*/
PGPError	PGPtlsSetDHPrime( PGPtlsSessionRef ref,
					PGPtlsPrime prime );

/*____________________________________________________________________________
	The following function gets the authenticated remote key after a
	successful handshake.  You must call this function after a successful
	handshake to verify that the remote key is authorized to make the
	connection.
____________________________________________________________________________*/
PGPError	PGPtlsGetRemoteAuthenticatedKey( PGPtlsSessionRef ref,
					PGPKeyDBObjRef *outKey,
					PGPKeyDBRef *	outKeyDB );

/*____________________________________________________________________________
	The following function returns the negotiated symmetric cipher.
	
	This function will return an error if called before a successful
	handshake.
____________________________________________________________________________*/
PGPError	PGPtlsGetNegotiatedCipherSuite( PGPtlsSessionRef ref,
					PGPtlsCipherSuiteNum *outCipher );

PGPError	PGPtlsGetState( PGPtlsSessionRef ref,
					PGPtlsProtocolState *outState );

/*____________________________________________________________________________
	The following two functions process data through TLS.
	
	It is an error to call these functions without having set a
	Read function pointer or Write function pointer.  Most applications
	will never need to use these functions as the function pointers
	are automatically configured by PGPsockets, and these functions
	are automatically called by the PGPsockets implementations of
	PGPWrite and PGPRead whenever a PGPtlsSessionRef has been set for
	a given socket.
____________________________________________________________________________*/
PGPError	PGPtlsReceive( PGPtlsSessionRef ref,
					void *			outBuffer,
					PGPSize *		bufferSize );

PGPError	PGPtlsSend( PGPtlsSessionRef ref,
					const void *	inBuffer,
					PGPSize 		inBufferLength );
					
PGPError	PGPtlsSetReceiveCallback( PGPtlsSessionRef	ref,
					PGPtlsReceiveProcPtr	tlsReceiveProc,
					void *					inData );

PGPError	PGPtlsSetSendCallback( PGPtlsSessionRef ref,
					PGPtlsSendProcPtr		tlsSendProc,
					void *					inData );

PGPError	PGPtlsSetPeekCallback( PGPtlsSessionRef	ref,
					PGPtlsPeekProcPtr		tlsPeekProc,
					void *					inData );

/*____________________________________________________________________________
	The following function is necessary *only* on a non-blocking socket.
	If a call to PGPtlsSend returns kPGPError_TLSWouldBlock, call
	the following function repeatedly until that error is no longer
	returned in order to make sure data is sent.  Another call to
	PGPtlsSend will also call this function automatically and queue
	any new data if necessary.
____________________________________________________________________________*/
PGPError	PGPtlsSendQueueIdle( PGPtlsSessionRef ref );

PGPSize		PGPtlsReceiveBufferSize( PGPtlsSessionRef	ref );

/*____________________________________________________________________________
	The following function gets the ID of the fatal alert which caused
	the TLS session to abort and go into the kPGPtls_FatalErrorState.
____________________________________________________________________________*/
PGPError		PGPtlsGetAlert( PGPtlsSessionRef ref, PGPtlsAlert *outAlert );
const PGPChar * PGPtlsGetAlertString( PGPtlsAlert alert );

PGP_END_C_DECLARATIONS

#endif /* ] Included_PGPtls_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

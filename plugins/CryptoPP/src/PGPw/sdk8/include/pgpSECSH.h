/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id: pgpSECSH.h,v 1.5 2002/08/06 20:11:16 dallen Exp $
____________________________________________________________________________*/
#ifndef Included_PGPsecsh_h	/* [ */
#define Included_PGPsecsh_h

#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS


/* Message Types */
#define	kPGPsecsh_Msg_None						0
#define	kPGPsecsh_Msg_Disconnect				1
#define	kPGPsecsh_SMsg_PublicKey				2
#define	kPGPsecsh_CMsg_SessionKey				3
#define	kPGPsecsh_CMsg_User						4
#define	kPGPsecsh_CMsg_AuthRHosts				5
#define	kPGPsecsh_CMsg_AuthRSA					6
#define	kPGPsecsh_SMsg_AuthRSAChallenge			7
#define	kPGPsecsh_CMsg_AuthRSAResponse			8
#define	kPGPsecsh_CMsg_AuthRSAPassword			9
#define	kPGPsecsh_CMsg_RequestPTY				10
#define	kPGPsecsh_CMsg_WindowSize				11
#define	kPGPsecsh_CMsg_ExecShell				12
#define	kPGPsecsh_CMsg_ExecCommand				13
#define	kPGPsecsh_SMsg_Success					14
#define	kPGPsecsh_SMsg_Failure					15
#define	kPGPsecsh_CMsg_StdinData				16
#define	kPGPsecsh_SMsg_StdoutData				17
#define	kPGPsecsh_SMsg_StderrData				18
#define	kPGPsecsh_CMsg_EOF						19
#define	kPGPsecsh_SMsg_ExitStatus				20
#define	kPGPsecsh_Msg_ChannelOpenConfirm		21
#define	kPGPsecsh_Msg_ChannelOpenFailure		22
#define	kPGPsecsh_Msg_ChannelData				23
#define	kPGPsecsh_Msg_ChannelClose				24
#define	kPGPsecsh_Msg_ChannelCloseConfirm		25
#define	kPGPsecsh_SMsg_X11Open					27
#define	kPGPsecsh_CMsg_PortForwardRequest		28
#define	kPGPsecsh_Msg_PortOpen					29
#define	kPGPsecsh_CMsg_AgentRequestForwarding	30
#define	kPGPsecsh_SMsg_AgentOpen				31
#define	kPGPsecsh_Msg_Ignore					32
#define	kPGPsecsh_CMsg_ExitConfirm				33
#define	kPGPsecsh_CMsg_X11RequestForwarding		34
#define	kPGPsecsh_CMsg_AuthRHostsRSA			35
#define	kPGPsecsh_SMsg_Debug					36
#define	kPGPsecsh_CMsg_RequestCompression		37
#define	kPGPsecsh_CMsg_MaxPacketSize			38
#define	kPGPsecsh_CMsg_AuthTIS					39
#define	kPGPsecsh_SMsg_AuthTISChallenge			40
#define	kPGPsecsh_CMsg_AuthTISResponse			41
#define	kPGPsecsh_CMsg_AuthKerberos				42
#define	kPGPsecsh_SMsg_AuthKerberosResponse		43
#define	kPGPsecsh_CMsg_HaveKerberosTgt			44


typedef struct PGPsecshContext *			PGPsecshContextRef;
typedef const struct PGPsecshContext *		PGPsecshConstContextRef;

#define	kInvalidPGPsecshContextRef			((PGPsecshContextRef) NULL)
#define PGPsecshContextRefIsValid( ref )	( (ref) != kInvalidPGPsecshContextRef )

typedef struct PGPsecshSession *			PGPsecshSessionRef;
typedef const struct PGPsecshSession *		PGPsecshConstSessionRef;

#define	kInvalidPGPsecshSessionRef			((PGPsecshSessionRef) NULL)
#define PGPsecshSessionRefIsValid( ref )	( (ref) != kInvalidPGPsecshSessionRef )


typedef PGPFlags		PGPsecshFlags;
#define kPGPsecshFlags_ServerSide				0x01
#define kPGPsecshFlags_ClientSide				0x02
#define kPGPsecshFlags_NonBlockingIO			0x04

typedef PGPFlags		PGPsecshProtocolFlags;
#define kPGPsecshProtocolFlags_ScreenNumber				0x01
#define kPGPsecshProtocolFlags_HostInFwdOpen			0x02

enum PGPsecshProtocolState_
{
	kPGPsecsh_IdleState				= 0,
	kPGPsecsh_FatalErrorState		= 1,
	kPGPsecsh_ClosedState			= 2,
	kPGPsecsh_HandshakeState		= 3,
	kPGPsecsh_ReadyState			= 4,

	PGP_ENUM_FORCE( PGPsecshProtocolState_ )
};
PGPENUM_TYPEDEF( PGPsecshProtocolState_, PGPsecshProtocolState );

enum PGPsecshAlert_
{
	kPGPsecsh_AT_CloseNotify			= 0,
	kPGPsecsh_AT_UnexpectedMessage		= 10,	/* FATAL */
	kPGPsecsh_AT_BadRecordCRC			= 20,	/* FATAL */
	kPGPsecsh_AT_DecryptionFailed		= 21,	/* FATAL */
	kPGPsecsh_AT_RecordOverflow			= 22,	/* FATAL */
	kPGPsecsh_AT_DecompressionFailure	= 30,	/* FATAL */
	kPGPsecsh_AT_HandshakeFailure		= 40,	/* FATAL */
	kPGPsecsh_AT_IDFailure				= 41,
	kPGPsecsh_AT_UnsupportedVersion		= 42,
	kPGPsecsh_AT_UnsupportedCert		= 43,
	kPGPsecsh_AT_CertRevoked			= 44,
	kPGPsecsh_AT_CertExpired			= 45,
	kPGPsecsh_AT_CertUnknown			= 46,
	kPGPsecsh_AT_IllegalParameter		= 47,	/* FATAL */
	kPGPsecsh_AT_UnknownCA				= 48,	/* FATAL */
	kPGPsecsh_AT_AccessDenied			= 49,	/* FATAL */
	kPGPsecsh_AT_DecodeError			= 50,	/* FATAL */
	kPGPsecsh_AT_DecryptError			= 51,
	kPGPsecsh_AT_ExportRestriction		= 60,	/* FATAL */
	kPGPsecsh_AT_ProtocolVersion		= 70,	/* FATAL */
	kPGPsecsh_AT_InsufficientSecurity	= 71,	/* FATAL */
	kPGPsecsh_AT_InternalError			= 80,	/* FATAL */
	kPGPsecsh_AT_UserCancelled			= 90,
	kPGPsecsh_AT_NoRenegotiation		= 100,
	
	kPGPsecsh_AT_None					= 255,

	PGP_ENUM_FORCE( PGPsecshAlert_ )
};
PGPENUM_TYPEDEF( PGPsecshAlert_, PGPsecshAlert );

/* The Send and Receive function pointers should return
	kPGPError_SECSHWouldBlock when the socket is non-blocking and the
	call would block.  The Send and Receive functions passed in will
	need to translate the platform-specific socket error in appropriate
	cases by using calls such as WSAGetLastError() on Win32.  Remember
	to call PGPsecshSendQueueIdle for non-blocking sockets also if
	kPGPError_SECSHWouldBlock is returned from a send on a non-blocking
	socket. */

typedef PGPInt32 (* PGPsecshReceiveProcPtr)(void *inData, void *outBuffer,
											PGPInt32 inBufferSize);
typedef PGPInt32 (* PGPsecshSendProcPtr)(void *inData, const void *inBuffer,
											PGPInt32 inBufferLength);


PGPError 	PGPNewSECSHContext( PGPContextRef context,
					PGPsecshContextRef *outRef );

PGPError 	PGPFreeSECSHContext( PGPsecshContextRef ref );

/*____________________________________________________________________________
	The following function activates or deactivates the session key cache
	for SECSH sessions.  This defaults to on but can be deactivated with this
	function to force all connections to proceed through the entire
	handshake.
____________________________________________________________________________*/
PGPError	PGPsecshSetCache( PGPsecshContextRef ref, PGPBoolean useCache );

PGPError	PGPsecshClearCache( PGPsecshContextRef ref );

PGPError	PGPNewSECSHSession( PGPsecshContextRef ref, PGPsecshSessionRef *outRef );

PGPError 	PGPFreeSECSHSession( PGPsecshSessionRef ref );

PGPError	PGPCopySECSHSession( PGPsecshSessionRef ref, PGPsecshSessionRef *outRef );

/* Default options are: client side, no protocol flags */
PGPError	PGPsecshSetProtocolOptions( PGPsecshSessionRef ref,
					PGPsecshFlags 			options,
					PGPsecshProtocolFlags	pflags );
					

/*____________________________________________________________________________
	The following function must be called to cleanly close a SECSH
	connection.  If it is not called, the session will not be able
	to be resumed from the session cache.
	
	In the event the application determines any problem with the
	connection such as the remote key not being valid, call this
	function with dontCache set to true in order to not cache the
	session keys.
____________________________________________________________________________*/
PGPError	PGPsecshClose( PGPsecshSessionRef	ref,
					PGPBoolean				dontCache );

/*____________________________________________________________________________
	The following function must be called to initiate the PGPsecsh session.
	Once a SECSH session has been assigned to a socket, no data can be sent
	over that socket by the application until the handshake is completed.
	Handshake completion is indicated by completion of this call without
	error or by checking the state of the PGPsecshSession.  It will be
	kPGPsecsh_ReadyState when the application layer may send and receive
	data securely.
	
	This function performs all negotiation of the SECSH connection.
____________________________________________________________________________*/
PGPError	PGPsecshHandshake( PGPsecshSessionRef ref );

/*____________________________________________________________________________
	The following function sets the local private authenticating key,
	as well as other relevant data.
	
	The passphrase, key etc. are retained in memory.
	It is an error not to specify a key.
	This function must be passed either PGPOPassphrase or PGPOPasskeyBuffer.
	inKeyObject must be in a PGP private key.  inHostKeys should be a keyset
	where the host key of the remote system is found, if we are checking
	for consistency of that key.  See PGPsecshGetRemoteAuthenticatedKey
	for how to learn what host key was used.
	inUserName is used to log in on the remote system.
	inHostName is used for the username on the host key if it isn't found
	on the inHostKeys keyset.
____________________________________________________________________________*/
PGPError	PGPsecshSetLocalPrivateKey( PGPsecshSessionRef ref,
					char *				inUserName,
					PGPKeyDBObjRef		inKeyObject,
					char *				inHostName,
					PGPKeySetRef		inHostKeys,
					PGPOptionListRef	firstOption, ... );
					
/*____________________________________________________________________________
	The following function gets the authenticated remote host key after a
	successful handshake.  You can call this function after a successful
	handshake to verify that the remote key is authorized to make the
	connection.  The key returned will be on the inHostKeys keyset if it
	matched one of the keys there; if it is a new host key which was not
	in that keyset then it will be in a keyset & keydb of its own.
____________________________________________________________________________*/
PGPError	PGPsecshGetRemoteAuthenticatedKey( PGPsecshSessionRef ref,
					PGPKeyDBObjRef *outKey,
					PGPKeyDBRef *	outKeyDB );

/*____________________________________________________________________________
	The following function exports a PGP key in SECSH format.  The resulting
	line can be copied into the SECSH host file.  inKey is the key to
	export, and inUserName is the name which is put at the end of the line,
	which is used only as a convenient label by SECSH software.
	The data is returned in a null-terminated allocated buffer which the
	caller should free.
____________________________________________________________________________*/
	PGPError
PGPsecshExportPublicKey(
					PGPKeyDBObjRef			inKey,
					char *					inUserName,
					char **					outBuffer,
					PGPSize *				outLength);

/*____________________________________________________________________________
	The following two functions process data through SECSH.
	
	It is an error to call these functions without having set a
	Read function pointer or Write function pointer.
____________________________________________________________________________*/
PGPError	PGPsecshReceive( PGPsecshSessionRef ref,
					PGPByte *		outType,
					void **			outBuffer,
					PGPSize *		bufferSize );

PGPError	PGPsecshSend( PGPsecshSessionRef ref,
					PGPByte			pktType,
					const void *	inBuffer,
					PGPSize 		inBufferLength );
					
/*____________________________________________________________________________
	The following two functions set the callbacks which do the actual I/O.

	The inData parameter is passed to the callback and may be e.g. a socket
	handle.
____________________________________________________________________________*/
PGPError	PGPsecshSetReceiveCallback( PGPsecshSessionRef	ref,
					PGPsecshReceiveProcPtr	secshReceiveProc,
					void *					inData );

PGPError	PGPsecshSetSendCallback( PGPsecshSessionRef ref,
					PGPsecshSendProcPtr		secshSendProc,
					void *					inData );


/*____________________________________________________________________________
	The following function is necessary *only* on a non-blocking socket.
	If a call to PGPsecshSend returns kPGPError_SECSHWouldBlock, call
	the following function repeatedly until that error is no longer
	returned in order to make sure data is sent.  Another call to
	PGPsecshSend will also call this function automatically and queue
	any new data if necessary.
____________________________________________________________________________*/
PGPError	PGPsecshSendQueueIdle( PGPsecshSessionRef ref );

PGPSize		PGPsecshReceiveBufferSize( PGPsecshSessionRef	ref );

/*____________________________________________________________________________
	The following function gets the ID of the fatal alert which caused
	the SECSH session to abort and go into the kPGPsecsh_FatalErrorState.
____________________________________________________________________________*/
PGPError	PGPsecshGetAlert( PGPsecshSessionRef ref, PGPsecshAlert *outAlert );


PGP_END_C_DECLARATIONS

#endif /* ] Included_PGPsecsh_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

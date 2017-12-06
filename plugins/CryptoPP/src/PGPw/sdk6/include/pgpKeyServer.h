/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: pgpKeyServer.h,v 1.43 1999/04/12 18:59:57 jason Exp $
____________________________________________________________________________*/

#ifndef Included_pgpKeyServer_h
#define Included_pgpKeyServer_h

#include "pgpOptionList.h"
#include "pgpErrors.h"
#include "pgpGroups.h"
#include "pgpTLS.h"


enum PGPKeyServerState_
{
	kPGPKeyServerState_Invalid						= 0,
	kPGPKeyServerState_Opening						= 1,
	kPGPKeyServerState_Querying						= 2,
	kPGPKeyServerState_ReceivingResults				= 3,
	kPGPKeyServerState_ProcessingResults			= 4,
	kPGPKeyServerState_Uploading					= 5,
	kPGPKeyServerState_Deleting						= 6,
	kPGPKeyServerState_Disabling					= 7,
	kPGPKeyServerState_Closing						= 8,

	kPGPKeyServerState_TLSUnableToSecureConnection	= 9,
	kPGPKeyServerState_TLSConnectionSecured			= 10,

	PGP_ENUM_FORCE(PGPKeyServerState_)
};

PGPENUM_TYPEDEF(PGPKeyServerState_, PGPKeyServerState);

enum PGPKeyServerProtocol_
{
	kPGPKeyServerProtocol_Invalid			= 0,
	kPGPKeyServerProtocol_LDAP				= 1,
	kPGPKeyServerProtocol_HTTP				= 2,
	kPGPKeyServerProtocol_LDAPS				= 3,
	kPGPKeyServerProtocol_HTTPS				= 4,
	
	PGP_ENUM_FORCE( PGPKeyServerProtocol_ )
};

PGPENUM_TYPEDEF( PGPKeyServerProtocol_, PGPKeyServerProtocol );

enum PGPKeyServerClass_
{
	kPGPKeyServerClass_Invalid		= 0,
	kPGPKeyServerClass_PGP			= 1,
	kPGPKeyServerClass_NetToolsCA	= 2,
	kPGPKeyServerClass_Verisign		= 3,
	kPGPKeyServerClass_Entrust		= 4,
	
	PGP_ENUM_FORCE( PGPKeyServerClass_ )
};

PGPENUM_TYPEDEF( PGPKeyServerClass_, PGPKeyServerClass );

enum PGPKeyServerKeySpace_ /* These are only valid for LDAP keyservers */
{
	kPGPKeyServerKeySpace_Invalid	= 0,
	kPGPKeyServerKeySpace_Default	= 1,
	kPGPKeyServerKeySpace_Normal	= 2,
	kPGPKeyServerKeySpace_Pending	= 3,

	PGP_ENUM_FORCE( PGPKeyServerKeySpace_ )
};

PGPENUM_TYPEDEF( PGPKeyServerKeySpace_, PGPKeyServerKeySpace );

enum PGPKeyServerAccessType_ /* These are only valid for LDAP keyservers */
{
	kPGPKeyServerAccessType_Invalid			= 0,
	kPGPKeyServerAccessType_Default			= 1,
	kPGPKeyServerAccessType_Normal			= 2,
	kPGPKeyServerAccessType_Administrator	= 3,

	PGP_ENUM_FORCE( PGPKeyServerAccessType_ )
};

PGPENUM_TYPEDEF( PGPKeyServerAccessType_, PGPKeyServerAccessType );

/*	PGPKeyServerMonitorValues are null terminated linked lists.
	The values member is a null terminated array of char*s.
*/

typedef struct PGPKeyServerMonitorValues
{
	char *								name;
	char **								values;
	struct PGPKeyServerMonitorValues *	next;
} PGPKeyServerMonitorValues;

typedef struct PGPKeyServerMonitor 
{
	PGPKeyServerRef				keyServerRef;
	PGPKeyServerMonitorValues *	valuesHead;
} PGPKeyServerMonitor;


typedef struct PGPKeyServerThreadStorage *	PGPKeyServerThreadStorageRef;
# define kInvalidPGPKeyServerThreadStorageRef				\
			((PGPKeyServerThreadStorageRef) NULL)
#define PGPKeyServerThreadStorageRefIsValid(ref)			\
			((ref) != kInvalidPGPKeyServerThreadStorageRef)

#if PGP_DEPRECATED	/* [ */

#define kPGPKeyServerType_Invalid	kPGPKeyServerProtocol_Invalid
#define kPGPKeyServerType_LDAP		kPGPKeyServerProtocol_LDAP
#define kPGPKeyServerType_HTTP		kPGPKeyServerProtocol_HTTP
#define kPGPKeyServerType_LDAPS		kPGPKeyServerProtocol_LDAPS
#define kPGPKeyServerType_HTTPS		kPGPKeyServerProtocol_HTTPS

typedef PGPKeyServerProtocol		PGPKeyServerType;

#endif	/* ] PGP_DEPRECATED */

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


/*	Use the idle event handler to receive periodic idle events during
	network calls. Usually this is used only in non-preemptive multi-tasking
	OSes to allow yielding in threads. Pre-emptive multi-tasking systems
	should probably not use the call as it interrupts the efficient wait state
	of threads waiting on network calls.
	
	Idle event handlers need to be added on a per thread basis.
	
	Returning an error from the idle event handler will cause the keyserver
	to quit processing and to return a kPGPError_UserAbort. */
PGPError			PGPSetKeyServerIdleEventHandler(
						PGPEventHandlerProcPtr inCallback,
						PGPUserValue inUserData);

PGPError			PGPGetKeyServerIdleEventHandler(
						PGPEventHandlerProcPtr * outCallback,
						PGPUserValue * outUserData);

/* Network library options */

PGPOptionListRef	PGPONetURL(PGPContextRef context, const char *url);

PGPOptionListRef	PGPONetHostName(PGPContextRef context,
							const char *hostName, PGPUInt16 port);

PGPOptionListRef	PGPONetHostAddress(PGPContextRef context,
							PGPUInt32 hostAddress, PGPUInt16 port);

PGPOptionListRef	PGPOKeyServerProtocol(PGPContextRef context,
							PGPKeyServerProtocol serverProtocol);

PGPOptionListRef	PGPOKeyServerKeySpace(PGPContextRef context,
							PGPKeyServerKeySpace serverSpace);

PGPOptionListRef	PGPOKeyServerAccessType(PGPContextRef context,
							PGPKeyServerAccessType accessType);
							
PGPOptionListRef	PGPOKeyServerCAKey(PGPContextRef context,
							PGPKeyRef caKey);

PGPOptionListRef	PGPOKeyServerRequestKey(PGPContextRef context,
							PGPKeyRef requestKey);

PGPOptionListRef	PGPOKeyServerSearchKey(PGPContextRef context,
							PGPKeyRef searchKey);
							
PGPOptionListRef	PGPOKeyServerSearchFilter(PGPContextRef context,
							PGPFilterRef searchFilter);

/* Static storage creation */
PGPError		PGPKeyServerCreateThreadStorage(
					PGPKeyServerThreadStorageRef * outPreviousStorage);
PGPError		PGPKeyServerDisposeThreadStorage(
					PGPKeyServerThreadStorageRef inPreviousStorage);
					
/* Initialize and close the keyserver library */
PGPError			PGPKeyServerInit(void);

PGPError			PGPKeyServerCleanup(void);


/*	Creating and freeing a keyserver ref. */
PGPError			PGPNewKeyServer(
						PGPContextRef 			inContext,
						PGPKeyServerClass		inClass,
						PGPKeyServerRef 		*outKeyServerRef,
						PGPOptionListRef		firstOption,
						... );
						
PGPError 			PGPFreeKeyServer(PGPKeyServerRef inKeyServerRef);
PGPError			PGPIncKeyServerRefCount(PGPKeyServerRef inKeyServerRef);


/*	Set and get the keyserver's event handler. Note that returning an error
	for a keyserver event will abort the current call. */
PGPError			PGPSetKeyServerEventHandler(
						PGPKeyServerRef inKeyServerRef,
						PGPEventHandlerProcPtr inCallback,
						PGPUserValue inUserData);

PGPError			PGPGetKeyServerEventHandler(
						PGPKeyServerRef inKeyServerRef,
						PGPEventHandlerProcPtr * outCallback,
						PGPUserValue * outUserData);
						
						
/*	Canceling a call to a keyserver. This is the only call that can be made
	to a keyserver that is currently in another call. Also, once you have
	returned from a canceled call, you may only close the keyserver. */
PGPError			PGPCancelKeyServerCall(PGPKeyServerRef inKeyServerRef);
						
						
/*	Opening and closing the keyserver. A keyserver ref can be opened and
	closed multiple times as necessary. */
PGPError			PGPKeyServerOpen(PGPKeyServerRef inKeyServerRef,
						PGPtlsSessionRef inTLSSessionRef);

PGPError			PGPKeyServerClose(PGPKeyServerRef inKeyServerRef);


/*	Get keyserver info. */
PGPError			PGPGetKeyServerTLSSession(PGPKeyServerRef inKeyServerRef,
						PGPtlsSessionRef * outTLSSessionRef);

PGPError			PGPGetKeyServerProtocol(PGPKeyServerRef inKeyServerRef,
						PGPKeyServerProtocol * outType);

PGPError			PGPGetKeyServerAccessType(PGPKeyServerRef inKeyServerRef,
						PGPKeyServerAccessType * outAccessType);

PGPError			PGPGetKeyServerKeySpace(PGPKeyServerRef inKeyServerRef,
						PGPKeyServerKeySpace * outKeySpace);

PGPError			PGPGetKeyServerPort(PGPKeyServerRef inKeyServerRef,
						PGPUInt16 * outPort);

PGPError			PGPGetKeyServerHostName(PGPKeyServerRef inKeyServerRef,
						char ** outHostName); /* Use PGPFreeData to free */

PGPError			PGPGetKeyServerAddress(PGPKeyServerRef inKeyServerRef,
						PGPUInt32 * outAddress);
						
PGPError			PGPGetKeyServerPath(PGPKeyServerRef inKeyServerRef,
						char ** outPath); /* Use PGPFreeData to free */

PGPContextRef		PGPGetKeyServerContext(PGPKeyServerRef inKeyServerRef);

/*	If there was an error string returned from the server, you can get it with
	this function. Note that if there is no string, the function will return
	kPGPError_NoErr and *outErrorString will be	NULL */
PGPError			PGPGetLastKeyServerErrorString(
						PGPKeyServerRef inKeyServerRef,
						char ** outErrorString); /* Use PGPFreeData to free */


/* These functions may be used with both HTTP and LDAP keyservers */
PGPError 			PGPQueryKeyServer(PGPKeyServerRef inKeyServerRef, 
						PGPFilterRef inFilterRef, 
						PGPKeySetRef * outFoundKeys);

PGPError 			PGPUploadToKeyServer(PGPKeyServerRef inKeyServerRef, 
						PGPKeySetRef inKeysToUpload, 
						PGPKeySetRef * outKeysThatFailed);
						

/* These functions may only be used with LDAP keyservers */
PGPError			PGPDeleteFromKeyServer(PGPKeyServerRef inKeyServerRef,
						PGPKeySetRef inKeysToDelete,
						PGPKeySetRef * outKeysThatFailed);

PGPError			PGPDisableFromKeyServer(PGPKeyServerRef inKeyServerRef,
						PGPKeySetRef inKeysToDisable,
						PGPKeySetRef * outKeysThatFailed);
						
PGPError			PGPSendGroupsToServer(PGPKeyServerRef inKeyServerRef,
						PGPGroupSetRef inGroupSetRef);
						
PGPError			PGPRetrieveGroupsFromServer(
						PGPKeyServerRef inKeyServerRef,
						PGPGroupSetRef * outGroupSetRef);

PGPError			PGPNewServerMonitor(PGPKeyServerRef inKeyServerRef,
						PGPKeyServerMonitor ** outMonitor);
						
PGPError			PGPFreeServerMonitor(PGPKeyServerMonitor * inMonitor);

/* X.509 Certificate Request functions */

PGPError			PGPSendCertificateRequest( 
						PGPKeyServerRef 	inKeyServerRef,
						PGPOptionListRef	firstOption,
						... );

PGPError			PGPRetrieveCertificate( 
						PGPKeyServerRef 	inKeyServerRef,
						PGPOptionListRef	firstOption,
						... );

PGPError			PGPRetrieveCertificateRevocationList( 
						PGPKeyServerRef 	inKeyServerRef,
						PGPOptionListRef	firstOption,
						... );
												
/***************************************************************************
****************************************************************************
	NOTE: These functions are deprecated and should not be used
	
	PGPGetKeyServerType 			-> PGPGetKeyServerProtocol
	PGPNewKeyServerFromURL			-> PGPNewKeyServer
	PGPNewKeyServerFromHostName		-> PGPNewKeyServer
	PGPNewKeyServerFromHostAddress	-> PGPNewKeyServer
	
****************************************************************************
***************************************************************************/

#if PGP_DEPRECATED	/* [ */

PGPError 			PGPNewKeyServerFromURL(PGPContextRef inContext, 
						const char * inURL, 
						PGPKeyServerAccessType inAccessType,
						PGPKeyServerKeySpace inKeySpace,
						PGPKeyServerRef * outKeyServerRef);

PGPError			PGPNewKeyServerFromHostName(PGPContextRef inContext,
						const char * inHostName,
						PGPUInt16 inPort, /* default for protocol if 0 */
						PGPKeyServerProtocol inType,
						PGPKeyServerAccessType inAccessType,
						PGPKeyServerKeySpace inKeySpace,
						PGPKeyServerRef * outKeyServerRef);
						
PGPError			PGPNewKeyServerFromHostAddress(PGPContextRef inContext,
						PGPUInt32 inAddress,
						PGPUInt16 inPort, /* default for protocol if 0 */
						PGPKeyServerProtocol inType,
						PGPKeyServerAccessType inAccessType,
						PGPKeyServerKeySpace inKeySpace,
						PGPKeyServerRef * outKeyServerRef);

PGPError			PGPGetKeyServerType(PGPKeyServerRef inKeyServerRef,
						PGPKeyServerType * outType);

#endif	/* ] PGP_DEPRECATED */

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif
